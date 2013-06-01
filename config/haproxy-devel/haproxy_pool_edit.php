<?php
/* $Id: load_balancer_pool_edit.php,v 1.24.2.23 2007/03/03 00:07:09 smos Exp $ */
/*
	haproxy_pool_edit.php
	part of pfSense (http://www.pfsense.com/)
	Copyright (C) 2009 Scott Ullrich <sullrich@pfsense.com>
	Copyright (C) 2008 Remco Hoef <remcoverhoef@pfsense.com>
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

$d_haproxyconfdirty_path = $g['varrun_path'] . "/haproxy.conf.dirty";

if (!is_array($config['installedpackages']['haproxy']['ha_pools']['item'])) {
	$config['installedpackages']['haproxy']['ha_pools']['item'] = array();
}

$a_pools = &$config['installedpackages']['haproxy']['ha_pools']['item'];

if (isset($_POST['id']))
	$id = $_POST['id'];
else
	$id = $_GET['id'];

if (isset($_GET['dup']))
	$id = $_GET['dup'];

global $simplefields;
$simplefields = array(
"name","cookie","balance","transparent_clientip","transparent_interface",
"check_type","checkinter","httpcheck_method","monitor_uri","monitor_httpversion","monitor_username","monitor_domain","monitor_agentport",
"connection_timeout","server_timeout","retries",
"stats_enabled","stats_username","stats_password","stats_uri","stats_realm","stats_admin","stats_node_enabled","stats_node","stats_desc","stats_refresh");

if (isset($id) && $a_pools[$id]) {
	$pconfig['advanced'] = base64_decode($a_pools[$id]['advanced']);
	$pconfig['advanced_backend'] = base64_decode($a_pools[$id]['advanced_backend']);
	$pconfig['a_servers']=&$a_pools[$id]['ha_servers']['item'];	
	
	foreach($simplefields as $stat)
		$pconfig[$stat] = $a_pools[$id][$stat];
}

if (isset($_GET['dup']))
	unset($id);

$changedesc = "Services: HAProxy: pools: ";
$changecount = 0;

if ($_POST) {
	$changecount++;

	unset($input_errors);
	$pconfig = $_POST;
	
	$reqdfields = explode(" ", "name");
	$reqdfieldsn = explode(",", "Name");		
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if ($_POST['stats_enabled']) {
		$reqdfields = explode(" ", "name stats_username stats_password stats_uri stats_realm");
		$reqdfieldsn = explode(",", "Name,Stats Username,Stats Password,Stats Uri,Stats Realm");		
		do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
	}
	
	if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['name']))
		$input_errors[] = "The field 'Name' contains invalid characters.";
	
	if ($_POST['checkinter'] !== "" && !is_numeric($_POST['checkinter']))
		$input_errors[] = "The field 'Check frequency' value is not a number.";
	
	if ($_POST['connection_timeout'] !== "" && !is_numeric($_POST['connection_timeout']))
		$input_errors[] = "The field 'Connection timeout' value is not a number.";

	if ($_POST['server_timeout'] !== "" && !is_numeric($_POST['server_timeout']))
		$input_errors[] = "The field 'Server timeout' value is not a number.";

	if ($_POST['retries'] !== "" && !is_numeric($_POST['retries']))
		$input_errors[] = "The field 'Retries' value is not a number.";

	// the colon ":" is invalid in the username, other than that pretty much any character can be used.
	if (preg_match("/[^a-zA-Z0-9!-\/;-~ ]/", $_POST['stats_username']))
		$input_errors[] = "The field 'Stats Username' contains invalid characters.";

	// the colon ":" can also be used in the password
	if (preg_match("/[^a-zA-Z0-9!-~ ]/", $_POST['stats_password']))
		$input_errors[] = "The field 'Stats Password' contains invalid characters.";

	/* Ensure that our pool names are unique */
	for ($i=0; isset($config['installedpackages']['haproxy']['ha_pools']['item'][$i]); $i++)
		if (($_POST['name'] == $config['installedpackages']['haproxy']['ha_pools']['item'][$i]['name']) && ($i != $id))
			$input_errors[] = "This pool name has already been used.  Pool names must be unique.";

	$a_servers=array();			
	for($x=0; $x<99; $x++) {
		$server_name     = $_POST['server_name'.$x];
		$server_address  = $_POST['server_address'.$x];
		$server_port     = $_POST['server_port'.$x];
		$server_ssl      = $_POST['server_ssl'.$x];
		$server_weight   = $_POST['server_weight'.$x];
		$server_status   = $_POST['server_status'.$x];
		$server_advanced = $_POST['server_advanced'.$x];

		if ($server_address) {
			$server = array();
			$server['name']     = $server_name;
			$server['address']  = $server_address;
			$server['port']     = $server_port;
			$server['ssl']      = $server_ssl;
			$server['weight']   = $server_weight;
			$server['status']   = $server_status;
			$server['advanced'] = $server_advanced;
			$a_servers[] = $server;

			if (preg_match("/[^a-zA-Z0-9\.\-_]/", $server_name))
				$input_errors[] = "The field 'Name' contains invalid characters.";
			if (preg_match("/[^a-zA-Z0-9\.\-_]/", $server_address))
				$input_errors[] = "The field 'Address' contains invalid characters.";

			if (!preg_match("/.{2,}/", $server_name))
				$input_errors[] = "The field 'Name' is required (and must be at least 2 characters).";

			if (!preg_match("/.{2,}/", $server_address))
				$input_errors[] = "The field 'Address' is required (and must be at least 2 characters).";


			if (!is_numeric($server_weight))
				$input_errors[] = "The field 'Weight' value is not a number.";

			if ($server_port && !is_numeric($server_port))
				$input_errors[] = "The field 'Port' value is not a number.";
		}
	}

	if (!$input_errors) {
		$pool = array();
		if(isset($id) && $a_pools[$id])
			$pool = $a_pools[$id];
			
		if ($pool['name'] != $_POST['name']) {
			// name changed:
			if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
				$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
			}
			$a_backend = &$config['installedpackages']['haproxy']['ha_backends']['item'];

			for ( $i = 0; $i < count($a_backend); $i++) {
				if ($a_backend[$i]['backend_serverpool'] == $pool['name'])
					$a_backend[$i]['backend_serverpool'] = $_POST['name'];
			}
		}

		if($pool['name'] != "")
			$changedesc .= " modified pool: '{$pool['name']}'";

		$pool['ha_servers']['item']=$a_servers;

		update_if_changed("name", $pool['name'], $_POST['name']);
		update_if_changed("cookie", $pool['cookie'], $_POST['cookie']);
		update_if_changed("advanced", $pool['advanced'], base64_encode($_POST['advanced']));
		update_if_changed("advanced_backend", $pool['advanced_backend'], base64_encode($_POST['advanced_backend']));
		update_if_changed("checkinter", $pool['checkinter'], $_POST['checkinter']);
		update_if_changed("monitor_uri", $pool['monitor_uri'], $_POST['monitor_uri']);

		global $simplefields;
		foreach($simplefields as $stat)
			update_if_changed($stat, $pool[$stat], $_POST[$stat]);
	
		if (isset($id) && $a_pools[$id]) {
			$a_pools[$id] = $pool;
		} else {
			$a_pools[] = $pool;
		}

		if ($changecount > 0) {
			touch($d_haproxyconfdirty_path);
			write_config($changedesc);			
			/*
			echo "<PRE>";
			print_r($config);
			echo "</PRE>";
			*/
		}

		header("Location: haproxy_pools.php");
		exit;
	}
	$pconfig['a_servers']=&$a_pools[$id]['ha_servers']['item'];	
}

