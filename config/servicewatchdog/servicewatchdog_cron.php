#!/usr/local/bin/php -f
<?php
require_once("globals.inc");
require_once("pfsense-utils.inc");
require_once("servicewatchdog.inc");

global $g;

/* Do nothing at bootup, unless the bootup indicators have been that way for at least 15 minutes. */
$max_boot_wait_time = 15 * 60;
if (($g['booting'] || file_exists("{$g['varrun_path']}/booting")) && (get_uptime_sec() < $max_boot_wait_time))
	return;

servicewatchdog_check_services();
?>
