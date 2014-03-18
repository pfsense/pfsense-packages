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

$a_acltypes = array();
$a_acltypes[] = array('name' => 'host_starts_with', 'descr' => 'Host starts with',
				'mode' => 'http', 'syntax' => 'hdr_beg(host) -i');
$a_acltypes[] = array('name' => 'host_ends_with', 'descr' => 'Host ends with',
				'mode' =>'http', 'syntax' => 'hdr_end(host) -i');
$a_acltypes[] = array('name' => 'host_matches', 'descr' => 'Host matches',
				'mode' =>'http', 'syntax' => 'hdr(host) -i');
$a_acltypes[] = array('name' => 'host_regex', 'descr' => 'Host regex',
				'mode' =>'http', 'syntax' => 'hdr_reg(host) -i');
$a_acltypes[] = array('name' => 'host_contains', 'descr' => 'Host contains',
				'mode' => 'http', 'syntax' => 'hdr_dir(host) -i');
$a_acltypes[] = array('name' => 'path_starts_with', 'descr' => 'Path starts with',
				'mode' => 'http', 'syntax' => 'path_beg -i');
$a_acltypes[] = array('name' => 'path_ends_with', 'descr' => 'Path ends with',
				'mode' => 'http', 'syntax' => 'path_end -i');
$a_acltypes[] = array('name' => 'path_matches', 'descr' => 'Path matches',
				'mode' => 'http', 'syntax' => 'path -i');
$a_acltypes[] = array('name' => 'path_regex', 'descr' => 'Path regex',
				'mode' => 'http', 'syntax' => 'path_reg -i');
$a_acltypes[] = array('name' => 'path_contains', 'descr' => 'Path contains',
				'mode' => 'http', 'syntax' => 'path_dir -i');
$a_acltypes[] = array('name' => 'source_ip', 'descr' => 'Source IP',
				'mode' => '', 'syntax' => 'src');

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

function haproxy_find_acl($name) {
	global $a_acltypes;

	/* XXX why is this broken from xmlsync? */
	if (!$a_acltypes) {
		$a_acltypes = array();
		$a_acltypes[] = array('name' => 'host_starts_with', 'descr' => 'Host starts with',
			'mode' => 'http', 'syntax' => 'hdr_beg(host) -i');
		$a_acltypes[] = array('name' => 'host_ends_with', 'descr' => 'Host ends with',
			'mode' =>'http', 'syntax' => 'hdr_end(host) -i');
		$a_acltypes[] = array('name' => 'host_matches', 'descr' => 'Host matches',
			'mode' =>'http', 'syntax' => 'hdr(host) -i');
		$a_acltypes[] = array('name' => 'host_regex', 'descr' => 'Host regex',
			'mode' =>'http', 'syntax' => 'hdr_reg(host) -i');
		$a_acltypes[] = array('name' => 'host_contains', 'descr' => 'Host contains',
			'mode' => 'http', 'syntax' => 'hdr_dir(host) -i');
		$a_acltypes[] = array('name' => 'path_starts_with', 'descr' => 'Path starts with',
			'mode' => 'http', 'syntax' => 'path_beg -i');
		$a_acltypes[] = array('name' => 'path_ends_with', 'descr' => 'Path ends with',
			'mode' => 'http', 'syntax' => 'path_end -i');
		$a_acltypes[] = array('name' => 'path_matches', 'descr' => 'Path matches',
			'mode' => 'http', 'syntax' => 'path -i');
		$a_acltypes[] = array('name' => 'path_regex', 'descr' => 'Path regex',
			'mode' => 'http', 'syntax' => 'path_reg -i');
		$a_acltypes[] = array('name' => 'path_contains', 'descr' => 'Path contains',
			'mode' => 'http', 'syntax' => 'path_dir -i');
		$a_acltypes[] = array('name' => 'source_ip', 'descr' => 'Source IP',
			'mode' => '', 'syntax' => 'src');
	}

	if($a_acltypes) {
		foreach ($a_acltypes as $acl) {
			if ($acl['name'] == $name)
				return $acl;
		}
	}
}

