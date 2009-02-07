<?php

/* $Id$ */
/*
	frickin.inc
	Copyright (C) 2006 Scott Ullrich
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

function frickin_custom_php_install_command() {
		global $config;
		if(file_exists("/tmp/frickin.tgz")) {
			exec("tar xzPf /tmp/frickin.tgz -C /");
			unlink("/tmp/frickin.tgz");
		}
		$fout = fopen("/usr/local/etc/rc.d/frickin.sh", "w");
		fwrite($fout, "#!/bin/sh\n# This package was automatically generated\n# by the pfSense package system.\n\n");
		fwrite($fout, "# PACKAGE: Frickin Proxy\n");
		fwrite($fout, "# EXECUTABLE: frickin\n");
		fwrite($fout, "/usr/local/sbin/frickin2");
		fwrite($fout, " -c /usr/local/etc/frickin.conf");
		fwrite($fout, " &\n");
		fclose($fout);

		$fout = fopen("/usr/local/etc/frickin.conf", "w");
		$frickin_conf = <<<EOD

proxy {
    user = "nobody";
    group = "nobody";
    client {
	queue = 5;	    # Maximum connection queue size
	max = 100;	    # Maximum number of concurrent clients
	multiple = FALSE;   # Allow multiple connections from same ip
    }
    session {
	connect = 15;	# Connect timout (seconds)
	idle = 60;	# Idle timeout (seconds)
    }
}

listeners {
    default {
	name = "Frickin PPTP Proxy";
	vendor = "Placid Development";
	ip = "127.0.0.1";
	port = 1723;
    }
}

EOD;

		fwrite($fout, "{$frickin_conf}");
		fclose($fout);

		mwexec("/usr/bin/killall frickin");
		mwexec("chmod a+rx /usr/local/etc/rc.d/frickin.sh");
		mwexec("/usr/local/etc/rc.d/frickin.sh");

//		file_notice("System -> Advanced -> Scrub", "pfSenses scrub option has been disabled.  See http://www.openbsd.org/faq/pf/scrub.html for more info.", "Scrub");
//		$config['system']['scrubnodf'] = "enabled";
//		write_config();

		filter_configure();
}

function frickin_custom_php_deinstall_command() {
		global $config;
		
//		unset($config['system']['scrubnodf']);
//		file_notice("System -> Advanced -> Scrub", "pfSenses scrub option has been enabled.  See http://www.openbsd.org/faq/pf/scrub.html for more info.", "Scrub");
//		write_config();
		
	    unlink_if_exists("/usr/local/etc/rc.d/frickin.sh");
	    unlink_if_exists("/usr/local/bin/frickin");
	    
	    mwexec("killall frickin");
	    
	    filter_configure();
}

function frickin_generate_rules($type) {
	global $config;

	$rules = "";

	$frickin_conf = $config['installedpackages']['frickin']['config'][0];
	if (!is_process_running('frickin')) {
		log_error("Frickin is installed but not started.  Not installing redirect rules.");
		return;
	}

	$ifaces = explode(',', $frickin_conf['active_interface']);
	$ifaces = array_map('convert_friendly_interface_to_real_interface_name', $ifaces);

	switch($type) {
		case 'nat':
			foreach ($ifaces as $iface) {
				if(!$iface) {
					//log_error("Not installing frickin pptp proxy rule.  Please set it up");
					//continue;
				}
				$rules .= "rdr on $iface proto tcp from any to any port = 1723 -> 127.0.0.1\n";
				$rules .= "rdr on $iface inet proto gre all -> 127.0.0.1\n";
			}
			break;
		case 'filter':
			$ext_if = get_real_wan_interface();
			$rules .= "pass out on $ext_if proto gre from any to any keep state\n";
			$rules .= "pass in on $ext_if proto gre from any to any keep state\n";
			break;
		default:
			break;
	}

	return $rules;
}

?>