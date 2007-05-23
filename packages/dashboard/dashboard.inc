<?php

function dashboard_install() {
	global $g, $config;
	if(!file_exists("/usr/local/www/index.php.before_dashboard")) {
		/* backup the pre-dashboard index.php file */
		mwexec("cp /usr/local/www/index.php /usr/local/www/index.php.before_dashboard");
	}
	mwexec("tar xzvpf /usr/local/pkg/widgets.tgz -C /usr/local/www/");
}

function dashboard_deinstall() {
	global $g, $config;
	if(file_exists("/usr/local/www/index.php.before_dashboard")) {
		/* restore the file prior to the dashboard package installation */
		mwexec("cp /usr/local/www/index.php.before_dashboard /usr/local/www/index.php");
	}
}

?>