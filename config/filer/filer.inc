<?php
/*
	filer.inc
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2010 Scott Ullrich <sullrich@gmail.com>
	Copyright (C) 2012 Marcello Coutinho
	Copyright (C) 2012 Brian Scholer
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
/* ========================================================================== */
function filer_text_area_decode($text) {
	return preg_replace('/\r\n/', "\n", base64_decode($text));
}

function sync_package_filer() {
	global $config, $g;
	$update_conf = 0;

	conf_mount_rw();

	/* Loop on configured files. */
	if ($config['installedpackages']['filer']['config'] != "") {
		$count = 0;
		foreach ($config['installedpackages']['filer']['config'] as $file) {
			if ($file['filedata'] == "" && file_exists($file['fullfile'])) {
				$config['installedpackages']['filer']['config'][$count]['filedata'] = base64_encode(file_get_contents($file['fullfile']));
				$file['filedata'] = base64_encode(file_get_contents($file['fullfile']));
				$update_conf++;
			}
			$count++;
			if (preg_match("/0?[0-7]{3}/", $file['mod'])) {
				$mod = octdec($file['mod']);
			} else {
				$mod = 0644;
			}

			/* Write file. */
			file_put_contents($file['fullfile'], filer_text_area_decode($file['filedata']), LOCK_EX);
			chmod($file['fullfile'], $mod);

			/* Check if there is a script to run after file save. */
			if ($file['cmd'] != "") {
				switch ($file['background']) {
					case "background":
						mwexec_bg($file['cmd']);
						break;
					case "foreground":
						mwexec($file['cmd']);
						break;
				}
			}
		}

		/* Write config if any file from filesystem was loaded. */
		if ($update_conf > 0) {
			write_config();
		}
	}

	conf_mount_ro();

	filer_sync_on_changes();
}

function filer_validate_input($post, &$input_errors) {
	foreach ($post as $key => $value) {
		if (empty($value)) {
			continue;
		}
		if (substr($key, 0, 3) == "mod" && !preg_match("/^0?[0-7]{3}$/", $value)) {
			$input_errors[] = "{$value} is not valid permissions mode number.";
		}
		if (substr($key, 0, 11) == "description" && !preg_match("@^[a-zA-Z0-9 _/.-]+$@", $value)) {
			$input_errors[] = "Do not use special characters in description.";
		}
		if (substr($key, 0, 8) == "fullfile" && !preg_match("@^[a-zA-Z0-9_/.-]+$@", $value)) {
			$input_errors[] = "Do not use special characters in filename.";
		}
	}
}

/* Uses XMLRPC to synchronize the changes to a remote node. */
function filer_sync_on_changes() {
	global $config, $g;

	log_error("[filer] filer_xmlrpc_sync.php is starting.");
	$synconchanges = $config['installedpackages']['filersync']['config'][0]['synconchanges'];
	if (!$synconchanges) {
		return;
	}
	foreach ($config['installedpackages']['filersync']['config'] as $rs) {
		foreach ($rs['row'] as $sh) {
			$sync_to_ip = $sh['ipaddress'];
			$password = $sh['password'];
			if ($sh['username']) {
				$username = $sh['username'];
			} else {
				$username = 'admin';
			}
			if ($password && $sync_to_ip) {
				filer_do_xmlrpc_sync($sync_to_ip, $username, $password);
			}
		}
	}
	log_error("[filer] filer_xmlrpc_sync.php is ending.");
}

/* Do the actual XMLRPC sync. */
function filer_do_xmlrpc_sync($sync_to_ip, $username, $password) {
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
	$xml['filer'] = $config['installedpackages']['filer'];

	/* Assemble XMLRPC payload. */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* Set a few variables needed for sync; code borrowed from filter.inc. */
	$url = $synchronizetoip;
	log_error("Beginning Filer XMLRPC sync to {$url}:{$port}.");
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
		$error = "A communications error occurred while attempting filer XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "filer Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting filer XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "filer Settings Sync", "");
	} else {
		log_error("filer XMLRPC sync successfully completed with {$url}:{$port}.");
	}

	/* Tell filer to reload our settings on the destination sync host. */
	$method = 'pfsense.exec_php';
	$execcmd = "require_once('/usr/local/pkg/filer.inc');\n";
	$execcmd .= "sync_package_filer();";
	/* Assemble XMLRPC payload. */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);

	log_error("filer XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, "250");
	if (!$resp) {
		$error = "A communications error occurred while attempting filer XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "filer Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting filer XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "filer Settings Sync", "");
	} else {
		log_error("filer XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
}

?>
