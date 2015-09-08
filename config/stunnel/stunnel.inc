<?php
/*
	stunnel.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2007-2009 Scott Ullrich
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
global $config;

$pf_version = substr(trim(file_get_contents("/etc/version")), 0, 3);
if ($pf_version == "2.1" || $pf_version == "2.2") {
	define('STUNNEL_LOCALBASE', '/usr/pbi/stunnel-' . php_uname("m"));
} else {
	define('STUNNEL_LOCALBASE', '/usr/local');
}
define('STUNNEL_ETCDIR', STUNNEL_LOCALBASE . "/etc/stunnel");

if (!isset($_GET['id']) and !isset($_POST['id'])) {
	if ($GLOBALS['config']['installedpackages']['stunnelcerts']['savemsg']) {
		$savemsg = $GLOBALS['config']['installedpackages']['stunnelcerts']['savemsg'];
		unset($GLOBALS['config']['installedpackages']['stunnelcerts']['savemsg']);
		write_config();
	}
}

if (isset($_GET['id'])) {
	$config['installedpackages']['stunnelcerts']['config'][$_GET['id']]['cert_chain'] =
		base64_decode($config['installedpackages']['stunnelcerts']['config'][$_GET['id']]['cert_chain']);
	$config['installedpackages']['stunnelcerts']['config'][$_GET['id']]['cert_key'] =
		base64_decode($config['installedpackages']['stunnelcerts']['config'][$_GET['id']]['cert_key']);
}

$certs = $config['installedpackages']['stunnelcerts']['config'];
is_array($certs) ? $num_certs = count($certs) : $num_certs = 0;
if (!isset($_GET['id']) and !isset($_POST['id']) and $num_certs) {
	for ($i = 0; $i < $num_certs; $i++) {
		$cert = $certs[$i];
		$_info = openssl_x509_parse(base64_decode($cert['cert_chain']));
		$valid = floor(($_info['validTo_time_t'] - time()) / 86400);
		if ($cert['cert_chain']) {
			if (!openssl_x509_check_private_key(base64_decode($cert['cert_chain']), base64_decode($cert['cert_key']))) {
				$_status = '<font color="#AA0000"><strong>Invalid key/cert!</strong></font>';
			} elseif ($valid < 30) {
				$_status = '<font color="#B27D4B">Expires in ' . $valid . ' days!</font>';
			} else {
				$_status = '<font color="#008800">OK (' . $valid . ' days)</font>';
			}
			$config['installedpackages']['stunnelcerts']['config'][$i]['status'] = $_status;
		} else {
			unset($config['installedpackages']['stunnelcerts']['config'][$i]);
		}
	}
}


$tunnels = $config['installedpackages']['stunnel']['config'];
is_array($tunnels) ? $num_tunnels = count($tunnels) : $num_tunnels = 0;
if (!isset($_GET['id']) and $num_tunnels) {
	for ($i = 0; $i < $num_tunnels; $i++) {
		$tunnel = $tunnels[$i];
		if ($tunnel['certificate']) {
			$certid = 0;
			if (is_array($config['installedpackages']['stunnelcerts']['config'])) {
				foreach ($config['installedpackages']['stunnelcerts']['config'] as $cert) {
					if ($tunnel['certificate'] == $cert['filename']) {
						$config['installedpackages']['stunnel']['config'][$i]['certificatelink']=
							'<a href="/pkg_edit.php?xml=stunnel_certs.xml&act=edit&id=' . $certid . '">' . $cert['description'] . '</a>';
					}
					$certid++;
				}
			}
		}
	}
}

function stunnel_printcsr() {
//	$GLOBALS['savemsg'] = "<pre>" . print_r($GLOBALS['config']['installedpackages']['stunnelcerts']['config'], true) . "</pre>";
}

function stunnel_addcerts($config) {
	$certs=$config['installedpackages']['stunnelcerts']['config'];
	$tunnels=$config['installedpackages']['stunnel']['config'];
	?>
	<script type="text/javascript">
	//<![CDATA[
	function addcerts() {
		<?php

		foreach ($certs as $cert) {
			echo("document.forms['iform'].certificate.appendChild(new Option('".$cert['description']."', '".$cert['filename']."'));");
		}

		?>
	}
	addcerts();
	for (var i=0; i < document.forms['iform'].certificate.length; i++) {
		<?php
		$filename=$tunnels[$_GET['id']]['certificate'];
		echo('if (document.forms["iform"].certificate[i].value == "'.$filename.'") {');
		?>
			document.forms['iform'].certificate[i].selected = true;
		} else {
			document.forms['iform'].certificate[i].selected = false;
		}
	}
	//]]>
	</script>
	<?php
}

function stunnel_disablefields() {
	?>
	<script type="text/javascript">
	//<![CDATA[
		document.forms['iform'].subject.readOnly=true;
		document.forms['iform'].filename.readOnly=true;
		document.forms['iform'].expiry.readOnly=true;
	//]]>
	</script>
	<?php
}

function stunnel_delete($config) {
	$cert=$config['installedpackages']['stunnelcerts']['config'][$_GET['id']];
	if (isset($_GET['id'])) {
		unlink_if_exists(STUNNEL_ETCDIR . '/' . $cert['filename'] . '.pem');
		unlink_if_exists(STUNNEL_ETCDIR . '/' . $cert['filename'] . '.key');
		unlink_if_exists(STUNNEL_ETCDIR . '/' . $cert['filename'] . '.chain');
	}
}

function stunnel_save($config) {
	$GLOBALS['config']['installedpackages']['stunnelcerts']['savemsg'] = '';
	conf_mount_rw();
	safe_mkdir(STUNNEL_ETCDIR, 0755);
	$fout = fopen(STUNNEL_ETCDIR . "/stunnel.conf", "w");
	fwrite($fout, "cert = " . STUNNEL_ETCDIR . "/stunnel.pem \n");
	fwrite($fout, "chroot = /var/tmp/stunnel \n");
	fwrite($fout, "setuid = stunnel \n");
	fwrite($fout, "setgid = stunnel \n");
	if (!is_array($config['installedpackages']['stunnel']['config'])) {
		$config['installedpackages']['stunnel']['config'] = array();
	}
	foreach ($config['installedpackages']['stunnel']['config'] as $pkgconfig) {
		fwrite($fout, "\n[" . $pkgconfig['description'] . "]\n");
		if ($pkgconfig['client']) {
			fwrite($fout, "client = yes" . "\n");
		}
		if ($pkgconfig['certificate']) {
			if (file_exists(STUNNEL_ETCDIR . '/'.$pkgconfig['certificate'].'.key') and file_exists(STUNNEL_ETCDIR . '/'.$pkgconfig['certificate'].'.chain')) {
				fwrite($fout, "key = " . STUNNEL_ETCDIR . "/" . $pkgconfig['certificate'] . ".key\n");
				fwrite($fout, "cert = " . STUNNEL_ETCDIR . "/" . $pkgconfig['certificate'] . ".chain\n");
			}
		}
		if ($pkgconfig['sourceip']) {
			fwrite($fout, "local = " . $pkgconfig['sourceip'] . "\n");
		}
		fwrite($fout, "accept = " . $pkgconfig['localip'] . ":" . $pkgconfig['localport'] . "\n");
		fwrite($fout, "connect = " . $pkgconfig['redirectip'] . ":" . $pkgconfig['redirectport'] . "\n");
		fwrite($fout, "TIMEOUTclose = 0\n\n");
	}
	fclose($fout);
	conf_mount_ro();
	stop_service("stunnel");
	start_service("stunnel");
}

function stunnel_save_cert($config) {
	$GLOBALS['config']['installedpackages']['stunnelcerts']['savemsg'] = '';
	if (isset($_POST['id'])) {
		if (!$_POST['cert_chain']) {
			$GLOBALS['config']['installedpackages']['stunnelcerts']['savemsg'] .= "Certificate chain must be specified!<br />";
		}
		if (!$_POST['cert_key']) {
			$GLOBALS['config']['installedpackages']['stunnelcerts']['savemsg'] .= "RSA Key must be specified!<br />";
		}
		if ($_POST['cert_chain'] and $_POST['cert_key']) {
			$_cert = openssl_x509_parse($_POST['cert_chain']);
			if ($_cert['hash']) {
				if (openssl_x509_check_private_key($_POST['cert_chain'], $_POST['cert_key'])) {
					file_put_contents(STUNNEL_ETCDIR . '/'. $_cert['hash'] . '.key', $_POST['cert_key']);
					file_put_contents(STUNNEL_ETCDIR . '/' . $_cert['hash'] . '.chain', $_POST['cert_chain']);
					file_put_contents(STUNNEL_ETCDIR . '/' . $_cert['hash'] . '.pem', $_POST['cert_key']."\n".$_POST['cert_chain']);
					system('chown stunnel:stunnel ' . STUNNEL_ETCDIR . '/*');
					chmod(STUNNEL_ETCDIR . '/' . $_cert['hash'] . '.key', 0600);
					chmod(STUNNEL_ETCDIR . '/' . $_cert['hash'] . '.pem', 0600);

					$_POST['filename'] = $_cert['hash'];
					$_POST['expiry_raw'] = $_cert['validTo_time_t'];
					$_POST['expiry'] = date('Y-m-d', $_cert['validTo_time_t']);
					$_POST['subject'] = $_cert['name'];
				} else {
					$GLOBALS['config']['installedpackages']['stunnelcerts']['savemsg'] .= "Certificate and key do not match!<br />";
					$_POST['filename'] = '';
				}
			} else {
				$GLOBALS['config']['installedpackages']['stunnelcerts']['savemsg'] .= "Couldn't parse certificate!<br />";
				$_POST['expiry_raw'] = '';
				$_POST['expiry'] = '';
				$_POST['subject'] = '';
				$_POST['filename'] = '';
			}
		}
		$_POST['cert_key'] = base64_encode($_POST['cert_key']);
		$_POST['cert_chain'] = base64_encode($_POST['cert_chain']);
		$_fname = $GLOBALS['config']['installedpackages']['stunnelcerts']['config'][$_POST['id']]['filename'];
		if ($_fname and $_fname != $_POST['filename']) {
			unlink_if_exists(STUNNEL_ETCDIR . '/' . $_fname . '.chain');
			unlink_if_exists(STUNNEL_ETCDIR . '/' . $_fname . '.key');
			unlink_if_exists(STUNNEL_ETCDIR . '/' . $_fname . '.pem');
		}
	}
}

function stunnel_install() {
	safe_mkdir(STUNNEL_ETCDIR);
	system("/usr/bin/openssl req -new -x509 -days 365 -nodes -out " . STUNNEL_ETCDIR . "/stunnel.pem -keyout " . STUNNEL_ETCDIR . "/stunnel.pem 2>/dev/null");
	chmod(STUNNEL_ETCDIR . "/stunnel.pem", 0600);
	@mkdir("/var/tmp/stunnel/var/tmp/run/stunnel", 0755, true);
	system("/usr/sbin/chown -R stunnel:stunnel /var/tmp/stunnel");
	$_rcfile['file'] = 'stunnel.sh';
	$_rcfile['start'] = STUNNEL_LOCALBASE . "/bin/stunnel " . STUNNEL_ETCDIR . "/stunnel.conf \n\t";
	$_rcfile['stop'] = "/usr/bin/killall stunnel \n\t";
	write_rcfile($_rcfile);
	unlink_if_exists("/usr/local/etc/rc.d/stunnel");

	$fout = fopen(STUNNEL_ETCDIR . "/stunnel.conf", "w");
	fwrite($fout, "cert = " . STUNNEL_ETCDIR . "/stunnel.pem \n");
	fwrite($fout, "chroot = /var/tmp/stunnel \n");
	fwrite($fout, "setuid = stunnel \n");
	fwrite($fout, "setgid = stunnel \n");
	if ($config['installedpackages']['stunnel']['config']) {
		foreach ($config['installedpackages']['stunnel']['config'] as $pkgconfig) {
			fwrite($fout, "\n[" . $pkgconfig['description'] . "]\n");
			if ($pkgconfig['sourceip']) {
				fwrite($fout, "local = " . $pkgconfig['sourceip'] . "\n");
			}
			fwrite($fout, "accept = " . $pkgconfig['localip'] . ":" . $pkgconfig['localport'] . "\n");
			fwrite($fout, "connect = " . $pkgconfig['redirectip'] . ":" . $pkgconfig['redirectport'] . "\n");
			fwrite($fout, "TIMEOUTclose = 0\n\n");
		}
	}
	fclose($fout);
}

function stunnel_deinstall() {
	rmdir_recursive("/var/tmp/stunnel");
	rmdir_recursive(STUNNEL_ETCDIR);
}

?>
