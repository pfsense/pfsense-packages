#!/usr/local/bin/php
<?php
/*
    carp_status.php
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
require("xmlparse_pkg.inc");

function gentitle_pkg($pgname) {
	global $config;
	return $config['system']['hostname'] . "." . $config['system']['domain'] . " - " . $pgname;
}

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<?php $title = "CARP: Status"; ?>
<title><?=gentitle_pkg($title);?></title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link href="gui.css" rel="stylesheet" type="text/css">
</head>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php
include("fbegin.inc");
?>
<p class="pgtitle"><?=$title?></p>
<form action="firewall_nat_out_load_balancing.php" method="post">
<?php if ($savemsg) print_info_box($savemsg); ?>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
 <ul id="tabnav">
  <li class="tabinact"><a href="/pkg.php?xml=carp.xml">CARP Settings</a></li>
  <li class="tabact">CARP Status</li>
 </ul>
</td></tr>
<tr><td class="tabcont">

<table width="100%" border="0" cellpadding="6" cellspacing="0">
<tr>
  <td class="listhdrr"><b><center>Interface</center></b></td>
  <td class="listhdrr"><b><center>Status</center></b></td>
  <td class="listhdrr"><b><center>Sync Status</center></b></td>
</tr>
<?php

if($config['installedpackages']['carp']['config'] <> "")
	foreach($config['installedpackages']['carp']['config'] as $carp) {
		$ipaddress = $carp['ipaddress'];
		$premption = $carp['premption'];
		$password = $carp['password'];
		$netmask = $carp['netmask'];
		$vhid = $carp['vhid'];
		$advskew = $carp['advskew'];
		$pfsync = $carp['pfsync'];
		$synciface = $carp['synciface'];
		$carp_int = find_carp_interface($ipaddress);
		$status = get_carp_interface_status($carp_int);
		if(isset($carp['balancing'])) $balancing = "true"; else $balancing = "false";
		if(isset($carp['premtpion'])) $premption = "true"; else $premption = "false";
		if($synciface <> "") $sync_status = get_pfsync_interface_status($synciface);
		echo "<tr>";
		echo "<td class=\"listlr\"><center>" . $ipaddress . " - " . $carp_int . "</td>";
		echo "<td class=\"listlr\"><center>" . $status . "<br>" . $sync_status . "</td>";
		echo "<td class=\"listlr\"><center>" . $synciface . "</td>";
		echo "</tr>";
	}

?>

</table>
</td></tr>
</table>

</form>
<?php include("fend.inc"); ?>
</body>
</html>

