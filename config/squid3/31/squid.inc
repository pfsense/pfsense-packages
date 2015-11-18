<?php
/* $Id$ */
/*
	squid.inc
	Copyright (C) 2006-2009 Scott Ullrich
	Copyright (C) 2006 Fernando Lemos
	Copyright (C) 2012 Martin Fuchs	
	Copyright (C) 2012-2013 Marcello Coutinho
	Copyright (C) 2013 Gekkenhuis
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

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('service-utils.inc');

if(!function_exists("filter_configure")) 
	require_once("filter.inc");
	
$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version > 2.0)
	define('SQUID_LOCALBASE', '/usr/pbi/squid-' . php_uname("m"));
else
  define('SQUID_LOCALBASE','/usr/local');
  
define('SQUID_CONFBASE', SQUID_LOCALBASE .'/etc/squid');
define('SQUID_CONFFILE', SQUID_CONFBASE . '/squid.conf');
define('SQUID_BASE', '/var/squid/');
define('SQUID_ACLDIR', '/var/squid/acl');
define('SQUID_PASSWD', '/var/etc/squid.passwd');
define('SQUID_LIB','/var/squid/lib');
define('SQUID_SSL_DB','/var/squid/lib/ssl_db');

$valid_acls = array();

$uname=posix_uname();
if ($uname['machine']=='amd64')
        ini_set('memory_limit', '250M');

        function sq_text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}


function squid_get_real_interface_address($iface) {
	global $config;

	$iface = convert_friendly_interface_to_real_interface_name($iface);
	$line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
	list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);

	return array($ip, long2ip(hexdec($netmask)));
}

function squid_chown_recursive($dir, $user, $group) {
	chown($dir, $user);
	chgrp($dir, $group);
	$handle = opendir($dir) ;
	while (($item = readdir($handle)) !== false) {
		if (($item != ".") && ($item != "..")) {
			$path = "$dir/$item";
			// Recurse unless it's the cache dir, that is slow and rarely necessary.
			if (is_dir($path) && (basename($dir) != "cache"))
				squid_chown_recursive($path, $user, $group);
			elseif (is_file($path)) {
				chown($path, $user);
				chgrp($path, $group);
			}
		}
	}
}

/* setup cache */
function squid_dash_z() {
	global $config;

	//Do nothing if there is no cache config
	if (!is_array($config['installedpackages']['squidcache']['config']))
		return;
		
	$settings = $config['installedpackages']['squidcache']['config'][0];

	// If the cache system is null, there is no need to initialize the (irrelevant) cache dir.
	if ($settings['harddisk_cache_system'] == "null")
		return;

	$cachedir =($settings['harddisk_cache_location'] ? $settings['harddisk_cache_location'] : '/var/squid/cache');

	if(!is_dir($cachedir.'/')) {
		log_error("Creating Squid cache dir $cachedir");
		@mkdir($cachedir, 0755, true);
		// Double check permissions here, should be safe to recurse cache dir if it's small here.
		mwexec("/usr/sbin/chown -R proxy:proxy $cachedir");
	}

	if(!is_dir($cachedir.'/00/')) {
		log_error("Creating squid cache subdirs in $cachedir");
		mwexec(SQUID_LOCALBASE. "/sbin/squid -k shutdown -f " . SQUID_CONFFILE);
		sleep(5);
		mwexec(SQUID_LOCALBASE. "/sbin/squid -k kill -f " . SQUID_CONFFILE);
		// Double check permissions here, should be safe to recurse cache dir if it's small here.
		mwexec("/usr/sbin/chown -R proxy:proxy $cachedir");
		mwexec(SQUID_LOCALBASE. "/sbin/squid -z -f " . SQUID_CONFFILE);
	}

	if(file_exists("/var/squid/cache/swap.state")) {
		chown("/var/squid/cache/swap.state", "proxy");
		chgrp("/var/squid/cache/swap.state", "proxy");
		exec("chmod a+rw /var/squid/cache/swap.state");
	}

}

function squid_is_valid_acl($acl) {
	global $valid_acls;
	if(!is_array($valid_acls))
		return;
	return in_array($acl, $valid_acls);
}

function squid_install_command() {
	global $config;
	global $g;
	update_status("Checking if there is configuration to migrate... One moment please...");
	/* migrate existing csv config fields */
	if (is_array($config['installedpackages']['squidauth']['config']))
		$settingsauth = $config['installedpackages']['squidauth']['config'][0];
	if (is_array($config['installedpackages']['squidcache']['config']))
		$settingscache = $config['installedpackages']['squidcache']['config'][0];
	if (is_array($config['installedpackages']['squidnac']['config']))
		$settingsnac = $config['installedpackages']['squidnac']['config'][0];
	if (is_array($config['installedpackages']['squid']['config']))
		$settingsgen = $config['installedpackages']['squid']['config'][0];
		
	/* Set storage system */
	if ($g['platform'] == "nanobsd") {
		$config['installedpackages']['squidcache']['config'][0]['harddisk_cache_system'] = 'null';
	}

	/* migrate auth settings */
	if (!empty($settingsauth['no_auth_hosts'])) {
		if(strstr($settingsauth['no_auth_hosts'], ",")) {
			$settingsauth['no_auth_hosts'] = base64_encode(implode("\n", explode(",", $settingsauth['no_auth_hosts'])));
			$config['installedpackages']['squidauth']['config'][0]['no_auth_hosts'] = $settingsauth['no_auth_hosts'];
		}
	}

	/* migrate cache settings */
	if (!empty($settingscache['donotcache'])) {
		if(strstr($settingscache['donotcache'], ",")) {
			$settingscache['donotcache'] = base64_encode(implode("\n", explode(",", $settingscache['donotcache'])));
			$config['installedpackages']['squidcache']['config'][0]['donotcache'] = $settingscache['donotcache'];
		}
	}

	/* migrate nac settings */
	if(! empty($settingsnac['allowed_subnets'])) {
		if(strstr($settingsnac['allowed_subnets'], ",")) {
			$settingsnac['allowed_subnets'] = base64_encode(implode("\n", explode(",", $settingsnac['allowed_subnets'])));
			$config['installedpackages']['squidnac']['config'][0]['allowed_subnets'] = $settingsnac['allowed_subnets'];
		}
	}
	if(! empty($settingsnac['banned_hosts'])) {
		if(strstr($settingsnac['banned_hosts'], ",")) {
			$settingsnac['banned_hosts'] = base64_encode(implode("\n", explode(",", $settingsnac['banned_hosts'])));
			$config['installedpackages']['squidnac']['config'][0]['banned_hosts'] = $settingsnac['banned_hosts'];
		}
	}

	if(! empty($settingsnac['banned_macs'])) {
		if(strstr($settingsnac['banned_macs'], ",")) {
			$settingsnac['banned_macs'] = base64_encode(implode("\n", explode(",", $settingsnac['banned_macs'])));
			$config['installedpackages']['squidnac']['config'][0]['banned_macs'] = $settingsnac['banned_macs'];
		}
	}

	if(! empty($settingsnac['unrestricted_hosts'])) {
		if(strstr($settingsnac['unrestricted_hosts'], ",")) {
			$settingsnac['unrestricted_hosts'] = base64_encode(implode("\n", explode(",", $settingsnac['unrestricted_hosts'])));
			$config['installedpackages']['squidnac']['config'][0]['unrestricted_hosts'] = $settingsnac['unrestricted_hosts'];
		}
	}

	if(! empty($settingsnac['unrestricted_macs'])) {
		if(strstr($settingsnac['unrestricted_macs'], ",")) {
			$settingsnac['unrestricted_macs'] = base64_encode(implode("\n", explode(",", $settingsnac['unrestricted_macs'])));
			$config['installedpackages']['squidnac']['config'][0]['unrestricted_macs'] = $settingsnac['unrestricted_macs'];
		}
	}

	if(! empty($settingsnac['whitelist'])) {
		if(strstr($settingsnac['whitelist'], ",")) {
			$settingsnac['whitelist'] = base64_encode(implode("\n", explode(",", $settingsnac['whitelist'])));
			$config['installedpackages']['squidnac']['config'][0]['whitelist'] = $settingsnac['whitelist'];
		}
	}

	if(! empty($settingsnac['blacklist'])) {
		if(strstr($settingsnac['blacklist'], ",")) {
			$settingsnac['blacklist'] = base64_encode(implode("\n", explode(",", $settingsnac['blacklist'])));
			$config['installedpackages']['squidnac']['config'][0]['blacklist'] = $settingsnac['blacklist'];
		}
	}
	
	if(! empty($settingsnac['block_user_agent'])) {
		if(strstr($settingsnac['block_user_agent'], ",")) {
			$settingsnac['block_user_agent'] = base64_encode(implode("\n", explode(",", $settingsnac['block_user_agent'])));
			$config['installedpackages']['squidnac']['config'][0]['block_user_agent'] = $settingsnac['block_user_agent'];
		}
	}
	
	if(! empty($settingsnac['block_reply_mime_type'])) {
		if(strstr($settingsnac['block_reply_mime_type'], ",")) {
			$settingsnac['block_reply_mime_type'] = base64_encode(implode("\n", explode(",", $settingsnac['block_reply_mime_type'])));
			$config['installedpackages']['squidnac']['config'][0]['block_reply_mime_type'] = $settingsnac['block_reply_mime_type'];
		}
	}
		
	/*Migrate reverse settings*/
	if (is_array($config['installedpackages']['squidreverse'])){
		$old_reverse_settings=$config['installedpackages']['squidreverse']['config'][0];
		
		//Settings
		if (!is_array($config['installedpackages']['squidreversegeneral'])){
			$config['installedpackages']['squidreversegeneral']['config'][0]=$old_reverse_settings;
			unset ($config['installedpackages']['squidreversegeneral']['config'][0]['reverse_cache_peer']);
			unset ($config['installedpackages']['squidreversegeneral']['config'][0]['reverse_uri']);
			unset ($config['installedpackages']['squidreversegeneral']['config'][0]['reverse_acl']);
			}
			
		//PEERS
		if (!is_array($config['installedpackages']['squidreversepeer'])){
			foreach (explode("\n",sq_text_area_decode($old_reverse_settings['reverse_cache_peer'])) as $cache_peers)
				foreach (explode(";",$cache_peers) as $cache_peer)
					$config['installedpackages']['squidreversepeer']['config'][]=array('description'=>'migrated',
																					'enable'=> 'on',
									  												'name'=> $cache_peer[0],
																					'port'=> $cache_peer[1],
																					'protocol' => $cache_peer[2]);
			}
			
		//MAPPINGS
		if (!is_array($config['installedpackages']['squidreverseuri'])){
			foreach (explode("\n",sq_text_area_decode($old_reverse_settings['reverse_acl'])) as $acls){
				foreach (explode(";",$acls) as $acl)
					array_push(${'peer_'.$acl[0]},$acl[1]);
				}
			foreach (explode("\n",sq_text_area_decode($old_reverse_settings['reverse_uri'])) as $uris)
				foreach (explode(";",$uris) as $uri){
					$peer_list=(is_array(${'peer_'.$uri[0]})?implode(",",${'peer_'.$uri[0]}):"");
					$config['installedpackages']['squidreverseuri']['config'][]=array('description'=>'migrated',
																					'enable'=> 'on',
									  												'name'=> $uri[0],
																					'uri'=> $uri[1],
																					'vhost' => $uri[2],
																					'peers'=>$peer_list);
				}
			}
	}
			
	update_status("Writing configuration... One moment please...");
	
	write_config();

	/* create cache */
	update_status("Creating squid cache pools... One moment please...");
	squid_dash_z();
	/* make sure pinger is executable */
	if(file_exists(SQUID_LOCALBASE. "/libexec/squid/pinger"))
		exec("/bin/chmod a+x ". SQUID_LOCALBASE. "/libexec/squid/pinger");
	if(file_exists("/usr/local/etc/rc.d/squid"))
		exec("/bin/rm /usr/local/etc/rc.d/squid");
	squid_write_rcfile();
	if(file_exists("/usr/local/pkg/swapstate_check.php"))
		exec("/bin/chmod a+x /usr/local/pkg/swapstate_check.php");
	write_rcfile(array(
		"file" => "sqp_monitor.sh",
		"start" => "/usr/local/pkg/sqpmon.sh &",
		"stop" => "ps awux | grep \"sqpmon\" | grep -v \"grep\" | grep -v \"php\" | awk '{ print $2 }' | xargs kill"));

	foreach (array(	SQUID_CONFBASE,
			SQUID_ACLDIR,
			SQUID_BASE,
			SQUID_LIB,
			SQUID_SSL_DB ) as $dir) {
		@mkdir($dir, 0755, true);
		squid_chown_recursive($dir, 'proxy', 'proxy');
	}

	/* kill any running proxy alarm scripts */
	update_status("Checking for running processes... One moment please...");
	log_error("Stopping any running proxy monitors");
	mwexec("/usr/local/etc/rc.d/sqp_monitor.sh stop");
	sleep(1);

	if (!file_exists(SQUID_CONFBASE . '/mime.conf') && file_exists(SQUID_CONFBASE . '/mime.conf.default'))
		copy(SQUID_CONFBASE . '/mime.conf.default', SQUID_CONFBASE . '/mime.conf');

	update_status("Checking cache... One moment please...");
	squid_dash_z();

	if (!is_service_running('squid')) {
		update_status("Starting... One moment please...");
		log_error("Starting Squid");
		mwexec_bg(SQUID_LOCALBASE. "/sbin/squid -f " . SQUID_CONFFILE);
	} else {
		update_status("Reloading Squid for configuration sync... One moment please...");
		log_error("Reloading Squid for configuration sync");
		mwexec(SQUID_LOCALBASE. "/sbin/squid -k reconfigure -f " . SQUID_CONFFILE);
	}

	/* restart proxy alarm scripts */
	log_error("Starting a proxy monitor script");
	mwexec_bg("/usr/local/etc/rc.d/sqp_monitor.sh start");

	update_status("Reconfiguring filter... One moment please...");
	filter_configure();
}

