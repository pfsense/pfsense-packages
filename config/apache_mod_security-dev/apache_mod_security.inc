<?php
/*
	apache_mod_security.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2009, 2010 Scott Ullrich
	Copyright (C) 2012-2013 Marcello Coutinho
	Copyright (C) 2013 Stephane Lapie <stephane.lapie@asahinet.com>
	Copyright (C) 2015 ESF, LLC
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
require_once("config.inc");
require_once("globals.inc");
require_once("notices.inc");
require_once("service-utils.inc");
require_once("util.inc");
require_once("xmlrpc.inc");
require_once("xmlrpc_client.inc");

$shortcut_section = "apache";
// Check to find out on which system the package is running
$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version == "2.1" || $pf_version == "2.2")
	define('APACHEDIR', '/usr/pbi/proxy_mod_security-' . php_uname("m"));
else
	define('APACHEDIR', '/usr/local');

define('APACHEVERSION', 'apache24');

// End of system check
define ('MODSECURITY_DIR','crs');
// Rules directory location
define("RULES_DIRECTORY", APACHEDIR . "/". MODSECURITY_DIR);
function apache_textarea_decode($base64){
	return preg_replace("/\r\n/","\n",base64_decode($base64));
}

function apache_get_real_interface_address($iface) {
	global $config;
	if ($iface == "All")
		return array("*", "");
	if (preg_match("/\d+\.\d+/",$iface))
		return array($iface, "");
	$iface = convert_friendly_interface_to_real_interface_name($iface);
	$line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
	list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);
	return array($ip, long2ip(hexdec($netmask)));
}

// Ensure NanoBSD can write. pkg_mgr will remount RO
conf_mount_rw();

// Startup function
function apache_mod_security_start() {
	exec(APACHEDIR . "/sbin/httpd -D NOHTTPACCEPT -k start");
}

// Shutdown function
function apache_mod_security_stop() {
	exec(APACHEDIR . "/sbin/httpd -k stop");
}

// Restart function
function apache_mod_security_restart() {
	if(is_process_running("httpd")) {
		exec(APACHEDIR . "/sbin/httpd -k graceful");
	} else {
		apache_mod_security_start();		
	}
}

// Install function
function apache_mod_security_install() {
	global $config, $g;

	// We might be reinstalling and a configuration
	// already exists.
	generate_apache_configuration();

	$filename = "apache_mod_security.sh";

	$start = "/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDPHP
	<?php
		require_once(\"functions.inc\");
		require_once(\"/usr/local/pkg/apache_mod_security.inc\");
		apache_mod_security_start();
	?>
ENDPHP\n";

	$stop = "/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDPHP
	<?php
		require_once(\"functions.inc\");
		require_once(\"/usr/local/pkg/apache_mod_security.inc\");
		apache_mod_security_stop();
	?>
ENDPHP\n";
	
	write_rcfile(array(
			"file" => $filename,
			"start" => $start,
			"stop" => $stop
		)
	);
}

// Deinstall package routines
function apache_mod_security_deinstall() {
	global $config, $g;
	apache_mod_security_stop();
	exec("/bin/rm -rf " . APACHEDIR . "/". MODSECURITY_DIR);
	exec("/bin/rm -f /usr/local/etc/rc.d/apache_mod_security.sh");
}

// Regenerate apache configuration and handle server restart
function apache_mod_security_resync() {
	global $config, $g;
	apache_mod_security_install();
	$dirs=array("base", "experimental","optional", "slr");
	log_error("apache_mod_security_package: configuration resync is starting.");
	if (! file_exists(APACHEDIR ."/". MODSECURITY_DIR . "/LICENSE")){
		exec ("/usr/local/bin/git clone https://github.com/SpiderLabs/owasp-modsecurity-crs.git ".APACHEDIR."/".MODSECURITY_DIR);
		//chdir (APACHEDIR."/".MODSECURITY_DIR);
		//exec ("/usr/local/bin/git checkout -q 2.2.8");
	}
	$write_config=0;
	foreach ($dirs as $dir){
		if ($handle = opendir(APACHEDIR ."/".MODSECURITY_DIR."/{$dir}_rules")) {
			$write_config++;
			$config['installedpackages']["modsecurityfiles{$dir}"]['config']=array();
			while (false !== ($entry = readdir($handle))) {
				if (preg_match("/(\S+).conf$/",$entry,$matches))
					$config["installedpackages"]["modsecurityfiles{$dir}"]["config"][]=array("file"=>$matches[1]);
			}
			closedir($handle);
		}
	}
	if ($write_config > 0)
		write_config();
	apache_mod_security_checkconfig();
	apache_mod_security_restart();
	log_error("apache_mod_security_package: configuration resync is ending.");

	if (is_array($config['installedpackages']['apachesync']['config'])){
		$apache_sync = $config['installedpackages']['apachesync']['config'][0];
		$synconchanges = $apache_sync['synconchanges'];
		$synctimeout = $apache_sync['synctimeout'] ?: '250';
		switch ($synconchanges) {
			case "manual":
				if (is_array($apache_sync['row'])) {
					$rs = $apache_sync['row'];
				} else {
					log_error("apache_mod_security_package:  XMLRPC sync is enabled but there are no hosts configured as replication targets.");
					return;
				}
				break;
			case "auto":
				if (is_array($config['hasync'])) {
					$system_carp = $config['hasync'];
					$rs[0]['ipaddress'] = $system_carp['synchronizetoip'];
					$rs[0]['username'] = $system_carp['username'];
					$rs[0]['password'] = $system_carp['password'];
					$rs[0]['syncdestinenable'] = FALSE;

					// XMLRPC sync is currently only supported over connections using the same protocol and port as this system
					if ($config['system']['webgui']['protocol'] == "http") {
						$rs[0]['syncprotocol'] = "http";
						$rs[0]['syncport'] = $config['system']['webgui']['port'] ?: '80';
					} else {
						$rs[0]['syncprotocol'] = "https";
						$rs[0]['syncport'] = $config['system']['webgui']['port'] ?: '443';
					}
					if ($system_carp['synchronizetoip'] == "") {
						log_error("apache_mod_security_package: XMLRPC CARP/HA sync is enabled but there are no system backup hosts configured as replication targets.");
						return;
					} else {
						$rs[0]['syncdestinenable'] = TRUE;
					}
				} else {
					log_error("apache_mod_security_package: XMLRPC sync is enabled, but there is no global backup host to push apache config.");
					return;
				}
				break;
			default:
				return;
				break;
		}
		if (is_array($rs)) {
			log_error("apache_mod_security_package: XMLRPC sync is starting.");
			foreach ($rs as $sh) {
				// Only sync enabled replication targets
				if ($sh['syncdestinenable']) {
					$sync_to_ip = $sh['ipaddress'];
					$port = $sh['syncport'];
					$username = $sh['username'] ?: 'admin';
					$password = $sh['password'];
					$protocol = $sh['syncprotocol'];

					$error = '';
					$valid = TRUE;

					if ($password == "") {
						$error = "Password parameter is empty. ";
						$valid = FALSE;
					}
					if (!is_ipaddr($sync_to_ip) && !is_hostname($sync_to_ip) && !is_domain($sync_to_ip)) {
						$error .= "Misconfigured Replication Target IP Address or Hostname. ";
						$valid = FALSE;
					}
					if (!is_port($port)) {
						$error .= "Misconfigured Replication Target Port. ";
						$valid = FALSE;
					}
					if ($valid) {
						apache_mod_security_do_xmlrpc_sync($sync_to_ip, $port, $protocol, $username, $password, $synctimeout);
					} else {
						log_error("apache_mod_security_package: XMLRPC sync with '{$sync_to_ip}' aborted due to the following error(s): {$error}");
					}
				}
			}
			log_error("apache_mod_security_package: XMLRPC sync completed.");
		}
	}
}

// Do the actual XMLRPC Sync
function apache_mod_security_do_xmlrpc_sync($sync_to_ip, $port, $protocol, $username, $password, $synctimeout) {
        global $config, $g;

	if ($username == "" || $password == "" || $sync_to_ip == "" || $port == "" || $protocol == "") {
		log_error("apache_mod_security_package: A required XMLRPC sync parameter (username, password, replication target, port or protocol) is empty ... aborting pkg sync");
		return;
	}

	// Take care of IPv6 literal address
	if (is_ipaddrv6($sync_to_ip)) {
		$sync_to_ip = "[{$sync_to_ip}]";
	}

	$url = "{$protocol}://{$sync_to_ip}";

	/* XML will hold the sections to sync. */
	$xml = array();
	$xml['apachesettings'] = $config['installedpackages']['apachesettings'];
	$xml['apachemodsecurity'] = $config['installedpackages']['apachemodsecurity'];
	$xml['apachemodsecuritysettings'] = $config['installedpackages']['apachemodsecuritysettings'];
	$xml['apachebalancer'] = $config['installedpackages']['apachebalancer'];
	$xml['apachelocation'] = $config['installedpackages']['apachelocation'];
	$xml['apachevirtualhost'] = $config['installedpackages']['apachevirtualhost'];
	$xml['apachelisten'] = $config['installedpackages']['apachelisten'];

	/* Assemble XMLRPC payload. */
	$params = array(XML_RPC_encode($password), XML_RPC_encode($xml));

	/* Set a few variables needed for sync code */
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if ($g['debug']) {
		$cli->setDebug(1);
	}
	/* Send our XMLRPC message and timeout after defined sync timeout value */
	$resp = $cli->send($msg, $synctimeout);
	if (!$resp) {
		$error = "A communications error occurred while attempting XMLRPC sync with {$url}:{$port}.";
		log_error("apache_mod_security_package: {$error}");
		file_notice("sync_settings", $error, "apache_mod_security Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("apache_mod_security_package: {$error}");
		file_notice("sync_settings", $error, "apache_mod_security Settings Sync", "");
	} else {
		log_error("apache_mod_security_package: XMLRPC sync successfully completed with {$url}:{$port}.");
	}

	/* Tell apache_mod_security to reload our settings on the destination sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/apache_mod_security.inc');\n";
	$execcmd .= "apache_mod_security_resync();";
	/* Assemble XMLRPC payload. */
	$params = array(XML_RPC_encode($password), XML_RPC_encode($execcmd));

	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, $synctimeout);
	if (!$resp) {
		$error = "A communications error occurred while attempting XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error("apache_mod_security_package: {$error}");
		file_notice("sync_settings", $error, "apache_mod_security Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("apache_mod_security_package: {$error}");
		file_notice("sync_settings", $error, "apache_mod_security Settings Sync", "");
	} else {
		log_error("apache_mod_security_package: XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}

}

function apache_mod_security_checkconfig() {
	global $config, $g;
	$status = mwexec(APACHEDIR ."/sbin/httpd -t");
	if($status) {
		$input_error = "apache_mod_security_package: There was an error parsing the Apache configuration: {$status}";
		log_error("apache_mod_security_package: There was an error parsing the Apache configuration: {$status}");
	}
}

// Generate mod_proxy specific configuration
function generate_apache_configuration() {
	global $config, $g;
	$mod_proxy = "";
	$write_config=0;
	// check current config
	if (is_array($config['installedpackages']['apachesettings']))
		$settings=$config['installedpackages']['apachesettings']['config'][0];
	else
		$settings=array();

	log_error("apache_mod_security_package: Re-generating Apache configuration start.");

	// Set global site e-mail
	if ($settings['globalsiteadminemail']){
		$global_site_email = $settings['globalsiteadminemail'];
		} 
	else {
		$global_site_email = "admin@admin.com";
		$config['installedpackages']['apachesettings']['config'][0]['globalsiteadminemail'] = "admin@admin.com";
		// update configuration with default value in this case
		log_error("apache_mod_security_package: WARNING!  Global site Administrator E-Mail address has not been set.  Defaulting to bogus e-mail address.");
		$write_config ++;
		}

	// Set ServerName
	if($settings['hostname'] != ""){
		$servername = $settings['hostname'];
		}
	else {
		$servername = php_uname('n');
		$config['installedpackages']['apachesettings']['config'][0]['hostname'] = `hostname`;
		// update configuration with default value in this case
		$write_config ++;
		}

	//check if servername has an valid ip
	$ip=gethostbyname(php_uname('n'));
	if ($ip==php_uname('n')){
		$error='apache_mod_security_package: Apache cannot start, hostname does not resolv. You can workaround this if you add a dns entry for '.php_uname('n').' or add a Host Overrides entry on services -> Dns Forwarder pointing '.php_uname('n').' to 127.0.0.1.';
		log_error($error);
		file_notice("apache_mod_security", $error, "apache_mod_security", "");
	}
	// Set global listening directive and ensure nothing is listening on this port already
	$iface_address = apache_get_real_interface_address($settings['globalbindtoipaddr']);
	$ip=$iface_address[0];
	$globalbind_ip = ($ip ? $ip : "*");
	$globalbind_port = $settings['globalbindtoport'];
	if ($globalbind_port == ""){
		$globalbind_port ="80";
		$config['installedpackages']['apachesettings']['config'][0]['globalbindtoipport'] = $globalbind_port;
		$write_config ++;
		}
	$global_listen ="{$globalbind_ip}:{$globalbind_port}";
	// update configuration with default value in this case
	if ($write_config > 0)
		write_config();
	
	// check if any daemon is using apache ip/port
	exec("/usr/bin/sockstat | grep -v ' httpd ' | awk '{ print $6 }' | grep ':{$globalbind_port}'",$socksstat);
	unset ($already_binded);
	if(is_array($socksstat)) {
		foreach($socksstat as $ss) {
			list($ss_ip,$ss_port)=explode(":",$ss);
			#check if port is in use
			if($ss_port == $globalbind_port) {
				#check if it's the same ip or any ip
				if ($globalbind_ip = "*" || $globalbind_ip == $ss_ip)
					$already_binded = true;
				$input_errors[] = "Sorry, there is a process already listening on port {$globalbind}";
				}
			}
		}
	if(isset($already_binded))
		log_error("apache_mod_security_package: Could not start daemon on port {$global_listen}. Another process is already bound to this port.");
	
	//performance settings
	//reference http://httpd.apache.org/docs/2.2/mod/mpm_common.html
	$keepalive=($settings['keepalive']?$settings['keepalive']:"on");
	$performance_settings="KeepAlive {$keepalive}\n";
	if ($settings['maxkeepalivereq'])
		$performance_settings .= "MaxKeepAliveRequests {$settings['maxkeepalivereq']}\n";
	if ($settings['keepalivetimeout'])
		$performance_settings .= "KeepAliveTimeout {$settings['keepalivetimeout']}\n";
	if ($settings['serverlimit'])
		$performance_settings .= "ServerLimit {$settings['serverlimit']}\n";
	if ($settings['startservers'])
		$performance_settings .= "StartServers {$settings['startservers']}\n";
	if ($settings['minsparethreads'])
		$performance_settings .= "MinSpareThreads {$settings['minsparethreads']}\n";
	if ($settings['maxsparethreads'])
		$performance_settings .= "MaxSpareThreads {$settings['maxsparethreads']}\n";
	if ($settings['threadslimit'])
		$performance_settings .= "ThreadsLimit {$settings['threadslimit']}\n";
	if ($settings['threadstacksize'])
		$performance_settings .= "ThreadStackSize {$settings['threadstacksize']}\n";
	if ($settings['threadsperchild'])
		$performance_settings .= "ThreadsPerChild {$settings['threadsperchild']}\n";
	if ($settings['maxclients'])
		$performance_settings .= "MaxClients {$settings['maxclients']}\n";
	if ($settings['maxrequestsperchild'])
		$performance_settings .= "MaxRequestsPerChild {$settings['maxrequestsperchild']}\n";	
	
	// Setup mem_cache
	if(file_exists(APACHEDIR ."/libexec/" . APACHEVERSION . "/mod_memcache.so") && $settings['memcachesize'] != "0") {
		//$mem_cache = "MCacheSize ".( $settings['memcachesize'] ? $settings['memcachesize'] : "100")."\n";
		}
		
	// CacheRoot Directive
	if($settings['diskcachesize'] != "0") {
		safe_mkdir("/var/db/apachemodsecuritycache");
		$cache_root .= "CacheRoot /var/db/apachemodsecuritycache\n";
		$cache_root .= "CacheMaxFileSize ".($settings['diskcachesize'] ? $settings['diskcachesize'] : "1000000")."\n";
	}

	// SecRequestBodyInMemoryLimit Directive
	$secrequestbodyinmemorylimit = ($settings['secrequestbodyinmemorylimit'] ? $settings['secrequestbodyinmemorylimit'] : "131072");

	// SecRequestBodyLimit
	$secrequestbodylimit = ($settings['secrequestbodylimit'] ? $settings['secrequestbodylimit'] :"10485760");

	// ErrorDocument
	$errordocument = ($settings['errordocument'] ? $settings['errordocument'] : "");

	// SecAuditEngine
	$secauditengine = ($settings['secauditengine'] ? $settings['secauditengine'] : "RelevantOnly");

	// SecReadStateLimit
	$secreadstatelimit = ($settings['SecReadStateLimit'] ? $settings['SecReadStateLimit'] :"");
	
	//Configure balancers/backends
	if (is_array($config['installedpackages']['apachebalancer'])){
		#load conf template
		include("/usr/local/pkg/apache_balancer.template");

		//write balancer conf
		file_put_contents(APACHEDIR."/etc/" . APACHEVERSION . "/Includes/balancers.conf",$balancer_config,LOCK_EX);
	}

	// configure modsecurity group options
	//chroot apache http://forums.freebsd.org/showthread.php?t=6858
	if (is_array($config['installedpackages']['apachemodsecuritygroups'])){
		unset($mods_group);
		foreach ($config['installedpackages']['apachemodsecuritygroups']['config'] as $mods_groups){
			//RULES_DIRECTORY
			foreach (split(",",$mods_groups['baserules']) as $baserule){
				$mods_group[$mods_groups['name']].="  Include ".RULES_DIRECTORY ."/base_rules/{$baserule}.conf\n";
			}
			foreach (split(",",$mods_groups['optionalrules']) as $baserule){
				$mods_group[$mods_groups['name']].="  Include ".RULES_DIRECTORY ."/optional_rules/{$baserule}.conf\n";
			}
			foreach (split(",",$mods_groups['slrrules']) as $baserule){
				$mods_group[$mods_groups['name']].="  Include ".RULES_DIRECTORY ."/slr_rules/{$baserule}.conf\n";
			}
			foreach (split(",",$mods_groups['experimentalrules']) as $baserule){
				$mods_group[$mods_groups['name']].="  Include ".RULES_DIRECTORY ."/experimental_rules/{$baserule}.conf\n";
			}
		}
	}
	//print "<PRE>";
	//var_dump($mods_group);

	//mod_security settings
	if (is_array($config['installedpackages']['apachemodsecuritysettings'])){
		$mods_settings=$config['installedpackages']['apachemodsecuritysettings']['config'][0];
	
		if ($mods_settings['crs10']=="" && file_exists(RULES_DIRECTORY .'/modsecurity_crs_10_setup.conf.example')){
			$config['installedpackages']['apachemodsecuritysettings']['config'][0]['crs10']=base64_encode(file_get_contents(RULES_DIRECTORY .'/modsecurity_crs_10_setup.conf.example'));
			write_config("modsecurity - Load crs 10 default setup file.");
			}
			
		$cr10_setup="Include ".RULES_DIRECTORY ."/modsecurity_crs_10_setup.conf\n";
		file_put_contents(RULES_DIRECTORY ."/modsecurity_crs_10_setup.conf",apache_textarea_decode($config['installedpackages']['apachemodsecuritygroups']['config'][0]['crs10']),LOCK_EX);
		}
	// create location(s) array
	if (is_array($config['installedpackages']['apachelocation'])){
		foreach ($config['installedpackages']['apachelocation']['config'] as $location)
			$apache_location[$location['name']]=$location;
		}
	//configure virtual hosts
	$namevirtualhosts=array();
	$namevirtualhosts[0]=$global_listen;
	if (is_array($config['installedpackages']['apachevirtualhost'])){
		$vh_config= <<<EOF
##################################################################################
# NOTE: This file was generated by the pfSense package management system.        #
#       Please do not edit this file by hand!   If you need to add functionality #
#       then edit /usr/local/pkg/apache_* files.                                 #
#                                                                                #
#       And don't forget to submit your changes to:                              #
#       https://github.com/pfsense/pfsense-packages                              #
##################################################################################


EOF;
		$default_port=array('http'=>'80', 'https'=> '443');
		foreach ($config['installedpackages']['apachevirtualhost']['config'] as $virtualhost){
			if (is_array($virtualhost['row']) && $virtualhost['enable'] == 'on'){
				$iface_address = apache_get_real_interface_address($virtualhost['interface']);
				$ip=$iface_address[0];
				$port=($virtualhost['port'] ? $virtualhost['port'] : $default_port[$virtualhost['proto']]);
				if (!in_array("{$ip}:{$port}",$namevirtualhosts))
					$namevirtualhosts[]="{$ip}:{$port}";

				$vh_config.="# {$virtualhost['description']}\n";
				$vh_config.="<VirtualHost {$ip}:{$port}>\n";
				$vh_config.=" ServerName ". preg_replace ("/\r\n(\S+)/","\n ServerAlias $1",base64_decode($virtualhost['primarysitehostname'])) ."\n";
				$vh_config.=" ServerAdmin ".($virtualhost['siteemail'] ? $virtualhost['siteemail'] : $settings['globalsiteadminemail'])."\n";
				#check log
				switch ($virtualhost['logfile']){
					case "default":
						$vh_config.=" ErrorLog /var/log/httpd-error.log\n";
						$vh_config.=" CustomLog /var/log/httpd.log combined\n"; 
					break;
					case "create":
						if(preg_match("/(\S+)/",base64_decode($virtualhost['primarysitehostname']),$matches))
						$vh_config.=" ErrorLog /var/log/httpd-{$matches[1]}-error.log\n";
						$vh_config.=" CustomLog /var/log/httpd-{$matches[1]}.log combined\n"; 
					break;
				}

				if($virtualhost['preserveproxyhostname'])
					$vh_config .= " ProxyPreserveHost on\n";

				#check ssl
				if(isset($virtualhost["ssl_cert"]) && $virtualhost["ssl_cert"] !="none" && $virtualhost["proto"] == "https") {
					$vh_config.= " SSLEngine on\n";
					$vh_config.= " SSLProtocol ". ($virtualhost['ssl_protocol'] ? $virtualhost['ssl_protocol'] : "all -SSLv2") ."\n";
					$vh_config.= " SSLProxyEngine ". ($virtualhost['ssl_proxy_engine'] ? "on" : "off") ."\n";
					$vh_config.= " SSLProxyVerify ". ($virtualhost['ssl_proxy_verify'] ? $virtualhost['ssl_proxy_verify'] : "none") ."\n";
					$vh_config.= " SSLCipherSuite ". ($virtualhost['ssl_cipher_suite'] ? $virtualhost['ssl_cipher_suite'] : "ALL:!ADH:!EXPORT56:RC4+RSA:+HIGH:+MEDIUM:+LOW:+SSLv2:+EXP:+eNULL") ."\n";
					$vh_config.= " SSLHonorCipherOrder ". ($virtualhost['ssl_honor_cipher_order'] ? "on" : "off") ."\n";
					
					$svr_cert = lookup_cert($virtualhost["ssl_cert"]);
					if ($svr_cert != false) {
						if(base64_decode($svr_cert['crt'])) {
							file_put_contents(APACHEDIR . "/etc/" . APACHEVERSION . "/{$virtualhost["ssl_cert"]}.crt",apache_textarea_decode($svr_cert['crt']),LOCK_EX);
							$vh_config.= " SSLCertificateFile ". APACHEDIR . "/etc/" . APACHEVERSION . "/{$virtualhost["ssl_cert"]}.crt\n";
							}
						if(base64_decode($svr_cert['prv'])) {
							file_put_contents(APACHEDIR . "/etc/" . APACHEVERSION . "/{$virtualhost["ssl_cert"]}.key",apache_textarea_decode($svr_cert['prv']),LOCK_EX);
							$vh_config.= " SSLCertificateKeyFile ". APACHEDIR . "/etc/" . APACHEVERSION . "/{$virtualhost["ssl_cert"]}.key\n";
			       			}
						}
					$svr_ca =lookup_ca($virtualhost["ssl_cert_chain"]);
					if ($svr_ca != false) {
							file_put_contents(APACHEDIR . "/etc/" . APACHEVERSION . "/{$virtualhost["ssl_cert_chain"]}.crt",apache_textarea_decode($svr_ca['crt']),LOCK_EX);
							$vh_config.= " SSLCertificateChainFile ". APACHEDIR . "/etc/" . APACHEVERSION . "/{$virtualhost["ssl_cert_chain"]}.crt\n";
							}
					$cli_ca =lookup_ca($virtualhost["reverse_int_ca"]);
					if ($cli_ca != false) {
							file_put_contents(APACHEDIR . "/etc/" . APACHEVERSION . "/{$virtualhost["reverse_int_ca"]}.crt",apache_textarea_decode($cli_ca['crt']),LOCK_EX);
							$vh_config.= " SSLCACertificateFile ". APACHEDIR . "/etc/" . APACHEVERSION . "/{$virtualhost["reverse_int_ca"]}.crt\n";
							}
				}
				#Custom Options
				$vh_config.= apache_textarea_decode($virtualhost['custom'])."\n\n";

				#Check virtualhost locations
				foreach ($virtualhost['row'] as $be){
					if ($be['location'] != "none"){
						$backend=$apache_location[$be['location']];

						$vh_config.="# {$backend['name']}\n";
						foreach ($config['installedpackages']['apachebalancer']['config'] as $balancer){
							if (is_array($balancer['row']) && $balancer['enable'] == 'on' && $balancer['name'] == $backend['balancer']){
								$vh_config.="# {$balancer['description']}\n";
								$vh_config.=" <Proxy balancer://{$balancer['name']}>\n";
								foreach($balancer['row'] as $balancer_server){
									$balancer_options =($balancer_server['port'] ? ":{$balancer_server['port']}" : "");

									$balancer_options.=($balancer_server['routeid'] ? " route={$balancer_server['routeid']}" : "");
									$balancer_options.=($balancer_server['loadfactor'] ? " loadfactor={$balancer_server['loadfactor']}" : "");
									if (isset($balancer_server['ping']) && $balancer_server['ping']!=""){
										$balancer_options.= " ping={$balancer_server['ping']}";
										$balancer_options.=($server['ttl'] ? " ttl={$balancer_server['ttl']}" : "");
									}
									$vh_config.="  BalancerMember {$balancer['proto']}://{$balancer_server['host']}{$balancer_options}\n";
								}
								#check if stick connections are set
								if ($balancer['row'][0]['routeid'] !="")
									$vh_config.="  ProxySet stickysession=ROUTEID\n";
								$vh_config.=" </Proxy>\n\n";
								break;
							}
						}
	
						$vh_config.=" <Location ".($backend['sitepath'] ? $backend['sitepath'] : "/").">\n";
						$vh_config.="  ProxyPass        balancer://{$backend['balancer']}".($backend['backendpath'] ? $backend['backendpath'] : "/")."\n";
						$vh_config.="  ProxyPassReverse balancer://{$backend['balancer']}".($backend['backendpath'] ? $backend['backendpath'] : "/")."\n";
						if ($backend['compress']== "no")
							$vh_config.="  SetInputFilter   INFLATE\n  SetOutputFilter  INFLATE\n";
						if ($backend['modsecgroup']!="" && $backend['modsecgroup']!="none" && $mods_settings['enablemodsecurity']=="on"){
							$vh_config.=$mods_group[$backend['modsecgroup']];
						}
						if (is_array($config['installedpackages']['apachemodsecuritymanipulation']) && $mods_settings['enablemodsecurity']=="on"){
							foreach($config['installedpackages']['apachemodsecuritymanipulation']['config'] as $manipulation){
								if ($backend['modsecmanipulation'] == $manipulation['name']){
									if (is_array($manipulation['row']))
										foreach ($manipulation['row'] as $secrule)
											$vh_config.="  {$secrule['type']} {$secrule['value']}\n";
									}
								}
							}
						$vh_config.= apache_textarea_decode($backend['custom'])."\n\n";
						$vh_config.=" </Location>\n\n";
					}
				}
				$vh_config.="</VirtualHost>\n";
				}
			}
		//write balancer conf
		file_put_contents(APACHEDIR."/etc/" . APACHEVERSION . "/Includes/virtualhosts.conf",$vh_config,LOCK_EX);
		}
	// check/fix perl version on mod_security util files
	$perl_files= array("httpd-guardian.pl","rules-updater.pl","runav.pl","arachni2modsec.pl","zap2modsec.pl","regression_tests/rulestest.pl");
	foreach ($perl_files as $perl_file){
		$file_path=RULES_DIRECTORY."/util/";
		if (file_exists($file_path.$perl_file)){
			$script=preg_replace("/#!\S+perl/","#!".APACHEDIR."/bin/perl",file_get_contents($file_path.$perl_file));
			file_put_contents($file_path.$perl_file,$script,LOCK_EX);
			}
		}
	// check/fix spread libs location
	$perl_libs= array("libspread.a","libspread.so.1");
	foreach ($perl_libs as $perl_lib){
		$file_path=APACHEDIR."/lib/";
		if (!file_exists("/lib/".$perl_lib) && file_exists("{$file_path}{$perl_lib}")){
			copy("{$file_path}{$perl_lib}","/lib/{$perl_lib}");
			if ($perl_lib == "libspread.so.1")
				copy("{$file_path}{$perl_lib}","/lib/libspread.so");
			}
	}
	
	if ($mods_settings!="")
		$SecGuardianLog="SecGuardianLog \"|".RULES_DIRECTORY."/util/httpd-guardian\"";
	
	//fix http-guardian.pl block bins
	//$file_path=APACHEDIR.MODSECURITY_DIR."/util/".$perl_lib;
	//if (file_exists("/lib/".$perl_lib) && file_exists($file_path.$perl_lib)){
	
	//old code	
	$mod_proxy .= <<<EOF

# Off when using ProxyPass
ProxyRequests off

<Proxy *>
	Order deny,allow
	Allow from all
</Proxy>

EOF;

	/*  
		#####################################################
		# Format for the Proxy servers:
		# Please do not delete these from the source file
		# in any "cleanups" that you feel you are performing.
		# They are here for sanity checking the code.
		#----------------1 backend ip---------------------
		#<VirtualHost IP:port>
		#  ServerAdmin $serveradmin
		#  ServerName $primarysitehostname
		#  ServerAlias $additionalsitehostnames
		#  ProxyPass / $backendwebserverURL
		#  ProxyPassReverse / $backendwebserverURL
		#</VirtualHost>
		#where serveralias will be a space-separated list of "additional site hostnames"
		#i.e. if you have www1.example.com and www2.example.com there, it's "ServerAlias www1.example.com www2.example.com"
		#-------------------------------------------------
		#------------mutliple backend ips-----------------
		# Add:
		#<Proxy balancer://$sitename>
		#  BalancerMember $backend1
		#  BalancerMember $backend2
		#</Proxy>
		# Change:
		# ProxyPass / balancer://$sitename/
		# ProxyPassReverse / balancer://$sitename/
		#-------------------------------------------------
		#####################################################
	*/
	$mod_proxy .= "\n";

	$configuredaliases = array();
	// Read already configured addresses
	if (is_array($settings['row'])){
		foreach($settings['row'] as $row) {
			if ($row['interface'] && $row['ipport'])
				$configuredaliases[] = $row; 
		}
	}
		
	// clear list of bound addresses before updating
	$config['installedpackages']['apachesettings']['config'][0]['row'] = array();
	// Process proxy sites
	// Configure NameVirtualHost directives
	$aliases = "";
	//add NameVirtualHost and listening entries to configured virtualhosts 
	foreach ($namevirtualhosts as $namevirtualhost){
		// explicit bind if not global ip:port
		if ($namevirtualhost != $global_listen) {
			$aliases .= "Listen $namevirtualhost\n";
			// Automatically add this to configuration
			$aplisten=split(":",$namevirtualhost);
			$config['installedpackages']['apachesettings']['config'][0]['row'][] = array('ipaddress' => $aplisten[0], 'ipport' => $aplisten[1]);
		}
	}
	// Process Status Page 
	$mod_status = "";
	if ($settings['statuspage'] == "on") {
		if($settings['extendedstatuspage']== "on"){
			$extendedstatus="ExtendedStatus On";
		}
		$mod_status .= <<<EOF
{$extendedstatus}
<Location /server-status>
	SetHandler server-status
	Order Deny,Allow
	Deny from all

EOF;
		$mod_status .= "Allow from ".($settings['netaccessstatus'] ? $settings['netaccessstatus'] : "All")."\n";
		$mod_status .= "</Location>\n";
	}

	// update configuration with actual ip bindings
	write_config($pkg['addedit_string']);


	// Setup mod_proxy entries $mod_proxy
	if($config['installedpackages']['apachemodsecurity']) {
		foreach($config['installedpackages']['apachemodsecurity']['config'] as $ams) {
			// Set rowhelper used variables
			$additionalsitehostnames = "";
			if (is_array($ams['row'])){
				foreach($ams['row'] as $row) {
					if ($row['additionalsitehostnames'])
						$additionalsitehostnames .= "{$row['additionalsitehostnames']} ";
					}
				}
			$backend_sites = ""; 
			$sslproxyengine = ""; 
			$backend_sites_count = 0;
			$balancer_members = ""; // not technically needed.
			if (is_array($ams['row'])){
			  foreach($ams['row'] as $row) {
				if ($row['webserveripaddr']) {
					$normalised_ipaddr = "";
					if (substr(trim($row['webserveripaddr']), 0, strlen("https:")) == "https:") {
						// if backend is https, then enable SSLProxyEngine
						$sslproxyengine = "SSLProxyEngine on";
					} else if (substr(trim($row['webserveripaddr']), 0, strlen("http:")) != "http:") {
				// Ensure leading http(s)://
						$normalised_ipaddr .= "http://";
					}
					$normalised_ipaddr .=  trim($row['webserveripaddr']);				
					$balancer_members .= " BalancerMember " . $normalised_ipaddr . "\n";
				// Ensure trailing /
					if(substr($normalised_ipaddr,-1) != "/") {
						$normalised_ipaddr .= "/";
					}
					$backend_sites .= $normalised_ipaddr . " ";
                                        $backend_sites_count++;
				}
			  }
			}
			// Set general items
			if($ams['siteemail'])
				$serveradmin = $ams['siteemail'];
			else 
				$serveradmin = $global_site_email;
			if($ams['primarysitehostname'])
				$primarysitehostname = $ams['primarysitehostname'];
			$sitename = str_replace(" ", "", $ams['sitename']);			
			// Set local listening directive
			if($ams['ipaddress'] && $ams['port']) 
				$local_ip_port = "{$ams['ipaddress']}:{$ams['port']}";
			else 
				$local_ip_port = $global_listen;
			// Is this item a load balancer
			if($backend_sites_count>1) {
				$balancer = true;
				$mod_proxy .= "<Proxy balancer://{$sitename}>\n";
				$mod_proxy .= $balancer_members;
				$mod_proxy .= "</Proxy>\n";
				$backend_sites = " balancer://{$sitename}/";
				$sitename = "";			// we are not using sitename in this case
			}
			// Set SSL items
			if($ams['siteurl'])
				$siteurl = $ams['siteurl'];
			if($ams['certificatefile'])
				$certificatefile = $ams['certificatefile'];
			if($ams['certificatekeyfile'])
				$certificatekeyfile = $ams['certificatekeyfile'];
			if($ams['certificatechainfile'])
				$certificatechainfile = $ams['certificatechainfile'];
			// Begin VirtualHost
			$mod_proxy .= "\n<VirtualHost {$local_ip_port}>\n";
			if($siteurl == "HTTPS" && $certificatefile && $certificatekeyfile) {
				$mod_proxy .= "  SSLEngine on\n";
				if ($certificatefile)
					$mod_proxy .= "  SSLCertificateFile /usr/local/etc/" . APACHEVERSION . "/$certificatefile\n";  
				if ($certificatekeyfile)
					$mod_proxy .= "  SSLCertificateKeyFile /usr/local/etc/" . APACHEVERSION . "/$certificatekeyfile\n";
				if ($certificatechainfile)
					$mod_proxy .= "  SSLCertificateChainFile /usr/local/etc/" . APACHEVERSION . "/$certificatechainfile\n";
			}
			if($sslproxyengine)
				$mod_proxy .= "  {$sslproxyengine}\n";
			if($additionalsitehostnames)
				$mod_proxy .= "  ServerAlias $additionalsitehostnames\n";
			if($serveradmin)
				$mod_proxy .= "  ServerAdmin $serveradmin\n";
			if($primarysitehostname)
				$mod_proxy .= "  ServerName $primarysitehostname \n";						
			if($backend_sites) {
				$mod_proxy .= "  ProxyPassReverse /{$sitename}  {$backend_sites}\n";
				$mod_proxy .= "  ProxyPass / {$backend_sites}\n";
			}
			if($ams['preserveproxyhostname']) 
				$mod_proxy .= "  ProxyPreserveHost on\n";
			$mod_proxy .= "</VirtualHost>\n\n";
			// End VirtualHost			
		}
	}

	if($config['installedpackages']['apachesettings']['config'][0]['modsecuritycustom'])  
		$mod_security_custom = $config['installedpackages']['apachesettings']['config'][0]['modsecuritycustom'];

	// Process and include rules
	if(is_dir(RULES_DIRECTORY)) {
		$mod_security_rules = "";
		$files = return_dir_as_array(RULES_DIRECTORY);
		foreach($files as $file) { 
			if(file_exists(RULES_DIRECTORY . "/" . $file)) {
				// XXX: TODO integrate snorts rule on / off thingie
				$file_txt = file_get_contents(RULES_DIRECTORY . "/" . $file);
				$mod_security_rules .= $file_txt . "\n";
			}
		}
	}

	#include file templates
	include ("/usr/local/pkg/apache_mod_security.template");
	include ("/usr/local/pkg/". APACHEVERSION .".template");

	file_put_contents(APACHEDIR . "/etc/" . APACHEVERSION . "/httpd.conf",$apache_config,LOCK_EX);

	log_error("apache_mod_security_package: Re-generating Apache configuration ending.");
}

?>
