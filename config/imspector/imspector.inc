<?php
/*
	imspector.inc
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2012 Marcello Coutinho.
	Copyright (C) 2011 Scott Ullrich <sullrich@gmail.com>.
	Copyright (C) 2011 Bill Marquette <billm@gmail.com>.
	Copyright (C) 2007 Ryan Wagoner <rswagoner@gmail.com>.
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
	require_once("functions.inc");
	require_once("service-utils.inc");

	/* IMSpector */

	define('IMSPECTOR_RCFILE', '/usr/local/etc/rc.d/imspector.sh');
	define('IMSPECTOR_ETC', '/usr/local/etc/imspector');
	define('IMSPECTOR_CONFIG', IMSPECTOR_ETC . '/imspector.conf');

	function imspector_warn ($msg) { syslog(LOG_WARNING, "imspector: {$msg}"); }

	function ims_text_area_decode($text){
		return preg_replace('/\r\n/', "\n",base64_decode($text));	
	}
	
	function imspector_action ($action) {
		if (file_exists(IMSPECTOR_RCFILE))
			mwexec(IMSPECTOR_RCFILE.' '.$action);
	}	

	function write_imspector_config($file, $text) {
		$conf = fopen($file, 'w');
		if(!$conf) {
			imspector_warn("Could not open {$file} for writing.");
			exit;
		}
		fwrite($conf, $text);
		fclose($conf);	
	}	

	function imspector_pf_rdr($iface, $port) {
		return "rdr pass on {$iface} inet proto tcp from any to any port = {$port} -> 127.0.0.1 port 16667\n";
	}

	function imspector_pf_rule($iface, $port) {
		return "pass in quick on {$iface} inet proto tcp from any to any port {$port} keep state\n";
	}		

	function imspector_proto_to_port ($proto)
	{
		switch ($proto) {
			case 'gadu-gadu':
				return 8074;
			case 'jabber':
				return 5222;
			case 'jabber-ssl':
				return 5223;
			case 'msn':
				return 1863;
			case 'icq':
				return 5190;
			case 'yahoo':
				return 5050;
			case 'irc':
				return 6667;
			default:
				return null;
		}
	}

	function validate_form_imspector($post, $input_errors) {
		if($post['iface_array'])
			foreach($post['iface_array'] as $iface)
				if($iface == 'wanx')
					$input_errors[] = 'It is a security risk to specify WAN in the \'Interface\' field';
	}

	function deinstall_package_imspector() {
		imspector_action('stop');

		unlink_if_exists(IMSPECTOR_RCFILE);
		unlink_if_exists(IMSPECTOR_CONFIG);
		unlink_if_exists(IMSPECTOR_ETC . '/badwords_custom.txt');
		unlink_if_exists(IMSPECTOR_ETC . '/acl_blacklist.txt');
		unlink_if_exists(IMSPECTOR_ETC . '/acl_whitelist.txt');
		unlink_if_exists('/usr/local/www/imspector_logs.php');

		//exec('pkg_delete imspector-0.4');
	}

	function imspector_generate_rules($type) {
		
		$rules = "";
		switch ($type) {
		case 'rdr':
		case 'nat':
			$rules = "# IMSpector rdr anchor\n";
			$rules .= "rdr-anchor \"imspector\"\n";
			break;
		case 'rule':
			$rules = "# IMSpector \n";
			$rules .= "anchor \"imspector\"\n";
			break;
		}

		return $rules;
	}

	function sync_package_imspector() {
		global $config;
		global $input_errors;

		/*detect boot process*/
		if (is_array($_POST)){
			if (preg_match("/\w+/",$_POST['__csrf_magic']))
				unset($boot_process);
			else
				$boot_process="on";
			}
				
		if (is_process_running('imspector') && isset($boot_process))
			return;

		/* check default options and sample files*/
		$load_samples=0;

		#bannedphraselist
		if (!is_array($config['installedpackages']['imspectoracls'])){
				$config['installedpackages']['imspectoracls']['config'][]=array('enable'=> 'on',
																				'description' => 'allow access to all ids',
																				'action' => 'allow',
																				'localid' => 'all',
																				'remoteid' => base64_encode('all'));
				$load_samples++;
			}
		$ims_acls = $config['installedpackages']['imspectoracls']['config'];
			
		if (is_array($config['installedpackages']['imspectorreplacements'])){
			if ($config['installedpackages']['imspectorreplacements']['config'][0]['badwords_list'] == "" && file_exists(IMSPECTOR_ETC . '/badwords.txt')){
				$config['installedpackages']['imspectorreplacements']['config'][0]['badwords_list'] = base64_encode(file_get_contents(IMSPECTOR_ETC . '/badwords.txt'));
				$load_samples++;
				}
			$ims_replacements = $config['installedpackages']['imspectorreplacements']['config'][0];
		}
					
		if (is_array($config['installedpackages']['imspector']))
			$ims_config = $config['installedpackages']['imspector']['config'][0];

		if($load_samples > 0)
			write_config();
			
		/*continue sync process*/
		log_error("Imspector: Saving changes.");
		config_lock();
			
		/* remove existing rules */
		exec('/sbin/pfctl -a imspector -Fr > /dev/null');
		exec('/sbin/pfctl -a imspector -Fn > /dev/null');	

		$ifaces_active = '';		

		if($ims_config['enable'] && $ims_config['proto_array'])
			$proto_array = explode(',', $ims_config['proto_array']);

		if($ims_config['enable'] && $ims_config['iface_array'])
			$iface_array = explode(',', $ims_config['iface_array']);

		if($iface_array && $proto_array) {
			foreach($iface_array as $iface) {
				$if = convert_friendly_interface_to_real_interface_name($iface);
				/* above function returns iface if fail */
				if($if!=$iface) {
					$addr = find_interface_ip($if);
					/* non enabled interfaces are displayed in list on imspector settings page */
					/* check that the interface has an ip address before adding parameters */
					if($addr) {
						foreach($proto_array as $proto) {
							if(imspector_proto_to_port($proto))	{
								/* we can use rdr pass to auto create the filter rule */
								$pf_rules .= imspector_pf_rdr($if,imspector_proto_to_port($proto));
							}
						}
						if(!$ifaces_active)
							$ifaces_active = "{$iface}";
						else
							$ifaces_active .= ", {$iface}";
					} else {
						imspector_warn("Interface {$iface} has no ip address, ignoring");
					}
				} else {
					imspector_warn("Could not resolve real interface for {$iface}");
				}
			}
			
			
			/*reload rules*/
			if($pf_rules) {
				log_error("Imspector: Reloading rules.");
				exec("echo \"{$pf_rules}\" | /sbin/pfctl -a imspector -f -");

				conf_mount_rw();

				/* generate configuration files */

				$conf['plugin_dir'] = '/usr/local/lib/imspector';

				foreach($proto_array as $proto)
					$conf[$proto . '_protocol'] = 'on';
					
				if($ims_config['log_file']) {
					@mkdir('/var/imspector');
					$conf['file_logging_dir'] = '/var/imspector';
				}
					
				if($ims_config['log_mysql']) {
					$conf['mysql_server'] = $ims_config['mysql_server'];
					$conf['mysql_database'] = $ims_config['mysql_database'];
					$conf['mysql_username'] = $ims_config['mysql_username'];
					$conf['mysql_password'] = $ims_config['mysql_password'];
				}

				if($ims_replacements['filter_badwords']) {
					write_imspector_config(IMSPECTOR_ETC . '/badwords_custom.txt', ims_text_area_decode($ims_replacements["badwords_list"]));
					$conf['badwords_filename'] = IMSPECTOR_ETC . '/badwords_custom.txt'; 
					}

				if($ims_replacements['block_files'])
					$conf['block_files'] = 'on';
					
				if($ims_replacements['block_webcams'])
					$conf['block_webcams'] = 'on';
 
				$acls="";
				$conf['acl_filename'] = IMSPECTOR_ETC . '/acls.txt';
				foreach ($ims_acls as $rule){
						if ($rule['enable']){
							$acls.= "{$rule['action']} {$rule['localid']} ".preg_replace("/\s+/"," ",base64_decode($rule['remoteid']))."\n"; 
						}
					}
				write_imspector_config(IMSPECTOR_ETC . '/acls.txt', $acls);
				
				// Handle Jabber SSL options
				if(isset($ims_config["ssl_ca_cert"]) && $ims_config["ssl_ca_cert"] != "none" &&
				   isset($ims_config["ssl_server_cert"]) && $ims_config["ssl_server_cert"] != "none") {
					$conf['ssl'] = "on";
					if(!is_dir(IMSPECTOR_ETC . "/ssl"))	
						mkdir(IMSPECTOR_ETC . "/ssl");
						
					$ca_cert = lookup_ca($ims_config["ssl_ca_cert"]);
					if ($ca_cert != false) {
						if(base64_decode($ca_cert['prv'])) {
							file_put_contents(IMSPECTOR_ETC . "/ssl/ssl_ca_key.pem", base64_decode($ca_cert['prv']));
							$conf['ssl_ca_key'] = IMSPECTOR_ETC . '/ssl/ssl_ca_key.pem';
						}
						if(base64_decode($ca_cert['crt'])) {
							file_put_contents(IMSPECTOR_ETC . "/ssl/ssl_ca_cert.pem", base64_decode($ca_cert['crt']));
							$conf['ssl_ca_cert'] = IMSPECTOR_ETC . "/ssl/ssl_ca_cert.pem";
						}
						$svr_cert = lookup_cert($ims_config["ssl_server_cert"]);
						if ($svr_cert != false) {
							if(base64_decode($svr_cert['prv'])) {
								file_put_contents(IMSPECTOR_ETC . "/ssl/ssl_server_key.pem", base64_decode($svr_cert['prv']));
								$conf['ssl_key'] = IMSPECTOR_ETC . '/ssl/ssl_server_key.pem';
							}
							
						}
						$conf['ssl_cert_dir'] = IMSPECTOR_ETC . '/ssl';
					}
				} else {
					// SSL Not enabled.  Make sure Jabber-SSL is not processed.
					unset($conf['jabber-ssl']);
					unset($conf['ssl']);
				}

				if (isset($ims_replacements['responder']) && $ims_replacements['responder'] == 'on') {
					$conf['responder_filename'] = IMSPECTOR_ETC . "/responder.db";
					if (isset($ims_replacements['prefix_message']) && $ims_replacements['prefix_message'] != '' ) {
						$conf['response_prefix'] = " .={$ims_replacements['prefix_message']}=.";
					}
					else{
						$conf['response_prefix'] = " .=Your activities are being logged=.";
					}
					if (isset($ims_replacements['notice_days']) && is_numeric($ims_replacements['notice_days'])) {
						if ($ims_replacements['notice_days'] != 0) {
							$conf['notice_days'] = $ims_replacements['notice_days'];
						}
					} else {
							$conf['notice_days'] = 1;
					}
					
					/*Custom recorded message response*/
					if(isset($ims_replacements['recorded_message']) && $ims_replacements['recorded_message'] != '' ){
						$conf['notice_response'] = ims_text_area_decode($ims_replacements['recorded_message']);
						}
					else{
						$conf['notice_response'] = "Your activities are being logged";
						}
					
					/*Filtered Frequency*/
					if (isset($ims_replacements['filtered_minutes']) && is_numeric($ims_replacements['filtered_minutes'])) {
						if ($ims_replacements['filtered_minutes'] != 0) {
							$conf['filtered_mins'] = $ims_replacements['filtered_minutes'];
						}
					} else {
							$conf['filtered_mins'] = 15;
					}
					
					/*Custom filtered message response*/
					if(isset($ims_replacements['filtered_message']) && $ims_replacements['filtered_message'] != '' ){
						$conf['filtered_response'] = ims_text_area_decode($ims_replacements['filtered_message']);
						}
					else{
						$conf['filtered_response'] = "Your message has been filtered";
					}
				}

				$conftext = '';
				foreach($conf as $var => $key)
					$conftext .= "{$var}={$key}\n";
				write_imspector_config(IMSPECTOR_CONFIG, $conftext);
				
				/*Check template settings*/
				if ($ims_config['template'] == "")
					$template="services_imspector_logs.php";
				else
					$template=$ims_config['template'];

				/*link template file*/
				$link="/usr/local/www/imspector_logs.php";
				unlink_if_exists($link);
				symlink("/usr/local/www/{$template}", $link);
					
				/* generate rc file start and stop */
				$stop = <<<EOD
/bin/pkill -x imspector
/bin/sleep 1
EOD;
				$start = $stop."\n\tldconfig -m /usr/local/lib/mysql\n";
				$start .= "\t/usr/local/sbin/imspector -c \"".IMSPECTOR_CONFIG."\"";

				write_rcfile(array(
					    'file' => 'imspector.sh',
					    'start' => $start,
					    'stop' => $stop
				    )
				);

				conf_mount_ro();			
			}
		}

		if(!$iface_array || !$proto_array || !$pf_rules) {
			/* no parameters user does not want imspector running */
			/* lets stop the service and remove the rc file */

			if(file_exists(IMSPECTOR_RCFILE)) {
				if(!$ims_config['enable'])
					log_error('Impsector: Stopping service: imspector disabled');
				else
					log_error('Impsector: Stopping service: no interfaces and/or protocols selected');
					
				imspector_action('stop');

				conf_mount_rw();
				unlink(IMSPECTOR_RCFILE);
				unlink(IMSPECTOR_CONFIG);
				@unlink(IMSPECTOR_ETC . '/badwords_custom.txt');
				@unlink(IMSPECTOR_ETC . '/acl_blacklist.txt');
				@unlink(IMSPECTOR_ETC . '/acl_whitelist.txt');
				conf_mount_ro();
			}			
		}
		else{
			/* if imspector not running start it */
			if(!is_process_running('imspector')) {
				log_error("Impsector: Starting service on interface: {$ifaces_active}");
				imspector_action('start');
				}
			/* or restart imspector if settings were changed */
			else{
				log_error("Impsector: Restarting service on interface: {$ifaces_active}");
				imspector_action('restart');
				}	
		}	
		config_unlock();
		
		/*check xmlrpc sync*/
		imspector_sync_on_changes();		
	}

	function imspector_get_ca_certs() {
		global $config;

		$ca_arr = array();
		$ca_arr[] = array('refid' => 'none', 'descr' => 'none');
		foreach ($config['ca'] as $ca) {
			$ca_arr[] = array('refid' => $ca['refid'], 'descr' => $ca['descr']);
		}
		return $ca_arr;
	}

	function imspector_get_server_certs() {
		global $config;
		$cert_arr = array();
		$cert_arr[] = array('refid' => 'none', 'descr' => 'none');
		
		foreach ($config['cert'] as $cert) {
			$cert_arr[] = array('refid' => $cert['refid'], 'descr' => $cert['descr']);
		}
		return $cert_arr;
	}
	
