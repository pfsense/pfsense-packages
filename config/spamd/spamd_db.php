<?php
/* $Id$ */
/*
	spamd_db.php
	part of the pfSense project
	Copyright (C) 2006, 2007, 2008 Scott Ullrich
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
	$limit = intval($_POST['limit']);
else
	$limit = "25";
	
/* handle AJAX operations */
if($_GET['action'] or $_POST['action']) {
	/*    echo back buttonid so it can be turned
	 *    back off when request is completed.
	 */
	echo $_GET['buttonid'] . "|";
	if($_GET['action'])
		$action = escapeshellarg($_GET['action']);
	if($_POST['action'])
		$action = escapeshellarg($_POST['action']);
	if($_GET['srcip'])
		$srcip = $_GET['srcip'];
	if($_POST['srcip'])
		$srcip = $_POST['srcip'];
	if($_POST['toaddress']) 
		$toaddress = escapeshellarg($_POST['toaddress']);
	$srcip = str_replace("<","",$srcip);
	$srcip = str_replace(">","",$srcip);
	$srcip = str_replace(" ","",$srcip);
	// Make input safe
	$srcip = escapeshellarg($srcip);
	/* execute spamdb command */
	if($action == "'whitelist'") {
		exec("/usr/local/sbin/spamdb -d {$srcip}");
		exec("/usr/local/sbin/spamdb -d {$srcip} -T");
		exec("/usr/local/sbin/spamdb -d {$srcip} -t");
		delete_from_blacklist($srcip);
		mwexec("/sbin/pfctl -q -t blacklist -T replace -f /var/db/blacklist.txt");
		exec("echo spamdb -a {$srcip} > /tmp/tmp");
		exec("/usr/local/sbin/spamdb -a {$srcip}");
	} else if($action == "'delete'") {
		exec("/usr/local/sbin/spamdb -d {$srcip}");
		exec("/usr/local/sbin/spamdb -d {$srcip} -T");
		exec("/usr/local/sbin/spamdb -d {$srcip} -t");
		delete_from_blacklist($srcip);
		mwexec("/sbin/pfctl -q -t spamd -T delete $srcip");
		mwexec("/sbin/pfctl -q -t blacklist -T replace -f /var/db/blacklist.txt");		
	} else if($action == "'spamtrap'") {
		exec("/usr/local/sbin/spamdb -d {$srcip}");
		exec("/usr/local/sbin/spamdb -d {$srcip} -T");
		exec("/usr/local/sbin/spamdb -d {$srcip} -t");
		exec("/usr/local/sbin/spamdb -a {$srcip} -T");
	} else if($action == "'trapped'") {
		exec("/usr/local/sbin/spamdb -T -d {$toaddress}");
		exec("/usr/local/sbin/spamdb -T -a '{$toaddress}'");	
	}
	/* signal a reload for real time effect. */
	mwexec("killall -HUP spamlogd");
	exit;
}

/* spam trap e-mail address */
if($_POST['spamtrapemail'] <> "") {
	$spamtrapemail = escapeshellarg($_POST['spamtrapemail']);
	exec("/usr/local/sbin/spamdb -d {$spamtrapemail}");
	exec("/usr/local/sbin/spamdb -d -T {$spamtrapemail}");
	exec("/usr/local/sbin/spamdb -d -t {$spamtrapemail}");
	exec("/usr/local/sbin/spamdb -T -a '{$toaddress}'");	

	mwexec("killall -HUP spamlogd");
	$savemsg = htmlentities($_POST['spamtrapemail']) . " added to spam trap database.";
}

if($_GET['getstatus'] <> "") {
	$status = exec("/usr/local/sbin/spamdb | grep " . escapeshellarg($_GET['getstatus']));
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
	$spamtrapemail = escapeshellarg($_GET['spamtrapemail']);
	$status = exec("spamdb -T -a {$spamtrapemail}");
	mwexec("killall -HUP spamlogd");
	if($status)
		echo $status;
	else 
		echo htmlentities($_POST['spamtrapemail']) . " added to spam trap database.";
	exit;
}

/* whitelist e-mail address */
if($_GET['whitelist'] <> "") {
	$spamtrapemail = escapeshellarg($_GET['spamtrapemail']);
	$status = exec("spamdb -a {$spamtrapemail}");
	mwexec("killall -HUP spamlogd");
	if($status)
		echo $status;
	else 
		echo htmlentities($_POST['spamtrapemail']) . " added to whitelist database.";
	exit;
}

