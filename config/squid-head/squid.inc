<?php
require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('service-utils.inc');

if(!function_exists("filter_configure")) 
	require_once("filter.inc");

define('RC_BASE', '/usr/local/etc/rc.d');
define('SQUID_CONFBASE', '/usr/local/etc/squid');
define('SQUID_LOGDIR', '/var/squid/log');
define('SQUID_CACHEDIR', '/var/squid/cache');
define('SQUID_ACLDIR', '/var/squid/acl');
define('SQUID_PASSWD', '/var/etc/squid.passwd');
define('MSNTAUTH_CONF', '/usr/local/etc/squid/msntauth.conf');

$valid_acls = array();


function squid_get_real_interface_address($iface) {
	global $config;

	$iface = convert_friendly_interface_to_real_interface_name($iface);
	$line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
	list($dummy, $ip, $dummy2, $netmask) = explode(' ', $line);

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

function squid_is_valid_acl($acl) {
	global $valid_acls;

	return in_array($acl, $valid_acls);
}

function squid_install_command() {
	/* Create cache */
	update_output_window('Initializing cache... This may take a moment...');
	mwexec('/usr/local/sbin/squid -z');

	$rc = array();
	$rc['file'] = 'squid.sh';
	$rc['start'] = '/usr/local/sbin/squid -D';
	$rc['stop'] = RC_BASE . "/proxy_monitor.sh stop\n";
	$rc['stop'] .= <<<EOD
/usr/local/sbin/squid -k shutdown
# Just to be sure...
killall squid 2>/dev/null
sleep 1
killall squid 2>/dev/null

EOD;
	$rc['restart'] = <<<EOD
if [ -z "`pgrep squid`" ]; then
		/usr/local/sbin/squid -D
	else
		/usr/local/sbin/squid -k reconfigure
	fi

EOD;
	write_rcfile($rc);

	$rc = array();
	$rc['file'] = 'proxy_monitor.sh';
	$rc['start'] = '/usr/local/bin/squid_monitor.sh';
	$rc['stop'] = 'killall squid_monitor.sh';
	write_rcfile($rc);

	foreach (array(	SQUID_CONFBASE,
			SQUID_LOGDIR,
			SQUID_ACLDIR,
			SQUID_CACHEDIR,
	) as $dir) {
			make_dirs($dir);
			squid_chown_recursive($dir, 'proxy', 'proxy');
	}

	restart_service('squid');
	sleep(1);
	restart_service('proxy_monitor');

	if (!file_exists(SQUID_CONFBASE . '/mime.conf')	&& file_exists(SQUID_CONFBASE . '/mime.conf.default'))
		copy(SQUID_CONFBASE . '/mime.conf.default', SQUID_CONFBASE . '/mime.conf');
}

function squid_deinstall_command() {
	stop_service('proxy_monitor');
	stop_service('squid');

	filter_configure();

	mwexec('rm -rf ' . SQUID_CACHEDIR);
}

function squid_before_form_general(&$pkg) {
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

function squid_validate_general($post, &$input_errors) {
	$icp_port = trim($post['icp_port']);
	if (!empty($icp_port) && !is_port($icp_port))
		$input_errors[] = 'You must enter a valid port number in the \'ICP port\' field';

	$children = $post['redirect_children'];
	if (!empty($children) && !is_numeric($children))
		$input_errors[] = 'You must enter a valid number for the \'Redirect children\' field';
}

function squid_validate_upstream($post, &$input_errors) {
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

function squid_validate_cache($post, &$input_errors) {
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

	foreach (explode(',', $post['donotcache']) as $host) {
		$host = trim($host);
		if (!is_ipaddr($host) && !is_domain($host))
			$input_errors[] = "$host is not a valid IP or host name";
	}
}

function squid_validate_nac($post, &$input_errors) {
	$allowed_subnets = explode(',', trim($post['allowed_subnets']));
	foreach ($allowed_subnets as $subnet) {
		$subnet = trim($subnet);
		if (!empty($subnet) && !is_subnet($subnet))
			$input_errors[] = "'$subnet' is not a valid CIDR range";
	}

	foreach (array(	'unrestricted_hosts',
			'banned_hosts',
			'whitelist',
			'blacklist',
	) as $hosts) {
		foreach (explode(',', $post[$hosts]) as $host) {
			$host = trim($host);
			if (!empty($host) && !is_ipaddr($host))
				$input_errors[] = "'$host' is not a valid IP address";
		}
	}

	foreach (array('unrestricted_macs', 'banned_macs') as $macs) {
		foreach (explode(',', $post[$macs]) as $mac) {
			$mac = trim($mac);
			if (!empty($mac) && !is_macaddr($mac))
				$input_errors[] = "'$mac' is not a valid MAC address";
		}
	}

	foreach (explode(',', $post['timelist']) as $time) {
		$time = trim($time);
		if (!empty($time) && !squid_is_timerange($time))
			$input_errors[] = "'$time' is not a valid time range";
	}
}

function squid_validate_traffic($post, &$input_errors) {
	$num_fields = array(
		'max_download_size' => 'Maximum download size',
		'max_upload_size' => 'Maximum upload size',
		'perhost_capping' => 'Per-host bandwidth capping',
		'overall_capping' => 'Overall bandwidth capping',
		'perhost_throttling' => 'Per-host bandwidth throttling',
		'overall_throttling' => 'Overall bandwidth throttling',
		'initial_bucket_level' => 'Initial bucket level',
	);
	foreach ($num_fields as $field => $name) {
		$value = trim($post[$field]);
		if (!is_numeric($value) || ($value < 0))
			$input_errors[] = "The field '$name' must contain a positive number";
	}
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
				$required = array(
					'ldap_basedn' => 'LDAP base DN',
					'ldap_filter' => 'LDAP filter',
				);
				foreach ($required as $field => $descr) {
					$value = trim($post[$field]);
					if (empty($value))
						$input_errors[] = "The field '$descr' is required";
				}

				$user = trim($post['ldap_user']);
				$password = trim($post['ldap_password']);
				if (!empty($password) && empty($user))
					$input_errors[] = 'You must specify an username if you specify a password';

				break;
			case 'radius':
				$secret = trim($post['radius_secret']);
				if (empty($secret))
					$input_errors[] = 'The field \'RADIUS secret\' is required';
				break;
			case 'msnt':
				$bdc = $post['msnt_bdc'];
				if (!empty($bdc) && !is_ipaddr($bdc) && !is_domain($bdc))
					$input_errors[] = "'$bdc' isn't a valid IP address or domain name";
				$domain = $post['msnt_domain'];
				if (empty($domain) || !is_domain($domain))
					$input_errors[] = 'You must enter a valid domain name in the \'NT domain\' field';
				break;
		}

		$no_auth = explode(',', trim($post['no_auth_hosts']));
		foreach ($no_auth as $host) {
			$host = trim($host);
			if (!empty($host) && !is_subnet($host))
				$input_errors[] = "'$host' isn't a valid CIDR range";
		}
	}
}

function squid_resync_general() {
	global $g, $config, $valid_acls;

	$settings = $config['installedpackages']['squid']['config'][0];
	$conf = '';

	if ($settings['transparent_proxy'] == 'on') {
		$conf .= <<<EOD
httpd_accel_host virtual
httpd_accel_port 80
httpd_accel_with_proxy on
httpd_accel_uses_host_header on

EOD;
	}

	$port = $settings['proxy_port'] ? $settings['proxy_port'] : 3128;
	$ifaces = $settings['active_interface'] ? $settings['active_interface'] : 'lan';
	$real_ifaces = array();
	foreach (explode(',', $ifaces) as $i => $iface) {
		$real_ifaces[] = squid_get_real_interface_address($iface);
		if (!empty($real_ifaces[$i][0]))
			$conf .= "http_port {$real_ifaces[$i][0]}:$port\n";
	}

	$icp_port = $settings['icp_port'] ? $settings['icp_port'] : 0;

	$pidfile = "{$g['varrun_path']}/squid.pid";
	$language = $settings['error_language'] ? $settings['error_language'] : 'English';
	$errordir = SQUID_CONFBASE . '/errors/' . $language;
	$hostname = $settings['visible_hostname'] ? $settings['visible_hostname'] : 'localhost';
	$email = $settings['admin_email'] ? $settings['admin_email'] : 'admin@localhost';

	$logdir_cache = SQUID_LOGDIR . '/cache.log';
	$logdir_access = $settings['log_enabled'] == 'on' ? SQUID_LOGDIR . '/access.log' : '/dev/null';

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
	}

	restart_service('proxy_monitor');

	return $conf;
}

