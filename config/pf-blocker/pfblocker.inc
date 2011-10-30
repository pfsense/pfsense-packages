<?php
/*
	pfblocker.inc
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
require_once("filter.inc");

function pfb_text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}

function cb_get_real_interface_address($iface) {
	global $config;
	$iface = convert_friendly_interface_to_real_interface_name($iface);
	$line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
	list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);
	return array($ip, long2ip(hexdec($netmask)));
}

function pfblocker_Range2CIDR($ip_min, $ip_max) {
	#function called without any args
	if ($ip_min == "" || $ip_max == "")
		return "";
	#function called with same ip in min and max
	if ($ip_min == $ip_max)
		return $ip_min. "/32";
	#convert ip to decimal numbers
	$ip_min_long=ip2long($ip_min);
	$ip_max_long=ip2long($ip_max);
	#check long results
	if ($ip_min_long == -1 || $ip_max_long  == -1)
		return "";
	#identify bits mask
	$bits=(32 -strlen(decbin($ip_max_long - $ip_min_long)));
	if ($bits < 0)
		return "";
	#identify first ip on range network
	$network=long2ip(bindec(substr(decbin($ip_min_long),0,$bits).preg_replace("/\d/","0",substr(decbin($ip_min_long),0,(32-$bits)))));
	#print decbin($ip_min_long)."\n".$network."\n";
	return $network . "/". (32 -strlen(decbin($ip_max_long - $ip_min_long)));
	}

function sync_package_pfblocker() {
	global $config;
	$pfblocker_config=$config['installedpackages']['pfblocker']['config'][0];
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
    $web_local .= "://127.0.0.1:".$port.'/pfblocker.php';
	
	#get all selected countries
	$countries=$config['installedpackages']['pfblockertopspammers']['config'][0]['countries'].",";
	foreach ($continents as $continent){
		if (is_array($config['installedpackages']['pfblocker'.strtolower(preg_replace('/ /','',$continent))]['config']))
			$countries.=$config['installedpackages']['pfblocker'.strtolower(preg_replace('/ /','',$continent))]['config'][0]['countries'].",";
	}
	$cb_files = explode(",", $countries);

	$pfbdir='/usr/local/pkg/pfblocker';

	#check folders
	if (!is_dir($pfbdir))
		mkdir ($pfbdir,0755);
	if (! is_dir('/var/db/aliastables/'))
		mkdir ('/var/db/aliastables/',0755);
		
	#get custom lists
	$whitelist=pfb_text_area_decode($pfblocker_config['whitelist']);
	$ips_in="";
	$ips_out="";
    foreach ($cb_files as $iso){
		if ($iso <> ""){
			if (file_exists($pfbdir.'/'.$iso.'.txt'))
				switch ($pfblocker_config['countryblock']){
					case "inbound":
						$ips_in.=file_get_contents($pfbdir.'/'.$iso.'.txt');
						break;
					case "outbound":
						$ips_out.=file_get_contents($pfbdir.'/'.$iso.'.txt');
						break;
					case "both":
						$ips_in.=file_get_contents($pfbdir.'/'.$iso.'.txt');
						$ips_out.=file_get_contents($pfbdir.'/'.$iso.'.txt');
						break;
					case "whitelist":
						$whitelist.=file_get_contents($pfbdir.'/'.$iso.'.txt');
					break;		
				}
		}
	}
	
	#Assign IP range lists
	foreach ($pfblocker_config['row'] as $row){
		$md5_url = md5($row['url']);
		#print $row['action']."<br>";
		if (file_exists($pfbdir."/".$md5_url.".txt")){
			${$row['action']}.= file_get_contents($pfbdir.'/'.$md5_url.'.txt');
			}
		else{
			if ($row['format'] == "gz")
				$url_list= gzfile($row['url']);
			else
				$url_list= file($row['url']);
			#extract range lists
			$new_file="";
			foreach ($url_list as $line){
				# CIDR format 192.168.0.0/16
				if (preg_match("/(\d+\.\d+\.\d+\.\d+\/\d+)/",$line,$matches)){
					${$row['action']}.= $matches[1]."\n";
					$new_file.= $matches[1]."\n";
				}
					
				# Network range 192.168.0.0-192.168.0.254
				if (preg_match("/(\d+\.\d+\.\d+\.\d+)-(\d+\.\d+\.\d+\.\d+)/",$line,$matches))
					$cidr= pfblocker_Range2CIDR($matches[1],$matches[2]);
					if ($cidr != ""){
						${$row['action']}.= $cidr."\n";
						$new_file.= $cidr."\n";
					}
			}
			if ($new_file != "")
				file_put_contents($pfbdir.'/'.$md5_url.'.txt',$new_file, LOCK_EX);
		}
		#print $row['url']."<br>" .$md5_url.".txt<br>";
		#var_dump(gzfile($row['url']));
	}
	
	#create all country block lists based on gui
	file_put_contents('/usr/local/pkg/pfb_in.txt',$ips_in, LOCK_EX);
	
	#create all country block lists based on gui
	file_put_contents('/usr/local/pkg/pfb_out.txt',$ips_out, LOCK_EX);
	
	#write white_list to filesystem
	file_put_contents('/usr/local/pkg/pfb_w.txt',$whitelist, LOCK_EX);

	
	#edit or assign alias "pfblockerInbound", "pfblockerOutbound" and "pfblockerWL" 
	$aliases=$config['aliases']['alias'];
	$new_aliases=array();
	$pfBlockerInbound='/var/db/aliastables/pfBlockerInbound.txt';
	if ($ips_in != "" && $config['installedpackages']['pfblocker']['config'][0]['enable_cb'] == "on"){
		#create or reaply alias
		$new_aliases[]=array("name"=> 'pfBlockerInbound',
					  		 "url"=> $web_local.'?pfb=in',
					  		 "updatefreq"=> "7",
					  		 "address"=>"",
					  		 "descr"=> "pfBlocker Inbound deny list",
					  		 "type"=> "urltable",
					  		 "detail"=> "DO NOT EDIT THIS ALIAS");
		#force alias file update
		if (file_exists($pfBlockerInbound))
			file_put_contents($pfBlockerInbound,$ips_in, LOCK_EX);
		}
	else{
		#remove previous aliastable if exist
		if (file_exists($pfBlockerInbound))
			unlink($pfBlockerInbound);
	}
	$pfBlockerOutbound='/var/db/aliastables/pfBlockerOutbound.txt';
	if ($ips_out != "" && $config['installedpackages']['pfblocker']['config'][0]['enable_cb'] == "on"){
		#create or reaply alias
		$new_aliases[]=array("name"=> 'pfBlockerOutbound',
					  		 "url"=> $web_local.'?pfb=out',
					  		 "updatefreq"=> "7",
					  		 "address"=>"",
					  		 "descr"=> "pfBlocker Outbound deny list",
					  		 "type"=> "urltable",
					  		 "detail"=> "DO NOT EDIT THIS ALIAS");
		#force alias file update
		if (file_exists($pfBlockerOutbound))
			file_put_contents($pfBlockerOutbound,$ips_out, LOCK_EX);
		}
	else{
		#remove previous aliastable if exist
		if (file_exists($pfBlockerOutbound))
			unlink($pfBlockerOutbound);
	}
	
	$pfblockerWL='/var/db/aliastables/pfBlockerWL.txt';
	if ($whitelist != "" && $config['installedpackages']['pfblocker']['config'][0]['enable_cb'] == "on"){
		#create or reaply alias
		$new_aliases[]=array("name"=> 'pfBlockerWL',
						  	 "url"=> $web_local.'?pfb=white',
						  	 "updatefreq"=> "7",
						  	 "address"=>"",
						  	 "descr"=> "pfBlocker White list",
						  	 "type"=> "urltable",
						  	 "detail"=> "DO NOT EDIT THIS ALIAS");
		#force alias file update
		if (file_exists($pfblockerWL))
			file_put_contents($pfblockerWL,$whitelist, LOCK_EX);
		}
	else{
		#remove previous aliastable if exist
		if (file_exists($pfblockerWL))
			unlink($pfblockerWL);
	}
	
	if (is_array($aliases))
	  foreach($aliases as $cbalias){
		if (! preg_match("/pfBlocker.*list/",$cbalias['descr']))
			$new_aliases[]=	$cbalias;
	}
	$config['aliases']['alias']=$new_aliases;
	
	# check pfBlocker filter options
	$ifaces = $pfblocker_config['inbound_interface'];
	if ($ifaces != "")
      foreach (explode(",", $ifaces) as $i => $iface) {
    	if ($whitelist != "" && $iface != ""){
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
					"source"=>array("address"=>"pfBlockerWL"),
    				"destination"=>array("any"=>""),
    				"descr"=>"pfBlocker Whitelist rule");

			if ($pfblocker_config['enable_log'])
				${$iface}[0]["log"]="";
    	}
		if ($ips_in != "" && $iface != ""){
			$action=($pfblocker_config['inbound_deny_action']!= ""?$pfblocker_config['inbound_deny_action']:"block");
			${$iface}[1]=array(	"id" => "",
							"type"=>$action,
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
							"source"=>array("address"=>"pfBlockerInbound"),
    						"destination"=>array("any"=>""),
    						"descr"=>"pfBlocker Inbound deny rule");

			if ($pfblocker_config['enable_log'])
				${$iface}[1]["log"]="";
		}
	}
	$ifaces = $pfblocker_config['outbound_interface'];
	if ($ifaces != "")
      foreach (explode(",", $ifaces) as $i => $iface) {
    	if ($whitelist != "" && $iface != ""){
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
    						"destination"=>array("address"=>"pfBlockerWL"),
    						"descr"=>"pfBlocker Whitelist rule");
		if ($pfblocker_config['enable_log'])
				${$iface}[2]["log"]="";			
    	}
		if ($ips_out != "" && $iface != ""){
			$action=($pfblocker_config['outbound_deny_action']!= ""?$pfblocker_config['outbound_deny_action']:"block");
			${$iface}[3]= array("id" => "",
							"type"=>$action,
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
    						"destination"=>array("address"=>"pfBlockerOutbound"),
    						"descr"=>"pfBlocker Outbound deny rule");
			if ($pfblocker_config['enable_log'])
				${$iface}[3]["log"]="";
			
		}

	}
	$last_iface="";
	$rules=$config['filter']['rule'];
	foreach ($rules as $rule){
		if ($rule['interface'] <> $last_iface){
			$last_iface = $rule['interface'];
			#apply pfblocker rules if enabled
			if ($config['installedpackages']['pfblocker']['config'][0]['enable_cb'] == "on" && is_array(${$rule['interface']}))
				foreach (${$rule['interface']} as $cb_rules)
					$new_rules[]=$cb_rules;
		}
		if (!preg_match("/pfBlocker.*rule/",$rule['descr']))
			$new_rules[]=$rule;	
	}
	$config['filter']['rule']=$new_rules;

	#check aliastable size
	preg_match("/(\d+)/",exec("/usr/bin/wc -l /usr/local/pkg/pfb_in.txt"),$matches);
	$count_ips_in = $matches[1];
	preg_match("/(\d+)/",exec("/usr/bin/wc -l /usr/local/pkg/pfb_out.txt"),$matches);
	$count_ips_out = $matches[1];
	preg_match("/(\d+)/",exec("/usr/bin/wc -l /usr/local/pkg/pfb_w.txt"),$matches);
	$count_ips_w = $matches[1];
	
	#get higher value
	$max=$count_ips_in;
	if ($max < $count_ips_out)
		$max = $count_ips_out;
	if ($max < $count_ips_w)
		$max = $count_ips_w;
	$sum=($count_ips_in + $count_ips_out + $count_ips_w);
	#check table size client option
	$table_limit =($config['system']['maximumtableentries']!= ""?$config['system']['maximumtableentries']:"100000");

	#check for possible table size erros
	$error_message="";
    if ($count_ips_in >= $table_limit )
    	$message='pfBlockerInbound alias table is too large. Reduce Inbound list or increase "Firewall Maximum Table Entries" value to at least '.($sum +1000).' in "system - advanced - Firewall/NAT".';
	if ($count_ips_out >= $table_limit )
    	$message='pfBlockerOutbound alias table is too large. Reduce Outbound List or  increase "Firewall Maximum Table Entries" value to at least '.($sum +1000).' in "system - advanced - Firewall/NAT".';
	if ($count_ips_w >= $table_limit )
    	$message='pfBlockerWL alias table is too large. Reduce whitelist or increase "Firewall Maximum Table Entries" value to at least '.($sum +1000).' in "system - advanced - Firewall/NAT ".';
    	
	if ($message == ""){
		#save and apply all changes*/

		#update pfctrl tables
		$tables = array ('pfBlockerOutbound' => 'pfb_out.txt',
				 'pfBlockerInbound'  => 'pfb_in.txt',
				 'pfBlockerWL'  => 'pfb_w.txt');
		foreach ($tables as $table => $pfb_file)
			exec("/sbin/pfctl -t " . escapeshellarg($table) . " -T replace -f /usr/local/pkg/" . escapeshellarg($pfb_file) . " 2>&1", $result_pfb);

		#write config
   		write_config();

		#load filter file after editing
		filter_configure();

		#sync config
		pfblocker_sync_on_changes();
		}
	else{
		log_error("[pfBlocker] ".$message);
		file_notice("pfBlocker", $message, "pfblocker rule apply", "");
	}
}

