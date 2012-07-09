<?php
/* $Id$ */
/*
 snort_blocked.php
 Copyright (C) 2006 Scott Ullrich
 All rights reserved.

 Modified for the Pfsense snort package v. 1.8+
 Copyright (C) 2009 Robert Zelaya Sr. Developer

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
require_once("/usr/local/pkg/snort/snort.inc");

if (!is_array($config['installedpackages']['snortglobal']['alertsblocks']))
	$config['installedpackages']['snortglobal']['alertsblocks'] = array();

$pconfig['brefresh'] = $config['installedpackages']['snortglobal']['alertsblocks']['brefresh'];
$pconfig['blertnumber'] = $config['installedpackages']['snortglobal']['alertsblocks']['blertnumber'];

if (empty($pconfig['blertnumber']))
	$bnentries = '500';
else
	$bnentries = $pconfig['blertnumber'];

if ($_POST['todelete'] || $_GET['todelete']) {
	$ip = "";
	if($_POST['todelete'])
		$ip = $_POST['todelete'];
	else if($_GET['todelete'])
		$ip = $_GET['todelete'];
	if (is_ipaddr($ip))
		exec("/sbin/pfctl -t snort2c -T delete {$ip}");
}

if ($_POST['remove']) {
	exec("/sbin/pfctl -t snort2c -T flush");
	header("Location: /snort/snort_blocked.php");
	exit;
}

/* TODO: build a file with block ip and disc */
if ($_POST['download'])
{
	$blocked_ips_array_save = "";
	exec('/sbin/pfctl -t snort2c -T show', $blocked_ips_array_save);
	/* build the list */
	if (is_array($blocked_ips_array_save) && count($blocked_ips_array_save) > 0) {
		ob_start(); //important or other posts will fail
		$save_date = exec('/bin/date "+%Y-%m-%d-%H-%M-%S"');
		$file_name = "snort_blocked_{$save_date}.tar.gz";
		exec('/bin/mkdir -p /tmp/snort_blocked');
		file_put_contents("/tmp/snort_blocked/snort_block.pf", "");
		foreach($blocked_ips_array_save as $counter => $fileline) {
			if (empty($fileline))
				continue;
			$fileline = trim($fileline, " \n\t");
			file_put_contents("/tmp/snort_blocked/snort_block.pf", "{$fileline}\n", FILE_APPEND);
		}

		exec("/usr/bin/tar cf /tmp/{$file_name} /tmp/snort_blocked");

		if(file_exists("/tmp/{$file_name}")) {
			header("Expires: Mon, 26 Jul 1997 05:00:00 GMT\n");
			header("Pragma: private"); // needed for IE
			header("Cache-Control: private, must-revalidate"); // needed for IE
			header('Content-type: application/force-download');
			header('Content-Transfer-Encoding: Binary');
			header("Content-length: " . filesize("/tmp/{$file_name}"));
			header("Content-disposition: attachment; filename = {$file_name}");
			readfile("/tmp/{$file_name}");
			ob_end_clean(); //importanr or other post will fail
			@unlink("/tmp/{$file_name}");
			exec("/bin/rm -fr /tmp/snort_blocked");
		} else
			$savemsg = "An error occurred while createing archive";
	} else
		$savemsg = "No content on snort block list";
}

if ($_POST['save'])
{
	/* no errors */
	if (!$input_errors) {
		$config['installedpackages']['snortglobal']['alertsblocks']['brefresh'] = $_POST['brefresh'] ? 'on' : 'off';
		$config['installedpackages']['snortglobal']['alertsblocks']['blertnumber'] = $_POST['blertnumber'];

		write_config();

		header("Location: /snort/snort_blocked.php");
		exit;
	}

}

/* build filter funcs */
function get_snort_alert_ip_src($fileline)
{
	/* SRC IP */
	$re1='.*?';   # Non-greedy match on filler
	$re2='((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))(?![\\d])'; # IPv4 IP Address 1

	if ($c=preg_match_all ("/".$re1.$re2."/is", $fileline, $matches4))
		return $matches4[1][0];

	return "";
}

