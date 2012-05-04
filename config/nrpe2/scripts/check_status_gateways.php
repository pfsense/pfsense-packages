#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine the flags of advanced system options set
require_once("global.inc");
require_once("gwstatus.inc");
require_once(PFSENSEINCDIR . "config.inc");

/* Use these to output status at the end
	PushNRPEMessage(STATE_WARNING, "Bad things about to happen");
*/

$bWarnApinger	= true; // Warn about apinger not running
$bWarnOffline	= true; // Warn about gateway down status
$bWarnDegraded	= true; // Warn about gateway packet loss / RTT status
$bIgnoreAge		= false; // Don't warn when GW status is too old
$nAgeFactor		= 3;	// Oldness factor ( e.g. 3 * check time configured in pfsense)

// See if one or more are specifically disabled
$szShortOpt	= "";
$szLongOpt	= array("NoWarnApinger::",
					"NoWarnOffline::",
					"NoWarnQuality::",
					"NoWarnStatusOld::",
					"OldnessFactor::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("NoWarnApinger", $vOptions)) 		{ $bWarnApinger	= false; }
if(array_key_exists("NoWarnOffline", $vOptions)) 		{ $bWarnOffline	= false; }
if(array_key_exists("NoWarnDegraded", $vOptions)) 		{ $bWarnDegraded= false; }
if(array_key_exists("IgnoreAge", 	$vOptions)) 		{ $bIgnoreAge	= true; }
if(array_key_exists("AgeFactor", 	$vOptions)) 		{ $nAgeFactor= $vOptions['AgeFactor']; }

///////////////////////
// Checks
///////////////////////

// Check for Apinger process
if($bWarnApinger)
{
	if(!GetApingerProcessStatus())
	{
		PushNRPEMessage(STATE_ERROR, "apinger process not running");
	}
}

// Get all gateways
$vGateways = GetAllGatewayStatuses();

	// Anything to check?
if(0 == count($vGateways))
{
	PushNRPEMessage(STATE_INFO, "No gateways are enabled for monitoring");
}
else
	{
	// Get gateway status
	EnumerateGWStatus($vGateways, $nAgeFactor);

	// Warn about gateways
	foreach($vGateways as $vGW):
	{
		switch($vGW['health'])
		{
			// Outdated info
			case GW_EXPIRED:
			{
				if($bIgnoreAge)
				{
					// Gateway status is outdated
					PushNRPEMessage(STATE_NOTICE, $vGW['name'] . " : Gateway status is " . $vGW['status_age'] . " old");		
				}
			}break;
			
			// Degraded state
			case GW_DEGRADED:
			{
				if($bWarnDegraded) { PushNRPEMessage(STATE_WARNING, $vGW['name'] . " is degraded; RTT = " . $vGW['delay'] . " PL = " . $vGW['loss']); }
			}break;
			
			// Offline state
			case GW_OFFLINE:
			{
				if($bWarnOffline) { PushNRPEMessage(STATE_ERROR, $vGW['name'] . " is offline; RTT = " . $vGW['delay'] . " PL = " . $vGW['loss']); }
			}break;
			
			// Unknown state
			case GW_UNKNOWN:
			{
				PushNRPEMessage(STATE_NOTICE, $vGW['name'] . " is in an unknown status; raw = " . $vGW['status']);
			}break;
		}

		
	}endforeach;
}



NRPEExit();










?>