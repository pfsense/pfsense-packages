#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine number of things and nice names
require_once("./global.inc");
require_once(PFSENSEINCDIR . "config.inc");
require_once(PFSENSEINCDIR . "interfaces.inc");


// Defaults (What to check)
$bCheckDropRate		= true;	// Check if the drop rate is too high
$bOutputStats		= true;	// Outputs statistics
$nMinTotalPackets	= 10000;// Minimum total packets through the queue for better statistics
$fDropRateWarn		= 0.20;	// Higher than 20% drop rate is warning
$fDropRateCritical	= 1.01;	// 101%; by default, no drop rate is considered critical

// See if one or more are specifically disabled
$szShortOpt	= "";
$szLongOpt	= array("NoDropRateCheck::",
					"NoStats::",
					"MinTotalPackets::",
					"DropRateWarn::",
					"DropRateCritical::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("NoDropRateCheck",	$vOptions)) 	{ $bCheckDropRate	= false; }
if(array_key_exists("NoStats",	$vOptions)) 			{ $bOutputStats	= false; }
if(array_key_exists("MinTotalPackets", 	$vOptions)) 	{ $nMinTotalPackets = $vOptions['MinTotalPackets']; }
if(array_key_exists("DropRateWarn", 	$vOptions)) 	{ $fDropRateWarn = $vOptions['DropRateWarn'] / 100; }
if(array_key_exists("DropRateCritical", $vOptions)) 	{ $fDropRateCritical = $vOptions['DropRateCritical'] / 100; }


// Execute checks
$vQueueInfo = GetQueueData();

// First, do drop rate checks
if($bCheckDropRate)
{
	$vQueueHighestDrop = array();
	
	foreach($vQueueInfo as $vQueue):
	{
		// Minimum packets must match
		if($vQueue['packets'] > $nMinTotalPackets)
		{
			$fDropPercentage = floatval($vQueue['dropped']) / floatval($vQueue['packets']);
			$szMsg = "Queue " . $vQueue['name'] . " exceeded drop rate: " . number_format(($fDropPercentage * 100), 2) . "%";
			
			// Get highest value for statistics later
			if($fDropPercentage > $vQueueHighestDrop['percentage'])
			{
				$vQueueHighestDrop = $vQueue;
				$vQueueHighestDrop['percentage'] = $fDropPercentage;
			}
			
			// Critical warning
			if($fDropPercentage > $fDropRateCritical)
			{
				PushNRPEMessage(STATE_CRITICAL, $szMsg);
			}
			// Warning
			else if($fDropPercentage > $fDropRateWarn)
			{
				PushNRPEMessage(STATE_WARNING, $szMsg);
			}
		}

	}endforeach;
	
	// Report statistics
	if(  (count($vQueueHighestDrop) > 0)
	   &&($bOutputStats))
	{
		$szMsg = "Highest drop rate on queue " . $vQueueHighestDrop['name'] . ": " . number_format(($vQueueHighestDrop['percentage'] * 100), 2) . "%";
		PushNRPEMessage(STATE_DEBUG, $szMsg);
	}
	
}


NRPEexit();


// From PFSense. but modified
function GetQueueData($bRootQueues = true) {
	exec("/sbin/pfctl -vsq", $stats_array);
	$vAllQueues = array();
	$vCurQueue = array();
	$nOutIndex = 0;
	foreach ($stats_array as $stats_line)
	{
		$match_array = "";

		if(preg_match("/(queue)(\\s+)((?:[A-Za-z0-9_]*))/",$stats_line))
		{
			$tmp = preg_split("/(queue)(\\s+)((?:[A-Za-z0-9_]*)).*?(\\s+)(on)(\\s+)((?:[A-Za-z0-9_]*))/", $stats_line, -1, PREG_SPLIT_DELIM_CAPTURE);
			$vCurQueue['name'] = $tmp[3] . " " . $tmp[5] . " " . $tmp[7];
		}
		if(preg_match("/.*?(pkts)(:).*?(\\d+)/",$stats_line))
		{
			$tmp = preg_split("/.*?(pkts)(:).*?(\\d+)/", $stats_line, -1, PREG_SPLIT_DELIM_CAPTURE);
			$vCurQueue['packets'] = $tmp[3];
		}
		if(preg_match("/.*?(dropped pkts)(:).*?(\\d+)/",$stats_line))
		{
			$tmp = preg_split("/.*?(dropped pkts)(:).*?(\\d+)/", $stats_line, -1, PREG_SPLIT_DELIM_CAPTURE);
			$vCurQueue['dropped'] = $tmp[3];
		}
		if(preg_match("/.*?(qlength)(:).*?(\\d+)(\\/).*?(\\d+)/",$stats_line))
		{
			$tmp = preg_split("/.*?(qlength)(:).*?(\\d+)(\\/).*?(\\d+)/", $stats_line, -1, PREG_SPLIT_DELIM_CAPTURE);
			$vCurQueue['backlog'] = $tmp[3];
			$vCurQueue['length'] = $tmp[5];
		}
		if(preg_match("/.*?(borrows)(:).*?(\\d+)/",$stats_line))
		{
			$tmp = preg_split("/.*?(borrows)(:).*?(\\d+)/", $stats_line, -1, PREG_SPLIT_DELIM_CAPTURE);
			$vCurQueue['borrows'] = $tmp[3];
		}
		if(preg_match("/.*?(suspends)(:).*?(\\d+)/",$stats_line))
		{
			$tmp = preg_split("/.*?(suspends)(:).*?(\\d+)/", $stats_line, -1, PREG_SPLIT_DELIM_CAPTURE);
			$vCurQueue['suspends'] = $tmp[3];
		
			// Adding to output is only applicable if a whole number of queue infos has been passed
			$vAllQueues[$nOutIndex++] = $vCurQueue;
			
		}
	}
	return $vAllQueues;
}

?>