<?php
/* $Id$ */
/*
	snort_blocked.php
	Copyright (C) 2006 Scott Ullrich
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
require("/usr/local/pkg/snort.inc");

if($_POST['todelete'] or $_GET['todelete']) {
	if($_POST['todelete'])
		$ip = $_POST['todelete'];
	if($_GET['todelete'])
		$ip = $_GET['todelete'];
	exec("/sbin/pfctl -t snort2c -T delete {$ip}");
}

$pgtitle = "Snort: Snort Rulesets";
include("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">
<?php include("fbegin.inc"); ?>

<p class="pgtitle"><?=$pgtitle?></font></p>

<form action="snort_rulesets.php" method="post" name="iform" id="iform">
<script src="/row_toggle.js" type="text/javascript"></script>
<script src="/javascript/sorttable.js" type="text/javascript"></script>
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
   <tr>
   		<td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Snort Settings"), false, "/pkg_edit.php?xml=snort.xml&id=0");
	$tab_array[] = array(gettext("Update Snort Rules"), false, "/snort_download_rules.php");
	$tab_array[] = array(gettext("Snort Categories"), false, "/snort_rulesets.php");
	$tab_array[] = array(gettext("Snort Rules"), false, "/snort_rules.php");
	$tab_array[] = array(gettext("Snort Blocked"), true, "/snort_blocked.php");
	$tab_array[] = array(gettext("Snort Whitelist"), false, "/pkg.php?xml=snort_whitelist.xml");
	$tab_array[] = array(gettext("Snort Alerts"), false, "/snort_alerts.php");
	$tab_array[] = array(gettext("Snort Advanced"), false, "/pkg_edit.php?xml=snort_advanced.xml&id=0");
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
						<table id="sortabletable1" class="sortable" width="100%" border="0" cellpadding="0" cellspacing="0">
						    <tr id="frheader">
								<td width="5%" class="listhdrr">Remove</td>
								<td class="listhdrr">IP</td>
								<td class="listhdrr">Alert Description</td>
							</tr>
<?php

	$associatealertip = $config['installedpackages']['snort']['config'][0]['associatealertip'];
	$ips = `/sbin/pfctl -t snort2c -T show`;
	$ips_array = split("\n", $ips);
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
		echo "\n<img title=\"Delete\" border=\"0\" name='todelete' id='todelete' alt=\"Delete\" src=\"./themes/{$g['theme']}/images/icons/icon_x.gif\"></a></td>";
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

This page lists hosts that have been blocked by Snort.   Hosts are automatically deleted every 60 minutes.
<?php include("fend.inc"); ?>

</body>
</html>

<?php

/* write out snort cache */
write_snort_config_cache($snort_config);

?>