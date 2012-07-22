<?php
/*
 * snort_rules.php
 *
 * Copyright (C) 2004, 2005 Scott Ullrich
 * Copyright (C) 2008, 2009 Robert Zelaya
 * Copyright (C) 2011 Ermal Luci
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

$snortdir = SNORTDIR;

if (!is_array($config['installedpackages']['snortglobal']['rule']))
	$config['installedpackages']['snortglobal']['rule'] = array();
$a_rule = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (is_null($id)) {
        header("Location: /snort/snort_interfaces.php");
        exit;
}

if (isset($id) && $a_rule[$id]) {
	$pconfig['enable'] = $a_rule[$id]['enable'];
	$pconfig['interface'] = $a_rule[$id]['interface'];
	$pconfig['rulesets'] = $a_rule[$id]['rulesets'];
	if (!empty($a_rule[$id]['customrules']))
		$pconfig['customrules'] = base64_decode($a_rule[$id]['customrules']);
}

function load_rule_file($incoming_file)
{
	//read file into string, and get filesize
	$contents = @file_get_contents($incoming_file);

	//split the contents of the string file into an array using the delimiter
	return explode("\n", $contents);
}

/* convert fake interfaces to real */
$if_real = snort_get_real_interface($pconfig['interface']);
$snort_uuid = $a_rule[$id]['uuid'];
$snortdownload = $config['installedpackages']['snortglobal']['snortdownload'];
$emergingdownload = $config['installedpackages']['snortglobal']['emergingthreats'];
$categories = explode("||", $pconfig['rulesets']);

if ($_GET['openruleset'])
	$currentruleset = $_GET['openruleset'];
else if ($_POST['openruleset'])
	$currentruleset = $_POST['openruleset'];
else
	$currentruleset = $categories[0];

$ruledir = "{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules";
$rulefile = "{$ruledir}/{$currentruleset}";
if ($currentruleset != 'custom.rules') {
if (!file_exists($rulefile)) {
	$input_errors[] = "{$currentruleset} seems to be missing!!! Please go to the Category tab and save again the rule to regenerate it.";
	$splitcontents = array();
} else
	//Load the rule file
	$splitcontents = load_rule_file($rulefile);
}

if ($_GET['act'] == "toggle" && $_GET['ids'] && !empty($splitcontents)) {

	$lineid= $_GET['ids'];

	//copy rule contents from array into string
	$tempstring = $splitcontents[$lineid];

	//explode rule contents into an array, (delimiter is space)
	$rule_content = explode(' ', $tempstring);

	$findme = "# alert"; //find string for disabled alerts
	$disabled = strstr($tempstring, $findme);

	//if find alert is false, then rule is disabled
	if ($disabled !== false) {
		//rule has been enabled
		$tempstring = substr($tempstring, 2);
	} else
		$tempstring = "# ". $tempstring;

	//copy string into array for writing
	$splitcontents[$lineid] = $tempstring;

	//write the new .rules file
	@file_put_contents($rulefile, implode("\n", $splitcontents));

	//write disable/enable sid to config.xml
	$sid = snort_get_rule_part($tempstring, 'sid:', ";", 0);
	if (is_numeric($sid)) {
		// rule_sid_on registers
		$sidon = explode("||", $a_rule[$id]['rule_sid_on']);
		if (!empty($sidon))
			$sidon = @array_flip($sidon);
		$sidoff = explode("||", $a_rule[$id]['rule_sid_off']);
		if (!empty($sidoff))
			$sidoff = @array_flip($sidoff);
		if ($disabled) {
			unset($sidoff["disablesid {$sid}"]);
			$sidon["enablesid {$sid}"] = count($sidon);
		} else {
			unset($sidon["enablesid {$sid}"]);
			$sidoff["disablesid {$sid}"] = count($sidoff);
		}
				
		$a_rule[$id]['rule_sid_on'] = implode("||", array_flip($sidon));
		$a_rule[$id]['rule_sid_off'] = implode("||", array_flip($sidoff));
		write_config();
	}

	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
	exit;
}

