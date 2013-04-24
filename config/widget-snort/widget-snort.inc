<?php

require_once("guiconfig.inc");
require_once("config.inc");

function widget_snort_uninstall() {

	global $config;

	/* Remove the Snort widget from the Dashboard display list */
	$widgets = $config['widgets']['sequence'];
	if (!empty($widgets)) {
		$widgetlist = explode(",", $widgets);
		foreach ($widgetlist as $key => $widget) {
			if (strstr($widget, "snort_alerts-container"))
				unset($widgetlist[$key]);
		}
		$config['widgets']['sequence'] = implode(",", $widgetlist);
		write_config();
	}

	/* Remove our associated file */
	unlink("/usr/local/www/widgets/include/widget-snort.inc");
	unlink("/usr/local/www/widgets/widgets/snort_alerts.widget.php");
	unlink("/usr/local/www/widgets/javascript/snort_alerts.js");
}

?>
