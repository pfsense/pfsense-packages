<?php

function dashboard_install() {
	global $g, $config;
	assign_privs();

	if (file_exists("/usr/local/www/fbegin.inc.before_dashboard"))
		mwexec("mv /usr/local/www/fbegin.inc.before_dashboard /usr/local/www/fbegin.inc");
	
	/* Copy, then rm the files for backup. For some odd reason, mv alone does
	 *  not always result in the proper file being used. */
	mwexec("cp /usr/local/www/index.php /usr/local/www/index.php.before_dashboard");
	mwexec("cp /usr/local/www/diag_logs_filter.php /usr/local/www/diag_logs_filter.php.before_dashboard");
	mwexec("cp /usr/local/www/diag_logs_filter_dynamic.php /usr/local/www/diag_logs_filter_dynamic.php.before_dashboard");

	mwexec("tar xzvpUf /usr/local/pkg/widgets.tgz -C /");
	assign_privs();
}

function assign_privs() {
	/* Fix permissions on replaced files */
	mwexec("chown root:wheel /usr/local/www/index.php");
	mwexec("chown root:wheel /usr/local/www/graph_cpu.php");
	mwexec("chown root:wheel /usr/local/www/stats.php");
	mwexec("chown root:wheel /usr/local/www/diag_logs_filter.php");
	mwexec("chown root:wheel /usr/local/www/diag_logs_filter_dynamic.php");
	mwexec("chmod ug+rw,o-w /usr/local/www/index.php");
	mwexec("chmod ug+rw,o-w /usr/local/www/graph_cpu.php");
	mwexec("chmod ug+rw,o-w /usr/local/www/stats.php");
	mwexec("chmod ug+rw,o-w /usr/local/www/diag_logs_filter.php");
	mwexec("chmod ug+rw,o-w /usr/local/www/diag_logs_filter_dynamic.php");
}

?>