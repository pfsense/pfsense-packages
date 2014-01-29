<?php
/*
 * snort_rules.php
 *
 * Copyright (C) 2004, 2005 Scott Ullrich
 * Copyright (C) 2008, 2009 Robert Zelaya
 * Copyright (C) 2011 Ermal Luci
 * Copyright (C) 2013, 2014 Bill Meeks
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

global $g, $rebuild_rules;

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
		$string = substr($string, 0, ($length - 2)) . "...";
	return $string; 
}

function add_title_attribute($tag, $title) {

	/********************************
	 * This function adds a "title" *
	 * attribute to the passed tag  *
	 * and sets the value to the    *
	 * value specified by "$title". *
	 ********************************/
	$result = "";
	if (empty($tag)) {
		// If passed an empty element tag, then
		// just create a <span> tag with title
		$result = "<span title=\"" . $title . "\">";
	}
	else {
		// Find the ending ">" for the element tag
		$pos = strpos($tag, ">");
		if ($pos !== false) {
			// We found the ">" delimter, so add "title"
			// attribute and close the element tag
			$result = substr($tag, 0, $pos) . " title=\"" . $title . "\">";
		}
		else {
			// We did not find the ">" delimiter, so
			// something is wrong, just return the
			// tag "as-is"
			$result = $tag;
		}
	}
	return $result;
}

/* convert fake interfaces to real */
$if_real = snort_get_real_interface($pconfig['interface']);
$snort_uuid = $a_rule[$id]['uuid'];
$snortcfgdir = "{$snortdir}/snort_{$snort_uuid}_{$if_real}";
$snortdownload = $config['installedpackages']['snortglobal']['snortdownload'];
$emergingdownload = $config['installedpackages']['snortglobal']['emergingthreats'];
$etprodownload = $config['installedpackages']['snortglobal']['emergingthreats_pro'];
$categories = explode("||", $pconfig['rulesets']);

// add the standard rules files to the categories list
$categories[] = "custom.rules";
$categories[] = "decoder.rules";
$categories[] = "preprocessor.rules";
$categories[] = "sensitive-data.rules";
if (!empty($a_rule[$id]['ips_policy']))
	$categories[] = "IPS Policy - " . ucfirst($a_rule[$id]['ips_policy']);
if ($a_rule[$id]['autoflowbitrules'] == 'on')
	$categories[] = "Auto-Flowbit Rules";
natcasesort($categories);

if ($_GET['openruleset'])
	$currentruleset = $_GET['openruleset'];
else if ($_POST['openruleset'])
	$currentruleset = $_POST['openruleset'];
else
	$currentruleset = $categories[0];

/* One last sanity check -- if the rules directory is empty, default to loading custom rules */
$tmp = glob("{$snortdir}/rules/*.rules");
if (empty($tmp))
	$currentruleset = "custom.rules";

$ruledir = "{$snortdir}/rules";
$rulefile = "{$ruledir}/{$currentruleset}";
if ($currentruleset != 'custom.rules') {
	// Read the current rules file into our rules map array.
	// If it is the auto-flowbits file, set the full path.
	if ($currentruleset == "Auto-Flowbit Rules")
		$rules_map = snort_load_rules_map("{$snortcfgdir}/rules/" . FLOWBITS_FILENAME);
	// Test for the special case of an IPS Policy file.
	elseif (substr($currentruleset, 0, 10) == "IPS Policy")
		$rules_map = snort_load_vrt_policy($a_rule[$id]['ips_policy']);
	// Test for preproc_rules file and set the full path.
	elseif (file_exists("{$snortdir}/preproc_rules/{$currentruleset}"))
		$rules_map = snort_load_rules_map("{$snortdir}/preproc_rules/{$currentruleset}");
	// Test for existence of regular text rules file and load it.
	elseif (file_exists($rulefile))
		$rules_map = snort_load_rules_map($rulefile);
	else
		$input_errors[] = gettext("{$currentruleset} seems to be missing!!! Please verify rules files have been downloaded, then go to the Categories tab and save the rule set again.");
}

/* Load up our enablesid and disablesid arrays with enabled or disabled SIDs */
$enablesid = snort_load_sid_mods($a_rule[$id]['rule_sid_on']);
$disablesid = snort_load_sid_mods($a_rule[$id]['rule_sid_off']);

