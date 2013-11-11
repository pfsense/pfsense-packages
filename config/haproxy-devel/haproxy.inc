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
require_once("haproxy_xmlrpcsyncclient.inc");

$d_haproxyconfdirty_path = $g['varrun_path'] . "/haproxy.conf.dirty";

$a_acltypes = array();
$a_acltypes[] = array('name' => 'host_starts_with', 'descr' => 'Host starts with',
				'mode' => 'http', 'syntax' => 'hdr_beg(host) -i %1$s');
$a_acltypes[] = array('name' => 'host_ends_with', 'descr' => 'Host ends with',
				'mode' =>'http', 'syntax' => 'hdr_end(host) -i %1$s');
$a_acltypes[] = array('name' => 'host_matches', 'descr' => 'Host matches',
				'mode' =>'http', 'syntax' => 'hdr(host) -i %1$s');
$a_acltypes[] = array('name' => 'host_regex', 'descr' => 'Host regex',
				'mode' =>'http', 'syntax' => 'hdr_reg(host) -i %1$s');
$a_acltypes[] = array('name' => 'host_contains', 'descr' => 'Host contains',
				'mode' => 'http', 'syntax' => 'hdr_dir(host) -i %1$s');
$a_acltypes[] = array('name' => 'path_starts_with', 'descr' => 'Path starts with',
				'mode' => 'http', 'syntax' => 'path_beg -i %1$s');
$a_acltypes[] = array('name' => 'path_ends_with', 'descr' => 'Path ends with',
				'mode' => 'http', 'syntax' => 'path_end -i %1$s');
$a_acltypes[] = array('name' => 'path_matches', 'descr' => 'Path matches',
				'mode' => 'http', 'syntax' => 'path -i %1$s');
$a_acltypes[] = array('name' => 'path_regex', 'descr' => 'Path regex',
				'mode' => 'http', 'syntax' => 'path_reg -i %1$s');
$a_acltypes[] = array('name' => 'path_contains', 'descr' => 'Path contains',
				'mode' => 'http', 'syntax' => 'path_dir -i %1$s');
$a_acltypes[] = array('name' => 'source_ip', 'descr' => 'Source IP',
				'mode' => '', 'syntax' => 'src %1$s');
$a_acltypes[] = array('name' => 'backendservercount', 'descr' => 'Minimum count usable servers',
				'mode' => '', 'syntax' => 'nbsrv(%2$s) ge %1$d', 'parameters' => 'value,backendname');
// 'ssl_sni_matches' was added in HAProxy1.5dev17
$a_acltypes[] = array('name' => 'ssl_sni_matches',  'descr' => 'Server Name Indication TLS extension matches', 
				'mode' => 'https', 'syntax' => 'req_ssl_sni -i %1$s', 'advancedoptions' => "tcp-request inspect-delay 5s\n\ttcp-request content accept if { req_ssl_hello_type 1 }");

$a_checktypes['none'] = array('name' => 'none', 'syntax' => '',
	'descr' => 'No health checks will be performed.');
$a_checktypes['Basic'] = array('name' => 'Basic', 'syntax' => '',
	'descr' => 'Basic socket connection check');
$a_checktypes['HTTP'] = array('name' => 'HTTP', 'syntax' => 'httpchk',
	'descr' => 'HTTP protocol to check on the servers health, can also be used for HTTPS servers(requirs checking the SSL box for the servers).', 'parameters' => "uri,method,version");
// 'Agent' was added in HAProxy1.5dev18
$a_checktypes['Agent'] = array('name' => 'Agent', 'syntax' => 'lb-agent-chk', 'usedifferenport' => 'yes', 
	'descr' => 'Use a TCP connection to read an ASCII string of the form 100%,75%,drain,down (others in haproxy manual)');
$a_checktypes['LDAP'] = array('name' => 'LDAP', 'syntax' => 'ldap-check',
	'descr' => 'Use LDAPv3 health checks for server testing');
$a_checktypes['MySQL'] = array('name' => 'MySQL', 'syntax' => 'mysql-check',
	'descr' => 'Use MySQL health checks for server testing', 'parameters' => 'username');
$a_checktypes['PostgreSQL'] = array('name' => 'PostgreSQL', 'syntax' => 'pgsql-check',
	'descr' => 'Use PostgreSQL health checks for server testing', 'parameters' => 'username');
$a_checktypes['Redis'] = array('name' => 'Redis', 'syntax' => 'redis-check',
	'descr' => 'Test that the server correctly talks REDIS protocol.');
$a_checktypes['SMTP'] = array('name' => 'SMTP', 'syntax' => 'smtpchk HELO',
	'descr' => 'Use SMTP HELO health checks for server testing', 'parameters' => 'domain');
$a_checktypes['ESMTP'] = array('name' => 'ESMTP', 'syntax' => 'smtpchk EHLO',
	'descr' => 'Use ESMTP EHLO health checks for server testing', 'parameters' => 'domain');
$a_checktypes['SSL'] = array('name' => 'SSL', 'syntax' => 'ssl-hello-chk',
	'descr' => 'Use SSLv3 client hello health checks for server testing.');

$a_httpcheck_method['OPTIONS'] = array('name' => 'OPTIONS', 'syntax' => 'OPTIONS');
$a_httpcheck_method['HEAD']    = array('name' => 'HEAD', 'syntax' => 'HEAD');
$a_httpcheck_method['GET']     = array('name' => 'GET', 'syntax' => 'GET');
$a_httpcheck_method['POST']    = array('name' => 'POST', 'syntax' => 'POST');
$a_httpcheck_method['PUT']     = array('name' => 'PUT', 'syntax' => 'PUT');
$a_httpcheck_method['DELETE']  = array('name' => 'DELETE', 'syntax' => 'DELETE');
$a_httpcheck_method['TRACE']   = array('name' => 'TRACE', 'syntax' => 'TRACE');

