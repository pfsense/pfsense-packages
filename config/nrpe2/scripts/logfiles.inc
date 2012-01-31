<?php

require_once("global.inc");


function GetLogData($szLogFile, $nNumMax, $szContains = "", $szDoesNotContain = "", $bShowInverted = True)
{
	$vRet = array();
	$vLines = array();

	// Prepare GREPs
	$szGreps = " ";
	
	if(is_array($szContains))
	{
		foreach($szContains as $agrep):
		{
			$szGreps .= " | grep \"{$agrep}\"";
		}endforeach;
	}
	if(is_array($szDoesNotContain))
	{
		foreach($szDoesNotContain as $agrep):
		{
			$szGreps .= " | grep -v \"{$agrep}\"";
		}endforeach;
	}
	
	// Get display mode
	$szSortFlag = ($bShowInverted) ? "-r" : "";
	
	$szCommand = "/usr/sbin/clog {$szLogFile}{$szGreps} | grep -v \"CLOG\" | grep -v \"\033\" | /usr/bin/tail {$szSortFlag} -n {$nNumMax}";
	exec($szCommand, $vLines);
	
	$i = 0;
	foreach($vLines as $szLogLine):
	{
		$szReported = "";	// Human readable when msg was reported
		$nUnixReported = 0;	// Unix time msg was reported
		if ($c=preg_match_all ("/".$re1.$re2.$re3.$re4.$re5."/is", $szLogLine, $matches))
		{
			
			// Attach correct year to log entry so that we can convert to unix time
			$nCurYear = strftime("%Y");
			$szReported		= $matches[1][0] . " " . $matches[3][0] . " " . $nCurYear . " " . $matches[5][0];
			// Create correct unix time
			$nUnixReported	= TimeFormatToUnix($szReported, "%b %e %Y %H:%M:%S");
			if($nUnixReported > time()) // This means the entry is a year old
			{
				$szReported		= $matches[1][0] . " " . $matches[3][0] . " " . $nCurYear-1 . " " . $matches[5][0];
				$nUnixReported	= TimeFormatToUnix($szReported, "%b %e %Y %H:%M:%S");
			}
		}
		
		$vRet[$i]['rep_time_u'] = LogDateTimeToUnixTime($szLogLine, $vRet[$i]['rep_time_f']);
		$vRet[$i++]['msg']		= $szLogLine;
	
	}endforeach;
	
	return $vRet;
}

function LogDateTimeToUnixTime($szLogDateTime, &$szHumanOut = "")
{
	$re1='((?:Jan(?:uary)?|Feb(?:ruary)?|Mar(?:ch)?|Apr(?:il)?|May|Jun(?:e)?|Jul(?:y)?|Aug(?:ust)?|Sep(?:tember)?|Sept|Oct(?:ober)?|Nov(?:ember)?|Dec(?:ember)?))';	# Month 1
	$re2='(\\s+)';	# White Space 1
	$re3='((?:(?:[0-2]?\\d{1})|(?:[3][01]{1})))(?![\\d])';	# Day 1
	$re4='(\\s+)';	# White Space 2
	$re5='((?:(?:[0-1][0-9])|(?:[2][0-3])|(?:[0-9])):(?:[0-5][0-9])(?::[0-5][0-9])?(?:\\s?(?:am|AM|pm|PM))?)';	# HourMinuteSec 1

	$szReported = "";	// Human readable when msg was reported
	$nUnixReported = 0;	// Unix time msg was reported
	if ($c=preg_match_all ("/".$re1.$re2.$re3.$re4.$re5."/is", $szLogDateTime, $matches))
	{
		// Attach correct year to log entry so that we can convert to unix time
		$nCurYear = strftime("%Y");
		$szReported		= $matches[1][0] . " " . $matches[3][0] . " " . $nCurYear . " " . $matches[5][0];
		// Create correct unix time
		$nUnixReported	= TimeFormatToUnix($szReported, "%b %e %Y %H:%M:%S");
		if($nUnixReported > time()) // This means the entry is a year old
		{
			$szReported		= $matches[1][0] . " " . $matches[3][0] . " " . $nCurYear-1 . " " . $matches[5][0];
			$nUnixReported	= TimeFormatToUnix($szReported, "%b %e %Y %H:%M:%S");
		}
	}
	
	$vRet[$i]['rep_time_u'] = $nUnixReported;
	
	$szHumanOut = $szReported;
	return $nUnixReported;
}


