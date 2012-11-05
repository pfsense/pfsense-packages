<?php
/*
	openvpn-client-export.inc
	Copyright (C) 2009 Scott Ullrich <sullrich@gmail.com>
	Copyright (C) 2008 Shrew Soft Inc
	Copyright (C) 2010 Ermal Luci
	All rights reserved.

	Parts of this code was originally based on vpn_ipsec_sad.php
	Copyright (C) 2003-2004 Manuel Kasper

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

require_once("globals.inc");
require_once("openvpn.inc");

function openvpn_client_export_install() {
	conf_mount_rw();
	$tarpath = "/usr/local/pkg/openvpn-client-export.tgz";
	$phpfile = "vpn_openvpn_export.php";
	$ovpndir = "/usr/local/share/openvpn";
	$workdir = "{$ovpndir}/client-export";

	if (!is_dir($workdir))
		mkdir($workdir, 0777, true);

	exec("/usr/bin/tar zxf {$tarpath} -C {$ovpndir}");
	conf_mount_ro();
}

function openvpn_client_export_deinstall() {
	conf_mount_rw();
	$phpfile = "vpn_openvpn_export.php";
	$phpfile2 = "vpn_openvpn_export_shared.php";
	$ovpndir = "/usr/local/share/openvpn";
	$workdir = "{$ovpndir}/client-export";

	unlink_if_exists("/usr/local/www/{$phpfile}");
	unlink_if_exists("/usr/local/www/{$phpfile2}");
	exec("/bin/rm -r {$workdir}");
	conf_mount_ro();
}

function openvpn_client_export_prefix($srvid, $usrid = null) {
	global $config;

	// lookup server settings
	$settings = $config['openvpn']['openvpn-server'][$srvid];
	if (empty($settings))
		return false;
	if ($settings['disable'])
		return false;

	$host = empty($config['system']['hostname']) ? "openvpn" : $config['system']['hostname'];
	$prot = ($settings['protocol'] == 'UDP' ? 'udp' : $settings['protocol']);
	$port = $settings['local_port'];
	
	$username = "";
	//$config['openvpn']['openvpn-server'];
	if ($usrid && is_numeric($usrid))
		$username = "-".$config['system']['user'][$usrid]['name'];

	return "{$host}-{$prot}-{$port}{$username}";
}

function openvpn_client_pem_to_pk12($outpath, $outpass, $crtpath, $keypath, $capath = false) {
	$eoutpath = escapeshellarg($outpath);
	$eoutpass = escapeshellarg($outpass);
	$ecrtpath = escapeshellarg($crtpath);
	$ekeypath = escapeshellarg($keypath);
	if ($capath) {
		$ecapath = escapeshellarg($capath);
		exec("/usr/bin/openssl pkcs12 -export -in {$ecrtpath} -inkey {$ekeypath} -certfile {$ecapath} -out {$eoutpath} -passout pass:{$eoutpass}");
	} else
		exec("/usr/bin/openssl pkcs12 -export -in {$ecrtpath} -inkey {$ekeypath} -out {$eoutpath} -passout pass:{$eoutpass}");

	unlink($crtpath);
	unlink($keypath);
	if ($capath)
		unlink($capath);
}

function openvpn_client_export_validate_config($srvid, $usrid, $crtid) {
	global $config, $g, $input_errors;

	// lookup server settings
	$settings = $config['openvpn']['openvpn-server'][$srvid];
	if (empty($settings)) {
		$input_errors[] = "Could not locate server configuration.";
		return false;
	}
	if ($settings['disable']) {
		$input_errors[] = "You cannot export for disabled servers.";
		return false;
	}

	// lookup server certificate info
	$server_cert = lookup_cert($settings['certref']);
	$server_ca = lookup_ca($server_cert['caref']);
	if (!$server_cert || !$server_ca) {
		$input_errors[] = "Could not locate certificate.";
		return false;
	}
	if (function_exists("cert_get_cn")) {
		$servercn = cert_get_cn($server_cert['crt']);
	}

	// lookup user info
	if (is_numeric($usrid)) {
		$user = $config['system']['user'][$usrid];
		if (!$user) {
			$input_errors[] = "Could not find user settings.";
			return false;
		}
	}

	// lookup user certificate info
	if ($settings['mode'] == "server_tls_user") {
		if ($settings['authmode'] == "Local Database") {
			$cert = $user['cert'][$crtid];
		} else {
			$cert = $config['cert'][$crtid];
		}
		if (!$cert)
			return false;
		// If $cert is not an array, it's a certref not a cert.
		if (!is_array($cert))
			$cert = lookup_cert($cert);
	} elseif (($settings['mode'] == "server_tls") || (($settings['mode'] == "server_tls_user") && ($settings['authmode'] != "Local Database"))) {
		$cert = $config['cert'][$crtid];
		if (!$cert)
			return false;
	} else
		$nokeys = true;

	return array($settings, $server_cert, $server_ca, $servercn, $user, $cert, $nokeys);
}

function openvpn_client_export_config($srvid, $usrid, $crtid, $useaddr, $quoteservercn, $usetoken, $nokeys = false, $proxy, $expformat = "baseconf", $outpass = "", $skiptls=false, $doslines=false, $openvpnmanager, $advancedoptions = "") {
	global $config, $input_errors, $g;

	$nl = ($doslines) ? "\r\n" : "\n";

	$validconfig = openvpn_client_export_validate_config($srvid, $usrid, $crtid);
	if ($validconfig) {
		list($settings, $server_cert, $server_ca, $servercn, $user, $cert, $nokeys) = $validconfig;
	} else {
		return false;
	}

	// determine basic variables
	if ($useaddr == "serveraddr") {
		$interface = $settings['interface'];
		if (!empty($settings['ipaddr']) && is_ipaddr($settings['ipaddr'])) {
			$server_host = $settings['ipaddr'];
		} else {
			if (!$interface || ($interface == "any"))
				$interface = "wan";
			$server_host = get_interface_ip($interface);
		}
	} else if ($useaddr == "serverhostname" || empty($useaddr)) {
		$server_host = empty($config['system']['hostname']) ? "" : "{$config['system']['hostname']}.";
		$server_host .= "{$config['system']['domain']}";
	} else
		$server_host = $useaddr;

	$server_port = $settings['local_port'];
	$proto = (strtoupper($settings['protocol']) == 'UDP' ? 'udp' : "tcp-client");

	$cipher = $settings['crypto'];

	// add basic settings
	$devmode = empty($settings['dev_mode']) ? "tun" : $settings['dev_mode'];
	$conf  = "dev {$devmode}{$nl}";
	if(! empty($settings['tunnel_networkv6'])) {
		$conf .= "tun-ipv6{$nl}";
	}
	$conf .= "persist-tun{$nl}";
	$conf .= "persist-key{$nl}";
	$conf .= "proto {$proto}{$nl}";
	$conf .= "cipher {$cipher}{$nl}";
	$conf .= "tls-client{$nl}";
	$conf .= "client{$nl}";
	$conf .= "resolv-retry infinite{$nl}";
	$conf .= "remote {$server_host} {$server_port}{$nl}";
	if (!empty($servercn)) {
		$qw = ($quoteservercn) ? "\"" : "";
		$conf .= "tls-remote {$qw}{$servercn}{$qw}{$nl}";
	}

	if (!empty($proxy)) {
		if ($proto == "udp") {
			$input_errors[] = "This server uses UDP protocol and cannot communicate with HTTP proxy.";
			return;
		}
		$conf .= "http-proxy {$proxy['ip']} {$proxy['port']} ";
		if ($proxy['proxy_authtype'] != "none") {
			if (!isset($proxy['passwdfile']))
				$proxy['passwdfile'] = openvpn_client_export_prefix($srvid, $usrid) . "-proxy";
			$conf .= " {$proxy['passwdfile']} {$proxy['proxy_authtype']}";
		}
		$conf .= "{$nl}";
	}

	// add user auth settings
	switch($settings['mode']) {
		case 'server_user':
		case 'server_tls_user':
			$conf .= "auth-user-pass{$nl}";
			break;
	}

	// add key settings
	$prefix = openvpn_client_export_prefix($srvid, $usrid);
	$cafile = "{$prefix}-ca.crt";
	if($nokeys == false) {
		if ($expformat == "inline") {
			$conf .= "ca [inline]{$nl}";
			$conf .= "cert [inline]{$nl}";
			$conf .= "key [inline]{$nl}";
		} elseif ($expformat == "yealink_t28") {
			$conf .= "ca /yealink/config/openvpn/keys/ca.crt{$nl}";
			$conf .= "cert /yealink/config/openvpn/keys/client1.crt{$nl}";
			$conf .= "key /yealink/config/openvpn/keys/client1.key{$nl}";
		} elseif ($expformat == "yealink_t38g") {
			$conf .= "ca /phone/config/openvpn/keys/ca.crt{$nl}";
			$conf .= "cert /phone/config/openvpn/keys/client1.crt{$nl}";
			$conf .= "key /phone/config/openvpn/keys/client1.key{$nl}";
		} elseif ($expformat == "yealink_t38g2") {
			$conf .= "ca /config/openvpn/keys/ca.crt{$nl}";
			$conf .= "cert /config/openvpn/keys/client1.crt{$nl}";
			$conf .= "key /config/openvpn/keys/client1.key{$nl}";
		} elseif ($expformat == "snom") {
			$conf .= "ca /openvpn/ca.crt{$nl}";
			$conf .= "cert /openvpn/phone1.crt{$nl}";
			$conf .= "key /openvpn/phone1.key{$nl}";
		} elseif ($usetoken) {
			$conf .= "ca {$cafile}{$nl}";
			$conf .= "cryptoapicert \"SUBJ:{$user['name']}\"{$nl}";
		} else {
			$conf .= "pkcs12 {$prefix}.p12{$nl}";
		}
	} else if ($settings['mode'] == "server_user") {
		if ($expformat == "inline")
			$conf .= "ca [inline]{$nl}";
		else
			$conf .= "ca {$cafile}{$nl}";
	}

	if ($settings['tls'] && !$skiptls) {
		if ($expformat == "inline")
			$conf .= "tls-auth [inline] 1{$nl}";
		elseif ($expformat == "yealink_t28")
			$conf .= "tls-auth /yealink/config/openvpn/keys/ta.key 1{$nl}";
		elseif ($expformat == "yealink_t38g")
			$conf .= "tls-auth /phone/config/openvpn/keys/ta.key 1{$nl}";
		elseif ($expformat == "yealink_t38g2")
			$conf .= "tls-auth /config/openvpn/keys/ta.key 1{$nl}";
		elseif ($expformat == "snom")
			$conf .= "tls-auth /openvpn/ta.key 1{$nl}";
		else
			$conf .= "tls-auth {$prefix}-tls.key 1{$nl}";
	}

	// Prevent MITM attacks by verifying the server certificate.
	// - Disable for now, it requires the server cert to include special options
	//$conf .= "remote-cert-tls server{$nl}";

	// Extra protection for the server cert, if it's supported
	if (function_exists("cert_get_purpose")) {
		if (is_array($server_cert) && ($server_cert['crt'])) {
			$purpose = cert_get_purpose($server_cert['crt'], true);
			if ($purpose['server'] == 'Yes')
				$conf .= "ns-cert-type server{$nl}";
		}
	}

	// add optional settings
	if ($settings['compression'])
		$conf .= "comp-lzo{$nl}";
	if ($settings['passtos'])
		$conf .= "passtos{$nl}";

	if ($openvpnmanager)
	{
		$conf .= $nl;
		$conf .= "# dont terminate service process on wrong password, ask again{$nl}";
		$conf .= "auth-retry interact{$nl}";
		$conf .= "# open management channel{$nl}";
		$conf .= "management 127.0.0.1 166{$nl}";
		$conf .= "# wait for management to explicitly start connection{$nl}";
		$conf .= "management-hold{$nl}";
		$conf .= "# query management channel for user/pass{$nl}";
		$conf .= "management-query-passwords{$nl}";
		$conf .= "# disconnect VPN when managment program connection is closed{$nl}";
		$conf .= "management-signal{$nl}";
		$conf .= "# forget password when management disconnects{$nl}";
		$conf .= "management-forget-disconnect{$nl}";
		$conf .= $nl;
	};
		
	// add advanced options
	$advancedoptions = str_replace("\r\n", "\n", $advancedoptions);
	$advancedoptions = str_replace("\n", $nl, $advancedoptions);
	$advancedoptions = str_replace(";", $nl, $advancedoptions);
	$conf .= $advancedoptions;
	$conf .= $nl;

	switch ($expformat) {
		case "zip":
			// create template directory
			$tempdir = "{$g['tmp_path']}/{$prefix}";
			mkdir($tempdir, 0700, true);

			file_put_contents("{$tempdir}/{$prefix}.ovpn", $conf);

			$cafile = "{$tempdir}/{$cafile}";
			file_put_contents("{$cafile}", base64_decode($server_ca['crt']));
			if ($settings['tls']) {
				$tlsfile = "{$tempdir}/{$prefix}-tls.key";
				file_put_contents($tlsfile, base64_decode($settings['tls']));
			}

			// write key files
			if ($settings['mode'] != "server_user") {
				$crtfile = "{$tempdir}/{$prefix}-cert.crt";
				file_put_contents($crtfile, base64_decode($cert['crt']));
				$keyfile = "{$tempdir}/{$prefix}.key";
				file_put_contents($keyfile, base64_decode($cert['prv']));

				// convert to pkcs12 format
				$p12file = "{$tempdir}/{$prefix}.p12";
				if ($usetoken)
					openvpn_client_pem_to_pk12($p12file, $outpass, $crtfile, $keyfile);
				else
					openvpn_client_pem_to_pk12($p12file, $outpass, $crtfile, $keyfile, $cafile);
			}
			exec("cd {$tempdir}/.. && /usr/local/bin/zip -r {$g['tmp_path']}/{$prefix}-config.zip {$prefix}");
			// Remove temporary directory
			exec("rm -rf {$tempdir}");
			return $g['tmp_path'] . "/{$prefix}-config.zip";
			break;
		case "inline":
			// Inline CA
			$conf .= "<ca>{$nl}" . base64_decode($server_ca['crt']) . "</ca>{$nl}";
			if ($settings['mode'] != "server_user") {
				// Inline Cert
				$conf .= "<cert>{$nl}" . base64_decode($cert['crt']) . "</cert>{$nl}";
				// Inline Key
				$conf .= "<key>{$nl}" . base64_decode($cert['prv']) . "</key>{$nl}";
			}
			// Inline TLS
			if ($settings['tls']) {
				$conf .= "<tls-auth>{$nl}" . base64_decode($settings['tls']) . "</tls-auth>{$nl} key-direction 1{$nl}";
			}
			return $conf;
			break;
		case "yealink_t28":
		case "yealink_t38g":
		case "yealink_t38g2":
			// create template directory
			$tempdir = "{$g['tmp_path']}/{$prefix}";
			$keydir  = "{$tempdir}/keys";
			mkdir($tempdir, 0700, true);
			mkdir($keydir, 0700, true);

			file_put_contents("{$tempdir}/vpn.cnf", $conf);

			$cafile = "{$keydir}/ca.crt";
			file_put_contents("{$cafile}", base64_decode($server_ca['crt']));
			if ($settings['tls']) {
				$tlsfile = "{$keydir}/ta.key";
				file_put_contents($tlsfile, base64_decode($settings['tls']));
			}

			// write key files
			if ($settings['mode'] != "server_user") {
				$crtfile = "{$keydir}/client1.crt";
				file_put_contents($crtfile, base64_decode($cert['crt']));
				$keyfile = "{$keydir}/client1.key";
				file_put_contents($keyfile, base64_decode($cert['prv']));
			}
			exec("tar -C {$tempdir} -cf {$g['tmp_path']}/client.tar ./keys ./vpn.cnf");
			// Remove temporary directory
			exec("rm -rf {$tempdir}");
			return $g['tmp_path'] . "/client.tar";
			break;
		case "snom":
			// create template directory
			$tempdir = "{$g['tmp_path']}/{$prefix}";
			mkdir($tempdir, 0700, true);

			file_put_contents("{$tempdir}/vpn.cnf", $conf);

			$cafile = "{$tempdir}/ca.crt";
			file_put_contents("{$cafile}", base64_decode($server_ca['crt']));
			if ($settings['tls']) {
				$tlsfile = "{$tempdir}/ta.key";
				file_put_contents($tlsfile, base64_decode($settings['tls']));
			}

			// write key files
			if ($settings['mode'] != "server_user") {
				$crtfile = "{$tempdir}/phone1.crt";
				file_put_contents($crtfile, base64_decode($cert['crt']));
				$keyfile = "{$tempdir}/phone1.key";
				file_put_contents($keyfile, base64_decode($cert['prv']));
			}
			exec("cd {$tempdir}/ && tar -cf {$g['tmp_path']}/vpnclient.tar *");
			// Remove temporary directory
			exec("rm -rf {$tempdir}");
			return $g['tmp_path'] . "/vpnclient.tar";
			break;
		default:
			return $conf;
	}
}

function openvpn_client_export_installer($srvid, $usrid, $crtid, $useaddr, $quoteservercn, $usetoken, $outpass, $proxy, $openvpnmanager, $advancedoptions, $openvpn_version = "2.1") {
	global $config, $g, $input_errors;
	$uname_p = trim(exec("uname -p"));

	switch ($openvpn_version) {
		case "2.3-x86":
			$client_install_exe = "openvpn-install-2.3-i686.exe";
			break;
		case "2.3-x64":
			$client_install_exe = "openvpn-install-2.3-x86_64.exe";
			break;
		default:
			$client_install_exe = "openvpn-install-2.2.exe";
	}

	$ovpndir = "/usr/local/share/openvpn";
	$workdir = "{$ovpndir}/client-export";
	if (!file_exists($workdir . "/template/{$client_install_exe}"))
		openvpn_client_export_install();

	$validconfig = openvpn_client_export_validate_config($srvid, $usrid, $crtid);
	if ($validconfig) {
		list($settings, $server_cert, $server_ca, $servercn, $user, $cert, $nokeys) = $validconfig;
	} else {
		return false;
	}

	// create template directory
	$tempdir = $g['tmp_path'] . "/openvpn-export-".uniqid();
	mkdir($tempdir, 0700, true);

	// create config directory
	$confdir = "{$tempdir}/config";
	if (!is_dir($conf_dir))
		mkdir($confdir, 0700, true);

	// copy the template directory
	exec("cp -r {$workdir}/template/* {$tempdir}");
	// and put the required installer exe in place
	exec("/bin/cp {$tempdir}/{$client_install_exe} {$tempdir}/openvpn-install.exe");

	// write configuration file
	$prefix = openvpn_client_export_prefix($srvid, $usrid);
	$cfgfile = "{$confdir}/{$prefix}-config.ovpn";
	if (!empty($proxy) && $proxy['proxy_authtype'] != "none") {
		$proxy['passwdfile'] = "{$prefix}-password";
		$pwdfle = "{$proxy['user']}\r\n";
		$pwdfle .= "{$proxy['password']}\r\n";
		file_put_contents("{$confdir}/{$proxy['passwdfile']}", $pwdfle);
	}
	$conf = openvpn_client_export_config($srvid, $usrid, $crtid, $useaddr, $quoteservercn, $usetoken,  $nokeys, $proxy, "", "baseconf", false, true, $openvpnmanager, $advancedoptions);
	if (!$conf) {
		$input_errors[] = "Could not create a config to export.";
		return false;
	}

	file_put_contents($cfgfile, $conf);

	$cafile = "{$tempdir}/config/{$prefix}-ca.crt";
	file_put_contents($cafile, base64_decode($server_ca['crt']));
	if ($settings['tls']) {
		$tlsfile = "{$tempdir}/config/{$prefix}-tls.key";
		file_put_contents($tlsfile, base64_decode($settings['tls']));
	}

	// write key files
	if ($settings['mode'] != "server_user") {
		$crtfile = "{$tempdir}/config/{$prefix}-{$user['name']}.crt";
		file_put_contents($crtfile, base64_decode($cert['crt']));
		$keyfile = "{$tempdir}/config/{$prefix}-{$user['name']}.key";
		file_put_contents($keyfile, base64_decode($cert['prv']));
		// convert to pkcs12 format
		$p12file = "{$tempdir}/config/{$prefix}.p12";
		if ($usetoken)
			openvpn_client_pem_to_pk12($p12file, $outpass, $crtfile, $keyfile);
		else
			openvpn_client_pem_to_pk12($p12file, $outpass, $crtfile, $keyfile, $cafile);
	}

	// 7zip the configuration data
	chdir($tempdir);
	$files  = "config ";
	if ($openvpnmanager)
		$files .= "OpenVPNManager ";
	//$files .= "procchain.exe ";
	$files .= "openvpn-install.exe ";
	$files .= "openvpn-postinstall.exe ";
	/*if ($usetoken)
		$files .= "procchain-import";
	else
		$files .= "procchain-standard";*/
	if ($usetoken)
		$procchain = 	';!@Install@!UTF-8!
