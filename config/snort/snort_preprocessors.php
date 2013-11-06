<?php
/*
 * snort_preprocessors.php
 * part of pfSense
 *
 * Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 * Copyright (C) 2008-2009 Robert Zelaya.
 * Copyright (C) 2011-2012 Ermal Luci
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


require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort.inc");

global $g, $rebuild_rules;
$snortlogdir = SNORTLOGDIR;

if (!is_array($config['installedpackages']['snortglobal'])) {
	$config['installedpackages']['snortglobal'] = array();
}
$vrt_enabled = $config['installedpackages']['snortglobal']['snortdownload'];

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (is_null($id)) {
        header("Location: /snort/snort_interfaces.php");
        exit;
}

$pconfig = array();
if (isset($id) && $a_nat[$id]) {
	$pconfig = $a_nat[$id];

	/* Get current values from config for page form fields */
	$pconfig['perform_stat'] = $a_nat[$id]['perform_stat'];
	$pconfig['host_attribute_table'] = $a_nat[$id]['host_attribute_table'];
	$pconfig['host_attribute_data'] = $a_nat[$id]['host_attribute_data'];
	$pconfig['max_attribute_hosts'] = $a_nat[$id]['max_attribute_hosts'];
	$pconfig['max_attribute_services_per_host'] = $a_nat[$id]['max_attribute_services_per_host'];
	$pconfig['max_paf'] = $a_nat[$id]['max_paf'];
	$pconfig['server_flow_depth'] = $a_nat[$id]['server_flow_depth'];
	$pconfig['http_server_profile'] = $a_nat[$id]['http_server_profile'];
	$pconfig['client_flow_depth'] = $a_nat[$id]['client_flow_depth'];
	$pconfig['stream5_reassembly'] = $a_nat[$id]['stream5_reassembly'];
	$pconfig['stream5_require_3whs'] = $a_nat[$id]['stream5_require_3whs'];
	$pconfig['stream5_track_tcp'] = $a_nat[$id]['stream5_track_tcp'];
	$pconfig['stream5_track_udp'] = $a_nat[$id]['stream5_track_udp'];
	$pconfig['stream5_track_icmp'] = $a_nat[$id]['stream5_track_icmp'];
	$pconfig['max_queued_bytes'] = $a_nat[$id]['max_queued_bytes'];
	$pconfig['max_queued_segs'] = $a_nat[$id]['max_queued_segs'];
	$pconfig['stream5_overlap_limit'] = $a_nat[$id]['stream5_overlap_limit'];
	$pconfig['stream5_policy'] = $a_nat[$id]['stream5_policy'];
	$pconfig['stream5_mem_cap'] = $a_nat[$id]['stream5_mem_cap'];
	$pconfig['stream5_tcp_timeout'] = $a_nat[$id]['stream5_tcp_timeout'];
	$pconfig['stream5_udp_timeout'] = $a_nat[$id]['stream5_udp_timeout'];
	$pconfig['stream5_icmp_timeout'] = $a_nat[$id]['stream5_icmp_timeout'];
	$pconfig['stream5_no_reassemble_async'] = $a_nat[$id]['stream5_no_reassemble_async'];
	$pconfig['stream5_dont_store_lg_pkts'] = $a_nat[$id]['stream5_dont_store_lg_pkts'];
	$pconfig['http_inspect'] = $a_nat[$id]['http_inspect'];
	$pconfig['http_inspect_memcap'] = $a_nat[$id]['http_inspect_memcap'];
	$pconfig['http_inspect_enable_xff'] = $a_nat[$id]['http_inspect_enable_xff'];
	$pconfig['http_inspect_log_uri'] = $a_nat[$id]['http_inspect_log_uri'];
	$pconfig['http_inspect_log_hostname'] = $a_nat[$id]['http_inspect_log_hostname'];
	$pconfig['noalert_http_inspect'] = $a_nat[$id]['noalert_http_inspect'];
	$pconfig['other_preprocs'] = $a_nat[$id]['other_preprocs'];
	$pconfig['ftp_preprocessor'] = $a_nat[$id]['ftp_preprocessor'];
	$pconfig['smtp_preprocessor'] = $a_nat[$id]['smtp_preprocessor'];
	$pconfig['sf_portscan'] = $a_nat[$id]['sf_portscan'];
	$pconfig['pscan_protocol'] = $a_nat[$id]['pscan_protocol'];
	$pconfig['pscan_type'] = $a_nat[$id]['pscan_type'];
	$pconfig['pscan_sense_level'] = $a_nat[$id]['pscan_sense_level'];
	$pconfig['pscan_memcap'] = $a_nat[$id]['pscan_memcap'];
	$pconfig['pscan_ignore_scanners'] = $a_nat[$id]['pscan_ignore_scanners'];
	$pconfig['dce_rpc_2'] = $a_nat[$id]['dce_rpc_2'];
	$pconfig['dns_preprocessor'] = $a_nat[$id]['dns_preprocessor'];
	$pconfig['sensitive_data'] = $a_nat[$id]['sensitive_data'];
	$pconfig['ssl_preproc'] = $a_nat[$id]['ssl_preproc'];
	$pconfig['pop_preproc'] = $a_nat[$id]['pop_preproc'];
	$pconfig['imap_preproc'] = $a_nat[$id]['imap_preproc'];
	$pconfig['sip_preproc'] = $a_nat[$id]['sip_preproc'];
	$pconfig['dnp3_preproc'] = $a_nat[$id]['dnp3_preproc'];
	$pconfig['modbus_preproc'] = $a_nat[$id]['modbus_preproc'];
	$pconfig['gtp_preproc'] = $a_nat[$id]['gtp_preproc'];
	$pconfig['ssh_preproc'] = $a_nat[$id]['ssh_preproc'];
	$pconfig['preproc_auto_rule_disable'] = $a_nat[$id]['preproc_auto_rule_disable'];
	$pconfig['protect_preproc_rules'] = $a_nat[$id]['protect_preproc_rules'];
	$pconfig['frag3_detection'] = $a_nat[$id]['frag3_detection'];
	$pconfig['frag3_overlap_limit'] = $a_nat[$id]['frag3_overlap_limit'];
	$pconfig['frag3_min_frag_len'] = $a_nat[$id]['frag3_min_frag_len'];
	$pconfig['frag3_policy'] = $a_nat[$id]['frag3_policy'];
	$pconfig['frag3_max_frags'] = $a_nat[$id]['frag3_max_frags'];
	$pconfig['frag3_memcap'] = $a_nat[$id]['frag3_memcap'];
	$pconfig['frag3_timeout'] = $a_nat[$id]['frag3_timeout'];

	/* If not using the Snort VRT rules, then disable */
	/* the Sensitive Data (sdf) preprocessor.         */
	if ($vrt_enabled == "off")
		$pconfig['sensitive_data'] = "off";

	/************************************************************/
	/* To keep new users from shooting themselves in the foot   */
	/* enable the most common required preprocessors by default */
	/* and set reasonable values for any options.               */
	/************************************************************/
	if (empty($pconfig['max_attribute_hosts']))
		$pconfig['max_attribute_hosts'] = '10000';
	if (empty($pconfig['max_attribute_services_per_host']))
		$pconfig['max_attribute_services_per_host'] = '10';
	if (empty($pconfig['max_paf']))
		$pconfig['max_paf'] = '16000';
	if (empty($pconfig['ftp_preprocessor']))
		$pconfig['ftp_preprocessor'] = 'on';
	if (empty($pconfig['smtp_preprocessor']))
		$pconfig['smtp_preprocessor'] = 'on';
	if (empty($pconfig['dce_rpc_2']))
		$pconfig['dce_rpc_2'] = 'on';
	if (empty($pconfig['dns_preprocessor']))
		$pconfig['dns_preprocessor'] = 'on';
	if (empty($pconfig['ssl_preproc']))
		$pconfig['ssl_preproc'] = 'on';
	if (empty($pconfig['pop_preproc']))
		$pconfig['pop_preproc'] = 'on';
	if (empty($pconfig['imap_preproc']))
		$pconfig['imap_preproc'] = 'on';
	if (empty($pconfig['sip_preproc']))
		$pconfig['sip_preproc'] = 'on';
	if (empty($pconfig['other_preprocs']))
		$pconfig['other_preprocs'] = 'on';
	if (empty($pconfig['ssh_preproc']))
		$pconfig['ssh_preproc'] = 'on';
	if (empty($pconfig['http_inspect_memcap']))
		$pconfig['http_inspect_memcap'] = "150994944";
	if (empty($pconfig['frag3_overlap_limit']))
		$pconfig['frag3_overlap_limit'] = '0';
	if (empty($pconfig['frag3_min_frag_len']))
		$pconfig['frag3_min_frag_len'] = '0';
	if (empty($pconfig['frag3_max_frags']))
		$pconfig['frag3_max_frags'] = '8192';
	if (empty($pconfig['frag3_policy']))
		$pconfig['frag3_policy'] = 'bsd';
	if (empty($pconfig['frag3_memcap']))
		$pconfig['frag3_memcap'] = '4194304';
	if (empty($pconfig['frag3_timeout']))
		$pconfig['frag3_timeout'] = '60';
	if (empty($pconfig['frag3_detection']))
		$pconfig['frag3_detection'] = 'on';
	if (empty($pconfig['stream5_reassembly']))
		$pconfig['stream5_reassembly'] = 'on';
	if (empty($pconfig['stream5_track_tcp']))
		$pconfig['stream5_track_tcp'] = 'on';
	if (empty($pconfig['stream5_track_udp']))
		$pconfig['stream5_track_udp'] = 'on';
	if (empty($pconfig['stream5_track_icmp']))
		$pconfig['stream5_track_icmp'] = 'off';
	if (empty($pconfig['stream5_require_3whs']))
		$pconfig['stream5_require_3whs'] = 'off';
	if (empty($pconfig['stream5_overlap_limit']))
		$pconfig['stream5_overlap_limit'] = '0';
	if (empty($pconfig['stream5_tcp_timeout']))
		$pconfig['stream5_tcp_timeout'] = '30';
	if (empty($pconfig['stream5_udp_timeout']))
		$pconfig['stream5_udp_timeout'] = '30';
	if (empty($pconfig['stream5_icmp_timeout']))
		$pconfig['stream5_icmp_timeout'] = '30';
	if (empty($pconfig['stream5_no_reassemble_async']))
		$pconfig['stream5_no_reassemble_async'] = 'off';
	if (empty($pconfig['stream5_dont_store_lg_pkts']))
		$pconfig['stream5_dont_store_lg_pkts'] = 'off';
	if (empty($pconfig['stream5_policy']))
		$pconfig['stream5_policy'] = 'bsd';
	if (empty($pconfig['pscan_protocol']))
		$pconfig['pscan_protocol'] = 'all';
	if (empty($pconfig['pscan_type']))
		$pconfig['pscan_type'] = 'all';
	if (empty($pconfig['pscan_memcap']))
		$pconfig['pscan_memcap'] = '10000000';
	if (empty($pconfig['pscan_sense_level']))
		$pconfig['pscan_sense_level'] = 'medium';
}

