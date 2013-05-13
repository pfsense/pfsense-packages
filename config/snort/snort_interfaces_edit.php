<?php
/*
 * snort_interfaces_edit.php
 *
 * Copyright (C) 2008-2009 Robert Zelaya.
 * Copyright (C) 2011-2012 Ermal Luci
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

if (!is_array($config['installedpackages']['snortglobal']))
	$config['installedpackages']['snortglobal'] = array();
$snortglob = $config['installedpackages']['snortglobal'];

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

$pconfig = array();
if (empty($snortglob['rule'][$id]['uuid'])) {
	/* Adding new interface, so flag rules to build. */
	$pconfig['uuid'] = snort_generate_id();
	$rebuild_rules = "on";
}
else {
	$pconfig['uuid'] = $a_rule[$id]['uuid'];
	$rebuild_rules = "off";
}
$snort_uuid = $pconfig['uuid'];

if (isset($id) && $a_rule[$id]) {
	/* old options */
	$pconfig = $a_rule[$id];
	if (!empty($pconfig['configpassthru']))
		$pconfig['configpassthru'] = base64_decode($pconfig['configpassthru']);
	if (empty($pconfig['uuid']))
		$pconfig['uuid'] = $snort_uuid;
	if (!$pconfig['interface'])
		$pconfig['interface'] = "wan";
}

if (isset($_GET['dup']))
	unset($id);

if ($_POST["Submit"]) {
	if (!$_POST['interface'])
		$input_errors[] = "Interface is mandatory";

	/* if no errors write to conf */
	if (!$input_errors) {
		$natent = $a_rule[$id];
		$natent['interface'] = $_POST['interface'];
		$natent['enable'] = $_POST['enable'] ? 'on' : 'off';
		$natent['uuid'] = $pconfig['uuid'];

		/* See if the HOME_NET, EXTERNAL_NET, WHITELIST or SUPPRESS LIST values were changed */
		$snort_reload = false;
		if ($_POST['homelistname'] && ($_POST['homelistname'] <> $natent['homelistname']))
			$snort_reload = true;
		if ($_POST['externallistname'] && ($_POST['externallistname'] <> $natent['externallistname']))
			$snort_reload = true;
		if ($_POST['suppresslistname'] && ($_POST['suppresslistname'] <> $natent['suppresslistname']))
			$snort_reload = true;
		if ($_POST['whitelistname'] && ($_POST['whitelistname'] <> $natent['whitelistname']))
			$snort_reload = true;

		if ($_POST['descr']) $natent['descr'] =  $_POST['descr']; else $natent['descr'] = strtoupper($natent['interface']);
		if ($_POST['performance']) $natent['performance'] = $_POST['performance']; else  unset($natent['performance']);
		/* if post = on use on off or rewrite the conf */
		if ($_POST['blockoffenders7'] == "on") $natent['blockoffenders7'] = 'on'; else $natent['blockoffenders7'] = 'off';
		if ($_POST['blockoffenderskill'] == "on") $natent['blockoffenderskill'] = 'on'; else unset($natent['blockoffenderskill']);
		if ($_POST['blockoffendersip']) $natent['blockoffendersip'] = $_POST['blockoffendersip']; else unset($natent['blockoffendersip']);
		if ($_POST['whitelistname']) $natent['whitelistname'] =  $_POST['whitelistname']; else unset($natent['whitelistname']);
		if ($_POST['homelistname']) $natent['homelistname'] =  $_POST['homelistname']; else unset($natent['homelistname']);
		if ($_POST['externallistname']) $natent['externallistname'] =  $_POST['externallistname']; else unset($natent['externallistname']);
		if ($_POST['suppresslistname']) $natent['suppresslistname'] =  $_POST['suppresslistname']; else unset($natent['suppresslistname']);
		if ($_POST['alertsystemlog'] == "on") { $natent['alertsystemlog'] = 'on'; }else{ $natent['alertsystemlog'] = 'off'; }
		if ($_POST['configpassthru']) $natent['configpassthru'] = base64_encode($_POST['configpassthru']); else unset($natent['configpassthru']);
		if ($_POST['cksumcheck']) $natent['cksumcheck'] = 'on'; else $natent['cksumcheck'] = 'off';

		$if_real = snort_get_real_interface($natent['interface']);
		if (isset($id) && $a_rule[$id]) {
			if ($natent['interface'] != $a_rule[$id]['interface']) {
				$oif_real = snort_get_real_interface($a_rule[$id]['interface']);
				snort_stop($a_rule[$id], $oif_real);
				exec("rm -r /var/log/snort_{$oif_real}" . $a_rule[$id]['uuid']);
				exec("mv -f {$snortdir}/snort_" . $a_rule[$id]['uuid'] . "_{$oif_real} {$snortdir}/snort_" . $a_rule[$id]['uuid'] . "_{$if_real}");
			}
			$a_rule[$id] = $natent;
		} else
			$a_rule[] = $natent;

		/* If Snort is disabled on this interface, stop any running instance */
		if ($natent['enable'] != 'on')
			snort_stop($natent, $if_real);

		/* Save configuration changes */
		write_config();

		/* Most changes don't require a rules rebuild, so default to "off" */
		$rebuild_rules = "off";

		/* Update snort.conf and snort.sh files for this interface */
		sync_snort_package_config();

		/*******************************************************/
		/* Signal Snort to reload configuration if we changed  */
		/* HOME_NET, the Whitelist, EXTERNAL_NET or Suppress   */
		/* list values.  The function only signals a running   */
		/* Snort instance to safely reload these parameters.   */
		/*******************************************************/
		if ($snort_reload == true)
			snort_reload_config($natent, $if_real);

		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		header("Location: /snort/snort_interfaces.php");
		exit;
	} else
		$pconfig = $_POST;
}

