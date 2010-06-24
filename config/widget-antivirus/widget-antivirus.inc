<?php

function widget_antivirus_uninstall() {
	unlink("/usr/local/www/widgets/include/antivirus_status.inc");
	unlink("/usr/local/www/widgets/widgets/antivirus_status.widget.php");
}

?>