/* Uses XMLRPC to synchronize the changes to a remote node */
function imspector_sync_on_changes() {
	global $config, $g;
	
	$synconchanges = $config['installedpackages']['imspectorsync']['config'][0]['synconchanges'];	
	if(!$synconchanges) 
		return;
	log_error("Imspector: xmlrpc sync is starting.");
	foreach ($config['installedpackages']['imspectorsync']['config'] as $rs ){
		foreach($rs['row'] as $sh){
			$sync_to_ip = $sh['ipaddress'];
			$password = $sh['password'];
			if($password && $sync_to_ip)
				imspector_do_xmlrpc_sync($sync_to_ip, $password);
			}
	}
	log_error("Imspector: xmlrpc sync is ending.");
}
/* Do the actual XMLRPC sync */
function imspector_do_xmlrpc_sync($sync_to_ip, $password) {
	global $config, $g;

	if(!$password)
		return;

	if(!$sync_to_ip)
		return;
	$username="admin";
	
	$xmlrpc_sync_neighbor = $sync_to_ip;
    if($config['system']['webgui']['protocol'] != "") {
		$synchronizetoip = $config['system']['webgui']['protocol'];
		$synchronizetoip .= "://";
    }
    $port = $config['system']['webgui']['port'];
    /* if port is empty lets rely on the protocol selection */
    if($port == "") {
		if($config['system']['webgui']['protocol'] == "http") 
			$port = "80";
		else 
			$port = "443";
    }
	$synchronizetoip .= $sync_to_ip;

	/* xml will hold the sections to sync */
	$xml = array();
	$xml['imspector'] = $config['installedpackages']['imspector'];
	$xml['imspectorreplacements'] = $config['installedpackages']['imspectorreplacements'];
	$xml['imspectoracls'] = $config['installedpackages']['imspectoracls'];
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Imspector: Beginning XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 250 seconds */
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting imspector XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "imspector Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting imspector XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "imspector Settings Sync", "");
	} else {
		log_error("imspector XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* tell imspector to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/imspector.inc');\n";
	$execcmd .= "sync_package_imspector();";
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);
	
	log_error("imspector XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting imspector XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "imspector Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting imspector XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "imspector Settings Sync", "");
	} else {
		log_error("imspector XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
	
}
?>