/* Define the "disabled_preproc_rules.log" file for this interface */
$iface = snort_get_friendly_interface($pconfig['interface']);
$disabled_rules_log = "{$snortlogdir}/{$iface}_disabled_preproc_rules.log";

if ($_POST['ResetAll']) {

	/* Reset all the preprocessor settings to defaults */
	$pconfig['perform_stat'] = "off";
	$pconfig['host_attribute_table'] = "off";
	$pconfig['max_attribute_hosts'] = '10000';
	$pconfig['max_attribute_services_per_host'] = '10';
	$pconfig['max_paf'] = '16000';
	$pconfig['server_flow_depth'] = "300";
	$pconfig['http_server_profile'] = "all";
	$pconfig['client_flow_depth'] = "300";
	$pconfig['stream5_reassembly'] = "on";
	$pconfig['stream5_require_3whs'] = "off";
	$pconfig['stream5_track_tcp'] = "on";
	$pconfig['stream5_track_udp'] = "on";
	$pconfig['stream5_track_icmp'] = "off";
	$pconfig['max_queued_bytes'] = "1048576";
	$pconfig['max_queued_segs'] = "2621";
	$pconfig['stream5_overlap_limit'] = "0";
	$pconfig['stream5_policy'] = "bsd";
	$pconfig['stream5_mem_cap'] = "8388608";
	$pconfig['stream5_tcp_timeout'] = "30";
	$pconfig['stream5_udp_timeout'] = "30";
	$pconfig['stream5_icmp_timeout'] = "30";
	$pconfig['stream5_no_reassemble_async'] = "off";
	$pconfig['stream5_dont_store_lg_pkts'] = "off";
	$pconfig['http_inspect'] = "on";
	$pconfig['http_inspect_enable_xff'] = "off";
	$pconfig['http_inspect_log_uri'] = "off";
	$pconfig['http_inspect_log_hostname'] = "off";
	$pconfig['noalert_http_inspect'] = "on";
	$pconfig['http_inspect_memcap'] = "150994944";
	$pconfig['other_preprocs'] = "on";
	$pconfig['ftp_preprocessor'] = "on";
	$pconfig['smtp_preprocessor'] = "on";
	$pconfig['sf_portscan'] = "off";
	$pconfig['pscan_protocol'] = "all";
	$pconfig['pscan_type'] = "all";
	$pconfig['pscan_sense_level'] = "medium";
	$pconfig['pscan_ignore_scanners'] = "";
	$pconfig['pscan_memcap'] = '10000000';
	$pconfig['dce_rpc_2'] = "on";
	$pconfig['dns_preprocessor'] = "on";
	$pconfig['sensitive_data'] = "off";
	$pconfig['ssl_preproc'] = "on";
	$pconfig['pop_preproc'] = "on";
	$pconfig['imap_preproc'] = "on";
	$pconfig['sip_preproc'] = "on";
	$pconfig['dnp3_preproc'] = "off";
	$pconfig['modbus_preproc'] = "off";
	$pconfig['gtp_preproc'] = "off";
	$pconfig['ssh_preproc'] = "on";
	$pconfig['preproc_auto_rule_disable'] = "off";
	$pconfig['protect_preproc_rules'] = "off";
	$pconfig['frag3_detection'] = "on";
	$pconfig['frag3_overlap_limit'] = "0";
	$pconfig['frag3_min_frag_len'] = "0";
	$pconfig['frag3_policy'] = "bsd";
	$pconfig['frag3_max_frags'] = "8192";
	$pconfig['frag3_memcap'] = "4194304";
	$pconfig['frag3_timeout'] = "60";

	/* Log a message at the top of the page to inform the user */
	$savemsg = "All preprocessor settings have been reset to the defaults.";
}
elseif ($_POST['Submit']) {
	$natent = array();
	$natent = $pconfig;

	if ($_POST['pscan_ignore_scanners'] && !is_alias($_POST['pscan_ignore_scanners']))
		$input_errors[] = "Only aliases are allowed for the Portscan IGNORE_SCANNERS option.";

	/* if no errors write to conf */
	if (!$input_errors) {
		/* post new options */
		if ($_POST['max_attribute_hosts'] != "") { $natent['max_attribute_hosts'] = $_POST['max_attribute_hosts']; }else{ $natent['max_attribute_hosts'] = "10000"; }
		if ($_POST['max_attribute_services_per_host'] != "") { $natent['max_attribute_services_per_host'] = $_POST['max_attribute_services_per_host']; }else{ $natent['max_attribute_services_per_host'] = "10"; }
		if ($_POST['max_paf'] != "") { $natent['max_paf'] = $_POST['max_paf']; }else{ $natent['max_paf'] = "16000"; }
		if ($_POST['server_flow_depth'] != "") { $natent['server_flow_depth'] = $_POST['server_flow_depth']; }else{ $natent['server_flow_depth'] = "300"; }
		if ($_POST['http_server_profile'] != "") { $natent['http_server_profile'] = $_POST['http_server_profile']; }else{ $natent['http_server_profile'] = "all"; }
		if ($_POST['client_flow_depth'] != "") { $natent['client_flow_depth'] = $_POST['client_flow_depth']; }else{ $natent['client_flow_depth'] = "300"; }
		if ($_POST['http_inspect_memcap'] != "") { $natent['http_inspect_memcap'] = $_POST['http_inspect_memcap']; }else{ $natent['http_inspect_memcap'] = "150994944"; }
		if ($_POST['stream5_overlap_limit'] != "") { $natent['stream5_overlap_limit'] = $_POST['stream5_overlap_limit']; }else{ $natent['stream5_overlap_limit'] = "0"; }
		if ($_POST['stream5_policy'] != "") { $natent['stream5_policy'] = $_POST['stream5_policy']; }else{ $natent['stream5_policy'] = "bsd"; }
		if ($_POST['stream5_mem_cap'] != "") { $natent['stream5_mem_cap'] = $_POST['stream5_mem_cap']; }else{ $natent['stream5_mem_cap'] = "8388608"; }
		if ($_POST['stream5_tcp_timeout'] != "") { $natent['stream5_tcp_timeout'] = $_POST['stream5_tcp_timeout']; }else{ $natent['stream5_tcp_timeout'] = "30"; }
		if ($_POST['stream5_udp_timeout'] != "") { $natent['stream5_udp_timeout'] = $_POST['stream5_udp_timeout']; }else{ $natent['stream5_udp_timeout'] = "30"; }
		if ($_POST['stream5_icmp_timeout'] != "") { $natent['stream5_icmp_timeout'] = $_POST['stream5_icmp_timeout']; }else{ $natent['stream5_icmp_timeout'] = "30"; }
		if ($_POST['max_queued_bytes'] != "") { $natent['max_queued_bytes'] = $_POST['max_queued_bytes']; }else{ $natent['max_queued_bytes'] = "1048576"; }
		if ($_POST['max_queued_segs'] != "") { $natent['max_queued_segs'] = $_POST['max_queued_segs']; }else{ $natent['max_queued_segs'] = "2621"; }
		if ($_POST['pscan_protocol'] != "") { $natent['pscan_protocol'] = $_POST['pscan_protocol']; }else{ $natent['pscan_protocol'] = "all"; }
		if ($_POST['pscan_type'] != "") { $natent['pscan_type'] = $_POST['pscan_type']; }else{ $natent['pscan_type'] = "all"; }
		if ($_POST['pscan_memcap'] != "") { $natent['pscan_memcap'] = $_POST['pscan_memcap']; }else{ $natent['pscan_memcap'] = "10000000"; }
		if ($_POST['pscan_sense_level'] != "") { $natent['pscan_sense_level'] = $_POST['pscan_sense_level']; }else{ $natent['pscan_sense_level'] = "medium"; }
		if ($_POST['frag3_overlap_limit'] != "") { $natent['frag3_overlap_limit'] = $_POST['frag3_overlap_limit']; }else{ $natent['frag3_overlap_limit'] = "0"; }
		if ($_POST['frag3_min_frag_len'] != "") { $natent['frag3_min_frag_len'] = $_POST['frag3_min_frag_len']; }else{ $natent['frag3_min_frag_len'] = "0"; }
		if ($_POST['frag3_policy'] != "") { $natent['frag3_policy'] = $_POST['frag3_policy']; }else{ $natent['frag3_policy'] = "bsd"; }
		if ($_POST['frag3_max_frags'] != "") { $natent['frag3_max_frags'] = $_POST['frag3_max_frags']; }else{ $natent['frag3_max_frags'] = "8192"; }
		if ($_POST['frag3_memcap'] != "") { $natent['frag3_memcap'] = $_POST['frag3_memcap']; }else{ $natent['frag3_memcap'] = "4194304"; }
		if ($_POST['frag3_timeout'] != "") { $natent['frag3_timeout'] = $_POST['frag3_timeout']; }else{ $natent['frag3_timeout'] = "60"; }

		if ($_POST['pscan_ignore_scanners'])
			$natent['pscan_ignore_scanners'] = $_POST['pscan_ignore_scanners'];
		else
			unset($natent['pscan_ignore_scanners']);

		$natent['perform_stat'] = $_POST['perform_stat'] ? 'on' : 'off';
		$natent['host_attribute_table'] = $_POST['host_attribute_table'] ? 'on' : 'off';
		$natent['http_inspect'] = $_POST['http_inspect'] ? 'on' : 'off';
		$natent['http_inspect_enable_xff'] = $_POST['http_inspect_enable_xff'] ? 'on' : 'off';
		$natent['http_inspect_log_uri'] = $_POST['http_inspect_log_uri'] ? 'on' : 'off';
		$natent['http_inspect_log_hostname'] = $_POST['http_inspect_log_hostname'] ? 'on' : 'off';
		$natent['noalert_http_inspect'] = $_POST['noalert_http_inspect'] ? 'on' : 'off';
		$natent['other_preprocs'] = $_POST['other_preprocs'] ? 'on' : 'off';
		$natent['ftp_preprocessor'] = $_POST['ftp_preprocessor'] ? 'on' : 'off';
		$natent['smtp_preprocessor'] = $_POST['smtp_preprocessor'] ? 'on' : 'off';
		$natent['sf_portscan'] = $_POST['sf_portscan'] ? 'on' : 'off';
		$natent['dce_rpc_2'] = $_POST['dce_rpc_2'] ? 'on' : 'off';
		$natent['dns_preprocessor'] = $_POST['dns_preprocessor'] ? 'on' : 'off';
		$natent['sensitive_data'] = $_POST['sensitive_data'] ? 'on' : 'off';
		$natent['ssl_preproc'] = $_POST['ssl_preproc'] ? 'on' : 'off';
		$natent['pop_preproc'] = $_POST['pop_preproc'] ? 'on' : 'off';
		$natent['imap_preproc'] = $_POST['imap_preproc'] ? 'on' : 'off';
		$natent['dnp3_preproc'] = $_POST['dnp3_preproc'] ? 'on' : 'off';
		$natent['modbus_preproc'] = $_POST['modbus_preproc'] ? 'on' : 'off';
		$natent['sip_preproc'] = $_POST['sip_preproc'] ? 'on' : 'off';
		$natent['modbus_preproc'] = $_POST['modbus_preproc'] ? 'on' : 'off';
		$natent['gtp_preproc'] = $_POST['gtp_preproc'] ? 'on' : 'off';
		$natent['ssh_preproc'] = $_POST['ssh_preproc'] ? 'on' : 'off';
		$natent['preproc_auto_rule_disable'] = $_POST['preproc_auto_rule_disable'] ? 'on' : 'off';
		$natent['protect_preproc_rules'] = $_POST['protect_preproc_rules'] ? 'on' : 'off';
		$natent['frag3_detection'] = $_POST['frag3_detection'] ? 'on' : 'off';
		$natent['stream5_reassembly'] = $_POST['stream5_reassembly'] ? 'on' : 'off';
		$natent['stream5_track_tcp'] = $_POST['stream5_track_tcp'] ? 'on' : 'off';
		$natent['stream5_track_udp'] = $_POST['stream5_track_udp'] ? 'on' : 'off';
		$natent['stream5_track_icmp'] = $_POST['stream5_track_icmp'] ? 'on' : 'off';
		$natent['stream5_require_3whs'] = $_POST['stream5_require_3whs'] ? 'on' : 'off';
		$natent['stream5_no_reassemble_async'] = $_POST['stream5_no_reassemble_async'] ? 'on' : 'off';
		$natent['stream5_dont_store_lg_pkts'] = $_POST['stream5_dont_store_lg_pkts'] ? 'on' : 'off';

		/* If 'preproc_auto_rule_disable' is off, then clear log file */
		if ($natent['preproc_auto_rule_disable'] == 'off')
			@unlink("{$disabled_rules_log}");

		if (isset($id) && $a_nat[$id])
			$a_nat[$id] = $natent;
		else {
			if (is_numeric($after))
				array_splice($a_nat, $after+1, 0, array($natent));
			else
				$a_nat[] = $natent;
		}

		write_config();

		/* Set flag to rebuild rules for this interface  */
		$rebuild_rules = true;

		/*************************************************/
		/* Update the snort.conf file and rebuild the    */
		/* rules for this interface.                     */
		/*************************************************/
		snort_generate_conf($natent);
		$rebuild_rules = false;

		/*******************************************************/
		/* Signal Snort to reload Host Attribute Table if one  */
		/* is configured and saved.                            */
		/*******************************************************/
		if ($natent['host_attribute_table'] == "on" && 
		    !empty($natent['host_attribute_data']))
			snort_reload_config($natent, "SIGURG");

		/* after click go to this page */
		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		header("Location: snort_preprocessors.php?id=$id");
		exit;
	}
}
elseif ($_POST['btn_import']) {
	if (is_uploaded_file($_FILES['host_attribute_file']['tmp_name'])) {
		$data = file_get_contents($_FILES['host_attribute_file']['tmp_name']);
		if ($data === false)
			$input_errors[] = gettext("Error uploading file {$_FILES['host_attribute_file']}!");
		else {
			if (isset($id) && $a_nat[$id]) {
				$a_nat[$id]['host_attribute_table'] = "on";
				$a_nat[$id]['host_attribute_data'] = base64_encode($data);
				$pconfig['host_attribute_data'] = $a_nat[$id]['host_attribute_data'];
				$a_nat[$id]['max_attribute_hosts'] = $pconfig['max_attribute_hosts'];
				$a_nat[$id]['max_attribute_services_per_host'] = $pconfig['max_attribute_services_per_host'];
				write_config();
			}
			header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
			header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
			header( 'Cache-Control: no-store, no-cache, must-revalidate' );
			header( 'Cache-Control: post-check=0, pre-check=0', false );
			header( 'Pragma: no-cache' );
			header("Location: snort_preprocessors.php?id=$id");
			exit;
		}
	}
	else
		$input_errors[] = gettext("No filename specified for import!");
}
elseif ($_POST['btn_edit_hat']) {
	if (isset($id) && $a_nat[$id]) {
		$a_nat[$id]['host_attribute_table'] = "on";
		$a_nat[$id]['max_attribute_hosts'] = $pconfig['max_attribute_hosts'];
		$a_nat[$id]['max_attribute_services_per_host'] = $pconfig['max_attribute_services_per_host'];
		write_config();
		header("Location: snort_edit_hat_data.php?id=$id");
		exit;
	}
}

