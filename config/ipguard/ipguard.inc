<?php

/* ========================================================================== */
/*
    ipguard.inc
    part of the ipguard package for pfSense (http://www.pfSense.com)
    Copyright (C) 2012 Marcello Coutinho
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
    All rights reserved.
                                                                              */
/* ========================================================================== */
/*
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
/* ========================================================================== */
 require_once("config.inc");
 require_once("util.inc");
 
function ipguard_custom_php_deinstall_command(){
	global $g, $config;

	conf_mount_rw();
	
	stop_service('ipguard');
	$ipguard_sh_file = "/usr/local/etc/rc.d/ipguard.sh";
	if (is_file($ipguard_sh_file))
		chmod($ipguard_sh_file,0444);

	conf_mount_ro();
	}

function ipguard_custom_php_write_config(){
	global $g, $config;
	
	# detect boot process
	if (is_array($_POST)){
		if (!preg_match("/\w+/",$_POST['__csrf_magic']))
			return;
		}

		
	if (is_array($config['installedpackages']['ipguard']['config'])){
		// Read config
		$new_config=array();
		foreach ($config['installedpackages']['ipguard']['config'] as $ipguard){
			if ($ipguard['enable'] && $ipguard['interface'] && $ipguard['mac'] && $ipguard['ip']){
				$new_config[$ipguard['interface']].= "{$ipguard['mac']} {$ipguard['ip']} {$ipguard['description']}\n";
			}
		}
	}

	//Save /etc/ssh/ipguard_extra
	$script="/usr/local/etc/rc.d/ipguard.sh";
	$start="";
	$stop="pkill -anx ipguard";
	conf_mount_rw();
	if (count ($new_config) > 0 && $ipguard['enable']){
			foreach ($new_config as $key => $value){
				$conf_file="/usr/local/etc/ipguard_{$key}.conf";
				file_put_contents($conf_file,$value,LOCK_EX);
				$config_file=file_put_contents($conf_file,$new_config[$key],LOCK_EX);
				$iface=convert_friendly_interface_to_real_interface_name($key);
				$start.="/usr/local/sbin/ipguard -l /var/log/ipguard_{$key}.log -p /var/run/ipguard_{$key}.pid -f {$conf_file} -u 300 -z {$iface}\n\t";
			}
			write_rcfile(array(
					'file' => 'ipguard.sh',
					'start' => $start,
					'stop' => $stop
					));
			restart_service('ipguard');
			
		}
	else{
		#remove config files
		stop_service('ipguard');
		$ipguard_sh_file = "/usr/local/etc/rc.d/ipguard.sh";
		if (is_file($ipguard_sh_file))
			chmod($ipguard_sh_file,0444);
	}
	// Mount Read-only
	conf_mount_ro();
	
	//sync config with other pfsense servers
	ipguard_sync_on_changes();
	}

/* Uses XMLRPC to synchronize the changes to a remote node */
function ipguard_sync_on_changes() {
	global $config, $g;
	
	if (is_array($config['installedpackages']['ipguardsync'])) {
		if ($config['installedpackages']['ipguardsync']['config'][0]['synconchanges'])	{
			log_error("[ipguard] xmlrpc sync is starting.");
			foreach ($config['installedpackages']['ipguardsync']['config'] as $rs ){
				foreach($rs['row'] as $sh){
					$sync_to_ip = $sh['ipaddress'];
					$password = $sh['password'];
					if($password && $sync_to_ip)
						ipguard_do_xmlrpc_sync($sync_to_ip, $password);
				}
			}
			log_error("[ipguard] xmlrpc sync is ending.");
		}
	}
}

/* Do the actual XMLRPC sync */
function ipguard_do_xmlrpc_sync($sync_to_ip, $password) {
	global $config, $g;

	if(!$password)
		return;

	if(!$sync_to_ip)
		return;

	$username='admin';
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
	$xml['ipguard'] = $config['installedpackages']['ipguard'];
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Beginning ipguard XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 250 seconds */
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting ipguard XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "ipguard Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting ipguard XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "ipguard Settings Sync", "");
	} else {
		log_error("ipguard XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* tell ipguard to reload our settings on the destination sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/ipguard.inc');\n";
	$execcmd .= "ipguard_custom_php_write_config();";
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);
	
	log_error("ipguard XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting ipguard XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "ipguard Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting ipguard XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "ipguard Settings Sync", "");
	} else {
		log_error("ipguard XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
}
	?>