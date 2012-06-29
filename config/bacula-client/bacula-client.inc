<?php

/* ========================================================================== */
/*
    bacula-client.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2012 Marcio Carlos
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
    All rights reserved.
                                                                              */
/* ========================================================================== */
/*
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
/* ========================================================================== */
 require_once("config.inc");
 require_once("util.inc");
 
function baculaclient_custom_php_install_command(){
	global $g, $config;
    conf_mount_rw();
	unlink_if_exists("/usr/local/etc/rc.d/bacula-fd");
	$fd = fopen("/usr/local/etc/rc.d/bacula-client.sh", "w");
	if(!$fd) {
		log_error("Could not open /usr/local/etc/rc.d/bacula-client.sh for writing.");
		return;
	}
	
	// Ensure bacula-fd has a+rx
	exec("chmod a+rx /usr/local/etc/rc.d/bacula-client.sh");
	
	$baculafd = <<<EOD
#!/bin/sh
#
# $FreeBSD: ports/sysutils/bacula-server/files/bacula-fd.in,v 1.7 2012/01/14 08:56:57 dougb Exp $
#
# PROVIDE: bacula_fd
# REQUIRE: DAEMON
# KEYWORD: shutdown
#
# Add the following lines to /etc/rc.conf.local or /etc/rc.conf
# to enable this service:
#
# bacula_fd_enable  (bool):  Set to NO by default.
#               Set it to YES to enable bacula_fd.
# bacula_fd_flags (params):  Set params used to start bacula_fd.
#

. /etc/rc.subr

name="bacula_fd"
rcvar=bacula_fd_enable
command=/usr/local/sbin/bacula-fd

load_rc_config $name

: \${bacula_fd_enable="YES"}
: \${bacula_fd_flags=" -u root -g wheel -v -c /usr/local/etc/bacula-fd.conf"}
: \${bacula_fd_pidfile="/var/run/bacula-fd.9102.pid"}

pidfile="\${bacula_fd_pidfile}"

run_rc_command "\$1"

EOD;

	fwrite($fd, $baculafd);
	fclose($fd);
	conf_mount_ro();
	if (!is_array($config['installedpackages']['baculaclient'])) {
		$config['installedpackages']['baculaclient']['config'][0]['ports'] = "9102";
		$config['installedpackages']['baculaclient']['config'][0]['jobs'] = "20";
	}
}

function baculaclient_custom_php_deinstall_command(){
	global $g, $config;

	conf_mount_rw();

	// 1. Delete our config file
	unlink_if_exists("/usr/local/etc/bacula-fd.conf");

	// 2. Re-run sshd config generation script
	exec("/usr/local/etc/rc.d/bacula-client.sh stop");
	conf_mount_ro();
	
}

function baculaclient_custom_php_write_config(){
	global $g, $config;
	exec("/usr/local/etc/rc.d/bacula-client.sh stop");
	$fd = fopen("/usr/local/etc/bacula-fd.conf", "w");
	if(!$fd) {
		log_error("Could not open /usr/local/etc/bacula-fd.conf for writing.");
		return;
	}
	if (is_array($config['installedpackages']['baculaclient']['config'])){
		// Mount Read-write
		conf_mount_rw();
		// Make Head
		/*
		$baculaclient_conf = <<<EOD
		# Bacula File Daemon Configuration file
		#  For Bacula release 5.2.6 (21 February 2012) -- pfsense 2.0.x
		# There is not much to change here except perhaps the
		# List Directors who are permitted to contact this File daemon
		EOD;
		*/
		// Read config
		foreach ($config['installedpackages']['baculaclient']['config'][0]['row'] as $bc) {
				// create Director
				if (strcmp($bc['type'],"0")) {
					$baculaclient_conf .= "Director { \n\t Name = ".$bc['director']."-dir #".$bc['description']."\n\t Password = ".'"'.$bc['password'].'"'."\n}\n"; 
					}
				if (strcmp($bc['type'],"2")){
					$baculaclient_conf .= "Director { \n\t Name = ".$bc['director']."-mon #".$bc['description']."\n\t Password = ".'"'.$bc['password'].'"'."\n\t Monitor = yes\n}\n"; 
					}
				if (strcmp($bc['type'],"1")){
					$baculaclient_conf .= "Director { \n\t Name = ".$bc['director']."-dir #".$bc['description']."\n\t Password = ".'"'.$bc['password'].'"'."\n}\n"; 
					$baculaclient_conf .= "Director { \n\t Name = ".$bc['director']."-mon #".$bc['description']."\n\t Password = ".'"'.$bc['password'].'"'."\n\t Monitor = yes\n}\n"; 
					$LocalDirector = $bc['director'];
					}
		}
	
		// create Messages
		$baculaclient_conf .= "Messages { \n\t Name = Standard \n\t director  = ".$LocalDirector."-dir = all, !skipped, !restored\n\t \n}\n";
		// create FielDaemon
		
		$port = $config['installedpackages']['baculaclient']['config'][0]['port'];
		$jobs = $config['installedpackages']['baculaclient']['config'][0]['jobs'];
		$baculaclient_conf .= "FileDaemon { \n\t Name = ".$filedaemon."-fd #\n\t FDport = ".$port."\n\t WorkingDirectory = /var/db/bacula\n\t Pid Directory = /var/run\n\t Maximun Concurrent Jobs = ".$jobs."\n \n}\n";
		fwrite($fd, $baculaclient_conf);
		fclose($fd);	
	}
	
	exec("/usr/local/etc/rc.d/bacula-client.sh start");
}

	// Mount Read-only
	conf_mount_ro();
	

	?>