function squid_deinstall_command() {
	global $config, $g;
	$plswait_txt = "This operation may take quite some time, please be patient.  Do not press stop or attempt to navigate away from this page during this process.";
	squid_install_cron(false);
	if (is_array($config['installedpackages']['squidcache']))
		$settings = $config['installedpackages']['squidcache']['config'][0];
	else
		$settings = array();
	$cachedir =($settings['harddisk_cache_location'] ? $settings['harddisk_cache_location'] : '/var/squid/cache');
	$logdir = ($settings['log_dir'] ? $settings['log_dir'] : '/var/squid/logs');
	update_status("Removing swap.state ... One moment please...");
	update_output_window("$plswait_txt");
	mwexec('rm -rf $cachedir/swap.state');
	mwexec('rm -rf $logdir');
	update_status("Finishing package cleanup.");
	mwexec("/usr/local/etc/rc.d/sqp_monitor.sh stop");
	mwexec('rm -f /usr/local/etc/rc.d/sqp_monitor.sh');
	mwexec("ps awux | grep \"squid\" | grep -v \"grep\" | awk '{ print $2 }' | xargs kill");
	mwexec("ps awux | grep \"dnsserver\" | grep -v \"grep\" | awk '{ print $2 }' | xargs kill");
	mwexec("ps awux | grep \"unlinkd\" | grep -v \"grep\" | awk '{ print $2 }' | xargs kill");
	update_status("Reloading filter...");
	filter_configure();
}

function squid_before_form_general(&$pkg) {
	$values = get_dir(SQUID_CONFBASE . '/errors/');
	// Get rid of '..' and '.' and ...
	array_shift($values);
	array_shift($values);
	array_shift($values);
	array_shift($values);

	$name = array();
	foreach ($values as $value)
		$names[] = implode(" ", explode("_", $value));

	$i = 0;
	foreach ($pkg['fields']['field'] as $field) {
		if ($field['fieldname'] == 'error_language')
			break;
		$i++;
	}
	$field = &$pkg['fields']['field'][$i];

	for ($i = 0; $i < count($values) - 1; $i++)
		$field['options']['option'][] = array('name' => $names[$i], 'value' => $values[$i]);
}

function squid_validate_general($post, &$input_errors) {
	global $config;
	if (is_array($config['installedpackages']['squid']))
		$settings = $config['installedpackages']['squid']['config'][0];
	else
		$settings = array();
	$port = ($settings['proxy_port'] ? $settings['proxy_port'] : 3128);
	$port = $post['proxy_port'] ? $post['proxy_port'] : $port;

	$icp_port = trim($post['icp_port']);
	if (!empty($icp_port) && !is_port($icp_port))
		$input_errors[] = 'You must enter a valid port number in the \'ICP port\' field';


	if (substr($post['log_dir'], -1, 1) == '/')
		$input_errors[] = 'You may not end log location with an / mark';

	if ($post['log_dir']{0} != '/')
		$input_errors[] = 'You must start log location with a / mark';
	if (strlen($post['log_dir']) <= 3)
		$input_errors[] = "That is not a valid log location dir";

	$log_rotate = trim($post['log_rotate']);
	if (!empty($log_rotate) && (!is_numeric($log_rotate) or ($log_rotate < 1)))

		$input_errors[] = 'You must enter a valid number of days in the \'Log rotate\' field';

	$webgui_port = $config['system']['webgui']['port'];
	if(($config['system']['webgui']['port'] == "") && ($config['system']['webgui']['protocol'] == "http")) {
		$webgui_port = 80;
	}
	if(($config['system']['webgui']['port'] == "") && ($config['system']['webgui']['protocol'] == "https")) {
		$webgui_port = 443;
	}

	if (($post['transparent_proxy'] != 'on') && ($port == $webgui_port)) {
		$input_errors[] = "You can not run squid on the same port as the webgui";
	}

	foreach (array('defined_ip_proxy_off') as $hosts) {
		foreach (explode(";", $post[$hosts]) as $host) {
			$host = trim($host);
			if (!empty($host) && !is_ipaddr($host) && !is_alias($host) && !is_hostname($host) && !is_subnet($host))
				$input_errors[] = "The entry '$host' is not a valid IP address, hostname, or alias";
		}
	}
	foreach (array('defined_ip_proxy_off_dest') as $hosts) {
		foreach (explode(";", $post[$hosts]) as $host) {
			$host = trim($host);
			if (!empty($host) && !is_ipaddr($host) && !is_alias($host) && !is_hostname($host) && !is_subnet($host))
				$input_errors[] = "The entry '$host' is not a valid IP address, hostname, or alias";
		}
	}

  if(!empty($post['dns_nameservers'])) {
	 $altdns = explode(";", ($post['dns_nameservers']));
	 foreach ($altdns as $dnssrv) {
    if (!is_ipaddr($dnssrv))
      $input_errors[] = 'You must enter a valid IP address in the \'Alternate DNS servers\' field';
	  break;
	 }}
}

function squid_validate_upstream($post, &$input_errors) {
	if ($post['enabled'] == 'on') {
		$addr = trim($post['proxyaddr']);
		if (empty($addr))
			$input_errors[] = 'The field \'Hostname\' is required';
		else {
			if (!is_ipaddr($addr) && !is_domain($addr))
				$input_errors[] = 'You must enter a valid IP address or host name in the \'Proxy hostname\' field';
		}

		foreach (array('proxyport' => 'TCP port', 'icpport' => 'ICP port') as $field => $name) {
			$port = trim($post[$field]);
			if (empty($port))
				$input_errors[] = "The field '$name' is required";
			else {
					if (!is_port($port))
					$input_errors[] = "The field '$name' must contain a valid port number, between 0 and 65535";
			}
		}
	}
}

