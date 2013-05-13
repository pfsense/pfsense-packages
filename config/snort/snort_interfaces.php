<?php
/*
 * snort_interfaces.php
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

$nocsrf = true;
require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort.inc");

global $g, $rebuild_rules;

$snortdir = SNORTDIR;

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

if (!is_array($config['installedpackages']['snortglobal']['rule']))
	$config['installedpackages']['snortglobal']['rule'] = array();
$a_nat = &$config['installedpackages']['snortglobal']['rule'];
$id_gen = count($config['installedpackages']['snortglobal']['rule']);

if (isset($_POST['del_x'])) {
	/* delete selected rules */
	if (is_array($_POST['rule'])) {
		conf_mount_rw();
		foreach ($_POST['rule'] as $rulei) {
			/* convert fake interfaces to real */
			$if_real = snort_get_real_interface($a_nat[$rulei]['interface']);
			$snort_uuid = $a_nat[$rulei]['uuid'];
			snort_stop($a_nat[$rulei], $if_real);
			exec("/bin/rm -r /var/log/snort/snort_{$if_real}{$snort_uuid}");
			exec("/bin/rm -r {$snortdir}/snort_{$snort_uuid}_{$if_real}");

			unset($a_nat[$rulei]);
		}
		conf_mount_ro();
	  
		/* If all the Snort interfaces are removed, then unset the config array. */
		if (empty($a_nat))
			unset($a_nat);

		write_config();
		sleep(2);
	  
		/* if there are no ifaces remaining do not create snort.sh */
		if (!empty($config['installedpackages']['snortglobal']['rule']))
			snort_create_rc();
		else {
			conf_mount_rw();
			@unlink('/usr/local/etc/rc.d/snort.sh');
			conf_mount_ro();
		}
	  
		sync_snort_package_config();
	  
		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		header("Location: /snort/snort_interfaces.php");
		exit;
	}

}

/* start/stop snort */
if ($_GET['act'] == 'bartoggle' && is_numeric($id)) {
	$snortcfg = $config['installedpackages']['snortglobal']['rule'][$id];
	$if_real = snort_get_real_interface($snortcfg['interface']);
	$if_friendly = snort_get_friendly_interface($snortcfg['interface']);

	if (snort_is_running($snortcfg['uuid'], $if_real, 'barnyard2') == 'no') {
		log_error("Toggle (barnyard starting) for {$if_friendly}({$snortcfg['descr']})...");
		sync_snort_package_config();
		snort_barnyard_start($snortcfg, $if_real);
	} else {
		log_error("Toggle (barnyard stopping) for {$if_friendly}({$snortcfg['descr']})...");
		snort_barnyard_stop($snortcfg, $if_real);
	}

	sleep(3); // So the GUI reports correctly
	header("Location: /snort/snort_interfaces.php");
	exit;
}

/* start/stop snort */
if ($_GET['act'] == 'toggle' && is_numeric($id)) {
	$snortcfg = $config['installedpackages']['snortglobal']['rule'][$id];
	$if_real = snort_get_real_interface($snortcfg['interface']);
	$if_friendly = snort_get_friendly_interface($snortcfg['interface']);

	if (snort_is_running($snortcfg['uuid'], $if_real) == 'yes') {
		log_error("Toggle (snort stopping) for {$if_friendly}({$snortcfg['descr']})...");
		snort_stop($snortcfg, $if_real);

		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
	} else {
		log_error("Toggle (snort starting) for {$if_friendly}({$snortcfg['descr']})...");

		/* set flag to rebuild interface rules before starting Snort */
		$rebuild_rules = "on";
		sync_snort_package_config();
		$rebuild_rules = "off";
		snort_start($snortcfg, $if_real);

		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
	}
	sleep(3); // So the GUI reports correctly
	header("Location: /snort/snort_interfaces.php");
	exit;
}

$pgtitle = "Services: $snort_package_version";
include_once("head.inc");

?>
<body link="#000000" vlink="#000000" alink="#000000">

<?php
include_once("fbegin.inc");
if ($pfsense_stable == 'yes')
	echo '<p class="pgtitle">' . $pgtitle . '</p>';
?>

<form action="snort_interfaces.php" method="post" enctype="multipart/form-data" name="iform" id="iform">
<?php
	/* Display Alert message */
	if ($input_errors)
		print_input_errors($input_errors); // TODO: add checks

	if ($savemsg)
		print_info_box($savemsg);

	//if (file_exists($d_snortconfdirty_path)) {
	if ($d_snortconfdirty_path_ls != '') {
		echo '<p>';

		if($savemsg)
			print_info_box_np("{$savemsg}");
		else {
			print_info_box_np(gettext(
			'The Snort configuration has changed for one or more interfaces.<br>' .
			'You must apply the changes in order for them to take effect.<br>'
			));
		}
	}
?>

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
        display_top_tabs($tab_array);
