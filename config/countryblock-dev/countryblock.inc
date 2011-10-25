<?php
/*
	countryblock.inc
	part of the Postfix package for pfSense
	Copyright (C) 2010 Erik Fonnesbeck
	Copyright (C) 2011 Marcello Coutinho

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

function cb_text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}

function cb_get_real_interface_address($iface) {
	global $config;
	$iface = convert_friendly_interface_to_real_interface_name($iface);
	$line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
	list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);
	return array($ip, long2ip(hexdec($netmask)));
}

function sync_package_countryblock() {
	global $config;
	$countryblock_config=$config['installedpackages']['countryblock']['config'][0];
	$continents= array("Africa","Antartica","Asia","Europe","North America","Oceania","South America");

	#get local web gui configuration
    $web_local=($config['system']['webgui']['protocol'] != ""?$config['system']['webgui']['protocol']:"http");
    $port = $config['system']['webgui']['port'];
    if($port == "") {
		if($config['system']['webgui']['protocol'] == "http") 
			$port = "80";
		else 
			$port = "443";
    }
    $web_local .= "://127.0.0.1:".$port.'/countryblock.php';
	
	#get all selected countries
	$countries=$config['installedpackages']['countryblock']['config'][0]['topspammers'].",";
	foreach ($continents as $continent){
		$countries.=$config['installedpackages']['countryblock'.strtolower(preg_replace('/ /','',$continent))]['config'][0]['countries'].",";
	}
	$cb_files = explode(",", $countries);
	$ips="";
    foreach ($cb_files as $iso){
		if ($iso <> ""){
		$ips.=file_get_contents('/usr/local/pkg/countryblock/'.$iso.'.txt');
		}
	}
	#create all ip block lists based on gui
	file_put_contents('/usr/local/pkg/cb.txt',$ips, LOCK_EX);
	
	#write white_list to filesystem
	file_put_contents('/usr/local/pkg/cbw.txt',cb_text_area_decode($countryblock_config['whitelist']), LOCK_EX);

	#edit or assign alias "Countryblock" and "CountryblockWL" 
	$aliases=$config['aliases']['alias'];
	print "<pre>";
	foreach($aliases as $cbalias){
		$cb_id =($cb_id == "name"?"name":array_search('Countryblock', $cbalias));
		$cbw_id =($cb_id == "name"?"name":array_search('CountryblockWL', $cbalias));
	}
	if ($cb_id <> 'name'){
		$config['aliases']['alias'][]=array(
					"name"=> 'Countryblock',
				  	"url"=> $web_local.'?cb=1',
				  	"updatefreq"=> "7",
				  	"address"=>"",
				  	"descr"=> "Countryblock deny list",
				  	"type"=> "urltable",
				  	"detail"=> "DO NOT EDIT THIS ALIAS");
	}
	if ($cbw_id <> 'name'){
		$config['aliases']['alias'][]=array(
					"name"=> 'CountryblockWL',
				  	"url"=> $web_local.'?cbw=1',
				  	"updatefreq"=> "7",
				  	"address"=>"",
				  	"descr"=> "Countryblock white list",
				  	"type"=> "urltable",
				  	"detail"=> "DO NOT EDIT THIS ALIAS");
	}
	if ($cb_id <> 'name' || $cbw_id <> 'name'){
		write_config();
	}
	
	# check contryblock filter options
	$rules=$config['filter']['rule'];
	$ifaces = $countryblock_config['inbound_interface'];
    foreach (explode(",", $ifaces) as $i => $iface) {
		${$iface}[0]=array("id" => "",
					"type"=>"pass",
					"tag"=>	"",
					"interface" => $iface,
					"tagged"=> "",
					"max"=>	 "",
					"max-src-nodes"=>"",
					"max-src-conn"=> "",
					"max-src-states"=>"",
					"statetimeout"=>"",
					"statetype"=>"keep state",
					"os"=> "",
					"source"=>array("address"=>"CoutryblockWL"),
    				"destination"=>array("any"=>""),
    				"descr"=>"Countryblock inbound whitelist rule");
		
		${$iface}[1]=array(	"id" => "",
							"type"=>"block",
							"tag"=>	"",
							"interface" => $iface,
							"tagged"=> "",
							"max"=>	 "",
							"max-src-nodes"=>"",
							"max-src-conn"=> "",
							"max-src-states"=>"",
							"statetimeout"=>"",
						    "statetype"=>"keep state",
							"os"=> "",
							"source"=>array("address"=>"Coutryblock"),
    						"destination"=>array("any"=>""),
    						"descr"=>"Countryblock inbound deny rule");

		if ($countryblock_config['enable_log']){
				${$iface}[0]["log"]="";
				${$iface}[1]["log"]="";
		}
	}
	$ifaces = $countryblock_config['outbound_interface'];
    foreach (explode(",", $ifaces) as $i => $iface) {
		${$iface}[2]=array(	"id" => "",
							"type"=>"pass",
							"tag"=>	"",
							"interface" => $iface,
							"tagged"=> "",
							"max"=>	 "",
							"max-src-nodes"=>"",
							"max-src-conn"=> "",
							"max-src-states"=>"",
							"statetimeout"=>"",
							"statetype"=>"keep state",
							"os"=> "",
    						"source"=>array("any"=>""),
    						"destination"=>array("address"=>"CoutryblockWL"),
    						"descr"=>"Countryblock outbound whitelist rule");
		${$iface}[3]= array("id" => "",
							"type"=>"block",
							"tag"=>	"",
							"interface" => $iface,
							"tagged"=> "",
							"max"=>	 "",
							"max-src-nodes"=>"",
							"max-src-conn"=> "",
							"max-src-states"=>"",
							"statetimeout"=>"",
							"statetype"=>"keep state",
							"os"=> "",
    						"source"=>array("any"=>""),
    						"destination"=>array("address"=>"Coutryblock"),
    						"descr"=>"Countryblock inbound deny rule");

		if ($countryblock_config['enable_log']){
				${$iface}[2]["log"]="";
				${$iface}[3]["log"]="";
		}
	}
	$last_iface="";
	foreach ($rules as $rule){
		if ($rule['interface'] <> $last_iface){
			$last_iface = $rule['interface'];
			#apply countryblock rules if enabled
			if ($config['installedpackages']['countryblock']['config'][0]['enable_cb'] == "on")
				foreach (${$rule['interface']} as $cb_rules)
					$new_rules[]=$cb_rules;
		}
		if (!preg_match("/Countryblock.*rule/",$rule['descr']))
			$new_rules[]=$rule;	
	}
	$config['filter']['rule']=$new_rules;
	write_config();
		
	countryblock_sync_on_changes();
}

function countryblock_validate_input($post, &$input_errors) {
	foreach ($post as $key => $value) {
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

function countryblock_php_install_command() {
	include_once '/usr/local/www/countryblock.php';
	countryblock_get_countries();
	sync_package_countryblock();
}

function countryblock_php_deinstall_command() {
	global $config;
	$config['installedpackages']['countryblock']['config'][0]['enable_cb']="";
	write_config();
	sync_package_countryblock();
}

/* Uses XMLRPC to synchronize the changes to a remote node */
function countryblock_sync_on_changes() {
	global $config, $g;
	log_error("[countryblock] countryblock_xmlrpc_sync.php is starting.");
	$synconchanges = $config['installedpackages']['countryblocksync']['config'][0]['synconchanges'];	
	if(!$synconchanges) 
		return;
	foreach ($config['installedpackages']['countryblocksync']['config'] as $rs ){
		foreach($rs['row'] as $sh){
		$sync_to_ip = $sh['ipaddress'];
		$password   = $sh['password'];
		if($password && $sync_to_ip)
			countryblock_do_xmlrpc_sync($sync_to_ip, $password);
		}
	}
	log_error("[countryblock] countryblock_xmlrpc_sync.php is ending.");
}

/* Do the actual XMLRPC sync */
function countryblock_do_xmlrpc_sync($sync_to_ip, $password) {
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
	$xml['countryblock'] = $config['installedpackages']['countryblock'];
	$xml['countryblockacl'] = $config['installedpackages']['countryblockacl'];
	$xml['countryblockrecipients'] = $config['installedpackages']['countryblockrecipients'];
	$xml['countryblockantispam'] = $config['installedpackages']['countryblockantispam'];
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Beginning countryblock XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 250 seconds */
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting countryblock XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "countryblock Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting countryblock XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "countryblock Settings Sync", "");
	} else {
		log_error("countryblock XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* tell countryblock to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/countryblock.inc');\n";
	$execcmd .= "sync_package_countryblock();";
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);

	log_error("countryblock XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting countryblock XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "countryblock Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting countryblock XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "countryblock Settings Sync", "");
	} else {
		log_error("countryblock XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}

}

?>