function squid_validate_cache($post, &$input_errors) {
	$num_fields = array(	'harddisk_cache_size' => 'Hard disk cache size',
				'memory_cache_size' => 'Memory cache size',
				'maximum_object_size' => 'Maximum object size',
	);
	foreach ($num_fields as $field => $name) {
		$value = trim($post[$field]);
		if (!is_numeric($value) || ($value < 0))
			$input_errors[] = "You must enter a valid value for '$field'";
	}

	$value = trim($post['minimum_object_size']);
	if (!is_numeric($value) || ($value < 0))
		$input_errors[] = 'You must enter a valid value for \'Minimum object size\'';

  if (!empty($post['cache_swap_low'])) {
    $value = trim($post['cache_swap_low']);
	  if (!is_numeric($value) || ($value > 100))
	  $input_errors[] = 'You must enter a valid value for \'Low-water-mark\'';
	}

  if (!empty($post['cache_swap_high'])) {		
	 $value = trim($post['cache_swap_high']);
	 if (!is_numeric($value) || ($value > 100))
		$input_errors[] = 'You must enter a valid value for \'High-water-mark\'';		
  }

	if ($post['donotcache'] != "") {
		foreach (split("\n", $post['donotcache']) as $host) {
			$host = trim($host);
			if (!is_ipaddr($host) && !is_domain($host))
				$input_errors[] = "The host '$host' is not a valid IP or host name";
		}
	}

	squid_dash_z();

}

function squid_validate_nac($post, &$input_errors) {
	$allowed_subnets = explode("\n", $post['allowed_subnets']);
	foreach ($allowed_subnets as $subnet) {
		$subnet = trim($subnet);
		if (!empty($subnet) && !is_subnet($subnet))
			$input_errors[] = "The subnet '$subnet' is not a valid CIDR range";
	}

	foreach (array(	'unrestricted_hosts', 'banned_hosts') as $hosts) {
		
		if (preg_match_all("@([0-9.]+)(/[0-9.]+|)@",$_POST[$hosts],$matches)){
			for ($x=0;$x < count($matches[1]);$x++){
				if ($matches[2][$x] == ""){
					if (!is_ipaddr($matches[1][$x]))
						$input_errors[] = "'{$matches[1][$x]}' is not a valid IP address";
					}
				else{
					if (!is_subnet($matches[0][$x]))
						$input_errors[] = "The subnet '{$matches[0][$x]}' is not a valid CIDR range";
					}
				}
		}
	}

	foreach (array('unrestricted_macs', 'banned_macs') as $macs) {
		foreach (explode("\n", $post[$macs]) as $mac) {
			$mac = trim($mac);
			if (!empty($mac) && !is_macaddr($mac))
				$input_errors[] = "The mac '$mac' is not a valid MAC address";
		}
	}

	foreach (explode(",", $post['timelist']) as $time) {
		$time = trim($time);
		if (!empty($time) && !squid_is_timerange($time))
			$input_errors[] = "The time range '$time' is not a valid time range";
	}

  if(!empty($post['ext_cachemanager'])) {
	 $extmgr = explode(";", ($post['ext_cachemanager']));
	 foreach ($extmgr as $mgr) {
    if (!is_ipaddr($mgr))
      $input_errors[] = 'You must enter a valid IP address in the \'External Cache Manager\' field';
	 }}
}

function squid_validate_traffic($post, &$input_errors) {
	$num_fields = array(	'max_download_size' => 'Maximum download size',
				'max_upload_size' => 'Maximum upload size',
				'perhost_throttling' => 'Per-host bandwidth throttling',
				'overall_throttling' => 'Overall bandwidth throttling',
	);
	foreach ($num_fields as $field => $name) {
		$value = trim($post[$field]);
		if (!is_numeric($value) || ($value < 0))
			$input_errors[] = "The field '$name' must contain a positive number";
	}

  if (!empty($post['quick_abort_min'])) {		
	 $value = trim($post['quick_abort_min']);
     if (!is_numeric($value))
	   $input_errors[] = "The field 'Finish when remaining KB' must contain a positive number";	
  }	
  
  if (!empty($post['quick_abort_max'])) {		
	 $value = trim($post['quick_abort_max']);
     if (!is_numeric($value))
	   $input_errors[] = "The field 'Abort when remaining KB' must contain a positive number";	
  }	
		
  if (!empty($post['quick_abort_pct'])) {		
	 $value = trim($post['quick_abort_pct']);
     if (!is_numeric($value) || ($value > 100))
	   $input_errors[] = "The field 'Finish when remaining %' must contain a percentage";	
  }	
}

function squid_validate_reverse($post, &$input_errors) {

	if(!empty($post['reverse_ip'])) {
		$reverse_ip = explode(";", ($post['reverse_ip']));
		foreach ($reverse_ip as $reip) {
		if (!is_ipaddr(trim($reip)))
			$input_errors[] = 'You must enter a valid IP address in the \'User-defined reverse-proxy IPs\' field'.' -> \''.$reip.'\' is invalid.';
	}}	
		
	$fqdn = trim($post['reverse_external_fqdn']);
	if (!empty($fqdn) && !is_domain($fqdn))
		$input_errors[] = 'The field \'external FQDN\' must contain a valid domain name';

	$port = trim($post['reverse_http_port']);
	if (!empty($port) && !is_port($port))
		$input_errors[] = 'The field \'reverse HTTP port\' must contain a valid port number';

	$port = trim($post['reverse_https_port']);
	if (!empty($port) && !is_port($port))
		$input_errors[] = 'The field \'reverse HTTPS port\' must contain a valid port number';

	if ($post['reverse_ssl_cert'] == 'none')
		$input_errors[] = 'A valid certificate for the external interface must be selected';

    if (($post['reverse_https'] != 'on') && ($post['reverse_owa'] == 'on')) {
      $input_errors[] = "You have to enable reverse HTTPS before enabling OWA support.";
      }

	if(!empty($post['reverse_owa_ip'])) {
		$reverse_owa_ip = explode(";", ($post['reverse_owa_ip']));
		foreach ($reverse_owa_ip as $reowaip) {
		if (!is_ipaddr(trim($reowaip)))
			$input_errors[] = 'You must enter a valid IP address in the \'CAS-Array / OWA frontend IP address\' field'.' -> \''.$reowaip.'\' is invalid.';
	}}	
	
    $contents = $post['reverse_cache_peer'];
    if(!empty($contents)) {
      $defs = explode("\r\n", ($contents));
        foreach ($defs as $def) {
          $cfg = explode(";",($def));
            if (!is_ipaddr($cfg[1])) 
              $input_errors[] = "please choose a valid IP in the cache peer configuration.";
            if (!is_port($cfg[2]))
              $input_errors[] = "please choose a valid port in the cache peer configuration.";
            if (($cfg[3] != 'HTTPS') && ($cfg[3] != 'HTTP'))
              $input_errors[] = "please choose HTTP or HTTPS in the cache peer configuration.";
    }}


}

function squid_validate_auth($post, &$input_errors) {
	$num_fields = array(	array('auth_processes', 'Authentication processes', 1),
				array('auth_ttl', 'Authentication TTL', 0),
	);
	foreach ($num_fields as $field) {
		$value = trim($post[$field[0]]);
		if (!empty($value) && (!is_numeric($value) || ($value < $field[2])))
			$input_errors[] = "The field '{$field[1]}' must contain a valid number greater than {$field[2]}";
	}

	$auth_method = $post['auth_method'];
	if (($auth_method != 'none') && ($auth_method != 'local')) {
		$server = trim($post['auth_server']);
		if (empty($server))
			$input_errors[] = 'The field \'Authentication server\' is required';
		else if (!is_ipaddr($server) && !is_domain($server))
			$input_errors[] = 'The field \'Authentication server\' must contain a valid IP address or domain name';

		$port = trim($post['auth_server_port']);
		if (!empty($port) && !is_port($port))
			$input_errors[] = 'The field \'Authentication server port\' must contain a valid port number';

		switch ($auth_method) {
			case 'ldap':
				$user = trim($post['ldap_user']);
				if (empty($user))
					$input_errors[] = 'The field \'LDAP server user DN\' is required';
				else if (!$user)
					$input_errors[] = 'The field \'LDAP server user DN\' must be a valid domain name';
				break;
			case 'radius':
				$secret = trim($post['radius_secret']);
				if (empty($secret))
					$input_errors[] = 'The field \'RADIUS secret\' is required';
				break;
			case 'msnt':
				foreach (explode(",", trim($post['msnt_secondary'])) as $server) {
					if (!empty($server) && !is_ipaddr($server) && !is_domain($server))
						$input_errors[] = "The host '$server' is not a valid IP address or domain name";
				}
				break;
		}

		$no_auth = explode("\n", $post['no_auth_hosts']);
		foreach ($no_auth as $host) {
			$host = trim($host);
			if (!empty($host) && !is_subnet($host))
				$input_errors[] = "The host '$host' is not a valid CIDR range";
		}
	}
}

