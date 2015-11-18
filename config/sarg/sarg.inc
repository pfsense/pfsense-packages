<?php
/*
	sarg.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2007 Joao Henrique F. Freitas
	Copyright (C) 2012-2013 Marcello Coutinho
	Copyright (C) 2015 ESF, LLC
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
$pf_version = substr(trim(file_get_contents("/etc/version")), 0, 3);
if ($pf_version == "2.1" || $pf_version == "2.2") {
	// Function to get squidGuard directory
	// each squidGuard version has a different directory
	function getsqGuardDir() {
		foreach (glob("/usr/pbi/*", GLOB_ONLYDIR) as $dirname) {
			if (preg_match("/squidguard-/i", $dirname)) {
				return trim($dirname);
				break;
			}
		}
	}
	define('SARG_DIR', '/usr/pbi/sarg-' . php_uname("m"));
	define('SQGARD_DIR', getsqGuardDir());
	define('SQUID_DIR', '/usr/pbi/squid-' . php_uname("m"));
	define('DANSG_DIR', '/usr/pbi/dansguardian-' . php_uname("m"));
} else {
	define('SARG_DIR', '/usr/local');
	define('SQGARD_DIR', '/usr/local');
	define('SQUID_DIR', '/usr/local');
	define('DANSG_DIR', '/usr/local');
}

$uname = posix_uname();
if ($uname['machine']=='amd64') {
	ini_set('memory_limit', '250M');
}

// STATIC VARS
$sarg_proxy['squid_rc'] = SQUID_DIR . '/etc/rc.d/squid.sh';
$sarg_proxy['squid_config'] = '/var/squid/logs/access.log';
$sarg_proxy['squidguard_config'] = SQGARD_DIR . '/etc/squidGuard/squidGuard.conf';
$sarg_proxy['squidguard_block_log'] = '/var/squidGuard/log/block.log';
$sarg_proxy['dansguardian_config'] = DANSG_DIR . '/etc/dansguardian/dansguardian.conf';
$sarg_proxy['dansguardian_log'] = '/var/log/dansguardian/access.log';
// END STATIC VARS

function sarg_text_area_decode($text) {
	return preg_replace('/\r\n/', "\n", base64_decode($text));
}

function sarg_resync() {
	global $config;
	if (($_POST['Submit'] == 'Save') || !isset($_POST['Submit'])) {
		sync_package_sarg();
	}
	if ($_POST['Submit'] == 'Force update now') {
		run_sarg();
	}
}

function log_rotate($log_file) {
	global $config, $g;

	// remove .10 rotate log file
	unlink_if_exists("{$log_file}.10");
	// rotate logs from 9 to 0
	$i = 9;
	while ($i >= 0) {
		if (file_exists("{$log_file}.{$i}")) {
			rename("{$log_file}.{$i}", "{$log_file}" . ($i + 1));
		}
		$i = $i - 1;
	}
	// rotate current log
	if (file_exists("$log_file")) {
		rename("{$log_file}", "{$log_file}.0");
	}
}

function run_sarg($id = -1) {
	global $config, $g, $sarg_proxy;
	// mount filesystem writeable
	conf_mount_rw();
	$cmd = SARG_DIR . "/bin/sarg";
	if ($id >= 0 && is_array($config['installedpackages']['sargschedule']['config'])) {
		$args = $config['installedpackages']['sargschedule']['config'][$id]['args'];
		$action = $config['installedpackages']['sargschedule']['config'][$id]['action'];
		$gzip = $config['installedpackages']['sargschedule']['config'][$id]['gzip'];
		$find = $config['installedpackages']['sargschedule']['config'][$id]['find'];
		$gziplevel = $config['installedpackages']['sargschedule']['config'][$id]['gziplevel'];
		$daylimit = $config['installedpackages']['sargschedule']['config'][$id]['daylimit'];
	} else {
		$args = $_POST['args'];
		$action = $_POST['action'];
		$gzip = $_POST['gzip'];
		$find = $_POST['find'];
		$gziplevel = $_POST['gziplevel'];
		$daylimit = "";
	}
	$find = (preg_match("/(\d+)/", $find, $find_matches) ? $find_matches[1] : "60");
	log_error("Sarg: force refresh now with {$args} args, compress({$gzip}) and {$action} action after sarg finish.");
	$gzip_script = "#!/bin/sh\n";
	if ($gzip == "on") {
		// remove old file if exists
		unlink_if_exists("/root/sarg_run_{$id}.sh");
		$gzip_script .= <<<EOF
for a in `/usr/bin/find /usr/local/sarg-reports -cmin -{$find} -type d -mindepth 1 -maxdepth 1`
do
echo \$a
/usr/bin/find \$a -name "*html" | /usr/bin/xargs gzip {$gziplevel}
done

EOF;

	}
	if (preg_match("/(\d+)/", $daylimit, $day_matches)) {
		$gzip_script .= <<<EOF
for a in `/usr/bin/find /usr/local/sarg-reports -ctime +{$find} -type d -mindepth 1 -maxdepth 1`
do
echo \$a
/bin/rm -rf \$a
done

EOF;
	}
	// create a new file to speedup find search
	file_put_contents("/root/sarg_run_{$id}.sh", $gzip_script, LOCK_EX);
	mwexec("export LC_ALL=C && " . $cmd . " " . $args);
	// check if there is a script to run after file save
	if (is_array($config['installedpackages']['sarg'])) {
		switch ($config['installedpackages']['sarg']['config'][0]['proxy_server']) {
			case "squidguard":
				if ($action == "both" || $action == "rotate") {
					log_error('Executing squidguard log rotate after sarg.');
					log_rotate($sarg_proxy['squidguard_block_log']);
					file_put_contents($sarg_proxy['squidguard_block_log'], "", LOCK_EX);
					chown($sarg_proxy['squidguard_block_log'], 'proxy');
					chgrp($sarg_proxy['squidguard_block_log'], 'proxy');
					mwexec(SQUID_DIR . '/sbin/squid -k reconfigure');
				}
			// leave this case without break to run squid rotate too.
			case "squid":
				if ($action == "both" || $action == "rotate") {
					log_error('Executing squid log rotate after sarg.');
					mwexec(SQUID_DIR . '/sbin/squid -k rotate');
				}
				if ($action == "both" || $action=="restart") {
					if (file_exists($sarg_proxy['squid_rc'])) {
						mwexec_bg($sarg_proxy['squid_rc'] . ' restart');
					}
				}
			break;
			case "dansguardian":
				if (preg_match('/\w+/', $action) && $action != "none") {
					log_rotate($sarg_proxy['dansguardian_log']);
					log_error('Restarting dansguardian after sarg and log rotate.');
					mwexec('/usr/bin/killall -HUP dansguardian');
				}
			break;
		}
	}
	// check compress option
	if ($gzip == "on") {
		mwexec_bg("/bin/sh /root/sarg_run_{$id}.sh");
	}
	// mount filesystem readonly
	conf_mount_ro();
}

function sync_package_sarg() {
	global $config, $g, $sarg_proxy;

	// detect boot process
	if (function_exists("platform_booting")) {
		if (platform_booting()) {
			return;
		}
	} elseif ($g['booting']) {
		return;
	}

	// check pkg.php sent a sync request
	$update_conf = 0;
	// mount filesystem writeable
	conf_mount_rw();
	if (!is_array($config['installedpackages']['sarg']['config'])) {
		$config['installedpackages']['sarg']['config'][0]['report_options'] = 'use_graphs,remove_temp_files,main_index,use_comma,date_time_by_bytes';
		$config['installedpackages']['sarg']['config'][0]['report_type'] = 'topusers,topsites,sites_users,users_sites,date_time,denied,auth_failures,site_user_time_date,downloads';
		$config['installedpackages']['sarg']['config'][0]['report_date_format'] = 'u';
		$config['installedpackages']['sarg']['config'][0]['report_charset'] = 'UTF-8';
		$config['installedpackages']['sarg']['config'][0]['topuser_num'] = '0';
		$config['installedpackages']['sarg']['config'][0]['authfail_report_limit'] = '0';
		$config['installedpackages']['sarg']['config'][0]['denied_report_limit'] = '0';
		$config['installedpackages']['sarg']['config'][0]['user_report_limit'] = '0';
		$config['installedpackages']['sarg']['config'][0]['lastlog'] = '0';
		$config['installedpackages']['sarg']['config'][0]['max_elapsed'] = '0';
	}
	$sarg = $config['installedpackages']['sarg']['config'][0];
	if (!is_array($config['installedpackages']['sarguser']['config'])) {
		$config['installedpackages']['sarguser']['config'][0]['user_sort_field'] = 'BYTES';
		$config['installedpackages']['sarguser']['config'][0]['exclude_userlist'] = $sarg['exclude_userlist'];
		$config['installedpackages']['sarguser']['config'][0]['include_userlist'] = $sarg['include_userlist'];
		$config['installedpackages']['sarguser']['config'][0]['usertab'] = $sarg['usertab'];
		$config['installedpackages']['sarguser']['config'][0]['ldap_filter_search'] = '(uid=%s)';
		$config['installedpackages']['sarguser']['config'][0]['ldap_target_attr'] = 'cn';
		$config['installedpackages']['sarguser']['config'][0]['ldap_port'] = '389';
		$config['installedpackages']['sarguser']['config'][0]['ntlm_user_format'] = 'domainname+username';
	}
	$sarguser = $config['installedpackages']['sarguser']['config'][0];
	$access_log = $sarg['proxy_server'];
	switch ($sarg['proxy_server']) {
		case 'dansguardian':
			$access_log = $sarg_proxy['dansguardian_log'];
			$dansguardian_conf = $sarg_proxy['dansguardian_config'];
			$dansguardian_filter_out_date = "dansguardian_filter_out_date on";
			$squidguard_conf = 'squidguard_conf none';
			break;
		case 'squidguard':
			$squidguard_conf = 'squidguard_conf ' . $sarg_proxy['squidguard_config'];
			$redirector_log_format = 'redirector_log_format #year#-#mon#-#day# #hour# #tmp#/#list#/#tmp#/#tmp#/#url#/#tmp# #ip#/#tmp# #user# #end#';
			// leave this case without break to include squid log file on squidguard option
		case 'squid':
			$access_log = $sarg_proxy['squid_config'];
			if (is_array($config['installedpackages']['squid']['config'])) {
				if (file_exists($config['installedpackages']['squid']['config'][0]['log_dir'] . '/access.log')) {
					$access_log = $config['installedpackages']['squid']['config'][0]['log_dir'] . '/access.log';
				}
			}
			break;
	}
	if (!file_exists($access_log) && $access_log !="") {
		$error = "Sarg config error: " . $sarg['proxy_server'] . " log file ($access_log) does not exists";
		log_error($error);
		file_notice("Sarg", $error, "Sarg Settings", "");
	}

	// general tab
	$graphs = (preg_match('/use_graphs/', $sarg['report_options']) ? "yes" : "no");
	$anonymous_output_files = (preg_match('/anonymous_output_files/', $sarg['report_options']) ? "yes" : "no");
	$resolve_ip = (preg_match('/resolve_ip/', $sarg['report_options']) ? "yes" : "no");
	$user_ip = (preg_match('/user_ip/', $sarg['report_options']) ? "yes" : "no");
	$sort_order = (preg_match('/user_sort_field_order/', $sarg['report_options']) ? "reverse" : "normal");
	$remove_temp_files = (preg_match('/remove_temp_files/', $sarg['report_options']) ? "yes" : "no");
	$main_index = (preg_match('/main_index/', $sarg['report_options']) ? "yes" : "no");
	$index_tree = (preg_match('/index_tree/', $sarg['report_options']) ? "file" : "date");
	$overwrite_report = (preg_match('/overwrite_report/', $sarg['report_options']) ? "yes" : "no");
	$use_comma = (preg_match('/use_comma/', $sarg['report_options']) ? "yes" : "no");
	$long_url = (preg_match('/long_url/', $sarg['report_options']) ? "yes" : "no");
	$privacy = (preg_match('/privacy/', $sarg['report_options']) ? "yes" : "no");
	$displayed_values = (preg_match('/displayed_values/', $sarg['report_options']) ? "abbreviation" : "bytes");
	$bytes_in_sites_users_report = (preg_match('/bytes_in_sites_users_report/', $sarg['report_options']) ? "yes" : "no");
	$date_time_by = (preg_match('/date_time_by_bytes/', $sarg['report_options']) ? "bytes" : "");
	$date_time_by .= (preg_match('/date_time_by_elap/', $sarg['report_options']) ? " elap" : "");
	if (empty($date_time_by)) {
		$date_time_by = "bytes";
	}
	$date_format = (preg_match("/\w/", $sarg['report_date_format']) ? $sarg['report_date_format'] : "u");
	$report_type = preg_replace('/,/', ' ', $sarg['report_type']);
	$report_charset = $sarg['report_charset'] ?: "UTF-8";
	$exclude_string = (empty($sarg['exclude_string']) ? "" : 'exclude_string "' . $sarg['exclude_string'] . '"');

	// limits
	$max_elapsed = $sarg['max_elapsed'] ?: "0";
	$lastlog = $sarg['lastlog'] ?: "0";
	$topuser_num = $sarg['topuser_num'] ?: "0";
	$authfail_report_limit = $sarg['authfail_report_limit'] ?: "0";
	$denied_report_limit = $sarg['denied_report_limit'] ?: "0";
	$report_limit = $sarg['user_report_limit'] ?: "0";
	$user_report_limit = "siteusers_report_limit " . $report_limit . "\n";
	$user_report_limit .= "user_report_limit " . $report_limit . "\n";
	if (preg_match("/(squidguard|dansguardian)/", $sarg['proxy_server'])) {
		$user_report_limit .= $sarg['proxy_server'] . "_report_limit " . $report_limit . "\n";
	}

	// user tab
	$ntlm_user_format = $sarguser['ntlm_user_format'] ?: 'domainname+username';
	if (!empty($sarguser['include_userlist'])) {
		$include_users = "$include_users " . $sarguser['include_userlist'];
	}
	if (empty($sarguser['usertab'])) {
		$usertab="none";
	} else {
		$usertab = SARG_DIR . "/etc/sarg/usertab.conf";
		file_put_contents(SARG_DIR . '/etc/sarg/usertab.conf', sarg_text_area_decode($sarguser['usertab']), LOCK_EX);
	}
	if ($sarguser['ldap_enable']) {
		$usertab = "ldap";
		$LDAPHost = (empty($sarguser['ldap_host']) ? "" : "LDAPHost " . $sarguser['ldap_host']);
		$LDAPort = (empty($sarguser['ldap_port']) ? "" : "LDAPPort " . $sarguser['ldap_port']);
		$LDAPBindDN = (empty($sarguser['ldap_bind_dn']) ? "" : "LDAPBindDN " . $sarguser['ldap_bind_dn']);
		$LDAPBindPW = (empty($sarguser['ldap_bind_pw']) ? "" : "LDAPBindPW " . $sarguser['ldap_bind_pw']);
		$LDAPBaseSearch = (empty($sarguser['ldap_base_search']) ? "" : "LDAPBaseSearch " . $sarguser['ldap_base_search']);
		$LDAPTargetAttr = (empty($sarguser['ldap_target_Attr']) ? "" : "LDAPTargetAttr " . $sarguser['ldap_target_Attr']);
		$LDAPFilterSearch = (empty($sarguser['ldap_filter_search']) ? "" : "LDAPFilterSearch " . $sarguser['ldap_filter_search']);
	}

	// move old reports
	if (is_dir("/usr/local/www/sarg-reports") && !is_dir("/usr/local/sarg-reports")) {
		rename("/usr/local/www/sarg-reports","/usr/local/sarg-reports");
	}

	// check dirs
	$dirs = array("/usr/local/sarg-reports", "/usr/local/www/sarg-images", "/usr/local/www/sarg-images/temp");
	foreach ($dirs as $dir) {
		if (!is_dir($dir)) {
			mkdir($dir, 0755, true);
		}
	}

	// images
	$simages = array("datetime.png", "graph.png", "sarg-squidguard-block.png", "sarg.png");
	$simgdir1 = "/usr/local/www/sarg-images";
	$simgdir2 = SARG_DIR . "/etc/sarg/images";
	foreach ($simages as $simage) {
		if (!file_exists("{$simgdir1}/{$simage}")) {
			copy("{$simgdir2}/{$simage}","{$simgdir1}/{$simage}");
		}
	}

	// create sarg config files
	$sarg_dir = SARG_DIR;
	include("/usr/local/pkg/sarg.template");
	file_put_contents(SARG_DIR . "/etc/sarg/sarg.conf", $sg, LOCK_EX);
	file_put_contents(SARG_DIR . '/etc/sarg/exclude_hosts.conf', sarg_text_area_decode($sarg['exclude_hostlist']), LOCK_EX);
	file_put_contents(SARG_DIR . '/etc/sarg/exclude_codes', sarg_text_area_decode($sarg['exclude_codelist']), LOCK_EX);
	file_put_contents(SARG_DIR . '/etc/sarg/hostalias',sarg_text_area_decode($sarg['hostalias']), LOCK_EX);
	file_put_contents(SARG_DIR . '/etc/sarg/exclude_users.conf', sarg_text_area_decode($sarguser['exclude_userlist']), LOCK_EX);

	// check cron_tab
	// TODO: Redo this mess to use install_cron_job() instead
	$new_cron = array();
	$cron_found = 0;
	if (is_array($config['cron']['item'])) {
		foreach($config['cron']['item'] as $cron) {
			if (preg_match("/usr.local.www.sarg.php/", $cron["command"])) {
				$cron_found++;
			} else {
				$new_cron['item'][] = $cron;
			}
		}
		$cron_cmd="/usr/local/bin/php --no-header /usr/local/www/sarg.php";
		$sarg_schedule_id = 0;
		if (is_array($config['installedpackages']['sargschedule']['config'])) {
			foreach ($config['installedpackages']['sargschedule']['config'] as $sarg_schedule) {
	 			if (preg_match('/(\d+)m/', $sarg_schedule['frequency'], $matches) && $sarg_schedule['enable']) {
					$new_cron['item'][] = array("minute" => "*/" . $matches[1],
									"hour" => "*",
									"mday" => "*",
									"month" => "*",
									"wday" => "*",
									"who" => "root",
									"command" => $cron_cmd . " " . $sarg_schedule_id);
					$config['cron'] = $new_cron;
					$cron_found++;
				}
				if (preg_match('/(\d+)h/', $sarg_schedule['frequency'], $matches) && $sarg_schedule['enable']) {
					$new_cron['item'][] = array("minute" => "0",
									"hour" => "*/" . $matches[1],
									"mday" => "*",
									"month" => "*",
									"wday" => "*",
									"who" => "root",
									"command" => $cron_cmd . " " . $sarg_schedule_id);
					$config['cron'] = $new_cron;
					$cron_found++;
				}
				if (preg_match('/(\d+)d/', $sarg_schedule['frequency'], $matches) && $sarg_schedule['enable']) {
					$new_cron['item'][] = array("minute" =>	"0",
									"hour" => "0",
									"mday" => "*/" . $matches[1],
									"month" => "*",
									"wday" => "*",
									"who" => "root",
									"command"=>	$cron_cmd . " " . $sarg_schedule_id);
					$config['cron']=$new_cron;
					$cron_found++;
				}
				$sarg_schedule_id++;
			}
		}

		// update cron
		if ($cron_found > 0) {
			$config['cron'] = $new_cron;
			write_config();
			configure_cron();
		}
	}

	// write config if any file from filesystem was loaded
	if ($update_conf > 0) {
		write_config();
	}

	// mount filesystem readonly
	conf_mount_ro();

	sarg_sync_on_changes();
}

