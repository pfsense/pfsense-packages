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

global $g;

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

	/* new options */
	$pconfig['perform_stat'] = $a_nat[$id]['perform_stat'];
	$pconfig['server_flow_depth'] = $a_nat[$id]['server_flow_depth'];
	$pconfig['client_flow_depth'] = $a_nat[$id]['client_flow_depth'];
	$pconfig['max_queued_bytes'] = $a_nat[$id]['max_queued_bytes'];
	$pconfig['max_queued_segs'] = $a_nat[$id]['max_queued_segs'];
	$pconfig['stream5_mem_cap'] = $a_nat[$id]['stream5_mem_cap'];
	$pconfig['http_inspect'] = $a_nat[$id]['http_inspect'];
	$pconfig['noalert_http_inspect'] = $a_nat[$id]['noalert_http_inspect'];
	$pconfig['other_preprocs'] = $a_nat[$id]['other_preprocs'];
	$pconfig['ftp_preprocessor'] = $a_nat[$id]['ftp_preprocessor'];
	$pconfig['smtp_preprocessor'] = $a_nat[$id]['smtp_preprocessor'];
	$pconfig['sf_portscan'] = $a_nat[$id]['sf_portscan'];
	$pconfig['dce_rpc_2'] = $a_nat[$id]['dce_rpc_2'];
	$pconfig['dns_preprocessor'] = $a_nat[$id]['dns_preprocessor'];
	$pconfig['sensitive_data'] = $a_nat[$id]['sensitive_data'];
	$pconfig['ssl_preproc'] = $a_nat[$id]['ssl_preproc'];
	$pconfig['pop_preproc'] = $a_nat[$id]['pop_preproc'];
	$pconfig['imap_preproc'] = $a_nat[$id]['imap_preproc'];
	$pconfig['dnp3_preproc'] = $a_nat[$id]['dnp3_preproc'];
	$pconfig['modbus_preproc'] = $a_nat[$id]['modbus_preproc'];
}

