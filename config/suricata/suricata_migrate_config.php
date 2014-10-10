<?php
/*
 * suricata_migrate_config.php
 *
 * Copyright (C) 2014 Bill Meeks
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

require_once("config.inc");
require_once("functions.inc");

/****************************************************************************/
/* The code in this module is called once during the post-install process   */
/* via an "include" line.  It is used to perform a one-time migration of    */
/* Suricata configuration parameters to any new format required by the      */
/* latest package version.                                                  */
/****************************************************************************/

global $config;

if (!is_array($config['installedpackages']['suricata']))
	$config['installedpackages']['suricata'] = array();
if (!is_array($config['installedpackages']['suricata']['rule']))
	$config['installedpackages']['suricata']['rule'] = array();

// Just exit if this is a clean install with no saved settings
if (empty($config['installedpackages']['suricata']['rule']))
	return;

$rule = &$config['installedpackages']['suricata']['rule'];

/****************************************************************************/
/* Loop through all the <rule> elements in the Suricata configuration and   */
/* migrate relevant parameters to the new format.                           */
/****************************************************************************/

$updated_cfg = false;
log_error("[Suricata] Checking configuration settings version...");

// Check the configuration version to see if XMLRPC Sync should
// auto-disabled as part of the upgrade due to config format changes.
if ($config['installedpackages']['suricata']['config'][0]['suricata_config_ver'] < 2 && 
    ($config['installedpackages']['suricatasync']['config'][0]['varsynconchanges'] == 'auto' ||
     $config['installedpackages']['suricatasync']['config'][0]['varsynconchanges'] == 'manual')) {
	$config['installedpackages']['suricatasync']['config'][0]['varsynconchanges'] = "disabled";
	log_error("[Suricata] Turning off Suricata Sync on this host due to configuration format changes in this update.  Upgrade all Suricata Sync targets to this same Suricata package version before re-enabling Suricata Sync.");
	$updated_cfg = true;
}

/**********************************************************/
/* Create new Auto SID Mgmt settings if not set           */
/**********************************************************/
	if (empty($config['installedpackages']['suricata']['config'][0]['auto_manage_sids'])) {
		$config['installedpackages']['suricata']['config'][0]['auto_manage_sids'] = "off";
		$config['installedpackages']['suricata']['config'][0]['sid_changes_log_limit_size'] = "250";
		$config['installedpackages']['suricata']['config'][0]['sid_changes_log_retention'] = "336";
		$updated_cfg = true;
	}

