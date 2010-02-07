<?php
/*
    haproxy.inc
	Copyright (C) 2009 Scott Ullrich <sullrich@pfsense.com>
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
stop_postcmd="haproxy_stop"

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
			haproxy_configure();
		?>
ENDOFF
}

haproxy_stop () {
        echo "Stopping haproxy."
        killall haproxy
}

run_rc_command "\$1"

EOD;

	$fd = fopen("/usr/local/etc/rc.d/haproxy.sh", "w");
	fwrite($fd, $haproxy);
	fclose($fd);
	exec("chmod a+rx /usr/local/etc/rc.d/haproxy.sh");

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
		if($a_global['advanced'])
			fwrite ($fd, "\t" . base64deode($a_global['advanced']) . "\n");
		fwrite ($fd, "\tmaxconn\t\t\t".$a_global['maxconn']."\n");
		if($a_global['remotesyslog'])
			fwrite ($fd, "\tlog\t\t\t{$a_global['remotesyslog']}\tlocal0\n");
		fwrite ($fd, "\tuid\t\t\t80\n");
		fwrite ($fd, "\tgid\t\t\t80\n");
		// Set numprocs if defined or use system default (#cores)
		if($a_global['nbproc'])
			$numprocs = $a_global['nbproc'];
		else 
			$numprocs = trim(`/sbin/sysctl kern.smp.cpus | cut -d" " -f2`);			
		fwrite ($fd, "\tnbproc\t\t\t$numprocs\n");
		fwrite ($fd, "\tchroot\t\t\t/var/empty\n");
		fwrite ($fd, "\tdaemon\n");
		fwrite ($fd, "\n");
	}

	// Construct and write out configuration file
	if(is_array($a_backends)) {
		foreach ($a_backends as $backend) {

			// Define our backend name
			$backendinfo = "listen {$backend['name']}\n";

			// Prepare ports for processing by splitting
			$portss = "{$backend['port']},";
			$ports = split(",", $portss);
			
			// Initialize variable
			$listenip = "";
			
			// Process and add bind directives for ports
			foreach($ports as $port) {
				if($port) {
					if($backend['extaddr'] == "any") 
						$listenip .=  "\tbind\t\t\t0.0.0.0:{$port}\n";
					elseif($backend['extaddr']) 
						$listenip .=  "\tbind\t\t\t{$backend['extaddr']}:{$port}\n";				
					else 
						$listenip .= "\tbind\t\t\t" . get_current_wan_address('wan') . ":{$port}\n";
				}
			}
			
			fwrite ($fd, "{$backendinfo}");
			fwrite ($fd, "{$listenip}");

			// Advanced pass thru
			if($backend['advanced']) {
				$advanced = base64_decode($backend['advanced']);
				fwrite($fd, "\t" . $advanced . "\n");
			}
			
			// https is an alias for tcp for clarity purpouses
			if(strtolower($backend['type']) == "https") {
				$backend_type = "tcp";
				$httpchk = "ssl-hello-chk";
			} else { 
				$backend_type = $backend['type'];
				$httpchk = "httpchk";
			}

            fwrite ($fd, "\tmode\t\t\t" . $backend_type . "\n");
			fwrite ($fd, "\tlog\t\t\tglobal\n");
			fwrite ($fd, "\toption\t\t\tdontlognull\n");

			if($backend['httpclose']) 
				fwrite ($fd, "\toption\t\t\thttpclose\n");

			if($backend['forwardfor']) 
				fwrite ($fd, "\toption\t\t\tforwardfor\n");

			if($backend['max_connections'])
				fwrite ($fd, "\tmaxconn\t\t\t" . $backend['max_connections'] . "\n");

			if($backend['client_timeout'])
				fwrite ($fd, "\tclitimeout\t\t" . $backend['client_timeout'] . "\n");

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

			fwrite ($fd, "\toption\t\t\t{$httpchk} HEAD " . $backend['monitor_uri'] . " HTTP/1.0\n");

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

			$a_acl=&$backend['ha_acls']['item'];
			if(!is_array($a_acl)) 
				$a_acl=array();
			
			foreach ($a_acl as $acl) {
				if ($acl['expression'] == "host_starts_with")
					$expr = 'hdr_beg(host) -i';
				else if ($acl['expression'] == "host_ends_with")
					$expr = 'hdr_end(host) -i';
				else if ($acl['expression'] == "host_matches")
					$expr = 'hdr(host) -i';
				else if ($acl['expression'] == "host_regex")
					$expr = 'hdr_reg(host) -i';
				else if ($acl['expression'] == "host_contains")
					$expr = 'hdr_dir(host) -i';
				else if ($acl['expression'] == "path_starts_with")
					$expr = 'path_beg -i';
				else if ($acl['expression'] == "path_ends_with")
					$expr = 'path_end -i';
				else if ($acl['expression'] == "path_matches")
					$expr = 'path -i';
				else if ($acl['expression'] == "path_regex")
					$expr = 'path_reg -i';
				else if ($acl['expression'] == "path_contains")
					$expr = 'path_dir -i';
				else if ($acl['expression'] == "source_ip")
					$expr = 'src';
				$expr .= " " . $acl['value'];	
				fwrite ($fd, "\tacl\t\t\t".$acl['name']."\t".$expr."\n");
			}

			$server['backend'] .= " ";
			if(is_array($a_servers)) {
				foreach ($a_servers as $server) {
					$backends_to_process = split(" ", $server['backend']);
					foreach($backends_to_process as $backends) {
						if($backends == "")
							continue;
						if($backends == $backend['name']) {
							$server_ports = array();
							if($server['status']=='active') {
								if($server['cookie']) 
									$cookie = " cookie {$server['cookie']} ";
								else 
									$cookie = "";
								if(!$server['port']) {
									foreach($ports as $port) {
										if($port) 
											$server_ports[] = $port;
									}
								} else {
									$server_ports[] = $server['port'];
								}
								if($server['advanced']) {
									$advanced = base64_decode($server['advanced']);
									$advanced_txt = " " . $advanced;
								} else {
									$advanced_txt = "";
								}
								if($server['checkinter']) 
									$checkinter = "check inter {$server['checkinter']}";
								else 
									$checkinter = "check inter 1000";
								foreach($server_ports as $pport) 
									fwrite ($fd, "\tserver\t\t\t" . $server['name'] . " " . $server['address'].":" . $pport . " $cookie " . " $checkinter weight " . $server['weight'] . "{$advanced_txt}\n");
							}
						}
					}
				}
			}
			fwrite ($fd, "\n");
		}
		// Sync HAProxy configuration (if enabled)
		if(isset($config['installedpackages']['haproxy']['enablesync'])) {
			if($config['installedpackages']['haproxy']['synchost1']) {
				haproxy_do_xmlrpc_sync($config['installedpackages']['haproxy']['synchost1'], 
					$config['installedpackages']['haproxy']['syncpassword']);
			}
			if($config['installedpackages']['haproxy']['synchost2']) {
				haproxy_do_xmlrpc_sync($config['installedpackages']['haproxy']['synchost2'], 
					$config['installedpackages']['haproxy']['syncpassword']);
			}
			if($config['installedpackages']['haproxy']['synchost3']) {
				haproxy_do_xmlrpc_sync($config['installedpackages']['haproxy']['synchost3'], 
					$config['installedpackages']['haproxy']['syncpassword']);				
			}
		}
	}

	// create config file
	fclose($fd);

	$freebsd_version = substr(trim(`uname -r`), 0, 1);
	if(!file_exists("/usr/bin/limits")) {
		exec("fetch -q -o /usr/bin/limits http://files.pfsense.org/extras/{$freebsd_version}/limits");
		exec("chmod a+rx /usr/bin/limits");
	}

	exec("/usr/bin/limits -n 300014");

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

function haproxy_do_xmlrpc_sync($sync_to_ip, $password) {
	global $config, $g;

	if(!$password)
		return;

	if(!$sync_to_ip)
		return;
	
	// Do not allow syncing to self.
	$donotsync = false;
	$lanip = find_interface_ip($config['interfaces']['lan']['if']);
	if($lanip == $sync_to_ip) 
		$donotsync = true;
	$wanip = find_interface_ip($config['interfaces']['wan']['if']);
	if($wanip == $sync_to_ip) 
		$donotsync = true;
	for ($j = 1; isset($config['interfaces']['opt' . $j]); $j++) {
		$optip = find_interface_ip($config['interfaces']['opt' . $j]['if']);
		if($optip == $sync_to_ip) 
		$donotsync = true;
	}
	if($donotsync) {
		log_error("Disallowing sync loop for HAProxy sync.");
		return;
	}

	$xmlrpc_sync_neighbor = $sync_to_ip;
    if($config['system']['webgui']['protocol'] != "") {
		$synchronizetoip = $config['system']['webgui']['protocol'];
		$synchronizetoip .= "://";
    }
    $port = $config['system']['webgui']['port'];
    /* if port is empty lets rely on the protocol selection */
    if($port == "") {
		if($config['system']['webgui']['protocol'] == "http") 
			$port = "80";
		else 
			$port = "443";
    }
	$synchronizetoip .= $sync_to_ip;

	/* xml will hold the sections to sync */
	$xml = array();
	$xml['haproxy'] = $config['installedpackages']['haproxy'];

	// Prevent sync loops
	unset($xml['synchost1']);
	unset($xml['synchost2']);
	unset($xml['synchost3']);
	unset($xml['syncpassword']);

	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Beginning HAProxy XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 250 seconds */
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting HAProxy XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "HAProxy Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting HAProxy XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "HAProxy Settings Sync", "");
	} else {
		log_error("HAProxy XMLRPC sync successfully completed with {$url}:{$port}.");
	}

	/* tell haproxy to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/haproxy.inc');\n";
	$execcmd .= "haproxy_configure();\n";
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);

	log_error("HAProxy XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting HAProxy XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "HAProxy Settings Reload", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting HAProxy XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "HAProxy Settings Sync", "");
	} else {
		log_error("HAProxy XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
}

?>
