<?php

function widget_havp_uninstall() {
	unlink("/usr/local/www/includes/havp_alerts.inc.php");
	unlink("/usr/local/www/widgets/helpers/havp_alerts_helper.php");
	unlink("/usr/local/www/widgets/include/havp_alerts.inc");
	unlink("/usr/local/www/widgets/javascript/havp_alerts.js");
	unlink("/usr/local/www/widgets/widgets/havp_alerts.widget.php");
}

?>