<?php
/*	unbound.inc
	(C)2010 Warren Baker
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1.	Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

	2.	Redistributions in binary form must reproduce the above copyright
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

if(!function_exists("is_service_running"))
	require_once("service-utils.inc");

if(!function_exists("get_dns_servers"))
	require_once("pfsense-utils.inc");

if(!function_exists("get_nameservers"))
	require_once("system.inc");
	

function unbound_initial_setup() {
	global $config, $g;

	// Make sure read-write
	conf_mount_rw();
	
	if (!array($config['installedpackages']['unbound']['config']))
		$config['installedpackages']['unbound']['config'] = array();

	$unbound_config = &$config['installedpackages']['unbound']['config'][0];

	// Ensure Unbound user exists
	exec("/usr/sbin/pw useradd unbound");

	// Setup unbound
	// Create and chown dirs
	mwexec("/bin/mkdir -p /usr/local/etc/unbound /usr/local/etc/unbound/dev");
	@chown("/usr/local/etc/unbound/.", "unbound");
	@chown("/usr/local/etc/unbound/dev.", "unbound");
	// Touch needed files
	@touch("/usr/local/etc/unbound/root.hints");
	@touch("/usr/local/etc/unbound/root-trust-anchor");
	// Ensure files and folders belong to unbound
	@chown("/usr/local/etc/unbound/root-trust-anchor", "unbound");
	@chgrp("/usr/local/etc/unbound/root-trust-anchor", "wheel");
	@chmod("/usr/local/etc/unbound/root-trust-anchor", 0600);
	// We do not need the sample conf or the default rc.d startup file
	@unlink_if_exists("/usr/local/etc/unbound/unbound.conf.sample");
	@unlink_if_exists("/usr/local/etc/rc.d/unbound");
		
	// Setup rc file for startup and shutdown.
	unbound_rc_setup();
	
	// Set initial interfaces that are allowed to query to lan, if that does not exist set it to the wan
	if(count($config['interfaces']) > 1)
		$unbound_config['active_interface'] = "lan";
	else
		$unbound_config['active_interface'] = "wan";
	
	unbound_anchor_setup();
	unbound_resync_config();
	unbound_keys_setup();

	exec("/usr/sbin/chown -R unbound:wheel /usr/local/etc/unbound/*");

	// Write out the XML config
	write_config();
	
	// Back to read-only
	conf_mount_ro();
}

function unbound_anchor_setup() {
	
	$conf = <<<EOD
. IN DS 19036 8 2 49AAC11D7B6F6446702E54A1607371607A1A41855200FD2CE1CDDE32F24E8FB5
EOD;

	file_put_contents("/usr/local/etc/unbound/root-trust-anchor", $conf);
	
}

function unbound_keys_setup() {
	
	// Generate SSL Keys for controlling the unbound server
	mwexec("/usr/local/sbin/unbound-control-setup");

}

function unbound_rc_setup() {
	global $config;


	// Startup process and idea taken from TinyDNS package (author sullrich@gmail.com)
	$filename = "unbound.sh";
	$start = "/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDPHP
<?php
	require_once(\"/usr/local/pkg/unbound.inc\");
	echo \"Starting and configuring Unbound...\";
	fetch_root_hints();
	unbound_control(\"start\");
	unbound_control(\"forward\");
	echo \"done.\\n\";
?>
ENDPHP\n";

$stop = "/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDPHP
<?php
	require_once(\"/usr/local/pkg/unbound.inc\");
	echo \"Stopping Unbound...\";
	unbound_control(\"termstop\");
	echo \"done.\\n\";
?>

ENDPHP\n";

	write_rcfile(array(
					"file" => $filename,
					"start" => $start,
					"stop" => $stop
					)
				);

}

function unbound_install() {
	
	conf_mount_rw();
	unbound_initial_setup();
	conf_mount_ro();
	
}

function unbound_control($action) {
	global $config, $g;
	
	$unbound_config = $config['installedpackages']['unbound']['config'][0];
	
	switch ($action) {
		case "forward":
				/* Dont utilize forward cmd if Unbound is doing DNS queries directly
				 * XXX: We could make this an option to then make pfSense use Unbound
				 * as the recursive nameserver instead of upstream ones(?)
				 */ 
				if ($unbound_config['forwarding_mode'] == "on") {
					// Get configured DNS servers and add them as forwarders
					if (!isset($config['system']['dnsallowoverride'])) {
						$ns = array_unique(get_nameservers());
						foreach($ns as $nameserver) {
							if($nameserver)
								$dns_servers .= " $nameserver";
						}
					} else {
						$ns = array_unique(get_dns_servers());
						foreach($ns as $nameserver) {
							if($nameserver)
								$dns_servers .= " $nameserver";
						}
					}
					
					if(is_service_running("unbound")) {
						unbound_ctl_exec("forward $dns_servers");
					} else {
						unbound_control("start");
						unbound_control("forward");
					}
				}
				break;
		
		case "start":
				//Start unbound
				if($unbound_config['unbound_status'] == "on") {
					unbound_ctl_exec("start");
					fetch_root_hints();
					sleep(1);
				}
				break;
		
		case "stop":
				//Stop unbound and unmount the file system
				if($unbound_config['unbound_status'] == "on") {
					unbound_ctl_exec("stop");	
				}
				break;
		
		case "termstop":
				//Stop Unbound by sigkillbypid();
				sigkillbypid("{$g['varrun_path']}/unbound.pid", "TERM");
				break;
		
		default:
				break;
				
		}
	
}

