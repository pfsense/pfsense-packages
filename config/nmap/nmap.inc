<?
/* $Id$ */
/*
	part of pfSense (https://www.pfsense.org/)

	Copyright (C) 2006 Bill Marquette - bill.marquette@gmail.com.
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

function nmap_custom_php_validation_command($post, $input_errors) {
	global $_POST, $savemsg, $config;
	if (empty($_POST['hostname'])) {
		$input_errors[] = gettext("You must enter an IP address to scan.");
	} elseif (!(is_ipaddr($_POST['hostname']) ||
			is_subnet($_POST['hostname']) ||
			is_hostname($_POST['hostname']))) {
		$input_errors[] = gettext("You must enter a valid IP address to scan.");
	}

	if(!empty($_POST['interface'])) {
		$interfaces = get_configured_interface_with_descr();
		if (!array_key_exists($_POST['interface'], $interfaces)) {
			$input_errors[] = gettext("Invalid interface.");
		}
	}
}

function nmap_custom_add_php_command() {
	$nmap_options = "";

	if (function_exists("is_ipaddrv6") && function_exists("is_subnetv6"))
		if (is_ipaddrv6($_POST['hostname']) || is_subnetv6($_POST['hostname']))
			$nmap_options .= " -6";

	switch($_POST['scanmethod']) {
		case 'syn':
			$nmap_options .= " -sS";
			break;
		case 'connect':
			$nmap_options .= " -sT";
			break;
		case 'icmp':
			$nmap_options .= " -sP";
			break;
		case 'udp':
			$nmap_options .= " -sU";
			break;
		case 'arp':
			$nmap_options .= " -sP -PR";
			break;
	}

	if($_POST['noping']) $nmap_options .= " -P0";
	if($_POST['servicever']) $nmap_options .= " -sV";
	if($_POST['osdetect']) $nmap_options .= " -O";
	if(!empty($_POST['interface'])) $nmap_options .= " -e " . get_real_interface($_POST['interface']);

	$nmap_options .= " " . escapeshellarg($_POST['hostname']);
	echo "Running: /usr/local/bin/nmap {$nmap_options}</br>";
	system("/usr/local/bin/nmap" . $nmap_options);
}

function nmap_get_interfaces() {
	global $config;
	$interfaces = get_configured_interface_with_descr();
	$nmap_ifs = array(array("name" => "Any", "value" => ""));
	foreach ($interfaces as $iface => $ifacename) {
		$tmp["name"]  = $ifacename;
		$tmp["value"] = $iface;
		$nmap_ifs[] = $tmp;
	}

	foreach (array('server', 'client') as $mode) {
		if (is_array($config['openvpn']["openvpn-{$mode}"])) {
			foreach ($config['openvpn']["openvpn-{$mode}"] as $id => $setting) {
				if (!isset($setting['disable'])) {
					$tmp["name"]  = gettext("OpenVPN") . " ".$mode.": ".htmlspecialchars($setting['description']);
					$tmp["value"] = 'ovpn' . substr($mode, 0, 1) . $setting['vpnid'];
					$nmap_ifs[] = $tmp;
				}
			}
		}
	}

	return $nmap_ifs;
}

?>
