<?php
/*
	openvpn-client-export.inc
	Copyright (C) 2008 Shrew Soft Inc
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

function openvpn_client_export_install() {
	$tarpath = "/tmp/openvpn-client-export.tgz";
	$phpfile = "vpn_openvpn_export.php";
	$ovpndir = "/usr/local/share/openvpn";
	$workdir = "{$ovpndir}/client-export";

	if(!is_dir("/usr/local/share/openvpn"))
		mkdir("/usr/local/share/openvpn");

	exec("/usr/bin/tar zxf {$tarpath} -C {$ovpndir}");
	unlink($tarpath);
	rename("{$workdir}/{$phpfile}", "/usr/local/www/{$phpfile}");
}

function openvpn_client_export_deinstall() {
	$phpfile = "vpn_openvpn_export.php";
	$ovpndir = "/usr/local/share/openvpn";
	$workdir = "{$ovpndir}/client-export";

	unlink("/usr/local/www/{$phpfile}");
	exec("/bin/rm -r {$workdir}");
}

function openvpn_client_export_prefix($srvid) {
	global $config;

	// lookup server settings
	$settings = $config['openvpn']['openvpn-server'][$srvid];
	if (empty($settings))
		return false;
	if ($settings['disable'])
		return false;

	$host = $config['system']['hostname'];
	$prot = ($settings['protocol'] == 'UDP' ? 'udp' : "tcp-{$mode}");
	$port = $settings['local_port'];

	return "{$host}-{$prot}-{$port}";
}

function openvpn_client_pem_to_pk12($outpath, $outpass, $crtpath, $keypath, $capath = false) {

	if ($capath)
		exec("/usr/bin/openssl pkcs12 -export -in {$crtpath} -inkey {$keypath} -certfile {$capath} -out {$outpath} -passout pass:{$outpass}");
	else
		exec("/usr/bin/openssl pkcs12 -export -in {$crtpath} -inkey {$keypath} -out {$outpath} -passout pass:{$outpass}");

	unlink($crtpath);
	unlink($keypath);
	if ($capath)
		unlink($capath);
}

function openvpn_client_export_config($srvid, $usrid, $crtid, $useaddr, $usetoken) {
	global $config;

	// lookup server settings
	$settings = $config['openvpn']['openvpn-server'][$srvid];
	if (empty($settings))
		return false;
	if ($settings['disable'])
		return false;

	// lookup server certificate info
	$server_cert =& lookup_cert($settings['certref']);
	$server_ca =& lookup_ca($server_cert['caref']);
	if (!$server_cert || !$server_ca)
		return false;

	// lookup user info
	$user =& $config['system']['user'][$usrid];
	if (!$user)
		return false;

	// determine basic variables
	if ($useaddr) {
		$interface = $settings['interface'];
		if (!$interface)
			$interface = 'WAN';
		$iface = convert_friendly_interface_to_real_interface_name($interface);
		$lines = explode(' ', trim(shell_exec("ifconfig {$iface} | grep inet | grep -v inet6")));
		$server_host = $lines[1];
	} else
		$server_host = "{$config['system']['hostname']}.{$config['system']['domain']}";
	$server_port = $settings['local_port'];
	$proto = ($settings['protocol'] == 'UDP' ? 'udp' : "tcp-client");
	$cipher = $settings['crypto'];

	// add basic settings
	$conf  = "dev tun\n";
	$conf .= "persist-tun\n";
	$conf .= "persist-key\n";
	$conf .= "proto {$proto}\n";
	$conf .= "cipher {$cipher}\n";
	$conf .= "tls-client\n";
	$conf .= "client\n";
	$conf .= "resolv-retry infinite\n";
	$conf .= "remote {$server_host} {$server_port}\n";

	// add user auth settings
	switch($settings['mode']) {
		case 'server_user':
		case 'server_tls_user':
			$conf .= "auth-user-pass\n";
			break;
	}

	// add key settings
	$prefix = openvpn_client_export_prefix($srvid);
	if ($usetoken) {
		$conf .= "ca {$prefix}-ca.crt\n";
		$conf .= "cryptoapicert \"SUBJ:{$user['name']}\"\n";
	} else {
		$conf .= "pkcs12 {$prefix}.p12\n";
	}

	if ($settings['tls'])
		$conf .= "tls-auth {$prefix}-tls.key\n";

	// add optional settings
	if ($settings['compression'])
		$conf .= "comp-lzo\n";
	if ($settings['passtos'])
		$conf .= "passtos\n";

	return $conf;
}

function openvpn_client_export_installer($srvid, $usrid, $crtid, $useaddr, $usetoken, $outpass) {
	global $config, $g;

	$ovpndir = "/usr/local/share/openvpn";
	$workdir = "{$ovpndir}/client-export";

	// lookup server settings
	$settings = $config['openvpn']['openvpn-server'][$srvid];
	if (empty($settings))
		return false;
	if ($settings['disable'])
		return false;

	// lookup server certificate info
	$server_cert =& lookup_cert($settings['certref']);
	$server_ca =& lookup_ca($server_cert['caref']);
	if (!$server_cert || !$server_ca)
		return false;

	// lookup user info
	$user =& $config['system']['user'][$usrid];
	if (!$user)
		return false;

	// lookup user certificate info
	$cert =& $user['cert'][$crtid];
	if (!$cert)
		return false;

	// create temp config directory
	$tempdir = $g['tmp_path']."/openvpn-export-".uniqid();
	mkdir($tempdir, 0700, true);

	// copy the template directory
	exec("cp -r {$workdir}/template/* {$tempdir}");

	// write cofiguration file
	$prefix = openvpn_client_export_prefix($srvid);
	$cfgfile = "{$tempdir}/config/{$prefix}-config.ovpn";
	$conf = openvpn_client_export_config($srvid, $usrid, $crtid, $useaddr, $usetoken);
	if (!$conf)
		return false;
	file_put_contents($cfgfile, $conf);

	// write key files
	$cafile = "{$tempdir}/config/{$prefix}-ca.crt";
	file_put_contents($cafile, base64_decode($server_ca['crt']));
	$crtfile = "{$tempdir}/config/{$prefix}-{$user['name']}.crt";
	file_put_contents($crtfile, base64_decode($cert['crt']));
	$keyfile = "{$tempdir}/config/{$prefix}-{$user['name']}.key";
	file_put_contents($keyfile, base64_decode($cert['prv']));
	if ($settings['tls']) {
		$tlsfile = "{$tempdir}/config/{$prefix}-tls.key\n";
		file_put_contents($tlsfile, base64_decode($settings['tls']));
	}

	// convert to pkcs12 format
	$p12file = "{$tempdir}/config/{$prefix}.p12";
	if ($usetoken)
		openvpn_client_pem_to_pk12($p12file, $outpass, $crtfile, $keyfile);
	else
		openvpn_client_pem_to_pk12($p12file, $outpass, $crtfile, $keyfile, $cafile);

	// 7zip the configuration data
	chdir($tempdir);
	$files  = "config ";
	$files .= "procchain.exe ";
	$files .= "openvpn-install.exe ";
	$files .= "openvpn-postinstall.exe ";
	if ($usetoken)
		$files .= "procchain-import";
	else
		$files .= "procchain-standard";
	exec("/usr/local/libexec/p7zip/7z -y a archive.7z {$files}");

	// create the final installer
	$outfile = "{$tempdir}-install.exe";
	chdir($g['tmp_path']);
	if ($usetoken)
		exec("/bin/cat {$tempdir}/7zS.sfx {$tempdir}/config-import {$tempdir}/archive.7z > {$outfile}");
	else
		exec("/bin/cat {$tempdir}/7zS.sfx {$tempdir}/config-standard {$tempdir}/archive.7z > {$outfile}");

	// cleanup
	exec("/bin/rm -r {$tempdir}");

	return $outfile;	
}

?>