function squid_install_cron($should_install) {
	global $config, $g;
	if($g['booting']==true) 
		return;
	$rotate_is_installed = false;
	$swapstate_is_installed = false;

	if(!$config['cron']['item'])
		return;
		
	if (is_array($config['installedpackages']['squidcache']))	
		$settings = $config['installedpackages']['squidcache']['config'][0];
	else
		$settings = array();
	
	$x=0;
	$rotate_job_id=-1;
	$swapstate_job_id=-1;
	foreach($config['cron']['item'] as $item) {
		if(strstr($item['task_name'], "squid_rotate_logs")) {
			$rotate_job_id = $x;
		} elseif(strstr($item['task_name'], "squid_check_swapstate")) {
	  		$swapstate_job_id = $x;
		}
		$x++;
	}
	$need_write = false;
	switch($should_install) {
		case true:
			$cachedir =($settings['harddisk_cache_location'] ? $settings['harddisk_cache_location'] : '/var/squid/cache');
			if($rotate_job_id < 0) {
				$cron_item = array();
				$cron_item['task_name'] = "squid_rotate_logs";
				$cron_item['minute'] = "0";
				$cron_item['hour'] = "0";
				$cron_item['mday'] = "*";
				$cron_item['month'] = "*";
				$cron_item['wday'] = "*";
				$cron_item['who'] = "root";
				$cron_item['command'] = "/bin/rm {$cachedir}/swap.state; ". SQUID_LOCALBASE."/sbin/squid -k rotate -f " . SQUID_CONFFILE;
				/* Add this cron_item as a new entry at the end of the item array. */
				$config['cron']['item'][] = $cron_item;
				$need_write = true;
			}
			if($swapstate_job_id < 0) {
				$cron_item = array();
				$cron_item['task_name'] = "squid_check_swapstate";
				$cron_item['minute'] = "*/15";
				$cron_item['hour'] = "*";
				$cron_item['mday'] = "*";
				$cron_item['month'] = "*";
				$cron_item['wday'] = "*";
				$cron_item['who'] = "root";
				$cron_item['command'] = "/usr/local/pkg/swapstate_check.php";
				/* Add this cron_item as a new entry at the end of the item array. */
				$config['cron']['item'][] = $cron_item;
				$need_write = true;
			}
			if ($need_write) {		
				parse_config(true);
				write_config("Adding Squid Cron Jobs");
			}
			break;
		case false:
			if($rotate_job_id >= 0) {
				unset($config['cron']['item'][$rotate_job_id]);
				$need_write = true;
			}
			if($swapstate_job_id >= 0) {
				unset($config['cron']['item'][$swapstate_job_id]);
				$need_write = true;
			}
			if ($need_write) {
				parse_config(true);
				write_config("Removing Squid Cron Jobs");
			}
			break;
	}
	configure_cron();
}

function squid_resync_general() {
	global $g, $config, $valid_acls;

	if (is_array($config['installedpackages']['squid']))
		$settings = $config['installedpackages']['squid']['config'][0];
	else
		$settings=array();
	$conf = "# This file is automatically generated by pfSense\n";
	$conf .= "# Do not edit manually !\n";

	$port = ($settings['proxy_port'] ? $settings['proxy_port'] : 3128);
	$ifaces = ($settings['active_interface'] ? $settings['active_interface'] : 'lan');
	$real_ifaces = array();
	foreach (explode(",", $ifaces) as $i => $iface) {
		$real_ifaces[] = squid_get_real_interface_address($iface);
		if($real_ifaces[$i][0]) {
			$conf .= "http_port {$real_ifaces[$i][0]}:$port\n";
		}
	}
	if (($settings['transparent_proxy'] == 'on')) {
		$conf .= "http_port 127.0.0.1:" . $settings['proxy_port'] . " intercept\n";
	}
	$icp_port = ($settings['icp_port'] ? $settings['icp_port'] : 7);
	$dns_v4_first= ($settings['dns_v4_first'] == "on" ? "on" : "off" );
	$pidfile = "{$g['varrun_path']}/squid.pid";
	$language = ($settings['error_language'] ? $settings['error_language'] : 'en');
	$icondir = SQUID_CONFBASE . '/icons';
	$hostname = ($settings['visible_hostname'] ? $settings['visible_hostname'] : 'localhost');
	$email = ($settings['admin_email'] ? $settings['admin_email'] : 'admin@localhost');

	$logdir = ($settings['log_dir'] ? $settings['log_dir'] : '/var/squid/logs');
	if (! is_dir($logdir)){
		@mkdir($logdir, 0755, true);
		squid_chown_recursive($logdir, 'proxy', 'proxy');
		}
	$logdir_cache = $logdir . '/cache.log';
	$logdir_access = ($settings['log_enabled'] == 'on' ? $logdir . '/access.log' : '/dev/null');

	$conf .= <<<EOD
icp_port {$icp_port}
dns_v4_first {$dns_v4_first}
pid_filename {$pidfile}
cache_effective_user proxy
cache_effective_group proxy
error_default_language {$language}
icon_directory {$icondir}
visible_hostname {$hostname}
cache_mgr {$email}
access_log {$logdir_access}
cache_log {$logdir_cache}
cache_store_log none
sslcrtd_children 0

EOD;

// Per squid docs, setting logfile_rotate to 0 is safe and causes a simple close/reopen.
// Rotating also ensures that swap.state is rewritten, so is useful even if the logs
//  are not being rotated.
$rotate = empty($settings['log_rotate']) ? 0 : $settings['log_rotate'];
$conf .= "logfile_rotate {$rotate}\n";
squid_install_cron(true);

	$conf .= <<<EOD
shutdown_lifetime 3 seconds

EOD;

	if ($settings['allow_interface'] == 'on') {
		$src = '';
		foreach ($real_ifaces as $iface) {
			list($ip, $mask) = $iface;
			$ip = long2ip(ip2long($ip) & ip2long($mask));
			$mask = 32-log((ip2long($mask) ^ ip2long('255.255.255.255'))+1,2);
			$src .= " $ip/$mask";
		}
		$conf .= "# Allow local network(s) on interface(s)\n";
		$conf .= "acl localnet src $src\n";
		$valid_acls[] = 'localnet';
	}
	if ($settings['disable_xforward']) $conf .= "forwarded_for off\n";
	if ($settings['disable_via']) $conf .= "via off\n";
	if ($settings['disable_squidversion']) $conf .= "httpd_suppress_version_string on\n";
	if (!empty($settings['uri_whitespace'])) $conf .= "uri_whitespace {$settings['uri_whitespace']}\n";
  else $conf .= "uri_whitespace strip\n"; //only used for first run

	if(!empty($settings['dns_nameservers'])) {
	  $altdns = explode(";", ($settings['dns_nameservers']));
    $conf .= "dns_nameservers ";
	 foreach ($altdns as $dnssrv) {
    $conf .= $dnssrv." ";
	 }
//  $conf .= "\n";  //Kill blank line after DNS-Servers
   }
	
  return $conf;
}


function squid_resync_cache() {
	global $config, $g;
	if (is_array($config['installedpackages']['squidcache']))
		$settings = $config['installedpackages']['squidcache']['config'][0];
	else
		$settings = array();
	//apply cache settings
	$cachedir =($settings['harddisk_cache_location'] ? $settings['harddisk_cache_location'] : '/var/squid/cache');
	$disk_cache_size = ($settings['harddisk_cache_size'] ? $settings['harddisk_cache_size'] : 100);
	$level1 = ($settings['level1_subdirs'] ? $settings['level1_subdirs'] : 16);
	$memory_cache_size = ($settings['memory_cache_size'] ? $settings['memory_cache_size'] : 8);
	$max_objsize = ($settings['maximum_object_size'] ? $settings['maximum_object_size']." KB" : "10 KB");
	$min_objsize = ($settings['minimum_object_size'] ? $settings['minimum_object_size'] : 0);
	$max_objsize_in_mem = ($settings['maximum_objsize_in_mem'] ? $settings['maximum_objsize_in_mem'] : 32);
	$cache_policy = ($settings['cache_replacement_policy'] ? $settings['cache_replacement_policy'] : 'heap LFUDA');
	$memory_policy = ($settings['memory_replacement_policy'] ? $settings['memory_replacement_policy'] : 'heap GDSF');
	$offline_mode = ($settings['enable_offline'] == 'on' ? 'on' : 'off');
	$conf = '';
	if (!isset($settings['harddisk_cache_system'])) {
		if ($g['platform'] == "nanobsd" || !is_array ($config['installedpackages']['squidcache']['config']))
			$disk_cache_system = 'null';
		else
			$disk_cache_system = 'ufs';
		}
	else{
		$disk_cache_system = $settings['harddisk_cache_system'];
		}
	#'null' storage type dropped. In-memory cache is always present. Remove all cache_dir options to prevent on-disk caching.
	if ($disk_cache_system != "null") {
		$disk_cache_opts =	"cache_dir {$disk_cache_system} {$cachedir} {$disk_cache_size} {$level1} 256";
	}
//check dynamic content
if(empty($settings['cache_dynamic_content'])){
	$conf.='acl dynamic urlpath_regex cgi-bin \?'."\n";
	$conf.="cache deny dynamic\n";
}
else{
	if(preg_match('/youtube/',$settings['refresh_patterns'])){
		$conf.=<<<EOC
# Break HTTP standard for flash videos. Keep them in cache even if asked not to.
refresh_pattern -i \.flv$ 10080 90% 999999 ignore-no-cache override-expire ignore-private

# Let the clients favorite video site through with full caching
acl youtube dstdomain .youtube.com
cache allow youtube

EOC;
	}
	if(preg_match('/windows/',$settings['refresh_patterns'])){
		$conf.=<<<EOC

# Windows Update refresh_pattern
range_offset_limit -1
refresh_pattern -i microsoft.com/.*\.(cab|exe|ms[i|u|f]|asf|wm[v|a]|dat|zip) 4320 80% 43200 reload-into-ims
refresh_pattern -i windowsupdate.com/.*\.(cab|exe|ms[i|u|f]|asf|wm[v|a]|dat|zip) 4320 80% 43200 reload-into-ims
refresh_pattern -i my.windowsupdate.website.com/.*\.(cab|exe|ms[i|u|f]|asf|wm[v|a]|dat|zip) 4320 80% 43200 reload-into-ims

EOC;
		}

if(preg_match('/symantec/',$settings['refresh_patterns'])){
		$conf.=<<<EOC

# Symantec refresh_pattern
range_offset_limit -1
refresh_pattern liveupdate.symantecliveupdate.com/.*\.(cab|exe|dll|msi) 10080 100% 43200 reload-into-ims
refresh_pattern symantecliveupdate.com/.*\.(cab|exe|dll|msi) 10080 100% 43200 reload-into-ims

EOC;
		}
if(preg_match('/avast/',$settings['refresh_patterns'])){
		$conf.=<<<EOC

# Avast refresh_pattern
range_offset_limit -1
refresh_pattern avast.com/.*\.(vpu|cab|stamp|exe) 10080 100% 43200 reload-into-ims

EOC;
		}
if(preg_match('/avira/',$settings['refresh_patterns'])){
		$conf.=<<<EOC

# Avira refresh_pattern
range_offset_limit -1
refresh_pattern personal.avira-update.com/.*\.(cab|exe|dll|msi|gz) 10080 100% 43200 reload-into-ims

EOC;
	}
	$refresh_conf=<<<EOC
	
# Add any of your own refresh_pattern entries above these.
refresh_pattern ^ftp:    1440  20%  10080
refresh_pattern ^gopher:  1440  0%  1440
refresh_pattern -i (/cgi-bin/|\?) 0  0%  0
refresh_pattern .    0  20%  4320
EOC;
  
}
	
	$conf .= <<<EOD
cache_mem $memory_cache_size MB
maximum_object_size_in_memory {$max_objsize_in_mem} KB
memory_replacement_policy {$memory_policy}
cache_replacement_policy {$cache_policy}
minimum_object_size {$min_objsize} KB
maximum_object_size {$max_objsize}
$disk_cache_opts
offline_mode {$offline_mode}
EOD;

	if (!empty($settings['cache_swap_low'])) $conf .= "cache_swap_low {$settings['cache_swap_low']}\n";
	if (!empty($settings['cache_swap_high'])) $conf .= "cache_swap_high {$settings['cache_swap_high']}\n";

	$donotcache = sq_text_area_decode($settings['donotcache']);
	if (!empty($donotcache)) {
		file_put_contents(SQUID_ACLDIR . '/donotcache.acl', $donotcache);
		$conf .= 'acl donotcache dstdomain "' . SQUID_ACLDIR . "/donotcache.acl\"\n";
		$conf .= 'cache deny donotcache';
	}
	elseif (file_exists(SQUID_ACLDIR . '/donotcache.acl')) {
     unlink(SQUID_ACLDIR . '/donotcache.acl');
    }
	return $conf.$refresh_conf;
}