function sarg_validate_input($post, &$input_errors) {
	global $config, $g;
	foreach ($post as $key => $value) {
		if (empty($value)) {
			continue;
		}
		// check dansguardian
		if (substr($key, 0, 12) == "proxy_server" && $value == "dansguardian") {
			if (is_array($config['installedpackages']['dansguardianlog'])) {
				if ($config['installedpackages']['dansguardianlog']['config'][0]['logfileformat'] != 3) {
					$input_errors[] = 'Sarg is only compatible with dansguardian squid mode log';
					$input_errors[] = 'Please change it on Services -> Dansguardian -> Report and log -> Log file format';
				}
			} else {
				$input_errors[]='dansguardian package not detected';
			}
		}

		# check squidguard
		if (substr($key, 0, 10) == "proxy_server" && $value == "squidguard") {
			if (!is_array($config['installedpackages']['squidguardgeneral'])) {
				$input_errors[]='squidguard package not detected';
			}
		}

		# check squid
		if (substr($key, 0, 5) == "proxy_server" && $value == "squid") {
			if (is_array($config['installedpackages']['squid'])) {
				if (!$config['installedpackages']['squid']['log_enabled']) {
					$input_errors[] = 'Squid logging not enabled';
				}
			} else {
				$input_errors[] = 'Squid package not installed';
			}
		}

		if (substr($key, 0, 11) == "description" && !preg_match("@^[a-zA-Z0-9 _/.-]+$@", $value)) {
			$input_errors[] = "Do not use special characters in description";
		}
		if (substr($key, 0, 8) == "fullfile" && !preg_match("@^[a-zA-Z0-9_/.-]+$@", $value)) {
			$input_errors[] = "Do not use special characters in filename";
		}
		// check cron option
		if ($key == "frequency" && (!preg_match("/^\d+(h|m|d)$/", $value) || $value == 0)) {
			$input_errors[] = "A valid number with a time reference is required for the field 'Update Frequency'";
		}
	}
}

