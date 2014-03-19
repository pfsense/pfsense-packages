<?php
/*
 * suricata_global.php
 * part of pfSense
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

global $g;

$suricatadir = SURICATADIR;

$pconfig['enable_vrt_rules'] = $config['installedpackages']['suricata']['config'][0]['enable_vrt_rules'];
$pconfig['oinkcode'] = $config['installedpackages']['suricata']['config'][0]['oinkcode'];
$pconfig['etprocode'] = $config['installedpackages']['suricata']['config'][0]['etprocode'];
$pconfig['enable_etopen_rules'] = $config['installedpackages']['suricata']['config'][0]['enable_etopen_rules'];
$pconfig['enable_etpro_rules'] = $config['installedpackages']['suricata']['config'][0]['enable_etpro_rules'];
$pconfig['rm_blocked'] = $config['installedpackages']['suricata']['config'][0]['rm_blocked'];
$pconfig['autoruleupdate'] = $config['installedpackages']['suricata']['config'][0]['autoruleupdate'];
$pconfig['autoruleupdatetime'] = $config['installedpackages']['suricata']['config'][0]['autoruleupdatetime'];
$pconfig['live_swap_updates'] = $config['installedpackages']['suricata']['config'][0]['live_swap_updates'];
$pconfig['log_to_systemlog'] = $config['installedpackages']['suricata']['config'][0]['log_to_systemlog'];
$pconfig['forcekeepsettings'] = $config['installedpackages']['suricata']['config'][0]['forcekeepsettings'];
$pconfig['snortcommunityrules'] = $config['installedpackages']['suricata']['config'][0]['snortcommunityrules'];

if (empty($pconfig['autoruleupdatetime']))
	$pconfig['autoruleupdatetime'] = '00:30';

if ($_POST['autoruleupdatetime']) {
	if (!preg_match('/^([01]?[0-9]|2[0-3]):?([0-5][0-9])$/', $_POST['autoruleupdatetime']))
		$input_errors[] = "Invalid Rule Update Start Time!  Please supply a value in 24-hour format as 'HH:MM'.";
}

if ($_POST['suricatadownload'] == "on" && empty($_POST['oinkcode']))
		$input_errors[] = "You must supply an Oinkmaster code in the box provided in order to enable Snort VRT rules!";

if ($_POST['enable_etpro_rules'] == "on" && empty($_POST['etprocode']))
		$input_errors[] = "You must supply a subscription code in the box provided in order to enable Emerging Threats Pro rules!";

/* if no errors move foward with save */
if (!$input_errors) {
	if ($_POST["save"]) {

		$config['installedpackages']['suricata']['config'][0]['enable_vrt_rules'] = $_POST['enable_vrt_rules'] ? 'on' : 'off';
		$config['installedpackages']['suricata']['config'][0]['snortcommunityrules'] = $_POST['snortcommunityrules'] ? 'on' : 'off';
		$config['installedpackages']['suricata']['config'][0]['enable_etopen_rules'] = $_POST['enable_etopen_rules'] ? 'on' : 'off';
		$config['installedpackages']['suricata']['config'][0]['enable_etpro_rules'] = $_POST['enable_etpro_rules'] ? 'on' : 'off';

		// If any rule sets are being turned off, then remove them
		// from the active rules section of each interface.  Start
		// by building an arry of prefixes for the disabled rules.
		$disabled_rules = array();
		$disable_ips_policy = false;
		if ($config['installedpackages']['suricata']['config'][0]['enable_vrt_rules'] == 'off') {
			$disabled_rules[] = VRT_FILE_PREFIX;
			$disable_ips_policy = true;
		}
		if ($config['installedpackages']['suricata']['config'][0]['snortcommunityrules'] == 'off')
			$disabled_rules[] = GPL_FILE_PREFIX;
		if ($config['installedpackages']['suricata']['config'][0]['enable_etopen_rules'] == 'off')
			$disabled_rules[] = ET_OPEN_FILE_PREFIX;
		if ($config['installedpackages']['suricata']['config'][0]['enable_etpro_rules'] == 'off')
			$disabled_rules[] = ET_PRO_FILE_PREFIX;

		// Now walk all the configured interface rulesets and remove
		// any matching the disabled ruleset prefixes.
		if (is_array($config['installedpackages']['suricata']['rule'])) {
			foreach ($config['installedpackages']['suricata']['rule'] as &$iface) {
				// Disable Snort IPS policy if VRT rules are disabled
				if ($disable_ips_policy) {
					$iface['ips_policy_enable'] = 'off';
					unset($iface['ips_policy']);
				}
				$enabled_rules = explode("||", $iface['rulesets']);
				foreach ($enabled_rules as $k => $v) {
					foreach ($disabled_rules as $d)
						if (strpos(trim($v), $d) !== false)
							unset($enabled_rules[$k]);
				}
				$iface['rulesets'] = implode("||", $enabled_rules);
			}
		}

		$config['installedpackages']['suricata']['config'][0]['oinkcode'] = $_POST['oinkcode'];
		$config['installedpackages']['suricata']['config'][0]['etprocode'] = $_POST['etprocode'];
		$config['installedpackages']['suricata']['config'][0]['rm_blocked'] = $_POST['rm_blocked'];
		$config['installedpackages']['suricata']['config'][0]['autoruleupdate'] = $_POST['autoruleupdate'];

		/* Check and adjust format of Rule Update Starttime string to add colon and leading zero if necessary */
		$pos = strpos($_POST['autoruleupdatetime'], ":");
		if ($pos === false) {
			$tmp = str_pad($_POST['autoruleupdatetime'], 4, "0", STR_PAD_LEFT);
			$_POST['autoruleupdatetime'] = substr($tmp, 0, 2) . ":" . substr($tmp, -2);
		}
		$config['installedpackages']['suricata']['config'][0]['autoruleupdatetime'] = str_pad($_POST['autoruleupdatetime'], 4, "0", STR_PAD_LEFT);
		$config['installedpackages']['suricata']['config'][0]['log_to_systemlog'] = $_POST['log_to_systemlog'] ? 'on' : 'off';
		$config['installedpackages']['suricata']['config'][0]['live_swap_updates'] = $_POST['live_swap_updates'] ? 'on' : 'off';
		$config['installedpackages']['suricata']['config'][0]['forcekeepsettings'] = $_POST['forcekeepsettings'] ? 'on' : 'off';

		$retval = 0;

		/* create whitelist and homenet file, then sync files */
		sync_suricata_package_config();

		write_config();

		/* forces page to reload new settings */
		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		header("Location: /suricata/suricata_global.php");
		exit;
	}
}

