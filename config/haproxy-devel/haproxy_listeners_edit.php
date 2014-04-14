<?php
/* $Id: load_balancer_pool_edit.php,v 1.24.2.23 2007/03/03 00:07:09 smos Exp $ */
/*
	haproxy_listeners_edit.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2009 Scott Ullrich <sullrich@pfsense.com>
	Copyright (C) 2008 Remco Hoef <remcoverhoef@pfsense.com>
	Copyright (C) 2013 PiBa-NL merging (some of the) "haproxy-devel" changes from: Marcello Coutinho <marcellocoutinho@gmail.com>
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
	AUTHOR BE LIABLE FOR ANY DIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
$shortcut_section = "haproxy";
require("guiconfig.inc");
require_once("haproxy.inc");
require_once("haproxy_utils.inc");
require_once("haproxy_htmllist.inc");
require_once("pkg_haproxy_tabs.inc");

/* Compatibility function for pfSense 2.0 */
if (!function_exists("cert_get_purpose")) {	
	function cert_get_purpose(){
		$result = array();
		$result['server'] = "Yes";
		return $result;
	}
}
/**/

function haproxy_js_acl_select($mode) {
	global $a_acltypes;

	$seltext = '';
	foreach ($a_acltypes as $key => $expr) {
		if ($expr['mode'] == '' || $expr['mode'] == $mode)
			$seltext .= "<option value='" . $key . "'>" . $expr['name'] .":<\/option>";
	}
	return $seltext;
}

if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
	$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
}

$a_backend = &$config['installedpackages']['haproxy']['ha_backends']['item'];
$a_pools = &$config['installedpackages']['haproxy']['ha_pools']['item'];

global $simplefields;
$simplefields = array('name','desc','status','secondary','primary_frontend','type','forwardfor','httpclose','extaddr','backend_serverpool',
	'max_connections','client_timeout','port','ssloffloadcert','dcertadv','ssloffload','ssloffloadacl','advanced_bind','ssloffloadacladditional');

if (isset($_POST['id']))
	$id = $_POST['id'];
else
	$id = $_GET['id'];

if (isset($_GET['dup']))
	$id = $_GET['dup'];

$id = get_frontend_id($id);

if (!is_numeric($id))
{
	//default value for new items.
	$pconfig['ssloffloadacl'] = "yes";
}

$servercerts = get_certificates_server();

$fields_sslCertificates=array();
$fields_sslCertificates[0]['name']="ssl_certificate";
$fields_sslCertificates[0]['columnheader']="Certificates";
$fields_sslCertificates[0]['colwidth']="95%";
$fields_sslCertificates[0]['type']="select";
$fields_sslCertificates[0]['size']="500px";
$fields_sslCertificates[0]['items']=&$servercerts;

$fields_aclSelectionList=array();
$fields_aclSelectionList[0]['name']="name";
$fields_aclSelectionList[0]['columnheader']="Name";
$fields_aclSelectionList[0]['colwidth']="30%";
$fields_aclSelectionList[0]['type']="textbox";
$fields_aclSelectionList[0]['size']="20";

$fields_aclSelectionList[1]['name']="expression";
$fields_aclSelectionList[1]['columnheader']="Expression";
$fields_aclSelectionList[1]['colwidth']="30%";
$fields_aclSelectionList[1]['type']="select";
$fields_aclSelectionList[1]['size']="10";
$fields_aclSelectionList[1]['items']=&$a_acltypes;

$fields_aclSelectionList[2]['name']="value";
$fields_aclSelectionList[2]['columnheader']="Value";
$fields_aclSelectionList[2]['colwidth']="35%";
$fields_aclSelectionList[2]['type']="textbox";
$fields_aclSelectionList[2]['size']="35";


if (isset($id) && $a_backend[$id]) {
	$pconfig['a_acl']=&$a_backend[$id]['ha_acls']['item'];	
	$pconfig['a_certificates']=&$a_backend[$id]['ha_certificates']['item'];
	$pconfig['advanced'] = base64_decode($a_backend[$id]['advanced']);
	foreach($simplefields as $stat)
		$pconfig[$stat] = $a_backend[$id][$stat];
}

if (isset($_GET['dup']))
	unset($id);

$changedesc = "Services: HAProxy: Frontend";
$changecount = 0;