function get_snort_alert_disc($fileline)
{
	/* disc */
	if (preg_match("/\[\*\*\]\s+(\[[0-9:]+\])\s+(.+)\s+(\[\*\*\])/", $fileline, $matches))
		return "{$matches[2]}";

	return "";
}

/* tell the user what settings they have */
$blockedtab_msg_chk = $config['installedpackages']['snortglobal']['rm_blocked'];
if ($blockedtab_msg_chk == "1h_b") {
	$blocked_msg = "hour";
}
if ($blockedtab_msg_chk == "3h_b") {
	$blocked_msg = "3 hours";
}
if ($blockedtab_msg_chk == "6h_b") {
	$blocked_msg = "6 hours";
}
if ($blockedtab_msg_chk == "12h_b") {
	$blocked_msg = "12 hours";
}
if ($blockedtab_msg_chk == "1d_b") {
	$blocked_msg = "day";
}
if ($blockedtab_msg_chk == "4d_b") {
	$blocked_msg = "4 days";
}
if ($blockedtab_msg_chk == "7d_b") {
	$blocked_msg = "7 days";
}
if ($blockedtab_msg_chk == "28d_b") {
	$blocked_msg = "28 days";
}

if ($blockedtab_msg_chk != "never_b")
{
	$blocked_msg_txt = "Hosts are removed every <strong>$blocked_msg</strong>.";
}else{
	$blocked_msg_txt = "Settings are set to never <strong>remove</strong> hosts.";
}

