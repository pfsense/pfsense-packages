<?php

function widget_snort_uninstall() {

	unlink("/usr/local/www/includes/snort_alerts.inc.php");
	unlink("/usr/local/www/widgets/helpers/snort_alerts_helper.php");
	unlink("/usr/local/www/widgets/include/snort_alerts.inc");
	unlink("/usr/local/www/widgets/javascript/snort_alerts.js");
	unlink("/usr/local/www/widgets/widgets/snort_alerts.widget.php");

}

?>