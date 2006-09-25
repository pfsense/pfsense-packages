<?php

function sync_package_snort() {
	global $config, $g;
	exec("mkdir -p /usr/local/etc/snort");
	exec("mkdir -p /var/log/snort");
	$first = 0;
	/* if list */
	$iflist = array("lan" => "LAN");
	for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++)
		$iflist['opt' . $i] = "opt{$i}";
	$whitelist = fopen("/var/db/whitelist","w");
	if(!$whitelist) {
		log_error("Cannot open whitelist for /var/db/writing.");
		exit;
	}
	foreach($iflist as $if) {
		/* XXX: write out if subnet */
	}
	fclose($whitelist);
	foreach($_POST['interface_array'] as $iface) {
		$if = convert_friendly_interface_to_real_interface_name($iface);
		if($if) {
			$ifaces_final .= " -i " . $if;
			$first = 1;
		}
	}
	$start  = "snort -c /usr/local/etc/snort/snort.conf -l /var/log/snort " . $ifaces_final . " -D";
	$start .= ";snort2c -s -w /var/db/whitelist -a /var/log/snort/alert";
	write_rcfile(array(
			"file" => "snort.sh",
			"start" => $start,
			"stop" => "/usr/bin/killall snort; killall snort2c"
		)
	);
	/* write out snort.conf */
	$snort_conf = generate_snort_conf();
	$conf = fopen("/usr/local/etc/snort/snort.conf","w");
	if(!$conf) {
		log_error("Could not open /usr/local/etc/snort/snort.conf for writing.");
		exit;
	}
	fwrite($conf, $snort_conf);
	fclose($conf);
	start_service("snort");
}