RunProgram="openvpn-postinstall.exe /Import"
;!@InstallEnd@!'
;
	else
		$procchain = 	';!@Install@!UTF-8!
RunProgram="openvpn-postinstall.exe"
;!@InstallEnd@!'
;
	file_put_contents("{$tempdir}/7zipConfig",$procchain);
	//$files .= "procchain ";
	

	if(file_exists("/usr/pbi/p7zip-{$uname_p}/bin/7z"))
		exec("/usr/pbi/p7zip-{$uname_p}/bin/7z -y a archive.7z {$files}");
	else
		exec("/usr/local/libexec/p7zip/7z -y a archive.7z {$files}");

	// create the final installer
	$outfile = "{$tempdir}-install.exe";
	chdir($g['tmp_path']);
	//if ($usetoken)
		//exec("/bin/cat {$tempdir}/7zS.sfx {$tempdir}/config-import {$tempdir}/archive.7z > {$outfile}");
	//else
	exec("/bin/cat {$tempdir}/7zS.sfx {$tempdir}/7zipConfig {$tempdir}/archive.7z > {$outfile}");

	// cleanup
	//exec("/bin/rm -r {$tempdir}");

	return $outfile;
}

function viscosity_openvpn_client_config_exporter($srvid, $usrid, $crtid, $useaddr, $quoteservercn, $usetoken, $outpass, $proxy, $openvpnmanager, $advancedoptions) {
	global $config, $g;
	$uname_p = trim(exec("uname -p"));

	$ovpndir = "/usr/local/share/openvpn/";
	if (!file_exists($workdir . "/template/openvpn-install.exe"))
		openvpn_client_export_install();

	$uniq = uniqid();
	$tempdir = $g['tmp_path'] . "/openvpn-export-" . $uniq;
	$zipfile = $g['tmp_path'] . "/{$uniq}-Viscosity.visc.zip";

	$validconfig = openvpn_client_export_validate_config($srvid, $usrid, $crtid);
	if ($validconfig) {
		list($settings, $server_cert, $server_ca, $servercn, $user, $cert, $nokeys) = $validconfig;
	} else {
		return false;
	}

	// create template directory
	mkdir($tempdir, 0700, true);
	mkdir($tempdir . "/Viscosity.visc", 0700, true);

	// Append new Viscosity.visc directory on top
	$tempdir = $tempdir . "/Viscosity.visc/";

	// write cofiguration file
	$prefix = openvpn_client_export_prefix($srvid, $usrid);
	if (!empty($proxy) && $proxy['proxy_authtype'] != "none") {
		$proxy['passwdfile'] = "config-password";
		$pwdfle = "{$proxy['user']}\n";
		$pwdfle .= "{$proxy['password']}\n";
		file_put_contents("{$tempdir}/{$proxy['passwdfile']}", $pwdfle);
	}

	$conf = openvpn_client_export_config($srvid, $usrid, $crtid, $useaddr, $quoteservercn, $usetoken, true, $proxy, "baseconf", "", true, $openvpnmanager, $advancedoptions);
	if (!$conf)
		return false;

	// We need to nuke the ca line from the above config if it exists.
	$conf = explode("\n", $conf);
	for ($i=0; $i < count($conf); $i++) {
		if (substr($conf[$i], 0, 3) == "ca ")
			unset($conf[$i]);
	}
	$conf = implode("\n", $conf);

	$friendly_name = $settings['description'];
	$visc_settings = <<<EOF
#-- Config Auto Generated By pfSense for Viscosity --#

#viscosity startonopen false
#viscosity dhcp true
#viscosity dnssupport true
#viscosity name {$friendly_name}

EOF;

	$configfile = "{$tempdir}/config.conf";
	$conf .= "ca ca.crt\n";
	$conf .= "tls-auth ta.key 1\n";
	if ($settings['mode'] != "server_user") {
		$conf .= <<<EOF
cert cert.crt
key key.key
EOF;
	}

	file_put_contents($configfile, $visc_settings . "\n" . $conf);

	//	ca.crt		cert.crt	config.conf	key.key		ta.key

	// write ca
	$cafile = "{$tempdir}/ca.crt";
	file_put_contents($cafile, base64_decode($server_ca['crt']));

	if ($settings['mode'] != "server_user") {

		// write user .crt
		$crtfile = "{$tempdir}/cert.crt";
		file_put_contents($crtfile, base64_decode($cert['crt']));

		// write user .key
		if (!empty($outpass)) {
			$keyfile = "{$tempdir}/key.key";
			$clearkeyfile = "{$tempdir}/key-clear.key";
			file_put_contents($clearkeyfile, base64_decode($cert['prv']));
			$eoutpass = escapeshellarg($outpass);
			$ekeyfile = escapeshellarg($keyfile);
			$eclearkeyfile = escapeshellarg($clearkeyfile);
			exec("/usr/bin/openssl rsa -in ${eclearkeyfile} -out ${ekeyfile} -des3 -passout pass:${eoutpass}");
			unlink($clearkeyfile);
		} else {
			$keyfile = "{$tempdir}/key.key";
			file_put_contents($keyfile, base64_decode($cert['prv']));
		}
	}

	// TLS support?
	if ($settings['tls']) {
		$tlsfile = "{$tempdir}/ta.key";
		file_put_contents($tlsfile, base64_decode($settings['tls']));
	}

	// Zip Viscosity file
	if(file_exists("/usr/pbi/zip-{$uname_p}/bin/zip"))
		exec("cd {$tempdir}/.. && /usr/pbi/zip-{$uname_p}/bin/zip -r {$zipfile} Viscosity.visc");
	else
		exec("cd {$tempdir}/.. && /usr/local/bin/zip -r {$zipfile} Viscosity.visc");

	// Remove temporary directory
	exec("rm -rf {$tempdir}");

	return $zipfile;

}

