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

require("guiconfig.inc");

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

	/* new options */
	$pconfig['def_ssl_ports_ignore'] = $a_nat[$id]['def_ssl_ports_ignore'];
	$pconfig['flow_depth'] = $a_nat[$id]['flow_depth'];
	$pconfig['perform_stat'] = $a_nat[$id]['perform_stat'];
	$pconfig['http_inspect'] = $a_nat[$id]['http_inspect'];
	$pconfig['other_preprocs'] = $a_nat[$id]['other_preprocs'];
	$pconfig['ftp_preprocessor'] = $a_nat[$id]['ftp_preprocessor'];
	$pconfig['smtp_preprocessor'] = $a_nat[$id]['smtp_preprocessor'];
	$pconfig['sf_portscan'] = $a_nat[$id]['sf_portscan'];
	$pconfig['dce_rpc_2'] = $a_nat[$id]['dce_rpc_2'];
	$pconfig['dns_preprocessor'] = $a_nat[$id]['dns_preprocessor'];
	
	/* old options */
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
	$pconfig['ip def_sip_proxy_ports'] = $a_nat[$id]['ip def_sip_proxy_ports'];
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
	$pconfig['interface'] = $a_nat[$id]['interface'];
	$pconfig['descr'] = $a_nat[$id]['descr'];
	$pconfig['performance'] = $a_nat[$id]['performance'];
	$pconfig['blockoffenders7'] = $a_nat[$id]['blockoffenders7'];
	$pconfig['alertsystemlog'] = $a_nat[$id]['alertsystemlog'];
	$pconfig['tcpdumplog'] = $a_nat[$id]['tcpdumplog'];
	$pconfig['snortunifiedlog'] = $a_nat[$id]['snortunifiedlog'];
	$pconfig['flow_depth'] = $a_nat[$id]['flow_depth'];
	$pconfig['rulesets'] = $a_nat[$id]['rulesets'];
	$pconfig['rule_sid_off'] = $a_nat[$id]['rule_sid_off'];
	$pconfig['rule_sid_on'] = $a_nat[$id]['rule_sid_on'];

if (isset($_GET['dup']))
	unset($id);	
}

/* convert fake interfaces to real */
$if_real = convert_friendly_interface_to_real_interface_name($pconfig['interface']);

