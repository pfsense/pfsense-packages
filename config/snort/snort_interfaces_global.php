<?php
/*
 * snort_interfaces_global.php
 * part of pfSense
 *
 * Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
 * Copyright (C) 2011-2012 Ermal Luci
 * All rights reserved.
 *
 * Copyright (C) 2008-2009 Robert Zelaya
 * Modified for the Pfsense snort package.
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

/* make things short  */
$pconfig['snortdownload'] = $config['installedpackages']['snortglobal']['snortdownload'];
$pconfig['oinkmastercode'] = $config['installedpackages']['snortglobal']['oinkmastercode'];
$pconfig['emergingthreats'] = $config['installedpackages']['snortglobal']['emergingthreats'];
$pconfig['rm_blocked'] = $config['installedpackages']['snortglobal']['rm_blocked'];
$pconfig['snortloglimit'] = $config['installedpackages']['snortglobal']['snortloglimit'];
$pconfig['snortloglimitsize'] = $config['installedpackages']['snortglobal']['snortloglimitsize'];
$pconfig['autorulesupdate7'] = $config['installedpackages']['snortglobal']['autorulesupdate7'];
$pconfig['forcekeepsettings'] = $config['installedpackages']['snortglobal']['forcekeepsettings'];
$pconfig['protect_preproc_rules'] = $config['installedpackages']['snortglobal']['protect_preproc_rules'];

/* if no errors move foward */
if (!$input_errors) {

	if ($_POST["Submit"]) {

		$config['installedpackages']['snortglobal']['snortdownload'] = $_POST['snortdownload'];
		$config['installedpackages']['snortglobal']['oinkmastercode'] = $_POST['oinkmastercode'];

		$config['installedpackages']['snortglobal']['emergingthreats'] = $_POST['emergingthreats'] ? 'on' : 'off';
		$config['installedpackages']['snortglobal']['rm_blocked'] = $_POST['rm_blocked'];
		$config['installedpackages']['snortglobal']['protect_preproc_rules'] = $_POST['protect_preproc_rules'] ? 'on' : 'off';
		if ($_POST['snortloglimitsize']) {
			$config['installedpackages']['snortglobal']['snortloglimit'] = $_POST['snortloglimit'];
			$config['installedpackages']['snortglobal']['snortloglimitsize'] = $_POST['snortloglimitsize'];
		} else {
			$config['installedpackages']['snortglobal']['snortloglimit'] = 'on';

			/* code will set limit to 21% of slice that is unused */
			$snortloglimitDSKsize = round(exec('df -k /var | grep -v "Filesystem" | awk \'{print $4}\'') * .22 / 1024);
			$config['installedpackages']['snortglobal']['snortloglimitsize'] = $snortloglimitDSKsize;
		}
		$config['installedpackages']['snortglobal']['autorulesupdate7'] = $_POST['autorulesupdate7'];
		$config['installedpackages']['snortglobal']['forcekeepsettings'] = $_POST['forcekeepsettings'] ? 'on' : 'off';

		$retval = 0;

		/* create whitelist and homenet file  then sync files */
		sync_snort_package_config();

		write_config();

		/* forces page to reload new settings */
		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		header("Location: /snort/snort_interfaces_global.php");
		exit;
	}
}

$pgtitle = 'Services: Snort: Global Settings';
include_once("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">

<?php
include_once("fbegin.inc");

if($pfsense_stable == 'yes')
	echo '<p class="pgtitle">' . $pgtitle . '</p>';

/* Display Alert message, under form tag or no refresh */
if ($input_errors)
	print_input_errors($input_errors); // TODO: add checks

?>

<script language="JavaScript">
<!--
function enable_snort_vrt(btn) {
	if (btn == 'off') {
		document.iform.oinkmastercode.disabled = "true";
		document.iform.protect_preproc_rules.disabled = "true";
	}
	if (btn == 'on') {
		document.iform.oinkmastercode.disabled = "";
		document.iform.protect_preproc_rules.disabled = "";
	}	
}
//-->
</script>


<form action="snort_interfaces_global.php" method="post" enctype="multipart/form-data" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabnavtbl">
<?php
        $tab_array = array();
        $tab_array[0] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[1] = array(gettext("Global Settings"), true, "/snort/snort_interfaces_global.php");
        $tab_array[2] = array(gettext("Updates"), false, "/snort/snort_download_updates.php");
        $tab_array[3] = array(gettext("Alerts"), false, "/snort/snort_alerts.php");
        $tab_array[4] = array(gettext("Blocked"), false, "/snort/snort_blocked.php");
        $tab_array[5] = array(gettext("Whitelists"), false, "/snort/snort_interfaces_whitelist.php");
        $tab_array[6] = array(gettext("Suppress"), false, "/snort/snort_interfaces_suppress.php");
        display_top_tabs($tab_array);
?>
</td></tr>
<tr>
	<td class="tabcont">
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
<tr>
	<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Please Choose The " .
		"Type Of Rules You Wish To Download"); ?></td>
