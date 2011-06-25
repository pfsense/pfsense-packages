<?php
/* $Id$ */
/*

 part of pfSense
 All rights reserved.

 Pfsense Old snort GUI 
 Copyright (C) 2006 Scott Ullrich.
 
 Pfsense snort GUI 
 Copyright (C) 2008-2012 Robert Zelaya.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. Neither the name of the pfSense nor the names of its contributors 
 may be used to endorse or promote products derived from this software without 
 specific prior written permission.

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

// unset crsf checks
if(isset($_POST['__csrf_magic'])) {
  unset($_POST['__csrf_magic']);
}

// -------------------------- START snort.conf -------------------------

/* func builds custom whitelests */
function build_base_whitelist($lanip, $wanip, $wangw, $wandns, $vips, $vpns, $userwhtips, $netlist) {
	
	// bring in settings from /etc/inc
	global $config;

	/* build an interface array list */
	if ($lanip === 'on') {
		$int_array = array('lan');
		for ($j = 1; isset ($config['interfaces']['opt' . $j]); $j++)
		{
			if(isset($config['interfaces']['opt' . $j]['enable']))
			if(isset($config['interfaces']['opt' . $j]['gateway']))
			$int_array[] = "opt{$j}";
		}
	
		/*  iterate through interface list and write out whitelist items
		 *  and also compile a home_net list for snort.
		 */
		foreach($int_array as $int) 
		{
			/* calculate interface subnet information */
			$ifcfg = $config['interfaces'][$int];
			$subnet = gen_subnet($ifcfg['ipaddr'], $ifcfg['subnet']);
			$subnetmask = gen_subnet_mask($ifcfg['subnet']);
			if($subnet == "pppoe" or $subnet == "dhcp") {
				$subnet = find_interface_ip("ng0");
				if($subnet) {
				$home_net .= "{$subnet} ";
				}
			} else {
				if ($subnet)
				if($ifcfg['subnet'])
				$home_net .= "{$subnet}/{$ifcfg['subnet']} ";
			}
		}
	}

	if($wanip === 'on') {
		// add all WAN ips to the whitelist 
		$wan_if = get_real_wan_interface();
		$ip = find_interface_ip($wan_if);
		if($ip) {
		$home_net .= "{$ip} ";
		}
	}

	if($wangw === 'on') {
		// Add Gateway on WAN interface to whitelist (For RRD graphs)
		$gw = get_interface_gateway('wan');
		if($gw) {
		$home_net .= "{$gw} ";
		}
	}

	if($wandns === 'on') {
		// Add DNS server for WAN interface to whitelist
		$dns_servers = get_dns_servers();
		foreach($dns_servers as $dns) {
			if($dns) {
			$home_net .= "{$dns} ";
			}
		}
	}		

	// TESTING: NEEDED 06202011
	if($vips === 'on') {
		// iterate all vips and add to whitelist
		if($config['virtualip'])
		foreach($config['virtualip']['vip'] as $vip)
		if($vip['subnet'])
		$home_net .= $vip['subnet'] . " ";
	}		

	// TESTING: NEEDED 06202011
	// grab a list of vpns and whitelist if user desires added by nestorfish 954
	if($vpns == 'on') {
		// chk what pfsense version were on
		if ($pfsense_stable == 'yes') {
			$vpns_list = get_vpns_list();
		}
		
		// chk what pfsense version were on
		if ($pfsense_stable == 'no') {
			$vpns_list = filter_get_vpns_list();
		}
		
		if (!empty($vpns_list)) {
			$home_net .= "$vpns_list ";
		}
	}
	
	// Add homenet, NETLIST
	if($userwhtips == 'on') {

		$whitelistArray = snortSql_fetchAllSettings('snortDB', 'SnortWhitelistips', 'filename', $netlist);
		
		foreach ($whitelistArray as $whiteListIp)
		{
			$home_net .= $whiteListIp['ip'] . ' ';
		}
			
	}
	
	// Add loopback to whitelist (ftphelper)
	if ($lanip === 'on') {
	$home_net .= '127.0.0.1';
	}
	
	// remove empty spaces
	$home_net = trim($home_net);
	
	// this is for snort.conf
	$home_net = str_replace(' ', ',', $home_net);
	// by Thrae, helps people with more than one gateway, breaks snort as is
	$home_net = str_replace(',,', ',', $home_net);
	
	if ($lanip !== 'on') {
		
		$snortHomeNetPieces = explode(',', $home_net);
		$home_net = '';
				
		$i = 1;
		$homeNetPieceCount = count($snortHomeNetPieces);
		foreach ($snortHomeNetPieces as $homeNetPiece)
		{		
			if (!empty($homeNetPiece) && $homeNetPieceCount !== $i) {
				$home_net .= $homeNetPiece . ',';
			}else{
				$home_net .= $homeNetPiece . '';
			}
			
			$i++;
		}
		
	}
	
	return $home_net;
}



