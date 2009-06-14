<?php

function rate_install() {
	global $g, $config;

	/* Copy files for backup. */
	mwexec("cp /usr/local/www/status_graph.php /usr/local/www/status_graph.php.orig");
	unlink("/usr/local/www/status_graph.php");
	mwexec("cp /usr/local/pkg/bandwidth_by_ip.php /usr/local/www/bandwidth_by_ip.php");
	mwexec("cp /usr/local/pkg/status_graph.php /usr/local/www/status_graph.php");

	assign_privs();
	/* Clear the APC cache so that the updates to installed files will work. */
	apc_clear_cache();
}

function rate_deinstall() {
	unlink("/usr/local/www/status_graph.php");
	unlink("/usr/local/www/bandwidth_by_ip.php");
	mwexec("cp /usr/local/www/status_graph.php.orig /usr/local/www/status_graph.php");
}

function assign_privs() {
	/* Fix permissions on replaced files */
	mwexec("chown root:wheel /usr/local/www/bandwidth_by_ip.php");
	mwexec("chown root:wheel /usr/local/www/status_graph.php");
}

?>