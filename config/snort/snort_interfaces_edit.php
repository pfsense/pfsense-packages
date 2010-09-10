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


/* always have a limit of (65535) numbers only or snort will not start do to id limits */
/* TODO: When inline gets added make the uuid the port number lisstening */
//function gen_snort_uuid($fileline)
//{
        /* return the first 5 */
        //if (preg_match("/...../", $fileline, $matches1))
        //{
        //$uuid_final =  "$matches1[0]";
        //}
//return $uuid_final;
//}

/* gen uuid for each iface !inportant */
if ($config['installedpackages']['snortglobal']['rule'][$id]['uuid'] == '') {
	//$snort_uuid = gen_snort_uuid(strrev(uniqid(true)));
$snort_uuid = 0;
while ($snort_uuid > 65535 || $snort_uuid == 0) {
	$snort_uuid = mt_rand(1, 65535);
	$pconfig['uuid'] = $snort_uuid;
	}
}

/* convert fake interfaces to real */
$if_real = convert_friendly_interface_to_real_interface_name2($a_nat[$id]['interface']);

if ($config['installedpackages']['snortglobal']['rule'][$id]['uuid'] != '') {
	$snort_uuid = $config['installedpackages']['snortglobal']['rule'][$id]['uuid'];
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
	$pconfig['descr'] = $a_nat[$id]['descr'];
	$pconfig['performance'] = $a_nat[$id]['performance'];
	$pconfig['blockoffenders7'] = $a_nat[$id]['blockoffenders7'];
	$pconfig['whitelistname'] = $a_nat[$id]['whitelistname'];
	$pconfig['homelistname'] = $a_nat[$id]['homelistname'];
	$pconfig['externallistname'] = $a_nat[$id]['externallistname'];
	$pconfig['suppresslistname'] = $a_nat[$id]['suppresslistname'];
	$pconfig['snortalertlogtype'] = $a_nat[$id]['snortalertlogtype'];
	$pconfig['alertsystemlog'] = $a_nat[$id]['alertsystemlog'];
	$pconfig['tcpdumplog'] = $a_nat[$id]['tcpdumplog'];
	$pconfig['snortunifiedlog'] = $a_nat[$id]['snortunifiedlog'];
	$pconfig['configpassthru'] = base64_decode($a_nat[$id]['configpassthru']);
	$pconfig['barnconfigpassthru'] = $a_nat[$id]['barnconfigpassthru'];
	$pconfig['rulesets'] = $a_nat[$id]['rulesets'];
	$pconfig['rule_sid_off'] = $a_nat[$id]['rule_sid_off'];
	$pconfig['rule_sid_on'] = $a_nat[$id]['rule_sid_on'];
	
		
	if (!$pconfig['interface'])
		$pconfig['interface'] = "wan";
} else {
	$pconfig['interface'] = "wan";
}

if (isset($_GET['dup']))
	unset($id);

/* alert file */
$d_snortconfdirty_path = "/var/run/snort_conf_{$snort_uuid}_{$if_real}.dirty";
	
	/* this will exec when alert says apply */
	if ($_POST['apply']) {
		
		if (file_exists("/var/run/snort_conf_{$snort_uuid}_.dirty")) {
				
				write_config();
			
				$if_real = convert_friendly_interface_to_real_interface_name2($a_nat[$id]['interface']);

				sync_snort_package_all($id, $if_real, $snort_uuid);
				sync_snort_package();
			
				unlink("/var/run/snort_conf_{$snort_uuid}_.dirty");
			
		}
		
		if (file_exists($d_snortconfdirty_path)) {
			
			write_config();
			
			sync_snort_package_all($id, $if_real, $snort_uuid);
			sync_snort_package();

			unlink($d_snortconfdirty_path);
			
		}
		
	}