function openvpn_client_export_sharedkey_config($srvid, $useaddr, $proxy, $zipconf = false) {
	global $config, $input_errors, $g;

	// lookup server settings
	$settings = $config['openvpn']['openvpn-server'][$srvid];
	if (empty($settings)) {
		$input_errors[] = "Could not locate server configuration.";
		return false;
	}
	if ($settings['disable']) {
		$input_errors[] = "You cannot export for disabled servers.";
		return false;
	}

	// determine basic variables
	if ($useaddr == "serveraddr") {
		$interface = $settings['interface'];
		if (!empty($settings['ipaddr']) && is_ipaddr($settings['ipaddr'])) {
			$server_host = $settings['ipaddr'];
		} else {
			if (!$interface)
				$interface = "wan";
			$server_host = get_interface_ip($interface);
		}
	} else if ($useaddr == "serverhostname" || empty($useaddr)) {
		$server_host = empty($config['system']['hostname']) ? "" : "{$config['system']['hostname']}.";
		$server_host .= "{$config['system']['domain']}";
	} else
		$server_host = $useaddr;

	$server_port = $settings['local_port'];
	$proto = (strtoupper($settings['protocol']) == 'UDP' ? 'udp' : "tcp-client");

	$cipher = $settings['crypto'];

	// add basic settings
	$conf  = "dev tun\n";
	if(! empty($settings['tunnel_networkv6'])) {
		$conf .= "tun-ipv6\n";
	}
	$conf .= "persist-tun\n";
	$conf .= "persist-key\n";
	$conf .= "proto {$proto}\n";
	$conf .= "cipher {$cipher}\n";
	$conf .= "client\n";
	$conf .= "resolv-retry infinite\n";
	$conf .= "remote {$server_host} {$server_port}\n";
	if ($settings['local_network']) {
		list($ip, $mask) = explode('/', $settings['local_network']);
		$mask = gen_subnet_mask($mask);
		$conf .= "route $ip $mask\n";
	}
	if (!empty($settings['tunnel_network'])) {
		list($ip, $mask) = explode('/', $settings['tunnel_network']);
		$mask = gen_subnet_mask($mask);
		$baselong = ip2long32($ip) & ip2long($mask);
		$ip1 = long2ip32($baselong + 1);
		$ip2 = long2ip32($baselong + 2);
		$conf .= "ifconfig $ip2 $ip1\n";
	}
	$conf .= "keepalive 10 60\n";
	$conf .= "ping-timer-rem\n";

	if (!empty($proxy)) {
		if ($proto == "udp") {
			$input_errors[] = "This server uses UDP protocol and cannot communicate with HTTP proxy.";
			return;
		}
		$conf .= "http-proxy {$proxy['ip']} {$proxy['port']} ";
		if ($proxy['proxy_authtype'] != "none") {
			if (!isset($proxy['passwdfile']))
				$proxy['passwdfile'] = openvpn_client_export_prefix($srvid) . "-proxy";
			$conf .= " {$proxy['passwdfile']} {$proxy['proxy_authtype']}";
		}
		$conf .= "\n";
	}

	// add key settings
	$prefix = openvpn_client_export_prefix($srvid);
	$shkeyfile = "{$prefix}.secret";
	$conf .= "secret {$shkeyfile}\n";

	// add optional settings
	if ($settings['compression'])
		$conf .= "comp-lzo\n";
	if ($settings['passtos'])
		$conf .= "passtos\n";

	if ($zipconf == true) {
		// create template directory
		$tempdir = "{$g['tmp_path']}/{$prefix}";
		mkdir($tempdir, 0700, true);

		file_put_contents("{$tempdir}/{$prefix}.ovpn", $conf);

		$shkeyfile = "{$tempdir}/{$shkeyfile}";
		file_put_contents("{$shkeyfile}", base64_decode($settings['shared_key']));

		if(file_exists("/usr/pbi/zip-{$uname_p}/bin/zip"))
			exec("cd {$tempdir}/.. && /usr/pbi/zip-{$uname_p}/bin/zip -r {$g['tmp_path']}/{$prefix}-config.zip {$prefix}");
		else
			exec("cd {$tempdir}/.. && /usr/local/bin/zip -r {$g['tmp_path']}/{$prefix}-config.zip {$prefix}");

		// Remove temporary directory
		exec("rm -rf {$tempdir}");
		return "{$prefix}-config.zip";
	} else
		return $conf;
}

?>
