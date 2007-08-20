<?php

function dashboard_install() {
	global $g, $config;
	assign_privs();
	if(!file_exists("/usr/local/www/index.php.before_dashboard")) {
		/* backup the pre-dashboard index.php file */
		mwexec("cp /usr/local/www/index.php /usr/local/www/index.php.before_dashboard");
		/* backup the pre-dashboard fbegin.inc file */
		mwexec("cp /usr/local/www/fbegin.inc /usr/local/www/fbegin.inc.before_dashboard");
	}
	mwexec("tar xzvpf /usr/local/pkg/widgets.tgz -C /usr/local/www/");
	assign_privs();
}

function dashboard_deinstall() {
	global $g, $config;
	assign_privs();
	if(file_exists("/usr/local/www/index.php.before_dashboard")) {
		/* restore the files prior to the dashboard package installation */
		mwexec("cp /usr/local/www/index.php.before_dashboard /usr/local/www/index.php");
		mwexec("cp /usr/local/www/fbegin.inc.before_dashboard /usr/local/www/fbegin.inc");
	}
	assign_privs();
}

function assign_privs() {
	mwexec("chmod a+rw /usr/local/www/index.php");
	mwexec("chmod a+rw /usr/local/www/fbegin.php");
	mwexec("chmod a+rw /usr/local/www/graph_cpu.php");
	mwexec("chmod a+rw /usr/local/www/stats.php");
}

?>