if ($_POST['customrules']) {
	$a_rule[$id]['customrules'] = base64_encode($_POST['customrules']);
	write_config();
	sync_snort_package_config();
	$output = "";
	$retcode = "";
	exec("snort -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/snort.conf -T 2>&1", $output, $retcode);
	if (intval($retcode) != 0) {
		$error = "";
		$start = count($output);
		$end = $start - 4;
		for($i = $start; $i > $end; $i--)
			$error .= $output[$i];
		$input_errors[] = "Custom rules have errors:\n {$error}";
	} else {
		header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
		exit;
	}
} else if ($_POST) {
	unset($a_rule[$id]['customrules']);
	write_config();
	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
	exit;
}

require_once("guiconfig.inc");
include_once("head.inc");

$if_friendly = snort_get_friendly_interface($pconfig['interface']);
$pgtitle = "Snort: {$if_friendly} Category: $currentruleset";
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php
include("fbegin.inc");
if ($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}

/* Display message */
if ($input_errors) {
        print_input_errors($input_errors); // TODO: add checks
}

if ($savemsg) {
        print_info_box($savemsg);
}

?>

<script language="javascript" type="text/javascript">
function go()
{
    var box = document.iform.selectbox;
    destination = box.options[box.selectedIndex].value;
    if (destination) 
		location.href = destination;
}
function popup(url) 
{
 params  = 'width='+screen.width;
 params += ', height='+screen.height;
 params += ', top=0, left=0'
 params += ', fullscreen=yes';

 newwin=window.open(url,'windowname4', params);
 if (window.focus) {newwin.focus()}
 return false;
}
</script>

<form action="/snort/snort_rules.php" method="post" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[] = array(gettext("If Settings"), false, "/snort/snort_interfaces_edit.php?id={$id}");
        $tab_array[] = array(gettext("Categories"), false, "/snort/snort_rulesets.php?id={$id}");
        $tab_array[] = array(gettext("Rules"), true, "/snort/snort_rules.php?id={$id}");
        $tab_array[] = array(gettext("Variables"), false, "/snort/snort_define_servers.php?id={$id}");
        $tab_array[] = array(gettext("Preprocessors"), false, "/snort/snort_preprocessors.php?id={$id}");
        $tab_array[] = array(gettext("Barnyard2"), false, "/snort/snort_barnyard.php?id={$id}");
        display_top_tabs($tab_array);
?>
</td></tr>
<tr>
	<td>
	<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td width="3%" class="list">&nbsp;</td>
		<td class="listhdr" colspan="7">
		<br/>Category:  
		<select id="selectbox" name="selectbox" class="formselect" onChange="go()">
			<option value='?id=<?=$id;?>&openruleset=custom.rules'>custom.rules</option>
		<?php
			$files = explode("||", $pconfig['rulesets']);
			foreach ($files as $value) {
				if ($snortdownload != 'on' && strstr($value, "snort"))
					continue;
				if ($emergingdownload != 'on' && strstr($value, "emerging"))
					continue;
				echo "<option value='?id={$id}&openruleset={$value}' ";
				if ($value === $currentruleset)
					echo "selected";
				echo ">{$value}</option>\n";
			}
		?>
		</select>
		<br/>
		</td>
		<td width="5%" class="list">&nbsp;</td>
	</tr>
<?php if ($currentruleset == 'custom.rules' || empty($pconfig['rulesets'])): ?>
	<tr>
		<td width="3%" class="list">&nbsp;</td>
		<td valign="top" class="vtable">
			<input type='hidden' name='openruleset' value='custom.rules'>
			<input type='hidden' name='id' value='<?=$id;?>'>
			
			<textarea wrap="on" cols="90" rows="50" name="customrules"><?=$pconfig['customrules'];?></textarea>
		</td>
	</tr>
	<tr>
		<td width="3%" class="list">&nbsp;</td>
		<td class="vtable">
			<input name="Submit" type="submit" class="formbtn" value="Save">
			<input type="button" class="formbtn" value="Cancel" onclick="history.back()">
		</td>
	</tr>