$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
if(strstr($pfSversion, "1.2"))
	$one_two = true;

$pgtitle = "HAProxy: Backend: Edit";
include("head.inc");

row_helper();

// 'processing' done, make all simple fields usable in html.
foreach($simplefields as $field){
	$pconfig[$field] = htmlspecialchars($pconfig[$field]);
}
?>

<input type='hidden' name='address_type' value='textbox' />

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
  <style type="text/css">
	.haproxy_stats_visible{display:none;}
	.haproxy_check_enabled{display:none;}
	.haproxy_check_http{display:none;}
	.haproxy_check_username{display:none;}
	.haproxy_check_smtp{display:none;}
	.haproxy_transparent_clientip{display:none;}
	.haproxy_check_agent{display:none;}
  </style>
<script language="javascript">
	function clearcombo(){
	  for (var i=document.iform.serversSelect.options.length-1; i>=0; i--){
		document.iform.serversSelect.options[i] = null;
	  }
	  document.iform.serversSelect.selectedIndex = -1;
	}

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
		setCSSdisplay(".haproxy_stats_visible", stats_enabled.checked);
		
		check_type = d.getElementById("check_type").value;
		check_type_description = d.getElementById("check_type_description");
		check_type_description.innerHTML=checktypes[check_type]["descr"]; 
		setCSSdisplay(".haproxy_check_enabled", check_type != 'none');
		setCSSdisplay(".haproxy_check_http", check_type == 'HTTP');
		setCSSdisplay(".haproxy_check_username", check_type == 'MySQL' ||  check_type == 'PostgreSQL');
		setCSSdisplay(".haproxy_check_smtp", check_type == 'SMTP' ||  check_type == 'ESMTP');
		setCSSdisplay(".haproxy_check_agent", check_type == 'Agent');

		transparent_clientip = d.getElementById("transparent_clientip");
		setCSSdisplay(".haproxy_transparent_clientip", transparent_clientip.checked);
		
		monitor_username = d.getElementById("monitor_username");
		sqlcheckusername = d.getElementById("sqlcheckusername");
		if(!browser_InnerText_support){
			sqlcheckusername.textContent = monitor_username.value;
		} else{
			sqlcheckusername.innerText = monitor_username.value;
		}
	}


