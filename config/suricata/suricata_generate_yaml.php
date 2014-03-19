<?php
/*
	suricata_generate_yaml.php

	Copyright (C) 2014 Bill Meeks
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

// Create required Suricata directories if they don't exist
$suricata_dirs = array( $suricatadir, $suricatacfgdir, "{$suricatacfgdir}/rules",
	"{$suricatalogdir}suricata_{$if_real}{$suricata_uuid}" );
foreach ($suricata_dirs as $dir) {
	if (!is_dir($dir))
		safe_mkdir($dir);
}

// Copy required generic files to the interface sub-directory
$config_files = array( "classification.config", "reference.config", "gen-msg.map", "unicode.map" );
foreach ($config_files as $file) {
	if (file_exists("{$suricatadir}{$file}"))
		@copy("{$suricatadir}{$file}", "{$suricatacfgdir}/{$file}");
}

// Create required files if they don't exist
$suricata_files = array( "{$suricatacfgdir}/magic" );
foreach ($suricata_files as $file) {
	if (!file_exists($file))
		file_put_contents($file, "\n");
}

// Read the configuration parameters for the passed interface
// and construct appropriate string variables for use in the
// suricata.yaml template include file.

// Set HOME_NET and EXTERNAL_NET for the interface
$home_net_list = suricata_build_list($suricatacfg, $suricatacfg['homelistname']);
$home_net = implode(",", $home_net_list);
$home_net = trim($home_net);
$external_net = '!$HOME_NET';
if (!empty($suricatacfg['externallistname']) && $suricatacfg['externallistname'] != 'default') {
	$external_net_list = suricata_build_list($suricatacfg, $suricatacfg['externallistname']);
	$external_net = implode(",", $external_net_list);
	$external_net = trim($external_net);
}

// Set default and user-defined variables for SERVER_VARS and PORT_VARS
$suricata_servers = array (
	"dns_servers" => "\$HOME_NET", "smtp_servers" => "\$HOME_NET", "http_servers" => "\$HOME_NET",
	"sql_servers" => "\$HOME_NET", "telnet_servers" => "\$HOME_NET", "dnp3_server" => "\$HOME_NET",
	"dnp3_client" => "\$HOME_NET", "modbus_server" => "\$HOME_NET", "modbus_client" => "\$HOME_NET",
	"enip_server" => "\$HOME_NET", "enip_client" => "\$HOME_NET",
	"aim_servers" => "64.12.24.0/23,64.12.28.0/23,64.12.161.0/24,64.12.163.0/24,64.12.200.0/24,205.188.3.0/24,205.188.5.0/24,205.188.7.0/24,205.188.9.0/24,205.188.153.0/24,205.188.179.0/24,205.188.248.0/24"
);
$addr_vars = "";
	foreach ($suricata_servers as $alias => $avalue) {
		if (!empty($suricatacfg["def_{$alias}"]) && is_alias($suricatacfg["def_{$alias}"])) {
			$avalue = trim(filter_expand_alias($suricatacfg["def_{$alias}"]));
			$avalue = preg_replace('/\s+/', ',', trim($avalue));
		}
		$addr_vars .= "    " . strtoupper($alias) . ": \"{$avalue}\"\n";
	}
$addr_vars = trim($addr_vars);
if(is_array($config['system']['ssh']) && isset($config['system']['ssh']['port']))
        $ssh_port = $config['system']['ssh']['port'];
else
        $ssh_port = "22";
$suricata_ports = array(
	"http_ports" => "80", 
	"oracle_ports" => "1521", 
	"ssh_ports" => $ssh_port, 
	"shellcode_ports" => "!80", 
	"DNP3_PORTS" => "20000", "file_data_ports" => "\$HTTP_PORTS,110,143"
);
$port_vars = "";
	foreach ($suricata_ports as $alias => $avalue) {
		if (!empty($suricatacfg["def_{$alias}"]) && is_alias($suricatacfg["def_{$alias}"])) {
			$avalue = trim(filter_expand_alias($suricatacfg["def_{$alias}"]));
			$avalue = preg_replace('/\s+/', ',', trim($avalue));
		}
		$port_vars .= "    " . strtoupper($alias) . ": \"{$avalue}\"\n";
	}
$port_vars = trim($port_vars);

// Define a Suppress List (Threshold) if one is configured
$suppress = suricata_find_list($suricatacfg['suppresslistname'], 'suppress');
if (!empty($suppress)) {
	$suppress_data = str_replace("\r", "", base64_decode($suppress['suppresspassthru']));
	@file_put_contents("{$suricatacfgdir}/threshold.config", $suppress_data);
}
else
	@file_put_contents("{$suricatacfgdir}/threshold.config", "");

// Add interface-specific detection engine settings
if (!empty($suricatacfg['max_pending_packets']))
	$max_pend_pkts = $suricatacfg['max_pending_packets'];
else
	$max_pend_pkts = 1024;

if (!empty($suricatacfg['detect_eng_profile']))
	$detect_eng_profile = $suricatacfg['detect_eng_profile'];
else
	$detect_eng_profile = "medium";

if (!empty($suricatacfg['sgh_mpm_context']))
	$sgh_mpm_ctx = $suricatacfg['sgh_mpm_context'];
else
	$sgh_mpm_ctx = "auto";

if (!empty($suricatacfg['mpm_algo']))
	$mpm_algo = $suricatacfg['mpm_algo'];
else
	$mpm_algo = "ac";

if (!empty($suricatacfg['inspect_recursion_limit']) || $suricatacfg['inspect_recursion_limit'] == '0')
	$inspection_recursion_limit = $suricatacfg['inspect_recursion_limit'];
else
	$inspection_recursion_limit = "";

// Add interface-specific logging settings
if ($suricatacfg['alertsystemlog'] == 'on')
	$alert_syslog = "yes";
else
	$alert_syslog = "no";

if ($suricatacfg['enable_stats_log'] == 'on')
	$stats_log_enabled = "yes";
else
	$stats_log_enabled = "no";

if (!empty($suricatacfg['stats_upd_interval']))
	$stats_upd_interval = $suricatacfg['stats_upd_interval'];
else
	$stats_upd_interval = "10";

if ($suricatacfg['append_stats_log'] == 'on')
	$stats_log_append = "yes";
else
	$stats_log_append = "no";

if ($suricatacfg['enable_http_log'] == 'on')
	$http_log_enabled = "yes";
else
	$http_log_enabled = "no";

if ($suricatacfg['append_http_log'] == 'on')
	$http_log_append = "yes";
else
	$http_log_append = "no";

if ($suricatacfg['enable_tls_log'] == 'on')
	$tls_log_enabled = "yes";
else
	$tls_log_enabled = "no";

if ($suricatacfg['tls_log_extended'] == 'on')
	$tls_log_extended = "yes";
else
	$tls_log_extended = "no";

if ($suricatacfg['enable_json_file_log'] == 'on')
	$json_log_enabled = "yes";
else
	$json_log_enabled = "no";

if ($suricatacfg['append_json_file_log'] == 'on')
	$json_log_append = "yes";
else
	$json_log_append = "no";

if ($suricatacfg['enable_tracked_files_magic'] == 'on')
	$json_log_magic = "yes";
else
	$json_log_magic = "no";

if ($suricatacfg['enable_tracked_files_md5'] == 'on')
	$json_log_md5 = "yes";
else
	$json_log_md5 = "no";
	
if ($suricatacfg['enable_file_store'] == 'on') {
	$file_store_enabled = "yes";
	if (!file_exists("{$suricatalogdir}suricata_{$if_real}{$suricata_uuid}/file.waldo"))
		@file_put_contents("{$suricatalogdir}suricata_{$if_real}{$suricata_uuid}/file.waldo", "");
}
else
	$file_store_enabled = "no";

if ($suricatacfg['enable_pcap_log'] == 'on')
	$pcap_log_enabled = "yes";
else
	$pcap_log_enabled = "no";

if (!empty($suricatacfg['max_pcap_log_size']))
	$pcap_log_limit_size = $suricatacfg['max_pcap_log_size'];
else
	$pcap_log_limit_size = "32";

if (!empty($suricatacfg['max_pcap_log_files']))
	$pcap_log_max_files = $suricatacfg['max_pcap_log_files'];
else
	$pcap_log_max_files = "1000";

if ($suricatacfg['barnyard_enable'] == 'on')
	$barnyard2_enabled = "yes";
else
	$barnyard2_enabled = "no";

if (isset($config['installedpackages']['suricata']['config'][0]['unified2_log_limit']))
	$unified2_log_limit = "{$config['installedpackages']['suricata']['config'][0]['unified2_log_limit']}mb";
else
	$unified2_log_limit = "32mb";

if (isset($suricatacfg['barnyard_sensor_id']))
	$unified2_sensor_id = $suricatacfg['barnyard_sensor_id'];
else
	$unified2_sensor_id = "0";

// Add interface-specific IP defrag settings
if (!empty($suricatacfg['frag_memcap']))
	$frag_memcap = $suricatacfg['frag_memcap'];
else
	$frag_memcap = "33554432";

if (!empty($suricatacfg['ip_max_trackers']))
	$ip_max_trackers = $suricatacfg['ip_max_trackers'];
else
	$ip_max_trackers = "65535";

if (!empty($suricatacfg['ip_max_frags']))
	$ip_max_frags = $suricatacfg['ip_max_frags'];
else
	$ip_max_frags = "65535";

if (!empty($suricatacfg['frag_hash_size']))
	$frag_hash_size = $suricatacfg['frag_hash_size'];
else
	$frag_hash_size = "65536";

if (!empty($suricatacfg['ip_frag_timeout']))
	$ip_frag_timeout = $suricatacfg['ip_frag_timeout'];
else
	$ip_frag_timeout = "60";

// Add interface-specific flow manager setttings
if (!empty($suricatacfg['flow_memcap']))
	$flow_memcap = $suricatacfg['flow_memcap'];
else
	$flow_memcap = "33554432";

if (!empty($suricatacfg['flow_hash_size']))
	$flow_hash_size = $suricatacfg['flow_hash_size'];
else
	$flow_hash_size = "65536";

if (!empty($suricatacfg['flow_prealloc']))
	$flow_prealloc = $suricatacfg['flow_prealloc'];
else
	$flow_prealloc = "10000";

if (!empty($suricatacfg['flow_emerg_recovery']))
	$flow_emerg_recovery = $suricatacfg['flow_emerg_recovery'];
else
	$flow_emerg_recovery = "30";

if (!empty($suricatacfg['flow_prune']))
	$flow_prune = $suricatacfg['flow_prune'];
else
	$flow_prune = "5";

// Add interface-specific flow timeout setttings
if (!empty($suricatacfg['flow_tcp_new_timeout']))
	$flow_tcp_new_timeout = $suricatacfg['flow_tcp_new_timeout'];
else
	$flow_tcp_new_timeout = "60";

if (!empty($suricatacfg['flow_tcp_established_timeout']))
	$flow_tcp_established_timeout = $suricatacfg['flow_tcp_established_timeout'];
else
	$flow_tcp_established_timeout = "3600";

if (!empty($suricatacfg['flow_tcp_closed_timeout']))
	$flow_tcp_closed_timeout = $suricatacfg['flow_tcp_closed_timeout'];
else
	$flow_tcp_closed_timeout = "120";

if (!empty($suricatacfg['flow_tcp_emerg_new_timeout']))
	$flow_tcp_emerg_new_timeout = $suricatacfg['flow_tcp_emerg_new_timeout'];
else
	$flow_tcp_emerg_new_timeout = "10";

if (!empty($suricatacfg['flow_tcp_emerg_established_timeout']))
	$flow_tcp_emerg_established_timeout = $suricatacfg['flow_tcp_emerg_established_timeout'];
else
	$flow_tcp_emerg_established_timeout = "300";

if (!empty($suricatacfg['flow_tcp_emerg_closed_timeout']))
	$flow_tcp_emerg_closed_timeout = $suricatacfg['flow_tcp_emerg_closed_timeout'];
else
	$flow_tcp_emerg_closed_timeout = "20";

if (!empty($suricatacfg['flow_udp_new_timeout']))
	$flow_udp_new_timeout = $suricatacfg['flow_udp_new_timeout'];
else
	$flow_udp_new_timeout = "30";

if (!empty($suricatacfg['flow_udp_established_timeout']))
	$flow_udp_established_timeout = $suricatacfg['flow_udp_established_timeout'];
else
	$flow_udp_established_timeout = "300";

if (!empty($suricatacfg['flow_udp_emerg_new_timeout']))
	$flow_udp_emerg_new_timeout = $suricatacfg['flow_udp_emerg_new_timeout'];
else
	$flow_udp_emerg_new_timeout = "10";

if (!empty($suricatacfg['flow_udp_emerg_established_timeout']))
	$flow_udp_emerg_established_timeout = $suricatacfg['flow_udp_emerg_established_timeout'];
else
	$flow_udp_emerg_established_timeout = "100";

if (!empty($suricatacfg['flow_icmp_new_timeout']))
	$flow_icmp_new_timeout = $suricatacfg['flow_icmp_new_timeout'];
else
	$flow_icmp_new_timeout = "30";

if (!empty($suricatacfg['flow_icmp_established_timeout']))
	$flow_icmp_established_timeout = $suricatacfg['flow_icmp_established_timeout'];
else
	$flow_icmp_established_timeout = "300";

if (!empty($suricatacfg['flow_icmp_emerg_new_timeout']))
	$flow_icmp_emerg_new_timeout = $suricatacfg['flow_icmp_emerg_new_timeout'];
else
	$flow_icmp_emerg_new_timeout = "10";

if (!empty($suricatacfg['flow_icmp_emerg_established_timeout']))
	$flow_icmp_emerg_established_timeout = $suricatacfg['flow_icmp_emerg_established_timeout'];
else
	$flow_icmp_emerg_established_timeout = "100";

// Add interface-specific stream settings
if (!empty($suricatacfg['stream_memcap']))
	$stream_memcap = $suricatacfg['stream_memcap'];
else
	$stream_memcap = "33554432";

if (!empty($suricatacfg['stream_max_sessions']))
	$stream_max_sessions = $suricatacfg['stream_max_sessions'];
else
	$stream_max_sessions = "262144";

if (!empty($suricatacfg['stream_prealloc_sessions']))
	$stream_prealloc_sessions = $suricatacfg['stream_prealloc_sessions'];
else
	$stream_prealloc_sessions = "32768";

if (!empty($suricatacfg['reassembly_memcap']))
	$reassembly_memcap = $suricatacfg['reassembly_memcap'];
else
	$reassembly_memcap = "67108864";

if (!empty($suricatacfg['reassembly_depth']) || $suricatacfg['reassembly_depth'] == '0')
	$reassembly_depth = $suricatacfg['reassembly_depth'];
else
	$reassembly_depth = "1048576";

if (!empty($suricatacfg['reassembly_to_server_chunk']))
	$reassembly_to_server_chunk = $suricatacfg['reassembly_to_server_chunk'];
else
	$reassembly_to_server_chunk = "2560";

if (!empty($suricatacfg['reassembly_to_client_chunk']))
	$reassembly_to_client_chunk = $suricatacfg['reassembly_to_client_chunk'];
else
	$reassembly_to_client_chunk = "2560";

if ($suricatacfg['enable_midstream_sessions'] == 'on')
	$stream_enable_midstream = "true";
else
	$stream_enable_midstream = "false";

if ($suricatacfg['enable_async_sessions'] == 'on')
	$stream_enable_async = "true";
else
	$stream_enable_async = "false";

// Add the OS-specific host policies if configured, otherwise
// just set default to BSD for all networks.
if (!is_array($suricatacfg['host_os_policy']['item']))
	$suricatacfg['host_os_policy']['item'] = array();
if (empty($suricatacfg['host_os_policy']['item']))
	$host_os_policy = "bsd: [0.0.0.0/0]";
else {
	foreach ($suricatacfg['host_os_policy']['item'] as $k => $v) {
		$engine = "{$v['policy']}: ";
		if ($v['bind_to'] <> "all") {
			$tmp = trim(filter_expand_alias($v['bind_to']));
			if (!empty($tmp)) {
				$engine .= "[";
				$tmp = preg_replace('/\s+/', ',', $tmp);
				$list = explode(',', $tmp);
				foreach ($list as $addr) {
					if (is_ipaddrv6($addr) || is_subnetv6($addr))
						$engine .= "\"{$addr}\", ";
					elseif (is_ipaddrv4($addr) || is_subnetv4($addr))
						$engine .= "{$addr}, ";
					else
						log_error("[suricata] WARNING: invalid IP address value '{$addr}' in Alias {$v['bind_to']} will be ignored.");
				}
				$engine = trim($engine, ' ,');
				$engine .= "]";
			}
			else {
				log_error("[suricata] WARNING: unable to resolve IP List Alias '{$v['bind_to']}' for Host OS Policy '{$v['name']}' ... ignoring this entry.");
				continue;
			}
		}
		else
			$engine .= "[0.0.0.0/0]";

		$host_os_policy .= "  {$engine}\n";
	}
	// Remove trailing newline
	$host_os_policy = trim($host_os_policy);
}

// Add the HTTP Server-specific policies if configured, otherwise
// just set default to IDS for all networks.
if (!is_array($suricatacfg['libhtp_policy']['item']))
	$suricatacfg['libhtp_policy']['item'] = array();
if (empty($suricatacfg['libhtp_policy']['item'])) {
	$http_hosts_default_policy = "default-config:\n     personality: IDS\n     request-body-limit: 4096\n     response-body-limit: 4096\n";
	$http_hosts_default_policy .= "     double-decode-path: no\n     double-decode-query: no\n";
}
else {
	foreach ($suricatacfg['libhtp_policy']['item'] as $k => $v) {
		if ($v['bind_to'] <> "all") {
			$engine = "server-config:\n     - {$v['name']}:\n";
			$tmp = trim(filter_expand_alias($v['bind_to']));
			if (!empty($tmp)) {
				$engine .= "         address: [";
				$tmp = preg_replace('/\s+/', ',', $tmp);
				$list = explode(',', $tmp);
				foreach ($list as $addr) {
					if (is_ipaddrv6($addr) || is_subnetv6($addr))
						$engine .= "\"{$addr}\", ";
					elseif (is_ipaddrv4($addr) || is_subnetv4($addr))
						$engine .= "{$addr}, ";
					else {
						log_error("[suricata] WARNING: invalid IP address value '{$addr}' in Alias {$v['bind_to']} will be ignored.");
						continue;
					}
				}
				$engine = trim($engine, ' ,');
				$engine .= "]\n";
				$engine .= "         personality: {$v['personality']}\n         request-body-limit: {$v['request-body-limit']}\n";
				$engine .= "         response-body-limit: {$v['response-body-limit']}\n";
				$engine .= "         double-decode-path: {$v['double-decode-path']}\n";
				$engine .= "         double-decode-query: {$v['double-decode-query']}\n";
				$http_hosts_policy .= "   {$engine}\n";
			}
			else {
				log_error("[suricata] WARNING: unable to resolve IP List Alias '{$v['bind_to']}' for Host OS Policy '{$v['name']}' ... ignoring this entry.");
				continue;
			}
		}
		else {
			$http_hosts_default_policy = "     personality: {$v['personality']}\n     request-body-limit: {$v['request-body-limit']}\n";
			$http_hosts_default_policy .= "     response-body-limit: {$v['response-body-limit']}\n";
			$http_hosts_default_policy .= "     double-decode-path: {$v['double-decode-path']}\n";
			$http_hosts_default_policy .= "     double-decode-query: {$v['double-decode-query']}\n";
		}
	}
	// Remove trailing newline
	$http_hosts_default_policy = trim($http_hosts_default_policy);
	$http_hosts_policy = trim($http_hosts_policy);
}

// Configure ASN1 max frames value
if (!empty($suricatacfg['asn1_max_frames']))
	$asn1_max_frames = $suricatacfg['asn1_max_frames'];
else
	$asn1_max_frames = "256";

// Create the rules files and save in the interface directory
suricata_prepare_rule_files($suricatacfg, $suricatacfgdir);

// Check and configure only non-empty rules files for the interface
$rules_files = "";
if (filesize("{$suricatacfgdir}/rules/".ENFORCING_RULES_FILENAME) > 0)
	$rules_files .= ENFORCING_RULES_FILENAME;
if (filesize("{$suricatacfgdir}/rules/".FLOWBITS_FILENAME) > 0)
	$rules_files .= "\n - " . FLOWBITS_FILENAME;
if (filesize("{$suricatacfgdir}/rules/custom.rules") > 0)
	$rules_files .= "\n - custom.rules";
$rules_files = ltrim($rules_files, '\n -');

// Add the general logging settings to the configuration (non-interface specific)
if ($config['installedpackages']['suricata']['config'][0]['log_to_systemlog'] == 'on')
	$suricata_use_syslog = "yes";
else
	$suricata_use_syslog = "no";

?>
