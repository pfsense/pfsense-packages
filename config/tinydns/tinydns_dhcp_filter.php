<?php
/*
	tinydns_dhcp_filter.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2006 Scott Ullrich
	Copyright (C) 2007 Goffredo Andreone
	Copyright (C) 2015 ESF, LLC
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

##|+PRIV
##|*IDENT=page-tinydns-dhcp-filter
##|*NAME=TinyDNS: DHCP Domains
##|*DESCR=Allow access to the TinyDNS View Domains page
##|*MATCH=tinydns_dhcp_filter*
##|-PRIV

require("/usr/local/pkg/tinydns.inc");
require("guiconfig.inc");

$closehead = false;
$pgtitle = "TinyDNS: DHCP Domains";
include("head.inc");

?>
<meta http-equiv="refresh" content="60;url=<?php print $_SERVER['SCRIPT_NAME']; ?>">
</head>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if ($savemsg) print_info_box($savemsg); ?>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=tinydns.xml&id=0");
	$tab_array[] = array(gettext("Add/Edit Record"), false, "/tinydns_filter.php");
	$tab_array[] = array(gettext("Failover Status"), false, "/tinydns_status.php");
	$tab_array[] = array(gettext("Logs"), false, "/tinydns_view_logs.php");
	$tab_array[] = array(gettext("Zone Sync"), false, "/pkg_edit.php?xml=tinydns_sync.xml");
	$tab_array[] = array(gettext("New domain wizard"), false, "/wizard.php?xml=new_zone_wizard.xml");	
	display_top_tabs($tab_array);
?>
</table>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
   <tr>
     <td class="tabcont" >
      <form action="tinydns_dhcp_filter.php" method="post">
    </form>
    </td>
   </tr>
   <tr>
    <td class="tabcont" >
      <table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
          <td width="45%" class="listhdrr">Fully Qualified Domain Name (Hostname)</td>
          <td width="15%" class="listhdrr">Record types</td>
          <td width="5%" class="listhdrr">rDNS</td>
          <td width="35%" class="listhdrr">IP Address or FQDN</td>
	</tr>

<?php
if(file_exists("/service/tinydns/root/data"))
	$tinydns_data = file_get_contents("/service/tinydns/root/data");
else
	$tinydns_data = "";

$datalen = strlen($tinydns_data);
$startofrecord = 0;
while ($startofrecord < $datalen ){	
	$endofrecord = strpos($tinydns_data,"\n",$startofrecord);
	$dnsrecord = substr($tinydns_data,$startofrecord,$endofrecord-$startofrecord);
	$startofrecord = $endofrecord + 1;
	
	$col1 = strpos($dnsrecord,":");
	$fqdn = substr($dnsrecord,1,$col1-1);
	$rtypes = tinydns_get_dns_record_type($dnsrecord);
	if($rtypes[0] == "SOA")
		$ip = substr($dnsrecord,$col1+2);
	else
		$ip = substr($dnsrecord,$col1+1);
		/* For root-servers get rid of the leading colon */	
		if($ip[0] == ":")$ip = substr($ip,1);
	if(!$rtypes[2])$rtypes[2] = "na";
	if(!$fqdn)$fqdn = ".";
	echo "<tr>";
	echo "<td class=\"listlr\">$fqdn</td>";
	echo "<td class=\"listlr\">$rtypes[0]  $rtypes[1]</td>";
	echo "<td class=\"listlr\">$rtypes[2]</td>";
	echo "<td class=\"listlr\">$ip</td>";
	echo "</tr>";
}	
?>
      </table>
     </td>
    </tr>
</table>
</div>
<?php include("fend.inc"); ?>
</body>
</html>
