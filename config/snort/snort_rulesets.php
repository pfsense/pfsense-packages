<?php
/*
 * snort_rulesets.php
 *
 * Copyright (C) 2006 Scott Ullrich
 * Copyright (C) 2009 Robert Zelaya
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

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (is_null($id)) {
	header("Location: /snort/snort_interfaces.php");
	exit;
}

function snort_remove_rules($files, $snortdir, $snort_uuid, $if_real) {

        if (empty($files))
                return;

        conf_mount_rw();
	foreach ($files as $file) {
		@unlink("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$file}");
		if (substr($file, -9) == ".so.rules") {
			$slib = substr($file, 6, -6);
			@unlink("{$snortdir}/snort_{$snort_uuid}_{$if_real}/dynamicrules/{$slib}");
		}
	}
        conf_mount_ro();
}

function snort_copy_rules($files, $snortdir, $snort_uuid, $if_real) {

        if (empty($files))
                return;

        conf_mount_rw();
        foreach ($files as $file) {
                if (!file_exists("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$file}"))
                        @copy("{$snortdir}/rules/{$file}", "{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$file}");
		if (substr($file, -9) == ".so.rules") {
			$slib = substr($enabled_item, 6, -6);
			if (!file_exists("{$snortdir}/snort_{$snort_uuid}_{$if_real}/dynamicrules/{$slib}"))
				@copy("/usr/local/lib/snort/dynamicrules/{$file}", "{$snortdir}/snort_{$snort_uuid}_{$if_real}/dynamicrules/{$slib}");
		}
        }
        conf_mount_ro();
}

if (isset($id) && $a_nat[$id]) {
	$pconfig['enable'] = $a_nat[$id]['enable'];
	$pconfig['interface'] = $a_nat[$id]['interface'];
	$pconfig['rulesets'] = $a_nat[$id]['rulesets'];
	$pconfig['autoflowbitrules'] = $a_nat[$id]['autoflowbitrules'];
	$pconfig['ips_policy_enable'] = $a_nat[$id]['ips_policy_enable'];
	$pconfig['ips_policy'] = $a_nat[$id]['ips_policy'];
}

$if_real = snort_get_real_interface($pconfig['interface']);
$snort_uuid = $a_nat[$id]['uuid'];
$snortdownload = $config['installedpackages']['snortglobal']['snortdownload'];
$emergingdownload = $config['installedpackages']['snortglobal']['emergingthreats'];

if (($snortdownload == 'off') || ($a_nat[$id]['ips_policy_enable'] != 'on'))
	$policy_select_disable = "disabled";

if ($a_nat[$id]['autoflowbitrules'] == 'on') {
	if (file_exists("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$flowbit_rules_file}"))
		$btn_view_flowb_rules = "";
	else
		$btn_view_flowb_rules = " disabled";
}
else
	$btn_view_flowb_rules = " disabled";

/* alert file */
if ($_POST["Submit"]) {
	$enabled_items = "";
	if (is_array($_POST['toenable']))
		$enabled_items = implode("||", $_POST['toenable']);
	else
		$enabled_items = $_POST['toenable'];

	$oenabled = explode("||", $a_nat[$id]['rulesets']);
	$nenabled = explode("||", $enabled_items);
	$tormv = array_diff($oenabled, $nenabled);
	snort_remove_rules($tormv, $snortdir, $snort_uuid, $if_real);
	$a_nat[$id]['rulesets'] = $enabled_items;

	/* Copy the new enabled rule category files to the rules directory. */
	snort_copy_rules(explode("||", $enabled_items), $snortdir, $snort_uuid, $if_real);

	/* Load our rules map in preparation for writing the enforcing rules file. */
	$enabled_rules = snort_load_rules_map("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/");

	if ($_POST['autoflowbits'] == "on") {
		$a_nat[$id]['autoflowbitrules'] = 'on';
		snort_write_flowbit_rules_file(snort_resolve_flowbits("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/"), "{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$flowbit_rules_file}");
	}
	else {
		$a_nat[$id]['autoflowbitrules'] = 'off';
		if (file_exists("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$flowbit_rules_file}"))
			@unlink("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$flowbit_rules_file}");
	}

	if ($_POST['ips_policy_enable'] == "on")
		$a_nat[$id]['ips_policy_enable'] = 'on';
	else
		$a_nat[$id]['ips_policy_enable'] = 'off';

	$a_nat[$id]['ips_policy'] = $_POST['ips_policy'];

	write_config();
	sync_snort_package_config();

	header("Location: /snort/snort_rulesets.php?id=$id");
	exit;
}