function unbound_get_network_interface_addresses($subnet=false, $mask=false) {
	global $config;
	
	/* calculate interface ip + subnet information */
	$interfaces = explode(",", $config['installedpackages']['unbound']['config'][0]['active_interface']);
	$unbound_interfaces = array();
	foreach ($interfaces as $unboundidx => $unboundif) {
		$unboundrealif = convert_friendly_interface_to_real_interface_name($unboundif);
		$unboundip = find_interface_ip($unboundrealif);
		$ipmask = find_interface_subnet($unboundrealif);
	
		// If $subnet is passed then calculate the beginning of the network range for the IP address
		if ($subnet)
			$network = gen_subnet($unboundip, $ipmask);
		else
			$network = $unboundip;
			
		if ($mask) 
			$unbound_interfaces[] = "$network/$ipmask";
		else {
			$unbound_interfaces[] = $network;
			// Check for CARP addresses and also return those
			if (isset($config['virtualip'])) {
				if(is_array($config['virtualip']['vip'])) {
					foreach($config['virtualip']['vip'] as $vip) {
						if (($vip['interface'] == $unboundif) && ($vip['mode'] == "carp")) {
							$virtual_ip = find_interface_ip(link_ip_to_carp_interface($vip['subnet']));
							$unbound_interfaces[] = $virtual_ip;
						}
					}
				}
			}
		}
	}
	
	return $unbound_interfaces;
	
}

function unbound_acls_config() {
	global $config;
	
	if(array($config['installedpackages']['unboundacls']['config'])) {
		$unbound_acls = $config['installedpackages']['unboundacls']['config'];
		$unboundcfg = "";
		foreach($unbound_acls as $unbound_acl){
			$unboundcfg .= "#{$unbound_acl['aclname']}\n";
			foreach($unbound_acl['row'] as $network) {
				$unboundcfg .= "access-control: {$network['acl_network']}/{$network['mask']} {$unbound_acl['aclaction']}\n";
			}			
		}
		return $unboundcfg;
	} else {
		return;
	}
}

