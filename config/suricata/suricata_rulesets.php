<?php
/*
 * suricata_rulesets.php
 *
 * Copyright (C) 2014 Bill Meeks
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
require_once("/usr/local/pkg/suricata/suricata.inc");

global $g, $rebuild_rules;

$suricatadir = SURICATADIR;
$flowbit_rules_file = FLOWBITS_FILENAME;

// Array of default events rules for Suricata
$default_rules = array( "decoder-events.rules", "files.rules", "http-events.rules", 
			"smtp-events.rules", "stream-events.rules", "tls-events.rules" );

if (!is_array($config['installedpackages']['suricata']['rule'])) {
	$config['installedpackages']['suricata']['rule'] = array();
}
$a_nat = &$config['installedpackages']['suricata']['rule'];

if (isset($_POST['id']) && is_numericint($_POST['id']))
	$id = $_POST['id'];
elseif (isset($_GET['id']) && is_numericint($_GET['id']))
	$id = htmlspecialchars($_GET['id']);
if (is_null($id))
	$id = 0;

if (isset($id) && $a_nat[$id]) {
	$pconfig['enable'] = $a_nat[$id]['enable'];
	$pconfig['interface'] = $a_nat[$id]['interface'];
	$pconfig['rulesets'] = $a_nat[$id]['rulesets'];
	$pconfig['autoflowbitrules'] = $a_nat[$id]['autoflowbitrules'];
	$pconfig['ips_policy_enable'] = $a_nat[$id]['ips_policy_enable'];
	$pconfig['ips_policy'] = $a_nat[$id]['ips_policy'];
}

$if_real = get_real_interface($pconfig['interface']);
$suricata_uuid = $a_nat[$id]['uuid'];
$snortdownload = $config['installedpackages']['suricata']['config'][0]['enable_vrt_rules'] == 'on' ? 'on' : 'off';
$emergingdownload = $config['installedpackages']['suricata']['config'][0]['enable_etopen_rules'] == 'on' ? 'on' : 'off';
$etpro = $config['installedpackages']['suricata']['config'][0]['enable_etpro_rules'] == 'on' ? 'on' : 'off';
$snortcommunitydownload = $config['installedpackages']['suricata']['config'][0]['snortcommunityrules'] == 'on' ? 'on' : 'off';

$no_emerging_files = false;
$no_snort_files = false;

/* Test rule categories currently downloaded to $SURICATADIR/rules and set appropriate flags */
if ($emergingdownload == 'on') {
	$test = glob("{$suricatadir}rules/" . ET_OPEN_FILE_PREFIX . "*.rules");
	$et_type = "ET Open";
}
elseif ($etpro == 'on') {
	$test = glob("{$suricatadir}rules/" . ET_PRO_FILE_PREFIX . "*.rules");
	$et_type = "ET Pro";
}
else
	$et_type = "Emerging Threats";
if (empty($test))
	$no_emerging_files = true;
$test = glob("{$suricatadir}rules/" . VRT_FILE_PREFIX . "*.rules");
if (empty($test))
	$no_snort_files = true;
if (!file_exists("{$suricatadir}rules/" . GPL_FILE_PREFIX . "community.rules"))
	$no_community_files = true;

if (($snortdownload != 'on') || ($a_nat[$id]['ips_policy_enable'] != 'on'))
	$policy_select_disable = "disabled";

// If a Snort VRT policy is enabled and selected, remove all Snort VRT
// rules from the configured rule sets to allow automatic selection.
if ($a_nat[$id]['ips_policy_enable'] == 'on') {
	if (isset($a_nat[$id]['ips_policy'])) {
		$disable_vrt_rules = "disabled";
		$enabled_sets = explode("||", $a_nat[$id]['rulesets']);

		foreach ($enabled_sets as $k => $v) {
			if (substr($v, 0, 6) == "suricata_")
				unset($enabled_sets[$k]);
		}
		$a_nat[$id]['rulesets'] = implode("||", $enabled_sets);
	}
}
else
	$disable_vrt_rules = "";