if ($_GET['act'] == "toggle" && $_GET['ids'] && !empty($rules_map)) {

	// Get the GID tag embedded in the clicked rule icon.
	$gid = $_GET['gid'];

	// Get the SID tag embedded in the clicked rule icon.
	$sid= $_GET['ids'];

	// See if the target SID is in our list of modified SIDs,
	// and toggle it back to default if present; otherwise,
	// add it to the appropriate modified SID list.
	if (isset($enablesid[$gid][$sid]))
		unset($enablesid[$gid][$sid]);
	elseif (isset($disablesid[$gid][$sid]))
		unset($disablesid[$gid][$sid]);
	else {
		if ($rules_map[$gid][$sid]['disabled'] == 1)
			$enablesid[$gid][$sid] = "enablesid";
		else
			$disablesid[$gid][$sid] = "disablesid";
	}

	// Write the updated enablesid and disablesid values to the config file.
	$tmp = "";
	foreach (array_keys($enablesid) as $k1) {
		foreach (array_keys($enablesid[$k1]) as $k2)
			$tmp .= "{$k1}:{$k2}||";
	}
	$tmp = rtrim($tmp, "||");

	if (!empty($tmp))
		$a_rule[$id]['rule_sid_on'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_on']);

	$tmp = "";
	foreach (array_keys($disablesid) as $k1) {
		foreach (array_keys($disablesid[$k1]) as $k2)
			$tmp .= "{$k1}:{$k2}||";
	}
	$tmp = rtrim($tmp, "||");

	if (!empty($tmp))
		$a_rule[$id]['rule_sid_off'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_off']);

	/* Update the config.xml file. */
	write_config();

	$_GET['openruleset'] = $currentruleset;
	$anchor = "rule_{$gid}_{$sid}";
}

if ($_GET['act'] == "disable_all" && !empty($rules_map)) {

	// Mark all rules in the currently selected category "disabled".
	foreach (array_keys($rules_map) as $k1) {
		foreach (array_keys($rules_map[$k1]) as $k2) {
			if (isset($enablesid[$k1][$k2]))
				unset($enablesid[$k1][$k2]);
			$disablesid[$k1][$k2] = "disablesid";
		}
	}

	// Write the updated enablesid and disablesid values to the config file.
	$tmp = "";
	foreach (array_keys($enablesid) as $k1) {
		foreach (array_keys($enablesid[$k1]) as $k2)
			$tmp .= "{$k1}:{$k2}||";
	}
	$tmp = rtrim($tmp, "||");

	if (!empty($tmp))
		$a_rule[$id]['rule_sid_on'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_on']);

	$tmp = "";
	foreach (array_keys($disablesid) as $k1) {
		foreach (array_keys($disablesid[$k1]) as $k2)
			$tmp .= "{$k1}:{$k2}||";
	}
	$tmp = rtrim($tmp, "||");

	if (!empty($tmp))
		$a_rule[$id]['rule_sid_off'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_off']);

	write_config();

	$_GET['openruleset'] = $currentruleset;
	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
	exit;
}

if ($_GET['act'] == "enable_all" && !empty($rules_map)) {

	// Mark all rules in the currently selected category "enabled".
	foreach (array_keys($rules_map) as $k1) {
		foreach (array_keys($rules_map[$k1]) as $k2) {
			if (isset($disablesid[$k1][$k2]))
				unset($disablesid[$k1][$k2]);
			$enablesid[$k1][$k2] = "enablesid";
		}
	}
	// Write the updated enablesid and disablesid values to the config file.
	$tmp = "";
	foreach (array_keys($enablesid) as $k1) {
		foreach (array_keys($enablesid[$k1]) as $k2)
			$tmp .= "{$k1}:{$k2}||";
	}
	$tmp = rtrim($tmp, "||");

	if (!empty($tmp))
		$a_rule[$id]['rule_sid_on'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_on']);

	$tmp = "";
	foreach (array_keys($disablesid) as $k1) {
		foreach (array_keys($disablesid[$k1]) as $k2)
			$tmp .= "{$k1}:{$k2}||";
	}
	$tmp = rtrim($tmp, "||");

	if (!empty($tmp))
		$a_rule[$id]['rule_sid_off'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_off']);

	write_config();

	$_GET['openruleset'] = $currentruleset;
	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
	exit;
}

if ($_GET['act'] == "resetcategory" && !empty($rules_map)) {

	// Reset any modified SIDs in the current rule category to their defaults.
	foreach (array_keys($rules_map) as $k1) {
		foreach (array_keys($rules_map[$k1]) as $k2) {
			if (isset($enablesid[$k1][$k2]))
				unset($enablesid[$k1][$k2]);
			if (isset($disablesid[$k1][$k2]))
				unset($disablesid[$k1][$k2]);
		}
	}

	// Write the updated enablesid and disablesid values to the config file.
	$tmp = "";
	foreach (array_keys($enablesid) as $k1) {
		foreach (array_keys($enablesid[$k1]) as $k2)
			$tmp .= "{$k1}:{$k2}||";
	}
	$tmp = rtrim($tmp, "||");

	if (!empty($tmp))
		$a_rule[$id]['rule_sid_on'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_on']);

	$tmp = "";
	foreach (array_keys($disablesid) as $k1) {
		foreach (array_keys($disablesid[$k1]) as $k2)
			$tmp .= "{$k1}:{$k2}||";
	}
	$tmp = rtrim($tmp, "||");

	if (!empty($tmp))
		$a_rule[$id]['rule_sid_off'] = $tmp;
	else				
		unset($a_rule[$id]['rule_sid_off']);

	write_config();

	$_GET['openruleset'] = $currentruleset;
	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
	exit;
}

if ($_GET['act'] == "resetall" && !empty($rules_map)) {

	// Remove all modified SIDs from config.xml and save the changes.
	unset($a_rule[$id]['rule_sid_on']);
	unset($a_rule[$id]['rule_sid_off']);

	/* Update the config.xml file. */
	write_config();

	$_GET['openruleset'] = $currentruleset;
	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
	exit;
}

if ($_POST['clear']) {
	unset($a_rule[$id]['customrules']);
	write_config();
	$rebuild_rules = true;
	snort_generate_conf($a_rule[$id]);
	$rebuild_rules = false;
	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
	exit;
}

if ($_POST['customrules']) {
	$a_rule[$id]['customrules'] = base64_encode($_POST['customrules']);
	write_config();
	$rebuild_rules = true;
	snort_generate_conf($a_rule[$id]);
	$rebuild_rules = false;
	$output = "";
	$retcode = "";
	exec("/usr/local/bin/snort -T -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/snort.conf 2>&1", $output, $retcode);
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
	/* Save new configuration */
	write_config();

	/*************************************************/
	/* Update the snort conf file and rebuild the    */
	/* rules for this interface.                     */
	/*************************************************/
	$rebuild_rules = true;
	snort_generate_conf($a_rule[$id]);
	$rebuild_rules = false;

	/* Soft-restart Snort to live-load new rules */
	snort_reload_config($a_rule[$id]);

	/* Return to this same page */
	header("Location: /snort/snort_rules.php?id={$id}&openruleset={$currentruleset}");
	exit;
}
else if ($_POST['cancel']) {

	/* Return to this same page */
	header("Location: /snort/snort_rules.php?id={$id}");
	exit;
}

require_once("guiconfig.inc");
include_once("head.inc");

$if_friendly = snort_get_friendly_interface($pconfig['interface']);
$pgtitle = gettext("Snort: Interface {$if_friendly} - Rules: {$currentruleset}");
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

<form action="/snort/snort_rules.php" method="post" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr><td>
	<?php
		$tab_array = array();
		$tab_array[0] = array(gettext("Snort Interfaces"), true, "/snort/snort_interfaces.php");
		$tab_array[1] = array(gettext("Global Settings"), false, "/snort/snort_interfaces_global.php");
		$tab_array[2] = array(gettext("Updates"), false, "/snort/snort_download_updates.php");
		$tab_array[3] = array(gettext("Alerts"), false, "/snort/snort_alerts.php");
		$tab_array[4] = array(gettext("Blocked"), false, "/snort/snort_blocked.php");
		$tab_array[5] = array(gettext("Whitelists"), false, "/snort/snort_interfaces_whitelist.php");
		$tab_array[6] = array(gettext("Suppress"), false, "/snort/snort_interfaces_suppress.php");
		$tab_array[7] = array(gettext("Sync"), false, "/pkg_edit.php?xml=snort/snort_sync.xml");
		display_top_tabs($tab_array);
		echo '</td></tr>';
		echo '<tr><td class="tabnavtbl">';
		$menu_iface=($if_friendly?substr($if_friendly,0,5)." ":"Iface ");
		$tab_array = array();
		$tab_array[] = array($menu_iface . gettext("Settings"), false, "/snort/snort_interfaces_edit.php?id={$id}");
		$tab_array[] = array($menu_iface . gettext("Categories"), false, "/snort/snort_rulesets.php?id={$id}");
		$tab_array[] = array($menu_iface . gettext("Rules"), true, "/snort/snort_rules.php?id={$id}");
		$tab_array[] = array($menu_iface . gettext("Variables"), false, "/snort/snort_define_servers.php?id={$id}");
		$tab_array[] = array($menu_iface . gettext("Preprocessors"), false, "/snort/snort_preprocessors.php?id={$id}");
		$tab_array[] = array($menu_iface . gettext("Barnyard2"), false, "/snort/snort_barnyard.php?id={$id}");
		display_top_tabs($tab_array);
	?>
	</td></tr>
	<tr><td><div id="mainarea">
		<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="4" cellspacing="0">
			<tr>
				<td class="listtopic"><?php echo gettext("Available Rule Categories"); ?></td>
			</tr>

			<tr>
				<td class="vncell" height="30px"><strong><?php echo gettext("Category:"); ?></strong>&nbsp;&nbsp;<select id="selectbox" name="selectbox" class="formselect" onChange="go()">
					<?php
					foreach ($categories as $value) {
						if ($snortdownload != 'on' && substr($value, 0, 6) == "snort_")
							continue;
						if ($emergingdownload != 'on' && substr($value, 0, 8) == "emerging")
							continue;
						if ($etprodownload != 'on' && substr($value, 0, 6) == "etpro-")
							continue;
						if (empty($value))
							continue;
						echo "<option value='?id={$id}&openruleset={$value}' ";
						if ($value == $currentruleset)
							echo "selected";
						echo ">{$value}</option>\n";
						}
					?>
					</select>&nbsp;&nbsp;&nbsp;<?php echo gettext("Select the rule category to view"); ?>
				</td>
			</tr>

		<?php if ($currentruleset == 'custom.rules'): ?>
			<tr>
				<td class="listtopic"><?php echo gettext("Defined Custom Rules"); ?></td>
			</tr>
			<tr>
				<td valign="top" class="vtable">
					<input type='hidden' name='openruleset' value='custom.rules'>
					<input type='hidden' name='id' value='<?=$id;?>'>
					<textarea wrap="soft" cols="90" rows="40" name="customrules"><?=$pconfig['customrules'];?></textarea>
				</td>
			</tr>
			<tr>
				<td>
					<input name="Submit" type="submit" class="formbtn" id="submit" value="<?php echo gettext(" Save "); ?>" title=" <?php echo gettext("Save custom rules"); ?>"/>&nbsp;&nbsp;
					<input name="cancel" type="submit" class="formbtn" id="cancel" value="<?php echo gettext("Cancel"); ?>" title="<?php echo gettext("Cancel changes and return to last page"); ?>"/>&nbsp;&nbsp;
					<input name="clear" type="submit" class="formbtn" id="clear" value="<?php echo gettext("Clear"); ?>" onclick="return confirm('<?php echo gettext("This will erase all custom rules for the interface.  Are you sure?"); ?>')" title="<?php echo gettext("Deletes all custom rules"); ?>"/>
				</td>
			</tr>
		<?php else: ?>
			<tr>
				<td class="listtopic"><?php echo gettext("Rule Signature ID (SID) Enable/Disable Overrides"); ?></td>
			</tr>
			<tr>
				<td class="vncell">
					<table width="100%" align="center" border="0" cellpadding="0" cellspacing="0">
						<tr>
							<td rowspan="4" width="48%" valign="middle"><input type="submit" name="apply" id="apply" value="<?php echo gettext("Apply"); ?>" class="formbtn" 
							title="<?php echo gettext("Click to rebuild the rules with your changes"); ?>"/>
							<input type='hidden' name='id' value='<?=$id;?>'/>
							<input type='hidden' name='openruleset' value='<?=$currentruleset;?>'/><br/><br/>
							<span class="vexpl"><span class="red"><strong><?php echo gettext("Note: ") . "</strong></span>" . 
							gettext("Snort must be restarted to activate any rule enable/disable changes made on this tab."); ?></span></td>
							<td class="vexpl" valign="middle"><?php echo "<a href='?id={$id}&openruleset={$currentruleset}&act=resetcategory'>
							<img src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\" width=\"15\" height=\"15\" 
							onmouseout='this.src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"' 
							onmouseover='this.src=\"../themes/{$g['theme']}/images/icons/icon_x_mo.gif\"' border='0' 
							title='" . gettext("Click to remove enable/disable changes for rules in the selected category only") . "'></a>"?>
							&nbsp;&nbsp;<?php echo gettext("Remove Enable/Disable changes in the current Category"); ?></td>
						</tr>
						<tr>
							<td class="vexpl" valign="middle"><?php echo "<a href='?id={$id}&openruleset={$currentruleset}&act=resetall'> 
							<img src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\" width=\"15\" height=\"15\" 
							onmouseout='this.src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"' 
							onmouseover='this.src=\"../themes/{$g['theme']}/images/icons/icon_x_mo.gif\"' border='0' 
							title='" . gettext("Click to remove all enable/disable changes for rules in all categories") . "'></a>"?>
							&nbsp;&nbsp;<?php echo gettext("Remove all Enable/Disable changes in all Categories"); ?></td>
						</tr>
						<tr>
							<td class="vexpl" valign="middle"><?php echo "<a href='?id={$id}&openruleset={$currentruleset}&act=disable_all'> 
							<img src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\" width=\"15\" height=\"15\" 
							onmouseout='this.src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"' 
							onmouseover='this.src=\"../themes/{$g['theme']}/images/icons/icon_x_mo.gif\"' border='0' 
							title='" . gettext("Click to disable all rules in the selected category") . "'></a>"?>
							&nbsp;&nbsp;<?php echo gettext("Disable all rules in the current Category"); ?></td>
						</tr>
						<tr>
							<td class="vexpl" valign="middle"><?php echo "<a href='?id={$id}&openruleset={$currentruleset}&act=enable_all'> 
							<img src=\"../themes/{$g['theme']}/images/icons/icon_plus.gif\" width=\"15\" height=\"15\" 
							onmouseout='this.src=\"../themes/{$g['theme']}/images/icons/icon_plus.gif\"' 
							onmouseover='this.src=\"../themes/{$g['theme']}/images/icons/icon_plus_mo.gif\"' border='0' 
							title='" . gettext("Click to enable all rules in the selected category") . "'></a>"?>
							&nbsp;&nbsp;<?php echo gettext("Enable all rules in the current Category"); ?></td>
						</tr>
						<?php if ($currentruleset == 'Auto-Flowbit Rules'): ?>
						<tr>
							<td colspan="3">&nbsp;</td>
						</tr>
						<tr>
							<td colspan="3" class="vexpl" align="center"><?php echo "<span class=\"red\"><b>" . gettext("WARNING: ") . "</b></span>" . 
							gettext("You should not disable flowbit rules!  Add Suppress List entries for them instead by ") . 
							"<a href='snort_rules_flowbits.php?id={$id}&openruleset={$currentruleset}&returl=" . urlencode($_SERVER['PHP_SELF']) . "' title=\"" . gettext("Add Suppress List entry for Flowbit Rule") . "\">" . 
							gettext("clicking here") . ".</a>";?></td>
						</tr>
						<?php endif;?>
					</table>
				</td>
			</tr>

			<tr>
				<td class="listtopic"><?php echo gettext("Selected Category's Rules"); ?></td>
			</tr>
			<tr>
				<td>

				<?php if ($currentruleset != 'decoder.rules' && $currentruleset != 'preprocessor.rules'): ?>
					<table id="myTable" class="sortable" style="table-layout: fixed;" width="100%" border="0" cellpadding="0" cellspacing="0">
						<colgroup>
							<col width="15" align="left" valign="middle">
							<col width="6%" align="center" axis="number">
							<col width="8%" align="center" axis="number">
							<col width="54" align="center" axis="string">
							<col width="14%" align="center" axis="string">
							<col width="11%" align="center" axis="string">
							<col width="14%" align="center" axis="string">
							<col width="11%" align="center" axis="string">
							<col axis="string">
							<col width="22" align="right" valign="middle">
						</colgroup>
						<thead>
						   <tr>
							<th class="list">&nbsp;</th>
							<th class="listhdrr"><?php echo gettext("GID"); ?></th>
							<th class="listhdrr"><?php echo gettext("SID"); ?></th>
							<th class="listhdrr"><?php echo gettext("Proto"); ?></th>
							<th class="listhdrr"><?php echo gettext("Source"); ?></th>
							<th class="listhdrr"><?php echo gettext("Port"); ?></th>
							<th class="listhdrr"><?php echo gettext("Destination"); ?></th>
							<th class="listhdrr"><?php echo gettext("Port"); ?></th>
							<th class="listhdrr"><?php echo gettext("Message"); ?></th>
							<th class="list"><a href="javascript: void(0)" 
							onclick="wopen('snort_rules_edit.php?id=<?=$id;?>&openruleset=<?=$currentruleset;?>','FileViewer',800,600)">
							<img src="../themes/<?= $g['theme']; ?>/images/icons/icon_service_restart.gif" <?php
							echo "onmouseover='this.src=\"../themes/{$g['theme']}/images/icons/icon_services_restart_mo.gif\"' 
							onmouseout='this.src=\"../themes/{$g['theme']}/images/icons/icon_service_restart.gif\"' ";?>				
							title="<?php echo gettext("Click to view full text of all the category rules"); ?>" width="17" height="17" border="0"></a></th>
						   </tr>
						</thead>
						<tbody>

					<?php
						$counter = $enable_cnt = $disable_cnt = 0;
						foreach ($rules_map as $k1 => $rulem) {
							foreach ($rulem as $k2 => $v) {
								$sid = $k2;
								$gid = $k1;

								if (isset($disablesid[$gid][$sid])) {
									$textss = "<span class=\"gray\">";
									$textse = "</span>";
									$iconb = "icon_reject_d.gif";
									$disable_cnt++;
									$title = gettext("Disabled by user. Click to toggle to default state");
								}
								elseif (($v['disabled'] == 1) && (!isset($enablesid[$gid][$sid]))) {
									$textss = "<span class=\"gray\">";
									$textse = "</span>";
									$iconb = "icon_block_d.gif";
									$disable_cnt++;
									$title = gettext("Disabled by default. Click to toggle to enabled state");
								}
								elseif (isset($enablesid[$gid][$sid])) {
									$textss = $textse = "";
									$iconb = "icon_reject.gif";
									$enable_cnt++;
									$title = gettext("Enabled by user. Click to toggle to default state");
								}
								else {
									$textss = $textse = "";
									$iconb = "icon_block.gif";
									$enable_cnt++;
									$title = gettext("Enabled by default. Click to toggle to disabled state");
								}

								// Pick off the first section of the rule (prior to the start of the MSG field),
								// and then use a REGX split to isolate the remaining fields into an array.
								$tmp = substr($v['rule'], 0, strpos($v['rule'], "("));
								$tmp = trim(preg_replace('/^\s*#+\s*/', '', $tmp));
								$rule_content = preg_split('/[\s]+/', $tmp);

								// Create custom <span> tags for the fields we truncate so we can 
								// have a "title" attribute for tooltips to show the full string.
								$srcspan = add_title_attribute($textss, $rule_content[2]);
								$srcprtspan = add_title_attribute($textss, $rule_content[3]);
								$dstspan = add_title_attribute($textss, $rule_content[5]);
								$dstprtspan = add_title_attribute($textss, $rule_content[6]);

								$protocol = $rule_content[1]; //protocol field
								$source = truncate($rule_content[2], 14); //source field
								$source_port = truncate($rule_content[3], 10); //source port field
								$destination = truncate($rule_content[5], 14); //destination field
								$destination_port = truncate($rule_content[6], 10); //destination port field
								$message = snort_get_msg($v['rule']);

						echo "<tr><td class=\"listt\" align=\"left\" valign=\"middle\"> $textss
								<a id=\"rule_{$gid}_{$sid}\" href='?id={$id}&openruleset={$currentruleset}&act=toggle&gid={$gid}&ids={$sid}'>
								<img src=\"../themes/{$g['theme']}/images/icons/{$iconb}\"
								width=\"11\" height=\"11\" border=\"0\"  
								title='{$title}'></a>
								$textse
							       </td>
							       <td class=\"listlr\" align=\"center\">
									{$textss}{$gid}{$textse}
							       </td>
							       <td class=\"listlr\" align=\"center\">
									{$textss}{$sid}{$textse}
							       </td>
							       <td class=\"listlr\" align=\"center\">
									{$textss}{$protocol}{$textse}
							       </td>
							       <td class=\"listlr\" align=\"center\">
									{$srcspan}{$source}</span>
							       </td>
							       <td class=\"listlr\" align=\"center\">
									{$srcprtspan}{$source_port}</span>
							       </td>
							       <td class=\"listlr\" align=\"center\">
									{$dstspan}{$destination}</span>
							       </td>
							       <td class=\"listlr\" align=\"center\">
								       {$dstprtspan}{$destination_port}</span>
							       </td>
								<td class=\"listbg\" style=\"word-wrap:break-word; whitespace:pre-line;\"><font color=\"white\"> 
									{$textss}{$message}{$textse}</font>
							       </td>";
						?>
								<td align="right" valign="middle" nowrap class="listt">
									<a href="javascript: void(0)" 
									onclick="wopen('snort_rules_edit.php?id=<?=$id;?>&openruleset=<?=$currentruleset;?>&ids=<?=$sid;?>&gid=<?=$gid;?>','FileViewer',800,600)">
									<img src="../themes/<?= $g['theme']; ?>/images/icons/icon_right.gif" 
									title="<?php echo gettext("Click to view the entire rule text"); ?>" width="17" height="17" border="0"></a>
								</td>
							</tr>
						<?php
								$counter++;
							}
						}
						unset($rulem, $v);
						?>
					    </tbody>
					</table>

				<?php else: ?>

					<table id="myTable" class="sortable" style="table-layout: fixed;" width="100%" border="0" cellpadding="0" cellspacing="0">
						<colgroup>
							<col width="15" align="left" valign="middle">
							<col width="6%" align="center" axis="number">
							<col width="6%" align="center" axis="number">
							<col width="22%" align="center" axis="string">
							<col width="15%" align="center" axis="string">
							<col align="left" axis="string">
							<col width="22" align="right" valign="middle">
						</colgroup>
						<thead>
						   <tr>
							<th class="list">&nbsp;</th>
							<th class="listhdrr"><?php echo gettext("GID"); ?></th>
							<th class="listhdrr"><?php echo gettext("SID"); ?></th>
							<th class="listhdrr"><?php echo gettext("Classification"); ?></th>
							<th class="listhdrr"><?php echo gettext("IPS Policy"); ?></th>
							<th class="listhdrr"><?php echo gettext("Message"); ?></th>
							<th class="list"><a href="javascript: void(0)" 
							onclick="wopen('snort_rules_edit.php?id=<?=$id;?>&openruleset=<?=$currentruleset;?>','FileViewer',800,600)">
							<img src="../themes/<?= $g['theme']; ?>/images/icons/icon_service_restart.gif" <?php
							echo "onmouseover='this.src=\"../themes/{$g['theme']}/images/icons/icon_services_restart_mo.gif\"' 
							onmouseout='this.src=\"../themes/{$g['theme']}/images/icons/icon_service_restart.gif\"' ";?>				
							title="<?php echo gettext("Click to view full text of all the category rules"); ?>" width="17" height="17" border="0"></a></th>
						   </tr>
						</thead>
						<tbody>
							<?php
								$counter = $enable_cnt = $disable_cnt = 0;
								foreach ($rules_map as $k1 => $rulem) {
									foreach ($rulem as $k2 => $v) {
										$sid = snort_get_sid($v['rule']);
										$gid = snort_get_gid($v['rule']);
										if (isset($disablesid[$gid][$sid])) {
											$textss = "<span class=\"gray\">";
											$textse = "</span>";
											$iconb = "icon_reject_d.gif";
											$disable_cnt++;
											$title = gettext("Disabled by user. Click to toggle to default state");
										}
										elseif (($v['disabled'] == 1) && (!isset($enablesid[$gid][$sid]))) {
											$textss = "<span class=\"gray\">";
											$textse = "</span>";
											$iconb = "icon_block_d.gif";
											$disable_cnt++;
											$title = gettext("Disabled by default. Click to toggle to enabled state");
										}
										elseif (isset($enablesid[$gid][$sid])) {
											$textss = $textse = "";
											$iconb = "icon_reject.gif";
											$enable_cnt++;
											$title = gettext("Enabled by user. Click to toggle to default state");
										}
										else {
											$textss = $textse = "";
											$iconb = "icon_block.gif";
											$enable_cnt++;
											$title = gettext("Enabled by default. Click to toggle to disabled state");
										}
										$message = snort_get_msg($v['rule']);
										$matches = array();
										if (preg_match('/(?:classtype\b\s*:)\s*(\S*\s*;)/iU', $v['rule'], $matches))
											$classtype = trim($matches[1], " ;");
										else
											$classtype = "No Classtype Defined";
										$matches = array();
										if (preg_match_all('/(\S*-ips)(?:\s*drop|alert)(?:,|\s*|;)/i', $v['rule'], $matches))
											$policy = implode("<br/>", $matches[1]);
										else
											$policy = "none";

										echo "<tr><td class=\"listt\" align=\"left\" valign=\"middle\"> $textss
										<a id=\"rule_{$sid}\" href='?id={$id}&openruleset={$currentruleset}&act=toggle&ids={$sid}&gid={$gid}'>
										<img src=\"../themes/{$g['theme']}/images/icons/{$iconb}\"
										width=\"11\" height=\"11\" border=\"0\"  
										title='{$title}'></a>
										$textse
								       		</td>
									       <td class=\"listlr\" align=\"center\">
											{$textss}{$gid}{$textse}
									       </td>
									       <td class=\"listlr\" align=\"center\">
											{$textss}{$sid}{$textse}
										</td>
										<td class=\"listlr\" align=\"center\">
											{$textss}{$classtype}</span>
							       			</td>
							       			<td class=\"listlr\" align=\"center\">
								       			{$textss}{$policy}</span>
								       		</td>
										<td class=\"listbg\" style=\"word-wrap:break-word; whitespace:pre-line;\"><font color=\"white\"> 
											{$textss}{$message}{$textse}</font>
							       			</td>";
									?>
										<td align="right" valign="middle" nowrap class="listt">
											<a href="javascript: void(0)" 
											onclick="wopen('snort_rules_edit.php?id=<?=$id;?>&openruleset=<?=$currentruleset;?>&ids=<?=$sid;?>&gid=<?=$gid;?>','FileViewer',800,600)">
											<img src="../themes/<?= $g['theme']; ?>/images/icons/icon_right.gif" 
											title="<?php echo gettext("Click to view the entire rule text"); ?>" width="17" height="17" border="0"></a>
										</td>
									   	</tr>
							<?php
										$counter++;
									}
								}
							unset($rulem, $v);
							?>
						</tbody>
					</table>
				<?php endif;?>
				</td>
			</tr>
			<tr>
				<td>
					<table width="100%" border="0" cellspacing="0" cellpadding="1">
						<tr>
							<td width="16"></td>
							<td class="vexpl" height="35" valign="top">
							<strong><?php echo gettext("---  Category Rules Summary  ---") . "</strong><br/>" . 
							gettext("Total Rules: {$counter}") . "&nbsp;&nbsp;&nbsp;&nbsp;" . 
							gettext("Enabled: {$enable_cnt}") . "&nbsp;&nbsp;&nbsp;&nbsp;" . 
							gettext("Disabled: {$disable_cnt}"); ?></td>
						</tr>
						<tr>
							<td width="16"><img src="../themes/<?= $g['theme']; ?>/images/icons/icon_block.gif"
								width="11" height="11"></td>
							<td><?php echo gettext("Rule default is Enabled"); ?></td>
						</tr>
						<tr>
							<td width="16"><img src="../themes/<?= $g['theme']; ?>/images/icons/icon_block_d.gif"
								width="11" height="11"></td>
							<td nowrap><?php echo gettext("Rule default is Disabled"); ?></td>
						</tr>
						<tr>
							<td width="16"><img src="../themes/<?= $g['theme']; ?>/images/icons/icon_reject.gif"
								width="11" height="11"></td>
							<td nowrap><?php echo gettext("Rule changed to Enabled by user"); ?></td>
						</tr>
						<tr>
							<td width="16"><img src="../themes/<?= $g['theme']; ?>/images/icons/icon_reject_d.gif"
								width="11" height="11"></td>
							<td nowrap><?php echo gettext("Rule changed to Disabled by user"); ?></td>
						</tr>
					</table>
				</td>
			</tr>
		<?php endif;?>
		</table>
	</div>
	</td>
	</tr>
</table>
</form>
<?php include("fend.inc"); ?>

<script language="javascript" type="text/javascript">
function go()
{
    var box = document.iform.selectbox;
    destination = box.options[box.selectedIndex].value;
    if (destination) 
		location.href = destination;
}

function wopen(url, name, w, h)
{
// Fudge factors for window decoration space.
// In my tests these work well on all platforms & browsers.
    w += 32;
    h += 96;
    var win = window.open(url,
        name, 
       'width=' + w + ', height=' + h + ', ' +
       'location=no, menubar=no, ' +
       'status=no, toolbar=no, scrollbars=yes, resizable=yes');
    win.resizeTo(w, h);
    win.focus();
}

<?php if (!empty($anchor)): ?>
    // Scroll the last enabled/disabled SID into view
    window.location.hash = "<?=$anchor; ?>";
    window.scrollBy(0,-60); 

<?php endif;?>

</script>
</body>
</html>
