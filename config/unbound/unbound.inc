<?php
/*	unbound.inc
	(C)2013 Warren Baker (warren@decoy.co.za)
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

// Define basedir constant for unbound according to FreeBSD version (PBI support or no PBI)
if (floatval(php_uname("r")) >= 8.3)
	define("UNBOUND_BASE", "/usr/pbi/unbound-" . php_uname("m"));
else
	define("UNBOUND_BASE", "/usr/local");

if(!function_exists("is_service_running"))
	require_once("service-utils.inc");

if(!function_exists("get_dns_servers"))
	require_once("pfsense-utils.inc");

if(!function_exists("get_nameservers"))
	require_once("system.inc");

if(!function_exists("gen_subnetv6") || !function_exists("gen_subnet"))
	require_once("util.inc");

function unbound_initial_setup() {
	global $config, $g;

	// Make sure read-write
	conf_mount_rw();

	if (!is_array($config['installedpackages']['unbound']['config']))
		$config['installedpackages']['unbound']['config'] = array();

	$unbound_config = &$config['installedpackages']['unbound']['config'][0];

	// Ensure Unbound user exists
	mwexec("/usr/sbin/pw useradd unbound", true);

	// Setup unbound
	// Create and chown dirs
	mwexec("/bin/mkdir -p " . UNBOUND_BASE . "/etc/unbound/dev");
	@chown(UNBOUND_BASE . "/etc/unbound/.", "unbound");
	@chown(UNBOUND_BASE . "/etc/unbound/dev.", "unbound");
	// Touch needed files
	@touch(UNBOUND_BASE . "/etc/unbound/root.hints");
	@touch(UNBOUND_BASE . "/etc/unbound/root-trust-anchor");
	// Ensure files and folders belong to unbound
	@chown(UNBOUND_BASE . "/etc/unbound/root-trust-anchor", "unbound");
	@chgrp(UNBOUND_BASE . "/etc/unbound/root-trust-anchor", "wheel");
	@chmod(UNBOUND_BASE . "/etc/unbound/root-trust-anchor", 0600);
	// We do not need the sample conf or the default rc.d startup file
	@unlink_if_exists(UNBOUND_BASE . "/etc/unbound/unbound.conf.sample");
	@unlink_if_exists(UNBOUND_BASE . "/etc/rc.d/unbound");
	@unlink_if_exists("/usr/local/etc/rc.d/unbound");

	// Setup rc file for startup and shutdown.
	unbound_rc_setup();

	/* Check to see if Set initial interfaces that are allowed to query to lan, if that does not exist set it to the wan
	 *
	 */
	if(!isset($unbound_config['active_interface'])) {
		if (count($config['interfaces']) > 1)
			$unbound_config['active_interface'] = "lan";
		else
			$unbound_config['active_interface'] = "wan";
	}

	unbound_anchor_setup();
	unbound_resync_config();
	unbound_keys_setup();

	exec("/usr/sbin/chown -R unbound:wheel " . UNBOUND_BASE . "/etc/unbound/*");

	// Write out the XML config
	write_config();

	// Back to read-only
	conf_mount_ro();
}

function unbound_anchor_setup() {

	$conf = <<<EOD
. IN DS 19036 8 2 49AAC11D7B6F6446702E54A1607371607A1A41855200FD2CE1CDDE32F24E8FB5
EOD;

	file_put_contents(UNBOUND_BASE . "/etc/unbound/root-trust-anchor", $conf);

}

function unbound_keys_setup() {

	// Generate SSL Keys for controlling the unbound server
	mwexec(UNBOUND_BASE . "/sbin/unbound-control-setup");

}

