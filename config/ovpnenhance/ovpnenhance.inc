<?php

function ovpnenhance_install() {
	global $g, $config;

mwexec("mv /usr/local/pkg/openvpn.inc_tls /etc/inc/openvpn.inc");
mwexec("ln -s /etc/inc/openvpn.inc /usr/local/pkg/openvpn.inc");
mwexec("mv /usr/local/pkg/openvpn.xml_tls /usr/local/pkg/openvpn.xml");
mwexec("mv /usr/local/pkg/openvpn_cli.xml_tls /usr/local/pkg/openvpn_cli.xml");
mwexec("mv /usr/local/pkg/openvpn_csc.xml_tls /usr/local/pkg/openvpn_csc.xml");
mwexec("rm /usr/local/pkg/ovpnenhance.inc");
}

?>