$if_friendly = snort_get_friendly_interface($pconfig['interface']);
$pgtitle = "Snort: Interface Edit: {$if_friendly}";
include_once("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<?php include("fbegin.inc"); ?>

<?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>

<?php
	/* Display Alert message */
	if ($input_errors) {
		print_input_errors($input_errors); // TODO: add checks
	}

	if ($savemsg) {
		print_info_box($savemsg);
	}
?>

<form action="snort_interfaces_edit.php<?php echo "?id=$id";?>" method="post" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabnavtbl">
<?php
        $tab_array = array();
	$tab_array[] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[] = array(gettext("If Settings"), true, "/snort/snort_interfaces_edit.php?id={$id}");
	$tab_array[] = array(gettext("Categories"), false, "/snort/snort_rulesets.php?id={$id}");
	$tab_array[] = array(gettext("Rules"), false, "/snort/snort_rules.php?id={$id}");
	$tab_array[] = array(gettext("Variables"), false, "/snort/snort_define_servers.php?id={$id}");
	$tab_array[] = array(gettext("Preprocessors"), false, "/snort/snort_preprocessors.php?id={$id}");
	$tab_array[] = array(gettext("Barnyard2"), false, "/snort/snort_barnyard.php?id={$id}");
        display_top_tabs($tab_array);
?>
</td></tr>
<tr><td class="tabcont">
<table width="100%" border="0" cellpadding="6" cellspacing="0">
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("General Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncellreq"><?php echo gettext("Enable"); ?></td>
		<td width="78%" valign="top" class="vtable">&nbsp;
	<?php
		if ($pconfig['enable'] == "on")
			$checked = "checked";
		echo "
			<input name=\"enable\" type=\"checkbox\" value=\"on\" $checked onClick=\"enable_change(false)\">
			&nbsp;&nbsp;" . gettext("Enable or Disable") . "\n";
	?>
		<br/>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncellreq"><?php echo gettext("Interface"); ?></td>
		<td width="78%" class="vtable">
			<select name="interface" class="formselect" tabindex="0">
		<?php
			if (function_exists('get_configured_interface_with_descr'))
				$interfaces = get_configured_interface_with_descr();
			else {
				$interfaces = array('wan' => 'WAN', 'lan' => 'LAN');
				for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++) {
					$interfaces['opt' . $i] = $config['interfaces']['opt' . $i]['descr'];
				}
			}
			foreach ($interfaces as $iface => $ifacename): ?>
				<option value="<?=$iface;?>"
			<?php if ($iface == $pconfig['interface']) echo "selected"; ?>><?=htmlspecialchars($ifacename);?>
				</option>
			<?php 	endforeach; ?>
			</select>&nbsp;&nbsp;
			<span class="vexpl"><?php echo gettext("Choose which interface this Snort instance applies to."); ?><br/>
				<span class="red"><?php echo gettext("Hint:"); ?> </span><?php echo gettext("in most cases, you'll want to use WAN here."); ?></span><br/></td>
	</tr>
	<tr>
				<td width="22%" valign="top" class="vncellreq"><?php echo gettext("Description"); ?></td>
				<td width="78%" class="vtable"><input name="descr" type="text"
					class="formfld" id="descr" size="40"
					value="<?=htmlspecialchars($pconfig['descr']);?>"> <br/>
				<span class="vexpl"><?php echo gettext("Enter a meaningful description here for your reference."); ?></span><br/></td>
	</tr>
<tr>
	<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Alert Settings"); ?></td>
</tr>
	<tr>
				<td width="22%" valign="top" class="vncell"><?php echo gettext("Send alerts to main " .
				"System logs"); ?></td>
				<td width="78%" class="vtable"><input name="alertsystemlog"
					type="checkbox" value="on"
				<?php if ($pconfig['alertsystemlog'] == "on") echo "checked"; ?>
				onClick="enable_change(false)">
				<?php echo gettext("Snort will send Alerts to the firewall's system logs."); ?></td>
	</tr>
	<tr>
				<td width="22%" valign="top" class="vncell"><?php echo gettext("Block offenders"); ?></td>
				<td width="78%" class="vtable">
					<input name="blockoffenders7" id="blockoffenders7" type="checkbox" value="on"
					<?php if ($pconfig['blockoffenders7'] == "on") echo "checked"; ?>
					onClick="enable_blockoffenders()">
				<?php echo gettext("Checking this option will automatically block hosts that generate a " .
				"Snort alert."); ?></td>
	</tr>
	<tr>
				<td width="22%" valign="top" class="vncell"><?php echo gettext("Kill states"); ?></td>
				<td width="78%" class="vtable">
					<input name="blockoffenderskill" id="blockoffenderskill" type="checkbox" value="on" <?php if ($pconfig['blockoffenderskill'] == "on") echo "checked"; ?>>
					<?php echo gettext("Checking this option will kill firewall states for the blocked IP"); ?>
				</td>
	</tr>
	<tr>
				<td width="22%" valign="top" class="vncell"><?php echo gettext("Which IP to block"); ?></td>
				<td width="78%" class="vtable">
					<select name="blockoffendersip" class="formselect" id="blockoffendersip">
				<?php
					foreach (array("src", "dst", "both") as $btype) {
						if ($btype == $pconfig['blockoffendersip'])
							echo "<option value='{$btype}' selected>";
						else
							echo "<option value='{$btype}'>";
						echo htmlspecialchars($btype) . '</option>';
					}
				?>
					</select>&nbsp;&nbsp;
				<?php echo gettext("Select which IP extracted from the packet you wish to block"); ?> 
				</td>
	</tr>
<tr>
	<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Performance Settings"); ?></td>
</tr>
	<tr>
				<td width="22%" valign="top" class="vncell"><?php echo gettext("Memory Performance"); ?></td>
				<td width="78%" class="vtable">
					<select name="performance" class="formselect" id="performance">
					<?php
					$interfaces2 = array('ac-bnfa' => 'AC-BNFA', 'ac-split' => 'AC-SPLIT', 'lowmem' => 'LOWMEM', 'ac-std' => 'AC-STD', 'ac' => 'AC',
					'ac-nq' => 'AC-NQ', 'ac-bnfa-nq' => 'AC-BNFA-NQ', 'lowmem-nq' => 'LOWMEM-NQ', 'ac-banded' => 'AC-BANDED', 
					'ac-sparsebands' => 'AC-SPARSEBANDS', 'acs' => 'ACS');
					foreach ($interfaces2 as $iface2 => $ifacename2): ?>
					<option value="<?=$iface2;?>"
					<?php if ($iface2 == $pconfig['performance']) echo "selected"; ?>>
						<?=htmlspecialchars($ifacename2);?></option>
						<?php endforeach; ?>
					</select>&nbsp;&nbsp;
				<?php echo gettext("Choose a search performance setting"); ?><br/>
				<span class="vexpl"><?php echo gettext("LOWMEM and AC-BNFA are recommended for low end " .
				"systems, AC-SPLIT: low memory, high performance, short-hand for search-method ac split-any-any, AC: high memory, " .
				"best performance, -NQ: the -nq option specifies that matches should not be queued and evaluated as they are found," . 
				" AC-STD: moderate memory, high performance, ACS: small memory, moderate performance, " .
				"AC-BANDED: small memory,moderate performance, AC-SPARSEBANDS: small memory, high performance."); ?>
				</span><br/></td>
	</tr>
	<tr>
				<td width="22%" valign="top" class="vncell"><?php echo gettext("Checksum Check Disable"); ?></td>
				<td width="78%" class="vtable">
					<input name="cksumcheck" id="cksumcheck" type="checkbox" value="on" <?php if ($pconfig['cksumcheck'] == "on") echo "checked"; ?>>
					<?php echo gettext("Disable checksum checking within Snort to improve performance."); ?>
					<br><span class="red"><?php echo gettext("Hint: ") . "</span>" . 
					gettext("Most of this is already done at the firewall/filter level, so it is usually safe to check this box."); ?>
				</td>
	</tr>
	<tr>
				<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Choose the networks " .
				"Snort should inspect and whitelist."); ?></td>
	</tr>
	<tr>
				<td width="22%" valign="top" class="vncell"><?php echo gettext("Home Net"); ?></td>
				<td width="78%" class="vtable">

					<select name="homelistname" class="formselect" id="homelistname">
					<?php
						echo "<option value='default' >default</option>";
						/* find whitelist names and filter by type */
						if (is_array($snortglob['whitelist']['item'])) {
							foreach ($snortglob['whitelist']['item'] as $value) {
								$ilistname = $value['name'];
								if ($ilistname == $pconfig['homelistname'])
									echo "<option value='$ilistname' selected>";
								else
									echo "<option value='$ilistname'>";
								echo htmlspecialchars($ilistname) . '</option>';
							}
						}
					?>
					</select>
					&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" class="formbtns" value="View List"  
					onclick="viewList('<?=$id;?>','homelistname')" id="btnHomeNet" 
					title="<?php echo gettext("Click to view currently selected Home Net contents"); ?>"/>
					<br/>
					<span class="vexpl"><?php echo gettext("Choose the Home Net you want this interface to use."); ?></span>
				 	<br/></br>
					<span class="red"><?php echo gettext("Note:"); ?></span>&nbsp;<?php echo gettext("Default Home " .
					"Net adds only local networks, WAN IPs, Gateways, VPNs and VIPs."); ?><br/>
					<span class="red"><?php echo gettext("Hint:"); ?></span>&nbsp;<?php echo gettext("Create an Alias to hold a list of " .
					"friendly IPs that the firewall cannot see or to customize the default Home Net."); ?><br/>
				</td>
	</tr>
	<tr>
				<td width="22%" valign="top" class="vncell"><?php echo gettext("External Net"); ?></td>
				<td width="78%" class="vtable">
					<select name="externallistname" class="formselect" id="externallistname">
					<?php
						echo "<option value='default' >default</option>";
						/* find whitelist names and filter by type */
						if (is_array($snortglob['whitelist']['item'])) {
							foreach ($snortglob['whitelist']['item'] as $value) {
								$ilistname = $value['name'];
								if ($ilistname == $pconfig['externallistname'])
									echo "<option value='$ilistname' selected>";
								else
									echo "<option value='$ilistname'>";
								echo htmlspecialchars($ilistname) . '</option>';
							}
						}
					?>
					</select>&nbsp;&nbsp;
					<span class="vexpl"><?php echo gettext("Choose the External Net you want this interface " .
					"to use."); ?></span>&nbsp;<br/><br/>
					<span class="red"><?php echo gettext("Note:"); ?></span>&nbsp;<?php echo gettext("Default " .
					"External Net is networks that are not Home Net."); ?><br/>
					<span class="red"><?php echo gettext("Hint:"); ?></span>&nbsp;<?php echo gettext("Most users should leave this " .
					"setting at default.  Create an Alias for custom External Net settings."); ?><br/>
				</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Whitelist"); ?></td>
		<td width="78%" class="vtable">
			<select name="whitelistname" class="formselect" id="whitelistname">
			<?php
				/* find whitelist names and filter by type, make sure to track by uuid */
				echo "<option value='default' >default</option>\n";
				if (is_array($snortglob['whitelist']['item'])) {
					foreach ($snortglob['whitelist']['item'] as $value) {
						if ($value['name'] == $pconfig['whitelistname'])
							echo "<option value='{$value['name']}' selected>";
						else
							echo "<option value='{$value['name']}'>";
						echo htmlspecialchars($value['name']) . '</option>';
					}
				}
			?>
			</select>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" class="formbtns" value="View List" onclick="viewList('<?=$id;?>','whitelistname')" 
			id="btnWhitelist" title="<?php echo gettext("Click to view currently selected Whitelist contents"); ?>"/>
			<br/>
			<span class="vexpl"><?php echo gettext("Choose the whitelist you want this interface to " .
			"use."); ?> </span><br/>&nbsp;<br/><span class="red"><?php echo gettext("Hint:"); ?></span>&nbsp;<?php echo gettext("Default " .
			"whitelist adds local networks, WAN IPs, Gateways, VPNs and VIPs.  Create an Alias to customize."); ?><br/>
			<span class="red"><?php echo gettext("Note:"); ?></span>&nbsp;<?php echo gettext("This option will only be used when block offenders is on."); ?>
		</td>
	</tr>
