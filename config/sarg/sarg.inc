<?php
/* ========================================================================== */
/*
	sarg.inc
    part of pfSense (http://www.pfSense.com)
	Copyright (C) 2007 Joao Henrique F. Freitas
    Copyright (C) 2012 Marcello Coutinho
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

// STATIC VARS
$sarg_proxy=array(	'squid_rc'=>'/usr/local/etc/rc.d/squid.sh',
					'squid_config'=>'/var/squid/logs/access.log',
					'squidguard_config'=>'/usr/local/etc/squidGuard/squidGuard.conf',
					'squidguard_block_log'=>'/var/squidGuard/log/block.log',
					'dansguardian_config'=>'/usr/local/etc/dansguardian/dansguardian.conf',
					'dansguardian_log'=>'/var/log/dansguardian/access.log');

// END STATIC VARS

function sarg_start() {
	global $g, $config;
	
	// reserved
}

function sarg_text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}

function sarg_resync() {
        global $config;
        if (($_POST['Submit'] == 'Save') || !isset($_POST['Submit']))
        	sync_package_sarg();
        if ($_POST['Submit'] == 'Force udpate now')
        	run_sarg();

}
function log_rotate($log_file){
	global $config, $g;
	
	#remove .10 rotate log file
	unlink_if_exists("$log_file".".10");
	#rotate logs from 9 to 0
	$i=9;
	while ($i>=0){
		if (file_exists($log_file.".".$i))
			rename ($log_file.".".$i,$log_file.".".($i+1));
		$i=$i-1;
	}
	#rotate current log
	if (file_exists("$log_file"))
		rename ($log_file,$log_file.".0");	
}
function run_sarg($id=-1) {
	global $config, $g,$sarg_proxy;
	#mount filesystem writeable
	conf_mount_rw();
	$cmd = "/usr/local/bin/sarg";
	if ($id > 0 && is_array($config['installedpackages']['sargschedule']['config'])){
		$args=$config['installedpackages']['sargschedule']['config'][$id]['args'];
		$action=$config['installedpackages']['sargschedule']['config'][$id]['action'];
		}
	else{
		$args=$_POST['args'];
		$action=$_POST['action'];
		}
	log_error("Sarg: force refresh now with '".$args."' args and ".$action." action after sarg finish.");

	mwexec($cmd. " ".$args);
	#check if there is a script to run after file save
	if (is_array($config['installedpackages']['sarg']))
		switch ($config['installedpackages']['sarg']['config'][0]['proxy_server']){
			case "squidguard":
				if ($action =="both" || $action=="rotate"){
					log_error('executing squidguard log rotate after sarg.');
					log_rotate($sarg_proxy['squidguard_block_log']);
					}
			#Leve this case without break to include squid log file on squidguard option
			case "squid":
				if ($action =="both" || $action=="rotate"){
					log_error('executing squid log rotate after sarg.');
					mwexec('squid -k rotate');
					}
				if ($action =="both" || $action=="restart"){
					if (file_exists($sarg_proxy['squid_rc']))
						mwexec_bg($sarg_proxy['squid_rc'].' restart');
					}
			break;
			case "dansguardian":
				if (preg_match('/\w+/',$action) && $action !="none"){
					log_rotate($sarg_proxy['dansguardian_log']);
					log_error('restarting dansguardian after sarg and log rotate.');
					mwexec('/usr/bin/killall -HUP dansguardian');
					}
			break;
		}
	#mount filesystem readonly
	conf_mount_ro();
}

function sync_package_sarg() {
	global $config, $g,$sarg_proxy;
	$update_conf=0;
	#mount filesystem writeable
	conf_mount_rw();
	if (!is_array($config['installedpackages']['sarg']['config']))
		$config['installedpackages']['sarg']['config'][0]=array('report_options'=>'use_graphs,remove_temp_files,main_index,use_comma',
																'user_sort_field'=>'BYTES',
																'report_type'=>'topusers,topsites,sites_users,users_sites,date_time,denied,auth_failures,site_user_time_date,downloads',
																'report_type'=>'u');
	$sarg=$config['installedpackages']['sarg']['config'][0];
	switch ($sarg['proxy_server']){
		case 'dansguardian':
			$access_log= $sarg_proxy['dansguardian_log'];
			$dansguardian_conf=$sarg_proxy['dansguardian_config'];
			$dansguardian_filter_out_date="dansguardian_filter_out_date on";
			$squidguard_conf='squidguard_conf none';
		break;
		case 'squidguard':
			$squidguard_conf='squidguard_conf '.$sarg_proxy['squidguard_config'];
			$redirector_log_format='redirector_log_format #year#-#mon#-#day# #hour# #tmp#/#list#/#tmp#/#tmp#/#url#/#tmp# #ip#/#tmp# #user# #end#';
			#Leve this case without break to include squid log file on squidguard option
		case 'squid':
			$access_log= $sarg_proxy['squid_config'];
			if (is_array($config['installedpackages']['squid']['config']))
				if (file_exists($config['installedpackages']['squid']['config'][0]['log_dir']. '/access.log'))
					$access_log = $config['installedpackages']['squid']['config'][0]['log_dir']. '/access.log';
		break;
	}
	if (!file_exists($access_log)){
		$error="Sarg config error: ".$sarg['proxy_server']." log file ($access_log) does not exists";
		log_error($error);
		file_notice("Sarg", $error, "Sarg Settings", "");
	}
		
	$graphs=(preg_match('/use_graphs/',$sarg['report_options'])?"yes":"no");
	$anonymous_output_files=(preg_match('/anonymous_output_files/',$sarg['report_options'])?"yes":"no");
	$resolve_ip=(preg_match('/resolve_ip/',$sarg['report_options'])?"yes":"no");
	$user_ip=(preg_match('/user_ip/',$sarg['report_options'])?"yes":"no");
	$sort_order=(preg_match('/user_ip/',$sarg['report_options'])?"REVERSE":"NORMAL");
	$remove_temp_files=(preg_match('/remove_temp_files/',$sarg['report_options'])?"yes":"no");
	$main_index=(preg_match('/main_index/',$sarg['report_options'])?"yes":"no");
	$index_tree=(preg_match('/index_tree/',$sarg['report_options'])?"file":"date");
	$overwrite_report=(preg_match('/overwrite_report/',$sarg['report_options'])?"yes":"no");
	$use_comma=(preg_match('/use_comma/',$sarg['report_options'])?"yes":"no");
	$long_url=(preg_match('/long_url/',$sarg['report_options'])?"yes":"no");
	$privacy=(preg_match('/privacy/',$sarg['report_options'])?"yes":"no");
	$date_format=(empty($sarg['report_date_format'])?"u":$sarg['report_date_format']);
	$report_type=preg_replace('/,/',' ',$sarg['report_type']);
	if(!empty($sarg['include_userlist']))
		$include_users="$include_users ".$sarg['include_userlist'];
		
	include("/usr/local/pkg/sarg.template");
	file_put_contents("/usr/local/etc/sarg/sarg.conf", $sg, LOCK_EX);
	file_put_contents('/usr/local/etc/sarg/exclude_users.conf', sarg_text_area_decode($sarg['exclude_userlist']),LOCK_EX);
	file_put_contents('/usr/local/etc/sarg/exclude_hosts.conf', sarg_text_area_decode($sarg['exclude_hostlist']),LOCK_EX);
	file_put_contents('/usr/local/etc/sarg/exclude_codes.conf', sarg_text_area_decode($sarg['exclude_codelist']),LOCK_EX);
	$exclude_string=(empty($sarg['exclude_string'])?"":'exclude_string"'.$sarg['exclude_string']."'");
	if(empty($sarg['include_userlist'])){
		$usertab="none";
		}
	else{
		$usertab="/usr/local/etc/sarg/usertab.conf";
		file_put_contents('/usr/local/etc/sarg/usertab.conf', sarg_text_area_decode($sarg['usertab']),LOCK_EX);
		}
	$dirs=array("/usr/local/www/sarg-reports");
		foreach ($dirs as $dir)
		if (!is_dir($dir))
			mkdir ($dir,0755,true);

	#check cron_tab
	$new_cron=array();
	$cron_found=0;
	if (is_array($config['cron']['item']))
		foreach($config['cron']['item'] as $cron){
			if (preg_match("/usr.local.www.sarg.php/",$cron["command"]))
				$cron_found++;
			else
				$new_cron['item'][]=$cron;
			}
		$cron_cmd="/usr/local/bin/php /usr/local/www/sarg.php";
		$sarg_schedule_id=0;
		if (is_array($config['installedpackages']['sargschedule']['config']))
			foreach ($config['installedpackages']['sargschedule']['config'] as $sarg_schedule){
	 			if(preg_match('/(\d+)m/',$sarg_schedule['frequency'],$matches) && $sarg_schedule['enable']){
					$new_cron['item'][]=array(	"minute" =>	"*/".$matches[1],
												"hour"	 =>	"*",
												"mday" 	 =>	"*",
												"month"  =>	"*",
												"wday"	 =>	"*",
												"who"	 =>	"root",
												"command"=>	$cron_cmd." ".$sarg_schedule_id);
					$config['cron']=$new_cron;
					$cron_found++;
					}
			  	if(preg_match('/(\d+)h/',$sarg_schedule['frequency'],$matches) && $sarg_schedule['enable']){
					$new_cron['item'][]=array(	"minute" =>	"0",
												"hour"	 =>	"*/".$matches[1],
												"mday" 	 =>	"*",
												"month"  =>	"*",
												"wday"	 =>	"*",
												"who"	 =>	"root",
												"command"=>	$cron_cmd." ".$sarg_schedule_id);
					$config['cron']=$new_cron;
					$cron_found++;
		  			}
		  		if(preg_match('/(\d+)d/',$sarg_schedule['frequency'],$matches) && $sarg_schedule['enable']){
					$new_cron['item'][]=array(	"minute" =>	"0",
												"hour"	 =>	"0",
												"mday" 	 =>	"*/".$matches[1],
												"month"  =>	"*",
												"wday"	 =>	"*",
												"who"	 =>	"root",
												"command"=>	$cron_cmd." ".$sarg_schedule_id);
					$config['cron']=$new_cron;
					$cron_found++;
			  		}
			  	$sarg_schedule_id++;
			 	}

	#update cron
	if ($cron_found > 0){
		$config['cron']=$new_cron;
		write_config();
		configure_cron();
		}
	#Write config if any file from filesystem was loaded
	if ($update_conf > 0)
		write_config();
		
	#mount filesystem readonly
	conf_mount_ro();
	
	sarg_sync_on_changes();
}

