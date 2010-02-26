<?php 
/*
	vpn_openvpn_export.php

	Copyright (C) 2008 Shrew Soft Inc.
	Copyright (C) 2010 Ermal Luçi
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

	DISABLE_PHP_LINT_CHECKING
*/

require("globals.inc");
require("guiconfig.inc");
require("openvpn-client-export.inc");

$pgtitle = array("OpenVPN", "Client Export Utility");

if (!is_array($config['openvpn']['openvpn-server']))
    $config['openvpn']['openvpn-server'] = array();

$a_server = $config['openvpn']['openvpn-server'];

if (!is_array($config['system']['user']))
    $config['system']['user'] = array();

$a_user = $config['system']['user'];

$ras_server = array();
foreach($a_server as $sindex => & $server) {
	if (isset($server['disable']))
		continue;
	$ras_user = array();
	if (stripos($server['mode'], "server") === false)
		continue;
	foreach($a_user as $uindex => & $user) {
		if (!is_array($user['cert']))
			continue;
		foreach($user['cert'] as $cindex => & $cert) {
			if ($cert['caref'] != $server['caref'])
				continue;
			$ras_userent = array();
			$ras_userent['uindex'] = $uindex;
			$ras_userent['cindex'] = $cindex;
			$ras_userent['name'] = $user['name'];
			$ras_userent['certname'] = $cert['name'];
			$ras_user[] = $ras_userent;
		}
	}
	if (!count($ras_user))
		continue;
	$ras_serverent = array();
	$prot = $server['protocol'];
	$port = $server['local_port'];
	if ($server['description'])
		$name = "{$server['description']} {$prot}:{$port}";
	else
		$name = "Server {$prot}:{$port}";
	$ras_serverent['index'] = $sindex;
	$ras_serverent['name'] = $name;
	$ras_serverent['users'] = $ras_user;
	$ras_server[] = $ras_serverent;
}

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

$act = $_GET['act'];
if (isset($_POST['act']))
	$act = $_POST['act'];

$error = false;
if($act == "conf") {
	$srvid = $_GET['srvid'];
	$usrid = $_GET['usrid'];
	$crtid = $_GET['crtid'];
	if (($srvid === false) || ($usrid === false) || ($crtid === false)) {
		pfSenseHeader("vpn_openvpn_export.php");
		exit;
	}
	if (empty($_GET['useaddr'])) {
		$error = true;
		$input_errors[] = "You need to specify an IP or hostname.";
	} else
		$useaddr = $_GET['useaddr'];

	$usetoken = $_GET['usetoken'];

	$proxy = "";
	if (!empty($_GET['proxy_addr']) || !empty($_GET['proxy_port'])) {
		$proxy = array();
		if (empty($_GET['proxy_addr'])) {
			$error = true;
			$input_errors[] = "You need to specify an address for the proxy port.";
		} else
			$proxy['ip'] = $_GET['proxy_addr'];
		if (empty($_GET['proxy_port'])) {
			$error = true;
			$input_errors[] = "You need to specify a port for the proxy ip.";
		} else
			$proxy['port'] = $_GET['proxy_port'];
		$proxy['proxy_authtype'] = $_GET['proxy_authtype'];
		if ($_GET['proxy_authtype'] != "none") {
			if (empty($_GET['proxy_user'])) {
				$error = true;
				$input_errors[] = "You need to specify a username with the proxy config.";
			} else
				$proxy['user'] = $_GET['proxy_user'];
			if (!empty($_GET['proxy_user']) && empty($_GET['proxy_password'])) {
				$error = true;
				$input_errors[] = "You need to specify a password with the proxy user.";
			} else
				$proxy['password'] = $_GET['proxy_password'];
		}
	}

	$exp_name = openvpn_client_export_prefix($srvid);
	$exp_data = openvpn_client_export_config($srvid, $usrid, $crtid, $useaddr, $usetoken, false, $proxy);
	if (!$exp_data) {
		$input_errors[] = "Failed to export config files!";
		$error = true;
	}
	if (!$error) {
		$exp_name = urlencode($exp_name."-config.ovpn");
		$exp_size = strlen($exp_data);

		header("Content-Type: application/octet-stream");
		header("Content-Disposition: attachment; filename={$exp_name}");
		header("Content-Length: $exp_size");
		echo $exp_data;

		exit;
	}
}

