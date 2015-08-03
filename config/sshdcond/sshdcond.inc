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
	global $g, $config;

	conf_mount_rw();

	/* We need to generate an outfile for our extra commands.
	The patched g_szSSHDFileGenerate php file then reads and appends that config.
	*/
	$fd = fopen("/etc/ssh/sshd_extra", 'w');
	fclose($fd);

	conf_mount_ro();
}

function sshdcond_custom_php_deinstall_command() {
	global $g, $config;

	conf_mount_rw();

	/* Delete our config file. */
	unlink_if_exists("/etc/ssh/sshd_extra");

	/* Re-run sshd config generation script. */
	restart_sshd();

	conf_mount_ro();
}

function sshdcond_custom_php_write_config() {
	global $g, $config, $pkg_interface;

	/* Detect boot process, do nothing during boot. */
	if (platform_booting()) {
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
	global $config, $g;

	if (is_array($config['installedpackages']['sshdcondsync'])) {
		if (!$config['installedpackages']['sshdcondsync']['config'][0]['synconchanges']) {
			return;
		}
	}

	log_error("[sshdcond] xmlrpc sync is starting.");
	foreach ($config['installedpackages']['sshdcondsync']['config'] as $rs) {
		foreach($rs['row'] as $sh) {
			$sync_to_ip = $sh['ipaddress'];
			$password = $sh['password'];
			if ($password && $sync_to_ip) {
				sshdcond_do_xmlrpc_sync($sync_to_ip, $password);
			}
		}
	}
	log_error("[sshdcond] xmlrpc sync is ending.");
}

/* Do the actual XMLRPC sync. */
function sshdcond_do_xmlrpc_sync($sync_to_ip, $password) {
	global $config, $g;

	if (!$password) {
		return;
	}

	if (!$sync_to_ip) {
		return;
	}

	$username='admin';
	$xmlrpc_sync_neighbor = $sync_to_ip;
	if ($config['system']['webgui']['protocol'] != "") {
		$synchronizetoip = $config['system']['webgui']['protocol'];
		$synchronizetoip .= "://";
	}
	$port = $config['system']['webgui']['port'];
	/* If port is empty, let's rely on the protocol selection. */
	if ($port == "") {
		if ($config['system']['webgui']['protocol'] == "http") {
			$port = "80";
		} else {
			$port = "443";
		}
	}
	$synchronizetoip .= $sync_to_ip;

	/* xml will hold the sections to sync. */
	$xml = array();
	$xml['sshdcond'] = $config['installedpackages']['sshdcond'];
	/* Assemble XMLRPC payload. */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* Set a few variables needed for sync code; borrowed from filter.inc. */
	$url = $synchronizetoip;
	log_error("Beginning sshdcond XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if ($g['debug']) {
		$cli->setDebug(1);
	}
	/* Send our XMLRPC message and timeout after 250 seconds. */
	$resp = $cli->send($msg, "250");
	if (!$resp) {
		$error = "A communications error occurred while attempting sshdcond XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "sshdcond Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting sshdcond XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "sshdcond Settings Sync", "");
	} else {
		log_error("sshdcond XMLRPC sync successfully completed with {$url}:{$port}.");
	}

	/* Tell sshdcond to reload our settings on the destination sync host. */
	$method = 'pfsense.exec_php';
	$execcmd = "require_once('/usr/local/pkg/sshdcond.inc');\n";
	$execcmd .= "sshdcond_custom_php_write_config();";
	/* Assemble XMLRPC payload. */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);

	log_error("sshdcond XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, "250");
	if (!$resp) {
		$error = "A communications error occurred while attempting sshdcond XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "sshdcond Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting sshdcond XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "sshdcond Settings Sync", "");
	} else {
		log_error("sshdcond XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
}
?>