function pfblocker_validate_input($post, &$input_errors) {
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

function pfblocker_php_install_command() {
	include_once '/usr/local/www/pfblocker.php';
	pfblocker_get_countries();
	sync_package_pfblocker();
}

function pfblocker_php_deinstall_command() {
	global $config;
	$config['installedpackages']['pfblocker']['config'][0]['enable_cb']="";
	write_config();
	sync_package_pfblocker();
}

/* Uses XMLRPC to synchronize the changes to a remote node */
function pfblocker_sync_on_changes() {
	global $config, $g;
	log_error("[pfblocker] pfblocker_xmlrpc_sync.php is starting.");
	$synconchanges = $config['installedpackages']['pfblockersync']['config'][0]['synconchanges'];	
	if(!$synconchanges) 
		return;
	foreach ($config['installedpackages']['pfblockersync']['config'] as $rs ){
		foreach($rs['row'] as $sh){
		$sync_to_ip = $sh['ipaddress'];
		$password   = $sh['password'];
		if($password && $sync_to_ip)
			pfblocker_do_xmlrpc_sync($sync_to_ip, $password);
		}
	}
	log_error("[pfblocker] pfblocker_xmlrpc_sync.php is ending.");
}

/* Do the actual XMLRPC sync */
function pfblocker_do_xmlrpc_sync($sync_to_ip, $password) {
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
	$xml['pfblocker'] = $config['installedpackages']['pfblocker'];
	$xml['pfblockertopspammers'] = $config['installedpackages']['pfblockertopspammers'];
	$xml['pfblockerafrica'] = $config['installedpackages']['pfblockerafrica'];
	$xml['pfblockerantartica'] = $config['installedpackages']['pfblockerantartica'];
	$xml['pfblockerasia'] = $config['installedpackages']['pfblockerasia'];
	$xml['pfblockereurope'] = $config['installedpackages']['pfblockereurope'];
	$xml['pfblockernorthamerica'] = $config['installedpackages']['pfblockernorthamerica'];
	$xml['pfblockeroceania'] = $config['installedpackages']['pfblockeroceania'];
	$xml['pfblockersouthamerica'] = $config['installedpackages']['pfblockersouthamerica'];
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Beginning pfblocker XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 250 seconds */
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting pfblocker XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "pfblocker Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting pfblocker XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "pfblocker Settings Sync", "");
	} else {
		log_error("pfblocker XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* tell pfblocker to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/pfblocker.inc');\n";
	$execcmd .= "sync_package_pfblocker();";
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);

	log_error("pfblocker XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting pfblocker XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "pfblocker Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting pfblocker XMLRPC exec with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "pfblocker Settings Sync", "");
	} else {
		log_error("pfblocker XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}

}

?>
