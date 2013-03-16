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

global $haproxy_sni_ssloffloading;
$haproxy_sni_ssloffloading=true;// can only be used with recent 1.5-dev17 builds.

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
if ($haproxy_sni_ssloffloading) {
	$a_acltypes[] = array('name' => 'ssl_sni_matches',  'descr' => 'Server Name Indication TLS extension matches', 
					'mode' => 'https', 'syntax' => 'req_ssl_sni -i', 'advancedoptions' => "tcp-request inspect-delay 5s\n\ttcp-request content accept if { req_ssl_hello_type 1 }");
}

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
		write_config();
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
		write_config();
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
		write_config();
	}

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
				write_config();
				configure_cron();
			}
		break;
		case false:
			if($is_installed == true) {
				if($x > 0) {
					unset($config['cron']['item'][$x]);
					parse_config(true);
					write_config();
				}
				configure_cron();
			}
		break;
	}
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
	if(!is_array($pool['ha_servers']['item']) && !$pool['stats_enabled']=='yes')
		return;
		
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
		$httpchk = "ssl-hello-chk";
	} else {
		$backend_type = $frontend['type'];
		if(strtolower($frontend['type']) == "http")
			$httpchk = "httpchk";
		else
			unset($httpchk);
	}
			
	fwrite ($fd, "\tmode\t\t\t" . $backend_type . "\n");

	if($pool['balance'])
		fwrite ($fd, "\tbalance\t\t\t" . $pool['balance'] . "\n");

	if(!$pool['connection_timeout'])
		$pool['connection_timeout'] = 30000;
	fwrite ($fd, "\tcontimeout\t\t" . $pool['connection_timeout'] . "\n");

	if(!$pool['server_timeout'])
		$pool['server_timeout'] = 30000;
	fwrite ($fd, "\tsrvtimeout\t\t" . $pool['server_timeout'] . "\n");

	if(!$pool['retries'])
		$pool['retries'] = 3;
	fwrite ($fd, "\tretries\t\t\t" . $pool['retries'] . "\n");

	if($pool['stats_enabled']=='yes') {
		fwrite ($fd, "\tstats\t\t\tenable\n");
		if($pool['stats_uri'])
			fwrite ($fd, "\tstats\t\t\turi ".$pool['stats_uri']."\n");
		if($pool['stats_realm'])
			fwrite ($fd, "\tstats\t\t\trealm " . $pool['stats_realm'] . "\n");
		else 
			fwrite ($fd, "\tstats\t\t\trealm .\n");
		fwrite ($fd, "\tstats\t\t\tauth " . $pool['stats_username'].":". $pool['stats_password']."\n");
		
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
		
	if ($httpchk)
		fwrite ($fd, "\toption\t\t\t{$httpchk} HEAD " . $uri . " HTTP/1.0\n");

	if ($pool['advanced_backend']) {
		$adv_be	= explode("\n", base64_decode($pool['advanced_backend']));
		foreach($adv_be as $adv_line) {
			$adv_line = trim($adv_line);
			if ($adv_line != "") {
				fwrite($fd, "\t" . $adv_line . "\n");
			}
		}
	}

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

	if (is_array($a_servers))
	{
		foreach($a_servers as $be) {
			if (!$be['status'] == "inactive")
				continue;
			
			if (!$be['name'])
				$be['name'] = $be['address'];
			if(!$be['status'] || $be['status'] != 'active') {
				$isbackup = $be['status'];
			} else {
				$isbackup = "";
			}
			$ssl = ($backend_type == "http" && $be['ssl'] == 'yes') ? ' ssl' : "";
			fwrite ($fd, "\tserver\t\t\t" . $be['name'] . " " . $be['address'].":" . $be['port'] . "$ssl $cookie $checkinter $isbackup weight " . $be['weight'] . "{$advanced_txt} {$be['advanced']}\n");
		}
	}
	fwrite ($fd, "\n");
}

function haproxy_configure() {
	// reload haproxy
	haproxy_writeconf();
	return haproxy_check_run(1);
}

function haproxy_check_writtenconfig_error(&$messages) {
	$retval = exec("haproxy -c -V -f /var/etc/haproxy.cfg 2>&1", $output, $err);
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
	return (strstr($retval, "Configuration file is valid"));
}