if ($_POST["save"]) {
	if ($_POST['ips_policy_enable'] == "on") {
		$a_nat[$id]['ips_policy_enable'] = 'on';
		$a_nat[$id]['ips_policy'] = $_POST['ips_policy'];
	}
	else {
		$a_nat[$id]['ips_policy_enable'] = 'off';
		unset($a_nat[$id]['ips_policy']);
	}

	// Always start with the default events and files rules
	$enabled_items = implode("||", $default_rules);
	if (is_array($_POST['toenable']))
		$enabled_items .= "||" . implode("||", $_POST['toenable']);
	else
		$enabled_items .=  "||{$_POST['toenable']}";

	$a_nat[$id]['rulesets'] = $enabled_items;

	if ($_POST['autoflowbits'] == "on")
		$a_nat[$id]['autoflowbitrules'] = 'on';
	else {
		$a_nat[$id]['autoflowbitrules'] = 'off';
		if (file_exists("{$suricatadir}suricata_{$suricata_uuid}_{$if_real}/rules/{$flowbit_rules_file}"))
			@unlink("{$suricatadir}suricata_{$suricata_uuid}_{$if_real}/rules/{$flowbit_rules_file}");
	}

	write_config();

	/*************************************************/
	/* Update the suricata.yaml file and rebuild the */
	/* rules for this interface.                     */
	/*************************************************/
	$rebuild_rules = true;
	suricata_generate_yaml($a_nat[$id]);
	$rebuild_rules = false;

	/* Signal Suricata to "live reload" the rules */
	suricata_reload_config($a_nat[$id]);
}
elseif ($_POST['unselectall']) {
	// Remove all but the default events and files rules
	$a_nat[$id]['rulesets'] = implode("||", $default_rules);

	if ($_POST['ips_policy_enable'] == "on") {
		$a_nat[$id]['ips_policy_enable'] = 'on';
		$a_nat[$id]['ips_policy'] = $_POST['ips_policy'];
	}
	else {
		$a_nat[$id]['ips_policy_enable'] = 'off';
		unset($a_nat[$id]['ips_policy']);
	}

	write_config();
	sync_suricata_package_config();
}
elseif ($_POST['selectall']) {
	// Start with the required default events and files rules
	$rulesets = $default_rules;

	if ($_POST['ips_policy_enable'] == "on") {
		$a_nat[$id]['ips_policy_enable'] = 'on';
		$a_nat[$id]['ips_policy'] = $_POST['ips_policy'];
	}
	else {
		$a_nat[$id]['ips_policy_enable'] = 'off';
		unset($a_nat[$id]['ips_policy']);
	}

	if ($emergingdownload == 'on') {
		$files = glob("{$suricatadir}rules/" . ET_OPEN_FILE_PREFIX . "*.rules");
		foreach ($files as $file)
			$rulesets[] = basename($file);
	}
	elseif ($etpro == 'on') {
		$files = glob("{$suricatadir}rules/" . ET_PRO_FILE_PREFIX . "*.rules");
		foreach ($files as $file)
			$rulesets[] = basename($file);
	}

	if ($snortcommunitydownload == 'on') {
		$files = glob("{$suricatadir}rules/" . GPL_FILE_PREFIX . "community.rules");
		foreach ($files as $file)
			$rulesets[] = basename($file);
	}

	/* Include the Snort VRT rules only if enabled and no IPS policy is set */
	if ($snortdownload == 'on' && $a_nat[$id]['ips_policy_enable'] == 'off') {
		$files = glob("{$suricatadir}rules/" . VRT_FILE_PREFIX . "*.rules");
		foreach ($files as $file)
			$rulesets[] = basename($file);
	}

	$a_nat[$id]['rulesets'] = implode("||", $rulesets);

	write_config();
	sync_suricata_package_config();
}

// See if we have any Auto-Flowbit rules and enable
// the VIEW button if we do.
if ($a_nat[$id]['autoflowbitrules'] == 'on') {
	if (file_exists("{$suricatadir}suricata_{$suricata_uuid}_{$if_real}/rules/{$flowbit_rules_file}") &&
	    filesize("{$suricatadir}suricata_{$suricata_uuid}_{$if_real}/rules/{$flowbit_rules_file}") > 0) {
		$btn_view_flowb_rules = " title=\"" . gettext("View flowbit-required rules") . "\"";
	}
	else
		$btn_view_flowb_rules = " disabled";
}
else
	$btn_view_flowb_rules = " disabled";

$enabled_rulesets_array = explode("||", $a_nat[$id]['rulesets']);

$if_friendly = convert_friendly_interface_to_friendly_descr($pconfig['interface']);
$pgtitle = gettext("Suricata IDS: Interface {$if_friendly} - Categories");
include_once("head.inc");
?>

<body link="#000000" vlink="#000000" alink="#000000">

<?php
include("fbegin.inc"); 

/* Display message */
if ($input_errors) {
	print_input_errors($input_errors); // TODO: add checks
}