if($act == "visc") {
	$srvid = $_GET['srvid'];
	$usrid = $_GET['usrid'];
	$crtid = $_GET['crtid'];
	if (($srvid === false) || ($usrid === false) || ($crtid === false)) {
		pfSenseHeader("vpn_openvpn_export.php");
		exit;
	}
	if (empty($_GET['useaddr'])) {
		$error = true;
		$input_errors[] = "You need to specify an IP or hostname.";
        } else
                $useaddr = $_GET['useaddr'];

	$usetoken = $_GET['usetoken'];
	$password = "";
	if ($_GET['password'])
		$password = $_GET['password']; ;

	$proxy = "";
        if (!empty($_GET['proxy_addr']) || !empty($_GET['proxy_port'])) {
                $proxy = array();
                if (empty($_GET['proxy_addr'])) {
                        $error = true;
                        $input_errors[] = "You need to specify an address for the proxy port.";
                } else
                        $proxy['ip'] = $_GET['proxy_addr'];
                if (empty($_GET['proxy_port'])) {
                        $error = true;
                        $input_errors[] = "You need to specify a port for the proxy ip.";
                } else
                        $proxy['port'] = $_GET['proxy_port'];
                $proxy['proxy_authtype'] = $_GET['proxy_authtype'];
                if ($_GET['proxy_authtype'] != "none") {
                        if (empty($_GET['proxy_user'])) {
                                $error = true;
                                $input_errors[] = "You need to specify a username with the proxy config.";
                        } else
                                $proxy['user'] = $_GET['proxy_user'];
                        if (!empty($_GET['proxy_user']) && empty($_GET['proxy_password'])) {
                                $error = true;
                                $input_errors[] = "You need to specify a password with the proxy user.";
                        } else
                                $proxy['password'] = $_GET['proxy_password'];
                }
        }

	$exp_name = openvpn_client_export_prefix($srvid);
	$exp_name = urlencode($exp_name."-Viscosity.visc.zip");
	$exp_path = viscosity_openvpn_client_config_exporter($srvid, $usrid, $crtid, $useaddr, $usetoken, $password, $proxy);
	if (!$exp_path) {
		$input_errors[] = "Failed to export config files!";
		$error = true;
	}
	if (!$error) {
		$exp_size = filesize($exp_path);

		header("Content-Type: application/octet-stream");
		header("Content-Disposition: attachment; filename={$exp_name}");
		header("Content-Length: $exp_size");
		readfile($exp_path);
		//unlink($exp_path);
		exit;
	}
}

if($act == "inst") {
	$srvid = $_GET['srvid'];
	$usrid = $_GET['usrid'];
	$crtid = $_GET['crtid'];
	if (($srvid === false) || ($usrid === false) || ($crtid === false)) {
		pfSenseHeader("vpn_openvpn_export.php");
		exit;
	}
	if (empty($_GET['useaddr'])) {
		$error = true;
		$input_errors[] = "You need to specify an IP or hostname.";
        } else
                $useaddr = $_GET['useaddr'];

	$usetoken = $_GET['usetoken'];
	$password = "";
	if ($_GET['password'])
		$password = $_GET['password'];

	$proxy = "";
        if (!empty($_GET['proxy_addr']) || !empty($_GET['proxy_port'])) {
                $proxy = array();
                if (empty($_GET['proxy_addr'])) {
                        $error = true;
                        $input_errors[] = "You need to specify an address for the proxy port.";
                } else
                        $proxy['ip'] = $_GET['proxy_addr'];
                if (empty($_GET['proxy_port'])) {
                        $error = true;
                        $input_errors[] = "You need to specify a port for the proxy ip.";
                } else
                        $proxy['port'] = $_GET['proxy_port'];
                $proxy['proxy_authtype'] = $_GET['proxy_authtype'];
                if ($_GET['proxy_authtype'] != "none") {
                        if (empty($_GET['proxy_user'])) {
                                $error = true;
                                $input_errors[] = "You need to specify a username with the proxy config.";
                        } else
                                $proxy['user'] = $_GET['proxy_user'];
                        if (!empty($_GET['proxy_user']) && empty($_GET['proxy_password'])) {
                                $error = true;
                                $input_errors[] = "You need to specify a password with the proxy user.";
                        } else
                                $proxy['password'] = $_GET['proxy_password'];
                }
        }

	$exp_name = openvpn_client_export_prefix($srvid);
	$exp_name = urlencode($exp_name."-install.exe");
	$exp_path = openvpn_client_export_installer($srvid, $usrid, $crtid, $useaddr, $usetoken, $password, $proxy);
	if (!$exp_path) {
		$input_errors[] = "Failed to export config files!";
		$error = true;
	}
	if (!$error) {
		$exp_size = filesize($exp_path);

		header("Content-Type: application/octet-stream");
		header("Content-Disposition: attachment; filename={$exp_name}");
		header("Content-Length: $exp_size");
		readfile($exp_path);
		unlink($exp_path);
		exit;
	}
}

