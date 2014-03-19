<?php
/* $Id$ */
/*
	unbound_status.php
	part of pfSense (https://www.pfsense.org/)

	Copyright (C) 2010 Scott Ullrich <sullrich@gmail.com>
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

// Define basedir constant for unbound according to FreeBSD version (PBI support or no PBI)
if (floatval(php_uname("r")) >= 8.3)
	define("UNBOUND_BASE", "/usr/pbi/unbound-" . php_uname("m"));
else
	define("UNBOUND_BASE", "/usr/local");

if(!is_process_running("unbound")) {
	Header("Location: /pkg_edit.php?xml=unbound.xml&id=0");
	exit;
}

$pgtitle = "Services: Unbound DNS Forwarder: Status";
include("head.inc");

function doCmdT($title, $command, $rows) {
	echo "<p>\n";
	echo "<a name=\"" . $title . "\">\n";
	echo "<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n";
	echo "<tr><td class=\"listtopic\">" . $title . "</td></tr>\n";
	echo "<tr><td class=\"listlr\"><textarea style=\"font-family:courier\"cols=\"101\" rows=\"$rows\">";		/* no newline after pre */

	if ($command == "dumpconfigxml") {
		$fd = @fopen("/conf/config.xml", "r");
		if ($fd) {
			while (!feof($fd)) {
				$line = fgets($fd);
				/* remove sensitive contents */
				$line = preg_replace("/<password>.*?<\\/password>/", "<password>xxxxx</password>", $line);
				$line = preg_replace("/<pre-shared-key>.*?<\\/pre-shared-key>/", "<pre-shared-key>xxxxx</pre-shared-key>", $line);
				$line = preg_replace("/<rocommunity>.*?<\\/rocommunity>/", "<rocommunity>xxxxx</rocommunity>", $line);
				$line = str_replace("\t", "    ", $line);
				echo htmlspecialchars($line,ENT_NOQUOTES);
			}
		}
		fclose($fd);
	} else {
		$execOutput = "";
		$execStatus = "";
		exec ($command . " 2>&1", $execOutput, $execStatus);
		for ($i = 0; isset($execOutput[$i]); $i++) {
			if ($i > 0) {
				echo "\n";
			}
			echo htmlspecialchars($execOutput[$i],ENT_NOQUOTES);
		}
	}
	echo "</textarea></tr>\n";
	echo "</table>\n";
}

/* Execute a command, giving it a title which is the same as the command. */
function doCmd($command) {
	doCmdT($command,$command);
}

/* Define a command, with a title, to be executed later. */
function defCmdT($title, $command, $rows = "20") {
	global $commands;
	$title = htmlspecialchars($title,ENT_NOQUOTES);
	$commands[] = array($title, $command, $rows);
}

/* Define a command, with a title which is the same as the command,
 * to be executed later.
 */
function defCmd($command) {
	defCmdT($command,$command);
}

/* List all of the commands as an index. */
function listCmds() {
	global $commands;
	echo "<p>" . gettext("This status page includes the following information") . ":\n";
	echo "<ul width=\"100%\">\n";
	for ($i = 0; isset($commands[$i]); $i++ ) {
		echo "<li><strong><a href=\"#" . $commands[$i][0] . "\">" . $commands[$i][0] . "</a></strong>\n";
	}
	echo "</ul>\n";
}

/* Execute all of the commands which were defined by a call to defCmd. */
function execCmds() {
	global $commands;
	for ($i = 0; isset($commands[$i]); $i++ ) {
		doCmdT($commands[$i][0], $commands[$i][1], $commands[$i][2]);
	}
}

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if ($savemsg) print_info_box($savemsg); ?>

<div id="mainlevel">
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td>
	<?php
		$tab_array = array();
		$tab_array[] = array(gettext("Unbound DNS Settings"), false, "/pkg_edit.php?xml=unbound.xml&amp;id=0");
		$tab_array[] = array(gettext("Unbound DNS Advanced Settings"), false, "/pkg_edit.php?xml=unbound_advanced.xml&amp;id=0");
		$tab_array[] = array(gettext("Unbound DNS ACLs"), false, "/unbound_acls.php");
		$tab_array[] = array(gettext("Unbound DNS Status"), true, "/unbound_status.php");
		display_top_tabs($tab_array, true);
	?>
			</td>
		</tr>
	</table>
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td class="tabcont" width="100%">
			<?php
				$entries = trim(exec(UNBOUND_BASE . "/sbin/unbound-control dump_cache | wc -l"));
				defCmdT("Unbound status", "unbound-control status", "6");
				defCmdT("Unbound stats", "unbound-control stats_noreset");
				defCmdT("Unbound stubs", "unbound-control list_stubs", "8");
				defCmdT("Unbound forwards", "unbound-control list_forwards");
				defCmdT("Unbound local zones", "unbound-control list_local_zones");
				defCmdT("Unbound local data", "unbound-control list_local_data");
				defCmdT("Unbound cache ($entries entries)", "unbound-control dump_cache", "60");
				defCmdT("Unbound configuration", "/bin/cat " . UNBOUND_BASE . "/etc/unbound/unbound.conf", "60");
				listCmds();
				execCmds();
			?>
			</td>
		</tr>
	</table>
	</div>
<?php include("fend.inc"); ?>
</body>
</html>