$pgtitle = "Services: Snort Blocked Hosts";
include_once("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">

<?php

include_once("fbegin.inc");

/* refresh every 60 secs */
if ($pconfig['brefresh'] == 'on')
	echo "<meta http-equiv=\"refresh\" content=\"60;url=/snort/snort_blocked.php\" />\n";
?>

<div class="body2"><?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>

<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[0] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[1] = array(gettext("Global Settings"), false, "/snort/snort_interfaces_global.php");
        $tab_array[2] = array(gettext("Updates"), false, "/snort/snort_download_updates.php");
        $tab_array[3] = array(gettext("Alerts"), false, "/snort/snort_alerts.php");
        $tab_array[4] = array(gettext("Blocked"), true, "/snort/snort_blocked.php");
        $tab_array[5] = array(gettext("Whitelists"), false, "/snort/snort_interfaces_whitelist.php");
        $tab_array[6] = array(gettext("Suppress"), false, "/snort/snort_interfaces_suppress.php");
        display_top_tabs($tab_array);
?>
</td></tr>
	<tr>
		<td>
		<div id="mainarea2">

		<table id="maintable" class="tabcont" width="100%" border="0"
			cellpadding="0" cellspacing="0">
			<tr>
				<td width="22%" colspan="0" class="listtopic">Last <?=$bnentries;?>
				Blocked.</td>
				<td width="78%" class="listtopic">This page lists hosts that have
				been blocked by Snort.&nbsp;&nbsp;<?=$blocked_msg_txt;?></td>
			</tr>
			<tr>
				<td width="22%" class="vncell">Save or Remove Hosts</td>
				<td width="78%" class="vtable">
				<form action="/snort/snort_blocked.php" method="post"><input
					name="download" type="submit" class="formbtn" value="Download"> All
				blocked hosts will be saved. <input name="remove" type="submit"
					class="formbtn" value="Clear"> <span class="red"><strong>Warning:</strong></span>
				all hosts will be removed.</form>
				</td>
			</tr>
			<tr>
				<td width="22%" class="vncell">Auto Refresh and Log View</td>
				<td width="78%" class="vtable">
				<form action="/snort/snort_blocked.php" method="post"><input
					name="save" type="submit" class="formbtn" value="Save"> Refresh <input
					name="brefresh" type="checkbox" value="on"
					<?php if ($config['installedpackages']['snortglobal']['alertsblocks']['brefresh']=="on" || $config['installedpackages']['snortglobal']['alertsblocks']['brefresh']=='') echo "checked"; ?>>
				<strong>Default</strong> is <strong>ON</strong>. <input
					name="blertnumber" type="text" class="formfld" id="blertnumber"
					size="5" value="<?=htmlspecialchars($bnentries);?>"> Enter the
				number of blocked entries to view. <strong>Default</strong> is <strong>500</strong>.
				</form>
				</td>
			</tr>
		</table>
		</div>
		<br>
		</td>
	</tr>

	<table class="tabcont" width="100%" border="0" cellspacing="0"
		cellpadding="0">
		<tr>
			<td>
			<table id="sortabletable1" class="sortable" width="100%" border="0"
				cellpadding="0" cellspacing="0">
				<tr id="frheader">
					<td width="5%" class="listhdrr">Remove</td>
					<td class="listhdrr">#</td>
					<td class="listhdrr">IP</td>
					<td class="listhdrr">Alert Description</td>
				</tr>
		<?php
			/* set the arrays */
			$blocked_ips = "";
			exec('/sbin/pfctl -t snort2c -T show', $blocked_ips);
			$blocked_ips_array = array();
		if (!empty($blocked_ips)) {
			$blocked_ips_array = array();
			if (is_array($blocked_ips)) {
				foreach ($blocked_ips as $blocked_ip) {
					if (empty($blocked_ip))
						continue;
					$blocked_ips_array[] = trim($blocked_ip, " \n\t");
				}
			}
			$input = array();
			$alert_ip_src_array = array();
			foreach (glob("/var/log/snort/*/alert") as $alert) {
				if ($pconfig['snortalertlogtype'] == 'full')
					$alerts_array = array_reverse(explode("\n\n", file_get_contents($alert)));
				else
					$alerts_array = array_reverse(file($alert));
				/* build the list and compare blocks to alerts */
				foreach($alerts_array as $counter => $fileline) {
					if (empty($fileline))
						continue;

					$alert_ip_src =  get_snort_alert_ip_src($fileline);
					$alert_ip_disc = get_snort_alert_disc($fileline);
					$alert_ip_src_array[] = $alert_ip_src;
					if (!empty($blocked_ips_array) && in_array("$alert_ip_src", $blocked_ips_array)) {
						if (!isset($input[$alert_ip_src]))
							$input[$alert_ip_src] = "{$alert_ip_disc}\n";
					}
				}
			}

			foreach($blocked_ips_array as $blocked_ip) {
				if (is_ipaddr($blocked_ip) && !isset($input[$blocked_ip]))
					$input[$blocked_ip] = "N\A\n";
			}

			/* buil final list, preg_match, buld html */
			$counter = 0;
			foreach($input as $blocked_ip => $blocked_desc) {
				if($counter > $bnentries)
					break;
				else
					$counter++;

				/* use one echo to do the magic*/
				echo "<tr>
			<td align=\"center\" valign=\"top\"'><a href='snort_blocked.php?todelete=" . trim(urlencode($blocked_ip)) . "'>
			<img title=\"Delete\" border=\"0\" name='todelete' id='todelete' alt=\"Delete\" src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"></a></td>
			<td>&nbsp;{$counter}</td>
			<td>&nbsp;{$blocked_ip}</td>
			<td>&nbsp;{$blocked_desc}</td>
			</tr>\n";

			}

			echo '</table>' . "\n";
			echo "\n<tr><td colspan='3' align=\"center\" valign=\"top\">{$counter} items listed.</td></tr>";
		} else
			echo "\n<tr><td colspan='3' align=\"center\" valign=\"top\"><br><strong>There are currently no items being blocked by snort.</strong></td></tr>";

		?>
			</td>
			</tr>
		</table>
		</td>
	</tr>
</table>
</div>
<?php
include("fend.inc");
?>
</body>
</html>
