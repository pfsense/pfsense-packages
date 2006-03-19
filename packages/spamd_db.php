<?php
/* $Id$ */
/*
	spamd_db.php
	Copyright (C) 2006 Scott Ullrich
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

if($_POST['filter'])
	$filter = $_POST['filter'];
if($_POST['not'])
	$not = true;
if($_POST['limit'])
	$limit = $_POST['limit'];
else
	$limit = "25";
	
/* handle AJAX operations */
if($_GET['action'] or $_POST['action']) {
	/*    echo back buttonid so it can be turned
	 *    back off when request is completed.
	 */
	echo $_GET['buttonid'] . "|";
	if($_GET['action'])
		$action = $_GET['action'];
	if($_POST['action'])
		$action = $_POST['action'];
	if($_GET['srcip'])
		$srcip = $_GET['srcip'];
	if($_POST['srcip'])
		$srcip = $_POST['srcip'];
	$srcip = str_replace("<","",$srcip);
	$srcip = str_replace(">","",$srcip);
	$srcip = str_replace(" ","",$srcip);
	/* execute spamdb command */
	if($action == "whitelist") {
		exec("echo spamdb -a {$srcip} > /tmp/tmp");
		exec("/usr/local/sbin/spamdb -a {$srcip}");
	} else if($action == "delete") {
		exec("/usr/local/sbin/spamdb -d {$srcip}");
		exec("/usr/local/sbin/spamdb -d \"<{$srcip}>\" -T");
		exec("/usr/local/sbin/spamdb -d \"<{$srcip}>\" -t");
	} else if($action == "spamtrap") {
		exec("/usr/local/sbin/spamdb -d {$srcip}");
		exec("/usr/local/sbin/spamdb -d \"<{$srcip}>\" -T");
		exec("/usr/local/sbin/spamdb -d \"<{$srcip}>\" -t");		
		exec("/usr/local/sbin/spamdb -a {$srcip} -T");
	} else if($action == "trapped") {
		exec("/usr/local/sbin/spamdb -a {$srcip} -t");
	}
	/* signal a reload for real time effect. */
	mwexec("killall -HUP spamlogd");
	exit;
}

/* spam trap e-mail address */
if($_POST['spamtrapemail'] <> "") {
	mwexec("/usr/local/sbin/spamdb -T -a \"<{$_POST['spamtrapemail']}>\"");
	mwexec("killall -HUP spamlogd");
	$savemsg = $_POST['spamtrapemail'] . " added to spam trap database.";
}

if($_GET['getstatus'] <> "") {
	$status = exec("/usr/local/sbin/spamdb | grep \"{$_GET['getstatus']}\"");
	if(stristr($status, "WHITE") == true) {
		echo "WHITE";
	} else if(stristr($status, "TRAPPED") == true) {
		echo "TRAPPED";
	} else if(stristr($status, "GREY") == true) {
		echo "GREY";
	} else if(stristr($status, "SPAMTRAP") == true) {
		echo "SPAMTRAP";
	} else {
		echo "NOT FOUND";	
	}	
	exit;
}

/* spam trap e-mail address */
if($_GET['spamtrapemail'] <> "") {
	$status = exec("spamdb -T -a \"<{$_GET['spamtrapemail']}>\"");
	mwexec("killall -HUP spamlogd");
	if($status)
		echo $status;
	else 
		echo $_POST['spamtrapemail'] . " added to spam trap database.";
	exit;
}

/* spam trap e-mail address */
if($_GET['whitelist'] <> "") {
	$status = exec("spamdb -a \"<{$_GET['spamtrapemail']}>\"");
	mwexec("killall -HUP spamlogd");
	if($status)
		echo $status;
	else 
		echo $_POST['spamtrapemail'] . " added to whitelist database.";
	exit;
}


$pgtitle = "SpamD: Database";
include("head.inc");

