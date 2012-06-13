<?php

require_once("globals.inc");
require_once("includes/snort_alerts.inc.php");

$snort_alerts_title = "Snort Alerts";
$snort_alerts_title_link = "snort/snort_alerts.php";

foreach (glob("{$g['varlog_path']}/snort/alert_*") as $alert) {
	$snort_alerts_logfile = $alert;
	$nentries = 10;
	$snort_alerts = get_snort_alerts($snort_alerts_logfile, $nentries);

	/* AJAX related routines */
	handle_snort_ajax($snort_alerts_logfile, $nentries);
}
if($_GET['lastsawtime'] or $_POST['lastsawtime'])
	exit;

?>
