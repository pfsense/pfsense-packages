<?php
/* $Id$ */
/*
	spamd_db.php
	Copyright (C) 2004 Scott Ullrich
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

if($_GET['action'] or $_POST['action']) {
	if($_GET['action'])
		$action = $_GET['action'];
	if($_POST['action'])
		$action = $_POST['action'];
	if($_GET['srcip'])
		$srcip = $_GET['srcip'];
	if($_POST['srcip'])
		$srcip = $_POST['srcip'];
	$pkgdb = split("\n", `/usr/local/sbin/spamdb`);
	if($action == "whitelist") {
		mwexec("/usr/local/sbin/spamdb -a {$srcip}");
	} else if($action == "spamtrap") {
		mwexec("/usr/local/sbin/spamdb -a {$srcip} -T");
	} else if($action == "trapped") {
		mwexec("/usr/local/sbin/spamdb -a {$srcip} -t");
	}
	mwexec("killall -HUP spamlogd");
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
<script type="text/javascript" language="javascript" src="row_toggle.js">
</script>
<script language="javascript">
function outputrule(req) {
	//alert(req.content);
}
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
Filter: <input name="filter" value="<?=$filter?>"></input> <input type="submit" value="Filter"><p>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
  <tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array("SpamD Sources", false, "/pkg.php?xml=spamd.xml");
	$tab_array[] = array("SpamD Whitelist", false, "/pkg.php?xml=spamd_whitelist.xml");
	$tab_array[] = array("SpamD Settings", false, "/pkg_edit.php?xml=spamd_settings.xml&id=0");
	$tab_array[] = array("SpamD Database", true, "/spamd_db.php");
	display_top_tabs($tab_array);
?>
 </td></tr>
  <tr>
    <td>
	<div id="mainarea">
	<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr id="frheader">
		  <td class="listhdrr">Action</td>
		  <td class="listhdrr">Type</td>
                  <td class="listhdrr">IP</td>
                  <td class="listhdrr">From</td>
                  <td class="listhdrr">To</td>
                  <td class="listhdr">Attempts</td>
		</tr>
<?php
	$pkgdb = split("\n", `/usr/local/sbin/spamdb`);
	$rows = 0;
	$lastseenip = "";
	$srcip = "|";
	foreach($pkgdb as $pkgdb_row) {
		$dontdisplay = false;
		$rowtext = "";
		$rowtext .= "<span class=\"{$rows}\"></span>";
		$rowtext .= "<tr id=\"{$rows}\">";
		$pkgdb_split = split("\|", $pkgdb_row);
		$rowtext .= "<td class=\"listr\">";
		$srcip = $pkgdb_split[1];
		$rowtext .= "<a onClick='getURL(\"spamd_db.php?srcip={$srcip}&action=whitelist\", outputrule);' href='#{$rows}'>Whitelist</a> ";
		$rowtext .= " | <a onClick='getURL(\"spamd_db.php?srcip={$srcip}&action=trapped\", outputrule);' href='#{$rows}'>Trapped</a> ";
		$rowtext .= " | <a onClick='getURL(\"spamd_db.php?srcip={$srcip}&action=spamtrap\", outputrule);' href='#{$rows}'>Blacklist</a> ";
		$rowtext .= "</td>";
		$column = 0;
		foreach($pkgdb_split as $col) {
			if($column == 4 || $column == 5  || $column == 6 || $column == 8) {
				$column++;
				continue;
			}
			if($col == "")
				$dontdisplay = true;
			$col = str_replace("<","",$col);
			$col = str_replace(">","",$col);
			$rowtext .= "<td class=\"listr\">{$col}</td>";
			$column++;
		}
		$rowtext .= "</tr>";
		if($srcip == "")
			$dontdisplay = true;
		if($lastseenip == $srcip and $filter == "")
			$dontdisplay = true;
		if($filter <> "") {
			if(strstr($rowtext, $filter) == true)
				$dontdisplay = false;
			else
				$dontdisplay = true;
		}
		if($dontdisplay == false) {
			echo $rowtext;
			$lastseenip = $srcip;
		}
		$rows++;
	}	
?>
	</table>
	</div>
	</td>
  </tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
