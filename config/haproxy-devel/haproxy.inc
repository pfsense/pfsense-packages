<?php
/*
    haproxy.inc
	Copyright (C) 2013 PiBa-NL
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
require_once("haproxy_utils.inc");
require_once("haproxy_xmlrpcsyncclient.inc");

$d_haproxyconfdirty_path = $g['varrun_path'] . "/haproxy.conf.dirty";

global $a_acltypes;
$a_acltypes = array();
$a_acltypes["host_starts_with"] = array('name' => 'Host starts with',
				'mode' => 'http', 'syntax' => 'hdr_beg(host) -i %1$s');
$a_acltypes["host_ends_with"] = array('name' => 'Host ends with',
				'mode' =>'http', 'syntax' => 'hdr_end(host) -i %1$s');
$a_acltypes["host_matches"] = array('name' => 'Host matches',
				'mode' =>'http', 'syntax' => 'hdr(host) -i %1$s');
$a_acltypes["host_regex"] = array('name' => 'Host regex',
				'mode' =>'http', 'syntax' => 'hdr_reg(host) -i %1$s');
$a_acltypes["host_contains"] = array('name' => 'Host contains',
				'mode' => 'http', 'syntax' => 'hdr_dir(host) -i %1$s');
$a_acltypes["path_starts_with"] = array('name' => 'Path starts with',
				'mode' => 'http', 'syntax' => 'path_beg -i %1$s');
$a_acltypes["path_ends_with"] = array('name' => 'Path ends with',
				'mode' => 'http', 'syntax' => 'path_end -i %1$s');
$a_acltypes["path_matches"] = array('name' => 'Path matches',
				'mode' => 'http', 'syntax' => 'path -i %1$s');
$a_acltypes["path_regex"] = array('name' => 'Path regex',
				'mode' => 'http', 'syntax' => 'path_reg -i %1$s');
$a_acltypes["path_contains"] = array('name' => 'Path contains',
				'mode' => 'http', 'syntax' => 'path_dir -i %1$s');
$a_acltypes["source_ip"] = array('name' => 'Source IP',
				'mode' => '', 'syntax' => 'src %1$s');
$a_acltypes["backendservercount"] = array('name' => 'Minimum count usable servers',
				'mode' => '', 'syntax' => 'nbsrv(%2$s) ge %1$d', 'parameters' => 'value,backendname');
// 'ssl_sni_matches' was added in HAProxy1.5dev17
$a_acltypes["ssl_sni_matches"] = array('name' => 'Server Name Indication TLS extension matches', 
				'mode' => 'https', 'syntax' => 'req_ssl_sni -i %1$s', 'advancedoptions' => "tcp-request inspect-delay 5s\n\ttcp-request content accept if { req_ssl_hello_type 1 }");

global $a_checktypes;
$a_checktypes = array();
$a_checktypes['none'] = array('name' => 'none', 'syntax' => '',
	'descr' => 'No health checks will be performed.');
$a_checktypes['Basic'] = array('name' => 'Basic', 'syntax' => '',
	'descr' => 'Basic socket connection check');
$a_checktypes['HTTP'] = array('name' => 'HTTP', 'syntax' => 'httpchk',
	'descr' => 'HTTP protocol to check on the servers health, can also be used for HTTPS servers(requirs checking the SSL box for the servers).', 'parameters' => "uri,method,version");
// 'Agent' was added in HAProxy1.5dev18, and removed in 1.5dev20, in favor of the seperate agent-check option.
$a_checktypes['Agent'] = array('name' => 'Agent', 'syntax' => 'lb-agent-chk', 'usedifferenport' => 'yes', 
	'descr' => 'Use a TCP connection to read an ASCII string of the form 100%,75%,drain,down (others in haproxy manual)',
	deprecated => true);
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

global $a_httpcheck_method;
$a_httpcheck_method = array();
$a_httpcheck_method['OPTIONS'] = array('name' => 'OPTIONS', 'syntax' => 'OPTIONS');
$a_httpcheck_method['HEAD']    = array('name' => 'HEAD', 'syntax' => 'HEAD');
$a_httpcheck_method['GET']     = array('name' => 'GET', 'syntax' => 'GET');
$a_httpcheck_method['POST']    = array('name' => 'POST', 'syntax' => 'POST');
$a_httpcheck_method['PUT']     = array('name' => 'PUT', 'syntax' => 'PUT');
$a_httpcheck_method['DELETE']  = array('name' => 'DELETE', 'syntax' => 'DELETE');
$a_httpcheck_method['TRACE']   = array('name' => 'TRACE', 'syntax' => 'TRACE');

global $a_closetypes;
$a_closetypes = array();
$a_closetypes['none'] = array('name' => 'none', 'syntax' => '',
	'descr' => 'No close headers will be changed.');
$a_closetypes['httpclose'] = array('name' => 'httpclose', 'syntax' => 'httpclose',
	'descr' => 'The "httpclose" option removes any "Connection" header both ways, and adds a "Connection: close" header in each direction. This makes it easier to disable HTTP keep-alive than the previous 4-rules block.');
$a_closetypes['http-server-close'] = array('name' => 'http-server-close', 'syntax' => 'http-server-close',
	'descr' => 'By default, when a client communicates with a server, HAProxy will only analyze, log, and process the first request of each connection. Setting "option http-server-close" enables HTTP connection-close mode on the server side while keeping the ability to support HTTP keep-alive and pipelining on the client side.  This provides the lowest latency on the client side (slow network) and the fastest session reuse on the server side to save server resources.');
$a_closetypes['forceclose'] = array('name' => 'forceclose', 'syntax' => 'forceclose',
	'descr' => 'Some HTTP servers do not necessarily close the connections when they receive the "Connection: close" set by "option httpclose", and if the client does not close either, then the connection remains open till the timeout expires. This causes high number of simultaneous connections on the servers and shows high global session times in the logs. Note that this option also enables the parsing of the full request and response, which means we can close the connection to the server very quickly, releasing some resources earlier than with httpclose.');
$a_closetypes['http-keep-alive'] = array('name' => 'http-keep-alive', 'syntax' => 'http-keep-alive',
	'descr' => 'By default, when a client communicates with a server, HAProxy will only analyze, log, and process the first request of each connection. Setting "option http-keep-alive" enables HTTP keep-alive mode on the client- and server- sides. This provides the lowest latency on the client side (slow network) and the fastest session reuse on the server side at the expense of maintaining idle connections to the servers. In general, it is possible with this option to achieve approximately twice the request rate that the "http-server-close" option achieves on small objects. There are mainly two situations where this option may be useful :   - when the server is non-HTTP compliant and authenticates the connection     instead of requests (eg: NTLM authentication)    - when the cost of establishing the connection to the server is significant     compared to the cost of retrieving the associated object from the server.');

global $a_servermodes;
$a_servermodes = array();
$a_servermodes["active"]['name'] = "active";
$a_servermodes["backup"]['name'] = "backup";
$a_servermodes["disabled"]['name'] = "disabled";
$a_servermodes["inactive"]['name'] = "inactive";

// http://www.exceliance.fr/sites/default/files/biblio/aloha_load_balancer_haproxy_cookie_persistence_methods_memo.pdf
global $a_cookiemode;
$a_cookiemode = array();
$a_cookiemode['passive'] 				= array('name' => 'Passive', 'syntax' => 'cookie <cookie name>',
	'descr' => 'Cookie is analysed on incoming request to choose server. HAProxy does not perform any insertion update or deletion on the Cookie or Set-Cookie. If the Cookie is not set, then the load-balancing algorithm is applied.');
$a_cookiemode['passive-silent']			= array('name' => 'Passive-silent', 'syntax' => 'cookie <cookie name> indirect',
	'descr' => 'Cookie is analysed on incoming request to choose server. HAProxy does not perform any insertion, update or deletion on the Cookie. Set-Cookie is removed from response if not required. If the Cookie is not set, then HAProxy applies the load-balancing algorithm.');
$a_cookiemode['reset']					= array('name' => 'Reset', 'syntax' => 'cookie <cookie name> rewrite',
	'descr' => 'Cookie is analysed on incoming request to choose server and Set-Cookie value is overwritten in response if present. If the Set-Cookie isn\'t sent by the server, then HAProxy won\'t set it.');
$a_cookiemode['set']					= array('name' => 'Insert', 'syntax' => 'cookie <cookie name> insert',
	'descr' => 'Cookie is analyzed on incoming request to choose server and Set-Cookie value is overwritten if present and set to an unknown value or inserted in response if not present.');
$a_cookiemode['set-silent'] 			= array('name' => 'Insert-silent', 'syntax' => 'cookie <cookie name> insert indirect',
	'descr' => 'Cookie is analyzed on incoming request to choose server and Set-Cookie value is overwritten if present, inserted in response if needed and removed if a valid Cookie was provided.');
$a_cookiemode['insert-only'] 			= array('name' => 'Insert-preserve', 'syntax' => 'cookie <cookie name> preserve insert',
	'descr' => 'Cookie is analyzed on incoming request to choose server. Set-Cookie value is set only if the server does not provide one or if the client came without the Cookie.');
$a_cookiemode['insert-only-silent'] 	= array('name' => 'Insert-preserve-silent', 'syntax' => 'cookie <cookie name> preserve insert indirect',
	'descr' => 'Cookie is analyzed on incoming request to choose server and Set-Cookie value is left untouched if present, inserted in response if needed or removed if not needed.');
$a_cookiemode['session-prefix'] 		= array('name' => 'Session-prefix', 'syntax' => 'cookie <cookie name> prefix',
	'descr' => 'Cookie is analyzed on incoming request to choose server whose Cookie Name prefix matches. Set Cookie value is prefixed using server line Cookie ID in response. Cookie is modified only between HAProxy and the client only');
$a_cookiemode['passive-session-prefix'] = array('name' => 'Passive-session-prefix', 'syntax' => 'cookie <cookie name> preserve prefix indirect',
	'descr' => 'Cookie is analysed on incoming request to choose server whose Cookie ID prefix matches.');
foreach($a_cookiemode as &$cookiemode)
	$cookiemode['descr'] = $cookiemode['descr'] . "\n\n" . $cookiemode['syntax'] . "";

global $a_sticky_type;
$a_sticky_type = array();
$a_sticky_type['none'] = array('name' => 'none',
	'descr' => "No stick-table will be used");
$a_sticky_type['stick_sslsessionid'] = array('name' => 'Stick on SSL-Session-ID',
	'descr' => "Only used on https frontends. Uses the SSL-Session-ID to persist clients to a server.");
$a_sticky_type['stick_sourceipv4'] = array('name' => 'Stick on SourceIP IPv4',
	'descr' => "Stick on the client ip, drawback is that multiple clients behind a natted public ip will be balanced to the same server.");
$a_sticky_type['stick_sourceipv6'] = array('name' => 'Stick on SourceIP IPv6',
	'descr' => "Stick on the client ip, drawback is that multiple clients behind a natted public ip will be balanced to the same server.");
$a_sticky_type['stick_cookie_value'] = array('name' => 'Stick on existing Cookie value',
	'descr' => "Stick on the value of a session cookie",
	'cookiedescr' => "Enables SSL-session-id based persistence. (only use on 'https' and 'tcp' frontends that use SSL)<br/>EXAMPLE: JSESSIONID PHPSESSIONID ASP.NET_SessionId");
$a_sticky_type['stick_rdp_cookie'] = array('name' => 'Stick on RDP-cookie',
	'descr' => "Uses a RDP-Cookie send by the mstsc client, note that not all clients send this.",
	'cookiedescr' => 'EXAMPLE: msts or mstshash');

if(!function_exists('group_ports')){
// function group_ports() is present in pfSense 2.2 in util.inc
/* create ranges of sequential port numbers (200:215) and remove duplicates */
function group_ports($ports) {
	if (!is_array($ports) || empty($ports))
		return;

	$uniq = array();
	foreach ($ports as $port) {
		if (is_portrange($port)) {
			list($begin, $end) = explode(":", $port);
			if ($begin > $end) {
				$aux = $begin;
				$begin = $end;
				$end = $aux;
			}
			for ($i = $begin; $i <= $end; $i++)
				if (!in_array($i, $uniq))
					$uniq[] = $i;
		} else if (is_port($port)) {
			if (!in_array($port, $uniq))
				$uniq[] = $port;
		}
	}
	sort($uniq, SORT_NUMERIC);

	$result = array();
	foreach ($uniq as $idx => $port) {
		if ($idx == 0) {
			$result[] = $port;
			continue;
		}

		$last = end($result);
		if (is_portrange($last))
			list($begin, $end) = explode(":", $last);
		else
			$begin = $end = $last;

		if ($port == ($end+1)) {
			$end++;
			$result[count($result)-1] = "{$begin}:{$end}";
		} else {
			$result[] = $port;
		}
	}

	return $result;
}
}
	
