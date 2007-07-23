<?php

function dashboard_install() {
	global $g, $config;
	if(!file_exists("/usr/local/www/index.php.before_dashboard")) {
		/* backup the pre-dashboard index.php file */
		mwexec("cp /usr/local/www/index.php /usr/local/www/index.php.before_dashboard");
		/* backup the pre-dashboard fbegin.inc file */
		mwexec("cp /usr/local/www/fbegin.inc /usr/local/www/fbegin.inc.before_dashboard");
	}
	mwexec("tar xzvpf /usr/local/pkg/widgets.tgz -C /usr/local/www/");
	mwexec("chmod a-w /usr/local/www/index.php");
	mwexec("chmod a-w /usr/local/www/fbegin.php");
	mwexec("chmod a-w /usr/local/www/graph_cpu.php");
	mwexec("chmod a-w /usr/local/www/stats.php");
}

function dashboard_deinstall() {
	global $g, $config;
	if(file_exists("/usr/local/www/index.php.before_dashboard")) {
		/* restore the files prior to the dashboard package installation */
		mwexec("chmod ug+rw /usr/local/www/index.php");
		mwexec("cp /usr/local/www/index.php.before_dashboard /usr/local/www/index.php");
		mwexec("cp /usr/local/www/fbegin.inc.before_dashboard /usr/local/www/fbegin.inc");
	}
}

?>