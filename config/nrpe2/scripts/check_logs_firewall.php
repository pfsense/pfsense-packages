#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine the flags of advanced system options set
require_once("global.inc");
require_once("logfiles.inc");
require_once(PFSENSEINCDIR . "config.inc");
require_once("interfaces.inc");
require_once("filter_log.inc");

/* Use these to output status at the end
	PushNRPEMessage(STATE_WARNING, "Bad things about to happen");
*/

// Default alerts (none)
$szAlerts			= "{Excessive Source IP Blocking (4hrs)|srcip|any|30%|40%|240}{Excessive Destination Port Blocking (4hrs)|dstport|any|30%|40%|240}";		// Special alerts
$bReportTopItems	= true;		// Always report top items
$vnTopItemTimes		= array(60,
							1440);	// Show top items for # of minutes
$vnTopItemTypes		= array("srcip",
							"dstip",
							"dstport",
							"proto",
							"tcpflags");
$vnMinutes			= array(   5,	// Number of minutes to generate info for
							  30,
							  60,
							 120,
							 240,
							 720,
						    1440);
$nPercentCalcThres			= 50;	// Minimum of n logs returned in order to calculate percentages
$nMaxLogLines				= 5000; // Maximum number of log lines ot analyze - reduce on slow systems if timeouts occur or performance degrades

							
// See if one or more are specifically disabled
$szShortOpt	= "";
$szLongOpt	= array("Alerts::",
					"NoTopItems::",
					"PercentCalcThres::",
					"MaxLogLines::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("Alerts", $vOptions))		{ $szAlerts	= $vOptions['Alerts']; }
if(array_key_exists("NoTopItems", $vOptions))	{ $bReportTopItems = false; }
if(array_key_exists("PercentCalcThres", $vOptions)) { $nPercentCalcThres = $vOptions['PercentCalcThres']; }
if(array_key_exists("MaxLogLines", $vOptions))	{ $nMaxLogLines = $vOptions['MaxLogLines']; }

/* {alert_name|alert_type|alert_match|warn_lvl|alert_lvl|timeframe}
	alert_name = Name for alert
	alert_type = Type of match: (  "act",			// Action
								   "interface",	// Interface
								   "proto",		// Protocol
								   "srcip",		// Src IP
								   "dstip",		// Dest IP
								   "srcport",		// Src Port
								   "dstport"); // Dest Port
	alert_match = String to match against alert
					- "any" can be used to warn if any single one exceeds the limit
	warn_lvl = level at which to issue a warning
	alert_lvl	= level at which to issue an alert
	timeframe	= timeframe to observe ( any of $vnMinutes minutes)
	
  vLogEntries structure
  GetItemizedData() data itemized by: 
  "act",			// Action
  "interface",	// Interface
  "proto",		// Protocol
  "srcip",		// Src IP
  "dstip",		// Dest IP
  "srcport",		// Src Port
  "dstport");		// Dest Port
*/
/*
    ["realint"]=>
    string(3) "de1"
    ["act"]=>
    string(5) "block"
    ["time"]=>
    string(15) "Dec 20 18:40:19"
    ["srcport"]=>
    string(0) ""
    ["srcip"]=>
    string(13) "192.168.1.152"
    ["dstport"]=>
    string(0) ""
    ["dstip"]=>
    string(13) "192.168.1.150"
    ["src"]=>
    string(13) "192.168.1.152"
    ["dst"]=>
    string(13) "192.168.1.150"
    ["interface"]=>
    string(3) "LAN"
    ["rulenum"]=>
    string(2) "35"
    ["proto"]=>
    string(4) "ICMP"
    ["tcpflags"]=>
    string(0) ""

*/

// Get logs
$szLogfile		= "{$g['varlog_path']}/filter.log";
$vLogData		= array();


// Get all log data
foreach($vnMinutes as $nMin):
{
	$vLogData[$nMin] = GetItemizedData($szLogfile, $nMin, $nPercentCalcThres, $nMaxLogLines);
}endforeach;


// Alerts
$vAlerts = ParseAlerts($szAlerts);
		/*$vAlerts[$i]['name']			= $vData[0];
		$vAlerts[$i]['type']			= $vData[1];
		$vAlerts[$i]['match']			= $vData[2];
		$vAlerts[$i]['warning_level']	= intval($vData[3]);
		$vAlerts[$i]['alert_level']		= intval($vData[4]);
		$vAlerts[$i]['timeframe']		=*/
foreach($vAlerts as $cAlert):
{
	$cItem		= array();
	
	if($cAlert['match'] == "any")
	{
		if($vLogData[$cAlert['timeframe']][$cAlert['type']])
		{
			$cItem			= reset($vLogData[$cAlert['timeframe']][$cAlert['type']]);
			$cItem['name']	= key($vLogData[$cAlert['timeframe']][$cAlert['type']]);
		}
	}
	else
	{
		$cItem = $vLogData[$cAlert['timeframe']][$cAlert['type']][$cAlert['match']];
		$cItem['name'] = $cAlert['match'];
	}
	
	// What match?
	$nWarningThres	 = intval($cAlert['warning_level']);
	$nWarningNum	 = ($cAlert['alert_percent']) ? ($cItem['percent'] * 100) : ($cItem['total']);
	$nAlertThres	 = intval($cAlert['alert_level']);
	$nAlertNum		 = ($cAlert['alert_percent']) ? ($cItem['percent'] * 100) : ($cItem['total']);
	
	// Now match
	if($nAlertNum > $nAlertThres)
	{
		$szSuffix = ($bAlertPercent) ? "%" : "";
		$szMsg =  "Alert " . $cAlert['name'] . " has been raised: Offender=" . $cItem['name'] . " Threshold=" . $nAlertThres . $szSuffix . " Actual=" . $nAlertNum . $szSuffix;
		PushNRPEMessage(STATE_ALERT, $szMsg);
	}
	elseif($nWarningNum > $nWarningThres)
	{
		$szSuffix = ($bAlertPercent) ? "%" : "";
		$szMsg =  "Warning " . $cAlert['name'] . " has been raised: Offender=" . $cItem['name'] . " Threshold=" . $nWarningThres . $szSuffix . " Actual=" . $nWarningNum . $szSuffix;
		PushNRPEMessage(STATE_WARNING, $szMsg);
	}	
	

}endforeach;


// Show top items for the data
if($bReportTopItems)
{
	foreach($vnTopItemTimes as $nTopInterval):
	{
		$szMsg = "Top Items for last " . $nTopInterval . " minutes:";
		
		foreach($vnTopItemTypes as $nItemType):
		{
			if(isset($vLogData[$nTopInterval][$nItemType]))
			{
				$nTopItemVal = reset($vLogData[$nTopInterval][$nItemType]);
				$nTopItemName= key($vLogData[$nTopInterval][$nItemType]);
								
				$szMsg .= " " . $nItemType . " = " . $nTopItemName . " (" . ($nTopItemVal['percent'])*100 . "%)";
			}

		}endforeach;
		PushNRPEMEssage(STATE_DEBUG, $szMsg);
	}endforeach;

}


NRPEExit();

// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
///////////////////////////////////////////
// Functions
///////////////////////////////////////////

function logcmp($a, $b)
{
	if($a == $b)
	{
		return 0;
	}
	return ($a['total'] < $b['total']) ? 1 : -1;
}

// Counts all items and sort them into an associative array
function GetItemizedData($szLogfile, $nMinutes, $nPercentCalcThres, $nMaxLogLines = 5000)
{
	// Itemize data by:
	$vItemTypes = array("act",			// Action
						"interface",	// Interface
						"proto",		// Protocol
						"srcip",		// Src IP
						"dstip",		// Dest IP
						"srcport",		// Src Port
						"dstport",		// Dest Port
						"tcpflags");	// TCP Flags

	// Get raw logs
	$vLogEntries = conv_log_filter($szLogfile, $nMaxLogLines, $nMaxLogLines);
	
	// Now we have only the pertinent records (last XX minutes)
		// Create associative array
		// Sort out logs older than nMinutes minutes
	$nOldestTime	= time() - ($nMinutes * 60);
	$vvCounts = array();
	$nTotalLogs = 0;
	$nNumLogEntries = count($vLogEntries);
	for($i = 0; $i < $nNumLogEntries; $i++)
	{
		if(LogDateTimeToUnixTime($vLogEntries[$i]['time']) >= $nOldestTime)
		{
			$nNumTypeEntries = count($vItemTypes);
			for($j = 0; $j < $nNumTypeEntries; $j++)
			{
				if(!empty($vLogEntries[$i][$vItemTypes[$j]]))
				{
					$vvCounts[$vItemTypes[$j]][$vLogEntries[$i][$vItemTypes[$j]]]['total']++;
				}
			}
			
			$nTotalLogs++;
		}
	}
	
	// Calculate all percentages
	foreach($vvCounts as &$vOuter):
	{
		// Itemize all information
		foreach($vOuter as &$cInner):
		{
			if($nTotalLogs >= $nPercentCalcThres) { $cInner['percent'] = round($cInner['total'] / $nTotalLogs, 3); }
			else							  	  { $cInner['percent'] = "n/a"; }
		}endforeach;
	
		// Sort all data
		uasort($vOuter, 'logcmp');
		
	}endforeach;

	return $vvCounts;

}


function ParseAlerts($szAlerts)
{
	// Split alert string
	$vszAlerts = preg_split("/[\\{\\}]/", $szAlerts);
	
	// Parse input
	$vAlerts	= array();
	$i			= 0;
	foreach($vszAlerts as $szAlert):
	{
		// This happens between tokens
		if($szAlert == "") { continue; }
		
		// Split alert
		$vData = preg_split("/(\\|)/", $szAlert);

		// Must be exactly 6 --> alert_name|alert_type|alert_match|warn_lvl|alert_lvl|timeframe}
		if(count($vData) != 6)
		{
			PushNRPEMessage(STATE_NOTICE, "Invalid alert ignored: " . $szAlert);
			continue;
		}
		
		// wan_lvl and alert_lvl must be numbers or percentages
		$bWarnPercent = false;
		$bAlertPercent= false;
		
		if(preg_match("/.*?(%)/", $vData[3]))
		{
			$arr = preg_split("/(%)/", $vData[3]);
			$vData[3]	= $arr[0];
			$bWarnPercent= true;
		}
		
		if(!is_numeric($vData[3]))
		{
			PushNRPEMessage(STATE_NOTICE, "Alert \"" . $vData[0] . "\" ignored: Warning level must be numeric or a percentage");
			continue;
		}
		
		if(preg_match("/.*?(%)/", $vData[4]))
		{
			$arr = preg_split("/(%)/", $vData[4]);
			$vData[4]	= $arr[0];
			$bAlertPercent = true;
		}
		
		if(!is_numeric($vData[4]))
		{
			PushNRPEMessage(STATE_NOTICE, "Alert \"" . $vData[0] . "\" ignored: Alert level must be numeric or a percentage");
			continue;
		}
		
		if(  (!is_numeric($vData[5]))
		   ||($vData[5] <= 0))
		{
			PushNRPEMessage(STATE_NOTICE, "Alert \"" . $vData[0] . "\" ignored: Timeframe must be numeric and > 0");
			continue;
		}
		
			// If both are set, then warning must be lower than alert
		if(  ($vData[3] > 0)
		   &&($vData[4] > 0)
		   &&(!($vData[3] < $vData[4])))
		{
			PushNRPEMessage(STATE_NOTICE, "Alert \"" . $vData[0] . "\" ignored: If both warning and alert levels are set, warning level must be lower than alert level");
		}
		
		// Parse data
		$vAlerts[$i]['name']			= $vData[0];
		$vAlerts[$i]['type']			= $vData[1];
		$vAlerts[$i]['match']			= $vData[2];
		$vAlerts[$i]['warning_level']	= intval($vData[3]);
		$vAlerts[$i]['warning_percent']	= $bWarnPercent;
		$vAlerts[$i]['alert_level']		= intval($vData[4]);
		$vAlerts[$i]['alert_percent']	= $bAlertPercent;
		$vAlerts[$i]['timeframe']		= intval($vData[5]);
		$i++;
	}endforeach;
	
	return $vAlerts;
	
}



?>