if ($savemsg) {
	print_info_box($savemsg);
}

?>

<form action="suricata_rulesets.php" method="post" name="iform" id="iform">
<input type="hidden" name="id" id="id" value="<?=$id;?>" />
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php    
	$tab_array = array();
	$tab_array[] = array(gettext("Suricata Interfaces"), true, "/suricata/suricata_interfaces.php");
	$tab_array[] = array(gettext("Global Settings"), false, "/suricata/suricata_global.php");
	$tab_array[] = array(gettext("Update Rules"), false, "/suricata/suricata_download_updates.php");
	$tab_array[] = array(gettext("Alerts"), false, "/suricata/suricata_alerts.php?instance={$id}");
	$tab_array[] = array(gettext("Suppress"), false, "/suricata/suricata_suppress.php");
	$tab_array[] = array(gettext("Logs Browser"), false, "/suricata/suricata_logs_browser.php?instance={$id}");
	$tab_array[] = array(gettext("Logs Mgmt"), false, "/suricata/suricata_logs_mgmt.php");
	display_top_tabs($tab_array);
	echo '</td></tr>';
	echo '<tr><td class="tabnavtbl">';
	$menu_iface=($if_friendly?substr($if_friendly,0,5)." ":"Iface ");
	$tab_array = array();
	$tab_array[] = array($menu_iface . gettext("Settings"), false, "/suricata/suricata_interfaces_edit.php?id={$id}");
	$tab_array[] = array($menu_iface . gettext("Categories"), true, "/suricata/suricata_rulesets.php?id={$id}");
	$tab_array[] = array($menu_iface . gettext("Rules"), false, "/suricata/suricata_rules.php?id={$id}");
        $tab_array[] = array($menu_iface . gettext("Flow/Stream"), false, "/suricata/suricata_flow_stream.php?id={$id}");
	$tab_array[] = array($menu_iface . gettext("App Parsers"), false, "/suricata/suricata_app_parsers.php?id={$id}");
	$tab_array[] = array($menu_iface . gettext("Variables"), false, "/suricata/suricata_define_vars.php?id={$id}");
	$tab_array[] = array($menu_iface . gettext("Barnyard2"), false, "/suricata/suricata_barnyard.php?id={$id}");
	display_top_tabs($tab_array);
?>
</td></tr>
<tr>
	<td>
	<div id="mainarea">
	<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
<?php 
	$isrulesfolderempty = glob("{$suricatadir}rules/*.rules");
	$iscfgdirempty = array();
	if (file_exists("{$suricatadir}suricata_{$suricata_uuid}_{$if_real}/rules/custom.rules"))
		$iscfgdirempty = (array)("{$suricatadir}suricata_{$suricata_uuid}_{$if_real}/rules/custom.rules"); ?>
<?php if (empty($isrulesfolderempty)): ?>
		<tr>
			<td class="vexpl"><br/>
		<?php printf(gettext("# The rules directory is empty:  %s%srules%s"), '<strong>',$suricatadir,'</strong>'); ?> <br/><br/>
		<?php echo gettext("Please go to the ") . '<a href="suricata_download_updates.php"><strong>' . gettext("Updates") . 
			'</strong></a>' . gettext(" tab to download the rules configured on the ") . 
			'<a href="suricata_interfaces_global.php"><strong>' . gettext("Global") . 
			'</strong></a>' . gettext(" tab."); ?>
			</td>
		</tr>