/* If Host Attribute Table option is enabled, but */
/* no Host Attribute data exists, flag an error.  */
if ($pconfig['host_attribute_table'] == 'on' && empty($pconfig['host_attribute_data']))
	$input_errors[] = gettext("The Host Attribute Table option is enabled, but no Host Attribute data has been loaded.  Data may be entered manually or imported from a suitable file.");

$if_friendly = snort_get_friendly_interface($pconfig['interface']);
$pgtitle = "Snort: Interface {$if_friendly}: Preprocessors and Flow";
include_once("head.inc");
?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="enable_change_all()">

<?php include("fbegin.inc"); ?>
<?php if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}


	/* Display Alert message */

	if ($input_errors) {
		print_input_errors($input_errors); // TODO: add checks
	}

	if ($savemsg) {
		print_info_box($savemsg);
	}

?>

<script type="text/javascript" src="/javascript/autosuggest.js">
</script>
<script type="text/javascript" src="/javascript/suggestions.js">
</script>

<form action="snort_preprocessors.php" method="post"
	enctype="multipart/form-data" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
		$tab_array = array();
		$tab_array[0] = array(gettext("Snort Interfaces"), true, "/snort/snort_interfaces.php");
		$tab_array[1] = array(gettext("Global Settings"), false, "/snort/snort_interfaces_global.php");
		$tab_array[2] = array(gettext("Updates"), false, "/snort/snort_download_updates.php");
		$tab_array[3] = array(gettext("Alerts"), false, "/snort/snort_alerts.php");
		$tab_array[4] = array(gettext("Blocked"), false, "/snort/snort_blocked.php");
		$tab_array[5] = array(gettext("Whitelists"), false, "/snort/snort_interfaces_whitelist.php");
		$tab_array[6] = array(gettext("Suppress"), false, "/snort/snort_interfaces_suppress.php");
		$tab_array[7] = array(gettext("Sync"), false, "/pkg_edit.php?xml=snort/snort_sync.xml");
		display_top_tabs($tab_array);
		echo '</td></tr>';
		echo '<tr><td>';
		$menu_iface=($if_friendly?substr($if_friendly,0,5)." ":"Iface ");
        $tab_array = array();
        $tab_array[] = array($menu_iface . gettext("Settings"), false, "/snort/snort_interfaces_edit.php?id={$id}");
        $tab_array[] = array($menu_iface . gettext("Categories"), false, "/snort/snort_rulesets.php?id={$id}");
        $tab_array[] = array($menu_iface . gettext("Rules"), false, "/snort/snort_rules.php?id={$id}");
        $tab_array[] = array($menu_iface . gettext("Variables"), false, "/snort/snort_define_servers.php?id={$id}");
        $tab_array[] = array($menu_iface . gettext("Preprocessors"), true, "/snort/snort_preprocessors.php?id={$id}");
        $tab_array[] = array($menu_iface . gettext("Barnyard2"), false, "/snort/snort_barnyard.php?id={$id}");
        display_top_tabs($tab_array);
