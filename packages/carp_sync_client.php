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

require_once("xmlrpc_client.inc"); /* Include client classes from our XMLRPC implementation. */
require_once("xmlparse_pkg.inc");  /* Include pfSense helper functions. */
require_once("config.inc");
require_once("functions.inc");

global $config;

if(!function_exists('carp_sync_xml')) {
	function carp_sync_xml($url, $password, $section, $section_xml, $method = 'pfsense.restore_config_section') {
		$params = array(new XML_RPC_Value($password, 'string'),
				new XML_RPC_Value($section, 'array'),
				new XML_RPC_Value($section_xml, 'array'));
		$msg = new XML_RPC_Message($method, $params);
		$cli = new XML_RPC_Client('/xmlrpc.php', $url);
		$cli->setCredentials('admin', $password);
		$resp = $cli->send($msg);
	}
}
if($already_processed != 1) {
    if($config['installedpackages']['carpsettings']['config'] != "" and
      is_array($config['installedpackages']['carpsettings']['config'])) {
	$already_processed = 1;
	foreach($config['installedpackages']['carpsettings']['config'] as $carp) {
	    if($carp['synchronizetoip'] != "" ) {
		$synchronizetoip = $carp['synchronizetoip'];
		$sections = array();
		$sections_xml = array();
		if($carp['synchronizerules'] != "" and is_array($config['filter'])) {
		    $sections_xml[] = new XML_RPC_Value(backup_config_section("filter"), 'string');
		    $sections[] = new XML_RPC_Value('filter', 'string');
		}
		if($carp['synchronizenat'] != "" and is_array($config['nat'])) {
		    $sections_xml[] = new XML_RPC_Value(backup_config_section("nat"), 'string');
                    $sections[] = new XML_RPC_Value('nat', 'string');
		}
		if($carp['synchronizealiases'] != "" and is_array($config['aliases'])) {
		    $sections_xml[] = new XML_RPC_Value(backup_config_section("aliases"), 'string');
                    $sections[] = new XML_RPC_Value('aliases', 'string');
		}
		if($carp['synchronizetrafficshaper'] != "" and is_array($config['shaper'])) {
		    $sections_xml[] = new XML_RPC_Value(backup_config_section("shaper"), 'string');
                    $sections[] = new XML_RPC_Value('shaper', 'string');
		}
		carp_sync_xml($synchronizetoip, $carp['password'], $sections, $sections_xml);
		$cli = new XML_RPC_Client('/xmlrpc.php', $synchronizetoip);
        	$msg = new XML_RPC_Message('pfsense.filter_configure', array(new XML_RPC_Value($carp['password'], 'string')));
        	$cli->setCredentials('admin', $carp['password']);
        	$cli->send($msg);
	    }
	}
    }
}
