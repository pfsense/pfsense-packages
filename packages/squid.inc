<?php
/* $Id$ */
/*
	squid.inc
	Copyright (C) 2006 Scott Ullrich
	Copyright (C) 2006 Fernando Lemos
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
require_once('filter.inc');
require_once('service-utils.inc');

define('SQUID_CONFBASE', '/usr/local/etc/squid');
define('SQUID_ACLDIR', '/var/squid/acl');
define('SQUID_PASSWD', '/var/etc/squid.passwd');

$valid_acls = array();


function squid_get_real_interface_address($iface) {
	global $config;

	$iface = convert_friendly_interface_to_real_interface_name($iface);
	$line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
	list($dummy, $ip, $dummy2, $netmask) = explode(' ', $line);

	return array($ip, long2ip(hexdec($netmask)));
}

/* setup cache */
function squid_dash_z() {
	mwexec("/usr/local/sbin/squid -z");
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

function squid_is_valid_acl($acl) {
	global $valid_acls;
	if(!is_array($valid_acls))
		return;
	return in_array($acl, $valid_acls);
}

function squid_install_command() {
	/* create cache */
	mwexec("/usr/local/sbin/squid -z");
	/* make sure pinger is executable */
	exec("/bin/chmod a+x /usr/local/libexec/squid/pinger");
	exec("/bin/rm /usr/local/etc/rc.d/squid");
	$rc = array();
	$rc['file'] = 'squid.sh';
	$rc['start'] = '/usr/local/sbin/squid -D';
	$rc['stop'] = <<<EOD
/usr/local/sbin/squid -k shutdown
# Just to be sure...
sleep 5
killall -9 squid 2>/dev/null
killall pinger 2>/dev/null

EOD;
	$rc['restart'] = <<<EOD
if [ -z "`pgrep squid`" ]; then
		/usr/local/sbin/squid -D
	else
		/usr/local/sbin/squid -k reconfigure
	fi

EOD;
	write_rcfile($rc);

	foreach (array(	SQUID_CONFBASE,
			SQUID_ACLDIR,
	) as $dir) {
			make_dirs($dir);
			squid_chown_recursive($dir, 'proxy', 'proxy');
	}

	/* kill any running proxy alarm scripts */
	log_error("Stopping any running proxy monitors");
	mwexec("ps awux | grep \"proxy_monitor\" | grep -v \"grep\" | grep -v \"php\" | awk '{ print $2 }' | xargs kill");
	sleep(1);
	/* restart proxy alarm scripts */
	log_error("Starting a proxy monitor script");
	mwexec_bg("/usr/local/etc/rc.d/proxy_monitor.sh");

	if (!file_exists(SQUID_CONFBASE . '/mime.conf') && file_exists(SQUID_CONFBASE . '/mime.conf.default'))
		copy(SQUID_CONFBASE . '/mime.conf.default', SQUID_CONFBASE . '/mime.conf');

}

function squid_deinstall_command() {
	global $config;
	$settings = $config['installedpackages']['squidcache']['config'][0];
	$cachedir =($settings['harddisk_cache_location'] ? $settings['harddisk_cache_location'] : '/var/squid/cache');

	mwexec('rm -rf $cachedir');
	mwexec('rm -f /usr/local/etc/rc.d/proxy_monitor.sh');
	mwexec("ps awux | grep \"proxy_monitor\" | grep -v \"grep\" | grep -v \"php\" | awk '{ print $2 }' | xargs kill");
	mwexec("ps awux | grep \"squid\" | grep -v \"grep\" | awk '{ print $2 }' | xargs kill");
	mwexec("ps awux | grep \"dnsserver\" | grep -v \"grep\" | awk '{ print $2 }' | xargs kill");
	mwexec("ps awux | grep \"unlinkd\" | grep -v \"grep\" | awk '{ print $2 }' | xargs kill");
	filter_configure();
}

function squid_before_form_general($pkg) {
	$values = get_dir(SQUID_CONFBASE . '/errors/');
	// Get rid of '..' and '.'
	array_shift($values);
	array_shift($values);
	$name = array();
	foreach ($values as $value)
		$names[] = implode(' ', explode('_', $value));

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
	$icp_port = trim($post['icp_port']);
	if (!empty($icp_port) && !is_port($icp_port))
		$input_errors[] = 'You must enter a valid port number in the \'ICP port\' field';

	if (substr($post['log_dir'], -1, 1) == '/')
		$input_errors[] = 'You may not end log location with an / mark';

	if ($post['log_dir']{0} != '/')
		$input_errors[] = 'You must start log location with a / mark';
	if (strlen($post['log_dir']) <= 3)
		$input_errors[] = "That is not a valid log location dir";

	if (($post['transparent_proxy'] == 'on')) {
		$port = 80;
	} else {
		$port = trim($post['proxy_port']);
	}

	$webgui_port = $config['system']['webgui']['port'];
	if($config['system']['webgui']['port'] == "") {
		$webgui_port = 80;
	}

	if ($port == $webgui_port) {
		$input_errors[] = "You can not run squid on the same port as the webgui";
	}
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
		if (!is_numeric($value) || ($value < 1))
			$input_errors[] = "You must enter a valid value for '$field'";
	}

	$value = trim($post['minimum_object_size']);
	if (!is_numeric($value) || ($value < 0))
		$input_errors[] = 'You must enter a valid value for \'Minimum object size\'';

	if ($post['donotcache'] != "") {
		foreach (explode(',', $post['donotcache']) as $host) {
			$host = trim($host);
			if (!is_ipaddr($host) && !is_domain($host))
				$input_errors[] = "The host '$host' is not a valid IP or host name";
		}
	}

}

