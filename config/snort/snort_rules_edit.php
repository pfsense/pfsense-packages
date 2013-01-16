<?php
/*
 * snort_rules_edit.php
 *
 * Copyright (C) 2004, 2005 Scott Ullrich
 * Copyright (C) 2011 Ermal Luci
 * All rights reserved.
 *
 * Adapted for FreeNAS by Volker Theile (votdev@gmx.de)
 * Copyright (C) 2006-2009 Volker Theile
 *
 * Adapted for Pfsense Snort package by Robert Zelaya
 * Copyright (C) 2008-2009 Robert Zelaya
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

$snortdir = SNORTDIR;

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}
$a_rule = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (is_null($id)) {
	header("Location: /snort/snort_interfaces.php");
	exit;
}

if (isset($id) && $a_rule[$id]) {
	$pconfig['enable'] = $a_rule[$id]['enable'];
	$pconfig['interface'] = $a_rule[$id]['interface'];
	$pconfig['rulesets'] = $a_rule[$id]['rulesets'];
}

/* convert fake interfaces to real */
$if_real = snort_get_real_interface($pconfig['interface']);
$snort_uuid = $a_rule[$id]['uuid'];
$file = $_GET['openruleset'];

//read file into string, and get filesize also chk for empty files
$contents = '';
if (file_exists("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$file}"))
	$contents = file_get_contents("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$file}");
else {
	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$file}");
	exit;
}

//split the contents of the string file into an array using the delimiter
$splitcontents = explode("\n", $contents);

$pgtitle = array(gettext("Advanced"), gettext("File Editor"));

?>

<?php include("head.inc");?>

<body link="#000000" vlink="#000000" alink="#000000">
	<?php if ($savemsg) print_info_box($savemsg); ?>
<?php include("fbegin.inc");?>

<form action="snort_rules_edit.php" method="post">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
	<td class="tabcont">
		<table width="100%" cellpadding="0" cellspacing="6" bgcolor="#eeeeee">
		<tr>
			<td>
				<input type="button" class="formbtn" value="Cancel" onclick="window.close()">
			</td>
		</tr>
		<tr>
			<td valign="top" class="label">
			<div style="background: #eeeeee;" id="textareaitem"><!-- NOTE: The opening *and* the closing textarea tag must be on the same line. -->
			<textarea wrap="off" rows="33" cols="90" name="code2"><?=$contents;?></textarea>
			</div>
			</td>
		</tr>
		</table>
	</td>
</tr>
</table>
</form>
<?php include("fend.inc");?>
</body>
</html>