function create_snort_homenet($snortNet, $getSnortHomeNet) {
	
	if ($snortNet === 'homenet') {
		
		$listName = $getSnortHomeNet['homelistname'];
		
		if ($listName == 'default' || empty($listName)) {
			return build_base_whitelist('on','on', 'on', 'on', 'on', 'on', 'off', '');
		}else{
			$getSnortWhitelist = snortSql_fetchAllSettings('snortDB', 'SnortWhitelist', 'filename', $listName);
			return build_base_whitelist('on', $getSnortWhitelist[0]['wanips'], $getSnortWhitelist[0]['wangateips'], $getSnortWhitelist[0]['wandnsips'], $getSnortWhitelist[0]['vips'], $getSnortWhitelist[0]['vpnips'], 'on', $listName);
		}		
	}
	
	if ($snortNet === 'externalnet') {
		$listName = $getSnortHomeNet['externallistname'];
		return build_base_whitelist('off', 'off', 'off', 'off', 'off', 'off', 'on', $listName);
	}

}

function generate_snort_conf($uuid)
{
	
	// Iface main setings
	$ifaceSettingsArray = snortSql_fetchAllSettings('snortDB', 'SnortIfaces', 'uuid', $uuid);
	
	// custom home nets
	// might need to make this same ass homenet
	$home_net = '[' . create_snort_homenet('homenet', $ifaceSettingsArray) . ']';

	if ($ifaceSettingsArray['externallistname'] === 'default'){
		$external_net = '!$HOME_NET';
	}else{
		$external_net = '[' . create_snort_homenet('externalnet', $ifaceSettingsArray) . ']';
	}

	// obtain external interface
	// XXX: make multi wan friendly
	$snort_ext_int = $ifaceSettingsArray['interface'];	

	// user added arguments
	$snort_config_pass_thru = str_replace("\r", '', base64_decode($ifaceSettingsArray['configpassthru']));	

	// define basic log filename 
	$snortunifiedlogbasic_type = "output unified: filename snort_{$ifaceSettingsArray['uuid']}.log, limit 128";	

	// define snortalertlogtype
	$snortalertlogtype = $ifaceSettingsArray['snortalertlogtype'];	
	
	if ($snortalertlogtype == 'fast' || $snortalertlogtype == 'full') {
		$snortalertlogtype_type = "output alert_{$snortalertlogtype}: alert";
	}else{
		$snortalertlogtype_type = '';
	}	
	
	// define alertsystemlog
	$alertsystemlog_info_chk = $ifaceSettingsArray['alertsystemlog'];
	if ($alertsystemlog_info_chk == on) {
		$alertsystemlog_type = "output alert_syslog: log_alert";
	}
		
	// define tcpdumplog
	$tcpdumplog_info_chk = $ifaceSettingsArray['tcpdumplog'];
	if ($tcpdumplog_info_chk == on) {
		$tcpdumplog_type = "output log_tcpdump: snort_{$ifaceSettingsArray['uuid']}.tcpdump";
	}
	
	// define snortunifiedlog
	$snortunifiedlog_info_chk = $ifaceSettingsArray['snortunifiedlog'];
	if ($snortunifiedlog_info_chk == on) {
		$snortunifiedlog_type = "output unified2: filename snort_{$ifaceSettingsArray['uuid']}.u2, limit 128";
	}

	// define snortsam
	$snortsam_info_chk = $ifaceSettingsArray['blockoffenders7'];
	if ($snortsam_info_chk === 'on') {
		$snortsam_type = "output alert_fwsam: 127.0.0.1:898/addpasshere";
	}else{
		$snortsam_type = '';
	}

	/* define threshold file */
	$threshold_info_chk = $ifaceSettingsArray['suppresslistname'];
	if ($threshold_info_chk !== 'default') {

		$threshold_info_chk = "include /usr/local/etc/snort/suppress/{$threshold_info_chk}";
	}

	/* define servers and ports snortdefservers */
	/* def DNS_SERVSERS */
	$def_dns_servers_info_chk = $ifaceSettingsArray['def_dns_servers'];
	if (empty($def_dns_servers_info_chk)) {
		$def_dns_servers_type = '$HOME_NET';
	}else{
		$def_dns_servers_type = "$def_dns_servers_info_chk";
	}	

	/* def DNS_PORTS */
	$def_dns_ports_info_chk = $ifaceSettingsArray['def_dns_ports'];
	if (empty($def_dns_ports_info_chk)) {
		$def_dns_ports_type = '53';
	}else{
		$def_dns_ports_type = "$def_dns_ports_info_chk";
	}	

	/* def SMTP_SERVSERS */
	$def_smtp_servers_info_chk = $ifaceSettingsArray['def_smtp_servers'];
	if (empty($def_smtp_servers_info_chk)) {
		$def_smtp_servers_type = '$HOME_NET';
	}else{
		$def_smtp_servers_type = $def_smtp_servers_info_chk;
	}

	/* def SMTP_PORTS */
	$def_smtp_ports_info_chk = $ifaceSettingsArray['def_smtp_ports'];
	if (empty($def_smtp_ports_info_chk)) {
		$def_smtp_ports_type = '25';
	}else{
		$def_smtp_ports_type = $def_smtp_ports_info_chk;
	}
		
	/* def MAIL_PORTS */
	$def_mail_ports_info_chk = $ifaceSettingsArray['def_mail_ports'];
	if (empty($def_mail_ports_info_chk)) {
		$def_mail_ports_type = '25,143,465,691';
	}else{
		$def_mail_ports_type = $def_mail_ports_info_chk;
	}
	
	/* def HTTP_SERVSERS */
	$def_http_servers_info_chk = $ifaceSettingsArray['def_http_servers'];
	if (empty($def_http_servers_info_chk)) {
		$def_http_servers_type = '$HOME_NET';
	}else{
		$def_http_servers_type = $def_http_servers_info_chk;
	}
	
	/* def WWW_SERVSERS */
	$def_www_servers_info_chk = $ifaceSettingsArray['def_www_servers'];
	if (empty($def_www_servers_info_chk)) {
		$def_www_servers_type = '$HOME_NET';
	}else{
		$def_www_servers_type = $def_www_servers_info_chk;
	}
	
	/* def HTTP_PORTS */
	$def_http_ports_info_chk = $ifaceSettingsArray['def_http_ports'];
	if (empty($def_http_ports_info_chk)) {
		$def_http_ports_type = '80';
	}else{
		$def_http_ports_type = $def_http_ports_info_chk;
	}

	/* def SQL_SERVSERS */
	$def_sql_servers_info_chk = $ifaceSettingsArray['def_sql_servers'];
	if (empty($def_sql_servers_info_chk)) {
		$def_sql_servers_type = '$HOME_NET';
	}else{
		$def_sql_servers_type = $def_sql_servers_info_chk;
	}
	
	/* def ORACLE_PORTS */
	$def_oracle_ports_info_chk = $ifaceSettingsArray['def_oracle_ports'];
	if (empty($def_oracle_ports_info_chk)) {
		$def_oracle_ports_type = '1521';
	}else{
		$def_oracle_ports_type = $def_oracle_ports_info_chk;
	}
	
	/* def MSSQL_PORTS */
	$def_mssql_ports_info_chk = $ifaceSettingsArray['def_mssql_ports'];
	if (empty($def_mssql_ports_info_chk)) {
		$def_mssql_ports_type = '1433';
	}else{
		$def_mssql_ports_type = $def_mssql_ports_info_chk;
	}
	
	/* def TELNET_SERVSERS */
	$def_telnet_servers_info_chk = $ifaceSettingsArray['def_telnet_servers'];
	if (empty($def_telnet_servers_info_chk)) {
		$def_telnet_servers_type = '$HOME_NET';
	}else{
		$def_telnet_servers_type = $def_telnet_servers_info_chk;
	}
	
	/* def TELNET_PORTS */
	$def_telnet_ports_info_chk = $ifaceSettingsArray['def_telnet_ports'];
	if (empty($def_telnet_ports_info_chk)) {
		$def_telnet_ports_type = '23';
	}else{
		$def_telnet_ports_type = $def_telnet_ports_info_chk;
	}
	
	/* def SNMP_SERVSERS */
	$def_snmp_servers_info_chk = $ifaceSettingsArray['def_snmp_servers'];
	if (empty($def_snmp_servers_info_chk)) {
		$def_snmp_servers_type = '$HOME_NET';
	}else{
		$def_snmp_servers_type = $def_snmp_servers_info_chk;
	}

	/* def SNMP_PORTS */
	$def_snmp_ports_info_chk = $ifaceSettingsArray['def_snmp_ports'];
	if (empty($def_snmp_ports_info_chk)) {
		$def_snmp_ports_type = '161';
	}else{
		$def_snmp_ports_type = $def_snmp_ports_info_chk;
	}
	
	/* def FTP_SERVSERS */
	$def_ftp_servers_info_chk = $ifaceSettingsArray['def_ftp_servers'];
	if (empty($def_ftp_servers_info_chk)) {
		$def_ftp_servers_type = '$HOME_NET';
	}else{
		$def_ftp_servers_type = $def_ftp_servers_info_chk;
	}

	/* def FTP_PORTS */
	$def_ftp_ports_info_chk = $ifaceSettingsArray['def_ftp_ports'];
	if (empty($def_ftp_ports_info_chk)) {
		$def_ftp_ports_type = '21';
	}else{
		$def_ftp_ports_type = $def_ftp_ports_info_chk;
	}
	
	/* def SSH_SERVSERS */
	$def_ssh_servers_info_chk = $ifaceSettingsArray['def_ssh_servers'];
	if (empty($def_ssh_servers_info_chk)) {
		$def_ssh_servers_type = '$HOME_NET';
	}else{
		$def_ssh_servers_type = $def_ssh_servers_info_chk;
	}
	
	/* if user has defined a custom ssh port, use it */
	if($config['system']['ssh']['port']) {
		$ssh_port = $config['system']['ssh']['port'];
	}else{
		$ssh_port = '22';
	}

	/* def SSH_PORTS */
	$def_ssh_ports_info_chk = $ifaceSettingsArray['def_ssh_ports'];
	if (empty($def_ssh_ports_info_chk)) {
		$def_ssh_ports_type = $ssh_port;
	}else{
		$def_ssh_ports_type = $def_ssh_ports_info_chk;
	}
	
	/* def POP_SERVSERS */
	$def_pop_servers_info_chk = $ifaceSettingsArray['def_pop_servers'];
	if (empty($def_pop_servers_info_chk)) {
		$def_pop_servers_type = '$HOME_NET';
	}else{
		$def_pop_servers_type = $def_pop_servers_info_chk;
	}
	
	/* def POP2_PORTS */
	$def_pop2_ports_info_chk = $ifaceSettingsArray['def_pop2_ports'];
	if (empty($def_pop2_ports_info_chk)) {
		$def_pop2_ports_type = '109';
	}else{
		$def_pop2_ports_type = $def_pop2_ports_info_chk;
	}

	/* def POP3_PORTS */
	$def_pop3_ports_info_chk = $ifaceSettingsArray['def_pop3_ports'];
	if (empty($def_pop3_ports_info_chk)) {
		$def_pop3_ports_type = '110';
	}else{
		$def_pop3_ports_type = $def_pop3_ports_info_chk;
	}
	
	/* def IMAP_SERVSERS */
	$def_imap_servers_info_chk = $ifaceSettingsArray['def_imap_servers'];
	if (empty($def_imap_servers_info_chk)) {
		$def_imap_servers_type = '$HOME_NET';
	}else{
		$def_imap_servers_type = $def_imap_servers_info_chk;
	}
	
	/* def IMAP_PORTS */
	$def_imap_ports_info_chk = $ifaceSettingsArray['def_imap_ports'];
	if (empty($def_imap_ports_info_chk)) {
		$def_imap_ports_type = '143';
	}else{
		$def_imap_ports_type = $def_imap_ports_info_chk;
	}
	/* def SIP_PROXY_IP */
	$def_sip_proxy_ip_info_chk = $ifaceSettingsArray['def_sip_proxy_ip'];
	if (empty($def_sip_proxy_ip_info_chk)) {
		$def_sip_proxy_ip_type = '$HOME_NET';
	}else{
		$def_sip_proxy_ip_type = "$def_sip_proxy_ip_info_chk";
	}
	
	/* def SIP_PROXY_PORTS */
	$def_sip_proxy_ports_info_chk = $ifaceSettingsArray['def_sip_proxy_ports'];
	if (empty($def_sip_proxy_ports_info_chk)) {
		$def_sip_proxy_ports_type = '5060:5090,16384:32768';
	}else{
		$def_sip_proxy_ports_type = $def_sip_proxy_ports_info_chk;
	}
		
	/* def AUTH_PORTS */
	$def_auth_ports_info_chk = $ifaceSettingsArray['def_auth_ports'];
	if (empty($def_auth_ports_info_chk)) {
		$def_auth_ports_type = '113';
	}else{
		$def_auth_ports_type = $def_auth_ports_info_chk;
	}
	
	/* def FINGER_PORTS */
	$def_finger_ports_info_chk = $ifaceSettingsArray['def_finger_ports'];
	if (empty($def_finger_ports_info_chk)) {
		$def_finger_ports_type = "79";
	}else{
		$def_finger_ports_type = $def_finger_ports_info_chk;
	}
	
	/* def IRC_PORTS */
	$def_irc_ports_info_chk = $ifaceSettingsArray['def_irc_ports'];
	if (empty($def_irc_ports_info_chk)) {
		$def_irc_ports_type = '6665,6666,6667,6668,6669,7000';
	}else{
		$def_irc_ports_type = $def_irc_ports_info_chk;
	}
	
	/* def NNTP_PORTS */
	$def_nntp_ports_info_chk = $ifaceSettingsArray['def_nntp_ports'];
	if (empty($def_nntp_ports_info_chk)) {
		$def_nntp_ports_type = '119';
	}else{
		$def_nntp_ports_type = $def_nntp_ports_info_chk;
	}
	
	/* def RLOGIN_PORTS */
	$def_rlogin_ports_info_chk = $ifaceSettingsArray['def_rlogin_ports'];
	if (empty($def_rlogin_ports_info_chk)) {
		$def_rlogin_ports_type = '513';
	}else{
		$def_rlogin_ports_type = $def_rlogin_ports_info_chk;
	}
	
	/* def RSH_PORTS */
	$def_rsh_ports_info_chk = $ifaceSettingsArray['def_rsh_ports'];
	if (empty($def_rsh_ports_info_chk)) {
		$def_rsh_ports_type = '514';
	}else{
		$def_rsh_ports_type = $def_rsh_ports_info_chk;
	}
	
	/* def SSL_PORTS */
	$def_ssl_ports_info_chk = $ifaceSettingsArray['def_ssl_ports'];
	if (empty($def_ssl_ports_info_chk)) {
		$def_ssl_ports_type = '443,465,563,636,989,990,992,993,994,995';
	}else{
		$def_ssl_ports_type = $def_ssl_ports_info_chk;
	}
	
	/* should we install a automatic update crontab entry?
	$automaticrulesupdate = $config['installedpackages']['snortglobal']['automaticrulesupdate7'];

	// if user is on pppoe, we really want to use ng0 interface
	if(isset($config['interfaces'][$snort_ext_int]['ipaddr']) && ($config['interfaces'][$snort_ext_int]['ipaddr'] == "pppoe"))
		$snort_ext_int = "ng0";

	// set the snort performance model */
	if($ifaceSettingsArray['performance']) {
		$snort_performance = $ifaceSettingsArray['performance'];
	}else{
		$snort_performance = "ac-bnfa";
	}
	
	// list rules in db that are on in a array
	$listEnabled_rulesets = array();
	$listEnabled_rulesets = snortSql_fetchAllSettings('snortDBrules', 'SnortRuleSets', 'rdbuuid', $ifaceSettingsArray['ruledbname']);
	
	if(!empty($listEnabled_rulesets)) {
		foreach($listEnabled_rulesets as $enabled_item)
		{
			if ($enabled_item['enable'] !== 'off') {
				$selected_rules_sections  .= "include \$RULE_PATH/{$enabled_item['rulesetname']}\n";
			}
		}
	}	
	

	/////////////////////////////

	/* preprocessor code */

	/* def perform_stat */


	$def_perform_stat_info_chk = $ifaceSettingsArray['perform_stat'];
	if ($def_perform_stat_info_chk === 'on') {
		$def_perform_stat_type = "preprocessor perfmonitor: time 300 file /var/log/snort/sn_{$ifaceSettingsArray['uuid']}.stats pktcnt 10000";
	}else{
		$def_perform_stat_type = '';
	}
	
	$def_flow_depth_info_chk = $ifaceSettingsArray['flow_depth'];
	if (empty($def_flow_depth_info_chk)) {
		$def_flow_depth_type = '0';
	}else{
		$def_flow_depth_type = $ifaceSettingsArray['flow_depth'];
	}

	/* def http_inspect */
	$snort_http_inspect = <<<EOD
#################
                #
# HTTP Inspect  #
                #
#################

preprocessor http_inspect: global iis_unicode_map unicode.map 1252

preprocessor http_inspect_server: server default \
                        ports  { 80 8080 }  \
                        non_strict \
                        non_rfc_char  { 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 }  \
                        flow_depth {$def_flow_depth_type}  \
                        apache_whitespace no \
                        directory no \
                        iis_backslash no \
                        u_encode yes \
                        ascii no \
                        chunk_length 500000 \
                        bare_byte yes \
                        double_decode yes \
                        iis_unicode no \
                        iis_delimiter no \
                        multi_slash no
	
EOD;

	$def_http_inspect_info_chk = $ifaceSettingsArray['http_inspect'];
	if ($def_http_inspect_info_chk === 'on') {
		$def_http_inspect_type = $snort_http_inspect;
	}else{
		$def_http_inspect_type = '';
	}
	
	
	/* def other_preprocs */
	$snort_other_preprocs = <<<EOD
##################
                 #
# Other preprocs #
                 #
##################

preprocessor rpc_decode: 111 32770 32771 32772 32773 32774 32775 32776 32777 32778 32779
preprocessor bo
	
EOD;

	$def_other_preprocs_info_chk = $ifaceSettingsArray['other_preprocs'];
	if ($def_other_preprocs_info_chk === 'on') {
		$def_other_preprocs_type = $snort_other_preprocs;
	}else{
		$def_other_preprocs_type = '';
	}
	
	/* def ftp_preprocessor */
	$snort_ftp_preprocessor = <<<EOD
#####################
                    #
# ftp preprocessor  #
                    #
#####################

preprocessor ftp_telnet: global \
inspection_type stateless

preprocessor ftp_telnet_protocol: telnet \
   normalize \
   ayt_attack_thresh 200

preprocessor ftp_telnet_protocol: \
    ftp server default \
    def_max_param_len 100 \
    ports { 21 } \
    ftp_cmds { USER PASS ACCT CWD SDUP SMNT QUIT REIN PORT PASV TYPE STRU MODE } \
    ftp_cmds { RETR STOR STOU APPE ALLO REST RNFR RNTO ABOR DELE RMD MKD PWD } \
    ftp_cmds { LIST NLST SITE SYST STAT HELP NOOP } \
    ftp_cmds { AUTH ADAT PROT PBSZ CONF ENC } \
    ftp_cmds { FEAT CEL CMD MACB } \
    ftp_cmds { MDTM REST SIZE MLST MLSD } \
    ftp_cmds { XPWD XCWD XCUP XMKD XRMD TEST CLNT } \
    alt_max_param_len 0 { CDUP QUIT REIN PASV STOU ABOR PWD SYST NOOP } \
    alt_max_param_len 100 { MDTM CEL XCWD SITE USER PASS REST DELE RMD SYST TEST STAT MACB EPSV CLNT LPRT } \
    alt_max_param_len 200 { XMKD NLST ALLO STOU APPE RETR STOR CMD RNFR HELP } \
    alt_max_param_len 256 { RNTO CWD } \
    alt_max_param_len 400 { PORT } \
    alt_max_param_len 512 { SIZE } \
    chk_str_fmt { USER PASS ACCT CWD SDUP SMNT PORT TYPE STRU MODE } \
    chk_str_fmt { RETR STOR STOU APPE ALLO REST RNFR RNTO DELE RMD MKD } \
    chk_str_fmt { LIST NLST SITE SYST STAT HELP } \
    chk_str_fmt { AUTH ADAT PROT PBSZ CONF ENC } \
    chk_str_fmt { FEAT CEL CMD } \
    chk_str_fmt { MDTM REST SIZE MLST MLSD } \
    chk_str_fmt { XPWD XCWD XCUP XMKD XRMD TEST CLNT } \
    cmd_validity MODE < char ASBCZ > \
    cmd_validity STRU < char FRP > \
    cmd_validity ALLO < int [ char R int ] > \
    cmd_validity TYPE < { char AE [ char NTC ] | char I | char L [ number ] } > \
    cmd_validity MDTM < [ date nnnnnnnnnnnnnn[.n[n[n]]] ] string > \
    cmd_validity PORT < host_port >

preprocessor ftp_telnet_protocol: ftp client default \
   max_resp_len 256 \
   bounce yes \
   telnet_cmds yes
	
EOD;

	$def_ftp_preprocessor_info_chk = $ifaceSettingsArray['ftp_preprocessor'];
	if ($def_ftp_preprocessor_info_chk === 'on') {
		$def_ftp_preprocessor_type = $snort_ftp_preprocessor;
	}else{
		$def_ftp_preprocessor_type = "";
	}
		
	/* def smtp_preprocessor */
	$snort_smtp_preprocessor = <<<EOD
#####################
                    #
# SMTP preprocessor #
                    #
#####################

preprocessor SMTP: \
    ports { 25 465 691 } \
    inspection_type stateful \
    normalize cmds \
    valid_cmds { MAIL RCPT HELP HELO ETRN EHLO EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET SEND SAML SOML AUTH TURN ETRN PIPELINING \
CHUNKING DATA DSN RSET QUIT ONEX QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    normalize_cmds { MAIL RCPT HELP HELO ETRN EHLO EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET SEND SAML SOML AUTH TURN ETRN \
PIPELINING CHUNKING DATA DSN RSET QUIT ONEX QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    max_header_line_len 1000 \ 
    max_response_line_len 512 \
    alt_max_command_line_len 260 { MAIL } \
    alt_max_command_line_len 300 { RCPT } \
    alt_max_command_line_len 500 { HELP HELO ETRN EHLO } \
    alt_max_command_line_len 255 { EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET } \
    alt_max_command_line_len 246 { SEND SAML SOML AUTH TURN ETRN PIPELINING CHUNKING DATA DSN RSET QUIT ONEX } \
    alt_max_command_line_len 246 { QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR } \
    alt_max_command_line_len 246 { XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    xlink2state { enable }
	
EOD;

	$def_smtp_preprocessor_info_chk = $ifaceSettingsArray['smtp_preprocessor'];
	if ($def_smtp_preprocessor_info_chk === 'on') {
		$def_smtp_preprocessor_type = $snort_smtp_preprocessor;
	}else{
		$def_smtp_preprocessor_type = '';
	}

	/* def sf_portscan */
	$snort_sf_portscan = <<<EOD
################
               #
# sf Portscan  #
               #
################

preprocessor sfportscan: scan_type { all } \
                         proto  { all } \
                         memcap { 10000000 } \
                         sense_level { medium } \
                         ignore_scanners { \$HOME_NET }
	
EOD;

	$def_sf_portscan_info_chk = $ifaceSettingsArray['sf_portscan'];
	if ($def_sf_portscan_info_chk === 'on') {
		$def_sf_portscan_type = $snort_sf_portscan;
	}else{
		$def_sf_portscan_type = '';
	}
	
	/* def dce_rpc_2 */
	$snort_dce_rpc_2 = <<<EOD
###############
              #
# NEW         #
# DCE/RPC 2   #
              #
###############

preprocessor dcerpc2: memcap 102400, events [smb, co, cl]
preprocessor dcerpc2_server: default, policy WinXP, \
    detect [smb [139,445], tcp 135, udp 135, rpc-over-http-server 593], \
    autodetect [tcp 1025:, udp 1025:, rpc-over-http-server 1025:], \
    smb_max_chain 3
	
EOD;

	$def_dce_rpc_2_info_chk = $ifaceSettingsArray['dce_rpc_2'];
	if ($def_dce_rpc_2_info_chk === 'on') {
		$def_dce_rpc_2_type = $snort_dce_rpc_2;
	}else{
		$def_dce_rpc_2_type = '';
	}
	
	/* def dns_preprocessor */
	$snort_dns_preprocessor = <<<EOD
####################
                   #
# DNS preprocessor #
                   #
####################

preprocessor dns: \
    ports { 53 } \
    enable_rdata_overflow
	
EOD;

	$def_dns_preprocessor_info_chk = $ifaceSettingsArray['dns_preprocessor'];
	if ($def_dns_preprocessor_info_chk === 'on') {
		$def_dns_preprocessor_type = $snort_dns_preprocessor;
	}else{
		$def_dns_preprocessor_type = '';
	}
	
	/* def SSL_PORTS IGNORE */
	$def_ssl_ports_ignore_info_chk = $ifaceSettingsArray['def_ssl_ports_ignore'];
	if (empty($def_ssl_ports_ignore_info_chk)) {
		$def_ssl_ports_ignore_type = 'preprocessor ssl: ports { 443 465 563 636 989 990 992 993 994 995 }, trustservers, noinspect_encrypted';
	}else{
		$def_ssl_ports_ignore_type = "preprocessor ssl: ports { {$def_ssl_ports_ignore_info_chk} }, trustservers, noinspect_encrypted";
	}
	
	/* stream5 queued settings */


	$def_max_queued_bytes_info_chk = $ifaceSettingsArray['max_queued_bytes'];
	if (empty($def_max_queued_bytes_info_chk)) {
		$def_max_queued_bytes_type = '';
	}else{
		$def_max_queued_bytes_type = ' max_queued_bytes ' . $config['installedpackages']['snortglobal']['rule'][$id]['max_queued_bytes'] . ',';
	}

	$def_max_queued_segs_info_chk = $ifaceSettingsArray['max_queued_segs'];
	if (empty($def_max_queued_segs_info_chk)) {
		$def_max_queued_segs_type = '';
	}else{
		$def_max_queued_segs_type = ' max_queued_segs ' . $config['installedpackages']['snortglobal']['rule'][$id]['max_queued_segs'] . ',';
	}


	/* build snort configuration file */
	/* TODO; feed back from pfsense users to reduce false positives */
	$snort_conf_text = <<<EOD

# snort configuration file
# generated by the pfSense
# package manager system
# see /usr/local/pkg/snort.inc
# for more information
#	snort.conf
#   Snort can be found at http://www.snort.org/
#
#	Copyright (C) 2009-2010 Robert Zelaya
#	part of pfSense
#	All rights reserved.
#
#	Redistribution and use in source and binary forms, with or without
#	modification, are permitted provided that the following conditions are met:
#
#	1. Redistributions of source code must retain the above copyright notice,
#	   this list of conditions and the following disclaimer.
#
#	2. Redistributions in binary form must reproduce the above copyright
#	   notice, this list of conditions and the following disclaimer in the
#	   documentation and/or other materials provided with the distribution.
#
#	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
#	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
#	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
#	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#	POSSIBILITY OF SUCH DAMAGE.

#########################
                        #
# Define Local Network  #
                        #
#########################

var HOME_NET {$home_net}
var EXTERNAL_NET {$external_net}

###################
                  #
# Define Servers  #
                  #
###################

var DNS_SERVERS [{$def_dns_servers_type}]
var SMTP_SERVERS [{$def_smtp_servers_type}]
var HTTP_SERVERS [{$def_http_servers_type}]
var SQL_SERVERS [{$def_sql_servers_type}]
var TELNET_SERVERS [{$def_telnet_servers_type}]
var SNMP_SERVERS [{$def_snmp_servers_type}]
var FTP_SERVERS [{$def_ftp_servers_type}]
var SSH_SERVERS [{$def_ssh_servers_type}]
var POP_SERVERS [{$def_pop_servers_type}]
var IMAP_SERVERS [{$def_imap_servers_type}]
var RPC_SERVERS \$HOME_NET
var WWW_SERVERS [{$def_www_servers_type}]
var SIP_PROXY_IP [{$def_sip_proxy_ip_type}]
var AIM_SERVERS \
[64.12.24.0/23,64.12.28.0/23,64.12.161.0/24,64.12.163.0/24,64.12.200.0/24,205.188.3.0/24,205.188.5.0/24,205.188.7.0/24,205.188.9.0/24,205.188.153.0/24,205.188.179.0/24,205.188.248.0/24]

########################
                       #
# Define Server Ports  #
                       #
########################

portvar HTTP_PORTS [{$def_http_ports_type}]
portvar SHELLCODE_PORTS !80
portvar ORACLE_PORTS [{$def_oracle_ports_type}]
portvar AUTH_PORTS [{$def_auth_ports_type}]
portvar DNS_PORTS [{$def_dns_ports_type}]
portvar FINGER_PORTS [{$def_finger_ports_type}]
portvar FTP_PORTS [{$def_ftp_ports_type}]
portvar IMAP_PORTS [{$def_imap_ports_type}]
portvar IRC_PORTS [{$def_irc_ports_type}]
portvar MSSQL_PORTS [{$def_mssql_ports_type}]
portvar NNTP_PORTS [{$def_nntp_ports_type}]
portvar POP2_PORTS [{$def_pop2_ports_type}]
portvar POP3_PORTS [{$def_pop3_ports_type}]
portvar SUNRPC_PORTS [111,32770,32771,32772,32773,32774,32775,32776,32777,32778,32779]
portvar RLOGIN_PORTS [{$def_rlogin_ports_type}]
portvar RSH_PORTS [{$def_rsh_ports_type}]
portvar SMB_PORTS [139,445]
portvar SMTP_PORTS [{$def_smtp_ports_type}]
portvar SNMP_PORTS [{$def_snmp_ports_type}]
portvar SSH_PORTS [{$def_ssh_ports_type}]
portvar TELNET_PORTS [{$def_telnet_ports_type}]
portvar MAIL_PORTS [{$def_mail_ports_type}]
portvar SSL_PORTS [{$def_ssl_ports_type}]
portvar SIP_PROXY_PORTS [{$def_sip_proxy_ports_type}]

# DCERPC NCACN-IP-TCP
portvar DCERPC_NCACN_IP_TCP [139,445]
portvar DCERPC_NCADG_IP_UDP [138,1024:]
portvar DCERPC_NCACN_IP_LONG [135,139,445,593,1024:]
portvar DCERPC_NCACN_UDP_LONG [135,1024:]
portvar DCERPC_NCACN_UDP_SHORT [135,593,1024:]
portvar DCERPC_NCACN_TCP [2103,2105,2107]
portvar DCERPC_BRIGHTSTORE [6503,6504]

#####################
                    #
# Define Rule Paths #
                    #
#####################

var RULE_PATH /usr/local/etc/snort/sn_{$ifaceSettingsArray['uuid']}/rules
# var PREPROC_RULE_PATH ./preproc_rules

################################
                               #
# Configure the snort decoder  #
                               #
################################

config checksum_mode: all
config disable_decode_alerts
config disable_tcpopt_experimental_alerts
config disable_tcpopt_obsolete_alerts
config disable_ttcp_alerts
config disable_tcpopt_alerts
config disable_ipopt_alerts
config disable_decode_drops

###################################
                                  #
# Configure the detection engine  #
# Use lower memory models         #
                                  #
###################################

config detection: search-method {$snort_performance} max_queue_events 5
config event_queue: max_queue 8 log 3 order_events content_length

#Configure dynamic loaded libraries
dynamicpreprocessor directory /usr/local/lib/snort/dynamicpreprocessor/
dynamicengine /usr/local/lib/snort/dynamicengine/libsf_engine.so
dynamicdetection directory /usr/local/lib/snort/dynamicrules/

###################
                  #
# Flow and stream #
                  #
###################

preprocessor frag3_global: max_frags 8192
preprocessor frag3_engine: policy bsd detect_anomalies

preprocessor stream5_global: max_tcp 8192, track_tcp yes, \
track_udp yes, track_icmp yes
preprocessor stream5_tcp: policy BSD, ports both all,{$def_max_queued_bytes_type}{$def_max_queued_segs_type} use_static_footprint_sizes
preprocessor stream5_udp:
preprocessor stream5_icmp:

##########################
                         #
# NEW                    #
# Performance Statistics #
                         #
##########################

{$def_perform_stat_type}

{$def_http_inspect_type}

{$def_other_preprocs_type}

{$def_ftp_preprocessor_type}

{$def_smtp_preprocessor_type}

{$def_sf_portscan_type}

############################
                           #
# OLD                      #
# preprocessor dcerpc: \   #
#    autodetect \          #
#    max_frag_size 3000 \  #
#    memcap 100000         #
                           #
############################

{$def_dce_rpc_2_type}

{$def_dns_preprocessor_type}

##############################
                             #
# NEW                        #
# Ignore SSL and Encryption  #
                             #
##############################

{$def_ssl_ports_ignore_type}

#####################
                    #
# Snort Output Logs #
                    #
#####################

$snortunifiedlogbasic_type
$snortalertlogtype_type
$alertsystemlog_type
$tcpdumplog_type
$snortmysqllog_info_chk
$snortunifiedlog_type
$snortsam_type
						
#################
                #
# Misc Includes #
                #
#################

include /usr/local/etc/snort/sn_{$ifaceSettingsArray['uuid']}/reference.config
include /usr/local/etc/snort/sn_{$ifaceSettingsArray['uuid']}/classification.config
$threshold_file_name

# Snort user pass through configuration
{$snort_config_pass_thru}

###################
                  #
# Rules Selection #
                  #
###################

{$selected_rules_sections}

EOD;

	return $snort_conf_text;
}


function create_snort_conf($uuid)
{
	// write out snort.conf

		if (!file_exists("/usr/local/etc/snort/sn_{$uuid}/snort.conf")) {
			exec("/usr/bin/touch /usr/local/etc/snort/sn_{$uuid}/snort.conf");
		}

		$snort_conf_text = generate_snort_conf($uuid);
		
		conf_mount_rw();
		$conf = fopen("/usr/local/etc/snort/sn_{$uuid}/snort.conf", "w");
		if(!$conf) {
			log_error("Could not open /usr/local/etc/snort/sn_{$uuid}/snort.conf for writing.");
			exit;
		}
		
		fwrite($conf, $snort_conf_text);
		fclose($conf);
		conf_mount_ro();
		
}

function build_snort_settings($uuid) {
	
	// create snort.conf
	create_snort_conf($uuid);
	
}

// -------------------------- END snort.conf -------------------------

?>
