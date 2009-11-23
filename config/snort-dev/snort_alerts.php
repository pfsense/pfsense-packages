<?php
/* $Id$ */
/*
	snort_alerts.php
	part of pfSense

	Copyright (C) 2005 Bill Marquette <bill.marquette@gmail.com>.
	Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
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

require("globals.inc");
require("guiconfig.inc");
require("/usr/local/pkg/snort/snort.inc");

$snort_logfile = "{$g['varlog_path']}/snort/alert";

$nentries = $config['syslog']['nentries'];
if (!$nentries)
	$nentries = 50;

if ($_POST['clear']) {
	exec("killall syslogd");
	conf_mount_rw();
	exec("rm {$snort_logfile}; touch {$snort_logfile}");
	conf_mount_ro();
	system_syslogd_start();
	exec("/usr/bin/killall -HUP snort");
	exec("/usr/bin/killall snort2c");
	if ($config['installedpackages']['snort']['config'][0]['blockoffenders'] == 'on')
		exec("/usr/local/bin/snort2c -w /var/db/whitelist -a /var/log/snort/alert");
}

$pgtitle = "Services: Snort: Snort Alerts";
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></p>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array("Snort Inertfaces", false, "/snort/snort_interfaces.php");
	$tab_array[] = array("Global Settings", false, "/snort/snort_interfaces_global.php");
	$tab_array[] = array("Rule Updates", false, "/snort/snort_download_rules.php");
	$tab_array[] = array("Alerts", true, "/snort/snort_alerts.php");
    $tab_array[] = array("Blocked", false, "/snort/snort_blocked.php");
	$tab_array[] = array("Whitelists", false, "/pkg.php?xml=/snort/snort_whitelist.xml");
	$tab_array[] = array("Help & Info", false, "/snort/snort_help_info.php");
	display_top_tabs($tab_array);
?>
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
		<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
		  <tr>
			<td colspan="2" class="listtopic">
			  Last <?=$nentries;?> Snort Alert entries</td>
		  </tr>
		  <?php dump_log_file($snort_logfile, $nentries); ?>
			<tr><td><br><form action="snort_alerts.php" method="post">
			<input name="clear" type="submit" class="formbtn" value="Clear log"></td></tr>
		</table>
	</div>
	</form>
	</td>
  </tr>
</table>
<?php include("fend.inc"); ?>
<meta http-equiv="refresh" content="60;url=<?php print $_SERVER['SCRIPT_NAME']; ?>">
</body>
</html>
<!-- <?php echo $snort_logfile; ?> -->

<?php

function dump_log_file($logfile, $tail, $withorig = true, $grepfor = "", $grepinvert = "") {
	global $g, $config;
    $logarr = "";
	exec("cat {$logfile} | /usr/bin/tail -n {$tail}", $logarr);
    foreach ($logarr as $logent) {
            if(!logent)
            	continue;
            $ww_logent = $logent;
			$ww_logent = str_replace("[", " [ ", $ww_logent);
			$ww_logent = str_replace("]", " ] ", $ww_logent);
            echo "<tr valign=\"top\">\n";
            echo "<td colspan=\"2\" class=\"listr\">" . make_clickable($ww_logent) . "&nbsp;</td>\n";
            echo "</tr>\n";
    }
}

?>