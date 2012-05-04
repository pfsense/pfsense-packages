#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine the flags of advanced system options set
require_once("global.inc");
require_once(PFSENSEINCDIR . "config.inc");

/* Use these to output status at the end
$nExitStatus = STATE_OK; // STATE_WARNING STATE_CRITICAL
$szExitMessage = array();
*/

// Default statuses
$bRebindCheck = true;
$bHTTPReferrerCheck = true;
$bConsoleProtect = true;
$bFirewallDisabled = true;

// See if one or more are specifically disabled
$szShortOpt	= "";
$szLongOpt	= array("NoRebindCheck::",
					"NoReferrerCheck::",
					"NoConsoleProtect::",
					"NoFWDisabledCheck::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("NoRebindCheck", $vOptions)) 		{ $bRebindCheck			= false; }
if(array_key_exists("NoReferrerCheck", $vOptions)) 		{ $bHTTPReferrerCheck	= false; }
if(array_key_exists("NoConsoleProtect", $vOptions))		{ $bConsoleProtect		= false; }
if(array_key_exists("NoFWDisabledCheck", $vOptions))	{ $bFirewallDisabled	= false; }


	// Check Rebind check disabled
if($bRebindCheck)
{
	if(isset($config['system']['webgui']['nodnsrebindcheck']))
	{
		PushNRPEMessage(STATE_WARNING, "DNS Rebinding check is disabled");
	}
}

	// Check HTTP Referrer check disabled
if($bHTTPReferrerCheck)
{
	if(isset($config['system']['webgui']['nohttpreferercheck']))
	{
		PushNRPEMessage(STATE_WARNING, "HTTP Referrer check disabled. You should add alternate hostnames instead");
	}
}

	// Check Console Password disabled
if($bConsoleProtect)
{
	if(!isset($config['system']['disableconsolemenu']))
	{
		PushNRPEMessage(STATE_WARNING, "Console is not password protected");
	}
}

	// Check if firewall is disabled
if($bFirewallDisabled)
{
	if(isset($config['system']['disablefilter']))
	{
		PushNRPEMessage(STATE_WARNING, "Firewall is administratively disabled");
	}
}

NRPEexit();

?>