function write_backend($fd, $name, $pool, $frontend) {
	if($pool['status'] != 'active' || !is_array($pool['ha_servers']['item']))
		return;

	fwrite ($fd, "backend " . $name . "\n");
	if($pool['cookie_name'] && strtolower($frontend['type']) == "http")
		fwrite ($fd, "\tcookie\t\t\t" . $pool['cookie_name'] . " insert indirect\n");

	// https is an alias for tcp for clarity purpouses
	if(strtolower($frontend['type']) == "https") {
		$backend_type = "tcp";
		$httpchk = "ssl-hello-chk";
	} else { 
		$backend_type = $frontend['type'];
		$httpchk = "httpchk";
	}

	fwrite ($fd, "\tmode\t\t\t" . $backend_type . "\n");

	if($frontend['balance'])
		fwrite ($fd, "\tbalance\t\t\t" . $frontend['balance'] . "\n");

	if($frontend['connection_timeout'])
		fwrite ($fd, "\tcontimeout\t\t" . $frontend['connection_timeout'] . "\n");

	if($frontend['server_timeout'])
		fwrite ($fd, "\tsrvtimeout\t\t" . $frontend['server_timeout'] . "\n");

	if($frontend['retries'])
		fwrite ($fd, "\tretries\t\t\t" . $frontend['retries'] . "\n");

	if($frontend['stats_enabled']=='yes') {
		fwrite ($fd, "\tstats\t\t\tenable\n");
		if($frontend['stats_uri'])
			fwrite ($fd, "\tstats\t\t\turi ".$frontend['stats_uri']."\n");
		if($frontend['stats_realm'])
			fwrite ($fd, "\tstats\t\t\trealm " . $frontend['stats_realm'] . "\n");
		else 
			fwrite ($fd, "\tstats\t\t\trealm .\n");
		fwrite ($fd, "\tstats\t\t\tauth " . $frontend['stats_username'].":". $frontend['stats_password']."\n");
	}

	$uri = $pool['monitor_uri'];
	if ($pool['monitor_uri'])
		$uri = $pool['monitor_uri'];
	else
		$uri = "/";
	fwrite ($fd, "\toption\t\t\t{$httpchk} HEAD " . $uri . " HTTP/1.0\n");

	if($pool['cookie']  && strtolower($frontend['type']) == "http") 
		$cookie = " cookie {$pool['cookie']} ";
	else 
		$cookie = "";
	if($pool['advanced']) {
		$advanced = base64_decode($pool['advanced']);
		$advanced_txt = " " . $advanced;
	} else {
		$advanced_txt = "";
	}
	if($pool['checkinter']) 
		$checkinter = "check inter {$pool['checkinter']}";
	else if (strtolower($frontend['type']) != "tcp")
		$checkinter = "check inter 1000";
	else
		$checkinter = "";

	$a_servers = &$pool['ha_servers']['item'];
	foreach($a_servers as $be) {
		if(!$be['port']) {
			// the listener can specify a default port
			$be['port'] = $frontend['svrport'];
		}
		if(!$be['port']) {
			// last resort, use the frontend port
			$ports = split(",", "{$frontend['port']},");
			$be['port'] = $ports[0];
		}
		if (!$be['name'])
			$be['name'] = $be['address'];
		if($be['backup']) {
			$isbackup = "backup";
		} else {
			$isbackup = "";
		}
		fwrite ($fd, "\tserver\t\t\t" . $be['name'] . " " . $be['address'].":" . $be['port'] . " $cookie " . " $checkinter $isbackup weight " . $be['weight'] . "{$advanced_txt}\n");
	}
	fwrite ($fd, "\n");
}

