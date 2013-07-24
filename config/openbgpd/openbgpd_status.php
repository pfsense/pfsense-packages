<?php
/* $Id$ */
/*
	openbgpd_status.php
	part of pfSense (http://www.pfsense.com/)
	Copyright (C) 2007 Scott Ullrich (sullrich@gmail.com)
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

$commands = array();

defCmdT("summary",	"OpenBGPD Summary",	"bgpctl show summary");
defCmdT("interfaces",	"OpenBGPD Interfaces",	"bgpctl show interfaces");
defCmdT("routing",	"OpenBGPD Routing",	"bgpctl show rib");
defCmdT("forwarding",	"OpenBGPD Forwarding",	"bgpctl show fib");
defCmdT("network",	"OpenBGPD Network",	"bgpctl show network");
defCmdT("nexthops",	"OpenBGPD Nexthops",	"bgpctl show nexthop");
defCmdT("ip",		"OpenBGPD IP",		"bgpctl show ip bgp");
defCmdT("neighbors",	"OpenBGPD Neighbors",	"bgpctl show neighbor");

if (isset($_REQUEST['isAjax'])) {
	if (isset($_REQUEST['cmd']) && isset($commands[$_REQUEST['cmd']]))
		echo htmlspecialchars_decode(doCmdT($commands[$_REQUEST['cmd']][1], $_REQUEST['limit']. $_REQUEST['filter']));
	exit;
}

if ($config['version'] >= 6)
	$pgtitle = array("OpenBGPD", "Status");
else
	$pgtitle = "OpenBGPD: Status";

include("head.inc");

function doCmdT($command, $limit = 0, $filter = "") {
	$grepline = "";
	if (!empty($filter))
		$grepline = " | grep " . escapeshellarg(htmlspecialchars($filter));

	$fd = popen("{$command}{$grepline} 2>&1", "r");
	$ct = 0;
	$cl = 0;
	$result = "";
	while (($line = fgets($fd)) !== FALSE) {
		if ($limit > 0 && $cl >= $limit)
			break;
		$result .= htmlspecialchars($line, ENT_NOQUOTES);
		if ($ct++ > 1000) {
			ob_flush();
			$ct = 0;
		}
		$cl++;
	}
	pclose($fd);

	return $result;
}

function showCmdT($idx, $title, $command) {
	echo "<p>\n";
	echo "<a name=\"" . $title . "\">&nbsp;</a>\n";
	echo "<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n";
	echo "<tr><td class=\"listtopic\">" . $title . "</td></tr>\n";
	echo "<tr><td class=\"listlr\"><pre>";		/* no newline after pre */
	echo doCmdT($command);
	echo "</pre></td></tr>\n";
	echo "</table>\n";
}

/* Define a command, with a title, to be executed later. */
function defCmdT($idx, $title, $command) {
	global $commands;
	$title = htmlspecialchars($title,ENT_NOQUOTES);
	$commands[$idx] = array($title, $command);
}

/* List all of the commands as an index. */
function listCmds() {
	global $commands;
	echo "<p>This status page includes the following information:\n";
	echo "<ul width=\"700\">\n";
	foreach ($commands as $idx => $command)
		echo "<li><strong><a href=\"#" . $command[0] . "\">" . $command[0] . "</a></strong></li>\n";
	echo "</ul>\n";
}

/* Execute all of the commands which were defined by a call to defCmd. */
function execCmds() {
	global $commands;
	foreach ($commands as $idx => $command)
		showCmdT($idx, $command[0], $command[1]);
}

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php
	if ($config['version'] < 6)
		echo '<p class="pgtitle">' . $pgtitle . '</font></p>';
?>

<?php if ($savemsg) print_info_box($savemsg); ?>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=openbgpd.xml&id=0");
	$tab_array[] = array(gettext("Neighbors"), false, "/pkg.php?xml=openbgpd_neighbors.xml");
	$tab_array[] = array(gettext("Groups"), false, "/pkg.php?xml=openbgpd_groups.xml");
	$tab_array[] = array(gettext("Raw config"), false, "/openbgpd_raw.php");
	$tab_array[] = array(gettext("Status"), true, "/openbgpd_status.php");
	display_top_tabs($tab_array);
?>
</table>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td class="tabcont" >
			<form action="tinydns_status.php" method="post">
			</form>
		</td>
	</tr>
	<tr>
		<td class="tabcont" >

			<div id="cmdspace" style="width:100%">
			<?php listCmds(); ?>

			<?php execCmds(); ?>
			</div>

		</td>
	</tr>
</table>
</div>

<?php include("fend.inc"); ?>

</body>
</html>