function GenericParseAlerts($szAlerts)
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

		// Must be exactly 5 --> alert_name|search_str|not_search_str|warn_lvl|alert_lvl|debug_info
		if(count($vData) != 7)
		{
			PushNRPEMessage(STATE_NOTICE, "Invalid alert ignored: " . $szAlert);
			continue;
		}
		
		// wan_lvl and alert_lvl must be numbers
		if(!is_numeric($vData[3]))
		{
			PushNRPEMessage(STATE_NOTICE, "Alert \"" . $vData[0] . "\" ignored: Warning level must be numeric");
			continue;
		}
		
		if(!is_numeric($vData[4]))
		{
			PushNRPEMessage(STATE_NOTICE, "Alert \"" . $vData[0] . "\" ignored: Alert level must be numeric");
			continue;
		}
		
		if(  (!is_numeric($vData[5]))
		   ||($vData[5] <= 0))
		{
			PushNRPEMessage(STATE_NOTICE, "Alert \"" . $vData[0] . "\" ignored: Timeframe must be numeric and > 0");
			continue;
		}

		if(  (!is_numeric($vData[6]))
		   ||(  ($vData[6] != 0)
			  &&($vData[6] != 1)))
		{
			PushNRPEMessage(STATE_NOTICE, "Alert \"" . $vData[0] . "\" ignored: Debug reporting must be boolean (0 or 1)");
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
		$vAlerts[$i]['search_string']	= $vData[1];
		$vAlerts[$i]['not_search_string']= $vData[2];
		$vAlerts[$i]['warning_level']	= intval($vData[3]);
		$vAlerts[$i]['alert_level']		= intval($vData[4]);
		$vAlerts[$i]['timeframe']		= intval($vData[5]);
		$vAlerts[$i]['debug_info']		= intval($vData[6]);
		$i++;
	}endforeach;
	
	return $vAlerts;
	
}

function GenericProcessAlerts($szLogFile, $szAlerts)
{
	// See if one or more are custom alerts are enabled
	$szShortOpt	= "";
	$szLongOpt	= array("Alerts::");

	$vOptions = getopt("", $szLongOpt);

	if(array_key_exists("Alerts", $vOptions))			{ $szAlerts	= $vOptions['Alerts']; }

	// Parse alerts
	$vAlerts = GenericParseAlerts($szAlerts);
	if(count($vAlerts) == 0)
	{
		PushNRPEMessage(STATE_NOTICE, "No alerts defined. Exiting.");
	}
	
	// Check for all alerts
	foreach($vAlerts as $cAlert):
	{
		$bAlertRaised	= false; // Whether or not an alert was raised
		$bWarningRaised	= false; // Whether or not a warning was raised
		$bReportAlerts	= ($cAlert['alert_level'] > 0);
		$bReportWarnings= ($cAlert['warning_level'] > 0);
		
		// Get data
		$nMaxGet = ($bReportAlerts) ? $cAlert['alert_level'] : $cAlert['warning_level'];
		$vEntries = GetLogData($szLogFile, $nMaxGet, array("{$cAlert['search_string']}"), "", true);
		
		// Determine how many occured within the timeframe
		$nOldestTime = time() - ($cAlert['timeframe'] * 60);

		$vnOldIndices = array();
		for($i = 0; $i < count($vEntries); $i++)
		{
			if($vEntries[$i]['rep_time_u'] < $nOldestTime) { $vnOldIndices[] = $i; }
		}
		for($i = 0; $i < count($vnOldIndices); $i++) { unset($vEntries[$vnOldIndices[$i]]); }
		
		// Report alerts and warnings
		if(  ($bReportAlerts)
		   &&((count($vEntries)) >= $cAlert['alert_level']))
		{
			PushNRPEMessage(STATE_ALERT, "Alert " . $cAlert['name'] . " has been raised");
			$bAlertRaised = true;
		}
		
		if(  ($bReportWarnings)
		   &&(!$bAlertRaised)
		   &&((count($vEntries)) >= $cAlert['warning_level']))
		{
			PushNRPEMessage(STATE_WARNING, "Warning " . $cAlert['name'] . " has been raised");
			$bWarningRaised = true;
		}
		
		// Report extended debug info
		if(  ($cAlert['debug_info'])
		   &&($bWarningRaised || $bAlertRaised))
		{
			$szMsg = "Monitor " . $cAlert['name'] . " - last captured syslog line: " . $vEntries[0]['msg'];
			
			PushNRPEMessage(STATE_DEBUG, $szMsg);
		}
		
		

	}endforeach;
	
	
}








?>