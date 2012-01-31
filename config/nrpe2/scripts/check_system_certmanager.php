#!/usr/local/bin/php -q
<?php

// Use pfsense GUI config to determine the flags of advanced system options set
require_once("global.inc");
require_once(PFSENSEINCDIR . "config.inc");
require_once(PFSENSEINCDIR . "certs.inc");

/* Use these to output status at the end
$nExitStatus = STATE_OK; // STATE_WARNING STATE_CRITICAL STATE_NEVERALERT
$szExitMessage = array();
*/

// Default minimum warning level
$nCAAlertLvl		= STATE_INFO;
$nCAWarnThreshold	= 31; // Warn days warning before expiration
$nServerCertAlertLvl= STATE_INFO;
$nServerWarnThreshold = 14; // Warn days warning before expiration
$nUserCertAlertLvl	= STATE_INFO;
$nUserWarnThreshold = 7; // Warn days warning before expiration

$bCRLChecks			= true; // Warn if CRL is not existing or outdated
$bIncludeUnused		= false; // Do not check certificates with no current assigned use

// See if one or more are specifically set
$szShortOpt	= "";
$szLongOpt	= array("CAAlert::",
					"CAThres::",
					"ServerAlert::",
					"ServerThres::",
					"UserAlert::",
					"UserThres::",
					"NoCRLWarn::",
					"CheckUnused::");
					
$vOptions = getopt("", $szLongOpt);

if(array_key_exists("CAAlert", $vOptions)) 		{ $nCAAlertLvl			= $vOptions['CAAlert']; }
if(array_key_exists("CAThres", $vOptions)) 		{ $nCAWarnThreshold		= $vOptions['CAThres']; }
if(array_key_exists("ServerAlert", $vOptions))	{ $nServerCertAlertLvl	= $vOptions['ServerAlert']; }
if(array_key_exists("ServerThres", $vOptions))	{ $nServerWarnThreshold	= $vOptions['ServerThres']; }
if(array_key_exists("UserAlert", $vOptions))	{ $nUserCertAlertLvl	= $vOptions['UserAlert']; }
if(array_key_exists("UserThres", $vOptions))	{ $nUserWarnThreshold	= $vOptions['UserThres']; }
if(array_key_exists("NoCRLWarn", $vOptions))	{ $bCRLChecks			= false; }
if(array_key_exists("CheckUnused", $vOptions))	{ $bIncludeUnused		= true; }

// Check CA Expiration
if($nCAAlertLvl <= STATE_INFO)
{
	// Expiration
	CheckExpiration($config['ca'], $nCAWarnThreshold, $nCAAlertLvl);

		// Existence of at least one CRL
	if($bCRLChecks)
	{
		$vCRLList = array();
		if(!empty($config['crl'])) { $vCRLList = $config['crl']; }
		
		if(!empty($vCRLList))
		{
			foreach($config['ca'] AS $vCA)
			{
				CheckCRLExists($vCA, $vCRLList);
			}
		}
	}
}

// Check Server Certificate Expiration
if($nServerCertAlertLvl <= STATE_INFO)
{
	$vCerts = array();
	
	if(!empty($config['cert']))
	{
		foreach($config['cert'] as $Cert):
		{
			if(  (false == is_user_cert($Cert['refid']))
			   &&(false == is_ca_cert($Cert['refid'])))
			{
				$vCerts[] = $Cert;
			}
		}endforeach;
	}
	
	CheckExpiration($vCerts, $nServerWarnThreshold, $nServerCertAlertLvl);
}

// Check User Certificate Expiration
if($nUserCertAlertLvl <= STATE_INFO)
{
	$vCerts = array();
	
	if(!empty($config['cert']))
	{
		foreach($config['cert'] as $Cert):
		{
			if(is_user_cert($Cert['refid']))
			{
				$vCerts[] = $Cert;
			}
		}endforeach;
	}
	
	CheckExpiration($vCerts, $nUserWarnThreshold, $nUserCertAlertLvl);
}

