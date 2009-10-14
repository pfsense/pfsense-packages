<?php
require("guiconfig.inc");

require_once("includes/havp_alerts.inc.php");

$havp_alerts_logfile = "{$g['varlog_path']}/havp/access.log";
$nentries = 5;
handle_havp_ajax($havp_alerts_logfile, $nentries);

?>
<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
<script src="/javascript/scriptaculous/scriptaculous.js" type="text/javascript"></script>
<script src="/widgets/javascript/havp_alerts.js" type="text/javascript"></script>
