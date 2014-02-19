<?php
/*
 * suricata_log_view.php
 *
 * Copyright (C) 2014 Bill Meeks
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

require_once("guiconfig.inc");
require_once("/usr/local/pkg/suricata/suricata.inc");

$contents = '';

// Read the contents of the argument passed to us.
// Is it a fully qualified path and file?
$logfile = htmlspecialchars($_GET['logfile'], ENT_QUOTES | ENT_HTML401);
if (file_exists($logfile))
	if (substr(realpath($logfile), 0, strlen(SURICATALOGDIR)) != SURICATALOGDIR)
		$contents = gettext("\n\nERROR -- File: {$logfile} can not be viewed!");
	else
		$contents = file_get_contents($logfile);
// It is not something we can display, so print an error.
else
	$contents = gettext("\n\nERROR -- File: {$logfile} not found!");

$pgtitle = array(gettext("Suricata"), gettext("Log File Viewer"));
?>

<?php include("head.inc");?>

<body link="#000000" vlink="#000000" alink="#000000">
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php // include("fbegin.inc");?>

<form action="suricata_log_view.php" method="post">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
	<td class="tabcont">
		<table width="100%" cellpadding="0" cellspacing="6" bgcolor="#eeeeee">
		<tr>
			<td class="pgtitle" colspan="2">Suricata: Log File Viewer</td>
		</tr>
		<tr>
			<td align="left" width="20%">
				<input type="button" class="formbtn" value="Return" onclick="window.close()">
			</td>
			<td align="right">
				<b><?php echo gettext("Log File: ") . '</b>&nbsp;' . $_GET['logfile']; ?>&nbsp;&nbsp;&nbsp;&nbsp;
			</td>
		</tr>
		<tr>
			<td colspan="2" valign="top" class="label">
			<div style="background: #eeeeee; width:100%; height:100%;" id="textareaitem"><!-- NOTE: The opening *and* the closing textarea tag must be on the same line. -->
			<textarea style="width:100%; height:100%;" readonly wrap="off" rows="33" cols="80" name="code2"><?=$contents;?></textarea>
			</div>
			</td>
		</tr>
		</table>
	</td>
</tr>
</table>
</form>
<?php // include("fend.inc");?>
</body>
</html>