function haproxy_writeconf() {
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
				$certcontent = base64_decode($cert['crt']).base64_decode($cert['prv']);
				file_put_contents("/var/etc/{$backend['name']}.{$backend['port']}.crt", $certcontent);
				unset($certcontent);
			}else{
				$ssl_crt="";
				unlink_if_exists("var/etc/{$backend['name']}.{$backend['port']}.crt");
			}
			
			if ($backend['extaddr']=='localhost')
				$backend['extaddr'] = "127.0.0.1";

			$bname = $backend['extaddr'] . ":" . $backend['port'];
			if (!is_array($a_bind[$bname])) {
				$a_bind[$bname] = array();
				$a_bind[$bname]['config'] = array();
				// Settings which are constant for a merged frontend
				$a_bind[$bname]['name'] = $backend['name'];
				$a_bind[$bname]['extaddr'] = $backend['extaddr'];
				$a_bind[$bname]['port'] = $backend['port'];
			}
			$b = &$a_bind[$bname];
			
			// Overwrite ?
			if ($backend['secondary'] != 'yes') {
				if (isset($b['type']))
					$input_errors[] = "Multiple primary frondends for $bname";
				$b['type'] = $backend['type'];
				$b['forwardfor'] = $backend['forwardfor'];
				$b['httpclose'] = $backend['httpclose'];
				$b['max_connections'] = $backend['max_connections'];
				$b['client_timeout'] = $backend['client_timeout'];
				$b['advanced'] = $backend['advanced'];
				$b['ssloffload'] = $backend['ssloffload'];
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
			// Initialize variable
			$listenip = "";
			
			// Process and add bind directives for ports
			foreach($ports as $port) {
				if($port) {
					if($bind['extaddr'] == "any") 
						$listenip .=  "\tbind\t\t\t0.0.0.0:{$port} {$ssl_info}\n";
					elseif($bind['extaddr']) 
						$listenip .=  "\tbind\t\t\t{$bind['extaddr']}:{$port} {$ssl_info}\n";	
					else 
						$listenip .= "\tbind\t\t\t" . get_current_wan_address('wan') . ":{$port} {$ssl_info}\n";
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
				if($bind['httpclose']) 
					fwrite ($fd, "\toption\t\t\thttpclose\n");

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
			
			fwrite ($fd, "\tclitimeout\t\t" . $bind['client_timeout'] . "\n");

	
			// Combine the rest of the listener configs
			$default_once = 0;
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
				if ($default_once == 0) {
					fwrite ($fd, "\tdefault_backend\t\t" . $poolname . "\n");
					$default_once++;
				}

				if (!isset($a_pendingpl[$poolname])) {
					$a_pendingpl[$poolname] = array();
					$a_pendingpl[$poolname]['name'] = $poolname;
					$a_pendingpl[$poolname]['frontend'] = $bconfig;
				}

				if (strtolower($bind['type']) == "http" && $bconfig['ssloffload'] && $bconfig['ssloffloadacl']) {
					$aclname = "SNI_" . $poolname;
					$cert_cn = cert_get_cn($bconfig['ssloffloadcert'] ,true);
					//$expr = "req_ssl_sni -i $cert_cn";
					$expr = "hdr(host) -i $cert_cn";
					fwrite ($fd, "\tacl\t\t\t" . $aclname . "\t" . $expr . "\n");
					fwrite ($fd, "\tuse_backend\t\t" . $poolname . " if " . $aclname . "\n");
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
					
					if ($acl['advancedoptions'] != '')
						$advancedextra[$acl['syntax']] = $acl['advancedoptions']."\n";
					$i++;
				}
			}
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

	if ($input_errors)
	{
		require_once("guiconfig.inc");
		print_input_errors($input_errors);
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

function haproxy_check_run($reload) {
	global $config, $g;

	$a_global = &$config['installedpackages']['haproxy'];

	exec("/usr/bin/limits -n 300014");

	if(isset($a_global['enable'])) {
		if (isset($a_global['carpdev'])) {
			$status = get_carp_interface_status($a_global['carpdev']);
			if ($status != "MASTER") {
				if (haproxy_is_running()) {
					log_error("Stopping haproxy on CARP backup.");
					exec("/bin/pkill -F /var/run/haproxy.pid haproxy");
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
			exec("/usr/local/sbin/haproxy -f /var/etc/haproxy.cfg -p /var/run/haproxy.pid -st `cat /var/run/haproxy.pid`");
		} else {
			exec("/usr/local/sbin/haproxy -f /var/etc/haproxy.cfg -p /var/run/haproxy.pid -D");
		}
		return (0);
	} else {
		if ($reload && haproxy_is_running()) {
			exec("/bin/pkill -F /var/run/haproxy.pid haproxy");
		}
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
		$error = "A communications error occurred while attempting HAProxy XMLRPC sync with {$url}:{$port} (exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "HAProxy Settings Reload", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting HAProxy XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "HAProxy Settings Sync", "");
	} else {
		log_error("HAProxy XMLRPC reload data success with {$url}:{$port} (exec_php).");
	}
}

function get_frontend_id($name) {
	global $a_backend;
	$i = 0;
	foreach($a_backend as $backend)
	{
		if ($backend['name'] == $name)
			return $i;
		$i++;
	}
	return null;
}

function get_frontend_ipport($fontend) {
	global $a_backend;
	if ($fontend['secondary'] == 'yes')
		$mainfontend = $a_backend[get_frontend_id($fontend['primary_frontend'])];
	else
		$mainfontend = $fontend;
	if($mainfontend['extaddr'] == "any") 
		$result = "0.0.0.0";
	elseif($mainfontend['extaddr']) 
		$result = $mainfontend['extaddr'];
	else 
		$result = get_current_wan_address('wan');
	return $result . ":" . $mainfontend['port'];
	return 'abc';
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
			$acl_item['descr'] = $acl['descr'] . " " . $entry['value'];
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

function phparray_to_javascriptarray($items, $javaMapName, $includeitems)
{
		phparray_to_javascriptarray_recursive(1,'',$items, $javaMapName, $includeitems);
}

?>
