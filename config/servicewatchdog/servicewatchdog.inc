<?php
require_once("config.inc");
require_once("services.inc");
require_once("service-utils.inc");
require_once("util.inc");

function servicewatchdog_cron_job() {
	global $config;
	if (!is_array($config['installedpackages']['servicewatchdog']['item'])) {
		$config['installedpackages']['servicewatchdog']['item'] = array();
	}
	$a_pwservices = &$config['installedpackages']['servicewatchdog']['item'];

	if (count($a_pwservices) > 0) {
		// Add the cron job if it doesn't exist.
		install_cron_job("/usr/local/pkg/servicewatchdog_cron.php", true, "*/1");
	} else {
		// Remove the cron job
		install_cron_job("/usr/local/pkg/servicewatchdog_cron.php", false, "*/1");
	}
}

function servicewatchdog_check_services() {
	global $config;
	if (!is_array($config['installedpackages']['servicewatchdog']['item'])) {
		$config['installedpackages']['servicewatchdog']['item'] = array();
	}
	$a_pwservices = &$config['installedpackages']['servicewatchdog']['item'];

	foreach ($a_pwservices as $svc) {
		if (!get_service_status($svc)) {
			$descr = strlen($svc['description']) > 50 ? substr($svc['description'], 0, 50) . "..." : $svc['description'];
			log_error("Service Watchdog detected service {$svc['name']} stopped. Restarting {$svc['name']} ({$descr})");
			service_control_start($svc['name'], $svc);
		}
	}
}

?>