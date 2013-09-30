<?php
/* $Id$ */
/*
	status_asterisk_log.php
	part of pfSense
	Copyright (C) 2009 Scott Ullrich <sullrich@gmail.com>.
        Copyright (C) 2012 robi <robreg@zsurob.hu>
	Copyright (C) 2012 Marcello Coutinho
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
##|*DESCR=Allow access to the 'Status: Asterisk Log' page.
##|*MATCH=asterisk_log.php*
##|-PRIV

require_once("guiconfig.inc");

$pgtitle = array(gettext("Status"),gettext("Asterisk Log"));
$shortcut_section = "asterisk";
include("head.inc");

/* Path to Asterisk log file */
//if ($g['platform'] == "nanobsd")
//	$log = "/tmp/asterisk.log";
//else
$log = "/var/log/asterisk/messages";

?>

<?php
/* Data input processing */
$cmd =  $_GET['cmd'];
//$cmd  = str_replace("+", " ", $cmd);

$file = $_SERVER["SCRIPT_NAME"];
$break = Explode('/', $file);
$pfile = $break[count($break) - 1]; 


if (file_exists($log)) {
	if ($cmd == "trim") {
		$trimres=shell_exec("tail -50 '$log' > /tmp/trimmed_asterisk.log && rm '$log' && mv /tmp/trimmed_asterisk.log '$log' && chown asterisk:asterisk '$log' && chmod g+w '$log'");
		header( 'Location: asterisk_log.php?savemsg=Log+trimmed.') ;
	}
	if ($cmd == "clear") {
		$trimres=shell_exec("rm '$log' && touch '$log' && chown asterisk:asterisk '$log' && chmod g+w '$log'");
		header( 'Location: asterisk_log.php?savemsg=Log+cleared.') ;
	}
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
					$tab_array[1] = array(gettext("Calls"), false, "asterisk_calls.php");
					$tab_array[2] = array(gettext("Log"), true, "asterisk_log.php");
					$tab_array[3] = array(gettext("Edit configuration"), false, "asterisk_edit_file.php");
					display_top_tabs($tab_array);
				?>
			</td>
		</tr>
		<tr>
			<td>
				<div id="mainarea">
				<table class="tabcont sortable" width="100%" border="0" cellpadding="6" cellspacing="0">
				<tr>
					<td colspan="2" class="listtopic">Last 50 Asterisk log entries</td>
				</tr>

				<tr valign="top"><td class="listlr" nowrap>

				<?php
					$showlog_command=shell_exec("tail -50 '$log'");
					echo nl2br($showlog_command);
				?>
				</td></tr>
				<?php
					echo "<tr><td colspan='6'><a href='$pfile?cmd=trim'><input type='button' name='command' value='Trim log' class='formbtn'></a>";
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
		<strong><?=gettext("Note:");?><br /></strong>
	</span>
	<?=gettext("Trim keeps the last 50 lines of the log.");?>
<?
if ($g['platform'] == "nanobsd")
        echo "<br>This log may be lost when rebooting the system.";
?>


</span>

<?php include("fend.inc"); ?>
</body>
</html>
