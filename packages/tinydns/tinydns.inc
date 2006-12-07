<?php

/* $Id$ */
/*
	tinydns.inc
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

function tinydns_custom_php_install_command() {
	global $g, $config;
	conf_mount_rw();
	/* setup daemon tools service area */
	exec("pw useradd Gtinydns");
	exec("pw useradd Gdnslog");
	conf_mount_ro();
}

function tinydns_custom_php_deinstall_command() {
	global $g, $config;
	conf_mount_rw();
	/* destroyh all daemontools items */
	exec("pw userdelete Gtinydns");
	exec("pw userdelete Gdnslog");
	conf_mount_ro();
}

function tinydns_custom_php_changeip_command() {
	global $g, $config;
	conf_mount_rw();
	$ip = $config['installedpackages']['tinydns']['ip'];
	exec("tinydns-conf Gtinydns Gdnslog /etc/tinydns {$ip}");
	exec("ln -s /etc/tinydns /service");
	sleep(5);
	exec("svstat /service/tinydns");
	conf_mount_ro();
}

function tinydns_create_zone_file() {
	$fd = fopen("/service/tinydns/root/data", "w");	
	global $g, $config;
	conf_mount_rw();

	conf_mount_ro();
}

?>