function squid_resync_cache() {
	global $config;

	$settings = $config['installedpackages']['squidcache']['config'][0];

	$cachedir = SQUID_CACHEDIR;
	$disk_cache_size = $settings['harddisk_cache_size'] ? $settings['harddisk_cache_size'] : 100;
	$level1 = $settings['level1_subdirs'] ? $settings['level1_subdirs'] : 16;
	$memory_cache_size = $settings['memory_cache_size'] ? $settings['memory_cache_size'] : 8;
	$max_objsize = $settings['maximum_object_size'] ? $settings['maximum_object_size'] : 4;
	$min_objsize = $settings['minimum_object_size'] ? $settings['minimum_object_size'] : 0;
	$cache_policy = $settings['cache_replacement_policy'] ? $settings['cache_replacement_policy'] : 'heap LFUDA';
	$memory_policy = $settings['memory_replacement_policy'] ? $settings['memory_replacement_policy'] : 'heap GDSF';
	$offline_mode = $settings['enable_offline'] == 'on' ? 'on' : 'off';

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

	$httpav_enabled = $config['installedpackages']['clamav']['config'][0]['scan_http'] == 'on';
	if ($httpav_enabled)
		return ('redirect_program /usr/local/bin/squirm');
	return '# No redirector configured';
}

function squid_resync_nac() {
	global $config, $valid_acls;

	$settings = $config['installedpackages']['squidnac']['config'][0];

	$conf = <<<EOD
acl all src 0.0.0.0/0
acl localhost src 127.0.0.1
acl safeports port 21 70 80 210 280 443 488 563 591 631 777 901 1025-65535
acl sslports port 443 563
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

	$settings = $config['installedpackages']['squidtraffic']['config'][0];
	$conf = '';

	$up_limit = $settings['max_upload_size'] ? $settings['max_upload_size'] : 0;
	$down_limit = $settings['max_download_size'] ? $settings['max_download_size'] : 0;
	$conf .= "request_body_max_size $up_limit KB\n";
	$conf .= 'reply_body_max_size ' . ($down_limit * 1024) . " allow all\n";

	foreach (array('perhost', 'overall') as $field) {
		$capping = $settings["{$field}_capping"];
		$throttling = $settings["{$field}_throttling"];

		if (!isset($capping) || $capping == 0)
			$capping = '-1';
		else
			$capping *= 1024; // Kbytes

		if (!isset($throttling) || $throttling == 0) {
			if ($capping == '-1')
				$throttling = '-1';
			else
				$throttling = $capping;
		} else {
			$throttling *= 1024; // Kbytes
		}

		$$field = "$throttling/$capping";
	}

	$initial_bucket_level = $settings['initial_bucket_level'];
	if (!isset($initial_bucket_level) || $initial_bucket_level == 0)
		$initial_bucket_level = '100%';
	else
		$initial_bucket_level *= 1024; // Kbytes

	$conf .= <<<EOD
delay_initial_bucket_level $initial_bucket_level
delay_pools 1
delay_class 1 2
delay_parameters 1 $overall $perhost

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

		if (!empty($contents)) { // avoid crashing Squid
			$conf .= 'acl throttle_exts url_regex -i "' . SQUID_ACLDIR . "/throttle_exts.acl\"\n";
			$conf .= "delay_access 1 allow throttle_exts\n";
			$conf .= "delay_access 1 deny all\n";
		}
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

	// Whitelist and blacklist also take precendence
	if (squid_is_valid_acl('whitelist'))
		$conf .= "http_access allow whitelist\n";
	if (squid_is_valid_acl('blacklist'))
		$conf .= "http_access deny blacklist\n";

	$transparent_proxy = $config['installedpackages']['squid']['config'][0]['transparent_proxy'] == 'on';
	$auth_method = ($settings['auth_method'] && !$transparent_proxy) ? $settings['auth_method'] : 'none';

	// Allow the remaining ACLs if no authentication is set
	if ($auth_method == 'none') {
		$allowed = array('localnet', 'allowed_subnets', 'unrestricted_hosts', 'unrestricted_macs');
		$allowed = array_filter($allowed, 'squid_is_valid_acl');
		foreach ($allowed as $acl)
			$conf .= "http_access allow $acl\n";
	}

	else {
		$noauth = implode(' ', array_map('trim', explode(',', $settings['no_auth_hosts'])));
		if (!empty($noauth)) {
			$conf .= "acl noauth src $noauth\n";
			$valid_acls[] = 'noauth';
		}

		// Set up the external authentication programs
		$auth_ttl = $settings['auth_ttl'] ? $settings['auth_ttl'] : 60;
		$processes = $settings['auth_processes'] ? $settings['auth_processes'] : 5;
		$prompt = $settings['auth_prompt'] ? $settings['auth_prompt'] : 'Please enter your credentials to access the proxy';
		switch ($auth_method) {
			case 'local':
				$conf .= 'auth_param basic program /usr/local/libexec/squid/ncsa_auth ' . SQUID_PASSWD . "\n";
				break;
			case 'ldap':
				$port = isset($settings['auth_port']) ? ":{$settings['auth_port']}" : '';
				$user = isset($settings['ldap_user']) ? "-D {$settings['ldap_user']}" : '';
				$password = isset($settings['ldap_password']) ? "-w '{$settings['ldap_password']}'" : '';
				$filter = isset($settings['ldap_filter']) ? "-f '{$settings['ldap_filter']}'" : '';
				$conf .= "auth_param basic program /usr/local/libexec/squid/squid_ldap_auth -b {$settings['ldap_basedn']} $user $password $filter {$settings['auth_server']}$port\n";
				break;
			case 'radius':
				$port = isset($settings['auth_port']) ? "-p {$settings['auth_server_port']}" : '';
				$conf .= "auth_param basic program /usr/local/libexec/squid/squid_radius_auth -w {$settings['radius_secret']} -h {$settings['auth_server']} $port\n";
				break;
			case 'msnt':
				$conf .= "auth_param basic program /usr/local/libexec/squid/msnt_auth\n";

				$bdc = trim($settings['msnt_bdc']);
				if (empty($bdc)) $bdc = $settings['auth_server'];
				$msntauth_conf = "server {$settings['auth_server']} $bdc {$settings['msnt_domain']}\n";
				file_put_contents(MSNTAUTH_CONF, $msntauth_conf);

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
	$conf = squid_resync_general() . "\n";
	$conf .= squid_resync_cache() . "\n";
	$conf .= squid_resync_redirector() . "\n";
	$conf .= squid_resync_upstream() . "\n";
	$conf .= squid_resync_nac() . "\n";
	$conf .= squid_resync_traffic() . "\n";
	$conf .= squid_resync_auth();
	squid_resync_users();

	file_put_contents(SQUID_CONFBASE . '/squid.conf', $conf);

	if (!is_dir(SQUID_CACHEDIR . '/0/0')) {
		log_error(SQUID_CACHEDIR  . ' does not exist. Creating.');
		mwexec('/usr/local/sbin/squid -z');
	}

	restart_service('squid');
	sleep(1);
	restart_service('proxy_monitor');

	filter_configure();
}

function squid_print_javascript_auth() {
	global $config;
	$transparent_proxy = $config['installedpackages']['squid']['config'][0]['transparent_proxy'] == 'on';

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
	document.iform.ldap_basedn.disabled = 1;
	document.iform.ldap_filter.disabled = 1;
	document.iform.radius_secret.disabled = 1;
	document.iform.msnt_bdc.disabled = 1;
	document.iform.msnt_domain.disabled = 1;
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
		document.iform.ldap_basedn.disabled = 1;
		document.iform.ldap_filter.disabled = 1;
		document.iform.radius_secret.disabled = 1;
		document.iform.msnt_bdc.disabled = 1;
		document.iform.msnt_domain.disabled = 1;
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
			document.iform.ldap_basedn.disabled = 1;
			document.iform.ldap_filter.disabled = 1;
			document.iform.radius_secret.disabled = 1;
			document.iform.msnt_bdc.disabled = 1;
			document.iform.msnt_domain.disabled = 1;
			break;
		case 'ldap':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 0;
			document.iform.ldap_user.disabled = 0;
			document.iform.ldap_password.disabled = 0;
			document.iform.ldap_basedn.disabled = 0;
			document.iform.ldap_filter.disabled = 0;
			document.iform.radius_secret.disabled = 1;
			document.iform.msnt_bdc.disabled = 1;
			document.iform.msnt_domain.disabled = 1;
			break;
		case 'radius':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 0;
			document.iform.ldap_user.disabled = 1;
			document.iform.ldap_password.disabled = 1;
			document.iform.ldap_basedn.disabled = 1;
			document.iform.ldap_filter.disabled = 1;
			document.iform.radius_secret.disabled = 0;
			document.iform.msnt_bdc.disabled = 1;
			document.iform.msnt_domain.disabled = 1;
			break;
		case 'msnt':
			document.iform.auth_server.disabled = 0;
			document.iform.auth_server_port.disabled = 1;
			document.iform.ldap_user.disabled = 1;
			document.iform.ldap_password.disabled = 1;
			document.iform.ldap_basedn.disabled = 1;
			document.iform.ldap_filter.disabled = 1;
			document.iform.radius_secret.disabled = 1;
			document.iform.msnt_bdc.disabled = 0;
			document.iform.msnt_domain.disabled = 0;
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
	if (!is_service_running('squid') || ($squid_conf['transparent_proxy'] != 'on')) {
		log_error('Squid is installed but not started. Not installing redirect rules.');
		return;
	}

	$port = isset($squid_conf['proxy_port']) ? $squid_conf['proxy_port'] : 3128;
	$ifaces = explode(',', $squid_conf['active_interface']);
	$ifaces = array_map('convert_friendly_interface_to_real_interface_name', $ifaces);

	switch($type) {
		case 'nat':
			foreach ($ifaces as $iface)
				$rules .= "rdr on $iface inet proto tcp from any to !($iface) port 80 -> ($iface) port $port\n";
			break;
		case 'filter':
			foreach ($ifaces as $iface)
				$rules .= "pass quick on $iface inet proto tcp from any to !($iface) port 80 flags S/SA keep state\n";
			break;
	}

	return $rules;
}
?>
