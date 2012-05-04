#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine number of servers and nice names
require_once("./global.inc");
require_once(PFSENSEINCDIR . "config.inc");
require_once(PFSENSEINCDIR . "openvpn.inc");

// Defaults (What to check)
$bCheckServiceRunning	= true; // Check if all openvpn client processes are launched
$bCheckStatus 			= true; // Check if all openvpn client processes are actually connected

// See if one or more are specifically disabled
$szShortOpt	= "";
$szLongOpt	= array("NoServiceCheck::",
					"NoStatusCheck::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("NoServiceCheck", $vOptions)) 		{ $bCheckServiceRunning	= false; }
if(array_key_exists("NoStatusCheck", $vOptions)) 		{ $bCheckStatus	= false; }



// Execute checks


// Now get the openvpn client config

	// Check Service
if($bCheckServiceRunning)
{
	$vClients = $config['openvpn']['openvpn-client'];
	$vToCheck = array();
	$i = 0;

	// Enumerate all Clients
	if(is_array($vClients))
	{
		foreach($vClients as $cClient):
		{
			// Only add enabled servers to our list
			if(!isset($cClient['disable']))
			{
				$vToCheck[$i]['DisplayName']	= $cClient['description'] . " (" . $cClient['interface'] . ":" . $cClient['local_port'] . " " . $cClient['protocol']. ")";
				$vToCheck[$i]['AbsConfigFile']	= $g['varetc_path'] . "/openvpn/client" . $cClient['vpnid'] . ".conf";
			}
					
			$i++;
		}endforeach;
	}
	
	// Ok, now check with ps if all these services are up and running
	if(is_array($vToCheck))
	{
		$i=0;
		foreach($vToCheck as $cClient):
		{
		//	echo exec("ps -A | awk '{print $7}' | grep -c \"" . $cClient['AbsConfigFile'] . "\"") . "\r\n";
			if(0 == exec("ps -Aw | grep -v \"grep\" | grep \"sbin/openvpn\" | grep -c \"" . $cClient['AbsConfigFile'] . "\""))
			{
				PushNRPEMessage(STATE_CRITICAL, $vToCheck[$i]['DisplayName'] . " not running");
			}
			$i++;
		}endforeach;
	}
}

	// Check status
if($bCheckStatus)
{
	$vClients = openvpn_get_active_clients();

	foreach($vClients as $cClient):
	{
		$nSeriousness = STATE_INFO;
		
		// If not UP, there's a problem
		switch($cClient['status'])
		{
			case "down": 
			case "connecting": { $nSeriousness = STATE_CRITICAL;	}break;
			case "up":		   { $nSeriousness = STATE_DEBUG;		}break;
			default:	 	   { $nSeriousness = STATE_NOTICE;	}break;
		}
		
		$szMsg = $cClient['name'] . " is in the following state: " . $cClient['status'];
		PushNRPEMessage($nSeriousness, $szMsg);
	
	}endforeach;
	
	
	
}

NRPEexit();

?>