function haproxy_configure() {
	global $config, $g;

	$a_global = &$config['installedpackages']['haproxy'];
	$a_backends = &$config['installedpackages']['haproxy']['ha_backends']['item'];
	$a_pools = &$config['installedpackages']['haproxy']['ha_pools']['item'];

	$fd = fopen("{$g['varetc_path']}/haproxy.cfg", "w");

	if(is_array($a_global)) {
		fwrite ($fd, "global\n");
		if($a_global['advanced'])
			fwrite ($fd, "\t" . base64_decode($a_global['advanced']) . "\n");
		fwrite ($fd, "\tmaxconn\t\t\t".$a_global['maxconn']."\n");
		if($a_global['remotesyslog'])
			fwrite ($fd, "\tlog\t\t\t{$a_global['remotesyslog']}\t{$a_global['logfacility']}\t{$a_global['loglevel']}\n");
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

	// Try and get a unique array for address:port as frontends can duplicate
	$a_bind = array();
	if(is_array($a_backends)) {
		foreach ($a_backends as $backend) {
			if($backend['status'] != 'active')
				continue;
			if(!$backend['pool'])
				continue;

			$bname = $backend['extaddr'] . ":" . $backend['port'];
			if (!is_array($a_bind[$bname])) {
				$a_bind[$bname] = array();
				$a_bind[$bname]['config'] = array();
				// Settings which are constant for a merged frontend
				$a_bind[$bname]['name'] = $backend['name'];
				$a_bind[$bname]['defaultpool'] = $backend['pool'] . "_" . strtolower($backend['type']);
				$a_bind[$bname]['extaddr'] = $backend['extaddr'];
				$a_bind[$bname]['port'] = $backend['port'];
			}
			$b = &$a_bind[$bname];
			
			// Overwrite ?
			$b['type'] = $backend['type'];
			$b['forwardfor'] = $backend['forwardfor'];
			$b['httpclose'] = $backend['httpclose'];
			$b['max_connections'] = $backend['max_connections'];
			$b['client_timeout'] = $backend['client_timeout'];
			$b['advanced'] = $backend['advanced'];

			// pointer to each backend 
			$b['config'][] = $backend;
		}
	}
	
	$a_pendingpl = array();

	// Construct and write out configuration file
	if(is_array($a_bind)) {
		foreach ($a_bind as $bind) {
			if (count($bind['config']) > 1)
				$frontendinfo = "frontend {$bind['name']}-merged\n";
			else
				$frontendinfo = "frontend {$bind['name']}\n";

			// Prepare ports for processing by splitting
			$portss = "{$bind['port']},";
			$ports = split(",", $portss);
			
			// Initialize variable
			$listenip = "";
			
			// Process and add bind directives for ports
			foreach($ports as $port) {
				if($port) {
					if($bind['extaddr'] == "any") 
						$listenip .=  "\tbind\t\t\t0.0.0.0:{$port}\n";
					elseif($bind['extaddr']) 
						$listenip .=  "\tbind\t\t\t{$bind['extaddr']}:{$port}\n";				
					else 
						$listenip .= "\tbind\t\t\t" . get_current_wan_address('wan') . ":{$port}\n";
				}
			}
			
			fwrite ($fd, "{$frontendinfo}");
			fwrite ($fd, "{$listenip}");

			// Advanced pass thru
			if($bind['advanced']) {
				$advanced = base64_decode($bind['advanced']);
				fwrite($fd, "\t" . $advanced . "\n");
			}
			
			// https is an alias for tcp for clarity purpouses
			if(strtolower($bind['type']) == "https") {
				$backend_type = "tcp";
				$httpchk = "ssl-hello-chk";
			} else { 
				$backend_type = $bind['type'];
				$httpchk = "httpchk";
			}

			fwrite ($fd, "\tmode\t\t\t" . $backend_type . "\n");
			fwrite ($fd, "\tlog\t\t\tglobal\n");
			fwrite ($fd, "\toption\t\t\tdontlognull\n");

			if($bind['httpclose']) 
				fwrite ($fd, "\toption\t\t\thttpclose\n");

			if($bind['forwardfor']) 
				fwrite ($fd, "\toption\t\t\tforwardfor\n");

			if($bind['max_connections'])
				fwrite ($fd, "\tmaxconn\t\t\t" . $bind['max_connections'] . "\n");

			if($bind['client_timeout'])
				fwrite ($fd, "\tclitimeout\t\t" . $bind['client_timeout'] . "\n");

			fwrite ($fd, "\tdefault_backend\t\t" . $bind['defaultpool'] . "\n");
	
			// Combine the rest of the listener configs
			$i = 0;
			foreach ($bind['config'] as $bconfig) {
				$a_acl=&$bconfig['ha_acls']['item'];
				if(!is_array($a_acl)) 
					$a_acl=array();
			
				$poolname = $bconfig['pool'] . "_" . strtolower($bconfig['type']);
				if (!isset($a_pendingpl[$poolname])) {
					$a_pendingpl[$poolname] = array();
					$a_pendingpl[$poolname]['name'] = $poolname;
					$a_pendingpl[$poolname]['frontend'] = $bconfig;
				}

				foreach ($a_acl as $entry) {
					$acl = haproxy_find_acl($entry['expression']);
					if (!$acl)
						continue;

					// Filter out acls for different modes
					if ($acl['mode'] != '' && $acl['mode'] != strtolower($bind['type']))
						continue;

					if ($acl['syntax'] != '')
						$expr = $acl['syntax'] . " " . $entry['value'];
					else
						$expr = $entry['expression'] . " " . $entry['value'];

					$aclname = $i . "_" . $entry['name'];
					fwrite ($fd, "\tacl\t\t\t" . $aclname . "\t" . $expr . "\n");
					fwrite ($fd, "\tuse_backend\t\t" . $poolname . " if " . $aclname . "\n");
					$i++;
				}

			}
			fwrite ($fd, "\n");
		}
	}
	if (is_array($a_pendingpl) && is_array($a_pools)) {
		foreach ($a_pendingpl as $pending) {
			foreach ($a_pools as $pool) {
				if ($pending['frontend']['pool'] == $pool['name']) {
					write_backend($fd, $pending['name'], $pool, $pending['frontend']);
				}
			}
		}
	}
	fwrite ($fd, "\n");

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

	// create config file
	fclose($fd);

	$freebsd_version = substr(trim(`uname -r`), 0, 1);
	if(!file_exists("/usr/bin/limits")) {
		exec("fetch -q -o /usr/bin/limits https://files.pfsense.org/extras/{$freebsd_version}/limits");
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
