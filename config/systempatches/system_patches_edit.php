<?php
/*
	system_patches_edit.php
	Copyright (C) 2012 Jim Pingle
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
	pfSense_MODULE:	system
*/

##|+PRIV
##|*IDENT=page-system-patches-edit
##|*NAME=System: Edit Patches
##|*DESCR=Allow access to the 'System: Edit Patches' page.
##|*MATCH=system_patches_edit.php*
##|-PRIV

require("guiconfig.inc");
require_once("itemid.inc");
require_once("patches.inc");

if (!is_array($config['installedpackages']['patches']['item'])) {
	$config['installedpackages']['patches']['item'] = array();
}
$a_patches = &$config['installedpackages']['patches']['item'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

if (isset($_GET['dup'])) {
	$id = $_GET['dup'];
	$after = $_GET['dup'];
}

if (isset($id) && $a_patches[$id]) {
	$pconfig['descr'] = $a_patches[$id]['descr'];
	$pconfig['location'] = $a_patches[$id]['location'];
	$pconfig['patch'] = $a_patches[$id]['patch'];
	$pconfig['patchlevel'] = $a_patches[$id]['patchlevel'];
	$pconfig['ignorewhitespace'] = isset($a_patches[$id]['ignorewhitespace']);
	$pconfig['autoapply'] = isset($a_patches[$id]['autoapply']);
	$pconfig['uniqid'] = $a_patches[$id]['uniqid'];
}

if (isset($_GET['dup']))
	unset($id);

unset($input_errors);

if ($_POST) {
	$pconfig = $_POST;

	/* input validation */
	if(empty($_POST['location'])) {
		$reqdfields = explode(" ", " patch");
		$reqdfieldsn = array(gettext("Description"),gettext("Patch Contents"));
	} else {
		$reqdfields = explode(" ", "descr location");
		$reqdfieldsn = array(gettext("Description"),gettext("URL/Commit ID"));
	}

	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if (!empty($_POST['location']) && !is_commit_id($_POST['location']) && !is_URL($_POST['location'])) {
		$input_errors[] = gettext("The supplied commit ID/URL appears to be invalid.");
	}
	if (!is_numeric($_POST['patchlevel'])) {
		$input_errors[] = gettext("Patch level must be numeric!");
	}

	if (!$input_errors) {
		$thispatch = array();

		$thispatch['descr'] = $_POST['descr'];
		$thispatch['location'] = patch_fixup_url($_POST['location']);
		if (!empty($_POST['patch'])) {
			$thispatch['patch'] = base64_encode($_POST['patch']);
		}
		if (is_github_url($thispatch['location']) && ($_POST['patchlevel'] == 0))
			$thispatch['patchlevel'] = 1;
		else
			$thispatch['patchlevel'] = $_POST['patchlevel'];
		$thispatch['ignorewhitespace'] = isset($_POST['ignorewhitespace']);
		$thispatch['autoapply'] = isset($_POST['autoapply']);
		if (empty($_POST['uniqid'])) {
			$thispatch['uniqid'] = uniqid();
		} else {
			$thispatch['uniqid'] = $_POST['uniqid'];
		}

		// Update the patch entry now
		if (isset($id) && $a_patches[$id])
			$a_patches[$id] = $thispatch;
		else {
			if (is_numeric($after))
				array_splice($a_patches, $after+1, 0, array($thispatch));
			else
				$a_patches[] = $thispatch;
		}

		write_config();
		header("Location: system_patches.php");
		return;
	}
}

$pgtitle = array(gettext("System"),gettext("Patches"), gettext("Edit"));
include("head.inc");

?>
<link rel="stylesheet" href="/pfCenter/javascript/chosen/chosen.css" />
</head>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<script src="/pfCenter/javascript/chosen/chosen.proto.js" type="text/javascript"></script>

<?php
include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<form action="system_patches_edit.php" method="post" name="iform" id="iform">
<table width="100%" border="0" cellpadding="6" cellspacing="0">
<tr>
	<td colspan="2" valign="top" class="listtopic"><?=gettext("Edit Patch Entry"); ?></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncellreq"><strong><?=gettext("Description"); ?></strong></td>
	<td width="78%" class="vtable">
		<input name="descr" type="text" class="formfld unknown" id="descr" size="40" value="<?=htmlspecialchars($pconfig['descr']);?>">
		<br> <span class="vexpl"><?=gettext("Enter a description here for your reference."); ?></span></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?=gettext("URL/Commit ID"); ?></td>
	<td width="78%" class="vtable">
		<input name="location" type="text" class="formfld unknown" id="location" size="40" value="<?=htmlspecialchars($pconfig['location']);?>">
		<br> <span class="vexpl"><?=gettext("Enter a URL to a patch, or a commit ID from the main github repository (NOT the tools or packages repos!)."); ?></span></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?=gettext("Patch Contents"); ?></td>
	<td width="78%" class="vtable">
		<textarea name="patch" class="" id="patch" ROWS="15" COLS="70" wrap="off"><?=base64_decode($pconfig['patch']);?></textarea>
		<br> <span class="vexpl"><?=gettext("The contents of the patch. You can paste a patch here, or enter a URL/commit ID above, it can then be fetched into here automatically."); ?></span></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?=gettext("Patch Level:"); ?></td>
	<td width="78%" class="vtable">
		<select name="patchlevel" class="formselect" id="patchlevel">
<?php		for ($i = 0; $i < 20; $i++): ?>
			<option value="<?=$i;?>" <?php if ($i == $pconfig['patchlevel']) echo "selected"; ?>><?=$i;?></option>
<?php 		endfor; ?>
		</select>
	</td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?=gettext("Ignore Whitespace"); ?></td>
	<td width="78%" class="vtable">
		<input name="ignorewhitespace" type="checkbox" id="ignorewhitespace" value="yes" <?php if ($pconfig['ignorewhitespace']) echo "checked"; ?>>
		<strong><?=gettext("Ignore Whitespace"); ?></strong><br />
		<span class="vexpl"><?=gettext("Set this option to ignore whitespace in the patch."); ?></span>
	</td>
</tr>
<!-- This isn't ready yet 
<tr>
	<td width="22%" valign="top" class="vncell"><?=gettext("Auto Apply"); ?></td>
	<td width="78%" class="vtable">
		<input name="autoapply" type="checkbox" id="autoapply" value="yes" <?php if ($pconfig['autoapply']) echo "checked"; ?>>
		<strong><?=gettext("Auto-Apply Patch"); ?></strong><br />
		<span class="vexpl"><?=gettext("Set this option to apply the patch automatically when possible, useful for patches to survive after firmware updates."); ?></span>
	</td>
</tr>
-->
<tr>
	<td width="22%" valign="top">&nbsp;</td>
	<td width="78%">Patch id: <?php echo $pconfig['uniqid']; ?></td>
</tr>
<tr>
	<td width="22%" valign="top">&nbsp;</td>
	<td width="78%">
		<input name="Submit" type="submit" class="formbtn" value="<?=gettext("Save"); ?>"> <input type="button" class="formbtn" value="<?=gettext("Cancel"); ?>" onclick="history.back()">
		<?php if (isset($id) && $a_patches[$id]): ?>
		<input name="id" type="hidden" value="<?=htmlspecialchars($id);?>">
		<input name="uniqid" type="hidden" value="<?=htmlspecialchars($pconfig['uniqid']);?>">
		<?php endif; ?>
	</td>
</tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
