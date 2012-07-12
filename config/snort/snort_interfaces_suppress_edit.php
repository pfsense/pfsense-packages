<?php
/*
 * firewall_aliases_edit.php
 * Copyright (C) 2004 Scott Ullrich
 * All rights reserved.
 *
 * originially part of m0n0wall (http://m0n0.ch/wall)
 * Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 * All rights reserved.
 *
 * modified for the pfsense snort package
 * Copyright (C) 2009-2010 Robert Zelaya.
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


if (!is_array($config['installedpackages']['snortglobal']))
	$config['installedpackages']['snortglobal'] = array();
$snortglob = $config['installedpackages']['snortglobal'];

if (!is_array($config['installedpackages']['snortglobal']['suppress']))
	$config['installedpackages']['snortglobal']['suppress'] = array();
if (!is_array($config['installedpackages']['snortglobal']['suppress']['item']))
	$config['installedpackages']['snortglobal']['suppress']['item'] = array();
$a_suppress = &$config['installedpackages']['snortglobal']['suppress']['item'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];


/* returns true if $name is a valid name for a whitelist file name or ip */
function is_validwhitelistname($name) {
	if (!is_string($name))
		return false;

	if (!preg_match("/[^a-zA-Z0-9\.\/]/", $name))
		return true;

	return false;
}

if (isset($id) && $a_suppress[$id]) {

	/* old settings */
	$pconfig['name'] = $a_suppress[$id]['name'];
	$pconfig['uuid'] = $a_suppress[$id]['uuid'];
	$pconfig['descr'] = $a_suppress[$id]['descr'];
	if (!empty($a_suppress[$id]['suppresspassthru']));
		$pconfig['suppresspassthru'] = base64_decode($a_suppress[$id]['suppresspassthru']);
	if (empty($a_suppress[$id]['uuid']))
		$pconfig['uuid'] = uniqid();
}

if ($_POST['submit']) {
	unset($input_errors);
	$pconfig = $_POST;

	$reqdfields = explode(" ", "name");
	$reqdfieldsn = array("Name");
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if(strtolower($_POST['name']) == "defaultwhitelist")
		$input_errors[] = "Whitelist file names may not be named defaultwhitelist.";

	if (is_validwhitelistname($_POST['name']) == false)
		$input_errors[] = "Whitelist file name may only consist of the characters a-z, A-Z and 0-9 _. Note: No Spaces. Press Cancel to reset.";

	/* check for name conflicts */
	foreach ($a_suppress as $s_list) {
		if (isset($id) && ($a_suppress[$id]) && ($a_suppress[$id] === $s_list))
			continue;

		if ($s_list['name'] == $_POST['name']) {
			$input_errors[] = "A whitelist file name with this name already exists.";
			break;
		}
	}


	if (!$input_errors) {
		$s_list = array();
		$s_list['name'] = $_POST['name'];
		$s_list['uuid'] = $suppress_uuid;
		$s_list['descr']  =  mb_convert_encoding($_POST['descr'],"HTML-ENTITIES","auto");
		if ($_POST['suppresspassthru'])
			$s_list['suppresspassthru'] = base64_encode($_POST['suppresspassthru']);

		if (isset($id) && $a_suppress[$id])
			$a_suppress[$id] = $s_list;
		else
			$a_suppress[] = $s_list;

		write_config();
		sync_snort_package_config();

		header("Location: /snort/snort_interfaces_suppress.php");
		exit;
	}
}

$pgtitle = "Services: Snort: Suppression: Edit $suppress_uuid";
include_once("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<?php
include("fbegin.inc");
if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}

if ($input_errors) print_input_errors($input_errors);
if ($savemsg)
	print_info_box($savemsg);

?>
<form action="/snort/snort_interfaces_suppress_edit.php" name="iform" id="iform" method="post">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabcont">
<table width="100%" border="0" cellpadding="6" cellspacing="0">
<tr>
	<td colspan="2" class="listtopic">Add the name and description of the file.</td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncellreq">Name</td>
	<td width="78%" class="vtable"><input name="name" type="text" id="name"
		class="formfld unkown" size="40" value="<?=htmlspecialchars($pconfig['name']);?>" /> <br />
	<span class="vexpl"> The list name may only consist of the
	characters a-z, A-Z and 0-9. <span class="red">Note: </span> No
	Spaces. </span></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell">Description</td>
	<td width="78%" class="vtable"><input name="descr" type="text"
		class="formfld unkown" id="descr" size="40" value="<?=$pconfig['descr'];?>" /> <br />
	<span class="vexpl"> You may enter a description here for your
	reference (not parsed). </span></td>
</tr>
<tr>
	<td colspan="2">
		<div style='background-color: #E0E0E0' id='redbox'>
		<table width='100%'>
			<tr>
				<td width='8%'>&nbsp;&nbsp;&nbsp;</td>
				<td width='70%'><font size="2" color='#FF850A'><b>NOTE:</b></font>
				<font color='#000000'>&nbsp;&nbsp;The threshold keyword
				is deprecated as of version 2.8.5. Use the event_filter keyword
				instead.</font></td>
			</tr>
		</table>
		</div>
	</td>
</tr>
<tr>
	<td colspan="2" valign="top" class="listtopic">Apply suppression or
	filters to rules. Valid keywords are 'suppress', 'event_filter' and
	'rate_filter'.</td>
</tr>
<tr>
	<td colspan="2" valign="top" class="vncell"><b>Example 1;</b>
		suppress gen_id 1, sig_id 1852, track by_src, ip 10.1.1.54<br>
		<b>Example 2;</b> event_filter gen_id 1, sig_id 1851, type limit,
		track by_src, count 1, seconds 60<br>
		<b>Example 3;</b> rate_filter gen_id 135, sig_id 1, track by_src,
		count 100, seconds 1, new_action log, timeout 10</td>
</tr>
<tr>
	<td width="10%" class="vncell">&nbsp;Advanced pass through</td>
	<td width="100%" class="vtable"><textarea wrap="off"
		name="suppresspassthru" cols="90" rows="28" id="suppresspassthru" class="formpre"><?=htmlspecialchars($pconfig['suppresspassthru']);?></textarea>
	</td>
</tr>
<tr>
	<td width="22%">&nbsp</td>
	<td width="78%"><input id="submit" name="submit" type="submit"
		class="formbtn" value="Save" /> <input id="cancelbutton"
		name="cancelbutton" type="button" class="formbtn" value="Cancel"
		onclick="history.back()" /> <?php if (isset($id) && $a_suppress[$id]): ?>
		<input name="id" type="hidden" value="<?=$id;?>" /> <?php endif; ?>
	</td>
</tr>
</table>
</td></tr>
</table>
</form>
<?php include("fend.inc"); ?>
<script type="text/javascript">
Rounded("div#redbox","all","#FFF","#E0E0E0","smooth");
</script>
</body>
</html>
