<?php
/*
	pfblocker.inc
	part of the Postfix package for pfSense
	Copyright (C) 2010 Erik Fonnesbeck
	Copyright (C) 2011-2012 Marcello Coutinho

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
require_once("services.inc");

$uname=posix_uname();
if ($uname['machine']=='amd64')
        ini_set('memory_limit', '250M');

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

function sync_package_pfblocker($cron="") {
	global $g,$config;
	
	# detect boot process or update via cron
	if (is_array($_POST) && $cron==""){
		if (!preg_match("/\w+/",$_POST['__csrf_magic'])){
			log_error("No pfBlocker action during boot process.");
			return;
			}
		}
		
	log_error("Starting pfBlocker sync process.");
	conf_mount_rw();
	
	#apply fetch timeout to pfsense-utils.inc
	$pfsense_utils=file_get_contents('/etc/inc/pfsense-utils.inc');
	$new_pfsense_utils=preg_replace("/\/usr\/bin\/fetch -q/","/usr/bin/fetch -T 5 -q",$pfsense_utils);
	if ($new_pfsense_utils != $pfsense_utils){
		file_put_contents('/etc/inc/pfsense-utils.inc',$new_pfsense_utils, LOCK_EX);
	}
	$pfblocker_enable=$config['installedpackages']['pfblocker']['config'][0]['enable_cb'];
	$pfblocker_config=$config['installedpackages']['pfblocker']['config'][0];
	$table_limit =($config['system']['maximumtableentries']!= ""?$config['system']['maximumtableentries']:"100000");
	
	#get local web gui configuration
	$web_local=($config['system']['webgui']['protocol'] != ""?$config['system']['webgui']['protocol']:"http");
	$port = $config['system']['webgui']['port'];
	if($port == "") {
		if($config['system']['webgui']['protocol'] == "http"){
			$port = "80";
		}
		else{
			$port = "443";
			}
		}
	$web_local .= "://127.0.0.1:".$port.'/pfblocker.php';
	
	#check folders
	$pfbdir='/usr/local/pkg/pfblocker';
	$pfb_alias_dir='/usr/local/pkg/pfblocker_aliases';
	$pfsense_alias_dir='/var/db/aliastables/';
	if (!is_dir($pfbdir)){
		mkdir ($pfbdir,0755);
	}
	if (!is_dir($pfb_alias_dir)){
		mkdir ($pfb_alias_dir,0755);
	}
	if (! is_dir($pfsense_alias_dir)){
		mkdir ($pfsense_alias_dir,0755);
	}

	$continents= array(	"Africa" => "pfBlockerAfrica",
					"Antartica" => "pfBlockerAntartica",
					"Asia" => "pfBlockerAsia",
					"Europe" => "pfBlockerEurope",
					"North America" => "pfBlockerNorthAmerica",
					"Oceania" => "pfBlockerOceania",
					"South America" => "pfBlockerSouthAmerica",
					"Top Spammers" => "pfBlockerTopSpammers");

	#create rules vars and arrays
	$new_aliases=array();
	$new_aliases_list=array();
	$permit_inbound=array();
	$permit_outbound=array();
	$deny_inbound=array();
	$deny_outbound=array();
	$aliases_list=array();
	
	#check if pfblocker is enabled or not.
	$deny_action_inbound=($pfblocker_config['inbound_deny_action']!= ""?$pfblocker_config['inbound_deny_action']:"block");
	$deny_action_outbound=($pfblocker_config['outbound_deny_action']!= ""?$pfblocker_config['outbound_deny_action']:"reject");
	$base_rule= array(	"id" => "",
				"tag"=>	"",
				"tagged"=> "",
				"max"=>	 "",
				"max-src-nodes"=>"",
				"max-src-conn"=> "",
				"max-src-states"=>"",
				"statetimeout"=>"",
				"statetype"=>"keep state",
				"os"=> "");
	
	#############################################
	#          Assign Countries 				#
	#############################################
	foreach ($continents as $continent => $pfb_alias){
		${$continent}="";
		if (is_array($config['installedpackages']['pfblocker'.strtolower(preg_replace('/ /','',$continent))]['config'])){
			$continent_config=$config['installedpackages']['pfblocker'.strtolower(preg_replace('/ /','',$continent))]['config'][0];
			if ($continent_config['action'] != 'Disabled' && $continent_config['action'] != '' && $pfblocker_enable == "on"){
				foreach (explode(",", $continent_config['countries']) as $iso){
					#var_dump ($iso);
					if ($iso <> "" && file_exists($pfbdir.'/'.$iso.'.txt')){
						${$continent} .= file_get_contents($pfbdir.'/'.$iso.'.txt');
					}
				}
				if($continent_config['countries'] != "" && $pfblocker_enable == "on"){
					#write alias files
					file_put_contents($pfb_alias_dir.'/'.$pfb_alias.'.txt',${$continent},LOCK_EX);
					file_put_contents($pfsense_alias_dir.'/'.$pfb_alias.'.txt',${$continent}, LOCK_EX);
					
					#Create alias config
					$new_aliases_list[]=$pfb_alias;
					$new_aliases[]=array(	"name"=> $pfb_alias,
										"url"=> $web_local.'?pfb='.$pfb_alias,
										"updatefreq"=> "32",
										"address"=>"",
										"descr"=> "pfBlocker country list",
										"type"=> "urltable",
										"detail"=> "DO NOT EDIT THIS ALIAS");
					
					#Create rule if action permits
					switch($continent_config['action']){
						case "Deny_Both":
						case "Deny_Outbound":
							$rule = $base_rule;
							$rule["type"] = $deny_action_outbound;
							$rule["descr"]= "$pfb_alias auto rule";
							$rule["source"]=array("any"=>"");
							$rule["destination"]= array("address"=> $pfb_alias);
							if ($pfblocker_config['enable_log']){
								$rule["log"]="";
							}
							$deny_outbound[]=$rule;
							if ($continent_config['action'] != "Deny_Both")
								break;
						case "Deny_Inbound":
							$rule = $base_rule;
							$rule["type"] = $deny_action_inbound;
							$rule["descr"]= "$pfb_alias auto rule";
							$rule["source"]= array("address"=> $pfb_alias);
							$rule["destination"]=array("any"=>"");
							if ($pfblocker_config['enable_log']){
								$rule["log"]="";
							}
							$deny_inbound[]=$rule;					
							break;
						case "Permit_Both":
						case "Permit_Outbound":
							$rule = $base_rule;
							$rule["type"] = "pass";
							$rule["descr"]= "$pfb_alias auto rule";
							$rule["source"]=array("any"=>"");
							$rule["destination"]= array("address"=> $pfb_alias);
							if ($pfblocker_config['enable_log']){
								$rule["log"]="";
							}
							$permit_outbound[]=$rule;					
							if ($continent_config['action'] != "Permit_Both")
								break;
						case "Permit_Inbound":
							$rule = $base_rule;
							$rule["type"] = "pass";
							$rule["descr"]= "$pfb_alias auto rule";
							$rule["source"]= array("address"=> $pfb_alias);
							$rule["destination"]=array("any"=>"");
							if ($pfblocker_config['enable_log']){
								$rule["log"]="";
							}
							$permit_inbound[]=$rule;					
							break;
						}
				
					}
				}
				else{
					#unlink continent list if any
					unlink_if_exists($pfb_alias_dir.'/'.$pfb_alias.'.txt');
				}
	  
			}
			#mark pfctl aliastable for cleanup
			if (!in_array($pfb_alias, $aliases_list)){
				$aliases_list[]=$pfb_alias;
			}
		}
	
		#############################################
		#          Assign lists 					#
		#############################################
		#print "<pre>";
		if($config['installedpackages']['pfblockerlists']['config'] != ""){
			foreach($config['installedpackages']['pfblockerlists']['config'] as $list){
				$alias="pfBlocker".preg_replace("/\W/","",$list['aliasname']);
				#print $list['aliasname'].$list['action']." ".$alias." ".$row['url']."<br>";
				if ($alias != "pfBlocker" && $list['action'] != "" && $list['action'] != 'Disabled' && $pfblocker_enable == "on"){
					#remove empty lists files if any
					if (is_array($list['row'])){
						foreach ($list['row'] as $row){
							#print $list['aliasname'].$list['action'].$list['cron']." ".$alias." ".$row['url']."$update_local<br>";
							if ($row['url'] != ""){
								$md5_url = md5($row['url']);
								if (file_exists($pfbdir."/".$md5_url.".txt")){ 
									${$alias}.= file_get_contents($pfbdir.'/'.$md5_url.'.txt');
								}
								else{
									if ($row['format'] == "gz"){
										$url_list= gzfile($row['url']);
									}
									else{
										$url_list= file($row['url']);
									}
									#extract range lists
									$new_file="";
									if (is_array($url_list)){
										foreach ($url_list as $line){
											# Network range 192.168.0.0-192.168.0.254
											if (preg_match("/(\d+\.\d+\.\d+\.\d+)-(\d+\.\d+\.\d+\.\d+)/",$line,$matches)){
												$a_cidr = ip_range_to_subnet_array($matches[1],$matches[2]);
												if (is_array($a_cidr)) {
													foreach ($a_cidr as $cidr) {
														${$alias}.= $cidr."\n";
														$new_file.= $cidr."\n";
													}
												}
											}
											# CIDR format 192.168.0.0/16
											else if (preg_match("/(\d+\.\d+\.\d+\.\d+\/\d+)/",$line,$matches)){
												${$alias}.= $matches[1]."\n";
												$new_file.= $matches[1]."\n";
											}
											# Single ip addresses 
											else if (preg_match("/(\d+\.\d+\.\d+\.\d+)\s+/",$line,$matches)){
												${$alias}.= $matches[1]."/32\n";
												$new_file.= $matches[1]."/32\n";
											}
										}
									}
									if ($new_file != ""){
										file_put_contents($pfbdir.'/'.$md5_url.'.txt',$new_file, LOCK_EX);
									}
								}
							}
						}
					}
					#check custom network list
					if (pfb_text_area_decode($list['custom']) != ""){
						${$alias}.=pfb_text_area_decode($list['custom'])."\n";
					}
					#save alias file if not empty
					if (${$alias} == ""){
						unlink_if_exists($pfb_alias_dir.'/'.$alias.'.txt');
					}
					else{
						file_put_contents($pfb_alias_dir.'/'.$alias.'.txt',${$alias}, LOCK_EX);
						file_put_contents($pfsense_alias_dir.'/'.$alias.'.txt',${$alias}, LOCK_EX);
						#create alias
						$new_aliases_list[]=$alias;
						$new_aliases[]=array(	"name"=> $alias,
											"url"=> $web_local.'?pfb='.$alias,
											"updatefreq"=> "32",
											"address"=>"",
											"descr"=> "pfBlocker user list",
											"type"=> "urltable",
											"detail"=> "DO NOT EDIT THIS ALIAS");
						#Create rule if action permits
						switch($list['action']){
							case "Deny_Both":
							case "Deny_Outbound":
								$rule = $base_rule;
								$rule["type"] = $deny_action_outbound;
								$rule["descr"]= "$alias auto rule";
								$rule["source"]=array("any"=>"");
								$rule["destination"]= array("address"=> $alias);
								if ($pfblocker_config['enable_log']){
									$rule["log"]="";
								}
								$deny_outbound[]=$rule;
								if ($list['action'] != "Deny_Both")
									break;
							case "Deny_Inbound":
								$rule = $base_rule;
								$rule["type"] = $deny_action_inbound;
								$rule["descr"]= "$alias auto rule";
								$rule["source"]= array("address"=> $alias);
								$rule["destination"]=array("any"=>"");
								if ($pfblocker_config['enable_log']){
									$rule["log"]="";
								}
								$deny_inbound[]=$rule;					
								break;
							case "Permit_Both":
							case "Permit_Outbound":
								$rule = $base_rule;
								$rule["type"] = "pass";
								$rule["descr"]= "$alias auto rule";
								$rule["source"]=array("any"=>"");
								$rule["destination"]= array("address"=> $alias);
								if ($pfblocker_config['enable_log']){
									$rule["log"]="";
								}
								$permit_outbound[]=$rule;					
								if ($list['action'] != "Permit_Both")
									break;
							case "Permit_Inbound":
								$rule = $base_rule;
								$rule["type"] = "pass";
								$rule["descr"]= "$alias auto rule";
								$rule["source"]= array("address"=> $alias);
								$rule["destination"]=array("any"=>"");
								if ($pfblocker_config['enable_log']){
									$rule["log"]="";
								}
								$permit_inbound[]=$rule;					
								break;
						}
					}
					#mark pfctl aliastable for cleanup
					if (!in_array($alias, $aliases_list)){
						$aliases_list[]=$alias;
					}
				}
				else{
					#unlink previous pfblocker alias list if any
					unlink_if_exists($pfb_alias_dir.'/'.$alias.'.txt');
				}
			}
		}
		#update pfsense alias table
		if (is_array($config['aliases']['alias'])){
			$aliases=$config['aliases']['alias'];
			foreach($aliases as $cbalias){
				if (preg_match("/pfBlocker/",$cbalias['name'])){
					#mark pfctl aliastable for cleaning
					if (!in_array($cbalias['name'], $aliases_list)){
						$aliases_list[]=$cbalias['name']; #mark aliastable for cleaning
					}
					#remove previous aliastable file if alias is not defined any more
					if (!in_array($cbalias['name'], $new_aliases_list)){
						unlink_if_exists("/var/db/aliastables/".$cbalias['name'].".txt");
					}
				}
				else{
					$new_aliases[]=	$cbalias;
					if (file_exists($pfb_alias_dir.'/'.$alias.'.txt') && $message ==""){
						preg_match("/(\d+)/",exec("/usr/bin/wc -l ".$pfb_alias_dir.'/'.$alias.'.txt'),$matches);
					}
					if (($matches[1] * 2.1)>= $table_limit ){
						#alias table too large
	    					$message= $alias .' alias table is too large. Reduce networks in list or increase "Firewall Maximum Table Entries" value to at least '. (int)($matches[1] * 2.1) .' in "system - advanced - Firewall/NAT".';
					}
				}
			}
		}
		#apply new alias table to xml
		if ($message == ""){
			$config['aliases']['alias']=$new_aliases;
		}
		#exit;	
		#############################################
		#          Assign rules 					#
		#############################################
		#print "<pre>";
		#var_dump($permit_inbound);
		#var_dump($permit_outbound);
		#var_dump($deny_inbound);
		#var_dump($deny_outbound);			
		#var_dump($pfblocker_config['inbound_interface']);
		#print count($deny_inbound) .count($deny_inbound);
		# Inbound filter options
		$inbound_interfaces = explode(",",$pfblocker_config['inbound_interface']);
		if (count($deny_inbound) > 0 || count($permit_inbound) > 0){
			if($pfblocker_config['inbound_interface'] == ""){
				$message="Unable to apply rules.Inbound Interface option not configured.";
			}
			if (in_array("lo0",$inbound_interfaces)){
				$message="Floating rules are not implemented in pfBlocker yet, choose Inbound Interface other than loopback or change action to Alias only.";
			}
		}
		
		# Outbound filter options
		$outbound_interfaces = explode(",",$pfblocker_config['outbound_interface']);
		if (count($deny_outbound) > 0 || count($permit_outbound) > 0){
			if($pfblocker_config['outbound_interface'] == ""){
				$message="Unable to apply rules.Outbound Interface option not configured.";
			}
			if (in_array("lo0",$outbound_interfaces)){
				$message="Floating rules are not implemented in pfBlocker yet, choose Outbound Interface other than loopback or change action to Alias only.";
			}
		}
		
		if ($message == ""){
			$rules=$config['filter']['rule'];
			$new_rules=array();
			$interfaces_processed=array();
			# The rules in the config come in priority order, 
			# but the interface to which each rule applies can be all mixed up in the list.
			# e.g. some WAN rules, then some LAN rules, then some floating rules, then more
			# LAN rules, some OPT1 rules, some more LAN rules and so on.
			# So we have to allow for this, and only add pfBlocker rules the first time an 
			# interface is found in the rules list.
			foreach ($rules as $rule){
				# If this next rule is for a non-blank interface, different from any interface already processed,
				# then add any needed pfblocker rules to the interface. This puts pfblocker rules at the
				# top of the list for each interface, after any built-in rules (e.g. anti-lockout)
				$found_new_interface = TRUE;
				foreach ($interfaces_processed as $processed_interface){
					if ($processed_interface == $rule['interface']){
						$found_new_interface = FALSE;
					}
				}
				if (($rule['interface'] != "") && ($found_new_interface)){
					$interfaces_processed[] = $rule['interface'];
					#apply pfblocker rules if enabled
				
					#Inbound
					foreach ($inbound_interfaces as $inbound_interface){
						if ($inbound_interface==$rule['interface']){
							#permit rules
							if (is_array($permit_inbound)){
								foreach ($permit_inbound as $cb_rules){
									$cb_rules['interface']=$rule['interface'];
									$new_rules[]=$cb_rules;
								}
							}
							#deny rules
							if (is_array($deny_inbound)){
								foreach ($deny_inbound as $cb_rules){
									$cb_rules['interface']=$rule['interface'];
									$new_rules[]=$cb_rules;
								}
							}
						}
					}
					#Outbound
					foreach ($outbound_interfaces as $outbound_interface){
						if ($outbound_interface==$rule['interface']){
							#permit rules
							if (is_array($permit_outbound)){
								foreach ($permit_outbound as $cb_rules){
									$cb_rules['interface']=$rule['interface'];
									$new_rules[]=$cb_rules;	
								}
							}
							#deny rules
							if (is_array($deny_outbound)){
								foreach ($deny_outbound as $cb_rules){
									$cb_rules['interface']=$rule['interface'];
									$new_rules[]=$cb_rules;	
								}
							}
						}
					}
				}
				#include all rules that are not from pfBlocker
				if (!preg_match("/pfBlocker.*rule/",$rule['descr']) && ($rule['interface'] != "" || $rule['floating']=="yes")){
					$new_rules[]=$rule;
				}
			}
			$config['filter']['rule']=$new_rules;
		}	
	
		if ($message == ""){
			#check cron
			$cron_found=0;
			$cron_cmd="/usr/local/bin/php -q /usr/local/www/pfblocker.php cron";
			if (is_array($config['cron']['item'])){
				$new_cron=array();
				foreach($config['cron']['item'] as $cron){
					if (preg_match("/usr.local.www.pfblocker.php cron/",$cron["command"])){
						#fix 0.1.4.6 missing php path
						if($cron["command"]==$cron_cmd && $pfblocker_enable == "on"){
							$new_cron['item'][]=$cron;
							$cron_found=1;
						}
					}
					else{
						$new_cron['item'][]=$cron;
					}
				}
				if ($cron_found == 0){
					if($pfblocker_enable == "on"){
						$new_cron['item'][]=array(	"minute" =>	"0",
										"hour"	 =>	"*",
										"mday" 	 =>	"*",
										"month"  =>	"*",
										"wday"	 =>	"*",
										"who"	 =>	"root",
										"command"=>	$cron_cmd);
					}
					$config['cron']=$new_cron;
				}
			}
		
			# to be removed in final version
			$aliases_list[]="pfBlockerInbound";	#remove previous version lists
			$aliases_list[]="pfBlockerOutbound";	#remove previous version lists
			$aliases_list[]="pfBlockerWL";		#remove previous version lists
			#exit;
			#update pfctrl tables
			foreach ($aliases_list as $table){
				exec("/sbin/pfctl -t " . escapeshellarg($table) . " -T kill 2>&1", $result_pfb);
			}

			#uncheck donation and credits check box
			$config['installedpackages']['pfblocker']['config'][0]['donation']="";
			$config['installedpackages']['pfblocker']['config'][0]['credits']="";
			#write config
			write_config();

			#update cron
			if ($cron_found == 0){
				configure_cron();
			}
		
			#load filter file after editing
			filter_configure();

			#sync config
			pfblocker_sync_on_changes();
		}
		else{
			log_error("[pfBlocker] ".$message);
			file_notice("pfBlocker", $message, "pfblocker rule apply", "");
		}
		conf_mount_ro();
	}

function pfblocker_validate_input($post, &$input_errors) {
	global $config;
	foreach ($post as $key => $value) {
		if (empty($value))
			continue;
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
			if($password && $sync_to_ip){
				pfblocker_do_xmlrpc_sync($sync_to_ip, $password);
			}
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
		if($config['system']['webgui']['protocol'] == "http"){
			$port = "80";
		}
		else{ 
			$port = "443";
		}
	}
	$synchronizetoip .= $sync_to_ip;

	/* xml will hold the sections to sync */
	$xml = array();
	$xml['pfblocker'] = $config['installedpackages']['pfblocker'];
	$xml['pfblockerlists'] = $config['installedpackages']['pfblockerlists'];
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
	if($g['debug']){
		$cli->setDebug(1);
	}
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
		$error = "A communications error occurred while attempting pfblocker XMLRPC sync with {$url}:{$port} (exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "pfblocker Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting pfblocker XMLRPC exec with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "pfblocker Settings Sync", "");
	} else {
		log_error("pfblocker XMLRPC reload data success with {$url}:{$port} (exec_php).");
	}

}

?>
