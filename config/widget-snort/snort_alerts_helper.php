<?php
require_once("globals.inc");
require_once("guiconfig.inc");
require_once("includes/snort_alerts.inc.php");

foreach (glob("{$g['varlog_path']}/snort/alert_*") as $alert) {
        $snort_alerts_logfile = $alert;
        $nentries = 5;
        $snort_alerts = get_snort_alerts($snort_alerts_logfile, $nentries);

        /* AJAX related routines */
        handle_snort_ajax($snort_alerts_logfile, $nentries);
}
if($_GET['lastsawtime'] or $_POST['lastsawtime'])
	exit;

?>
<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
<script src="/javascript/scriptaculous/scriptaculous.js" type="text/javascript"></script>
<script src="/widgets/javascript/snort_alerts.js" type="text/javascript"></script>