function delete_from_blacklist($srcip) {
	config_lock();
	$blacklist = split("\n", file_get_contents("/var/db/blacklist.txt"));
	$fd = fopen("/var/db/blacklist.txt", "w");
	foreach($blacklist as $bl) {
		if($bl <> "")
			if(!stristr($bl, $srcip))
				fwrite($fd, "{$bl}\n");
	}
	fclose($fd);
	mwexec("/sbin/pfctl -q -t spamd -T delete {$srcip}");
	mwexec("/sbin/pfctl -q -t blacklist -T replace -f /var/db/blacklist.txt");
	config_unlock();
}

function delete_from_whitelist($srcip) {
	config_lock();
	$whitelist = split("\n", file_get_contents("/var/db/whitelist.txt"));
	$fd = fopen("/var/db/whitelist.txt", "w");
	foreach($whitelist as $wl) {
		if($wl <> "")
			if(!stristr($wl, $srcip))
				fwrite($fd, "{$wl}\n");
	}
	fclose($fd);
	mwexec("/sbin/pfctl -q -t spamd -T delete $srcip");
	mwexec("/sbin/pfctl -q -t whitelist -T replace -f /var/db/whitelist.txt");
	config_unlock();
}

$pgtitle = "SpamD: Database";
include("head.inc");

if(file_exists("/var/db/whitelist.txt"))
	$whitelist_items = `cat /var/db/whitelist.txt | wc -l`;
else 
	$whitelist_items = 0;
	
if(file_exists("/var/db/blacklist.txt"))
	$blacklist_items = `cat /var/db/blacklist.txt | wc -l`;
else 
	$blacklist_items = 0;

// Get an overall count of the database
$spamdb_items = `/usr/local/sbin/spamdb | wc -l`;

// Get blacklist and whitelist count from database
$spamdb_white = `/usr/local/sbin/spamdb | grep WHITE | wc -l`;
$spamdb_black = `/usr/local/sbin/spamdb | grep BLACK | wc -l`;
$spamdb_grey = `/usr/local/sbin/spamdb | grep GREY | wc -l`;

// Now count the user contributed whitelist and blacklist count
$whitelist_items = $whitelist_items + $spamdb_white;
$blacklist_items = $blacklist_items + $spamdb_black;

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
		