$pgtitle = gettext("Suricata: Global Settings");
include_once("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">

<?php
include_once("fbegin.inc");

if($pfsense_stable == 'yes')
	echo '<p class="pgtitle">' . $pgtitle . '</p>';

/* Display Alert message, under form tag or no refresh */
if ($input_errors)
	print_input_errors($input_errors);

?>

<form action="suricata_global.php" method="post" enctype="multipart/form-data" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[] = array(gettext("Suricata Interfaces"), false, "/suricata/suricata_interfaces.php");
        $tab_array[] = array(gettext("Global Settings"), true, "/suricata/suricata_global.php");
	$tab_array[] = array(gettext("Update Rules"), false, "/suricata/suricata_download_updates.php");
	$tab_array[] = array(gettext("Alerts"), false, "/suricata/suricata_alerts.php");
	$tab_array[] = array(gettext("Suppress"), false, "/suricata/suricata_suppress.php");
	$tab_array[] = array(gettext("Logs Browser"), false, "/suricata/suricata_logs_browser.php");
	$tab_array[] = array(gettext("Logs Mgmt"), false, "/suricata/suricata_logs_mgmt.php");
        display_top_tabs($tab_array);
?>
</td></tr>
<tr>
	<td>
	<div id="mainarea">
	<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
<tr>
	<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Please Choose The Type Of Rules You Wish To Download");?></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Install ") . "<strong>" . gettext("Emerging Threats") . "</strong>" . gettext(" rules");?></td>
	<td width="78%" class="vtable">
		<table width="100%" border="0" cellpadding="2" cellspacing="0">
			<tr>
				<td valign="top" width="8%"><input name="enable_etopen_rules" type="checkbox" value="on" onclick="enable_et_rules();" 
				<?php if ($config['installedpackages']['suricata']['config'][0]['enable_etopen_rules']=="on") echo "checked"; ?>/></td>
				<td><span class="vexpl"><?php echo gettext("ETOpen is an open source set of Snort rules whose coverage " .
				"is more limited than ETPro."); ?></span></td>
			</tr>
			<tr>
				<td valign="top" width="8%"><input name="enable_etpro_rules" type="checkbox" value="on" onclick="enable_pro_rules();" 
				<?php if ($config['installedpackages']['suricata']['config'][0]['enable_etpro_rules']=="on") echo "checked"; ?>/></td>
				<td><span class="vexpl"><?php echo gettext("ETPro for Snort offers daily updates and extensive coverage of current malware threats."); ?></span></td>
			</tr>
		<tr>
			<td>&nbsp;</td>
			<td><a href="http://www.emergingthreats.net/solutions/etpro-ruleset/" target="_blank"><?php echo gettext("Sign Up for an ETPro Account"); ?> </a></td>
		</tr>
		<tr>
			<td>&nbsp;</td>
			<td class="vexpl"><?php echo "<span class='red'><strong>" . gettext("Note:") . "</strong></span>" . "&nbsp;" . 
			gettext("The ETPro rules contain all of the ETOpen rules, so the ETOpen rules are not required and are disabled when the ETPro rules are selected."); ?></td>
		</tr>
		</table>
		<table id="etpro_code_tbl" width="100%" border="0" cellpadding="2" cellspacing="0">
		<tr>
			<td colspan="2">&nbsp;</td>
		</tr>
		<tr>
			<td colspan="2" valign="top"><b><span class="vexpl"><?php echo gettext("ETPro Subscription Configuration"); ?></span></b></td>
		</tr>
		<tr>
			<td valign="top"><span class="vexpl"><strong><?php echo gettext("Code:"); ?></strong></span></td>
			<td><input name="etprocode" type="text" class="formfld unknown" id="etprocode" size="52" 
			value="<?=htmlspecialchars($pconfig['etprocode']);?>"/><br/>
			<?php echo gettext("Obtain an ETPro subscription code and paste it here."); ?></td>
		</tr>
		</table>
	</td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Install ") . "<strong>" . gettext("Snort VRT") . "</strong>" . gettext(" rules");?></td>
	<td width="78%" class="vtable">
		<table width="100%" border="0" cellpadding="2" cellspacing="0">
		<tr>
			<td><input name="enable_vrt_rules" type="checkbox" id="enable_vrt_rules" value="on" onclick="enable_snort_vrt();" 
			<?php if($pconfig['enable_vrt_rules']=='on') echo 'checked'; ?>/></td>
			<td><span class="vexpl"><?php echo gettext("Snort VRT free Registered User or paid Subscriber rules"); ?></span></td>
		<tr>
			<td>&nbsp;</td>
			<td><a href="https://www.snort.org/signup" target="_blank"><?php echo gettext("Sign Up for a free Registered User Rule Account"); ?> </a><br/>
			<a href="http://www.snort.org/vrt/buy-a-subscription" target="_blank">
			<?php echo gettext("Sign Up for paid Sourcefire VRT Certified Subscriber Rules"); ?></a></td>
		</tr>
		</table>
		<table id="snort_oink_code_tbl" width="100%" border="0" cellpadding="2" cellspacing="0">
		<tr>
			<td colspan="2">&nbsp;</td>
		</tr>
		<tr>
			<td colspan="2" valign="top"><b><span class="vexpl"><?php echo gettext("Snort VRT Oinkmaster Configuration"); ?></span></b></td>
		</tr>
		<tr>
			<td valign="top"><span class="vexpl"><strong><?php echo gettext("Code:"); ?></strong></span></td>
			<td><input name="oinkcode" type="text" class="formfld unknown" id="oinkcode" size="52" 
			value="<?=htmlspecialchars($pconfig['oinkcode']);?>"/><br/>
			<?php echo gettext("Obtain a snort.org Oinkmaster code and paste it here."); ?></td>
		</tr>
		</table>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Install ") . "<strong>" . gettext("Snort Community") . "</strong>" . gettext(" rules");?></td>
	<td width="78%" class="vtable">
		<table width="100%" border="0" cellpadding="2" cellspacing="0">
			<tr>
				<td valign="top" width="8%"><input name="snortcommunityrules" type="checkbox" value="on"
				<?php if ($config['installedpackages']['suricata']['config'][0]['snortcommunityrules']=="on") echo " checked";?>/></td>
				<td class="vexpl"><?php echo gettext("The Snort Community Ruleset is a GPLv2 VRT certified ruleset that is distributed free of charge " . 
				"without any VRT License restrictions.  This ruleset is updated daily and is a subset of the subscriber ruleset.");?>
				<br/><br/><?php echo "<span class=\"red\"><strong>" . gettext("Note:  ") . "</strong></span>" . 
				gettext("If you are a Snort VRT Paid Subscriber, the community ruleset is already built into your download of the ") . 
				gettext("Snort VRT rules, and there is no benefit in adding this rule set.");?><br/></td>
			</tr>
		</table></td>
