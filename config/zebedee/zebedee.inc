<?php
/*
	zebedee.inc
	part of the Postfix package for pfSense
	Copyright (C) 2010 Erik Fonnesbeck
	Copyright (C) 2011 Marcello Coutinho
	Copyright (C) 2011 Jorge Lustosa
	
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
require_once("util.inc");
require_once("functions.inc");
require_once("pkg-utils.inc");
require_once("globals.inc");

function zb_text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}

function zb_get_real_interface_address($iface) {
	global $config;
	$iface = convert_friendly_interface_to_real_interface_name($iface);
	$line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
	list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);
	return array($ip, long2ip(hexdec($netmask)));
}

function sync_package_zebedee() {
	global $config;


	$zebedee_config=$config['installedpackages']['zebedee']['config'][0];

# set  Default values 
if($zebedee_config['keylength']=="")$zebedee_config['keylength']="256" ;
if($zebedee_config['keylifetime']=="")$zebedee_config['keylifetime']="36000" ;
if($zebedee_config['maxbufsize']=="")$zebedee_config['maxbufsize']="16383" ;
	
	
# Write main zebedee configuration file 
$fd = fopen("/usr/local/etc/server.zbd", "w");
$cfg_file = <<<EOF
verbosity   {$zebedee_config['verbosity']}
server      true            
detached    {$zebedee_config['detached']}            
udpmode     {$zebedee_config['udpmode']}           
ipmode      {$zebedee_config['ipmode']}
keygenlevel {$zebedee_config['keygenlevel']}
compression {$zebedee_config['compression']}:{$zebedee_config['compression_level']}
keylength   {$zebedee_config['keylength']}
keylifetime {$zebedee_config['keylifetime']}
maxbufsize  {$zebedee_config['maxbufsize']}
logfile     '/var/log/zebedee.log'   
checkidfile '/usr/local/etc/clients.id'   
include     '/usr/local/etc/tunnels.zbd'
EOF;

	fwrite($fd, $cfg_file);
	fclose($fd);

	// manual restart of zebede 
	mwexec_bg("/usr/local/etc/rc.d/zebedee.sh");
	
}

function zebedee_tunnels() {
	global $config;

	$zebedee_config=$config['installedpackages']['zebedeetunnels']['config'][0]['row'];
	$redirect = $config['installedpackages']['zebedeetunnels']['config'][0]['redirect'] ; 
	
	foreach ($zebedee_config as $k => $v) 
	{
		// especify only one port for this host 
		if($v['port']=="") $end=" " ; else $end = ":".$v['port'] ; 
		$tunnels .= "target ".$v['ipaddress'].$end."\n" ; 
	}
		
			
# Write tunnels and targets configuration file 
$fd = fopen("/usr/local/etc/tunnels.zbd", "w");
$cfg_file = <<<EOF
{$tunnels}
redirect {$redirect}
EOF;

	fwrite($fd, $cfg_file);
	fclose($fd);
	
	// manual restart of zebede
	mwexec_bg("/usr/local/etc/rc.d/zebedee.sh");
	
}


function zebedee_key() 
{ 
	
	global $config;

	$zebedee_config=$config['installedpackages']['zebedeekeys']['config'];
	$priv = exec("cat /usr/local/etc/zebedee/".$_REQUEST['id'].".priv") ; 
	
	if(!$priv)
	{ 
		$gen_private_key = exec("/usr/local/bin/zebedee -p >> /usr/local/etc/zebedee/".$_REQUEST['id'].".priv") ; 
		$public_key = exec("/usr/local/bin/zebedee -P -f /usr/local/etc/zebedee/".$_REQUEST['id'].".priv") ;
		$private_key = exec("cat /usr/local/etc/zebedee/".$_REQUEST['id'].".priv ") ;
	}
	else
	{ 
		$private_key = exec("cat /usr/local/etc/zebedee/".$_REQUEST['id'].".priv ") ;
		$public_key = exec("/usr/local/bin/zebedee -P -f /usr/local/etc/zebedee/".$_REQUEST['id'].".priv") ;
	}

		$private_key = substr($private_key,12,40)  ;
		$public_key = substr($public_key,0,40)  ;
		$config['installedpackages']['zebedeekeys']['config'][$_REQUEST['id']]['private_key'] = $private_key ;  
		$config['installedpackages']['zebedeekeys']['config'][$_REQUEST['id']]['public_key'] = $public_key ;  
		
		write_config();
	
	// write clients.id file 
	foreach ($config['installedpackages']['zebedeekeys']['config'] as $key)
	{ 
		$clients .= $key["public_key"]." ".$key["ident"]."\n" ; 
	}	

	$fd = fopen("/usr/local/etc/clients.id", "w");
$cfg_file = <<<EOF
{$clients}
EOF;
	fwrite($fd, $cfg_file);
	fclose($fd);

	// redirect 
	header("Location: zebedee_keys.php");
	
	exit ; 
	
}


function zebedee_start(){
	global $config;
	
	//need be implemented
	
	
}

function zebedee_validate_input($post, &$input_errors) {
	foreach ($post as $key => $value) 
	{
		if (empty($value))
			continue;
		if($key == "greet_time" && !preg_match("/(\d+),(\d+)(s|m|h|w)/",$value))
				$input_errors[] = "Wrong greet time sintax.";			
		if($key == "message_size_limit" && !is_numeric($value))
				$input_errors[] = "Message size limit must be numeric.";
		if($key == "process_limit" && !is_numeric($value))
				$input_errors[] = "Process limit must be numeric.";	
		if($key == "freq" && (!preg_match("/^\d+(h|m|d)$/",$value) || $value == 0))
				$input_errors[] = "A valid number with a time reference is required for the field 'Frequency'";
		if (substr($key, 0, 2) == "dc" && !is_hostname($value))
				$input_errors[] = "{$value} is not a valid host name.";
		if (substr($key, 0, 6) == "domain" && is_numeric(substr($key, 6))) {
			if (!is_domain($value))
				$input_errors[] = "{$value} is not a valid domain name.";
		} else if (substr($key, 0, 12) == "mailserverip" && is_numeric(substr($key, 12))) {
			if (empty($post['domain' . substr($key, 12)]))
				$input_errors[] = "Domain for {$value} cannot be blank.";
			if (!is_ipaddr($value) && !is_hostname($value))
				$input_errors[] = "{$value} is not a valid IP address or host name.";
		}
	}
}

function zebedee_php_install_command() {
	sync_package_zebedee();
}

function zebedee_php_deinstall_command() {
	
	
	mwexec_bg("killall -9 zebedee");
	
	sleep(1);
	conf_mount_rw();
	unlink_if_exists("/usr/local/etc/rc.d/zebedee.sh");
	conf_mount_ro();
}

/* Uses XMLRPC to synchronize the changes to a remote node */
function zebedee_sync_on_changes() {
	global $config, $g;
	log_error("[zebedee] zebedee xml_rpc is starting.");
	$synconchanges = $config['installedpackages']['zebedeesync']['config'][0]['synconchanges'];	
	if(!$synconchanges) 
		return;
	foreach ($config['installedpackages']['zebedeesync']['config'] as $rs ){
		foreach($rs['row'] as $sh){
		$sync_to_ip = $sh['ipaddress'];
		$password   = $sh['password'];
		if($password && $sync_to_ip)
			zebedee_do_xmlrpc_sync($sync_to_ip, $password);
		}
	}
	log_error("[zebedee] postfix_xmlrpc_sync.php is ending.");
}

/* Do the actual XMLRPC sync */
function zebedee_do_xmlrpc_sync($sync_to_ip, $password) {
	global $config, $g;

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
	$xml['zebedee'] = $config['installedpackages']['zebedee'];
	$xml['zebedeetunnels'] = $config['installedpackages']['zebedeetunnels'];
	$xml['zebedeekeys'] = $config['installedpackages']['zebedeekeys'];
	$xml['zebedeesync'] = $config['installedpackages']['zebedeesync'];
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Beginning Postfix XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 250 seconds */
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting zebedee XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "Zebedee Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting zebedee XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "Postfix Settings Sync", "");
	} else {
		log_error("Zebedee XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* tell zebedee to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/zebedee.inc');\n";
	$execcmd .= "sync_package_zebedee();";
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);

	log_error("zebedee XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting zebedee XMLRPC sync with {$url}:{$port} (exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "zebedee Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting zebedee XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "zebedee Settings Sync", "");
	} else {
		log_error("zebedee XMLRPC reload data success with {$url}:{$port} (exec_php).");
	}

}

?>