include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<script language="JavaScript">
	var viscosityAvailable = false;
<!--

var servers = new Array();
<?php foreach ($ras_server as $sindex => & $server): ?>
servers[<?=$sindex;?>] = new Array();
servers[<?=$sindex;?>][0] = '<?=$server['index'];?>';
servers[<?=$sindex;?>][1] = new Array();
<?php 	foreach ($server['users'] as $uindex => & $user): ?>
servers[<?=$sindex;?>][1][<?=$uindex;?>] = new Array();
servers[<?=$sindex;?>][1][<?=$uindex;?>][0] = '<?=$user['uindex'];?>';
servers[<?=$sindex;?>][1][<?=$uindex;?>][1] = '<?=$user['cindex'];?>';
servers[<?=$sindex;?>][1][<?=$uindex;?>][2] = '<?=$user['name'];?>';
servers[<?=$sindex;?>][1][<?=$uindex;?>][3] = '<?=str_replace("'", "\\'", $user['certname']);?>';
<?		endforeach; ?>
<?	endforeach; ?>

function download_begin(act, i) {

	var index = document.getElementById("server").selectedIndex;
	var users = servers[index][1];
	var useaddr;

	if (document.getElementById("useaddr").value == "other") {
		if (document.getElementById("useaddr_hostname").value == "") {
			alert("Please specify an IP address or hostname.");
			return;
		}
		useaddr = document.getElementById("useaddr_hostname").value;
	} else
		useaddr = document.getElementById("useaddr").value;

	var usetoken = 0;
	if (document.getElementById("usetoken").checked)
		usetoken = 1;
	var usepass = 0;
	if (document.getElementById("usepass").checked)
		usepass = 1;

	var pass = document.getElementById("pass").value;
	var conf = document.getElementById("conf").value;
	if (usepass && (act == "inst")) {
		if (!pass || !conf) {
			alert("The password or confirm field is empty");
			return;
		}
		if (pass != conf) {
			alert("The password and confirm fields must match");
			return;
		}
	}

	var useproxy = 0;
	var useproxypass = 0;
        if (document.getElementById("useproxy").checked)
                useproxy = 1;

        var proxyaddr = document.getElementById("proxyaddr").value;
        var proxyport = document.getElementById("proxyport").value;
        if (useproxy) {
                if (!proxyaddr || !proxyport) {
                        alert("The proxy ip and port cannot be empty");
                        return;
                }

        	if (document.getElementById("useproxypass").value != 'none')
                	useproxypass = 1;

		var proxyauth = document.getElementById("useproxypass").value;
        	var proxyuser = document.getElementById("proxyuser").value;
        	var proxypass = document.getElementById("proxypass").value;
        	var proxyconf = document.getElementById("proxyconf").value;
        	if (useproxypass) {
			if (!proxyuser) {
				alert("Please fill the proxy username and passowrd.");
				return;
			}
                	if (!proxypass || !proxyconf) {
                        	alert("The proxy password or confirm field is empty");
                       		return;
                	}
                	if (proxypass != proxyconf) {
                        	alert("The proxy password and confirm fields must match");
                        	return;
                	}
        	}
        }

	var dlurl;
	dlurl  = "/vpn_openvpn_export.php?act=" + act;
	dlurl += "&srvid=" + servers[index][0];
	dlurl += "&usrid=" + users[i][0];
	dlurl += "&crtid=" + users[i][1];
	dlurl += "&useaddr=" + useaddr;
	dlurl += "&usetoken=" + usetoken;
	if (usepass)
		dlurl += "&password=" + pass;
	if (useproxy) {
		dlurl += "&proxy_addr=" + proxyaddr;
		dlurl += "&proxy_port=" + proxyport;
               	dlurl += "&proxy_authtype=" + proxyauth;
		if (useproxypass) {
                	dlurl += "&proxy_user=" + proxyuser;
                	dlurl += "&proxy_password=" + proxypass;
		}
	}

	window.open(dlurl,"_self");
}