</tr>
	<td width="22%" valign="top" class="vncell"><?php printf(gettext("Install %sSnort VRT%s rules"), '<strong>' , '</strong>'); ?></td>
	<td width="78%" class="vtable">
		<table width="100%" border="0" cellpadding="2" cellspacing="0">
		<tr>
			<td><input name="snortdownload" type="radio" id="snortdownload" value="off" onclick="enable_snort_vrt('off')"  
			<?php if($pconfig['snortdownload']=='off' || $pconfig['snortdownload']=='') echo 'checked'; ?> >&nbsp;&nbsp;</td>
			<td><span class="vexpl"><?php printf(gettext("Do %sNOT%s Install"), '<strong>',  '</strong>'); ?></span></td>
		</tr>
		<tr>
			<td><input name="snortdownload" type="radio" id="snortdownload" value="on" onclick="enable_snort_vrt('on')" 
			<?php if($pconfig['snortdownload']=='on') echo 'checked'; ?>></td>
			<td><span class="vexpl"><?php echo gettext("Install Basic Rules or Premium rules"); ?></span></td>
		<tr>
			<td>&nbsp;</td>
			<td><a href="https://www.snort.org/signup" target="_blank"><?php echo gettext("Sign Up for a Basic Rule Account"); ?> </a><br>
			<a href="http://www.snort.org/vrt/buy-a-subscription" target="_blank">
			<?php echo gettext("Sign Up for Sourcefire VRT Certified Premium Rules.  This Is Highly Recommended"); ?></a></td>
		</tr>
		<tr>
			<td colspan="2">&nbsp;</td>
		</tr>
		</table>
		<table width="100%" border="0" cellpadding="2" cellspacing="0">
		<tr>
			<td colspan="2" valign="top"><b><span class="vexpl"><?php echo gettext("Oinkmaster Configuration"); ?></span></b></td>
		</tr>
		<tr>
			<td valign="top"><span class="vexpl"><strong><?php echo gettext("Code"); ?><strong></span</td>
			<td><input name="oinkmastercode" type="text"
				class="formfld" id="oinkmastercode" size="52"
				value="<?=htmlspecialchars($pconfig['oinkmastercode']);?>" 
				<?php if($pconfig['snortdownload']<>'on') echo 'disabled'; ?>><br>
			<?php echo gettext("Obtain a snort.org Oinkmaster code and paste it here."); ?></td>
		</tr>
		<tr>
			<td colspan="2">&nbsp;</td>
		</tr>
		<tr>
			<td colspan="2" valign="middle"><b><span class="vexpl"><?php echo gettext("Protect Customized Preprocessor Rules"); ?></span></b></td>
		</tr>
		<tr>
			<td valign="top"><input name="protect_preproc_rules" id="protect_preproc_rules" type="checkbox" value="yes"
				<?php if ($config['installedpackages']['snortglobal']['protect_preproc_rules']=="on") echo "checked"; 
				if($pconfig['snortdownload']<>'on') echo ' disabled'; ?> ><br>
			</td>
			<td><span class="vexpl"><?php echo gettext("Check this box if you maintain customized preprocessor rules files "); ?><br>
				<?php echo gettext("and do not want them overwritten by automatic rule updates."); ?><br>
				<?php printf(gettext("%sHint:%s  Most users should leave this unchecked."), '<span class="red"><strong>', '</strong></span>'); ?></span></br>
			</td>
		</tr>
	</table>

</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php printf(gettext("Install %sEmergingthreats%s " .
	"rules"), '<strong>' , '</strong>'); ?></td>
	<td width="78%" class="vtable"><input name="emergingthreats"
		type="checkbox" value="yes"
		<?php if ($config['installedpackages']['snortglobal']['emergingthreats']=="on") echo "checked"; ?>
		><br>
	<?php echo gettext("Emerging Threats is an open source community that produces fastest " .
	"moving and diverse Snort Rules."); ?></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Update rules " .
	"automatically"); ?></td>
	<td width="78%" class="vtable">
		<select name="autorulesupdate7" class="formselect" id="autorulesupdate7">
		<?php
		$interfaces3 = array('never_up' => gettext('NEVER'), '6h_up' => gettext('6 HOURS'), '12h_up' => gettext('12 HOURS'), '1d_up' => gettext('1 DAY'), '4d_up' => gettext('4 DAYS'), '7d_up' => gettext('7 DAYS'), '28d_up' => gettext('28 DAYS'));
		foreach ($interfaces3 as $iface3 => $ifacename3): ?>
		<option value="<?=$iface3;?>"
		<?php if ($iface3 == $pconfig['autorulesupdate7']) echo "selected"; ?>>
			<?=htmlspecialchars($ifacename3);?></option>
			<?php endforeach; ?>
	</select><br>
	<span class="vexpl"><?php echo gettext("Please select the update times for rules."); ?></br>
	<?php printf(gettext("%sHint%s: in most cases, every 12 hours is a good choice."), '<span class="red"><strong>','</strong></span>'); ?></span></td>