/* Uses XMLRPC to synchronize the changes to a remote node */
function sarg_sync_on_changes() {
	global $config, $g;

	if (is_array($config['installedpackages']['sargsync']['config'])) {
		$sarg_sync = $config['installedpackages']['sargsync']['config'][0];
		$synconchanges = $sarg_sync['synconchanges'];
		$synctimeout = $sarg_sync['synctimeout'] ?: '250';
		switch ($synconchanges) {
			case "manual":
				if (is_array($sarg_sync['row'])) {
					$rs = $sarg_sync['row'];
				} else {
					log_error("[sarg] XMLRPC sync is enabled but there are no hosts configured as replication targets.");
					return;
				}
				break;
			case "auto":
				if (is_array($config['hasync'])) {
					$system_carp = $config['hasync'];
					$rs[0]['ipaddress'] = $system_carp['synchronizetoip'];
					$rs[0]['username'] = $system_carp['username'];
					$rs[0]['password'] = $system_carp['password'];
					$rs[0]['syncdestinenable'] = FALSE;

					// XMLRPC sync is currently only supported over connections using the same protocol and port as this system
					if ($config['system']['webgui']['protocol'] == "http") {
						$rs[0]['syncprotocol'] = "http";
						$rs[0]['syncport'] = $config['system']['webgui']['port'] ?: '80';
					} else {
						$rs[0]['syncprotocol'] = "https";
						$rs[0]['syncport'] = $config['system']['webgui']['port'] ?: '443';
					}
					if ($system_carp['synchronizetoip'] == "") {
						log_error("[sarg] XMLRPC CARP/HA sync is enabled but there are no system backup hosts configured as replication targets.");
						return;
					} else {
						$rs[0]['syncdestinenable'] = TRUE;
					}
				} else {
					log_error("[sarg] XMLRPC CARP/HA sync is enabled but there are no system backup hosts configured as replication targets.");
					return;
				}
				break;
			default:
				return;
				break;
		}
		if (is_array($rs)) {
			log_error("[sarg] XMLRPC sync is starting.");
			foreach ($rs as $sh) {
				// Only sync enabled replication targets
				if ($sh['syncdestinenable']) {
					$sync_to_ip = $sh['ipaddress'];
					$port = $sh['syncport'];
					$username = $sh['username'] ?: 'admin';
					$password = $sh['password'];
					$protocol = $sh['syncprotocol'];

					$error = '';
					$valid = TRUE;

					if ($password == "") {
						$error = "Password parameter is empty. ";
						$valid = FALSE;
					}
					if (!is_ipaddr($sync_to_ip) && !is_hostname($sync_to_ip) && !is_domain($sync_to_ip)) {
						$error .= "Misconfigured Replication Target IP Address or Hostname. ";
						$valid = FALSE;
					}
					if (!is_port($port)) {
						$error .= "Misconfigured Replication Target Port. ";
						$valid = FALSE;
					}
					if ($valid) {
						sarg_do_xmlrpc_sync($sync_to_ip, $port, $protocol, $username, $password, $synctimeout);
					} else {
						log_error("[sarg] XMLRPC sync with '{$sync_to_ip}' aborted due to the following error(s): {$error}");
					}
				}
			}
			log_error("[sarg] XMLRPC sync completed.");
		}
 	}
}

