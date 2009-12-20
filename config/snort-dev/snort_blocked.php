<?php
/* $Id$ */
/*
	snort_blocked.php
	Copyright (C) 2006 Scott Ullrich
	Copyright (C) 2009 Robert Zelaya
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

require("guiconfig.inc");
require("/usr/local/pkg/snort/snort.inc");

if($_POST['todelete'] or $_GET['todelete']) {
	if($_POST['todelete'])
		$ip = $_POST['todelete'];
	if($_GET['todelete'])
		$ip = $_GET['todelete'];
	exec("/sbin/pfctl -t snort2c -T delete {$ip}");
}

if ($_POST['remove']) {

exec("/sbin/pfctl -t snort2c -T flush");
sleep(1);
header("Location: /snort/snort_blocked.php");

}


$pgtitle = "Snort: Services: Snort Blocked Hosts";
include("head.inc");

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

?>

<body link="#000000" vlink="#000000" alink="#000000">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></p>

<form action="snort_blocked.php" method="post" name="iform" id="iform">
<script src="/row_toggle.js" type="text/javascript"></script>
<script src="/javascript/sorttable.js" type="text/javascript"></script>
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
   <tr>
   		<td>
<?php
	$tab_array = array();
	$tab_array[] = array("Snort Inertfaces", false, "/snort/snort_interfaces.php");
	$tab_array[] = array("Global Settings", false, "/snort/snort_interfaces_global.php");
	$tab_array[] = array("Rule Updates", false, "/snort/snort_download_rules.php");
	$tab_array[] = array("Alerts", false, "/snort/snort_alerts.php");
    $tab_array[] = array("Blocked", true, "/snort/snort_blocked.php");
	$tab_array[] = array("Whitelists", false, "/pkg.php?xml=/snort/snort_whitelist.xml");
	$tab_array[] = array("Help & Info", false, "/snort/snort_help_info.php");
	display_top_tabs($tab_array);
?>
  		</td>
  </tr>
  <tr>
    <td>
		<div id="mainarea">
			<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
				<tr> 
					<td>
					<?php
					if ($blockedtab_msg_chk != "never_b")
					{
						echo "<span class=\"red\"><strong>Note:</strong></span><br>This page lists hosts that have been blocked by Snort. Hosts are automatically deleted every <strong>$blocked_msg</strong>.<br><br>";
					}else{
						echo "<span class=\"red\"><strong>Note:</strong></span><br>This page lists hosts that have been blocked by Snort. Snort package settings are set to never <strong>remove</strong> hosts.<br><br>";
					}
					?>
					<input name="remove" type="submit" class="formbtn" value="Remove"> all blocked hosts.<br><br>
						<table id="sortabletable1" class="sortable" width="100%" border="0" cellpadding="0" cellspacing="0">
						    <tr id="frheader">
								<td width="5%" class="listhdrr">Remove</td>
								<td class="listhdrr">IP</td>
								<td class="listhdrr">Alert Description</td>
							</tr>
<?php

	$associatealertip = $config['installedpackages']['snortglobal']['associatealertip'];
	// $ips = `/sbin/pfctl -t snort2c -T show`;
	/* this improves loading of ips by a factor of 10 */
	exec('/sbin/pfctl -t snort2c -T show > /tmp/snort_block.cache');
	sleep(1);
	$ips_array = file('/tmp/snort_block.cache');
	//$ips_array = split("\n", $ips);
	$counter = 0;
	foreach($ips_array as $ip) {
		if(!$ip)
			continue;
		$ww_ip = str_replace(" ", "", $ip);
		$counter++;
		if($associatealertip)
			$alert_description = get_snort_alert($ww_ip);
		else
			$alert_description = "";
		echo "\n<tr>";
		echo "\n<td align=\"center\" valign=\"top\"'><a href='snort_blocked.php?todelete=" . trim(urlencode($ww_ip)) . "'>";
		echo "\n<img title=\"Delete\" border=\"0\" name='todelete' id='todelete' alt=\"Delete\" src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"></a></td>";
		echo "\n<td>&nbsp;{$ww_ip}</td>";
		echo "\n<td>&nbsp;{$alert_description}<!-- |{$ww_ip}| get_snort_alert($ww_ip); --></td>";
		echo "\n</tr>";
	}
	echo "\n<tr><td colspan='3'>&nbsp;</td></tr>";
	if($counter < 1)
		echo "\n<tr><td colspan='3' align=\"center\" valign=\"top\">There are currently no items being blocked by snort.</td></tr>";
	else
		echo "\n<tr><td colspan='3' align=\"center\" valign=\"top\">{$counter} items listed.</td></tr>";

?>

						</table>
		    		</td>
		  		</tr>
			</table>
		</div>
	</td>
  </tr>
</table>

</form>

<p>

<?php

if ($blockedtab_msg_chk != "never_b")
{
echo "This page lists hosts that have been blocked by Snort. Hosts are automatically deleted every <strong>$blocked_msg</strong>.";
}else{
echo "This page lists hosts that have been blocked by Snort. Snort package settings are set to never <strong>remove</strong> hosts.";
}

?>

<?php include("fend.inc"); ?>

</body>
</html>

<?php

/* write out snort cache */
conf_mount_rw();
write_snort_config_cache($snort_config);
conf_mount_ro();
?>