<?php
/*
	status_nut.php
	part of pfSense (http://www.pfsense.com/)

	Copyright (C) 2007 Ryan Wagoner <ryan@wgnrs.dynu.com>.
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
	<td>
      <table width="100%" class="tabcont" cellspacing="0" cellpadding="6">
<?php
	if($nut_config['monitor'] == 'local') {
		print("<tr><td width=\"100px\" class=\"vncellreq\">Monitoring:</td><td class=\"vtable\">Local UPS</td><tr>\n");
		$handle = popen("upsc {$nut_config['name']}@localhost","r");
	} elseif($nut_config['monitor'] == 'remote') {
		print("<tr><td width=\"100px\" class=\"vncellreq\">Monitoring:</td><td class=\"vtable\">Remote UPS</td><tr>\n");
		$handle = popen("upsc {$nut_config['remotename']}@{$nut_config['remoteaddr']}","r");
	}

	if($handle) {
		$read = fread($handle, 4096);
		pclose($handle);

		/* parse upsc into array */
		$read = explode("\n",$read);		
		$ups = array();
		foreach($read as $line) {
			$line = explode(':', $line);
			$ups[$line[0]] = trim($line[1]);
		}
	
		print("<tr><td class=\"vncellreq\">Model:</td><td class=\"vtable\">{$ups['ups.model']}</td><tr>\n");

		print('<tr><td class="vncellreq">Status:</td><td class="vtable">');
		$status = explode(' ',$ups['ups.status']);
		foreach($status as $condition) {
			switch ($condition) {
				case WAIT:
					print('Waiting... ');
					break;
				case OL:
					print('On Line ');
					break;
				case LB:
					print('Battery Low ');
					break;
				default:
					print("{$condition} ");
					break;
			}
		}
		print("</td><tr>\n");

		print("<tr><td class=\"vncellreq\">Load:</td><td class=\"vtable\">{$ups['ups.load']}%</td><tr>\n");
		print("<tr><td class=\"vncellreq\">Battery Charge:</td><td class=\"vtable\">{$ups['battery.charge']}%</td><tr>\n");
		print("<tr><td class=\"vncellreq\">Battery Voltage:</td><td class=\"vtable\">{$ups['battery.voltage']}</td><tr>\n");
		print("<tr><td class=\"vncellreq\">Input Voltage:</td><td class=\"vtable\">{$ups['input.voltage']}V</td><tr>\n");
		print("<tr><td class=\"vncellreq\">Output Voltage:</td><td class=\"vtable\">{$ups['output.voltage']}V</td><tr>\n");
		print("<tr><td class=\"vncellreq\">Temperature:</td><td class=\"vtable\">{$ups['ups.temperature']}</td><tr>\n");
	} else {
		/* display error */
		print("<tr><td class=\"vncellreq\">ERROR:</td><td class=\"vtable\">Can\'t parse data from upsc!</td><tr>\n");
	}
?>
	  </table>
    </td>
  </tr>
</table>
<?php 
	/* display upsc array */
	/*print('<pre>'); print_r($ups); print('</pre>');*/
?>
</div>
<?php include("fend.inc"); ?>
</body>
</html>
