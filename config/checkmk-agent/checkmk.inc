<?php
/*
	checkmk.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2013 Marcello Coutinho
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
require_once("filter.inc");
require_once("globals.inc");
require_once("interfaces.inc");
require_once("notices.inc");
require_once("pfsense-utils.inc");
require_once("util.inc");
require_once("xmlrpc.inc");
require_once("xmlrpc_client.inc");

define('ETC_SERVICES', '/etc/services');

function checkmk_install() {
	/*
	Download last STABLE check_mk version (1.2.5i7) from upstream git repository.
	IMPORTANT NOTE: Newer versions require bash instead of sh!
	*/
	$checkmk_bin = "/usr/local/bin/check_mk_agent";
	$checkmk_url = 'http://git.mathias-kettner.de/git/?p=check_mk.git;a=blob_plain;f=agents/check_mk_agent.freebsd;hb=e13899bde8bdafe13780427811c8153c59be807f';
	mwexec("/usr/bin/fetch -o {$checkmk_bin} \"{$checkmk_url}\"");
	chmod($checkmk_bin, 0755);

	/* Detect possible junk left over after previous bad package versions */
	checkmk_decrapify();
}

function checkmk_deinstall() {
	/* Remove entry from /etc/services file */
	checkmk_cleanup_etc_services_file();
	/* Remove check_mk_agent script fetched via checkmk_install() */
	unlink_if_exists("/usr/local/bin/check_mk_agent");
	/* Detect possible junk left over after previous bad package versions */
	checkmk_decrapify();
}

function checkmk_decrapify() {
	$i = 0;
	if (exec("/usr/bin/wc -l /etc/hosts.allow | /usr/bin/awk '{ print $1 }'") > 5) {
		log_error("[check_mk-agent] Possibly redundant lines found in /etc/hosts.allow.");
		$i++;
	}
	if (exec("/usr/bin/wc -l /etc/inetd.conf | /usr/bin/awk '{ print $1 }'") > 1) {
		log_error("[check_mk-agent] Possibly redundant lines found in /etc/inetd.conf.");
		$i++;
	}
	if (file_exists("/etc/rc.conf.local")) {
		log_error("[check_mk-agent] /etc/rc.conf.local file found; this file does not exist normally on pfSense.");
		$i++;
	}
	if ($i > 0) {
		log_error("[check_mk-agent] Inconsistent configuration files; possibly caused by previous check_mk package versions.");
		log_error("[check_mk-agent] Please, compare those against default distribution files at https://github.com/pfsense/pfsense and fix as required manually.");
		file_notice("check_mk-agent", "Inconsistent configuration files found, possibly caused by previous check_mk package versions. See Status - System Logs - General for details.", "Packages", "");
	}
}

function checkmk_text_area_decode($text) {
	return preg_replace('/\r\n/', "\n", base64_decode($text));
}

function checkmk_cleanup_etc_services_file() {
	preg_match_all("/check_mk.*/", file_get_contents(ETC_SERVICES), $matches);
	foreach ($matches[0] as $match => $value) {
		if (!empty($value)) {
			remove_text_from_file(ETC_SERVICES, "{$value}\n");
		}
	}
}

function sync_package_checkmk() {
	global $config, $g, $mk_config;

	if (!is_array($config['installedpackages']['checkmk']['config'])) {
		return;
	}
	$mk_config = $config['installedpackages']['checkmk']['config'][0];

	$checkmk_bin = "/usr/local/bin/check_mk_agent";
	$checkmk_url = 'http://git.mathias-kettner.de/git/?p=check_mk.git;a=blob_plain;f=agents/check_mk_agent.freebsd;hb=e13899bde8bdafe13780427811c8153c59be807f';
	if (!file_exists($checkmk_bin) && $mk_config['checkmkenable'] == "on") {
		$error = "ERROR: check_mk-agent binary file not found.";
		$error .= " You can manually download it using this cmd: fetch -o {$checkmk_bin} \"{$checkmk_url}\"";
		log_error($error);
		file_notice("check_mk-agent", $error, "checkmk save config", "");
		return;
	}

	conf_mount_rw();

	/* Check /etc/services file; remove any previous entries first since port could have changed */
	checkmk_cleanup_etc_services_file();
	$port = ($mk_config['checkmkport'] ? $mk_config['checkmkport'] : "6556");
	if ($mk_config['checkmkenable'] == "on") {
		$mk_service_file = "check_mk	{$port}/tcp   #check_mk agent\n";
		add_text_to_file(ETC_SERVICES, $mk_service_file);
	}

	conf_mount_ro();

	/* Run XMLRPC sync if not booting */
	if (function_exists("platform_booting")) {
		if (platform_booting()) {
			return;
		}
	} elseif ($g['booting']) {
		return;
	} else {
		checkmk_sync_on_changes();
	}
}

