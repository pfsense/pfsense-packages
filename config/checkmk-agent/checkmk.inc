<?php
/* ========================================================================== */
/*
	checkmk.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2013 Marcello Coutinho
    All rights reserved.
                                                                              */
/* ========================================================================== */
/*
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
/* ========================================================================== */

define('ETC_SERVICES','/etc/services');
define('ETC_INETD','/etc/inetd.conf');
define('ETC_HOSTS_ALLOW','/etc/hosts.allow');
define('ETC_RC_CONF','/etc/rc.conf.local');

function checkmk_install() {
	// Download latest check_mk version from head repo
	$checkmk_bin="/usr/local/bin/check_mk_agent";
	mwexec("fetch -o {$checkmk_bin} 'http://git.mathias-kettner.de/git/?p=check_mk.git;a=blob_plain;f=agents/check_mk_agent.freebsd;hb=HEAD'");
	chmod($checkmk_bin,0755);
	sync_package_checkmk();
}

function checkmk_deinstall() {
	// reserved
}

function checkmk_start() {
	global $g, $config;
	
	// reserved
}

function checkmk_text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}
function sync_package_checkmk() {
	global $config, $g;
	$update_conf=0;

	if (!is_array($config['installedpackages']['checkmk']['config']))
		return;
	
	$mk_config=$config['installedpackages']['checkmk']['config'][0];
		
	$checkmk_bin="/usr/local/bin/check_mk_agent";
	if (!file_exists($checkmk_bin) && $mk_config['checkmkenable']=="on"){
		$error = "Check_mk-agent Binary file not found";
		log_error($error." You can manually download it using this cmd: fetch -o {$checkmk_bin} 'http://git.mathias-kettner.de/git/?p=check_mk.git;a=blob_plain;f=agents/check_mk_agent.freebsd;hb=HEAD'");
		file_notice("Check_mk-agent", $error, "checkmk save config", "");
		return;
		}
	//mount filesystem writeable
	conf_mount_rw();
	

	// check services file
	$mk_services= file(ETC_SERVICES);
	$port=($mk_config['checkmkport'] ? $mk_config['checkmkport'] : "6556");
	foreach($mk_services as $mk_service){
		if (!preg_match("/check_mk/",$mk_service))
			$mk_service_file.=chop($mk_service)."\n";
		}
	if ($mk_config['checkmkenable']=="on")
		$mk_service_file.="check_mk	{$port}/tcp   #check_mk agent\n";
	file_put_contents(ETC_SERVICES,$mk_service_file,LOCK_EX);
	
	// check inetd file
	$mk_inetds= file(ETC_INETD);
	foreach($mk_inetds as $mk_inetd){
		if (!preg_match("/check_mk/",$mk_inetd))
			$mk_inetd_file.=chop($mk_inetd)."\n";
		}
	if ($mk_config['checkmkenable']=="on")
		$mk_inetd_file.="check_mk	stream	tcp	nowait		root	/usr/local/bin/check_mk_agent check_mk\n";
	file_put_contents(ETC_INETD,$mk_inetd_file,LOCK_EX);
		 
	// check hosts.allow
	$mk_hosts= file(ETC_HOSTS_ALLOW);
	$inet_daemons_count=0;
	foreach($mk_hosts as $mk_host){
		if (!preg_match("/check_mk/",$mk_host))
			$mk_hosts_file.=chop($mk_host)."\n";
		if (preg_match("/^\w+/"))
			$inet_daemons_count++;
		}
	if ($mk_config['checkmkenable']=="on")
		foreach (explode(',',$mk_config['checkmkhosts']) as $check_mk_host){
			$mk_hosts_file.="check_mk : {$check_mk_host} : allow\n";
			$inet_daemons_count++;
			}
	file_put_contents(ETC_HOSTS_ALLOW,$mk_hosts_file,LOCK_EX);
	
	//check inetd daemon rc_conf option
	$mk_rc_confs= file(ETC_RC_CONF);
	foreach($mk_rc_confs as $mk_rc_conf){
		if (!preg_match("/inetd_/",$mk_rc_conf))
			$mk_rc_conf_file.=chop($mk_rc_conf)."\n";
		}
	if ($mk_config['checkmkenable']=="on"){
		$mk_rc_conf_file.='inetd_enable="YES"'."\n";
		$mk_rc_conf_file.='inetd_flags="-wW"'."\n";
		}
		
	file_put_contents(ETC_RC_CONF,$mk_rc_conf_file,LOCK_EX);
	if ($inet_daemons_count > 0)
		mwexec("/etc/rc.d/inetd restart");
	else
		mwexec("/etc/rc.d/inetd stop");

	//Write config if any file from filesystem was loaded
	if ($update_conf > 0)
		write_config();
		
	// mount filesystem readonly
	conf_mount_ro();
	
	checkmk_sync_on_changes();
}

