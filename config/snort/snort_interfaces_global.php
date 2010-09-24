<?php
/*
 snort_interfaces_global.php
 part of m0n0wall (http://m0n0.ch/wall)

 Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
 All rights reserved.

 Copyright (C) 2008-2009 Robert Zelaya
 Modified for the Pfsense snort package.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */


require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");
require_once("/usr/local/pkg/snort/snort.inc");

global $g;

$d_snort_global_dirty_path = '/var/run/snort_global.dirty';

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

/* this will exec when alert says apply */
if ($_POST['apply']) {

	if (file_exists("$d_snort_global_dirty_path")) {
		conf_mount_rw();
			
		/* create whitelist and homenet file  then sync files */
		sync_snort_package_empty();
		sync_snort_package();

		unlink("$d_snort_global_dirty_path");
			
		write_config();
		conf_mount_ro();
	}
}



/* if no errors move foward */
if (!$input_errors) {

	if ($_POST["Submit"]) {

		$config['installedpackages']['snortglobal']['snortdownload'] = $_POST['snortdownload'];
		$config['installedpackages']['snortglobal']['oinkmastercode'] = $_POST['oinkmastercode'];
		$config['installedpackages']['snortglobal']['emergingthreats'] = $_POST['emergingthreats'] ? on : off;
		$config['installedpackages']['snortglobal']['rm_blocked'] = $_POST['rm_blocked'];
		$config['installedpackages']['snortglobal']['snortloglimit'] = $_POST['snortloglimit'];
		$config['installedpackages']['snortglobal']['snortloglimitsize'] = $_POST['snortloglimitsize'];
		$config['installedpackages']['snortglobal']['autorulesupdate7'] = $_POST['autorulesupdate7'];
		$config['installedpackages']['snortglobal']['snortalertlogtype'] = $_POST['snortalertlogtype'];
		$config['installedpackages']['snortglobal']['forcekeepsettings'] = $_POST['forcekeepsettings'] ? on : off;

		write_config();
		sleep(2);

		$retval = 0;


		$snort_snortloglimit_info_ck = $config['installedpackages']['snortglobal']['snortloglimit'];
		if ($snort_snortloglimit_info_ck == 'on') {
			snort_snortloglimit_install_cron('');
			snort_snortloglimit_install_cron('true');
		}

		if ($snort_snortloglimit_info_ck == 'off') {
			snort_snortloglimit_install_cron('');
		}


		/* set the snort block hosts time IMPORTANT */
		$snort_rm_blocked_info_ck = $config['installedpackages']['snortglobal']['rm_blocked'];
		if ($snort_rm_blocked_info_ck == "never_b")
		$snort_rm_blocked_false = "";
		else
		$snort_rm_blocked_false = "true";

		if ($snort_rm_blocked_info_ck != "")
		{
			snort_rm_blocked_install_cron("");
			snort_rm_blocked_install_cron($snort_rm_blocked_false);
		}

		/* set the snort rules update time */
		$snort_rules_up_info_ck = $config['installedpackages']['snortglobal']['autorulesupdate7'];
		if ($snort_rules_up_info_ck == "never_up")
		$snort_rules_up_false = "";
		else
		$snort_rules_up_false = "true";

		if ($snort_rules_up_info_ck != "")
		{
			snort_rules_up_install_cron("");
			snort_rules_up_install_cron($snort_rules_up_false);
		}


		touch($d_snort_global_dirty_path);
		$savemsg = get_std_save_message($retval);
		write_config();

		sync_snort_package();

		/* forces page to reload new settings */
		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		header("Location: /snort/snort_interfaces_global.php");

	}
}