if ($_POST['unselectall']) {
	if (!empty($pconfig['rulesets']))
		snort_remove_rules(explode("||", $pconfig['rulesets']), $snortdir, $snort_uuid, $if_real);

	$a_nat[$id]['rulesets'] = "";

	write_config();
	sync_snort_package_config();

	header("Location: /snort/snort_rulesets.php?id=$id");
	exit;
}

if ($_POST['selectall']) {
	$rulesets = array();
	if ($emergingdownload == 'on') {
		$files = glob("{$snortdir}/rules/emerging*.rules");
		foreach ($files as $file)
			$rulesets[] = basename($file);
	}
	if ($snortdownload == 'on') {
		$files = glob("{$snortdir}/rules/snort*.rules");
		foreach ($files as $file)
			$rulesets[] = basename($file);
	}
	snort_copy_rules($rulesets, $snortdir, $snort_uuid, $if_real);

	$a_nat[$id]['rulesets'] = implode("||", $rulesets);

	write_config();
	sync_snort_package_config();

	header("Location: /snort/snort_rulesets.php?id=$id");
	exit;
}

$enabled_rulesets_array = explode("||", $a_nat[$id]['rulesets']);
include_once("head.inc");
?>

<body link="#000000" vlink="#000000" alink="#000000">

<?php
include("fbegin.inc"); 
$if_friendly = snort_get_friendly_interface($pconfig['interface']);
$pgtitle = "Snort: Interface {$if_friendly} Categories";

if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>

<?php
/* Display message */
if ($input_errors) {
	print_input_errors($input_errors); // TODO: add checks
}

if ($savemsg) {
	print_info_box($savemsg);
}

?>

<script language="javascript" type="text/javascript">
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

function enable_change()
{
 var endis = !(document.iform.ips_policy_enable.checked);
 document.iform.ips_policy.disabled=endis;
}
</script>

<form action="snort_rulesets.php" method="post" name="iform" id="iform">
<input type="hidden" name="id" id="id" value="<?=$id;?>" />
<table width="99%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[] = array(gettext("If Settings"), false, "/snort/snort_interfaces_edit.php?id={$id}");
        $tab_array[] = array(gettext("Categories"), true, "/snort/snort_rulesets.php?id={$id}");
        $tab_array[] = array(gettext("Rules"), false, "/snort/snort_rules.php?id={$id}");
        $tab_array[] = array(gettext("Variables"), false, "/snort/snort_define_servers.php?id={$id}");
        $tab_array[] = array(gettext("Preprocessors"), false, "/snort/snort_preprocessors.php?id={$id}");
        $tab_array[] = array(gettext("Barnyard2"), false, "/snort/snort_barnyard.php?id={$id}");
        display_top_tabs($tab_array);
?>
</td></tr>
<tr>
	<td>
	<div id="mainarea">
	<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
<?php 
	$isrulesfolderempty = glob("{$snortdir}/rules/*.rules");
	$iscfgdirempty = glob("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/*.rules");
	if (empty($isrulesfolderempty) && empty($iscfgdirempty)):
?>
		<tr>
			<td>
		<?php printf(gettext("# The rules directory is empty. %s/rules"), $snortdir); ?> <br/>
		<?php echo gettext("Please go to the updates page to download/fetch the rules configured."); ?>	
			</td>
		</tr>
<?php else: 
	$colspan = 6;
	if ($emergingdownload != 'on')
		$colspan -= 2;
	if ($snortdownload != 'on')
		$colspan -= 4;