function squid_validate_nac($post, $input_errors) {
	$allowed_subnets = explode(',', trim($post['allowed_subnets']));
	foreach ($allowed_subnets as $subnet) {
		$subnet = trim($subnet);
		if (!empty($subnet) && !is_subnet($subnet))
			$input_errors[] = "The subnet '$subnet' is not a valid CIDR range";
	}

	foreach (array(	'unrestricted_hosts', 'banned_hosts') as $hosts) {
		foreach (explode(',', $post[$hosts]) as $host) {
			$host = trim($host);
			if (!empty($host) && !is_ipaddr($host))
				$input_errors[] = "The host '$host' is not a valid IP address";
		}
	}

	foreach (array('unrestricted_macs', 'banned_macs') as $macs) {
		foreach (explode(',', $post[$macs]) as $mac) {
			$mac = trim($mac);
			if (!empty($mac) && !is_macaddr($mac))
				$input_errors[] = "The mac '$mac' is not a valid MAC address";
		}
	}

	foreach (explode(',', $post['timelist']) as $time) {
		$time = trim($time);
		if (!empty($time) && !squid_is_timerange($time))
			$input_errors[] = "The time range '$time' is not a valid time range";
	}
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
				foreach (explode(trim($post['msnt_secondary'])) as $server) {
					if (!empty($server) && !is_ipaddr($server) && !is_domain($server))
						$input_errors[] = "The host '$server' is not a valid IP address or domain name";
				}
				break;
		}

		$no_auth = explode(',', trim($post['no_auth_hosts']));
		foreach ($no_auth as $host) {
			$host = trim($host);
			if (!empty($host) && !is_subnet($host))
				$input_errors[] = "The host '$host' is not a valid CIDR range";
		}
	}
}