function haproxy_portoralias_to_list($port_or_alias) {
	// input: a port or aliasname: 80 https MyPortAlias
	// returns: a array of ports and portranges 80 443 8000:8010
	global $aliastable;
	$portresult = array();
	if (alias_get_type($port_or_alias) == "port") {
		$aliasports = $aliastable[$port_or_alias];
		$ports = explode(' ',$aliasports);
		foreach($ports as $port) {
			$portresults = haproxy_portoralias_to_list($port);
			$portresult = array_merge($portresult, $portresults);
		}
		return $portresult;
	} else if (is_portrange($port_or_alias)) {
		return (array)$port_or_alias;
	} else if (is_port($port_or_alias)) {
		if (getservbyname($port_or_alias, "tcp"))
			return (array)getservbyname($port_or_alias, "tcp");
		if (getservbyname($port_or_alias, "udp"))
			return (array)getservbyname($port_or_alias, "udp");
		return (array)$port_or_alias;
	}
	else 
		return null;
}

function haproxy_custom_php_deinstall_command() {
	global $static_output;
	$static_output .= "HAProxy, running haproxy_custom_php_deinstall_command()\n";
	update_output_window($static_output);
	$static_output .= "HAProxy, deleting haproxy webgui\n";
	update_output_window($static_output);
	exec("rm /usr/local/etc/rc.d/haproxy.sh");
	$static_output .= "HAProxy, installing cron job if needed\n";
	update_output_window($static_output);
	haproxy_install_cron(false);
	$static_output .= "HAProxy, running haproxy_custom_php_deinstall_command() DONE\n";
	update_output_window($static_output);
}

