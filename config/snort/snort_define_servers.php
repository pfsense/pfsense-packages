<?php
/* $Id$ */
/*
	snort_interfaces.php
	part of m0n0wall (http://m0n0.ch/wall)

	Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
	Copyright (C) 2008-2009 Robert Zelaya.
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

/* 

TODO: Nov 12 09
Clean this code up its ugly
Important add error checking

*/

//require_once("globals.inc");
require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");
require_once("/usr/local/pkg/snort/snort.inc");

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}

//nat_rules_sort();
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

if (isset($_GET['dup'])) {
        $id = $_GET['dup'];
        $after = $_GET['dup'];
}


if (isset($id) && $a_nat[$id]) {

	/* old options */
	$pconfig['def_ssl_ports_ignore'] = $a_nat[$id]['def_ssl_ports_ignore'];
	$pconfig['flow_depth'] = $a_nat[$id]['flow_depth'];
	$pconfig['max_queued_bytes'] = $a_nat[$id]['max_queued_bytes'];
	$pconfig['max_queued_segs'] = $a_nat[$id]['max_queued_segs'];
	$pconfig['perform_stat'] = $a_nat[$id]['perform_stat'];
	$pconfig['http_inspect'] = $a_nat[$id]['http_inspect'];
	$pconfig['other_preprocs'] = $a_nat[$id]['other_preprocs'];
	$pconfig['ftp_preprocessor'] = $a_nat[$id]['ftp_preprocessor'];
	$pconfig['smtp_preprocessor'] = $a_nat[$id]['smtp_preprocessor'];
	$pconfig['sf_portscan'] = $a_nat[$id]['sf_portscan'];
	$pconfig['dce_rpc_2'] = $a_nat[$id]['dce_rpc_2'];
	$pconfig['dns_preprocessor'] = $a_nat[$id]['dns_preprocessor'];
	$pconfig['def_dns_servers'] = $a_nat[$id]['def_dns_servers'];
	$pconfig['def_dns_ports'] = $a_nat[$id]['def_dns_ports'];
	$pconfig['def_smtp_servers'] = $a_nat[$id]['def_smtp_servers'];
	$pconfig['def_smtp_ports'] = $a_nat[$id]['def_smtp_ports'];
	$pconfig['def_mail_ports'] = $a_nat[$id]['def_mail_ports'];
	$pconfig['def_http_servers'] = $a_nat[$id]['def_http_servers'];
	$pconfig['def_www_servers'] = $a_nat[$id]['def_www_servers'];
	$pconfig['def_http_ports'] = $a_nat[$id]['def_http_ports'];	      
	$pconfig['def_sql_servers'] = $a_nat[$id]['def_sql_servers'];
	$pconfig['def_oracle_ports'] = $a_nat[$id]['def_oracle_ports'];
	$pconfig['def_mssql_ports'] = $a_nat[$id]['def_mssql_ports'];
	$pconfig['def_telnet_servers'] = $a_nat[$id]['def_telnet_servers'];
	$pconfig['def_telnet_ports'] = $a_nat[$id]['def_telnet_ports'];
	$pconfig['def_snmp_servers'] = $a_nat[$id]['def_snmp_servers'];
	$pconfig['def_snmp_ports'] = $a_nat[$id]['def_snmp_ports'];
	$pconfig['def_ftp_servers'] = $a_nat[$id]['def_ftp_servers'];
	$pconfig['def_ftp_ports'] = $a_nat[$id]['def_ftp_ports'];
	$pconfig['def_ssh_servers'] = $a_nat[$id]['def_ssh_servers'];
	$pconfig['def_ssh_ports'] = $a_nat[$id]['def_ssh_ports'];
	$pconfig['def_pop_servers'] = $a_nat[$id]['def_pop_servers'];
	$pconfig['def_pop2_ports'] = $a_nat[$id]['def_pop2_ports'];
	$pconfig['def_pop3_ports'] = $a_nat[$id]['def_pop3_ports'];
	$pconfig['def_imap_servers'] = $a_nat[$id]['def_imap_servers'];
	$pconfig['def_imap_ports'] = $a_nat[$id]['def_imap_ports'];
	$pconfig['def_sip_proxy_ip'] = $a_nat[$id]['def_sip_proxy_ip'];
	$pconfig['def_sip_proxy_ports'] = $a_nat[$id]['def_sip_proxy_ports'];
	$pconfig['def_auth_ports'] = $a_nat[$id]['def_auth_ports'];
	$pconfig['def_finger_ports'] = $a_nat[$id]['def_finger_ports'];
	$pconfig['def_irc_ports'] = $a_nat[$id]['def_irc_ports'];
	$pconfig['def_nntp_ports'] = $a_nat[$id]['def_nntp_ports'];
	$pconfig['def_rlogin_ports'] = $a_nat[$id]['def_rlogin_ports'];
	$pconfig['def_rsh_ports'] = $a_nat[$id]['def_rsh_ports'];
	$pconfig['def_ssl_ports'] = $a_nat[$id]['def_ssl_ports'];
	$pconfig['barnyard_enable'] = $a_nat[$id]['barnyard_enable'];
	$pconfig['barnyard_mysql'] = $a_nat[$id]['barnyard_mysql'];
	$pconfig['enable'] = $a_nat[$id]['enable'];
	$pconfig['uuid'] = $a_nat[$id]['uuid'];
	$pconfig['interface'] = $a_nat[$id]['interface'];
	$pconfig['whitelistname'] = $a_nat[$id]['whitelistname'];
	$pconfig['homelistname'] = $a_nat[$id]['homelistname'];
	$pconfig['externallistname'] = $a_nat[$id]['externallistname'];
	$pconfig['suppresslistname'] = $a_nat[$id]['suppresslistname'];
	$pconfig['descr'] = $a_nat[$id]['descr'];
	$pconfig['performance'] = $a_nat[$id]['performance'];
	$pconfig['blockoffenders7'] = $a_nat[$id]['blockoffenders7'];
	$pconfig['alertsystemlog'] = $a_nat[$id]['alertsystemlog'];
	$pconfig['tcpdumplog'] = $a_nat[$id]['tcpdumplog'];
	$pconfig['snortunifiedlog'] = $a_nat[$id]['snortunifiedlog'];
	$pconfig['configpassthru'] = $a_nat[$id]['configpassthru'];
	$pconfig['barnconfigpassthru'] = $a_nat[$id]['barnconfigpassthru'];
	$pconfig['rulesets'] = $a_nat[$id]['rulesets'];
	$pconfig['rule_sid_off'] = $a_nat[$id]['rule_sid_off'];
	$pconfig['rule_sid_on'] = $a_nat[$id]['rule_sid_on'];

if (isset($_GET['dup']))
	unset($id);	
}

