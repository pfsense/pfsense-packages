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

global $g, $flowbit_rules_file, $rebuild_rules;

$snortdir = SNORTDIR;
$rules_map = array();

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

function truncate($string, $length) {

	/********************************
	 * This function truncates the  *
	 * passed string to the length  *
	 * specified adding ellipsis if *
	 * truncation was necessary.    *
	 ********************************/
	if (strlen($string) > $length)
		$string = substr($string, 0, ($length - 3)) . "...";
	return $string; 
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

if (empty($categories[0]) && ($currentruleset != "custom.rules")) {
	if (!empty($a_rule[$id]['ips_policy']))
		$currentruleset = "IPS Policy - " . ucfirst($a_rule[$id]['ips_policy']);
	else
		$currentruleset = "custom.rules";
}

/* One last sanity check -- if the rules directory is empty, default to loading custom rules */
$tmp = glob("{$snortdir}/rules/*.rules");
if (empty($tmp))
	$currentruleset = "custom.rules";

$ruledir = "{$snortdir}/rules";
$rulefile = "{$ruledir}/{$currentruleset}";
if ($currentruleset != 'custom.rules') {
	// Read the current rules file into our rules map array.
	// Test for the special case of an IPS Policy file.
	if (substr($currentruleset, 0, 10) == "IPS Policy")
		$rules_map = snort_load_vrt_policy($a_rule[$id]['ips_policy']);
	elseif (!file_exists($rulefile))
		$input_errors[] = gettext("{$currentruleset} seems to be missing!!! Please verify rules files have been downloaded, then go to the Categories tab and save the rule set again.");
	else
		$rules_map = snort_load_rules_map($rulefile);
}

/* Load up our enablesid and disablesid arrays with enabled or disabled SIDs */
$enablesid = snort_load_sid_mods($a_rule[$id]['rule_sid_on'], "enablesid");
$disablesid = snort_load_sid_mods($a_rule[$id]['rule_sid_off'], "disablesid");

if ($_GET['act'] == "toggle" && $_GET['ids'] && !empty($rules_map)) {

	// Get the SID tag embedded in the clicked rule icon.
	$sid= $_GET['ids'];

	// See if the target SID is in our list of modified SIDs,
	// and toggle it if present; otherwise, add it to the
	// appropriate list.
	if (isset($enablesid[$sid])) {
		unset($enablesid[$sid]);
		if (!isset($disablesid[$sid]))
			$disablesid[$sid] = "disablesid";
	}
	elseif (isset($disablesid[$sid])) {
		unset($disablesid[$sid]);
		if (!isset($enablesid[$sid]))
			$enablesid[$sid] = "enablesid";
	}
	else {
		if ($rules_map[1][$sid]['disabled'] == 1)
			$enablesid[$sid] = "enablesid";
		else
			$disablesid[$sid] = "disablesid";
	}

	// Write the updated enablesid and disablesid values to the config file.
	$tmp = "";
	foreach ($enablesid as $k => $v) {
		$tmp .= "||{$v} {$k}";
	}
	if (!empty($tmp))
		$a_rule[$id]['rule_sid_on'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_on']);
	$tmp = "";
	foreach ($disablesid as $k => $v) {
		$tmp .= "||{$v} {$k}";
	}
	if (!empty($tmp))
		$a_rule[$id]['rule_sid_off'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_off']);

	/* Update the config.xml file. */
	write_config();

	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
	exit;
}

if ($_GET['act'] == "resetcategory" && !empty($rules_map)) {

	// Reset any modified SIDs in the current rule category to their defaults.
	foreach (array_keys($rules_map) as $k1) {
		foreach (array_keys($rules_map[$k1]) as $k2) {
			if (isset($enablesid[$k2]))
				unset($enablesid[$k2]);
			if (isset($disablesid[$k2]))
				unset($disablesid[$k2]);
		}
	}

	// Write the updated enablesid and disablesid values to the config file.
	$tmp = "";
	foreach ($enablesid as $k => $v) {
		$tmp .= "||{$v} {$k}";
	}
	if (!empty($tmp))
		$a_rule[$id]['rule_sid_on'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_on']);
	$tmp = "";
	foreach ($disablesid as $k => $v) {
		$tmp .= "||{$v} {$k}";
	}
	if (!empty($tmp))
		$a_rule[$id]['rule_sid_off'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_off']);
	write_config();

	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
	exit;
}

if ($_GET['act'] == "resetall" && !empty($rules_map)) {

	// Remove all modified SIDs from config.xml and save the changes.
	unset($a_rule[$id]['rule_sid_on']);
	unset($a_rule[$id]['rule_sid_off']);

	/* Update the config.xml file. */
	write_config();

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
	}
	else {
		header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
		exit;
	}
}

else if ($_POST['apply']) {
	write_config();
	$rebuild_rules = "on";
	sync_snort_package_config();
	$rebuild_rules = "off";
	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
	exit;
}
else if($_POST) {
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
		<td class="listhdr" colspan="4">
		<br/>Category:  
		<select id="selectbox" name="selectbox" class="formselect" onChange="go()">
			<option value='?id=<?=$id;?>&openruleset=custom.rules'>custom.rules</option>
		<?php
			$files = explode("||", $pconfig['rulesets']);
			if ($a_rule[$id]['ips_policy_enable'] == 'on')
				$files[] = "IPS Policy - " . ucfirst($a_rule[$id]['ips_policy']);
			natcasesort($files);
			foreach ($files as $value) {
				if ($snortdownload != 'on' && substr($value, 0, 6) == "snort_")
					continue;
				if ($emergingdownload != 'on' && substr($value, 0, 8) == "emerging")
					continue;
				if (empty($value))
					continue;
				echo "<option value='?id={$id}&openruleset={$value}' ";
				if ($value == $currentruleset)
					echo "selected";
				echo ">{$value}</option>\n";
			}
		?>
		</select>
		<br/>
		</td>
		<td class="listhdr" colspan="3" valign="middle">
<?php if ($currentruleset != 'custom.rules'): ?>
			<?php echo "<a href='?id={$id}&openruleset={$currentruleset}&act=resetcategory'>
			<img src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\" 
			onmouseout='this.src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"' 
			onmouseover='this.src=\"../themes/{$g['theme']}/images/icons/icon_x_mo.gif\"' border='0' 
			title='" . gettext("Click to remove enable/disable changes for rules in the selected category only") . "'></a>"?>
			&nbsp;<?php echo gettext("Remove Enable/Disable changes in the current Category");?><br>
			<?php echo "<a href='?id={$id}&openruleset={$currentruleset}&act=resetall'>
			<img src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"  
			onmouseout='this.src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"' 
			onmouseover='this.src=\"../themes/{$g['theme']}/images/icons/icon_x_mo.gif\"' border='0' 
			title='" . gettext("Click to remove all enable/disable changes for rules in all categories") . "'></a>"?>
			&nbsp;<?php echo gettext("Remove all Enable/Disable changes in all Categories");?>
<?php endif;?>
		&nbsp;</td>
		<td width="3%" class="list">&nbsp;</td>
	</tr>
<?php if ($currentruleset == 'custom.rules'): ?>
	<tr>
		<td width="3%" class="list">&nbsp;</td>
		<td colspan="7" valign="top" class="vtable">
			<input type='hidden' name='openruleset' value='custom.rules'>
			<input type='hidden' name='id' value='<?=$id;?>'>
			
			<textarea wrap="on" cols="85" rows="40" name="customrules"><?=$pconfig['customrules'];?></textarea>
		</td>
		<td width="3%" class="list">&nbsp;</td>
	</tr>
	<tr>
		<td width="3%" class="list">&nbsp;</td>
		<td colspan="7" class="vtable">
			<input name="Submit" type="submit" class="formbtn" value="Save">
			<input type="button" class="formbtn" value="Cancel" onclick="history.back()">
		</td>
		<td width="3%" class="list">&nbsp;</td>
	</tr>
<?php else: ?>
	<tr>
		<td width="3%" class="list">&nbsp;</td>
		<td colspan="7" class="listhdr" ><input type="submit" name="apply" id="apply" value="Apply Changes" class="formbtn">
			&nbsp;&nbsp;&nbsp;<?php echo gettext("Click to restart Snort with your changes."); ?>
			<input type='hidden' name='id' value='<?=$id;?>'></td>
		<td width="3%" align="center" valign="middle" class="listt"><a href="javascript: void(0)"
				onclick="popup('snort_rules_edit.php?id=<?=$id;?>&openruleset=<?=$currentruleset;?>')">
				<img src="../themes/<?= $g['theme']; ?>/images/icons/icon_service_restart.gif" <?php
				echo "onmouseover='this.src=\"../themes/{$g['theme']}/images/icons/icon_services_restart_mo.gif\"' 
				onmouseout='this.src=\"../themes/{$g['theme']}/images/icons/icon_service_restart.gif\"' ";?>				
				title="<?php echo gettext("Click to view all rules"); ?>" width="17" height="17" border="0"></a></td>
	</tr>
	<tr id="frheader">
		<td width="3%" class="list">&nbsp;</td>
		<td width="9%" class="listhdrr"><?php echo gettext("SID"); ?></td>
		<td width="2%" class="listhdrr"><?php echo gettext("Proto"); ?></td>
		<td width="14%" class="listhdrr"><?php echo gettext("Source"); ?></td>
		<td width="12%" class="listhdrr"><?php echo gettext("Port"); ?></td>
		<td width="14%" class="listhdrr"><?php echo gettext("Destination"); ?></td>
		<td width="12%" class="listhdrr"><?php echo gettext("Port"); ?></td>
		<td width="31%" class="listhdrr"><?php echo gettext("Message"); ?></td>
		<td width="3%" class="list">&nbsp;</td>
	</tr>
<?php
	foreach (array_keys($rules_map) as $k1) {
		foreach (array_keys($rules_map[$k1]) as $k2) {
			$sid = snort_get_sid($rules_map[$k1][$k2]['rule']);
			$gid = snort_get_gid($rules_map[$k1][$k2]['rule']);
			if (isset($disablesid[$sid])) {
				$textss = "<span class=\"gray\">";
				$textse = "</span>";
				$iconb = "icon_reject_d.gif";
			}
			elseif (($rules_map[$k1][$k2]['disabled'] == 1) && (!isset($enablesid[$sid]))) {
				$textss = "<span class=\"gray\">";
				$textse = "</span>";
				$iconb = "icon_block_d.gif";
			}
			elseif (isset($enablesid[$sid])) {
				$textss = $textse = "";
				$iconb = "icon_reject.gif";
			}
			else {
				$textss = $textse = "";
				$iconb = "icon_block.gif";
			}

			// Pick off the first section of the rule (prior to the start of the MSG field),
			// and then use a REGX split to isolate the remaining fields into an array.
			$tmp = substr($rules_map[$k1][$k2]['rule'], 0, strpos($rules_map[$k1][$k2]['rule'], "("));
			$tmp = trim(preg_replace('/^\s*#+\s*/', '', $tmp));
			$rule_content = preg_split('/[\s]+/', $tmp);

			$protocol = truncate($rule_content[1], 5); //protocol location
			$source = truncate($rule_content[2], 13); //source location
			$source_port = truncate($rule_content[3], 11); //source port location
			$destination = truncate($rule_content[5], 13); //destination location
			$destination_port = truncate($rule_content[6], 11); //destination port location
			$message = snort_get_msg($rules_map[$k1][$k2]['rule']);

			echo "<tr><td width=\"3%\" class=\"listt\" align=\"center\" valign=\"middle\"> $textss
				<a href='?id={$id}&openruleset={$currentruleset}&act=toggle&ids={$sid}'>
				<img src=\"../themes/{$g['theme']}/images/icons/{$iconb}\"
				width=\"10\" height=\"10\" border=\"0\"  
				title='" . gettext("Click to toggle enabled/disabled state") . "'></a>
				$textse
			       </td>
		       <td width=\"9%\" class=\"listlr\">
				$textss $sid $textse
		       </td>
		       <td width=\"2%\" class=\"listlr\">
				$textss $protocol $textse
		       </td>
		       <td width=\"14%\" class=\"listlr\">
				$textss $source $textse
		       </td>
		       <td width=\"12%\" class=\"listlr\">
				$textss $source_port $textse
		       </td>
		       <td width=\"14%\" class=\"listlr\">
				$textss $destination $textse
		       </td>
		       <td width=\"12%\" class=\"listlr\">
			       $textss $destination_port $textse
		       </td>
			<td width=\"31%\" class=\"listbg\" style=\"word-break:break-all;\"><font color=\"white\"> 
				$textss $message $textse
		       </td>";
	?>
			<td width="3%" align="center" valign="middle" nowrap class="listt">
				<a href="javascript: void(0)"
				onclick="popup('snort_rules_edit.php?id=<?=$id;?>&openruleset=<?=$currentruleset;?>&ids=<?=$sid;?>&gid=<?=$gid;?>')"><img
				src="../themes/<?= $g['theme']; ?>/images/icons/icon_right.gif"
				title="<?php echo gettext("Click to view rule"); ?>" width="17" height="17" border="0"></a>
				<!-- Codes by Quackit.com -->
			</td>
		</tr>
<?php
		}
	}
?>
		
	</table>
	</td>
</tr>
<?php endif;?>
<tr>
	<td colspan="9">
<?php if ($currentruleset != 'custom.rules'): ?>
	<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="1">
		<tr>
			<td width="16"><img
				src="../themes/<?= $g['theme']; ?>/images/icons/icon_block.gif"
				width="11" height="11"></td>
			<td><?php echo gettext("Rule default is Enabled"); ?></td>
		</tr>
		<tr>
			<td><img
				src="../themes/<?= $g['theme']; ?>/images/icons/icon_block_d.gif"
				width="11" height="11"></td>
			<td nowrap><?php echo gettext("Rule default is Disabled"); ?></td>
		</tr>
		<tr>
			<td><img
				src="../themes/<?= $g['theme']; ?>/images/icons/icon_reject.gif"
				width="11" height="11"></td>
			<td nowrap><?php echo gettext("Rule changed to Enabled by user"); ?></td>
		</tr>
		<tr>
			<td><img
				src="../themes/<?= $g['theme']; ?>/images/icons/icon_reject_d.gif"
				width="11" height="11"></td>
			<td nowrap><?php echo gettext("Rule changed to Disabled by user"); ?></td>
		</tr>
	</table>
<?php endif;?>
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