function unbound_resync_config() {
	global $config, $g;
	
	if (!array($config['installedpackages']['unbound']['config']))
		$config['installedpackages']['unbound']['config'] = array();

	$unbound_config = &$config['installedpackages']['unbound']['config'][0];
	
	// Add networks physically attached to allowed networks and then call the acls		
	$interfaces = unbound_get_network_interface_addresses(true, true);
	foreach($interfaces as $allowed_network) {
		$unbound_allowed_networks .= "access-control: $allowed_network allow\n";
	}
	$unbound_allowed_networks .= unbound_acls_config();
	
	if($unbound_config['dnssec_status'] == "on") {
		$module_config = "validator iterator";
		$anchor_file = "auto-trust-anchor-file: /usr/local/etc/unbound/root-trust-anchor";
	} else {
		$module_config = "iterator";
	}
	
	// Interfaces to bind to
	$interface_ips = unbound_get_network_interface_addresses();
	foreach($interface_ips as $ifip) {
		$unbound_bind_interfaces .="interface: $ifip\n";
	}
	
	/* Harden DNSSec responses - if DNSSec is absent, zone is marked as bogus
	 * XXX: for now we always have this set to yes
	 */
	$unbound_config['harden-dnssec-stripped'] = "yes";
	
	// Host entries
	$host_entries = unbound_add_host_entries();
	
	// Domain Overrides
	$domain_overrides = unbound_add_domain_overrides();
	
	// Unbound Statistics
	if($unbound_config['stats'] == "on") {
		$stats_interval = $unbound_config['stats_interval'];
		$cumulative_stats = $unbound_config['cumulative_stats'];
		if ($unbound_config['extended_stats'] == "on")
			$extended_stats = "yes";
		else
			$extended_stats = "no";	
	} else {
		$stats_interval = "0";
		$cumulative_stats = "no";
		$extended_stats = "no";
	}
	
	// Private-address support for DNS Rebinding
	if($unbound_config['private_address'] == "on") {
		$pvt_addr = <<<EOF
# For DNS Rebinding prevention
private-address: 10.0.0.0/8
private-address: 172.16.0.0/12
private-address: 192.168.0.0/16
private-address: 192.254.0.0/16
# private-address: fd00::/8
# private-address: fe80::/10
# Set private domains in case authorative name server returns a RFC1918 IP address
EOF;

	// Add private-domain options
	$private_domains = unbound_add_domain_overrides(true);

	}

	//Setup optimization
	$optimization = unbound_optimization();

	$unbound_conf = <<<EOD
#########################
# Unbound configuration #
#########################

###
# Server config
###
server:
{$optimization['number_threads']}
{$optimization['msg_cache_slabs']}
{$optimization['rrset_cache_slabs']}
{$optimization['infra_cache_slabs']}
{$optimization['key_cache_slabs']}
{$optimization['msg_cache_size']}
{$optimization['rrset_cache_size']}
{$optimization['outgoing_range']}
{$optimization['so_rcvbuf']}
num-queries-per-thread: 1024
verbosity: 1
port: 53
do-ip4: yes
do-ip6: no
do-udp: yes
do-tcp: yes
do-daemonize: yes
statistics-interval: {$stats_interval}
extended-statistics: {$extended_stats}
statistics-cumulative: {$cumulative_stats}
# Interface IP(s) to bind to
{$unbound_bind_interfaces}
chroot: ""
username: "unbound"
directory: "/usr/local/etc/unbound"
pidfile: "{$g['varrun_path']}/unbound.pid"
root-hints: "root.hints"
harden-dnssec-stripped: {$unbound_config['harden-dnssec-stripped']}
harden-referral-path: no
prefetch: yes
prefetch-key: yes
use-syslog: yes
module-config: "{$module_config}"
unwanted-reply-threshold: 10000000
{$anchor_file}
#### Access Control ####
# Local attached networks allowed to utilize service and any user added ACLs
access-control: 127.0.0.0/8 allow
{$unbound_allowed_networks}
{$pvt_addr}
{$private_domains}

# Host entries
{$host_entries}
# Domain overrides
{$domain_overrides}

###
# Remote Control Config
###
remote-control:
control-enable: yes
control-interface: 127.0.0.1
control-port: 953
server-key-file: "/usr/local/etc/unbound/unbound_server.key"
server-cert-file: "/usr/local/etc/unbound/unbound_server.pem"
control-key-file: "/usr/local/etc/unbound/unbound_control.key"
control-cert-file: "/usr/local/etc/unbound/unbound_control.pem"

EOD;

	file_put_contents("/usr/local/etc/unbound/unbound.conf", $unbound_conf);
	
}

function unbound_ctl_exec($cmd) {
	
	mwexec("/usr/local/sbin/unbound-control $cmd");
		
}


/* unbound_optimization - custom settings that can be configured to make Unbound perform better
 * in larger installations.
 */