?>
</td></tr>
<tr>
	<td>
	<div id="mainarea2">
	<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr id="frheader">
			<td width="3%" class="list">&nbsp;</td>
			<td width="10%" class="listhdrr"><?php echo gettext("If"); ?></td>
			<td width="13%" class="listhdrr"><?php echo gettext("Snort"); ?></td>
			<td width="10%" class="listhdrr"><?php echo gettext("Performance"); ?></td>
			<td width="10%" class="listhdrr"><?php echo gettext("Block"); ?></td>
			<td width="12%" class="listhdrr"><?php echo gettext("Barnyard2"); ?></td>
			<td width="30%" class="listhdr"><?php echo gettext("Description"); ?></td>
			<td width="3%" class="list">
			<table border="0" cellspacing="0" cellpadding="0">
				<tr>
					<td></td>
					<td align="center" valign="middle"><a href="snort_interfaces_edit.php?id=<?php echo $id_gen;?>"><img
						src="../themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif"
						width="17" height="17" border="0" title="<?php echo gettext('add interface');?>"></a></td>
				</tr>
			</table>
			</td>
		</tr>
<?php $nnats = $i = 0;

// Turn on buffering to speed up rendering
ini_set('output_buffering','true');

// Start buffering to fix display lag issues in IE9 and IE10
ob_start(null, 0);

/* If no interfaces are defined, then turn off the "no rules" warning */
$no_rules_footnote = false;
if ($id_gen == 0)
	$no_rules = false;
else
	$no_rules = true;

foreach ($a_nat as $natent): ?>
	<tr valign="top" id="fr<?=$nnats;?>">
<?php

/* convert fake interfaces to real and check if iface is up */
/* There has to be a smarter way to do this */
	$if_real = snort_get_real_interface($natent['interface']);
	$snort_uuid = $natent['uuid'];
	if (snort_is_running($snort_uuid, $if_real) == 'no')
		$iconfn = 'pass';
	else
		$iconfn = 'block';
	if (snort_is_running($snort_uuid, $if_real, 'barnyard2') == 'no')
		$biconfn = 'pass';
	else
		$biconfn = 'block';

	/* See if interface has any rules defined and set boolean flag */
	$no_rules = true;
	if (isset($natent['customrules']) && !empty($natent['customrules']))
		$no_rules = false;
	if (isset($natent['rulesets']) && !empty($natent['rulesets']))
		$no_rules = false;
	if (isset($natent['ips_policy']) && !empty($natent['ips_policy']))
		$no_rules = false;
	/* Do not display the "no rules" warning if interface disabled */
	if ($natent['enable'] == "off")
		$no_rules = false;
	if ($no_rules)
		$no_rules_footnote = true;