function squid_resync_upstream() {
	global $config;
	$conf = "\n#Remote proxies\n";
	if (is_array($config['installedpackages']['squidremote']['config']))
	  foreach ($config['installedpackages']['squidremote']['config'] as $settings){
		if ($settings['enable'] == 'on') {
			$conf .= "cache_peer {$settings['proxyaddr']} {$settings['hierarchy']} {$settings['proxyport']} ";
			if ($settings['icpport'] == '7')
			  	$conf .= "{$settings['icpport']} {$settings['icpoptions']} {$settings['peermethod']} {$settings['allowmiss']} ";
	    	else
	      		$conf .= "{$settings['icpport']} ";
	      	#auth settings
			if (!empty($settings['username']) && !empty($settings['password'])){
				$conf .= " login={$settings['username']}:{$settings['password']}";
				}
			else{
				$conf .= "{$settings['authoption']} ";
			}
			#other options settings
			if (!empty($settings['weight']))
				$conf .= "weight={$settings['weight']} ";
			if (!empty($settings['basetime']))
				$conf .= "basetime={$settings['basetime']} ";
			if (!empty($settings['ttl']))
				$conf .= "ttl={$settings['ttl']} ";
			if (!empty($settings['nodelay']))
				$conf .= "no-delay";
		}
		$conf .= "\n";
	 }
	return $conf;
}

function squid_resync_redirector() {
	global $config;

	$httpav_enabled = ($config['installedpackages']['clamav']['config'][0]['scan_http'] == 'on');
	if ($httpav_enabled) {
		$conf = "url_rewrite_program /usr/local/bin/squirm\n";
	} else {
		$conf = "# No redirector configured\n";
	}
	return $conf;
}

function squid_resync_nac() {
	global $config, $valid_acls;

	$port = ($settings['proxy_port'] ? $settings['proxy_port'] : 3128);
	if (is_array($config['installedpackages']['squidnac']))
		$settings = $config['installedpackages']['squidnac']['config'][0];
	else
		$settings = array();
	$webgui_port = $config['system']['webgui']['port'];
	$addtl_ports = $settings['addtl_ports'];
	$addtl_sslports = $settings['addtl_sslports'];

	$conf = <<<EOD

# Setup some default acls
acl allsrc src all
acl localhost src 127.0.0.1/32
acl safeports port 21 70 80 210 280 443 488 563 591 631 777 901 $webgui_port $port 1025-65535 $addtl_ports
acl sslports port 443 563 $webgui_port $addtl_sslports
acl manager proto cache_object
acl purge method PURGE
acl connect method CONNECT

# Define protocols used for redirects
acl HTTP proto HTTP
acl HTTPS proto HTTPS


EOD;

	$allowed_subnets = preg_replace("/\s+/"," ",sq_text_area_decode($settings['allowed_subnets']));
	#$allowed = "";
	#foreach ($allowed_subnets as $subnet) {
	#	if(!empty($subnet)) {
	#		$subnet = trim($subnet);
	#		$allowed .= "$subnet ";
	#	}
	#}
	if (!empty($allowed_subnets)) {
		$conf .= "acl allowed_subnets src $allowed_subnets\n";
		$valid_acls[] = 'allowed_subnets';
	}

	$options = array(	'unrestricted_hosts' => 'src',
				'banned_hosts' => 'src',
				'whitelist' => 'dstdom_regex -i',
				'blacklist' => 'dstdom_regex -i',
				'block_user_agent' => 'browser -i',
				'block_reply_mime_type' => 'rep_mime_type -i',
	);
	foreach ($options as $option => $directive) {
		$contents = sq_text_area_decode($settings[$option]);
		if (!empty($contents)) {
			file_put_contents(SQUID_ACLDIR . "/$option.acl", $contents);
			$conf .= "acl $option $directive \"" . SQUID_ACLDIR . "/$option.acl\"\n";
			$valid_acls[] = $option;
		}
		elseif (file_exists(SQUID_ACLDIR . "/$option.acl")) {
      unlink(SQUID_ACLDIR . "/$option.acl");
    }
	}

	$conf .= <<<EOD
http_access allow manager localhost

EOD;

	if (is_array($config['installedpackages']['squidcache'])){
		$settings_ch = $config['installedpackages']['squidcache']['config'][0];
		if(!empty($settings_ch['ext_cachemanager'])) {
	  		$extmgr = explode(";", ($settings_ch['ext_cachemanager']));
    		$conf .= "\n# Allow external cache managers\n";
	 		foreach ($extmgr as $mgr) {
    			$conf .= "acl ext_manager src {$mgr}\n";
	 			}
	 		$conf .= "http_access allow manager ext_manager\n";
			}
		}
	
  $conf .= <<<EOD
  
http_access deny manager
http_access allow purge localhost
http_access deny purge
http_access deny !safeports
http_access deny CONNECT !sslports

# Always allow localhost connections
http_access allow localhost

EOD;

	return $conf;
}

function squid_resync_traffic() {
	global $config, $valid_acls;

	if(!is_array($valid_acls))
		return;
	if (is_array($config['installedpackages']['squidtraffic']))
		$settings = $config['installedpackages']['squidtraffic']['config'][0];
	else
		$settings = array();
		
	$conf = '';
	if (!empty($settings['quick_abort_min']) || ($settings['quick_abort_min']) == "0")
		$conf .= "quick_abort_min {$settings['quick_abort_min']} KB\n";
	if (!empty($settings['quick_abort_max']) || ($settings['quick_abort_max']) == "0")
		$conf .= "quick_abort_max {$settings['quick_abort_max']} KB\n";
	if (!empty($settings['quick_abort_pct']))
		$conf .= "quick_abort_pct {$settings['quick_abort_pct']}\n";	

	$up_limit = ($settings['max_upload_size'] ? $settings['max_upload_size'] : 0);
	$down_limit = ($settings['max_download_size'] ? $settings['max_download_size'] : 0);
	$conf .= "request_body_max_size $up_limit KB\n";
	if ($down_limit != 0)
		$conf .= 'reply_body_max_size ' . $down_limit . " KB allsrc \n";			

	
	// Only apply throttling past 10MB
	// XXX: Should this really be hardcoded?
	$threshold = 10 * 1024 * 1024;
	$overall = $settings['overall_throttling'];
	if (!isset($overall) || ($overall == 0))
		$overall = -1;
	else
		$overall *= 1024;
	$perhost = $settings['perhost_throttling'];
	if (!isset($perhost) || ($perhost == 0))
		$perhost = -1;
	else
		$perhost *= 1024;
	$conf .= <<<EOD
delay_pools 1
delay_class 1 2
delay_parameters 1 $overall/$overall $perhost/$perhost
delay_initial_bucket_level 100

EOD;

	if(! empty($settings['unrestricted_hosts'])) {
		foreach (array('unrestricted_hosts') as $item) {
			if (in_array($item, $valid_acls))
				$conf .= "# Do not throttle unrestricted hosts\n";
				$conf .= "delay_access 1 deny $item\n";
		}
	}

	if ($settings['throttle_specific'] == 'on') {
		$exts = array();
		$binaries = 'bin,cab,sea,ar,arj,tar,tgz,gz,tbz,bz2,zip,7z,exe,com';
		$cdimages = 'iso,bin,mds,nrg,gho,bwt,b5t,pqi';
		$multimedia = 'aiff?,asf,avi,divx,mov,mp3,mp4,wmv,mpe?g,qt,ra?m';
		foreach (array(	'throttle_binaries' => $binaries,
				'throttle_cdimages' => $cdimages,
				'throttle_multimedia' => $multimedia) as $field => $set) {
			if ($settings[$field] == 'on')
				$exts = array_merge($exts, explode(",", $set));
		}

		foreach (explode(",", $settings['throttle_others']) as $ext) {
			if (!empty($ext)) $exts[] = $ext;
		}

		$contents = '';
		foreach ($exts as $ext)
			$contents .= "\.$ext\$\n";
		file_put_contents(SQUID_ACLDIR . '/throttle_exts.acl', $contents);

		$conf .= "# Throttle extensions matched in the url\n";
		$conf .= "acl throttle_exts urlpath_regex -i \"" . SQUID_ACLDIR . "/throttle_exts.acl\"\n";
		$conf .= "delay_access 1 allow throttle_exts\n";
		$conf .= "delay_access 1 deny allsrc\n";
	}
	else
		$conf .= "delay_access 1 allow allsrc\n";

	return $conf;
}

