#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine the flags of advanced system options set
require_once("global.inc");
require_once(PFSENSEINCDIR . "config.inc");

/* Use these to output status at the end
	PushNRPEMessage(STATE_WARNING, "Bad things about to happen");
*/

$bCheckDefaultGateway = true;

// See if one or more are specifically disabled
$szShortOpt	= "";
$szLongOpt	= array("NoDefaultGWCheck::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("NoDefaultGWCheck", $vOptions)) 	{ $bCheckDefaultGateway	= false; }


// Check

if($bCheckDefaultGateway)
{
	$bHasDefault = false;
	foreach($config['gateways'] as $vGW):
	{
		if(isset($vGW['defaultgw']))
		{
			$bHasDefault = true;
			break;
		}
	}endforeach;
		
	if(!$bHasDefault)
	{
		PushNRPEMessage(STATE_DEBUG, "No default gateway set");
	}
}


NRPEExit();


?>