<?php else: ?>
	<tr>
		<td width="3%" class="list">&nbsp;</td>
		<td colspan="7" class="listhdr" >&nbsp;</td>
		<td width="5%" class="list">&nbsp;</td>
	</tr>
	<tr id="frheader">
		<td width="3%" class="list">&nbsp;</td>
		<td width="7%" class="listhdr">SID</td>
		<td width="4%" class="listhdrr">Proto</td>
		<td width="15%" class="listhdrr">Source</td>
		<td width="10%" class="listhdrr">Port</td>
		<td width="15%" class="listhdrr">Destination</td>
		<td width="10%" class="listhdrr">Port</td>
		<td width="30%" class="listhdrr">Message</td>
		<td width="5%" class="list">&nbsp;</td>
	</tr>
<?php
	foreach ( $splitcontents as $counter => $value )
	{
		$disabled = "False";
		$comments = "False";
		$findme = "# alert"; //find string for disabled alerts
		$disabled_pos = strstr($value, $findme);

		$counter2 = 1;
		$sid = snort_get_rule_part($value, 'sid:', ';', 0);
		//check to see if the sid is numberical
		if (!is_numeric($sid))
			continue;

		//if find alert is false, then rule is disabled
		if ($disabled_pos !== false){
			$counter2 = $counter2+1;
			$textss = "<span class=\"gray\">";
			$textse = "</span>";
			$iconb = "icon_block_d.gif";

			$ischecked = "";
		} else {
			$textss = $textse = "";
			$iconb = "icon_block.gif";

			$ischecked = "checked";
		}

		$rule_content = explode(' ', $value);

		$protocol = $rule_content[$counter2];//protocol location
		$counter2++;
		$source = substr($rule_content[$counter2], 0, 20) . "...";//source location
		$counter2++;
		$source_port = $rule_content[$counter2];//source port location
		$counter2 = $counter2+2;
		$destination = substr($rule_content[$counter2], 0, 20) . "...";//destination location
		$counter2++;
		$destination_port = $rule_content[$counter2];//destination port location

		if (strstr($value, 'msg: "'))
			$message = snort_get_rule_part($value, 'msg: "', '";', 0);
		else if (strstr($value, 'msg:"'))
			$message = snort_get_rule_part($value, 'msg:"', '";', 0);

		echo "<tr><td width='3%' class='listt'> $textss
			<a href='?id={$id}&openruleset={$currentruleset}&act=toggle&ids={$counter}'>
			<img src='../themes/{$g['theme']}/images/icons/{$iconb}'
			width='10' height='10' border='0'
			title='click to toggle enabled/disabled status'></a>
			$textse
		       </td>
		       <td width='7%' class=\"listlr\">
				$textss $sid $textse
		       </td>
		       <td width='4%' class=\"listlr\">
				$textss $protocol $textse
		       </td>
		       <td width='15%' class=\"listlr\">
				$textss $source $textse
		       </td>
		       <td width='10%' class=\"listlr\">
				$textss $source_port $textse
		       </td>
		       <td width='15%' class=\"listlr\">
				$textss $destination $textse
		       </td>
		       <td width='10%' class=\"listlr\">
			       $textss $destination_port $textse
		       </td>
			<td width='30%' class=\"listbg\"><font color=\"white\"> 
				$textss $message $textse
		       </td>";
	?>
			<td width='5%' valign="middle" nowrap class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			<tr>
				<td><a href="javascript: void(0)"
					onclick="popup('snort_rules_edit.php?id=<?=$id;?>&openruleset=<?=$currentruleset;?>')"><img
					src="../themes/<?= $g['theme']; ?>/images/icons/icon_right.gif"
					title="edit rule" width="17" height="17" border="0"></a></td>
					<!-- Codes by Quackit.com -->
			</tr>
			</table>
			</td>
		</tr>
<?php

	}
?>
		
	</table>
	</td>
</tr>
<?php endif;?>
<tr>
	<td colspan="9">
	<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
		<tr>
			<td width="16"><img
				src="../themes/<?= $g['theme']; ?>/images/icons/icon_block.gif"
				width="11" height="11"></td>
			<td>Rule Enabled</td>
		</tr>
		<tr>
			<td><img
				src="../themes/<?= $g['theme']; ?>/images/icons/icon_block_d.gif"
				width="11" height="11"></td>
			<td nowrap>Rule Disabled</td>
		</tr>
		<tr>
			<td colspan="10">
			<p><!--<strong><span class="red">Warning:<br/> </span></strong>Editing these r</p>-->
			</td>
		</tr>
	</table>
	</td>
</tr>
</table>
</td>
</tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