function unbound_optimization() {
	global $config;

	$unbound_config = $config['installedpackages']['unbound']['config'][0];	
	$optimization_settings = array();
	
	// Set the number of threads equal to number of CPUs.
	// Use 1 (disable threading) if for some reason this sysctl fails.
	$numprocs = intval(trim(`/sbin/sysctl kern.smp.cpus | /usr/bin/cut -d" " -f2`));
	if($numprocs > 0)
		$optimization['number_threads'] = "num-threads: {$numprocs}";
	else
		$optimization['number_threads'] = "num-threads: 1";
	
	// Slabs to help reduce lock contention.
	if ($numprocs > 4) {
		$optimization['msg_cache_slabs'] = "msg-cache-slabs: {$numprocs}";
		$optimization['rrset_cache_slabs'] = "rrset-cache-slabs: {$numprocs}";
		$optimization['infra_cache_slabs'] = "infra-cache-slabs: {$numprocs}";
		$optimization['key_cache_slabs'] = "key-cache-slabs: {$numprocs}";
	} else {
		$optimization['msg_cache_slabs'] = "msg-cache-slabs: 4";
		$optimization['rrset_cache_slabs'] = "rrset-cache-slabs: 4";
		$optimization['infra_cache_slabs'] = "infra-cache-slabs: 4";
		$optimization['key_cache_slabs'] = "key-cache-slabs: 4";
	}
	
	// Memory usage - default is 4Mb if nothing has been selected
	if(isset($unbound_config['msg_cache_size'])) {
		$rr = $unbound_config['msg_cache_size']*2;
		$optimization['msg_cache_size'] = "msg-cache-size: {$unbound_config['msg_cache_size']}m";
		$optimization['rrset_cache_size'] = "rrset-cache-size: {$rr}m";
	} else {
		$optimization['msg_cache_size'] = "msg-cache-size: 4m";
		$optimization['rrset_cache_size'] = "rrset_cache_size: 8m";
	}
	
	// More outgoing connections per thread otherwise assign a default of 4096 for a single thread
	if($numprocs > 0) {
		$or = (1024/$numprocs) - 50;
		$optimization['outgoing_range'] = "outgoing-range: {$or}";
	} else {
		$optimization['outgoing_range'] = "outgoing-range: {4096}";
	}

	// Larger socket buffer for busy servers
	// Check that it is set to 4MB (by default the OS has it configured to 4MB)
	foreach ($config['sysctl']['item'] as $tunable) {
		if ($tunable['tunable'] == 'kern.ipc.maxsockbuf') {
			$so = floor(($tunable['value']/1024/1024)-1);
			// Check to ensure that the number is not a negative
			if ($so > 0)
				$optimization['so_rcvbuf'] = "so-rcvbuf: {$so}m";
			else
				unset($optimization['so_rcvbuf']);
			
		}
	}
	// Safety check in case kern.ipc.maxsockbuf is deleted.
	if(!isset($optimization['so_rcvbuf']))
		$optimization['so_rcvbuf'] = "#so-rcvbuf: 4m";

	return $optimization;
}

function fetch_root_hints() {

	$destination_file = "/usr/local/etc/unbound/root.hints";
	if (filesize($destination_file) == 0 ) {
		conf_mount_rw();
		$fout = fopen($destination_file, "w");
		$url = "ftp://ftp.internic.net/domain/named.cache";
	
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch,CURLOPT_RETURNTRANSFER, 1);
		curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, '25');
		$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
		$data = curl_exec($ch);
		curl_close($ch);

		fwrite($fout, $data);
		fclose($fout);
		conf_mount_ro();
	
		return ($http_code == 200) ? true : $http_code;
	} else {
		return false;
	}
}

