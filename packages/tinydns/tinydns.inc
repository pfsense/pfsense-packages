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
	$fd = fopen("/usr/local/etc/rc.d/svscan.sh", "w");

	$svscan = <<<EOD
#!/bin/sh

# PROVIDE: svscan
# REQUIRE: LOGIN
# KEYWORD: FreeBSD

MINSEGMENT=10240
MAXSEGMENT=20480
MAXFILESIZE=10240
MAXFD=100
MAXCHILD=40

# ulimits
ulimit -d \${MINSEGMENT}
ulimit -f \${MAXFILESIZE}
ulimit -m \${MAXSEGMENT}
ulimit -n \${MAXFD}
ulimit -s \${MINSEGMENT}
ulimit -u \${MAXCHILD}

. /etc/rc.subr

name="svscan"
rcvar=`set_rcvar`
command="/usr/local/bin/svscan"
svscan_enable=\${svscan_enable-"NO"}
svscan_servicedir=\${svscan_servicedir-"/var/service"}

start_cmd="svscan_start"
stop_postcmd="svscan_stop_post"

load_rc_config \$name

required_dirs="\${svscan_servicedir}"

svscan_start () {
        echo "Starting svscan."
        /usr/bin/env \
        PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
        /usr/sbin/daemon -f /bin/sh -c "\$command \$svscan_servicedir 2>&1 | /usr/local/bin/readproctitle service errors: ................................................................................................................................................................................................................................................................................................................................................................................................................ &" > /dev/null
}

svscan_stop_post () {
        echo "Stopping svscan."
        find -L "\$svscan_servicedir" -mindepth 1 -maxdepth 2 -type d \( \! -path "\$svscan_servicedir/*/*" -or -name 'log' \) -print0 | xargs -0 /usr/local/bin/svc -dx
}

run_rc_command "\$1"
	
EOD;
	
	fwrite($fd, $svscan);
	fclose($fd);	
	conf_mount_ro();
}

function tinydns_custom_php_deinstall_command() {
	global $g, $config;
	conf_mount_rw();
	/* destroy all daemontools items */
	exec("/usr/sbin/pw groupdel Gtinydns");
	exec("/usr/sbin/pw groupdel Gdnslog");	
	exec("/usr/sbin/pw userdel Gtinydns");
	exec("/usr/sbin/pw userdel Gdnslog");
	conf_mount_ro();
}

function tinydns_custom_php_changeip_command() {
	global $g, $config;
	conf_mount_rw();
	$ip = $config['installedpackages']['tinydns']['config'][0]['ipaddress'];
	/* setup daemon tools service area */
	exec("/usr/sbin/pw useradd Gtinydns");
	exec("/usr/sbin/pw useradd Gdnslog");
	exec("/usr/local/bin/tinydns-conf Gtinydns Gdnslog /etc/tinydns {$ip}");
	exec("/bin/ln -s /etc/tinydns /service");
	sleep(5);
	exec("/usr/local/bin/svstat /service/tinydns");
	conf_mount_ro();
}

function tinydns_create_zone_file() {
	global $g, $config;
	conf_mount_rw();
	$fd = fopen("/service/tinydns/root/data", "w");	
	if($config['installedpackages']['tinydnsdomains'])
		foreach($config['installedpackages']['tinydnsdomains']['config'] as $domain) {
			$record_data = "";
			$hostname = $domain['hostname'];
			$ipaddress = $domain['ipaddress'];
			switch ($domain['recordtype']) {
				case "SOA":
					$record_data = ".{$hostname}::{$ipaddress}";
					break;
				case "MX":
					$record_data = "@{$hostname}:{$ipaddress}";
					break;
				case "A":
					$record_data = "+{$hostname}:{$ipaddress}";
					break;
				case "PTR":
					$record_data = "={$hostname}:{$ipaddress}";
					break;
				case "CNAME":
					$record_data = "C{$hostname}:{$ipaddress}";
					break;					
			}
			if($record_data)
				fwrite($fd, $record_data . "\n");
		}
	fclose($fd);
	/* tell tinydns to reload zone file */
	exec("cd /service/tinydns/root && /usr/local/bin/tinydns-data");
	conf_mount_ro();
}

?>