/* convert fake interfaces to real */
$if_real = convert_friendly_interface_to_real_interface_name2($pconfig['interface']);

$snort_uuid = $config['installedpackages']['snortglobal']['rule'][$id]['uuid'];

/* alert file */
$d_snortconfdirty_path = "/var/run/snort_conf_{$snort_uuid}_{$if_real}.dirty";


	if ($_POST["Submit"]) {

	/* check for overlaps */

/* if no errors write to conf */
	if (!$input_errors) {
		$natent = array();
	/* repost the options already in conf */
	if ($pconfig['interface'] != "") { $natent['interface'] = $pconfig['interface']; }
	if ($pconfig['enable'] != "") { $natent['enable'] = $pconfig['enable']; }
	if ($pconfig['uuid'] != "") { $natent['uuid'] = $pconfig['uuid']; }
	if ($pconfig['descr'] != "") { $natent['descr'] = $pconfig['descr']; }
	if ($pconfig['performance'] != "") { $natent['performance'] = $pconfig['performance']; }
	if ($pconfig['blockoffenders7'] != "") { $natent['blockoffenders7'] = $pconfig['blockoffenders7']; }
	if ($pconfig['alertsystemlog'] != "") { $natent['alertsystemlog'] = $pconfig['alertsystemlog']; }
	if ($pconfig['tcpdumplog'] != "") { $natent['tcpdumplog'] = $pconfig['tcpdumplog']; }
	if ($pconfig['snortunifiedlog'] != "") { $natent['snortunifiedlog'] = $pconfig['snortunifiedlog']; }	
	if ($pconfig['def_ssl_ports_ignore'] != "") { $natent['def_ssl_ports_ignore'] = $pconfig['def_ssl_ports_ignore']; }
	if ($pconfig['flow_depth'] != "") { $natent['flow_depth'] = $pconfig['flow_depth']; }
	if ($pconfig['max_queued_bytes'] != "") { $natent['max_queued_bytes'] = $pconfig['max_queued_bytes']; }
	if ($pconfig['max_queued_segs'] != "") { $natent['max_queued_segs'] = $pconfig['max_queued_segs']; }
	if ($pconfig['perform_stat'] != "") { $natent['perform_stat'] = $pconfig['perform_stat']; }
	if ($pconfig['http_inspect'] != "") { $natent['http_inspect'] = $pconfig['http_inspect']; }
	if ($pconfig['other_preprocs'] != "") { $natent['other_preprocs'] = $pconfig['other_preprocs']; }
	if ($pconfig['ftp_preprocessor'] != "") { $natent['ftp_preprocessor'] = $pconfig['ftp_preprocessor']; }
	if ($pconfig['smtp_preprocessor'] != "") { $natent['smtp_preprocessor'] = $pconfig['smtp_preprocessor']; }
	if ($pconfig['sf_portscan'] != "") { $natent['sf_portscan'] = $pconfig['sf_portscan']; }
	if ($pconfig['dce_rpc_2'] != "") { $natent['dce_rpc_2'] = $pconfig['dce_rpc_2']; }
	if ($pconfig['dns_preprocessor'] != "") { $natent['dns_preprocessor'] = $pconfig['dns_preprocessor']; }
	if ($pconfig['barnyard_enable'] != "") { $natent['barnyard_enable'] = $pconfig['barnyard_enable']; }
	if ($pconfig['barnyard_mysql'] != "") { $natent['barnyard_mysql'] = $pconfig['barnyard_mysql']; }
	if ($pconfig['rulesets'] != "") { $natent['rulesets'] = $pconfig['rulesets']; }
	if ($pconfig['rule_sid_off'] != "") { $natent['rule_sid_off'] = $pconfig['rule_sid_off']; }
	if ($pconfig['rule_sid_on'] != "") { $natent['rule_sid_on'] = $pconfig['rule_sid_on']; }
	if ($pconfig['configpassthru'] != "") { $natent['configpassthru'] = $pconfig['configpassthru'];	}
	if ($pconfig['barnconfigpassthru'] != "") { $natent['barnconfigpassthru'] = $pconfig['barnconfigpassthru'];	}	
	if ($pconfig['whitelistname'] != "") { $natent['whitelistname'] = $pconfig['whitelistname']; }
	if ($pconfig['homelistname'] != "") { $natent['homelistname'] = $pconfig['homelistname'];	}
	if ($pconfig['externallistname'] != "") { $natent['externallistname'] = $pconfig['externallistname'];	}
	if ($pconfig['suppresslistname'] != "") { $natent['suppresslistname'] = $pconfig['suppresslistname'];	}

		
		/* post new options */
		if ($_POST['def_dns_servers'] != "") { $natent['def_dns_servers'] = $_POST['def_dns_servers']; }else{ $natent['def_dns_servers'] = ""; }
		if ($_POST['def_dns_ports'] != "") { $natent['def_dns_ports'] = $_POST['def_dns_ports']; }else{ $natent['def_dns_ports'] = ""; }		
		if ($_POST['def_smtp_servers'] != "") { $natent['def_smtp_servers'] = $_POST['def_smtp_servers']; }else{ $natent['def_smtp_servers'] = ""; }
		if ($_POST['def_smtp_ports'] != "") { $natent['def_smtp_ports'] = $_POST['def_smtp_ports']; }else{ $natent['def_smtp_ports'] = ""; }
		if ($_POST['def_mail_ports'] != "") { $natent['def_mail_ports'] = $_POST['def_mail_ports']; }else{ $natent['def_mail_ports'] = ""; }
		if ($_POST['def_http_servers'] != "") { $natent['def_http_servers'] = $_POST['def_http_servers']; }else{ $natent['def_http_servers'] = ""; }		
		if ($_POST['def_www_servers'] != "") { $natent['def_www_servers'] = $_POST['def_www_servers']; }else{ $natent['def_www_servers'] = ""; }
		if ($_POST['def_http_ports'] != "") { $natent['def_http_ports'] = $_POST['def_http_ports']; }else{ $natent['def_http_ports'] = ""; }
		if ($_POST['def_sql_servers'] != "") { $natent['def_sql_servers'] = $_POST['def_sql_servers']; }else{ $natent['def_sql_servers'] = ""; }
		if ($_POST['def_oracle_ports'] != "") { $natent['def_oracle_ports'] = $_POST['def_oracle_ports']; }else{ $natent['def_oracle_ports'] = ""; }		
		if ($_POST['def_mssql_ports'] != "") { $natent['def_mssql_ports'] = $_POST['def_mssql_ports']; }else{ $natent['def_mssql_ports'] = ""; }
		if ($_POST['def_telnet_servers'] != "") { $natent['def_telnet_servers'] = $_POST['def_telnet_servers']; }else{ $natent['def_telnet_servers'] = ""; }
		if ($_POST['def_telnet_ports'] != "") { $natent['def_telnet_ports'] = $_POST['def_telnet_ports']; }else{ $natent['def_telnet_ports'] = ""; }
		if ($_POST['def_snmp_servers'] != "") { $natent['def_snmp_servers'] = $_POST['def_snmp_servers']; }else{ $natent['def_snmp_servers'] = ""; }		
		if ($_POST['def_snmp_ports'] != "") { $natent['def_snmp_ports'] = $_POST['def_snmp_ports']; }else{ $natent['def_snmp_ports'] = ""; }
		if ($_POST['def_ftp_servers'] != "") { $natent['def_ftp_servers'] = $_POST['def_ftp_servers']; }else{ $natent['def_ftp_servers'] = ""; }
		if ($_POST['def_ftp_ports'] != "") { $natent['def_ftp_ports'] = $_POST['def_ftp_ports']; }else{ $natent['def_ftp_ports'] = ""; }
		if ($_POST['def_ssh_servers'] != "") { $natent['def_ssh_servers'] = $_POST['def_ssh_servers']; }else{ $natent['def_ssh_servers'] = ""; }				
		if ($_POST['def_ssh_ports'] != "") { $natent['def_ssh_ports'] = $_POST['def_ssh_ports']; }else{ $natent['def_ssh_ports'] = ""; }
		if ($_POST['def_pop_servers'] != "") { $natent['def_pop_servers'] = $_POST['def_pop_servers']; }else{ $natent['def_pop_servers'] = ""; }
		if ($_POST['def_pop2_ports'] != "") { $natent['def_pop2_ports'] = $_POST['def_pop2_ports']; }else{ $natent['def_pop2_ports'] = ""; }
		if ($_POST['def_pop3_ports'] != "") { $natent['def_pop3_ports'] = $_POST['def_pop3_ports']; }else{ $natent['def_pop3_ports'] = ""; }		
		if ($_POST['def_imap_servers'] != "") { $natent['def_imap_servers'] = $_POST['def_imap_servers']; }else{ $natent['def_imap_servers'] = ""; }
		if ($_POST['def_imap_ports'] != "") { $natent['def_imap_ports'] = $_POST['def_imap_ports']; }else{ $natent['def_imap_ports'] = ""; }
		if ($_POST['def_sip_proxy_ip'] != "") { $natent['def_sip_proxy_ip'] = $_POST['def_sip_proxy_ip']; }else{ $natent['def_sip_proxy_ip'] = ""; }
		if ($_POST['def_sip_proxy_ports'] != "") { $natent['def_sip_proxy_ports'] = $_POST['def_sip_proxy_ports']; }else{ $natent['def_sip_proxy_ports'] = ""; }		
		if ($_POST['def_auth_ports'] != "") { $natent['def_auth_ports'] = $_POST['def_auth_ports']; }else{ $natent['def_auth_ports'] = ""; }
		if ($_POST['def_finger_ports'] != "") { $natent['def_finger_ports'] = $_POST['def_finger_ports']; }else{ $natent['def_finger_ports'] = ""; }
		if ($_POST['def_irc_ports'] != "") { $natent['def_irc_ports'] = $_POST['def_irc_ports']; }else{ $natent['def_irc_ports'] = ""; }
		if ($_POST['def_nntp_ports'] != "") { $natent['def_nntp_ports'] = $_POST['def_nntp_ports']; }else{ $natent['def_nntp_ports'] = ""; }		
		if ($_POST['def_rlogin_ports'] != "") { $natent['def_rlogin_ports'] = $_POST['def_rlogin_ports']; }else{ $natent['def_rlogin_ports'] = ""; }
		if ($_POST['def_rsh_ports'] != "") { $natent['def_rsh_ports'] = $_POST['def_rsh_ports']; }else{ $natent['def_rsh_ports'] = ""; }
		if ($_POST['def_ssl_ports'] != "") { $natent['def_ssl_ports'] = $_POST['def_ssl_ports']; }else{ $natent['def_ssl_ports'] = ""; }


		if (isset($id) && $a_nat[$id])
			$a_nat[$id] = $natent;
		else {
			if (is_numeric($after))
				array_splice($a_nat, $after+1, 0, array($natent));
			else
				$a_nat[] = $natent;
		}

		write_config();
        
		/* after click go to this page */
		
		touch($d_snortconfdirty_path);
		
		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		sleep(2);
				
		header("Location: snort_define_servers.php?id=$id");
		
		exit;
	}
}
	
	/* this will exec when alert says apply */
	if ($_POST['apply']) {
		
		if (file_exists($d_snortconfdirty_path)) {
			
			write_config();
			
			sync_snort_package_all($id, $if_real, $snort_uuid);
			sync_snort_package();
			
			unlink($d_snortconfdirty_path);
			
		}
		
	}