?>
</td></tr>
<tr><td><div id="mainarea">
<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
	<tr>
		<td colspan="2" align="left" valign="middle">
		<?php echo gettext("Rules may be dependent on preprocessors!  Disabling preprocessors may result in "); ?>
		<?php echo gettext("Snort start failures unless dependent rules are also disabled."); ?>
		<?php echo gettext("The Auto-Rule Disable feature can be used, but note the warning about compromising protection.  " . 
		"Defaults will be used where no user input is provided."); ?></td>
	</tr>
	<tr>

		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Preprocessors Configuration"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable Performance Stats"); ?></td>
		<td width="78%" class="vtable"><input name="perform_stat" type="checkbox" value="on" 
			<?php if ($pconfig['perform_stat']=="on") echo "checked"; ?>>
			<?php echo gettext("Collect Performance Statistics for this interface."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Protect Customized Preprocessor Rules"); ?></td>
		<td width="78%" class="vtable"><input name="protect_preproc_rules" type="checkbox" value="on" 
			<?php if ($pconfig['protect_preproc_rules']=="on") echo "checked "; 
			if ($vrt_enabled <> 'on') echo "disabled"; ?>>
			<?php echo gettext("Check this box if you maintain customized preprocessor text rules files for this interface."); ?>
			<table width="100%" border="0" cellpadding="2" cellpadding="2">
				<tr>
					<td width="3%">&nbsp;</td>
					<td><?php echo gettext("Enable this only if you use customized preprocessor text rules files and " . 
					"you do not want them overwritten by automatic Snort VRT rule updates.  " . 
					"This option is disabled when Snort VRT rules download is not enabled on the Global Settings tab."); ?><br/><br/>
					<?php echo "<span class=\"red\"><strong>" . gettext("Hint: ") . "</strong></span>" . 
					gettext("Most users should leave this unchecked."); ?></td> 
				</tr>
			</table>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Auto Rule Disable"); ?></td>
		<td width="78%" class="vtable"><input name="preproc_auto_rule_disable" type="checkbox" value="on" 
			<?php if ($pconfig['preproc_auto_rule_disable']=="on") echo "checked"; ?>>
			<?php echo gettext("Auto-disable text rules dependent on disabled preprocessors for this interface.  "); 
			echo gettext("Default is ") . '<strong>' . gettext("Not Checked"); ?></strong>.<br/>
			<table width="100%" border="0" cellpadding="2" cellpadding="2">
				<tr>
					<td width="3%">&nbsp;</td>
					<td><span class="red"><strong><?php echo gettext("Warning:  "); ?></strong></span>
					<?php echo gettext("Enabling this option allows Snort to automatically disable any text rules " . 
					"containing rule options or content modifiers that are dependent upon the preprocessors " . 
					"you have not enabled.  This may facilitate starting Snort without errors related to " . 
					"disabled preprocessors, but can substantially compromise the level of protection by " .
					"automatically disabling detection rules."); ?></td>
				</tr>
			<?php if (file_exists($disabled_rules_log) && filesize($disabled_rules_log) > 0): ?>
				<tr>
					<td width="3%">&nbsp;</td>
					<td class="vexpl"><input type="button" class="formbtn" value="View" onclick="wopen('snort_rules_edit.php?id=<?=$id;?>&openruleset=<?=$disabled_rules_log;?>','FileViewer',800,600)"/>
					&nbsp;&nbsp;&nbsp;<?php echo gettext("Click to view the list of currently auto-disabled rules"); ?></td>
				</tr>
			<?php endif; ?>
			</table>
		</td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Host Attribute Table Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?></td>
		<td width="78%" class="vtable"><input name="host_attribute_table"
			type="checkbox" value="on" id="host_attribute_table" onclick="host_attribute_table_enable_change();" 
			<?php if ($pconfig['host_attribute_table']=="on") echo "checked"; ?>>
			<?php echo gettext("Use a Host Attribute Table file to auto-configure applicable preprocessors.  " .
				"Default is "); ?><strong><?php echo gettext("Not Checked"); ?></strong>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Host Attribute Data"); ?></td>
		<td width="78%" class="vtable"><strong><?php echo gettext("Import From File"); ?></strong><br/>
			<input name="host_attribute_file" type="file" class="formfld unknown" value="on" id="host_attribute_file" size="40" 
			<?php if ($pconfig['host_attribute_table']<>"on") echo "disabled"; ?>>&nbsp;&nbsp;
			<input type="submit" name="btn_import" id="btn_import" value="Import" class="formbtn" 
			<?php if ($pconfig['host_attribute_table']<>"on") echo "disabled"; ?>><br/>
			<?php echo gettext("Choose the Host Attributes file to use for auto-configuration."); ?><br/><br/>
			<span class="red"><strong><?php echo gettext("Warning: "); ?></strong></span>
			<?php echo gettext("The Host Attributes file has a required format.  See the "); ?><a href="http://manual.snort.org/" target="_blank">
			<?php echo gettext("Snort Manual"); ?></a><?php echo gettext(" for details.  " . 
			"An improperly formatted file may cause Snort to crash or fail to start.  The combination of "); ?>
			<a href="http://nmap.org/" target="_blank"><?php echo gettext("NMap"); ?></a><?php echo gettext(" and "); ?>
			<a href="http://code.google.com/p/hogger/" target="_blank"><?php echo gettext("Hogger"); ?></a><?php echo gettext(" or "); ?>
			<a href="http://gamelinux.github.io/prads/" target="_blank"><?php echo gettext("PRADS"); ?></a><?php echo gettext(" can be used to " .
			"scan networks and automatically generate a suitable Host Attribute Table file for import."); ?><br/><br/>
			<input type="submit" id="btn_edit_hat" name="btn_edit_hat" value="<?php if (!empty($pconfig['host_attribute_data'])) {echo gettext(" Edit ");} else {echo gettext("Create");} ?>" 
			class="formbtn" 
			<?php if ($pconfig['host_attribute_table']<>"on") echo "disabled"; ?>>&nbsp;&nbsp;
			<?php if (!empty($pconfig['host_attribute_data'])) {echo gettext("Click to View or Edit the Host Attribute data.");}
			 else {echo gettext("Click to Create Host Attribute data manually.");}
			if ($pconfig['host_attribute_table']=="on" && empty($pconfig['host_attribute_data'])){
				echo "<br/><br/><span class=\"red\"><strong>" . gettext("Warning: ") . "</strong></span>" . 
				gettext("No Host Attribute Data loaded - import from a file or enter it manually.");
			} ?></td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Maximum Hosts"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="max_attribute_hosts" type="text" class="formfld" id="max_attribute_hosts" size="6" 
				value="<?=htmlspecialchars($pconfig['max_attribute_hosts']);?>" 
				<?php if ($pconfig['host_attribute_table']<>"on") echo "disabled"; ?>>&nbsp;&nbsp;
				<?php echo gettext("Max number of hosts to read from the Attribute Table.  Min is ") . 
				"<strong>" . gettext("32") . "</strong>" . gettext(" and Max is ") . "<strong>" . 
				gettext("524288") . "</strong>"; ?>.</td>
			</tr>
		</table>
		<?php echo gettext("Sets a limit on the maximum number of hosts to read from the Attribute Table. If the number of hosts in " .
		"the table exceeds this value, an error is logged and the remainder of the hosts are ignored.  " . 
		"Default is ") . "<strong>" . gettext("10000") . "</strong>"; ?>.<br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Maximum Services Per Host"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="max_attribute_services_per_host" type="text" class="formfld" id="max_attribute_services_per_host" size="6" 
				value="<?=htmlspecialchars($pconfig['max_attribute_services_per_host']);?>"
				<?php if ($pconfig['host_attribute_table']<>"on") echo "disabled"; ?>>&nbsp;&nbsp;
				<?php echo gettext("Max number of  per host services to read from the Attribute Table.  Min is ") . 
				"<strong>" . gettext("1") . "</strong>" . gettext(" and Max is ") . "<strong>" . 
				gettext("65535") . "</strong>"; ?>.</td>
			</tr>
		</table>
		<?php echo gettext("Sets the per host limit of services to read from the Attribute Table. For a given host, if the number of " .
		"services read exceeds this value, an error is logged and the remainder of the services for that host are ignored. " . 
		"Default is ") . "<strong>" . gettext("10") . "</strong>"; ?>.<br/>
		</td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Protocol Aware Flushing Setting"); ?></td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Protocol Aware Flushing Maximum PDU"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="max_paf" type="text" class="formfld" id="max_paf" size="6"
				value="<?=htmlspecialchars($pconfig['max_paf']);?>">&nbsp;&nbsp;
				<?php echo gettext("Max number of PDUs to be reassembled into a single PDU.  Min is ") . 
				"<strong>" . gettext("0") . "</strong>" . gettext(" (off) and Max is ") . "<strong>" . 
				gettext("63780") . "</strong>"; ?>.</td>
			</tr>
		</table>
		<?php echo gettext("Multiple PDUs within a single TCP segment, as well as one PDU spanning multiple TCP segments, will be " .
		"reassembled into one PDU per packet for each PDU.  PDUs larger than the configured maximum will be split into multiple packets. " . 
		"Default is ") . "<strong>" . gettext("16000") . "</strong>.  " . gettext("A value of 0 disables Protocol Aware Flushing."); ?>.<br/>
		</td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("HTTP Inspect Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?></td>
		<td width="78%" class="vtable"><input name="http_inspect" 
			type="checkbox" value="on" id="http_inspect" onclick="http_inspect_enable_change();" 
			<?php if ($pconfig['http_inspect']=="on" || empty($pconfig['http_inspect'])) echo "checked"; ?>>
			<?php echo gettext("Use HTTP Inspect to " .
				"Normalize/Decode and detect HTTP traffic and protocol anomalies.  Default is "); ?>
			<strong><?php echo gettext("Checked"); ?></strong>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable XFF/True-Client-IP"); ?></td>
		<td width="78%" class="vtable"><input name="http_inspect_enable_xff"
			type="checkbox" value="on" id="http_inspect_enable_xff"  
			<?php if ($pconfig['http_inspect_enable_xff']=="on") echo "checked"; ?>>
			<?php echo gettext("Log original client IP present in X-Forwarded-For or True-Client-IP " .
				"HTTP headers.  Default is "); ?>
			<strong><?php echo gettext("Not Checked"); ?></strong>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable URI Logging"); ?></td>
		<td width="78%" class="vtable"><input name="http_inspect_log_uri"
			type="checkbox" value="on" id="http_inspect_log_uri"  
			<?php if ($pconfig['http_inspect_log_uri']=="on") echo "checked"; ?>>
			<?php echo gettext("Parse URI data from the HTTP request and log it with other session data." .
				"  Default is "); ?>
			<strong><?php echo gettext("Not Checked"); ?></strong>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable Hostname Logging"); ?></td>
		<td width="78%" class="vtable"><input name="http_inspect_log_hostname"
			type="checkbox" value="on" id="http_inspect_log_hostname"  
			<?php if ($pconfig['http_inspect_log_hostname']=="on") echo "checked"; ?>>
			<?php echo gettext("Parse Hostname data from the HTTP request and log it with other session data." .
				"  Default is "); ?>
			<strong><?php echo gettext("Not Checked"); ?></strong>.</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("HTTP Inspect Memory Cap"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="http_inspect_memcap" type="text" class="formfld"
					id="http_inspect_memcap" size="6"
					value="<?=htmlspecialchars($pconfig['http_inspect_memcap']);?>">&nbsp;&nbsp;
					<?php echo gettext("Max memory in bytes to use for URI and Hostname logging.  Min is ") . 
				"<strong>" . gettext("2304") . "</strong>" . gettext(" and Max is ") . "<strong>" . 
				gettext("603979776") . "</strong>" . gettext(" (576 MB)"); ?>.</td>
			</tr>
		</table>
		<?php echo gettext("Maximum amount of memory the preprocessor will use for logging the URI and Hostname data. The default " .
		"value is ") . "<strong>" . gettext("150,994,944") . "</strong>" . gettext(" (144 MB)."); ?>
		<?php echo gettext("  This option determines the maximum HTTP sessions that will log URI and Hostname data at any given instant. ") . 
		gettext("  Max Logged Sessions = MEMCAP / 2304"); ?>.<br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("HTTP server flow depth"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="server_flow_depth" type="text" class="formfld"
					id="server_flow_depth" size="6"
					value="<?=htmlspecialchars($pconfig['server_flow_depth']);?>">&nbsp;&nbsp;<?php echo gettext("<strong>-1</strong> " .
				"to <strong>65535</strong> (<strong>-1</strong> disables HTTP " .
				"inspect, <strong>0</strong> enables all HTTP inspect)"); ?></td>
			</tr>
		</table>
		<?php echo gettext("Amount of HTTP server response payload to inspect. Snort's " .
		"performance may increase by adjusting this value."); ?><br/>
		<?php echo gettext("Setting this value too low may cause false negatives. Values above 0 " .
		"are specified in bytes.  Recommended setting is maximum (65535). Default value is <strong>300</strong>"); ?><br/>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("HTTP server profile"); ?> </td>
		<td width="78%" class="vtable">
			<select name="http_server_profile" class="formselect" id="http_server_profile">
			<?php
			$profile = array('All', 'Apache', 'IIS', 'IIS4_0', 'IIS5_0');
			foreach ($profile as $val): ?>
			<option value="<?=strtolower($val);?>"
			<?php if (strtolower($val) == $pconfig['http_server_profile']) echo "selected"; ?>>
				<?=gettext($val);?></option>
				<?php endforeach; ?>
			</select>&nbsp;&nbsp;<?php echo gettext("Choose the profile type of the protected web server.  The default is ") . 
			"<strong>" . gettext("All") . "</strong>"; ?><br/>
			<?php echo gettext("IIS_4.0 and IIS_5.0 are identical to IIS except they alert on the ") . 
			gettext("double decoding vulnerability present in those versions."); ?><br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("HTTP client flow depth"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="client_flow_depth" type="text" class="formfld" 
					id="client_flow_depth" size="6"
					value="<?=htmlspecialchars($pconfig['client_flow_depth']);?>"> <?php echo gettext("<strong>-1</strong> " .
				"to <strong>1460</strong> (<strong>-1</strong> disables HTTP " .
				"inspect, <strong>0</strong> enables all HTTP inspect)"); ?></td>
			</tr>
		</table>
		<?php echo gettext("Amount of raw HTTP client request payload to inspect. Snort's " .
		"performance may increase by adjusting this value."); ?><br/>
		<?php echo gettext("Setting this value too low may cause false negatives. Values above 0 " .
		"are specified in bytes.  Recommended setting is maximum (1460). Default value is <strong>300</strong>"); ?><br/>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Disable HTTP Alerts"); ?></td>
		<td width="78%" class="vtable"><input name="noalert_http_inspect" 
			type="checkbox" value="on" id="noalert_http_inspect" 
			<?php if ($pconfig['noalert_http_inspect']=="on" || empty($pconfig['noalert_http_inspect'])) echo "checked"; ?>
			onClick="enable_change(false);"> <?php echo gettext("Turn off alerts from HTTP Inspect " .
				"preprocessor.  This has no effect on HTTP rules.  Default is "); ?>
			<strong><?php echo gettext("Checked"); ?></strong>.</td>
	</tr>

	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Frag3 Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?></td>
		<td width="78%" class="vtable"><input name="frag3_detection" type="checkbox" value="on" onclick="frag3_enable_change();" 
			<?php if ($pconfig['frag3_detection']=="on") echo "checked "; ?>
			onClick="enable_change(false)">
		<?php echo gettext("Use Frag3 Engine to detect IDS evasion attempts via target-based IP packet fragmentation.  Default is ") . 
		"<strong>" . gettext("Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Memory Cap"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="frag3_memcap" type="text" class="formfld"
					id="frag3_memcap" size="6"
					value="<?=htmlspecialchars($pconfig['frag3_memcap']);?>">
				<?php echo gettext("Memory cap (in bytes) for self preservation."); ?>.</td>
			</tr>
		</table>
		<?php echo gettext("The maximum amount of memory allocated for Frag3 fragment reassembly.  Default value is ") . 
		"<strong>" . gettext("4MB") . "</strong>"; ?>.<br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Maximum Fragments"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="frag3_max_frags" type="text" class="formfld"
					id="frag3_max_frags" size="6"
					value="<?=htmlspecialchars($pconfig['frag3_max_frags']);?>">
				<?php echo gettext("Maximum simultaneous fragments to track."); ?></td>
			</tr>
		</table>
		<?php echo gettext("The maximum number of simultaneous fragments to track.  Default value is ") .
		"<strong>8192</strong>."; ?><br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Overlap Limit"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="frag3_overlap_limit" type="text" class="formfld"
					id="frag3_overlap_limit" size="6"
					value="<?=htmlspecialchars($pconfig['frag3_overlap_limit']);?>">
				<?php echo gettext("Minimum is ") . "<strong>0</strong>" . gettext(" (unlimited), values greater than zero set the overlapped fragments per packet limit."); ?></td>
			</tr>
		</table>
		<?php echo gettext("Sets the limit for the number of overlapping fragments allowed per packet.  Default value is ") .
		"<strong>0</strong>" . gettext(" (unlimited)."); ?><br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Minimum Fragment Length"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="frag3_min_frag_len" type="text" class="formfld"
					id="frag3_min_frag_len" size="6"
					value="<?=htmlspecialchars($pconfig['frag3_min_frag_len']);?>">
				<?php echo gettext("Minimum is ") . "<strong>0</strong>" . gettext(" (check is disabled).  Fragments smaller than or equal to this limit are considered malicious."); ?></td>
			</tr>
		</table>
		<?php echo gettext("Defines smallest fragment size (payload size) that should be considered valid.  Default value is ") .
		"<strong>0</strong>" . gettext(" (check is disabled)."); ?><br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Timeout"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="frag3_timeout" type="text" class="formfld"
					id="frag3_timeout" size="6"
					value="<?=htmlspecialchars($pconfig['frag3_timeout']);?>">
				<?php echo gettext("Timeout period in seconds for fragments in the engine."); ?></td>
			</tr>
		</table>
		<?php echo gettext("Fragments in the engine for longer than this period will be automatically dropped.  Default value is ") . 
		"<strong>" . gettext("60 ") . "</strong>" . gettext("seconds."); ?><br/>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Target Policy"); ?> </td>
		<td width="78%" class="vtable">
			<select name="frag3_policy" class="formselect" id="frag3_policy">
			<?php
			$profile = array( 'BSD', 'BSD-Right', 'First', 'Last', 'Linux', 'Solaris', 'Windows' );
			foreach ($profile as $val): ?>
			<option value="<?=strtolower($val);?>" 
			<?php if (strtolower($val) == $pconfig['frag3_policy']) echo "selected"; ?>>
				<?=gettext($val);?></option>
				<?php endforeach; ?>
			</select>&nbsp;&nbsp;<?php echo gettext("Choose the IP fragmentation target policy appropriate for the protected hosts.  The default is ") . 
			"<strong>" . gettext("BSD") . "</strong>"; ?>.<br/>
			<?php echo gettext("Available OS targets are BSD, BSD-Right, First, Last, Linux, Solaris and Windows."); ?><br/>
		</td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Stream5 Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?></td>
		<td width="78%" class="vtable"><input name="stream5_reassembly" type="checkbox" value="on" onclick="stream5_enable_change();"  
			<?php if ($pconfig['stream5_reassembly']=="on") echo "checked"; ?>>
		<?php echo gettext("Use Stream5 session reassembly for TCP, UDP and/or ICMP traffic.  Default is ") . 
		"<strong>" . gettext("Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Protocol Tracking"); ?></td>
		<td width="78%" class="vtable">
			<input name="stream5_track_tcp" type="checkbox" value="on" id="stream5_track_tcp"  
				<?php if ($pconfig['stream5_track_tcp']=="on") echo "checked"; ?>>
				<?php echo gettext("Track and reassemble TCP sessions.  Default is ") . 
				"<strong>" . gettext("Checked") . "</strong>."; ?>
				<br/>
			<input name="stream5_track_udp" type="checkbox" value="on" id="stream5_track_udp" 
				<?php if ($pconfig['stream5_track_udp']=="on") echo "checked"; ?>>
				<?php echo gettext("Track and reassemble UDP sessions.  Default is ") . 
				"<strong>" . gettext("Checked") . "</strong>."; ?>
				<br/>
			<input name="stream5_track_icmp" type="checkbox" value="on" id="stream5_track_icmp" 
				<?php if ($pconfig['stream5_track_icmp']=="on") echo "checked"; ?>>
				<?php echo gettext("Track and reassemble ICMP sessions.  Default is ") . 
				"<strong>" . gettext("Not Checked") . "</strong>."; ?>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Require 3-Way Handshake"); ?></td>
		<td width="78%" class="vtable"><input name="stream5_require_3whs" type="checkbox" value="on" 
			<?php if ($pconfig['stream5_require_3whs']=="on") echo "checked "; ?>>
		<?php echo gettext("Establish sessions only on completion of SYN/SYN-ACK/ACK handshake.  Default is ") . 
		"<strong>" . gettext("Not Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Do Not Reassemble Async"); ?></td>
		<td width="78%" class="vtable"><input name="stream5_no_reassemble_async" type="checkbox" value="on" 
			<?php if ($pconfig['stream5_no_reassemble_async']=="on") echo "checked "; ?>>
		<?php echo gettext("Do not queue packets for reassembly if traffic has not been seen in both directions.  Default is ") . 
		"<strong>" . gettext("Not Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Do Not Store Large TCP Packets"); ?></td>
		<td width="78%" class="vtable">
			<input name="stream5_dont_store_lg_pkts" type="checkbox" value="on" 
			<?php if ($pconfig['stream5_dont_store_lg_pkts']=="on") echo "checked"; ?>>
			<?php echo gettext("Do not queue large packets in reassembly buffer to increase performance.  Default is ") . 
			"<strong>" . gettext("Not Checked") . "</strong>"; ?>.<br/>
			<?php echo "<span class=\"red\"><strong>" . gettext("Warning:  ") . "</strong></span>" . 
			gettext("Enabling this option could result in missed packets.  Recommended setting is not checked."); ?></td>  
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Max Queued Bytes"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="max_queued_bytes" type="text" class="formfld" 
					id="max_queued_bytes" size="6"
					value="<?=htmlspecialchars($pconfig['max_queued_bytes']);?>">
				<?php echo gettext("Minimum is <strong>1024</strong>, Maximum is <strong>1073741824</strong> " .
				"( default value is <strong>1048576</strong>, <strong>0</strong> " .
				"means Maximum )"); ?>.</td>
			</tr>
		</table>
		<?php echo gettext("The number of bytes to be queued for reassembly for TCP sessions in " .
		"memory. Default value is <strong>1048576</strong>"); ?>.<br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Max Queued Segs"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="max_queued_segs" type="text" class="formfld" 
					id="max_queued_segs" size="6"
					value="<?=htmlspecialchars($pconfig['max_queued_segs']);?>">
				<?php echo gettext("Minimum is <strong>2</strong>, Maximum is <strong>1073741824</strong> " .
				"( default value is <strong>2621</strong>, <strong>0</strong> means " .
				"Maximum )"); ?>.</td>
			</tr>
		</table>
		<?php echo gettext("The number of segments to be queued for reassembly for TCP sessions " .
		"in memory. Default value is <strong>2621</strong>"); ?>.<br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Memory Cap"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="stream5_mem_cap" type="text" class="formfld" 
					id="stream5_mem_cap" size="6"
					value="<?=htmlspecialchars($pconfig['stream5_mem_cap']);?>">
				<?php echo gettext("Minimum is <strong>32768</strong>, Maximum is <strong>1073741824</strong> " .
				"( default value is <strong>8388608</strong>) "); ?>.</td>
			</tr>
		</table>
		<?php echo gettext("The memory cap in bytes for TCP packet storage " .
		"in RAM. Default value is <strong>8388608</strong> (8 MB)"); ?>.<br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Overlap Limit"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="stream5_overlap_limit" type="text" class="formfld" 
					id="stream5_overlap_limit" size="6"
					value="<?=htmlspecialchars($pconfig['stream5_overlap_limit']);?>">
				<?php echo gettext("Minimum is ") . "<strong>0</strong>" . gettext(" (unlimited), and the maximum is ") . 
				"<strong>255</strong>."; ?></td>
			</tr>
		</table>
		<?php echo gettext("Sets the limit for the number of overlapping fragments allowed per packet.  Default value is ") .
		"<strong>0</strong>" . gettext(" (unlimited)."); ?><br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("TCP Session Timeout"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="stream5_tcp_timeout" type="text" class="formfld" 
					id="stream5_tcp_timeout" size="6"
					value="<?=htmlspecialchars($pconfig['stream5_tcp_timeout']);?>">
				<?php echo gettext("TCP Session timeout in seconds.  Minimum is ") . "<strong>1</strong>" . gettext(" and the maximum is ") . 
				"<strong>86400</strong>" . gettext(" (approximately 1 day)"); ?>.</td>
			</tr>
		</table>
		<?php echo gettext("Sets the session reassembly timeout period for TCP packets.  Default value is ") .
		"<strong>30</strong>" . gettext(" seconds."); ?><br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("UDP Session Timeout"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="stream5_udp_timeout" type="text" class="formfld" 
					id="stream5_udp_timeout" size="6"
					value="<?=htmlspecialchars($pconfig['stream5_udp_timeout']);?>">
				<?php echo gettext("UDP Session timeout in seconds.  Minimum is ") . "<strong>1</strong>" . gettext(" and the maximum is ") . 
				"<strong>86400</strong>" . gettext(" (approximately 1 day)"); ?>.</td>
			</tr>
		</table>
		<?php echo gettext("Sets the session reassembly timeout period for UDP packets.  Default value is ") .
		"<strong>30</strong>" . gettext(" seconds."); ?><br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("ICMP Session Timeout"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="stream5_icmp_timeout" type="text" class="formfld" 
					id="stream5_icmp_timeout" size="6"
					value="<?=htmlspecialchars($pconfig['stream5_icmp_timeout']);?>">
				<?php echo gettext("ICMP Session timeout in seconds.  Minimum is ") . "<strong>1</strong>" . gettext(" and the maximum is ") . 
				"<strong>86400</strong>" . gettext(" (approximately 1 day)"); ?>.</td>
			</tr>
		</table>
		<?php echo gettext("Sets the session reassembly timeout period for ICMP packets.  Default value is ") .
		"<strong>30</strong>" . gettext(" seconds."); ?><br/>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("IP Target Policy"); ?></td>
		<td width="78%" class="vtable">
			<select name="stream5_policy" class="formselect" id="stream5_policy"> 
			<?php
			$profile = array( 'BSD', 'First', 'HPUX', 'HPUX10', 'Irix', 'Last', 'Linux', 'MacOS', 'Old-Linux', 
					 'Solaris', 'Vista', 'Windows', 'Win2003' );
			foreach ($profile as $val): ?>
			<option value="<?=strtolower($val);?>" 
			<?php if (strtolower($val) == $pconfig['stream5_policy']) echo "selected"; ?>>
				<?=gettext($val);?></option>
				<?php endforeach; ?>
			</select>&nbsp;&nbsp;<?php echo gettext("Choose the TCP reassembly target policy appropriate for the protected hosts.  The default is ") . 
			"<strong>" . gettext("BSD") . "</strong>"; ?>.<br/>
			<?php echo gettext("Available OS targets are BSD, First, HPUX, HPUX10, Irix, Last, Linux, MacOS, Old Linux, Solaris, Vista, Windows, and Win2003 Server."); ?><br/>
		</td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Portscan Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?></td>
		<td width="78%" class="vtable"><input name="sf_portscan" onclick="sf_portscan_enable_change();" 
			type="checkbox" value="on" id="sf_portscan"   
			<?php if ($pconfig['sf_portscan']=="on") echo "checked"; ?>>
		<?php echo gettext("Use Portscan Detection to detect various types of port scans and sweeps.  Default is ") . 
		"<strong>" . gettext("Not Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Protocol"); ?> </td>
		<td width="78%" class="vtable">
			<select name="pscan_protocol" class="formselect" id="pscan_protocol"> 
			<?php
			$protos = array('all', 'tcp', 'udp', 'icmp', 'ip');
			foreach ($protos as $val): ?>
			<option value="<?=$val;?>"
			<?php if ($val == $pconfig['pscan_protocol']) echo "selected"; ?>> 
				<?=gettext($val);?></option>
				<?php endforeach; ?>
			</select>&nbsp;&nbsp;<?php echo gettext("Choose the Portscan protocol type to alert for (all, tcp, udp, icmp or ip).  Default is ") . 
			"<strong>" . gettext("all") . "</strong>."; ?><br/>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Scan Type"); ?> </td>
		<td width="78%" class="vtable">
			<select name="pscan_type" class="formselect" id="pscan_type"> 
			<?php
			$protos = array('all', 'portscan', 'portsweep', 'decoy_portscan', 'distributed_portscan');
			foreach ($protos as $val): ?>
			<option value="<?=$val;?>"
			<?php if ($val == $pconfig['pscan_type']) echo "selected"; ?>> 
				<?=gettext($val);?></option>
				<?php endforeach; ?>
			</select>&nbsp;&nbsp;<?php echo gettext("Choose the Portscan scan type to alert for.  Default is ") . 
			"<strong>" . gettext("all") . "</strong>."; ?><br/>
			<table width="100%" border="0" cellpadding="0" cellspacing="0">
			  <tr>
				<td><?php echo gettext("PORTSCAN: one->one scan; one host scans multiple ports on another host."); ?></td>
			  </tr>
			  <tr> 
				<td><?php echo gettext("PORTSWEEP: one->many scan; one host scans a single port on multiple hosts."); ?></td>
			  </tr>
			  <tr>
				<td><?php echo gettext("DECOY_PORTSCAN: one->one scan; attacker has spoofed source address inter-mixed with real scanning address."); ?></td>
			  </tr>
			  <tr>
				<td><?php echo gettext("DISTRIBUTED_PORTSCAN: many->one scan; multiple hosts query one host for open services."); ?></td>
			  </tr>
			  <tr>
				<td><?php echo gettext("ALL: alerts for all of the above scan types."); ?></td>
			  </tr>
			</table>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Sensitivity"); ?> </td>
		<td width="78%" class="vtable">
			<select name="pscan_sense_level" class="formselect" id="pscan_sense_level"> 
			<?php
			$levels = array('low', 'medium', 'high');
			foreach ($levels as $val): ?>
			<option value="<?=$val;?>"
			<?php if ($val == $pconfig['pscan_sense_level']) echo "selected"; ?>> 
				<?=gettext(ucfirst($val));?></option>
				<?php endforeach; ?>
			</select>&nbsp;&nbsp;<?php echo gettext("Choose the Portscan sensitivity level (Low, Medium, High).  Default is ") . 
			"<strong>" . gettext("Medium") . "</strong>."; ?><br/>
			<table width="100%" border="0" cellpadding="0" cellspacing="0">
			  <tr>
				<td><?php echo gettext("LOW: alerts generated on error packets from the target host; "); ?>
				<?php echo gettext("this setting should see few false positives.  "); ?></td>
			  </tr>
			  <tr>
				<td><?php echo gettext("MEDIUM: tracks connection counts, so will generate filtered alerts; may "); ?>
				    <?php echo gettext("false positive on active hosts."); ?></td>
			  </tr>
			  <tr>
				<td><?php echo gettext("HIGH: tracks hosts using a time window; will catch some slow scans, but is "); ?>
				    <?php echo gettext("very sensitive to active hosts."); ?></td>
			  </tr>
			</table>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Memory Cap"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="pscan_memcap" type="text" class="formfld" 
					id="pscan_memcap" size="6"
					value="<?=htmlspecialchars($pconfig['pscan_memcap']);?>">
				<?php echo gettext("Maximum memory in bytes to allocate for portscan detection.  ") . 
				gettext("Default is ") . "<strong>" . gettext("10000000") . "</strong>" . 
				gettext(" (10 MB)"); ?>.</td>
			</tr>
		</table>
		<?php echo gettext("The maximum number of bytes to allocate for portscan detection.  The higher this number, ") . 
		gettext("the more nodes that can be tracked.  Default is ") . 
		"<strong>10,000,000</strong>" . gettext(" bytes.  (10 MB)"); ?><br/>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Ignore Scanners"); ?></td>
		<td width="78%" class="vtable">
			<input name="pscan_ignore_scanners" type="text" size="40" autocomplete="off" class="formfldalias" id="pscan_ignore_scanners" 
			value="<?=$pconfig['pscan_ignore_scanners'];?>" title="<?=trim(filter_expand_alias($pconfig['pscan_ignore_scanners']));?>">&nbsp;&nbsp;<?php echo gettext("Leave blank for default.  ") . 
			gettext("Default value is ") . "<strong>" . gettext("\$HOME_NET") . "</strong>"; ?>.<br/>
			<?php echo gettext("Ignores the specified entity as a source of scan alerts.  Entity must be a defined alias."); ?><br/>
		</td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("General Preprocessor Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable RPC Decode and Back Orifice detector"); ?></td>
		<td width="78%" class="vtable"><input name="other_preprocs" type="checkbox" value="on" 
			<?php if ($pconfig['other_preprocs']=="on") echo "checked"; ?>>
		<?php echo gettext("Normalize/Decode RPC traffic and detects Back Orifice traffic on the network.  Default is ") . 
		"<strong>" . gettext("Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable FTP and Telnet Normalizer"); ?></td>
		<td width="78%" class="vtable"><input name="ftp_preprocessor" type="checkbox" value="on" 
			<?php if ($pconfig['ftp_preprocessor']=="on") echo "checked"; ?>>
		<?php echo gettext("Normalize/Decode FTP and Telnet traffic and protocol anomalies.  Default is ") . 
		"<strong>" . gettext("Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable POP Normalizer"); ?></td>
		<td width="78%" class="vtable"><input name="pop_preproc" type="checkbox" value="on" 
			<?php if ($pconfig['pop_preproc']=="on") echo "checked"; ?>>
		<?php echo gettext("Normalize/Decode POP protocol for enforcement and buffer overflows.  Default is ") . 
		"<strong>" . gettext("Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable IMAP Normalizer"); ?></td>
		<td width="78%" class="vtable"><input name="imap_preproc" type="checkbox" value="on" 
			<?php if ($pconfig['imap_preproc']=="on") echo "checked"; ?>>
		<?php echo gettext("Normalize/Decode IMAP protocol for enforcement and buffer overflows.  Default is ") . 
		"<strong>" . gettext("Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable SMTP Normalizer"); ?></td>
		<td width="78%" class="vtable"><input name="smtp_preprocessor" type="checkbox" value="on" 
			<?php if ($pconfig['smtp_preprocessor']=="on") echo "checked"; ?>>
		<?php echo gettext("Normalize/Decode SMTP protocol for enforcement and buffer overflows.  Default is ") . 
		"<strong>" . gettext("Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable DCE/RPC2 Detection"); ?></td>
		<td width="78%" class="vtable"><input name="dce_rpc_2" type="checkbox" value="on" 
			<?php if ($pconfig['dce_rpc_2']=="on") echo "checked"; ?>>
		<?php echo gettext("The DCE/RPC preprocessor detects and decodes SMB and DCE/RPC traffic.  Default is ") . 
		"<strong>" . gettext("Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable SIP Detection"); ?></td>
		<td width="78%" class="vtable"><input name="sip_preproc" type="checkbox" value="on" 
			<?php if ($pconfig['sip_preproc']=="on") echo "checked"; ?>>
		<?php echo gettext("The SIP preprocessor decodes SIP traffic and detects some vulnerabilities.  Default is ") . 
		"<strong>" . gettext("Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable GTP Detection"); ?></td>
		<td width="78%" class="vtable"><input name="gtp_preproc" type="checkbox" value="on" 
			<?php if ($pconfig['gtp_preproc']=="on") echo "checked"; ?>>
		<?php echo gettext("The GTP preprocessor decodes GPRS Tunneling Protocol traffic and detects intrusion attempts."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable SSH Detection"); ?></td>
		<td width="78%" class="vtable"><input name="ssh_preproc" type="checkbox" value="on" 
			<?php if ($pconfig['ssh_preproc']=="on") echo "checked"; ?>>
		<?php echo gettext("The SSH preprocessor detects various Secure Shell exploit attempts."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable DNS Detection"); ?></td>
		<td width="78%" class="vtable"><input name="dns_preprocessor" type="checkbox" value="on" 
			<?php if ($pconfig['dns_preprocessor']=="on") echo "checked"; ?>>
		<?php echo gettext("The DNS preprocessor decodes DNS Response traffic and detects vulnerabilities.  Default is ") . 
		"<strong>" . gettext("Checked") . "</strong>"; ?>.</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable SSL Data"); ?></td>
		<td width="78%" class="vtable">
			<input name="ssl_preproc" type="checkbox" value="on"  
			<?php if ($pconfig['ssl_preproc']=="on") echo "checked"; ?>>
		<?php echo gettext("SSL data searches for irregularities during SSL protocol exchange.  Default is ") . 
		"<strong>" . gettext("Checked") . "</strong>"; ?>.</td>	
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable Sensitive Data"); ?></td>
		<td width="78%" class="vtable">
			<input name="sensitive_data" type="checkbox" value="on" 
			<?php if ($pconfig['sensitive_data'] == "on")
				 echo "checked";
			      elseif ($vrt_enabled == "off")
				 echo "disabled";
			?>>
			<?php echo gettext("Sensitive data searches for credit card or Social Security numbers and e-mail addresses in data."); ?>
		<br/>
		<span class="red"><strong><?php echo gettext("Note: "); ?></strong></span><?php echo gettext("To enable this preprocessor, you must select the Snort VRT rules on the Global Settings tab."); ?>
		</td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("SCADA Preprocessor Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable Modbus Detection"); ?></td>
		<td width="78%" class="vtable">
			<input name="modbus_preproc" type="checkbox" value="on" 
			<?php if ($pconfig['modbus_preproc']=="on") echo "checked"; ?>>
			<?php echo gettext("Modbus is a protocol used in SCADA networks.  The default port is TCP 502.") . "<br/>" . 
		  	"<span class=\"red\"><strong>" . gettext("Note: ") . "</strong></span>" . 
			gettext("If your network does not contain Modbus-enabled devices, you can leave this preprocessor disabled."); ?>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable DNP3 Detection"); ?></td>
		<td width="78%" class="vtable">
			<input name="dnp3_preproc" type="checkbox" value="on" 
			<?php if ($pconfig['dnp3_preproc']=="on") echo "checked"; ?>>
			<?php echo gettext("DNP3 is a protocol used in SCADA networks.  The default port is TCP 20000.") . "<br/>" . 
		  	"<span class=\"red\"><strong>" . gettext("Note: ") . "</strong></span>" . 
			gettext("If your network does not contain DNP3-enabled devices, you can leave this preprocessor disabled."); ?>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top">&nbsp;</td>
		<td width="78%">
			<input name="Submit" type="submit" class="formbtn" value="Save" title="<?php echo 
			gettext("Save preprocessor settings"); ?>">
			<input name="id" type="hidden" value="<?=$id;?>">&nbsp;&nbsp;&nbsp;&nbsp;
			<input name="ResetAll" type="submit" class="formbtn" value="Reset" title="<?php echo 
			gettext("Reset all settings to defaults") . "\" onclick=\"return confirm('" . 
			gettext("WARNING:  This will reset ALL preprocessor settings to their defaults.  Click OK to continue or CANCEL to quit.") . 
			"');\""; ?>></td>
	</tr>
	<tr>
		<td width="22%" valign="top">&nbsp;</td>
		<td width="78%"><span class="vexpl"><span class="red"><strong><?php echo gettext("Note: "); ?></strong></span></span>
			<?php echo gettext("Please save your settings before you exit.  Preprocessor changes will rebuild the rules file.  This "); ?>
			<?php echo gettext("may take several seconds.  Snort must also be restarted to activate any changes made on this screen."); ?></td>
	</tr>
</table>
</div>
</td></tr></table>
</form>
<script type="text/javascript">
<?php
        $isfirst = 0;
        $aliases = "";
        $addrisfirst = 0;
        $portisfirst = 0;
        $aliasesaddr = "";
        $aliasesports = "";
        if(isset($config['aliases']['alias']) && is_array($config['aliases']['alias']))
                foreach($config['aliases']['alias'] as $alias_name) {
                        if ($alias_name['type'] == "host" || $alias_name['type'] == "network") {
				if (trim(filter_expand_alias($alias_name['name'])) == "")
					continue;
				if($addrisfirst == 1) $aliasesaddr .= ",";
				$aliasesaddr .= "'" . $alias_name['name'] . "'";
				$addrisfirst = 1;
			} else if ($alias_name['type'] == "port") {
				if($portisfirst == 1) $aliasesports .= ",";
				$aliasesports .= "'" . $alias_name['name'] . "'";
				$portisfirst = 1;
			}
                }
?>

        var addressarray=new Array(<?php echo $aliasesaddr; ?>);
        var portsarray=new Array(<?php echo $aliasesports; ?>);

function createAutoSuggest() {
<?php
	echo "objAlias = new AutoSuggestControl(document.getElementById('pscan_ignore_scanners'), new StateSuggestions(addressarray));\n";
?>
}

setTimeout("createAutoSuggest();", 500);

function frag3_enable_change() {
	if (!document.iform.frag3_detection.checked) {
		var msg = "WARNING:  Disabling the Frag3 preprocessor is not recommended!\n\n";
		msg = msg + "Snort may fail to start because of other dependent preprocessors or ";
		msg = msg + "rule options.  Are you sure you want to disable it?\n\n";
		msg = msg + "Click OK to disable Frag3, or CANCEL to quit.";
		if (!confirm(msg)) {
			document.iform.frag3_detection.checked=true;
		}
	}
	var endis = !(document.iform.frag3_detection.checked);
	document.iform.frag3_overlap_limit.disabled=endis;
	document.iform.frag3_min_frag_len.disabled=endis;
	document.iform.frag3_policy.disabled=endis;
	document.iform.frag3_max_frags.disabled=endis;
	document.iform.frag3_memcap.disabled=endis;
	document.iform.frag3_timeout.disabled=endis;
}

function host_attribute_table_enable_change() {
	var endis = !(document.iform.host_attribute_table.checked);
	document.iform.host_attribute_file.disabled=endis;
	document.iform.btn_import.disabled=endis;
	document.iform.btn_edit_hat.disabled=endis;
	document.iform.max_attribute_hosts.disabled=endis;
	document.iform.max_attribute_services_per_host.disabled=endis;
}

function http_inspect_enable_change() {
	var endis = !(document.iform.http_inspect.checked);
	document.iform.http_inspect_enable_xff.disabled=endis;
	document.iform.server_flow_depth.disabled=endis;
	document.iform.client_flow_depth.disabled=endis;
	document.iform.http_server_profile.disabled=endis;
	document.iform.http_inspect_memcap.disabled=endis;
	document.iform.http_inspect_log_uri.disabled=endis;
	document.iform.http_inspect_log_hostname.disabled=endis;
}

function sf_portscan_enable_change() {
	var endis = !(document.iform.sf_portscan.checked);
	document.iform.pscan_protocol.disabled=endis;
	document.iform.pscan_type.disabled=endis;
	document.iform.pscan_memcap.disabled=endis;
	document.iform.pscan_sense_level.disabled=endis;
	document.iform.pscan_ignore_scanners.disabled=endis;
}

function stream5_enable_change() {
	if (!document.iform.stream5_reassembly.checked) {
		var msg = "WARNING:  Stream5 is a critical preprocessor, and disabling it is not recommended!  ";
		msg = msg + "The following preprocessors require Stream5 and will be automatically disabled if currently enabled:\n\n";
		msg = msg + "    SMTP\t\tPOP\t\tSIP\n";
		msg = msg + "    SENSITIVE_DATA\tSF_PORTSCAN\tDCE/RPC 2\n";
		msg = msg + "    IMAP\t\tDNS\t\tSSL\n";
		msg = msg + "    GTP\t\tDNP3\t\tMODBUS\n\n";
		msg = msg + "Snort may fail to start because of other preprocessors or rule options dependent on Stream5.  ";
		msg = msg + "Are you sure you want to disable it?\n\n";
		msg = msg + "Click OK to disable Stream5, or CANCEL to quit.";
		if (!confirm(msg)) {
			document.iform.stream5_reassembly.checked=true;
		}
		else {
			alert("If Snort fails to start with Stream5 disabled, examine the system log for clues.");
			document.iform.smtp_preprocessor.checked=false;
			document.iform.dce_rpc_2.checked=false;
			document.iform.sip_preproc.checked=false;
			document.iform.sensitive_data.checked=false;
			document.iform.imap_preproc.checked=false;
			document.iform.pop_preproc.checked=false;
			document.iform.ssl_preproc.checked=false;
			document.iform.dns_preprocessor.checked=false;
			document.iform.modbus_preproc.checked=false;
			document.iform.dnp3_preproc.checked=false;
			document.iform.sf_portscan.checked=false;
			sf_portscan_enable_change();
		}
	}

	var endis = !(document.iform.stream5_reassembly.checked);
	document.iform.max_queued_bytes.disabled=endis;
	document.iform.max_queued_segs.disabled=endis;
	document.iform.stream5_mem_cap.disabled=endis;
	document.iform.stream5_policy.disabled=endis;
	document.iform.stream5_overlap_limit.disabled=endis;
	document.iform.stream5_no_reassemble_async.disabled=endis;
	document.iform.stream5_dont_store_lg_pkts.disabled=endis;
	document.iform.stream5_tcp_timeout.disabled=endis;
	document.iform.stream5_udp_timeout.disabled=endis;
	document.iform.stream5_icmp_timeout.disabled=endis;
}

function enable_change_all() {
	http_inspect_enable_change();
	sf_portscan_enable_change();

	// Enable/Disable Frag3 settings
	var endis = !(document.iform.frag3_detection.checked);
	document.iform.frag3_overlap_limit.disabled=endis;
	document.iform.frag3_min_frag_len.disabled=endis;
	document.iform.frag3_policy.disabled=endis;
	document.iform.frag3_max_frags.disabled=endis;
	document.iform.frag3_memcap.disabled=endis;
	document.iform.frag3_timeout.disabled=endis;

	// Enable/Disable Stream5 settings
	endis = !(document.iform.stream5_reassembly.checked);
	document.iform.max_queued_bytes.disabled=endis;
	document.iform.max_queued_segs.disabled=endis;
	document.iform.stream5_mem_cap.disabled=endis;
	document.iform.stream5_policy.disabled=endis;
	document.iform.stream5_overlap_limit.disabled=endis;
	document.iform.stream5_no_reassemble_async.disabled=endis;
	document.iform.stream5_dont_store_lg_pkts.disabled=endis;
	document.iform.stream5_tcp_timeout.disabled=endis;
	document.iform.stream5_udp_timeout.disabled=endis;
	document.iform.stream5_icmp_timeout.disabled=endis;
}

function wopen(url, name, w, h)
{
// Fudge factors for window decoration space.
// In my tests these work well on all platforms & browsers.
    w += 32;
    h += 96;
    var win = window.open(url,
        name, 
       'width=' + w + ', height=' + h + ', ' +
       'location=no, menubar=no, ' +
       'status=no, toolbar=no, scrollbars=yes, resizable=yes');
    win.resizeTo(w, h);
    win.focus();
}

// Set initial state of form controls
enable_change_all();

</script>
<?php include("fend.inc"); ?>
</body>
</html>
