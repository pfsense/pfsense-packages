<?php

require_once("global.inc");

require_once(PFSENSEINCDIR . "interfaces.inc");
require_once(PFSENSEINCDIR . "gwlb.inc");

define(GW_ONLINE,	0); // Healthy gateway
define(GW_DEGRADED,	1); // Packet loss / Latency
define(GW_OFFLINE,	2); // Gateway down
define(GW_EXPIRED,	4); // Gateway status invalid due to age
define(GW_UNKNOWN,	5); // Gateway status cannot be enumerated

function EnumerateGWStatus(&$vGateways, $nAgeFactor)
{
	// Enumerate gateway health level and warn about gateways
	foreach($vGateways as &$vGW):
	{
		if(stristr($vGW['status'], "down")) { $vGW['health'] = GW_OFFLINE; }
		elseif(  (stristr($vGW['status'], "delay"))
			   ||(stristr($vGW['status'], "loss")))
											{ $vGW['health'] = GW_DEGRADED; }
		elseif(stristr($vGW['status'], "none")) { $vGW['health'] = GW_ONLINE; }
		else								{ $vGW['health'] = GW_UNKNOWN; }

		$nTimeDiff = time() - TimeFormatToUnix($vGW['lastcheck'], "%a, %d %b %Y %H:%M:%S %z");
		$nCheckInterval = $vGW['check_interval'];

		if($nTimeDiff > ($nCheckInterval * $nAgeFactor))
		{
			$vGW['health'] = GW_EXPIRED;
			$vGW['status_age'] = TimeDiffToAgoFormat($nTimeDiff);
		}
		
	}endforeach;
}

function GetApingerProcessStatus()
{
	return (1 == exec("ps -Aw | grep -v \"grep\" | grep -c \"apinger\""));
}

function GetAllGatewayStatuses()
{
	global $config;

	$vGWs = return_gateways_status(true);
	$vGWConfig = $config['gateways']['gateway_item'];

	foreach($vGWConfig as $vGWConfigItem):
	{
		$vGWs[$vGWConfigItem['name']]['check_interval'] = $vGWConfigItem['interval'];
	}endforeach;
	
	return $vGWs;
}
















?>