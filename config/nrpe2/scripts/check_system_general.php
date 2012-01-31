#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine the flags of advanced system options set
require_once("global.inc");
require_once(PFSENSEINCDIR . "config.inc");

/* Use these to output status at the end
	PushNRPEMessage(STATE_WARNING, "Bad things about to happen");
*/

// Default statuses
$bHostnameChecks 	= true;
$bDomainSuffixChecks= true;
$bSingleDNSWarn		= true;
$bDNSChecks 		= true;
$bNTPChecks 		= true;
$szDomainName		= "www.google.com"; // Domain name used for DNS tests
$nNumDNSLookups		= 2;
$fDNSLookupPassRate	= 0.6; // Passing rate less than 50% generates warning

// See if one or more are specifically disabled
$szShortOpt	= "";
$szLongOpt	= array("NoHostnameChecks::",
					"NoDomainSuffixChecks::",
					"NoSingleDNSWarn::",
					"NoDNSChecks::",
					"NoNTPChecks::",
					"DNSCheckName::",
					"NumDNSLookups::",
					"DNSLookupPassRate::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("NoHostnameChecks", $vOptions)) 	{ $bHostnameChecks		= false; }
if(array_key_exists("NoDomainSuffixChecks", $vOptions)) { $bDomainSuffixChecks	= false; }
if(array_key_exists("NoSingleDNSWarn", $vOptions))		{ $bSingleDNSWarn		= false; }
if(array_key_exists("NoDNSChecks", $vOptions))			{ $bDNSChecks			= false; }
if(array_key_exists("NoNTPChecks", $vOptions))			{ $bNTPChecks			= false; }
if(array_key_exists("DNSCheckName", $vOptions))			{ $szDomainName			= $vOptions['DNSCheckName']; }
if(array_key_exists("NumDNSLookups", $vOptions))		{ $nNumDNSLookups		= $vOptions['NumDNSLookups']; }
if(array_key_exists("DNSLookupPassRate", $vOptions))	{ $fDNSLookupPassRate	= $vOptions['DNSLookupPassRate']; }


	// Check if hostname is valid
if($bHostnameChecks)
{
	$szHostname = $config['system']['hostname'];

	if("" == $szHostname)
	{
		PushNRPEMessage(STATE_ERROR, "No hostname set");
	}
	else if("pfsense" == $szHostname)
	{
		PushNRPEMessage(STATE_WARNING, "Default hostname set");
	}
	
	if(!IsValidHostname($szHostname))
	{
		PushNRPEMessage(STATE_NOTICE, "Hostname constitutes invalid DNS");
	}
}

	// Check if dns suffix is valid
if($bDomainSuffixChecks)
{
	$szDomainSuffix = $config['system']['domain'];

	if("" == $szDomainSuffix)
	{
		PushNRPEMessage(STATE_ERROR, "No DNS suffix set");
	}
	elseif("localdomain" == $szDomainSuffix)
	{
		PushNRPEMessage(STATE_WARNING, "Default DNS suffix set");
	}
	elseif(!IsValidDNS($szDomainSuffix))
	{
		PushNRPEMessage(STATE_WARNING, "DNS Suffix contains constitutes invalid DNS");
	}
}

	// Check DNS server config
if(bDNSChecks)
{
	// Enumerate how many DNS servers are set
	// Ignore localhost
	$nNumDNSServers = 0;
	$szValidDNSServers = array();
	if(count($config['system']['dnsserver']) > 0)
	{
		foreach($config['system']['dnsserver'] as $szDNSSrv):
		{
			if(  (IsValidIPv4($szDNSSrv) || IsValidIPv6($szDNSSrv))
			   &&($szDNSSrv != "127.0.0.1" && $szDNSSrv != "::1"))
			{ $nNumDNSServers++; $szValidDNSServers[] = $szDNSSrv; }
		}endforeach;
	}
	
	// See if we have any defined
	if($nNumDNSServers < 1)
	{
		PushNRPEMessage(STATE_ERROR, "No DNS servers assigned");
	}
	elseif($nNumDNSServers < 2)
	{
		$nSeverity = ($bSingleDNSWarn) ? STATE_WARNING : STATE_DEBUG;
		PushNRPEMessage($nSeverity, "No secondary DNS server defined");
	}

	
	// Do a name resolution
	$bOverallSuccess= false; // At least one DNS server succeeded with lookups
	if($nNumDNSServers > 0)
	{
		for($i = 0; $i < $nNumDNSServers; $i++)
		{
			$nNumSuccess = $nNumDNSLookups;
			
			for($j = 0; $j < $nNumDNSLookups; $j++)
			{
				if(1 == exec("nslookup -timeout=1 " . $szDomainName . " " . $szValidDNSServers[$i] . " | grep -c \"connection timed out\""))
				{ $nNumSuccess--; }
			}
			
			// Determine pass or fail
			if(($nNumSuccess / $nNumDNSLookups) < $fDNSLookupPassRate)
			{
				$szMessage = "DNS Server " . $szValidDNSServers[$i] . " failed " . ($nNumDNSLookups - $nNumSuccess) . " out of " . $nNumDNSLookups . " times resolving " . $szDomainName;
				PushNRPEMessage(STATE_WARNING, $szMessage);
			}
			else { $bOverallSuccess = true; }
		}
		
		if(!$bOverallSuccess)
		{
			PushNRPEMessage(STATE_CRITICAL, "All DNS servers are failing to resolve " . $szDomainName);
		}
		
		
	}
}

if($bNTPChecks)
{
	$szNTPServer = $config['system']['timeservers'];
	
	if(!(  (IsValidIPv4($szNTPServer))
	     ||(IsValidIPv6($szNTPServer))
	     ||(IsValidDNSPart($szNTPServer) && !(false == dns_get_record($szNTPServer)))))
	{
		PushNRPEMessage(STATE_WARNING, "Invalid NTP time server defined");
	}
	

}


NRPEexit();


// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
///////////////////////////////////////////
// Functions
///////////////////////////////////////////

// Checks if a part of an FQDN is valid
function IsValidHostname($sz)
{
	return preg_match("/^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$/", $sz);
}

function IsValidDNS($sz)
{
	return preg_match("/^([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]{2,6}$/", $sz);
}

function IsValidIPv4($sz)
{
	/* Not implemented */
	return true;
}

function IsValidIPv6($sz)
{
	/* Not implemented */
	return true;
}





?>