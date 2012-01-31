#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine the flags of advanced system options set
require_once("global.inc");
require_once("logfiles.inc");
require_once(PFSENSEINCDIR . "config.inc");

/* Use these to output status at the end
	PushNRPEMessage(STATE_WARNING, "Bad things about to happen");
*/

// Default (Login failed only)
$szAlerts = "";


//{alert_name|search_str|not_search_str|warn_lvl|alert_lvl}{search_str|not_search_str|warn_lvl|alert_lvl|timeframe|report_debug}
// Warning level and alert level can be 0 if warning or alert shouldn't be issued
// 	However, if both are set, then alert must be higher than warning
// timeframe is in minutes
// report_debug 0 or 1

$szLogfile = "{$g['varlog_path']}/ipsec.log";

GenericProcessAlerts($szLogfile, $szAlerts);

NRPEexit();
?>