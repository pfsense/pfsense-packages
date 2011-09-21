<?php
/* $Id$ */
/*

 part of pfSense
 All rights reserved.

 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 All rights reserved.

 Pfsense Old snort GUI 
 Copyright (C) 2006 Scott Ullrich.
 
 Pfsense snort GUI 
 Copyright (C) 2008-2012 Robert Zelaya.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. Neither the name of the pfSense nor the names of its contributors 
 may be used to endorse or promote products derived from this software without 
 specific prior written permission.

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

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_new.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");

//Set no caching
header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
header("Cache-Control: no-store, no-cache, must-revalidate");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");

// set page vars

$uuid = $_GET['uuid'];
if (isset($_POST['uuid']))
$uuid = $_POST['uuid'];

if ($uuid == '') {
	echo 'error: no uuid';
	exit(0);
}


$a_list = snortSql_fetchAllSettings('snortDB', 'SnortIfaces', 'uuid', $uuid);


	$pgtitle = "Snort: Interface Define Servers:";
	include("/usr/local/pkg/snort/snort_head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<!-- loading msg -->
<div id="loadingWaiting">
	<div class="snortModal" style="top: 200px; left: 700px;">
		<div class="snortModalTop">
			<!-- <div class="snortModalTopClose"><a href="javascript:hideLoading('#loadingWaiting');"><img src="/snort/images/close_9x9.gif" border="0" height="9" width="9"></a></div> -->
		</div>
		<div class="snortModalTitle">
	  		<p><img src="./images/loading.gif" /><br><br>Please Wait...</p>
	  	</div>
		<div>
		<p class="loadingWaitingMessage"></p>
	  	</div>
	</div>  
</div>

<?php include("fbegin.inc"); ?>
<!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2">
<a href="../index.php" id="status-link2">
<img src="./images/transparent.gif" border="0"></img>
</a>
</div>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
				<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
				<li><a href="/snort/snort_interfaces_edit.php?uuid=<?=$uuid;?>"><span>If Settings</span></a></li>
				<li><a href="/snort/snort_rulesets.php?uuid=<?=$uuid;?>"><span>Categories</span></a></li>
				<li><a href="/snort/snort_rules.php?uuid=<?=$uuid;?>"><span>Rules</span></a></li>
				<li><a href="/snort/snort_rulesets_ips.php?uuid=<?=$uuid;?>"><span>Ruleset Ips</span></a></li>
				<li class="newtabmenu_active"><a href="/snort/snort_define_servers.php?uuid=<?=$uuid;?>"><span>Servers</span></a></li>
				<li><a href="/snort/snort_preprocessors.php?uuid=<?=$uuid;?>"><span>Preprocessors</span></a></li>
				<li><a href="/snort/snort_barnyard.php?uuid=<?=$uuid;?>"><span>Barnyard2</span></a></li>		
		</ul>
		</div>

		</td>
	</tr>
	<tr>
		<td id="tdbggrey">		
		<table width="100%" border="0" cellpadding="10px" cellspacing="0">
		<tr>
		<td class="tabnavtbl">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<!-- START MAIN AREA -->
		
		<form id="iform" >
		<input type="hidden" name="snortSaveSettings" value="1" /> <!-- what to do, save -->
		<input type="hidden" name="dbName" value="snortDB" /> <!-- what db-->
		<input type="hidden" name="dbTable" value="SnortIfaces" /> <!-- what db table-->
		<input type="hidden" name="ifaceTab" value="snort_define_servers" /> <!-- what interface tab -->
		<input name="uuid" type="hidden" value="<?=$uuid; ?>"> 

			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%"><span class="vexpl">
					<span class="red"><strong>Note:</strong></span><br>
					Please save your settings before you click start.<br>
					Please make sure there are <strong>no spaces</strong> in your definitions. 
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Define Servers</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define DNS_SERVERS</td>
				<td width="78%" class="vtable">
					<input name="def_dns_servers" type="text" class="formfld" id="def_dns_servers" size="40" value="<?=$a_list['def_dns_servers']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define DNS_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_dns_ports" type="text" class="formfld" id="def_dns_ports" size="40" value="<?=$a_list['def_dns_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 53.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define SMTP_SERVERS</td>
				<td width="78%" class="vtable">
					<input name="def_smtp_servers" type="text" class="formfld" id="def_smtp_servers" size="40" value="<?=$a_list['def_smtp_servers']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define SMTP_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_smtp_ports" type="text" class="formfld" id="def_smtp_ports" size="40" value="<?=$a_list['def_smtp_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 25.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define Mail_Ports</td>
				<td width="78%" class="vtable">
					<input name="def_mail_ports" type="text" class="formfld" id="def_mail_ports" size="40" value="<?=$a_list['def_mail_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 25,143,465,691.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define HTTP_SERVERS</td>
				<td width="78%" class="vtable">
					<input name="def_http_servers" type="text" class="formfld" id="def_http_servers" size="40" value="<?=$a_list['def_http_servers']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define WWW_SERVERS</td>
				<td width="78%" class="vtable">
					<input name="def_www_servers" type="text" class="formfld" id="def_www_servers" size="40" value="<?=$a_list['def_www_servers']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define HTTP_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_http_ports" type="text" class="formfld" id="def_http_ports" size="40" value="<?=$a_list['def_http_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 80.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define SQL_SERVERS</td>
				<td width="78%" class="vtable">
					<input name="def_sql_servers" type="text" class="formfld" id="def_sql_servers" size="40" value="<?=$a_list['def_sql_servers']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define ORACLE_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_oracle_ports" type="text" class="formfld" id="def_oracle_ports" size="40" value="<?=$a_list['def_oracle_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 1521.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define MSSQL_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_mssql_ports" type="text" class="formfld" id="def_mssql_ports" size="40" value="<?=$a_list['def_mssql_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 1433.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define TELNET_SERVERS</td>
				<td width="78%" class="vtable">
					<input name="def_telnet_servers" type="text" class="formfld" id="def_telnet_servers" size="40" value="<?=$a_list['def_telnet_servers']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define TELNET_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_telnet_ports" type="text" class="formfld" id="def_telnet_ports" size="40" value="<?=$a_list['def_telnet_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 23.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define SNMP_SERVERS</td>
				<td width="78%" class="vtable">
					<input name="def_snmp_servers" type="text" class="formfld" id="def_snmp_servers" size="40" value="<?=$a_list['def_snmp_servers']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define SNMP_PORTS</td>
				<td width="78%" class="vtable">
				<input name="def_snmp_ports" type="text" class="formfld" id="def_snmp_ports" size="40" value="<?=$a_list['def_snmp_ports']; ?>"> 
				<br>
				<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 161.</span></td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define FTP_SERVERS</td>
				<td width="78%" class="vtable">
					<input name="def_ftp_servers" type="text" class="formfld" id="def_ftp_servers" size="40" value="<?=$a_list['def_ftp_servers']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define FTP_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_ftp_ports" type="text" class="formfld" id="def_ftp_ports" size="40" value="<?=$a_list['def_ftp_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 21.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define SSH_SERVERS</td>
				<td width="78%" class="vtable">
					<input name="def_ssh_servers" type="text" class="formfld" id="def_ssh_servers" size="40" value="<?=$a_list['def_ssh_servers']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define SSH_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_ssh_ports" type="text" class="formfld" id="def_ssh_ports" size="40" value="<?=$a_list['def_ssh_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is Pfsense SSH port.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define POP_SERVERS</td>
				<td width="78%" class="vtable">
					<input name="def_pop_servers" type="text" class="formfld" id="def_pop_servers" size="40" value="<?=$a_list['def_pop_servers']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define POP2_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_pop2_ports" type="text" class="formfld" id="def_pop2_ports" size="40" value="<?=$a_list['def_pop2_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 109.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define POP3_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_pop3_ports" type="text" class="formfld" id="def_pop3_ports" size="40" value="<?=$a_list['def_pop3_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 110.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define IMAP_SERVERS</td>
				<td width="78%" class="vtable">
					<input name="def_imap_servers" type="text" class="formfld" id="def_imap_servers" size="40" value="<?=$a_list['def_imap_servers']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define IMAP_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_imap_ports" type="text" class="formfld" id="def_imap_ports" size="40" value="<?=$a_list['def_imap_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 143.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define SIP_PROXY_IP</td>
				<td width="78%" class="vtable">
					<input name="def_sip_proxy_ip" type="text" class="formfld" id="def_sip_proxy_ip" size="40" value="<?=$a_list['def_sip_proxy_ip']; ?>"> 
					<br>
					<span class="vexpl">Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define SIP_PROXY_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_sip_proxy_ports" type="text" class="formfld" id="def_sip_proxy_ports" size="40" value="<?=$a_list['def_sip_proxy_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 5060:5090,16384:32768.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define AUTH_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_auth_ports" type="text" class="formfld" id="def_auth_ports" size="40" value="<?=$a_list['def_auth_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 113.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define FINGER_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_finger_ports" type="text" class="formfld" id="def_finger_ports" size="40" value="<?=$a_list['def_finger_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 79.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define IRC_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_irc_ports" type="text" class="formfld" id="def_irc_ports" size="40" value="<?=$a_list['def_irc_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 6665,6666,6667,6668,6669,7000.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define NNTP_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_nntp_ports" type="text" class="formfld" id="def_nntp_ports" size="40" value="<?=$a_list['def_nntp_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 119.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define RLOGIN_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_rlogin_ports" type="text" class="formfld" id="def_rlogin_ports" size="40" value="<?=$a_list['def_rlogin_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 513.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define RSH_PORTS</td>
				<td width="78%" class="vtable">
					<input name="def_rsh_ports" type="text" class="formfld" id="def_rsh_ports" size="40" value="<?=$a_list['def_rsh_ports']; ?>"> 
					<br>
					<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 514.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define SSL_PORTS</td>
				<td width="78%" class="vtable">
				<input name="def_ssl_ports" type="text" class="formfld" id="def_ssl_ports" size="40" value="<?=$a_list['def_ssl_ports']; ?>"> 
				<br>
				<span class="vexpl">Example: Specific ports "25,443" or All ports betwen "5060:5090 . Default is 25,443,465,636,993,995.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%">
					<input name="Submit" type="submit" class="formbtn" value="Save"> 
					<input id="cancel" type="button" class="formbtn" value="Cancel">
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%">
					<span class="vexpl"><span class="red"><strong>Note:</strong></span>
					<br>
					Please save your settings before you click start.</span>
				</td>
			</tr>


			
		
		</form>
		<!-- STOP MAIN AREA -->
		</table>
		</td>
		</tr>			
		</table>
	</td>
	</tr>
</table>
</div>


<!-- footer do not touch below -->
<?php 
include("fend.inc"); 
echo $snort_custom_rnd_box;
?>


</body>
</html>
