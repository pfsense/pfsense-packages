<?php

require_once("globals.inc");
require_once("includes/snort_alerts.inc.php");

$snort_alerts_title = "Snort Alerts";
$snort_alerts_title_link = "snort/snort_alerts.php";

$snort_alerts_logfile = "{$g['varlog_path']}/snort/alert";
$nentries = 10;
$snort_alerts = get_snort_alerts($snort_alerts_logfile, $nentries);

/* AJAX related routines */
handle_snort_ajax($snort_alerts_logfile, $nentries = 10);

?>
