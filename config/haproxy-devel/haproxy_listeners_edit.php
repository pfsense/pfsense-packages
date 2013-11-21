<?php
/* $Id: load_balancer_pool_edit.php,v 1.24.2.23 2007/03/03 00:07:09 smos Exp $ */
/*
	haproxy_listeners_edit.php
	part of pfSense (http://www.pfsense.com/)
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

require("guiconfig.inc");
require_once("haproxy.inc");
require_once("haproxy_utils.inc");

/* Compatibility function for pfSense 2.0 */
if (!function_exists("cert_get_purpose")) {	
	function cert_get_purpose(){
		$result = array();
		$result['server'] = "Yes";
		return $result;
	}
}
/**/

function get_certificat_usage($refid) {
	$usage = array();
	$cert = lookup_cert($refid);
	if (is_cert_revoked($cert))
		$usage[] = "Revoked";
	if (is_webgui_cert($refid))
		$usage[] = "webConfigurator";
	if (is_user_cert($refid))
		$usage[] = "User Cert";
	if (is_openvpn_server_cert($refid))
		$usage[] = "OpenVPN Server";
	if (is_openvpn_client_cert($refid))
		$usage[] = "OpenVPN Client";
	if (is_ipsec_cert($cert['refid']))
		$usage[] = "IPsec Tunnel";
	if (function_exists("is_captiveportal_cert"))
		if (is_captiveportal_cert($refid))
			$usage[] = "Captive Portal";
	
	return $usage;
}

// This function (is intended to) provides a uniform way to retrieve a list of server certificates
function get_certificates_server($get_includeWebCert=false) {
	global $config;
	$certificates=array();
	$a_cert = &$config['cert'];
	foreach ($a_cert as $cert)
	{
		if ($get_ca == false && is_webgui_cert($cert['refid']))
			continue;

		$purpose = cert_get_purpose($cert['crt']);
		//$certserverpurpose = $purpose['server'] == 'Yes' ? " [Server certificate]" : "";
		$certserverpurpose = "";

		$selected = "";
		$caname = "";
		$inuse = "";
		$revoked = "";
		$ca = lookup_ca($cert['caref']);
		if ($ca)
			$caname = " (CA: {$ca['descr']})";
		if ($pconfig['certref'] == $cert['refid'])
			$selected = "selected";
		if (cert_in_use($cert['refid']))
			$inuse = " *In Use";
		if (is_cert_revoked($cert))
		$revoked = " *Revoked";
		
		$usagestr="";
		$usage = get_certificat_usage($cert['refid']);
		foreach($usage as $use){
			$usagestr .= " " . $use;
		}		
		if ($usagestr != "")
			$usagestr = " (".trim($usagestr).")";
		
		$certificates[$cert['refid']]['name'] = $cert['descr'] . $caname . $certserverpurpose . $inuse . $revoked . $usagestr;
	}
	return $certificates;
}

function haproxy_acl_select($mode) {
	global $a_acltypes;

	$seltext = '';
	foreach ($a_acltypes as $expr) {
		if ($expr['mode'] == '' || $expr['mode'] == $mode)
			$seltext .= "<option value='" . $expr['name'] . "'>" . $expr['descr'] .":</option>";
	}
	return $seltext;
}

$d_haproxyconfdirty_path = $g['varrun_path'] . "/haproxy.conf.dirty";

if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
	$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
}

$a_backend = &$config['installedpackages']['haproxy']['ha_backends']['item'];
$a_pools = &$config['installedpackages']['haproxy']['ha_pools']['item'];

global $simplefields;
$simplefields = array('name','desc','status','secondary','primary_frontend','type','forwardfor','httpclose','extaddr','backend_serverpool',
	'max_connections','client_timeout','port','ssloffloadcert','dcertadv','ssloffload','ssloffloadacl','advanced_bind');

if (isset($_POST['id']))
	$id = $_POST['id'];
else
	$id = $_GET['id'];

if (isset($_GET['dup']))
	$id = $_GET['dup'];

$id = get_frontend_id($id);

