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

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

$ids = $_GET['ids'];
if (isset($_POST['ids']))
	$ids = $_POST['ids'];

if (isset($id) && $a_nat[$id]) {
	$pconfig['enable'] = $a_nat[$id]['enable'];
	$pconfig['interface'] = $a_nat[$id]['interface'];
	$pconfig['rulesets'] = $a_nat[$id]['rulesets'];
}

//get rule id
$lineid = $_GET['ids'];
if (isset($_POST['ids']))
	$lineid = $_POST['ids'];

$file = $_GET['openruleset'];
if (isset($_POST['openruleset']))
	$file = $_POST['openruleset'];

//read file into string, and get filesize also chk for empty files
$contents = '';
if (filesize($file) > 0 )
	$contents = file_get_contents($file);

//delimiter for each new rule is a new line
$delimiter = "\n";

//split the contents of the string file into an array using the delimiter
$splitcontents = explode($delimiter, $contents);
$findme = "# alert"; //find string for disabled alerts
$highlight = "yes";
if (strstr($splitcontents[$lineid], $findme))
	$highlight = "no";
if ($highlight == "no")
	$splitcontents[$lineid] = substr($splitcontents[$lineid], 2);

if (!function_exists('get_middle')) {
	function get_middle($source, $beginning, $ending, $init_pos) {
		$beginning_pos = strpos($source, $beginning, $init_pos);
		$middle_pos = $beginning_pos + strlen($beginning);
		$ending_pos = strpos($source, $ending, $beginning_pos);
		$middle = substr($source, $middle_pos, $ending_pos - $middle_pos);
		return $middle;
	}
}

if ($_POST) {
	if ($_POST['save']) {

		//copy string into file array for writing
		if ($_POST['highlight'] == "yes")
			$splitcontents[$lineid] = $_POST['code'];
		else
			$splitcontents[$lineid] = "# " . $_POST['code'];

		//write disable/enable sid to config.xml
		$sid = get_middle($splitcontents[$lineid], 'sid:', ';', 0);
		if (is_numeric($sid)) {
			// rule_sid_on registers
			if (!empty($a_nat[$id]['rule_sid_on']))
				$a_nat[$id]['rule_sid_on'] = str_replace("||enablesid $sid", "", $a_nat[$id]['rule_sid_on']);
			if (!empty($a_nat[$id]['rule_sid_on']))
				$a_nat[$id]['rule_sid_off'] = str_replace("||disablesid $sid", "", $a_nat[$id]['rule_sid_off']);
			if ($_POST['highlight'] == "yes")
				$a_nat[$id]['rule_sid_on'] = "||enablesid $sid" . $a_nat[$id]['rule_sid_on'];
			else
				$a_nat[$id]['rule_sid_off'] = "||disablesid $sid" . $a_nat[$id]['rule_sid_off'];
		}

		//write the new .rules file
		@file_put_contents($file, implode($delimiter, $splitcontents));

		write_config();

		echo "<script> opener.window.location.reload(); window.close(); </script>";
		exit;
	}
}

$pgtitle = array(gettext("Advanced"), gettext("File Editor"));

?>

<?php include("head.inc");?>

<body link="#000000" vlink="#000000" alink="#000000">
	<?php if ($savemsg) print_info_box($savemsg); ?>
<?php include("fbegin.inc");?>

<form action="snort_rules_edit.php" method="post">
<input type='hidden' name='id' value='<?=$id;?>' />
<input type='hidden' name='ids' value='<?=$ids;?>' />
<input type='hidden' name='openruleset' value='<?=$file;?>' />
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
	<td class="tabcont">
		<table width="100%" cellpadding="0" cellspacing="6" bgcolor="#eeeeee">
		<tr>
			<td>
				<input name="save" type="submit" class="formbtn" id="save" value="save" />
				<input type="button" class="formbtn" value="Cancel" onclick="window.close()">
				<hr noshade="noshade" />
				Disable original rule :<br/>

				<input id="highlighting_enabled" name="highlight2" type="radio" value="yes" <?php if($highlight == "yes") echo " checked=\"checked\""; ?> />
				<label for="highlighting_enabled"><?=gettext("Enabled");?> </label>
				<input id="highlighting_disabled" name="highlight2" type="radio" value="no" <?php if($highlight == "no") echo " checked=\"checked\""; ?> />
				<label for="highlighting_disabled"> <?=gettext("Disabled");?></label>
			</td>
		</tr>
		<tr> 
			<td valign="top" class="label"> 
			<textarea wrap="off" cols="90" rows="3" name="code"><?=$splitcontents[$lineid];?></textarea>
			</td> 
		</tr> 
		<tr>
			<td valign="top" class="label">
			<div style="background: #eeeeee;" id="textareaitem"><!-- NOTE: The opening *and* the closing textarea tag must be on the same line. -->
			<textarea disabled wrap="off" rows="33" cols="90" name="code2"><?=$contents;?></textarea>
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