</script>
<script type="text/javascript">
	rowname[0] = "server_name";
	rowtype[0] = "textbox";
	rowsize[0] = "30";
	rowname[1] = "server_address";
	rowtype[1] = "textbox";
	rowsize[1] = "20";
	rowname[2] = "server_port";
	rowtype[2] = "textbox";
	rowsize[2] = "5";
	rowname[3] = "server_ssl";
	rowtype[3] = "checkbox";
	rowsize[3] = "5";
	rowname[4] = "server_weight";
	rowtype[4] = "textbox";
	rowsize[4] = "5";
	rowname[5] = "server_status";
	rowtype[5] = "select";
	rowsize[5] = "1";
	rowname[6] = "server_advanced";
	rowtype[6] = "textbox";
	rowsize[6] = "20";
</script>
<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php endif; ?>
	<form action="haproxy_pool_edit.php" method="post" name="iform" id="iform">
	<div class="tabcont">
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Edit HAProxy pool</td>
		</tr>	
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Name</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="name" type="text" <?if(isset($pconfig['name'])) echo "value=\"{$pconfig['name']}\"";?> size="16" maxlength="16">
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Cookie</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="cookie" type="text" <?if(isset($pconfig['cookie'])) echo "value=\"{$pconfig['cookie']}\"";?>size="64"><br/>
				  This value will be checked in incoming requests, and the first
				  operational pool possessing the same value will be selected. In return, in
				  cookie insertion or rewrite modes, this value will be assigned to the cookie
				  sent to the client. There is nothing wrong in having several servers sharing
				  the same cookie value, and it is in fact somewhat common between normal and
				  backup servers. See also the "cookie" keyword in backend section.
				
			</td>
		</tr>
		<tr align="left">
			<td class="vncell" colspan="3"><strong>Server list</strong>
			
			<table class="" width="100%" cellpadding="0" cellspacing="0" id='servertable'>
	                <tr>
	                  <td width="20%" class="listhdrr">Name</td>
	                  <td width="10%" class="listhdrr">Address</td>
	                  <td width="5%" class="listhdrr">Port</td>
	                  <td width="5%" class="listhdrr">SSL</td>
	                  <td width="8%" class="listhdrr">Weight</td>
	                  <td width="5%" class="listhdr">Backup</td>
	                  <td width="15%" class="listhdr">Advanced</td>
	                  <td width="4%" class=""></td>
			</tr>
			<?php 
			$a_servers=$pconfig['a_servers'];

			if (!is_array($a_servers)) {
				$a_servers=array();
			}

			$counter=0;
			foreach ($a_servers as $server) {
			?>
			<tr id="tr_view_<?=$counter;?>" name="tr_view_<?=$counter;?>" ondblclick="editRow(<?=$counter;?>); return false;" >
			<td class="vtable listlr"><?=$server['name']; ?></td>
			<td class="vtable listr"><?=$server['address']; ?></td>
			<td class="vtable listr"><?=$server['port']; ?></td>
			<td class="vtable listr"><?=$server['ssl']=='yes'?'yes':'no'; ?></td>
			<td class="vtable listr"><?=$server['weight']; ?></td>
			<td class="vtable listr"><?=$server['status']; ?></td>
			<td class="vtable listr"><?=htmlspecialchars($server['advanced']); ?></td>
			<td class="list">
			  <table border="0" cellspacing="0" cellpadding="1"><tr>
			  <td valign="middle">
			  <img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" title="edit entry" width="17" height="17" border="0" onclick="editRow(<?=$counter;?>); return false;">
			  </td>
			  <td valign="middle">
			  <img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" title="delete entry" width="17" height="17" border="0" onclick="deleteRow(<?=$counter;?>, 'servertable'); return false;">
			  </td>
			  <td valign="middle">
			  <img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="duplicate entry" width="17" height="17" border="0" onclick="dupRow(<?=$counter;?>, 'servertable'); return false;">
			  </td></tr></table>
			</td>
			</tr>
			<tr id="tr_edit_<?=$counter;?>" name="tr_edit_<?=$counter;?>" style="display: none;">
				<td class="vtable">
				  <input name="server_name<?=$counter;?>" id="server_name<?=$counter;?>" type="text" value="<?=$server['name']; ?>" size="30"/></td>
				<td class="vtable">
				  <input name="server_address<?=$counter;?>" id="server_address<?=$counter;?>" type="text" value="<?=$server['address']; ?>" size="20"/></td>
				<td class="vtable">
				  <input name="server_port<?=$counter;?>" id="server_port<?=$counter;?>" type="text" value="<?=$server['port']; ?>" size="5"/></td>
				<td class="vtable">
				  <input name="server_ssl<?=$counter;?>" id="server_ssl<?=$counter;?>" type="checkbox" value="yes" <?=$server['ssl']=='yes'?"checked":""; ?> size="5"/></td>
				<td class="vtable">
				  <input name="server_weight<?=$counter;?>" id="server_weight<?=$counter;?>" type="text" value="<?=$server['weight']; ?>" size="5"/></td>
				<td class="vtable">
				<select name="server_status<?=$counter;?>" id="server_status<?=$counter;?>">
				  <option value="active" <?php if($server['status']=='active') echo "SELECTED";?>>active</option>
				  <option value="backup" <?php  if($server['status']=='backup') echo "SELECTED";?>>backup</option>
				  <option value="disabled" <?php  if($server['status']=='disabled') echo "SELECTED";?>>disabled</option>
				  <option value="inactive" <?php  if($server['status']=='inactive') echo "SELECTED";?>>inactive</option>
				</select>
				</td>
				<td class="vtable">
				  <input name="server_advanced<?=$counter;?>" id="server_advanced<?=$counter;?>" type="text" value="<?=htmlspecialchars($server['advanced']); ?>" size="20"/></td>
				<td class="list">
			         <table border="0" cellspacing="0" cellpadding="1"><tr>
			         <td valign="middle">
				  <img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" title="delete entry" width="17" height="17" border="0" onclick="removeRow(this); return false;">
			         </td>
			         <td valign="middle">
				 <img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="duplicate entry" width="17" height="17" border="0" onclick="dupRow(<?=$counter;?>, 'servertable'); return false;">
			         </td></tr></table>
				</td>
			</tr>
			<?php
			$counter++;
			}
			?>
			</table>
			<a onclick="javascript:addRowTo('servertable'); return false;" href="#">
			<img border="0" src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" alt="" title="add another entry" />
			</a>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Balance</td>
			<td width="78%" class="vtable" colspan="1">
				<table width="100%">
				<tr>
					<td width="25%" valign="top">
						<input type="radio" name="balance" id="balance" value="roundrobin"<?php if($pconfig['balance'] == "roundrobin") echo " CHECKED"; ?>>Round robin</input>
					</td>
					<td>
						  Each server is used in turns, according to their weights.
		                  This is the smoothest and fairest algorithm when the server's
		                  processing time remains equally distributed. This algorithm
		                  is dynamic, which means that server weights may be adjusted
		                  on the fly for slow starts for instance.
					</td>
				</tr>
				<tr>
					<td width="25%" valign="top">
						<input type="radio" name="balance" id="balance" value="static-rr"<?php if($pconfig['balance'] == "static-rr") echo " CHECKED"; ?>>Static Round Robin</input>
					</td>
					<td>
						Each server is used in turns, according to their weights.
				This algorithm is as similar to roundrobin except that it is
				static, which means that changing a server's weight on the
				fly will have no effect. On the other hand, it has no design
				limitation on the number of servers, and when a server goes
				up, it is always immediately reintroduced into the farm, once
				the full map is recomputed. It also uses slightly less CPU to
				run (around -1%).					
					</td>
				</tr>
				<tr>
					<td width="25%" valign="top">
						<input type="radio" name="balance" id="balance" value="leastconn"<?php if($pconfig['balance'] == "leastconn") echo " CHECKED"; ?>>Least Connections</input>
					</td>
					<td>
						  The server with the lowest number of connections receives the
				connection. Round-robin is performed within groups of servers
				of the same load to ensure that all servers will be used. Use
				of this algorithm is recommended where very long sessions are
				expected, such as LDAP, SQL, TSE, etc... but is not very well
				suited for protocols using short sessions such as HTTP. This
				algorithm is dynamic, which means that server weights may be
				adjusted on the fly for slow starts for instance.
					</td>
				</tr>
				  <tr><td valign="top"><input type="radio" name="balance" id="balance" value="source"<?php if($pconfig['balance'] == 
"source") echo " CHECKED"; ?>>Source</input></td><td>
		 			  The source IP address is hashed and divided by the total
	                  weight of the running servers to designate which server will
	                  receive the request. This ensures that the same client IP
	                  address will always reach the same server as long as no
	                  server goes down or up. If the hash result changes due to the
	                  number of running servers changing, many clients will be
					  directed to a different server. This algorithm is generally
	                  used in TCP mode where no cookie may be inserted. It may also
	                  be used on the Internet to provide a best-effort stickyness
	                  to clients which refuse session cookies. This algorithm is
	                  static, which means that changing a server's weight on the
	                  fly will have no effect.
					</td>
				</tr>
				</table>
			</td>
		</tr>
		<tr align="left" style="display:none;">
			<td width="22%" valign="top" class="vncell">Transparent ClientIP</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="transparent_clientip" name="transparent_clientip" type="checkbox" value="yes" <?php if ($pconfig['transparent_clientip']=='yes') echo "checked"; ?> onclick='updatevisibility();'>
				Use Client-IP to connect to backend servers.
				<div class="haproxy_transparent_clientip">
			
			<?
				$interfaces = get_configured_interface_with_descr();
				$interfaces2 = array();
				foreach($interfaces as $key => $name)
				{
					
					$interfaces2[$key]['name'] = $name;
				}
				echo_html_select("transparent_interface",$interfaces2,$pconfig['transparent_interface']?$pconfig['transparent_interface']:"lan","","updatevisibility();");	
			?>Interface that will connect to the backend server. (this will generally be your LAN or OPT1(dmz) interface)<br/>			
				</div>
				<br/>
				Connect transparently to the backend server's so the connection seams to come straight from the client ip address.
				For proper workings this requires the reply's traffic to pass through pfSense by means of correct routing.
				(uses the option "source 0.0.0.0 usesrc clientip")
				<br/><br/>
				Note : When this is enabled for a single backend HAProxy will run as 'root', which reduces security.
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Per server pass thru</td>
			<td width="78%" class="vtable" colspan="2">
				<input type="text" name='advanced' id='advanced' value='<?php echo $pconfig['advanced']; ?>' size="64">
				<br/>
				NOTE: paste text into this box that you would like to pass thru. Applied to each 'server' line.
			</td>
		</tr>

		<tr align="left">
			<td width="22%" valign="top" class="vncell">Backend pass thru</td>
			<td width="78%" class="vtable" colspan="2">
				<textarea  rows="4" cols="70" name='advanced_backend' id='advanced_backend'><?php echo $pconfig['advanced_backend']; ?></textarea>
				<br/>
				NOTE: paste text into this box that you would like to pass thru. Applied to the backend section.
			</td>
		</tr>

	</table>
	<br/>
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Health checking</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Health check method</td>
			<td width="78%" class="vtable" colspan="2">
				<?
				echo_html_select("check_type",$a_checktypes,$pconfig['check_type']?$pconfig['check_type']:"HTTP","","updatevisibility();");
				?><br/>
				<textarea readonly="yes" cols="60" rows="2" id="check_type_description" name="check_type_description" style="padding:5px; border:1px dashed #990000; background-color: #ffffff; color: #000000; font-size: 8pt;"></textarea>
			</td>
		</tr>
		<tr align="left" class="haproxy_check_enabled">
			<td width="22%" valign="top" class="vncell">Check frequency</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="checkinter" type="text" <?if(isset($pconfig['checkinter'])) echo "value=\"{$pconfig['checkinter']}\"";?>size="20"> milliseconds
				<br/>For HTTP/HTTPS defaults to 1000 if left blank. For TCP no check will be performed if left empty.
			</td>
		</tr>
		<tr align="left" class="haproxy_check_http">
			<td width="22%" valign="top" class="vncell">Http check method</td>
			<td width="78%" class="vtable" colspan="2">
				<?
				echo_html_select("httpcheck_method",$a_httpcheck_method,$pconfig['httpcheck_method']);
				?>
				<br/>OPTIONS is the method usually best to perform server checks, HEAD and GET can also be used
			</td>
		</tr>
		<tr align="left" class="haproxy_check_http">
			<td width="22%" valign="top" class="vncell">Http check URI</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="monitor_uri" type="text" <?if(isset($pconfig['monitor_uri'])) echo "value=\"{$pconfig['monitor_uri']}\"";?>size="64">
				<br/>Defaults to / if left blank.
			</td>
		</tr>
		<tr align="left" class="haproxy_check_http">
			<td width="22%" valign="top" class="vncell">Http check version</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="monitor_httpversion" type="text" <?if(isset($pconfig['monitor_httpversion'])) echo "value=\"{$pconfig['monitor_httpversion']}\"";?>size="64">
				<br/>Defaults to "HTTP/1.0" if left blank.
				Note that the Host field is mandatory in HTTP/1.1, and as a trick, it is possible to pass it
				after "\r\n" following the version string like this:<br/>
				&nbsp;&nbsp;&nbsp;&nbsp;"<i>HTTP/1.1\r\nHost:\ www</i>"<br/>
				Also some hosts might require an accept parameter like this:<br/>
				&nbsp;&nbsp;&nbsp;&nbsp;"<i>HTTP/1.0\r\nHost:\ webservername:8080\r\nAccept:\ */*</i>"
			</td>
		</tr>
		<tr align="left" class="haproxy_check_username">
			<td width="22%" valign="top" class="vncell">Check with Username</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="monitor_username" id="monitor_username" type="text" <?if(isset($pconfig['monitor_username'])) echo "value=\"{$pconfig['monitor_username']}\"";?>size="64" onchange="updatevisibility();" onkeyup="updatevisibility();">
				<br/>
				This is the username which will be used when connecting to MySQL/PostgreSQL server.
				<pre>