if ($_POST) {
	$natent = array();
	$natent = $pconfig;

	/* if no errors write to conf */
	if (!$input_errors) {
		/* post new options */
		if ($_POST['server_flow_depth'] != "") { $natent['server_flow_depth'] = $_POST['server_flow_depth']; }else{ $natent['server_flow_depth'] = ""; }
		if ($_POST['client_flow_depth'] != "") { $natent['client_flow_depth'] = $_POST['client_flow_depth']; }else{ $natent['client_flow_depth'] = ""; }
		if ($_POST['max_queued_bytes'] != "") { $natent['max_queued_bytes'] = $_POST['max_queued_bytes']; }else{ $natent['max_queued_bytes'] = ""; }
		if ($_POST['max_queued_segs'] != "") { $natent['max_queued_segs'] = $_POST['max_queued_segs']; }else{ $natent['max_queued_segs'] = ""; }
		if ($_POST['stream5_mem_cap'] != "") { $natent['stream5_mem_cap'] = $_POST['stream5_mem_cap']; }else{ $natent['stream5_mem_cap'] = ""; }

		$natent['perform_stat'] = $_POST['perform_stat'] ? 'on' : 'off';
		$natent['http_inspect'] = $_POST['http_inspect'] ? 'on' : 'off';
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

		if (isset($id) && $a_nat[$id])
			$a_nat[$id] = $natent;
		else {
			if (is_numeric($after))
				array_splice($a_nat, $after+1, 0, array($natent));
			else
				$a_nat[] = $natent;
		}

		write_config();

		$if_real = snort_get_real_interface($pconfig['interface']);
		sync_snort_package_config();

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

$if_friendly = snort_get_friendly_interface($pconfig['interface']);
$pgtitle = "Snort: Interface {$if_real} Preprocessors and Flow";
include_once("head.inc");
?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

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

<form action="snort_preprocessors.php" method="post"
	enctype="multipart/form-data" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[] = array(gettext("If Settings"), false, "/snort/snort_interfaces_edit.php?id={$id}");
        $tab_array[] = array(gettext("Categories"), false, "/snort/snort_rulesets.php?id={$id}");
        $tab_array[] = array(gettext("Rules"), false, "/snort/snort_rules.php?id={$id}");
        $tab_array[] = array(gettext("Variables"), false, "/snort/snort_define_servers.php?id={$id}");
        $tab_array[] = array(gettext("Preprocessors"), true, "/snort/snort_preprocessors.php?id={$id}");
        $tab_array[] = array(gettext("Barnyard2"), false, "/snort/snort_barnyard.php?id={$id}");
        display_top_tabs($tab_array);
?>
</td></tr>
<tr><td class="tabcont">
<table width="100%" border="0" cellpadding="6" cellspacing="0">
	<tr>
		<td width="22%" valign="top">&nbsp;</td>
		<td width="78%"><span class="vexpl"><span class="red"><strong<?php echo gettext("Note:"); ?>>
		</strong></span><br>
		<?php echo gettext("Rules may be dependent on preprocessors!"); ?><br>
		<?php echo gettext("Defaults will be used when there is no user input."); ?><br></td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Performance Statistics"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?></td>
		<td width="78%" class="vtable"><input name="perform_stat"
			type="checkbox" value="on"
			<?php if ($pconfig['perform_stat']=="on") echo "checked"; ?>
			onClick="enable_change(false)"> <?php echo gettext("Collect Performance Statistics for this interface."); ?></td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("HTTP Inspect Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?></td>
		<td width="78%" class="vtable"><input name="http_inspect"
			type="checkbox" value="on"
			<?php if ($pconfig['http_inspect']=="on") echo "checked"; ?>
			onClick="enable_change(false)"> <?php echo gettext("Use HTTP Inspect to " .
				"Normalize/Decode and detect HTTP traffic and protocol anomalies."); ?></td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("HTTP server flow depth"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="server_flow_depth" type="text" class="formfld"
					id="flow_depth" size="6"
					value="<?=htmlspecialchars($pconfig['server_flow_depth']);?>"> <?php echo gettext("<strong>-1</strong> " .
				"to <strong>65535</strong> (<strong>-1</strong> disables HTTP " .
				"inspect, <strong>0</strong> enables all HTTP inspect)"); ?></td>
			</tr>
		</table>
		<?php echo gettext("Amount of HTTP server response payload to inspect. Snort's " .
		"performance may increase by adjusting this value."); ?><br>
		<?php echo gettext("Setting this value too low may cause false negatives. Values above 0 " .
		"are specified in bytes.  Recommended setting is maximum (65535). Default value is <strong>300</strong>"); ?><br>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("HTTP client flow depth"); ?></td>
		<td class="vtable">
		<table cellpadding="0" cellspacing="0">
			<tr>
				<td><input name="client_flow_depth" type="text" class="formfld"
					id="flow_depth" size="6"
					value="<?=htmlspecialchars($pconfig['client_flow_depth']);?>"> <?php echo gettext("<strong>-1</strong> " .
				"to <strong>1460</strong> (<strong>-1</strong> disables HTTP " .
				"inspect, <strong>0</strong> enables all HTTP inspect)"); ?></td>
			</tr>
		</table>
		<?php echo gettext("Amount of raw HTTP client request payload to inspect. Snort's " .
		"performance may increase by adjusting this value."); ?><br>
		<?php echo gettext("Setting this value too low may cause false negatives. Values above 0 " .
		"are specified in bytes.  Recommended setting is maximum (1460). Default value is <strong>300</strong>"); ?><br>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Disable HTTP Alerts"); ?></td>
		<td width="78%" class="vtable"><input name="noalert_http_inspect"
			type="checkbox" value="on"
			<?php if ($pconfig['noalert_http_inspect']=="on") echo "checked"; ?>
			onClick="enable_change(false)"> <?php echo gettext("Tick to turn off alerts from the HTTP Inspect " .
				"preprocessor.  This has no effect on HTTP rules in the rule set."); ?></td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Stream5 Settings"); ?></td>
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
				"means Maximum )"); ?></td>
			</tr>
		</table>
		<?php echo gettext("The number of bytes to be queued for reassembly for TCP sessions in " .
		"memory. Default value is <strong>1048576</strong>"); ?><br>
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
				"Maximum )"); ?></td>
			</tr>
		</table>
		<?php echo gettext("The number of segments to be queued for reassembly for TCP sessions " .
		"in memory. Default value is <strong>2621</strong>"); ?><br>
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
				"( default value is <strong>8388608</strong>) "); ?></td>
			</tr>
		</table>
		<?php echo gettext("The memory cap in bytes for TCP packet storage " .
		"in RAM. Default value is <strong>8388608</strong> (8 MB)"); ?><br>
		</td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("General Preprocessor Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br>
		<?php echo gettext("RPC Decode and Back Orifice detector"); ?></td>
		<td width="78%" class="vtable"><input name="other_preprocs"
			type="checkbox" value="on"
			<?php if ($pconfig['other_preprocs']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("Normalize/Decode RPC traffic and detects Back Orifice traffic on the network."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br>
		<?php echo gettext("FTP and Telnet Normalizer"); ?></td>
		<td width="78%" class="vtable"><input name="ftp_preprocessor"
			type="checkbox" value="on"
			<?php if ($pconfig['ftp_preprocessor']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("Normalize/Decode FTP and Telnet traffic and protocol anomalies."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br>
		<?php echo gettext("POP Normalizer"); ?></td>
		<td width="78%" class="vtable"><input name="pop_preproc"
			type="checkbox" value="on"
			<?php if ($pconfig['pop_preproc']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("Normalize/Decode POP protocol for enforcement and buffer overflows."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br>
		<?php echo gettext("IMAP Normalizer"); ?></td>
		<td width="78%" class="vtable"><input name="imap_preproc"
			type="checkbox" value="on"
			<?php if ($pconfig['imap_preproc']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("Normalize/Decode IMAP protocol for enforcement and buffer overflows."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br>
		<?php echo gettext("SMTP Normalizer"); ?></td>
		<td width="78%" class="vtable"><input name="smtp_preprocessor"
			type="checkbox" value="on"
			<?php if ($pconfig['smtp_preprocessor']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("Normalize/Decode SMTP protocol for enforcement and buffer overflows."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br>
		<?php echo gettext("Portscan Detection"); ?></td>
		<td width="78%" class="vtable"><input name="sf_portscan"
			type="checkbox" value="on"
			<?php if ($pconfig['sf_portscan']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("Detects various types of portscans and portsweeps."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br>
		<?php echo gettext("DCE/RPC2 Detection"); ?></td>
		<td width="78%" class="vtable"><input name="dce_rpc_2"
			type="checkbox" value="on"
			<?php if ($pconfig['dce_rpc_2']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("The DCE/RPC preprocessor detects and decodes SMB and DCE/RPC traffic."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br>
		<?php echo gettext("DNS Detection"); ?></td>
		<td width="78%" class="vtable"><input name="dns_preprocessor"
			type="checkbox" value="on"
			<?php if ($pconfig['dns_preprocessor']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("The DNS preprocessor decodes DNS Response traffic and detects some vulnerabilities."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br> <?php echo gettext("SSL Data"); ?></td>
		<td width="78%" class="vtable">
			<input name="ssl_preproc" type="checkbox" value="on"
			<?php if ($pconfig['ssl_preproc']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("SSL data searches for irregularities during SSL protocol exchange"); ?>	
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br> <?php echo gettext("Sensitive Data"); ?></td>
		<td width="78%" class="vtable">
			<input name="sensitive_data" type="checkbox" value="on"
			<?php if ($pconfig['sensitive_data']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("Sensitive data searches for credit card or Social Security numbers in data"); ?>	
		</td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("SCADA Preprocessor Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br> <?php echo gettext("Modbus Detection"); ?></td>
		<td width="78%" class="vtable">
			<input name="modbus_preproc" type="checkbox" value="on"
			<?php if ($pconfig['modbus_preproc']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("Modbus is a protocol used in SCADA networks.  The default port is TCP 502. If your network does " .
			"not contain Modbus-enabled devices, you should leave this preprocessor disabled."); ?>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Enable"); ?> <br> <?php echo gettext("DNP3 Detection"); ?></td>
		<td width="78%" class="vtable">
			<input name="dnp3_preproc" type="checkbox" value="on"
			<?php if ($pconfig['dnp3_preproc']=="on") echo "checked"; ?>
			onClick="enable_change(false)"><br>
		<?php echo gettext("DNP3 is a protocol used in SCADA networks.  The default port is TCP 20000. If your network does " .
			"not contain DNP3-enabled devices, you should leave this preprocessor disabled."); ?>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top">&nbsp;</td>
				<td width="78%">
					<input name="Submit" type="submit" class="formbtn" value="Save">
					<input name="id" type="hidden" value="<?=$id;?>"></td>
			</tr>
			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%"><span class="vexpl"><span class="red"><strong><?php echo gettext("Note:"); ?></strong></span>
				<br>
				<?php echo gettext("Please save your settings before you click Start."); ?> </td>
			</tr>
</table>
</td></tr></table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
