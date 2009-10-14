<?php
require("guiconfig.inc");

require_once("includes/snort_alerts.inc.php");

$snort_alerts_logfile = "{$g['varlog_path']}/snort/alert";
$nentries = 5;
handle_snort_ajax($snort_alerts_logfile, $nentries);

?>
<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
<script src="/javascript/scriptaculous/scriptaculous.js" type="text/javascript"></script>
<script src="/widgets/javascript/snort_alerts.js" type="text/javascript"></script>
