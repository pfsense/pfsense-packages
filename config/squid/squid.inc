<?php
/* $Id$ */
/*
	squid.inc
	Copyright (C) 2006-2009 Scott Ullrich
	Copyright (C) 2006 Fernando Lemos
	Copyright (C) 2008 Martin Fuchs	
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

define('SQUID_CONFBASE', '/usr/local/etc/squid');
define('SQUID_ACLDIR', '/var/squid/acl');
define('SQUID_PASSWD', '/var/etc/squid.passwd');

$valid_acls = array();

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
			if (is_dir($path))
				squid_chown_recursive($path, $user, $group);
			else {
				chown($path, $user);
				chgrp($path, $group);
			}
		}
	}
}

/* setup cache */
function squid_dash_z() {
	global $config;
	$settings = $config['installedpackages']['squidcache']['config'][0];
	$cachedir =($settings['harddisk_cache_location'] ? $settings['harddisk_cache_location'] : '/var/squid/cache');

	if(!is_dir($cachedir.'/')) {
		log_error("Creating Squid cache dir $cachedir");
		make_dirs($cachedir);
		squid_chown_recursive($cachedir, 'proxy', 'proxy');
	}

	if(!is_dir($cachedir.'/00/')) {
		log_error("Creating squid cache subdirs in $cachedir");
		mwexec("/usr/local/sbin/squid -k shutdown");
		sleep(5);
		mwexec("/usr/local/sbin/squid -k kill");
		mwexec("/usr/local/sbin/squid -z");
	}

	if(file_exists("/var/squid/cache/swap.state"))
		exec("chmod a+rw /var/squid/cache/swap.state");

}

function squid_is_valid_acl($acl) {
	global $valid_acls;
	if(!is_array($valid_acls))
		return;
	return in_array($acl, $valid_acls);
}

function squid_install_command() {
	global $config;
	/* migrate existing csv config fields */
	$settingsauth = $config['installedpackages']['squidauth']['config'][0];
	$settingscache = $config['installedpackages']['squidcache']['config'][0];
	$settingsnac = $config['installedpackages']['squidnac']['config'][0];

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
	
	update_status("Writing configuration... One moment please...");
	
	write_config();

	/* create cache */
	update_status("Creating squid cache pools... One moment please...");
	squid_dash_z();
	/* make sure pinger is executable */
	if(file_exists("/usr/local/libexec/squid/pinger"))
		exec("/bin/chmod a+x /usr/local/libexec/squid/pinger");
	if(file_exists("/usr/local/etc/rc.d/squid"))
		exec("/bin/rm /usr/local/etc/rc.d/squid");
	$rc = array();
	$rc['file'] = 'squid.sh';
	$rc['start'] = <<<EOD
if [ -z "`ps auxw | grep "[s]quid -D"|awk '{print $2}'`" ];then
	/usr/local/sbin/squid -D
fi

EOD;
	$rc['stop'] = <<<EOD
/usr/local/sbin/squid -k shutdown
# Just to be sure...
sleep 5
killall -9 squid 2>/dev/null
killall pinger 2>/dev/null

EOD;
	$rc['restart'] = <<<EOD
if [ -z "`ps auxw | grep "[s]quid -D"|awk '{print $2}'`" ];then
		/usr/local/sbin/squid -D
	else
		/usr/local/sbin/squid -k reconfigure
	fi

EOD;
	update_status("Writing rc.d files... One moment please...");
	write_rcfile($rc);

	exec("chmod a+rx /usr/local/libexec/squid/dnsserver");

	foreach (array(	SQUID_CONFBASE,
			SQUID_ACLDIR,
	) as $dir) {
			make_dirs($dir);
			squid_chown_recursive($dir, 'proxy', 'proxy');
	}

	/* kill any running proxy alarm scripts */
	update_status("Checking for running processes... One moment please...");
	log_error("Stopping any running proxy monitors");
	mwexec("ps awux | grep \"proxy_monitor\" | grep -v \"grep\" | grep -v \"php\" | awk '{ print $2 }' | xargs kill");
	sleep(1);

	if (!file_exists(SQUID_CONFBASE . '/mime.conf') && file_exists(SQUID_CONFBASE . '/mime.conf.default'))
		copy(SQUID_CONFBASE . '/mime.conf.default', SQUID_CONFBASE . '/mime.conf');

	update_status("Checking cache... One moment please...");
	squid_dash_z();

	if (!is_service_running('squid')) {
		update_status("Starting... One moment please...");
		log_error("Starting Squid");
		mwexec_bg("/usr/local/sbin/squid -D");
	} else {
		update_status("Reloading Squid for configuration sync... One moment please...");
		log_error("Reloading Squid for configuration sync");
		mwexec("/usr/local/sbin/squid -k reconfigure");
	}

	/* restart proxy alarm scripts */
	log_error("Starting a proxy monitor script");
	mwexec_bg("/usr/local/etc/rc.d/proxy_monitor.sh");

	update_status("Reconfiguring filter... One moment please...");
	filter_configure();
}

