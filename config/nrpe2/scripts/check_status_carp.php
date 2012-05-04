#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine the flags of advanced system options set
require_once("global.inc");
require_once(PFSENSEINCDIR . "config.inc");
require_once(PFSENSEINCDIR . "pfsense-utils.inc");

/* Use these to output status at the end
	PushNRPEMessage(STATE_WARNING, "Bad things about to happen");
*/

$bWarnInit		= true; // Check for INIT state
$bWarnSplit		= true; // Check for split state (Some master, some backup)
$bWarnWrongState= true;	// I.e. BACKUP on a host that isn't primary

$nAdvBackupRgn	= (1 * 256) + 100; // This is used to determine whether the CARP IP is configured as primary or backup
								   // This should be overridden via command line argument "AdvBackupRgn"
								   // in cases where Masters may advertise with a higher base or skew
								   // The minimum value is defined as (BASE * 256) + SKEW
								 

// See if one or more are specifically disabled
$szShortOpt	= "";
$szLongOpt	= array("NoWarnInit::",
					"NoWarnSplit::",
					"NoWarnWrongState::",
					"AdvBackupRgn::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("NoWarnInit", $vOptions)) 		{ $bWarnInit	= false; }
if(array_key_exists("NoWarnSplit", $vOptions)) 		{ $bWarnSplit	= false; }
if(array_key_exists("NoWarnWrongState", $vOptions)) { $bWarnWrongState	= false; }
if(array_key_exists("AdvBackupRgn", $vOptions)) 	{ $nAdvBackupRgn= $vOptions['AdvBackupRgn']; }


// Check
$nNumTotal	= 0;	// Number of total CARP interfaces

$vMasterIFs	= array(); // MASTER status interfaces
$vBackupIFs	= array(); // BACKUP status interfaces
$vInitIFs	= array(); // INIT status interfaces

// Enumerate statuses
foreach($config['virtualip']['vip'] as $vCARP)
{
	if($vCARP['mode'] != "carp") { continue; }
	
	$szStatus = get_carp_interface_status("vip" . $vCARP['vhid']);
	
	// Add our status field for later
	$vCARP['current_status'] = $szStatus;
	
	switch($szStatus)
	{
		case "MASTER":	{ $vMasterIFs[] = $vCARP; }break;
		case "BACKUP":	{ $vBackupIFs[] = $vCARP; }break;
		case "INIT": 	{ $vInitIFs[]   = $vCARP; }break;
	}
	$nNumTotal++;
}

// Warn for interfaces in INIT state
if($bWarnInit)
{
	if(count($vInitIFs) > 0)
	{
		foreach($vInitIFs as $vIF):
		{
			$szMsg = "Interface " . GetIFFriendlyName($vCARP) . " in INIT state";
			PushNRPEMessage(STATE_ERROR, $szMsg);
		}endforeach;
	}	
}

/*
// For testing, simulate split
{
	for($i = 0; $i < 5; $i++)
	{
		$vSimCARP['mode']		= "carp";
		$vSimCARP['interface']	= "lan";
		$vSimCARP['vhid']		= rand(50,100);
		$vSimCARP['advskew']	= (rand(0,1)==1) ? "0" : "100";
		$vSimCARP['advbase']	= 1;
		$vSimCARP['password']	= "simulated";
		$vSimCARP['descr']		= "simulated carp IF";
		$vSimCARP['type']		= "single";
		$vSimCARP['subnet']		= "172.16." . rand(0,31) . "." . rand(0,254);
		$vSimCARP['subnet_bits']= 24;
		
		switch(rand(0,1))
		{
		case 0: { $vSimCARP['current_status'] = "MASTER"; $vMasterIFs[] = $vSimCARP; }break;
		case 1: { $vSimCARP['current_status'] = "BACKUP"; $vBackupIFs[] = $vSimCARP; }break;
		case 2: { $vSimCARP['current_status'] = "INIT";   $vInitIFs[] = $vSimCARP; }break;
		}
		$nNumTotal++;
	}
}
*/

// Warn for split state

if($bWarnSplit)
{
	$nNumMaster = count($vMasterIFs);
	$nNumBackup	= count($vBackupIFs);
	$nNumInit	= count($vInitIFs);
	
	if(  ($nNumTotal != $nNumMaster)
	   &&($nNumTotal != $nNumBackup)
	   &&($nNumTotal != $nNumInit))
	{
		$szMsg = "CARP interfaces are in split state:\n";
			// Dump all interface statuses
		foreach($vMasterIFs as $vCARP): { DumpIFStatusToMessage($vCARP, $szMsg); }endforeach;
		foreach($vBackupIFs as $vCARP): { DumpIFStatusToMessage($vCARP, $szMsg); }endforeach;
		foreach($vInitIFs   as $vCARP): { DumpIFStatusToMessage($vCARP, $szMsg); }endforeach;
		
		PushNRPEMessage(STATE_CRITICAL, $szMsg);
	}
	
}

/*
// For testing, simulate wrong states
{
	for($i = 0; $i < 4; $i++)
	{
		$vSimCARP['mode']		= "carp";
		$vSimCARP['interface']	= "lan";
		$vSimCARP['vhid']		= 100+$i;
		$vSimCARP['advbase']	= 1;
		$vSimCARP['advskew']	= "0";
		$vSimCARP['password']	= "simulated";
		$vSimCARP['descr']		= "simulated carp IF";
		$vSimCARP['type']		= "single";
		$vSimCARP['subnet']		= "172.16.1.10" . $i;
		$vSimCARP['subnet_bits']= 24;
		
		switch(rand(1,1))
		{
		case 0: { $vSimCARP['current_status'] = "MASTER"; $vMasterIFs[] = $vSimCARP; }break;
		case 1: { $vSimCARP['current_status'] = "BACKUP"; $vBackupIFs[] = $vSimCARP; }break;
		}
		$nNumTotal++;
	}
}
*/

// Warn if Master is in backup state or backup in master state
if($bWarnWrongState)
{
	// See if all that are masters should be masters (Check if we're a BACKUP running as MASTER)
	if(count($vMasterIFs) > 0)
	{
		$nServingAsBackup = 0;
		
		foreach($vMasterIFs as $vIF):
		{
			$nAdv = ($vIF['advbase'] * 256) + $vIF['advskew']; // Check what the interface is configured to be advertising as
			// We shouldn't be master for this VIP
			if($nAdv >= $nAdvBackupRgn)
			{
				$szMsg = GetIFFriendlyName($vIF) . " is being serviced by this backup host";
				PushNRPEMessage(STATE_NOTICE, $szMsg);
				$nServingAsBackup++;
			}
		}endforeach;
		
		if(count($vMasterIFs) == $nServingAsBackup)
		{
			$szMsg = "This host has taken over servicing all IPs";
			PushNRPEMessage(STATE_WARNING, $szMsg);
		}
	}
	
	// See if all that are backups should be backups (Check if we're a MASTER running as BACKUP)
	if(count($vBackupIFs) > 0)
	{
		foreach($vBackupIFs as $vIF):
		{
			$nAdv = ($vIF['advbase'] * 256) + $vIF['advskew'];
			// We shouldn't be backup for this VIP
			// This is particularly catastrophic especially if this host is still online
			// This is usually accompanied by a split state
			if($nAdv < $nAdvBackupRgn)
			{
				$szMsg = GetIFFriendlyName($vIF) . " is no longer serviced by its primary host";
				PushNRPEMessage(STATE_ERROR, $szMsg);
			}
		}endforeach;
	}

}


NRPEExit();



function GetIFFriendlyName($vIF)
{
	if(0 == strlen($vIF['descr'])) { $vIF['descr'] = "No Name"; }
	return $vIF['descr'] . " (vip" . $vIF['vhid'] . " " . $vIF['subnet'] . "/" . $vIF['subnet_bits'] . ")";
}

function DumpIFStatusToMessage($vIF, &$sz)
{
	$sz .= GetIFFriendlyName($vIF) . " -> " . $vIF['current_status'] . "\n";			
}














?>