function squid_get_server_certs() {
	global $config;
	$cert_arr = array();
	$cert_arr[] = array('refid' => 'none', 'descr' => 'none');
	foreach ($config['cert'] as $cert) {
		$cert_arr[] = array('refid' => $cert['refid'], 'descr' => $cert['descr']);
	}
	return $cert_arr;
}	

#squid reverse
include('/usr/local/pkg/squid_reverse.inc');

function squid_resync_auth() {
	global $config, $valid_acls;

	if (is_array($config['installedpackages']['squidauth']['config']))
		$settings = $config['installedpackages']['squidauth']['config'][0];
	else
		$settings = array();
		
	if (is_array($config['installedpackages']['squidnac']['config']))
		$settingsnac = $config['installedpackages']['squidnac']['config'][0];
	else
		$settingsnac = array();
		
	if (is_array($config['installedpackages']['squid']['config']))
		$settingsconfig = $config['installedpackages']['squid']['config'][0];
	else
		$settingsconfig = array();
	
	$conf = '';

	// Package integration
	if(!empty($settingsconfig['custom_options']))
		$conf.="# Package Integration\n".preg_replace('/;/',"\n",$settingsconfig['custom_options'])."\n\n";
	
	// Custom User Options
	$conf .= "# Custom options\n".sq_text_area_decode($settingsconfig['custom_options_squid3'])."\n\n";

	// Deny the banned guys before allowing the good guys
	if(! empty($settingsnac['banned_hosts'])) {
		if (squid_is_valid_acl('banned_hosts')) {
			$conf .= "# These hosts are banned\n";
			$conf .= "http_access deny banned_hosts\n";
		}
	}
	if(! empty($settingsnac['banned_macs'])) {
		if (squid_is_valid_acl('banned_macs')) {
			$conf .= "# These macs are banned\n";
			$conf .= "http_access deny banned_macs\n";
		}
	}

	// Unrestricted hosts take precedence over blacklist
	if(! empty($settingsnac['unrestricted_hosts'])) {
		if (squid_is_valid_acl('unrestricted_hosts') && $settings['unrestricted_auth']!= "on") {
			$conf .= "# These hosts do not have any restrictions\n";
			$conf .= "http_access allow unrestricted_hosts\n";
		}
	}
	if(! empty($settingsnac['unrestricted_macs'])) {
		if (squid_is_valid_acl('unrestricted_macs')) {
			$conf .= "# These hosts do not have any restrictions\n";
			$conf .= "http_access allow unrestricted_macs\n";
		}
	}

	// Whitelist and blacklist also take precedence over other allow rules
	if(! empty($settingsnac['whitelist'])) {
		if (squid_is_valid_acl('whitelist')) {
			$conf .= "# Always allow access to whitelist domains\n";
			$conf .= "http_access allow whitelist\n";
		}
	}
	if(! empty($settingsnac['blacklist'])) {
		if (squid_is_valid_acl('blacklist')) {
			$conf .= "# Block access to blacklist domains\n";
			$conf .= "http_access deny blacklist\n";
		}
	}
	if(! empty($settingsnac['block_user_agent'])) {
		if (squid_is_valid_acl('block_user_agent')) {
			$conf .= "# Block access with user agents and browsers\n";
			$conf .= "http_access deny block_user_agent\n";
		}
	}
	if(! empty($settingsnac['block_reply_mime_type'])) {
		if (squid_is_valid_acl('block_reply_mime_type')) {
			$conf .= "# Block access with mime type in the reply\n";
			$conf .= "http_reply_access deny block_reply_mime_type\n";
		}
	}

	$transparent_proxy = ($settingsconfig['transparent_proxy'] == 'on');
	$auth_method = (($settings['auth_method'] && !$transparent_proxy) ? $settings['auth_method'] : 'none');
	// Allow the remaining ACLs if no authentication is set
	if ($auth_method == 'none') {
		$conf .="# Setup allowed acls\n";
		$allowed = array('allowed_subnets');
		if ($settingsconfig['allow_interface'] == 'on') {
			$conf .= "# Allow local network(s) on interface(s)\n";
			$allowed[] = "localnet";
		}
		$allowed = array_filter($allowed, 'squid_is_valid_acl');
		foreach ($allowed as $acl)
			$conf .= "http_access allow $acl\n";
	}
	else {
		$noauth = implode(' ', explode("\n", base64_decode($settings['no_auth_hosts'])));
		if (!empty($noauth)) {
			$conf .= "acl noauth src $noauth\n";
			$valid_acls[] = 'noauth';
		}

		// Set up the external authentication programs
		$auth_ttl = ($settings['auth_ttl'] ? $settings['auth_ttl'] : 60);
		$processes = ($settings['auth_processes'] ? $settings['auth_processes'] : 5);
		$prompt = ($settings['auth_prompt'] ? $settings['auth_prompt'] : 'Please enter your credentials to access the proxy');
		switch ($auth_method) {
			case 'local':
				$conf .= 'auth_param basic program '.SQUID_LOCALBASE.'/libexec/squid/ncsa_auth ' . SQUID_PASSWD . "\n";
				break;
			case 'ldap':
				$port = (isset($settings['auth_server_port']) ? ":{$settings['auth_server_port']}" : '');
				$password = (isset($settings['ldap_pass']) ? "-w {$settings['ldap_pass']}" : '');
				$conf .= "auth_param basic program " . SQUID_LOCALBASE . "/libexec/squid/squid_ldap_auth -v {$settings['ldap_version']} -b {$settings['ldap_basedomain']} -D {$settings['ldap_user']} $password -f \"{$settings['ldap_filter']}\" -u {$settings['ldap_userattribute']} -P {$settings['auth_server']}$port\n";
				break;
			case 'radius':
				$port = (isset($settings['auth_server_port']) ? "-p {$settings['auth_server_port']}" : '');
				$conf .= "auth_param basic program ". SQUID_LOCALBASE . "/libexec/squid/squid_radius_auth -w {$settings['radius_secret']} -h {$settings['auth_server']} $port\n";
				break;
			case 'msnt':
				$conf .= "auth_param basic program ". SQUID_LOCALBASE . "/libexec/squid/msnt_auth\n";
				squid_resync_msnt();
				break;
		}
		$conf .= <<<EOD
auth_param basic children $processes
auth_param basic realm $prompt
auth_param basic credentialsttl $auth_ttl minutes
acl password proxy_auth REQUIRED

EOD;

		// Onto the ACLs
		$password = array('localnet', 'allowed_subnets');
		$passwordless = array('unrestricted_hosts');
		if ($settings['unrestricted_auth'] == 'on') {
			// Even the unrestricted hosts should authenticate
			$password = array_merge($password, $passwordless);
			$passwordless = array();
		}
		$passwordless[] = 'noauth';
		$password = array_filter($password, 'squid_is_valid_acl');
		$passwordless = array_filter($passwordless, 'squid_is_valid_acl');

		// Allow the ACLs that don't need to authenticate
		foreach ($passwordless as $acl)
			$conf .= "http_access allow $acl\n";

		// Allow the other ACLs as long as they authenticate
		foreach ($password as $acl)
			$conf .= "http_access allow password $acl\n";
	}

	$conf .= "# Default block all to be sure\n";
	$conf .= "http_access deny allsrc\n";

	return $conf;
}

function squid_resync_users() {
	global $config;

	$users = $config['installedpackages']['squidusers']['config'];
	$contents = '';
	if (is_array($users)) {
		foreach ($users as $user)
			$contents .= $user['username'] . ':' . crypt($user['password'], base64_encode($user['password'])) . "\n";
	}
	file_put_contents(SQUID_PASSWD, $contents);
	chown(SQUID_PASSWD, 'proxy');
	chmod(SQUID_PASSWD, 0600);
}

function squid_resync_msnt() {
	global $config;

	if (is_array($config['installedpackages']['squidauth']))
		$settings = $config['installedpackages']['squidauth']['config'][0];
	else
		$settings = array();
	$pdcserver = $settings['auth_server'];
	$bdcserver = str_replace(',',' ',$settings['msnt_secondary']);
	$ntdomain = $settings['auth_ntdomain'];

	file_put_contents(SQUID_CONFBASE."/msntauth.conf","server {$pdcserver} {$bdcserver} {$ntdomain}");
	chown(SQUID_CONFBASE."/msntauth.conf", 'proxy');
	chmod(SQUID_CONFBASE."/msntauth.conf", 0600);
}