</tr>
<tr>
	<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Rules Update Settings"); ?></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Update Interval"); ?></td>
	<td width="78%" class="vtable">
		<select name="autoruleupdate" class="formselect" id="autoruleupdate" onchange="enable_change_rules_upd()">
		<?php
		$interfaces3 = array('never_up' => gettext('NEVER'), '6h_up' => gettext('6 HOURS'), '12h_up' => gettext('12 HOURS'), '1d_up' => gettext('1 DAY'), '4d_up' => gettext('4 DAYS'), '7d_up' => gettext('7 DAYS'), '28d_up' => gettext('28 DAYS'));
		foreach ($interfaces3 as $iface3 => $ifacename3): ?>
		<option value="<?=$iface3;?>"
		<?php if ($iface3 == $pconfig['autoruleupdate']) echo "selected"; ?>>
			<?=htmlspecialchars($ifacename3);?></option>
			<?php endforeach; ?>
	</select>&nbsp;&nbsp;<?php echo gettext("Please select the interval for rule updates. Choosing ") . 
	"<strong>" . gettext("NEVER") . "</strong>" . gettext(" disables auto-updates."); ?><br/><br/>
	<?php echo "<span class=\"red\"><strong>" . gettext("Hint: ") . "</strong></span>" . gettext("in most cases, every 12 hours is a good choice."); ?></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Update Start Time"); ?></td>
	<td width="78%" class="vtable"><input type="text" class="formfld time" name="autoruleupdatetime" id="autoruleupdatetime" size="4" 
	maxlength="5" value="<?=$pconfig['autoruleupdatetime'];?>" <?php if ($pconfig['autoruleupdate'] == "never_up") {echo "disabled";} ?>/>&nbsp;&nbsp;
	<?php echo gettext("Enter the rule update start time in 24-hour format (HH:MM). Default is ") . "<strong>" . gettext("00:03") . "</strong>"; ?>.<br/><br/>
	<?php echo gettext("Rules will update at the interval chosen above starting at the time specified here. For example, using the default " . 
	"start time of 00:03 and choosing 12 Hours for the interval, the rules will update at 00:03 and 12:03 each day."); ?></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Live Rule Swap on Update"); ?></td>
	<td width="78%" class="vtable"><input name="live_swap_updates" id="live_swap_updates" type="checkbox" value="yes"
	<?php if ($config['installedpackages']['suricata']['config'][0]['live_swap_updates']=="on") echo " checked"; ?>/>
	&nbsp;<?php echo gettext("Enable \"Live Swap\" reload of rules after downloading an update.  Default is ") . "<strong>" . gettext("Not Checked") . "</strong>"; ?><br/><br/>
	<?php echo gettext("When enabled, Suricata will perform a live load of the new rules following an update instead of a hard restart.  " . 
	"If issues are encountered with live load, uncheck this option to perform a hard restart of all Suricata instances following an update."); ?></td>
