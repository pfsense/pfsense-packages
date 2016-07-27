<?php

require_once("includes/havp_alerts.inc.php");

$havp_alerts_title = "HAVP Alerts";
$havp_alerts_title_link = "havp_alerts.php";

$havp_alerts_logfile = "{$g['varlog_path']}/havp/access.log";
$nentries = 5;
$havp_alerts = get_havp_alerts($havp_alerts_logfile, $nentries);

/* AJAX related routines */
handle_havp_ajax($havp_alerts_logfile, $nentries = 5);

?>
