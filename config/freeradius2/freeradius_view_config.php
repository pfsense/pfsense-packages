<?php
/*
	freeradius_view_config.php
	part of pfSense (http://www.pfsense.com/)
	Copyright (C) 2011 - 2012 Alexander Wilke <nachtfalkeaw@web.de>
	Copyright (C) 2011 Marcello Coutinho <marcellocoutinho@gmail.com>
	based on postfix_view_config.php
	based on varnish_view_config.
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
function get_file($file){
	$files['radiusd']="/usr/local/etc/raddb/radiusd.conf";
	$files['eap']="/usr/local/etc/raddb/eap.conf";
	$files['sql']="/usr/local/etc/raddb/sql.conf";
	$files['clients']="/usr/local/etc/raddb/clients.conf";
	$files['users']="/usr/local/etc/raddb/users";
	$files['virtual-server-default']="/usr/local/etc/raddb/sites-enabled/default";
	$files['ca']="/usr/local/etc/raddb/certs/ca.cnf";
	$files['server']="/usr/local/etc/raddb/certs/server.cnf";
	$files['client']="/usr/local/etc/raddb/certs/client.cnf";
	$files['index']="/usr/local/etc/raddb/certs/index.txt";
	$files['ldap']="/usr/local/etc/raddb/modules/ldap";


	if ($files[$file]!="" && file_exists($files[$file])){
		print '<textarea rows="50" cols="100%">';
		print $files[$file]."\n".file_get_contents($files[$file]);
		print '</textarea>';
	}
}

if ($_REQUEST['file']!=""){
	get_file($_REQUEST['file']);
	}
else{
	$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
	if(strstr($pfSversion, "1.2"))
		$one_two = true;
	
	$pgtitle = "FreeRADIUS: View Configuration";
	include("head.inc");
	
	?>
	<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
	<?php include("fbegin.inc"); ?>
	
	<?php if($one_two): ?>
	<p class="pgtitle"><?=$pgtitle?></font></p>
	<?php endif; ?>
	
	<?php if ($savemsg) print_info_box($savemsg); ?>
	
	<form action="freeradius_view_config.php" method="post">
		
	<div id="mainlevel">
		<table width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr><td>
	<?php
		$tab_array = array();
		$tab_array[] = array(gettext("Users"), false, "/pkg.php?xml=freeradius.xml");
		$tab_array[] = array(gettext("NAS / Clients"), false, "/pkg.php?xml=freeradiusclients.xml");
		$tab_array[] = array(gettext("Interfaces"), false, "/pkg.php?xml=freeradiusinterfaces.xml");
		$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=freeradiussettings.xml&id=0");
		$tab_array[] = array(gettext("EAP"), false, "/pkg_edit.php?xml=freeradiuseapconf.xml&id=0");
		$tab_array[] = array(gettext("SQL"), false, "/pkg_edit.php?xml=freeradiussqlconf.xml&id=0");
		$tab_array[] = array(gettext("Certificates"), false, "/pkg_edit.php?xml=freeradiuscerts.xml&id=0");
		$tab_array[] = array(gettext("LDAP"), false, "/pkg_edit.php?xml=freeradiusmodulesldap.xml&id=0");
		$tab_array[] = array(gettext("View config"), true, "/freeradius_view_config.php");
		$tab_array[] = array(gettext("XMLRPC Sync"), false, "/pkg_edit.php?xml=freeradiussync.xml&amp;id=0");
		display_top_tabs($tab_array);
	?>
			</td></tr>
	 		<tr>
	    		<td>
					<div id="mainarea">
						<table class="tabcont" width="100%" border="0" cellpadding="8" cellspacing="0">
						<tr><td></td></tr>
						<tr>
						<td colspan="2" valign="top" class="listtopic"><?=gettext("View freeRADIUS configuration files"); ?></td></tr>
						<tr><td></td></tr>
						</tr>
							<tr>
							<td class="tabcont" >
							<input type="button" onClick="get_freeradius_file('radiusd');" id='btn_radiusd' value="radiusd.conf">&nbsp;
							<input type="button" onClick="get_freeradius_file('eap');" id='btn_eap' value="eap.conf">&nbsp;
							<input type="button" onClick="get_freeradius_file('sql');" id='btn_sql' value="sql.conf">&nbsp;
							<input type="button" onClick="get_freeradius_file('clients');" id='btn_clients' value="clients.conf">&nbsp;
							<input type="button" onClick="get_freeradius_file('users');" id='btn_users' value="users">&nbsp;
							<input type="button" onClick="get_freeradius_file('virtual-server-default');" id='btn_virtual-server-default' value="virtual-server-default">&nbsp;
							<input type="button" onClick="get_freeradius_file('ca');" id='btn_ca' value="ca.cnf">&nbsp;
							<input type="button" onClick="get_freeradius_file('server');" id='btn_server' value="server.cnf">&nbsp;
							<input type="button" onClick="get_freeradius_file('client');" id='btn_client' value="client.cnf">&nbsp;
							<input type="button" onClick="get_freeradius_file('index');" id='btn_index' value="index.txt">&nbsp;
							<input type="button" onClick="get_freeradius_file('ldap');" id='btn_ldap' value="ldap">&nbsp;
							</td>
								</tr>
								<tr>
	     						<td class="tabcont" >
	     						<div id="file_div"></div>
									
								</td>
							</tr>
						</table>
					</div>
				</td>
			</tr>
		</table>
	</div>
	</form>
	<script type="text/javascript">
	function get_freeradius_file(file) {
			$('btn_'+file).value="reading...";
			var pars = 'file='+file;
			var url = "/freeradius_view_config.php";
			var myAjax = new Ajax.Request(
				url,
				{
					method: 'post',
					parameters: pars,
					onComplete: activitycallback_postfix_file
				});
			}
		function activitycallback_postfix_file(transport) {
			$('file_div').innerHTML = transport.responseText;
			$('btn_radiusd').value="radiusd.conf";
			$('btn_eap').value="eap.conf";
			$('btn_sql').value="sql.conf";
			$('btn_clients').value="clients.conf";
			$('btn_users').value="users";
			$('btn_virtual').value="virtual-server-default";
			$('btn_ca').value="ca.cnf";
			$('btn_server').value="server.cnf";
			$('btn_client').value="client.cnf";
			$('btn_index').value="index.txt";
			$('btn_ldap').value="ldap";
			scroll(0,0);
		}
	</script>
	<?php 
	include("fend.inc"); 
	}
	?>
	</body>
	</html>
