<?php
/*
    haproxy.inc
    Copyright (C) 2008 Remco Hoef
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

/* include all configuration functions */
require_once("functions.inc");
require_once("pkg-utils.inc");
require_once("notices.inc");

$d_haproxyconfdirty_path = $g['varrun_path'] . "/haproxy.conf.dirty";

function haproxy_custom_php_deinstall_command() {
	exec("rm /usr/local/sbin/haproxy");
	exec("rm /usr/local/pkg/haproxy.inc");
	exec("rm /usr/local/www/haproxy*");
}

function haproxy_custom_php_install_command() {
	global $g, $config;
	conf_mount_rw();

	$haproxy = <<<EOD
#!/bin/sh

# PROVIDE: haproxy
# REQUIRE: LOGIN
# KEYWORD: FreeBSD

. /etc/rc.subr

name="haproxy"
rcvar=`set_rcvar`
command="/usr/local/bin/haproxy"
haproxy_enable=\${haproxy-"YES"}

start_cmd="haproxy_start"
stop_postcmd="haproxy_stop_post"

load_rc_config \$name

haproxy_start () {
        echo "Starting haproxy."
        /usr/bin/env \
        PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
		/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDOFF
		<?php
			require_once("globals.inc");		
			require_once("functions.inc");
			require_once("haproxy.inc");
			function haproxy_configure();
		?>
ENDOFF
}

haproxy_stop_post () {
        echo "Stopping haproxy."
        killall haproxy
}

run_rc_command "\$1"

EOD;

	fwrite($fd, $haproxy);
	fclose($fd);
	conf_mount_ro();

	exec("/usr/local/etc/rc.d/haproxy.sh start");
}

function haproxy_configure() {
	global $config, $g;

	$a_global = &$config['installedpackages']['haproxy'];
	$a_backends = &$config['installedpackages']['haproxy']['ha_backends']['item'];
	$a_frontends = &$config['installedpackages']['haproxy']['ha_frontends']['item'];
	$a_servers = &$config['installedpackages']['haproxy']['ha_servers']['item'];

	$fd = fopen("{$g['varetc_path']}/haproxy.cfg", "w");

	if(is_array($a_global)) {
		fwrite ($fd, "global\n");
		fwrite ($fd, "\tmaxconn\t\t\t".$a_global['maxconn']."\n");
		fwrite ($fd, "\tlog\t\t\t127.0.0.1 local0\n");
		fwrite ($fd, "\tuid\t\t\t200\n");
		fwrite ($fd, "\tgid\t\t\t200\n");
		$numprocs = trim(`/sbin/sysctl kern.smp.cpus | cut -d" " -f2`);
		fwrite ($fd, "\tnbproc\t\t\t$numprocs\n");
		fwrite ($fd, "\tchroot\t\t\t/var/empty\n");
		fwrite ($fd, "\tdaemon\n");
		fwrite ($fd, "\n");
	}

	if(is_array($a_backends)) {
		foreach ($a_backends as $backend) {
			if($frontend['extaddr']=='any') 
				$listenip =  "listen {$backend['name']}\t\t\t0.0.0.0:".$backend['port']."\n";
			else if($frontend['extaddr']=='') 
				$listenip = "listen {$backend['name']}\t\t\t" . get_current_wan_address('wan').":" . $backend['port']."\n";
			else 
				$listenip =  "listen {$backend['name']}\t\t\t" . $backend['extaddr'] . ":" . $backend['port']."\n";

			fwrite ($fd, "{$listenip}");
            fwrite ($fd, "\tmode\t\t\t".$backend['type'] . "\n");
			fwrite ($fd, "\tlog\t\t\tglobal\n");
			fwrite ($fd, "\toption\t\t\tdontlognull\n");
			fwrite ($fd, "\toption\t\t\thttpclose\n");
			fwrite ($fd, "\toption\t\t\tforwardfor\n");
			
			if($backend['max_connections'])
				fwrite ($fd, "\tmaxconn\t\t\t" . $backend['max_connections'] . "\n");

			if($backend['client_timeout'])
				fwrite ($fd, "\tclitimeout\t\t" . $backend['client_timeout'] . "\n");

			if($backend['type'])
				fwrite ($fd, "\tmode\t\t\t" . $backend['type'] . "\n");

			if($backend['balance'])
				fwrite ($fd, "\tbalance\t\t\t" . $backend['balance'] . "\n");

			if($backend['connection_timeout'])
				fwrite ($fd, "\tcontimeout\t\t" . $backend['connection_timeout'] . "\n");

			if($backend['server_timeout'])
				fwrite ($fd, "\tsrvtimeout\t\t" . $backend['server_timeout'] . "\n");

			if($backend['retries'])
				fwrite ($fd, "\tretries\t\t\t" . $backend['retries'] . "\n");

			if($backend['cookie_name'])
				fwrite ($fd, "\tcookie\t\t\t" . $backend['cookie_name'] . " insert indirect\n");

			fwrite ($fd, "\toption\t\t\thttpchk HEAD " . $backend['monitor_uri'] . " HTTP/1.0\n");

			if($backend['stats_enabled']=='yes') {
				fwrite ($fd, "\tstats\t\t\tenable\n");
				if($backend['stats_uri'])
					fwrite ($fd, "\tstats\t\t\turi ".$backend['stats_uri']."\n");
				if($backend['stats_realm'])
					fwrite ($fd, "\tstats\t\t\trealm " . $backend['stats_realm'] . "\n");
				else 
					fwrite ($fd, "\tstats\t\t\trealm .\n");
				fwrite ($fd, "\tstats\t\t\tauth " . $backend['stats_username'].":". $backend['stats_password']."\n");
			}

			$a_acl=&$frontend['ha_acls']['item'];
			if(!is_array($a_acl)) 
				$a_acl=array();
			
			foreach ($a_acl as $acl) 
				fwrite ($fd, "\tacl\t\t\t".$acl['name']."\t\t".$acl['expression']."\n");

			if(is_array($a_servers)) {
				foreach ($a_servers as $server) {
					if($server['backend']==$backend['name']) {
						if($server['status']=='active') 
							fwrite ($fd, "\tserver\t\t\t".$server['name']." ".$server['address'].":".$server['port']." cookie ".$server['cookie']." check inter 1000 weight ".$server['weight']."\n");
					}
				}
			}
			fwrite ($fd, "\n");
		}
	}

	// create config file
	fclose($fd);

	// reload haproxy
	if(isset($a_global['enable'])) {
		if(is_process_running('haproxy')) {
			exec("/usr/local/sbin/haproxy -f /var/etc/haproxy.cfg -p /var/run/haproxy.pid -st `cat /var/run/haproxy.pid`");
		} else {
			exec("/usr/local/sbin/haproxy -f /var/etc/haproxy.cfg -p /var/run/haproxy.pid -D");
		}
		return (0);
	} else {
		return (1);
	}
}

?>