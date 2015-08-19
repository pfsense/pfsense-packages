<?php
/*
	bacula-client.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2012 Marcio Carlos Braga Antao
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
require_once("config.inc");
require_once("util.inc");

$pf_version = substr(trim(file_get_contents("/etc/version")), 0, 3);
if ($pf_version == "2.1" || $pf_version == "2.2") {
	define('BACULA_LOCALBASE', '/usr/pbi/bacula-' . php_uname("m"));
} else {
	define('BACULA_LOCALBASE','/usr/local');
}
define('BACULA_STARTUP_SCRIPT', '/usr/local/etc/rc.d/bacula-fd.sh');

function baculaclient_custom_php_install_command() {
	baculaclient_custom_php_write_config();
}

function baculaclient_custom_php_deinstall_command(){
	conf_mount_rw();

	// Delete our config file
	unlink_if_exists(BACULA_LOCALBASE . "/etc/bacula/bacula-fd.conf");
	// Stop service and delete our rc file
	stop_service("bacula-client");
	unlink_if_exists(BACULA_STARTUP_SCRIPT);

	conf_mount_ro();
}

function baculaclient_custom_php_write_config(){
	global $config, $LocalDirector;
	conf_mount_rw();

	// Check config_file
	$startup_file = BACULA_LOCALBASE . "/etc/rc.d/bacula-fd";
	if (file_exists($startup_file)) {
		$startup_script = file_get_contents($startup_file);
		$startup_script = preg_replace("/NO/","YES", $startup_script);
		$startup_script = preg_replace("@/usr/local/etc/bacula-fd.conf@", BACULA_LOCALBASE . "/etc/bacula/bacula-fd.conf", $startup_script);
		$startup_script = preg_replace("@/usr/local/etc/bacula/bacula-fd.conf@", BACULA_LOCALBASE . "/etc/bacula/bacula-fd.conf", $startup_script);
		file_put_contents(BACULA_STARTUP_SCRIPT, $startup_script, LOCK_EX);
		// Ensure bacula-fd is executable
		chmod(BACULA_STARTUP_SCRIPT, 0755);
	}

	// Check config
	if (is_array($config['installedpackages']['baculaclient']['config'])) {
		$baculaclient_conf="";
		foreach ($config['installedpackages']['baculaclient']['config'] as $bc) {
			// Create Director
			switch ($bc['type']) {
				case "Director":
					$baculaclient_conf .= "Director { \n\tName = {$bc['director']}-dir #{$bc['description']}\n\tPassword = \"{$bc['password']}\"\n}\n";
					break;
				case "Monitor":
					$baculaclient_conf .= "Director { \n\tName = {$bc['director']}-mon #{$bc['description']}\n\tPassword = \"{$bc['password']}\"\n\tMonitor = yes\n}\n";
					break;
				case "Local":
					$baculaclient_conf .= "Director { \n\tName = {$bc['director']}-dir #{$bc['description']}\n\tPassword = \"{$bc['password']}\"\n}\n";
					$baculaclient_conf .= "Director { \n\tName = {$bc['director']}-mon #{$bc['description']}\n\tPassword = \"{$bc['password']}\"\n\tMonitor = yes\n}\n";
					$LocalDirector = $bc['director'];
			}

		}

		// Create Messages
		if (!empty($LocalDirector)) {
			$baculaclient_conf .= "Messages { \n\tName = Standard \n\tdirector  = {$LocalDirector}-dir = all, !skipped, !restored\n}\n";
		}
		// Create FileDaemon
		if (is_array($config['installedpackages']['baculaclientfd']['config'])) {
			$port = $config['installedpackages']['baculaclientfd']['config'][0]['port'] ?: '9102';
			$jobs = $config['installedpackages']['baculaclientfd']['config'][0]['jobs'] ?: '20';
		}
		if (!empty($LocalDirector)) {
			$baculaclient_conf .= "FileDaemon { \n\tName = {$LocalDirector}-fd #\n\tFDport = {$port}\n\tWorkingDirectory = /var/db/bacula\n\tPid Directory = /var/run\n\tMaximum Concurrent Jobs = {$jobs}\n}\n";
		}

		// Write config file and start service
		file_put_contents(BACULA_LOCALBASE . "/etc/bacula/bacula-fd.conf", $baculaclient_conf, LOCK_EX);
		if (!empty($LocalDirector)) {
			restart_service("bacula-client");
		} else {
			log_error("[bacula-client] You need at least one local Director in order to run bacula-client.");
		}

		conf_mount_ro();
	}
}
?>
