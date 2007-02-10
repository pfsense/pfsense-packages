<?php
/*
	services_imspector_logs.php
	part of pfSense (http://www.pfsense.com/)

	JavaScript Code is GPL Licensed from SmoothWall Express.

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

/* variables */
$log_dir = '/var/log/imspector';

$protocol_color			= '#06264d';
$local_color 			= '#1d398b';
$remote_color 			= '#2149c1';
$conversation_color		= '#335ebe';

$local_user_color		= 'blue';
$local_user_bgcolor		= '#e5e5f3';
$remote_user_color		= 'green';
$remote_user_bgcolor	= '#efeffa';

/* functions */
function convert_dir_list ($topdir) {
	if (!is_dir($topdir)) return;
	if ($dh = opendir($topdir)) {
		while (($file = readdir($dh)) !== false) {
			if(!preg_match('/^\./', $file) == 0) continue;
			if (is_dir("$topdir/$file")) {
				$list .= convert_dir_list("$topdir/$file");
			} else {
				$list .= "$topdir/$file\n";
			}
		}
		closedir($dh);
	}
	return $list;
}

/* ajax response */
if ($_POST['mode'] == "render") {

	/* user list */
	print(str_replace(array($log_dir,'/'),array('','|'),convert_dir_list($log_dir)));
	print("--END--\n");

	/* log files */
	if ($_POST['section'] != "none") {
		$section = explode('|',$_POST['section']);
		$protocol = $section[0];	
		$localuser = $section[1];
		$remoteuser = $section[2];
		$conversation = $section[3];

		/* conversation title */
		print(implode(', ', $section)."\n");
		print("--END--\n");

		/* conversation content */
		$filename = $log_dir.'/'.implode('/', $section);
		if($fd = fopen($filename, 'r')) {
			print("<table width='100%' border='0' cellpadding='2' cellspacing='0'>\n");
			while (!feof($fd)) {
				$line = fgets($fd);
				if(feof($fd)) continue;

				preg_match('/([^,]*),([^,]*),([^,]*),(.*)/', $line, $matches);
				$address = $matches[1];
				$timestamp = $matches[2];
				$type = $matches[3];
				$data = $matches[4];

				if($type == '1') $user = "&lt;<span style='color: $remote_user_color;'>$remoteuser</span>&gt;";
				if($type == '2') $user = "&lt;<span style='color: $local_user_color;'>$localuser</span>&gt;";

				if($type == '1') $bgcolor = $remote_user_bgcolor;
				if($type == '2') $bgcolor = $local_user_bgcolor;

				$time = strftime("%H:%M:%S", $timestamp);

				print("<tr bgcolor='$bgcolor'><td style='width: 30px; vertical-align: top;'>[$time]</td>\n
						<td style=' width: 60px; vertical-align: top;'>$user</td>\n
						<td style='vertical-align: top;'>$data</td></tr>\n");
			}
			print("</table>\n");
			fclose($fd);
		}
	}
	exit;
}
/* defaults to this page but if no settings are present, redirect to setup page */
if(!$config['installedpackages']['imspector']['config'][0]['iface_array'] ||
	!$config['installedpackages']['imspector']['config'][0]['enable'])
	Header("Location: /pkg_edit.php?xml=imspector.xml&id=0");

$pgtitle = "Services: IMSpector Log Viewer";
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
	$tab_array[] = array(gettext("IMSpector Log Viwer "), true, "/services_imspector_logs.php");
	$tab_array[] = array(gettext("IMSpector Settings "), false, "/pkg_edit.php?xml=imspector.xml&id=0");
	display_top_tabs($tab_array);
?>
</table>

<?php
$zz = <<<EOD
<script type="text/javascript">
var section = 'none';
var moveit = 1;
var the_timeout;

function xmlhttpPost()
{
	var xmlHttpReq = false;
	var self = this;

	if (window.XMLHttpRequest)
		self.xmlHttpReq = new XMLHttpRequest();
	else if (window.ActiveXObject)
		self.xmlHttpReq = new ActiveXObject("Microsoft.XMLHTTP");

	self.xmlHttpReq.open('POST', 'services_imspector_logs.php', true);
	self.xmlHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');

	self.xmlHttpReq.onreadystatechange = function() {
		if (self.xmlHttpReq && self.xmlHttpReq.readyState == 4)
			updatepage(self.xmlHttpReq.responseText);
	}

	document.getElementById('im_status').style.display = "inline";
	self.xmlHttpReq.send("mode=render&section=" + section);
}

function updatepage(str)
{
	/* update the list of conversations ( if we need to ) */
	var parts = str.split("--END--\\n");
	var lines = parts[0].split("\\n");
			
	for (var line = 0 ; line < lines.length ; line ++) {
		var a = lines[line].split("|");

		if (!a[1] || !a[2] || !a[3]) continue;

		/* create titling information if needed */
		if (!document.getElementById(a[1])) {
			document.getElementById('im_convos').innerHTML += 
				"<div id='" + a[1] + "_t' style='width: 100%; background-color: #d9d9f3; color: $protocol_color;'>" + a[1] + "</div>" +
				"<div id='" + a[1] + "' style='width: 100%; background-color: #e5e5f3;'></div>";
		}
		if (!document.getElementById(a[1] + "_" + a[2])) {
			var imageref = "";
			if (a[0]) imageref = "<img src='" + a[0] + "' alt='" + a[1] + "'/>";
			document.getElementById(a[1]).innerHTML += 
				"<div id='" + a[1] + "_" + a[2] + "_t' style='width: 100%; color: $local_color; padding-left: 5px;'>" + imageref + a[2] + "</div>" + 
				"<div id='" + a[1] + "_" + a[2] + "' style='width: 100%; background-color: #efeffa; border-bottom: solid 1px #d9d9f3;'></div>";
		}
		if (!document.getElementById(a[1] + "_" + a[2] + "_" + a[3])) {
			document.getElementById(a[1] + "_" + a[2]).innerHTML += 
				"<div id='" + a[1] + "_" + a[2] + "_" + a[3] + "_t' style='width: 100%; color: $remote_color; padding-left: 10px;'>" + a[3] + "</div>" + 
				"<div id='" + a[1] + "_" + a[2] + "_" + a[3] + "' style='width: 100%;'></div>";
		}
		if (!document.getElementById(a[1] + "_" + a[2] + "_" + a[3] + "_" + a[4])) {
			document.getElementById(a[1] + "_" + a[2] + "_" + a[3]).innerHTML += 
				"<div id='" + a[1] + "_" + a[2] + "_" + a[3] + "_" + a[4] + 
				"' style='width: 100%; color: $conversation_color; cursor: pointer; padding-left: 15px;' onClick=" + 
				'"' + "setsection('" + a[1] + "|" + a[2] + "|" + a[3] + "|" + a[4] + "');" + '"' + "' + >&raquo;" + a[4] + "</div>";
		}
	}

	/* determine the title of this conversation */
	var details = parts[1].split(",");
	var title = details[0] + " conversation between <span style='color: $local_user_color;'>" + details[ 1 ] +
		"</span> and <span style='color: $remote_user_color;'>" + details[2] + "</span>";
	if (!details[1]) title = "&nbsp;";
	if (!parts[2]) parts[2] = "&nbsp;";

	document.getElementById('im_status').style.display = "none";
	var bottom  = parseInt(document.getElementById('im_content').scrollTop);
	var bottom2 = parseInt(document.getElementById('im_content').style.height);
	var absheight = parseInt( bottom + bottom2 );
	if (absheight == document.getElementById('im_content').scrollHeight) {
		moveit = 1;
	} else {
		moveit = 0;
	}
	document.getElementById('im_content').innerHTML = parts[2];
	if (moveit == 1) {
		document.getElementById('im_content').scrollTop = 0;
		document.getElementById('im_content').scrollTop = document.getElementById('im_content').scrollHeight;
	}
	document.getElementById('im_content_title').innerHTML = title;
	the_timeout = setTimeout( "xmlhttpPost();", 5000 );
}

function setsection(value)
{
	section = value;
	clearTimeout(the_timeout);
	xmlhttpPost();
	document.getElementById('im_content').scrollTop = 0;
	document.getElementById('im_content').scrollTop = document.getElementById('im_content').scrollHeight;
}
</script>
EOD;
print($zz);
?>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
	<td class="tabcont">
      <div style='width: 100%; text-align: right;'><span id='im_status' style='display: none;'>Updating</span>&nbsp;</div>
      <table width="100%">
        <tr>
		  <td width="15%" bgcolor="#efeffa" style="overflow: auto; border: solid 1px #c0c0c0;">
      	    <div id="im_convos" style="height: 400px; overflow: auto; overflow-x: hidden;"></div>
      	  </td>
          <td width="75%" bgcolor="#efeffa" style="border: solid 1px #c0c0c0;">
      	    <div id="im_content_title" style="height: 20px; overflow: auto; vertical-align: top; background-color: #d9d9f3;"></div>
            <div id="im_content" style="height: 380px; overflow: auto; vertical-align: bottom; overflow-x: hidden;"></div>
          </td>
        </tr>
      </table>
    </td>
  </tr>
</table>

<script type="text/javascript">xmlhttpPost();</script>

</div>
<?php include("fend.inc"); ?>
</body>
</html>