<table>
<tr><td align="right">Filter by test:</td><td><input name="filter" value="<?=$filter?>"></input></td><td><input type="submit" value="Filter"></td><td>&nbsp;&nbsp;Inverse filter (NOT):</td><td><input type="checkbox" id="not" name="not" <?php if($not) echo " CHECKED"; ?>></td></tr>
<tr><td align="right">Limit:</td><td><input name="limit" value="<?=$limit?>"></input></td></tr>
<tr><td>&nbsp;</td></tr>
<tr><td align="right">* Add spam trap E-mail address:</td><td><input name="spamtrapemail" value="<?=$spamtrapemail?>"></input></td><td><input type="submit" value="Add"></td></tr>
</table><br>
		
		
		
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
			$cmd = "/usr/local/sbin/spamdb | grep -v \"" . escapeshellarg($filter) . "\" | tail -n {$limit}";
			fwrite($fd, $cmd);
			fclose($fd);
			$pkgdb = split("\n", `$cmd`);
			if(file_exists("/var/db/blacklist.txt")) {
				$cmd = "cat /var/db/blacklist.txt | grep -v \"" . escapeshellarg($filter) . "\" ";
				$pkgdba = split("\n", `$cmd`);
				foreach($pkgdba as $pkg) {
					$pkgdb[] = "TRAPPED|{$pkg}|1149324397";	
				}				
			}			
		} else {
			
			$cmd = "/usr/local/sbin/spamdb | grep " . escapeshellarg($filter) . " | tail -n {$limit}";

			$pkgdb = split("\n", `$cmd`);
			if(file_exists("/var/db/blacklist.txt")) {
				$cmd = "cat /var/db/blacklist.txt | grep " . escapeshellarg($filter);
				$pkgdba = split("\n", `$cmd`);
				foreach($pkgdba as $pkg) {
					$pkgdb[] = "TRAPPED|{$pkg}|1149324397";
				}
				echo "<!-- $pkgdb -->";
			}
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
		if(!$pkgdb_row) 
			continue;
		$pkgdb_split = split("\|", $pkgdb_row);

		/*

  			For TRAPPED entries the format is:

        	type|ip|expire

  				where type will be TRAPPED, IP will be the IP address blacklisted due to
  				hitting a spamtrap, and expire will be when the IP is due to be removed
  				from the blacklist.

  			For GREY entries, the format is:

        	type|source IP|helo|from|to|first|pass|expire|block|pass

  			For WHITE entries, the format is:

        	type|source IP|||first|pass|expire|block|pass

		*/
		switch($pkgdb_split[0]) {
			case "SPAMTRAP":
				$recordtype = htmlentities($pkgdb_split[0]);
				$srcip = htmlentities($pkgdb_split[1]);
				$fromaddress = htmlentities($pkgdb_split[3]);
				$toaddress = htmlentities($pkgdb_split[4]);
				$attempts = htmlentities($pkgdb_split[8]);			
				break;			
			case "TRAPPED":
				$recordtype = htmlentities($pkgdb_split[0]);
				$srcip = htmlentities($pkgdb_split[1]);
				$fromaddress = "";
				$toaddress = "";
				$attempts = "";
				break;
			case "GREY":
				$recordtype = htmlentities($pkgdb_split[0]);
				$srcip = htmlentities($pkgdb_split[1]);
				$fromaddress = htmlentities($pkgdb_split[3]);
				$toaddress = htmlentities($pkgdb_split[4]);
				$attempts = htmlentities($pkgdb_split[8]);			
				break;
			case "WHITE":
				$recordtype = htmlentities($pkgdb_split[0]);
				$srcip = htmlentities($pkgdb_split[1]);
				$fromaddress = "";
				$toaddress = "";
				$attempts = htmlentities($pkgdb_split[8]);			
				break;
		}
		if($srcip == "" and $fromaddress == "" and $toaddress == "") 
			continue;
		echo "<tr id=\"{$rows}\">";
		echo "<td class=\"listr\">{$recordtype}</td>";		
		echo "<td class=\"listr\">{$srcip}</td>";
		echo "<td class=\"listr\">{$fromaddress}</td>";		
		echo "<td class=\"listr\">{$toaddress}</td>";
		echo "<td class=\"listr\">{$attempts}</td>";
		echo "<td>";
		$rowtext = "<NOBR><a href='javascript:toggle_on(\"w{$rows}\", \"/themes/{$g['theme']}/images/icons/icon_plus_p.gif\"); getURL(\"spamd_db.php?buttonid=w{$rows}&srcip={$srcip}&action=whitelist\", outputrule);'><img title=\"Add to whitelist\" name='w{$rows}' id='w{$rows}' border=\"0\" alt=\"Add to whitelist\" src=\"/themes/{$g['theme']}/images/icons/icon_plus.gif\"></a> ";
		$rowtext .= "<a href='javascript:toggle_on(\"b{$rows}\", \"/themes/{$g['theme']}/images/icons/icon_trapped_p.gif\");getURL(\"spamd_db.php?buttonid=b{$rows}&srcip={$srcip}&action=trapped\", outputrule);'><img title=\"Blacklist\" name='b{$rows}' id='b{$rows}' border=\"0\" alt=\"Blacklist\" src=\"/themes/{$g['theme']}/images/icons/icon_trapped.gif\"></a> ";
		$rowtext .= "<a href='javascript:toggle_on(\"d{$rows}\", \"/themes/{$g['theme']}/images/icons/icon_x_p.gif\");getURL(\"spamd_db.php?buttonid=d{$rows}&srcip={$srcip}&action=delete\", outputrule);'><img title=\"Delete\" border=\"0\" name='d{$rows}' id='d{$rows}' alt=\"Delete\" src=\"./themes/{$g['theme']}/images/icons/icon_x.gif\"></a>";
		$rowtext .= "<a href='javascript:toggle_on(\"s{$rows}\", \"/themes/{$g['theme']}/images/icons/icon_plus_bl_p.gif\");getURL(\"spamd_db.php?buttonid=s{$rows}&spamtrapemail={$toaddress}&action=spamtrap\", outputrule);'><img title=\"Spamtrap\" name='s{$rows}' id='s{$rows}' border=\"0\" alt=\"Spamtrap\" src=\"./themes/{$g['theme']}/images/icons/icon_plus_bl.gif\"></a> ";

		echo $rowtext;
		
		echo "</td></tr>";

		$rows++;
	}	
?>	</td></tr></table>
	<tr><td>
		<?php echo "<font face=\"arial\"><p><b>" . $rows . "</b> rows returned."; ?>
		<p>
		* NOTE: adding an e-mail address to the spamtrap automatically traps any server trying to send e-mail to this address.
	</td></tr>
	</table>
	</div>
	</td>
  </tr>
</table>
</form>
<br>
<span class="vexpl"><strong><span class="red">Note:</span> Clicking on the action icons will invoke a AJAX query and the page will not refresh.   Click refresh in you're browser if you wish to view the changes in status.</strong></span>
<br>
		<p><font size="-2"><b>Database totals:</b><br><font size="-3"><br>
		<?php
			echo "{$whitelist_items} total items in the whitelist.<br>";
			echo "{$blacklist_items} total items in the blacklist.<br>";
			echo "{$spamdb_grey} total items in the greylist.<br>";			
			echo "{$spamdb_items} total items in the SpamDB.<br>";
		?>
<?php include("fend.inc"); ?>
</body>
</html>