function unbound_rc_setup() {
	global $config;

	// Startup process and idea taken from TinyDNS package (author sullrich@gmail.com)
	$filename = "unbound.sh";
	$start = "/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDPHP
<?php
	require_once(\"/usr/local/pkg/unbound.inc\");
	echo \"Starting and configuring Unbound...\";
	unbound_control(\"anchor_update\");
	unbound_control(\"start\");
	unbound_control(\"restore_cache\");
	echo \"done.\\n\";
?>
ENDPHP\n";

$stop = "/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDPHP
<?php
	require_once(\"/usr/local/pkg/unbound.inc\");
	echo \"Stopping Unbound...\";
	unbound_control(\"dump_cache\");
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
	unbound_initial_setup();
}

function unbound_control($action) {
	global $config, $g;

	$unbound_config = $config['installedpackages']['unbound']['config'][0];
	$cache_dumpfile = "/var/tmp/unbound_cache";

	switch ($action) {
		case "start":
				//Start unbound
				if($unbound_config['enable'] == "on") {
					if(!is_service_running("unbound"))
						unbound_ctl_exec("start");
						/* Link dnsmasq.pid to prevent dhcpleases logging error */
						if (!is_link("/var/run/dnsmasq.pid")) {
							@unlink("/var/run/dnsmasq.pid");
							mwexec("/bin/ln -s /var/run/unbound.pid /var/run/dnsmasq.pid");
						}
					mwexec_bg("/usr/local/bin/unbound_monitor.sh");
				}
				break;

		case "stop":
				//Stop unbound and unmount the file system
				if($unbound_config['enable'] == "on") {
					mwexec_bg("/usr/local/bin/unbound_monitor.sh stop");
					unbound_ctl_exec("stop");
				}
				break;

		case "termstop":
				//Stop Unbound by sigkillbypid();
				mwexec_bg("/usr/local/bin/unbound_monitor.sh stop");
				sigkillbypid("{$g['varrun_path']}/unbound.pid", "TERM");
				break;

		case "dump_cache":
				//Dump Unbound's Cache
				if($unbound_config['dumpcache'] == "on")
					unbound_ctl_exec("dump_cache > $cache_dumpfile");
				break;

		case "restore_cache":
				//Restore Unbound's Cache
				if ((is_service_running("unbound")) && ($unbound_config['dumpcache'] == "on")) {
					if(file_exists($cache_dumpfile) && filesize($cache_dumpfile) > 0)
						unbound_ctl_exec("load_cache < /var/tmp/unbound_cache");
				}
				break;
		case "anchor_update":
				//Update the Root Trust Anchor
				conf_mount_rw();
				mwexec(UNBOUND_BASE . "/sbin/unbound-anchor -a " . UNBOUND_BASE . "/etc/unbound/root-trust-anchor", true);
				conf_mount_ro();
				break;
		default:
				break;

		}

}

function unbound_get_network_interface_addresses() {
	global $config;

	$interfaces = $config['interfaces'];
	$unbound_config = $config['installedpackages']['unbound']['config'][0];
	$unboundint = explode(",", $unbound_config['active_interface']);
	$unbound_interfaces = array();
	$i = 0;

	foreach ($unboundint as $unboundidx => $unboundif) {
		/* Configure IPv4 addresses */
		if (is_ipaddr($interfaces[$unboundif]['ipaddr'])) {
			$unbound_interfaces[$i]['ipv4']['ipaddr'] = $interfaces[$unboundif]['ipaddr'];
			$unbound_interfaces[$i]['ipv4']['subnet'] = $interfaces[$unboundif]['subnet'];
			$unbound_interfaces[$i]['ipv4']['network'] = gen_subnet($unbound_interfaces[$i]['ipv4']['ipaddr'],$unbound_interfaces[$i]['ipv4']['subnet']);

			// Check for CARP addresses and also return those - only IPv4 for now
			if (isset($config['virtualip'])) {
				if(is_array($config['virtualip']['vip'])) {
					foreach($config['virtualip']['vip'] as $vip) {
						if (($vip['interface'] == $unboundif) && ($vip['mode'] == "carp")) {
							$virtual_ip = find_interface_ip(link_ip_to_carp_interface($vip['subnet']));
							if ($virtual_ip == '') {
								log_error("Unbound DNS: There was a problem setting up the Virtual IP for the interface ".link_ip_to_carp_interface($vip['subnet']));
							} else {
								$unbound_interfaces[$i]['virtual']['ipaddr'] = $virtual_ip;
								$unbound_interfaces[$i]['virtual']['subnet'] = $vip['subnet_bits'];
								$unbound_interfaces[$i]['virtual']['network'] = $virtual_ip;
							}
						}
					}
				}
			}
		} else if (isset($interfaces[$unboundif]['ipaddr'])) {
			/* Find the interface IP address for
			 * XXX - this only works for IPv4 currently - the pfSense module needs IPv6 love
			 */
			$unboundrealif = convert_friendly_interface_to_real_interface_name($unboundif);
			$unbound_interfaces[$i]['ipv4']['ipaddr'] = find_interface_ip($unboundrealif);
			$unbound_interfaces[$i]['ipv4']['subnet'] = find_interface_subnet($unboundrealif);
			$unbound_interfaces[$i]['ipv4']['network'] = gen_subnet($unbound_interfaces[$i]['ipv4']['ipaddr'],$unbound_interfaces[$i]['ipv4']['subnet']);
		}

		/* Configure IPv6 addresses */
		if(function_exists("is_ipaddrv6")) {
			if(is_ipaddrv6($interfaces[$unboundif]['ipaddrv6'])) {
				$unbound_interfaces[$i]['ipv6']['ipaddr'] = $interfaces[$unboundif]['ipaddrv6'];
				$unbound_interfaces[$i]['ipv6']['subnet'] = $interfaces[$unboundif]['subnetv6'];
				$unbound_interfaces[$i]['ipv6']['network'] = gen_subnetv6($unbound_interfaces[$i]['ipv6']['ipaddr'], $unbound_interfaces[$i]['ipv6']['subnet']);
			}
		}
		/* Lastly check for loopback addresses*/
		if($unboundif == "lo0") {
			$unbound_interfaces[$i]['loopback']['ipaddr'] = "127.0.0.1";
			if (function_exists("is_ipaddrv6"))
				$unbound_interfaces[$i]['loopback6']['ipaddr'] = "::1";
		}
		$i++;
	}
	return $unbound_interfaces;
}

function unbound_get_query_interface_addresses() {
	global $config;

	$interfaces = $config['interfaces'];
	$unbound_config = $config['installedpackages']['unbound']['config'][0];
	/* If no query interface is configured then just return false */
	if (empty($unbound_config['query_interface']))
		return false;
	else
		$unboundint = explode(",", $unbound_config['query_interface']);
	$unbound_interfaces = array();
	$i = 0;

	foreach ($unboundint as $unboundidx => $unboundif) {
		/* Configure IPv4 addresses */
		if (is_ipaddr($interfaces[$unboundif]['ipaddr'])) {
			$unbound_interfaces[$i]['ipv4']['ipaddr'] = $interfaces[$unboundif]['ipaddr'];
			$unbound_interfaces[$i]['ipv4']['subnet'] = $interfaces[$unboundif]['subnet'];
			$unbound_interfaces[$i]['ipv4']['network'] = gen_subnet($unbound_interfaces[$i]['ipv4']['ipaddr'],$unbound_interfaces[$i]['ipv4']['subnet']);

			// Check for CARP addresses and also return those - only IPv4 for now
			if (isset($config['virtualip'])) {
				if(is_array($config['virtualip']['vip'])) {
					foreach($config['virtualip']['vip'] as $vip) {
						if (($vip['interface'] == $unboundif) && ($vip['mode'] == "carp")) {
							$virtual_ip = find_interface_ip(link_ip_to_carp_interface($vip['subnet']));
							if ($virtual_ip == '') {
								log_error("Unbound DNS: There was a problem setting up the Virtual IP for the interface ".link_ip_to_carp_interface($vip['subnet']));
							} else {
								$unbound_interfaces[$i]['virtual']['ipaddr'] = $virtual_ip;
							}
						}
					}
				}
			}
		} else if(isset($interfaces[$unboundif]['ipaddr'])) {
			/* Find the interface IP address for
			 * XXX - this only works for IPv4 currently - the pfSense module needs IPv6 love
			 */
			$unboundrealif = convert_friendly_interface_to_real_interface_name($unboundif);
			$unbound_interfaces[$i]['ipv4']['ipaddr'] = find_interface_ip($unboundrealif);
			$unbound_interfaces[$i]['ipv4']['subnet'] = find_interface_subnet($unboundrealif);
			$unbound_interfaces[$i]['ipv4']['network'] = gen_subnet($unbound_interfaces[$i]['ipv4']['ipaddr'],$unbound_interfaces[$i]['ipv4']['subnet']);
		}

		/* Configure IPv6 addresses */
		if(function_exists("is_ipaddrv6")) {
			if(is_ipaddrv6($interfaces[$unboundif]['ipaddrv6'])) {
				$unbound_interfaces[$i]['ipv6']['ipaddr'] = $interfaces[$unboundif]['ipaddrv6'];
				$unbound_interfaces[$i]['ipv6']['subnet'] = $interfaces[$unboundif]['subnetv6'];
				$unbound_interfaces[$i]['ipv6']['network'] = gen_subnetv6($unbound_interfaces[$i]['ipv6']['ipaddr'], $unbound_interfaces[$i]['ipv6']['subnet']);
			}
		}
		/* Lastly check for loopback addresses*/
		if($unboundif == "lo0") {
			$unbound_interfaces[$i]['loopback']['ipaddr'] = "127.0.0.1";
			if (function_exists("is_ipaddrv6"))
				$unbound_interfaces[$i]['loopback6']['ipaddr'] = "::1";
		}
		$i++;
	}
	return $unbound_interfaces;
}


function unbound_acls_config() {
	global $config;

	/* Configure the ACLs */
	if (is_array($config['installedpackages']['unboundacls']['config'])) {
		$unbound_acls = $config['installedpackages']['unboundacls']['config'];
		$unboundcfg = "";
		foreach($unbound_acls as $unbound_acl){
			$unboundcfg .= "#{$unbound_acl['aclname']}\n";
			foreach($unbound_acl['row'] as $network) {
				if ($unbound_acl['aclaction'] == "allow snoop")
					$unbound_acl['aclaction'] = "allow_snoop";
				$unboundcfg .= "access-control: {$network['acl_network']}/{$network['mask']} {$unbound_acl['aclaction']}\n";
			}
		}
		return $unboundcfg;
	} else
		return;
}

function unbound_resync_config() {
	global $config, $g, $input_errors;

	$unbound_base = UNBOUND_BASE;

	if (!is_array($config['installedpackages']['unbound']['config']))
		$config['installedpackages']['unbound']['config'] = array();

	$unbound_config = &$config['installedpackages']['unbound']['config'][0];

	// Interfaces to bind to and setup acls for nics
	$unbound_bind_interfaces = "";
	$unbound_allowed_networks = "";
	$unboundnetcfg = unbound_get_network_interface_addresses();
	foreach($unboundnetcfg as $netent) {
		foreach($netent as $nettype => $entry) {
			$unbound_bind_interfaces .="interface: {$entry['ipaddr']}\n";
			if($entry['ipaddr'] != "127.0.0.1" && $entry['ipaddr'] != "::1" && $nettype != "virtual")
				$unbound_allowed_networks .= "access-control: {$entry['network']}/{$entry['subnet']} allow\n";
		}
	}
	if($unboundquerycfg = unbound_get_query_interface_addresses()) {
		$unbound_query_interfaces = "# Interfaces to query from\n";
		foreach($unboundquerycfg as $qent) {
			foreach($qent as $entry)
				$unbound_query_interfaces .= "outgoing-interface: {$entry['ipaddr']}\n";
		}
	}

	/* Configure user configured ACLs */
	$unbound_allowed_networks .= unbound_acls_config();

	if($unbound_config['dnssec_status'] == "on") {
		$module_config = "validator iterator";
		$anchor_file = "auto-trust-anchor-file: " . UNBOUND_BASE . "/etc/unbound/root-trust-anchor";
	} else
		$module_config = "iterator";

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
	if ($unbound_config['private_address'] == "on") {
		$pvt_addr = <<<EOF
# For DNS Rebinding prevention
private-address: 10.0.0.0/8
private-address: 172.16.0.0/12
private-address: 192.168.0.0/16
private-address: 169.254.0.0/16
private-address: fd00::/8
private-address: fe80::/10
# Set private domains in case authorative name server returns a RFC1918 IP address
EOF;

		// Add private-domain options
		$private_domains = unbound_add_domain_overrides(true);
	}

	//Setup optimization
	$optimization = unbound_optimization();

	$adv_config = &$config['installedpackages']['unboundadvanced']['config'][0];
	// Setup Advanced options
	$log_verbosity = (isset($adv_config['unbound_verbosity'])) ? $adv_config['unbound_verbosity'] : "1";
	$hide_id = ($adv_config['hide_id'] == "on") ? "yes" : "no";
	$hide_version = ($adv_config['hide_version'] == "on") ? "yes" : "no";
	$harden_glue = ($adv_config['harden_glue'] == "on") ? "yes" : "no";
	$harden_dnssec_stripped = ($adv_config['harden_dnssec_stripped'] == "on") ? "yes" : "no";
	$prefetch = ($adv_config['prefetch'] == "on") ? "yes" : "no";
	$prefetch_key = ($adv_config['prefetch_key'] == "on") ? "yes" : "no";
	$outgoing_num_tcp = isset($adv_config['outgoing_num_tcp']) ? $adv_config['outgoing_num_tcp'] : "10";
	$incoming_num_tcp = isset($adv_config['incoming_num_tcp']) ? $adv_config['incoming_num_tcp'] : "10";
	$edns_buffer_size = (!empty($adv_config['edns_buffer_size'])) ? $adv_config['edns_buffer_size'] : "4096";
	$num_queries_per_thread = (!empty($adv_config['num_queries_per_thread'])) ? $adv_config['num_queries_per_thread'] : "4096";
	$jostle_timeout = (!empty($adv_config['jostle_timeout'])) ? $adv_config['jostle_timeout'] : "200";
	$cache_max_ttl = (!empty($adv_config['cache_max_ttl'])) ? $adv_config['cache_max_ttl'] : "86400";
	$cache_min_ttl = (!empty($adv_config['cache_min_ttl'])) ? $adv_config['cache_min_ttl'] : "0";
	$infra_host_ttl = (!empty($adv_config['infra_host_ttl'])) ? $adv_config['infra_host_ttl'] : "900";
	$infra_lame_ttl = (!empty($adv_config['infra_lame_ttl'])) ? $adv_config['infra_lame_ttl'] : "900";
	$infra_cache_numhosts = (!empty($adv_config['infra_cache_numhosts'])) ? $adv_config['infra_cache_numhosts'] : "10000";
	$unwanted_reply_threshold = (!empty($adv_config['unwanted_reply_threshold'])) ? $adv_config['unwanted_reply_threshold'] : "0";


	$unbound_conf = <<<EOD
#########################
# Unbound configuration #
#########################

###
# Server config
###
server:
chroot: ""
username: "unbound"
directory: "{$unbound_base}/etc/unbound"
pidfile: "{$g['varrun_path']}/unbound.pid"
harden-referral-path: no
prefetch: {$prefetch}
prefetch-key: {$prefetch_key}
use-syslog: yes
port: 53
verbosity: {$log_verbosity}
do-ip4: yes
do-ip6: yes
do-udp: yes
do-tcp: yes
do-daemonize: yes
module-config: "{$module_config}"
unwanted-reply-threshold: {$unwanted_reply_threshold}
num-queries-per-thread: {$num_queries_per_thread}
jostle-timeout: {$jostle_timeout}
infra-host-ttl: {$infra_host_ttl}
prefetch: {$prefetch}
prefetch-key: {$prefetch_key}
infra-cache-numhosts: {$infra_cache_numhosts}
outgoing-num-tcp: {$outgoing_num_tcp}
incoming-num-tcp: {$incoming_num_tcp}
edns-buffer-size: {$edns_buffer_size}
statistics-interval: {$stats_interval}
extended-statistics: {$extended_stats}
statistics-cumulative: {$cumulative_stats}
cache-max-ttl: {$cache_max_ttl}
cache-min-ttl: {$cache_min_ttl}
harden-dnssec-stripped: {$harden_dnssec_stripped}
hide-identity: {$hide_id}
hide-version: {$hide_version}
harden-glue: {$harden_glue}
{$optimization['number_threads']}
{$optimization['msg_cache_slabs']}
{$optimization['rrset_cache_slabs']}
{$optimization['infra_cache_slabs']}
{$optimization['key_cache_slabs']}
{$optimization['msg_cache_size']}
{$optimization['rrset_cache_size']}
outgoing-range: 8192
{$optimization['so_rcvbuf']}
{$optimization['so_sndbuf']}


# Interface IP(s) to bind to
{$unbound_bind_interfaces}

{$unbound_query_interfaces}

{$anchor_file}

#### Access Control ####
# Local attached networks allowed to utilize service and any user added ACLs
access-control: 127.0.0.0/8 allow
access-control: ::1 allow
{$unbound_allowed_networks}
{$pvt_addr}
{$private_domains}

# Host entries
{$host_entries}
# Domain overrides
{$domain_overrides}

EOD;

	# Handle custom options
	if (!empty($adv_config['custom_options'])) {
		$custom_options = explode(";", ($adv_config['custom_options']));
		$unbound_conf .= "\n# Unbound Custom options\n";
		foreach ($custom_options as $ent)
			$unbound_conf .= $ent."\n";
	}

	// Set up forward-zones if configured
	if ($unbound_config['forwarding_mode'] == "on") {
		$dnsservers = array();
		if (isset($config['system']['dnsallowoverride'])) {
			$ns = array_unique(get_nameservers());
			foreach($ns as $nameserver) {
				if ($nameserver)
					$dnsservers[] = $nameserver;
				}
		} else {
			$ns = array_unique(get_dns_servers());
			foreach($ns as $nameserver) {
				if ($nameserver)
					$dnsservers[] = $nameserver;
			}
		}

		if (!empty($dnsservers)) {
			$unbound_conf .=<<<EOD
forward-zone:
	name: "."

EOD;
			foreach($dnsservers as $dnsserver)
				$unbound_conf .= "\tforward-addr: $dnsserver\n";
		}
	}

	$unbound_conf .= <<<EOD

###
# Remote Control Config
###
remote-control:
control-enable: yes
control-interface: 127.0.0.1
control-port: 953
server-key-file: "{$unbound_base}/etc/unbound/unbound_server.key"
server-cert-file: "{$unbound_base}/etc/unbound/unbound_server.pem"
control-key-file: "{$unbound_base}/etc/unbound/unbound_control.key"
control-cert-file: "{$unbound_base}/etc/unbound/unbound_control.pem"

EOD;


	conf_mount_rw();
	file_put_contents("{$unbound_base}/etc/unbound/unbound.conf", $unbound_conf);
	conf_mount_ro();

}

function unbound_ctl_exec($cmd) {

	mwexec(UNBOUND_BASE . "/sbin/unbound-control $cmd");

}


/* unbound_optimization - custom settings that can be configured to make Unbound perform better
 * in larger installations.
 */
function unbound_optimization() {
	global $config;

	$unbound_config = $config['installedpackages']['unboundadvanced']['config'][0];
	$optimization_settings = array();

	// Set the number of threads equal to number of CPUs.
	// Use 1 (disable threading) if for some reason this sysctl fails.
	$numprocs = intval(trim(`/sbin/sysctl kern.smp.cpus | /usr/bin/cut -d" " -f2`));
	if ($numprocs > 1) {
		$optimization['number_threads'] = "num-threads: {$numprocs}";
		$optimize_num = pow(2,floor(log($numprocs,2)));
	} else {
		$optimization['number_threads'] = "num-threads: 1";
		$optimize_num = 4;
	}

	// Slabs to help reduce lock contention.
	$optimization['msg_cache_slabs'] = "msg-cache-slabs: {$optimize_num}";
	$optimization['rrset_cache_slabs'] = "rrset-cache-slabs: {$optimize_num}";
	$optimization['infra_cache_slabs'] = "infra-cache-slabs: {$optimize_num}";
	$optimization['key_cache_slabs'] = "key-cache-slabs: {$optimize_num}";

	// Memory usage - default is 4Mb if nothing has been selected
	if(isset($unbound_config['msg_cache_size'])) {
		$rr = $unbound_config['msg_cache_size']*2;
		$optimization['msg_cache_size'] = "msg-cache-size: {$unbound_config['msg_cache_size']}m";
		$optimization['rrset_cache_size'] = "rrset-cache-size: {$rr}m";
	} else {
		$optimization['msg_cache_size'] = "msg-cache-size: 4m";
		$optimization['rrset_cache_size'] = "rrset-cache-size: 8m";
	}

	// Larger socket buffer for busy servers
	// Check that it is set to 4MB (by default the OS has it configured to 4MB)
	foreach ($config['sysctl']['item'] as $tunable) {
		if ($tunable['tunable'] == 'kern.ipc.maxsockbuf') {
			if ($tunable['value'] == 'default')
				$maxsockbuf = '4262144';
			else
				$maxsockbuf = $tunable['value'];
			$so = floor(($maxsockbuf/1024/1024)-1);
			// Check to ensure that the number is not a negative
			if ($so > 0) {
				$optimization['so_rcvbuf'] = "so-rcvbuf: {$so}m";
				$optimization['so_sndbuf'] = "so-sndbuf: {$so}m";
			} else {
				$optimization['so_rcvbuf'] = "#so-rcvbuf: 4m";
				$optimization['so_sndbuf'] = "#so-sndbuf: 4m";
			}
		}
	}

	return $optimization;
}

function unbound_validate($post, $type=null) {
	global $config, $input_errors;

	if($post['enable'] == "on" && isset($config['dnsmasq']['enable']))
		$input_errors[] = "The system dns-forwarder is still active. Disable it before enabling the Unbound service.";

	/* Validate the access lists */
	if($type == "acl") {
		$acls = $post;
		// Check to ensure values entered is an action that is in the list
		if ($acls['aclaction'] != 'refuse' && $acls['aclaction'] != 'allow' && $acls['aclaction'] != 'allow_snoop' && $acls['aclaction'] != 'deny')
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
	} else if($type == "advanced") {
		if(!is_numeric($post['cache_max_ttl']))
			$input_errors[] = "You must enter a valid number in 'Maximum TTL for RRsets and messages'.";
		if(!is_numeric($post['cache_min_ttl']))
			$input_errors[] = "You must enter a valid number in 'Minimum TTL for RRsets and messages'.";
		if(!is_numeric($post['infra_host_ttl']))
				$input_errors[] = "You must enter a valid number in 'TTL for Host cache entries'.";
		if(!is_numeric($post['infra_cache_numhosts']))
				$input_errors[] = "You must enter a valid number in 'Number of Hosts to cache'.";

	} else if($type == "basic") {
		/* Validate settings */
		if($post['active_interface'] == "")
			$input_errors[] = "You need to select at least one interface to be used by the Unbound DNS service.";
	}
}

function unbound_reconfigure() {
	global $config;

	$unbound_config = $config['installedpackages']['unbound']['config'][0];

	if ($unbound_config['enable'] != "on") {
		if (is_service_running("unbound"))
			unbound_control("termstop");
	} else {
		if (is_service_running("unbound")) {
			unbound_control("dump_cache");
			unbound_control("termstop");
		}
		unbound_resync_config();
		unbound_control("start");
		if (is_service_running("unbound"))
			unbound_control("restore_cache");
	}
}

function unbound_uninstall() {
	global $g, $config;

	conf_mount_rw();

	unbound_control("termstop");
	// Remove pkg config directory and startup file
	mwexec("rm -rf " . UNBOUND_BASE . "/etc/unbound");
	@unlink("/usr/local/etc/rc.d/unbound.sh");
	@unlink("{$g['varlog_path']}/unbound.log");
	@unlink("/var/tmp/unbound_cache");

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

/*
 * Setup /etc/hosts entries by overriding with local-data
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
	// IPv6 entries
	if(function_exists("is_ipaddrv6")) {
		$unbound_entries .= "local-data-ptr: \"::1 localhost\"\n";
		$unbound_entries .= "local-data: \"localhost AAAA ::1\"\n";
		$unbound_entries .= "local-data: \"localhost.{$syscfg['domain']} AAAA ::1\"\n";
	}

	$added_item_v4 = array();
	$added_item_v6 = array();
	if ($config['interfaces']['lan']) {
		$current_host = $syscfg['hostname'].".".$syscfg['domain'];
		$cfgip = get_interface_ip("lan");
		if (is_ipaddr($cfgip)) {
			$unbound_entries .= "local-data-ptr: \"{$cfgip} {$current_host}\"\n";
			$unbound_entries .= "local-data: \"{$current_host} A {$cfgip}\"\n";
			$unbound_entries .= "local-data: \"{$syscfg['hostname']} A {$cfgip}\"\n";
			$added_item_v4[$current_host] = true;
		}
		$cfgip6 = get_interface_ipv6("lan");
		if (is_ipaddrv6($cfgip6)) {
			$unbound_entries .= "local-data-ptr: \"{$cfgip6} {$current_host}\"\n";
			$unbound_entries .= "local-data: \"{$current_host} AAAA {$cfgip6}\"\n";
			$unbound_entries .= "local-data: \"{$syscfg['hostname']} AAAA {$cfgip6}\"\n";
			$added_item_v6[$current_host] = true;
		}
	} else {
		$sysiflist = get_configured_interface_list();
		foreach ($sysiflist as $sysif) {
			if (!interface_has_gateway($sysif)) {
				$current_host = $syscfg['hostname'].".".$syscfg['domain'];
				$cfgip = get_interface_ip($sysif);
				if (is_ipaddr($cfgip)) {
					$unbound_entries .= "local-data-ptr: \"{$cfgip} {$current_host}\"\n";
					$unbound_entries .= "local-data: \"{$current_host} A {$cfgip}\"\n";
					$unbound_entries .= "local-data: \"{$syscfg['hostname']} A {$cfgip}\"\n";
					$added_item_v4[$current_host] = true;
				}
				$cfgip6 = get_interface_ipv6($sysif);
				if (is_ipaddr($cfgip6)) {
					$unbound_entries .= "local-data-ptr: \"{$cfgip6} {$current_host}\"\n";
					$unbound_entries .= "local-data: \"{$current_host} AAAA {$cfgip6}\"\n";
					$unbound_entries .= "local-data: \"{$syscfg['hostname']} AAAA {$cfgip6}\"\n";
					$added_item_v6[$current_host] = true;
				}
				if (is_ipaddr($cfgip) || is_ipaddr($cfgip6))
					break;
			}
		}
	}

	// DNSMasq entries static host entries
	if (isset($dnsmasqcfg['hosts'])) {
		$hosts = $dnsmasqcfg['hosts'];
		$host_entries = "# DNSMasq Host overrides\n";
		$added_item = array();
		foreach ($hosts as $host) {
			$current_host = ($host['host'] != "") ? $host['host'].".".$host['domain'] : $host['domain'];
			if (function_exists("is_ipaddrv6") && is_ipaddrv6($host['ip'])) {
				if (!$added_item_v6[$current_host]) {
					$host_entries .= "local-data-ptr: \"{$host['ip']} {$current_host}\"\n";
					$host_entries .= "local-data: \"{$current_host} IN AAAA {$host['ip']}\"\n";
					$added_item_v6[$current_host] = true;
				}
				if ($host['aliases']['item'] && is_array($host['aliases']['item']))
					foreach ($host['aliases']['item'] as $alias) {
						$current_alias = ($alias['host'] != "") ? $alias['host'].".".$alias['domain'] : $alias['domain'];
						if (!$added_item_v6[$current_alias]) {
							$host_entries .= "local-data: \"{$current_alias} IN AAAA {$host['ip']}\"\n";
							$added_item_v6[$current_alias] = true;
							if ((!$added_item[$current_alias]) && (!empty($alias['description'])) && ($unboundcfg['txtsupport'] == 'on')) {
								$host_entries .= "local-data: '{$current_alias} TXT \"".addslashes($alias['description'])."\"'\n";
								$added_item[$current_alias] = true;
							}
						}
					}
			} else {
				if (!$added_item_v4[$current_host]) {
					$host_entries .= "local-data-ptr: \"{$host['ip']} {$current_host}\"\n";
					$host_entries .= "local-data: \"{$current_host} IN A {$host['ip']}\"\n";
					$added_item_v4[$current_host] = true;
				}
				if ($host['aliases']['item'] && is_array($host['aliases']['item']))
					foreach ($host['aliases']['item'] as $alias) {
						$current_alias = ($alias['host'] != "") ? $alias['host'].".".$alias['domain'] : $alias['domain'];
						if (!$added_item_v4[$current_alias]) {
							$host_entries .= "local-data: \"{$current_alias} IN A {$host['ip']}\"\n";
							$added_item_v4[$current_alias] = true;
							if ((!$added_item[$current_alias]) && (!empty($alias['description'])) && ($unboundcfg['txtsupport'] == 'on')) {
								$host_entries .= "local-data: '{$current_alias} TXT \"".addslashes($alias['description'])."\"'\n";
								$added_item[$current_alias] = true;
							}
						}
					}
			}
			if ((!$added_item[$current_host]) && (!empty($host['descr'])) && ($unboundcfg['txtsupport'] == 'on')) {
				$host_entries .= "local-data: '{$current_host} TXT \"".addslashes($host['descr'])."\"'\n";
				$added_item[$current_host] = true;
			}
		}
		$unbound_entries .= $host_entries;
	}
	
	// Static DHCP entries
	$host_entries = "# DHCP Reservations\n";
	if (isset($unboundcfg['regdhcpstatic']) && is_array($config['dhcpd'])) {
		foreach ($config['dhcpd'] as $dhcpif => $dhcpifconf)
			if(is_array($dhcpifconf['staticmap']) && isset($dhcpifconf['enable']))
				foreach ($dhcpifconf['staticmap'] as $host)
					if ($host['ipaddr'] && $host['hostname']) {
						$current_host = $host['hostname'].".".$syscfg['domain'];
						if (!$added_item_v4[$current_host]) {
							$host_entries .= "local-data-ptr: \"{$host['ipaddr']} {$current_host}\"\n";
							$host_entries .= "local-data: \"{$current_host} IN A {$host['ipaddr']}\"\n";
							$added_item_v4[$current_host] = true;
							if ((!$added_item[$current_host]) && (!empty($host['descr'])) && ($unboundcfg['txtsupport'] == 'on')) {
								$host_entries .= "local-data: '{$host['hostname']}.{$syscfg['domain']} TXT \"".addslashes($host['descr'])."\"'\n";
								$added_item[$current_host] = true;
							}
						}
					}
		$unbound_entries .= $host_entries;
	}
	
	// Static DHCPv6 entries
	$host_entries = "# DHCPv6 reservations\n";
	if (isset($unboundcfg['regdhcpstatic']) && is_array($config['dhcpdv6'])) {
		foreach ($config['dhcpdv6'] as $dhcpif => $dhcpifconf)
			if(is_array($dhcpifconf['staticmap']) && isset($dhcpifconf['enable']))
				foreach ($dhcpifconf['staticmap'] as $host)
					if ($host['ipaddrv6'] && $host['hostname']) {
						$current_host = $host['hostname'].".".$syscfg['domain'];
						if (!$added_item_v6[$current_host]) {
							$host_entries .= "local-data-ptr: \"{$host['ipaddrv6']} {$current_host}\"\n";
							$host_entries .= "local-data: \"{$current_host} IN AAAA {$host['ipaddrv6']}\"\n";
							$added_item_v6[$current_host] = true;
							if ((!$added_item[$current_host]) && (!empty($host['descr'])) && ($unboundcfg['txtsupport'] == 'on')) {
								$host_entries .= "local-data: '{$host['hostname']}.{$syscfg['domain']} TXT \"".addslashes($host['descr'])."\"'\n";
								$added_item[$current_host] = true;
							}
						}
					}
		$unbound_entries .= $host_entries;
	}

	// Handle DHCPLeases added host entries
	$dhcplcfg = read_hosts();
	$host_entries = "# /etc/hosts entries\n";
	if(is_array($dhcplcfg)) {
		foreach($dhcplcfg as $key=>$host) {
			$current_host = $host['fqdn'];
			if (function_exists("is_ipaddrv6") && is_ipaddrv6($host['ipaddr'])) {
				if (!$added_item_v6[$current_host]) {
					$host_entries .= "local-data-ptr: \"{$host['ipaddr']} {$host['fqdn']}\"\n";
					$host_entries .= "local-data: \"{$host['fqdn']} IN AAAA {$host['ipaddr']}\"\n";
					$added_item_v6[$current_host] = true;
					if ((!empty($host['name'])) && (!$added_item_v6[$host['name']])) {
						$host_entries .= "local-data-ptr: \"{$host['ipaddr']} {$host['name']}\"\n";
						$host_entries .= "local-data: \"{$host['name']} IN AAAA {$host['ipaddr']}\"\n";
						$added_item_v6[$host['name']] = true;
					}
				}
			} else {
				if (!$added_item_v4[$current_host]) {
					$host_entries .= "local-data-ptr: \"{$host['ipaddr']} {$host['fqdn']}\"\n";
					$host_entries .= "local-data: \"{$host['fqdn']} IN A {$host['ipaddr']}\"\n";
					$added_item_v4[$current_host] = true;
					if ((!empty($host['name'])) && (!$added_item_v4[$host['name']])) {
						$host_entries .= "local-data-ptr: \"{$host['ipaddr']} {$host['name']}\"\n";
						$host_entries .= "local-data: \"{$host['name']} IN A {$host['ipaddr']}\"\n";
						$added_item_v4[$host['name']] = true;
					}
				}
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
			if (!isset($result[$domain_key]))
				$result[$domain_key] = array();
			$result[$domain_key][] = $domain['ip'];
		}

		$domain_entries = "";
		foreach($result as $domain=>$ips) {
			if ($pvt == true) {
				if (strpos($domain, "in-addr.arpa") !== false)
					$domain_entries .= "local-zone: \"$domain\" transparent\n";
				else
					$domain_entries .= "private-domain: \"$domain\"\n";
				if (isset($config['installedpackages']['unbound']['config'][0]['dnssec_status']))
					$domain_entries .= "domain-insecure: \"$domain\"\n";
			} else {
				$domain_entries .= "stub-zone:\n";
				$domain_entries .= "\tname: \"$domain\"\n";
				foreach($ips as $ip)
					$domain_entries .= "\tstub-addr: $ip\n";
				$domain_entries .= "\tstub-prime: no\n";
			}
		}
		return $domain_entries;
	}
}

function unbound_acl_id_used($id) {
	global $config;

	if (is_array($config['installedpackages']['unboundacls']['config']))
		foreach ($config['installedpackages']['unboundacls']['config'] as & $acls)
			if ($id == $acls['aclid'])
				return true;

	return false;
}

function unbound_get_next_id() {

	$aclid = 0;
	while(unbound_acl_id_used($aclid))
		$aclid++;

	return $aclid;
}

?>
