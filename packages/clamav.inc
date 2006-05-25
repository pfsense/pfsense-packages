<?php
/*
	clamav.inc
        part of the ClamAV package for pfSense
	Copyright (C) 2006 Rajkumar S <raj@linuxense.com>
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
function sync_package_clamav() {
	conf_mount_rw();
	config_lock();
	global $config;
	$fd = fopen("/usr/local/etc/freshclam.conf","w");
	fwrite($fd, "DatabaseDirectory /var/db/clamav\n");
	fwrite($fd, "UpdateLogFile /var/log/clamav/freshclam.log\n");
	fwrite($fd, "DatabaseOwner clamav\n");
	fwrite($fd, "AllowSupplementaryGroups\n");
	fwrite($fd, "DatabaseMirror database.clamav.net\n");
	fclose($fd);
	$cronjob = "*/120   *       *       *       *       root            /usr/bin/nice -n20 /usr/local/bin/freshclam";
	add_text_to_file("/etc/crontab", $cronjob);
	conf_mount_ro();
	config_unlock();
	mwexec("killall -HUP cron");
}

function clamav_install_command() {
	global $config, $g;
	mwexec ("mkdir -p /var/db/clamav");
	mwexec ("chown clamav:clamav /var/db/clamav/");
	mwexec ("chown clamav:clamav /var/log/clamav/");
	update_output_window("Updating Virus Signatures...");
	mwexec ("/usr/local/bin/freshclam");
	sync_package_clamav();
}

function clamav_deinstall_command() {
	global $config, $g;
	conf_mount_rw();
	unlink_if_exists("/usr/local/etc/rc.d/clamav.sh");
	unlink_if_exists("/var/db/clamav/daily.cvd");
	unlink_if_exists("/var/db/clamav/main.cvd");
	unlink_if_exists("/var/db/clamav");
	$cronjob = "*/120   *       *       *       *       root            /usr/bin/nice -n20 /usr/local/bin/freshclam";
	#remove_text_to_file("/etc/crontab", $cronjob);
	conf_mount_ro();
}

?>
