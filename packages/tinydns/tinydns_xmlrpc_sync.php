<?php

/* $Id$ */
/*
	tinydns_xmlrcpc_sync.php
	Copyright (C) 2006 Scott Ullrich
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

/* NOTE: this file gets included from the pfSense filter.inc plugin process */

require_once("/usr/local/pkg/tinydns.inc");
require_once("service-utils.inc");

if(!$config) {
 	log_error("\$config is not enabled!!");
} else {
	if(!$g['booting'])
		tinydns_do_xmlrpc_sync();
}

function tinydns_do_xmlrpc_sync() {
	global $config, $g;
	$syncxmlrpc = $config['installedpackages']['tinydns']['config'][0]['syncxmlrpc'];
	/* option enabled? */
	if(!$syncxmlrpc)
		return;

	$password = $config['installedpackages']['carpsettings']['config'][0]['password'];

	if(!$config['installedpackages']['carpsettings']['config'][0]['synchronizetoip'])
		return;

	$sync_to_ip = $config['installedpackages']['carpsettings']['config'][0]['synchronizetoip'];

	log_error("[tinydns] tinydns_xmlrpc_sync.php is starting.");
	$xmlrpc_sync_neighbor = $sync_to_ip;
    if($config['system']['webgui']['protocol'] != "") {
		$synchronizetoip = $config['system']['webgui']['protocol'];
		$synchronizetoip .= "://";
    }
    $port = $config['system']['webgui']['port'];
    /* if port is empty lets rely on the protocol selection */
    if($port == "") {
		if($config['system']['webgui']['protocol'] == "http") {
			$port = "80";
		} else {
			$port = "443";
		}
    }
	$synchronizetoip .= $sync_to_ip;

	/* xml will hold the sections to sync */
	$xml = array();
	$xml['installedpackages']['tinydns'] = $config['installedpackages']['tinydns'];
	$xml['installedpackages']['tinydnsdomains'] = $config['installedpackages']['tinydnsdomains'];

	//print_r($xml);

	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	$method = 'pfsense.merge_config_section';

	/* Sync! */
	log_error("Beginning tinydns XMLRPC sync to {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 240 seconds */
	$resp = $cli->send($msg, "999");
	if(!$resp) {
		$error = "A communications error occured while attempting tinydns XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "tinydns Settings Sync", "");
	} elseif($resp->faultCode()) {
		$error = "An error code was received while attempting tinydns XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "tinydns Settings Sync", "");
	} else {
		log_error("tinydns XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	log_error("[tinydns] tinydns_xmlrpc_sync.php is ending.");
}

?>