<?php
/*
	vhosts_php_edit.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2008 Mark J Crane
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
require("guiconfig.inc");
require("/usr/local/pkg/vhosts.inc");

$a_vhosts = &$config['installedpackages']['vhosts']['config'];

$id = $_GET['id'];
if (isset($_POST['id'])) {
	$id = $_POST['id'];
}

$a_vhosts = &$config['installedpackages']['vhosts']['config'];
$a_service = $config['installedpackages']['service'];

if ($_GET['act'] == "del") {
	if ($_GET['type'] == 'php') {
		if ($a_vhosts[$_GET['id']]) {
			// Get vhost info
			$x = 0;
			$y = 0;
			foreach ($a_vhosts as $rowhelper) {
				if (strlen($rowhelper['certificate']) > 0) {
					$y++;
				}
				if ($_GET['id'] == $x) {
					// Return the id
					$id = $x;
					$host = $rowhelper['host'];
					$ipaddress = $rowhelper['ipaddress'];
					$port = $rowhelper['port'];
					$directory = $rowhelper['directory'];
					if (strlen($rowhelper['certificate']) > 0) {
						$ssl = true;
						$ssl_id = $y;
					} else {
						$ssl = false;
					}
				}
				$x++;
			}

			// Delete vhosts entry
			unset($a_vhosts[$_GET['id']]);

			// Delete the SSL files and service
			if ($ssl) {
				unlink_if_exists("/var/etc/vhosts-{$ipaddress}-{$port}-ssl.conf");
				unlink_if_exists("/var/etc/cert-vhosts-{$ipaddress}-{$port}.pem");
				unlink_if_exists("/usr/local/etc/rc.d/vhosts-{$ipaddress}-{$port}-ssl.sh");
				$service_id = get_service_id ($a_service, 'rcfile', "vhosts-{$ipaddress}-{$port}-ssl.sh");
				if (is_int($service_id)) {
					exec("kill `cat /var/run/vhosts-{$ipaddress}-{$port}-ssl.pid`");
					unset($config['installedpackages']['service'][$service_id]);
				}
			}

			write_config();
			header("Location: vhosts_php.php");
			exit;
		}
	}
}

if (isset($id) && $a_vhosts[$id]) {
	$pconfig['host'] = $a_vhosts[$id]['host'];
	$pconfig['ipaddress'] = $a_vhosts[$id]['ipaddress'];
	$pconfig['port'] = $a_vhosts[$id]['port'];
	$pconfig['directory'] = $a_vhosts[$id]['directory'];
	if (strlen($a_vhosts[$id]['certificate']) > 0) {
		$pconfig['certificate'] = base64_decode($a_vhosts[$id]['certificate']);
	}
	if (strlen($a_vhosts[$id]['privatekey']) > 0) {
		$pconfig['privatekey'] = base64_decode($a_vhosts[$id]['privatekey']);
	}
	$pconfig['enabled'] = $a_vhosts[$id]['enabled'];
	$pconfig['description'] = $a_vhosts[$id]['description'];
}

if ($_POST) {

	unset($input_errors);
	$pconfig = $_POST;

	if (!$input_errors) {

		$ent = array();
		$ent['host'] = $_POST['host'];
		$ent['ipaddress'] = $_POST['ipaddress'];
		$ent['port'] = $_POST['port'];
		$ent['directory'] = $_POST['directory'];

		if (strlen($_POST['certificate']) > 0) {
			$ent['certificate'] = base64_encode($_POST['certificate']);
		} else {
			$ent['certificate'] = '';
		}
		if (strlen($_POST['privatekey']) > 0) {
			$ent['privatekey'] = base64_encode($_POST['privatekey']);
		} else {
			$ent['privatekey'] = '';
		}
		$ent['enabled'] = $_POST['enabled'];
		$ent['description'] = $_POST['description'];

		if (isset($id) && $a_vhosts[$id]) {
			// Update
			$a_vhosts[$id] = $ent;
		} else {
			// Add
			$a_vhosts[] = $ent;
		}

		write_config();
		vhosts_sync_package();

		header("Location: vhosts_php.php");
		exit;
	}
}

$pgtitle = "vHosts: Edit";
include("head.inc");

?>

<script type="text/javascript">
//<![CDATA[
function show_advanced_config() {
	document.getElementById("showadvancedbox").innerHTML='';
	aodiv = document.getElementById('showadvanced');
	aodiv.style.display = "block";
}
//]]>
</script>
<script type="text/javascript">
//<![CDATA[
function openwindow(url) {
	var oWin = window.open(url,"pfSensePop","width=620,height=400,top=150,left=150");
	if (oWin==null || typeof(oWin)=="undefined") {
		return false;
	} else {
		return true;
	}
}
//]]>
</script>

<body link="#0000CC" vlink="#000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if ($input_errors) print_input_errors($input_errors); ?>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabnavtbl">
<?php

	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), true, "/packages/vhosts/vhosts_php.php");
	display_top_tabs($tab_array);

?>
</td></tr>
</table>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabcont" >
	<br />
	<form action="vhosts_php_edit.php" method="post" name="iform" id="iform">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td width="22%" valign="top" class="vncellreq">Host</td>
			<td width="78%" class="vtable">
				<input name="host" type="text" class="formfld" id="host" size="40" value="<?=htmlspecialchars($pconfig['host']);?>" />
				<br />
				Required. If the host is intended for internal you can use the DNS forwarder to set a host name that is valid inside the local network. Default: vhost01.local
			</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncellreq">IP Address</td>
			<td width="78%" class="vtable">
				<input name="ipaddress" type="text" class="formfld" id="ipaddress" size="40" value="<?=htmlspecialchars($pconfig['ipaddress']);?>" />
				<br />
				Required. Make sure the IP and Port combination does not conflict with the local system. Example: 192.168.0.1
			</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncell">Port</td>
			<td width="78%" class="vtable">
				<input name="port" type="text" class="formfld" id="port" size="40" value="<?=htmlspecialchars($pconfig['port']);?>" />
				<br />
				Make sure the IP and Port combination does not conflict with the local system. Default: 10081
			</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncell">Directory</td>
			<td width="78%" class="vtable">
				<input name="directory" type="text" class="formfld" id="directory" size="40" value="<?=htmlspecialchars($pconfig['directory']);?>" />
				<br />
				This vHosts directory is located in /usr/local/vhosts. The default directory is the host name.
				<br />
				example: vhost01.local
			</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncell">Certificate</td>
			<td width="78%" class="vtable">
				<textarea name="certificate" cols="65" rows="7" id="certificate" class="formpre"><?=htmlspecialchars($pconfig['certificate']);?></textarea>
				<br />
				Paste a signed certificate in X.509 PEM format here. <a href="javascript:if(openwindow('/packages/vhosts/system_advanced_create_certs.php') == false) alert('Popup blocker detected. Action aborted.');" >Create</a> certificates automatically.
			</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncell">Key</td>
			<td width="78%" class="vtable">
				<textarea name="privatekey" cols="65" rows="7" id="privatekey" class="formpre"><?=htmlspecialchars($pconfig['privatekey']);?></textarea>
				<br />
				Paste an RSA private key in PEM format here.
			</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncellreq">Enabled</td>
			<td width="78%" class="vtable">
			<?php
				echo "<select name='enabled' class='formfld'>\n";
				echo "\t<option></option>\n";
				switch (htmlspecialchars($pconfig['enabled'])) {
					case "true":
						echo "\t<option value='true' selected='yes'>true</option>\n";
						echo "\t<option value='false'>false</option>\n";
						break;
					case "false":
						echo "\t<option value='true'>true</option>\n";
						echo "\t<option value='false' selected='yes'>false</option>\n";
						break;
					default:
						echo "\t<option value='true' selected='yes'>true</option>\n";
						echo "\t<option value='false'>false</option>\n";
				}
				echo "</select>\n";
				?>
			</td>
		</tr>
		<tr>
			<td width="25%" valign="top" class="vncell">Description</td>
			<td width="75%" class="vtable">
				<input name="description" type="text" class="formfld" id="description" size="40" value="<?=htmlspecialchars($pconfig['description']);?>" />
				<br /><span class="vexpl">Enter the description here.<br /></span>
			</td>
		</tr>
		<tr>
			<td valign="top">&nbsp;</td>
			<td>
				<input name="Submit" type="submit" class="formbtn" value="Save" />&nbsp;<input class="formbtn" type="button" value="Cancel" onclick="history.back()" />
				<?php if (isset($id) && $a_vhosts[$id]): ?>
					<input name="id" type="hidden" value="<?=$id;?>" />
				<?php endif; ?>
			</td>
		</tr>
		</table>
	</form>
	<br />

</td></tr>
</table>

</div>

<?php include("fend.inc"); ?>
</body>
</html>
