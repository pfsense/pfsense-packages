<?php
/*
	services_imspector_logs.php
	part of pfSense (http://www.pfsense.com/)

	JavaScript Code is GPL Licensed from SmoothWall Express.

	Copyright (C) 2007 Ryan Wagoner <rswagoner@gmail.com>.
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
$log_dir = '/var/imspector';
$imspector_config = $config['installedpackages']['imspector']['config'][0];

$border_color			= '#c0c0c0';
$default_bgcolor		= '#eeeeee';

$list_protocol_color	= '#000000';
$list_local_color 		= '#000000';
$list_remote_color 		= '#000000';
$list_convo_color		= '#000000';

$list_protocol_bgcolor	= '#cccccc';
$list_local_bgcolor		= '#dddddd';
$list_remote_bgcolor	= '#eeeeee';
$list_end_bgcolor		= '#bbbbbb';

$convo_title_color		= 'black';
$convo_local_color		= 'blue';
$convo_remote_color		= 'red';

$convo_title_bgcolor	= '#cccccc';
$convo_local_bgcolor	= '#dddddd';
$convo_remote_bgcolor	= '#eeeeee';

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
				$new_format = '([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),(.*)';
				$old_format = '([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),(.*)';
				preg_match("/${new_format}|${old_format}/", $line, $matches);
				$address = $matches[1];
				$timestamp = $matches[2];
				$direction = $matches[3];
				$type = $matches[4];
				$filtered = $matches[5];
				if(count($matches) == 8) {
					$category = $matches[6];
					$data = $matches[7];
				} else {
					$category = "";
					$data = $matches[6];
				}

				if($direction == '0') {
					$bgcolor = $convo_remote_bgcolor;
					$user = "&lt;<span style='color: $convo_remote_color;'>$remoteuser</span>&gt;";
				}
				if($direction == '1') {
					$bgcolor = $convo_local_bgcolor;	
					$user = "&lt;<span style='color: $convo_local_color;'>$localuser</span>&gt;";
				}

				$time = strftime("%H:%M:%S", $timestamp);

				print("<tr bgcolor='$bgcolor'><td style='width: 30px; vertical-align: top;'>[$time]</td>\n
						<td style=' width: 60px; vertical-align: top;'>$user</td>\n
						<td style=' width: 60px; vertical-align: top;'>$category</td>\n
						<td style='vertical-align: top;'>$data</td></tr>\n");
			}
			print("</table>\n");
			fclose($fd);
		}
	}
	exit;
}
/* defaults to this page but if no settings are present, redirect to setup page */
if(!$imspector_config["enable"] || !$imspector_config["iface_array"] || !$imspector_config["proto_array"])
	Header("Location: /pkg_edit.php?xml=imspector.xml&id=0");

$pgtitle = "Services: IMSpector Log Viewer";
include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
//$pfSenseHead->addMeta("<meta http-equiv=\"refresh\" content=\"120;url={$_SERVER['SCRIPT_NAME']}\" />");
//echo $pfSenseHead->getHTML();
?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<script type="text/javascript" language="javascript" src="/javascript/scriptaculous/prototype.js"></script>
<script type="text/javascript" language="javascript" src="/javascript/scriptaculous/scriptaculous.js"></script>
<?php include("fbegin.inc"); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("IMSpector Log Viewer "), true, "/services_imspector_logs.php");
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

function xmlhttpPost(){
	var url = "/services_imspector_logs.php"
	new Ajax.Request(url, {
		method: 'post',
		parameters: {
			mode: 'render',
			section: section
		},
		onSuccess: function(transport){
			var response = transport.responseText || "";
			updatepage(response);
			$('im_status').style.display = "none";
		},
		onLoading: function(){
			$('im_status').style.display = "inline";
		}
	});
}

