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

$pgtitle = "OpenBGPD: Status";
include("head.inc");

function doCmdT($title, $command) {
    echo "<p>\n";
    echo "<a name=\"" . $title . "\">\n";
    echo "<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n";
    echo "<tr><td class=\"listtopic\">" . $title . "</td></tr>\n";
    echo "<tr><td class=\"listlr\"><pre>";		/* no newline after pre */

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
    echo "</pre></tr>\n";
    echo "</table>\n";
}

/* Execute a command, giving it a title which is the same as the command. */
function doCmd($command) {
    doCmdT($command,$command);
}

/* Define a command, with a title, to be executed later. */
function defCmdT($title, $command) {
    global $commands;
    $title = htmlspecialchars($title,ENT_NOQUOTES);
    $commands[] = array($title, $command);
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
    echo "<p>This status page includes the following information:\n";
    echo "<ul width=\"700\">\n";
    for ($i = 0; isset($commands[$i]); $i++ ) {
        echo "<li><strong><a href=\"#" . $commands[$i][0] . "\">" . $commands[$i][0] . "</a></strong>\n";
    }
    echo "</ul>\n";
}

/* Execute all of the commands which were defined by a call to defCmd. */
function execCmds() {
    global $commands;
    for ($i = 0; isset($commands[$i]); $i++ ) {
        doCmdT($commands[$i][0], $commands[$i][1]);
    }
}

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<?php if ($savemsg) print_info_box($savemsg); ?>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=openbgpd.xml&id=0");
	$tab_array[] = array(gettext("Neighbors"), false, "/pkg.php?xml=openbgpd_neighbors.xml");
	$tab_array[] = array(gettext("Groups"), false, "/pkg.php?xml=openbgpd_groups.xml");
	$tab_array[] = array(gettext("Raw config"), true, "/openbgpd_raw.php");
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

<?php 

defCmdT("OpenBGPD Summary","bgpctl show summary"); 
defCmdT("OpenBGPD Interfaces","bgpctl show interfaces"); 
defCmdT("OpenBGPD Routing","bgpctl show rib"); 
defCmdT("OpenBGPD Routing","bgpctl show fib"); 
defCmdT("OpenBGPD Network","bgpctl show network"); 
defCmdT("OpenBGPD Nexthops","bgpctl show nexthop"); 
defCmdT("OpenBGPD IP","bgpctl show ip"); 
defCmdT("OpenBGPD Neighbors","bgpctl show neighbor"); 

?>
		<div id="cmdspace" style="width:100%">
		<?php listCmds(); ?>
		
		<?php execCmds(); ?>
		</div>

      </table>
     </td>
    </tr>
</table>
</div>

<?php include("fend.inc"); ?>

</body>
</html>
