<?php
/*
	ntop.inc
	part of pfSense (http://www.pfSense.org/)
	Copyright (C) 2011-2013 Jim Pingle
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
require_once("pkg-utils.inc");

function ntop_php_install_command() {
	safe_mkdir("/var/db/ntop/rrd/graphics", 0755);
	mwexec("/bin/chmod -R 0755 /var/db/ntop");
	mwexec("/usr/sbin/chown -R nobody:nobody /var/db/ntop");
	mwexec("/bin/cp -Rp /usr/local/lib/X11/fonts/webfonts/ /usr/local/lib/X11/fonts/TTF/");
	ntop_upgrade_config();
}

function ntop_php_deinstall_command() {
	global $config;

	/* Wipe data and settings if the user does not wish to keep them */
	if (is_array($config['installedpackages']['ntop'])) {
		$ntop_config = $config['installedpackages']['ntop']['config'][0];
	} else {
		$ntop_config = array();
	}
	if ($ntop_config['keepdata'] != "on") {
		if (is_dir("/var/db/ntop/")) {
			mwexec("rm -rf /var/db/ntop/");
		}
		unset($config['installedpackages']['ntop']);
		write_config("[ntop] Removed package settings on uninstall.");
		log_error(gettext("[ntop] Removed package data and settings since 'Keep Data/Settings' is disabled."));
	}
}

function ntop_upgrade_config() {
	global $config;
	/* Fix flipped --no-interface-merge configuration meaning */
	if (is_array($config['installedpackages']['ntop'])) {
		if (isset($config['installedpackages']['ntop']['config'][0]['allowmerge'])) {
			$config['installedpackages']['ntop']['config'][0]['disallowmerge'] = "on";
			unset($config['installedpackages']['ntop']['config'][0]['allowmerge']);
		}
	}
}

function sync_package_ntop() {
	global $config, $g;

	if (is_array($config['installedpackages']['ntop'])) {
		$ntop_config = $config['installedpackages']['ntop']['config'][0];
	} else {
		$ntop_config = array();
	}
	conf_mount_rw();

	/* Just stop services and unlink rc script if disabled */
	if ($ntop_config['enable'] != "on") {
		ntop_stop_service();
		unlink_if_exists("/usr/local/etc/rc.d/ntop.sh");
		return;
	}
	
	/* Set up ntop interfaces */
	$ifaces_final = "";
	$first = 0;
	$mergeifs = ($ntop_config['disallowmerge'] == "on") ? "" : "-M";

	foreach ($ntop_config['interface_array'] as $iface) {
		$if = convert_friendly_interface_to_real_interface_name($iface);
		if ($if) {
			if ($first == 1) {
				$ifaces_final .= ",";
			}
			$ifaces_final .= $if;
			$first = 1;
		}
	}

	$start = "/usr/local/bin/ntop -i {$ifaces_final} -u root -d -4 {$mergeifs} -x 8102 -X 8192 &";
	write_rcfile(array("file" => "ntop.sh", "start" => $start, "stop" => "/usr/bin/killall ntop"));

	/* Set up admin password and (re)start services if not booting */
	if ((function_exists("platform_booting")) && (!platform_booting())) {
		ntop_stop_service();
		ntop_set_password();
		start_service("ntop");
	} elseif (!($g['booting'])) {
		ntop_stop_service();
		ntop_set_password();
		start_service("ntop");
	}
		
	conf_mount_ro();
}

function ntop_stop_service() {
	if (is_service_running("ntop")) {
		stop_service("ntop");
		// Wait for ntop to shut down cleanly.
		for ($i = 0; $i <= 10; $i++) {
			if (!is_process_running("ntop")) {
				break;
			}
			sleep(2);
		}
	}
}

function ntop_set_password() {
	global $config;

	if (is_array($config['installedpackages']['ntop'])) {
		$ntop_config = $config['installedpackages']['ntop']['config'][0];
	} else {
		$ntop_config = array();
	}
	$ntop_password = $ntop_config['password'] ?: "admin";
	unlink_if_exists("/var/db/ntop/ntop_pw.db");
	mwexec("/usr/local/bin/ntop --set-admin-password={$ntop_password}");
	sleep(2);
}

function ntop_validate_input($post, &$input_errors) {
	if (empty($post['password']) || empty($post['passwordagain'])) {
		$input_errors[] = "You must provide (and confirm) ntop's password.";
	}
	
	if ($post['password'] != $post['passwordagain']) {
		$input_errors[] = "The provided passwords did not match.";
	}
}

?>