function haproxy_custom_php_install_command() {
	global $g, $config, $static_output;
	$static_output .= "HAProxy, running haproxy_custom_php_install_command()\n";
	update_output_window($static_output);
	
	$static_output .= "HAProxy, conf_mount_rw\n";
	update_output_window($static_output);
	conf_mount_rw();
	
	$static_output .= "HAProxy, create '/usr/local/etc/rc.d/haproxy.sh'\n";
	update_output_window($static_output);
	$haproxy = <<<EOD
#!/bin/sh

# PROVIDE: haproxy
# REQUIRE: LOGIN
# KEYWORD: FreeBSD

. /etc/rc.subr

name="haproxy"
rcvar=`set_rcvar`
command="/usr/pbi/haproxy-devel-`uname -m`/sbin/haproxy"
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



	$static_output .= "HAProxy, update configuration\n";
	update_output_window($static_output);
	
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
	if(is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
		foreach ($config['installedpackages']['haproxy']['ha_backends']['item'] as &$bind) {
			if($bind['httpclose'] && $bind['httpclose'] == "yes" ) {
				$bind['httpclose'] = "httpclose";
				$writeconfigupdate = true;
			}
			if (!$bind['extaddr']){
				$bind['extaddr'] = "wan_ipv4";
				$writeconfigupdate = true;
			}
			if ($bind['extaddr'] == "localhost"){
				$bind['extaddr'] = "localhost_ipv4";
				$writeconfigupdate = true;
			}
			if ($bind['extaddr'] == "any"){
				$bind['extaddr'] = "any_ipv4";
				$writeconfigupdate = true;
			}
		}
	}
	if ($writeconfigupdate) {
		$static_output .= "HAProxy, write updated config\n";
		update_output_window($static_output);
		write_config("HAProxy, update xml config version");
	}
	
	$static_output .= "HAProxy, conf_mount_ro\n";
	update_output_window($static_output);
	conf_mount_ro();
	
	$static_output .= "HAProxy, starting haproxy (if previously enabled)\n";
	update_output_window($static_output);
	haproxy_check_run(1);
	
	$static_output .= "HAProxy, running haproxy_custom_php_install_command() DONE\n";
	update_output_window($static_output);
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
		foreach ($a_acltypes as $key => $acl) {
			if ($key == $name)
				return $acl;
		}
	}
}