if ($_POST["Submit"]) {

	
	
		// if ($config['installedpackages']['snortglobal']['rule']) {
			if ($_POST['descr'] == '' && $pconfig['descr'] == '') {
				$input_errors[] = "Please  enter a description for your reference.";
				}
			
			if ($id == "" && $config['installedpackages']['snortglobal']['rule'][0]['interface'] != "") {

			$rule_array = $config['installedpackages']['snortglobal']['rule'];
			$id_c = -1;
			foreach ($rule_array as $value) {

			$id_c += 1;

			$result_lan = $config['installedpackages']['snortglobal']['rule'][$id_c]['interface'];
			$if_real = convert_friendly_interface_to_real_interface_name2($result_lan);

				if ($_POST['interface'] == $result_lan) {	
				$input_errors[] = "Interface $result_lan is in use. Please select another interface.";
					}			
				}
			}

	/* check for overlaps */
	foreach ($a_nat as $natent) {
		if (isset($id) && ($a_nat[$id]) && ($a_nat[$id] === $natent))
			continue;
		if ($natent['interface'] != $_POST['interface'])
			continue;
	}

	/* if no errors write to conf */
	if (!$input_errors) {
		$natent = array();

		/* write to conf for 1st time or rewrite the answer */
		$natent['interface'] = $_POST['interface'] ? $_POST['interface'] : $pconfig['interface'];
		/* if post write to conf or rewite the answer */
		$natent['enable'] = $_POST['enable'] ? on : off;
		$natent['uuid'] = $pconfig['uuid'];
		$natent['descr'] = $_POST['descr'] ? $_POST['descr'] : $pconfig['descr'];
		$natent['performance'] = $_POST['performance'] ? $_POST['performance'] : $pconfig['performance'];
		/* if post = on use on off or rewrite the conf */
		if ($_POST['blockoffenders7'] == "on") { $natent['blockoffenders7'] = on; }else{ $natent['blockoffenders7'] = off; } if ($_POST['enable'] == "") { $natent['blockoffenders7'] = $pconfig['blockoffenders7']; }
		$natent['whitelistname'] = $_POST['whitelistname'] ? $_POST['whitelistname'] : $pconfig['whitelistname'];
		$natent['homelistname'] = $_POST['homelistname'] ? $_POST['homelistname'] : $pconfig['homelistname'];
		$natent['externallistname'] = $_POST['externallistname'] ? $_POST['externallistname'] : $pconfig['externallistname'];
		$natent['suppresslistname'] = $_POST['suppresslistname'] ? $_POST['suppresslistname'] : $pconfig['suppresslistname'];
		$natent['snortalertlogtype'] = $_POST['snortalertlogtype'] ? $_POST['snortalertlogtype'] : $pconfig['snortalertlogtype'];
		if ($_POST['alertsystemlog'] == "on") { $natent['alertsystemlog'] = on; }else{ $natent['alertsystemlog'] = off; } if ($_POST['enable'] == "") { $natent['alertsystemlog'] = $pconfig['alertsystemlog']; }
		if ($_POST['tcpdumplog'] == "on") { $natent['tcpdumplog'] = on; }else{ $natent['tcpdumplog'] = off; } if ($_POST['enable'] == "") { $natent['tcpdumplog'] = $pconfig['tcpdumplog']; }
		if ($_POST['snortunifiedlog'] == "on") { $natent['snortunifiedlog'] = on; }else{ $natent['snortunifiedlog'] = off; } if ($_POST['enable'] == "") { $natent['snortunifiedlog'] = $pconfig['snortunifiedlog']; }
		$natent['configpassthru'] = base64_encode($_POST['configpassthru']) ? base64_encode($_POST['configpassthru']) : $pconfig['configpassthru'];
		/* if optiion = 0 then the old descr way will not work */

	/* rewrite the options that are not in post */
	/* make shure values are set befor repost or conf.xml will be broken */
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
	if ($pconfig['def_sip_proxy_ports'] != "") { $natent['def_sip_proxy_ports'] = $pconfig['def_sip_proxy_ports']; }
	if ($pconfig['def_auth_ports'] != "") { $natent['def_auth_ports'] = $pconfig['def_auth_ports']; }
	if ($pconfig['def_finger_ports'] != "") { $natent['def_finger_ports'] = $pconfig['def_finger_ports']; }
	if ($pconfig['def_irc_ports'] != "") { $natent['def_irc_ports'] = $pconfig['def_irc_ports']; }
	if ($pconfig['def_nntp_ports'] != "") { $natent['def_nntp_ports'] = $pconfig['def_nntp_ports']; }
	if ($pconfig['def_rlogin_ports'] != "") { $natent['def_rlogin_ports'] = $pconfig['def_rlogin_ports']; }
	if ($pconfig['def_rsh_ports'] != "") { $natent['def_rsh_ports'] = $pconfig['def_rsh_ports']; }
	if ($pconfig['def_ssl_ports'] != "") { $natent['def_ssl_ports'] = $pconfig['def_ssl_ports']; }
	if ($pconfig['barnyard_enable'] != "") { $natent['barnyard_enable'] = $pconfig['barnyard_enable']; }
	if ($pconfig['barnyard_mysql'] != "") { $natent['barnyard_mysql'] = $pconfig['barnyard_mysql'];	}
	if ($pconfig['barnconfigpassthru'] != "") { $natent['barnconfigpassthru'] = $pconfig['barnconfigpassthru'];	}
	if ($pconfig['rulesets'] != "") { $natent['rulesets'] = $pconfig['rulesets']; }
	if ($pconfig['rule_sid_off'] != "") { $natent['rule_sid_off'] = $pconfig['rule_sid_off']; }
	if ($pconfig['rule_sid_on'] != "") { $natent['rule_sid_on'] = $pconfig['rule_sid_on'];	}


		if (isset($id) && $a_nat[$id])
			$a_nat[$id] = $natent;
		else {
			if (is_numeric($after))
				array_splice($a_nat, $after+1, 0, array($natent));
			else
				$a_nat[] = $natent;
		}

		write_config();
		
		touch("$d_snortconfdirty_path");
		
		/* if snort.sh crashed this will remove the pid */
		exec('/bin/rm /tmp/snort.sh.pid');
		
		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		sleep(2);
		header("Location: /snort/snort_interfaces_edit.php?id=$id");

		exit;
	}
}
	
		if ($_POST["Submit2"]) {
		
		sync_snort_package_all($id, $if_real, $snort_uuid);
		sync_snort_package();
		sleep(1);
		
		Running_Start($snort_uuid, $if_real, $id);
				
		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		sleep(2);
		header("Location: /snort/snort_interfaces_edit.php?id=$id");
		}

		if ($_POST["Submit3"])
		{
		
			Running_Stop($snort_uuid, $if_real, $id);
			
			header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
			header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
			header( 'Cache-Control: no-store, no-cache, must-revalidate' );
			header( 'Cache-Control: post-check=0, pre-check=0', false );
			header( 'Pragma: no-cache' );
			sleep(2);
			header("Location: /snort/snort_interfaces_edit.php?id=$id");

		}

	/* This code needs to be below headers */
	if (isset($config['installedpackages']['snortglobal']['rule'][$id]['interface']))
	{
	
		$snort_up_ck2_info = Running_Ck($snort_uuid, $if_real, $id);
		
		if	($snort_up_ck2_info == 'no') {
			$snort_up_ck = '<input name="Submit2" type="submit" class="formbtn" value="Start" onClick="enable_change(true)">';
		}else{
			$snort_up_ck = '<input name="Submit3" type="submit" class="formbtn" value="Stop" onClick="enable_change(true)">';
			}
			
	}else{
		$snort_up_ck = '';				
	}
		

