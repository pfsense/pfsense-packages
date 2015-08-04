<?php
/*
	e2guardian.inc
	Copyright (C) 2015 Marcello Coutinho
	part of pfSense (http://www.pfSense.org/)
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

require_once("util.inc");
require_once("globals.inc");
global $g;
$shortcut_section = "e2guardian";

if (is_dir('/usr/pbi/e2guardian-' . php_uname("m"))) {
	define('E2GUARDIAN_DIR', '/usr/pbi/e2guardian-' . php_uname("m"));
} else {
	define('E2GUARDIAN_DIR', '/usr/local');
}
define('E2GUARDIAN_DIRTYPATH', "{$g['varrun_path']}/e2guardian.conf.dirty");

$uname = posix_uname();
if ($uname['machine'] == 'amd64') {
	ini_set('memory_limit', '250M');
}

function e2_text_area_decode($text) {
	return preg_replace('/\r\n/', "\n", base64_decode($text));
}

function e2_get_real_interface_address($iface) {
	global $config;
	$iface = convert_friendly_interface_to_real_interface_name($iface);
	$line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
	$postfix_enabled = $config['installedpackages']['postfix']['config'][0]['enable_postfix'];
	list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);
	return array($ip, long2ip(hexdec($netmask)));
}

function check_ca_hashes() {
	global $config, $g;

	// check certificates
	$cert_count = 0;
	if (is_dir('/usr/local/share/certs')) {
		if ($handle = opendir('/usr/local/share/certs')) {
			while (false !== ($file = readdir($handle))) {
				if (preg_match ("/\d+.0/", $file)) {
					$cert_count++;
				}
			}
		}
	}
	closedir($handle);
	// why exactly this value?!
	if ($cert_count < 10) {
		conf_mount_rw();

		// create ca-root hashes from ca-root-nss package
		log_error("Creating root certificate bundle hashes from the Mozilla Project");
		$cas = file('/usr/local/share/certs/ca-root-nss.crt');
		$cert = 0;
		foreach ($cas as $ca) {
			if (preg_match("/--BEGIN CERTIFICATE--/", $ca)) {
				$cert = 1;
			}
			if ($cert == 1) {
				$crt .= $ca;
			}
			if (preg_match("/-END CERTIFICATE-/", $ca)) {
				file_put_contents("/tmp/cert.pem", $crt, LOCK_EX);
				$cert_hash = array();
				exec("/usr/bin/openssl x509 -hash -noout -in /tmp/cert.pem", $cert_hash);
				file_put_contents("/usr/local/share/certs/". $cert_hash[0] . ".0", $crt, LOCK_EX);
				$crt = "";
				$cert = 0;
			}
		}

		conf_mount_ro();
	}
}
function sync_package_e2guardian($via_rpc = "no", $install_process = false) {
	global $config, $g;

	// detect boot process
	if (platform_booting()) {
		unset($boot_process);
	} else {
		$boot_process = "on";
	}

	if (is_process_running('e2guardian') && isset($boot_process) && $via_rpc == "no") {
		log_error("[E2guardian] - Detected boot process pr:" . is_process_running('e2guardian') . " bp:" . isset($boot_process) . " rpc:" . $via_rpc);
		return;
	} else {
		log_error("[E2guardian] - Save settings package call pr:" . is_process_running('e2guardian') . " bp:" . isset($boot_process) . " rpc:" . $via_rpc);
	}

	if (isset($_POST["apply"])) {
		e2guardian_start($via_rpc, $install_process);
		unlink_if_exists(E2GUARDIAN_DIRTYPATH);
		return;
	}

	// assign xml arrays
	if (!is_array($config['installedpackages']['e2guardian'])) {
		$config['installedpackages']['e2guardian']['config'][0] = array(
			'interface' => 'lo0',
			'daemon_options' => 'softrestart');
	}
	$e2guardian = $config['installedpackages']['e2guardian']['config'][0];
	if (!is_array($config['installedpackages']['e2guardianconfig'])) {
		$config['installedpackages']['e2guardianconfig']['config'][0] = array(
			'auth_plugin' => '',
			'scan_options' => 'scancleancache,createlistcachefiles,deletedownloadedtempfiles',
			'weightedphrasemode' => '2',
			'preservecase' => '0',
			'phrasefiltermode' => '2',
			'cron' => 'day');
	}
	$e2guardian_config= $config['installedpackages']['e2guardianconfig']['config'][0];
	if (!is_array($config['installedpackages']['e2guardianlog'])) {
		$config['installedpackages']['e2guardianlog']['config'][0] = array(
			'report_level' => '3',
			'report_language' => 'ukenglish',
			'report_options' => 'showweightedfound,usecustombannedimage,nonstandarddelimiter',
			'logging_options' => 'logconnectionhandlingerrors',
			'loglevel' => '2',
			'logexceptionhits' => '2',
			'logfileformat' => '1');
	}
	$e2guardian_log = $config['installedpackages']['e2guardianlog']['config'][0];
	if (is_array($config['installedpackages']['e2guardianlimits'])) {
		$e2guardian_limits = $config['installedpackages']['e2guardianlimits']['config'][0];
	}
	if (is_array($config['installedpackages']['e2guardianusers'])) {
		$e2guardian_users = $config['installedpackages']['e2guardianusers']['config'][0];
	}
	if (is_array($config['installedpackages']['e2guardianblacklist']['config'])) {
		$e2guardian_blacklist = $config['installedpackages']['e2guardianblacklist']['config'][0];
	}

	// daemon options
	$e2guardian_enabled = $e2guardian['enable'];
	$filterport = ($e2guardian['filterports'] ? $e2guardian['filterports'] : "8080");
	$softrestart = (preg_match('/softrestart/', $e2guardian['daemon_options']) ? "on" : "off");
	$nodaemon = (preg_match('/nodaemon/', $e2guardian['daemon_options']) ? "yes" : "off");
	if (preg_match("/(\d+)\/(\d+)/", $e2guardian['children'], $matches)) {
		$minchildren = $matches[1];
		$maxchildren = $matches[2];
	} else {
		$minchildren = 8;
		$maxchildren = 120;
	}
	if (preg_match("/(\d+)\/(\d+)/", $e2guardian['sparechildren'], $matches)) {
		$minsparechildren = $matches[1];
		$maxsparechildren = $matches[2];
	} else {
		$minsparechildren = 8;
		$maxsparechildren = 64;
	}
	$maxagechildren = ($e2guardian['maxagechildren'] ? $e2guardian['maxagechildren'] : "500");
	$maxips = ($e2guardian['maxips'] ? $e2guardian['maxips'] : "0");
	$preforkchildren = ($e2guardian['preforkchildren']? $e2guardian['preforkchildren'] : "10");
	$proxyip = ($e2guardian['proxyip'] ? $e2guardian['proxyip'] : "127.0.0.1");
	$proxyport = ($e2guardian['proxyport']?$e2guardian['proxyport'] : "3128");
	$proxytimeout = ($e2guardian['proxytimeout'] ? $e2guardian['proxytimeout'] : "30");

	// general options
	$urlcachenumber = ($e2guardian_config['urlcachenumber'] ? $e2guardian_config['urlcachenumber'] : "1000");
	$urlcacheage = ($e2guardian_config['urlcacheage'] ? $e2guardian_config['urlcacheage'] : "900");
	$scancleancache = (preg_match('/scancleancache/', $e2guardian_config['scan_options']) ? "on" : "off");
	$hexdecodecontent = (preg_match('/hexdecodecontent/', $e2guardian_config['scan_options']) ? "on" : "off");
	$forcequicksearch = (preg_match('/forcequicksearch/', $e2guardian_config['scan_options']) ? "on" : "off");
	$reverseaddresslookups = (preg_match('/reverseaddresslookups/', $e2guardian_config['scan_options']) ? "on" : "off");
	$reverseclientiplookups = (preg_match('/reverseclientiplookups/', $e2guardian_config['scan_options']) ? "on" : "off");
	$logclienthostnames = (preg_match('/logclienthostnames/', $e2guardian_config['scan_options']) ? "on" : "off");
	$createlistcachefiles = (preg_match('/createlistcachefiles/', $e2guardian_config['scan_options']) ? "on" : "off");
	$prefercachedlists = (preg_match('/prefercachedlists/', $e2guardian_config['scan_options']) ? "on" : "off");
	$deletedownloadedtempfiles = (preg_match('/deletedownloadedtempfiles/', $e2guardian_config['scan_options']) ? "on" : "off");
	$weightedphrasemode = ($e2guardian_config['weightedphrasemode'] ? $e2guardian_config['weightedphrasemode'] : "2");
	$phrasefiltermode = ($e2guardian_config['phrasefiltermode'] ? $e2guardian_config['phrasefiltermode'] : "2");
	$preservecase = ($e2guardian_config['preservecase'] ? $e2guardian_config['preservecase'] : "0");
	$clamdscan = (preg_match('/clamdscan/', $e2guardian_config['content_scanners']) ? "on" : "off");
	$icapscan = (preg_match('/icapscan/', $e2guardian_config['content_scanners']) ? "on" : "off");
	$contentscannertimeout = ($e2guardian_config['contentscannertimeout'] ? $e2guardian_config['contentscannertimeout'] : "60");
	$contentscanexceptions = ($e2guardian_config['contentscanexceptions'] ? "on" : "off");
	$recheckreplacedurls = (preg_match('/recheckreplacedurls/', $e2guardian_config['misc_options']) ? "on" : "off");
	$forwardedfor = (preg_match('/forwardedfor/', $e2guardian_config['misc_options']) ? "on" : "off");
	$recheckreplacedurls = (preg_match('/icapscan/', $e2guardian_config['misc_options']) ? "on" : "off");
	$usexforwardedfor = (preg_match('/usexforwardedfor/', $e2guardian_config['misc_options']) ? "on" : "off");
	$authplugin = (preg_match('/usr/', $e2guardian_config['auth_plugin']) ? "authplugin = '" . $e2guardian_config['auth_plugin'] . "'" : "");
	if ($e2guardian_config['auth_plugin'] != "") {
		$auth_plugins = explode(",", $e2guardian_config['auth_plugin']);
		$authplugin = "";
		foreach ($auth_plugins as $auth_selected) {
			if ($auth_selected != "none") {
				$authplugin .= "authplugin = '" . preg_replace("@/usr/local@", E2GUARDIAN_DIR, $auth_selected) . "'\n";
			}
		}
	}

	// limits
	$maxuploadsize = ($e2guardian_limits['maxuploadsize'] ? $e2guardian_limits['maxuploadsize'] : "-1");
	$maxcontentfiltersize = ($e2guardian_limits['maxcontentfiltersize'] ? $e2guardian_limits['maxcontentfiltersize'] : "256");
	$maxcontentramcachescansize = ($e2guardian_limits['maxcontentramcachescansize'] ? $e2guardian_limits['maxcontentramcachescansize'] : "1000");
	$maxcontentfilecachescansize = ($e2guardian_limits['maxcontentfilecachescansize'] ? $e2guardian_limits['maxcontentfilecachescansize'] : "2000");
	$initialtrickledelay = ($e2guardian_limits['initialtrickledelay'] ? $e2guardian_limits['initialtrickledelay'] : "20");
	$trickledelay = ($e2guardian_limits['trickledelay'] ? $e2guardian_limits['trickledelay'] : "20");

	// report and log
	$reportlevel = ($e2guardian_log['report_level'] ? $e2guardian_log['report_level'] : "3");
	if ($reportlevel == 1 || $reportlevel == 2) {
		if (preg_match("@(\w+://[a-zA-Z0-9.:/\-]+)@", $e2guardian_log['reportingcgi'], $cgimatches)) {
			$accessdeniedaddress = "accessdeniedaddress = '" . $cgimatches[1] . "'";
		} else {
			log_error("e2guardian - " . $e2guardian_log['reportingcgi'] . " is not a valid access denied cgi url");
			file_notice("e2guardian - " . $e2guardian_log['reportingcgi'] . " is not a valid access denied cgi url", "");
		}
	}
	$accessdenied = ($e2guardian_log['reportingcgi'] ? $e2guardian_log['report_level'] : "3");
	$reportlanguage = ($e2guardian_log['report_language'] ? $e2guardian_log['report_language'] : "ukenglish");
	$showweightedfound = (preg_match('/showweightedfound/', $e2guardian_log['report_options']) ? "on" : "off");
	$usecustombannedflash = (preg_match('/usecustombannedflash/', $e2guardian_log['report_options']) ? "on" : "off");
	if (file_exists(E2GUARDIAN_DIR . '/share/e2guardian/blockedflash.swf')) {
		$custombannedflashfile = "custombannedflashfile = '" . E2GUARDIAN_DIR . "/share/e2guardian/blockedflash.swf'";
	}
	$usecustombannedimage = (preg_match('/usecustombannedimage/', $e2guardian_log['report_options'])? "on" : "off");
	$nonstandarddelimiter = (preg_match('/nonstandarddelimiter/', $e2guardian_log['report_options'])? "on" : "off");

	$logchildprocesshandling = (preg_match('/logchildprocesshandling/', $e2guardian_log['logging_options']) ? "on" : "off");
	$logconnectionhandlingerrors = (preg_match('/logconnectionhandlingerrors/', $e2guardian_log['logging_options']) ? "on" : "off");
	$nologger = (preg_match('/nologger/', $e2guardian_log['logging_options']) ? "on" : "off");
	$logadblocks = (preg_match('/logadblocks/', $e2guardian_log['logging_options']) ? "on" : "off");
	$anonymizelogs = (preg_match('/anonymizelogs/', $e2guardian_log['logging_options']) ? "on" : "off");

	$loglevel = ($e2guardian_log['loglevel'] ? $e2guardian_log['loglevel'] : "2");
	$logexceptionhits = ($e2guardian_log['logexceptionhits'] ? $e2guardian_log['logexceptionhits'] : "2");
	$logfileformat = ($e2guardian_log['logfileformat'] ? $e2guardian_log['logfileformat'] : "1");

	// check files, create sample files
	$files = array(
		"/e2guardianf1.conf",
		"/lists/filtergroupslist",
		"/lists/bannedphraselist",
		"/lists/exceptionphraselist",
		"/lists/weightedphraselist",
		"/lists/exceptionsitelist",
		"/lists/bannedsitelist",
		"/lists/greysitelist",
		"/lists/logsitelist",
		"/lists/bannedregexpurllist",
		"/lists/bannedurllist",
		"/lists/exceptionregexpurllist",
		"/lists/exceptionurllist",
		"/lists/greyurllist",
		"/lists/logregexpurllist",
		"/lists/logurllist",
		"/lists/urlregexplist",
		"/lists/exceptionfilesitelist",
		"/lists/exceptionfileurllist",
		"/lists/searchengineregexplist",
		"/lists/bannedsearchtermlist",
		"/lists/weightedsearchtermlist",
		"/lists/exceptionsearchtermlist",
		"/lists/contentregexplist",
		"/lists/exceptionextensionlist",
		"/lists/bannedextensionlist",
		"/lists/exceptionmimetypelist",
		"/lists/bannedmimetypelist",
		"/lists/headerregexplist",
		"/lists/bannedregexpheaderlist",
		"/lists/authplugins/ipgroups",
		"/lists/contentscanners/exceptionvirusextensionlist",
		"/lists/contentscanners/exceptionvirusmimetypelist",
		"/lists/contentscanners/exceptionvirussitelist",
		"/lists/contentscanners/exceptionvirusurllist",
		"/lists/exceptioniplist",
		"/lists/pics");

	$e2guardian_dir = E2GUARDIAN_DIR . "/etc/e2guardian";
	foreach ($files as $file) {
		if (!file_exists($e2guardian_dir . $file.'.sample')) {
			$new_file = "";
			$install_file = file($e2guardian_dir . $file);
			foreach ($install_file as $line) {
				if (!preg_match("/Include/", $line)) {
					$new_file .= $line;
				}
			}
			file_put_contents($e2guardian_dir . $file.'.sample', $new_file, LOCK_EX);
		}
	}

	$load_samples = 0;

	// SSL men-in-the-middle feature
	$dirs = array("/var/log/e2guardian/stats", "/etc/ssl/demoCA", "/etc/ssl/demoCA/private", "/etc/ssl/demoCA/crl", "/etc/ssl/demoCA/certs", $e2guardian_dir . "/ssl/generatedcerts", $e2guardian_dir . "/ssl/generatedlinks");
	foreach ($dirs as $dir) {
		if (!is_dir($dir)) {
			mkdir ($dir, 0755, true);
		}
	}
	$ca_cert = lookup_ca($e2guardian_config["dca"]);
	if ($ca_cert != false) {
		if (base64_decode($ca_cert['prv'])) {
			file_put_contents("/etc/ssl/demoCA/private/cakey.pem", base64_decode($ca_cert['prv']));
			$ca_pk = "caprivatekeypath = '/etc/ssl/demoCA/private/cakey.pem'";
		}
		if (base64_decode($ca_cert['crt'])) {
			$cert_hash = array();
			file_put_contents("/etc/ssl/demoCA/cacert.pem", base64_decode($ca_cert['crt']));
			exec("/usr/bin/openssl x509 -hash -noout -in /etc/ssl/demoCA/cacert.pem", $cert_hash);
			file_put_contents("/usr/local/share/certs/" . $cert_hash[0] . ".0", base64_decode($ca_cert['crt']));
			$ca_pem = "cacertificatepath = '/etc/ssl/demoCA/cacert.pem'";
			$generatedcertpath= "generatedcertpath = '" . $e2guardian_dir . "/ssl/generatedcerts'";
			$generatedlinkpath= "generatedlinkpath = '" . $e2guardian_dir . "/ssl/generatedlinks'";
		}
		$svr_cert = lookup_cert($e2guardian_config["dcert"]);
		if ($svr_cert != false) {
			if (base64_decode($svr_cert['prv'])) {
				file_put_contents("/etc/ssl/demoCA/private/serverkey.pem", base64_decode($svr_cert['prv']) . base64_decode($svr_cert['crt']));
				$cert_key = "certprivatekeypath = '/etc/ssl/demoCA/private/serverkey.pem' ";
			}
		}
	}

	// contentscanners preg_replace patterns
	$match[0] = "/(conf)/";
	$match[1] = "/(\/usr.local)/";
	$match[2] = "/,/";
	$replace[0] = "$1'";
	$replace[1] = "contentscanner = '" . E2GUARDIAN_DIR;
	$replace[2] = "\n";

	$contentscanners = preg_replace($match, $replace, $e2guardian_config['content_scanners']);

	// includes preg_replace patterns
	$match[0] = "/(.)$/";
	$match[1] = "/\/usr.local/";
	$match[2] = "/,/";
	$replace[0] = "$1>\n";
	$replace[1] = "\n.Include<" . E2GUARDIAN_DIR;
	$replace[2] = ">";

	/*
	Phrase ACL
	Create a default setup if not exists
	*/
	if (!is_array($config['installedpackages']['e2guardianphraseacl']['config'])) {
		$banned_file = file(E2GUARDIAN_DIR . "/etc/e2guardian/lists/bannedphraselist");
		foreach ($banned_file as $file_line) {
			if (preg_match ("/^.Include<(\S+)>/", $file_line, $matches)) {
				$banned_includes .= $matches[1] . ",";
			}
		}

		$weighted_file = file(E2GUARDIAN_DIR . "/etc/e2guardian/lists/weightedphraselist");
		foreach ($weighted_file as $file_line) {
			if (preg_match ("/^.Include<(\S+)>/", $file_line, $matches)) {
				$weighted_includes .= $matches[1] . ",";
			}
		}
		$config['installedpackages']['e2guardianphraseacl']['config'][0] = array(
			'name' => 'Default',
			'description' => 'Default Phrase access list setup',
			'banned_enabled' => "on",
			'weighted_enabled' => "on",
			'exception_enabled' => "on",
			'banned_includes' => substr($banned_includes, 0, -1),
			'weighted_includes' => substr($weighted_includes, 0, -1));
	}
	// loop on array
	$count = 0;
	if (is_array($config['installedpackages']['e2guardianphraseacl']['config'])) {
		foreach ($config['installedpackages']['e2guardianphraseacl']['config'] as $e2guardian_phrase) {
			// bannedphraselist
			if ($e2guardian_phrase['banned_phraselist'] == "" && file_exists($e2guardian_dir . '/lists/bannedphraselist.sample')) {
				$config['installedpackages']['e2guardianphraseacl']['config'][$count]['banned_phraselist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/bannedphraselist.sample'));
				$load_samples++;
			}
			$includes = preg_replace($match, $replace, $e2guardian_phrase['banned_includes']);
			file_put_contents($e2guardian_dir . "/lists/bannedphraselist." . $e2guardian_phrase['name'], ($e2guardian_phrase['banned_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianphraseacl']['config'][$count]['banned_phraselist']) . $includes : ""), LOCK_EX);

			// weightedphraselist
			if ($e2guardian_phrase['weighted_phraselist'] == "" && file_exists($e2guardian_dir . '/lists/weightedphraselist.sample')) {
				$config['installedpackages']['e2guardianphraseacl']['config'][$count]['weighted_phraselist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/weightedphraselist.sample'));
				$load_samples++;
			}
			$includes = preg_replace($match, $replace, $e2guardian_phrase['weighted_includes']);
			file_put_contents($e2guardian_dir . "/lists/weightedphraselist." . $e2guardian_phrase['name'], ($e2guardian_phrase['weighted_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianphraseacl']['config'][$count]['weighted_phraselist']) . $includes : ""), LOCK_EX);

			// exceptionphraselist
			if ($e2guardian_phrase['exception_phraselist'] == "" && file_exists($e2guardian_dir . '/lists/exceptionphraselist.sample')) {
				$config['installedpackages']['e2guardianphraseacl']['config'][$count]['exception_phraselist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/exceptionphraselist.sample'));
				$load_samples++;
			}
			file_put_contents($e2guardian_dir . "/lists/exceptionphraselist." . $e2guardian_phrase['name'], ($e2guardian_phrase['exception_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianphraseacl']['config'][$count]['exception_phraselist']) : ""), LOCK_EX);
			$count++;
		}
	}

	/*
	Site ACL
	Create a default setup if not exists
	*/
	if (!is_array($config['installedpackages']['e2guardiansiteacl']['config'])) {
		$config['installedpackages']['e2guardiansiteacl']['config'][0] = array(
			'name' => 'Default',
			'description' => 'Default Site access list setup',
			'exceptionsite_enabled' => "on",
			'bannedsite_enabled' => "on",
			'greysite_enabled' => "on",
			'urlsite_enabled' => "on");
	}
	// loop on array
	$count = 0;
	foreach ($config['installedpackages']['e2guardiansiteacl']['config'] as $e2guardian_site) {
		// exceptionsitelist
		if ($e2guardian_site['exception_sitelist'] == "" && file_exists($e2guardian_dir . '/lists/exceptionsitelist.sample')) {
			$config['installedpackages']['e2guardiansiteacl']['config'][$count]['exception_sitelist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/exceptionsitelist.sample'));
			$load_samples++;
		}
		$includes = preg_replace($match, $replace, $e2guardian_site['exception_includes']);
		file_put_contents($e2guardian_dir . "/lists/exceptionsitelist." . $e2guardian_site['name'], ($e2guardian_site['exceptionsite_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardiansiteacl']['config'][$count]['exception_sitelist']) . $includes : ""), LOCK_EX);

		// exceptionfilesitelist
		if ($e2guardian_site['exceptionfile_sitelist'] == "" && file_exists($e2guardian_dir . '/lists/exceptionfilesitelist.sample')) {
			$config['installedpackages']['e2guardiansiteacl']['config'][$count]['exceptionfile_sitelist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/exceptionfilesitelist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/exceptionfilesitelist." . $e2guardian_site['name'], ($e2guardian_site['exceptionsite_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardiansiteacl']['config'][$count]['exceptionfile_sitelist']) : ""), LOCK_EX);

		// bannedsitelist
		if ($e2guardian_site['banned_sitelist'] == "" && file_exists($e2guardian_dir . '/lists/bannedsitelist.sample')) {
			$config['installedpackages']['e2guardiansiteacl']['config'][$count]['banned_sitelist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/bannedsitelist.sample'));
			$load_samples++;
		}
		$includes = preg_replace($match, $replace, $e2guardian_site['banned_includes']);
		file_put_contents($e2guardian_dir . "/lists/bannedsitelist." . $e2guardian_site['name'], ($e2guardian_site['bannedsite_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardiansiteacl']['config'][$count]['banned_sitelist']) . $includes : ""), LOCK_EX);

		// greysitelist
		if ($e2guardian_site['grey_sitelist'] == "" && file_exists($e2guardian_dir . '/lists/greysitelist.sample')) {
			$config['installedpackages']['e2guardiansiteacl']['config'][$count]['grey_sitelist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/greysitelist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/greysitelist." . $e2guardian_site['name'], ($e2guardian_site['greysite_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardiansiteacl']['config'][$count]['grey_sitelist']) : ""), LOCK_EX);

		//logsitelist
		if ($e2guardian_site['log_sitelist'] == "" && file_exists($e2guardian_dir . '/lists/logsitelist.sample')) {
			$config['installedpackages']['e2guardiansiteacl']['config'][$count]['log_sitelist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/logsitelist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/logsitelist." . $e2guardian_site['name'], ($e2guardian_site['urlsite_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardiansiteacl']['config'][$count]['log_sitelist']) : ""), LOCK_EX);
		$count++;
	}

	/*
	URL ACL
	create a default setup if not exists
	*/
	if (!is_array($config['installedpackages']['e2guardianurlacl']['config'])) {
		$config['installedpackages']['e2guardianurlacl']['config'][0] = array(
			'name' => 'Default',
			'description' => 'Default Url access list setup',
			'bannedurl_enabled' => "on",
			'exceptionurl_enabled' => "on",
			'contenturl_enabled' => "on",
			'greyurl_enabled' => "on");
	}
	// loop on array
	$count = 0;
	foreach ($config['installedpackages']['e2guardianurlacl']['config'] as $e2guardian_url) {
		// bannedurllist
		if ($e2guardian_url['banned_urllist'] == "" && file_exists($e2guardian_dir . '/lists/bannedurllist.sample')) {
			$config['installedpackages']['e2guardianurlacl']['config'][$count]['banned_urllist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/bannedurllist.sample'));
			$load_samples++;
		}
		$includes = preg_replace($match, $replace, $e2guardian_url['banned_includes']);
		file_put_contents($e2guardian_dir . "/lists/bannedurllist." . $e2guardian_url['name'], ($e2guardian_url['bannedurl_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianurlacl']['config'][$count]['banned_urllist']) . $includes : ""), LOCK_EX);

		// bannedregexpurllist
		if ($e2guardian_url['bannedregexp_urllist'] == "" && file_exists($e2guardian_dir . '/lists/bannedregexpurllist.sample')) {
			$config['installedpackages']['e2guardianurlacl']['config'][$count]['bannedregexp_urllist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/bannedregexpurllist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/bannedregexpurllist." . $e2guardian_url['name'], ($e2guardian_url['bannedurl_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianurlacl']['config'][$count]['bannedregexp_urllist']) : ""), LOCK_EX);

		// greyurllist
		if ($e2guardian_url['grey_urllist'] == "" && file_exists($e2guardian_dir . '/lists/greyurllist.sample')) {
			$config['installedpackages']['e2guardianurlacl']['config'][$count]['grey_urllist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/greyurllist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/greyurllist." . $e2guardian_url['name'], ($e2guardian_url['greyurl_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianurlacl']['config'][$count]['grey_urllist']) : ""), LOCK_EX);

		// exceptionfileurllist
		if ($e2guardian_url['exceptionfile_urllist'] == "" && file_exists($e2guardian_dir . '/lists/exceptionfileurllist.sample')) {
			$config['installedpackages']['e2guardianurlacl']['config'][$count]['exceptionfile_urllist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/exceptionfileurllist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/exceptionfileurllist." . $e2guardian_url['name'], ($e2guardian_url['exceptionurl_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianurlacl']['config'][$count]['exceptionfile_urllist']) : ""), LOCK_EX);

		// exceptionregexpurllist
		if ($e2guardian_url['exceptionregexp_urllist'] == "" && file_exists($e2guardian_dir . '/lists/exceptionregexpurllist.sample')) {
			$config['installedpackages']['e2guardianurlacl']['config'][$count]['exceptionregexp_urllist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/exceptionregexpurllist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/exceptionregexpurllist." . $e2guardian_url['name'], ($e2guardian_url['exceptionurl_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianurlacl']['config'][$count]['exceptionregexp_urllist']) : ""), LOCK_EX);

		// exceptionurllist
		if ($e2guardian_url['exception_urllist'] == "" && file_exists($e2guardian_dir . '/lists/exceptionurllist.sample')) {
			$config['installedpackages']['e2guardianurlacl']['config'][$count]['exception_urllist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/exceptionurllist.sample'));
			$load_samples++;
		}
		$includes = preg_replace($match, $replace, $e2guardian_url['exception_includes']);
		file_put_contents($e2guardian_dir . "/lists/exceptionurllist." . $e2guardian_url['name'], ($e2guardian_url['exceptionurl_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianurlacl']['config'][$count]['exception_urllist']) . $includes : ""), LOCK_EX);

		// urlregexplist
		if ($e2guardian_url['modify_urllist'] == "" && file_exists($e2guardian_dir . '/lists/urlregexplist.sample')) {
			$config['installedpackages']['e2guardianurlacl']['config'][$count]['modify_urllist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/urlregexplist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/urlregexplist." . $e2guardian_url['name'], ($e2guardian_url['contenturl_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianurlacl']['config'][$count]['modify_urllist']) : ""), LOCK_EX);

		// logurllist
		if ($e2guardian_url['log_urllist'] == "" && file_exists($e2guardian_dir . '/lists/logurllist.sample')) {
			$config['installedpackages']['e2guardianurlacl']['config'][$count]['log_urllist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/logurllist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/logurllist." . $e2guardian_url['name'], ($e2guardian_url['logurl_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianurlacl']['config'][$count]['log_urllist']) : ""), LOCK_EX);

		// logregexpurllist
		if ($e2guardian_url['logregexp_urllist'] == "" && file_exists($e2guardian_dir . '/lists/logregexpurllist.sample')) {
			$config['installedpackages']['e2guardianurlacl']['config'][$count]['logregexp_urllist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/logregexpurllist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/logregexpurllist." . $e2guardian_url['name'], ($e2guardian_url['logurl_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianurlacl']['config'][$count]['logregexp_urllist']) : ""), LOCK_EX);
		$count++;
	}

	/*
	Pics ACL
	Create a default setup if not exists
	*/
	if (!is_array($config['installedpackages']['e2guardianpicsacl']['config'])) {
		$config['installedpackages']['e2guardianpicsacl']['config'][0] = array(
			'name' => 'Default',
			'description' => 'Default file access list setup');
	}
	// loop on array
	$count = 0;
	foreach ($config['installedpackages']['e2guardianpicsacl']['config'] as $e2guardian_pics) {
		//pics
		if ($e2guardian_pics['pics'] == "" && file_exists($e2guardian_dir . '/lists/pics.sample')) {
			$config['installedpackages']['e2guardianpicsacl']['config'][$count]['pics'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/pics.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/pics." . $e2guardian_pics['name'], ($e2guardian_pics['pics_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianpicsacl']['config'][$count]['pics']) : ""), LOCK_EX);
		$count++;
	}

	/*
	Search ACL
	Create a default setup if not exists
	*/
	if (!is_array($config['installedpackages']['e2guardiansearchacl']['config'])) {
		$config['installedpackages']['e2guardiansearchacl']['config'][0] = array(
			'name' => 'Default',
			'description' => 'Default search engine list setup');
	}
	// loop on array
	$count = 0;
	foreach ($config['installedpackages']['e2guardiansearchacl']['config'] as $e2guardian_search) {
		// searchengineregexplist
		if ($e2guardian_search['searchengineregexplist'] == "" && file_exists($e2guardian_dir . '/lists/searchengineregexplist.sample')) {
			$config['installedpackages']['e2guardiansearchacl']['config'][$count]['searchengineregexplist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/searchengineregexplist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/searchengineregexplist." . $e2guardian_search['name'], ($e2guardian_search['regexp_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardiansearchacl']['config'][$count]['searchengineregexplist']) : ""), LOCK_EX);

		// bannedsearchtermlist
		if ($e2guardian_search['banned_searchtermlist'] == "" && file_exists($e2guardian_dir . '/lists/bannedsearchtermlist.sample')) {
			$config['installedpackages']['e2guardiansearchacl']['config'][$count]['banned_searchtermlist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/bannedsearchtermlist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/bannedsearchtermlist." . $e2guardian_search['name'], ($e2guardian_search['banned_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardiansearchacl']['config'][$count]['banned_searchtermlist']) : ""), LOCK_EX);

		// weightedsearchtermlist
		if ($e2guardian_search['weighted_searchtermlist'] == "" && file_exists($e2guardian_dir . '/lists/weightedsearchtermlist.sample')) {
			$config['installedpackages']['e2guardiansearchacl']['config'][$count]['weighted_searchtermlist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/weightedsearchtermlist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/weightedsearchtermlist." . $e2guardian_search['name'], ($e2guardian_search['weighted_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardiansearchacl']['config'][$count]['weighted_searchtermlist']) : ""), LOCK_EX);

		// exceptionsearchtermlist
		if ($e2guardian_search['exception_searchtermlist'] == "" && file_exists($e2guardian_dir . '/lists/exceptionsearchtermlist.sample')) {
			$config['installedpackages']['e2guardiansearchacl']['config'][$count]['exception_searchtermlist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/exceptionsearchtermlist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/exceptionsearchtermlist." . $e2guardian_search['name'], ($e2guardian_search['exception_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardiansearchacl']['config'][$count]['exception_searchtermlist']) : ""), LOCK_EX);
		$count++;
		}

	/*
	File ACL
	Ccreate a default setup if not exists
	*/
	if (!is_array($config['installedpackages']['e2guardianfileacl']['config'])) {
		$config['installedpackages']['e2guardianfileacl']['config'][0] = array(
			'name' => 'Default',
			'description' => 'Default file access list setup',
			'exception_enabled' => "on",
			'banned_enabled' => "on");
	}
	// loop on array
	$count = 0;
	foreach ($config['installedpackages']['e2guardianfileacl']['config'] as $e2guardian_file) {
		// exceptionextensionlist
		if ($e2guardian_file['exception_extensionlist'] == "" && file_exists($e2guardian_dir . '/lists/exceptionextensionlist.sample')) {
			$config['installedpackages']['e2guardianfileacl']['config'][$count]['exception_extensionlist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/exceptionextensionlist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/exceptionextensionlist." . $e2guardian_file['name'], ($e2guardian_file['exception_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianfileacl']['config'][$count]['exception_extensionlist']) : ""), LOCK_EX);

		// exceptionmimetypelist
		if ($e2guardian_file['exception_mimetypelist'] == "" && file_exists($e2guardian_dir . '/lists/exceptionmimetypelist.sample')) {
			$config['installedpackages']['e2guardianfileacl']['config'][$count]['exception_mimetypelist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/exceptionmimetypelist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/exceptionmimetypelist." . $e2guardian_file['name'], ($e2guardian_file['exception_enabled'] ? e2_text_area_decode($config['installedpackages']['exception_mimetypelist']) : ""), LOCK_EX);

		// bannedextensionlist
		if ($e2guardian_file['banned_extensionlist'] == "" && file_exists($e2guardian_dir . '/lists/bannedextensionlist.sample')) {
			$config['installedpackages']['e2guardianfileacl']['config'][$count]['banned_extensionlist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/bannedextensionlist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/bannedextensionlist." . $e2guardian_file['name'], ($e2guardian_file['banned_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianfileacl']['config'][$count]['banned_extensionlist']) : ""), LOCK_EX);

		// bannedmimetypelist
		if ($e2guardian_file['banned_mimetypelist'] == "" && file_exists($e2guardian_dir . '/lists/bannedmimetypelist.sample')) {
			$config['installedpackages']['e2guardianfileacl']['config'][$count]['banned_mimetypelist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/bannedmimetypelist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/bannedmimetypelist." . $e2guardian_file['name'], ($e2guardian_file['banned_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianfileacl']['config'][$count]['banned_mimetypelist']) : ""), LOCK_EX);
		$count++;
	}

	/*
	Header ACL
	Ccreate a default setup if not exists
	*/
	if (!is_array($config['installedpackages']['e2guardianheaderacl']['config'])) {
		$config['installedpackages']['e2guardianheaderacl']['config'][0] = array(
			'name' => 'Default',
			'description' => 'Default header access list setup');
	}
	// loop on array
	$count = 0;
	foreach ($config['installedpackages']['e2guardianheaderacl']['config'] as $e2guardian_header) {
		// headerregexplist
		if ($e2guardian_header['header_regexplist'] == "" && file_exists($e2guardian_dir . '/lists/headerregexplist.sample')) {
			$config['installedpackages']['e2guardianheaderacl']['config'][$count]['header_regexplist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/headerregexplist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/headerregexplist." . $e2guardian_header['name'], ($e2guardian_header['regexp_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianheaderacl']['config'][$count]['header_regexplist']) : ""), LOCK_EX);

		// bannedregexpheaderlist
		if ($e2guardian_header['banned_regexpheaderlist'] == "" && file_exists($e2guardian_dir . '/lists/bannedregexpheaderlist.sample')) {
			$config['installedpackages']['e2guardianheaderacl']['config'][$count]['banned_regexpheaderlist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/bannedregexpheaderlist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/bannedregexpheaderlist." . $e2guardian_header['name'], ($e2guardian_header['banned_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianheaderacl']['config'][$count]['banned_regexpheaderlist']) : ""), LOCK_EX);

		$count++;
	}

	/*
	Content ACL
	Create a default setup if not exists
	*/
	if (!is_array($config['installedpackages']['e2guardiancontentacl']['config'])) {
		$config['installedpackages']['e2guardiancontentacl']['config'][0] = array(
			'name' => 'Default',
			'description' => 'Default content setup');
	}
	// loop on array
	$count = 0;
	foreach ($config['installedpackages']['e2guardiancontentacl']['config'] as $e2guardian_content) {
		//content_regexplist
		if ($e2guardian_content['content_regexplist'] == "" && file_exists($e2guardian_dir . '/lists/contentregexplist.sample')) {
			$config['installedpackages']['e2guardiancontentacl']['config'][$count]['content_regexplist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/contentregexplist.sample'));
			$load_samples++;
		}
		file_put_contents($e2guardian_dir . "/lists/contentregexplist." . $e2guardian_content['name'], ($e2guardian_content['content_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardiancontentacl']['config'][$count]['content_regexplist']) : ""), LOCK_EX);
		$count++;
	}

	/*
	Antivirus ACL
	Create a default setup if not exists
	*/
	if (!is_array($config['installedpackages']['e2guardianantivirusacl']['config'])) {
		$config['installedpackages']['e2guardianantivirusacl']['config'][0] = array();
	}

	$e2guardian_antivirus = $config['installedpackages']['e2guardianantivirusacl']['config'][0];
	// exceptionvirusmimetypelist
	if ($e2guardian_antivirus['mime_list'] == "" && file_exists($e2guardian_dir . '/lists/contentscanners/exceptionvirusmimetypelist.sample')) {
		$config['installedpackages']['e2guardianantivirusacl']['config'][0]['mime_list'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/contentscanners/exceptionvirusmimetypelist.sample'));
		$load_samples++;
	}
	file_put_contents($e2guardian_dir . "/lists/contentscanners/exceptionvirusmimetypelist", ($e2guardian_antivirus['mime_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianantivirusacl']['config'][0]['mime_list']) : ""), LOCK_EX);

	// exceptionvirussitelist
	if ($e2guardian_antivirus['site_list'] == "" && file_exists($e2guardian_dir . '/lists/contentscanners/exceptionvirussitelist.sample')) {
		$config['installedpackages']['e2guardianantivirusacl']['config'][0]['site_list'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/contentscanners/exceptionvirussitelist.sample'));
		$load_samples++;
	}
	file_put_contents($e2guardian_dir . "/lists/contentscanners/exceptionvirussitelist", ($e2guardian_antivirus['site_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianantivirusacl']['config'][0]['site_list']) : ""), LOCK_EX);

	// exceptionvirusurllist
	if ($e2guardian_antivirus['url_list'] == "" && file_exists($e2guardian_dir . '/lists/contentscanners/exceptionvirusurllist.sample')) {
		$config['installedpackages']['e2guardianantivirusacl']['config'][0]['url_list'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/contentscanners/exceptionvirusurllist.sample'));
		$load_samples++;
	}
	file_put_contents($e2guardian_dir . "/lists/contentscanners/exceptionvirusurllist", ($e2guardian_antivirus['url_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianantivirusacl']['config'][0]['url_list']) : ""), LOCK_EX);

	// exceptionvirusextensionlist
	if ($e2guardian_antivirus['extension_list'] == "" && file_exists($e2guardian_dir . '/lists/contentscanners/exceptionvirusextensionlist.sample')) {
		$config['installedpackages']['e2guardianantivirusacl']['config'][0]['extension_list'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/contentscanners/exceptionvirusextensionlist.sample'));
		$load_samples++;
	}
	file_put_contents($e2guardian_dir . "/lists/contentscanners/exceptionvirusextensionlist", ($e2guardian_antivirus['extension_enabled'] ? e2_text_area_decode($config['installedpackages']['e2guardianantivirusacl']['config'][0]['extension_list']):""), LOCK_EX);

	// log report
	if ($e2guardian_log['report_file'] == "" && file_exists("/usr/local/share/e2guardian/languages/" . $reportlanguage . "/template.html")) {
		$report_file = file_get_contents("/usr/local/share/e2guardian/languages/" . $reportlanguage . "/template.html");
		$report_file = preg_replace('/<.*(html|head)>/', '', $report_file);
		$config['installedpackages']['e2guardianlog']['config'][0]['report_file'] = base64_encode($report_file);
		$e2guardian_log['report_file'] = base64_encode($report_file);
		$load_samples++;
	}

	/*
	Exception IP List
	Create a default setup if not exists
	*/
	if (!is_array($config['installedpackages']['e2guardianips']['config'])) {
		$config['installedpackages']['e2guardianips']['config'][0] = array("exceptioniplist" => "");
	}
	if ($config['installedpackages']['e2guardianips']['config'][0]['exceptioniplist'] == "" && file_exists($e2guardian_dir . '/lists/exceptioniplist.sample')) {
		$config['installedpackages']['e2guardianips']['config'][0]['exceptioniplist'] = base64_encode(file_get_contents($e2guardian_dir . '/lists/exceptioniplist.sample'));
		$load_samples++;
	}
	file_put_contents($e2guardian_dir . "/lists/exceptioniplist", e2_text_area_decode($config['installedpackages']['e2guardianips']['config'][0]['exceptioniplist']), LOCK_EX);

	if ($load_samples > 0) {
		write_config();
	}

	/* Filtergroups */
	if (!is_array($config['installedpackages']['e2guardiangroups']['config'])) {
		$config['installedpackages']['e2guardiangroups']['config'][0] = array(
			'name' => 'Default',
			'description' => 'Default e2guardian filtergroup',
			'picsacl' => "Default",
			'phraseacl' => "Default",
			'siteacl' => "Default",
			'extensionacl' => "Default",
			'headeracl' => "Default",
			'contentacl' => "Default",
			'searchacl' => "Default",
			'urlacl' => "Default",
			'group_options' => "scancleancache,infectionbypasserrorsonly",
			'reportinglevel' => '3',
			'group_name_source' => 'name',
			'mode' => "1",
			'report_level' =>"global");
	}
	$groups = array("scancleancache", "hexdecodecontent", "blockdownloads", "enablepics", "deepurlanalysis", "infectionbypasserrorsonly", "disablecontentscan", "sslcheckcert", "sslmitm");
	// loop on array
	$count = 1;
	$user_xml = "";
	$filtergroupslist = "";
	foreach ($config['installedpackages']['e2guardiangroups']['config'] as $e2guardian_groups) {
		$e2guardian_group_name = strtolower($e2guardian_groups['name']);
		$dgfg[$count] = $e2guardian_group_name;
		$e2guardian_groups['blockdownloads'] = ($e2guardian_groups['blockdownloads'] ? $e2guardian_groups['blockdownloads'] : "off");
		$e2guardian_groups['weightedphrasemode'] = (preg_match("/\d/", $e2guardian_groups['weightedphrasemode']) ? $e2guardian_groups['weightedphrasemode'] : $e2guardian_config['weightedphrasemode']);
		$e2guardian_groups['naughtynesslimit'] = ($e2guardian_groups['naughtynesslimit'] ? $e2guardian_groups['naughtynesslimit'] : "50");
		$e2guardian_groups['searchtermlimit'] = ($e2guardian_groups['searchtermlimit'] ? $e2guardian_groups['searchtermlimit'] : "30");
		$e2guardian_groups['categorydisplaythreshold'] = ($e2guardian_groups['categorydisplaythreshold'] ? $e2guardian_groups['categorydisplaythreshold'] : "0");
		$e2guardian_groups['embeddedurlweight'] = ($e2guardian_groups['embeddedurlweight'] ? $e2guardian_groups['embeddedurlweight'] : "0");
		$e2guardian_groups['bypass'] = ($e2guardian_groups['bypass'] ? $e2guardian_groups['bypass'] : "0");
		$e2guardian_groups['infectionbypass'] = ($e2guardian_groups['infectionbypass'] ? $e2guardian_groups['infectionbypass'] : "0");
		$e2guardian_groups['maxuploadsize'] = (is_numeric($e2guardian_groups['maxuploadsize']) ? $e2guardian_groups['maxuploadsize'] : $maxuploadsize);
		switch ($e2guardian_groups['reportinglevel']) {
			case "1":
			case "2":
				$groupreportinglevel = "reportinglevel = " . $e2guardian_groups['reportinglevel'];
				if (preg_match("@(\w+://[a-zA-Z0-9.:/\-]+)@", $e2guardian_groups['reportingcgi'], $cgimatches)) {
					$groupaccessdeniedaddress = "accessdeniedaddress = '" . $cgimatches[1] . "'";
				} else {
					log_error('E2guardian - Group ' . $e2guardian_groups['name'] . ' does not have a valid access denied cgi url.');
					file_notice('E2guardian - Group ' . $e2guardian_groups['name'] . ' does not have a valid access denied cgi url.', "");
				}
				break;
			case "-1":
			case "0":
			case "3":
				$groupreportinglevel = "reportinglevel = " . $e2guardian_groups['reportinglevel'];
				$groupaccessdeniedaddress = "";
				break;
			default:
				$groupreportinglevel = "";
				$groupaccessdeniedaddress = "";
		}

		foreach ($groups as $group) {
			$e2guardian_groups[$group] = (preg_match("/$group/", $e2guardian_groups['group_options']) ? "on" : "off");
		}


		// create group list files
		$lists = array(
			"phraseacl" => array("bannedphrase", "weightedphrase", "exceptionphrase"),
			"siteacl" => array("bannedsite", "greysite", "exceptionsite", "exceptionfilesite", "logsite"),
			"urlacl" => array("bannedurl", "greyurl", "exceptionurl", "exceptionregexpurl", "bannedregexpurl", "urlregexp", "exceptionfileurl", "logurl", "logregexpurl"),
			"contentacl" => array("contentregexp"),
			"extensionacl" => array("exceptionextension", "exceptionmimetype", "bannedextension", "bannedmimetype"),
			"headeracl" => array("headerregexp", "bannedregexpheader"),
			"searchacl" => array("searchengineregexp", "bannedsearchterm", "weightedsearchterm", "exceptionsearchterm")
		);
		foreach ($lists as $list_key => $list_array) {
			// verify groups acls to avoid errors on empty acl group options
			if (!preg_match("/\w+/", $e2guardian_groups[$list_key])) {
				log_error("e2guardian - Config warning, Group {$e2guardian_group_name} {$list_key} cannot be empty! Trying to load sample values");
			}
			foreach ($list_array as $list_value) {
				// read all access lists applied to this group option
				foreach (explode(",", $e2guardian_groups[$list_key]) as $dacl) {
					if (!is_array(${$list_value})) {
						${$list_value} = array();
					}
					$dacl = (preg_match("/\w+/", $dacl)? $dacl : "sample");
					$file_temp = file_get_contents(E2GUARDIAN_DIR . "/etc/e2guardian/lists/{$list_value}list.{$dacl}") . "\n";
					${$list_value} = array_merge(explode("\n", $file_temp), ${$list_value});
				}
				// add a package warning
				array_unshift(${$list_value}, "#Do not edit this file.", "#It's created by e2guardian package and overwritten every config save.");
				// save group file and unset array
				file_put_contents(E2GUARDIAN_DIR . "/etc/e2guardian/lists/{$list_value}list.g_{$e2guardian_groups['name']}", implode("\n", array_unique(${$list_value})) . "\n", LOCK_EX);
				unset(${$list_value});
			}
		}

		$e2_dir = E2GUARDIAN_DIR;
		include ("/usr/local/pkg/e2guardianfx.conf.template");
		file_put_contents($e2guardian_dir . "/e2guardianf" . $count . ".conf", $dgf, LOCK_EX);

		if ($config['installedpackages']['e2guardianusers']['config'][0][$e2guardian_group_name] != "") {
			$import_users = explode("\n", base64_decode($config['installedpackages']['e2guardianusers']['config'][0][$e2guardian_group_name]));
			asort($import_users);
			$config['installedpackages']['e2guardianusers']['config'][0][$e2guardian_group_name] = base64_encode(implode("\n", $import_users));
			foreach ($import_users as $new_user) {
				if (preg_match("/(\S+)\s+(\S+)/", $new_user, $matches)) {
					$filtergroupslist .= $matches[1] . "=filter" . $count . " #" . $matches[2] . "\n";
				} elseif (preg_match("/(\S+)/", $new_user, $matches)) {
					$filtergroupslist .= $matches[1] . "=filter" . $count . "\n";
				}
			}
		}
		if ($config['installedpackages']['e2guardianips']['config'][0][$e2guardian_group_name] != "") {
			$import_ips = explode("\n", base64_decode($config['installedpackages']['e2guardianips']['config'][0][$e2guardian_group_name]));
			asort($import_ips);
			$config['installedpackages']['e2guardianips']['config'][0][$e2guardian_group_name] = base64_encode(implode("\n", $import_ips));
			foreach ($import_ips as $new_ip) {
				if (preg_match("/(\S+)\s+(.*)/", $new_ip, $matches)) {
					$filtergroupsiplist .= $matches[1] . " = filter" . $count . " #" . $matches[2] . "\n";
				} elseif (preg_match("/(\S+)/", $new_ip, $matches)) {
					$filtergroupsiplist .= $matches[1] . " = filter" . $count . "\n";
				}
			}
		}
		$filtergroup_count = count($import_users);
		$filtergroupip_count = count($import_ips);
		// default group catch all unauth groups as well non listed users
		if ($count > 1)
		$user_xml .=<<<EOF
		<field>
			<name>{$e2guardian_groups['description']} ({$filtergroup_count})</name>
			<type>listtopic</type>
		</field>
		<field>
			<fieldname>{$e2guardian_group_name}</fieldname>
			<fielddescr>{$e2guardian_groups['name']}</fielddescr>
			<description><![CDATA[Include users for this group one per line<br/>Hint:PFSENSE\marcelloc #Marcello Coutinho]]></description>
			<type>textarea</type>

			<cols>80</cols><rows>12</rows>
			<encoding>base64</encoding>
		</field>
EOF;
		$ips_xml .=<<<EOF
		<field>
			<name>{$e2guardian_groups['description']} ({$filtergroupip_count})</name>
			<type>listtopic</type>
		</field>
		<field>
			<fieldname>{$e2guardian_group_name}</fieldname>
			<fielddescr>{$e2guardian_groups['name']}</fielddescr>
			<description><![CDATA[Include ip addresses and or ip addresses/netmask for this group one per line<br/>Hint:&nbsp;192.168.1.0/255.255.255.0<br/>192.168.1.5]]></description>
			<type>textarea</type>

			<cols>80</cols><rows>12</rows>
			<encoding>base64</encoding>
		</field>
EOF;

		$count++;
	}
	if ($user_xml == "") {
		$user_xml .=<<<EOF
		<field>
			<name>Users</name>
			<type>listtopic</type>
		</field>
		<field>
			<fielddescr>Users</fielddescr>
			<fieldname>info_checkbox</fieldname>
			<type>checkbox</type>
			<description><![CDATA[E2guardian users are required only when you have more then one group.<br/>All unauthenticated users or unlisted users will match first filter group.]]></description>
		</field>
EOF;
	}
	// create/update filtergroupslist
	file_put_contents($e2guardian_dir . "/lists/filtergroupslist", $filtergroupslist, LOCK_EX);
	// create/update filtergroupsiplist
	file_put_contents($e2guardian_dir . "/lists/authplugins/ipgroups", $filtergroupsiplist, LOCK_EX);
	// create/update userlist xml file
	$ips_xml_header = file_get_contents("/usr/local/pkg/e2guardian_ips_header.template");
	$user_xml_header = file_get_contents("/usr/local/pkg/e2guardian_users_header.template");
	$user_xml_footer = file_get_contents("/usr/local/pkg/e2guardian_users_footer.template");
	file_put_contents("/usr/local/pkg/e2guardian_users.xml", $user_xml_header . $user_xml . $user_xml_footer, LOCK_EX);
	file_put_contents("/usr/local/pkg/e2guardian_ips.xml", $ips_xml_header . $ips_xml . $user_xml_footer, LOCK_EX);

	// create report template
	if (is_dir("/usr/local/share/e2guardian/languages/" . $reportlanguage)) {
		file_put_contents("/usr/local/share/e2guardian/languages/" . $reportlanguage . "/template.html", e2_text_area_decode($e2guardian_log['report_file']), LOCK_EX);
	}
	// check blacklist download files
	if ($install_process == true) {
		require_once("/usr/local/www/e2guardian.php");
		fetch_blacklist(false, true);
		update_output_window("Blacklist check done, continuing package config sync.");
	} else {
		if ($e2guardian_blacklist['cron'] == "force_download") {
			log_error("Blacklist update process started");
			file_notice("E2guardian - Blacklist update process started", "");
			file_put_contents("/root/e2guardian_custom.script", base64_decode($e2guardian_blacklist['custom_script']), LOCK_EX);
			if ($e2guardian_blacklist['enable_custom_script'] && $e2guardian_blacklist['custom_script'] != "") {
				mwexec_bg("/root/e2guardian_custom.script");
			} else {
				mwexec_bg("/usr/local/bin/php /usr/local/www/e2guardian.php fetch_blacklist");
			}
		}
		// update xml categories from downloaded file
		if ($e2guardian_blacklist['cron'] == "force_update") {
			$config['installedpackages']['e2guardianblacklist']['config'][0]['cron'] = "never";
			mwexec_bg("/usr/local/bin/php /usr/local/www/e2guardian.php update_lists");
		}

		// import default blacklists
		if (!is_array($config['installedpackages']['e2guardianblacklistsurls']['config'])) {
			mwexec_bg("/usr/local/bin/php /usr/local/www/e2guardian.php update_lists");
		}
	}
	// get clamav user
	$cconf = E2GUARDIAN_DIR . "/etc/clamd.conf";
	$cconf_file = file_get_contents($cconf);
	if (preg_match("/User (\w+)/", $cconf_file, $matches)) {
		mwexec("/usr/sbin/pw user show {$matches[1]} || /usr/sbin/pw user add -n {$matches[1]} -s /usr/sbin/nologin");
		$daemonuser = $matches[1];
		$daemongroup = 'nobody';
	} else {
		$daemonuser = 'nobody';
		$daemongroup = 'nobody';
	}
	$filtergroups = ($count > 1?($count -1):1);
	$filterip = "";
	$filterports = "";
	foreach (explode(",", $e2guardian['interface']) as $i => $iface) {
		$real_ifaces[] = e2_get_real_interface_address($iface);
		if ($real_ifaces[$i][0]) {
			$filterip .= "filterip = " . $real_ifaces[$i][0] . "\n";
		}
		$filterports .= "filterports = " . $filterport . "\n";
	}
	$filterip = ($filterip == "" ? "filterip = " : $filterip);
	$filterports = ($filterports == "" ? "filterports = $filterport" : $filterports);
	include("/usr/local/pkg/e2guardian.conf.template");

	// check crontab
	$new_cron = array();
	$cron_found = 0;
	if (is_array($config['cron']['item'])) {
		foreach ($config['cron']['item'] as $cron) {
			if (preg_match("@(" . E2GUARDIAN_DIR . "|/usr/local)/(bin.freshclam|www/e2guardian)@", $cron["command"])) {
				$cron_found++;
			} else {
				$new_cron['item'][] = $cron;
			}
		}
	}
	$cron_cmd = E2GUARDIAN_DIR . "/bin/freshclam";
	if ($e2guardian_config['cron'] && preg_match("/clamd/", $e2guardian_config['content_scanners'])) {
		switch ($e2guardian_config['cron']) {
			case "day":
				$new_cron['item'][] = array(
					"minute" => "0",
					"hour" => "0",
					"mday" => "*",
					"month" => "*",
					"wday" => "*",
					"who" => "root",
					"command" => $cron_cmd);
				$config['cron'] = $new_cron;
				$cron_found++;
				break;
			case "02days":
				$new_cron['item'][] = array(
					"minute" => "0",
					"hour" => "0",
					"mday" => "*/2",
					"month" => "*",
					"wday" => "*",
					"who" => "root",
					"command" => $cron_cmd);
				$config['cron'] = $new_cron;
				$cron_found++;
				break;
			case "week":
				$new_cron['item'][] = array(
					"minute" => "0",
					"hour" => "0",
					"mday" => "*/7",
					"month" => "*",
					"wday" => "*",
					"who" => "root",
					"command" => $cron_cmd);
				$config['cron'] = $new_cron;
				$cron_found++;
				break;
		}
	}
	$cron_cmd = "/usr/local/bin/php /usr/local/www/e2guardian.php fetch_blacklist";
	if ($e2guardian_blacklist['cron']) {
		switch ($e2guardian_blacklist['cron']) {
			case "day":
				$new_cron['item'][] = array(
					"minute" => "0",
					"hour" => "0",
					"mday" => "*",
					"month" => "*",
					"wday" => "*",
					"who" => "root",
					"command" => $cron_cmd);
				$config['cron'] = $new_cron;
				$cron_found++;
				break;
			case "02days":
				$new_cron['item'][] = array(
					"minute" => "0",
					"hour" => "0",
					"mday" => "*/2",
					"month" => "*",
					"wday" => "*",
					"who" => "root",
					"command" => $cron_cmd);
				$config['cron'] = $new_cron;
				$cron_found++;
				break;
			case "week":
				$new_cron['item'][] = array(
					"minute" => "0",
					"hour" => "0",
					"mday" => "*/7",
					"month" => "*",
					"wday" => "*",
					"who" => "root",
					"command" => $cron_cmd);
				$config['cron'] = $new_cron;
				$cron_found++;
				break;
		}
	}
	$cron_cmd = "/usr/local/bin/php /usr/local/www/e2guardian_ldap.php";
	if (is_array($config['installedpackages']['e2guardiangroups']['config'])) {
		foreach ($config['installedpackages']['e2guardiangroups']['config'] as $e2guardian_groups) {
			$e2_group_source = ($e2guardian_groups['groupnamesource'] != "" ? $e2guardian_groups['groupnamesource'] : "name");
			if (preg_match('/(\d+)m/', $e2guardian_groups['freq'], $matches)) {
				$new_cron['item'][] = array(
					"minute" => "*/". $matches[1],
					"hour" => "*",
					"mday" => "*",
					"month" => "*",
					"wday" => "*",
					"who" => "root",
					"command" => "{$cron_cmd} $e2_group_source '{$e2guardian_groups[$e2_group_source]}'");
				$config['cron'] = $new_cron;
				$cron_found++;
			}
			if (preg_match('/(\d+)h/', $e2guardian_groups['freq'], $matches)) {
				$new_cron['item'][] = array(
					"minute" => "0",
					"hour" => "*/". $matches[1],
					"mday" => "*",
					"month" => "*",
					"wday" => "*",
					"who" => "root",
					"command" => "{$cron_cmd} $e2_group_source '{$e2guardian_groups[$e2_group_source]}'");
				$config['cron'] = $new_cron;
				$cron_found++;
			}
		}
	}
	// write files
	conf_mount_rw();
	write_config();

	// update cron
	if ($cron_found > 0) {
		$config['cron'] = $new_cron;
		write_config();
		configure_cron();
	}

	$dirs = array(
		E2GUARDIAN_DIR . '/etc/e2guardian/lists/bannedrooms/',
		'/var/log/e2guardian');

	foreach ($dirs as $dir) {
		if (!is_dir($dir)) {
			mkdir ($dir, 0755, true);
		}
	}

	// update file owner
	mwexec("chown -R $daemonuser:$daemongroup " . E2GUARDIAN_DIR . "/etc/e2guardian");
	mwexec("chown -R $daemonuser:$daemongroup /var/log/e2guardian");

	// create config files
	file_put_contents($e2guardian_dir . "/e2guardian.conf", $dg, LOCK_EX);

	// check virus_scanner options
	$libexec_dir = E2GUARDIAN_DIR . "/libexec/e2guardian/";
	if ($install_process == true) {
		update_output_window("Skipping clamav check during package install.");
	}
	if (preg_match("/clamd/", $e2guardian_config['content_scanners']) && $install_process == false) {
		if (!(file_exists('/var/db/clamav/main.cvd') || file_exists('/var/db/clamav/main.cld'))) {
			file_notice("E2guardian - No antivirus database found for clamav, running freshclam in background.", "");
			log_error('No antivirus database found for clamav, running freshclam in background. Content-scanner may not work until freshclam finishes.');
			mwexec_bg(E2GUARDIAN_DIR . '/bin/freshclam && /usr/local/etc/rc.d/clamav-clamd');
		}
		// clamdscan.conf e2guardian file
		$cconf = E2GUARDIAN_DIR . "/etc/e2guardian/contentscanners/clamdscan.conf";
		$cconf_file = file_get_contents($cconf);
		if (preg_match('/#clamdudsfile/', $cconf_file)) {
			$cconf_file = preg_replace('/#clamdudsfile/', 'clamdudsfile', $cconf_file);
			file_put_contents($cconf, $cconf_file, LOCK_EX);
		}
		// clamd conf file
		$cconf = E2GUARDIAN_DIR . "/etc/clamd.conf";
		$cconf_file = file_get_contents($cconf);
		if (preg_match("/User (\w+)/", $cconf_file, $matches)) {
			// clamd script file
			$script = '/usr/local/etc/rc.d/clamav-clamd';
			$script_file = file($script);
			$new_clamav_startup = "";
			$cpreg_m[0] = "@NO@";
			$cpreg_m[1] = "@/usr/local@";
			$cpreg_r[0] = "YES";
			$cpreg_r[1] = E2GUARDIAN_DIR;
			foreach ($script_file as $script_line) {
				if (preg_match("/command=/", $script_line)) {
					$new_clamav_startup .= 'if [ ! -d /var/run/clamav ];then /bin/mkdir /var/run/clamav;fi' . "\n";
					$new_clamav_startup .= 'if [ ! -d /var/db/clamav ];then /bin/mkdir /var/db/clamav;fi' . "\n";
					$new_clamav_startup .= 'if [ ! -d /var/log/clamav ];then /bin/mkdir -p /var/log/clamav;fi' . "\n";
					$new_clamav_startup .= "chown -R " . $matches[1] . " /var/run/clamav\n";
					$new_clamav_startup .= "chown -R " . $matches[1] . " /var/db/clamav\n";
					$new_clamav_startup .= "chown -R " . $matches[1] . " /var/log/clamav\n";
					$new_clamav_startup .= $script_line;
				} elseif (!preg_match("/(mkdir|chown|sleep|mailscanner)/", $script_line)) {
					$new_clamav_startup .= preg_replace($cpreg_m, $cpreg_r, $script_line);
				}
			}
			file_put_contents($script, $new_clamav_startup, LOCK_EX);
			chmod ($script, 0755);

			if (file_exists('/var/run/e2guardian.pid') && is_process_running('clamd')) {
				log_error('Stopping clamav-clamd');
				mwexec("$script stop");
			}
			unlink_if_exists("/tmp/.dguardianipc");
			unlink_if_exists("/tmp/.dguardianurlipc");
			if (!is_process_running('clamd')) {
				log_error('Starting clamav-clamd');
				mwexec_bg("$script start");
			}
		}
	}
	// mark config changes
	touch(E2GUARDIAN_DIRTYPATH);

}
function e2guardian_check_config() {
	global $savemsg, $config;

	if (!isset($config['installedpackages']['e2guardian']['config'][0]['enable'])) {
		return 1;
	}

	if (isset($_REQUEST['apply'])) {
		e2guardian_start("no", false, true);
		return;
	}

	if (file_exists(E2GUARDIAN_DIRTYPATH)) {
		$savemsg = "E2guardian configuration has been changed.<br/>You must apply in order to take effect.<br/>";
	}
	return 0;
}

function e2guardian_start($via_rpc = "no", $install_process = false, $force_start = false) {
	global $g, $config, $savemsg;

	unlink_if_exists(E2GUARDIAN_DIRTYPATH);

	if (!$force_start) {
		if (e2guardian_check_config() <> 0) {
			return;
		}
	}

	$script = '/usr/local/etc/rc.d/e2guardian.sh';
	unlink_if_exists('/usr/local/etc/rc.d/e2guardian');
	if ($config['installedpackages']['e2guardian']['config'][0]['enable'] == "on") {
		copy('/usr/local/pkg/e2guardian_rc.template', $script);
		chmod ($script, 0755);
		if (is_process_running('e2guardian')) {
			log_error('Reloading E2guardian');
			exec("/usr/local/sbin/e2guardian -r");
		} else {
			log_error('Starting E2guardian');
			mwexec("$script start");
		}
	} else {
		if (is_process_running('e2guardian')) {
			log_error('E2guardian is disabled, stopping process...');
			mwexec("$script stop");
		}
		if (file_exists($script)) {
			chmod ($script, 444);
		}
	}

	if (!file_exists(E2GUARDIAN_DIR . '/etc/e2guardian/lists/phraselists/pornography/weighted_russian_utf8')) {
		file_put_contents(E2GUARDIAN_DIR . '/etc/e2guardian/lists/phraselists/pornography/weighted_russian_utf8', "", LOCK_EX);
	}

	// check ca certs hashes
	check_ca_hashes();

	// avoid sync during boot process
	if (!platform_booting() || $via_rpc == "yes") {
		/* Uses XMLRPC to synchronize the changes to a remote node */
		if (is_array($config['installedpackages']['e2guardiansync']['config'])) {
			$e2_sync = $config['installedpackages']['e2guardiansync']['config'][0];
			$synconchanges = $e2_sync['synconchanges'];
			$synctimeout = $e2_sync['synctimeout'];
			switch ($synconchanges) {
				case "manual":
					if (is_array($e2_sync[row])) {
						$rs = $e2_sync[row];
					} else {
						log_error("[E2guardian] xmlrpc sync is enabled but there are no hosts to push on e2guardian config.");
						return;
					}
					break;
				case "auto":
					if (is_array($config['installedpackages']['carpsettings']) && is_array($config['installedpackages']['carpsettings']['config'])) {
						$system_carp = $config['installedpackages']['carpsettings']['config'][0];
						$rs[0]['ipaddress'] = $system_carp['synchronizetoip'];
						$rs[0]['username'] = $system_carp['username'];
						$rs[0]['password'] = $system_carp['password'];
						if (!is_ipaddr($system_carp['synchronizetoip'])) {
							log_error("[E2guardian] xmlrpc sync is enabled but there are no system backup hosts to push squid config.");
							return;
						}
					} else {
						log_error("[E2guardian] xmlrpc sync is enabled but there are no system backup hosts to push squid config.");
						return;
					}
					break;
				default:
					return;
				break;
			}
			if (is_array($rs)) {
				log_error("[E2guardian] xmlrpc sync is starting.");
				foreach ($rs as $sh) {
					$sync_to_ip = $sh['ipaddress'];
					$password = $sh['password'];
					$username = ($sh['username']?$sh['username']:"admin");
					if ($password && $sync_to_ip) {
						e2guardian_do_xmlrpc_sync($sync_to_ip, $username, $password, $sync_type, $synctimeout);
					}
				}
				log_error("[E2guardian] xmlrpc sync is ending.");
			}
		}
	}
}

function e2guardian_validate_input($post, &$input_errors) {
	foreach ($post as $key => $value) {
		if (preg_match("/^(name|description)$/", $key) && $value == "") {
			$input_errors[] = "{$key} could not be empty.";
		} else if ($key == "name" && $value == "sample") {
			$input_errors[] = "{$value} cannot be used as name.";
		} else if ($key == "name" && preg_match("/\W/", $value)) {
			$input_errors[] = "{$value} cannot be used as name. Use only a-z 0-9 characters";
		} else if ($key == "group_options") {
			$acls = array("pics", "phrase", "site", "url", "extension", "header", "content", "search");
			foreach ($acls as $gacl) {
				if (!array_key_exists($gacl . "acl", $post)) {
					$input_errors[] = ucfirst($gacl) . " must has at least one acl assigned.";
				}
			}
		} else if (empty($value)) {
			continue;
		} else if ($key == "freq" && (!preg_match("/^\d+(h|m|d)$/", $value) || $value == 0)) {
			$input_errors[] = "A valid number with a time reference is required for the field 'Update Frequency'";
		}
	}
}

function e2guardian_php_install_command() {
	sync_package_e2guardian("no", true);
}

function e2guardian_php_deinstall_command() {
	global $config, $g;
	if (is_process_running('e2guardian')) {
		log_error("stopping e2guardian..");
		mwexec("/usr/local/etc/rc.d/e2guardian.sh stop");
		sleep(1);
	}

	if (file_exists("/usr/local/etc/rc.d/e2guardian.sh")) {
		conf_mount_rw();
		chmod ("/usr/local/etc/rc.d/e2guardian.sh", 0444);
		conf_mount_ro();
	}
}

function e2guardian_do_xmlrpc_sync($sync_to_ip, $username, $password, $sync_type, $synctimeout) {
	global $config, $g;

	if (!$username) {
		return;
	}

	if (!$password) {
		return;
	}

	if (!$sync_to_ip) {
		return;
	}

	if (!$synctimeout) {
		$synctimeout = 30;
	}
	$xmlrpc_sync_neighbor = $sync_to_ip;
	if ($config['system']['webgui']['protocol'] != "") {
		$synchronizetoip = $config['system']['webgui']['protocol'];
		$synchronizetoip .= "://";
	}
	$port = $config['system']['webgui']['port'];
	/* if port is empty lets rely on the protocol selection */
	if ($port == "") {
		if ($config['system']['webgui']['protocol'] == "http") {
			$port = "80";
		} else {
			$port = "443";
		}
	}
	$synchronizetoip .= $sync_to_ip;

	/* xml will hold the sections to sync */
	$xml = array();
	log_error("Include e2guardian config");
	$xml['e2guardian'] = $config['installedpackages']['e2guardian'];
	$xml['e2guardianantivirusacl'] = $config['installedpackages']['e2guardianantivirusacl'];
	$xml['e2guardianconfig'] = $config['installedpackages']['e2guardianconfig'];
	$xml['e2guardianblacklist'] = $config['installedpackages']['e2guardianblacklist'];
	$xml['e2guardianldap'] = $config['installedpackages']['e2guardianldap'];
	$xml['e2guardiancontentacl'] = $config['installedpackages']['e2guardiancontentacl'];
	$xml['e2guardianfileacl'] = $config['installedpackages']['e2guardianfileacl'];
	$xml['e2guardiangroups'] = $config['installedpackages']['e2guardiangroups'];
	$xml['e2guardianheaderacl'] = $config['installedpackages']['e2guardianheaderacl'];
	$xml['e2guardianlimits'] = $config['installedpackages']['e2guardianlimits'];
	$xml['e2guardianlog'] = $config['installedpackages']['e2guardianlog'];
	$xml['e2guardianphraseacl'] = $config['installedpackages']['e2guardianphraseacl'];
	$xml['e2guardianpicsacl'] = $config['installedpackages']['e2guardianpicsacl'];
	$xml['e2guardiansearchacl'] = $config['installedpackages']['e2guardiansearchacl'];
	$xml['e2guardiansiteacl'] = $config['installedpackages']['e2guardiansiteacl'];
	$xml['e2guardianurlacl'] = $config['installedpackages']['e2guardianurlacl'];
	$xml['e2guardianusers'] = $config['installedpackages']['e2guardianusers'];
	$xml['e2guardianips'] = $config['installedpackages']['e2guardianips'];

	if (count($xml) > 0) {
		/* assemble XMLRPC payload */
		$params = array(
			XML_RPC_encode($password),
			XML_RPC_encode($xml)
		);

		/* set a few variables needed for sync code borrowed from filter.inc */
		$url = $synchronizetoip;
		log_error("Beginning e2guardian XMLRPC sync to {$url}:{$port}.");
		$method = 'pfsense.merge_installedpackages_section_xmlrpc';
		$msg = new XML_RPC_Message($method, $params);
		$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
		$cli->setCredentials($username, $password);
		if ($g['debug']) {
			$cli->setDebug(1);
		}
		/* send our XMLRPC message and timeout after $synctimeout seconds */
		$resp = $cli->send($msg, $synctimeout);
		if (!$resp) {
			$error = "A communications error occurred while attempting e2guardian XMLRPC sync with {$url}:{$port}.";
			log_error($error);
			file_notice("sync_settings", $error, "e2guardian Settings Sync", "");
		} elseif ($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, $synctimeout);
			$error = "An error code was received while attempting e2guardian XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "e2guardian Settings Sync", "");
		} else {
			log_error("e2guardian XMLRPC sync successfully completed with {$url}:{$port}.");
		}

		/* tell e2guardian to reload our settings on the destination sync host. */
		$method = 'pfsense.exec_php';
		$execcmd = "require_once('/usr/local/pkg/e2guardian.inc');\n";
		$execcmd .= "sync_package_e2guardian('yes');";

		/* assemble XMLRPC payload */
		$params = array(
			XML_RPC_encode($password),
			XML_RPC_encode($execcmd)
		);

		log_error("e2guardian XMLRPC reload data {$url}:{$port}.");
		$msg = new XML_RPC_Message($method, $params);
		$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
		$cli->setCredentials($username, $password);
		$resp = $cli->send($msg, $synctimeout);
		if (!$resp) {
			$error = "A communications error occurred while attempting e2guardian XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
			log_error($error);
			file_notice("sync_settings", $error, "e2guardian Settings Sync", "");
		} elseif ($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, $synctimeout);
			$error = "An error code was received while attempting e2guardian XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "e2guardian Settings Sync", "");
		} else {
			log_error("e2guardian XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
		}
	}
}

?>