function checkmk_generate_rules($type) {
	global $config;

	if (is_array($config['installedpackages']['checkmk']['config'])) {
		$mk_config = $config['installedpackages']['checkmk']['config'][0];
	} else {
		$mk_config = array();
	}
	$mk_config = $config['installedpackages']['checkmk']['config'][0];
	if ($mk_config['checkmkenable'] != "on") {
		return;
	}

	if ($type != "nat") {
		return;
	}

	/* Add checkmk daemon to inetd */
	$inetd_fd = fopen("/var/etc/inetd.conf", "a+");
	fwrite($inetd_fd, "check_mk\t\tstream\ttcp\tnowait\t\troot\t/usr/local/bin/check_mk_agent\tcheck_mk \n");
	fclose($inetd_fd);

	/* Generate NAT rules */
	if (!empty($mk_config['checkmkifaces'])) {
		$checkmkifs = explode(",", $mk_config['checkmkifaces']);
		$checkmkhosts = $mk_config['checkmkhosts'] ?: "any";
		$checkmkport = $mk_config['checkmkport'] ?: "6556";
		foreach ($checkmkifs as $checkmkif) {
			if (empty($checkmkif)) {
				continue;
			}
			$interface = get_real_interface($checkmkif);
			if (empty($interface)) {
				continue;
			}
			$ip = find_interface_ip($interface);
			if (!is_ipaddrv4($ip)) {
				continue;
			}
		
			if (is_subnetv4($checkmkhosts) || is_ipaddr($checkmkhosts) || $checkmkhosts == "any") {
				$natrules .= "rdr on {$interface} proto tcp from {$checkmkhosts} to {$ip} port {$checkmkport} -> 127.0.0.1 port {$checkmkport}\n";
			} elseif (is_alias($checkmkhosts)) {
				$natrules .= "rdr on {$interface} proto tcp from \${$checkmkhosts} to {$ip} port {$checkmkport} -> 127.0.0.1 port {$checkmkport}\n";
			}
		}
	}
	return $natrules;
}

function checkmk_validate_input($post, &$input_errors) {
	if (!empty($post["checkmkport"]) && !is_port($post["checkmkport"])) {
		$input_errors[] = "You must specify a valid port in 'Listen Port' field.";
		
	}
	if (empty($post["checkmkifaces"])) {
		$input_errors[] = "One or more 'Listen Interface(s)' must be selected";
	}
	if (!empty($post["checkmkhosts"]) && !(is_alias($post["checkmkhosts"]) || is_subnetv4($post["checkmkhosts"]) || is_ipaddrv4($post["checkmkhosts"]))) {
		$input_errors[] = "You must specify a valid IP address, subnet or alias in 'Hosts Allowed' field.";
	}
}

/* Uses XMLRPC to synchronize the changes to a remote node. */
function checkmk_sync_on_changes() {
	global $config;

	if (is_array($config['installedpackages']['checkmksync']['config'])) {
		$checkmk_sync = $config['installedpackages']['checkmksync']['config'][0];
		$synconchanges = $checkmk_sync['synconchanges'];
		$synctimeout = $checkmk_sync['synctimeout'] ?: '250';
		switch ($synconchanges) {
			case "manual":
				if (is_array($checkmk_sync['row'])) {
					$rs = $checkmk_sync['row'];
				} else {
					log_error("[check_mk-agent] XMLRPC sync is enabled but there are no hosts configured as replication targets.");
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
						log_error("[check_mk-agent] XMLRPC CARP/HA sync is enabled but there are no system backup hosts configured as replication targets.");
						return;
					} else {
						$rs[0]['syncdestinenable'] = TRUE;
					}
				} else {
					log_error("[check_mk-agent] XMLRPC CARP/HA sync is enabled but there are no system backup hosts configured as replication targets.");
					return;
				}
				break;
			default:
				return;
				break;
		}
		if (is_array($rs)) {
			log_error("[check_mk-agent] XMLRPC sync is starting.");
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
						checkmk_do_xmlrpc_sync($sync_to_ip, $port, $protocol, $username, $password, $synctimeout);
					} else {
						log_error("[check_mk-agent] XMLRPC sync with '{$sync_to_ip}' aborted due to the following error(s): {$error}");
					}
				}
			}
			log_error("[check_mk-agent] XMLRPC sync completed.");
		}
 	}
}

/* Do the actual XMLRPC sync. */
function checkmk_do_xmlrpc_sync($sync_to_ip, $port, $protocol, $username, $password, $synctimeout) {
	global $config, $g;

	if ($username == "" || $password == "" || $sync_to_ip == "" || $port == "" || $protocol == "") {
		log_error("[check_mk-agent] A required XMLRPC sync parameter (username, password, replication target, port or protocol) is empty ... aborting pkg sync");
		return;
	}

	// Take care of IPv6 literal address
	if (is_ipaddrv6($sync_to_ip)) {
		$sync_to_ip = "[{$sync_to_ip}]";
	}

	$url = "{$protocol}://{$sync_to_ip}";

	/* XML will hold the sections to sync. */
	$xml = array();
	$xml['checkmk'] = $config['installedpackages']['checkmk'];
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
		log_error("[check_mk-agent] {$error}");
		file_notice("sync_settings", $error, "checkmk Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("[check_mk-agent] {$error}");
		file_notice("sync_settings", $error, "checkmk Settings Sync", "");
	} else {
		log_error("[check_mk-agent] XMLRPC sync successfully completed with {$url}:{$port}.");
	}

	/* Tell check_mk to reload our settings on the destination sync host. */
	$method = 'pfsense.exec_php';
	$execcmd = "require_once('/usr/local/pkg/checkmk.inc');\n";
	$execcmd .= "sync_package_checkmk();";
	/* Assemble XMLRPC payload. */
	$params = array(XML_RPC_encode($password), XML_RPC_encode($execcmd));

	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, $synctimeout);
	if (!$resp) {
		$error = "A communications error occurred while attempting XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error("[check_mk-agent] {$error}");
		file_notice("sync_settings", $error, "checkmk Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("[check_mk-agent] {$error}");
		file_notice("sync_settings", $error, "checkmk Settings Sync", "");
	} else {
		log_error("[check_mk-agent] XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
}

?>