function squid_deinstall_command() {
	global $config, $g;
	$plswait_txt = "This operation may take quite some time, please be patient.  Do not press stop or attempt to navigate away from this page during this process.";
	squid_install_cron(false);
	$settings = &$config['installedpackages']['squidcache']['config'][0];
	$cachedir =($settings['harddisk_cache_location'] ? $settings['harddisk_cache_location'] : '/var/squid/cache');
	$logdir = ($settings['log_dir'] ? $settings['log_dir'] : '/var/squid/log');
	update_status("Removing swap.state ... One moment please...");
	update_output_window("$plswait_txt");
	mwexec('rm -rf $cachedir/swap.state');
	mwexec('rm -rf $logdir');
	update_status("Finishing package cleanup.");
	mwexec('rm -f /usr/local/etc/rc.d/proxy_monitor.sh');
	mwexec("ps awux | grep \"proxy_monitor\" | grep -v \"grep\" | grep -v \"php\" | awk '{ print $2 }' | xargs kill");
	mwexec("ps awux | grep \"squid\" | grep -v \"grep\" | awk '{ print $2 }' | xargs kill");
	mwexec("ps awux | grep \"dnsserver\" | grep -v \"grep\" | awk '{ print $2 }' | xargs kill");
	mwexec("ps awux | grep \"unlinkd\" | grep -v \"grep\" | awk '{ print $2 }' | xargs kill");
	update_status("Reloading filter...");
	filter_configure_sync();
}

