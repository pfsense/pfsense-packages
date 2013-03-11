<?php
/*
	apache_mod_security.inc
	part of apache_mod_security package (http://www.pfSense.com)
	Copyright (C) 2009, 2010 Scott Ullrich
	Copyright (C) 2012 Marcello Coutinho
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code MUST retain the above copyright notice,
	   this list of conditions and the following disclaimer.

	2. Redistributions in binary form MUST reproduce the above copyright
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

// Check to find out on which system the package is running
$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version > 2.0)
	define('APACHEDIR', '/usr/pbi/proxy_mod_security-' . php_uname("m"));
else
  define('APACHEDIR', '/usr/local');
// End of system check
define ('MODSECURITY_DIR','modsecurity-crs_2.2.5');
// Rules directory location
define("rules_directory", APACHEDIR . "/". MODSECURITY_DIR);
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

// Needed mod_security directories
if(!is_dir(APACHEDIR . "/". MODSECURITY_DIR)) 
	safe_mkdir(APACHEDIR . "/". MODSECURITY_DIR);

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
	if (! file_exists(APACHEDIR ."/". MODSECURITY_DIR . "/LICENSE"))
		exec ("tar -xzf /usr/local/pkg/modsecurity-crs_2.2.5.tar.gz -C ".APACHEDIR);
	$write_config=0;
	foreach ($dirs as $dir){
		if ($handle = opendir(APACHEDIR ."/".MODSECURITY_DIR."/{$dir}_rules")) {
		$write_config++;
		$config['installedpackages']["modsecurityfiles{$dir}"]['config']=array();
	    while (false !== ($entry = readdir($handle))) {
    	    if (preg_match("/(\S+).conf/",$entry,$matches))
    	    	$config["installedpackages"]["modsecurityfiles{$dir}"]["config"][]=array("file"=>$matches[1]);
    		}
    	closedir($handle);
		}
	}
	if ($write_config > 0)
		write_config();
	apache_mod_security_checkconfig();
	apache_mod_security_restart();
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
		$setting=sarray();

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
	$globalbind_ip = ($settings['globalbindtoipaddr'] ? $settings['globalbindtoipaddr'] : "*");
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
	$performance_settings="KeepAlive {$settings['keepalive']}\n";
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
	if(file_exists(APACHEDIR ."/libexec/apache22/mod_memcache.so") && $settings['memcachesize'] != "0") {
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

		#check balancer members
		foreach ($config['installedpackages']['apachebalancer']['config'] as $balancer){
			if (is_array($balancer['row']) && $balancer['enable'] == 'on'){
				$balancer_config.="# {$balancer['description']}\n";
				$balancer_config.="<Proxy balancer://{$balancer['name']}>\n";
				foreach($balancer['row'] as $server){
					$options =($server['port'] ? ":{$server['port']}" : "");
					
					$options.=($server['routeid'] ? " route={$server['routeid']}" : "");
					$options.=($server['loadfactor'] ? " loadfactor={$server['loadfactor']}" : "");
					if (isset($server['ping'])){
						$options.= " ping={$server['ping']}";
						$options.=($server['ttl'] ? " ttl={$server['ttl']}" : "");
						}
					$balancer_config.="  BalancerMember {$balancer['proto']}://{$server['host']}{$options}\n"; 
					}
				#check if stick connections are set
				if ($balancer['row'][0]['routeid'] !="")
					$balancer_config.="  ProxySet stickysession=ROUTEID\n";
				$balancer_config.="</Proxy>\n\n";
				}
			}
		//write balancer conf
		file_put_contents(APACHEDIR."/etc/apache22/Includes/balancers.conf",$balancer_config,LOCK_EX);
		}
		
	//configure virtual hosts
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
					$vh_config.= " SSLEngine on\n SSLProtocol all -SSLv2\n SSLProxyEngine on\n SSLProxyVerify none\n";
					$vh_config.= " SSLCipherSuite ALL:!ADH:!EXPORT56:RC4+RSA:+HIGH:+MEDIUM:+LOW:+SSLv2:+EXP:+eNULL\n";
					
    				$svr_cert = lookup_cert($virtualhost["ssl_cert"]);
					if ($svr_cert != false) {
						if(base64_decode($svr_cert['crt'])) {
							file_put_contents(APACHEDIR . "/etc/apache22/{$virtualhost["ssl_cert"]}.crt",apache_textarea_decode($svr_cert['crt']),LOCK_EX);
							$vh_config.= " SSLCertificateFile ". APACHEDIR . "/etc/apache22/{$virtualhost["ssl_cert"]}.crt\n";
							}
						if(base64_decode($svr_cert['prv'])) {
							file_put_contents(APACHEDIR . "/etc/apache22/{$virtualhost["ssl_cert"]}.key",apache_textarea_decode($svr_cert['prv']),LOCK_EX);
							$vh_config.= " SSLCertificateKeyFile ". APACHEDIR . "/etc/apache22/{$virtualhost["ssl_cert"]}.key\n";
			       			}
						}
					$svr_ca =lookup_ca($virtualhost["reverse_int_ca"]);
					if ($svr_ca != false) {
							file_put_contents(APACHEDIR . "/etc/apache22/{$virtualhost["reverse_int_ca"]}.crt",apache_textarea_decode($svr_ca['crt']),LOCK_EX);
							$vh_config.= " SSLCACertificateFile ". APACHEDIR . "/etc/apache22/{$virtualhost["reverse_int_ca"]}.crt\n";
							}
					}
				#Custom Options
				$vh_config.= apache_textarea_decode($virtualhost['custom'])."\n\n";

				#Check virtualhost locations
				foreach ($virtualhost['row'] as $backend){
					$vh_config.=" <Location ".($backend['sitepath'] ? $backend['sitepath'] : "/").">\n";
					$vh_config.="  ProxyPass        balancer://{$backend['balancer']}{$backend['backendpath']}\n";
					$vh_config.="  ProxyPassReverse balancer://{$backend['balancer']}{$backend['backendpath']}\n";
					if ($backend['compress']== "no")
						$vh_config.="  SetInputFilter   INFLATE\n  SetOutputFilter  INFLATE\n";
					if (is_array($config['installedpackages']['apachemodsecuritymanipulation'])){
						foreach($config['installedpackages']['apachemodsecuritymanipulation']['config'] as $manipulation){
							if ($backend['modsecmanipulation'] == $manipulation['name']){
								if (is_array($manipulation['row']))
									foreach ($manipulation['row'] as $secrule)
										$vh_config.="  {$secrule['type']} {$secrule['value']}\n";
								}
							}
						}
					$vh_config.=" </Location>\n\n";
					}
				$vh_config.="</VirtualHost>\n";
				}
			}
		//write balancer conf
		file_put_contents(APACHEDIR."/etc/apache22/Includes/virtualhosts.conf",$vh_config,LOCK_EX);
		}
	// check/fix perl version on mod_security util files
	$perl_files= array("httpd-guardian.pl","rules-updater.pl","runav.pl","arachni2modsec.pl","zap2modsec.pl","regression_tests/rulestest.pl");
	foreach ($perl_files as $perl_file){
		$file_path=rules_directory."/util/";
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
	
	//mod_security settings
	if (is_array($config['installedpackages']['apachemodsecuritysettings']['config'])){
		$mods_settings=$config['installedpackages']['apachemodsecuritysettings']['config'][0];
		if ($mods_settings!="")
			$SecGuardianLog="SecGuardianLog \"|".rules_directory."/util/httpd-guardian\"";
		}
	
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
			if ($row['ipaddress'] && $row['ipport'])
				$configuredaliases[] = $row; 
		}
	}
		
	// clear list of bound addresses before updating
	$config['installedpackages']['apachesettings']['config'][0]['row'] = array();

	// Process proxy sites
	// Configure NameVirtualHost directives
	$aliases = "";
	$processed = array();
	if(is_array($config['installedpackages']['apachemodsecurity'])) {
		foreach($config['installedpackages']['apachemodsecurity']['config'] as $ams) {
			if($ams['ipaddress'] && $ams['port']) 
				$local_ip_port = "{$ams['ipaddress']}:{$ams['port']}";
			else 
				$local_ip_port = $global_listen;
			// Do not add entries twice.
			if(!in_array($local_ip_port, $processed)) {
				// explicit bind if not global ip:port
				if ($local_ip_port != $global_listen) {
					$aliases .= "Listen $local_ip_port\n";
					// Automatically add this to configuration
					$config['installedpackages']['apachesettings']['config'][0]['row'][] = array('ipaddress' => $ams['ipaddress'], 'ipport' => $ams['port']); 
				}
				$mod_proxy .= "NameVirtualHost $local_ip_port\n";
				$processed[] = $local_ip_port;
			}
		}
	}

//** Uncomment to allow adding ip/ports not used by any site proxies
//** Otherwise unused addresses/ports will be automatically deleted from the configuration
//	foreach ($configuredaliases as $ams) {
//		$local_ip_port = "{$ams['ipaddress']}:{$ams['ipport']}";
//               if(!in_array($local_ip_port, $processed)) {
//                	// explicit bind if not global ip:port
//                        if ($local_ip_port != $global_listen) {
//                        	$aliases .= "Listen $local_ip_port\n";    
//                                // Automatically add this to configuration
//                                $config['installedpackages']['apachesettings']['config'][0]['row'][] = array('ipaddress' => $ams['ipaddress'], 'ipport' => $ams['ipport']);
//                        }
//		}
//	}

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
					$mod_proxy .= "  SSLCertificateFile /usr/local/etc/apache22/$certificatefile\n";  
				if ($certificatekeyfile)
					$mod_proxy .= "  SSLCertificateKeyFile /usr/local/etc/apache22/$certificatekeyfile\n";
				if ($certificatechainfile)
					$mod_proxy .= "  SSLCertificateChainFile /usr/local/etc/apache22/$certificatechainfile\n";
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
	if(is_dir(rules_directory)) {
		$mod_security_rules = "";
		$files = return_dir_as_array(rules_directory);
		foreach($files as $file) { 
			if(file_exists(rules_directory . "/" . $file)) {
				// XXX: TODO integrate snorts rule on / off thingie
				$file_txt = file_get_contents(rules_directory . "/" . $file);
				$mod_security_rules .= $file_txt . "\n";
			}
		}
	}

	#include file templates
	include ("/usr/local/pkg/apache.template");

	file_put_contents(APACHEDIR . "/etc/apache22/httpd.conf",$apache_config,LOCK_EX);
}

?>
