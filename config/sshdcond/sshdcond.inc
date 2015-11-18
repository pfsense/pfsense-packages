<?php
/*
	sshdcond.inc
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2012 Marcello Coutinho
	Copyright (C) 2012 Han Van (namezero@afim.info)
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

require_once("config.inc");
require_once("util.inc");

function restart_sshd() {
	mwexec_bg("/etc/sshd");
}

function sshdcond_custom_php_install_command() {
	/* We need to generate an outfile for our extra commands.
	The patched g_szSSHDFileGenerate php file then reads and appends that config.
	*/
	$fd = fopen("/etc/ssh/sshd_extra", 'w');
	fclose($fd);

}

function sshdcond_custom_php_deinstall_command() {
	/* Delete our config file. */
	unlink_if_exists("/etc/ssh/sshd_extra");

	/* Re-run sshd config generation script. */
	restart_sshd();

}

function sshdcond_custom_php_write_config() {
	global $g, $config;

	/* Detect boot process, do nothing during boot. */
	if (function_exists("platform_booting")) {
		if (platform_booting()) {
			return;
		}
	} elseif ($g['booting']) {
		return;
	}

	conf_mount_rw();

	$sshd_extra = "";
	if (is_array($config['installedpackages']['sshdcond']['config'])) {

		/* Read config. */
		foreach ($config['installedpackages']['sshdcond']['config'] as $sshdcond) {
			if ($sshdcond['enable'] && is_array($sshdcond['row'])) {
				$sshd_extra.= "Match {$sshdcond['matchtype']} {$sshdcond['matchvalue']}\n";
				foreach ($sshdcond['row'] as $sshd) {
					/* Check sshd value for spaces. */
					if (preg_match ("/\s+/",$sshd['sshdvalue'])) {
						$sshd['sshdvalue'] = '"' . $sshd['sshdvalue'] . '"';
					}

					/* Check that sshdvalue is not empty. */
					if ($sshd['sshdvalue'] != "") {
						$sshd_extra .= "\t {$sshd['sshdoption']} {$sshd['sshdvalue']}\n";
					}

					/* Apply file permission if option is ChrootDirectory. */
					if ($sshd['sshdoption'] == "ChrootDirectory" && file_exists($sshd['sshdvalue'])) {
						chown($sshd['sshdvalue'], 'root');
						chgrp($sshd['sshdvalue'], 'operator');
					}
				}
			}
		}
	}

	/* Save /etc/ssh/sshd_extra. */
	file_put_contents("/etc/ssh/sshd_extra", $sshd_extra, LOCK_EX);

	/* Restart sshd and re-mount read-only. */
	restart_sshd();
	conf_mount_ro();

	/* Sync config with other pfSense servers. */
	sshdcond_sync_on_changes();
}

/* Uses XMLRPC to synchronize the changes to a remote node. */
function sshdcond_sync_on_changes() {
	global $config;

	if (is_array($config['installedpackages']['sshdcondsync']['config'])) {
		$sshdcond_sync = $config['installedpackages']['sshdcondsync']['config'][0];
		$synconchanges = $sshdcond_sync['synconchanges'];
		$synctimeout = $sshdcond_sync['synctimeout'] ?: '150';
		switch ($synconchanges) {
			case "manual":
				if (is_array($sshdcond_sync['row'])) {
					$rs = $sshdcond_sync['row'];
				} else {
					log_error("[sshdcond] XMLRPC sync is enabled but there are no hosts configured as replication targets.");
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
						log_error("[sshdcond] XMLRPC CARP/HA sync is enabled but there are no system backup hosts configured as replication targets.");
						return;
					} else {
						$rs[0]['syncdestinenable'] = TRUE;
					}
				} else {
					log_error("[sshdcond] XMLRPC CARP/HA sync is enabled but there are no system backup hosts configured as replication targets.");
					return;
				}
				break;			
			default:
				return;
				break;
		}
		if (is_array($rs)) {
			log_error("[sshdcond] XMLRPC sync is starting.");
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
						sshdcond_do_xmlrpc_sync($sync_to_ip, $port, $protocol, $username, $password, $synctimeout);
					} else {
						log_error("[sshdcond] XMLRPC sync with '{$sync_to_ip}' aborted due to the following error(s): {$error}");
					}
				}
			}
			log_error("[sshdcond] XMLRPC sync completed.");
		}
 	}
}

/* Do the actual XMLRPC sync. */
function sshdcond_do_xmlrpc_sync($sync_to_ip, $port, $protocol, $username, $password, $synctimeout) {
	global $config, $g;

	if ($username == "" || $password == "" || $sync_to_ip == "" || $port == "" || $protocol == "") {
		log_error("[sshdcond] A required XMLRPC sync parameter (username, password, replication target, port or protocol) is empty ... aborting pkg sync");
		return;
	}

	// Take care of IPv6 literal address
	if (is_ipaddrv6($sync_to_ip)) {
		$sync_to_ip = "[{$sync_to_ip}]";
	}

	$url = "{$protocol}://{$sync_to_ip}";

	/* XML will hold the sections to sync. */
	$xml = array();
	$xml['sshdcond'] = $config['installedpackages']['sshdcond'];
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
		log_error("[sshdcond] {$error}");
		file_notice("sync_settings", $error, "sshdcond Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting sshdcond XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("[sshdcond] {$error}");
		file_notice("sync_settings", $error, "sshdcond Settings Sync", "");
	} else {
		log_error("[sshdcond] XMLRPC sync successfully completed with {$url}:{$port}.");
	}

	/* Tell sshdcond to reload our settings on the destination sync host. */
	$method = 'pfsense.exec_php';
	$execcmd = "require_once('/usr/local/pkg/sshdcond.inc');\n";
	$execcmd .= "sshdcond_custom_php_write_config();";
	/* Assemble XMLRPC payload. */
	$params = array(XML_RPC_encode($password), XML_RPC_encode($execcmd));

	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, $synctimeout);
	if (!$resp) {
		$error = "A communications error occurred while attempting sshdcond XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error("[sshdcond] {$error}");
		file_notice("sync_settings", $error, "sshdcond Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting sshdcond XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("[sshdcond] {$error}");
		file_notice("sync_settings", $error, "sshdcond Settings Sync", "");
	} else {
		log_error("[sshdcond] XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
}
?>
