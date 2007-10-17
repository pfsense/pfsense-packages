<?php

/* $Id$ */
/*
	openbgpd.inc
    Copyright (C) 2007 Scott Ullrich (sullrich@gmail.com)
	part of pfSense
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

function openbgpd_install_conf() {
	global $config, $g;
	
	conf_mount_rw();
	
	$openbgpd_conf = &$config['installedpackages']['openbgpd']['config'][0];
	$openbgpd_rows = &$config['installedpackages']['openbgpd']['config'][0]['row'];
	$openbgpd_groups = &$config['installedpackages']['openbgpdgroups']['config'];
	$openbgpd_neighbors = &$config['installedpackages']['openbgpdneighbors']['config'];

	$fd = fopen("/usr/local/etc/bgpd.conf", "w");
	
	$conffile = "";
	
	// Setup AS #
	if($openbgpd_conf['asnum']) 
		$conffile .= "AS {$openbgpd_conf['asnum']}\n";
	
	// Setup holdtime if defined.  Default is 90.
	if($openbgpd_conf['holdtime']) 
		$conffile .= "holdtime {$openbgpd_conf['holdtime']}\n";

	// Specify listen ip
	if($openbgpd_conf['listenip']) 
		$conffile .= "listen on {$openbgpd_conf['listenip']}\n";

	// Specify router id
	if($openbgpd_conf['routerid']) 
		$conffile .= "router-id {$openbgpd_conf['routerid']}\n";

	// Handle advertised networks
	if($config['installedpackages']['openbgpd']['config'][0]['row'])
		if(is_array($openbgpd_rows))
			foreach($openbgpd_rows as $row) 
				$conffile .= "network {$row['networks']}\n";
	
	// Attach neighbors to their respective group owner
	if(is_array($openbgpd_groups)) {	
		foreach($openbgpd_groups as $group) {
			$conffile .= "group \"{$group['name']}\" {\n";
			$conffile .= "	remote-as {$group['remoteas']}\n";
			foreach($openbgpd_neighbors as $neighbor) {
				if($neighbor['groupname'] == $group['name']) {
					$conffile .= "	neighbor {$neighbor['neighbor']} {\n";
					$conffile .= "		descr \"{$neighbor['descr']}\"\n";
					foreach($neighbor['row'] as $row) {
						$conffile .= "		{$row['paramaters']} {$row['parmvalue']} \n";
					}		
					$conffile .= "	}\n";
				}
			}
			$conffile .= "}\n";
		}
	}	

	// Handle neighbors that do not have a group assigned to them	
	foreach($openbgpd_neighbors as $neighbor) {
		$used_this_item = false;
		if($neighbor['groupname'] == "") {
			$conffile .= "	neighbor {$neighbor['neighbor']} {\n";
			$conffile .= "		descr \"{$neighbor['descr']}\"\n";
			$used_this_item = true;
			foreach($neighbor['row'] as $row) {
				$conffile .= "		{$row['paramaters']} {$row['parmvalue']} \n";
			}
			if($used_this_item)		
				$conffile .= "	}\n";
		}
	}
	if($used_this_item)
		$conffile .= "}\n";
	
	// OpenBGPD filters
	$conffile .= "deny from any\n";
	$conffile .= "deny to any\n";
	foreach($openbgpd_neighbors as $neighbor) {
		$conffile .= "allow from {$neighbor['neighbor']}\n";
		$conffile .= "allow to {$neighbor['neighbor']}\n";	
	}
	
	// Write out the configuration file
	fwrite($fd, "# This file was created by the pfSense package manager.  Do not edit!\n\n");	
	fwrite($fd, $conffile);
	
	// Close file handle
	fclose($fd);
	
	// Create rc.d file
	$fd = fopen("/usr/local/etc/rc.d/bgpd.sh","w");
	fwrite($fd, "#!/bin/sh\n\n");
	fwrite($fd, "# This file was created by the pfSense package manager.  Do not edit!\n\n");	
	fwrite($fd, "bgpd\n");
	fclose($fd);
	exec("chmod a+rx /usr/local/etc/rc.d/bgpd.sh");
	exec("chmod a-rw /usr/local/etc/bgpd.conf");
	exec("chmod a+rw /usr/local/etc/bgpd.conf");
	
	// bgpd process running?  if so reload, elsewise start.
	if(is_openbgpd_running() == true) {
		exec("bgpctl reload");
	} else {
		exec("bgpd");
	}
	
	conf_mount_ro();
}

function deinstall_openbgpd() {
	exec("rm /usr/local/etc/rc.d/bgpd.sh");
	exec("rm /usr/local/www/openbgpd_status.php");
	exec("killall bgpd");
}

function check_group_usage($groupname) {
	global $config, $g;
	$openbgpd_conf = &$config['installedpackages']['openbgpd']['config'][0];
	$openbgpd_rows = &$config['installedpackages']['openbgpd']['config'][0]['row'];
	$openbgpd_groups = &$config['installedpackages']['openbgpdgroups']['config'];
	$openbgpd_neighbors = &$config['installedpackages']['openbgpdneighbors']['config'];
	if(is_array($openbgpd_groups)) {	
		foreach($openbgpd_groups as $group) {
			foreach($openbgpd_neighbors as $neighbor) {
				if($neighbor['groupname'] == $group['name']) 
					return $neighbor['groupname'];
			}
		}
	}
	return "";
}

function grey_out_value_boxes() {
	echo <<<EOF
	<script language="javascript">
		function grey_out_value_boxes() {
			var x = 0;
			for(x=0; x<99; x++) {
				if( \$('paramaters' + x) ) {
					var fieldvalue = $('paramaters' + x).options[$('paramaters' + x).selectedIndex].text;
					var length = fieldvalue.length;
					length = length -2;
					var last_two = fieldvalue.substring(length);
					var without_last_two = fieldvalue.substring(0,length);
					if( \$('parmvalue' + x) ) {
						if(last_two != ' X') {			
							\$('parmvalue' + x).value = '';
							\$('parmvalue' + x).disabled = true;
						} else {
							\$('parmvalue' + x).disabled = false;
						}
					}
				}
			}
			var timerID  = setTimeout("grey_out_value_boxes()", 1200);
			
		}	
		grey_out_value_boxes();
	</script>	

	
EOF;
		
}

function is_openbgpd_running() {
	$status = `ps awux | grep bgpd | grep "parent" | wc -l | awk '{ print \$1 }'`;
	if(intval($status) > 0) 
		return true;
	else 
		return false;
}

?>