<tr>
	<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Choose a suppression or filtering " .
	"file if desired."); ?></td>
</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Suppression and filtering"); ?></td>
		<td width="78%" class="vtable">
			<select name="suppresslistname" class="formselect" id="suppresslistname">
		<?php
			echo "<option value='default' >default</option>\n";
			if (is_array($snortglob['suppress']['item'])) {
				$slist_select = $snortglob['suppress']['item'];
				foreach ($slist_select as $value) {
					$ilistname = $value['name'];
					if ($ilistname == $pconfig['suppresslistname'])
						echo "<option value='$ilistname' selected>";
					else
						echo "<option value='$ilistname'>";
					echo htmlspecialchars($ilistname) . '</option>';
				}
			}
		?>
		</select>
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" class="formbtns" value="View List" onclick="viewList('<?=$id;?>','suppresslistname', 'suppress')" 
		id="btnSuppressList" title="<?php echo gettext("Click to view currently selected Suppression List contents"); ?>"/>
		<br/>
		<span class="vexpl"><?php echo gettext("Choose the suppression or filtering file you " .
		"want this interface to use."); ?> </span><br/>&nbsp;<br/><span class="red"><?php echo gettext("Note: ") . "</span>" . 
		gettext("Default option disables suppression and filtering."); ?>
		</td>
	</tr>
