#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine number of servers and nice names
require_once("./global.inc");
require_once(PFSENSEINCDIR . "config.inc");
require_once(PFSENSEINCDIR . "openvpn.inc");

// Defaults (What to check)
$bCheckServiceRunning = true; // Check if all openvpn processes are launched

// See if one or more are specifically disabled
$szShortOpt	= "";
$szLongOpt	= array("NoServiceCheck::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("NoServiceCheck", $vOptions)) 		{ $bCheckServiceRunning	= false; }


// Execute checks

if($bCheckServiceRunning)
{
	$vServers = $config['openvpn']['openvpn-server'];
	$vToCheck = array();
	$i = 0;
	
	// Enumerate all servers
	if(is_array($vServers))
	{
		foreach($vServers as $cServer):
		{
			// Only add enabled servers to our list
			if(!isset($cServer['disable']))
			{
				$vToCheck[$i]['DisplayName']	= $cServer['description'] . " (" . $cServer['interface'] . ":" . $cServer['local_port'] . " " . $cServer['protocol']. ")";
				$vToCheck[$i]['AbsConfigFile']	= $g['varetc_path'] . "/openvpn/server" . $cServer['vpnid'] . ".conf";
			}
					
			$i++;
		}endforeach;
	}

	// Ok, now check with ps if all these services are up and running
	if(is_array($vToCheck))
	{
		$i=0;
		foreach($vToCheck as $cServer):
		{
		//	echo exec("ps -A | awk '{print $7}' | grep -c \"" . $cServer['AbsConfigFile'] . "\"") . "\r\n";
			if(0 == exec("ps -Aw | grep -v \"grep\" | grep \"sbin/openvpn\" | grep -c \"" . $cServer['AbsConfigFile'] . "\""))
			{
				PushNRPEMessage(STATE_CRITICAL, $vToCheck[$i]['DisplayName'] . " not running");
			}
			$i++;
		}endforeach;
	}
}

NRPEexit();

?>