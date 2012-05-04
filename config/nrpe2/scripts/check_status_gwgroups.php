#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine the flags of advanced system options set
require_once("global.inc");
require_once("gwstatus.inc");
require_once(PFSENSEINCDIR . "gwlb.inc");
require_once(PFSENSEINCDIR . "config.inc");

/* Use these to output status at the end
	PushNRPEMessage(STATE_WARNING, "Bad things about to happen");
*/

$bWarnApinger		= true; // Warn about apinger not running
$bWarnTierOffline	= true; // Warn about tier being offline status
$bWarnTierDegraded	= true; // Warn about overall tier packet loss / RTT status
$nTierDegradedThresOffline	= 0.5; // Threshold of offline GWs at which the tier is considered degraded
$nTierDegradedThresDegraded	= 0.5; // Threshold of degraded GWs at which the tier is considered degraded
$nGroupDegradedThresOffline = 0.75; // Threshold at which the whole group is considered to be degraded (% tiers offline)
$nGroupDegradedThresDegraded = 0.75; // Threshold at which the whole group is considered to be degraded (% tiers degraded)
$bIgnoreAge			= false; // Don't warn when GW status is too old
$bReportExtended	= true;	// Report extended info for GW groups even if they're healthy
$nAgeFactor			= 3;	// Oldness factor ( e.g. 3 * check time configured in pfsense)

$nMaxTiers			= 5; // PFSense dicatates this - this is not CMDline overridable