</tr>
<tr>
	<td colspan="2" valign="top" class="listtopic"><?php echo gettext("General Settings"); ?></td>
</tr>

<tr>
<?php $snortlogCurrentDSKsize = round(exec('df -k /var | grep -v "Filesystem" | awk \'{print $4}\'') / 1024); ?>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Log Directory Size " .
	"Limit"); ?><br/>
	<br/>
	<br/>
	<span class="red"><strong><?php echo gettext("Note"); ?></span>:</strong><br>
	<?php echo gettext("Available space is"); ?> <strong><?php echo $snortlogCurrentDSKsize; ?>MB</strong></td>
	<td width="78%" class="vtable">
	<table cellpadding="0" cellspacing="0">
		<tr>
			<td colspan="2"><input name="snortloglimit" type="radio"
				id="snortloglimit" value="on" 
<?php if($pconfig['snortloglimit']=='on') echo 'checked'; ?>><span class="vexpl">
		<strong><?php echo gettext("Enable"); ?></strong> <?php echo gettext("directory size limit"); ?> (<strong><?php echo gettext("Default"); ?></strong>)</span></td>
		</tr>
		<tr>
			<td colspan="2"><input name="snortloglimit" type="radio"
				id="snortloglimit" value="off" 
<?php if($pconfig['snortloglimit']=='off') echo 'checked'; ?>> <span class="vexpl"><strong><?php echo gettext("Disable"); ?></strong>
			<?php echo gettext("directory size limit"); ?></span><br>
			<br>
			<span class="red"><strong><?php echo gettext("Warning"); ?></span>:</strong> <?php echo gettext("Nanobsd " .
			"should use no more than 10MB of space."); ?></td>
		</tr>
		<tr>
			<td>&nbsp;</td>
		</tr>
	</table>
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td><span class="vexpl"><?php echo gettext("Size in"); ?> <strong>MB</strong><span></td>
			<td><input name="snortloglimitsize" type="text"
				class="formfld" id="snortloglimitsize" size="7"
				value="<?=htmlspecialchars($pconfig['snortloglimitsize']);?>">&nbsp;&nbsp;
			<?php echo gettext("Default is"); ?> <strong>20%</strong> <?php echo gettext("of available space."); ?></td>
	
	</table>

</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Remove blocked hosts " .
	"every"); ?></td>
	<td width="78%" class="vtable">
		<select name="rm_blocked" class="formselect" id="rm_blocked">
		<?php
		$interfaces3 = array('never_b' => gettext('NEVER'), '1h_b' => gettext('1 HOUR'), '3h_b' => gettext('3 HOURS'), '6h_b' => gettext('6 HOURS'), '12h_b' => gettext('12 HOURS'), '1d_b' => gettext('1 DAY'), '4d_b' => gettext('4 DAYS'), '7d_b' => gettext('7 DAYS'), '28d_b' => gettext('28 DAYS'));
		foreach ($interfaces3 as $iface3 => $ifacename3): ?>
		<option value="<?=$iface3;?>"
		<?php if ($iface3 == $pconfig['rm_blocked']) echo "selected"; ?>>
			<?=htmlspecialchars($ifacename3);?></option>
			<?php endforeach; ?>
	</select><br>
	<?php echo gettext("Please select the amount of time you would like hosts to be blocked for."); ?></br>
	<?php printf(gettext("%sHint:%s in most cases, 1 hour is a good choice."), '<span class="red"><strong>', '</strong></span>'); ?></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell"><?php echo gettext("Keep snort settings " .
	"after deinstall"); ?></td>
	<td width="78%" class="vtable"><input name="forcekeepsettings"
		id="forcekeepsettings" type="checkbox" value="yes"
		<?php if ($config['installedpackages']['snortglobal']['forcekeepsettings']=="on") echo "checked"; ?>
		><br>
	<?php echo gettext("Settings will not be removed during deinstall."); ?></td>
</tr>
<tr>
	<td width="22%" valign="top">
	<td width="78%">
		<input name="Submit" type="submit" class="formbtn" value="Save" >
	</td>
</tr>
<tr>
	<td width="22%" valign="top">&nbsp;</td>
	<td width="78%"><span class="vexpl"><span class="red"><strong><?php echo gettext("Note:"); ?><br>
	</strong></span> <?php echo gettext("Changing any settings on this page will affect all " .
	"interfaces. Please, double check if your oink code is correct and " .
				"the type of snort.org account you hold."); ?></span></td>
</tr>
	</table>
</td></tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