function server_changed() {

	var table = document.getElementById("users");
	while (table.rows.length > 1 )
		table.deleteRow(1);

	var index = document.getElementById("server").selectedIndex;
	var users = servers[index][1];
	for (i=0; i < users.length; i++) {
		var row = table.insertRow(table.rows.length);
		var cell0 = row.insertCell(0);
		var cell1 = row.insertCell(1);
		var cell2 = row.insertCell(2);
		cell0.className = "listlr";
		cell0.innerHTML = users[i][2];
		cell1.className = "listr";
		cell1.innerHTML = users[i][3];
		cell2.className = "listr";
		cell2.innerHTML  = "<a href='javascript:download_begin(\"conf\"," + i + ")'>Configuration</a>";
		cell2.innerHTML += "&nbsp;/&nbsp;";
		cell2.innerHTML += "<a href='javascript:download_begin(\"inst\"," + i + ")'>Windows Installer</a>";
		cell2.innerHTML += "&nbsp;/&nbsp;";
		cell2.innerHTML += "<a href='javascript:download_begin(\"visc\"," + i + ")'>Viscosity Bundle</a>";
	}
}

function useaddr_changed(obj) {

	if (obj.value == "other")
		$('HostName').show();
	else
		$('HostName').hide();
	
}

function usepass_changed() {

	if (document.getElementById("usepass").checked)
		document.getElementById("usepass_opts").style.display = "";
	else
		document.getElementById("usepass_opts").style.display = "none";
}

function useproxy_changed(obj) {

	if ((obj.id == "useproxy" && obj.checked) ||
		$(obj.id).value != 'none') {
		$(obj.id + '_opts').show();
	} else {
		$(obj.id + '_opts').hide();
	}
}
//-->
</script>
<?php
	if ($input_errors)
		print_input_errors($input_errors);
	if ($savemsg)
		print_info_box($savemsg);