$a_closetypes['none'] = array('name' => 'none', 'syntax' => '',
	'descr' => 'No close headers will be changed.');
$a_closetypes['httpclose'] = array('name' => 'httpclose', 'syntax' => 'httpclose',
	'descr' => 'The "httpclose" option removes any "Connection" header both ways, and adds a "Connection: close" header in each direction. This makes it easier to disable HTTP keep-alive than the previous 4-rules block.');
$a_closetypes['http-server-close'] = array('name' => 'http-server-close', 'syntax' => 'http-server-close',
	'descr' => 'By default, when a client communicates with a server, HAProxy will only analyze, log, and process the first request of each connection. Setting "option http-server-close" enables HTTP connection-close mode on the server side while keeping the ability to support HTTP keep-alive and pipelining on the client side.  This provides the lowest latency on the client side (slow network) and the fastest session reuse on the server side to save server resources.');
$a_closetypes['forceclose'] = array('name' => 'forceclose', 'syntax' => 'forceclose',
	'descr' => 'Some HTTP servers do not necessarily close the connections when they receive the "Connection: close" set by "option httpclose", and if the client does not close either, then the connection remains open till the timeout expires. This causes high number of simultaneous connections on the servers and shows high global session times in the logs. Note that this option also enables the parsing of the full request and response, which means we can close the connection to the server very quickly, releasing some resources earlier than with httpclose.');