function updatepage(str)
{
	/* update the list of conversations ( if we need to ) */
	var parts = str.split("--END--\\n");
	var lines = parts[0].split("\\n");
			
	for (var line = 0 ; line < lines.length ; line ++) {
		var a = lines[line].split("|");

		if (!a[1] || !a[2] || !a[3]) continue;

		var image = a[0];
		var protocol = a[1];
		var login = a[2];
		var destination = a[3];
		var date = a[4];
		

		/* create titling information if needed */
		if (!$(protocol)) {
			$('im_convos').innerHTML += 
				"<div id='" + protocol + "_t' style='width: 100%; background-color: $list_protocol_bgcolor; color: $list_protocol_color;'>" + protocol + "</div>" +
				"<div id='" + protocol + "' style='width: 100%; background-color: $list_local_bgcolor;'></div>";
		}
		if (!$(protocol + "_" + login)) {
			var imageref = "";
			if (image) imageref = "<img src='" + image + "' alt='" + protocol + "'/>";
			$(protocol).innerHTML += 
				"<div id='" + protocol + "_" + login + "_t' style='width: 100%; color: $list_local_color; padding-left: 5px;'>" + imageref + login + "</div>" + 
				"<div id='" + protocol + "_" + login + "' style='width: 100%; background-color: $list_remote_bgcolor; border-bottom: solid 1px $list_end_bgcolor;'></div>";
		}
		if (!$(protocol + "_" + login + "_" + destination)) {
			$(protocol + "_" + login).innerHTML += 
				"<div id='" + protocol + "_" + login + "_" + destination + "_t' style='width: 100%; color: $list_remote_color; padding-left: 10px;'>" + destination + "</div>" + 
				"<div id='" + protocol + "_" + login + "_" + destination + "' style='width: 100%;'></div>";
		}
		if (!$(protocol + "_" + login + "_" + destination + "_" + date)) {
			/* XXX: use observer instead of onclick */
			$(protocol + "_" + login + "_" + destination).innerHTML += 
				"<div id='" + protocol + "_" + login + "_" + destination + "_" + date + 
				"' style='width: 100%; color: $list_convo_color; cursor: pointer; padding-left: 15px;' onClick=" + 
				'"' + "setsection('" + protocol + "|" + login + "|" + destination + "|" + date + "');" + '"' + "' + >&raquo;" + date + "</div>";
		}
	}

	/* determine the title of this conversation */
	var details = parts[1].split(",");
	var title = details[0] + " conversation between <span style='color: $convo_local_color;'>" + details[1] +
		"</span> and <span style='color: $convo_remote_color;'>" + details[2] + "</span>";
	if (!details[1]) title = "&nbsp;";
	if (!parts[2]) parts[2] = "&nbsp;";

	var bottom  = parseInt($('im_content').scrollTop);
	var bottom2 = parseInt($('im_content').style.height);
	var absheight = parseInt( bottom + bottom2 );
	if (absheight == $('im_content').scrollHeight) {
		moveit = 1;
	} else {
		moveit = 0;
	}
	$('im_content').update(parts[2]);
	if (moveit == 1) {
		$('im_content').scrollTop = 0;
		$('im_content').scrollTop = $('im_content').scrollHeight;
	}
	$('im_content_title').update(title);
	the_timeout = setTimeout( "xmlhttpPost();", 5000 );
}

function setsection(value)
{
	section = value;
	clearTimeout(the_timeout);
	xmlhttpPost();
	$('im_content').scrollTop = 0;
	$('im_content').scrollTop = $('im_content').scrollHeight;
}

document.observe('dom:loaded', function() {
	xmlhttpPost();
});

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
          <td width="15%" bgcolor="<?=$default_bgcolor?>" style="overflow: auto; border: solid 1px <?=$border_color?>;">
            <div id="im_convos" style="height: 400px; overflow: auto; overflow-x: hidden;"></div>
          </td>
          <td width="75%" bgcolor="<?=$default_bgcolor?>" style="border: solid 1px <?=$border_color?>;">
            <div id="im_content_title" style="height: 20px; overflow: auto; vertical-align: top; 
              color: <?=$convo_title_color?>; background-color: <?=$convo_title_bgcolor?>;"></div>
            <div id="im_content" style="height: 380px; overflow: auto; vertical-align: bottom; overflow-x: hidden;"></div>
          </td>
        </tr>
      </table>
    </td>
  </tr>
</table>
</div>
<?php include("fend.inc"); ?>
</body>
</html>