<tr>
	<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Arguments here will " .
	"be automatically inserted into the Snort configuration."); ?></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Advanced configuration pass through"); ?></td>
	<td width="78%" class="vtable">
		<textarea wrap="off" name="configpassthru" cols="65" rows="12" id="configpassthru"><?=htmlspecialchars($pconfig['configpassthru']);?></textarea>
	</td>
</tr>
<tr>
	<td width="22%" valign="top"></td>
	<td width="78%"><input name="Submit" type="submit" class="formbtn" value="Save" title="<?php echo 
			gettext("Click to save settings and exit"); ?>"/>
			<input name="id" type="hidden" value="<?=$id;?>"/>
	</td>
</tr>
<tr>
	<td width="22%" valign="top">&nbsp;</td>
	<td width="78%"><span class="vexpl"><span class="red"><strong><?php echo gettext("Note: ") . "</strong></span>" . 
		gettext("Please save your settings before you attempt to start Snort."); ?>	
	</td>
</tr>
</table>
</td></tr>
</table>
</form>
<script language="JavaScript">
<!--
function enable_blockoffenders() {
	var endis = !(document.iform.blockoffenders7.checked);
	document.iform.blockoffenderskill.disabled=endis;
	document.iform.blockoffendersip.disabled=endis;
	document.iform.whitelistname.disabled=endis;
	document.iform.btnWhitelist.disabled=endis;
}

