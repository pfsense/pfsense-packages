<?php

function rate_install() {
	global $g, $config;
	conf_mount_rw();
	/* Copy files for backup. */
	mwexec("cp /usr/local/www/status_graph.php /usr/local/www/status_graph.php.orig");
	unlink_if_exists("/usr/local/www/status_graph.php");
	mwexec("cp /usr/local/pkg/bandwidth_by_ip.php /usr/local/www/bandwidth_by_ip.php");
	mwexec("cp /usr/local/pkg/status_graph.php /usr/local/www/status_graph.php");

	rate_assign_privs();
	/* Clear the APC cache so that the updates to installed files will work. */
	apc_clear_cache();
	conf_mount_ro();	
}

function rate_deinstall() {
	conf_mount_rw();
	unlink_if_exists("/usr/local/www/status_graph.php");
	unlink_if_exists("/usr/local/www/bandwidth_by_ip.php");
	mwexec("cp /usr/local/www/status_graph.php.orig /usr/local/www/status_graph.php");
	conf_mount_ro();	
}

function rate_assign_privs() {
	conf_mount_rw();
	/* Fix permissions on replaced files */
	mwexec("chown root:wheel /usr/local/www/bandwidth_by_ip.php");
	mwexec("chown root:wheel /usr/local/www/status_graph.php");
	conf_mount_ro();	
}

?>