if ($_POST) {

	/* check for overlaps */

/* if no errors write to conf */
	if (!$input_errors) {
		$natent = array();
		/* repost the options already in conf */
	if ($pconfig['interface'] != "") { $natent['interface'] = $pconfig['interface']; }
	if ($pconfig['enable'] != "") { $natent['enable'] = $pconfig['enable']; }
	if ($pconfig['descr'] != "") { $natent['descr'] = $pconfig['descr']; }
	if ($pconfig['performance'] != "") { $natent['performance'] = $pconfig['performance']; }
	if ($pconfig['blockoffenders7'] != "") { $natent['blockoffenders7'] = $pconfig['blockoffenders7']; }
	if ($pconfig['alertsystemlog'] != "") { $natent['alertsystemlog'] = $pconfig['alertsystemlog']; }
	if ($pconfig['tcpdumplog'] != "") { $natent['tcpdumplog'] = $pconfig['tcpdumplog']; }
	if ($pconfig['snortunifiedlog'] != "") { $natent['snortunifiedlog'] = $pconfig['snortunifiedlog']; }
	if ($pconfig['barnyard_enable'] != "") { $natent['barnyard_enable'] = $pconfig['barnyard_enable']; }
	if ($pconfig['barnyard_mysql'] != "") { $natent['barnyard_mysql'] = $pconfig['barnyard_mysql']; }
	if ($pconfig['def_dns_servers'] != "") { $natent['def_dns_servers'] = $pconfig['def_dns_servers']; }
	if ($pconfig['def_dns_ports'] != "") { $natent['def_dns_ports'] = $pconfig['def_dns_ports']; }
	if ($pconfig['def_smtp_servers'] != "") { $natent['def_smtp_servers'] = $pconfig['def_smtp_servers']; }
	if ($pconfig['def_smtp_ports'] != "") { $natent['def_smtp_ports'] = $pconfig['def_smtp_ports']; }
	if ($pconfig['def_mail_ports'] != "") { $natent['def_mail_ports'] = $pconfig['def_mail_ports']; }
	if ($pconfig['def_http_servers'] != "") { $natent['def_http_servers'] = $pconfig['def_http_servers']; }
	if ($pconfig['def_www_servers'] != "") { $natent['def_www_servers'] = $pconfig['def_www_servers']; }
	if ($pconfig['def_http_ports'] != "") { $natent['def_http_ports'] = $pconfig['def_http_ports'];	}      
	if ($pconfig['def_sql_servers'] != "") { $natent['def_sql_servers'] = $pconfig['def_sql_servers']; }
	if ($pconfig['def_oracle_ports'] != "") { $natent['def_oracle_ports'] = $pconfig['def_oracle_ports']; }
	if ($pconfig['def_mssql_ports'] != "") { $natent['def_mssql_ports'] = $pconfig['def_mssql_ports']; }
	if ($pconfig['def_telnet_servers'] != "") { $natent['def_telnet_servers'] = $pconfig['def_telnet_servers']; }
	if ($pconfig['def_telnet_ports'] != "") { $natent['def_telnet_ports'] = $pconfig['def_telnet_ports']; }
	if ($pconfig['def_snmp_servers'] != "") { $natent['def_snmp_servers'] = $pconfig['def_snmp_servers']; }
	if ($pconfig['def_snmp_ports'] != "") { $natent['def_snmp_ports'] = $pconfig['def_snmp_ports']; }
	if ($pconfig['def_ftp_servers'] != "") { $natent['def_ftp_servers'] = $pconfig['def_ftp_servers']; }
	if ($pconfig['def_ftp_ports'] != "") { $natent['def_ftp_ports'] = $pconfig['def_ftp_ports']; }
	if ($pconfig['def_ssh_servers'] != "") { $natent['def_ssh_servers'] = $pconfig['def_ssh_servers']; }
	if ($pconfig['def_ssh_ports'] != "") { $natent['def_ssh_ports'] = $pconfig['def_ssh_ports']; }
	if ($pconfig['def_pop_servers'] != "") { $natent['def_pop_servers'] = $pconfig['def_pop_servers']; }
	if ($pconfig['def_pop2_ports'] != "") { $natent['def_pop2_ports'] = $pconfig['def_pop2_ports']; }
	if ($pconfig['def_pop3_ports'] != "") { $natent['def_pop3_ports'] = $pconfig['def_pop3_ports']; }
	if ($pconfig['def_imap_servers'] != "") { $natent['def_imap_servers'] = $pconfig['def_imap_servers']; }
	if ($pconfig['def_imap_ports'] != "") { $natent['def_imap_ports'] = $pconfig['def_imap_ports']; }
	if ($pconfig['def_sip_proxy_ip'] != "") { $natent['def_sip_proxy_ip'] = $pconfig['def_sip_proxy_ip']; }
	if ($pconfig['ip def_sip_proxy_ports'] != "") { $natent['ip def_sip_proxy_ports'] = $pconfig['ip def_sip_proxy_ports']; }
	if ($pconfig['def_auth_ports'] != "") { $natent['def_auth_ports'] = $pconfig['def_auth_ports']; }
	if ($pconfig['def_finger_ports'] != "") { $natent['def_finger_ports'] = $pconfig['def_finger_ports']; }
	if ($pconfig['def_irc_ports'] != "") { $natent['def_irc_ports'] = $pconfig['def_irc_ports']; }
	if ($pconfig['def_nntp_ports'] != "") { $natent['def_nntp_ports'] = $pconfig['def_nntp_ports']; }
	if ($pconfig['def_rlogin_ports'] != "") { $natent['def_rlogin_ports'] = $pconfig['def_rlogin_ports']; }
	if ($pconfig['def_rsh_ports'] != "") { $natent['def_rsh_ports'] = $pconfig['def_rsh_ports']; }
	if ($pconfig['def_ssl_ports'] != "") { $natent['def_ssl_ports'] = $pconfig['def_ssl_ports']; }
	if ($pconfig['rulesets'] != "") { $natent['rulesets'] = $pconfig['rulesets']; }
	if ($pconfig['rule_sid_off'] != "") { $natent['rule_sid_off'] = $pconfig['rule_sid_off']; }
	if ($pconfig['rule_sid_on'] != "") { $natent['rule_sid_on'] = $pconfig['rule_sid_on']; }
		
		/* post new options */
		$natent['perform_stat'] = $_POST['perform_stat'];
		if ($_POST['def_ssl_ports_ignore'] != "") { $natent['def_ssl_ports_ignore'] = $_POST['def_ssl_ports_ignore']; }else{ $natent['def_ssl_ports_ignore'] = ""; }
		if ($_POST['flow_depth'] != "") { $natent['flow_depth'] = $_POST['flow_depth']; }else{ $natent['flow_depth'] = ""; }
		$natent['perform_stat'] = $_POST['perform_stat'] ? on : off;
		$natent['http_inspect'] = $_POST['http_inspect'] ? on : off;
		$natent['other_preprocs'] = $_POST['other_preprocs'] ? on : off;
		$natent['ftp_preprocessor'] = $_POST['ftp_preprocessor'] ? on : off;
		$natent['smtp_preprocessor'] = $_POST['smtp_preprocessor'] ? on : off;
		$natent['sf_portscan'] = $_POST['sf_portscan'] ? on : off;
		$natent['dce_rpc_2'] = $_POST['dce_rpc_2'] ? on : off;
		$natent['dns_preprocessor'] = $_POST['dns_preprocessor'] ? on : off;		

		if (isset($id) && $a_nat[$id])
			$a_nat[$id] = $natent;
		else {
			if (is_numeric($after))
				array_splice($a_nat, $after+1, 0, array($natent));
			else
				$a_nat[] = $natent;
		}

		/* enable this if you want the user to aprove changes */
		// touch($d_natconfdirty_path);

		write_config();
        
		/* after click go to this page */
		header("Location: snort_preprocessors.php?id=$id");
		exit;
	}
}

