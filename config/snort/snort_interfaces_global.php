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
$pconfig['snortalertlogtype'] = $config['installedpackages']['snortglobal']['snortalertlogtype'];
$pconfig['forcekeepsettings'] = $config['installedpackages']['snortglobal']['forcekeepsettings'];

/* if no errors move foward */
if (!$input_errors) {

	if ($_POST["Submit"]) {

		$config['installedpackages']['snortglobal']['snortdownload'] = $_POST['snortdownload'];
		$config['installedpackages']['snortglobal']['oinkmastercode'] = $_POST['oinkmastercode'];
		$config['installedpackages']['snortglobal']['emergingthreats'] = $_POST['emergingthreats'] ? 'on' : 'off';
		$config['installedpackages']['snortglobal']['rm_blocked'] = $_POST['rm_blocked'];
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
		$config['installedpackages']['snortglobal']['snortalertlogtype'] = $_POST['snortalertlogtype'];
		$config['installedpackages']['snortglobal']['forcekeepsettings'] = $_POST['forcekeepsettings'] ? 'on' : 'off';

		$retval = 0;

		write_config();

		/* create whitelist and homenet file  then sync files */
		sync_snort_package_config();

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


if ($_POST["Reset"]) {

	function snort_deinstall_settings() {
		global $config, $g, $id, $if_real, $snortdir;

		exec("/usr/usr/bin/killall snort");
		sleep(2);
		exec("/usr/usr/bin/killall -9 snort");
		sleep(2);
		exec("/usr/usr/bin/killall barnyard2");
		sleep(2);
		exec("/usr/usr/bin/killall -9 barnyard2");
		sleep(2);

		/* Remove snort cron entries Ugly code needs smoothness*/
		if (!function_exists('snort_deinstall_cron')) {
			function snort_deinstall_cron($cronmatch) {
				global $config, $g;


				if(!$config['cron']['item'])
					return;

				$x=0;
				$is_installed = false;
				foreach($config['cron']['item'] as $item) {
					if (strstr($item['command'], $cronmatch)) {
						$is_installed = true;
						break;
					}
					$x++;
				}
				if($is_installed == true)
					unset($config['cron']['item'][$x]);

				configure_cron();
			}
		}

		snort_deinstall_cron("snort2c");
		snort_deinstall_cron("snort_check_for_rule_updates.php");


		/* Unset snort registers in conf.xml IMPORTANT snort will not start with out this */
		/* Keep this as a last step */
		unset($config['installedpackages']['snortglobal']);

		/* remove all snort iface dir */
		exec("rm -r {$snortdir}/snort_*");
		exec('rm /var/log/snort/*');
	}

	snort_deinstall_settings();
	write_config(); /* XXX */

	header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
	header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
	header( 'Cache-Control: no-store, no-cache, must-revalidate' );
	header( 'Cache-Control: post-check=0, pre-check=0', false );
	header( 'Pragma: no-cache' );
	header("Location: /snort/snort_interfaces_global.php");
	exit;
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
	<td colspan="2" valign="top" class="listtopic">Please Choose The
			Type Of Rules You Wish To Download</td>
</tr>
	<td width="22%" valign="top" class="vncell">Install Snort.org rules</td>
	<td width="78%" class="vtable">
		<table cellpadding="0" cellspacing="0">
		<tr>
			<td colspan="2"><input name="snortdownload" type="radio"
				id="snortdownload" value="off" onClick="enable_change(false)"
<?php if($pconfig['snortdownload']=='off' || $pconfig['snortdownload']=='') echo 'checked'; ?>>
			Do <strong>NOT</strong> Install</td>
		</tr>
		<tr>
			<td colspan="2"><input name="snortdownload" type="radio"
				id="snortdownload" value="on" onClick="enable_change(false)"
<?php if($pconfig['snortdownload']=='on') echo 'checked'; ?>> Install
			Basic Rules or Premium rules <br>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a
				href="https://www.snort.org/signup" target="_blank">Sign Up for a
			Basic Rule Account</a><br>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a
				href="http://www.snort.org/vrt/buy-a-subscription"
				target="_blank">Sign Up for Sourcefire VRT Certified Premium
			Rules. This Is Highly Recommended</a></td>
		</tr>
		<tr>
			<td>&nbsp;</td>
		</tr>
	</table>
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="optsect_t2">Oinkmaster code</td>
		</tr>
		<tr>
			<td class="vncell" valign="top">Code</td>
			<td class="vtable"><input name="oinkmastercode" type="text"
				class="formfld" id="oinkmastercode" size="52"
				value="<?=htmlspecialchars($pconfig['oinkmastercode']);?>"><br>
			Obtain a snort.org Oinkmaster code and paste here.</td>
	
	</table>

</tr>
<tr>
	<td width="22%" valign="top" class="vncell">Install <strong>Emergingthreats</strong>
	rules</td>
	<td width="78%" class="vtable"><input name="emergingthreats"
		type="checkbox" value="yes"
		<?php if ($config['installedpackages']['snortglobal']['emergingthreats']=="on") echo "checked"; ?>
		onClick="enable_change(false)"><br>
	Emerging Threats is an open source community that produces fastest
	moving and diverse Snort Rules.</td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell">Update rules
	automatically</td>
	<td width="78%" class="vtable">
		<select name="autorulesupdate7" class="formselect" id="autorulesupdate7">
		<?php
		$interfaces3 = array('never_up' => 'NEVER', '6h_up' => '6 HOURS', '12h_up' => '12 HOURS', '1d_up' => '1 DAY', '4d_up' => '4 DAYS', '7d_up' => '7 DAYS', '28d_up' => '28 DAYS');
		foreach ($interfaces3 as $iface3 => $ifacename3): ?>
		<option value="<?=$iface3;?>"
		<?php if ($iface3 == $pconfig['autorulesupdate7']) echo "selected"; ?>>
			<?=htmlspecialchars($ifacename3);?></option>
			<?php endforeach; ?>
	</select><br>
	<span class="vexpl">Please select the update times for rules.<br>
	Hint: in most cases, every 12 hours is a good choice.</span></td>
</tr>
<tr>
	<td colspan="2" valign="top" class="listtopic">General Settings</td>
</tr>

<tr>
<?php $snortlogCurrentDSKsize = round(exec('df -k /var | grep -v "Filesystem" | awk \'{print $4}\'') / 1024); ?>
	<td width="22%" valign="top" class="vncell">Log Directory Size
	Limit<br/>
	<br/>
	<br/>
	<span class="red"><strong>Note</span>:</strong><br>
	Available space is <strong><?php echo $snortlogCurrentDSKsize; ?>MB</strong></td>
	<td width="78%" class="vtable">
	<table cellpadding="0" cellspacing="0">
		<tr>
			<td colspan="2"><input name="snortloglimit" type="radio"
				id="snortloglimit" value="on" onClick="enable_change(false)"
<?php if($pconfig['snortloglimit']=='on') echo 'checked'; ?>>
			<strong>Enable</strong> directory size limit (<strong>Default</strong>)</td>
		</tr>
		<tr>
			<td colspan="2"><input name="snortloglimit" type="radio"
				id="snortloglimit" value="off" onClick="enable_change(false)"
<?php if($pconfig['snortloglimit']=='off') echo 'checked'; ?>> <strong>Disable</strong>
			directory size limit<br>
			<br>
			<span class="red"><strong>Warning</span>:</strong> Nanobsd
			should use no more than 10MB of space.</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
		</tr>
	</table>
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td class="vncell3">Size in <strong>MB</strong></td>
			<td class="vtable"><input name="snortloglimitsize" type="text"
				class="formfld" id="snortloglimitsize" size="7"
				value="<?=htmlspecialchars($pconfig['snortloglimitsize']);?>">
			Default is <strong>20%</strong> of available space.</td>
	
	</table>

</tr>

<tr>
	<td width="22%" valign="top" class="vncell">Remove blocked hosts
	every</td>
	<td width="78%" class="vtable">
		<select name="rm_blocked" class="formselect" id="rm_blocked">
		<?php
		$interfaces3 = array('never_b' => 'NEVER', '1h_b' => '1 HOUR', '3h_b' => '3 HOURS', '6h_b' => '6 HOURS', '12h_b' => '12 HOURS', '1d_b' => '1 DAY', '4d_b' => '4 DAYS', '7d_b' => '7 DAYS', '28d_b' => '28 DAYS');
		foreach ($interfaces3 as $iface3 => $ifacename3): ?>
		<option value="<?=$iface3;?>"
		<?php if ($iface3 == $pconfig['rm_blocked']) echo "selected"; ?>>
			<?=htmlspecialchars($ifacename3);?></option>
			<?php endforeach; ?>
	</select><br>
	<span class="vexpl">Please select the amount of time you would like
	hosts to be blocked for.<br>
	Hint: in most cases, 1 hour is a good choice.</span></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell">Alerts file description
	type</td>
	<td width="78%" class="vtable">
		<select name="snortalertlogtype" class="formselect" id="snortalertlogtype">
		<?php
		$logtype = array('full' => 'FULL', 'fast' => 'SHORT');
		foreach ($logtype as $logt => $logdescr): ?>
		<option value="<?=$logt;?>"
		<?php if ($logt == $pconfig['snortalertlogtype']) echo "selected"; ?>>
			<?=htmlspecialchars($logdescr);?></option>
		<?php endforeach; ?>
	</select><br>
	<span class="vexpl">Please choose the type of Alert logging you will
	like see in your alert file.<br>
	Hint: Best pratice is to chose full logging.</span>&nbsp;<span
		class="red"><strong>WARNING:</strong></span>&nbsp;<strong>On
	change, alert file will be cleared.</strong></td>
</tr>
<tr>
	<td width="22%" valign="top" class="vncell">Keep snort settings
	after deinstall</td>
	<td width="78%" class="vtable"><input name="forcekeepsettings"
		id="forcekeepsettings" type="checkbox" value="yes"
		<?php if ($config['installedpackages']['snortglobal']['forcekeepsettings']=="on") echo "checked"; ?>
		onClick="enable_change(false)"><br>
	Settings will not be removed during deinstall.</td>
</tr>
<tr>
	<td width="22%" valign="top"><input name="Reset" type="submit"
		class="formbtn" value="Reset"
		onclick="return confirm('Do you really want to delete all global and interface settings?')"><span
		class="red"><strong>&nbsp;WARNING:</strong><br>
	This will reset all global and interface settings.</span></td>
	<td width="78%"><input name="Submit" type="submit" class="formbtn"
		value="Save" onClick="enable_change(true)"> 
	</td>
</tr>
<tr>
	<td width="22%" valign="top">&nbsp;</td>
	<td width="78%"><span class="vexpl"><span class="red"><strong>Note:<br>
	</strong></span> Changing any settings on this page will affect all
	interfaces. Please, double check if your oink code is correct and
				the type of snort.org account you hold.</span></td>
</tr>
	</table>
</td></tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