<?php else: ?>
		<tr>
			<td>
			<table width="100%" border="0"
				cellpadding="0" cellspacing="0">
			<tr>
				<td colspan="4" class="listtopic"><?php echo gettext("Automatic flowbit resolution"); ?><br/></td>
			</tr>
			<tr>
				<td colspan="4" valign="center" class="listn">
					<table width="100%" border="0" cellpadding="2" cellspacing="0">
					   <tr>
						<td width="15%" class="listn"><?php echo gettext("Resolve Flowbits"); ?></td>
						<td width="85%"><input name="autoflowbits" id="autoflowbitrules" type="checkbox" value="on" 
						<?php if ($a_nat[$id]['autoflowbitrules'] == "on" || empty($a_nat[$id]['autoflowbitrules'])) echo "checked"; ?>/>
						&nbsp;&nbsp;<span class="vexpl"><?php echo gettext("If checked, Suricata will auto-enable rules required for checked flowbits.  ");
						echo gettext("The Default is "); ?><strong><?php echo gettext("Checked."); ?></strong></span></td>
					   </tr>
					   <tr>
						<td width="15%" class="vncell">&nbsp;</td>
						<td width="85%" class="vtable">
						<?php echo gettext("Suricata will examine the enabled rules in your chosen " .
						"rule categories for checked flowbits.  Any rules that set these dependent flowbits will " .
						"be automatically enabled and added to the list of files in the interface rules directory."); ?><br/></td>
					   </tr>
					   <tr>
						<td width="15%" class="listn"><?php echo gettext("Auto Flowbit Rules"); ?></td>
						<td width="85%"><input type="button" class="formbtns" value="View" onclick="parent.location='suricata_rules_flowbits.php?id=<?=$id;?>'" <?php echo $btn_view_flowb_rules; ?>/>
						&nbsp;&nbsp;<span class="vexpl"><?php echo gettext("Click to view auto-enabled rules required to satisfy flowbit dependencies"); ?></span></td>
					   </tr>
					   <tr>
						<td width="15%">&nbsp;</td>
						<td width="85%">
						<?php echo "<span class=\"red\"><strong>" . gettext("Note:  ") . "</strong></span>" . gettext("Auto-enabled rules generating unwanted alerts should have their GID:SID added to the Suppression List for the interface."); ?>
						<br/></td>
					   </tr>
					</table>
				</td>
			</tr>

		<?php if ($snortdownload == 'on'): ?>
			<tr>
				<td colspan="4" class="listtopic"><?php echo gettext("Snort IPS Policy selection"); ?><br/></td>
			</tr>
			<tr>
				<td colspan="4" valign="center" class="listn">
					<table width="100%" border="0" cellpadding="2" cellspacing="0">
					   <tr>
						<td width="15%" class="listn"><?php echo gettext("Use IPS Policy"); ?></td>
						<td width="85%"><input name="ips_policy_enable" id="ips_policy_enable" type="checkbox" value="on" <?php if ($a_nat[$id]['ips_policy_enable'] == "on") echo "checked"; ?>
						<?php if ($snortdownload != "on") echo "disabled" ?> onClick="enable_change()"/>&nbsp;&nbsp;<span class="vexpl">
						<?php echo gettext("If checked, Suricata will use rules from one of three pre-defined Snort IPS policies."); ?></span></td>
					   </tr>
					   <tr>
						<td width="15%" class="vncell" id="ips_col1">&nbsp;</td>
						<td width="85%" class="vtable" id="ips_col2">
  						<?php echo "<span class=\"red\"><strong>" . gettext("Note:  ") . "</strong></span>" . gettext("You must be using the Snort VRT rules to use this option."); ?>
						<?php echo gettext("Selecting this option disables manual selection of Snort VRT categories in the list below, " .
						"although Emerging Threats categories may still be selected if enabled on the Global Settings tab.  " .
						"These will be added to the pre-defined Snort IPS policy rules from the Snort VRT."); ?><br/></td>
					   </tr>
					   <tr id="ips_row1">
						<td width="15%" class="listn"><?php echo gettext("IPS Policy Selection"); ?></td>
						<td width="85%"><select name="ips_policy" class="formselect" <?=$policy_select_disable?> >
									<option value="connectivity" <?php if ($pconfig['ips_policy'] == "connected") echo "selected"; ?>><?php echo gettext("Connectivity"); ?></option>
									<option value="balanced" <?php if ($pconfig['ips_policy'] == "balanced") echo "selected"; ?>><?php echo gettext("Balanced"); ?></option>
									<option value="security" <?php if ($pconfig['ips_policy'] == "security") echo "selected"; ?>><?php echo gettext("Security"); ?></option>
								</select>
						&nbsp;&nbsp;<span class="vexpl"><?php echo gettext("Snort IPS policies are:  Connectivity, Balanced or Security."); ?></span></td>
					   </tr>
					   <tr id="ips_row2">
						<td width="15%">&nbsp;</td>
						<td width="85%">
						<?php echo gettext("Connectivity blocks most major threats with few or no false positives.  " . 
						"Balanced is a good starter policy.  It is speedy, has good base coverage level, and covers " . 
						"most threats of the day.  It includes all rules in Connectivity." . 
						"Security is a stringent policy.  It contains everything in the first two " .
						"plus policy-type rules such as Flash in an Excel file."); ?><br/></td>
					   </tr>
					</table>
				</td>
			</tr>
		<?php endif; ?>
			<tr>
				<td colspan="4" class="listtopic"><?php echo gettext("Select the rulesets Suricata will load at startup"); ?><br/></td>
			</tr>
			<tr>
				<td colspan="4">
					<table width=90% align="center" border="0" cellpadding="2" cellspacing="0">
						<tr height="45px">
							<td valign="middle"><input value="Select All" class="formbtns" type="submit" name="selectall" id="selectall" title="<?php echo gettext("Add all to enforcing rules"); ?>"/></td>
							<td valign="middle"><input value="Unselect All" class="formbtns" type="submit" name="unselectall" id="unselectall" title="<?php echo gettext("Remove all from enforcing rules"); ?>"/></td>
							<td valign="middle"><input value=" Save " class="formbtns" type="submit" name="save" id="save" title="<?php echo gettext("Save changes to enforcing rules and rebuild"); ?>"/></td>
							<td valign="middle"><span class="vexpl"><?php echo gettext("Click to save changes and auto-resolve flowbit rules (if option is selected above)"); ?></span></td>
						</tr>
					</table>
			</tr>
			<?php if ($no_community_files)
				$msg_community = "NOTE: Snort Community Rules have not been downloaded.  Perform a Rules Update to enable them.";
			      else
				$msg_community = "Snort GPLv2 Community Rules (VRT certified)";
			      $community_rules_file = GPL_FILE_PREFIX . "community.rules";
			?>
			<?php if ($snortcommunitydownload == 'on'): ?>
			<tr id="frheader">
				<td width="5%" class="listhdrr"><?php echo gettext("Enabled"); ?></td>
				<td colspan="5" class="listhdrr"><?php echo gettext('Ruleset: Snort GPLv2 Community Rules');?></td>
			</tr>
			<?php if (in_array($community_rules_file, $enabled_rulesets_array)): ?>
			<tr>
				<td width="5" class="listr" align="center" valign="top">
				<input type="checkbox" name="toenable[]" value="<?=$community_rules_file;?>" checked="checked"/></td>
				<td colspan="5" class="listr"><a href='suricata_rules.php?id=<?=$id;?>&openruleset=<?=$community_rules_file;?>'><?php echo gettext("{$msg_community}"); ?></a></td>
			</tr>
			<?php else: ?>
			<tr>
				<td width="5" class="listr" align="center" valign="top">
				<input type="checkbox" name="toenable[]" value="<?=$community_rules_file;?>" <?php if ($snortcommunitydownload == 'off') echo "disabled"; ?>/></td>
				<td colspan="5" class="listr"><?php echo gettext("{$msg_community}"); ?></td>
			</tr>
			<?php endif; ?>
			<?php endif; ?>

			<?php if ($no_emerging_files && ($emergingdownload == 'on' || $etpro == 'on'))
				  $msg_emerging = "have not been downloaded.";
			      else
				  $msg_emerging = "are not enabled.";
			      if ($no_snort_files && $snortdownload == 'on')
				  $msg_snort = "have not been downloaded.";
			      else
				  $msg_snort = "are not enabled.";
			?>
			<tr id="frheader">
				<?php if ($emergingdownload == 'on' && !$no_emerging_files): ?>
					<td width="5%" class="listhdrr" align="center"><?php echo gettext("Enabled"); ?></td>
					<td width="45%" class="listhdrr"><?php echo gettext('Ruleset: ET Open Rules');?></td>
				<?php elseif ($etpro == 'on' && !$no_emerging_files): ?>
					<td width="5%" class="listhdrr" align="center"><?php echo gettext("Enabled"); ?></td>
					<td width="45%" class="listhdrr"><?php echo gettext('Ruleset: ET Pro Rules');?></td>
				<?php else: ?>
					<td colspan="2" align="center" width="50%" class="listhdrr"><?php echo gettext("{$et_type} rules {$msg_emerging}"); ?></td>
				<?php endif; ?>
				<?php if ($snortdownload == 'on' && !$no_snort_files): ?>
					<td width="5%" class="listhdrr" align="center"><?php echo gettext("Enabled"); ?></td>
					<td width="45%" class="listhdrr"><?php echo gettext('Ruleset: Snort VRT Rules');?></td>
				<?php else: ?>
					<td colspan="2" align="center" width="50%" class="listhdrr"><?php echo gettext("Snort VRT rules {$msg_snort}"); ?></td>
				<?php endif; ?>
				</tr>
			<?php
				$emergingrules = array();
				$snortrules = array();
				if (empty($isrulesfolderempty))
					$dh  = opendir("{$suricatadir}suricata_{$suricata_uuid}_{$if_real}/rules/");
				else
					$dh  = opendir("{$suricatadir}rules/");
				while (false !== ($filename = readdir($dh))) {
					$filename = basename($filename);
					if (substr($filename, -5) != "rules")
						continue;
					if (strstr($filename, ET_OPEN_FILE_PREFIX) && $emergingdownload == 'on')
						$emergingrules[] = $filename;
					else if (strstr($filename, ET_PRO_FILE_PREFIX) && $etpro == 'on')
						$emergingrules[] = $filename;
					else if (strstr($filename, VRT_FILE_PREFIX) && $snortdownload == 'on') {
						$snortrules[] = $filename;
					}
				}
				sort($emergingrules);
				sort($snortrules);
				$i = count($emergingrules);
				if ($i < count($snortrules))
					$i = count($snortrules);

				for ($j = 0; $j < $i; $j++) {
					echo "<tr>\n";
					if (!empty($emergingrules[$j])) {
						$file = $emergingrules[$j];
						echo "<td width='5%' class='listr' align=\"center\" valign=\"top\">";
						if(is_array($enabled_rulesets_array)) {
							if(in_array($file, $enabled_rulesets_array))
								$CHECKED = " checked=\"checked\"";
							else
								$CHECKED = "";
						} else
							$CHECKED = "";
						echo "	\n<input type='checkbox' name='toenable[]' value='$file' {$CHECKED} />\n";
						echo "</td>\n";
						echo "<td class='listr' width='45%' >\n";
						if (empty($CHECKED))
							echo $file;
						else
							echo "<a href='suricata_rules.php?id={$id}&openruleset=" . urlencode($file) . "'>{$file}</a>\n";
						echo "</td>\n";
					} else
						echo "<td class='listbggrey' width='30%' colspan='2'><br/></td>\n";

					if (!empty($snortrules[$j])) {
						$file = $snortrules[$j];
						echo "<td class='listr' width='5%' align=\"center\" valign=\"top\">";
						if(is_array($enabled_rulesets_array)) {
							if (!empty($disable_vrt_rules))
								$CHECKED = $disable_vrt_rules;
							elseif(in_array($file, $enabled_rulesets_array))
								$CHECKED = " checked=\"checked\"";
							else
								$CHECKED = "";
						} else
							$CHECKED = "";
						echo "	\n<input type='checkbox' name='toenable[]' value='{$file}' {$CHECKED} />\n";
						echo "</td>\n";
						echo "<td class='listr' width='45%' >\n";
						if (empty($CHECKED) || $CHECKED == "disabled")
							echo $file;
						else
							echo "<a href='suricata_rules.php?id={$id}&openruleset=" . urlencode($file) . "'>{$file}</a>\n";
						echo "</td>\n";
					} else
						echo "<td class='listbggrey' width='50%' colspan='2'><br/></td>\n";
				echo "</tr>\n";
			}
		?>
	</table>
	</td>