$pgtitle = "Snort: Interface $id$if_real Preprocessors and Flow";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php 
include("fbegin.inc");
?>
<style type="text/css">
.alert {
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
<noscript><div class="alert" ALIGN=CENTER><img src="../themes/nervecenter/images/icons/icon_alert.gif"/><strong>Please enable JavaScript to view this content</CENTER></div></noscript>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<form action="snort_preprocessors.php" method="post" enctype="multipart/form-data" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
<?php
if($id != "") 
{

    $tab_array = array();
    $tab_array[] = array("Snort Interfaces", false, "/snort/snort_interfaces.php");
    $tab_array[] = array("If Settings", false, "/snort/snort_interfaces_edit.php?id={$id}");
    $tab_array[] = array("Categories", false, "/snort/snort_rulesets.php?id={$id}");
    $tab_array[] = array("Rules", false, "/snort/snort_rules.php?id={$id}");
    $tab_array[] = array("Servers", false, "/snort/snort_define_servers.php?id={$id}");
    $tab_array[] = array("Preprocessors", true, "/snort/snort_preprocessors.php?id={$id}");
    $tab_array[] = array("Barnyard2", false, "/snort/snort_barnyard.php?id={$id}");
    display_top_tabs($tab_array);

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
				<div class=\"alert\" ALIGN=CENTER><img src=\"../themes/nervecenter/images/icons/icon_alert.gif\"/><strong>You can not edit options without an interface ID.</CENTER></div>\n";
				
				}
				?>
				<tr>
					<td width="22%" valign="top">&nbsp;</td>
					<td width="78%"><span class="vexpl"><span class="red"><strong>Note: </strong></span><br>
					RULES MAY DEPENDENT ON Preprocessors!<br>
					Please save your settings befor you click start.<br>					
					</td>
				</tr>
				<tr>
					<td width="22%" valign="top" class="vncell">Enable <br>Performance Statistics</td>
					<td width="78%" class="vtable">
					<input name="perform_stat" type="checkbox" value="on" <?php if ($pconfig['perform_stat']=="on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Performance Statistics for this interface.</td>
				</tr>
				<tr>
					<td width="22%" valign="top" class="vncell">Enable <br>HTTP Inspect</td>
					<td width="78%" class="vtable">
					<input name="http_inspect" type="checkbox" value="on" <?php if ($pconfig['http_inspect']=="on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Normalize/Decode and detect HTTP traffic and protocol anomalies.</td>
				</tr>
				<tr>
				<td valign="top" class="vncell">HTTP server flow depth</td>
				<td class="vtable">
					<table cellpadding="0" cellspacing="0">
					<tr>
                    <td><input name="flow_depth" type="text" class="formfld" id="flow_depth" size="5" value="<?=htmlspecialchars($pconfig['flow_depth']);?>"> <strong>-1</strong> to <strong>1460</strong> (<strong>-1</strong> disables HTTP inspect, <strong>0</strong> enables all HTTP inspect)</td>
					</tr>
					</table>
						Amount of HTTP server response payload to inspect. Snort's performance may increase by ajusting this value.<br>
						Setting this value too low may cause false negatives. Value above 0 is in bytes.<br>
						<strong>Default value is 0</strong></td>
				</tr>
				<tr>
					<td width="22%" valign="top" class="vncell">Enable <br>RPC Decode and Back Orifice detector</td>
					<td width="78%" class="vtable">
					<input name="other_preprocs" type="checkbox" value="on" <?php if ($pconfig['other_preprocs']=="on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Normalize/Decode RPC traffic and detects Back Orifice traffic on the network.</td>
				</tr>
				<tr>
					<td width="22%" valign="top" class="vncell">Enable <br>FTP & Telnet Normalizer</td>
					<td width="78%" class="vtable">
					<input name="ftp_preprocessor" type="checkbox" value="on" <?php if ($pconfig['ftp_preprocessor']=="on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Normalize/Decode FTP & Telnet traffic and protocol anomalies.</td>
				</tr>
				<tr>
					<td width="22%" valign="top" class="vncell">Enable <br>SMTP Normalizer</td>
					<td width="78%" class="vtable">
					<input name="smtp_preprocessor" type="checkbox" value="on" <?php if ($pconfig['smtp_preprocessor']=="on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Normalize/Decode SMTP protocol for enforcement and buffer overflows.</td>
				</tr>
				<tr>
					<td width="22%" valign="top" class="vncell">Enable <br>Portscan Detection</td>
					<td width="78%" class="vtable">
					<input name="sf_portscan" type="checkbox" value="on" <?php if ($pconfig['sf_portscan']=="on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Detects various types of portscans and portsweeps.</td>
				</tr>
				<tr>
					<td width="22%" valign="top" class="vncell">Enable <br>DCE/RPC2 Detection</td>
					<td width="78%" class="vtable">
					<input name="dce_rpc_2" type="checkbox" value="on" <?php if ($pconfig['dce_rpc_2']=="on") echo "checked"; ?> onClick="enable_change(false)"><br>
					The dcerpc preprocessor detects and decodes SMB and DCE/RPC traffic.</td>
				</tr>
				<tr>
					<td width="22%" valign="top" class="vncell">Enable <br>DNS Detection</td>
					<td width="78%" class="vtable">
					<input name="dns_preprocessor" type="checkbox" value="on" <?php if ($pconfig['dns_preprocessor']=="on") echo "checked"; ?> onClick="enable_change(false)"><br>
					The dns preprocessor (currently) decodes DNS Response traffic and detects a few vulnerabilities.</td>
				</tr> 
                <tr>
                  <td width="22%" valign="top" class="vncell">Define SSL_IGNORE</td>
                  <td width="78%" class="vtable">
                    <input name="def_ssl_ports_ignore" type="text" class="formfld" id="def_ssl_ports_ignore" size="40" value="<?=htmlspecialchars($pconfig['def_ssl_ports_ignore']);?>">
                    <br> <span class="vexpl"> Encrypted traffic should be ignored by Snort for both performance reasons and to reduce false positives.<br>
					Default: "443 465 563 636 989 990 992 993 994 995".</span> <strong>Please use spaces and not commas.</strong></td>
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
		Please save your settings befor you click start. </td>
	</tr>
  </table>
  </table>
</form>

<script language="JavaScript">
<!--
enable_change(false);
//-->
</script>
<?php include("fend.inc"); ?>
</body>
</html>