// Now process the interface-specific settings
foreach ($rule as &$r) {

	// Initialize arrays for supported preprocessors if necessary
	if (!is_array($r['libhtp_policy']['item']))
		$r['libhtp_policy']['item'] = array();

	$pconfig = array();
	$pconfig = $r;

	/***********************************************************/
	/* This setting is deprecated in Suricata 2.0 and higher,  */
	/* so remove it from the configuration.                    */
	/***********************************************************/
	if (isset($pconfig['stream_max_sessions'])) {
		unset($pconfig['stream_max_sessions']);
		$updated_cfg = true;
	}

	/***********************************************************/
	/* HTTP server personalities for "Apache" and "Apache_2_2" */
	/* are deprecated and replaced with "Apache_2" in Suricata */
	/* versions greater than 2.0.                              */
	/***********************************************************/
	$http_serv = &$pconfig['libhtp_policy']['item'];
	foreach ($http_serv as &$policy) {
		if ($policy['personality'] == "Apache" || $policy['personality'] == "Apache_2_2") {
			$policy['personality'] = "Apache_2";
			$updated_cfg = true;
		}
		// Set new URI inspect option for Suricata 2.0 and higher
		if (!isset($policy['uri-include-all'])) {
			$policy['uri-include-all'] = "no";
			$updated_cfg = true;
		}
	}

	/***********************************************************/
	/* Add the new 'dns-events.rules' file to the rulesets.    */
	/***********************************************************/
	if (strpos($pconfig['rulesets'], "dns-events.rules") === FALSE) {
		$pconfig['rulesets'] = rtrim($pconfig['rulesets'], "||") . "||dns-events.rules";	
		$updated_cfg = true;
	}

	/***********************************************************/
	/* Add new interface promisc mode value and default 'on'.  */
	/***********************************************************/
	if (empty($pconfig['intf_promisc_mode'])) {
		$pconfig['intf_promisc_mode'] = "on";
		$updated_cfg = true;
	}

	/***********************************************************/
	/* Add new HTTP Log Extended Info setting if not present   */
	/***********************************************************/
	if (!isset($pconfig['http_log_extended'])) {
		$pconfig['http_log_extended'] = "on";
		$updated_cfg = true;
	}

	/***********************************************************/
	/* Add new EVE logging settings if not present             */
	/***********************************************************/
	if (!isset($pconfig['eve_output_type'])) {
		$pconfig['eve_output_type'] = "file";
		$updated_cfg = true;
	}
	if (empty($pconfig['eve_systemlog_facility'])) {
		$pconfig['eve_systemlog_facility'] = "local1";
		$updated_cfg = true;
	}
	if (empty($pconfig['eve_systemlog_priority'])) {
		$pconfig['eve_systemlog_priority'] = "info";
		$updated_cfg = true;
	}
	if (!isset($pconfig['eve_log_alerts'])) {
		$pconfig['eve_log_alerts'] = "on";
		$updated_cfg = true;
	}
	if (!isset($pconfig['eve_log_http'])) {
		$pconfig['eve_log_http'] = "on";
		$updated_cfg = true;
	}
	if (!isset($pconfig['eve_log_dns'])) {
		$pconfig['eve_log_dns'] = "on";
		$updated_cfg = true;
	}
	if (!isset($pconfig['eve_log_tls'])) {
		$pconfig['eve_log_tls'] = "on";
		$updated_cfg = true;
	}
	if (!isset($pconfig['eve_log_files'])) {
		$pconfig['eve_log_files'] = "on";
		$updated_cfg = true;
	}
	if (!isset($pconfig['eve_log_ssh'])) {
		$pconfig['eve_log_ssh'] = "on";
		$updated_cfg = true;
	}

	/******************************************************************/
	/* Create default log size and retention limits if not set        */
	/******************************************************************/
	if (!isset($pconfig['alert_log_retention']) && $pconfig['alert_log_retention'] != '0') {
		$pconfig['alert_log_retention'] = "336";
		$updated_cfg = true;
	}
	if (!isset($pconfig['alert_log_limit_size']) && $pconfig['alert_log_limit_size'] != '0') {
		$pconfig['alert_log_limit_size'] = "500";
		$updated_cfg = true;
	}

	if (!isset($pconfig['block_log_retention']) && $pconfig['block_log_retention'] != '0') {
		$pconfig['block_log_retention'] = "336";
		$updated_cfg = true;
	}
	if (!isset($pconfig['block_log_limit_size']) && $pconfig['block_log_limit_size'] != '0') {
		$pconfig['block_log_limit_size'] = "500";
		$updated_cfg = true;
	}

	if (!isset($pconfig['dns_log_retention']) && $pconfig['dns_log_retention'] != '0') {
		$pconfig['dns_log_retention'] = "168";
		$updated_cfg = true;
	}
	if (!isset($pconfig['dns_log_limit_size']) && $pconfig['dns_log_limit_size'] != '0') {
		$pconfig['dns_log_limit_size'] = "750";
		$updated_cfg = true;
	}

	if (!isset($pconfig['eve_log_retention']) && $pconfig['eve_log_retention'] != '0') {
		$pconfig['eve_log_retention'] = "168";
		$updated_cfg = true;
	}
	if (!isset($pconfig['eve_log_limit_size']) && $pconfig['eve_log_limit_size'] != '0') {
		$pconfig['eve_log_limit_size'] = "5000";
		$updated_cfg = true;
	}

	if (!isset($pconfig['files_json_log_retention']) && $pconfig['files_json_log_retention'] != '0') {
		$pconfig['files_json_log_retention'] = "168";
		$updated_cfg = true;
	}
	if (!isset($pconfig['files_json_log_limit_size']) && $pconfig['files_json_log_limit_size'] != '0') {
		$pconfig['files_json_log_limit_size'] = "1000";
		$updated_cfg = true;
	}

	if (!isset($pconfig['http_log_retention']) && $pconfig['http_log_retention'] != '0') {
		$pconfig['http_log_retention'] = "168";
		$updated_cfg = true;
	}
	if (!isset($pconfig['http_log_limit_size']) && $pconfig['http_log_limit_size'] != '0') {
		$pconfig['http_log_limit_size'] = "1000";
		$updated_cfg = true;
	}

	if (!isset($pconfig['stats_log_retention']) && $pconfig['stats_log_retention'] != '0') {
		$pconfig['stats_log_retention'] = "168";
		$updated_cfg = true;
	}
	if (!isset($pconfig['stats_log_limit_size']) && $pconfig['stats_log_limit_size'] != '0') {
		$pconfig['stats_log_limit_size'] = "500";
		$updated_cfg = true;
	}

	if (!isset($pconfig['tls_log_retention']) && $pconfig['tls_log_retention'] != '0') {
		$pconfig['tls_log_retention'] = "336";
		$updated_cfg = true;
	}
	if (!isset($pconfig['tls_log_limit_size']) && $pconfig['tls_log_limit_size'] != '0') {
		$pconfig['tls_log_limit_size'] = "500";
		$updated_cfg = true;
	}

	if (!isset($pconfig['file_store_retention']) && $pconfig['file_store_retention'] != '0') {
		$pconfig['file_store_retention'] = "168";
		$updated_cfg = true;
	}

	if (!isset($pconfig['u2_archive_log_retention']) && $pconfig['u2_archive_log_retention'] != '0') {
		$pconfig['u2_archive_log_retention'] = "168";
		$updated_cfg = true;
	}

	/************************************************************/
	/* Create new DNS App-Layer parser settings if not set      */
	/************************************************************/
	if (empty($pconfig['dns_global_memcap'])) {
		$pconfig['dns_global_memcap'] = "16777216";
		$updated_cfg = true;
	}
	if (empty($pconfig['dns_state_memcap'])) {
		$pconfig['dns_state_memcap'] = "524288";
		$updated_cfg = true;
	}
	if (empty($pconfig['dns_request_flood_limit'])) {
		$pconfig['dns_request_flood_limit'] = "500";
		$updated_cfg = true;
	}
	if (empty($pconfig['dns_parser_udp'])) {
		$pconfig['dns_parser_udp'] = "yes";
		$updated_cfg = true;
	}
	if (empty($pconfig['dns_parser_tcp'])) {
		$pconfig['dns_parser_tcp'] = "yes";
		$updated_cfg = true;
	}

	/***********************************************************/
	/* Create new HTTP App-Layer parser settings if not set    */
	/***********************************************************/
	if (empty($pconfig['http_parser'])) {
		$pconfig['http_parser'] = "yes";
		$updated_cfg = true;
	}
	if (empty($pconfig['http_parser_memcap'])) {
		$pconfig['http_parser_memcap'] = "67108864";
		$updated_cfg = true;
	}

	/**********************************************************/
	/* Create other App-Layer parser settings if not set      */
	/**********************************************************/
	if (empty($pconfig['tls_parser'])) {
		$pconfig['tls_parser'] = "yes";
		$updated_cfg = true;
	}
	if (empty($pconfig['smtp_parser'])) {
		$pconfig['smtp_parser'] = "yes";
		$updated_cfg = true;
	}
	if (empty($pconfig['imap_parser'])) {
		$pconfig['imap_parser'] = "detection-only";
		$updated_cfg = true;
	}
	if (empty($pconfig['ssh_parser'])) {
		$pconfig['ssh_parser'] = "yes";
		$updated_cfg = true;
	}
	if (empty($pconfig['ftp_parser'])) {
		$pconfig['ftp_parser'] = "yes";
		$updated_cfg = true;
	}
	if (empty($pconfig['dcerpc_parser'])) {
		$pconfig['dcerpc_parser'] = "yes";
		$updated_cfg = true;
	}
	if (empty($pconfig['smb_parser'])) {
		$pconfig['smb_parser'] = "yes";
		$updated_cfg = true;
	}
	if (empty($pconfig['msn_parser'])) {
		$pconfig['msn_parser'] = "detection-only";
		$updated_cfg = true;
	}

	// Save the new configuration data into the $config array pointer
	$r = $pconfig;
}
// Release reference to final array element
unset($r);

// Write out the new configuration to disk if we changed anything
if ($updated_cfg)
	log_error("[Suricata] Settings successfully migrated to new configuration format...");
else
	log_error("[Suricata] Configuration version is current...");

?>
