<?php
/* $Id$ */
/*
	status_asterisk.php
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
##|*NAME=Status: Asterisk page
##|*DESCR=Allow access to the 'Status: Asterisk' page.
##|*MATCH=sasterisk_cmd.php*
##|-PRIV

require_once("guiconfig.inc");

$pgtitle = array(gettext("Status"),gettext("Asterisk"));
$shortcut_section = "asterisk";
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php
/* Data input processing */
$cmd =  $_GET['cmd'];
$cmd  = str_replace("+", " ", $cmd);

if ($cmd == "") {
	$cmd = "core show settings";
}

$file = $_SERVER["SCRIPT_NAME"];
$break = Explode('/', $file);
$pfile = $break[count($break) - 1]; 

?>
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td>
				<?php
					$tab_array = array();
					$tab_array[0] = array(gettext("Commands"), true, "asterisk_cmd.php");
					$tab_array[1] = array(gettext("Calls"), false, "asterisk_calls.php");
					$tab_array[2] = array(gettext("Log"), false, "asterisk_log.php");
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
						<td class="listtopic">
						<table><tr>
						<?php
						/* Print command buttons */
						echo "<td align='center'><a href='$pfile?cmd=sip+show+registry'><input type='button' name='command' value='SIP Registry' class='formbtns' style='width: 100px'></a></td>";
						echo "<td align='center'><a href='$pfile?cmd=sip+show+peers'><input type='button' name='command' value='SIP Peers' class='formbtns' style='width: 100px'></a></td>";
						echo "<td align='center'><a href='$pfile?cmd=sip+show+channels'><input type='button' name='command' value='SIP Channels' class='formbtns' style='width: 100px'></a></td>";
						echo "<td align='center'><a href='$pfile?cmd=core+show+channels'><input type='button' name='command' value='Channels' class='formbtns' style='width: 100px'></a></td>";
						echo "<td align='center'><a href='$pfile?cmd=core+show+codecs+audio'><input type='button' name='command' value='Codecs' class='formbtns' style='width: 100px'></a></td>";
						echo "<td align='center'><a href='$pfile?cmd=core+show+translation+recalc+10'><input type='button' name='command' value='Translation' class='formbtns' style='width: 100px'></a></td>";
						echo "<td align='center'><a href='$pfile?cmd=sip+show+settings'><input type='button' name='command' value='SIP Settings' class='formbtns' style='width: 100px'></a></td>";
						echo "</tr><tr>";
						//echo "<td></td>";
						echo "<td align='center'><a href='$pfile?cmd=sip+reload'><input type='button' name='command' value='Reload SIP' class='formbtns' style='width: 100px'></a></td>";
						echo "<td align='center'><a href='$pfile?cmd=dialplan+reload'><input type='button' name='command' value='Reload Extensions' class='formbtns' style='width: 100px'></a></td>";
						echo "<td align='center'><a href='$pfile?cmd=core+reload'><input type='button' name='command' value='Reload Core' class='formbtns' style='width: 100px'></a></td>";
						echo "<td align='center'><a href='$pfile?cmd=core+show+uptime'><input type='button' name='command' value='Uptime' class='formbtns' style='width: 100px'></a></td>";
						echo "<td align='center'><a href='$pfile?cmd=core+restart+now'><input type='button' name='command' value='Restart Asterisk' class='formbtns' style='width: 100px'></a></td>";
						echo "<td align='right' colspan='2'><form name='input' action='$pfile' method='get'><input type='text' name='cmd' style='width: 145px'><input type='submit' value='SEND' class='formbtns' style='width: 50px'></form> </td>";
						?>
						</tr></table>
						</td>
					</tr>
					<tr valign="top">
						<td class="listlr" nowrap>
						<?php
						/* Run commands and print results */
						$asterisk_command=shell_exec("asterisk -rx '$cmd'");
						echo "<pre style='font-size:11px; background:white'>";
						echo $asterisk_command;
						echo "</pre>";
						?>
						</td>
					</tr>
				</table>
				</div>
			</td>
		</tr>
	</table>
<?php include("fend.inc"); ?>
