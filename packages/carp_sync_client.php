
<?php
/*
	carp_sync.php
        part of pfSense (www.pfSense.com)
	Copyright (C) 2005 Scott Ullrich (sullrich@gmail.com) and
        Colin Smith (ethethlay@gmail.com)
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

	TODO:
		* SSL support!

*/

require_once("xmlrpc_client.inc"); // Include client classes from our XMLRPC implementation.
require_once("xmlparse_pkg.inc");  // Include pfSense helper functions.
require_once("config.inc");
require_once("functions.inc");

function carp_sync_xml($url, $password, $section, $section_xml, $method = 'pfsense.restore_config_section') {
	$params = array(new XML_RPC_Value($password, 'string'),
			new XML_RPC_Value($section, 'string'),
			new XML_RPC_Value($section_xml, 'string'));
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client($url, '/xmlrpc.php');
	$cli->setCredentials('admin', $password);
	$resp = $cli->send($msg);
	return true;
}

if($already_processed != 1)
    if($config['installedpackages']['carpsettings']['config'] != "" and
      is_array($config['installedpackages']['carpsettings']['config'])) {
	$already_processed = 1;
	foreach($config['installedpackages']['carpsettings']['config'] as $carp) {
	    if($carp['synchronizetoip'] <> "" ) {
		$synchronizetoip = $carp['synchronizetoip'];
		if($carp['synchronizerules'] != "" and is_array($config['filter'])) {
		    $current_rules_section = backup_config_section("filter");
		    carp_sync_xml($carp['synchronizetoip'], $carp['password'], 'filter', $current_rules_section);
		}
		if($carp['synchronizenat'] != "" and is_array($config['nat'])) {
		    $current_nat_section = backup_config_section("nat");
		    carp_sync_xml($carp['synchronizetoip'], $carp['password'], 'nat', $current_nat_section);
		}
		if($carp['synchronizealiases'] != "" and is_array($config['aliases'])) {
		    $current_aliases_section = backup_config_section("aliases");
		    carp_sync_xml($carp['synchronizetoip'], $carp['password'], 'alias', $current_aliases_section);
		}
		if($carp['synchronizetrafficshaper'] != "" and is_array($config['shaper'])) {
		    $current_shaper_section = backup_config_section("shaper");
		    carp_sync_xml($carp['synchronizetoip'], $carp['password'], 'shaper', $current_shaper_section);
		}
        	$msg = new XML_RPC_Message('pfsense.filter_configure', array(new XML_RPC_Value($password, 'string')));
        	$cli = new XML_RPC_Client($url, '/xmlrpc.php');
        	$cli->setCredentials('admin', $carp['password']);
        	$cli->send($msg);
	    }
	}
    }
?>