?>
<body link="#000000" vlink="#000000" alink="#000000">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<form action="spamd_db.php" method="post" name="iform">
<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
<script src="/javascript/scriptaculous/scriptaculous.js" type="text/javascript"></script>
<script type="text/javascript" language="javascript" src="row_toggle.js"></script>
<script src="/javascript/sorttable.js"></script>
<script language="javascript">
function outputrule(req) {
	if(req.content != '') {
		/* response is split by | */
		var itemsplit = req.content.split("|");
		/* turn back off the button */
		toggle_off(itemsplit[0]);
		/* uh oh, we've got an error of some sort */
		if(itemsplit[1] != "")
			alert('An error was detected.\n\n' + req.content);
	}
}
/* toggle button to be on during AJAX request */
function toggle_on(button, image) {
	var item = document.getElementById(button);
	item.src = image;
}
/* turn off button by stripping _p out */
function toggle_off(button) {
	/* no text back?  thats bad. */
	if(button == '')
		return;
	var item = document.getElementById(button);
	var currentbutton = item.src;
	currentbutton = currentbutton.replace("_p.", ".");
	item.src = currentbutton;
	new Effect.Shake(item);
}
/* delete a row */
function delete_row_db(row) {
	row++;
	var el = document.getElementById('maintable');
	el.deleteRow(row);
}
/* standard issue AJAX handler */
if (typeof getURL == 'undefined') {
	getURL = function(url, callback) {
		if (!url)
			throw 'No URL for getURL';
		try {
			if (typeof callback.operationComplete == 'function')
				callback = callback.operationComplete;
		} catch (e) {}
			if (typeof callback != 'function')
				throw 'No callback function for getURL';
		var http_request = null;
		if (typeof XMLHttpRequest != 'undefined') {
		    http_request = new XMLHttpRequest();
		}
		else if (typeof ActiveXObject != 'undefined') {
			try {
				http_request = new ActiveXObject('Msxml2.XMLHTTP');
			} catch (e) {
				try {
					http_request = new ActiveXObject('Microsoft.XMLHTTP');
				} catch (e) {}
			}
		}
		if (!http_request)
			throw 'Both getURL and XMLHttpRequest are undefined';		
		http_request.onreadystatechange = function() {
			if (http_request.readyState == 4) {
				callback( { success : true,
				  content : http_request.responseText,
				  contentType : http_request.getResponseHeader("Content-Type") } );
			}
		}
		http_request.open('GET', url, true);
		http_request.send(null);
	}
}
</script>
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if (file_exists($d_natconfdirty_path)): ?><p>
<?php endif; ?>
<table>
<tr><td align="right">Filter by test:</td><td><input name="filter" value="<?=$filter?>"></input></td><td><input type="submit" value="Filter"></td><td>Inverse filter (NOT):</td><td><input type="checkbox" id="not" name="not" <?php if($not) echo " CHECKED"; ?>></td></tr>
<tr><td align="right">Limit:</td><td><input name="limit" value="<?=$limit?>"></input></td></tr>
<tr><td>&nbsp;</td></tr>
<tr><td align="right">Add spam trap E-mail address:</td><td><input name="spamtrapemail" value="<?=$spamtrapemail?>"></input></td><td><input type="submit" value="Add"></td></tr>
</table><br>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
  <tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array("SpamD External Sources", false, "/pkg.php?xml=spamd.xml");
	$tab_array[] = array("SpamD Whitelist", false, "/pkg.php?xml=spamd_whitelist.xml");
	$tab_array[] = array("SpamD Settings", false, "/pkg_edit.php?xml=spamd_settings.xml&id=0");
	$tab_array[] = array("SpamD Database", true, "/spamd_db.php");
	display_top_tabs($tab_array);
?>
 </td></tr>
  <tr>
    <td>
	<div id="mainarea">
		<table id="maintable" name="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr><td>
			<table id="sortabletable1" name="sortabletable1" class="sortable" width="100%" border="0" cellpadding="0" cellspacing="0">
			    <tr id="frheader">
					<td class="listhdrr">Type</td>
					<td class="listhdrr">IP</td>
					<td class="listhdrr">From</td>
					<td class="listhdrr">To</td>
					<td class="listhdr">Attempts</td>
					<td class="list"></td>
				</tr>