function squid_resync() {
	global $config;
	
	# detect boot process
	if (is_array($_POST)){
		if (preg_match("/\w+/",$_POST['__csrf_magic']))
			unset($boot_process);
		else
			$boot_process="on";
	}
	
	if (is_process_running('squid') && isset($boot_process))
		return;
	
	conf_mount_rw();
	foreach (array(	SQUID_CONFBASE,
			SQUID_ACLDIR,
			SQUID_BASE,
			SQUID_LIB,
			SQUID_SSL_DB ) as $dir) {
		@mkdir($dir, 0755, true);
		chown($dir, 'proxy');
		chgrp($dir, 'proxy');
		squid_chown_recursive($dir, 'proxy', 'proxy');
	}
	$conf = squid_resync_general() . "\n";
	$conf .= squid_resync_cache() . "\n";
	$conf .= squid_resync_redirector() . "\n";
	$conf .= squid_resync_upstream() . "\n";
	$conf .= squid_resync_nac() . "\n";
	$conf .= squid_resync_traffic() . "\n";
	$conf .= squid_resync_reverse() . "\n";	
	$conf .= squid_resync_auth();
	squid_resync_users();
	squid_write_rcfile();
		
	if(!isset($boot_process))
		squid_sync_on_changes();
	
	#write config file
	file_put_contents(SQUID_CONFBASE . '/squid.conf', $conf);
		
	/* make sure pinger is executable */
	if(file_exists(SQUID_LOCALBASE . "/libexec/squid/pinger"))
		exec("chmod a+x " . SQUID_LOCALBASE . "/libexec/squid/pinger");
	
	$log_dir="";
	#check if squid is enabled
	if (is_array($config['installedpackages']['squid']['config'])){
		if ($config['installedpackages']['squid']['config'][0]['active_interface']!= "")
		$log_dir = $config['installedpackages']['squid']['config'][0]['log_dir'].'/';
		}
	#check if squidreverse is enabled
	else if (is_array($config['installedpackages']['squidreversegeneral']['config'])){
		if ($config['installedpackages']['squidreversegeneral']['config'][0]['reverse_interface'] != "")
			$log_dir="/var/squid/logs/";
		}
	#do not start squid if there is no log dir
	if ($log_dir != ""){
		if(!is_dir($log_dir)) {
			log_error("Creating squid log dir $log_dir");
			@mkdir($log_dir, 0755, true);
			squid_chown_recursive($log_dir, 'proxy', 'proxy');
			}
	
		squid_dash_z();
		
		if (!is_service_running('squid')) {
			log_error("Starting Squid");
			mwexec(SQUID_LOCALBASE . "/sbin/squid -f " . SQUID_CONFFILE);
			} 
		else {
			if (!isset($boot_process)){
				log_error("Reloading Squid for configuration sync");
				mwexec(SQUID_LOCALBASE . "/sbin/squid -k reconfigure -f " . SQUID_CONFFILE);
				}
		}
	
		// Sleep for a couple seconds to give squid a chance to fire up fully.
		for ($i=0; $i < 10; $i++) {
			if (!is_service_running('squid'))
				sleep(1);
		}
		filter_configure();
	}
	conf_mount_ro();
}

function squid_print_javascript_auth() {
	global $config;
	$transparent_proxy = ($config['installedpackages']['squid']['config'][0]['transparent_proxy'] == 'on');

	// No authentication for transparent proxy
	if ($transparent_proxy) {
		$javascript = <<<EOD
<script language="JavaScript">
<!--
function on_auth_method_changed() {
	document.iform.auth_method.disabled = 1;
	document.iform.auth_server.disabled = 1;
	document.iform.auth_ntdomain.disabled = 1;
	document.iform.auth_server_port.disabled = 1;
	document.iform.ldap_user.disabled = 1;
	document.iform.ldap_version.disabled = 1;
	document.iform.ldap_userattribute.disabled = 1;
	document.iform.ldap_filter.disabled = 1;
	document.iform.ldap_pass.disabled = 1;
	document.iform.ldap_basedomain.disabled = 1;
	document.iform.radius_secret.disabled = 1;
	document.iform.msnt_secondary.disabled = 1;
	document.iform.auth_prompt.disabled = 1;
	document.iform.auth_processes.disabled = 1;
	document.iform.auth_ttl.disabled = 1;
	document.iform.unrestricted_auth.disabled = 1;
	document.iform.no_auth_hosts.disabled = 1;
}
-->
</script>

EOD;
	}
	else {
		$javascript = <<<EOD
<script language="JavaScript">
<!--
function on_auth_method_changed() {
	var field = document.iform.auth_method;
	var auth_method = field.options[field.selectedIndex].value;

	if (auth_method == 'none') {
		document.iform.auth_server.disabled = 1;
		document.iform.auth_server_port.disabled = 1;
		document.iform.auth_ntdomain.disabled = 1;
		document.iform.ldap_user.disabled = 1;
		document.iform.ldap_version.disabled = 1;
		document.iform.ldap_userattribute.disabled = 1;
		document.iform.ldap_filter.disabled = 1;
		document.iform.ldap_pass.disabled = 1;
		document.iform.ldap_basedomain.disabled = 1;
		document.iform.radius_secret.disabled = 1;
		document.iform.msnt_secondary.disabled = 1;
		document.iform.auth_prompt.disabled = 1;
		document.iform.auth_processes.disabled = 1;
		document.iform.auth_ttl.disabled = 1;
		document.iform.unrestricted_auth.disabled = 1;
		document.iform.no_auth_hosts.disabled = 1;
	}
	else {
		document.iform.auth_prompt.disabled = 0;
		document.iform.auth_processes.disabled = 0;
		document.iform.auth_ttl.disabled = 0;
		document.iform.unrestricted_auth.disabled = 0;
		document.iform.no_auth_hosts.disabled = 0;
	}

	switch (auth_method) {
		case 'local':
			document.iform.auth_server.disabled = 1;
			document.iform.auth_server_port.disabled = 1;
			document.iform.auth_ntdomain.disabled = 1;
			document.iform.ldap_user.disabled = 1;
			document.iform.ldap_pass.disabled = 1;
			document.iform.ldap_version.disabled = 1;
			document.iform.ldap_userattribute.disabled = 1;
			document.iform.ldap_filter.disabled = 1;
			document.iform.ldap_basedomain.disabled = 1;
			document.iform.radius_secret.disabled = 1;
			document.iform.msnt_secondary.disabled = 1;
			break;
		case 'ldap':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 0;
			document.iform.ldap_user.disabled = 0;
			document.iform.ldap_pass.disabled = 0;
			document.iform.ldap_version.disabled = 0;
			document.iform.ldap_userattribute.disabled = 0;
			document.iform.ldap_filter.disabled = 0;
			document.iform.ldap_basedomain.disabled = 0;
			document.iform.radius_secret.disabled = 1;
			document.iform.msnt_secondary.disabled = 1;
			document.iform.auth_ntdomain.disabled = 1;
			break;
		case 'radius':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 0;
			document.iform.ldap_user.disabled = 1;
			document.iform.ldap_pass.disabled = 1;
			document.iform.ldap_version.disabled = 1;
			document.iform.ldap_userattribute.disabled = 1;
			document.iform.ldap_filter.disabled = 1;
			document.iform.ldap_basedomain.disabled = 1;
			document.iform.radius_secret.disabled = 0;
			document.iform.msnt_secondary.disabled = 1;
			document.iform.auth_ntdomain.disabled = 1;
			break;
		case 'msnt':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 1;
			document.iform.auth_ntdomain.disabled = 0;
			document.iform.ldap_user.disabled = 1;
			document.iform.ldap_pass.disabled = 1;
			document.iform.ldap_version.disabled = 1;
			document.iform.ldap_userattribute.disabled = 1;
			document.iform.ldap_filter.disabled = 1;
			document.iform.ldap_basedomain.disabled = 1;
			document.iform.radius_secret.disabled = 1;
			document.iform.msnt_secondary.disabled = 0;
			break;
	}
}
-->
</script>

EOD;
	}

	print($javascript);
}

function squid_print_javascript_auth2() {
	print("<script language=\"JavaScript\">on_auth_method_changed()</script>\n");
}

