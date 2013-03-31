<?php
/*
 * snort_log_view.php
 *
 * Copyright (C) 2004, 2005 Scott Ullrich
 * Copyright (C) 2011 Ermal Luci
 * All rights reserved.
 *
 * Adapted for FreeNAS by Volker Theile (votdev@gmx.de)
 * Copyright (C) 2006-2009 Volker Theile
 *
 * Adapted for Pfsense Snort package by Robert Zelaya
 * Copyright (C) 2008-2009 Robert Zelaya
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
require_once("/usr/local/pkg/snort/snort.inc");

global $update_log;

$contents = '';

// Read the contents of the argument passed to us.
// Is it a fully qualified path and file?
if (file_exists($_GET['logfile']))
	$contents = file_get_contents($_GET['logfile']);
// It is not something we can display, so print an error.
else
	$contents = gettext("\n\nERROR -- File: {$_GET['logfile']} not found!");

$pgtitle = array(gettext("Snort"), gettext("Log File Viewer"));
?>

<?php include("head.inc");?>

<body link="#000000" vlink="#000000" alink="#000000">
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php include("fbegin.inc");?>

<form action="snort_log_view.php" method="post">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
	<td class="tabcont">
		<table width="100%" cellpadding="0" cellspacing="6" bgcolor="#eeeeee">
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
			<div style="background: #eeeeee;" id="textareaitem"><!-- NOTE: The opening *and* the closing textarea tag must be on the same line. -->
			<textarea readonly wrap="off" rows="33" cols="90" name="code2"><?=$contents;?></textarea>
			</div>
			</td>
		</tr>
		</table>
	</td>
</tr>
</table>
</form>
<?php include("fend.inc");?>
</body>
</html>