$pgtitle = "Snort: Interface $id$if_real Define Servers";
include("/usr/local/pkg/snort/snort_head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<?php include("fbegin.inc"); ?>
<?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>

<?php
echo "{$snort_general_css}\n";
?>

<div class="body2">

<noscript><div class="alert" ALIGN=CENTER><img src="../themes/nervecenter/images/icons/icon_alert.gif"/><strong>Please enable JavaScript to view this content</CENTER></div></noscript>

<form action="snort_define_servers.php" method="post" enctype="multipart/form-data" name="iform" id="iform">

<?php

	/* Display Alert message */

	if ($input_errors) {
	print_input_errors($input_errors); // TODO: add checks
	}

	if ($savemsg) {
	print_info_box2($savemsg);
	}

	//if (file_exists($d_snortconfdirty_path)) {
	if (file_exists($d_snortconfdirty_path) || file_exists("/var/run/snort_conf_{$snort_uuid}_.dirty")) {
	echo '<p>';

		if($savemsg) {
			print_info_box_np2("{$savemsg}");
		}else{
			print_info_box_np2('
			The Snort configuration has changed and snort needs to be restarted on this interface.<br>
			You must apply the changes in order for them to take effect.<br>
			');
		}
	}

?>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
<?php
if($id != "") 
{

echo '<div class="snorttabs" style="margin:1px 0px; width:775px;">' . "\n";
echo '<!-- Tabbed bar code -->' . "\n";
echo '<ul class="snorttabs">' . "\n";
	echo '<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>' . "\n";
	echo "<li><a href=\"/snort/snort_interfaces_edit.php?id={$id}\"><span>If Settings</span></a></li>\n";
    echo "<li><a href=\"/snort/snort_rulesets.php?id={$id}\"><span>Categories</span></a></li>\n";
    echo "<li><a href=\"/snort/snort_rules.php?id={$id}\"><span>Rules</span></a></li>\n";
	echo "<li class=\"snorttabs_active\"><a href=\"/snort/snort_define_servers.php?id={$id}\"><span>Servers</span></a></li>\n";
    echo "<li><a href=\"/snort/snort_preprocessors.php?id={$id}\"><span>Preprocessors</span></a></li>\n";
    echo "<li><a href=\"/snort/snort_barnyard.php?id={$id}\"><span>Barnyard2</span></a></li>\n";
echo '</ul>' . "\n";
echo '</div>' . "\n";

}
?>
</td>
</tr>
				<tr>
				<td class="tabcont">
				<table width="100%" border="0" cellpadding="6" cellspacing="0">
				<?php
				/* display error code if there is no id */
				if($id == "") 
				{
				echo "				
				<style type=\"text/css\">
				.noid {
				position:absolute;
				top:10px;
				left:0px;
				width:94%;
				background:#FCE9C0;
				background-position: 15px; 
				border-top:2px solid #DBAC48;
				border-bottom:2px solid #DBAC48;
				padding: 15px 10px 85% 50px;
				}
				</style> 
				<div class=\"alert\" ALIGN=CENTER><img src=\"/themes/nervecenter/images/icons/icon_alert.gif\"/><strong>You can not edit options without an interface ID.</CENTER></div>\n";
				
				}
				?>
				<tr>
					<td width="22%" valign="top">&nbsp;</td>
					<td width="78%"><span class="vexpl"><span class="red"><strong>Note:</strong></span><br>
					Please save your settings before you click start.<br>
					Please make sure there are <strong>no spaces</strong> in your definitions.
					</td>
				</tr>
                <tr>
                  <td colspan="2" valign="top" class="listtopic">Define Servers</td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define DNS_SERVERS</td>
                  <td width="78%" class="vtable">
                    <input name="def_dns_servers" type="text" class="formfld" id="def_dns_servers" size="40" value="<?=htmlspecialchars($pconfig['def_dns_servers']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define DNS_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_dns_ports" type="text" class="formfld" id="def_dns_ports" size="40" value="<?=htmlspecialchars($pconfig['def_dns_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 53.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define SMTP_SERVERS</td>
                  <td width="78%" class="vtable">
                    <input name="def_smtp_servers" type="text" class="formfld" id="def_smtp_servers" size="40" value="<?=htmlspecialchars($pconfig['def_smtp_servers']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define SMTP_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_smtp_ports" type="text" class="formfld" id="def_smtp_ports" size="40" value="<?=htmlspecialchars($pconfig['def_smtp_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 25.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define Mail_Ports</td>
                  <td width="78%" class="vtable">
                    <input name="def_mail_ports" type="text" class="formfld" id="def_mail_ports" size="40" value="<?=htmlspecialchars($pconfig['def_mail_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 25,143,465,691.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define HTTP_SERVERS</td>
                  <td width="78%" class="vtable">
                    <input name="def_http_servers" type="text" class="formfld" id="def_http_servers" size="40" value="<?=htmlspecialchars($pconfig['def_http_servers']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define WWW_SERVERS</td>
                  <td width="78%" class="vtable">
                    <input name="def_www_servers" type="text" class="formfld" id="def_www_servers" size="40" value="<?=htmlspecialchars($pconfig['def_www_servers']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define HTTP_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_http_ports" type="text" class="formfld" id="def_http_ports" size="40" value="<?=htmlspecialchars($pconfig['def_http_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 80.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define SQL_SERVERS</td>
                  <td width="78%" class="vtable">
                    <input name="def_sql_servers" type="text" class="formfld" id="def_sql_servers" size="40" value="<?=htmlspecialchars($pconfig['def_sql_servers']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define ORACLE_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_oracle_ports" type="text" class="formfld" id="def_oracle_ports" size="40" value="<?=htmlspecialchars($pconfig['def_oracle_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 1521.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define MSSQL_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_mssql_ports" type="text" class="formfld" id="def_mssql_ports" size="40" value="<?=htmlspecialchars($pconfig['def_mssql_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 1433.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define TELNET_SERVERS</td>
                  <td width="78%" class="vtable">
                    <input name="def_telnet_servers" type="text" class="formfld" id="def_telnet_servers" size="40" value="<?=htmlspecialchars($pconfig['def_telnet_servers']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define TELNET_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_telnet_ports" type="text" class="formfld" id="def_telnet_ports" size="40" value="<?=htmlspecialchars($pconfig['def_telnet_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 23.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define SNMP_SERVERS</td>
                  <td width="78%" class="vtable">
                    <input name="def_snmp_servers" type="text" class="formfld" id="def_snmp_servers" size="40" value="<?=htmlspecialchars($pconfig['def_snmp_servers']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define SNMP_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_snmp_ports" type="text" class="formfld" id="def_snmp_ports" size="40" value="<?=htmlspecialchars($pconfig['def_snmp_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 161.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define FTP_SERVERS</td>
                  <td width="78%" class="vtable">
                    <input name="def_ftp_servers" type="text" class="formfld" id="def_ftp_servers" size="40" value="<?=htmlspecialchars($pconfig['def_ftp_servers']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define FTP_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_ftp_ports" type="text" class="formfld" id="def_ftp_ports" size="40" value="<?=htmlspecialchars($pconfig['def_ftp_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 21.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define SSH_SERVERS</td>
                  <td width="78%" class="vtable">
                    <input name="def_ssh_servers" type="text" class="formfld" id="def_ssh_servers" size="40" value="<?=htmlspecialchars($pconfig['def_ssh_servers']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define SSH_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_ssh_ports" type="text" class="formfld" id="def_ssh_ports" size="40" value="<?=htmlspecialchars($pconfig['def_ssh_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is Pfsense SSH port.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define POP_SERVERS</td>
                  <td width="78%" class="vtable">
                    <input name="def_pop_servers" type="text" class="formfld" id="def_pop_servers" size="40" value="<?=htmlspecialchars($pconfig['def_pop_servers']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define POP2_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_pop2_ports" type="text" class="formfld" id="def_pop2_ports" size="40" value="<?=htmlspecialchars($pconfig['def_pop2_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 109.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define POP3_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_pop3_ports" type="text" class="formfld" id="def_pop3_ports" size="40" value="<?=htmlspecialchars($pconfig['def_pop3_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 110.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define IMAP_SERVERS</td>
                  <td width="78%" class="vtable">
                    <input name="def_imap_servers" type="text" class="formfld" id="def_imap_servers" size="40" value="<?=htmlspecialchars($pconfig['def_imap_servers']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define IMAP_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_imap_ports" type="text" class="formfld" id="def_imap_ports" size="40" value="<?=htmlspecialchars($pconfig['def_imap_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 143.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define SIP_PROXY_IP</td>
                  <td width="78%" class="vtable">
                    <input name="def_sip_proxy_ip" type="text" class="formfld" id="def_sip_proxy_ip" size="40" value="<?=htmlspecialchars($pconfig['def_sip_proxy_ip']);?>">
                    <br> <span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define SIP_PROXY_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_sip_proxy_ports" type="text" class="formfld" id="def_sip_proxy_ports" size="40" value="<?=htmlspecialchars($pconfig['def_sip_proxy_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 5060:5090,16384:32768.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define AUTH_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_auth_ports" type="text" class="formfld" id="def_auth_ports" size="40" value="<?=htmlspecialchars($pconfig['def_auth_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 113.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define FINGER_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_finger_ports" type="text" class="formfld" id="def_finger_ports" size="40" value="<?=htmlspecialchars($pconfig['def_finger_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 79.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define IRC_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_irc_ports" type="text" class="formfld" id="def_irc_ports" size="40" value="<?=htmlspecialchars($pconfig['def_irc_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 6665,6666,6667,6668,6669,7000.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define NNTP_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_nntp_ports" type="text" class="formfld" id="def_nntp_ports" size="40" value="<?=htmlspecialchars($pconfig['def_nntp_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 119.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define RLOGIN_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_rlogin_ports" type="text" class="formfld" id="def_rlogin_ports" size="40" value="<?=htmlspecialchars($pconfig['def_rlogin_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 513.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define RSH_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_rsh_ports" type="text" class="formfld" id="def_rsh_ports" size="40" value="<?=htmlspecialchars($pconfig['def_rsh_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 514.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Define SSL_PORTS</td>
                  <td width="78%" class="vtable">
                    <input name="def_ssl_ports" type="text" class="formfld" id="def_ssl_ports" size="40" value="<?=htmlspecialchars($pconfig['def_ssl_ports']);?>">
                    <br> <span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 25,443,465,636,993,995.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top">&nbsp;</td>
                  <td width="78%">
                    <input name="Submit" type="submit" class="formbtn" value="Save"> <input type="button" class="formbtn" value="Cancel" onclick="history.back()">
                    <?php if (isset($id) && $a_nat[$id]): ?>
                    <input name="id" type="hidden" value="<?=$id;?>">
                    <?php endif; ?>
                  </td>
                </tr>
	<tr>
	  <td width="22%" valign="top">&nbsp;</td>
	  <td width="78%"><span class="vexpl"><span class="red"><strong>Note:</strong></span>
	  <br>
		Please save your settings before you click start. </td>
	</tr>
  </table>
  </table>
</form>

</div>


<?php include("fend.inc"); ?>
</body>
</html>