function squid_resync_general() {
	global $g, $config, $valid_acls;

	$settings = $config['installedpackages']['squid']['config'][0];
	$conf = '';

	$port = ($settings['proxy_port'] ? $settings['proxy_port'] : 3128);
	$ifaces = ($settings['active_interface'] ? $settings['active_interface'] : 'lan');
	$real_ifaces = array();
	foreach (explode(',', $ifaces) as $i => $iface) {
		$real_ifaces[] = squid_get_real_interface_address($iface);
		if($real_ifaces[$i][0])
			$conf .= "http_port {$real_ifaces[$i][0]}";
                        if (($settings['transparent_proxy'] == 'on')) {
                           $conf .= ":80 transparent\n";
                        } else {
			$conf .= ":$port\n";
		}
	}

	$icp_port = ($settings['icp_port'] ? $settings['icp_port'] : 0);

	$pidfile = "{$g['varrun_path']}/squid.pid";
	$language = ($settings['error_language'] ? $settings['error_language'] : 'English');
	$errordir = SQUID_CONFBASE . '/errors/' . $language;
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
visible_hostname $hostname
cache_mgr $email

cache_access_log $logdir_access
cache_log $logdir_cache
cache_store_log none
shutdown_lifetime 3 seconds

EOD;

	if ($settings['allow_interface'] == 'on') {
		$src = '';
		foreach ($real_ifaces as $iface) {
			list($ip, $mask) = $iface;
			$ip = long2ip(ip2long($ip) & ip2long($mask));
			$src .= " $ip/$mask";
		}
		$conf .= "acl localnet src $src\n";
		$valid_acls[] = 'localnet';
		$conf .= <<<EOD
acl get method GET
http_access allow get
acl post method POST
http_access allow post

EOD;

	}

	return $conf;
}

