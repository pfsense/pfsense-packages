#!/usr/local/bin/php
<?php
/*
	$Id$

        arpwatch_reports.php
        Copyright (C) 2005 Colin Smith
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

require_once("guiconfig.inc");
require_once("service-utils.inc");

$logfile = "/var/log/arp.dat";

if ($_POST['clear']) {
        stop_service("arpwatch");
	unlink_if_exists($logfile);
	touch($logfile);
	start_service("arpwatch");
}

if(file_exists($logfile)) {
	$rawrep = file($logfile);
	foreach($rawrep as $line) {
	      	$todo = preg_split('/\s/', $line);
		$rawmac = explode(":", trim($todo[0]));
		foreach($rawmac as $set) $mac[] = str_pad($set, 2, "0", STR_PAD_LEFT);
		$newmac = implode(":", $mac);
	        $report[$todo[1]][] = array(
	                                        "mac" => $newmac,
	                                        "timestamp" => trim($todo[2]),
	                                        "hostname" => trim($todo[3]) ? trim($todo[3]) : "Unknown"
	                        );
		unset($mac);
	}
}
$pgtitle = "arpwatch: Reports";
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></p>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
        <tr>
                <td>
<?php
        $tab_array = array();
        $tab_array[] = array("Settings", false, "pkg_edit.php?xml=arpwatch.xml&id=0");
	$tab_array[] = array("Reports", true, "arpwatch_reports.php");
	display_top_tabs($tab_array);
?>
                </td>
        </tr>
        <tr>
                <td>
                        <div id="mainarea">
                        <table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
                                <tr>
                                        <td colspan="4" class="listtopic">arp.dat entries</td>
                                </tr>
				<tr>
					<td width="15%" class="listhdrr">IP</td>
					<td width="25%" class="listhdrr">Timestamp</td>
					<td width="15%" class="listhdrr">MAC</td>
					<td width="45%" class="listhdrr">Hostname</td>
				</tr>
				<?php
					if($report)
						foreach($report as $ip => $rawentries) {
							$printip = true;
							$entries = $rawentries;
							sort($entries);
							foreach($entries as $entry) {
								echo '<tr>';
								if($printip) {
									echo '<td class="listlr">' . $ip . '</td>';
									$stampclass = "listr";
									$printip = false;
								} else {
									$stampclass = "listlr";
									echo '<td></td>';
								}
								echo '<td class="' . $stampclass . '">' .
								date("D M j G:i:s", $entry['timestamp']) .
								'</td>';
								echo '<td class="listr">' . $entry['mac'] . '</td>';
								echo '<td class="listr">' . $entry['hostname'] . '</td>';
								echo '</tr>';
							}
						}
				?>
                                <tr>
                                        <td>
                                                <br>
                                                <form action="arpwatch_reports.php" method="post">
                                                <input name="clear" type="submit" class="formbtn" value="Clear log">
                                                </form>
                                        </td>
                                </tr>
                        </table>
                </div>
                </td>
        </tr>
</table>