USE mysql;
CREATE USER '<span id="sqlcheckusername" name="sqlcheckusername"></span>'@'&lt;pfSenseIP&gt;';
FLUSH PRIVILEGES;</pre>
			</td>
		</tr>
		<tr align="left" class="haproxy_check_smtp">
			<td width="22%" valign="top" class="vncell">Domain</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="monitor_domain" type="text" <?if(isset($pconfig['monitor_domain'])) echo "value=\"{$pconfig['monitor_domain']}\"";?>size="64">
			</td>
		</tr>
		<tr align="left" class="haproxy_check_agent">
			<td width="22%" valign="top" class="vncell">Agentport</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="monitor_agentport" type="text" <?if(isset($pconfig['monitor_agentport'])) echo "value=\"{$pconfig['monitor_agentport']}\"";?>size="64">
				<br/>
				Fill in the TCP portnumber the healthcheck should be performed on.
			</td>
		</tr>
	</table>
	<br/>
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Advanced settings</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Connection timeout</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="connection_timeout" type="text" <?if(isset($pconfig['connection_timeout'])) echo "value=\"{$pconfig['connection_timeout']}\"";?> size="64">
				<div>the time (in milliseconds) we give up if the connection does not complete within (default 30000).</div>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Server timeout</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="server_timeout" type="text" <?if(isset($pconfig['server_timeout'])) echo "value=\"{$pconfig['server_timeout']}\"";?> size="64">
				<div>the time (in milliseconds) we accept to wait for data from the server, or for the server to accept data (default 30000).</div>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Retries</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="retries" type="text" <?if(isset($pconfig['retries'])) echo "value=\"{$pconfig['retries']}\"";?> size="64">
				<div>After a connection failure to a server, it is possible to retry, potentially