function squid_generate_rules($type) {
	global $config;

	$squid_conf = $config['installedpackages']['squid']['config'][0];
	
	//check captive portal option
	$cp_file='/etc/inc/captiveportal.inc';
	$pfsense_version=preg_replace("/\s/","",file_get_contents("/etc/version"));
	$port = ($settings['proxy_port'] ? $settings['proxy_port'] : 3128);
	$cp_inc = file($cp_file);
	$new_cp_inc="";
	$found_rule=0;
	foreach ($cp_inc as $line){
		$new_line=$line;
		//remove applied squid patch
		if (preg_match('/} set 1 skipto 65314/',$line)){
			$found_rule++;
			$new_line ="";
			}
		//add squid patch option based on current config	            
		if (preg_match('/set 1 pass ip from any to/',$line) && $squid_conf['patch_cp']){
			$found_rule++;
			$new_line = "\t".'$cprules .= "add {$rulenum} set 1 skipto 65314 ip from any to {$ips} '.$port.' in\n";'."\n";
			$new_line .= $line;
			}
		if (preg_match('/set 1 pass ip from {/',$line) && $squid_conf['patch_cp']){
			$found_rule++;
			$new_line = "\t".'$cprules .= "add {$rulenum} set 1 skipto 65314 ip from {$ips} '.$port.' to any out\n";'."\n";
			$new_line .= $line;
			}
		$new_cp_inc .= $new_line;
		}
	if (!file_exists('/root/'.$pfsense_version.'.captiveportal.inc.backup')) {
		copy ($cp_file,'/root/'.$pfsense_version.'.captiveportal.inc.backup');
		}
	if($found_rule > 0){
		file_put_contents($cp_file,$new_cp_inc, LOCK_EX);
		}
	
	//normal squid rule check
	if (($squid_conf['transparent_proxy'] != 'on') || ($squid_conf['allow_interface'] != 'on')) {
		return;
	}

	if (!is_service_running('squid')) {
		log_error("SQUID is installed but not started.  Not installing \"{$type}\" rules.");
		return;
	}

	$ifaces = explode(",", $squid_conf['active_interface']);
	$ifaces = array_map('convert_friendly_interface_to_real_interface_name', $ifaces);
	$port = ($squid_conf['proxy_port'] ? $squid_conf['proxy_port'] : 3128);

	$fw_aliases = filter_generate_aliases();
	if(strstr($fw_aliases, "pptp ="))
		$PPTP_ALIAS = "\$pptp";
	else 
		$PPTP_ALIAS = "\$PPTP";
	if(strstr($fw_aliases, "PPPoE ="))
		$PPPOE_ALIAS = "\$PPPoE";
	else 
		$PPPOE_ALIAS = "\$pppoe";
		
	switch($type) {
		case 'nat':
			$rules .= "\n# Setup Squid proxy redirect\n";
			if ($squid_conf['private_subnet_proxy_off'] == 'on') {
				foreach ($ifaces as $iface) {
					$rules .= "no rdr on $iface proto tcp from any to { 192.168.0.0/16, 172.16.0.0/12, 10.0.0.0/8 } port 80\n";
				}
				/* Handle PPPOE case */
				if(($config['pppoe']['mode'] == "server" && $config['pppoe']['localip']) || (function_exists("is_pppoe_server_enabled") && is_pppoe_server_enabled())) {
					$rules .= "no rdr on $PPPOE_ALIAS proto tcp from any to { 192.168.0.0/16, 172.16.0.0/12, 10.0.0.0/8 } port 80\n";
				}
				/* Handle PPTP case */
				if($config['pptpd']['mode'] == "server" && $config['pptpd']['localip']) {
					$rules .= "no rdr on $PPTP_ALIAS proto tcp from any to { 192.168.0.0/16, 172.16.0.0/12, 10.0.0.0/8 } port 80\n";
				}
			}
			if (!empty($squid_conf['defined_ip_proxy_off'])) {
				$defined_ip_proxy_off = explode(";", $squid_conf['defined_ip_proxy_off']);
				$exempt_ip = "";
				foreach ($defined_ip_proxy_off as $ip_proxy_off) {
					if(!empty($ip_proxy_off)) {
						$ip_proxy_off = trim($ip_proxy_off);
						if (is_alias($ip_proxy_off))
							$ip_proxy_off = '$'.$ip_proxy_off;
						$exempt_ip .= ", $ip_proxy_off";
					}
				}
				$exempt_ip = substr($exempt_ip,2);
				foreach ($ifaces as $iface) {
					$rules .= "no rdr on $iface proto tcp from { $exempt_ip } to any port 80\n";         
				}
				/* Handle PPPOE case */
				if(($config['pppoe']['mode'] == "server" && $config['pppoe']['localip']) || (function_exists("is_pppoe_server_enabled") && is_pppoe_server_enabled())) {
					$rules .= "no rdr on $PPPOE_ALIAS proto tcp from { $exempt_ip } to any port 80\n";
				}
				/* Handle PPTP case */
				if($config['pptpd']['mode'] == "server" && $config['pptpd']['localip']) {
					$rules .= "no rdr on $PPTP_ALIAS proto tcp from { $exempt_ip } to any port 80\n";
				}
			}
			if (!empty($squid_conf['defined_ip_proxy_off_dest'])) {
				$defined_ip_proxy_off_dest = explode(";", $squid_conf['defined_ip_proxy_off_dest']);
				$exempt_dest = "";
				foreach ($defined_ip_proxy_off_dest as $ip_proxy_off_dest) {
					if(!empty($ip_proxy_off_dest)) {
						$ip_proxy_off_dest = trim($ip_proxy_off_dest);
						if (is_alias($ip_proxy_off_dest))
							$ip_proxy_off_dest = '$'.$ip_proxy_off_dest;
						$exempt_dest .= ", $ip_proxy_off_dest";
					}
				}
				$exempt_dest = substr($exempt_dest,2);
				foreach ($ifaces as $iface) {
					$rules .= "no rdr on $iface proto tcp from any to { $exempt_dest } port 80\n";
				}
				/* Handle PPPOE case */
				if(($config['pppoe']['mode'] == "server" && $config['pppoe']['localip']) || (function_exists("is_pppoe_server_enabled") && is_pppoe_server_enabled())) {
					$rules .= "no rdr on $PPPOE_ALIAS proto tcp from any to { $exempt_dest } port 80\n";
				}
				/* Handle PPTP case */
				if($config['pptpd']['mode'] == "server" && $config['pptpd']['localip']) {
					$rules .= "no rdr on $PPTP_ALIAS proto tcp from any to { $exempt_dest } port 80\n";
				}
			}
			foreach ($ifaces as $iface) {
				$rules .= "rdr on $iface proto tcp from any to !($iface) port 80 -> 127.0.0.1 port " . $squid_conf['proxy_port'] . "\n";
			}
			/* Handle PPPOE case */
			if(($config['pppoe']['mode'] == "server" && $config['pppoe']['localip']) || (function_exists("is_pppoe_server_enabled") && is_pppoe_server_enabled())) {
				$rules .= "rdr on $PPPOE_ALIAS proto tcp from any to !127.0.0.1 port 80 -> 127.0.0.1 port " . $squid_conf['proxy_port'] . "\n";
			}
			/* Handle PPTP case */
			if($config['pptpd']['mode'] == "server" && $config['pptpd']['localip']) {
				$rules .= "rdr on $PPTP_ALIAS proto tcp from any to !127.0.0.1 port 80 -> 127.0.0.1 port " . $squid_conf['proxy_port'] . "\n";
			}
			$rules .= "\n";
			break;
		case 'filter':
		case 'rule':
			foreach ($ifaces as $iface) {
				$rules .= "# Setup squid pass rules for proxy\n";
				$rules .= "pass in quick on $iface proto tcp from any to !($iface) port 80 flags S/SA keep state\n";
				$rules .= "pass in quick on $iface proto tcp from any to !($iface) port $port flags S/SA keep state\n";
				$rules .= "\n";
			};
			if($config['pppoe']['mode'] == "server" && $config['pppoe']['localip']) {
				$rules .= "pass in quick on $PPPOE_ALIAS proto tcp from any to !127.0.0.1 port $port flags S/SA keep state\n";			
			}			
			if($config['pptpd']['mode'] == "server" && $config['pptpd']['localip']) {
				$rules .= "pass in quick on $PPTP_ALIAS proto tcp from any to !127.0.0.1 port $port flags S/SA keep state\n";
			}
			break;
		default:
			break;
	}

	return $rules;
}

function squid_write_rcfile() {
	/* Declare a variable for the SQUID_CONFFILE constant. */
	/* Then the variable can be referenced easily in the Heredoc text that generates the rc file. */
	$squid_conffile_var = SQUID_CONFFILE;
	$squid_local_base = SQUID_LOCALBASE;
	$rc = array();
	$rc['file'] = 'squid.sh';
	$rc['start'] = <<<EOD
if [ -z "`ps auxw | grep "[s]quid "|awk '{print $2}'`" ];then
	{$squid_local_base}/sbin/squid -f {$squid_conffile_var}
fi

EOD;

	$rc['stop'] = <<<EOD
{$squid_local_base}/sbin/squid -k shutdown -f {$squid_conffile_var}
# Just to be sure...
sleep 5
killall -9 squid 2>/dev/null
killall pinger 2>/dev/null

EOD;
	$rc['restart'] = <<<EOD
if [ -z "`ps auxw | grep "[s]quid "|awk '{print $2}'`" ];then
		{$squid_local_base}/sbin/squid -f {$squid_conffile_var}
	else
		{$squid_local_base}/sbin/squid -k reconfigure -f {$squid_conffile_var}
	fi

EOD;
	conf_mount_rw();
	write_rcfile($rc);
	conf_mount_ro();
}

/* Uses XMLRPC to synchronize the changes to a remote node */
function squid_sync_on_changes() {
	global $config, $g;
	if (is_array($config['installedpackages']['squidsync']['config'])){
		$squid_sync=$config['installedpackages']['squidsync']['config'][0];
		$synconchanges = $squid_sync['synconchanges'];
		$synctimeout = $squid_sync['synctimeout'];
		switch ($synconchanges){
			case "manual":
				if (is_array($squid_sync['row'])){
					$rs=$squid_sync['row'];
					}
				else{
					log_error("[squid] xmlrpc sync is enabled but there is no hosts to push on squid config.");
					return;
					}
				break;
			case "auto":
					if (is_array($config['hasync'])) {
						$system_carp = $config['hasync'];
						$rs[0]['ipaddress']=$system_carp['synchronizetoip'];
						$rs[0]['username']=$system_carp['username'];
						$rs[0]['password']=$system_carp['password'];
					}
					else{
						log_error("[squid] xmlrpc sync is enabled but there is no system backup hosts to push squid config.");
						return;
					}
				break;			
			default:
				return;
			break;
		}
		if (is_array($rs)){
			log_error("[squid] xmlrpc sync is starting.");
			foreach($rs as $sh){
				$sync_to_ip = $sh['ipaddress'];
				$password = $sh['password'];
				if($sh['username'])
					$username = $sh['username'];
				else
					$username = 'admin';
				if($password && $sync_to_ip)
					squid_do_xmlrpc_sync($sync_to_ip, $username, $password,$synctimeout);
				}
			log_error("[squid] xmlrpc sync is ending.");
			}
 		}
}
/* Do the actual XMLRPC sync */
function squid_do_xmlrpc_sync($sync_to_ip, $username, $password, $synctimeout) {
	global $config, $g;

	if(!$username)
		return;
		
	if(!$password)
		return;

	if(!$sync_to_ip)
		return;

	if(!$synctimeout)
		$synctimeout=250;
		
		
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
	$xml['squid'] = $config['installedpackages']['squid'];
	$xml['squidupstream'] = $config['installedpackages']['squidupstream'];
	$xml['squidcache'] = $config['installedpackages']['squidcache'];
	$xml['squidnac'] = $config['installedpackages']['squidnac'];
	$xml['squidtraffic'] = $config['installedpackages']['squidtraffic'];
	$xml['squidreversegeneral'] = $config['installedpackages']['squidreversegeneral'];
	$xml['squidreversepeer'] = $config['installedpackages']['squidreversepeer'];
	$xml['squidreverseuri'] = $config['installedpackages']['squidreverseuri'];
	$xml['squidauth'] = $config['installedpackages']['squidauth'];
	$xml['squidusers'] = $config['installedpackages']['squidusers'];
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Beginning squid XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after defined sync timeout value*/
	$resp = $cli->send($msg, $synctimeout);
	if(!$resp) {
		$error = "A communications error occurred while attempting squid XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "squid Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting squid XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "squid Settings Sync", "");
	} else {
		log_error("squid XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* tell squid to reload our settings on the destination sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/squid.inc');\n";
	$execcmd .= "squid_resync();";
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);
	
	log_error("squid XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, $synctimeout);
	if(!$resp) {
		$error = "A communications error occurred while attempting squid XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "squid Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting squid XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "squid Settings Sync", "");
	} else {
		log_error("squid XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
	
}
?>
