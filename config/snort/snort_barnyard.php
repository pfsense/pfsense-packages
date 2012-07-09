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
require_once("/usr/local/pkg/snort/snort.inc");

global $g;

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (is_null($id)) {
        header("Location: /snort/snort_interfaces.php");
        exit;
}

if (!is_array($config['installedpackages']['snortglobal']['rule']))
	$config['installedpackages']['snortglobal']['rule'] = array();
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

if (isset($_GET['dup'])) {
	$id = $_GET['dup'];
	$after = $_GET['dup'];
}

$pconfig = array();
if (isset($id) && $a_nat[$id]) {
	/* old options */
	$pconfig = $a_nat[$id];
	$pconfig['barnyard_enable'] = $a_nat[$id]['barnyard_enable'];
	$pconfig['barnyard_mysql'] = $a_nat[$id]['barnyard_mysql'];
	$pconfig['barnconfigpassthru'] = base64_decode($a_nat[$id]['barnconfigpassthru']);
}

if (isset($_GET['dup']))
	unset($id);

$if_real = snort_get_real_interface($pconfig['interface']);
$snort_uuid = $pconfig['uuid'];

/* alert file */
$d_snortconfdirty_path = "/var/run/snort_conf_{$snort_uuid}_{$if_real}.dirty";

if ($_POST) {

	/* XXX: Mising error reporting?! 
	 * check for overlaps 
	foreach ($a_nat as $natent) {
		if (isset($id) && ($a_nat[$id]) && ($a_nat[$id] === $natent))
			continue;
		if ($natent['interface'] != $_POST['interface'])
			continue;
	}
	*/

	/* if no errors write to conf */
	if (!$input_errors) {
		$natent = array();
		/* repost the options already in conf */
		$natent = $pconfig;

		$natent['barnyard_enable'] = $_POST['barnyard_enable'] ? 'on' : 'off';
		$natent['barnyard_mysql'] = $_POST['barnyard_mysql'] ? $_POST['barnyard_mysql'] : $pconfig['barnyard_mysql'];
		$natent['barnconfigpassthru'] = $_POST['barnconfigpassthru'] ? base64_encode($_POST['barnconfigpassthru']) : $pconfig['barnconfigpassthru'];
		if ($_POST['barnyard_enable'] == "on")
			$natent['snortunifiedlog'] = 'on';
		else
			$natent['snortunifiedlog'] = 'off';

		if (isset($id) && $a_nat[$id])
			$a_nat[$id] = $natent;
		else {
			if (is_numeric($after))
				array_splice($a_nat, $after+1, 0, array($natent));
			else
				$a_nat[] = $natent;
		}

		write_config();
		sync_snort_package_config();

		/* after click go to this page */
		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		header("Location: snort_barnyard.php?id=$id");
		exit;
	}
}

$pgtitle = "Snort: Interface: $id$if_real Barnyard2 Edit";
include_once("head.inc");

?>
<body
	link="#0000CC" vlink="#0000CC" alink="#0000CC">


<?php include("fbegin.inc"); ?>
<?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include_once("fbegin.inc");
?>
<script language="JavaScript">
<!--

function enable_change(enable_change) {
	endis = !(document.iform.barnyard_enable.checked || enable_change);
	// make shure a default answer is called if this is envoked.
	endis2 = (document.iform.barnyard_enable);

    document.iform.barnyard_mysql.disabled = endis;
    document.iform.barnconfigpassthru.disabled = endis;
}
//-->
</script>


<?php
	/* Display Alert message */
	if ($input_errors) {
		print_input_errors($input_errors); // TODO: add checks
	}

	if ($savemsg) {
		print_info_box($savemsg);
	}

	?>

<form action="snort_barnyard.php" method="post"
	enctype="multipart/form-data" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[] = array(gettext("If Settings"), false, "/snort/snort_interfaces_edit.php?id={$id}");
        $tab_array[] = array(gettext("Categories"), false, "/snort/snort_rulesets.php?id={$id}");
        $tab_array[] = array(gettext("Rules"), false, "/snort/snort_rules.php?id={$id}");
        $tab_array[] = array(gettext("Servers"), false, "/snort/snort_define_servers.php?id={$id}");
        $tab_array[] = array(gettext("Preprocessors"), false, "/snort/snort_preprocessors.php?id={$id}");
        $tab_array[] = array(gettext("Barnyard2"), true, "/snort/snort_barnyard.php?id={$id}");
        display_top_tabs($tab_array);
?>
</td></tr>
	<tr>
		<td class="tabcont">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
			<tr>
				<td colspan="2" valign="top" class="listtopic">General Barnyard2
				Settings</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncellreq">Enable</td>
				<td width="78%" class="vtable">
					<input name="barnyard_enable" type="checkbox" value="on" <?php if ($pconfig['barnyard_enable'] == "on") echo "checked"; ?>  onClick="enable_change(false)">
					<strong>Enable Barnyard2 </strong><br>
					This will enable barnyard2 for this interface. You will also have to set the database credentials.</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Mysql Settings</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell">Log to a Mysql Database</td>
				<td width="78%" class="vtable"><input name="barnyard_mysql"
					type="text" class="formfld" id="barnyard_mysql" size="100"
					value="<?=htmlspecialchars($pconfig['barnyard_mysql']);?>"> <br>
				<span class="vexpl">Example: output database: alert, mysql,
				dbname=snort user=snort host=localhost password=xyz<br>
				Example: output database: log, mysql, dbname=snort user=snort
				host=localhost password=xyz</span></td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Advanced Settings</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell">Advanced configuration
				pass through</td>
				<td width="78%" class="vtable"><textarea name="barnconfigpassthru"
					cols="100" rows="7" id="barnconfigpassthru" ><?=htmlspecialchars($pconfig['barnconfigpassthru']);?></textarea>
				<br>
				Arguments here will be automatically inserted into the running
				barnyard2 configuration.</td>
			</tr>
			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%">
					<input name="Submit" type="submit" class="formbtn" value="Save">
					<input name="id" type="hidden" value="<?=$id;?>"> </td>
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