if (isset($id) && $a_backend[$id]) {
	$pconfig['a_acl']=&$a_backend[$id]['ha_acls']['item'];	
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
		$reqdfields = explode(" ", "name type port max_connections");
		$reqdfieldsn = explode(",", "Name,Type,Port,Max connections");
	} else {
		$reqdfields = explode(" ", "name");
		$reqdfieldsn = explode(",", "Name");
	}
	
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['name']))
		$input_errors[] = "The field 'Name' contains invalid characters.";

	if ($pconfig['secondary'] != "yes") {
		if (!is_numeric($_POST['max_connections']))
			$input_errors[] = "The field 'Max connections' value is not a number.";

		$ports = split(",", $_POST['port'] . ",");
		foreach($ports as $port)
			if ($port && !is_numeric($port))
				$input_errors[] = "The field 'Port' value is not a number.";

		if ($_POST['client_timeout'] !== "" && !is_numeric($_POST['client_timeout']))
			$input_errors[] = "The field 'Client timeout' value is not a number.";
	}

	/* Ensure that our pool names are unique */
	for ($i=0; isset($config['installedpackages']['haproxy']['ha_backends']['item'][$i]); $i++)
		if (($_POST['name'] == $config['installedpackages']['haproxy']['ha_backends']['item'][$i]['name']) && ($i != $id))
			$input_errors[] = "This frontend name has already been used. Frontend names must be unique. $i != $id";

	$a_acl=array();			
	$acl_names=array(); 
	for($x=0; $x<99; $x++) {
		$acl_name=$_POST['acl_name'.$x];
		$acl_expression=$_POST['acl_expression'.$x];
		$acl_value=$_POST['acl_value'.$x];

		if ($acl_name) {
			$acl_names[]=$acl_name;

			$acl=array();
			$acl['name']=$acl_name;
			$acl['expression']=$acl_expression;
			$acl['value']=$acl_value;
			$a_acl[]=$acl;

			if (preg_match("/[^a-zA-Z0-9\.\-_]/", $acl_name))
				$input_errors[] = "The field 'Name' contains invalid characters.";

			if (!preg_match("/.{1,}/", $acl_value))
				$input_errors[] = "The field 'Value' is required.";

			if (!preg_match("/.{2,}/", $acl_name))
				$input_errors[] = "The field 'Name' is required.";

			}
	}

	$pconfig['a_acl']=$a_acl;

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

$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
if(strstr($pfSversion, "1.2"))
	$one_two = true;

if (!$id)
{
	//default value for new items.
	$pconfig['ssloffloadacl'] = "yes";
}

$pgtitle = "HAProxy: Frontend: Edit";
include("head.inc");

$primaryfrontends = get_haproxy_frontends($pconfig['name']);
$interfaces = haproxy_get_bindable_interfaces();
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
  <style type="text/css">
	.haproxy_mode_http{display:none;}
	.haproxy_ssloffloading_enabled{display:none;}
	.haproxy_primary{}
	.haproxy_secondary{display:none;}
  </style>

