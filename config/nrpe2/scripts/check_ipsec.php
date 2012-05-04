#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine number of servers and nice names
require_once("./global.inc");
require_once(PFSENSEINCDIR . "config.inc");
require_once(PFSENSEINCDIR . "interfaces.inc");
require_once(PFSENSEINCDIR . "ipsec.inc");

// Defaults (What to check)
$bWarnDisabled = true; // Warn if phase1 / 2 are disabled

// See if one or more are specifically disabled
$szShortOpt	= "";
$szLongOpt	= array("NoWarnDisabled::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("NoWarnDisabled", $vOptions))  { $bWarnDisabled	= false; }


// Execute checks

// From PFSense IPSec GUI
$vPhase2 = &$config['ipsec']['phase2'];
$spd = ipsec_dump_spd();
$sad = ipsec_dump_sad();

// Ok, now check with PFSense GUI functions if all these services are up and running

// All of IPSec administratively disabled?
if(!isset($config['ipsec']['enable']))
{
	PushNRPEMessage(STATE_WARNING, "IPSec is administratively disabled");
}
// Check phase 2's
else
{
	foreach ($vPhase2 as $ph2ent)
	{
		ipsec_lookup_phase1($ph2ent,$ph1ent);
		
		// Assemble printable info
		if(strlen($ph1ent['descr']) == 0) { $ph1ent['descr'] = "Untitled Phase 1"; }
		if(strlen($ph2ent['descr']) == 0) { $ph2ent['descr'] = "Untitled Phase 2"; }
		
		$szLink = $ph1ent['descr'] . "/" . $ph2ent['descr'];
		
		$szRemote =  "";
		if($ph2ent['remoteid']['type'] == "network")	{ $szRemote .= "network " . $ph2ent['remoteid']['address'] . "/" . $ph2ent['remoteid']['netbits']; }
		elseif($ph2ent['remoteid']['type'] == "address"){ $szRemote .= "host " . $ph2ent['remoteid']['address']; }
		
		// Check status
		if(  (  (isset($ph2ent['disabled']))
			  ||(isset($ph1ent['disabled']))))
		{
			// Phase disabled
			if($bWarnDisabled)
			{
				$szMsg = "Link " . $szLink . " to remote " . $szRemote . " is disabled";
				PushNRPEMessage(STATE_WARNING, $szMsg);
			}
		}
		elseif(!ipsec_phase2_status($spd,$sad,$ph1ent,$ph2ent))
		{
			// critical: down
			$szMsg = "Link " . $szLink . " to remote " . $szRemote . " is down";
			PushNRPEMessage(STATE_CRITICAL, $szMsg);
		}
	}
}















NRPEexit();
















?>