</tr>
<tr>
	<td colspan="2" valign="top" class="listtopic"><?php echo gettext("General Settings"); ?></td>
</tr>
<tr style="display:none;">
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Remove Blocked Hosts Interval"); ?></td>
	<td width="78%" class="vtable">
		<select name="rm_blocked" class="formselect" id="rm_blocked">
		<?php
		$interfaces3 = array('never_b' => gettext('NEVER'), '15m_b' => gettext('15 MINS'), '30m_b' => gettext('30 MINS'), '1h_b' => gettext('1 HOUR'), '3h_b' => gettext('3 HOURS'), '6h_b' => gettext('6 HOURS'), '12h_b' => gettext('12 HOURS'), '1d_b' => gettext('1 DAY'), '4d_b' => gettext('4 DAYS'), '7d_b' => gettext('7 DAYS'), '28d_b' => gettext('28 DAYS'));
		foreach ($interfaces3 as $iface3 => $ifacename3): ?>
		<option value="<?=$iface3;?>"
		<?php if ($iface3 == $pconfig['rm_blocked']) echo "selected"; ?>>
			<?=htmlspecialchars($ifacename3);?></option>
			<?php endforeach; ?>
	</select>&nbsp;
	<?php echo gettext("Please select the amount of time you would like hosts to be blocked."); ?><br/><br/>
	<?php echo "<span class=\"red\"><strong>" . gettext("Hint:") . "</strong></span>" . gettext(" in most cases, 1 hour is a good choice.");?></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Log to System Log"); ?></td>
	<td width="78%" class="vtable"><input name="log_to_systemlog" id="log_to_systemlog" type="checkbox" value="yes"
	<?php if ($config['installedpackages']['suricata']['config'][0]['log_to_systemlog']=="on") echo " checked"; ?>/>&nbsp;
	<?php echo gettext("Copy Suricata messages to the firewall system log."); ?></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Keep Suricata Settings After Deinstall"); ?></td>
	<td width="78%" class="vtable"><input name="forcekeepsettings" id="forcekeepsettings" type="checkbox" value="yes"
	<?php if ($config['installedpackages']['suricata']['config'][0]['forcekeepsettings']=="on") echo " checked"; ?>/>&nbsp;
	<?php echo gettext("Settings will not be removed during package deinstallation."); ?></td>