if($bCRLChecks)
{
	/* Do CRL lifetime checks here */
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

// Checks a whole array of certificates for validity. Warns $nWarnDaysBefore
function CheckExpiration($CertList, $nWarnDaysBefore, $nAlertLvl)
{
	if($nAlertLvl < STATE_EMERGENCY) { return; } // Nothing to do

	global $nExitStatus;	// Set exit status here
	global $szExitMessage;	// Set status messages here
	global $bIncludeUnused;
	
	if(count($CertList) > 0)
	{
		foreach($CertList as $vCRT):
		{
			// Skip revoked certificates
			if(is_cert_revoked($vCRT)) { continue; }
			
			// Possibly skip unused certificates
			if(  (false == cert_in_use($vCRT['refid']))
			   &&(!$bIncludeUnused))
			{ continue; }
			
			// Identify 

			// Check validity		
			$nExpiresIn = 0;
			$nCRTValidityState = CheckCertValidity(base64_decode($vCRT['crt']), $nWarnDaysBefore, $nExpiresIn);
			if($nCRTValidityState < STATE_INFO)
			{
				$vX509Crt = openssl_x509_parse(base64_decode($vCRT['crt']), false);

				// Only bother if our alert level is higher
				if($nCRTValidityState < $nAlertLvl)
				{
					// Has expired?
					if($nCRTValidityState == STATE_CRITICAL)
					{
						$szMessage = GetCertificateType($vCRT['refid']) . " has expired: " . $vCRT['descr'] . " (" . $vX509Crt['name'] . ")";
						PushNRPEMessage(STATE_CRITICAL, $szMessage);
					}
					
					// AboutToExpire?
					if($nCRTValidityState == STATE_WARNING)
					{
						$szMessage = GetCertificateType($vCRT['refid']) . " expires in " . round($nExpiresIn,1) . " days: " . $vCRT['descr'] . " (" . $vX509Crt['name'] . ")";
						PushNRPEMessage(STATE_WARNING, $szMessage);
					}
				}
			}
			
		}endforeach;
	}
}

// Checks Certificate type and returns type as string
function GetCertificateType($certref)
{
	if(is_webgui_cert($certref))	{ return "WebGUI"; }
	else if(is_user_cert($certref))	{ return "User"; }
	else if(  (is_openvpn_server_cert($certref))
			||(is_openvpn_client_cert($certref)))
									{ return "OpenVPN"; }
	else if(is_ipsec_cert($certref)){ return "IPSec"; }
	else if(is_ca_cert($certref))	{ return "CA"; }
	else 							{ return "Unknown/Unused certificate"; }
}

// Checks whether a certificate is valid for more than $ValidForNDays from now
// $nExpiresIn is an optional parameter returning in how many days it will expire
// Returns: STATE_OK if			: validity > $ValidForNDays
//			STATE_WARNING if	: 0 < valdity < $ValidForNDays
//			STATE_CRITICAL if	: 0 > $ValidForNDays (expired)
function CheckCertValidity($X509CertData, $nValidForNDays, &$nExpiresIn)
{
	$nRet = STATE_INFO;

	$vCRT = openssl_x509_parse($X509CertData, false);
		// Calculate relative certificate expiration
	$nRelCertExp = $vCRT['validTo_time_t'] - time(); 

	// Critical if negative (already expired)
	if($nRelCertExp < 0)
	{
		// Critical
		$nRet = STATE_CRITICAL;
	}
	// Else see if warningit expires withing the given epoch
	else if($nRelCertExp < ($nValidForNDays * 86400))
	{
		// Warning
		$nRet = STATE_WARNING;
	}
	
	$nExpiresIn = ($nRelCertExp / 86400);
	
	return $nRet;
}

// Checks the validity of a CRL
function CheckCRLExists($vCA, $vCRLList)
{
	global $nExitStatus;	// Set exit status here
	global $szExitMessage;	// Set status messages here
	
	$bFound = false;

	foreach($vCRLList as $vCRL)
	{
		if($vCRL['caref'] == $vCA['refid']) { $bFound = true; break; }
	}
	
	// No valid CRL was found for this CA
	if(!$bFound)
	{
		PushNRPEMessage(STATE_WARNING, "CA " . $vCA['descr'] . " has no CRLs defined");
	}
	
}





?>










