<?php
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