</tr>
<tr>
	<td colspan="2" align="center"><input name="save" type="submit" class="formbtn" value="Save"/></td>
</tr>
<tr>
	<td colspan="2" class="vexpl" align="center"><span class="red"><strong><?php echo gettext("Note:");?></strong>&nbsp;
	</span><?php echo gettext("Changing any settings on this page will affect all Suricata-configured interfaces.");?></td>
</tr>
	</table>
</div><br/>
</td></tr>
</table>
</form>
<?php include("fend.inc"); ?>

<script language="JavaScript">
<!--
function enable_snort_vrt() {
	var endis = !(document.iform.enable_vrt_rules.checked);
	if (endis)
		document.getElementById("snort_oink_code_tbl").style.display = "none";
	else
		document.getElementById("snort_oink_code_tbl").style.display = "table";
}

function enable_et_rules() {
	var endis = document.iform.enable_etopen_rules.checked;
	if (endis) {
		document.iform.enable_etpro_rules.checked = !(endis);
		document.getElementById("etpro_code_tbl").style.display = "none";
	}
}

function enable_pro_rules() {
	var endis = document.iform.enable_etpro_rules.checked;
	if (endis) {
		document.iform.enable_etopen_rules.checked = !(endis);
		document.iform.etprocode.disabled = "";
		document.getElementById("etpro_code_tbl").style.display = "table";
	}
	else {
		document.iform.etprocode.disabled = "true";
		document.getElementById("etpro_code_tbl").style.display = "none";
	}
}

function enable_change_rules_upd() {
	if (document.iform.autoruleupdate.selectedIndex == 0)
		document.iform.autoruleupdatetime.disabled="true";
	else
		document.iform.autoruleupdatetime.disabled="";		
}

// Initialize the form controls state based on saved settings
enable_snort_vrt();
enable_et_rules();
enable_pro_rules();
enable_change_rules_upd();

//-->
</script>

</body>
</html>