function squid_resync_cache() {
	global $config;

	$settings = $config['installedpackages']['squidcache']['config'][0];

	$cachedir =($settings['harddisk_cache_location'] ? $settings['harddisk_cache_location'] : '/var/squid/cache');
	$disk_cache_size = ($settings['harddisk_cache_size'] ? $settings['harddisk_cache_size'] : 100);
	$level1 = ($settings['level1_subdirs'] ? $settings['level1_subdirs'] : 16);
	$memory_cache_size = ($settings['memory_cache_size'] ? $settings['memory_cache_size'] : 8);
	$max_objsize = ($settings['maximum_object_size'] ? $settings['maximum_object_size'] : 10);
	$min_objsize = ($settings['minimum_object_size'] ? $settings['minimum_object_size'] : 0);
	$cache_policy = ($settings['cache_replacement_policy'] ? $settings['cache_replacement_policy'] : 'heap LFUDA');
	$memory_policy = ($settings['memory_replacement_policy'] ? $settings['memory_replacement_policy'] : 'heap GDSF');
	$offline_mode = ($settings['enable_offline'] == 'on' ? 'on' : 'off');

	$conf = <<<EOD
cache_dir diskd $cachedir $disk_cache_size $level1 256
cache_mem $memory_cache_size MB
maximum_object_size $max_objsize KB
minimum_object_size $min_objsize KB
cache_replacement_policy $cache_policy
memory_replacement_policy $memory_policy
offline_mode $offline_mode

EOD;

	$donotcache = trim(implode("\n", array_map('trim', explode(',', $settings['donotcache']))));
	if (!empty($donotcache)) {
		file_put_contents(SQUID_ACLDIR . '/donotcache.acl', $donotcache);
		$conf .= 'acl donotcache dstdomain "' . SQUID_ACLDIR . "/donotcache.acl\"\n";
		$conf .= 'no_cache deny donotcache';
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
	if ($httpav_enabled)
		return ('redirect_program /usr/local/bin/squirm');
	return '# No redirector configured';
}

function squid_resync_nac() {
	global $config, $valid_acls;

	$settings = $config['installedpackages']['squidnac']['config'][0];
	$webgui_port = $config['system']['webgui']['port'];

	$conf = <<<EOD
acl all src 0.0.0.0/0
acl localhost src 127.0.0.1
acl safeports port 21 70 80 210 280 443 488 563 591 631 777 901 $webgui_port 1025-65535
acl sslports port 443 563 $webgui_port
acl manager proto cache_object
acl purge method PURGE
acl connect method CONNECT
acl dynamic urlpath_regex cgi-bin \?

EOD;

	$allowed = implode(' ', array_map('trim', explode(',', $settings['allowed_subnets'])));
	if (!empty($allowed)) {
		$conf .= "acl allowed_subnets src $allowed\n";
		$valid_acls[] = 'allowed_subnets';
	}

	$options = array(	'unrestricted_hosts' => 'src',
				'unrestricted_macs' => 'arp',
				'banned_hosts' => 'src',
				'banned_macs' => 'arp',
				'whitelist' => 'url_regex -i',
				'blacklist' => 'url_regex -i',
	);
	foreach ($options as $option => $directive) {
		$contents = trim(implode("\n", array_map('trim', explode(',', $settings[$option]))));
		if (!empty($contents)) {
			file_put_contents(SQUID_ACLDIR . "/$option.acl", $contents);
			$conf .= "acl $option $directive \"" . SQUID_ACLDIR . "/$option.acl\"\n";
			$valid_acls[] = $option;
		}
	}

	$conf .= <<<EOD
no_cache deny dynamic
http_access allow manager localhost
http_access deny manager
http_access allow purge localhost
http_access deny purge
http_access deny !safeports
http_access deny CONNECT !sslports

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

	$up_limit = ($settings['max_upload_size'] ? $settings['max_upload_size'] : 0);
	$down_limit = ($settings['max_download_size'] ? $settings['max_download_suze'] : 0);
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

	foreach (array('unrestricted_hosts', 'unrestricted_macs') as $item) {
		if (in_array($item, $valid_acls))
			$conf .= "delay_access 1 deny $item\n";
	}

	if ($settings['throttle_specific'] == 'on') {
		$exts = array();
		$binaries = 'bin,cab,sea,ar,arj,tar,tgz,gz,tbz,bz2,zip,exe,com';
		$cdimages = 'iso,bin,mds,nrg,gho,bwt,b5t,pqi';
		$multimedia = 'aiff?,asf,avi,divx,mov,mp3,mp4,mpe?g,qt,ra?m';
		foreach (array(	'throttle_binaries' => $binaries,
				'throttle_cdimages' => $cdimages,
				'throttle_multimedia' => $multimedia) as $field => $set) {
			if ($settings[$field] == 'on')
				$exts = array_merge($exts, explode(',', $set));
		}

		foreach (explode(',', $settings['throttle_others']) as $ext) {
			if (!empty($ext)) $exts[] = $ext;
		}

		$contents = '';
		foreach ($exts as $ext)
			$contents .= "\.$ext\$\n";
		file_put_contents(SQUID_ACLDIR . '/throttle_exts.acl', $contents);

		$conf .= 'acl throttle_exts url_regex -i "' . SQUID_ACLDIR . '/throttle_exts.acl"';
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
	$conf = '';

	// Deny the banned guys before allowing the good guys
	$banned = array(	'banned_hosts',
				'banned_macs',
	);
	$banned = array_filter($banned, 'squid_is_valid_acl');
	foreach ($banned as $acl)
			$conf .= "http_access deny $acl\n";

	// Unrestricted hosts take precendence over blacklist
	if (squid_is_valid_acl('unrestricted_hosts'))
		$conf .= "http_access allow unrestricted_hosts\n";
	if (squid_is_valid_acl('unrestricted_macs'))
		$conf .= "http_access allow unrestricted_macs\n";
	// Whitelist and blacklist also take precendence
	if (squid_is_valid_acl('whitelist'))
		$conf .= "http_access allow whitelist\n";
	if (squid_is_valid_acl('blacklist'))
		$conf .= "http_access deny blacklist\n";

	$transparent_proxy = ($config['installedpackages']['squid']['config'][0]['transparent_proxy'] == 'on');
	$auth_method = (($settings['auth_method'] && !$transparent_proxy) ? $settings['auth_method'] : 'none');

	// Allow the remaining ACLs if no authentication is set
	if ($auth_method == 'none') {
		if ($settings['allow_interface'] == 'on') {
			$allowed = array('localnet', 'allowed_subnets');
			$allowed = array_filter($allowed, 'squid_is_valid_acl');
			foreach ($allowed as $acl)
				$conf .= "http_access allow $acl\n";
		}
	}
	else {
		$noauth = implode(' ', array_map('trim', explode(',', $settings['no_auth_hosts'])));
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
				$conf .= "auth_param basic program /usr/local/libexec/squid/squid_ldap_auth -b {$settings['ldap_basedomain']} -D {$settings['ldap_user']} $password -f \"(&(objectClass=person)(cn=%s))\" -u cn -P {$settings['auth_server']}$port\n";
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
		$passwordless = array('unrestricted_hosts', 'unrestricted_macs');
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
	$conf = squid_resync_general() . "\n";
	$conf .= squid_resync_cache() . "\n";
	$conf .= squid_resync_redirector() . "\n";
	$conf .= squid_resync_upstream() . "\n";
	$conf .= squid_resync_nac() . "\n";
	$conf .= squid_resync_traffic() . "\n";
	$conf .= squid_resync_auth();
	squid_resync_users();

	/* make sure pinger is executable */
	exec("chmod a+x /usr/local/libexec/squid/pinger");

	file_put_contents(SQUID_CONFBASE . '/squid.conf', $conf);

	$disk_cache_location = $config['installedpackages']['squidcache']['config'][0]['harddisk_cache_location'].'/';
	$log_dir = $config['installedpackages']['squid']['config'][0]['log_dir'].'/';

	if(!is_dir($log_dir)) {
		make_dirs($log_dir);
		squid_chown_recursive($log_dir, 'proxy', 'proxy');
	}

	if(!is_dir($disk_cache_location)) {
		make_dirs($disk_cache_location);
		squid_chown_recursive($disk_cache_location, 'proxy', 'proxy');	
	}
	if(!is_dir($disk_cache_location.'01/')) {
		log_error($disk_cache_location." does not exist.  Creating.");
		mwexec("/usr/local/sbin/squid -z");
	}


	if (!is_service_running('squid')) {
		log_error("Starting Squid");
		mwexec("/usr/local/sbin/squid -z");
		mwexec_bg("/usr/local/sbin/squid -D");
		mwexec_bg("/usr/local/etc/rc.d/proxy_monitor.sh");
	} else {
		log_error("Reloading Squid for configuration sync");
		mwexec("/usr/local/sbin/squid -k reconfigure");
	}

	filter_configure();
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
			document.iform.ldap_basedomain.disabled = 1;
			document.iform.radius_secret.disabled = 1;
			document.iform.msnt_secondary.disabled = 1;
			break;
		case 'ldap':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 0;
			document.iform.ldap_user.disabled = 0;
			document.iform.ldap_password.disabled = 0;
			document.iform.ldap_basedomain.disabled = 0;
			document.iform.radius_secret.disabled = 1;
			document.iform.msnt_secondary.disabled = 1;
			break;
		case 'radius':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 0;
			document.iform.ldap_user.disabled = 1;
			document.iform.ldap_password.disabled = 1;
			document.iform.ldap_basedomain.disabled = 1;
			document.iform.radius_secret.disabled = 0;
			document.iform.msnt_secondary.disabled = 1;
			break;
		case 'msnt':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 1;
			document.iform.ldap_user.disabled = 1;
			document.iform.ldap_password.disabled = 1;
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

	$ifaces = explode(',', $squid_conf['active_interface']);
	$ifaces = array_map('convert_friendly_interface_to_real_interface_name', $ifaces);

	switch($type) {
	case 'nat':
		foreach ($ifaces as $iface)
			$rules .= "# Setup Squid transparent proxy redirect\n";
			$rules .= "rdr on $iface proto tcp from any to !($iface) port 80 -> ($iface) port 80\n";
			$rules .= "\n";
		break;
	case 'filter':
		foreach ($ifaces as $iface)
			$rules .= "# Setup squid pass rules for transparent proxy\n";
			$rules .= "pass in quick on $iface proto tcp from any to !($iface) port 80 flags S/SA keep state\n";
			$rules .= "\n";
		break;
	default:
		break;
	}

	return $rules;
}
?>