function squid_before_form_general($pkg) {
	$values = get_dir(SQUID_CONFBASE . '/errors/');
	// Get rid of '..' and '.'
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

function squid_validate_general($post, $input_errors) {
	global $config;
	$settings = $config['installedpackages']['squid']['config'][0];
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
		$input_errors[] = 'You must enter a valid number of days \'Log rotate\' field';

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

	if (($post['transparent_proxy'] != 'on') && ($post['private_subnet_proxy_off'] == 'on')) {
		$input_errors[] = "You can not bypass traffic to private subnets  without using the transparent proxy.";
	}
	
	if (($post['transparent_proxy'] != 'on') && !empty($post['defined_ip_proxy_off'])) {
		$input_errors[] = "You can not bypass traffic from specific IPs without using the transparent proxy.";
	}	

	foreach (array('defined_ip_proxy_off') as $hosts) {
		foreach (explode(";", $post[$hosts]) as $host) {
			$host = trim($host);
			if (!empty($host) && !is_ipaddr($host))
				$input_errors[] = "The entry '$host' is not a valid IP address";
		}
	}

  if(!empty($post['dns_nameservers'])) {
	 $altdns = explode(";", ($post['dns_nameservers']));
	 foreach ($altdns as $dnssrv) {
    if (!is_ipaddr($dnssrv))
      $input_errors[] = 'You must enter a valid IP address in the \'Alternate DNS servers\' field';
	 }}
}

function squid_validate_upstream($post, $input_errors) {
	if ($post['proxy_forwarding'] == 'on') {
		$addr = trim($post['proxy_addr']);
		if (empty($addr))
			$input_errors[] = 'The field \'Hostname\' is required';
		else {
			if (!is_ipaddr($addr) && !is_domain($addr))
				$input_errors[] = 'You must enter a valid IP address or host name in the \'Proxy hostname\' field';
		}

		foreach (array('proxy_port' => 'TCP port', 'icp_port' => 'ICP port') as $field => $name) {
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

function squid_validate_cache($post, $input_errors) {
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

function squid_validate_nac($post, $input_errors) {
	$allowed_subnets = explode("\n", $post['allowed_subnets']);
	foreach ($allowed_subnets as $subnet) {
		$subnet = trim($subnet);
		if (!empty($subnet) && !is_subnet($subnet))
			$input_errors[] = "The subnet '$subnet' is not a valid CIDR range";
	}

	foreach (array(	'unrestricted_hosts', 'banned_hosts') as $hosts) {
		foreach (explode("\n", $post[$hosts]) as $host) {
			$host = trim($host);
			if (!empty($host) && !is_ipaddr($host))
				$input_errors[] = "The host '$host' is not a valid IP address";
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

function squid_validate_traffic($post, $input_errors) {
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
	   $input_errors[] = "The field 'Finish when remaining %' must contain a percentaged value";	
  }	

}

function squid_validate_auth($post, $input_errors) {
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
	$is_installed = false;
	if(!$config['cron']['item'])
		return;
	$x=0;
	foreach($config['cron']['item'] as $item) {
		if(strstr($item['command'], "/usr/local/sbin/squid")) {
			$is_installed = true;
			break;
		}
		$x++;
	}
	switch($should_install) {
		case true:
			if(!$is_installed) {
				$cron_item = array();
				$cron_item['minute'] = "0";
				$cron_item['hour'] = "0";
				$cron_item['mday'] = "*";
				$cron_item['month'] = "*";
				$cron_item['wday'] = "*";
				$cron_item['who'] = "root";
				$cron_item['command'] = "/usr/local/sbin/squid -k rotate";		
				$config['cron']['item'][] = $cron_item;
				parse_config(true);
				write_config("Squid Log Rotation");
				configure_cron();
			}
		break;
		case false:
			if($is_installed == true) {
				if($x > 0) {
					unset($config['cron']['item'][$x]);
					parse_config(true);
					write_config();
				}
				configure_cron();
			}
		break;
	}
}

function squid_resync_general() {
	global $g, $config, $valid_acls;

	$settings = $config['installedpackages']['squid']['config'][0];
	$conf = "# This file is automatically generated by pfSense\n";
	$conf = "# Do not edit manually !\n";

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
		$conf .= "http_port 127.0.0.1:80 transparent\n";
	}
	$icp_port = ($settings['icp_port'] ? $settings['icp_port'] : 0);

	$pidfile = "{$g['varrun_path']}/squid.pid";
	$language = ($settings['error_language'] ? $settings['error_language'] : 'English');
	$errordir = SQUID_CONFBASE . '/errors/' . $language;
	$icondir = SQUID_CONFBASE . '/icons';
	$hostname = ($settings['visible_hostname'] ? $settings['visible_hostname'] : 'localhost');
	$email = ($settings['admin_email'] ? $settings['admin_email'] : 'admin@localhost');

	$logdir = ($settings['log_dir'] ? $settings['log_dir'] : '/var/squid/log');

	$logdir_cache = $logdir . '/cache.log';
	$logdir_access = ($settings['log_enabled'] == 'on' ? $logdir . '/access.log' : '/dev/null');

	$conf .= <<<EOD
icp_port $icp_port

pid_filename $pidfile
cache_effective_user proxy
cache_effective_group proxy
error_directory $errordir
icon_directory $icondir
visible_hostname $hostname
cache_mgr $email
access_log $logdir_access
cache_log $logdir_cache
cache_store_log none

EOD;

  if (!empty($settings['log_rotate'])) {
    $conf .= "logfile_rotate {$settings['log_rotate']}\n";
    squid_install_cron(true);
    }
  else {
    squid_install_cron(false);
    }

	$conf .= <<<EOD
shutdown_lifetime 3 seconds

EOD;

	if ($settings['allow_interface'] == 'on') {
		$src = '';
		foreach ($real_ifaces as $iface) {
			list($ip, $mask) = $iface;
			$ip = long2ip(ip2long($ip) & ip2long($mask));
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
	global $config;

	$settings = $config['installedpackages']['squidcache']['config'][0];

	$cachedir =($settings['harddisk_cache_location'] ? $settings['harddisk_cache_location'] : '/var/squid/cache');
	$disk_cache_size = ($settings['harddisk_cache_size'] ? $settings['harddisk_cache_size'] : 100);
	$disk_cache_system = ($settings['harddisk_cache_system'] ? $settings['harddisk_cache_system'] : 'aufs');
	$level1 = ($settings['level1_subdirs'] ? $settings['level1_subdirs'] : 16);
	$memory_cache_size = ($settings['memory_cache_size'] ? $settings['memory_cache_size'] : 8);
	$max_objsize = ($settings['maximum_object_size'] ? $settings['maximum_object_size'] : 10);
	$min_objsize = ($settings['minimum_object_size'] ? $settings['minimum_object_size'] : 0);
	$cache_policy = ($settings['cache_replacement_policy'] ? $settings['cache_replacement_policy'] : 'heap LFUDA');
	$memory_policy = ($settings['memory_replacement_policy'] ? $settings['memory_replacement_policy'] : 'heap GDSF');
	$offline_mode = ($settings['enable_offline'] == 'on' ? 'on' : 'off');

	$conf = <<<EOD
cache_mem $memory_cache_size MB
maximum_object_size_in_memory 32 KB
memory_replacement_policy $memory_policy
cache_replacement_policy $cache_policy
cache_dir $disk_cache_system $cachedir $disk_cache_size $level1 256
minimum_object_size $min_objsize KB
maximum_object_size $max_objsize KB
offline_mode $offline_mode

EOD;

	if (!empty($settings['cache_swap_low'])) $conf .= "cache_swap_low {$settings['cache_swap_low']}\n";
	if (!empty($settings['cache_swap_high'])) $conf .= "cache_swap_high {$settings['cache_swap_high']}\n";

	$donotcache = base64_decode($settings['donotcache']);
	if (!empty($donotcache)) {
		file_put_contents(SQUID_ACLDIR . '/donotcache.acl', $donotcache);
		$conf .= 'acl donotcache dstdomain "' . SQUID_ACLDIR . "/donotcache.acl\"\n";
		$conf .= 'cache deny donotcache';
	}
	elseif (file_exists(SQUID_ACLDIR . '/donotcache.acl')) {
     unlink(SQUID_ACLDIR . '/donotcache.acl');
    }
  
	return $conf;
}

function squid_resync_upstream() {
	global $config;
	$settings = $config['installedpackages']['squidupstream']['config'][0];

	$conf = '';
	if ($settings['proxy_forwarding'] == 'on') {
		$conf .= "cache_peer {$settings['proxy_addr']} parent {$settings['proxy_port']} {$settings['icp_port']} ";

		if (!empty($settings['username']))
			$conf .= " login={$settings['username']}";
		if (!empty($settings['password']))
			$conf .= ":{$settings['password']}";
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
  $settings = $config['installedpackages']['squidnac']['config'][0];
	$webgui_port = $config['system']['webgui']['port'];

	$conf = <<<EOD

# Setup some default acls
acl all src 0.0.0.0/0.0.0.0
acl localhost src 127.0.0.1/255.255.255.255
acl safeports port 21 70 80 210 280 443 488 563 591 631 777 901 $webgui_port $port 1025-65535
acl sslports port 443 563 $webgui_port
acl manager proto cache_object
acl purge method PURGE
acl connect method CONNECT
acl dynamic urlpath_regex cgi-bin \?

EOD;

	$allowed_subnets = explode("\n", base64_decode($settings['allowed_subnets']));
	$allowed = "";
	foreach ($allowed_subnets as $subnet) {
		if(!empty($subnet)) {
			$subnet = trim($subnet);
			$allowed .= "$subnet ";
		}
	}
	if (!empty($allowed)) {
		$conf .= "acl allowed_subnets src $allowed\n";
		$valid_acls[] = 'allowed_subnets';
	}

	$options = array(	'unrestricted_hosts' => 'src',
				'banned_hosts' => 'src',
				'whitelist' => 'dstdom_regex -i',
				'blacklist' => 'dstdom_regex -i',
	);
	foreach ($options as $option => $directive) {
		$contents = base64_decode($settings[$option]);
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
cache deny dynamic
http_access allow manager localhost

EOD;

	if(!empty($settings['ext_cachemanager'])) {
	  $extmgr = explode(";", ($settings['ext_cachemanager']));
    $count = 1;
    $conf .= "\n# Allow external cache managers\n";
//    $conf .= "acl ext_manager src ".$settings['ext_cachemanager']."\n";
	 foreach ($extmgr as $mgr) {
    $conf .= "acl ext_manager_".$count." src ";
    $conf .= $mgr." ";
    $conf .= "\n";
    $conf .= "http_access allow manager ext_manager_".$count."\n";
    $count += 1;
	 }}
	
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
	$settings = $config['installedpackages']['squidtraffic']['config'][0];
	$conf = '';

  if (!empty($settings['quick_abort_min']) || ($settings['quick_abort_min']) == "0") $conf .= "quick_abort_min {$settings['quick_abort_min']} KB\n";
  if (!empty($settings['quick_abort_max']) || ($settings['quick_abort_max']) == "0") $conf .= "quick_abort_max {$settings['quick_abort_max']} KB\n";
  if (!empty($settings['quick_abort_pct'])) $conf .= "quick_abort_pct {$settings['quick_abort_pct']}\n";	

	$up_limit = ($settings['max_upload_size'] ? $settings['max_upload_size'] : 0);
	$down_limit = ($settings['max_download_size'] ? $settings['max_download_size'] : 0);
	$conf .= "request_body_max_size $up_limit KB\n";
	$conf .= 'reply_body_max_size ' . ($down_limit * 1024) . " allow all\n";

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
		$conf .= "delay_access 1 deny all\n";
	}
	else
		$conf .= "delay_access 1 allow all\n";

	return $conf;
}

function squid_resync_auth() {
	global $config, $valid_acls;

	$settings = $config['installedpackages']['squidauth']['config'][0];
	$settingsnac = $config['installedpackages']['squidnac']['config'][0];
	$settingsconfig = $config['installedpackages']['squid']['config'][0];
	$conf = '';

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

	// Unrestricted hosts take precendence over blacklist
	if(! empty($settingsnac['unrestricted_hosts'])) {
		if (squid_is_valid_acl('unrestricted_hosts')) {
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

	// Whitelist and blacklist also take precendence over other allow rules
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

	$transparent_proxy = ($settingsconfig['transparent_proxy'] == 'on');
	$auth_method = (($settings['auth_method'] && !$transparent_proxy) ? $settings['auth_method'] : 'none');
	// Allow the remaining ACLs if no authentication is set
	if ($auth_method == 'none') {
		if ($settingsconfig['allow_interface'] == 'on') {
			$conf .= "# Allow local network(s) on interface(s)\n";
			$allowed = array('localnet', 'allowed_subnets');
			$allowed = array_filter($allowed, 'squid_is_valid_acl');
			foreach ($allowed as $acl)
				$conf .= "http_access allow $acl\n";
		}
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
				$conf .= 'auth_param basic program /usr/local/libexec/squid/ncsa_auth ' . SQUID_PASSWD . "\n";
				break;
			case 'ldap':
				$port = (isset($settings['auth_port']) ? ":{$settings['auth_port']}" : '');
				$password = (isset($settings['ldap_pass']) ? "-w {$settings['ldap_pass']}" : '');
				$conf .= "auth_param basic program /usr/local/libexec/squid/squid_ldap_auth -v {$settings['ldap_version']} -b {$settings['ldap_basedomain']} -D {$settings['ldap_user']} $password -f \"{$settings['ldap_filter']}\" -u uid -P {$settings['auth_server']}$port\n";
				break;
			case 'radius':
				$port = (isset($settings['auth_port']) ? "-p {$settings['auth_server_port']}" : '');
				$conf .= "auth_param basic program /usr/local/libexec/squid/squid_radius_auth -w {$settings['radius_secret']} -h {$settings['auth_server']} $port\n";
				break;
			case 'msnt':
				$conf .= "auth_param basic program /usr/local/libexec/squid/msnt_auth\n";
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

  if(!empty($config['installedpackages']['squid']['config'][0]['custom_options'])) {
	 $custopts = explode(";", ($config['installedpackages']['squid']['config'][0]['custom_options']));
	 $conf .= "# Custom options\n";
	 foreach ($custopts as $custopt) {
			$conf .= $custopt."\n";
	 }
	}

	$conf .= "# Default block all to be sure\n";
	$conf .= "http_access deny all\n";

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

function squid_resync() {
	global $config;
	conf_mount_rw();
	$conf = squid_resync_general() . "\n";
	$conf .= squid_resync_cache() . "\n";
	$conf .= squid_resync_redirector() . "\n";
	$conf .= squid_resync_upstream() . "\n";
	$conf .= squid_resync_nac() . "\n";
	$conf .= squid_resync_traffic() . "\n";
	$conf .= squid_resync_auth();
	squid_resync_users();

	/* make sure pinger is executable */
	if(file_exists("/usr/local/libexec/squid/pinger"))
		exec("chmod a+x /usr/local/libexec/squid/pinger");

	foreach (array(	SQUID_CONFBASE,
			SQUID_ACLDIR,
	) as $dir) {
			make_dirs($dir);
			squid_chown_recursive($dir, 'proxy', 'proxy');
	}

	file_put_contents(SQUID_CONFBASE . '/squid.conf', $conf);

	$log_dir = $config['installedpackages']['squid']['config'][0]['log_dir'].'/';

	if(!is_dir($log_dir)) {
		log_error("Creating squid log dir $log_dir");
		make_dirs($log_dir);
		squid_chown_recursive($log_dir, 'proxy', 'proxy');
	}

	squid_dash_z();

	if (!is_service_running('squid')) {
		log_error("Starting Squid");
		mwexec_bg("/usr/local/sbin/squid -D");
	} else {
		log_error("Reloading Squid for configuration sync");
		mwexec("/usr/local/sbin/squid -k reconfigure");
	}

	filter_configure();
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
	document.iform.auth_server_port.disabled = 1;
	document.iform.ldap_user.disabled = 1;
	document.iform.ldap_version.disabled = 1;
	document.iform.ldap_filter.disabled = 1;
	document.iform.ldap_password.disabled = 1;
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
		document.iform.ldap_user.disabled = 1;
		document.iform.ldap_version.disabled = 1;
		document.iform.ldap_filter.disabled = 1;
		document.iform.ldap_password.disabled = 1;
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
			document.iform.ldap_user.disabled = 1;
			document.iform.ldap_password.disabled = 1;
			document.iform.ldap_version.disabled = 1;
			document.iform.ldap_filter.disabled = 1;
			document.iform.ldap_basedomain.disabled = 1;
			document.iform.radius_secret.disabled = 1;
			document.iform.msnt_secondary.disabled = 1;
			break;
		case 'ldap':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 0;
			document.iform.ldap_user.disabled = 0;
			document.iform.ldap_password.disabled = 0;
			document.iform.ldap_version.disabled = 0;
			document.iform.ldap_filter.disabled = 0;
			document.iform.ldap_basedomain.disabled = 0;
			document.iform.radius_secret.disabled = 1;
			document.iform.msnt_secondary.disabled = 1;
			break;
		case 'radius':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 0;
			document.iform.ldap_user.disabled = 1;
			document.iform.ldap_password.disabled = 1;
			document.iform.ldap_version.disabled = 1;
			document.iform.ldap_filter.disabled = 1;
			document.iform.ldap_basedomain.disabled = 1;
			document.iform.radius_secret.disabled = 0;
			document.iform.msnt_secondary.disabled = 1;
			break;
		case 'msnt':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 1;
			document.iform.ldap_user.disabled = 1;
			document.iform.ldap_password.disabled = 1;
			document.iform.ldap_version.disabled = 1;
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
	if (!is_service_running('squid')) {
		log_error("SQUID is installed but not started.  Not installing redirect rules.");
		return;
	}

	if (($squid_conf['transparent_proxy'] != 'on') || ($squid_conf['allow_interface'] != 'on')) {
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
			}
			if (!empty($squid_conf['defined_ip_proxy_off'])) {
				$defined_ip_proxy_off = explode(";", $squid_conf['defined_ip_proxy_off']);
				$exempt_ip = "";
				foreach ($defined_ip_proxy_off as $ip_proxy_off) {
					if(!empty($ip_proxy_off)) {
						$ip_proxy_off = trim($ip_proxy_off);
						$exempt_ip .= ", $ip_proxy_off";
					}
				}
				$exempt_ip = substr($exempt_ip,2);
				foreach ($ifaces as $iface) {
					$rules .= "no rdr on $iface proto tcp from { $exempt_ip } to any port 80\n";         
				}          
			}     
			foreach ($ifaces as $iface) {
				$rules .= "rdr on $iface proto tcp from any to !($iface) port 80 -> 127.0.0.1 port 80\n";
			}
			/* Handle PPPOE case */
			if($config['pppoe']['mode'] == "server" && $config['pppoe']['localip']) {
				$rules .= "rdr on $PPPOE_ALIAS proto tcp from any to !127.0.0.1 port 80 -> 127.0.0.1 port 80\n";
			}
			/* Handle PPTP case */
			if($config['pptpd']['mode'] == "server" && $config['pptpd']['localip']) {
				$rules .= "rdr on $PPTP_ALIAS proto tcp from any to !127.0.0.1 port 80 -> 127.0.0.1 port 80\n";
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

?>
