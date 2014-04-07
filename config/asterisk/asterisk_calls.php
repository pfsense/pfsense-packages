<?php
/* $Id$ */
/*
	status_asterisk_calls.php
	part of pfSense
	Copyright (C) 2009 Scott Ullrich <sullrich@gmail.com>.
	Copyright (C) 2013 robi <robreg@zsurob.hu>
	All rights reserved.

	originally part of m0n0wall (http://m0n0.ch/wall)
	Copyright (C) 2003-2005 Manuel Kasper <mk@neon1.net>.
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
/*	
	pfSense_MODULE:	asterisk
*/

##|+PRIV
##|*IDENT=page-status-asterisk
##|*NAME=Status: Asterisk Calls page
##|*DESCR=Allow access to the 'Status: Asterisk Calls' page.
##|*MATCH=asterisk_calls.php*
##|-PRIV

require_once("guiconfig.inc");

$pgtitle = array(gettext("Status"),gettext("Asterisk Calls"));
$shortcut_section = "asterisk";
include("head.inc");

/* Path to call log database */
$callog = "/var/log/asterisk/cdr-csv/Master.csv";

/* Data input processing */
$cmd =  $_GET['cmd'];

$file = $_SERVER["SCRIPT_NAME"];
$break = Explode('/', $file);
$pfile = $break[count($break) - 1]; 

if (file_exists($callog))
	switch ($cmd){
		case "trim":
		$trimres=shell_exec("tail -50 '$callog' > /tmp/trimmed_asterisk.csv && rm '$callog' && mv /tmp/trimmed_asterisk.csv '$callog' && chown asterisk:asterisk '$callog' && chmod g+w '$callog'");
		header( 'Location: asterisk_calls.php?savemsg=Calls+log+trimmed.') ;
		break;

		case "clear":
		$trimres=shell_exec("rm '$callog' && touch '$callog' && chown asterisk:asterisk '$callog' && chmod g+w '$callog'");
		header( 'Location: asterisk_calls.php?savemsg=Calls+log+cleared.') ;
		break;

		case "download":
		// session_cache_limiter('none'); //*Use before session_start()
		// session_start();

			header('Content-Description: File Transfer');
			header('Content-Type: application/octet-stream');
			header('Content-Disposition: attachment; filename='.basename($callog));
			header('Content-Transfer-Encoding: binary');
			header('Expires: 0');
			header('Cache-Control: must-revalidate');
			header('Pragma: public');
			header('Content-Length: ' . filesize($callog));
			ob_clean();
			flush();
			readfile($callog);
			exit;
		break;
	}
?>


<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
	<?php include("fbegin.inc"); ?>
	<?php
	$savemsg = $_GET["savemsg"];
	if ($savemsg) {
	  print_info_box($savemsg);
	}
	?>
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td>
				<?php
					$tab_array = array();
					$tab_array[0] = array(gettext("Commands"), false, "asterisk_cmd.php");
					$tab_array[1] = array(gettext("Calls"), true, "asterisk_calls.php");
					$tab_array[2] = array(gettext("Log"), false, "asterisk_log.php");
					$tab_array[3] = array(gettext("Edit configuration"), false, "asterisk_edit_file.php");
					display_top_tabs($tab_array);
				?>
			</td>
		</tr>
		<tr>
			<td>
				<div id="mainarea">
				<?php
					if (file_exists($callog))
						$file_handle = fopen($callog, "r");
				?>
					<table class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
						<tr>
							<td colspan="6" class="listtopic">Last 50 Asterisk calls</td>
						</tr>
						<tr>
							<td nowrap class="listhdrr"><?=gettext("From");?></td>
							<td nowrap class="listhdrr"><?=gettext("To");?></a></td>
							<td nowrap class="listhdrr"><?=gettext("Start");?></td>
							<td nowrap class="listhdrr"><?=gettext("End");?></a></td>
							<td nowrap class="listhdrr"><?=gettext("Duration");?></a></td>
							<td nowrap class="listhdrr"><?=gettext("Status");?></td>
						</tr>
				<?php
					$out = '';
					if (file_exists($callog)){
						while (!feof($file_handle) ) {
							$lin = fgetcsv($file_handle, 102400);
							if ($lin[12] != "") {
								$out = "<tr>" . $out;
								$out = "<td class='listlr'>" . utf8_decode(str_replace('"', '', $lin[4])) . "</td><td class='listlr'>" . $lin[2] . "</td><td class='listlr'>" . $lin[9] . "</td><td class='listlr'>" . $lin[11] . "</td><td class='listlr'>" . gmdate("G:i:s", $lin[12]) . "</td><td class='listlr'>" . $lin[14] . "</td>" . $out;
								$out = "</tr>" . $out;
							}
						}
						fclose($file_handle);
					}
					echo $out;
					echo "<tr><td colspan='6'><a href='$pfile?cmd=download'><input type='button' name='command' value='Download' class='formbtn'></a>";
					echo "<a href='$pfile?cmd=trim'><input type='button' name='command' value='Trim log' class='formbtn'></a>";
					echo "<a href='$pfile?cmd=clear'><input type='button' name='command' value='Clear log' class='formbtn'></a></td></tr>";
				?>
				</table>
				</div>
			</td>
		</tr>
	</table>

<p/>

<span class="vexpl">
	<span class="red">
		<strong><?=gettext("Notes:");?><br /></strong>
	</span>
	<?=gettext("Listed in reverse order (latest on top).");?> <br>
	<?=gettext("Duration includes ringing time.");?> <br>
	<?=gettext("Trim keeps the last 50 entries.");?>

<?
if ($g['platform'] == "nanobsd")
        echo "<br>This log may be lost when rebooting the system.";
?>


</span>


<?php include("fend.inc"); ?>
</body>
