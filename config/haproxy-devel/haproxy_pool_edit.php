<?php
/* $Id: load_balancer_pool_edit.php,v 1.24.2.23 2007/03/03 00:07:09 smos Exp $ */
/*
	haproxy_pool_edit.php
	part of pfSense (http://www.pfsense.com/)
	Copyright (C) 2013 PiBa-NL
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
require_once("haproxy.inc");
require_once("haproxy_utils.inc");
require_once("haproxy_htmllist.inc");
require_once("pkg_haproxy_tabs.inc");

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
"agent_check","agent_port","agent_inter",
"connection_timeout","server_timeout","retries",
"stats_enabled","stats_username","stats_password","stats_uri","stats_realm","stats_admin","stats_node_enabled","stats_node","stats_desc","stats_refresh");

$fields_servers=array();
$fields_servers[0]['name']="name";
$fields_servers[0]['columnheader']="Name";
$fields_servers[0]['colwidth']="20%";
$fields_servers[0]['type']="textbox";
$fields_servers[0]['size']="30";
$fields_servers[1]['name']="address";
$fields_servers[1]['columnheader']="Address";
$fields_servers[1]['colwidth']="10%";
$fields_servers[1]['type']="textbox";
$fields_servers[1]['size']="20";
$fields_servers[2]['name']="port";
$fields_servers[2]['columnheader']="Port";
$fields_servers[2]['colwidth']="5%";
$fields_servers[2]['type']="textbox";
$fields_servers[2]['size']="5";
$fields_servers[3]['name']="ssl";
$fields_servers[3]['columnheader']="SSL";
$fields_servers[3]['colwidth']="5%";
$fields_servers[3]['type']="checkbox";
$fields_servers[3]['size']="30";
$fields_servers[4]['name']="weight";
$fields_servers[4]['columnheader']="Weight";
$fields_servers[4]['colwidth']="8%";
$fields_servers[4]['type']="textbox";
$fields_servers[4]['size']="5";
$fields_servers[5]['name']="status";
$fields_servers[5]['columnheader']="Mode";
$fields_servers[5]['colwidth']="5%";
$fields_servers[5]['type']="select";
$fields_servers[5]['size']="5";
$fields_servers[5]['items']=&$a_servermodes;
$fields_servers[6]['name']="advanced";
$fields_servers[6]['columnheader']="Advanced";
$fields_servers[6]['colwidth']="15%";
$fields_servers[6]['type']="textbox";
$fields_servers[6]['size']="20";

if (isset($id) && $a_pools[$id]) {
	$pconfig['advanced'] = base64_decode($a_pools[$id]['advanced']);
	$pconfig['advanced_backend'] = base64_decode($a_pools[$id]['advanced_backend']);
	$pconfig['a_servers']=&$a_pools[$id]['ha_servers']['item'];	
	
	foreach($simplefields as $stat)
		$pconfig[$stat] = $a_pools[$id][$stat];
}

if (isset($_GET['dup']))
	unset($id);

$changedesc = "Services: HAProxy: Backend server pool: ";
$changecount = 0;

if ($_POST) {
	$changecount++;

	unset($input_errors);
	$pconfig = $_POST;
	
	$reqdfields = explode(" ", "name");
	$reqdfieldsn = explode(",", "Name");		
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if ($_POST['stats_enabled']) {
		$reqdfields = explode(" ", "name stats_uri");
		$reqdfieldsn = explode(",", "Name,Stats Uri");		
		do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
		if ($_POST['stats_username']) {
			$reqdfields = explode(" ", "stats_password stats_realm");
			$reqdfieldsn = explode(",", "Stats Password,Stats Realm");		
			do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
		}
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

	if (preg_match("/[^a-zA-Z0-9\-_]/", $_POST['stats_node']))
		$input_errors[] = "The field 'Stats Node' contains invalid characters. Should be a string with digits(0-9), letters(A-Z, a-z), hyphen(-) or underscode(_)";
		
	/* Ensure that our pool names are unique */
	for ($i=0; isset($config['installedpackages']['haproxy']['ha_pools']['item'][$i]); $i++)
		if (($_POST['name'] == $config['installedpackages']['haproxy']['ha_pools']['item'][$i]['name']) && ($i != $id))
			$input_errors[] = "This pool name has already been used.  Pool names must be unique.";

	$a_servers = haproxy_htmllist_get_values($fields_servers);
	foreach($a_servers as $server){
		$server_name    = $server['name'];
		$server_address = $server['address'];
		$server_port    = $server['port'];
		$server_weight  = $server['weight'];

		if (preg_match("/[^a-zA-Z0-9\.\-_]/", $server_name))
			$input_errors[] = "The field 'Name' contains invalid characters.";

		if (!is_ipaddr($server_address) && !is_hostname($server_address))
			$input_errors[] = "The field 'Address' is not a valid ip address or hostname.";

		if (!preg_match("/.{2,}/", $server_name))
			$input_errors[] = "The field 'Name' is required (and must be at least 2 characters).";

		if ($server_weight && !is_numeric($server_weight))
			$input_errors[] = "The field 'Weight' value is not a number.";

		if ($server_port && !is_numeric($server_port))
			$input_errors[] = "The field 'Port' value is not a number.";
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

$closehead = false;
$pgtitle = "HAProxy: Backend server pool: Edit";
include("head.inc");

// 'processing' done, make all simple fields usable in html.
foreach($simplefields as $field){
	$pconfig[$field] = htmlspecialchars($pconfig[$field]);
}

?>
  <style type="text/css">
	.haproxy_stats_visible{display:none;}
	.haproxy_check_enabled{display:none;}
	.haproxy_check_http{display:none;}
	.haproxy_check_username{display:none;}
	.haproxy_check_smtp{display:none;}
	.haproxy_transparent_clientip{display:none;}
	.haproxy_check_agent{display:none;}
	.haproxy_agent_check{display:none;}
  </style>
</head>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<script type="text/javascript">
	function htmllist_get_select_options(tableId) {
		return "<?=haproxy_js_select_options($a_servermodes);?>";
	}
	
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
		
		setCSSdisplay(".haproxy_agent_check", agent_check.checked);

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
<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php endif; ?>
	<form action="haproxy_pool_edit.php" method="post" name="iform" id="iform">
	
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
	  <tr><td class="tabnavtbl">
	  <?php
		haproxy_display_top_tabs_active($haproxy_tab_array['haproxy'], "backend");
	  ?>
	  </td></tr>
  <tr>
    <td>
	<div class="tabcont">
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Edit HAProxy Backend server pool</td>
		</tr>	
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Name</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="name" type="text" <?if(isset($pconfig['name'])) echo "value=\"{$pconfig['name']}\"";?> size="16" maxlength="16" />
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Cookie</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="cookie" type="text" <?if(isset($pconfig['cookie'])) echo "value=\"{$pconfig['cookie']}\"";?>size="64" /><br/>
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
			<?
			$counter=0;
			$a_servers = $pconfig['a_servers'];
			haproxy_htmllist("tableA_servers", $a_servers, $fields_servers);
			?>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Balance</td>
			<td width="78%" class="vtable" colspan="1">
				<table width="100%">
				<tr>
					<td width="25%" valign="top">
						<input type="radio" name="balance" value="roundrobin"<?php if($pconfig['balance'] == "roundrobin") echo " CHECKED"; ?> />Round robin
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
						<input type="radio" name="balance" value="static-rr"<?php if($pconfig['balance'] == "static-rr") echo " CHECKED"; ?> />Static Round Robin
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
						<input type="radio" name="balance" value="leastconn"<?php if($pconfig['balance'] == "leastconn") echo " CHECKED"; ?> />Least Connections
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
				  <tr><td valign="top"><input type="radio" name="balance" value="source"<?php if($pconfig['balance'] == "source") echo " CHECKED"; ?> />Source
				  </td>
				  <td>
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
			<td width="22%" valign="top" class="vncell">Transparent ClientIP</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="transparent_clientip" name="transparent_clientip" type="checkbox" value="yes" <?php if ($pconfig['transparent_clientip']=='yes') echo "checked"; ?> onclick='updatevisibility();' />
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
				<input type="text" name='advanced' id='advanced' value='<?php echo $pconfig['advanced']; ?>' size="64" />
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
		<tr><td>&nbsp;</td></tr>
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
				<input name="checkinter" type="text" <?if(isset($pconfig['checkinter'])) echo "value=\"{$pconfig['checkinter']}\"";?> size="20" /> milliseconds
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
				<input name="monitor_uri" type="text" <?if(isset($pconfig['monitor_uri'])) echo "value=\"{$pconfig['monitor_uri']}\"";?>size="64" />
				<br/>Defaults to / if left blank.
			</td>
		</tr>
		<tr align="left" class="haproxy_check_http">
			<td width="22%" valign="top" class="vncell">Http check version</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="monitor_httpversion" type="text" <?if(isset($pconfig['monitor_httpversion'])) echo "value=\"{$pconfig['monitor_httpversion']}\"";?> size="64" />
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
				<input name="monitor_username" id="monitor_username" type="text" <?if(isset($pconfig['monitor_username'])) echo "value=\"{$pconfig['monitor_username']}\"";?>size="64" onchange="updatevisibility();" onkeyup="updatevisibility();" />
				<br/>
				This is the username which will be used when connecting to MySQL/PostgreSQL server.
				<pre>
USE mysql;
CREATE USER '<span id="sqlcheckusername"></span>'@'&lt;pfSenseIP&gt;';
FLUSH PRIVILEGES;</pre>
			</td>
		</tr>
		<tr align="left" class="haproxy_check_smtp">
			<td width="22%" valign="top" class="vncell">Domain</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="monitor_domain" type="text" <?if(isset($pconfig['monitor_domain'])) echo "value=\"{$pconfig['monitor_domain']}\"";?> size="64" />
			</td>
		</tr>
		<tr align="left" class="haproxy_check_agent">
			<td width="22%" valign="top" class="vncell">Agentport</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="monitor_agentport" type="text" <?if(isset($pconfig['monitor_agentport'])) echo "value=\"{$pconfig['monitor_agentport']}\"";?> size="64" />
				<br/>
				Fill in the TCP portnumber the healthcheck should be performed on.
			</td>
		</tr>
		<tr><td>&nbsp;</td></tr>
		<tr>
			<td colspan="2" valign="top" class="listtopic">Agent checks</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Use agent checks</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="agent_check" name="agent_check" type="checkbox" value="yes" <?php if ($pconfig['agent_check']=='yes') echo "checked"; ?> onclick='updatevisibility();' />
				Use a TCP connection to read an ASCII string of the form 100%,75%,drain,down (more about this in the <a href='http://cbonte.github.io/haproxy-dconv/configuration-1.5.html#agent-check' target='_blank'>haproxy manual</a>)
			</td>
		</tr>
		<tr align="left" class="haproxy_agent_check">
			<td width="22%" valign="top" class="vncell">Agent port</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="agent_port" type="text" <?if(isset($pconfig['agent_port'])) echo "value=\"{$pconfig['agent_port']}\"";?> size="64" />
				<br/>
				Fill in the TCP portnumber the healthcheck should be performed on.
			</td>
		</tr>
		<tr align="left" class="haproxy_agent_check">
			<td width="22%" valign="top" class="vncell">Agent interval</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="agent_inter" type="text" <?if(isset($pconfig['agent_inter'])) echo "value=\"{$pconfig['agent_inter']}\"";?> size="64" />
				<br/>
				Interval between two agent checks, defaults to 2000 ms.
			</td>
		</tr>
		<tr><td>&nbsp;</td></tr>
		<tr>
			<td colspan="2" valign="top" class="listtopic">Advanced settings</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Connection timeout</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="connection_timeout" type="text" <?if(isset($pconfig['connection_timeout'])) echo "value=\"{$pconfig['connection_timeout']}\"";?> size="20" />
				<div>the time (in milliseconds) we give up if the connection does not complete within (default 30000).</div>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Server timeout</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="server_timeout" type="text" <?if(isset($pconfig['server_timeout'])) echo "value=\"{$pconfig['server_timeout']}\"";?> size="20" />
				<div>the time (in milliseconds) we accept to wait for data from the server, or for the server to accept data (default 30000).</div>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Retries</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="retries" type="text" <?if(isset($pconfig['retries'])) echo "value=\"{$pconfig['retries']}\"";?> size="20" />
				<div>After a connection failure to a server, it is possible to retry, potentially
on another server. This is useful if health-checks are too rare and you don't
want the clients to see the failures. The number of attempts to reconnect is
set by the 'retries' parameter.</div>
			</td>
		</tr>
		<tr><td>&nbsp;</td></tr>
		<tr>
			<td colspan="2" valign="top" class="listtopic">Statistics</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Stats Enabled</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_enabled" name="stats_enabled" type="checkbox" value="yes" <?php if ($pconfig['stats_enabled']=='yes') echo "checked"; ?> onclick='updatevisibility();' />
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_uri_row'>
			<td width="22%" valign="top" class="vncellreq">Stats Uri</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_uri" name="stats_uri" type="text" <?if(isset($pconfig['stats_uri'])) echo "value=\"{$pconfig['stats_uri']}\"";?> size="64" /><br/>
				This url can be used when this same backend is used for passing connections to backends<br/>
				EXAMPLE: / or /haproxy?stats
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_scope_row'>
			<td width="22%" valign="top" class="vncell">Stats Scope</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_scope" name="stats_scope" type="text" <?if(isset($pconfig['stats_scope'])) echo "value=\"{$pconfig['stats_scope']}\"";?> size="64" /><br/>
				Determines which frontends and backends are shown, leave empty to show all.<br/>
				EXAMPLE: frontendA,backend1,backend2
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_realm_row'>
			<td width="22%" valign="top" class="vncell">Stats Realm</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_realm" name="stats_realm" type="text" <?if(isset($pconfig['stats_realm'])) echo "value=\"{$pconfig['stats_realm']}\"";?> size="64" /><br/>
				The realm is shown when authentication is requested by haproxy.<br/>
				EXAMPLE: haproxystats
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_username_row'>
			<td width="22%" valign="top" class="vncell">Stats Username</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_username" name="stats_username" type="text" <?if(isset($pconfig['stats_username'])) echo "value=\"".$pconfig['stats_username']."\"";?> size="64" />
				EXAMPLE: admin
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_password_row'>
			<td width="22%" valign="top" class="vncell">Stats Password</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_password" name="stats_password" type="password" <?
					if(isset($pconfig['stats_password'])) 
						echo "value=\"".$pconfig['stats_password']."\"";
					?> size="64" />
				EXAMPLE: 1Your2Secret3P@ssword
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_node_admin_row'>
			<td width="22%" valign="top" class="vncell">Stats Admin</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_admin" name="stats_admin" type="checkbox" value="yes" <?php if ($pconfig['stats_admin']=='yes') echo "checked"; ?> />
				Makes available the options disable/enable/softstop/softstart/killsessions from the stats page.<br/>
				Note: This is not persisted when haproxy restarts. For publicly visible stats pages this should be disabled.
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_node_row'>
			<td width="22%" valign="top" class="vncell">Stats Nodename</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_node" name="stats_node" type="text" <?if(isset($pconfig['stats_node'])) echo "value=\"{$pconfig['stats_node']}\"";?> size="64" /><br/>
				The short name is displayed in the stats and helps to differentiate which server in a cluster is actually serving clients.
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_desc_row'>
			<td width="22%" valign="top" class="vncell">Stats Description</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_desc" name="stats_desc" type="text" <?if(isset($pconfig['stats_desc'])) echo "value=\"{$pconfig['stats_desc']}\"";?> size="64" /><br/><br/>
				The description is displayed behind the Nodename set above.
			</td>
		</tr>
		<tr class="haproxy_stats_visible" align="left" id='stats_refresh_row'>
			<td width="22%" valign="top" class="vncell">Stats Refresh</td>
			<td width="78%" class="vtable" colspan="2">
				<input id="stats_refresh" name="stats_refresh" type="text" <?if(isset($pconfig['stats_refresh'])) echo "value=\"{$pconfig['stats_refresh']}\"";?> size="10" maxlength="30" /><br/>
				Specify the refresh rate of the stats page in seconds, or specified time unit (us, ms, s, m, h, d).
			</td>
		</tr>
		<tr><td>&nbsp;</td></tr>
		<tr align="left">
			<td width="22%" valign="top">&nbsp;</td>
			<td width="78%">
				<input name="Submit" type="submit" class="formbtn" value="Save" />
				<input type="button" class="formbtn" value="Cancel" onclick="history.back()" />
				<?php if (isset($id) && $a_pools[$id]): ?>
				<input name="id" type="hidden" value="<?=$id;?>" />
				<?php endif; ?>
			</td>
		</tr>
	</table>
	</div>
	</td></tr></table>
	</form>
<br/>
<script type="text/javascript">
<?
	phparray_to_javascriptarray($fields_servers,"fields_servers",Array('/*','/*/name','/*/type','/*/size','/*/items','/*/items/*','/*/items/*/*','/*/items/*/*/name'));
	phparray_to_javascriptarray($a_checktypes,"checktypes",Array('/*','/*/name','/*/descr'));
?>
	browser_InnerText_support = (document.getElementsByTagName("body")[0].innerText != undefined) ? true : false;

	totalrows =  <?php echo $counter; ?>;
	updatevisibility();
</script>
<?php
haproxy_htmllist_js();
include("fend.inc"); ?>
</body>
</html>
