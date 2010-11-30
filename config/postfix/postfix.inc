<?php
/*
	postfix.inc
	part of the Postfix package for pfSense
	Copyright (C) 2010 Erik Fonnesbeck
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

require_once("util.inc");

function sync_package_postfix() {
	global $config;

	$relay_domains = "";
	$transport = "";

	if (is_array($config['installedpackages']['postfix']['config'])) {
		foreach ($config['installedpackages']['postfix']['config'] as $postfix_config) {
			if (is_array($postfix_config['row'])) {
				foreach ($postfix_config['row'] as $postfix_row) {
					$relay_domains .= ' ' . $postfix_row['domain'];
					if (!empty($postfix_row['mailserverip']))
						$transport .= $postfix_row['domain'] . " smtp:[" . $postfix_row['mailserverip'] . "]\n";
				}
			}
		}
	}

	$postfix_main =
		"relay_domains ={$relay_domains}\n" .
		"transport_maps = hash:/usr/local/etc/postfix/transport\n" .
		"local_recipient_maps =\n" .
		"mydestination =\n" .
		"mynetworks_style = host\n";

	conf_mount_rw();

	log_error("Writing out configuration");
	file_put_contents("/usr/local/etc/postfix/main.cf", $postfix_main, LOCK_EX);
	file_put_contents("/usr/local/etc/postfix/transport", $transport, LOCK_EX);
	exec("/usr/local/sbin/postmap /usr/local/etc/postfix/transport");
	if (!is_dir("/etc/mail"))
		mkdir("/etc/mail", 0755);
	if (!file_exists("/etc/mail/aliases"))
		touch("/etc/mail/aliases");
	exec("/usr/local/bin/newaliases");

	$start = "/usr/local/sbin/postfix start\n";
	$stop = "/usr/local/sbin/postfix stop\n";
	log_error("Writing rc_file");		
	write_rcfile(array("file" => "postfix.sh", "start" => $start, "stop" => $stop));

	conf_mount_ro();

	log_error("Stopping postfix");
	mwexec("/usr/local/etc/rc.d/postfix.sh stop");
	sleep(1);
	log_error("Starting postfix");
	mwexec_bg("/usr/local/etc/rc.d/postfix.sh start");
	log_error("Postfix setup completed");	
}

function postfix_validate_input($post, &$input_errors) {
	foreach ($post as $key => $value) {
		if (empty($value))
			continue;
		if (substr($key, 0, 6) == "domain" && is_numeric(substr($key, 6))) {
			if (!is_domain($value))
				$input_errors[] = "{$value} is not a valid domain name.";
		} else if (substr($key, 0, 12) == "mailserverip" && is_numeric(substr($key, 12))) {
			if (empty($post['domain' . substr($key, 12)]))
				$input_errors[] = "Domain for {$value} cannot be blank.";
			if (!is_ipaddr($value) && !is_hostname($value))
				$input_errors[] = "{$value} is not a valid IP address or host name.";
		}
	}
}

function postfix_php_install_command() {
	sync_package_postfix();
}

function postfix_php_deinstall_command() {
	mwexec("/usr/local/etc/rc.d/postfix.sh stop");
	sleep(1);
	conf_mount_rw();
	unlink_if_exists("/usr/local/etc/rc.d/postfix.sh");
	conf_mount_ro();
}

?>