<?php
	if($filter) {
		if($not) {
			$fd = fopen("/tmp/spamdb", "w");
			$cmd = "/usr/local/sbin/spamdb | grep -v \"" . $filter . "\" | tail -n {$limit}";
			fwrite($fd, $cmd);
			fclose($fd);
			$pkgdb = split("\n", `$cmd`);
		} else {
			$cmd = "/usr/local/sbin/spamdb | grep \"{$filter}\" | tail -n {$limit}";
			$pkgdb = split("\n", `$cmd`);
		}
	} else {
		$pkgdb = split("\n", `/usr/local/sbin/spamdb | tail -n {$limit}`);
	}
	$rows = 0;
	$lastseenip = "";
	$srcip = "|";
	foreach($pkgdb as $pkgdb_row) {
		if($rows > $limit)
			break;
		$dontdisplay = false;
		$rowtext = "";
		$rowtext .= "<span class=\"{$rows}\"></span>";
		$rowtext .= "<tr id=\"{$rows}\">";
		$pkgdb_split = split("\|", $pkgdb_row);
		$column = 0;
		foreach($pkgdb_split as $col) {
			if($column == 2) {
				if(strstr($pkgdb_row, "TRAPPED")) {
					$column++;
					continue;
				}
			}
			/* dont display these columns */
			if($column == 4 || $column == 5  || $column == 6 || $column == 8) {
				$column++;
				continue;
			}
			/* don't display if column blank */
			$col = str_replace("<","",$col);
			$col = str_replace(">","",$col);
			/*   if string is really long allow it to be wrapped by
			 *   replacing @ with space@
                         */
			if(strlen($col)>25) {
				$col = str_replace("@"," @",$col);
				$col = str_replace("-"," -",$col);
				$col = str_replace("."," .",$col);
			}
			$rowtext .= "<td class=\"listr\">&nbsp;{$col}&nbsp;</td>";
			$column++;
		}
		if(strstr($pkgdb_row, "TRAPPED")) {
			for($x=0; $x<3; $x++) {
				$rowtext .= "<td class=\"listr\"></td>";
			}
		}
		if(strstr($pkgdb_row, "SPAMTRAP")) {
			for($x=0; $x<3; $x++) {
				$rowtext .= "<td class=\"listr\"></td>";
			}
		}		
		$rowtext .= "<td class=\"list\">&nbsp;";
		$srcip = $pkgdb_split[1];
		$lastrow = $rows - 1;
		$rowtext .= "<NOBR><a href='javascript:toggle_on(\"w{$rows}\", \"/themes/{$g['theme']}/images/icons/icon_plus_p.gif\"); getURL(\"spamd_db.php?buttonid=w{$rows}&srcip={$srcip}&action=whitelist\", outputrule);'><img title=\"Add to whitelist\" name='w{$rows}' id='w{$rows}' border=\"0\" alt=\"Add to whitelist\" src=\"/themes/{$g['theme']}/images/icons/icon_plus.gif\"></a> ";
		$rowtext .= "<a href='javascript:toggle_on(\"b{$rows}\", \"/themes/{$g['theme']}/images/icons/icon_trapped_p.gif\");getURL(\"spamd_db.php?buttonid=b{$rows}&srcip={$srcip}&action=trapped\", outputrule);'><img title=\"Blacklist\" name='b{$rows}' id='b{$rows}' border=\"0\" alt=\"Blacklist\" src=\"/themes/{$g['theme']}/images/icons/icon_trapped.gif\"></a> ";
		$rowtext .= "<a href='javascript:toggle_on(\"d{$rows}\", \"/themes/{$g['theme']}/images/icons/icon_x_p.gif\");getURL(\"spamd_db.php?buttonid=d{$rows}&srcip={$srcip}&action=delete\", outputrule);'><img title=\"Delete\" border=\"0\" name='d{$rows}' id='d{$rows}' alt=\"Delete\" src=\"./themes/{$g['theme']}/images/icons/icon_x.gif\"></a>";
		$rowtext .= "<a href='javascript:delete_row_db(\"{$rows}\"); toggle_on(\"s{$rows}\", \"/themes/{$g['theme']}/images/icons/icon_plus_bl_p.gif\");getURL(\"spamd_db.php?buttonid=s{$rows}&srcip={$srcip}&action=spamtrap\", outputrule);'><img title=\"Spamtrap\" name='s{$rows}' id='s{$rows}' border=\"0\" alt=\"Spamtrap\" src=\"./themes/{$g['theme']}/images/icons/icon_plus_bl.gif\"></a> ";
		$rowtext .= "</NOBR>&nbsp;</td>";		
		$rowtext .= "</tr>";
		if($srcip == "")
			$dontdisplay = true;
		if($lastseenip == $srcip and $filter == "")
			$dontdisplay = true;
		if($dontdisplay == false) {
			echo $rowtext;
			$lastseenip = $srcip;
		}
		$rows++;
	}	
?>	</td></tr></table>
	</table>
	</div>
	</td>
  </tr>
</table>
</form>
<br>
<span class="vexpl"><strong><span class="red">Note:</span> Clicking on the action icons will invoke a AJAX query and the page will not refresh.   Click refresh in you're browser if you wish to view the changes in status.</strong></span>
<br>
<?php include("fend.inc"); ?>
</body>
</html>