function checkmk_validate_input($post, &$input_errors) {
	foreach ($post as $key => $value) {
		if (empty($value))
			continue;
		if (substr($key, 0, 3) == "port" && !preg_match("/^\d+$/", $value))
				$input_errors[] = "{$value} is no a valid port number";
		if (substr($key, 0, 11) == "description" && !preg_match("@^[a-zA-Z0-9 _/.-]+$@", $value))
				$input_errors[] = "Do not use special characters on description";
		if (substr($key, 0, 8) == "fullfile" && !preg_match("@^[a-zA-Z0-9_/.-]+$@", $value))
				$input_errors[] = "Do not use special characters on filename";
				
	}
}
##############################################
/* Uses XMLRPC to synchronize the changes to a remote node */
function checkmk_sync_on_changes() {
	global $config, $g;
	if (is_array($config['installedpackages']['checkmksync']['config'])){
		$checkmk_sync=$config['installedpackages']['checkmksync']['config'][0];
		$synconchanges = $checkmk_sync['synconchanges'];
		$synctimeout = $checkmk_sync['synctimeout'];
		switch ($synconchanges){
			case "manual":
				if (is_array($checkmk_sync[row])){
					$rs=$checkmksync[row];
					}
				else{
					log_error("[Check_mk-agent] xmlrpc sync is enabled but there is no hosts to push on squid config.");
					return;
					}
				break;
			case "auto":
					if (is_array($config['installedpackages']['carpsettings']) && is_array($config['installedpackages']['carpsettings']['config'])){
						$system_carp=$config['installedpackages']['carpsettings']['config'][0];
						$rs[0]['ipaddress']=$system_carp['synchronizetoip'];
						$rs[0]['username']=$system_carp['username'];
						$rs[0]['password']=$system_carp['password'];
					}
					else{
						log_error("[Check_mk-agent] xmlrpc sync is enabled but there is no system backup hosts to push squid config.");
						return;
					}
				break;			
			default:
				return;
			break;
		}
		if (is_array($rs)){
			log_error("[Check_mk-agent] xmlrpc sync is starting.");
			foreach($rs as $sh){
				$sync_to_ip = $sh['ipaddress'];
				$password = $sh['password'];
				if($sh['username'])
					$username = $sh['username'];
				else
					$username = 'admin';
				if($password && $sync_to_ip)
					checkmk_do_xmlrpc_sync($sync_to_ip, $username, $password,$synctimeout);
				}
			log_error("[Check_mk-agent] xmlrpc sync is ending.");
			}
 		}
}
##############################################
/* Do the actual XMLRPC sync */
function checkmk_do_xmlrpc_sync($sync_to_ip, $username, $password,$synctimeout) {
	global $config, $g;

	if(!$username)
		return;
		
	if(!$password)
		return;

	if(!$sync_to_ip)
		return;

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
	$xml['checkmk'] = $config['installedpackages']['checkmk'];
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("[Check_mk-agent] Beginning checkmk XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 250 seconds */
	$resp = $cli->send($msg, $synctimeout);
	if(!$resp) {
		$error = "[Check_mk-agent] A communications error occurred while attempting checkmk XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "checkmk Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "[Check_mk-agent] An error code was received while attempting checkmk XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "checkmk Settings Sync", "");
	} else {
		log_error("[Check_mk-agent] XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* tell checkmk to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/checkmk.inc');\n";
	$execcmd .= "sync_package_checkmk();";
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);
	
	log_error("[Check_mk-agent] XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, $synctimeout);
	if(!$resp) {
		$error = "[Check_mk-agent] A communications error occurred while attempting checkmk XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "checkmk Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "[Check_mk-agent] An error code was received while attempting checkmk XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "checkmk Settings Sync", "");
	} else {
		log_error("[Check_mk-agent] XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
	
}

?>
