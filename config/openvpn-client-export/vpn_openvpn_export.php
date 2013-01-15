<?php
/*
	vpn_openvpn_export.php

	Copyright (C) 2008 Shrew Soft Inc.
	Copyright (C) 2010 Ermal Lu�i
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

if (!is_array($config['cert']))
	$config['cert'] = array();

$a_cert = $config['cert'];

$ras_server = array();
foreach($a_server as $sindex => $server) {
	if (isset($server['disable']))
		continue;
	$ras_user = array();
	$ras_certs = array();
	if (stripos($server['mode'], "server") === false)
		continue;
	if (($server['mode'] == "server_tls_user") && ($server['authmode'] == "Local Database")) {
		foreach($a_user as $uindex => $user) {
			if (!is_array($user['cert']))
				continue;
			foreach($user['cert'] as $cindex => $cert) {
				// If $cert is not an array, it's a certref not a cert.
				if (!is_array($cert))
					$cert = lookup_cert($cert);

				if ($cert['caref'] != $server['caref'])
					continue;
				$ras_userent = array();
				$ras_userent['uindex'] = $uindex;
				$ras_userent['cindex'] = $cindex;
				$ras_userent['name'] = $user['name'];
				$ras_userent['certname'] = $cert['descr'];
				$ras_user[] = $ras_userent;
			}
		}
	} elseif (($server['mode'] == "server_tls") || (($server['mode'] == "server_tls_user") && ($server['authmode'] != "Local Database"))) {
		foreach($a_cert as $cindex => $cert) {
			if (($cert['caref'] != $server['caref']) || ($cert['refid'] == $server['certref']))
				continue;
			$ras_cert_entry['cindex'] = $cindex;
			$ras_cert_entry['certname'] = $cert['descr'];
			$ras_cert_entry['certref'] = $cert['refid'];
			$ras_certs[] = $ras_cert_entry;
		}
	}

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
	$ras_serverent['certs'] = $ras_certs;
	$ras_serverent['mode'] = $server['mode'];
	$ras_server[] = $ras_serverent;
}

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

$act = $_GET['act'];
if (isset($_POST['act']))
	$act = $_POST['act'];

if (!empty($act)) {

	$srvid = $_GET['srvid'];
	$usrid = $_GET['usrid'];
	$crtid = $_GET['crtid'];
	if ($srvid === false) {
		pfSenseHeader("vpn_openvpn_export.php");
		exit;
	} else if (($config['openvpn']['openvpn-server'][$srvid]['mode'] != "server_user") &&
		(($usrid === false) || ($crtid === false))) {
		pfSenseHeader("vpn_openvpn_export.php");
		exit;
	}

	if ($config['openvpn']['openvpn-server'][$srvid]['mode'] == "server_user")
		$nokeys = true;
	else
		$nokeys = false;

	if (empty($_GET['useaddr'])) {
		$input_errors[] = "You need to specify an IP or hostname.";
	} else
		$useaddr = $_GET['useaddr'];
	$advancedoptions = $_GET['advancedoptions'];
	$openvpnmanager = $_GET['openvpnmanager'];

	$quoteservercn = $_GET['quoteservercn'];
	$usetoken = $_GET['usetoken'];
	if ($usetoken && ($act == "confinline"))
		$input_errors[] = "You cannot use Microsoft Certificate Storage with an Inline configuration.";
	if ($usetoken && (($act == "conf_yealink_t28") || ($act == "conf_yealink_t38g") || ($act == "conf_yealink_t38g2") || ($act == "conf_snom")))
		$input_errors[] = "You cannot use Microsoft Certificate Storage with a Yealink or SNOM configuration.";
	$password = "";
	if ($_GET['password'])
		$password = $_GET['password'];

	$proxy = "";
	if (!empty($_GET['proxy_addr']) || !empty($_GET['proxy_port'])) {
		$proxy = array();
		if (empty($_GET['proxy_addr'])) {
			$input_errors[] = "You need to specify an address for the proxy port.";
		} else
			$proxy['ip'] = $_GET['proxy_addr'];
		if (empty($_GET['proxy_port'])) {
			$input_errors[] = "You need to specify a port for the proxy ip.";
		} else
			$proxy['port'] = $_GET['proxy_port'];
		$proxy['proxy_authtype'] = $_GET['proxy_authtype'];
		if ($_GET['proxy_authtype'] != "none") {
			if (empty($_GET['proxy_user'])) {
				$input_errors[] = "You need to specify a username with the proxy config.";
			} else
				$proxy['user'] = $_GET['proxy_user'];
			if (!empty($_GET['proxy_user']) && empty($_GET['proxy_password'])) {
				$input_errors[] = "You need to specify a password with the proxy user.";
			} else
				$proxy['password'] = $_GET['proxy_password'];
		}
	}

	$exp_name = openvpn_client_export_prefix($srvid, $usrid, $crtid);

	if(substr($act, 0, 4) == "conf") {
		switch ($act) {
			case "confzip":
				$exp_name = urlencode($exp_name."-config.zip");
				$expformat = "zip";
				break;
			case "conf_yealink_t28":
				$exp_name = urlencode("client.tar");
				$expformat = "yealink_t28";
				break;
			case "conf_yealink_t38g":
				$exp_name = urlencode("client.tar");
				$expformat = "yealink_t38g";
				break;
			case "conf_yealink_t38g2":
				$exp_name = urlencode("client.tar");
				$expformat = "yealink_t38g2";
				break;
			case "conf_snom":
				$exp_name = urlencode("vpnclient.tar");
				$expformat = "snom";
				break;
			case "confinline":
				$exp_name = urlencode($exp_name."-config.ovpn");
				$expformat = "inline";
				break;
			default:
				$exp_name = urlencode($exp_name."-config.ovpn");
				$expformat = "baseconf";
		}
		$exp_path = openvpn_client_export_config($srvid, $usrid, $crtid, $useaddr, $quoteservercn, $usetoken, $nokeys, $proxy, $expformat, $password, false, false, $openvpnmanager, $advancedoptions);
	}

	if($act == "visc") {
		$exp_name = urlencode($exp_name."-Viscosity.visc.zip");
		$exp_path = viscosity_openvpn_client_config_exporter($srvid, $usrid, $crtid, $useaddr, $quoteservercn, $usetoken, $password, $proxy, $openvpnmanager, $advancedoptions);
	}

	if(substr($act, 0, 4) == "inst") {
		$exp_name = urlencode($exp_name."-install.exe");
		$exp_path = openvpn_client_export_installer($srvid, $usrid, $crtid, $useaddr, $quoteservercn, $usetoken, $password, $proxy, $openvpnmanager, $advancedoptions, substr($act, 5));
	}

	if (!$exp_path) {
		$input_errors[] = "Failed to export config files!";
	}

	if (empty($input_errors)) {
		if (($act == "conf") || ($act == "confinline")) {
			$exp_size = strlen($exp_path);
		} else {
			$exp_size = filesize($exp_path);
		}
		header('Pragma: ');
		header('Cache-Control: ');
		header("Content-Type: application/octet-stream");
		header("Content-Disposition: attachment; filename={$exp_name}");
		header("Content-Length: $exp_size");
		if (($act == "conf") || ($act == "confinline")) {
			echo $exp_path;
		} else {
			readfile($exp_path);
			@unlink($exp_path);
		}
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
<?php foreach ($ras_server as $sindex => $server): ?>
servers[<?=$sindex;?>] = new Array();
servers[<?=$sindex;?>][0] = '<?=$server['index'];?>';
servers[<?=$sindex;?>][1] = new Array();
servers[<?=$sindex;?>][2] = '<?=$server['mode'];?>';
servers[<?=$sindex;?>][3] = new Array();
<?php		foreach ($server['users'] as $uindex => $user): ?>
servers[<?=$sindex;?>][1][<?=$uindex;?>] = new Array();
servers[<?=$sindex;?>][1][<?=$uindex;?>][0] = '<?=$user['uindex'];?>';
servers[<?=$sindex;?>][1][<?=$uindex;?>][1] = '<?=$user['cindex'];?>';
servers[<?=$sindex;?>][1][<?=$uindex;?>][2] = '<?=$user['name'];?>';
servers[<?=$sindex;?>][1][<?=$uindex;?>][3] = '<?=str_replace("'", "\\'", $user['certname']);?>';
<?		endforeach; ?>
<?php		$c=0;
		foreach ($server['certs'] as $cert): ?>
servers[<?=$sindex;?>][3][<?=$c;?>] = new Array();
servers[<?=$sindex;?>][3][<?=$c;?>][0] = '<?=$cert['cindex'];?>';
servers[<?=$sindex;?>][3][<?=$c;?>][1] = '<?=str_replace("'", "\\'", $cert['certname']);?>';
<?		$c++;
		endforeach; ?>
<?	endforeach; ?>

function download_begin(act, i, j) {

	var index = document.getElementById("server").selectedIndex;
	var users = servers[index][1];
	var certs = servers[index][3];
	var useaddr;

	var advancedoptions;

	if (document.getElementById("useaddr").value == "other") {
		if (document.getElementById("useaddr_hostname").value == "") {
			alert("Please specify an IP address or hostname.");
			return;
		}
		useaddr = document.getElementById("useaddr_hostname").value;
	} else
		useaddr = document.getElementById("useaddr").value;

	advancedoptions = document.getElementById("advancedoptions").value;

	var quoteservercn = 0;
	if (document.getElementById("quoteservercn").checked)
		quoteservercn = 1;
	var usetoken = 0;
	if (document.getElementById("usetoken").checked)
		usetoken = 1;
	var usepass = 0;
	if (document.getElementById("usepass").checked)
		usepass = 1;
	var openvpnmanager = 0;
	if (document.getElementById("openvpnmanager").checked)
		openvpnmanager = 1;

	var pass = document.getElementById("pass").value;
	var conf = document.getElementById("conf").value;
	if (usepass && (act.substring(0,4) == "inst")) {
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
	dlurl += "&srvid=" + escape(servers[index][0]);
	if (users[i]) {
		dlurl += "&usrid=" + escape(users[i][0]);
		dlurl += "&crtid=" + escape(users[i][1]);
	}
	if (certs[j]) {
		dlurl += "&usrid=";
		dlurl += "&crtid=" + escape(certs[j][0]);
	}
	dlurl += "&useaddr=" + escape(useaddr);
	dlurl += "&quoteservercn=" + escape(quoteservercn);
	dlurl += "&openvpnmanager=" + escape(openvpnmanager);
	dlurl += "&usetoken=" + escape(usetoken);
	if (usepass)
		dlurl += "&password=" + escape(pass);
	if (useproxy) {
		dlurl += "&proxy_addr=" + escape(proxyaddr);
		dlurl += "&proxy_port=" + escape(proxyport);
		dlurl += "&proxy_authtype=" + escape(proxyauth);
		if (useproxypass) {
			dlurl += "&proxy_user=" + escape(proxyuser);
			dlurl += "&proxy_password=" + escape(proxypass);
		}
	}

	dlurl += "&advancedoptions=" + escape(advancedoptions);

	window.open(dlurl,"_self");
}

function server_changed() {

	var table = document.getElementById("users");
	while (table.rows.length > 1 )
		table.deleteRow(1);

	var index = document.getElementById("server").selectedIndex;
	var users = servers[index][1];
	var certs = servers[index][3];
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
		cell2.innerHTML = "<a href='javascript:download_begin(\"conf\"," + i + ", -1)'>Configuration</a>";
		cell2.innerHTML += "<br/>";
		cell2.innerHTML += "<a href='javascript:download_begin(\"confinline\"," + i + ", -1)'>Inline Configuration</a>";
		cell2.innerHTML += "<br/>";
		cell2.innerHTML += "<a href='javascript:download_begin(\"confzip\"," + i + ", -1)'>Configuration archive</a>";
		cell2.innerHTML += "<br/>Windows Installers:<br/>";
		cell2.innerHTML += "&nbsp;&nbsp; ";
		cell2.innerHTML += "<a href='javascript:download_begin(\"inst\"," + i + ", -1)'>2.2</a>";
		cell2.innerHTML += "&nbsp;&nbsp; ";
		cell2.innerHTML += "<a href='javascript:download_begin(\"inst-2.3-x86\"," + i + ", -1)'>2.3-x86</a>";
//		cell2.innerHTML += "&nbsp;&nbsp; ";
//		cell2.innerHTML += "<a href='javascript:download_begin(\"inst-2.3-x64\"," + i + ", -1)'>2.3-x64</a>";
		cell2.innerHTML += "<br/>";
		cell2.innerHTML += "<a href='javascript:download_begin(\"visc\"," + i + ", -1)'>Viscosity Bundle</a>";
	}
	for (j=0; j < certs.length; j++) {
		var row = table.insertRow(table.rows.length);
		var cell0 = row.insertCell(0);
		var cell1 = row.insertCell(1);
		var cell2 = row.insertCell(2);
		cell0.className = "listlr";
		if (servers[index][2] == "server_tls") {
			cell0.innerHTML = "Certificate (SSL/TLS, no Auth)";
		} else {
			cell0.innerHTML = "Certificate with External Auth";
		}
		cell1.className = "listr";
		cell1.innerHTML = certs[j][1];
		cell2.className = "listr";
		cell2.innerHTML = "<a href='javascript:download_begin(\"conf\", -1," + j + ")'>Configuration</a>";
		cell2.innerHTML += "<br/>";
		cell2.innerHTML += "<a href='javascript:download_begin(\"confinline\", -1," + j + ")'>Inline Configuration</a>";
		cell2.innerHTML += "<br/>";
		cell2.innerHTML += "<a href='javascript:download_begin(\"confzip\", -1," + j + ")'>Configuration archive</a>";
		cell2.innerHTML += "<br/>Windows Installers:<br/>";
		cell2.innerHTML += "&nbsp;&nbsp; ";
		cell2.innerHTML += "<a href='javascript:download_begin(\"inst\", -1," + j + ")'>2.2</a>";
		cell2.innerHTML += "&nbsp;&nbsp; ";
		cell2.innerHTML += "<a href='javascript:download_begin(\"inst-2.3-x86\", -1," + j + ")'>2.3-x86</a>";
//		cell2.innerHTML += "&nbsp;&nbsp; ";
//		cell2.innerHTML += "<a href='javascript:download_begin(\"inst-2.3-x64\", -1," + j + ")'>2.3-x64</a>";
		cell2.innerHTML += "<br/>";
		cell2.innerHTML += "<a href='javascript:download_begin(\"visc\", -1," + j + ")'>Viscosity Bundle</a>";
		if (servers[index][2] == "server_tls") {
			cell2.innerHTML += "<br/>Yealink SIP Handsets: <br/>";
			cell2.innerHTML += "&nbsp;&nbsp; ";
			cell2.innerHTML += "<a href='javascript:download_begin(\"conf_yealink_t28\", -1," + j + ")'>T28</a>";
			cell2.innerHTML += "&nbsp;&nbsp; ";
			cell2.innerHTML += "<a href='javascript:download_begin(\"conf_yealink_t38g\", -1," + j + ")'>T38G (1)</a>";
			cell2.innerHTML += "&nbsp;&nbsp; ";
			cell2.innerHTML += "<a href='javascript:download_begin(\"conf_yealink_t38g2\", -1," + j + ")'>T38G (2)</a>";
			cell2.innerHTML += "<br/>";
			cell2.innerHTML += "<a href='javascript:download_begin(\"conf_snom\", -1," + j + ")'>SNOM SIP Handset</a>";
		}
	}
	if (servers[index][2] == 'server_user') {
		var row = table.insertRow(table.rows.length);
		var cell0 = row.insertCell(0);
		var cell1 = row.insertCell(1);
		var cell2 = row.insertCell(2);
		cell0.className = "listlr";
		cell0.innerHTML = "Authentication Only (No Cert)";
		cell1.className = "listr";
		cell1.innerHTML = "none";
		cell2.className = "listr";
		cell2.innerHTML = "<a href='javascript:download_begin(\"conf\"," + i + ")'>Configuration</a>";
		cell2.innerHTML += "<br/>";
		cell2.innerHTML += "<a href='javascript:download_begin(\"confinline\"," + i + ")'>Inline Configuration</a>";
		cell2.innerHTML += "<br/>";
		cell2.innerHTML += "<a href='javascript:download_begin(\"confzip\"," + i + ")'>Configuration archive</a>";
		cell2.innerHTML += "<br/>Windows Installers:<br/>";
		cell2.innerHTML += "&nbsp;&nbsp; ";
		cell2.innerHTML += "<a href='javascript:download_begin(\"inst\"," + i + ")'>2.2</a>";
		cell2.innerHTML += "&nbsp;&nbsp; ";
		cell2.innerHTML += "<a href='javascript:download_begin(\"inst-2.3-x86\"," + i + ")'>2.3-x86</a>";
//		cell2.innerHTML += "&nbsp;&nbsp; ";
//		cell2.innerHTML += "<a href='javascript:download_begin(\"inst-2.3-x64\"," + i + ")'>2.3-x64</a>";
		cell2.innerHTML += "<br/>";
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
		$(obj.id + 'pass').value != 'none') {
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
				$tab_array[] = array(gettext("Wizards"), false, "wizard.php?xml=openvpn_wizard.xml");
				$tab_array[] = array(gettext("Client Export"), true, "vpn_openvpn_export.php");
				$tab_array[] = array(gettext("Shared Key Export"), false, "vpn_openvpn_export_shared.php");
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
											<?php if (is_array($config['dyndnses']['dyndns'])): ?>
												<?php foreach ($config['dyndnses']['dyndns'] as $ddns): ?>
													<option value="<?php echo $ddns["host"] ?>">DynDNS: <?php echo $ddns["host"] ?></option>
												<?php endforeach; ?>
											<?php endif; ?>
											<option value="other">Other</option>
										</select>
										<br />
										<div style="display:none;" name="HostName" id="HostName">
											<input name="useaddr_hostname" id="useaddr_hostname" />
											<span class="vexpl">
												Enter the hostname or IP address the client will use to connect to this server.
											</span>
										</div>
									</td>
								</tr>
							</table>
						</td>
					</tr>
					<tr>
						<td width="22%" valign="top" class="vncell">Quote Server CN</td>
						<td width="78%" class="vtable">
							<table border="0" cellpadding="2" cellspacing="0">
								<tr>
									<td>
										<input name="quoteservercn" id="quoteservercn" type="checkbox" value="yes">
									</td>
									<td>
										<span class="vexpl">
											 Enclose the server CN in quotes. Can help if your server CN contains spaces and certain clients cannot parse the server CN. Some clients have problems parsing the CN with quotes. Use only as needed.
										</span>
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
											Use a password to protect the pkcs12 file contents or key in Viscosity bundle.
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
											 &nbsp;     IP Address :&nbsp;
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
						<td width="22%" valign="top" class="vncell">Management&nbsp;Interface<br/>OpenVPNManager</td>
						<td width="78%" class="vtable">
							<table border="0" cellpadding="2" cellspacing="0">
								<tr>
									<td>
										<input name="openvpnmanager" id="openvpnmanager" type="checkbox" value="yes">
									</td>
									<td>
										<span class="vexpl">
											 This will change the generated .ovpn configuration to allow for usage of the management interface.
											 And include the OpenVPNManager program in the "Windows Installers". With this OpenVPN can be used also by non-administrator users.
											 This is also usefull for Windows7/Vista systems where elevated permissions are needed to add routes to the system.
										</span>
									</td>
								</tr>
							</table>
						</td>
					</tr>
					<tr>
						<td colspan="2" class="list" height="12">&nbsp;</td>
					</tr>
					<tr>
						<td width="22%" valign="top" class="vncell">Additional configuration options</td>
						<td width="78%" class="vtable">
							<textarea rows="6" cols="78" name="advancedoptions" id="advancedoptions"></textarea><br/>
							<?=gettext("Enter any additional options you would like to add to the OpenVPN client export configuration here, separated by a line break or semicolon"); ?><br/>
							<?=gettext("EXAMPLE: remote-random"); ?>;
						</td>
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
				<table width="100%" width="100%" border="0" cellpadding="5" cellspacing="10">
					<tr>
						<td align="right" valign="top" width="5%"><?= gettext("NOTE:") ?></td>
						<td><?= gettext("If you expect to see a certain client in the list but it is not there, it is usually due to a CA mismatch between the OpenVPN server instance and the client certificates found in the User Manager.") ?></td>
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
