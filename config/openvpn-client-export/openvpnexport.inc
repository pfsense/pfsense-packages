<?php

global $priv_list;

$priv_list['page-openvpn-client-export'] = array();
$priv_list['page-openvpn-client-export']['name'] = "WebCfg - OpenVPN: Client Export Utility";
$priv_list['page-openvpn-client-export']['descr'] = "Allow access to the OpenVPN: Client Export Utility page.";
$priv_list['page-openvpn-client-export']['match'] = array();
$priv_list['page-openvpn-client-export']['match'][] = "vpn_openvpn_export.php*";

?>
