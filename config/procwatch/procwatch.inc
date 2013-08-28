<?php
require_once("config.inc");
require_once("services.inc");
require_once("service-utils.inc");
require_once("util.inc");

function procwatch_cron_job() {
	global $config;
	if (!is_array($config['installedpackages']['procwatch']['item'])) {
		$config['installedpackages']['procwatch']['item'] = array();
	}
	$a_pwservices = &$config['installedpackages']['procwatch']['item'];

	if (count($a_pwservices) > 0) {
		// Add the cron job if it doesn't exist.
		install_cron_job("/usr/local/pkg/procwatch_cron.php", true, "*/1");
	} else {
		// Remove the cron job
		install_cron_job("/usr/local/pkg/procwatch_cron.php", false, "*/1");
	}
}

function procwatch_check_services() {
	global $config;
	if (!is_array($config['installedpackages']['procwatch']['item'])) {
		$config['installedpackages']['procwatch']['item'] = array();
	}
	$a_pwservices = &$config['installedpackages']['procwatch']['item'];

	foreach ($a_pwservices as $svc) {
		if (!is_service_running($svc['name'])) {
			$descr = strlen($svc['description']) > 50 ? substr($svc['description'], 0, 50) . "..." : $svc['description'];
			log_error("ProcWatch detected service {$svc['name']} stopped. Restarting {$svc['name']} ({$descr})");
			service_control_start($svc['name'], $svc);
		}
	}
}

?>