<?php
/*
	status_nut.php
	part of pfSense (http://www.pfsense.com/)

	Copyright (C) 2006 Ryan Wagoner <ryan@wgnrs.dynu.com>.
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

$nut_config = $config['installedpackages']['nut']['config'][0];

/* Defaults to this page but if no settings are present, redirect to setup page */
if(!$nut_config['monitor'])
	Header("Location: /pkg_edit.php?xml=nut.xml&id=0");

$pgtitle = "Status: NUT Status";
include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
//$pfSenseHead->addMeta("<meta http-equiv=\"refresh\" content=\"120;url={$_SERVER['SCRIPT_NAME']}\" />");
//echo $pfSenseHead->getHTML();

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<?php if ($savemsg) print_info_box($savemsg); ?>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("NUT Status "), true, "/status_nut.php");
	$tab_array[] = array(gettext("NUT Settings "), false, "/pkg_edit.php?xml=nut.xml&id=0");
	display_top_tabs($tab_array);
?>
</table>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
	<td class="tabcont">
<?php
	if($nut_config['monitor'] == 'local') {
		/* grab values from upsc */
		$handle = popen("upsc {$nut_config['name']}@localhost","r");
		$read = fread($handle, 4096);
		pclose($handle);

		/* parse upsc into array */
		$read = explode("\n",$read);		
		$ups = array();
		foreach($read as $line) {
			$line = explode(':', $line);
			$ups[$line[0]] = $line[1];
		}

		print("Status: {$ups['ups.status']}<br />");
		print("Battery Charge: {$ups['battery.charge']}<br />");
		print("Battery Voltage: {$ups['battery.voltage']}<br />");
		print("Input Voltage: {$ups['input.voltage']}<br />");
		print("Output Voltage: {$ups['output.voltage']}<br />");
		
		/*print('<pre>'); print_r($ups); print('</pre>');*/
	}
?>
    </td>
  </tr>
</table>
</div>
<?php include("fend.inc"); ?>
</body>
</html>
