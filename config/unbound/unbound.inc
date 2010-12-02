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

if(!function_exists("get_dns_servers"))
	require_once("pfsense-utils.inc");

if(!function_exists("get_nameservers"))
	require_once("system.inc");
	

function unbound_initial_setup() {
	global $config, $g;

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
	@chown("{$g['varlog_path']}/unbound.log", "unbound");
	// We do not need the sample conf or the default rc.d startup file
	@unlink_if_exists("/usr/local/etc/unbound/unbound.conf.sample");
	@unlink_if_exists("/usr/local/etc/rc.d/unbound");
		
	// Setup rc file for startup and shutdown.
	unbound_rc_setup();

	// Disable DNSMasq and enable UNBound
	$unbound_config['unbound_status'] = "on";
	
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
						unbound_ctl_exec("reload");
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
				foreach($config['virtualip']['vip'] as $vip) {
					if (($vip['interface'] == $unboundif) && ($vip['mode'] == "carp")) {
						$virtual_ip = find_interface_ip(link_ip_to_carp_interface($vip['subnet']));
						$unbound_interfaces[] = $virtual_ip;
					}
				}
			}
		}
	}
	
	return $unbound_interfaces;
	
}

function unbound_resync_config() {
	global $config, $g;
	
	if (!array($config['installedpackages']['unbound']['config']))
		$config['installedpackages']['unbound']['config'] = array();

	$unbound_config = &$config['installedpackages']['unbound']['config'][0];
			
	$interfaces = unbound_get_network_interface_addresses(true, true);
	foreach($interfaces as $allowed_network) {
		$unbound_allowed_networks .= "access-control: $allowed_network allow\n";
	}
	
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
	
	$unbound_conf = <<<EOD
#########################
# Unbound configuration #
#########################

###
# Server config
###
server:
verbosity: 1
port: 53
do-ip4: yes
do-ip6: no
do-udp: yes
do-tcp: yes
do-daemonize: yes
statistics-interval: 3600
extended-statistics: yes
statistics-cumulative: no
# Interface IP(s) to bind to
{$unbound_bind_interfaces}
chroot: ""
username: "unbound"
directory: "/usr/local/etc/unbound"
pidfile: "{$g['varrun_path']}/unbound.pid"
root-hints: "root.hints"
harden-dnssec-stripped: {$unbound_config['harden-dnssec-stripped']}
harden-referral-path: no
private-address: 10.0.0.0/8
private-address: 172.16.0.0/12
private-address: 192.168.0.0/16
prefetch: yes
prefetch-key: yes
use-syslog: yes
module-config: "{$module_config}"
unwanted-reply-threshold: 10000000
{$anchor_file}
# Networks allowed to utilize service
access-control: 127.0.0.0/8 allow
{$unbound_allowed_networks}
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

function fetch_root_hints() {

	$destination_file = "/usr/local/etc/unbound/root.hints";
	if (filesize($destination_file) == 0 ) {
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
	
		return ($http_code == 200) ? true : $http_code;
	} else {
		return false;
	}
}

function unbound_reconfigure() {
	global $config,$g;
	
	$unbound_config = $config['installedpackages']['unbound']['config'][0];
	
	if ($unbound_config['unbound_status'] != "on") {
		if(is_service_running("unbound")) {
			unbound_control("termstop");
		}
	} else {
		if(isset($config['dnsmasq']['enable'])) {
			unset($config['dnsmasq']['enable']);
			sigkillbypid("{$g['varrun_path']}/dnsmasq.pid", "TERM");
		}
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

/* Setup /etc/hosts entries by overriding with local-data
 */
function unbound_add_host_entries() {
	global $config;

	if (isset($config['dnsmasq']['hosts'])) {
		$hosts = $config['dnsmasq']['hosts'];
		$host_entries = "";
		$added_item = array();
		foreach ($hosts as $host) {
			$current_host = $host['host'];
			if(!$added_item[$current_host]) {
				$host_entries .= "local-data: '{$host['host']}.{$host['domain']}. IN A {$host['ip']}'\n";
				if (!empty($host['descr']))
					$host_entries .= "local-data: '{$host['host']}.{$host['domain']}. TXT \"{$host['descr']}\"'\n";
				// Do not add duplicate entries
				$added_item[$current_host] = true;
			}
		}
		return $host_entries;	
	}
}

/* Setup any domain overrides that have been configured with local-zone
 */
function unbound_add_domain_overrides() {
	global $config;

	if (isset($config['dnsmasq']['domainoverrides'])) {
		$domains = $config['dnsmasq']['domainoverrides'];

		// Domain overrides that have multiple entries need multiple forward-addr: added
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
			$domain_entries .= "forward-zone:\n";
			$domain_entries .= "\tname: \"$domain\"\n";
			foreach($ips as $ip) {
				$domain_entries .= "\tforward-addr: $ip\n";
			}
		}
		return $domain_entries;
	}
}

?>