?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
 	<tr>
		<td>
			<?php 
				$tab_array = array();
				$tab_array[] = array(gettext("Server"), false, "vpn_openvpn_server.php");
				$tab_array[] = array(gettext("Client"), false, "vpn_openvpn_client.php");
				$tab_array[] = array(gettext("Client Specific Overrides"), false, "vpn_openvpn_csc.php");
				$tab_array[] = array(gettext("Client Export"), true, "vpn_openvpn_export.php");
				display_top_tabs($tab_array);
			?>
		</td>
	</tr>
	<tr>
		<td id="mainarea">
			<div class="tabcont">
				<table width="100%" border="0" cellpadding="6" cellspacing="0">
					<tr>
						<td width="22%" valign="top" class="vncellreq">Remote Access Server</td>
						<td width="78%" class="vtable">
							<select name="server" id="server" class="formselect" onChange="server_changed()">
								<?php foreach($ras_server as & $server): ?>
								<option value="<?=$server['sindex'];?>"><?=$server['name'];?></option>
								<?php endforeach; ?>
							</select>
						</td>
					</tr>
					<tr>
						<td width="22%" valign="top" class="vncell">Host Name Resolution</td>
						<td width="78%" class="vtable">
							<table border="0" cellpadding="2" cellspacing="0">
								<tr>
									<td>
										<select name="useaddr" id="useaddr" class="formselect" onChange="useaddr_changed(this)">
											<option value="serveraddr" >Interface IP Address</option>
											<option value="serverhostname" >Installation hostname</option>
											<option value="other">Other</option>
										</select>
										<br />
										<div style="display:none;" name="HostName" id="HostName">
											<input name="useaddr_hostname" id="useaddr_hostname" />
											<span class="vexpl">
												Enter the hostname or ip address desired to be used for the config.
											</span>
										</div>
									</td>
								</tr>
							</table>
						</td>
					</tr>
					<tr>
						<td width="22%" valign="top" class="vncell">Certificate Export Options</td>
						<td width="78%" class="vtable">
							<table border="0" cellpadding="2" cellspacing="0">
								<tr>
									<td>
										<input name="usetoken" id="usetoken" type="checkbox" value="yes">
									</td>
									<td>
										<span class="vexpl">
											 Use Microsoft Certificate Storage instead of local files.
										</span>
									</td>
								</tr>
							</table>
							<table border="0" cellpadding="2" cellspacing="0">
								<tr>
									<td>
										<input name="usepass" id="usepass" type="checkbox" value="yes" onClick="usepass_changed()">
									</td>
									<td>
										<span class="vexpl">
											Use a password to protect the pkcs12 file contents.
										</span>
									</td>
								</tr>
							</table>
							<table border="0" cellpadding="2" cellspacing="0" id="usepass_opts" style="display:none">
								<tr>
									<td align="right">
										<span class="vexpl">
											 &nbsp;Password :&nbsp;
										</span>
									</td>
									<td>
										<input name="pass" id="pass" type="password" class="formfld pwd" size="20" value="" />
									</td>
								</tr>
								<tr>
									<td align="right">
										<span class="vexpl">
											 &nbsp;Confirm :&nbsp;
										</span>
									</td>
									<td>
										<input name="conf" id="conf" type="password" class="formfld pwd" size="20" value="" />
									</td>
								</tr>
							</table>
						</td>
					</tr>
					<tr>
                                                <td width="22%" valign="top" class="vncell">Use HTTP Proxy</td>
                                                <td width="78%" class="vtable">
							 <table border="0" cellpadding="2" cellspacing="0">
                                                                <tr>
                                                                        <td>
                                                                                <input name="useproxy" id="useproxy" type="checkbox" value="yes" onClick="useproxy_changed(this)">

                                                                        </td>
                                                                        <td>
                                                                                <span class="vexpl">
                                                                                        Use HTTP proxy to communicate with the server.
                                                                                </span>
                                                                        </td>
                                                                </tr>
                                                        </table>
                                                        <table border="0" cellpadding="2" cellspacing="0" id="useproxy_opts" style="display:none">
                                                                <tr>
                                                                        <td align="right" width='25%'>
                                                                                <span class="vexpl">
                                                                                         &nbsp;     Ip Addr :&nbsp;
                                                                                </span>
                                                                        </td>
                                                                        <td>
                                                                                <input name="proxyaddr" id="proxyaddr" class="formfld unknown" size="20" value="" />
                                                                        </td>
                                                                </tr>
                                                                <tr>
                                                                        <td align="right" width='25%'>
                                                                                <span class="vexpl">
                                                                                         &nbsp;      Port :&nbsp;
                                                                                </span>
                                                                                                                <td>
                                                                                <input name="proxyport" id="proxyport" class="formfld unknown" size="5" value="" />
                                                                        </td>
                                                                </tr>
							<br />
                                                                <tr>
                                                                        <td width="25%">

                                                                        </td>
									<td>
										<select name="useproxypass" id="useproxypass" class="formselect" onChange="useproxy_changed(this)">
											<option value="none">none</option>
											<option value="basic">basic</option>
											<option value="ntlm">ntlm</option>
										</select>
                                                                                <span class="vexpl">
                                                                                        Choose HTTP proxy authentication if any.
                                                                                </span>
							<br />
                                                        <table border="0" cellpadding="2" cellspacing="0" id="useproxypass_opts" style="display:none">
								<tr>
                                                                        <td align="right" width="25%">
                                                                                <span class="vexpl">
                                                                                         &nbsp;Username :&nbsp;
                                                                                </span>
                                                                        </td>
                                                                        <td>
                                                                                <input name="proxyuser" id="proxyuser" class="formfld unknown" size="20" value="" />
                                                                        </td>
                                                                </tr>
                                                                <tr>
                                                                        <td align="right" width="25%">
                                                                                <span class="vexpl">
                                                                                         &nbsp;Password :&nbsp;
                                                                                </span>
                                                                        </td>
                                                                        <td>
                                                                                <input name="proxypass" id="proxypass" type="password" class="formfld pwd" size="20" value="" />
                                                                        </td>
                                                                </tr>
                                                                <tr>
                                                                        <td align="right" width="25%">
                                                                                <span class="vexpl">
                                                                                         &nbsp;Confirm :&nbsp;
                                                                                </span>
					                                                                        <td>
                                                                                <input name="proxyconf" id="proxyconf" type="password" class="formfld pwd" size="20" value="" />
                                                                        </td>
                                                                </tr>
                                                        </table>
                                                                        </td>
                                                                </tr>
							</table>
                                                </td>
                                        </tr>
					<tr>
						<td colspan="2" class="list" height="12">&nbsp;</td>
					</tr>
					<tr>
						<td colspan="2" valign="top" class="listtopic">Client Install Packages</td>
					</tr>
				</table>
				<table width="100%" id="users" width="100%" border="0" cellpadding="0" cellspacing="0">
					<tr>
						<td width="25%" class="listhdrr"><?=gettext("User");?></td>
						<td width="50%" class="listhdrr"><?=gettext("Certificate Name");?></td>
						<td width="25%" class="listhdrr"><?=gettext("Export");?></td>
					</tr>
				</table>
			</div>
		</td>
	</tr>
</table>
<script language="JavaScript">
<!--
server_changed();
//-->
</script>
</body>
<?php include("fend.inc"); ?>