// See if one or more are specifically disabled
$szShortOpt	= "";
$szLongOpt	= array("NoWarnApinger::",
					"NoWarnTierOffline::",
					"NoWarnTierDegraded::",
					"TierDegradedThresOffline::",
					"TierDegradedThresDegraded::",
					"GroupDegradedThresOffline::",
					"GroupDegradedThresDegraded::",
					"IgnoreAge::",
					"NoReportExtended::",
					"AgeFactor::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("NoWarnApinger", 				$vOptions)) { $bWarnApinger	= false; }
if(array_key_exists("NoWarnTierOffline",			$vOptions)) { $bWarnTierOffline	= false; }
if(array_key_exists("NoWarnTierDegraded",			$vOptions)) { $bWarnTierDegraded= false; }
if(array_key_exists("TierDegradedThresOffline", 	$vOptions)) { $nTierDegradedThresOffline = $vOptions['TierDegradedThresOffline']; }
if(array_key_exists("TierDegradedThresDegraded", 	$vOptions)) { $nTierDegradedThresDegraded = $vOptions['TierDegradedThresDegraded']; }
if(array_key_exists("GroupDegradedThresOffline", 	$vOptions)) { $nGroupDegradedThresOffline = $vOptions['GroupDegradedThresOffline']; }
if(array_key_exists("GroupDegradedThresDegraded", 	$vOptions)) { $nGroupDegradedThresDegraded= $vOptions['GroupDegradedThresDegraded']; }
if(array_key_exists("IgnoreAge", 					$vOptions))	{ $bIgnoreAge	= true; }
if(array_key_exists("NoReportExtended", 			$vOptions))	{ $bReportExtended	= false; }
if(array_key_exists("nAgeFactor", 					$vOptions))	{ $nAgeFactor = $vOptions['nAgeFactor']; }

// Check for Apinger process
if($bWarnApinger)
{
	if(!GetApingerProcessStatus())
	{
		PushNRPEMessage(STATE_ERROR, "apinger process not running");
	}
}

if(0 == count($config['gateways']['gateway_group']))
{
	PushNRPEMessage(STATE_INFO, "No gateway groups are configured");
}

// Get all gateway groups and determine their status
$vGWGroups = array();
$nGWGroupsIndex = 0;

// Get all gateways for association
$vGateways = GetAllGatewayStatuses();
	// No need to check here, if there are GW groups, there must be gateways configured!
EnumerateGWStatus($vGateways, $nAgeFactor);


// Create group array with gateway info
foreach($config['gateways']['gateway_group'] as $vGWG):
{
	$vGWGroups[$nGWGroupsIndex]['name'] = $vGWG['name'];
	$vGWGroups[$nGWGroupsIndex]['descr'] = $vGWG['descr'];
	
	// Create the tiered structure
	foreach($vGWG['item'] as $vEntry):
	{
		$vStr = explode("|", $vEntry);
		if(count($vStr) != 2) // Name and Tier only
		{ PushNRPEMessage(STATE_NOTICE, "Error processing GW Group info: " . $vEntry); }
		
		// Dandy up to here, create a GW item for the group and tier
		$vGWEntry = $vGateways[$vStr[0]];

		$vGWGroups[$nGWGroupsIndex]['tiers'][$vStr[1]][] = $vGWEntry;	
	}endforeach;
	
	$nGWGroupsIndex++;
}endforeach;


// Determine roll-up status for tiers
foreach($vGWGroups as &$vGWG):
{
	$nNumTiers = count($vGWG['tiers']);
	$nTiersDegraded	= 0;
	$nTiersOffline	= 0;
		
	foreach($vGWG['tiers'] as &$vTier):
	{
		$nNumGWs = count($vTier);
		$nGWsExpired	= 0;
		$nGWsDegraded	= 0;
		$nGWsOffline	= 0;
		$nGWsUnknown	= 0;
		$nGWOnline		= 0;
		
		foreach($vTier as $vGW):
		{
			switch($vGW['health'])
			{
				// Outdated info
				case GW_EXPIRED:	{ if(!$bIgnoreAge) { $nGWsExpired++; } }break;
				// Degraded state
				case GW_DEGRADED:	{ $nGWsDegraded++; }break;
				// Offline state
				case GW_OFFLINE: 	{ $nGWsOffline++; }break;
				// Unknown state
				case GW_ONLINE:		{ $nGWOnline++; }break;
				// Online state
				default:			{ $nGWsUnknown++; }break;
			}
		}endforeach;

		// Determine tier health
		if($bIgnoreAge) { $nGWsExpired = 0; }
		$nOfflineRatio = ($nGWsOffline + $nGWsExpired + $nGWsUnknown) / $nNumGWs;
		$nDegradedRatio= $nGWsDegraded / $nNumGWs;
		
		// Extended health info string:
		$vTier['ext_info'] = "Offline=" . ($nGWsOffline / $nNumGWs)*100 .
							 "%\tDegraded=" . ($nGWsDegraded / $nNumGWs)*100 .
							 "%\tExpired=" . ($nGWsExpired / $nNumGWs)*100 .
							 "%\tUnknown=" . ($nGWsUnknown / $nNumGWs)*100 . "%";
		
		// Offline issues
		if($nGWsOffline == $nNumGWs)
		{
			$vTier['health'] = GW_OFFLINE;
			$nTiersOffline++;
		}
		elseif($nOfflineRatio >= $nTierDegradedThresOffline)
		{
			$vTier['health'] = GW_DEGRADED;
			$nTiersDegraded++;
		}
		// Degraded issues
		else if($nDegradedRatio >= $nTierDegradedThresDegraded)
		{
			$vTier['health'] = GW_DEGRADED;
			$nTiersDegraded++;
		}
		else { $vTier['health'] = GW_ONLINE; } 
		
		// If Tier is unhealthy at all (even if it's not a warning/failure), make sure extended info is available
		if($nGWOnline != $nNumGWs)
		{
			$vTier['ext_avail'] = True;
		}

	}endforeach; // Tier

	// Determine overall group health
	$nGroupOfflineRatio = $nTiersOffline / $nNumTiers;
	$nGroupDegradedRatio= $nTiersDegraded / $nNumTiers;

	// Offline issues
	if($nTiersOffline == $nNumTiers)
	{
		$vGWG['health'] = GW_OFFLINE;
	}
	elseif($nGroupOfflineRatio > $nGroupDegradedThresOffline)
	{
		$vGWG['health'] = GW_DEGRADED;
	}
	// Degraded issues
	else if($nGroupDegradedRatio > $nGroupDegradedThresDegraded)
	{
		$vGWG['health'] = GW_DEGRADED;
	}
	else { $vGWG['health'] = GW_ONLINE; }
	
	
	///////////////////////
	// Report everything //
	///////////////////////
	
	if($vGWG['health'] == GW_OFFLINE)
	{
		PushNRPEMessage(STATE_CRITICAL, "Gateway group " . $vGWG['name'] . " is offline");
	}
	else if($vGWG['health'] == GW_DEGRADED)
	{
		PushNRPEMessage(STATE_WARNING, "Gateway group " . $vGWG['name'] . " is degraded");
	}

	
	// Otherwise see if non-group affecting issues exist and should be reported
	if(  ($bWarnTierOffline)
	   ||($bWarnTierDegraded))
	{
		for($i = 0; $i < $nMaxTiers; $i++)
		{
			if(isset($vGWG['tiers'][$i]))
			{
				switch($vGWG['tiers'][$i]['health'])
				{
					case GW_DEGRADED:	{ if($bWarnTierDegraded) { PushNRPEMessage(STATE_NOTICE, $vGWG['name'] . "/Tier" . $i . " is degraded" ); } }break;
					case GW_OFFLINE:	{ if($bWarnTierOffline){ PushNRPEMessage(STATE_NOTICE, $vGWG['name'] . " /Tier" . $i . " is offline" ); } }break;
				}
				
				if(  ($bReportExtended)
				   &&($vGWG['tiers'][$i]['ext_avail']))
				{
					$szMsg = $vGWG['name'] . "/Tier" . $i . ": " . $vGWG['tiers'][$i]['ext_info'];
					PushNRPEMessage(STATE_DEBUG, $szMsg);
				}
			}
		}

	}
	
	
	
	
	
		
}endforeach; // GW Group



NRPEExit();






?>