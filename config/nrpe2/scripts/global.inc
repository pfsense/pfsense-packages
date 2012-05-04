<?php

// Include directory for core pfsense
define(PFSENSEGUIDIR, "/usr/local/www/");
// Include directory for pfsense packages, etc (i.e. openvpn)
define(PFSENSEINCDIR, "/etc/inc/");

// NRPE States for exiting
define(NRPE_RET_OK,			0);
define(NRPE_RET_WARNING,	1);
define(NRPE_RET_CRITICAL,	2);

// Reporting states (from scripts)
define(STATE_EMERGENCY,	0);
define(STATE_ALERT,		1);
define(STATE_CRITICAL,	2);
define(STATE_ERROR,		3);
define(STATE_WARNING,	4);
define(STATE_NOTICE,	5);
define(STATE_INFO,		6);
define(STATE_DEBUG,		7);

// Global parameters for exiting
$vExitState	= array();

$szOKStatus = "Healthy";
$szOKCode 	= STATE_INFO;

function PushNRPEMessage($nSeverity, $szMsg)
{
	global $vExitState;
	static $nIndex = 0;
	$vExitState[$nIndex]['code']	= $nSeverity;
	$vExitState[$nIndex++]['msg']	= $szMsg;
}

function NRPEexit()
{
	global $vExitState;
	global $szOKStatus;
	
	// Determine if we need to add an all ok message
	if(0 == count($vExitState))
	{
		PushNRPEMessage(STATE_INFO, "Healthy");
	}
	else
	{
		$nWorst = STATE_DEBUG;
		foreach($vExitState as $cState):
		{
			if($cState['code'] < $nWorst) { $nWorst = $cState['code']; }
		}endforeach;
		
		if(STATE_DEBUG == $nWorst) { PushNRPEMessage(STATE_INFO, "Healthy"); }
	}
	
	// Sort message so that Critical are on top, followed by warnings, then info
	usort($vExitState, "SeverityCompare");
	
	// Categorize messages
	MessagesToNRPEOutput($vExitState);
	

	// Assemble final message
	$strFinalMessage = "";
	foreach($vExitState as $vStatus):
	{
		$strFinalMessage .= $vStatus['msg'] . "\n";
	}endforeach;

	echo $strFinalMessage;
	exit($vExitState[0]['code']);
}

function SeverityCompare($vA, $vB)
{
	// Sort by severity of message
    if ($vA['code'] == $vB['code'])
	{ return 0; }
	
    return ($vA['code'] < $vB['code']) ? -1 : 1;
}

function MessagesToNRPEOutput(&$vMessages)
{
	foreach($vMessages as &$vMsg):
	{
		// Format message
		if(strlen($vMsg['msg']) > 0)
		{ $vMsg['msg'] = SyslogSeverityToString($vMsg['code']) . " " . $vMsg['msg']; }
		else
		{ $vMsg['msg'] = SyslogSeverityToString($vMsg['code']). " No output by script"; }
		
		// Categorize output
		if($vMsg['code'] <= STATE_ERROR)
		{
			$vMsg['code'] = NRPE_RET_CRITICAL;
		}
		elseif($vMsg['code'] <= STATE_NOTICE)
		{
			$vMsg['code'] = NRPE_RET_WARNING;
		}
		else
		{
			$vMsg['code'] = NRPE_RET_OK;
		}
	}endforeach;
}

function SyslogSeverityToString($nSeverity)
{
	switch($nSeverity)
	{
		case STATE_EMERGENCY:return "EMERGENCY: ";	break;
		case STATE_ALERT:	 return "ALERT:     ";	break;
		case STATE_CRITICAL: return "CRITICAL:  ";	break;
		case STATE_ERROR:	 return "ERROR:     ";	break;
		case STATE_WARNING:  return "WARNING:   ";	break;
		case STATE_NOTICE:	 return "NOTICE:    ";	break;
		case STATE_INFO:	 return "INFO:      ";	break;
		case STATE_DEBUG:	 return "DEBUG:     ";	break;
		default:			 return "OTHER:     ";	break;
		
	}
}

function TimeFormatToUnix($szDate, $szFormat)
{
	$vData = strptime($szDate, $szFormat);  //"%a, %d %b %Y %H:%M:%S %z"
	
	// Rectify data
	$vData['tm_mon']++;
	$vData['tm_year'] += 1900;
	
	$nUnixTime = mktime($vData['tm_hour'], $vData['tm_min'], $vData['tm_sec'], $vData['tm_mon'], $vData['tm_mday'], $vData['tm_year']);
	return $nUnixTime;
}

function TimeDiffToAgoFormat($nTimeDiff)
{
	$nDays = floor(($nTimeDiff / (60 * 60 * 24)));
	
	$nRem = $nTimeDiff % (60 * 60 * 24);
	
	$nHours = floor($nRem / (60 * 60));

	$nRem = $nRem % (60 * 60);
	
	$nMins = floor($nRem / (60));
	
	$nRem = $nRem % (60);
	
	$nSecs = floor($nRem);
	
	$szFormat = $nDays . "d " . $nHours . "h " . $nMins . "m " . $nSecs . "s";

	return $szFormat;
}


?>