if ($_POST["Reset"]) {

	function snort_deinstall_settings() {

		global $config, $g, $id, $if_real;
		conf_mount_rw();


		exec("/usr/usr/bin/killall snort");
		sleep(2);
		exec("/usr/usr/bin/killall -9 snort");
		sleep(2);
		exec("/usr/usr/bin/killall barnyard2");
		sleep(2);
		exec("/usr/usr/bin/killall -9 barnyard2");
		sleep(2);

		/* Remove snort cron entries Ugly code needs smoothness*/
		function snort_rm_blocked_deinstall_cron($should_install)
		{
			global $config, $g;
			conf_mount_rw();

			$is_installed = false;

			if(!$config['cron']['item'])
			return;

			$x=0;
			foreach($config['cron']['item'] as $item)
			{
				if (strstr($item['command'], "snort2c"))
				{
					$is_installed = true;
					break;
				}

				$x++;
					
			}
			if($is_installed == true)
			{
				if($x > 0)
				{
					unset($config['cron']['item'][$x]);
					write_config();
					conf_mount_rw();
				}

				configure_cron();

			}
			conf_mount_ro();

		}

		function snort_rules_up_deinstall_cron($should_install)
		{
			global $config, $g;
			conf_mount_rw();

			$is_installed = false;

			if(!$config['cron']['item'])
			return;

			$x=0;
			foreach($config['cron']['item'] as $item) {
				if (strstr($item['command'], "snort_check_for_rule_updates.php")) {
					$is_installed = true;
					break;
				}
				$x++;
			}
			if($is_installed == true) {
				if($x > 0) {
					unset($config['cron']['item'][$x]);
					write_config();
					conf_mount_rw();
				}
				configure_cron();
			}
		}

		snort_rm_blocked_deinstall_cron("");
		snort_rules_up_deinstall_cron("");


		/* Unset snort registers in conf.xml IMPORTANT snort will not start with out this */
		/* Keep this as a last step */
		unset($config['installedpackages']['snortglobal']);
		write_config();
		conf_mount_rw();

		/* remove all snort iface dir */
		exec('rm -r /usr/local/etc/snort/snort_*');
		exec('rm /var/log/snort/*');

		conf_mount_ro();

	}

	snort_deinstall_settings();

	header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
	header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
	header( 'Cache-Control: no-store, no-cache, must-revalidate' );
	header( 'Cache-Control: post-check=0, pre-check=0', false );
	header( 'Pragma: no-cache' );
	sleep(2);
	header("Location: /snort/snort_interfaces_global.php");

	exit;
}