<?php if($one_two): ?>
<script type="text/javascript" src="/javascript/scriptaculous/prototype.js"></script>
<script type="text/javascript" src="/javascript/scriptaculous/scriptaculous.js"></script>
<?php endif; ?>
<script type="text/javascript">
	// Global Variables
	var rowname = new Array(99);
	var rowtype = new Array(99);
	var newrow  = new Array(99);
	var rowsize = new Array(99);

	for (i = 0; i < 99; i++) {
	        rowname[i] = '';
	        rowtype[i] = '';
	        newrow[i] = '';
	        rowsize[i] = '25';
	}

	var field_counter_js = 0;
	var loaded = 0;
	var is_streaming_progress_bar = 0;
	var temp_streaming_text = "";

	var addRowTo = (function() {
	    return (function (tableId) {
	        var d, tbody, tr, td, bgc, i, ii, j, type, seltext;
		var btable, btbody, btr, btd;

	        d = document;
		type = d.getElementById("type").value;
		if (type == 'health')
			seltext = "<?php echo haproxy_acl_select('health');?>";
		else if (type == 'tcp')
			seltext = "<?php echo haproxy_acl_select('tcp');?>";
		else if (type == 'https')
			seltext = "<?php echo haproxy_acl_select('https');?>";
		else
			seltext = "<?php echo haproxy_acl_select('http');?>";
		if (seltext == '') {
			alert("No ACL types available in current listener mode");
			return;
		}

	        tbody = d.getElementById(tableId).getElementsByTagName("tbody").item(0);
	        tr = d.createElement("tr");
	        totalrows++;
		tr.setAttribute("id","aclrow" + totalrows);
	        for (i = 0; i < field_counter_js; i++) {
	                td = d.createElement("td");
	                if(rowtype[i] == 'textbox') {
				td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows +
					"'></input><input size='" + rowsize[i] + "' name='" + rowname[i] + totalrows +
					"' id='" + rowname[i] + totalrows +
				       	"'></input> ";
	                } else if(rowtype[i] == 'select') {
				td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows +
					"'></input><select name='" + rowname[i] + totalrows + 
					"' id='" + rowname[i] + totalrows +
					"'>" + seltext + "</select> ";
	                } else {
				td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows +
					"'></input><input type='checkbox' name='" + rowname[i] + totalrows +
				        "' id='" + rowname[i] + totalrows + "'></input> ";
	                }
	                tr.appendChild(td);
	        }
		td = d.createElement("td");
		td.rowSpan = "1";
		td.setAttribute("class","list");

		// Recreate the button table.
		btable = document.createElement("table");
		btable.setAttribute("border", "0");
		btable.setAttribute("cellspacing", "0");
		btable.setAttribute("cellpadding", "1");
		btbody = document.createElement("tbody");
		btr = document.createElement("tr");
		btd = document.createElement("td");
		btd.setAttribute("valign", "middle");
		btd.innerHTML = '<img src="/themes/' + theme + '/images/icons/icon_x.gif" title="delete entry" width="17" height="17" border="0" onclick="removeRow(this); return false;">';
		btr.appendChild(btd);
		btd = document.createElement("td");
		btd.setAttribute("valign", "middle");
		btd.innerHTML = '<img src="/themes/' + theme + "/images/icons/icon_plus.gif\" title=\"duplicate entry\" width=\"17\" height=\"17\" border=\"0\" onclick=\"dupRow(" + totalrows + ", 'acltable'); return false;\">";
		btr.appendChild(btd);
		btbody.appendChild(btr);
		btable.appendChild(btbody);

		td.appendChild(btable);
	        tr.appendChild(td);
	        tbody.appendChild(tr);
	    });
	})();

	function dupRow(rowId, tableId) {
		var dupEl;
		var newEl;

		addRowTo(tableId);
		for (i = 0; i < field_counter_js; i++) {
			dupEl = document.getElementById(rowname[i] + rowId);
			newEl = document.getElementById(rowname[i] + totalrows);
			if (dupEl && newEl)
				newEl.value = dupEl.value;
		}
	}

	function removeRow(el) {
	    var cel;
	    // Break out of one table first
	    while (el && el.nodeName.toLowerCase() != "table")
		    el = el.parentNode;
	    while (el && el.nodeName.toLowerCase() != "tr")
	            el = el.parentNode;

	    if (el && el.parentNode) {
	        cel = el.getElementsByTagName("td").item(0);
	        el.parentNode.removeChild(el);
	    }
	}

	function find_unique_field_name(field_name) {
	        // loop through field_name and strip off -NUMBER
	        var last_found_dash = 0;
	        for (var i = 0; i < field_name.length; i++) {
	                // is this a dash, if so, update
	                //    last_found_dash
	                if (field_name.substr(i,1) == "-" )
	                        last_found_dash = i;
	        }
	        if (last_found_dash < 1)
	                return field_name;
	        return(field_name.substr(0,last_found_dash));
	}

	rowname[0] = "acl_name";
	rowtype[0] = "textbox";
	rowsize[0] = "20";

	rowname[1] = "acl_expression";
	rowtype[1] = "select";
	rowsize[1] = "10";

	rowname[2] = "acl_value";
	rowtype[2] = "textbox";
	rowsize[2] = "35";

	function setCSSdisplay(cssID, display)
	{
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
	
	function updatevisibility()
	{
		d = document;
		ssloffload = d.getElementById("ssloffload");
		type = d.getElementById("type");
		secondary = d.getElementById("secondary");
		primary_frontend = d.getElementById("primary_frontend");
		
		if (secondary.checked)
			type = primaryfrontends[primary_frontend.value]['ref']['type'];
		else
			type = d.getElementById("type").value;
			
		setCSSdisplay(".haproxy_ssloffloading_enabled", ssloffload.checked);
		setCSSdisplay(".haproxy_mode_http", type == "http");
		setCSSdisplay(".haproxy_primary", !secondary.checked);
		setCSSdisplay(".haproxy_secondary", secondary.checked);
		
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
		var acl = [ <?php foreach ($a_acltypes as $expr) echo "'".$expr['name']."'," ?> ];
		var mode = [ <?php foreach ($a_acltypes as $expr) echo "'".$expr['mode']."'," ?> ];

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
	}
</script>
<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php endif; ?>
<form action="haproxy_listeners_edit.php" method="post" name="iform" id="iform">
	<div class="tabcont">
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Edit haproxy listener</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncellreq">Name</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="name" type="text" <?if(isset($pconfig['name'])) echo "value=\"{$pconfig['name']}\"";?> size="25" maxlength="25">
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Description</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="desc" type="text" <?if(isset($pconfig['desc'])) echo "value=\"{$pconfig['desc']}\"";?> size="64">
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
				<input id="secondary" name="secondary" type="checkbox" value="yes" <?php if ($pconfig['secondary']=='yes') echo "checked"; ?> onclick="updatevisibility();"/>
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
				<input name="port" type="text" <?if(isset($pconfig['port'])) echo "value=\"{$pconfig['port']}\"";?> size="30" maxlength="500">
				<div>The port to listen to.  To specify multiple ports, separate with a comma (,). EXAMPLE: 80,443</div>
			</td>
		</tr>
		<tr class="haproxy_primary" align="left">
			<td width="22%" valign="top" class="vncellreq">Max connections</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="max_connections" type="text" <?if(isset($pconfig['max_connections'])) echo "value=\"{$pconfig['max_connections']}\"";?> size="10" maxlength="10">
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
			<table class="" width="100%" cellpadding="0" cellspacing="0" id='acltable'>
				<tr>
				  <td width="35%" class="">Name</td>
				  <td width="40%" class="">Expression</td>
				  <td width="20%" class="">Value</td>
				  <td width="5%" class=""></td>
				</tr>
				<?php 
				$a_acl=$pconfig['a_acl'];

				if (!is_array($a_acl)) {
					$a_acl=array();
				}

				$counter=0;
				foreach ($a_acl as $acl) {
					$t = haproxy_find_acl($acl['expression']);
					$display = '';
					if (!$t || ($t['mode'] != '' && $t['mode'] != strtolower($pconfig['type'])))
						$display = 'style="display: none;"';
				?>
				<tr id="aclrow<?=$counter;?>" <?=$display;?>>
					<td><input name="acl_name<?=$counter;?>" id="acl_name<?=$counter;?>" type="text" value="<?=$acl['name']; ?>" size="20"/></td>
					<td>
					<select name="acl_expression<?=$counter;?>" id="acl_expression<?=$counter;?>">
					<?php
					foreach ($a_acltypes as $expr) { ?>
						<option value="<?=$expr['name'];?>"<?php if($acl['expression'] == $expr['name']) echo " SELECTED"; ?>><?=$expr['descr'];?>:</option>
					<?php } ?>
					</select>
					</td>
					<td><input name="acl_value<?=$counter;?>" id="acl_value<?=$counter;?>" type="text" value="<?=$acl['value']; ?>" size="35"/></td>
					<td class="list">
						 <table border="0" cellspacing="0" cellpadding="1"><tr>
						 <td valign="middle">
					  <img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" title="delete entry" width="17" height="17" border="0" onclick="removeRow(this); return false;">
						 </td>
						 <td valign="middle">
					 <img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="duplicate entry" width="17" height="17" border="0" onclick="dupRow(<?=$counter;?>, 'acltable'); return false;">
						 </td></tr></table>
					</td>
				</tr>
				<?php
				$counter++;
				}
				?>
			</table>
			<a onclick="javascript:addRowTo('acltable'); return false;" href="#">
			<img border="0" src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" alt="" title="add another entry" />
			</a><br/>
			acl's with the same name wil be 'combined', acl's with different names will be evaluated seperately.<br/>
			For more information about ACL's please see <a href='http://haproxy.1wt.eu/download/1.5/doc/configuration.txt' target='_new'>HAProxy Documentation</a> Section 7 - Using ACL's
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
				<input name="client_timeout" type="text" <?if(isset($pconfig['client_timeout'])) echo "value=\"{$pconfig['client_timeout']}\"";?> size="10" maxlength="10">
				<div>the time (in milliseconds) we accept to wait for data from the client, or for the client to accept data (default 30000).</div>
			</td>
		</tr>
		<tr align="left" class="haproxy_mode_http">
			<td width="22%" valign="top" class="vncell">Use 'forwardfor' option</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="forwardfor" name="forwardfor" type="checkbox" value="yes" <?php if ($pconfig['forwardfor']=='yes') echo "checked"; ?>>
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
				<input name="advanced_bind" type="text" <?if(isset($pconfig['advanced_bind'])) echo "value=\"".htmlspecialchars($pconfig['advanced_bind'])."\"";?> size="64">
				<br/>
				NOTE: paste text into this box that you would like to pass behind the bind option.
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Advanced pass thru</td>
			<td width="78%" class="vtable" colspan="2">
				<textarea name='advanced' rows="4" cols="70" id='advanced'><?php echo htmlspecialchars($pconfig['advanced']); ?></textarea>
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
				<input id="ssloffload" name="ssloffload" type="checkbox" value="yes" <?php if ($pconfig['ssloffload']=='yes') echo "checked";?> onclick="updatevisibility();"><strong>Use Offloading</strong></input>
				<br/>
				The SSL Offloading will reduce web servers load by encrypt data to users on internet and send it without encrytion to internal servers.  
			</td>
		</tr>
		<tr class="haproxy_ssloffloading_enabled" align="left">
			<td width="22%" valign="top" class="vncell">Certificate</td>
			<td width="78%" class="vtable" colspan="2">
				<?  
					$servercerts = get_certificates_server();
					echo_html_select("ssloffloadcert", $servercerts, $pconfig['ssloffloadcert'], '<b>No Certificates defined.</b> <br/>Create one under <a href="system_certmanager.php">System &gt; Cert Manager</a>.');
				?>
				<br/>
				NOTE: choose the cert to use on this frontend.
			</td>
		</tr>
		<tr class="haproxy_ssloffloading_enabled" align="left">
			<td width="22%" valign="top" class="vncell">ACL for certificate CN</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="ssloffloadacl" name="ssloffloadacl" type="checkbox" value="yes" <?php if ($pconfig['ssloffloadacl']=='yes') echo "checked";?> onclick="updatevisibility();">Add ACL for certificate CommonName.</input>
			</td>
		</tr>
		<tr class="haproxy_ssloffloading_enabled haproxy_primary" align="left">
			<td width="22%" valign="top" class="vncell">Advanced ssl options</td>
			<td width="78%" class="vtable" colspan="2">
				<input type='text' name='dcertadv' size="64" id='dcertadv' <?if(isset($pconfig['dcertadv'])) echo "value=\"{$pconfig['dcertadv']}\"";?> size="10" maxlength="64">
				<br/>
				NOTE: Paste additional ssl options(without commas) to include on ssl listening options.<br>
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
				<input name="Submit" type="submit" class="formbtn" value="Save">  
				<input type="button" class="formbtn" value="Cancel" onclick="history.back()">
				<?php if (isset($id) && $a_backend[$id]): ?>
				<input name="id" type="hidden" value="<?=$a_backend[$id]['name'];?>">
				<?php endif; ?>
			</td>
		</tr>
		<tr>
			<td colspan='3'>
					<span class="vexpl"><b>NOTE:</b> You must add a firewall rule permitting access to this frontend!</span>
			</td>
		</tr>
	</table>
	</div>
	</form>
<br>
<script type="text/javascript">
<?
	phparray_to_javascriptarray($primaryfrontends,"primaryfrontends",Array('/*','/*/name','/*/ref','/*/ref/type','/*/ref/ssloffload'));
	phparray_to_javascriptarray($a_closetypes,"closetypes",Array('/*','/*/name','/*/descr'));
	
?>

</script>
<script type="text/javascript">
	field_counter_js = 3;
	rows = 1;
	totalrows =  <?php echo $counter; ?>;
	loaded =  <?php echo $counter; ?>;
	
	updatevisibility();
</script>
<?php include("fend.inc"); ?>
</body>
</html>
