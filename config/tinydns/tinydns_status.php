<?php
/* $Id$ */
/*
	tinydns_status.php
	part of pfSense (https://www.pfsense.org/)

	Copyright (C) 2006 Scott Ullrich <sullrich@gmail.com>
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

/* Defaults to this page but if no settings are present, redirect to setup page */
if(!$config['installedpackages']['tinydnsdomains']['config'])
	Header("Location: /wizard.php?xml=new_zone_wizard.xml");

if(!$config['installedpackages']['tinydns']['config'][0]['ipaddress']) 
	Header("Location: /pkg_edit.php?xml=tinydns.xml&id=0&savemsg=Please+set+the+binding+ip+address+for+server+operation");

$pgtitle = "TinyDNS: Status";
include("head.inc");

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<?php endif; ?>

<?php if ($savemsg) print_info_box($savemsg); ?>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=tinydns.xml&id=0");
	$tab_array[] = array(gettext("Add/Edit Record"), false, "/tinydns_filter.php");
	$tab_array[] = array(gettext("Failover Status"), true, "/tinydns_status.php");
	$tab_array[] = array(gettext("Logs"), false, "/tinydns_view_logs.php");
	$tab_array[] = array(gettext("Zone Sync"), false, "/pkg_edit.php?xml=tinydns_sync.xml&id=0");
	$tab_array[] = array(gettext("New domain wizard"), false, "/wizard.php?xml=new_zone_wizard.xml");	
	display_top_tabs($tab_array);
?>
</table>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
   <tr>
     <td class="tabcont" >
      <form action="tinydns_status.php" method="post">
    </form>
    </td>
   </tr>
   <tr>
    <td class="tabcont" >
      <table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
          <td width="55%" class="listhdrr">IP</td>
          <td width="15%" class="listhdrr">Status</td>
          <td width="15%" class="listhdrr">In Service</td>
          <td width="25%" class="listhdrr">Response time</td>
		</tr>

<?php
$pingdir = return_dir_as_array("/var/db/pingstatus");
if(file_exists("/var/run/service/tinydns/root/data"))
	$tinydns_data = file_get_contents("/var/run/service/tinydns/root/data");
else
	$tinydns_data = "";
if($config['installedpackages']['tinydnsdomains'])
foreach($config['installedpackages']['tinydnsdomains']['config'] as $ping) {
	if($ping['recordtype'] == "SOA")
		continue;
	if(!$ping['row'])
		continue;
	$ipaddress = $ping['ipaddress'];
	$hostname  = $ping['hostname'];
	$monitorip = $ping['monitorip'];
	if(file_exists("/var/db/pingstatus/$monitorip"))
		$status = file_get_contents("/var/db/pingstatus/$monitorip");
	else
		$status = "N/A";
	if(stristr($tinydns_data, "+{$hostname}:{$ipaddress}"))
		$inservice = "<FONT COLOR='GREEN'>YES</FONT>";
	else
		$inservice = "<FONT COLOR='BLUE'>NO</FONT>";
	echo "<tr>";
	echo "<td class=\"listlr\">";
	echo "$hostname<br>&nbsp;&nbsp;&nbsp;|->&nbsp;$ipaddress";
	echo "</td>";
	echo "<td class=\"listlr\">";
	if(stristr($status,"DOWN"))
		echo "<FONT COLOR='red'>DOWN</FONT>";
	else
		echo "UP";
	echo "</td>";

	echo "<td class=\"listlr\">";
	echo $inservice;
	echo "</td>";

	echo "<td class=\"listlr\">";
	if(!$monitorip)
		$monitorip = $ipaddress;
	if(file_exists("/var/db/pingmsstatus/$monitorip"))
		$msstatus = file_get_contents("/var/db/pingmsstatus/$monitorip");
	else
		$msstatus = "N/A";
	echo "<!-- " . $monitorip . " -->" . $msstatus;
	echo "</td>";
	echo "</tr>";

	foreach($ping['row'] as $row) {
		$ipaddress = $row['failoverip'];
		$monitorip = $row['monitorip'];
		if(file_exists("/var/db/pingstatus/$monitorip"))
			$status = file_get_contents("/var/db/pingstatus/$monitorip");
		else
			$status = "N/A";
		echo "<tr>";
		echo "<td class=\"listlr\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|->&nbsp;&nbsp;";
		echo $ipaddress;
		if($row['loadbalance'])
			echo " (LB)";
		if(stristr($tinydns_data, "+{$hostname}:{$row['failoverip']}"))
			$inservice = "<FONT COLOR='GREEN'>YES</FONT>";
		else
			$inservice = "<FONT COLOR='BLUE'>NO</FONT>";
		echo "</td>";
		echo "<td class=\"listlr\">";
		if(stristr($status,"DOWN"))
			echo "<FONT COLOR='red'>DOWN</FONT>";
		else
			echo "UP";
		echo "</td>";

		echo "<td class=\"listlr\">";
		echo $inservice;
		echo "</td>";

		echo "<td class=\"listlr\">";
		if(!$monitorip)
			$monitorip = $ipaddress;
		if(file_exists("/var/db/pingmsstatus/$monitorip"))
			$msstatus = file_get_contents("/var/db/pingmsstatus/$monitorip");
		else
			$msstatus = "N/A";

		echo "<!-- " . $monitorip . " -->" . $msstatus;
		echo "</td>";
		echo "</tr>";
	}
	echo "<tr><td>&nbsp;</td></tr>";
}
?>
      </table>
     </td>
    </tr>
</table>
</div>
<?php include("fend.inc"); ?>
<meta http-equiv="refresh" content="60;url=<?php print $_SERVER['SCRIPT_NAME']; ?>">
</body>
</html>