function write_backend($fd, $name, $pool, $frontend) {
	if(!is_array($pool['ha_servers']['item']) && !$pool['stats_enabled']=='yes')
		return;
	global $a_checktypes, $a_cookiemode;
		
	$a_servers = &$pool['ha_servers']['item'];
	$frontendtype = $frontend['type'];
	$frontend_ip = haproxy_interface_ip($frontend['extaddr']);
	
	fwrite ($fd, "backend " . $name . "\n");
	// https is an alias for tcp for clarity purposes
	if($frontendtype == "https") {
		$backend_mode = "tcp";
	} else {
		$backend_mode = $frontendtype;
	}
	fwrite ($fd, "\tmode\t\t\t" . $backend_mode . "\n");

	if ($frontendtype == "http") {
		if ($pool["persist_cookie_enabled"] == "yes") {
			$cookie_mode = $pool["persist_cookie_mode"];
			$cookie_cachable = $pool["persist_cookie_cachable"];
			$cookiesyntax = $a_cookiemode[$cookie_mode]["syntax"];
			$cookie = str_replace("<cookie name>", $pool["persist_cookie_name"], $cookiesyntax);
			$cookie .= $cookie_cachable == "yes" ? "" : " nocache";
			fwrite ($fd, "\t" . $cookie . "\n");
		}
	}	
	switch($pool["persist_sticky_type"]) {
	case 'stick_sslsessionid':
		if ($frontendtype == "https") {
			fwrite ($fd, "\ttcp-request inspect-delay 5s\n");
			fwrite ($fd, "\tstick-table type binary len 32 size ".$pool["persist_stick_tablesize"]." expire ".$pool["persist_stick_expire"]."\n");
			fwrite ($fd, "\tacl clienthello req_ssl_hello_type 1\n");
			fwrite ($fd, "\tacl serverhello rep_ssl_hello_type 2\n");
			fwrite ($fd, "\ttcp-request content accept if clienthello\n");
			fwrite ($fd, "\ttcp-response content accept if serverhello\n");
			fwrite ($fd, "\tstick on payload_lv(43,1) if clienthello\n");
			fwrite ($fd, "\tstick store-response payload_lv(43,1) if serverhello\n");
		}
		break;
	case 'stick_rdp_cookie':
		//tcp-request content accept if RDP_COOKIE
		//fwrite ($fd, "\tstick on req.rdp_cookie(msts)\n");
		fwrite ($fd, "\tstick-table type binary len 32 size ".$pool["persist_stick_tablesize"]." expire ".$pool["persist_stick_expire"]."\n");
		fwrite ($fd, "\tstick on req.rdp_cookie(mstshash)\n");
		break;
	case 'stick_sourceipv4':
		fwrite ($fd, "\tstick-table type ip size ".$pool["persist_stick_tablesize"]." expire ".$pool["persist_stick_expire"]."\n");
		fwrite ($fd, "\tstick on src\n");
		break;
	case 'stick_sourceipv6':
		fwrite ($fd, "\tstick-table type ip size ".$pool["persist_stick_tablesize"]." expire ".$pool["persist_stick_expire"]."\n");
		fwrite ($fd, "\tstick on src\n");
		break;
	case 'stick_cookie_value':
		if ($frontendtype == "http") {
			fwrite ($fd, "\tstick-table type string len {$pool["persist_stick_length"]} size ".$pool["persist_stick_tablesize"]." expire ".$pool["persist_stick_expire"]."\n");
			fwrite ($fd, "\tstick store-response res.cook({$pool["persist_stick_cookiename"]})\n");
			fwrite ($fd, "\tstick on req.cook({$pool["persist_stick_cookiename"]})\n");
		}
		break;
	}
	
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
			
		if ($pool['stats_username'] && $pool['stats_password'])
			fwrite ($fd, "\tstats\t\t\tauth " . haproxy_escapestring($pool['stats_username']).":". haproxy_escapestring($pool['stats_password'])."\n");
		
		if($pool['stats_admin']=='yes')
			fwrite ($fd, "\tstats\t\t\tadmin if TRUE" . "\n");
		
		if($pool['stats_node'])
			fwrite ($fd, "\tstats\t\t\tshow-node " . $pool['stats_node'] . "\n");
		if($pool['stats_desc'])
			fwrite ($fd, "\tstats\t\t\tshow-desc " . haproxy_escapestring($pool['stats_desc']) . "\n");
		if($pool['stats_refresh'])
			fwrite ($fd, "\tstats\t\t\trefresh " . $pool['stats_refresh'] . "\n");
		
		if ($pool['stats_scope']) {
			$scope_items = explode(",", $pool['stats_scope']);
			foreach($scope_items as $scope_item)
				fwrite ($fd, "\tstats\t\t\tscope " . $scope_item . "\n");
		}
	}

	$uri = $pool['monitor_uri'];
	if ($pool['monitor_uri'])
		$uri = $pool['monitor_uri'];
	else
		$uri = "/";
		
	if ($optioncheck)
		fwrite ($fd, "\toption\t\t\t{$optioncheck}\n");

	if ($pool["strict_transport_security"] && is_numeric($pool["strict_transport_security"])){
		fwrite ($fd, "\trspadd Strict-Transport-Security:\ max-age={$pool["strict_transport_security"]};\n");
	}
	
	if ($pool['advanced_backend']) {
		$adv_be	= explode("\n", base64_decode($pool['advanced_backend']));
		foreach($adv_be as $adv_line) {
			if ($adv_line != "") {
				fwrite($fd, "\t" . str_replace("\r", "", $adv_line) . "\n");
			}
		}
	}

	if($pool['advanced']) {
		$advanced = base64_decode($pool['advanced']);
		$advanced_txt = " " . $advanced;
	} else {
		$advanced_txt = "";
	}
	
	if ($check_type != 'none')
	{
		if($pool['checkinter']) 
			$checkinter = " check inter {$pool['checkinter']}";
		else 
			$checkinter = " check inter 1000";
	}
	
	//agent-check requires at least haproxy v1.5dev20
	if ($pool['agent_check'])
		$agentcheck = " agent-check agent-inter {$pool['agent_inter']} agent-port {$pool['agent_port']}";

	if (is_array($a_servers))
	{
		foreach($a_servers as $be) {
			if ($be['status'] == "inactive")
				continue;
			if($be['cookie'] && $frontendtype == "http") 
				$cookie = " cookie {$be['cookie']}";
			else 
				$cookie = "";
			
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
				$ssl = $frontendtype == "http" ? ' ssl' : ' check-ssl';
			}
			$weight = "";
			if (is_numeric($be['weight'])){
				$weight = " weight " . $be['weight'];
			}
			fwrite ($fd, "\tserver\t\t\t" . $be['name'] . " " . $be['address'].":" . $be['port'] . "$ssl$cookie$checkinter$checkport$agentcheck $isbackup$weight{$advanced_txt} {$be['advanced']}\n");
		}
	}
	fwrite ($fd, "\n");
}