?>
		<td class="listt">
			<input type="checkbox" id="frc<?=$nnats;?>" name="rule[]" value="<?=$i;?>" onClick="fr_bgcolor('<?=$nnats;?>')" style="margin: 0; padding: 0;">
			</td>
		<td class="listr" 
			id="frd<?=$nnats;?>"
			ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
			<?php
				echo snort_get_friendly_interface($natent['interface']);
			?>
		</td>
		<td class="listr" 
			id="frd<?=$nnats;?>"
			ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
			<?php
			$check_snort_info = $config['installedpackages']['snortglobal']['rule'][$nnats]['enable'];
			if ($check_snort_info == "on") {
				echo strtoupper("enabled");
				echo "<a href='?act=toggle&id={$i}'>
					<img src='../themes/{$g['theme']}/images/icons/icon_{$iconfn}.gif'
					width='13' height='13' border='0'
					title='" . gettext('click to toggle start/stop snort') . "'></a>";
				echo ($no_rules) ? "&nbsp;<img src=\"../themes/{$g['theme']}/images/icons/icon_frmfld_imp.png\" width=\"15\" height=\"15\" border=\"0\">" : "";
			} else
				echo strtoupper("disabled");
			?>
		</td>
		<td class="listr" 
			id="frd<?=$nnats;?>"
			ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
			<?php
			$check_performance_info = $config['installedpackages']['snortglobal']['rule'][$nnats]['performance'];
			if ($check_performance_info != "") {
				$check_performance = $check_performance_info;
			}else{
				$check_performance = "lowmem";
			}
			?> <?=strtoupper($check_performance);?>
		</td>
		<td class="listr" 
			id="frd<?=$nnats;?>"
			ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
			<?php
			$check_blockoffenders_info = $config['installedpackages']['snortglobal']['rule'][$nnats]['blockoffenders7'];
			if ($check_blockoffenders_info == "on")
			{
				$check_blockoffenders = enabled;
			} else {
				$check_blockoffenders = disabled;
			}
			?> <?=strtoupper($check_blockoffenders);?>
		</td>
		<td class="listr" 
			id="frd<?=$nnats;?>"
			ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
			<?php
			$check_snortbarnyardlog_info = $config['installedpackages']['snortglobal']['rule'][$nnats]['barnyard_enable'];
			if ($check_snortbarnyardlog_info == "on") {
				echo strtoupper("enabled");
				echo "<a href='?act=bartoggle&id={$i}'>
					<img src='../themes/{$g['theme']}/images/icons/icon_{$biconfn}.gif'
					width='13' height='13' border='0'
					title='" . gettext('click to toggle start/stop barnyard') . "'></a>";
			} else
				echo strtoupper("disabled");
			?>
		</td>
		<td class="listbg" 
			ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
			<font color="#ffffff"> <?=htmlspecialchars($natent['descr']);?>&nbsp;
		</td>
		<td valign="middle" class="list" nowrap>
			<table border="0" cellspacing="0" cellpadding="0">
				<tr>
					<td><a href="snort_interfaces_edit.php?id=<?=$i;?>"><img
						src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif"
						width="17" height="17" border="0" title="<?php echo gettext('edit interface'); ?>"></a>
					</td>
				</tr>
			</table>
		</td>	
		</tr>
		<?php $i++; $nnats++; endforeach; ob_end_flush(); ?>
			<tr>
				<td class="list"></td>
				<td class="list" colspan="6">
					<?php if ($no_rules_footnote): ?><br><img src="../themes/<?= $g['theme']; ?>/images/icons/icon_frmfld_imp.png" width="15" height="15" border="0">
						<span class="red">&nbsp;&nbsp <?php echo gettext("WARNING: Marked interface currently has no rules defined for Snort"); ?></span>
					<?php else: ?>&nbsp;
					<?php endif; ?>					 
				</td>
				<td class="list" valign="middle" nowrap>
					<table border="0" cellspacing="0" cellpadding="0">
						<tr>
							<td><?php if ($nnats == 0): ?><img
								src="../themes/<?= $g['theme']; ?>/images/icons/icon_x_d.gif"
								width="17" height="17" title="<?php echo gettext("delete selected interface"); ?>" border="0"><?php else: ?>
								<input name="del" type="image"
								src="../themes/<?= $g['theme']; ?>/images/icons/icon_x.gif"
								width="17" height="17" title="<?php echo gettext("delete selected interface"); ?>"
								onclick="return confirm('Do you really want to delete the selected Snort mapping?')"><?php endif; ?></td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
		</div>
		</td>
	</tr>
</table>

<br>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>
			<table class="tabcont" width="100%" border="0" cellpadding="1" cellspacing="1">
				<tr>
					<td colspan="3"><span class="red"><strong><?php echo gettext("Note:"); ?></strong></span> <br>
						<?php echo gettext('This is the <strong>Snort Menu</strong> where you can see an over ' .
						'view of all your interface settings.  ' .
						'Please visit the <strong>Global Settings</strong> tab before adding ' . 'an interface.'); ?>
					</td>
				</tr>
				<tr>
					<td colspan="3"><br>
					</td>
				</tr>
				<tr>
					<td colspan="3"><span class="red"><strong><?php echo gettext("Warning:"); ?></strong></span><br>
						<strong><?php echo gettext("New settings will not take effect until interface restart."); ?></strong>
					</td>
				</tr>
				<tr>
					<td colspan="3"><br>
					</td>
				</tr>
				<tr>
					<td><strong>Click</strong> on the <img src="../themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif"
						width="17" height="17" border="0" title="<?php echo gettext("Add Icon"); ?>"> icon to add 
						an interface.
					</td>
					<td width="3%">&nbsp;
					</td>
					<td><strong>Click</strong> on the <img src="../themes/<?= $g['theme']; ?>/images/icons/icon_pass.gif"
						width="13" height="13" border="0" title="<?php echo gettext("Start Icon"); ?>"> icon to <strong>start</strong>
						snort and barnyard2.
					</td>
				</tr>
				<tr>
					<td><strong>Click</strong> on the <img src="../themes/<?= $g['theme']; ?>/images/icons/icon_e.gif"
						width="17" height="17" border="0" title="<?php echo gettext("Edit Icon"); ?>"> icon to edit 
						an interface and settings.
					<td width="3%">&nbsp;
					</td>
					<td><strong>Click</strong> on the <img src="../themes/<?= $g['theme']; ?>/images/icons/icon_block.gif"
						width="13" height="13" border="0" title="<?php echo gettext("Stop Icon"); ?>"> icon to <strong>stop</strong>
						snort and barnyard2.
					</td>
				</tr>
				<tr>
					<td colspan="3"><strong> Click</strong> on the <img src="../themes/<?= $g['theme']; ?>/images/icons/icon_x.gif"
						width="17" height="17" border="0" title="<?php echo gettext("Delete Icon"); ?>"> icon to
						delete an interface and settings.
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
</form>
<?php
include("fend.inc");
?>
</body>
</html>