on another server. This is useful if health-checks are too rare and you don't
want the clients to see the failures. The number of attempts to reconnect is
set by the 'retries' parameter.</div>
			</td>
		</tr>
	</table>
	<br/>&nbsp;<br/>	
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Statistics</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Stats Enabled</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_enabled" name="stats_enabled" type="checkbox" value="yes" <?php if ($pconfig['stats_enabled']=='yes') echo "checked"; ?> onclick='updatevisibility();'>
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_realm_row' name='stats_realm_row'>
			<td width="22%" valign="top" class="vncellreq">Stats Realm</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_realm" name="stats_realm" type="text" <?if(isset($pconfig['stats_realm'])) echo "value=\"{$pconfig['stats_realm']}\"";?> size="64"><br/>
				EXAMPLE: haproxystats
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_uri_row' name='stats_uri_row'>
			<td width="22%" valign="top" class="vncellreq">Stats Uri</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_uri" name="stats_uri" type="text" <?if(isset($pconfig['stats_uri'])) echo "value=\"{$pconfig['stats_uri']}\"";?> size="64"><br/>
				EXAMPLE: /haproxy?stats
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_username_row' name='stats_username_row'>
			<td width="22%" valign="top" class="vncellreq">Stats Username</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_username" name="stats_username" type="text" <?if(isset($pconfig['stats_username'])) echo "value=\"".$pconfig['stats_username']."\"";?> size="64">
			</td>
		</tr>
		
		<tr class="haproxy_stats_visible" align="left" id='stats_password_row' name='stats_password_row'>
			<td width="22%" valign="top" class="vncellreq">Stats Password</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_password" name="stats_password" type="password" <?
					if(isset($pconfig['stats_password'])) 
						echo "value=\"".$pconfig['stats_password']."\"";
					?> size="64">
				<br/>
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_node_admin_row' name='stats_node_enabled_row'>
			<td width="22%" valign="top" class="vncell">Stats Admin</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_admin" name="stats_admin" type="checkbox" value="yes" <?php if ($pconfig['stats_admin']=='yes') echo "checked"; ?>>
				<br/>
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_node_enabled_row' name='stats_node_enabled_row'>
			<td width="22%" valign="top" class="vncell">Stats Enable Node Name</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_node_enabled" name="stats_node_enabled" type="checkbox" value="yes" <?php if ($pconfig['stats_node_enabled']=='yes') echo "checked"; ?>>
				<br/>
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_node_row' name='stats_node_row'>
			<td width="22%" valign="top" class="vncell">Stats Node</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_node" name="stats_node" type="text" <?if(isset($pconfig['stats_node'])) echo "value=\"{$pconfig['stats_node']}\"";?> size="64"><br/>
				The node name is displayed in the stats and helps to differentiate which server in a cluster is actually serving clients.<br/>
				Leave blank to use the system name.
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_desc_row' name='stats_desc_row'>
			<td width="22%" valign="top" class="vncell">Stats Description</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_desc" name="stats_desc" type="text" <?if(isset($pconfig['stats_node'])) echo "value=\"{$pconfig['stats_desc']}\"";?> size="64"><br/>
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_refresh_row' name='stats_refresh_row'>
			<td width="22%" valign="top" class="vncell">Stats Refresh</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_refresh" name="stats_refresh" type="text" <?if(isset($pconfig['stats_refresh'])) echo "value=\"{$pconfig['stats_refresh']}\"";?> size="10" maxlength="30"><br/>
				Specify the refresh rate of the stats page in seconds, or specified time unit (us, ms, s, m, h, d).
			</td>
		</tr>
	</table>	
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr align="left">
			<td width="22%" valign="top">&nbsp;</td>
			<td width="78%">
				<input name="Submit" type="submit" class="formbtn" value="Save">  
				<input type="button" class="formbtn" value="Cancel" onclick="history.back()">
				<?php if (isset($id) && $a_pools[$id]): ?>
				<input name="id" type="hidden" value="<?=$id;?>">
				<?php endif; ?>
			</td>
		</tr>
	</table>
	</div>
	</form>
