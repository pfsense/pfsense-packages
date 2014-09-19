#!/usr/local/bin/php -f 
<?php
require_once("/usr/local/pkg/snort/snort.inc");
global $g;
switch (strtolower($argv[1])) {
	case "start":
		if (!file_exists("{$g['varrun_path']}/snort_pkg_starting.lck")) {
			touch("{$g['varrun_path']}/snort_pkg_starting.lck");
			snort_start_all_interfaces();
			unlink_if_exists("{$g['varrun_path']}/snort_pkg_starting.lck");
		}
		break;

	case "stop":
		snort_stop_all_interfaces();
		unlink_if_exists("{$g['varrun_path']}/snort_pkg_starting.lck");
		break;

	case "restart":
		snort_stop_all_interfaces();
		touch("{$g['varrun_path']}/snort_pkg_starting.lck");
		snort_start_all_interfaces();
		unlink_if_exists("{$g['varrun_path']}/snort_pkg_starting.lck");
		break;

	default:
		echo "WARNING: ignoring unsupported command - '{$argv[1]}'\n";		
}
?>
