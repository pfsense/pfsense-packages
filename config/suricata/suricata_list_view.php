<?php
/*
 * suricata_list_view.php
 *
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

require_once("guiconfig.inc");
require_once("/usr/local/pkg/suricata/suricata.inc");

global $g, $config;

$contents = '';

if (isset($_GET['id']) && is_numericint($_GET['id']))
	$id = htmlspecialchars($_GET['id']);

$wlist = htmlspecialchars($_GET['wlist']);
$type = htmlspecialchars($_GET['type']);

if (isset($id) && isset($wlist)) {
	$a_rule = $config['installedpackages']['suricata']['rule'][$id];
	if ($type == "homenet") {
		$list = suricata_build_list($a_rule, $wlist);
		$contents = implode("\n", $list);
	}
	elseif ($type == "whitelist") {
		$list = suricata_build_list($a_rule, $wlist, true);
		$contents = implode("\n", $list);
	}
	elseif ($type == "suppress") {
		$list = suricata_find_list($wlist, $type);
		$contents = str_replace("\r", "", base64_decode($list['suppresspassthru']));
	}
	else
		$contents = gettext("\n\nERROR -- Requested List Type entity is not valid!");
}
else
	$contents = gettext("\n\nERROR -- Supplied interface or List entity is not valid!");

$pgtitle = array(gettext("Suricata"), gettext(ucfirst($type) . " Viewer"));
?>

<?php include("head.inc");?>

<body link="#000000" vlink="#000000" alink="#000000">
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php // include("fbegin.inc");?>

<form action="suricata_list_view.php" method="post">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
	<td class="tabcont">
		<table width="100%" cellpadding="0" cellspacing="6" bgcolor="#eeeeee">
		<tr>
			<td class="pgtitle" colspan="2">Suricata: <?php echo gettext(ucfirst($type) . " Viewer"); ?></td>
		</tr>
		<tr>
			<td align="left" width="20%">
				<input type="button" class="formbtn" value="Return" onclick="window.close()">
			</td>
			<td align="right">
				<b><?php echo gettext(ucfirst($type) . ": ") . '</b>&nbsp;' . $_GET['wlist']; ?>&nbsp;&nbsp;&nbsp;&nbsp;
			</td>
		</tr>
		<tr>
			<td colspan="2" valign="top" class="label">
			<div style="background: #eeeeee; width:100%; height:100%;" id="textareaitem"><!-- NOTE: The opening *and* the closing textarea tag must be on the same line. -->
			<textarea style="width:100%; height:100%;" readonly wrap="off" rows="25" cols="80" name="code2"><?=htmlspecialchars($contents);?></textarea>
			</div>
			</td>
		</tr>
		</table>
	</td>
</tr>
</table>
</form>
<?php // include("fend.inc");?>
</body>
</html>