<br>
<?php include("fend.inc"); ?>
<script type="text/javascript">
<?
	phparray_to_javascriptarray($a_checktypes,"checktypes",Array('/*','/*/name','/*/descr'));
?>
	browser_InnerText_support = (document.getElementsByTagName("body")[0].innerText != undefined) ? true : false;

	field_counter_js = 7;
	rows = 1;
	totalrows =  <?php echo $counter; ?>;
	loaded =  <?php echo $counter; ?>;
	updatevisibility();
</script>
</body>
</html>

<?php

function row_helper() {
	$options = <<<EOD
  <option value='active' SELECTED>active</option>"+
"  <option value='backup'>backup</option>"+
"  <option value='disabled'>disabled</option>"+
"  <option value='inactive'>inactive</option>
EOD;

	echo <<<EOF
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
	var d, tbody, tr, td, bgc, i, ii, j;
	var btable, btbody, btr, btd;

	d = document;
	tbody = d.getElementById(tableId).getElementsByTagName("tbody").item(0);
	tr = d.createElement("tr");
	totalrows++;
	for (i = 0; i < field_counter_js; i++) {
		td = d.createElement("td");
		if(rowtype[i] == 'textbox') {
			td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows +
				"'></input><input size='" + rowsize[i] + "' name='" + rowname[i] + totalrows +
				"' id='" + rowname[i] + totalrows + "'></input> ";
		} else if(rowtype[i] == 'select') {
			td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows +
				"'></input><select size='" + rowsize[i] + "' name='" + rowname[i] + totalrows +
			       	"' id='" + rowname[i] + totalrows + "'>$options</select> ";
		} else {
			td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows +
				"'></input><input type='checkbox' name='" + rowname[i] + totalrows +
				"' id='" + rowname[i] + totalrows + "' value='yes'></input> ";
		}
		td.setAttribute("class","vtable");
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
	btd.innerHTML = '<img src="/themes/' + theme + "/images/icons/icon_plus.gif\" title=\"duplicate entry\" width=\"17\" height=\"17\" border=\"0\" onclick=\"dupRow(" + totalrows + ", 'servertable'); return false;\">";
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
	    if(rowtype[i] == 'checkbox')
		newEl.checked = dupEl.checked;
	    else
                newEl.value = dupEl.value;
    }
}

function deleteRow(rowId, tableId) {
	var view = document.getElementById("tr_view_" + rowId);
	var edit = document.getElementById("tr_edit_" + rowId);

	view.parentNode.removeChild(view);
	edit.parentNode.removeChild(edit);
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
function editRow(num) {
    var trview = document.getElementById('tr_view_' + num);
    var tredit = document.getElementById('tr_edit_' + num);

    trview.style.display='none';
    tredit.style.display='';
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
</script>

EOF;

}

?>