$pgtitle = "Snort: Interface Edit: $id $snort_uuid $if_real";
include("/usr/local/pkg/snort/snort_head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php 
include("fbegin.inc");
?>

<?php
echo "{$snort_general_css}\n";
?>

<!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

<div class="body2">

<noscript><div class="alert" ALIGN=CENTER><img src="/themes/nervecenter/images/icons/icon_alert.gif"/><strong>Please enable JavaScript to view this content</strong></div></noscript>
<script language="JavaScript">
<!--

function enable_change(enable_change) {
	endis = !(document.iform.enable.checked || enable_change);
	// make shure a default answer is called if this is envoked.
	endis2 = (document.iform.enable);

<?php
/* make shure all the settings exist or function hide will not work */
/* if $id is emty allow if and discr to be open */
if($config['installedpackages']['snortglobal']['rule'][$id]['interface'] != '') 
{
echo "	
	document.iform.interface.disabled = endis2;
	document.iform.descr.disabled = endis;\n";
}
?>
	document.iform.performance.disabled = endis;
	document.iform.blockoffenders7.disabled = endis;
	document.iform.alertsystemlog.disabled = endis;
	document.iform.whitelistname.disabled = endis;
	document.iform.externallistname.disabled = endis;
	document.iform.homelistname.disabled = endis;
	document.iform.suppresslistname.disabled = endis;
	document.iform.tcpdumplog.disabled = endis;
	document.iform.snortunifiedlog.disabled = endis;
	document.iform.configpassthru.disabled = endis;
}
//-->
</script>
<?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<form action="snort_interfaces_edit.php<?php echo "?id=$id";?>" method="post" enctype="multipart/form-data" name="iform" id="iform">

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
if ($a_nat[$id]['interface'] != '') {
	 /* get the interface name */
		$first = 0;
        $snortInterfaces = array(); /* -gtm  */

        $if_list = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
        $if_array = split(',', $if_list);
        //print_r($if_array);
        if($if_array) {
                foreach($if_array as $iface2) {
                        $if2 = convert_friendly_interface_to_real_interface_name2($iface2);

                        if($config['interfaces'][$iface2]['ipaddr'] == "pppoe") {
                                $if2 = "ng0";
                        }

                        /* build a list of user specified interfaces -gtm */
                        if($if2){
                          array_push($snortInterfaces, $if2);
                          $first = 1;
                        }
                }

                if (count($snortInterfaces) < 1) {
                        log_error("Snort will not start.  You must select an interface for it to listen on.");
                        return;
                }
        }

}


echo '<div class="snorttabs" style="margin:1px 0px; width:775px;">' . "\n";
echo '<!-- Tabbed bar code -->' . "\n";
echo '<ul class="snorttabs">' . "\n";
	if (!file_exists("/var/run/snort_conf_{$snort_uuid}_.dirty")) {
		echo '<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>' . "\n";
	}
	echo "<li  class=\"snorttabs_active\"><a href=\"/snort/snort_interfaces_edit.php?id={$id}\"><span>If Settings</span></a></li>\n";
	/* hide user tabs when no settings have be saved */
	if ($config['installedpackages']['snortglobal']['rule'][$id]['interface'] != '') {
		if (!file_exists("/var/run/snort_conf_{$snort_uuid}_.dirty")) {
    echo "<li><a href=\"/snort/snort_rulesets.php?id={$id}\"><span>Categories</span></a></li>\n";
    echo "<li><a href=\"/snort/snort_rules.php?id={$id}\"><span>Rules</span></a></li>\n";
	echo "<li><a href=\"/snort/snort_define_servers.php?id={$id}\"><span>Servers</span></a></li>\n";
    echo "<li><a href=\"/snort/snort_preprocessors.php?id={$id}\"><span>Preprocessors</span></a></li>\n";
    echo "<li><a href=\"/snort/snort_barnyard.php?id={$id}\"><span>Barnyard2</span></a></li>\n";
		}
	}
echo '</ul>' . "\n";
echo '</div>' . "\n";

?>

</td>
</tr>
				<tr>
				<td class="tabcont">
				<table width="100%" border="0" cellpadding="6" cellspacing="0">
                <tr>
                  <td colspan="2" valign="top" class="listtopic">General Settings</td>
                </tr>
				<tr>
				<td width="22%" valign="top" class="vncellreq2">Interface</td>
				<td width="22%" valign="top" class="vtable">&nbsp;
					<?php
					// <input name="enable" type="checkbox" value="yes" checked onClick="enable_change(false)">
					// care with spaces
					if ($pconfig['enable'] == "on")
					$checked = checked;

					$onclick_enable = "onClick=\"enable_change(false)\">";
					
					echo "
					<input name=\"enable\" type=\"checkbox\" value=\"on\" $checked $onclick_enable
					&nbsp;&nbsp;Enable or Disable</td>\n\n";
					?>
				</td>
				</tr>
				<tr>
                  <td width="22%" valign="top" class="vncellreq2">Interface</td>
                  <td width="78%" class="vtable">
					<select name="interface" class="formfld">
						<?php
						$interfaces = array('wan' => 'WAN', 'lan' => 'LAN');
						for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++) {
							$interfaces['opt' . $i] = $config['interfaces']['opt' . $i]['descr'];
						}
						foreach ($interfaces as $iface => $ifacename): ?>
						<option value="<?=$iface;?>" <?php if ($iface == $pconfig['interface']) echo "selected"; ?>>
						<?=htmlspecialchars($ifacename);?>
						</option>
						<?php endforeach; ?>
					</select><br>
                     <span class="vexpl">Choose which interface this rule applies to.<br>
                     Hint: in most cases, you'll want to use WAN here.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncellreq2">Description</td>
                  <td width="78%" class="vtable">
                    <input name="descr" type="text" class="formfld" id="descr" size="40" value="<?=htmlspecialchars($pconfig['descr']);?>">
                    <br> <span class="vexpl">You may enter a description here for your reference (not parsed).</span></td>
                </tr>
				<tr>
					<td width="22%" valign="top" class="vncell2">Memory Performance</td>
					<td width="78%" class="vtable">
					<select name="performance" class="formfld" id="performance">
						<?php
							$interfaces2 = array('ac-bnfa' => 'AC-BNFA', 'lowmem' => 'LOWMEM', 'ac-std' => 'AC-STD', 'ac' => 'AC', 'ac-banded' => 'AC-BANDED', 'ac-sparsebands' => 'AC-SPARSEBANDS', 'acs' => 'ACS');
							foreach ($interfaces2 as $iface2 => $ifacename2): ?>
							<option value="<?=$iface2;?>" <?php if ($iface2 == $pconfig['performance']) echo "selected"; ?>>
							<?=htmlspecialchars($ifacename2);?>
							</option>
						<?php endforeach; ?>
					</select><br>
					<span class="vexpl">Lowmem and ac-bnfa are recommended for low end systems, Ac: high memory, best performance, ac-std: moderate memory,high performance, acs: small memory, moderateperformance, ac-banded: small memory,moderate performance, ac-sparsebands: small memory, high performance.<br>
					</span></td>
				</tr>
                <tr>
                  <td colspan="2" valign="top" class="listtopic">Choose the networks snort should inspect and whitelist.</td>
                </tr>
				<tr>
				<td width="22%" valign="top" class="vncell2">Home net</td>
				<td width="78%" class="vtable">
				<select name="homelistname" class="formfld" id="homelistname">		
						<?php
						/* find whitelist names and filter by type */
						$hlist_select = $config['installedpackages']['snortglobal']['whitelist']['item'];										
						$hid = -1;
						if ($pconfig['homelistname'] == 'default'){ $selected  = 'selected'; }
							$wlist_sub2 = preg_match('/^([a-zA-z0-9]+)/', $pconfig['homelistname'], $hlist_sub);
							echo "<option value=\"default\" $selected>default</option>
							";
							foreach ($hlist_select as $value):
							$hid += 1;
							if ($config['installedpackages']['snortglobal']['whitelist']['item'][$hid]['snortlisttype'] == 'netlist') {
								$ilistname = $config['installedpackages']['snortglobal']['whitelist']['item'][$hid]['name'];
								$whitelist_uuid = $config['installedpackages']['snortglobal']['whitelist']['item'][$hid]['uuid'];
									if ($ilistname == $hlist_sub[0]){
										echo "<option value=\"$ilistname $whitelist_uuid\" selected>";
									}else{
										echo "<option value=\"$ilistname $whitelist_uuid\">";
									}
								echo htmlspecialchars($ilistname) . '</option>
								';
							}
							endforeach;			
						?>							
				</select><br>
				<span class="vexpl">Choose the home net you will like this rule to use.
				</span>&nbsp;<span class="red">Note:</span>&nbsp;Default home net adds only local networks.<br>
				<span class="red">Hint:</span>&nbsp;Most users add a list of friendly ips that the firewall cant see.</td>
				</tr>
				<tr>
				<td width="22%" valign="top" class="vncell2">External net</td>
				<td width="78%" class="vtable">
				<select name="externallistname" class="formfld" id="externallistname">
						<?php
						/* find whitelist names and filter by type */
						$exlist_select = $config['installedpackages']['snortglobal']['whitelist']['item'];										
						$exid = -1;
						if ($pconfig['externallistname'] == 'default'){ $selected  = 'selected'; }
							preg_match('/^([a-zA-z0-9]+)/', $pconfig['externallistname'], $exlist_sub);
							echo "<option value=\"default\" $selected>default</option>
							";
							foreach ($exlist_select as $value):
							$exid += 1;
							if ($config['installedpackages']['snortglobal']['whitelist']['item'][$exid]['snortlisttype'] == 'netlist') {
								$ilistname = $config['installedpackages']['snortglobal']['whitelist']['item'][$exid]['name'];
								$whitelist_uuid = $config['installedpackages']['snortglobal']['whitelist']['item'][$exid]['uuid'];
									if ($ilistname == $exlist_sub[0]){
										echo "<option value=\"$ilistname $whitelist_uuid\" selected>";
									}else{
										echo "<option value=\"$ilistname $whitelist_uuid\">";
									}
								echo htmlspecialchars($ilistname) . '</option>
								';
							}
							endforeach;			
						?>							
				</select><br>
				<span class="vexpl">Choose the external net you will like this rule to use.
				</span>&nbsp;<span class="red">Note:</span>&nbsp;Default external net, networks that are not home net.<br>
				<span class="red">Hint:</span>&nbsp;Most users should leave this setting at default.</td>
				</tr>
				<tr>
				<td width="22%" valign="top" class="vncell2">Block offenders</td>
				<td width="78%" class="vtable">
					<input name="blockoffenders7" type="checkbox" value="on" <?php if ($pconfig['blockoffenders7'] == "on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Checking this option will automatically block hosts that generate a Snort alert.</td>
				</tr>
				<tr>
				<td width="22%" valign="top" class="vncell2">Whitelist</td>
				<td width="78%" class="vtable">
				<select name="whitelistname" class="formfld" id="whitelistname">		
						<?php
						/* find whitelist names and filter by type, make sure to track by uuid */
						$wlist_select = $config['installedpackages']['snortglobal']['whitelist']['item'];										
						$wid = -1;
						if ($pconfig['whitelistname'] == 'default'){ $selected  = 'selected'; }
							preg_match('/^([a-zA-z0-9]+)/', $pconfig['whitelistname'], $wlist_sub);
							echo "<option value=\"default\" $selected>default</option>
							";
							foreach ($wlist_select as $value):
							$wid += 1;
							if ($config['installedpackages']['snortglobal']['whitelist']['item'][$wid]['snortlisttype'] == 'whitelist') {
								$ilistname = $config['installedpackages']['snortglobal']['whitelist']['item'][$wid]['name'];
								$whitelist_uuid = $config['installedpackages']['snortglobal']['whitelist']['item'][$wid]['uuid'];
									if ($ilistname == $wlist_sub[0]){
										echo "<option value=\"$ilistname $whitelist_uuid\" selected>";
									}else{
										echo "<option value=\"$ilistname $whitelist_uuid\">";
									}
								echo htmlspecialchars($ilistname) . '</option>
								';
							}
							endforeach;			
						?>							
				</select><br>
				<span class="vexpl">Choose the whitelist you will like this rule to use.
				</span>&nbsp;<span class="red">Note:</span>&nbsp;Default whitelist adds only local networks.</td>
				</tr>
				
				<tr>
				<td width="22%" valign="top" class="vncell2">Suppression and filtering</td>
				<td width="78%" class="vtable">
				<select name="suppresslistname" class="formfld" id="suppresslistname">		
						<?php
						/* find whitelist names and filter by type, make sure to track by uuid */
						$slist_select = $config['installedpackages']['snortglobal']['suppress']['item'];										
						$sid = -1;
						if ($pconfig['suppresslistname'] == 'default'){ $selected  = 'selected'; }
							preg_match('/^([a-zA-z0-9]+)/', $pconfig['suppresslistname'], $slist_sub);
							echo "<option value=\"default\" $selected>default</option>
							";
							foreach ($slist_select as $value):
							$sid += 1;
								$ilistname = $config['installedpackages']['snortglobal']['suppress']['item'][$sid]['name'];
								$suppress_uuid = $config['installedpackages']['snortglobal']['suppress']['item'][$sid]['uuid'];
									if ($ilistname == $slist_sub[0]){
										echo "<option value=\"$ilistname $suppress_uuid\" selected>";
									}else{
										echo "<option value=\"$ilistname $suppress_uuid\">";
									}
								echo htmlspecialchars($ilistname) . '</option>
								';
							endforeach;			
						?>							
				</select><br>
				<span class="vexpl">Choose the suppression or filtering file you will like this rule to use.
				</span>&nbsp;<span class="red">Note:</span>&nbsp;Default option disables suppression and filtering.</td>
				</tr>
				
                <tr>
                  <td colspan="2" valign="top" class="listtopic">Choose the types of logs snort should create.</td>
                </tr>
				<tr>
				<td width="22%" valign="top" class="vncell2">Send alerts to main System logs</td>
				<td width="78%" class="vtable">
					<input name="alertsystemlog" type="checkbox" value="on" <?php if ($pconfig['alertsystemlog'] == "on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Snort will send Alerts to the Pfsense system logs.</td>
				</tr>
				<tr>
				<td width="22%" valign="top" class="vncell2">Log to a Tcpdump file</td>
				<td width="78%" class="vtable">
					<input name="tcpdumplog" type="checkbox" value="on" <?php if ($pconfig['tcpdumplog'] == "on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Snort will log packets to a tcpdump-formatted file. The file then can be analyzed by an application such as Wireshark which understands pcap file formats. <span class="red"><strong>WARNING:</strong></span> File may become large.</td>
				</tr>
				<tr>
				<td width="22%" valign="top" class="vncell2">Log Alerts to a snort unified2 file</td>
				<td width="78%" class="vtable">
					<input name="snortunifiedlog" type="checkbox" value="on" <?php if ($pconfig['snortunifiedlog'] == "on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Snort will log Alerts to a file in the UNIFIED2 format. This is a requirement for barnyard2.</td>
				</tr>
                <tr>
                  <td colspan="2" valign="top" class="listtopic">Arguments here will be automatically inserted into the snort configuration.</td>
                </tr>
                <tr> 
                  <td width="22%" valign="top" class="vncell2">Advanced configuration pass through</td>
                  <td width="78%" class="vtable"> 
                    <textarea wrap="off" name="configpassthru" cols="75" rows="12" id="configpassthru" class="formpre2"><?=htmlspecialchars($pconfig['configpassthru']);?></textarea>
                    </td>
                </tr>
                <tr>
                  <td width="22%" valign="top"></td>
                  <td width="78%">
                    <input name="Submit" type="submit" class="formbtn" value="Save"> <?php echo $snort_up_ck; ?> <input type="button" class="formbtn" value="Cancel" onclick="history.back()">
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

<script language="JavaScript">
<!--
enable_change(false);
//-->
</script>

<?php include("fend.inc"); ?>

<div>

</body>
</html>