$pgtitle = 'Services: Snort: Global Settings';
include_once("/usr/local/pkg/snort/snort_head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">

<script>
			jQuery(document).ready(function(){
			
				//Examples of how to assign the ColorBox event to elements
				jQuery(".example8").colorbox({width:"820px", height:"700px", iframe:true, overlayClose:false});
				
			});
		</script>

<?php
echo "{$snort_general_css}\n";
echo "$snort_interfaces_css\n";
?>

<?php include("fbegin.inc"); ?>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img
	src="./images/transparent.gif" border="0"></img></a></div>

<?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>

<noscript>
<div class="alert" ALIGN=CENTER><img
	src="../themes/<?php echo $g['theme']; ?>/images/icons/icon_alert.gif" /><strong>Please
enable JavaScript to view this content
</CENTER></div>
</noscript>


<form action="snort_interfaces_global.php" method="post"
	enctype="multipart/form-data" name="iform" id="iform"><?php
	/* Display Alert message, under form tag or no refresh */
	if ($input_errors) {
		print_input_errors($input_errors); // TODO: add checks
	}

	if (!$input_errors) {
		if (file_exists($d_snort_global_dirty_path)) {

			print_info_box_np2('
			The Snort configuration has changed and snort needs to be restarted on this interface.<br>
			You must apply the changes in order for them to take effect.<br>
			');
		}
	}
	?>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="snorttabs" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="snorttabs">
			<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
			<li class="snorttabs_active"><a
				href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
			<li><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
			<li><a href="/snort/snort_alerts.php"><span>Alerts</span></a></li>
			<li><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
			<li><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
			<li><a href="/snort/snort_interfaces_suppress.php"><span>Suppress</span></a></li>
			<li><a class="example8" href="/snort/help_and_info.php"><span>Help</span></a></li>
		</ul>
		</div>

		</td>
	</tr>

	<tr>
		<td class="tabcont">
		<table id="maintable2" width="100%" border="0" cellpadding="6"
			cellspacing="0">
			<tr>
				<tr>
					<td colspan="2" valign="top" class="listtopic">Please Choose The
					Type Of Rules You Wish To Download</td>
				</tr>
				<td width="22%" valign="top" class="vncell2">Install Snort.org rules</td>
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
						<td class="vncell2" valign="top">Code</td>
						<td class="vtable"><input name="oinkmastercode" type="text"
							class="formfld" id="oinkmastercode" size="52"
							value="<?=htmlspecialchars($pconfig['oinkmastercode']);?>"><br>
						Obtain a snort.org Oinkmaster code and paste here.</td>
				
				</table>
			
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Install <strong>Emergingthreats</strong>
				rules</td>
				<td width="78%" class="vtable"><input name="emergingthreats"
					type="checkbox" value="yes"
					<?php if ($config['installedpackages']['snortglobal']['emergingthreats']=="on") echo "checked"; ?>
					onClick="enable_change(false)"><br>
				Emerging Threats is an open source community that produces fastest
				moving and diverse Snort Rules.</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Update rules
				automatically</td>
				<td width="78%" class="vtable"><select name="autorulesupdate7"
					class="formfld" id="autorulesupdate7">
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
				<td width="22%" valign="top" class="vncell2">Log Directory Size
				Limit<br>
				<br>
				<br>
				<br>
				<br>
				<br>
				<span class="red"><strong>Note</span>:</strong><br>
				Available space is <strong><?php echo $snortlogCurrentDSKsize; ?>MB</strong></td>
				<td width="78%" class="vtable">
				<table cellpadding="0" cellspacing="0">
					<tr>
						<td colspan="2"><input name="snortloglimit" type="radio"
							id="snortloglimit" value="on" onClick="enable_change(false)"
			<?php if($pconfig['snortloglimit']=='on' || $pconfig['snortloglimit']=='') echo 'checked'; ?>>
						<strong>Enable</strong> directory size limit (<strong>Default</strong>)</td>
					</tr>
					<tr>
						<td colspan="2"><input name="snortloglimit" type="radio"
							id="snortloglimit" value="off" onClick="enable_change(false)"
			<?php if($pconfig['snortloglimit']=='off') echo 'checked'; ?>> <strong>Disable</strong>
						directory size limit<br>
						<br>
						<span class="red"><strong>Warning</span>:</strong> Pfsense Nanobsd
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
				<td width="22%" valign="top" class="vncell2">Remove blocked hosts
				every</td>
				<td width="78%" class="vtable"><select name="rm_blocked"
					class="formfld" id="rm_blocked">
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
				<td width="22%" valign="top" class="vncell2">Alerts file description
				type</td>
				<td width="78%" class="vtable"><select name="snortalertlogtype"
					class="formfld" id="snortalertlogtype">
					<?php
					$interfaces4 = array('full' => 'FULL', 'fast' => 'SHORT');
					foreach ($interfaces4 as $iface4 => $ifacename4): ?>
					<option value="<?=$iface4;?>"
					<?php if ($iface4 == $pconfig['snortalertlogtype']) echo "selected"; ?>>
						<?=htmlspecialchars($ifacename4);?></option>
						<?php endforeach; ?>
				</select><br>
				<span class="vexpl">Please choose the type of Alert logging you will
				like see in your alert file.<br>
				Hint: Best pratice is to chose full logging.</span>&nbsp;<span
					class="red"><strong>WARNING:</strong></span>&nbsp;<strong>On
				change, alert file will be cleared.</strong></td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Keep snort settings
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
					value="Save" onClick="enable_change(true)"> <input type="button"
					class="formbtn" value="Cancel" onclick="history.back()"></td>
			</tr>
			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%"><span class="vexpl"><span class="red"><strong>Note:<br>
				</strong></span> Changing any settings on this page will affect all
				interfaces. Please, double check if your oink code is correct and
				the type of snort.org account you hold.</span></td>
			</tr>
		</table>
		</td>
	</tr>
</table>
</form>

</div>

					<?php include("fend.inc"); ?>

					<?php echo "$snort_custom_rnd_box\n"; ?>

</body>
</html>