/* do the actual XMLRPC sync */
function sarg_do_xmlrpc_sync($sync_to_ip, $port, $protocol, $username, $password, $synctimeout) {
	global $config, $g;

	if ($username == "" || $password == "" || $sync_to_ip == "" || $port == "" || $protocol == "") {
		log_error("[sarg] A required XMLRPC sync parameter (username, password, replication target, port or protocol) is empty ... aborting pkg sync");
		return;
	}

	// Take care of IPv6 literal address
	if (is_ipaddrv6($sync_to_ip)) {
		$sync_to_ip = "[{$sync_to_ip}]";
	}

	$url = "{$protocol}://{$sync_to_ip}";

	/* XML will hold the sections to sync. */
	$xml = array();
	$xml['sarg'] = $config['installedpackages']['sarg'];
	$xml['sarguser'] = $config['installedpackages']['sarguser'];
	/* Assemble XMLRPC payload. */
	$params = array(XML_RPC_encode($password), XML_RPC_encode($xml));

	/* Set a few variables needed for sync code */
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if ($g['debug']) {
		$cli->setDebug(1);
	}
	/* Send our XMLRPC message and timeout after defined sync timeout value */
	$resp = $cli->send($msg, $synctimeout);
	if (!$resp) {
		$error = "A communications error occurred while attempting XMLRPC sync with {$url}:{$port}.";
		log_error("[sarg] {$error}");
		file_notice("sync_settings", $error, "sarg Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("[sarg] {$error}");
		file_notice("sync_settings", $error, "sarg Settings Sync", "");
	} else {
		log_error("[sarg] XMLRPC sync successfully completed with {$url}:{$port}.");
	}

	/* Tell sarg to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd = "require_once('/usr/local/pkg/sarg.inc');\n";
	$execcmd .= "sync_package_sarg();";
	/* Assemble XMLRPC payload. */
	$params = array(XML_RPC_encode($password), XML_RPC_encode($execcmd));

	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, $synctimeout);
	if (!$resp) {
		$error = "A communications error occurred while attempting XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error("[sarg] {$error}");
		file_notice("sync_settings", $error, "sarg Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("[sarg] {$error}");
		file_notice("sync_settings", $error, "sarg Settings Sync", "");
	} else {
		log_error("[sarg] XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
}

?>