function haproxy_configure() {
	global $g;
	// reload haproxy
	return haproxy_check_run(1);
}

function haproxy_check_and_run(&$messages, $reload) {
	global $g;
	$testpath = "{$g['varetc_path']}/haproxy_test";
	haproxy_writeconf($testpath);
	$retval = exec("haproxy -c -V -f $testpath/haproxy.cfg 2>&1", $output, $err);
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
		rmdir_recursive($testpath);
		$ok = haproxy_check_run(1) == 0;
		$messages = $haproxy_run_message;
	}
	return $ok;
}
function haproxy_write_certificate_file($filename, $certid) {
	$cert = lookup_cert($certid);

	$certcontent = base64_decode($cert['crt']);
	$certcontent .= "\r\n".base64_decode($cert['prv']);
	
	$certchaincontent = ca_chain($cert);
	if ($certchaincontent != "") {
		$certcontent .= "\r\n" . $certchaincontent;
	}
	unset($certchaincontent);
	file_put_contents($filename, $certcontent);
	unset($certcontent);
	unset($cert);
}

function haproxy_writeconf($configpath) {
	global $config;

	$configfile = $configpath . "/haproxy.cfg";

	rmdir_recursive($configpath);
	make_dirs($configpath);
	
	$a_global = &$config['installedpackages']['haproxy'];
	$a_frontends = &$config['installedpackages']['haproxy']['ha_backends']['item'];
	$a_backends = &$config['installedpackages']['haproxy']['ha_pools']['item'];
	
	$fd = fopen($configfile, "w");
	if(is_array($a_global)) {
		fwrite ($fd, "global\n");
		if ($a_global['maxconn'])
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
		fwrite ($fd, "\tssl-server-verify none\n");
		
		// Keep the advanced options on the bottom of the global settings, to allow additional sections to be easely added
		if($a_global['advanced']) {
			$adv = explode("\n", base64_decode($a_global['advanced']));
			foreach($adv as $adv_line) {
				fwrite($fd, "\t" . str_replace("\r", "", $adv_line) . "\n");

			}
		}
		fwrite ($fd, "\n");
		
		$localstatsport = $a_global['localstatsport'];
		if ($localstatsport){
			fwrite ($fd, "listen HAProxyLocalStats\n");
			fwrite ($fd, "\tbind 127.0.0.1:$localstatsport\n");
			fwrite ($fd, "\tmode http\n");
			fwrite ($fd, "\tstats enable\n");
			if (is_numeric($a_global['localstats_refreshtime']))
				fwrite ($fd, "\tstats refresh {$a_global['localstats_refreshtime']}\n");
			fwrite ($fd, "\tstats admin if TRUE\n");
			fwrite ($fd, "\tstats uri /haproxy_stats.php?haproxystats=1\n");
			fwrite ($fd, "\ttimeout client 5000\n");
			fwrite ($fd, "\ttimeout connect 5000\n");
			fwrite ($fd, "\ttimeout server 5000\n");
			fwrite ($fd, "\n");
		}
	}

	// Try and get a unique array for address:port as frontends can duplicate
	$a_bind = array();
	if(is_array($a_frontends)) {
		foreach ($a_frontends as $frontend) {
			if($frontend['status'] != 'active')
			{
				unlink_if_exists("var/etc/{$frontend['name']}.{$frontend['port']}.crt");
				continue;
			}
			if(!$frontend['backend_serverpool'])
			{
				unlink_if_exists("var/etc/{$frontend['name']}.{$frontend['port']}.crt");
				continue;
			}
			$primaryfrontend = get_primaryfrontend($frontend);
			$bname = get_frontend_ipport($frontend);

			//check ssl info
			if (strtolower($primaryfrontend['type']) == "http" && $frontend['ssloffload']){
				//ssl crt ./server.pem ca-file ./ca.crt verify optional crt-ignore-err all crl-file ./ca_crl.pem
				$filename = "$configpath/{$frontend['name']}.{$frontend['port']}.pem";
				$ssl_crt = " crt $filename";
				haproxy_write_certificate_file($filename, $frontend['ssloffloadcert']);
				$subfolder = "$configpath/{$frontend['name']}.{$frontend['port']}";
				$certs = $frontend['ha_certificates']['item'];
				if (is_array($certs)){
					if (count($certs) > 0){
						make_dirs($subfolder);
						foreach($certs as $cert){
							haproxy_write_certificate_file("$subfolder/{$cert['ssl_certificate']}.pem", $cert['ssl_certificate']);
						}
						$ssl_crt .= " crt $subfolder";
					}
				}
			}else{
				$ssl_crt="";
				unlink_if_exists("var/etc/{$frontend['name']}.{$frontend['port']}.crt");
			}

			if (!is_array($a_bind[$bname])) {
				$a_bind[$bname] = array();
				$a_bind[$bname]['config'] = array();
				// Settings which are used only from the primary frontend
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
			
			if (($frontend['secondary'] != 'yes') && ($frontend['name'] != $b['name'])) {
				// only 1 frontend can be the primary for a set of frontends that share 1 address:port.
					$input_errors[] = "Multiple primary frontends for $bname use the 'Shared Frontend' option instead";
			}
			
			if ($ssl_crt != "") {
				if ($b['ssl_info'] == "")
					$b['ssl_info'] = "ssl {$frontend['dcertadv']}";
				$b['ssl_info'] .= $ssl_crt;
			}

			// pointer to each frontend 
			$b['config'][] = $frontend;
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
			
			if($bind['type'] == "http") {
				// ssl offloading is only possible in http mode.
				$ssl_info = $bind['ssl_info'];
				$advanced_bind = $bind['advanced_bind'];
			} else {
				$ssl_info = "";
				$advanced_bind = "";
			}
			// Initialize variable
			$listenip = "";
			
			// Process and add bind directives for ports
			$ip = haproxy_interface_ip($bind['extaddr']);
			if ($ip){
				foreach($ports as $alias_or_port) {
					if($alias_or_port) {
						$portsnumeric = group_ports(haproxy_portoralias_to_list($alias_or_port));
						foreach($portsnumeric as $portnumeric) {
							$portnumeric = str_replace(":","-",$portnumeric);
							$listenip .=  "\tbind\t\t\t$ip:{$portnumeric} {$ssl_info} {$advanced_bind}\n";
						}
					}
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

			// https is an alias for tcp for clarity purposes
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

	
			// Combine the rest of the frontend configs
			$default_backend = "";
			$i = 0;
			foreach ($bind['config'] as $frontend) {
				$a_acl = get_frontend_acls($frontend);

				$poolname = $frontend['backend_serverpool'] . "_" . strtolower($bind['type']);

				if (!isset($a_pendingpl[$poolname])) {
					$a_pendingpl[$poolname] = array();
					$a_pendingpl[$poolname]['name'] = $poolname;
					$a_pendingpl[$poolname]['backend'] = $frontend['backend_serverpool'];
					$a_pendingpl[$poolname]['frontend'] = $bind;
				}
				
				// Write this out once, and must be before any backend config text
				if (($default_backend == "" || $frontend['secondary'] != 'yes') && count($a_acl) == 0 ) {
					$default_backend = $poolname;
				}
				
				// combine acl's with same name to allow for 'combined checks' to check for example hostname and fileextension together..
				$a_acl_combine = array();
				foreach ($a_acl as $entry) {
					$name = $entry['ref']['name'];
					$a_acl_combine[$name][] = $entry['ref'];
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
			if ($default_backend)
				fwrite ($fd, "\tdefault_backend\t\t" . $default_backend . "\n");
			
			foreach($advancedextra as $extra)
				fwrite ($fd, "\t".$extra."\n");
			fwrite ($fd, "\n");
		}
	}
	// Construct and write out configuration for each "backend"
	if (is_array($a_pendingpl) && is_array($a_backends)) {
		foreach ($a_pendingpl as $pending) {
			foreach ($a_backends as $pool) {
				if ($pending['backend'] == $pool['name']) {
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

function haproxy_get_transparent_backends(){
	global $config;
	$a_backends = &$config['installedpackages']['haproxy']['ha_pools']['item'];
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
					$item['name'] = $be['name'];
					$item['interface'] = $real_if;
					$item['address'] = $be['address'];
					$item['port'] = $be['port'];
					$transparent_backends[] = $item;
				}
			}
		}
	}
	return $transparent_backends;
}

function haproxy_generate_rules($type) {
	// called by filter.inc when pfSense rules generation happens
	global $g, $config;
	$rules = "";
	switch($type) {
	case 'filter':
		$transparent_backends = haproxy_get_transparent_backends();
		foreach($transparent_backends as $tb){
			// This sloppy rule is needed because of ipfw is used to 'catch' return traffic.
			$rules .= "# allow HAProxy transparent traffic\n";
			$rules .= "pass  out  quick  on {$tb['interface']} inet proto tcp  from any to {$tb['address']} port {$tb['port']} flags S/SA keep state ( sloppy )  label \"HAPROXY_transparent_rule_{$tb['name']}\"\n";
		}		
		break;
	}
	return $rules;
}

function load_ipfw_rules() {
	// On FreeBSD 8 pf does not support "divert-reply" so ipfw is needed.
	global $g, $config;
	$ipfw_zone_haproxy = "haproxy";
	
	$a_backends = &$config['installedpackages']['haproxy']['ha_pools']['item'];
	
	haproxy_load_modules();

	$transparent_backends = haproxy_get_transparent_backends();
	
	$transparent_interfaces = array();
	foreach($transparent_backends as $transparent_backend){
		$interface = $transparent_backend['interface'];
		$transparent_interfaces[$interface] = 1;
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

function haproxy_plugin_carp($pluginparams) {
	// called by pfSense when a CARP interface changes its state (called multiple times when multiple interfaces change state)
	// $pluginparams['type'] always 'carp'
	// $pluginparams['event'] either 'rc.carpmaster' or 'rc.carpbackup'
	// $pluginparams['interface'] contains the affected interface
	$type = $pluginparams['type'];
	$event = $pluginparams['event'];
	$interface = $pluginparams['interface'];
	haproxy_check_run(0);
}

function haproxy_check_run($reload) {
	global $config, $g, $haproxy_run_message;

	$haproxylock = lock("haproxy", LOCK_EX);
	$a_global = &$config['installedpackages']['haproxy'];
	$configpath = "{$g['varetc_path']}/haproxy";
	
	if ($reload)
		haproxy_writeconf($configpath);

	if(isset($a_global['enable'])) {
		if (isset($a_global['carpdev'])) {
			$status = get_carp_interface_status($a_global['carpdev']);
			if ($status != "MASTER") {
				if (haproxy_is_running()) {
					log_error("Stopping haproxy on CARP backup.");
					//exec("/bin/pkill -F /var/run/haproxy.pid haproxy");//doesnt work for multiple pid's in a pidfile
					haproxy_kill();
				}
				unlock($haproxylock);
				return (0);
			} else if (haproxy_is_running() && $reload == 0) { 
				unlock($haproxylock);
				return (0);
			}
			log_error("Starting haproxy on CARP master.");
			/* fallthrough */
		} else if ($reload == 0){
			unlock($haproxylock);
			return (0);
		}

		if(use_transparent_clientip_proxying()) {
			filter_configure();
			load_ipfw_rules();
		} else
			mwexec("/usr/local/sbin/ipfw_context -d haproxy", true);
		
		if (haproxy_is_running()) {
			if (isset($a_global['terminate_on_reload']))
				$sf_st = "-st";//terminate old process as soon as the new process is listening
			else
				$sf_st = "-sf";//finish serving existing connections exit when done, and the new process is listening
			exec("/usr/local/sbin/haproxy -f {$configpath}/haproxy.cfg -p /var/run/haproxy.pid $sf_st `cat /var/run/haproxy.pid` 2>&1", $output, $errcode);
		} else {
			exec("/usr/local/sbin/haproxy -f {$configpath}/haproxy.cfg -p /var/run/haproxy.pid -D 2>&1", $output, $errcode);
		}
		foreach($output as $line)
			$haproxy_run_message .= "<br/>" . htmlspecialchars($line) . "\n";
	} else {
		if ($reload && haproxy_is_running()) {
			//exec("/bin/pkill -F /var/run/haproxy.pid haproxy");//doesnt work for multiple pid's in a pidfile
			haproxy_kill();
		}
		$errcode = 0;
	}
	unlock($haproxylock);
	return ($errcode);
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

function get_frontend_ipport($frontend,$userfriendly=false) {
	$mainfrontend = get_primaryfrontend($frontend);
	$result = haproxy_interface_ip($mainfrontend['extaddr'], $userfriendly);
	if ($userfriendly and is_ipaddrv6($result))
		$result = "[{$result}]";
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

function get_frontend_acls($frontend) {
	$mainfrontend = get_primaryfrontend($frontend);
	$result = array();
	$a_acl = &$frontend['ha_acls']['item'];
	if (is_array($a_acl))
	{
		foreach ($a_acl as $entry) {
			$acl = haproxy_find_acl($entry['expression']);
			if (!$acl)
				continue;

			// Filter out acls for different modes
			if ($acl['mode'] != '' && $acl['mode'] != strtolower($mainfrontend['type']))
				continue;
			
			$acl_item = array();
			$acl_item['descr'] = $acl['name'] . ": " . $entry['value'];
			$acl_item['ref'] = $entry;
			
			$result[] = $acl_item;
		}
	}
	
	if (strtolower($mainfrontend['type']) == "http" && $mainfrontend['ssloffload']) {
		$a_acl = &$frontend['ha_acls']['item'];
		if(!is_array($a_acl)) 
			$a_acl=array();
	
		$poolname = $frontend['backend_serverpool'] . "_" . strtolower($frontend['type']);
		$aclname = "SNI_" . $poolname;
		if ($frontend['ssloffloadacl']){
			$cert = lookup_cert($frontend['ssloffloadcert']);
			$cert_cn = cert_get_cn($cert['crt']);
			$descr = haproxy_escape_acl_name($cert['descr']);
			unset($cert);
			$acl_item = array();
			$acl_item['descr'] = "Certificate ACL ".$cert_cn;
			$acl_item['ref'] = array('name' => "{$aclname}_{$descr}",'expression' => 'host_matches', 'value' => $cert_cn);
			$result[] = $acl_item;
		}
		if ($frontend['ssloffloadacladditional']){
			$certs = $frontend['ha_certificates']['item'];
			if (is_array($certs)){
				foreach($certs as $certref){
					$cert = lookup_cert($certref['ssl_certificate']);
					$cert_cn = cert_get_cn($cert['crt']);
					$descr = haproxy_escape_acl_name($cert['descr']);
					unset($cert);
					$acl_item = array();
					$acl_item['descr'] = "Additional certificate ACLs: ".$cert_cn;
					$acl_item['ref'] = array('name' => "{$aclname}_{$descr}",'expression' => 'host_matches', 'value' => $cert_cn);
					$result[] = $acl_item;
				}
			}
		}
	}
	return $result;
}

function get_backend($name) {
	global $config;
	$a_backend = &$config['installedpackages']['haproxy']['ha_pools']['item'];
	if(is_array($a_backend))
		foreach($a_backend as $key => $backend)
		{
			if ($backend['name'] == $name)
				return $backend;
		}
	return null;
}

function haproxy_escapestring($configurationsting) {
	$result = str_replace('\\', '\\\\', $configurationsting);
	$result = str_replace(' ', '\\ ', $result);
	return str_replace('#', '\\#', $result);
}

function haproxy_escape_acl_name($aclname) {
	return preg_replace_callback('([^A-Za-z0-9\._\-\:])', function($match){return "_".dechex(ord($match[0]));}, $aclname);
}

function haproxy_find_create_certificate($certificatename) {
	global $g;
	$cert = lookup_cert_by_name($certificatename);
	if (is_array($cert))
		return $cert;
	global $config;
	$a_cert =& $config['cert'];	
	$cert = array();
	$cert['refid'] = uniqid();
	$cert['descr'] = gettext($certificatename);
	mwexec("/usr/local/bin/openssl genrsa 1024 > {$g['tmp_path']}/ssl.key");
	mwexec("/usr/local/bin/openssl req -new -x509 -nodes -sha256 -days 2000 -key {$g['tmp_path']}/ssl.key > {$g['tmp_path']}/ssl.crt");
	$crt = file_get_contents("{$g['tmp_path']}/ssl.crt");
	$key = file_get_contents("{$g['tmp_path']}/ssl.key");
	unlink("{$g['tmp_path']}/ssl.key");
	unlink("{$g['tmp_path']}/ssl.crt");
	cert_import($cert, $crt, $key);
	$a_cert[] = $cert;
	return $cert;
}

?>