function unbound_validate($post, $acl=false) {
	global $config, $input_errors;

	if($post['unbound_status'] == "on" && isset($config['dnsmasq']['enable']))
		$input_errors[] = "The system dns-forwarder is still active. Disable it before enabling the Unbound service.";
		
	if($acl) {
		$acls = $post;
		// Check to ensure values entered is an action that is in the list
		if ($acls['aclaction'] != 'refuse' && $acls['aclaction'] != 'allow' && $acls['aclaction'] && 'allow_snoop' && $acls['aclaction'] != 'deny')
			$input_errors[] = "{$acls['aclaction']} is not a valid ACL Action. Please select one of the four actions defined in the list.";
		
		// Make sure there is at least 1 network defined.
		if (!isset($acls['acl_network0']))	
			$input_errors[] = "You need to specify at least one network to create a valid ACL.";
			
		$count = 0;
		// Get number of rows added, should be passed by the form - will look into that later
		for($i=0; $i<99; $i++) {
			if (isset($acls['acl_network'.$i])) {
				// Check to ensure values entered are networks
				if(!is_ipaddr($acls['acl_network'.$i]) && !is_subnet($acls['mask'.$i]))
					$input_errors[] = "{$acls['acl_network'.$i]}/{$acls['mask'.$i]} is not a valid network.";
			}				
		}
	}	
}

function unbound_reconfigure() {
	global $config, $g, $input_errors;
	
	$unbound_config = $config['installedpackages']['unbound']['config'][0];
	
	if ($unbound_config['unbound_status'] != "on") {
		if(is_service_running("unbound")) {
			unbound_control("termstop");
		}
	} else {
		if(is_service_running("unbound")) {
			unbound_control("termstop");
		}
		unbound_resync_config();
		unbound_control("start");
		unbound_control("forward");
	}	
}

function unbound_uninstall() {
	global $g, $config;

	conf_mount_rw();

	unbound_control("termstop");
	// Remove pkg config directory and startup file
	mwexec("rm -rf /usr/local/etc/unbound");
	mwexec("rm -f /usr/local/etc/rc.d/unbound.sh");
	mwexec("rm -f {$g['varlog_path']}/unbound.log");

	// Remove unbound user
	exec("/usr/sbin/pw userdel unbound");

    conf_mount_ro();

}

function read_hosts() {
	
	// Open /etc/hosts and extract the only dhcpleases info
	$etc_hosts = array();
   	foreach (file('/etc/hosts') as $line) {
		$d = preg_split('/\s/', $line, -1, PREG_SPLIT_NO_EMPTY);
		if (empty($d) || substr(reset($d), 0, 1) == "#")
			continue;
		if ($d[3] == "#") {
			$ip = array_shift($d);
			$fqdn = array_shift($d);
			$name = array_shift($d);
			if ($fqdn != "empty") {
				if ($name != "empty")
					array_push($etc_hosts, array(ipaddr => "$ip", fqdn => "$fqdn", name => "$name"));
				else
					array_push($etc_hosts, array(ipaddr => "$ip", fqdn => "$fqdn"));
			}
		}
	}
	return $etc_hosts;
}

/* Setup /etc/hosts entries by overriding with local-data
 */