function sarg_validate_input($post, &$input_errors) {
	global $config,$g;
	foreach ($post as $key => $value) {
		if (empty($value))
			continue;
		# check dansguardian
		if (substr($key, 0, 12) == "proxy_server" && $value == "dansguardian"){
			if (is_array($config['installedpackages']['dansguardianlog'])){
				if ($config['installedpackages']['dansguardianlog']['config'][0]['logfileformat']!=3){
					$input_errors[]='Sarg is only compatible with dansguardian log squid mode';
					$input_errors[]='Please change it on service -> dansguarian -> report and log -> log file format';
				}
			}
			else
				$input_errors[]='dansguardian package not detected';
			}

		# check squidguard
		if (substr($key, 0, 10) == "proxy_server" && $value == "squidguard")
			if (!is_array($config['installedpackages']['squidguardgeneral']))
				$input_errors[]='squidguard package not detected';

		# check squid		
		if (substr($key, 0, 5) == "proxy_server" && $value == "squid"){
			if (is_array($config['installedpackages']['squid']))
				if (!$config['installedpackages']['squid']['log_enabled'])
					$input_errors[]='squidlogs not enabled';
			else
				$input_errors[]='squid package not installed';
				}
			
		if (substr($key, 0, 11) == "description" && !preg_match("@^[a-zA-Z0-9 _/.-]+$@", $value))
				$input_errors[] = "Do not use special characters on description";
		if (substr($key, 0, 8) == "fullfile" && !preg_match("@^[a-zA-Z0-9_/.-]+$@", $value))
				$input_errors[] = "Do not use special characters on filename";
		#check cron option
		if($key == "frequency" && (!preg_match("/^\d+(h|m|d)$/",$value) || $value == 0))
				$input_errors[] = "A valid number with a time reference is required for the field 'Update Frequency'";		
	}
}


