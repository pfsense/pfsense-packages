<?php
/* $Id$ */
/*
	halt.php
	part of pfSense
	Copyright (C) 2004 Scott Ullrich
	All rights reserved.

	part of m0n0wall as reboot.php (http://m0n0.ch/wall)
	Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
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


header("snort_help_info.php");

$pgtitle = "Snort: Services: Help and Info";
include('head.inc');
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></p>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td>
<?php
	$tab_array = array();
	$tab_array[] = array("Snort Inertfaces", false, "/snort/snort_interfaces.php");
	$tab_array[] = array("Global Settings", false, "/snort/snort_interfaces_global.php");
	$tab_array[] = array("Rule Updates", false, "/snort/snort_download_rules.php");
	$tab_array[] = array("Alerts", false, "/snort/snort_alerts.php");
    $tab_array[] = array("Blocked", false, "/snort/snort_blocked.php");
	$tab_array[] = array("Whitelists", false, "/pkg.php?xml=/snort_whitelist.xml");
	$tab_array[] = array("Help & Info", true, "/snort/snort_help_info.php");
	display_top_tabs($tab_array);
?>
    </td>
</tr>
	<td>
	<iFrame src='/snort/help_and_info.html' frameborder=0 width=780px height=600px name='name' marginheight=0 marginwidth=0 scrolling=on></iFrame>
	</td>

</table>

<?php include("fend.inc"); ?>
</body>
</html>