function generate_snort_conf() {
	global $config, $g;

	/* XXX: set SSH port from config variable */
	$ssh_port = "22";
	/* XXX: generate home net */
	$home_net = "";

	/* XXX: generate rule section */
	$selected_rules_sections = "";

	$snort_conf = <<<EOD

var AIM_SERVERS [64.12.24.0/24,64.12.25.0/24,64.12.26.14/24,64.12.28.0/24,64.12.29.0/24,64.12.161.0/24,64.12.163.0/24,205.188.5.0/24,205.188.9.0/24]
var HTTP_PORTS 80
var SHELLCODE_PORTS !$HTTP_PORTS
var ORACLE_PORTS 1521
var HOME_NET {$home_net}
var TELNET_SERVERS $HOME_NET
var SQL_SERVERS $HOME_NET
var HTTP_SERVERS $HOME_NET
var SMTP_SERVERS $HOME_NET
var DNS_SERVERS $HOME_NET
var RULE_PATH .
var EXTERNAL_NET !$HOME_NET
var SSH_PORTS {$ssh_port}

#Output plugins
output database: alert
output alert_syslog: LOG_AUTH LOG_ALERT LOG_CONS LOG_NDELAY LOG_PERROR LOG_PID

#Flow and stream
preprocessor flow: stats_interval 0 hash 2
preprocessor frag2
preprocessor stream4: disable_evasion_alerts,detect_scans

preprocessor stream4_reassemble: both, ports all

#XLink2State mini proc
preprocessor xlink2state: ports { 25 691 }

#HTTP Inspect
preprocessor http_inspect: global iis_unicode_map unicode.map 1252

preprocessor http_inspect_server: server default \
                        ports  { 80 8080 3128 }  \
                        no_alerts \
                        non_strict \
                        non_rfc_char  { 0x00 }  \
                        flow_depth 0  \
                        apache_whitespace yes \
                        directory no \
                        iis_backslash no \
                        u_encode yes \
                        ascii no \
                        chunk_length 500000 \
                        bare_byte yes \
                        double_decode yes \
                        iis_unicode yes \
                        iis_delimiter yes \
                        multi_slash no

#Other preprocs
preprocessor rpc_decode: 111 32771
preprocessor bo
preprocessor telnet_decode

#Flow Portscan
preprocessor flow-portscan: \
        talker-sliding-scale-factor 0.50 \
        talker-fixed-threshold 30 \
        talker-sliding-threshold 30 \
        talker-sliding-window 20 \
        talker-fixed-window 30 \
        scoreboard-rows-talker 30000 \
        server-watchnet $HOME_NET \
        server-ignore-limit 200 \
        server-rows 65535 \
        server-learning-time 14400 \
        server-scanner-limit 4 \
        scanner-sliding-window 20 \
        scanner-sliding-scale-factor 0.50 \
        scanner-fixed-threshold 15 \
        scanner-sliding-threshold 40 \
        scanner-fixed-window 15 \
        scoreboard-rows-scanner 30000 \
        alert-mode once \
        output-mode msg \
        tcp-penalties on


#Required files
include classification.config
include reference.config

#Rulesets, all optional

{$selected_rules_sections}

# XXX: axe below, use $selected_rules_sections

#General
include $RULE_PATH/bleeding.rules
include $RULE_PATH/ftp.rules
include $RULE_PATH/telnet.rules
include $RULE_PATH/dns.rules
include $RULE_PATH/tftp.rules
include $RULE_PATH/x11.rules
include $RULE_PATH/misc.rules
include $RULE_PATH/nntp.rules
include $RULE_PATH/other-ids.rules
# include $RULE_PATH/shellcode.rules
include $RULE_PATH/community-ftp.rules
include $RULE_PATH/community-misc.rules

#Mostly Spyware
include $RULE_PATH/bleeding-malware.rules

#Network issues
include $RULE_PATH/bad-traffic.rules
include $RULE_PATH/snmp.rules

#Exploits and direct attacks
include $RULE_PATH/exploit.rules
include $RULE_PATH/bleeding-exploit.rules
include $RULE_PATH/community-exploit.rules

#Scans and recon
include $RULE_PATH/scan.rules
include $RULE_PATH/bleeding-scan.rules

#Unusual stuff
include $RULE_PATH/finger.rules

#R-services, etc
include $RULE_PATH/rpc.rules
include $RULE_PATH/rservices.rules

#DOS
include $RULE_PATH/dos.rules
include $RULE_PATH/ddos.rules
include $RULE_PATH/bleeding-dos.rules

#Web issues
include $RULE_PATH/web-cgi.rules
include $RULE_PATH/web-coldfusion.rules
include $RULE_PATH/web-iis.rules
include $RULE_PATH/web-frontpage.rules
include $RULE_PATH/web-misc.rules
include $RULE_PATH/web-client.rules
include $RULE_PATH/web-php.rules
include $RULE_PATH/web-attacks.rules
include $RULE_PATH/bleeding-web.rules
include $RULE_PATH/community-web-cgi.rules
include $RULE_PATH/community-web-client.rules
include $RULE_PATH/community-web-dos.rules
include $RULE_PATH/community-web-misc.rules

#SQL and DB sigs
include $RULE_PATH/sql.rules
include $RULE_PATH/oracle.rules
include $RULE_PATH/mysql.rules
include $RULE_PATH/community-sql-injection.rules

#Informational stuff
#include $RULE_PATH/icmp.rules
include $RULE_PATH/info.rules
# include $RULE_PATH/icmp-info.rules

#Windows stuff
include $RULE_PATH/netbios.rules

#Compromise responses
include $RULE_PATH/attack-responses.rules
include $RULE_PATH/bleeding-attack_response.rules

#Mail sigs
include $RULE_PATH/smtp.rules
include $RULE_PATH/imap.rules
include $RULE_PATH/pop2.rules
include $RULE_PATH/pop3.rules
include $RULE_PATH/community-mail-client.rules

#Trojans, Viruses, and spyware
include $RULE_PATH/backdoor.rules
include $RULE_PATH/virus.rules
include $RULE_PATH/bleeding-virus.rules
include $RULE_PATH/community-virus.rules

#Policy Sigs
include $RULE_PATH/policy.rules
include $RULE_PATH/porn.rules
include $RULE_PATH/chat.rules
include $RULE_PATH/p2p.rules
include $RULE_PATH/multimedia.rules
include $RULE_PATH/bleeding-policy.rules
include $RULE_PATH/bleeding-p2p.rules
include $RULE_PATH/bleeding-inappropriate.rules
include $RULE_PATH/community-game.rules
include $RULE_PATH/community-inappropriate.rules

#Experimental
include $RULE_PATH/experimental.rules

EOD;

	return $snort_conf;
}

?>