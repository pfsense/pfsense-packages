<?php
/* $Id$ */
/*
	havp_log.php
	Copyright (C) 2014 Andrew Nikitin <andrey.b.nikitin@gmail.com>.
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

require("guiconfig.inc");

if (file_exists("/usr/local/pkg/havp.inc"))
   require_once("/usr/local/pkg/havp.inc");
else echo "No havp.inc found";

$nentries = $config['syslog']['nentries'];
if (!$nentries)
	$nentries = 50;

if ($_POST['clear']) 
	file_put_contents(HVDEF_HAVP_ERRORLOG, '');

function dump_havp_errorlog($logfile, $tail) {
	global $g, $config;
	$sor = isset($config['syslog']['reverse']) ? "-r" : "";
	$logarr = "";
	$grepline = "  ";
	if (is_dir($logfile)) {
		$logarr = array("File $logfile is a directory.");
	} elseif(file_exists($logfile) && filesize($logfile) == 0) {
		$logarr = array(" ... Log file is empty.");
	} else {
		exec("cat " . escapeshellarg($logfile) . "{$grepline} | /usr/bin/tail {$sor} -n " . escapeshellarg($tail), $logarr);
	}
	foreach ($logarr as $logent) {
			$logent = preg_split("/\s+/", $logent, 3);
			echo "<tr valign=\"top\">\n";
			$entry_date_time = htmlspecialchars($logent[0] . " " .  $logent[1]);
			$entry_text = htmlspecialchars($logent[2]);
			echo "<td class=\"listlr nowrap\" width=\"130\">{$entry_date_time}</td>\n";
			echo "<td class=\"listr\">{$entry_text}</td>\n";
			echo "</tr>\n";
	}
}

$pgtitle = "Antivirus: HAVP log";
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("General page"), false, "antivirus.php");
	$tab_array[] = array(gettext("HTTP proxy"), false, "pkg_edit.php?xml=havp.xml&amp;id=0");
	$tab_array[] = array(gettext("Settings"), false, "pkg_edit.php?xml=havp_avset.xml&amp;id=0");
	$tab_array[] = array(gettext("Log"), true, "havp_log.php");
	display_top_tabs($tab_array);
?>
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
		<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
		  <tr>
			<td colspan="2" class="listtopic">
			  <?php printf(gettext("Last %s HAVP log entries"), $nentries);?></td>
		  </tr>
		  <?php dump_havp_errorlog(HVDEF_HAVP_ERRORLOG, $nentries); ?>
		<tr><td><br/>
		<form action="havp_log.php" method="post">
			<input name="clear" type="submit" class="formbtn" value="<?=gettext("Clear log"); ?>" /></form></td></tr>
		</table>
	</div>
	</td>
  </tr>
</table>
<?php include("fend.inc"); ?>
</body>
</html>