function haproxy_custom_php_deinstall_command() {
	exec("cd /var/db/pkg && pkg_delete `ls | grep haproxy`");
	exec("rm /usr/local/pkg/haproxy.inc");
	exec("rm /usr/local/www/haproxy*");
	exec("rm /usr/local/etc/rc.d/haproxy.sh");
	exec("rm /etc/devd/haproxy.conf");
	exec("/etc/rc.d/devd restart");
	haproxy_install_cron(false);
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
check_cmd="haproxy_check"
extra_commands="check"

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

haproxy_check () {
        echo "Checking haproxy."
        /usr/bin/env \
        PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
		/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDOFF
		<?php
			require_once("globals.inc");		
			require_once("functions.inc");
			require_once("haproxy.inc");
			haproxy_check_run(0);
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

	$devd = <<<EOD
notify 0 {
        match "system"          "IFNET";
        match "subsystem"       "carp[0-9]+";
        match "type"            "LINK_UP";
        action "/usr/local/etc/rc.d/haproxy.sh check";
};
notify 0 {
        match "system"          "IFNET";
        match "subsystem"       "carp[0-9]+";
        match "type"            "LINK_DOWN";
        action "/usr/local/etc/rc.d/haproxy.sh check";
};

EOD;
	exec("mkdir -p /etc/devd");
	$fd = fopen("/etc/devd/haproxy.conf", "w");
	fwrite($fd, $devd);
	fclose($fd);
	exec("/etc/rc.d/devd restart");

	$writeconfigupdate = false;
	/* Do XML upgrade from haproxy 0.31 to haproxy-dev */
	if (is_array($config['installedpackages']['haproxy']['ha_servers'])) {
		/* We have an old config */
		$config['installedpackages']['haproxy']['ha_pools']['item'] = array();
		$a_global = &$config['installedpackages']['haproxy'];
		$a_backends = &$config['installedpackages']['haproxy']['ha_backends']['item'];
		$a_oldservers = &$config['installedpackages']['haproxy']['ha_servers']['item'];
		$a_pools = &$config['installedpackages']['haproxy']['ha_pools']['item'];

		foreach ($a_backends as $id => $be) {
			$a_backends[$id]['status'] = 'active';
		}
		$id = 0;
		foreach ($a_oldservers as $oldserver) {
			$pool=$oldserver;
			/* make server sub array */
			$server=array();
			$server['name'] = $oldserver['name'];
			$server['address'] = $oldserver['address'];
			$server['port'] = $oldserver['port'];
			$server['weight'] = $oldserver['weight'];
			$a_servers=array();
			$a_servers[]=$server;
			/* set new pool */
			$pool['name'] = "pool$id";
			$id++;
			$pool['ha_servers']['item']=$a_servers;
			/* link to frontend */
			foreach ($a_backends as $id => $be) {
				if ($a_backends[$id]['name'] == $oldserver['backend']) {
					$a_backends[$id]['backend_serverpool'] = $pool['name'];
					$pool['monitor_uri'] = $be['monitor_uri'];
					unset($a_backends[$id]['monitor_uri']);
					break;
				}
			}
			unset($pool['backend']);
			unset($pool['address']);
			unset($pool['port']);
			unset($pool['weight']);
			$a_pools[] = $pool;
		}
		unset($config['installedpackages']['haproxy']['ha_servers']);
		$writeconfigupdate = true;
	}
	
	/* XML update to: pkg v1.3 and 'pool' changed to 'backend_serverpool' because 'pool' was added to listtags() in xmlparse.inc */
	if (is_array($config['installedpackages']['haproxy']['ha_backends']['item'][0]['pool']))
	{
		foreach($config['installedpackages']['haproxy']['ha_backends']['item'] as &$frontend)
		{
			$backend_serverpool = $frontend['pool'][0];
			$frontend['backend_serverpool'] = $backend_serverpool;
			unset($frontend['pool']);
		}
		$writeconfigupdate = true;
	}
	//also move setting for existing 2.0 installations as only the new variable is used 
	if (isset($config['installedpackages']['haproxy']['ha_backends']['item'][0]['pool']))
	{
		foreach($config['installedpackages']['haproxy']['ha_backends']['item'] as &$frontend)
		{
			$backend_serverpool = $frontend['pool'];
			$frontend['backend_serverpool'] = $backend_serverpool;
			unset($frontend['pool']);
		}
		$writeconfigupdate = true;
	}
	// update config to "haproxy-devel 1.5-dev19 pkg v0.5"
	$a_backends = &$config['installedpackages']['haproxy']['ha_backends']['item'];
	if(is_array($a_backends)) {
		foreach ($a_backends as &$bind) {
			if($bind['httpclose'] && $bind['httpclose'] == "yes" ) {
				$bind['httpclose'] = "httpclose";
				$writeconfigupdate = true;
			}
		}
	}
	if ($writeconfigupdate)
		write_config("haproxy, update xml config version");
	
	conf_mount_ro();

	exec("/usr/local/etc/rc.d/haproxy.sh start");
}

function haproxy_install_cron($should_install) {
	global $config, $g;
	if($g['booting']==true) 
		return;
	$is_installed = false;
	if(!$config['cron']['item'])
		return;
	$x=0;
	foreach($config['cron']['item'] as $item) {
		if(strstr($item['command'], "/usr/local/etc/rc.d/haproxy.sh")) {
			$is_installed = true;
			break;
		}
		$x++;
	}
	switch($should_install) {
		case true:
			if(!$is_installed) {
				$cron_item = array();
				$cron_item['minute'] = "*/2";
				$cron_item['hour'] = "*";
				$cron_item['mday'] = "*";
				$cron_item['month'] = "*";
				$cron_item['wday'] = "*";
				$cron_item['who'] = "root";
				$cron_item['command'] = "/usr/local/etc/rc.d/haproxy.sh check";		
				$config['cron']['item'][] = $cron_item;
				parse_config(true);
				write_config("haproxy, install cron CARP job");
				configure_cron();
			}
		break;
		case false:
			if($is_installed == true) {
				if($x > 0) {
					unset($config['cron']['item'][$x]);
					parse_config(true);
					write_config("haproxy, remove cron CARP job");
				}
				configure_cron();
			}
		break;
	}
}

function haproxy_find_acl($name) {
	global $a_acltypes;
	if($a_acltypes) {
		foreach ($a_acltypes as $acl) {
			if ($acl['name'] == $name)
				return $acl;
		}
	}
}

function write_backend($fd, $name, $pool, $frontend) {
	if(!is_array($pool['ha_servers']['item']) && !$pool['stats_enabled']=='yes')
		return;
	global $a_checktypes;
		
	$a_servers = &$pool['ha_servers']['item'];

	unset($sslserverpresent);
	if (is_array($a_servers))
	{
		foreach($a_servers as $be) {
			if (!$be['status'] == "inactive")
				continue;
			if ($be['ssl'])
				$sslserverpresent = true;
		}
	}
	
	fwrite ($fd, "backend " . $name . "\n");
	if($pool['cookie_name'] && strtolower($frontend['type']) == "http")
		fwrite ($fd, "\tcookie\t\t\t" . $pool['cookie_name'] . " insert indirect\n");

	// https is an alias for tcp for clarity purpouses
	if(strtolower($frontend['type']) == "https") {
		$backend_type = "tcp";
	} else {
		$backend_type = $frontend['type'];
	}
			
	fwrite ($fd, "\tmode\t\t\t" . $backend_type . "\n");
	
	unset($checkport);
	$check_type = $pool['check_type'];
	if ($check_type != 'none')
	{
		$optioncheck = $a_checktypes[$check_type]['syntax'];
		if ($check_type == "MySQL" || $check_type == "PostgreSQL")
			$optioncheck .= " user " . $pool['monitor_username'];
		if ($check_type == "SMTP" || $check_type == "ESMTP")
			$optioncheck .= " " . $pool['monitor_domain'];
		if ($check_type == "HTTP") {
			$uri = $pool['monitor_uri'];
			if (!$uri)
				$uri = "/";
			$optioncheck .= " {$pool['httpcheck_method']} {$uri} {$pool['monitor_httpversion']}";
		}
		if ($check_type == "Agent") {
			$checkport = " port " . $pool['monitor_agentport'];
		}
	} else {
		$optioncheck = "httpchk";
	}
	
	if($pool['balance'])
		fwrite ($fd, "\tbalance\t\t\t" . $pool['balance'] . "\n");

	if(!$pool['connection_timeout'])
		$pool['connection_timeout'] = 30000;
	fwrite ($fd, "\ttimeout connect\t\t" . $pool['connection_timeout'] . "\n");

	if(!$pool['server_timeout'])
		$pool['server_timeout'] = 30000;
	fwrite ($fd, "\ttimeout server\t\t" . $pool['server_timeout'] . "\n");

	if(!$pool['retries'])
		$pool['retries'] = 3;
	fwrite ($fd, "\tretries\t\t\t" . $pool['retries'] . "\n");

	if ($pool['transparent_clientip'])
		fwrite ($fd, "\tsource 0.0.0.0 usesrc clientip\n");
		
	if($pool['stats_enabled']=='yes') {
		fwrite ($fd, "\tstats\t\t\tenable\n");
		if($pool['stats_uri'])
			fwrite ($fd, "\tstats\t\t\turi ".$pool['stats_uri']."\n");
		if($pool['stats_realm'])
			fwrite ($fd, "\tstats\t\t\trealm " . haproxy_escapestring($pool['stats_realm']) . "\n");
		else 
			fwrite ($fd, "\tstats\t\t\trealm .\n");
		fwrite ($fd, "\tstats\t\t\tauth " . haproxy_escapestring($pool['stats_username']).":". haproxy_escapestring($pool['stats_password'])."\n");
		
		if($pool['stats_admin']=='yes')
			fwrite ($fd, "\tstats\t\t\tadmin if TRUE" . "\n");
		
		if($pool['stats_node_enabled']=='yes')
			fwrite ($fd, "\tstats\t\t\tshow-node " . $pool['stats_node'] . "\n");
		if($pool['stats_desc'])
			fwrite ($fd, "\tstats\t\t\tshow-desc " . $pool['stats_desc'] . "\n");
		if($pool['stats_refresh'])
			fwrite ($fd, "\tstats\t\t\trefresh " . $pool['stats_refresh'] . "\n");
	}

	$uri = $pool['monitor_uri'];
	if ($pool['monitor_uri'])
		$uri = $pool['monitor_uri'];
	else
		$uri = "/";
		
	if ($optioncheck)
		fwrite ($fd, "\toption\t\t\t{$optioncheck}\n");

	if ($pool['advanced_backend']) {
		$adv_be	= explode("\n", base64_decode($pool['advanced_backend']));
		foreach($adv_be as $adv_line) {
			if ($adv_line != "") {
				fwrite($fd, "\t" . str_replace("\r", "", $adv_line) . "\n");
			}
		}
	}

	if($pool['cookie'] && strtolower($frontend['type']) == "http") 
		$cookie = " cookie {$pool['cookie']} ";
	else 
		$cookie = "";
	if($pool['advanced']) {
		$advanced = base64_decode($pool['advanced']);
		$advanced_txt = " " . $advanced;
	} else {
		$advanced_txt = "";
	}
	
	if ($check_type != 'none')
	{
		if($pool['checkinter']) 
			$checkinter = "check inter {$pool['checkinter']}";
		else 
			$checkinter = "check inter 1000";
	}

	if (is_array($a_servers))
	{
		foreach($a_servers as $be) {
			if ($be['status'] == "inactive")
				continue;
			
			if (!$be['name'])
				$be['name'] = $be['address'];
			if(!$be['status'] || $be['status'] != 'active') {
				$isbackup = $be['status'];
			} else {
				$isbackup = "";
			}
			$ssl = "";
			if ($be['ssl'] == 'yes')
			{
				$ssl = $backend_type == "http" ? ' ssl' : ' check-ssl';
			}
			fwrite ($fd, "\tserver\t\t\t" . $be['name'] . " " . $be['address'].":" . $be['port'] . "$ssl $cookie $checkinter$checkport $isbackup weight " . $be['weight'] . "{$advanced_txt} {$be['advanced']}\n");
		}
	}
	fwrite ($fd, "\n");
}

function haproxy_configure() {
	global $g;
	// reload haproxy
	haproxy_writeconf("{$g['varetc_path']}/haproxy.cfg");
	return haproxy_check_run(1);
}

function haproxy_check_and_run(&$messages, $reload) {
	global $g;
	$configname = "{$g['varetc_path']}/haproxy.cfg";
	haproxy_writeconf("$configname.new");
	$retval = exec("haproxy -c -V -f $configname.new 2>&1", $output, $err);
	$messages = "";
	if ($err > 1)
		$messages = "<h2><strong>FATAL ERROR CODE: $err while starting haproxy</strong></h2>";
	elseif ($err == 1)
		$messages = "Errors found while starting haproxy";
		
	if ((count($output) > 1) && $output[0] != "Configuration file is valid")
	{
		foreach($output as $line)
			$messages .= "<br/>" . htmlspecialchars($line) . "\n";
	}
	$ok = strstr($retval, "Configuration file is valid");
	if ($ok && $reload) {
		global $haproxy_run_message;
		exec("mv $configname.new $configname");
		$ok = haproxy_check_run(1) == 0;
		$messages = $haproxy_run_message;
	}
	return $ok;
}

function haproxy_writeconf($configfile) {
	global $config;

	$a_global = &$config['installedpackages']['haproxy'];
	$a_backends = &$config['installedpackages']['haproxy']['ha_backends']['item'];
	$a_pools = &$config['installedpackages']['haproxy']['ha_pools']['item'];

	$fd = fopen($configfile, "w");

	if(is_array($a_global)) {
		fwrite ($fd, "global\n");
		fwrite ($fd, "\tmaxconn\t\t\t".$a_global['maxconn']."\n");
		if($a_global['remotesyslog'])
			fwrite ($fd, "\tlog\t\t\t{$a_global['remotesyslog']}\t{$a_global['logfacility']}\t{$a_global['loglevel']}\n");
		fwrite ($fd, "\tstats socket /tmp/haproxy.socket level admin\n");
		
		if(!use_transparent_clientip_proxying())
			fwrite ($fd, "\tuid\t\t\t80\n");
			
		fwrite ($fd, "\tgid\t\t\t80\n");
		// Set numprocs if defined or use system default (#cores)
		if($a_global['nbproc'])
			$numprocs = $a_global['nbproc'];
		else 
			$numprocs ="1";
		fwrite ($fd, "\tnbproc\t\t\t$numprocs\n");
		fwrite ($fd, "\tchroot\t\t\t/var/empty\n");
		fwrite ($fd, "\tdaemon\n");
		
		// Keep the advanced options on the bottom of the global settings, to allow additional sections to be easely added
		if($a_global['advanced']) {
			$adv = explode("\n", base64_decode($a_global['advanced']));
			foreach($adv as $adv_line) {
				fwrite($fd, "\t" . str_replace("\r", "", $adv_line) . "\n");

			}
		}
		fwrite ($fd, "\n");
	}

	// Try and get a unique array for address:port as frontends can duplicate
	$a_bind = array();
	if(is_array($a_backends)) {
		foreach ($a_backends as $backend) {
			if($backend['status'] != 'active')
			{
				unlink_if_exists("var/etc/{$backend['name']}.{$backend['port']}.crt");
				continue;
			}
			if(!$backend['backend_serverpool'])
			{
				unlink_if_exists("var/etc/{$backend['name']}.{$backend['port']}.crt");
				continue;
			}

			//check ssl info
			if (strtolower($backend['type']) == "http" && $backend['ssloffload']){
				//ssl crt ./server.pem ca-file ./ca.crt verify optional crt-ignore-err all crl-file ./ca_crl.pem
				$ssl_crt=" crt /var/etc/{$backend['name']}.{$backend['port']}.crt";
				$cert = lookup_cert($backend['ssloffloadcert']);
				$certcontent = base64_decode($cert['crt'])."\r\n".base64_decode($cert['prv']);
				file_put_contents("/var/etc/{$backend['name']}.{$backend['port']}.crt", $certcontent);
				unset($certcontent);
			}else{
				$ssl_crt="";
				unlink_if_exists("var/etc/{$backend['name']}.{$backend['port']}.crt");
			}
			
			$bname = get_frontend_ipport($backend);

			if (!is_array($a_bind[$bname])) {
				$a_bind[$bname] = array();
				$a_bind[$bname]['config'] = array();
				// Settings which are used only from the primary frontend
				$primaryfrontend = get_primaryfrontend($backend);
				$a_bind[$bname]['name']            = $primaryfrontend['name'];
				$a_bind[$bname]['extaddr']         = $primaryfrontend['extaddr'];
				$a_bind[$bname]['port']            = $primaryfrontend['port'];
				$a_bind[$bname]['type']            = $primaryfrontend['type'];
				$a_bind[$bname]['forwardfor']      = $primaryfrontend['forwardfor'];
				$a_bind[$bname]['httpclose']       = $primaryfrontend['httpclose'];
				$a_bind[$bname]['max_connections'] = $primaryfrontend['max_connections'];
				$a_bind[$bname]['client_timeout']  = $primaryfrontend['client_timeout'];
				$a_bind[$bname]['advanced']        = $primaryfrontend['advanced'];
				$a_bind[$bname]['ssloffload']      = $primaryfrontend['ssloffload'];
				$a_bind[$bname]['advanced_bind']   = $primaryfrontend['advanced_bind'];
			}
			$b = &$a_bind[$bname];
			
			if (($backend['secondary'] != 'yes') && ($backend['name'] != $b['name'])) {
				// only 1 frontend can be the primary for a set of frontends that share 1 address:port.
					$input_errors[] = "Multiple primary frondends for $bname";
			}
			
			if ($ssl_crt != "") {
				if ($b['ssl_info'] == "")
					$b['ssl_info'] = "ssl {$backend['dcertadv']}";
				$b['ssl_info'] .= $ssl_crt;
			}

			// pointer to each backend 
			$b['config'][] = $backend;
		}
	}
	
	$a_pendingpl = array();

	// Construct and write out configuration for each "frontend"
	if(is_array($a_bind)) {
		foreach ($a_bind as $bind) {
			if (count($bind['config']) > 1)
				$frontendinfo = "frontend {$bind['name']}-merged\n";
			else
				$frontendinfo = "frontend {$bind['name']}\n";

			$advancedextra = array();
			
			// Prepare ports for processing by splitting
			$portss = "{$bind['port']},";
			$ports = split(",", $portss);
			$ssl_info = $bind['ssl_info'];
			$advanced_bind = $bind['advanced_bind'];
			// Initialize variable
			$listenip = "";
			
			// Process and add bind directives for ports
			foreach($ports as $port) {
				if($port) {
					if($bind['extaddr'] == "any") 
						$listenip .=  "\tbind\t\t\t0.0.0.0:{$port} {$ssl_info} {$advanced_bind}\n";
					elseif($bind['extaddr']) 
						$listenip .=  "\tbind\t\t\t{$bind['extaddr']}:{$port} {$ssl_info} {$advanced_bind}\n";	
					else 
						$listenip .= "\tbind\t\t\t" . get_current_wan_address('wan') . ":{$port} {$ssl_info} {$advanced_bind}\n";
				}
			}
			
			fwrite ($fd, "{$frontendinfo}");
			fwrite ($fd, "{$listenip}");

			// Advanced pass thru
			if($bind['advanced']) {
				$advanced	= explode("\n", base64_decode($bind['advanced']));
				foreach($advanced as $adv_line) {
					if ($adv_line != "") {
						fwrite($fd, "\t" . str_replace("\r", "", $adv_line) . "\n");
					}
				}
			}

			// https is an alias for tcp for clarity purpouses
			if($bind['type'] == "https") {
				$backend_type = "tcp";
			} else {
				$backend_type = $bind['type'];
			}

			fwrite ($fd, "\tmode\t\t\t" . $backend_type . "\n");
			fwrite ($fd, "\tlog\t\t\tglobal\n");
			fwrite ($fd, "\toption\t\t\tdontlognull\n");

			if ($backend_type == 'http')
			{
				if($bind['httpclose'] && $bind['httpclose'] != "none" ) 
					fwrite ($fd, "\toption\t\t\t{$bind['httpclose']}\n");

				if($bind['forwardfor']) {
					fwrite ($fd, "\toption\t\t\tforwardfor\n");
					if($bind['ssloffload'] == "yes")
						fwrite ($fd, "\treqadd X-Forwarded-Proto:\ https\n");
					else
						fwrite ($fd, "\treqadd X-Forwarded-Proto:\ http\n");
				}
			}

			if($bind['max_connections'])
				fwrite ($fd, "\tmaxconn\t\t\t" . $bind['max_connections'] . "\n");

			if(!$bind['client_timeout'])
				$bind['client_timeout'] = 30000;
			
			fwrite ($fd, "\ttimeout client\t\t" . $bind['client_timeout'] . "\n");

	
			// Combine the rest of the listener configs
			$default_backend = "";
			$i = 0;
			foreach ($bind['config'] as $bconfig) {
				$a_acl=&$bconfig['ha_acls']['item'];
				if(!is_array($a_acl)) 
					$a_acl=array();
			
				$poolname = $bconfig['backend_serverpool'] . "_" . strtolower($bconfig['type']);

				// Create different pools if the svrport is set
				if ($bconfig['svrport'] > 0)
					$poolname .= "_" . $bconfig['svrport'];

				// Write this out once, and must be before any backend config text
				if ($default_backend == "" || $bconfig['secondary'] != 'yes') {
					$default_backend = $poolname;
				}

				if (!isset($a_pendingpl[$poolname])) {
					$a_pendingpl[$poolname] = array();
					$a_pendingpl[$poolname]['name'] = $poolname;
					$a_pendingpl[$poolname]['frontend'] = $bconfig;
				}

				if (strtolower($bind['type']) == "http" && $bconfig['ssloffload'] && $bconfig['ssloffloadacl']) {
					$aclname = "SNI_" . $poolname;
					$cert_cn = cert_get_cn($bconfig['ssloffloadcert'] ,true);
					$a_acl[] = array('name' => $aclname,'expression' => 'host_matches', 'value' => $cert_cn);
				}
				
				// combine acl's with same name to allow for 'combined checks' to check for example hostname and fileextension together..
				$a_acl_combine = array();
				foreach ($a_acl as $entry) {
					$name = $entry['name'];
					$a_acl_combine[$name][] = $entry;
				}
				
				foreach ($a_acl_combine as $a_usebackend) {
					$aclnames = "";
					foreach ($a_usebackend as $entry) {
						$acl = haproxy_find_acl($entry['expression']);
						if (!$acl)
							continue;

						// Filter out acls for different modes
						if ($acl['mode'] != '' && $acl['mode'] != strtolower($bind['type']))
							continue;

						$expr = sprintf($acl['syntax'],$entry['value'],$poolname);

						$aclname = $i . "_" . $entry['name'];
						$aclnames .= $aclname." ";
						fwrite ($fd, "\tacl\t\t\t" . $aclname . "\t" . $expr . "\n");
						
						if ($acl['advancedoptions'] != '')
							$advancedextra[$acl['syntax']] = $acl['advancedoptions']."\n";
						$i++;
					}
					fwrite ($fd, "\tuse_backend\t\t" . $poolname . " if " . $aclnames . "\n");
				}
			}
			fwrite ($fd, "\tdefault_backend\t\t" . $default_backend . "\n");
			
			foreach($advancedextra as $extra)
				fwrite ($fd, "\t".$extra."\n");
			fwrite ($fd, "\n");
		}
	}
	// Construct and write out configuration for each "backend"
	if (is_array($a_pendingpl) && is_array($a_pools)) {
		foreach ($a_pendingpl as $pending) {
			foreach ($a_pools as $pool) {
				if ($pending['frontend']['backend_serverpool'] == $pool['name']) {
					write_backend($fd, $pending['name'], $pool, $pending['frontend']);
				}
			}
		}
	}
	fwrite ($fd, "\n");

	// close config file
	fclose($fd);

	if ($input_errors)
	{
		require_once("guiconfig.inc");
		print_input_errors($input_errors);
	} else {
		// Only sync to xmlrpc backup machine if no errors are found in config
		if(isset($config['installedpackages']['haproxy']['enablesync'])) {
			haproxy_do_xmlrpc_sync();
		}
	}
	
	if (isset($a_global['carpdev']))
		haproxy_install_cron(true);
	else
		haproxy_install_cron(false);

	$freebsd_version = substr(trim(`uname -r`), 0, 1);
	if(!file_exists("/usr/bin/limits")) {
		exec("fetch -q -o /usr/bin/limits http://files.pfsense.org/extras/{$freebsd_version}/limits");
		exec("chmod a+rx /usr/bin/limits");
	}
}

function haproxy_is_running() {
        $running = (shell_exec("/bin/pgrep -x haproxy") != '');
        return $running;
}


function haproxy_load_modules() {
	// On FreeBSD 8 ipfw is needed to allow 'transparent' proxying (getting reply's to a non-local ip to pass back to the client-socket)..
	// On FreeBSD 9 it is probably possible to do the same with the pf option "divert-reply"
	mute_kernel_msgs();
        if (!is_module_loaded("ipfw.ko")) {
                mwexec("/sbin/kldload ipfw");
                /* make sure ipfw is not on pfil hooks */
                mwexec("/sbin/sysctl net.inet.ip.pfil.inbound=\"pf\" net.inet6.ip6.pfil.inbound=\"pf\"" .
                        " net.inet.ip.pfil.outbound=\"pf\" net.inet6.ip6.pfil.outbound=\"pf\"");
        }
	/* Activate layer2 filtering */
	mwexec("/sbin/sysctl net.link.ether.ipfw=1");
	unmute_kernel_msgs();
}

function use_transparent_clientip_proxying() {
	global $config;
	$a_backends = &$config['installedpackages']['haproxy']['ha_pools']['item'];
	if (is_array($a_backends)) {
		foreach ($a_backends as $backend) {
			if ($backend["transparent_clientip"] == 'yes') {
				return true;
				break;
			}
		}
	}
	return false;
}

function load_ipfw_rules() {
	// On FreeBSD 8 pf does not support "divert-reply" so ipfw is needed.
	global $g, $config;
	$ipfw_zone_haproxy = "haproxy";
	
	$a_backends = &$config['installedpackages']['haproxy']['ha_pools']['item'];
	
	haproxy_load_modules();
	
	$transparent_interfaces = array();
	$transparent_backends = array();
	foreach ($a_backends as $backend) {
		if ($backend["transparent_clientip"] != 'yes')
			continue;
		$real_if = get_real_interface($backend["transparent_interface"]);
		$a_servers = &$backend['ha_servers']['item'];
		foreach($a_servers as $server) {	
			if (is_array($a_servers)) {
			
				foreach($a_servers as $be) {
					if (!$be['status'] == "inactive")
						continue;
					if (!is_ipaddr($be['address']))
						continue;
					$item = array();
					$item['address'] = $be['address'];
					$item['port'] = $be['port'];
					$item['interface'] = $real_if;
					$transparent_backends[] = $item;
					$transparent_interfaces[$real_if] = 1;
				}
			}
		}
	}
	mwexec("/usr/local/sbin/ipfw_context -a $ipfw_zone_haproxy", true);
	
	foreach($transparent_interfaces as $transparent_if => $value) {
		mwexec("/usr/local/sbin/ipfw_context -a $ipfw_zone_haproxy -n $transparent_if", true);
	}
	
	$rulenum = 64000; // why that high? captiveportal.inc also does it...
	$rules = "flush\n";
	foreach($transparent_backends as $transparent_be) {
		$rules .= "add $rulenum fwd localhost tcp from {$transparent_be["address"]} {$transparent_be["port"]} to any in recv {$transparent_be["interface"]}\n";
		$rulenum++;
	}
	

	file_put_contents("{$g['tmp_path']}/ipfw_{$ipfw_zone_haproxy}.haproxy.rules", $rules);
	mwexec("/usr/local/sbin/ipfw_context -s $ipfw_zone_haproxy", true);
	mwexec("/sbin/ipfw -x $ipfw_zone_haproxy -q {$g['tmp_path']}/ipfw_{$ipfw_zone_haproxy}.haproxy.rules", true);
}

function haproxy_check_run($reload) {
	global $config, $g, $haproxy_run_message;

	$a_global = &$config['installedpackages']['haproxy'];

	exec("/usr/bin/limits -n 300014");

	if(use_transparent_clientip_proxying())
		load_ipfw_rules();
	else
		mwexec("/usr/local/sbin/ipfw_context -d haproxy", true);
	
	if(isset($a_global['enable'])) {
		if (isset($a_global['carpdev'])) {
			$status = get_carp_interface_status($a_global['carpdev']);
			if ($status != "MASTER") {
				if (haproxy_is_running()) {
					log_error("Stopping haproxy on CARP backup.");
					//exec("/bin/pkill -F /var/run/haproxy.pid haproxy");//doesnt work for multiple pid's in a pidfile
					haproxy_kill();
				}
				return (0);
			} else if (haproxy_is_running() && $reload == 0) { 
				return (0);
			}
			log_error("Starting haproxy on CARP master.");
			/* fallthrough */
		} else if ($reload == 0)
			return (0);

		if (haproxy_is_running()) {
			if (isset($a_global['terminate_on_reload']))
				$sf_st = "-st";//terminate old process as soon as the new process is listening
			else
				$sf_st = "-sf";//finish serving existing connections exit when done, and the new process is listening
			exec("/usr/local/sbin/haproxy -f /var/etc/haproxy.cfg -p /var/run/haproxy.pid $sf_st `cat /var/run/haproxy.pid` 2>&1", $output, $errcode);
		} else {
			exec("/usr/local/sbin/haproxy -f /var/etc/haproxy.cfg -p /var/run/haproxy.pid -D 2>&1", $output, $errcode);
		}
		foreach($output as $line)
			$haproxy_run_message .= "<br/>" . htmlspecialchars($line) . "\n";
		return ($errcode);
	} else {
		if ($reload && haproxy_is_running()) {
			//exec("/bin/pkill -F /var/run/haproxy.pid haproxy");//doesnt work for multiple pid's in a pidfile
			haproxy_kill();
		}
		return (0);
	}
}

function haproxy_kill($killimmediately = true) {
	if ($killimmediately)
		$signal = "KILL"; // stop now
	else
		$signal = "USR1"; // stop when all connections are closed
	killprocesses("haproxy", "/var/run/haproxy.pid", $signal);
}

function killprocesses($processname, $pidfile, $signal = "KILL") {
	exec("kill -$signal `pgrep -x $processname | grep -w -f $pidfile`");
}

function haproxy_sync_xmlrpc_settings() {
	global $config;
	// preserve 'old' sync settings, that should not be overwritten by xmlrpc-sync.
	$enable = isset($config['installedpackages']['haproxy']['enablesync']);
	
	$config['installedpackages']['haproxy'] = $config['installedpackages']['haproxysyncpkg'];
	unset($config['installedpackages']['haproxysyncpkg']);
	
	// restore 'old' settings.
	$config['installedpackages']['haproxy']['enablesync'] = $enable ? true : false;
	
	write_config("haproxy, xmlrpc config synced"); // Write new 'merged' configuration
}

function haproxy_do_xmlrpc_sync() {
	$syncinfo = array();
	$syncinfo['sync_logname'] = "HAProxy";
	$syncinfo['data'] = haproxy_xmlrpc_sync_prepare_config();
	$syncinfo['sync_include'] = "/usr/local/pkg/haproxy.inc";	
	$syncinfo['sync_done_execute'] = "haproxy_xmlrpc_sync_configure";	
	xmlrpc_sync_execute($syncinfo);
}

function haproxy_xmlrpc_sync_prepare_config() {
	/* xml will hold the sections to sync */
	global $config;
	$xml = array();
	$xml['haproxysyncpkg'] = $config['installedpackages']['haproxy'];
	return $xml;
}

function haproxy_xmlrpc_sync_configure() {
	// this function is called by xmlrpc after config has been synced.
	
	haproxy_sync_xmlrpc_settings();
	haproxy_configure(); // Configure HAProxy config files to use the new configuration.
	
	// sync 2nd and further nodes in the chain if applicable.
	if(isset($config['installedpackages']['haproxy']['enablesync'])) {
		haproxy_do_xmlrpc_sync();
	}
}

function get_frontend_id($name) {
	global $config;
	$a_backend = &$config['installedpackages']['haproxy']['ha_backends']['item'];
	$i = 0;
	foreach($a_backend as $backend)
	{
		if ($backend['name'] == $name)
			return $i;
		$i++;
	}
	return null;
}

function get_primaryfrontend($frontend) {
	global $config;
	$a_backend = &$config['installedpackages']['haproxy']['ha_backends']['item'];
	if ($frontend['secondary'] == 'yes')
		$mainfrontend = $a_backend[get_frontend_id($frontend['primary_frontend'])];
	else
		$mainfrontend = $frontend;
	return $mainfrontend;
}

function get_frontend_ipport($frontend) {
	$mainfrontend = get_primaryfrontend($frontend);
	if($mainfrontend['extaddr'] == "any") 
		$result = "0.0.0.0";
	elseif ($mainfrontend['extaddr'] == "localhost")
		$result = "127.0.0.1";
	elseif($mainfrontend['extaddr']) 
		$result = $mainfrontend['extaddr'];
	else 
		$result = get_current_wan_address('wan');
	return $result . ":" . $mainfrontend['port'];
}

function haproxy_check_config() {
	global $config;
	$a_backends = &$config['installedpackages']['haproxy']['ha_backends']['item'];
	$result = false;
	$activefrontends = array();
	$issues = array();
	
	foreach($a_backends as $frontend) {
		if (($frontend['status'] != 'active') || ($frontend['secondary'] == 'yes'))
			continue;
		$ipport = get_frontend_ipport($frontend);
		if (isset($activefrontends[$ipport]))
			$issues['P_'.$ipport] = "Multiple primary frontends with IP:Port \"$ipport\"";
		else
			$activefrontends[$ipport] = true;
	}
	foreach($a_backends as $frontend) {
		if (($frontend['status'] != 'active') || ($frontend['secondary'] != 'yes'))
			continue;
		$ipport = get_frontend_ipport($frontend);
		if (!isset($activefrontends[$ipport]))
			$issues['S_'.$frontend['name']] = "Secondary frontend \"{$frontend['name']}\" without active primary frontend.";
	}
	foreach ($issues as $item)
		$result .= ($result == false ? "" : "<br/>") . $item;	
	return $result;
}

function get_haproxy_frontends($excludeitem="") {
	global $config;
	$a_frontend = &$config['installedpackages']['haproxy']['ha_backends']['item'];
	$result = array();
	foreach($a_frontend as &$frontend)
	{
		if ($frontend['secondary'])
			continue;
		if ($frontend['name'] == $excludeitem)
			continue;
		
		$serveradress = "{$frontend['extaddr']}:{$frontend['port']}";
		$result[$frontend['name']]['name'] = "{$frontend['name']} - {$frontend['type']} ({$serveradress})";
		$result[$frontend['name']]['ref'] = &$frontend;
	}
	asort($result, SORT_STRING);
	return $result;
}

function get_frontent_acls($frontend) {
	$result = array();
	$a_acl = &$frontend['ha_acls']['item'];
	if (is_array($a_acl))
	{
		foreach ($a_acl as $entry) {
			$acl = haproxy_find_acl($entry['expression']);
			if (!$acl)
				continue;

			// Filter out acls for different modes
			if ($acl['mode'] != '' && $acl['mode'] != strtolower($frontend['type']))
				continue;
			
			$acl_item = array();
			$acl_item['descr'] = $acl['descr'] . ": " . $entry['value'];
			$acl_item['ref'] = $entry;
			
			$result[] = $acl_item;
		}
	}
	return $result;
}

function phparray_to_javascriptarray_recursive($nestID, $path, $items, $nodeName, $includeitems) {
	$offset = str_repeat('  ',$nestID);
	$itemName = "item$nestID";
	echo "{$offset}$nodeName = {};\n";
	if (is_array($items))
		foreach ($items as $key => $item)
		{
			if (in_array($path.'/'.$key, $includeitems))
				$subpath = $path.'/'.$key;
			else
				$subpath = $path.'/*';
			if (in_array($subpath, $includeitems) || in_array($path.'/*', $includeitems)) {
				if (is_array($item)) {
					$subNodeName = "item$nestID";
					phparray_to_javascriptarray_recursive($nestID+1, $subpath, $items[$key], $subNodeName, $includeitems);
					echo "{$offset}{$nodeName}['{$key}'] = $itemName;\n";
				} else
					echo "{$offset}{$nodeName}['$key'] = '$item';\n";
			}
		}
}

function phparray_to_javascriptarray($items, $javaMapName, $includeitems) {
		phparray_to_javascriptarray_recursive(1,'',$items, $javaMapName, $includeitems);
}

function haproxy_escapestring($configurationsting) {
	$result = str_replace('\\', '\\\\', $configurationsting);
	$result = str_replace(' ', '\\ ', $result);
	return str_replace('#', '\\#', $result);
}

function echo_html_select($name, $keyvaluelist, $selected, $listEmptyMessage="", $onchangeEvent="") {
	if (count($keyvaluelist)>0){
		if ($onchangeEvent != "")
			$onchangeEvent .= " onchange=$onchangeEvent";
		echo "<select name=\"$name\" id=\"$name\" class=\"formselect\"$onchangeEvent>";
		foreach($keyvaluelist as $key => $desc){
			$selectedhtml = $key == $selected ? "selected" : "";
			echo "<option value=\"{$key}\" {$selectedhtml}>{$desc['name']}</option>";
		}
		echo "</select>";
	} else {
		echo $listEmptyMessage;
	}
}

?>
