<?php
/* $Id: load_balancer_pool_edit.php,v 1.24.2.23 2007/03/03 00:07:09 smos Exp $ */
/*
	haproxy_frontends_edit.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2013 Marcello Coutinho
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
$shortcut_section = "haproxy";
require("guiconfig.inc");

$d_haproxyconfdirty_path = $g['varrun_path'] . "/haproxy.conf.dirty";

if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
	$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
}

$a_backend = &$config['installedpackages']['haproxy']['ha_backends']['item'];

if (isset($_POST['id']))
	$id = $_POST['id'];
else
	$id = $_GET['id'];

if (isset($id) && $a_backend[$id]) {
	$pconfig['name'] = $a_backend[$id]['name'];
	$pconfig['desc'] = $a_backend[$id]['desc'];
	$pconfig['connection_timeout'] = $a_backend[$id]['connection_timeout'];
	$pconfig['server_timeout'] = $a_backend[$id]['server_timeout'];
	$pconfig['retries'] = $a_backend[$id]['retries'];
	
	$pconfig['type'] = $a_backend[$id]['type'];
	$pconfig['balance'] = $a_backend[$id]['balance'];
	$pconfig['monitor_uri'] = $a_backend[$id]['monitor_uri'];

	$pconfig['forwardfor'] = $a_backend[$id]['forwardfor'];
	$pconfig['httpclose'] = $a_backend[$id]['httpclose'];

	$pconfig['stats_enabled'] = $a_backend[$id]['stats_enabled'];
	$pconfig['stats_username'] = $a_backend[$id]['stats_username'];
	$pconfig['stats_password'] = $a_backend[$id]['stats_password'];
	$pconfig['stats_uri'] = $a_backend[$id]['stats_uri'];
	$pconfig['stats_realm'] = $a_backend[$id]['stats_realm'];
	$pconfig['stats_node_enabled'] = $a_backend[$id]['stats_node_enabled'];
	$pconfig['stats_node'] = $a_backend[$id]['stats_node'];
	$pconfig['stats_desc'] = $a_backend[$id]['stats_desc'];
	$pconfig['stats_refresh'] = $a_backend[$id]['stats_refresh'];
		
	$pconfig['type'] = $a_backend[$id]['type'];
	$pconfig['extaddr'] = $a_backend[$id]['extaddr'];	
	$pconfig['max_connections'] = $a_backend[$id]['max_connections'];	
	$pconfig['client_timeout'] = $a_backend[$id]['client_timeout'];	
	$pconfig['port'] = $a_backend[$id]['port'];	
	$pconfig['a_acl']=&$a_backend[$id]['ha_acls']['item'];	
	$pconfig['advanced'] = base64_decode($a_backend[$id]['advanced']);

}

$changedesc = "Services: HAProxy: Frontend";
$changecount = 0;

if ($_POST) {
	$changecount++;

	unset($input_errors);
	$pconfig = $_POST;
	
	if ($_POST['stats_enabled']) {
		$reqdfields = explode(" ", "name connection_timeout server_timeout stats_username stats_password stats_uri stats_realm");
		$reqdfieldsn = explode(",", "Name,Connection timeout,Server timeout,Stats Username,Stats Password,Stats Uri,Stats Realm");		
	} else {
		$reqdfields = explode(" ", "name connection_timeout server_timeout");
		$reqdfieldsn = explode(",", "Name,Connection timeout,Server timeout");		
	}
	
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	$reqdfields = explode(" ", "name type port max_connections client_timeout");
	$reqdfieldsn = explode(",", "Name,Type,Port,Max connections,Client timeout");
	
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['name']))
		$input_errors[] = "The field 'Name' contains invalid characters.";

	if (!is_numeric($_POST['connection_timeout']))
		$input_errors[] = "The field 'Connection timeout' value is not a number.";

	if (!is_numeric($_POST['server_timeout']))
		$input_errors[] = "The field 'Server timeout' value is not a number.";

	if (!$_POST['retries'] && is_numeric($_POST['retries']))
		$input_errors[] = "The field 'Retries' value is not a number.";
		
	if ($_POST['stats_enabled'] == "yes"){
		if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['stats_username']))
			$input_errors[] = "The field 'Stats Username' contains invalid characters.";

		if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['stats_password']))
			$input_errors[] = "The field 'Stats Password' contains invalid characters.";
		}

	if (!is_numeric($_POST['max_connections']))
		$input_errors[] = "The field 'Max connections' value is not a number.";

	$ports = split(",", $_POST['port'] . ",");
	foreach($ports as $port)
		if ($port && !is_numeric($port))
			$input_errors[] = "The field 'Port' value is not a number.";

	if (!is_numeric($_POST['client_timeout']))
		$input_errors[] = "The field 'Client timeout' value is not a number.";

	/* Ensure that our pool names are unique */
	for ($i=0; isset($config['installedpackages']['haproxy']['ha_backends']['item'][$i]); $i++)
		if (($_POST['name'] == $config['installedpackages']['haproxy']['ha_backends']['item'][$i]['name']) && ($i != $id))
			$input_errors[] = "This backend name has already been used. Frontend names must be unique.";

	$a_acl=array();			
	$acl_names=array(); 
	for($x=0; $x<99; $x++) {
		$acl_name=$_POST['acl_name'.$x];
		$acl_expression=$_POST['acl_expression'.$x];

		if ($acl_name) {
			// check for duplicates
			if (in_array($acl_name, $acl_names)) {
				$input_errors[] = "The name '$acl_name' is duplicate.";
			}

			$acl_names[]=$acl_name;

			$acl=array();
			$acl['name']=$acl_name;
			$acl['expression']=$acl_expression;
			$a_acl[]=$acl;

			if (preg_match("/[^a-zA-Z0-9\.\-_]/", $acl_name))
				$input_errors[] = "The field 'Name' contains invalid characters.";

			if (!preg_match("/.{2,}/", $acl_expression))
				$input_errors[] = "The field 'Expression' is required.";

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
		
		if ($backend['name']!=$_POST['name']) {
			// name changed:
			// * update servers
			// * update frontend (default backend and acl)
			if (!is_array($config['installedpackages']['haproxy']['ha_servers']['item'])) {
				$config['installedpackages']['haproxy']['ha_servers']['item'] = array();
			}
			$a_server = &$config['installedpackages']['haproxy']['ha_servers']['item'];

			for ( $i = 0; $i < count($a_server); $i++) {
				if ($a_server[$i]['backend']==$backend['name']) {
					$a_server[$i]['backend']=$_POST['name'];
				}
			}

			if (!is_array($config['installedpackages']['haproxy']['ha_frontends']['item'])) {
				$config['installedpackages']['haproxy']['ha_frontends']['item'] = array();
			}
			$a_frontend = &$config['installedpackages']['haproxy']['ha_frontends']['item'];

			for ( $i = 0; $i < count($a_frontend); $i++) {
				if ($a_frontend[$i]['backend']==$backend['name']) {
					$a_frontend[$i]['backend']=$_POST['name'];
				}
				
				if (!is_array($a_frontend[$i]['ha_acls']['item'])) {
					$a_frontend[$i]['ha_acls']['item'] = array();
				}
				
				$a_acl = &$a_frontend[$i]['ha_acls']['item'];
				for ( $j = 0; $j < count($a_acl); $j++) {
					if ($a_acl[$j]['backend']==$backend['name']) {
						$a_acl[$j]['backend']=$_POST['name'];
					}
				}
			}
		}

		update_if_changed("name", $backend['name'], $_POST['name']);
		update_if_changed("description", $backend['desc'], $_POST['desc']);
		update_if_changed("connection_timeout", $backend['connection_timeout'], $_POST['connection_timeout']);
		update_if_changed("server_timeout", $backend['server_timeout'], $_POST['server_timeout']);
		update_if_changed("retries", $backend['retries'], $_POST['retries']);
		update_if_changed("type", $backend['type'], $_POST['type']);
		update_if_changed("balance", $backend['balance'], $_POST['balance']);
		update_if_changed("cookie_name", $backend['cookie_name'], $_POST['cookie_name']);
		update_if_changed("monitor_uri", $backend['monitor_uri'], $_POST['monitor_uri']);
		update_if_changed("forwardfor", $backend['forwardfor'], $_POST['forwardfor']);
		update_if_changed("httpclose", $backend['httpclose'], $_POST['httpclose']);
		update_if_changed("stats_enabled", $backend['stats_enabled'], $_POST['stats_enabled']);
		update_if_changed("stats_username", $backend['stats_username'], $_POST['stats_username']);
		update_if_changed("stats_password", $backend['stats_password'], $_POST['stats_password']);
		update_if_changed("stats_uri", $backend['stats_uri'], $_POST['stats_uri']);
		update_if_changed("stats_realm", $backend['stats_realm'], $_POST['stats_realm']);
		update_if_changed("stats_node_enabled", $backend['stats_node_enabled'], $_POST['stats_node_enabled']);
		update_if_changed("stats_node", $backend['stats_node'], $_POST['stats_node']);
		update_if_changed("stats_desc", $backend['stats_desc'], $_POST['stats_desc']);
		update_if_changed("stats_desc", $backend['stats_refresh'], $_POST['stats_refresh']);
		update_if_changed("type", $backend['type'], $_POST['type']);
		update_if_changed("port", $backend['port'], $_POST['port']);
		update_if_changed("extaddr", $backend['extaddr'], $_POST['extaddr']);
		update_if_changed("max_connections", $backend['max_connections'], $_POST['max_connections']);
		update_if_changed("client_timeout", $backend['client_timeout'], $_POST['client_timeout']);
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

		header("Location: haproxy_frontends.php");
		exit;
	}
}

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