function unbound_add_host_entries() {
	global $config;
	
	/* XXX: break this out into a separate config file and make use of include */
	
	$unboundcfg = $config['installedpackages']['unbound']['config'][0];
	$syscfg = $config['system'];
	$dnsmasqcfg = $config['dnsmasq'];
	
	$unbound_entries = "local-zone: \"{$syscfg['domain']}\" transparent\n";
	// IPv4 entries
	$unbound_entries .= "local-data-ptr: \"127.0.0.1 localhost\"\n";
	$unbound_entries .= "local-data: \"localhost A 127.0.0.1\"\n";
	$unbound_entries .= "local-data: \"localhost.{$syscfg['domain']} A 127.0.0.1\"\n";
	
	if ($config['interfaces']['lan']) {
		$cfgip = get_interface_ip("lan");
		if (is_ipaddr($cfgip)) {
			$unbound_entries .= "local-data-ptr: \"{$cfgip} {$syscfg['hostname']}.{$syscfg['domain']}\"\n";
			$unbound_entries .= "local-data: \"{$syscfg['hostname']}.{$syscfg['domain']} A {$cfgip}\"\n";
			$unbound_entries .= "local-data: \"{$syscfg['hostname']} A {$cfgip}\"\n";
		}
	} else {
		$sysiflist = get_configured_interface_list();
		foreach ($sysiflist as $sysif) {
			if (!interface_has_gateway($sysif)) {
				$cfgip = get_interface_ip($sysif);
				if (is_ipaddr($cfgip)) {
					$unbound_entries .= "local-data-ptr: \"{$cfgip} {$syscfg['hostname']}.{$syscfg['domain']}\"\n";
					$unbound_entries .= "local-data: \"{$syscfg['hostname']}.{$syscfg['domain']} A {$cfgip}\"\n";
					$unbound_entries .= "local-data: \"{$syscfg['hostname']} A {$cfgip}\"\n";
					break;
				}
			}
		}
	}

	// DNSMasq entries static host entries
	if (isset($dnsmasqcfg['hosts'])) {
		$hosts = $dnsmasqcfg['hosts'];
		$host_entries = "";
		$added_item = array();
		foreach ($hosts as $host) {
			$current_host = $host['host'];
			if(!$added_item[$current_host]) {
				$host_entries .= "local-data-ptr: \"{$host['ip']} {$host['host']}.{$host['domain']}\"\n";
				$host_entries .= "local-data: \"{$host['host']}.{$host['domain']} IN A {$host['ip']}\"\n";
				if (!empty($host['descr']) && $unboundcfg['txtsupport'] == 'on')
					$host_entries .= "local-data: '{$host['host']}.{$host['domain']} TXT \"".addslashes($host['descr'])."\"'\n";
					
				// Do not add duplicate entries
				$added_item[$current_host] = true;
			}
		}
		$unbound_entries .= $host_entries;	
	}
	// Static DHCP entries
	$host_entries = "";
	if (isset($unboundcfg['regdhcpstatic']) && is_array($config['dhcpd'])) {
		foreach ($config['dhcpd'] as $dhcpif => $dhcpifconf)
			if(is_array($dhcpifconf['staticmap']) && isset($dhcpifconf['enable']))
				foreach ($dhcpifconf['staticmap'] as $host)
					if ($host['ipaddr'] && $host['hostname']) {
						$host_entries .= "local-data-ptr: \"{$host['ipaddr']} {$host['hostname']}.{$syscfg['domain']}\"\n";
						$host_entries .= "local-data: \"{$host['hostname']}.{$syscfg['domain']} IN A {$host['ipaddr']}\"\n";
						if (!empty($host['descr']) && $unboundcfg['txtsupport'] == 'on')
							$host_entries .= "local-data: '{$host['hostname']}.{$syscfg['domain']} TXT \"".addslashes($host['descr'])."\"'\n";
					}
		$unbound_entries .= $host_entries;
    }

	// Handle DHCPLeases added host entries
	$dhcplcfg = read_hosts();
	$host_entries = "";
	if(is_array($dhcplcfg)) {
		foreach($dhcplcfg as $key=>$host) {
			$host_entries .= "local-data-ptr: \"{$host['ipaddr']} {$host['fqdn']}\"\n";
			$host_entries .= "local-data: \"{$host['fqdn']} IN A {$host['ipaddr']}\"\n";
			if (!empty($host['name'])) {
				$host_entries .= "local-data-ptr: \"{$host['ipaddr']} {$host['name']}\"\n";
				$host_entries .= "local-data: \"{$host['name']} IN A {$host['ipaddr']}\"\n";
			}
		}
		$unbound_entries .= $host_entries;
	}
	return $unbound_entries;
}

/* Setup any domain overrides that have been configured with stub-zone parameter
 */
function unbound_add_domain_overrides($pvt=false) {
	global $config;

	if (isset($config['dnsmasq']['domainoverrides'])) {
		$domains = $config['dnsmasq']['domainoverrides'];

		// Domain overrides that have multiple entries need multiple stub-addr: added
		$sorted_domains = msort($domains, "domain");
		$result = array();		
		foreach($sorted_domains as $domain) {
			$domain_key = current($domain);
			if(!isset($result[$domain_key])) {
				$result[$domain_key] = array();
			}
			$result[$domain_key][] = $domain['ip'];
		}
		
		$domain_entries = "";
		foreach($result as $domain=>$ips) {
			if($pvt == true) {
				$domain_entries .= "private-domain: \"$domain\"\n";
			} else {
				$domain_entries .= "stub-zone:\n";
				$domain_entries .= "\tname: \"$domain\"\n";
				foreach($ips as $ip) {
					$domain_entries .= "\tstub-addr: $ip\n";
				}
				$domain_entries .= "\tstub-prime: no\n";
			}
		}
		return $domain_entries;
	}	
}

?>