<?php
/*
    status_ospfd.php
    Copyright (C) 2010 Nick Buraglio; nick@buraglio.com
	Copyright (C) 2010 Scott Ullrich <sullrich@pfsense.org>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INClUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
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

$pgtitle = "OpenOSPFd: Status";
include("head.inc");

/* List all of the commands as an index. */
function listCmds() {
    global $commands;
    echo "<br/>This status page includes the following information:\n";
    echo "<ul width=\"100%\">\n";
    for ($i = 0; isset($commands[$i]); $i++ ) {
        echo "<li><strong><a href=\"#" . $commands[$i][0] . "\">" . $commands[$i][0] . "</a></strong></li>\n";
    }
    echo "</ul>\n";
}

function execCmds() {
    global $commands;
    for ($i = 0; isset($commands[$i]); $i++ ) {
        doCmdT($commands[$i][0], $commands[$i][1]);
    }
}

/* Define a command, with a title, to be executed later. */
function defCmdT($title, $command) {
    global $commands;
    $title = htmlspecialchars($title,ENT_NOQUOTES);
    $commands[] = array($title, $command);
}

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

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

?>

<html>
	<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
		<?php include("fbegin.inc"); ?>
		<?php if($one_two): ?>
			<p class="pgtitle"><?=$pgtitle?></font></p>
		<?php endif; ?>
		<?php if ($savemsg) print_info_box($savemsg); ?>

		<table width="100%" border="0" cellpadding="0" cellspacing="0">
  			<tr><td class="tabnavtbl">
<?php
				$tab_array = array();
				$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=openospfd.xml&id=0");
				$tab_array[] = array(gettext("Interface Settings"), false, "/pkg.php?xml=openospfd_interfaces.xml");
				$tab_array[] = array(gettext("Status"), true, "/status_ospfd.php");
				display_top_tabs($tab_array);
			?>
			</td></tr>
			  <tr>
			    <td>
				<div id="mainarea">
					<table class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
						<tr>
							<td>
<?php
								defCmdT("OpenOSPFd Summary","/usr/local/sbin/ospfctl show summary"); 
								defCmdT("OpenOSPFd Neighbors","/usr/local/sbin/ospfctl show neighbor"); 
								defCmdT("OpenOSPFd FIB","/usr/local/sbin/ospfctl show fib");
								defCmdT("OpenOSPFd RIB","/usr/local/sbin/ospfctl show rib"); 
								defCmdT("OpenOSPFd Interfaces","/usr/local/sbin/ospfctl show interfaces"); 
								defCmdT("OpenOSPFD Database","/usr/local/sbin/ospfctl show database"); 
?>
								<div id="cmdspace" style="width:100%">
									<?php listCmds(); ?>		
									<?php execCmds(); ?>
								</div>
							</td>
						</tr>
					</table>
				</div>
				</td>
			   </tr>
		</table>
		<?php include("fend.inc"); ?>
	</body>
</html>
