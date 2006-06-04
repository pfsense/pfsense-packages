<?php
/* $Id$ */
/*
	routed.inc
	Copyright (C) 2006 Bill Marquette
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

function setup_routed() {
	global $config;
	$gw = "";

	if ($config['installedpackages']['routed']['config'][0]['enable'] == "on") {
		/* if user selected individual interfaces */
		if (is_array($config['installedpackages']['routed']['config'][0]['interface_array'])) {
			$ifarr = $config['installedpackages']['routed']['config'][0]['interface_array'];
			foreach($ifarr as $iface) {
				$gw .= setup_etc_gateways($iface);
			}
		} else {
			/* setup for all interfaces */
			$gw = setup_etc_gateways();
		}
		$fd = fopen("/etc/gateways", "w");
		fwrite($fd, $gw);
		fclose($fd);
		restart_routed();
	} else {
		stop_routed();
	}
}

function setup_etc_gateways($iface="") {
	global $config;

	if ($iface != "") {
		$realif= convert_friendly_interface_to_real_interface_name($iface);
		$ret = "if={$realif} "; 	
	}
	if ($config['installedpackages']['routed']['config'][0]['ripversion'] == "2") {
		$ret .= "ripv2 ";
	}
	$passwd = $config['installedpackages']['routed']['config'][0]['passwd'];
	if ($passwd != "") {
		$ret .= "passwd={$passwd} ";
	}
	$ret .= "\n";

	return $ret;
}

function start_routed() {
	mwexec("/sbin/routed");
}

function stop_routed() {
	mwexec("killall routed");
}

function restart_routed() {
	stop_routed();
	start_routed();
}

?>