?>
		<tr>
			<td>
			<table id="sortabletable1" class="sortable" width="100%" border="0"
				cellpadding="0" cellspacing="0">
			<tr>
				<td colspan="6" class="listtopic"><?php echo gettext("Automatic flowbit resolution"); ?><br/></td>
			</tr>
			<tr>
				<td colspan="6" valign="center" class="listn">
					<table width="100%" border="0" cellpadding="0" cellspacing="0">
					   <tr>
						<td width="15%" class="listn"><?php echo gettext("Resolve Flowbits"); ?></td>
						<td width="85%"><input name="autoflowbits" id="autoflowbitrules" type="checkbox" value="on" <?php if ($a_nat[$id]['autoflowbitrules'] == "on") echo "checked"; ?>/></td>
					   </tr>
					   <tr>
						<td width="15%" class="vncell">&nbsp;</td>
						<td width="85%" class="vtable">
						<?php echo gettext("If ticked, Snort will examine the enabled rules in your chosen " .
						"rule categories for checked flowbits.  Any rules that set these dependent flowbits will " .
						"be automatically enabled and added to the list of files in the interface rules directory."); ?><br/><br/></td>
					   </tr>
					   <tr>
						<td width="15%" class="listn"><?php echo gettext("Auto Flowbit Rules"); ?></td>
						<td width="85%"><input type="button" class="formbtn" value="View" onclick="popup('snort_rules_edit.php?id=<?=$id;?>&openruleset=<?=$flowbit_rules_file;?>')" <?php echo $btn_view_flowb_rules; ?>/></td>
					   </tr>
					   <tr>
						<td width="15%">&nbsp;</td>
						<td width="85%">
						<?php echo gettext("Click to view auto-enabled rules required to satisfy flowbit " .
						"dependencies from the selected rule categories below.  Auto-enabled rules generating unwanted alerts " .
						"should have their GID:SID added to the Suppression List for the interface."); ?><br/><br/></td>
					   </tr>
					</table>
				</td>
			</tr>
			<tr>
				<td colspan="6" class="listtopic"><?php echo gettext("Snort IPS Policy Selection"); ?><br/></td>
			</tr>
			<tr>
				<td colspan="6" valign="center" class="listn">
					<table width="100%" border="0" cellpadding="0" cellspacing="0">
					   <tr>
						<td width="15%" class="listn"><?php echo gettext("Use IPS Policy"); ?></td>
						<td width="85%"><input name="ips_policy_enable" id="ips_policy_enable" type="checkbox" value="on" <?php if ($a_nat[$id]['ips_policy_enable'] == "on") echo "checked"; ?>
						<?php if ($snortdownload == "off") echo "disabled" ?> onClick="enable_change()"/></td>
					   </tr>
					   <tr>
						<td width="15%" class="vncell">&nbsp;</td>
						<td width="85%" class="vtable">
						<?php echo gettext("If ticked, Snort will use rules from the pre-defined IPS policy " .
						"selected below.  You must be using the Snort VRT rules to use this option."); ?><br/><br/></td>
					   </tr>
					   <tr>
						<td width="15%" class="listn"><?php echo gettext("IPS Policy"); ?></td>
						<td width="85%"><select name="ips_policy" class="formselect" <?=$policy_select_disable?> >
									<option value="connectivity" <?php if ($pconfig['ips_policy'] == "connected") echo "selected"; ?>><?php echo gettext("Connectivity"); ?></option>
									<option value="balanced" <?php if ($pconfig['ips_policy'] == "balanced") echo "selected"; ?>><?php echo gettext("Balanced"); ?></option>
									<option value="security" <?php if ($pconfig['ips_policy'] == "security") echo "selected"; ?>><?php echo gettext("Security"); ?></option>
								</select>
						</td>
					   </tr>
					   <tr>
						<td width="15%">&nbsp;</td>
						<td width="85%">
						<?php echo gettext("Snort IPS policies are:  Connectivity, Balanced or Security.  " .
						"Connectivity blocks most major threats with few or no false positives.  Balanced is a good starter policy.  It " .
						"is speedy, has good base coverage level, and covers most threats of the day.  It includes all rules in Connectivity.  " .
						"Security is a stringent policy.  It contains everything in the first two plus policy-type rules such as Flash in an Excel file."); ?><br/><br/></td>
					   </tr>
					</table>
				</td>
			</tr>
			<tr>
				<td colspan="6" class="listtopic"><?php echo gettext("Check the rulesets that you would like Snort to load at startup."); ?><br/></td>
			</tr>
			<tr>
				<td colspan="1" align="middle" valign="center"><br/><input value="Select All" type="submit" name="selectall" id="selectall" /><br/></td>
				<td colspan="1" align="middle" valign="center"><br/><input value="Unselect All" type="submit" name="unselectall" id="selectall" /><br/></td>
				<td colspan="1" align="middle" valign="center"><br/><input value="Save" class="formbtn" type="submit" name="Submit" id="Submit" /></td>
				<td colspan="3" valign="center"><?php echo gettext("Click to save changes and auto-resolve flowbit rules (if option is selected above)"); ?><br/></td>
			</tr>
			<tr>    <td colspan="6">&nbsp;</td> </tr>
			<tr id="frheader">
				<?php if ($emergingdownload == 'on'): ?>
					<td width="5%" class="listhdrr"><?php echo gettext("Enabled"); ?></td>
					<td width="25%" class="listhdrr"><?php echo gettext('Ruleset: Emerging Threats');?></td>
				<?php else: ?>
					<td colspan="2" width="30%" class="listhdrr"><?php echo gettext("Emerging rules have not been enabled"); ?></td>
				<?php endif; ?>
				<?php if ($snortdownload == 'on'): ?>
					<td width="5%" class="listhdrr"><?php echo gettext("Enabled"); ?></td>
					<td width="25%" class="listhdrr"><?php echo gettext('Ruleset: Snort');?></td>
					<td width="5%" class="listhdrr"><?php echo gettext("Enabled"); ?></td>
					<td width="25%" class="listhdrr"><?php echo gettext('Ruleset: Snort SO');?></td>
				<?php else: ?>
					<td colspan="2" width="60%" class="listhdrr"><?php echo gettext("Snort rules have not been enabled"); ?></td>
				<?php endif; ?>
				</tr>
			<?php
				$emergingrules = array();
				$snortsorules = array();
				$snortrules = array();
				if (empty($isrulesfolderempty))
					$dh  = opendir("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/");
				else
					$dh  = opendir("{$snortdir}/rules/");
				while (false !== ($filename = readdir($dh))) {
					$filename = basename($filename);
					if (substr($filename, -5) != "rules")
						continue;
					if (strstr($filename, "emerging") && $emergingdownload == 'on')
						$emergingrules[] = $filename;
					else if (strstr($filename, "snort") && $snortdownload == 'on') {
						if (strstr($filename, ".so.rules"))
							$snortsorules[] = $filename;
						else
							$snortrules[] = $filename;
					}
				}
				sort($emergingrules);
				sort($snortsorules);
				sort($snortrules);
				$i = count($emergingrules);
				if ($i < count($snortsorules))
					$i = count(snortsorules);
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
						echo "<td class='listr' width='25%' >\n";
						if (empty($CHECKED))
							echo $file;
						else
							echo "<a href='snort_rules.php?id={$id}&openruleset=" . urlencode($file) . "'>{$file}</a>\n";
						echo "</td>\n";
					} else
						echo "<td class='listbggrey' width='30%' colspan='2'><br/></td>\n";

					if (!empty($snortrules[$j])) {
						$file = $snortrules[$j];
						echo "<td class='listr' width='5%' align=\"center\" valign=\"top\">";
						if(is_array($enabled_rulesets_array)) {
							if(in_array($file, $enabled_rulesets_array))
								$CHECKED = " checked=\"checked\"";
							else
								$CHECKED = "";
						} else
							$CHECKED = "";
						echo "	\n<input type='checkbox' name='toenable[]' value='{$file}' {$CHECKED} />\n";
						echo "</td>\n";
						echo "<td class='listr' width='25%' >\n";
						if (empty($CHECKED))
							echo $file;
						else
							echo "<a href='snort_rules.php?id={$id}&openruleset=" . urlencode($file) . "'>{$file}</a>\n";
						echo "</td>\n";
					} else
						echo "<td class='listbggrey' width='30%' colspan='2'><br/></td>\n";
					if (!empty($snortsorules[$j])) {
						$file = $snortsorules[$j];
						echo "<td class='listr' width='5%' align=\"center\" valign=\"top\">";
						if(is_array($enabled_rulesets_array)) {
							if(in_array($file, $enabled_rulesets_array))
								$CHECKED = " checked=\"checked\"";
							else
								$CHECKED = "";
						} else
							$CHECKED = "";
						echo "	\n<input type='checkbox' name='toenable[]' value='{$file}' {$CHECKED} />\n";
						echo "</td>\n";
						echo "<td class='listr' width='25%' >\n";
							echo $file;
						echo "</td>\n";
					} else
						echo "<td class='listbggrey' width='30%' colspan='2'><br/></td>\n";
				echo "</tr>\n";
			}
		?>
	</table>
	</td>
</tr>
<tr>
<td colspan="6" class="vtable">&nbsp;<br/></td>
</tr>
			<tr>
				<td colspan="2" align="middle" valign="center"><br/><input value="Save" type="submit" name="Submit" id="Submit" class="formbtn" /></td>
				<td colspan="4" valign="center">&nbsp;<br><br/></td>
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
</body>
</html>