if ($_POST) {
	$changecount++;

	unset($input_errors);
	$pconfig = $_POST;
	
	
	if ($pconfig['secondary'] != "yes") {
		$reqdfields = explode(" ", "name type port");
		$reqdfieldsn = explode(",", "Name,Type,Port");
	} else {
		$reqdfields = explode(" ", "name");
		$reqdfieldsn = explode(",", "Name");
	}
	
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['name']))
		$input_errors[] = "The field 'Name' contains invalid characters.";

	if ($pconfig['secondary'] != "yes") {
		if ($_POST['max_connections'] && !is_numeric($_POST['max_connections']))
			$input_errors[] = "The field 'Max connections' value is not a number.";

		$ports = split(",", $_POST['port'] . ",");
		foreach($ports as $port)
			if ($port && !is_numeric($port) && !is_portoralias($port))
				$input_errors[] = "The field 'Port' value '".htmlspecialchars($port)."' is not a number or alias thereof.";

		if ($_POST['client_timeout'] !== "" && !is_numeric($_POST['client_timeout']))
			$input_errors[] = "The field 'Client timeout' value is not a number.";
	}

	/* Ensure that our pool names are unique */
	for ($i=0; isset($config['installedpackages']['haproxy']['ha_backends']['item'][$i]); $i++)
		if (($_POST['name'] == $config['installedpackages']['haproxy']['ha_backends']['item'][$i]['name']) && ($i != $id))
			$input_errors[] = "This frontend name has already been used. Frontend names must be unique. $i != $id";

	$a_certificates = haproxy_htmllist_get_values($fields_sslCertificates);
	$pconfig['a_certificates'] = $a_certificates;
	
	$a_acl = haproxy_htmllist_get_values($fields_aclSelectionList);
	$pconfig['a_acl'] = $a_acl;
	
	foreach($a_acl as $acl) {
		$acl_name = $acl['name'];
		$acl_value = $acl['value'];
		
		if (preg_match("/[^a-zA-Z0-9\.\-_]/", $acl_name))
			$input_errors[] = "The field 'Name' contains invalid characters.";

		if (!preg_match("/.{1,}/", $acl_value))
			$input_errors[] = "The field 'Value' is required.";

		if (!preg_match("/.{2,}/", $acl_name))
			$input_errors[] = "The field 'Name' is required with at least 2 characters.";
	}

	if (!$input_errors) {
		$backend = array();
		if(isset($id) && $a_backend[$id])
			$backend = $a_backend[$id];
			
		if($backend['name'] != "")
			$changedesc .= " modified '{$backend['name']}' pool:";
			
		// update references to this primary frontend
		if ($backend['name'] != $_POST['name']) {
			foreach($a_backend as &$frontend) {
				if ($frontend['primary_frontend'] == $backend['name']) {
					$frontend['primary_frontend'] = $_POST['name'];
				}
			}
		}
		
		foreach($simplefields as $stat)
			update_if_changed($stat, $backend[$stat], $_POST[$stat]);
		
		update_if_changed("advanced", $backend['advanced'], base64_encode($_POST['advanced']));
		$backend['ha_acls']['item'] = $a_acl;
		$backend['ha_certificates']['item'] = $a_certificates;

		if (isset($id) && $a_backend[$id]) {
			$a_backend[$id] = $backend;
		} else {
			$a_backend[] = $backend;
		}

		if ($changecount > 0) {
			touch($d_haproxyconfdirty_path);
			write_config($changedesc);
		}

		header("Location: haproxy_listeners.php");
		exit;
	}
}

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

$closehead = false;
$pgtitle = "HAProxy: Frontend: Edit";
include("head.inc");

if (!isset($_GET['dup']))
	$excludefrontend = $pconfig['name'];
$primaryfrontends = get_haproxy_frontends($excludefrontend);
$interfaces = haproxy_get_bindable_interfaces();

?>
  <style type="text/css">
	.haproxy_mode_http{display:none;}
	.haproxy_ssloffloading_enabled{display:none;}
	.haproxy_primary{}
	.haproxy_secondary{display:none;}
  </style>
  <script type="text/javascript" src="/javascript/suggestions.js"></script>
  <script type="text/javascript" src="/javascript/autosuggest.js"></script>
</head>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<?php if($one_two): ?>
<script type="text/javascript" src="/javascript/scriptaculous/prototype.js"></script>
<script type="text/javascript" src="/javascript/scriptaculous/scriptaculous.js"></script>
<?php endif; ?>

