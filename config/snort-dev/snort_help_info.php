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
header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
header( "Cache-Control: no-cache, must-revalidate" );
header( "Pragma: no-cache" );

$pgtitle = "Snort: Services: Help and Info";
include('head.inc');
?>
<style type="text/css">
iframe
{
	border: 0;
}

#footer2
{
	position: relative;
	top: -2px;
	background-color: #cccccc;
	background-image: none;
	background-repeat: repeat;
	background-attachment: scroll;
	background-position: 0% 0%;
	padding-top: 0px;
	padding-right: 0px;
	padding-bottom: 0px;
	padding-left: 0px;
}

</style>
<body>
<?php include("fbegin.inc"); ?>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td>
<?php
	$tab_array = array();
	$tab_array[] = array("Snort Interfaces", false, "/snort/snort_interfaces.php");
	$tab_array[] = array("Global Settings", false, "/snort/snort_interfaces_global.php");
	$tab_array[] = array("Rule Updates", false, "/snort/snort_download_rules.php");
	$tab_array[] = array("Alerts", false, "/snort/snort_alerts.php");
    $tab_array[] = array("Blocked", false, "/snort/snort_blocked.php");
	$tab_array[] = array("Whitelists", false, "/pkg.php?xml=/snort/snort_whitelist.xml");
	$tab_array[] = array("Help & Info", true, "/snort/snort_help_info.php");
	display_top_tabs($tab_array);
?>
    </td>
</tr>
</table>
<div>
	<iframe style="width: 780px; height: 600px; overflow-x: hidden;" src='/snort/help_and_info.php'></iframe>
</div>
</div>
	<div id="footer2">
		<IMG SRC="./images/footer.jpg" width="780px" height="63" ALT="Apps">
			<font size="1">Snort® is a registered trademark of Sourcefire, Inc., Barnyard2® is a registered trademark of securixlive.com., Orion® copyright Robert Zelaya., 
			Emergingthreats is a registered trademark of emergingthreats.net., Mysql® is a registered trademark of Mysql.com.</font>
	</div>
</div>
        <div id="footer">
			<a target="_blank" href="http://www.pfsense.org/?gui12" class="redlnk">pfSense</a> is &copy;
			 2004 - 2009 by <a href="http://www.bsdperimeter.com" class="tblnk">BSD Perimeter LLC</a>. All Rights Reserved.
			<a href="/license.php" class="tblnk">view license</a>]
			<br/>

			<a target="_blank" href="https://portal.pfsense.org/?guilead=true" class="tblnk">Commercial Support Available</a>
		</div> <!-- Footer DIV -->
</body>
</html>
