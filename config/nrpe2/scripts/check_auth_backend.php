#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine the flags of advanced system options set
require_once("global.inc");
require_once(PFSENSEINCDIR . "config.inc");
require_once(PFSENSEINCDIR . "auth.inc");

/* Use these to output status at the end
	PushNRPEMessage(STATE_WARNING, "Bad things about to happen");
*/

// Default parameters
//$szAlerts = "{Webadmin Login Failed|webConfigurator authentication error for||5|10|720|1}{SSH Login failed|error: PAM: authentication error for||5|10|720|1}";

// Command line overrides
/*
$szShortOpt	= "";
$szLongOpt	= array("LogFile::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("LogFile", $vOptions))		{ $szFilename = $vOptions['LogFile']; }
*/





NRPEexit();












?>