<script type="text/javascript">
	function htmllist_get_select_options(tableId) {
		var seltext;
		seltext = "";
		var type;
		var secondary = d.getElementById("secondary");
		var primary_frontend = d.getElementById("primary_frontend");		
		if ((secondary !== null) && (secondary.checked))
			type = primaryfrontends[primary_frontend.value]['ref']['type'];
		else
			type = d.getElementById("type").value;
		
		if (tableId == 'tableA_acltable'){	
			if (type == 'health')
				seltext = "<?php echo haproxy_js_acl_select('health');?>";
			else if (type == 'tcp')
				seltext = "<?php echo haproxy_js_acl_select('tcp');?>";
			else if (type == 'https')
				seltext = "<?php echo haproxy_js_acl_select('https');?>";
			else
				seltext = "<?php echo haproxy_js_acl_select('http');?>";
			if (seltext == '') {
				alert("No ACL types available in current frontend type");
				return;
			}
		}
		if (tableId == 'tableA_sslCertificates'){
			seltext = "<?=haproxy_js_select_options($servercerts);?>";
		}
		return seltext;
	}

	function setCSSdisplay(cssID, display) {
		var ss = document.styleSheets;
		for (var i=0; i<ss.length; i++) {
			var rules = ss[i].cssRules || ss[i].rules;
			for (var j=0; j<rules.length; j++) {
				if (rules[j].selectorText === cssID) {
					rules[j].style.display = display ? "" : "none";
				}
			}
		}
	}
	
	function updatevisibility()	{
		d = document;
		ssloffload = d.getElementById("ssloffload");
		
		var type;
		var secondary = d.getElementById("secondary");
		var primary_frontend = d.getElementById("primary_frontend");		
		if ((secondary !== null) && (secondary.checked))
			type = primaryfrontends[primary_frontend.value]['ref']['type'];
		else
			type = d.getElementById("type").value;
			
		setCSSdisplay(".haproxy_ssloffloading_enabled", ssloffload.checked);
		setCSSdisplay(".haproxy_mode_http", type == "http");
		if (secondary !== null) {
			setCSSdisplay(".haproxy_primary", !secondary.checked);
			setCSSdisplay(".haproxy_secondary", secondary.checked);
		}
		
		type_change(type);
		
		http_close = d.getElementById("httpclose").value;
		http_close_description = d.getElementById("http_close_description");
		http_close_description.innerHTML=closetypes[http_close]["descr"];
		http_close_description.setAttribute('style','padding:5px; border:1px dashed #990000; background-color: #ffffff; color: #000000; font-size: 8pt; height:30px');
		http_close_description.setAttribute('style','padding:5px; border:1px dashed #990000; background-color: #ffffff; color: #000000; font-size: 8pt; height:'+http_close_description.scrollHeight+'px');
	}
	
	function type_change(type) {
		var d, i, j, el, row;
		var count = <?=count($a_acltypes);?>;
		var acl = [ <?php foreach ($a_acltypes as $key => $expr) echo "'".$key."'," ?> ];
		var mode = [ <?php foreach ($a_acltypes as $key => $expr) echo "'".$expr['mode']."'," ?> ];

        d = document;
		for (i = 0; i < 99; i++) {
			el = d.getElementById("acl_expression" + i);
			row = d.getElementById("aclrow" + i);
			if (!el)
				continue;
			for (j = 0; j < count; j++) {
				if (acl[j] == el.value) {
					if (mode[j] != '' && mode[j] != type) {
						Effect.Fade(row,{ duration: 1.0 });
					} else {
						Effect.Appear(row,{ duration: 1.0 });
					}
				}
			}
		}
		
		for (i = 0; i < 99; i++) {
			el = d.getElementById("expression" + i);
			//row_v = d.getElementById("tr_view_" + i);
			row_e = d.getElementById("tr_edit_" + i);
			if (!el)
				continue;
			for (j = 0; j < count; j++) {
				if (acl[j] == el.value) {
					if (mode[j] != '' && mode[j] != type) {
						//Effect.Fade(row_v,{ duration: 1.0 });
						Effect.Fade(row_e,{ duration: 1.0 });
					} else {
						//Effect.Appear(row_v,{ duration: 1.0 });
						Effect.Appear(row_e,{ duration: 1.0 });
					}
				}
			}
		}
		
	}