$pgtitle = "HAProxy: Frontend: Edit";
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
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
	        d = document;
	        tbody = d.getElementById(tableId).getElementsByTagName("tbody").item(0);
	        tr = d.createElement("tr");
	        totalrows++;
	        for (i = 0; i < field_counter_js; i++) {
	                td = d.createElement("td");
	                if(rowtype[i] == 'textbox') {
	                        td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows + "'></input><input size='" + rowsize[i] + "' name='" + rowname[i] + totalrows + "'></input> ";
	                } else if(rowtype[i] == 'select') {
	                        td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows + "'></input><select name='" + rowname[i] + totalrows + "'><?php foreach ($a_backend as $backend) {?><option value=\"<?=$backend['name']?>\"><?=$backend['name']?></option><?php }?></select> ";
	                } else {
	                        td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows + "'></input><input type='checkbox' name='" + rowname[i] + totalrows + "'></input> ";
	                }
	                tr.appendChild(td);
	        }
	        td = d.createElement("td");
	        td.rowSpan = "1";
			td.className = "list";
	        td.innerHTML = '<img src="/themes/' + theme + '/images/icons/icon_x.gif" width="17" height="17" border="0" onclick="removeRow(this); return false;">';
	        tr.appendChild(td);
	        tbody.appendChild(tr);
	    });
	})();

	function removeRow(el) {
	    var cel;
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
	rowsize[0] = "30";

	rowname[1] = "acl_expression";
	rowtype[1] = "textbox";
	rowsize[1] = "35";

	rowname[2] = "acl_backend";
	rowtype[2] = "select";
	rowsize[2] = "10";

	function toggle_stats() {
		var stats_enabled=document.getElementById('stats_enabled');
		var stats_realm_row=document.getElementById('stats_realm_row');
		var stats_username_row=document.getElementById('stats_username_row');
		var stats_password_row=document.getElementById('stats_password_row');
		var stats_uri_row=document.getElementById('stats_uri_row');
		var stats_node_enabled_row=document.getElementById('stats_node_enabled_row');
		var stats_node_row=document.getElementById('stats_node_row');
		var stats_desc_row=document.getElementById('stats_desc_row');
		var stats_desc_row=document.getElementById('stats_refresh_row');
		
		if (stats_enabled.checked) {
			stats_realm_row.style.display='';
			stats_username_row.style.display='';
			stats_password_row.style.display='';
			stats_uri_row.style.display='';
			stats_node_enabled_row.style.display='';
			stats_node_row.style.display='';
			stats_desc_row.style.display='';
			stats_refresh_row.style.display='';
		} else {
			stats_realm_row.style.display='none';
			stats_username_row.style.display='none';
			stats_password_row.style.display='none';
			stats_uri_row.style.display='none';
			stats_node_enabled_row.style.display='none';
			stats_node_row.style.display='none';
			stats_desc_row.style.display='none';
			stats_refresh_row.style.display='none';
		}
	}
</script>
<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php endif; ?>
<form action="haproxy_frontends_edit.php" method="post" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabnavtbl">
  <?php
        /* active tabs */
        $tab_array = array();
		$tab_array[] = array("Settings", false, "haproxy_global.php");
        $tab_array[] = array("Frontends", true, "haproxy_frontends.php");		
		$tab_array[] = array("Servers", false, "haproxy_servers.php");
		$tab_array[] = array("Sync", false, "pkg_edit.php?xml=haproxy_sync.xml");
		display_top_tabs($tab_array);
  ?>
  </td></tr>
  <tr><td>
	<div id="mainarea">
	<table class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Edit haproxy backend</td>
		</tr>		
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Name</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="name" type="text" <?if(isset($pconfig['name'])) echo "value=\"{$pconfig['name']}\"";?> size="25" maxlength="25">
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Description</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="desc" type="text" <?if(isset($pconfig['desc'])) echo "value=\"{$pconfig['desc']}\"";?> size="64">
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Type</td>
			<td width="78%" class="vtable" colspan="2">
				<select name="type" id="type" onchange="type_change();">
					<option value="http"<?php if($pconfig['type'] == "http") echo " SELECTED"; ?>>HTTP</option>
					<option value="https"<?php if($pconfig['type'] == "https") echo " SELECTED"; ?>>HTTPS</option>
					<option value="tcp"<?php if($pconfig['type'] == "tcp") echo " SELECTED"; ?>>TCP</option>
					<option value="health"<?php if($pconfig['type'] == "health") echo " SELECTED"; ?>>Health</option>
				</select>
			</td>
		</tr>
		</tr>
				<tr align="left">
					<td width="22%" valign="top" class="vncellreq">Port</td>
					<td width="78%" class="vtable" colspan="2">
						<input name="port" type="text" <?if(isset($pconfig['port'])) echo "value=\"{$pconfig['port']}\"";?> size="6" maxlength="500">
						<div>The port to listen to.  To specify multiple ports, separate with a comma (,). EXAMPLE: 80,443</div>
					</td>
				</tr>
				<tr>
				  <td width="22%" valign="top" class="vncellreq">Listen address</td>
				  <td width="78%" class="vtable">
					<select name="extaddr" class="formfld">
						<option value="" <?php if (!$pconfig['extaddr']) echo "selected"; ?>>Interface address</option>
						<option value="127.0.0.1" <?php if($pconfig['extaddr'] == "127.0.0.1") echo "selected"; ?>>127.0.0.1 (Localhost)</option>
					<?php
						if (is_array($config['virtualip']['vip'])):
							foreach ($config['virtualip']['vip'] as $sn):
					?>
						<option value="<?=$sn['subnet'];?>" <?php if ($sn['subnet'] == $pconfig['extaddr']) echo "selected"; ?>>
							<?=htmlspecialchars("{$sn['subnet']} ({$sn['descr']})");?>
						</option>
					<?php
							endforeach;
						endif; 	
					?>
						<option value="any" <?php if($pconfig['extaddr'] == "any") echo "selected"; ?>>any</option>
					</select>
					<br/>
					<span class="vexpl">
						If you want this rule to apply to another IP address than the IP address of the interface chosen above,
						select it here (you need to define <a href="firewall_virtual_ip.php">Virtual IP</a> addresses on the first).  
						Also note that if you are trying to redirect connections on the LAN select the "any" option.<br>
						While using carp, select localhost and forward via NAT.
					</span>
				  </td>
				</tr>	
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Balance</td>
			<td width="78%" class="vtable" colspan="2">
				<table width="100%">
				<tr>
					<td width="20%" valign="top">
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
					<td width="20%" valign="top">
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
					<td width="20%" valign="top">
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
			<tr align="left">
				<td width="22%" valign="top" class="vncell">Use 'forwardfor' option</td>
				<td width="78%" class="vtable" colspan="2">
					<input id="forwardfor" name="forwardfor" type="checkbox" value="yes" <?php if ($pconfig['forwardfor']=='yes') echo "checked"; ?>>
					<br/>
					The 'forwardfor' option creates an HTTP 'X-Forwarded-For' header which
					contains the client's IP address. This is useful to let the final web server
					know what the client address was (eg for statistics on domains)
				</td>
			</tr>
			<tr align="left">
				<td width="22%" valign="top" class="vncell">Use 'httpclose' option</td>
				<td width="78%" class="vtable" colspan="2">
					<input id="httpclose" name="httpclose" type="checkbox" value="yes" <?php if ($pconfig['httpclose']=='yes') echo "checked"; ?>>
					<br/>
					The 'httpclose' option removes any 'Connection' header both ways, and
					adds a 'Connection: close' header in each direction. This makes it easier to
					disable HTTP keep-alive than the previous 4-rules block.
				</td>
			</tr>
		<tr>
			<td colspan="2" valign="top" class="listtopic">Connection limits</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Connection timeout</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="connection_timeout" type="text" <?if(isset($pconfig['connection_timeout'])) echo "value=\"{$pconfig['connection_timeout']}\"";?> size="64">
				<div>the time (in milliseconds) we give up if the connection does not complete within (30000).</div>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Server timeout</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="server_timeout" type="text" <?if(isset($pconfig['server_timeout'])) echo "value=\"{$pconfig['server_timeout']}\"";?> size="64">
				<div>the time (in milliseconds) we accept to wait for data from the server, or for the server to accept data (30000).</div>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Retries</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="retries" type="text" <?if(isset($pconfig['retries'])) echo "value=\"{$pconfig['retries']}\"";?> size="6">
				<div>After a connection failure to a server, it is possible to retry, potentially
on another server. This is useful if health-checks are too rare and you don't
want the clients to see the failures. The number of attempts to reconnect is
set by the 'retries' parameter (2).</div>
			</td>
		</tr>
		<tr align="left">
					<td width="22%" valign="top" class="vncell">Max connections</td>
					<td width="78%" class="vtable" colspan="2">
						<input name="max_connections" type="text" <?if(isset($pconfig['max_connections'])) echo "value=\"{$pconfig['max_connections']}\"";?> size="10" maxlength="10">
					</td>
				</tr>
				<tr align="left">
					<td width="22%" valign="top" class="vncell">Client timeout</td>
					<td width="78%" class="vtable" colspan="2">
						<input name="client_timeout" type="text" <?if(isset($pconfig['client_timeout'])) echo "value=\"{$pconfig['client_timeout']}\"";?> size="10" maxlength="10">
						<div>the time (in milliseconds) we accept to wait for data from the client, or for the client to accept data (30000).</div>
					</td>
				</tr>
		<tr>
			<td colspan="2" valign="top" class="listtopic">Backend stats</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Stats Enabled</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_enabled" name="stats_enabled" type="checkbox" value="yes" <?php if ($pconfig['stats_enabled']=='yes') echo "checked"; ?> onclick='toggle_stats();'><br/>
				EXAMPLE: haproxystats
			</td>
		</tr>
		<tr align="left" id='stats_realm_row' name='stats_realm_row' <?if ($pconfig['stats_enabled']!='yes') echo "style=\"display: none;\"";?>>
			<td width="22%" valign="top" class="vncellreq">Stats Realm</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_realm" name="stats_realm" type="text" <?if(isset($pconfig['stats_realm'])) echo "value=\"{$pconfig['stats_realm']}\"";?> size="64">
			</td>
		</tr>
		<tr align="left" id='stats_uri_row' name='stats_uri_row' <?if ($pconfig['stats_enabled']!='yes') echo "style=\"display: none;\"";?>>
			<td width="22%" valign="top" class="vncellreq">Stats Uri</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_uri" name="stats_uri" type="text" <?if(isset($pconfig['stats_uri'])) echo "value=\"{$pconfig['stats_uri']}\"";?> size="64"><br/>
				EXAMPLE: /haproxy?stats
			</td>
		</tr>
		<tr align="left" id='stats_username_row' name='stats_username_row' <?if ($pconfig['stats_enabled']!='yes') echo "style=\"display: none;\"";?>>
			<td width="22%" valign="top" class="vncellreq">Stats Username</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_username" name="stats_username" type="text" <?if(isset($pconfig['stats_username'])) echo "value=\"{$pconfig['stats_username']}\"";?> size="25">
			</td>
		</tr>
		
		<tr align="left" id='stats_password_row' name='stats_password_row' <?if ($pconfig['stats_enabled']!='yes') echo "style=\"display: none;\"";?>>
			<td width="22%" valign="top" class="vncellreq">Stats Password</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_password" name="stats_password" type="password" <?if(isset($pconfig['stats_password'])) echo "value=\"{$pconfig['stats_password']}\"";?> size="25">
				<br/>
			</td>
		</tr>
		<tr align="left" id='stats_node_enabled_row' name='stats_node_enabled_row' <?if ($pconfig['stats_enabled']!='yes') echo "style=\"display: none;\"";?>>
			<td width="22%" valign="top" class="vncell">Stats Enable Node Name</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_node_enabled" name="stats_node_enabled" type="checkbox" value="yes" <?php if ($pconfig['stats_node_enabled']=='yes') echo "checked"; ?>>
				<br/>
			</td>
		</tr>
		<tr align="left" id='stats_node_row' name='stats_node_row' <?if ($pconfig['stats_enabled']!='yes') echo "style=\"display: none;\"";?>>
			<td width="22%" valign="top" class="vncell">Stats Node</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_node" name="stats_node" type="text" <?if(isset($pconfig['stats_node'])) echo "value=\"{$pconfig['stats_node']}\"";?> size="64"><br/>
				The node name is displayed in the stats and helps to differentiate which server in a cluster is actually serving clients.<br/>
				Leave blank to use the system name.
			</td>
		</tr>
		<tr align="left" id='stats_desc_row' name='stats_desc_row' <?if ($pconfig['stats_enabled']!='yes') echo "style=\"display: none;\"";?>>
			<td width="22%" valign="top" class="vncell">Stats Description</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_desc" name="stats_desc" type="text" <?if(isset($pconfig['stats_node'])) echo "value=\"{$pconfig['stats_desc']}\"";?> size="64"><br/>
			</td>
		</tr>
		<tr align="left" id='stats_refresh_row' name='stats_refresh_row' <?if ($pconfig['stats_enabled']!='yes') echo "style=\"display: none;\"";?>>
			<td width="22%" valign="top" class="vncell">Stats Refresh</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_refresh" name="stats_refresh" type="text" <?if(isset($pconfig['stats_refresh'])) echo "value=\"{$pconfig['stats_refresh']}\"";?> size="6" maxlength="30"><br/>
				Specify the refresh rate of the stats page in seconds, or specified time unit (us, ms, s, m, h, d).
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" id="monitorport_text" class="vncell">Monitor Uri</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="monitor_uri" type="text" <?if(isset($pconfig['monitor_uri'])) echo "value=\"{$pconfig['monitor_uri']}\"";?> size="50" maxlength="50">
				<br/>
				Example: / or /index.php or /index.html or /testmypage.cgi
			</td>
		</tr>
				
<?php
/*
				<tr>
					<td width="22%" valign="top" class="vncell">Access Control lists</td>
					<td width="78%" class="vtable" colspan="2" valign="top">
									<table class="" width="100%" cellpadding="0" cellspacing="0" id='acltable'>
		                <tr>
		                  <td width="35%" class="">Name</td>
		                  <td width="40%" class="">Expression</td>
		                  <td width="20%" class="">Backend</td>
		                  <td width="5%" class=""></td>
						</tr>
						<?php 
							$a_acl=$pconfig['a_acl'];

							if (!is_array($a_acl)) {
								$a_acl=array();
							}

							$i=0;
							foreach ($a_acl as $acl) {
						?>
						<tr>
							<td><input name="acl_name<?=$i;?>" type="text" value="<?=$acl['name']; ?>" size="30"/></td>
							<td><input name="acl_expression<?=$i;?>" type="text" value="<?=$acl['expression']; ?>" size="35"/></td>
							<td>
								<select name="acl_backend<?=$i;?>" id="acl_backend<?=$i;?>">
								<?php
									if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
										$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
									}
									$a_backend = &$config['installedpackages']['haproxy']['ha_backends']['item'];
									foreach ($a_backend as $backend) { ?>
										<option value="<?=$backend['name'];?>"<?php if($acl['backend'] == $backend['name']) echo " SELECTED"; ?>><?=$backend['name'];?></option>
								<?php $i++; } ?>
								</select>
							</td>
		                  <td class="list"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0" onclick="removeRow(this); return false;"></td>
						</tr>
						<?php
							$i++;
							}
						?>
					</table>
					<a onclick="javascript:addRowTo('acltable'); return false;" href="#">
						<img border="0" src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" alt="" title="add another entry" />
					</a><br/>
					Fore more information about ACL's please see <a href='http://haproxy.1wt.eu/download/1.3/doc/configuration.txt' target='_new'>HAProxy Documentation</a> Section 7 - Using ACL's
					</td>
			</tr>
*/
?>
		<tr>
			<td colspan="2" valign="top" class="listtopic">Advanced pass thru</td>
		</tr>
			<tr align="left" colspan="2" >
				<td width="100%" class="vtable" colspan="2">
					<textarea name='advanced' rows="6" cols="90" id='advanced'><?php echo $pconfig['advanced']; ?></textarea>
					<br/>
					NOTE: paste text into this box that you would like to pass thru.
				</td>
			</tr>
			<tr align="left">
			<td width="22%" valign="top">&nbsp;</td>
			<td width="78%">
				<input name="Submit" type="submit" class="formbtn" value="Save">  
				<input type="button" class="formbtn" value="Cancel" onclick="history.back()">
				<?php if (isset($id) && $a_backend[$id]): ?>
				<input name="id" type="hidden" value="<?=$id;?>">
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
	</td></tr></table>
	</form>
<br>
<script type="text/javascript">
	field_counter_js = 3;
	rows = 1;
	<?php $counter=0; ?>
	totalrows =  <?php echo $counter; ?>;;
	loaded =  <?php echo $counter; ?>;;
</script>
<?php include("fend.inc"); ?>
</body>
</html>