function enable_change(enable_change) {
	endis = !(document.iform.enable.checked || enable_change);
	// make sure a default answer is called if this is invoked.
	endis2 = (document.iform.enable);
	document.iform.performance.disabled = endis;
	document.iform.blockoffenders7.disabled = endis;
	document.iform.blockoffendersip.disabled=endis;
	document.iform.blockoffenderskill.disabled=endis;
	document.iform.alertsystemlog.disabled = endis;
	document.iform.externallistname.disabled = endis;
	document.iform.cksumcheck.disabled = endis;
	document.iform.homelistname.disabled = endis;
	document.iform.whitelistname.disabled=endis;
	document.iform.suppresslistname.disabled = endis;
	document.iform.configpassthru.disabled = endis;
	document.iform.btnHomeNet.disabled=endis;
	document.iform.btnWhitelist.disabled=endis;
	document.iform.btnSuppressList.disabled=endis;
}

function wopen(url, name, w, h) {
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

function getSelectedValue(elemID) {
	var ctrl = document.getElementById(elemID);
	return ctrl.options[ctrl.selectedIndex].value;
}

function viewList(id, elemID, elemType) {
	if (typeof elemType == "undefined") {
		elemType = "whitelist";
	}
	var url = "snort_list_view.php?id=" + id + "&wlist=";
	url = url + getSelectedValue(elemID) + "&type=" + elemType;
	wopen(url, 'WhitelistViewer', 640, 480);
}
//-->
</script>
<?php include("fend.inc"); ?>
</body>
</html>