</script>
<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php endif; ?>
<form action="haproxy_listeners_edit.php" method="post" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
  <?php
	haproxy_display_top_tabs_active($haproxy_tab_array['haproxy'], "frontend");
  ?>
  </td></tr>
  <tr>
    <td>
	<div class="tabcont">
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Edit haproxy listener</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncellreq">Name</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="name" type="text" <?if(isset($pconfig['name'])) echo "value=\"{$pconfig['name']}\"";?> size="25" maxlength="25" />
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Description</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="desc" type="text" <?if(isset($pconfig['desc'])) echo "value=\"{$pconfig['desc']}\"";?> size="64" />
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Status</td>
			<td width="78%" class="vtable" colspan="2">
				<select name="status" id="status">
					<option value="active"<?php if($pconfig['status'] == "active") echo " SELECTED"; ?>>Active</option>
					<option value="disabled"<?php if($pconfig['status'] == "disabled") echo " SELECTED"; ?>>Disabled</option>
				</select>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Shared Frontend</td>
			<td width="78%" class="vtable" colspan="2">
				<?if (count($primaryfrontends)==0){ ?>
				<b>At least 1 primary frontend is needed.</b><br/><br/>
				<? } else{ ?>
				<input id="secondary" name="secondary" type="checkbox" value="yes" <?php if ($pconfig['secondary']=='yes') echo "checked"; ?> onclick="updatevisibility();" />
				<? } ?>
				This can be used to host a second or more website on the same IP:Port combination.<br/>
				Use this setting to configure multiple backends/accesslists for a single frontend.<br/>
				All settings of which only 1 can exist will be hidden.<br/>
				The frontend settings will be merged into 1 set of frontend configuration.
			</td>
		</tr>
		<tr class="haproxy_secondary" align="left">
			<td width="22%" valign="top" class="vncellreq">Primary frontend</td>
			<td width="78%" class="vtable" colspan="2">
				<?
				echo_html_select('primary_frontend',$primaryfrontends, $pconfig['primary_frontend'],"You must first create a 'primary' frontend.","updatevisibility();");
				?>
			</td>
		</tr>
		<tr class="haproxy_primary">
			  <td width="22%" valign="top" class="vncellreq">External address</td>
			  <td width="78%" class="vtable">
				<?
				echo_html_select('extaddr', $interfaces, $pconfig['extaddr']);
				?>
				<br />
				<span class="vexpl">
					If you want this rule to apply to another IP address than the IP address of the interface chosen above,
					select it here (you need to define <a href="firewall_virtual_ip.php">Virtual IP</a> addresses on the first).  
					Also note that if you are trying to redirect connections on the LAN select the "any" option.
				</span>
			  </td>
		</tr>
		<tr class="haproxy_primary" align="left">
			<td width="22%" valign="top" class="vncellreq">External port</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="port" id="port" type="text" <?if(isset($pconfig['port'])) echo "value=\"{$pconfig['port']}\"";?> size="10" maxlength="500" />
				<div>The port to listen to. To specify multiple ports, separate with a comma (,). EXAMPLE: 80,8000</div>
			</td>
		</tr>
		<tr class="haproxy_primary" align="left">
			<td width="22%" valign="top" class="vncell">Max connections</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="max_connections" type="text" <?if(isset($pconfig['max_connections'])) echo "value=\"{$pconfig['max_connections']}\"";?> size="10" maxlength="10" />
			</td>
		</tr>	
		<tr>
			  <td width="22%" valign="top" class="vncellreq">Backend server pool</td>
			  <td width="78%" class="vtable">
			  
				<select id="backend_serverpool" name="backend_serverpool" class="formfld">
				<?php
					if (is_array($a_pools)) {
						foreach ($a_pools as $p) {
							$selected = $p['name'] == $pconfig['backend_serverpool'] ? 'selected' : '';
							$name = htmlspecialchars("{$p['name']}");
							echo "<option value=\"{$p['name']}\" $selected>$name</option>";
						}
					} else { 	
						echo "<option value=\"-\">-</option>";
					}
				?>
				</select>
		<tr class="haproxy_primary" align="left">
			<td width="22%" valign="top" class="vncellreq">Type</td>
			<td width="78%" class="vtable" colspan="2">
				<select name="type" id="type" onchange="updatevisibility();">
					<option value="http"<?php if($pconfig['type'] == "http") echo " SELECTED"; ?>>HTTP</option>
					<option value="https"<?php if($pconfig['type'] == "https") echo " SELECTED"; ?>>HTTPS</option>
					<option value="tcp"<?php if($pconfig['type'] == "tcp") echo " SELECTED"; ?>>TCP</option>
					<option value="health"<?php if($pconfig['type'] == "health") echo " SELECTED"; ?>>Health</option>
				</select><br/>
				<span class="vexpl">
					This defines the processing type of HAProxy, and will determine the availabe options for acl checks and also several other options.<br/>
					Please note that for https encryption/decryption on HAProxy with a certificate the processing type needs to be set to 'http'.
				</span>
			</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncell">Access Control lists</td>
			<td width="78%" class="vtable" colspan="2" valign="top">
			<?
			$counter=0;
			$a_acl = $pconfig['a_acl'];
			haproxy_htmllist("tableA_acltable", $a_acl, $fields_aclSelectionList, true);
			?>
			<br/>
			acl's with the same name wil be 'combined', acl's with different names will be evaluated seperately.<br/>
			For more information about ACL's please see <a href='http://haproxy.1wt.eu/download/1.5/doc/configuration.txt' target='_blank'>HAProxy Documentation</a> Section 7 - Using ACL's
			</td>
		</tr>
	</table>
	<br/>&nbsp;<br/>
	<table class="haproxy_primary" width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Advanced settings</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Client timeout</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="client_timeout" type="text" <?if(isset($pconfig['client_timeout'])) echo "value=\"{$pconfig['client_timeout']}\"";?> size="10" maxlength="10" />
				<div>the time (in milliseconds) we accept to wait for data from the client, or for the client to accept data (default 30000).</div>
			</td>
		</tr>
		<tr align="left" class="haproxy_mode_http">
			<td width="22%" valign="top" class="vncell">Use 'forwardfor' option</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="forwardfor" name="forwardfor" type="checkbox" value="yes" <?php if ($pconfig['forwardfor']=='yes') echo "checked"; ?> />
				<br/>
				The 'forwardfor' option creates an HTTP 'X-Forwarded-For' header which
				contains the client's IP address. This is useful to let the final web server
				know what the client address was. (eg for statistics on domains)<br/>
				<br/>
				It is important to note that as long as HAProxy does not support keep-alive connections, 
				only the first request of a connection will receive the header. For this reason, 
				it is important to ensure that option httpclose is set when using this option.
			</td>
		</tr>
		<tr align="left" class="haproxy_mode_http">
			<td width="22%" valign="top" class="vncell">Use 'httpclose' option</td>
			<td width="78%" class="vtable" colspan="2">
				<?
					echo_html_select("httpclose",$a_closetypes,$pconfig['httpclose']?$pconfig['httpclose']:"none","","updatevisibility();");
				?><br/>
				<textarea readonly="yes" cols="70" rows="3" id="http_close_description" name="http_close_description" style="padding:5px; border:1px dashed #990000; background-color: #ffffff; color: #000000; font-size: 8pt;"></textarea>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Bind pass thru</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="advanced_bind" type="text" <?if(isset($pconfig['advanced_bind'])) echo "value=\"".htmlspecialchars($pconfig['advanced_bind'])."\"";?> size="64" />
				<br/>
				NOTE: paste text into this box that you would like to pass behind the bind option.
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Advanced pass thru</td>
			<td width="78%" class="vtable" colspan="2">
				<? $textrowcount = max(substr_count($pconfig['advanced'],"\n"), 2) + 2; ?>
				<textarea name='advanced' rows="<?=$textrowcount;?>" cols="70" id='advanced'><?php echo htmlspecialchars($pconfig['advanced']); ?></textarea>
				<br/>
				NOTE: paste text into this box that you would like to pass thru.
			</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
		</tr>
	</table>
	<table class="haproxy_mode_http" width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">SSL Offloading</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Use Offloading</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="ssloffload" name="ssloffload" type="checkbox" value="yes" <?php if ($pconfig['ssloffload']=='yes') echo "checked";?> onclick="updatevisibility();" /><strong>Use Offloading</strong>
				<br/>
				SSL Offloading will reduce web servers load by maintaining and encrypting connection with users on internet while sending and retrieving data without encrytion to internal servers.
				Also more ACL rules and http logging may be configured when this option is used. 
				Certificates can be imported into the <a href="/system_camanager.php" target="_blank">pfSense "Certificate Authority Manager"</a>
				Please be aware this possibly will not work with all web applications. Some applications will require setting the SSL checkbox on the backend server configurations so the connection to the webserver will also be a encrypted connection, in that case there will be a slight overall performance loss.
			</td>
		</tr>
		<tr class="haproxy_ssloffloading_enabled" align="left">
			<td width="22%" valign="top" class="vncell">Certificate</td>
			<td width="78%" class="vtable" colspan="2">
				<?  
					echo_html_select("ssloffloadcert", $servercerts, $pconfig['ssloffloadcert'], '<b>No Certificates defined.</b> <br/>Create one under <a href="system_certmanager.php">System &gt; Cert Manager</a>.');
				?>
				<br/>
				NOTE: choose the cert to use on this frontend.
				<br/>
				<input id="ssloffloadacl" name="ssloffloadacl" type="checkbox" value="yes" <?php if ($pconfig['ssloffloadacl']=='yes') echo "checked";?> onclick="updatevisibility();" />Add ACL for certificate CommonName.
			</td>
		</tr>
		<tr class="haproxy_ssloffloading_enabled">
			<td width="22%" valign="top" class="vncell">Additional certificates</td>
			<td width="78%" class="vtable" colspan="2" valign="top">
			Which of these certificate will be send will be determined by haproxys SNI recognition. If the browser does not send SNI this will not work properly. (IE on XP is one example, possibly also older browsers or mobile devices)
			<?
			$a_certificates = $pconfig['a_certificates'];
			haproxy_htmllist("tableA_sslCertificates", $a_certificates, $fields_sslCertificates);
			?>
				<br/>
				<input id="ssloffloadacladditional" name="ssloffloadacladditional" type="checkbox" value="yes" <?php if ($pconfig['ssloffloadacladditional']=='yes') echo "checked";?> onclick="updatevisibility();" />Add ACL for certificate CommonName.
			</td>
		</tr>
		<tr class="haproxy_ssloffloading_enabled haproxy_primary" align="left">
			<td width="22%" valign="top" class="vncell">Advanced ssl options</td>
			<td width="78%" class="vtable" colspan="2">
				<input type='text' name='dcertadv' size="64" id='dcertadv' <?if(isset($pconfig['dcertadv'])) echo 'value="'.htmlspecialchars($pconfig['dcertadv']).'"';?> />
				<br/>
				NOTE: Paste additional ssl options(without commas) to include on ssl listening options.<br/>
				some options: force-sslv3, force-tlsv10 force-tlsv11 force-tlsv12 no-sslv3 no-tlsv10 no-tlsv11 no-tlsv12 no-tls-tickets
			</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
		</tr>
	</table>
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr align="left">
			<td width="22%" valign="top">&nbsp;</td>
			<td width="78%">
				<input name="Submit" type="submit" class="formbtn" value="Save" />  
				<input type="button" class="formbtn" value="Cancel" onclick="history.back()" />
				<?php if (isset($id) && $a_backend[$id]): ?>
				<input name="id" type="hidden" value="<?=$a_backend[$id]['name'];?>" />
				<?php endif; ?>
			</td>
		</tr>
		<tr>
			<td colspan='3'>
					<span class="vexpl"><b>NOTE:</b> You must add a firewall rule permitting access to this frontend!</span>
			</td>
		</tr>
	</table>
	</div></td></tr></table>
	</form>
<br/>
<script type="text/javascript">
<?
	phparray_to_javascriptarray($primaryfrontends,"primaryfrontends",Array('/*','/*/name','/*/ref','/*/ref/type','/*/ref/ssloffload'));
	phparray_to_javascriptarray($a_closetypes,"closetypes",Array('/*','/*/name','/*/descr'));
	phparray_to_javascriptarray($fields_sslCertificates,"fields_sslCertificates",Array('/*','/*/name','/*/type','/*/size','/*/items','/*/items/*','/*/items/*/*','/*/items/*/*/name'));
	phparray_to_javascriptarray($fields_aclSelectionList,"fields_acltable",Array('/*','/*/name','/*/type','/*/size','/*/items','/*/items/*','/*/items/*/*','/*/items/*/*/name'));
?>
</script>
<script type="text/javascript">
	totalrows =  <?php echo $counter; ?>;
	updatevisibility();
	
	var customarray  = <?= json_encode(get_alias_list(array("port", "url_ports", "urltable_ports"))) ?>;
	var oTextbox1 = new AutoSuggestControl(document.getElementById("port"), new StateSuggestions(customarray));
</script>
<?php 
haproxy_htmllist_js();
include("fend.inc"); ?>
</body>
</html>