</tr>
<tr>
<td colspan="4" class="vexpl">&nbsp;<br/></td>
</tr>
			<tr>
				<td colspan="4" align="center" valign="middle">
				<input value="Save" type="submit" name="save" id="save" class="formbtn" title=" <?php echo gettext("Click to Save changes and rebuild rules"); ?>"/></td>
			</tr>
<?php endif; ?>
</table>
</div>
</td>
</tr>
</table>
</form>
<?php
include("fend.inc");
?>

<script language="javascript" type="text/javascript">

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

function enable_change()
{
 var endis = !(document.iform.ips_policy_enable.checked);
 document.iform.ips_policy.disabled=endis;

 if (endis) {
	document.getElementById("ips_row1").style.display="none";
	document.getElementById("ips_row2").style.display="none";
	document.getElementById("ips_col1").className="vexpl";
	document.getElementById("ips_col2").className="vexpl";
 }
 else {
	document.getElementById("ips_row1").style.display="table-row";
	document.getElementById("ips_row2").style.display="table-row";
	document.getElementById("ips_col1").className="vncell";
	document.getElementById("ips_col2").className="vtable";
 }
 for (var i = 0; i < document.iform.elements.length; i++) {
    if (document.iform.elements[i].type == 'checkbox') {
       var str = document.iform.elements[i].value;
       if (str.substr(0,6) == "snort_")
          document.iform.elements[i].disabled = !(endis);
    }
 }
}

// Set initial state of dynamic HTML form controls
enable_change();

</script>

</body>
</html>
