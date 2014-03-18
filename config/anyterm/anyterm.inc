<?php
/*
    anyterm.inc
	pfSense package (http://www.pfSense.com)
	Copyright (C) 2009 Scott Ullrich <sullrich@pfsense.org>
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

function anyterm_deinstall() {
	global $g, $config;
	conf_mount_rw();
	exec("killall anytermd");
	exec("rm /usr/local/etc/rc.d/anyterm.sh");
	exec("rm /usr/local/sbin/anytermd");
}

function anyterm_install() {
	global $g, $config;

	conf_mount_rw();

	// Grab latest version of executablevi /
	$freebsdv=trim(`uname -r | cut -d'.' -f1`);
	`fetch -q -o /usr/local/sbin/ https://packages.pfsense.org/packages/config/anyterm/binaries{$freebsdv}/anytermd`;
	exec("chmod a+rx /usr/local/sbin/anytermd");

	if($config['installedpackages']['anyterm']['config'][0]['username']) 
		$port = " --port {$config['installedpackages']['anyterm']['config'][0]['port']}";

	// This will bring up the pfSense style menu
	$anytermd_command = "anytermd --user root --command '/etc/rc.initial' --auth trivial $port";

			$anyterm = <<<EOD
#!/bin/sh

# PROVIDE: anyterm
# REQUIRE: LOGIN
# KEYWORD: FreeBSD

. /etc/rc.subr

name="anyterm"
rcvar=`set_rcvar`
command="/usr/local/bin/anyterm"
anyterm_enable=\${anyterm-"YES"}

start_cmd="anyterm_start"
stop_postcmd="anyterm_stop"

load_rc_config \$name

anyterm_start () {
	if [ -f /var/etc/anyterm.conf ]; then
		if [ `ps awux | grep -v grep | grep anytermd | wc -l` -gt 0 ]; then
			killall anytermd 
		fi
		/usr/bin/env \
			PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
				/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDOFF
					<?php
						require_once("globals.inc");		
						require_once("functions.inc");
						require_once("anyterm.inc");
						anyterm_install();
					?>
ENDOFF

	fi
}

anyterm_stop () {
	echo "Stopping anyterm."
	killall anyterm
}

run_rc_command "\$1"

EOD;

	// Write out file
	$fd = fopen("/usr/local/etc/rc.d/anyterm.sh", "w");
	fwrite($fd, $anyterm);
	fclose($fd);
	// Make executable
	exec("chmod a+rx /usr/local/etc/rc.d/anyterm.sh");

	// Write out Anyterm configuration file (username and password for auth)
	$fd = fopen("/var/etc/anyterm.conf", "w");
	fwrite($fd, $config['installedpackages']['anyterm']['config'][0]['username'] . "\n");
	fwrite($fd, $config['installedpackages']['anyterm']['config'][0]['password'] . "\n");
	fclose($fd);
	exec("chmod a-r /var/etc/anyterm.conf ; chmod ug+r /var/etc/anyterm.conf ; killall anytermd");
	exec($anytermd_command);

}

?>