/* Uses XMLRPC to synchronize the changes to a remote node */
function sarg_sync_on_changes() {
	global $config, $g;
	
	log_error("[sarg] sarg_xmlrpc_sync.php is starting.");
	$synconchanges = $config['installedpackages']['sargsync']['config'][0]['synconchanges'];	
	if(!$synconchanges) 
		return;
	foreach ($config['installedpackages']['sargsync']['config'] as $rs ){
		foreach($rs['row'] as $sh){
			$sync_to_ip = $sh['ipaddress'];
			$password = $sh['password'];
			if($sh['username'])
				$username = $sh['username'];
			else
				$username = 'admin';
			if($password && $sync_to_ip)
				sarg_do_xmlrpc_sync($sync_to_ip, $username, $password);
			}
	}
	log_error("[sarg] sarg_xmlrpc_sync.php is ending.");
}
/* Do the actual XMLRPC sync */
function sarg_do_xmlrpc_sync($sync_to_ip, $username, $password) {
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
	$xml['sarg'] = $config['installedpackages']['sarg'];
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Beginning sarg XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 250 seconds */
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting sarg XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "sarg Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting sarg XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "sarg Settings Sync", "");
	} else {
		log_error("sarg XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* tell sarg to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/sarg.inc');\n";
	$execcmd .= "sync_package_sarg();";
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);
	
	log_error("sarg XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting sarg XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "sarg Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting sarg XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "sarg Settings Sync", "");
	} else {
		log_error("sarg XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
	
}

function sarg_php_install_command() {
	sync_package_sarg();
}

function sarg_php_deinstall_command() {
	// reserved
}

?>
