<?php
/* $Id$ */
/*
	snort.inc
	Copyright (C) 2006 Scott Ullrich
	part of pfSense
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
require_once("pfsense-utils.inc");

/* Allow additional execution time 0 = no limit. */
ini_set('max_execution_time', '9999');
ini_set('max_input_time', '9999');

/* define oinkid */
if($config['installedpackages']['snort'])
	$oinkid = $config['installedpackages']['snort']['config'][0]['oinkmastercode'];

function sync_package_snort_reinstall() 
{
	global $config;
	if(!$config['installedpackages']['snort'])
		return;
    
	/* create snort configuration file */
	create_snort_conf();
  
	/* start snort service */
	start_service("snort");
}

function sync_package_snort() 
{
	global $config, $g;

	mwexec("mkdir -p /var/log/snort/");

	if(!file_exists("/var/log/snort/alert"))
		touch("/var/log/snort/alert");

	/* snort -> advanced features */
	$bpfbufsize = $config['installedpackages']['snortadvanced']['config'][0]['bpfbufsize'];
	$bpfmaxbufsize = $config['installedpackages']['snortadvanced']['config'][0]['bpfmaxbufsize'];
	$bpfmaxinsns = $config['installedpackages']['snortadvanced']['config'][0]['bpfmaxinsns'];

	/* set the snort performance model */
	if($config['installedpackages']['snort']['config'][0]['performance'])
		$snort_performance = $config['installedpackages']['snort']['config'][0]['performance'];
	else
		$snort_performance = "ac-bnfa";

	conf_mount_rw();
	/* create a few directories and ensure the sample files are in place */
	exec("/bin/mkdir -p /usr/local/etc/snort");
	exec("/bin/mkdir -p /var/log/snort");
	exec("/bin/cp /usr/local/etc/snort/unicode.map-sample /usr/local/etc/snort/unicode.map");
	exec("/bin/cp /usr/local/etc/snort/classification.config-sample /usr/local/etc/snort/classification.config");
	exec("/bin/cp /usr/local/etc/snort/gen-msg.map-sample /usr/local/etc/snort/gen-msg.map");
	exec("/bin/cp /usr/local/etc/snort/generators-sample /usr/local/etc/snort/generators");
	exec("/bin/cp /usr/local/etc/snort/reference.config-sample /usr/local/etc/snort/reference.config");
	exec("/bin/cp /usr/local/etc/snort/sid-msg.map-sample /usr/local/etc/snort/sid-msg.map");
	exec("/bin/cp /usr/local/etc/snort/sid-sample /usr/local/etc/snort/sid");
	exec("/bin/cp /usr/local/etc/snort/threshold.conf-sample /usr/local/etc/snort/threshold.conf");
	exec("/bin/cp /usr/local/etc/snort/unicode.map-sample /usr/local/etc/snort/unicode.map");
	exec("/bin/rm -f /usr/local/etc/rc.d/snort");

	$first = 0;
	$snortInterfaces = array(); /* -gtm  */
	
	$if_list = $config['installedpackages']['snort']['config'][0]['iface_array'];
	$if_array = split(',', $if_list);
	//print_r($if_array);
	if($if_array) {
		foreach($if_array as $iface) {
			$if = convert_friendly_interface_to_real_interface_name($iface);
			
			if($config['interfaces'][$iface]['ipaddr'] == "pppoe") {
				$if = "ng0";
			}
	      
			/* build a list of user specified interfaces -gtm */      
			if($if){      
			  array_push($snortInterfaces, $if);
			  $first = 1;
			}
		}	
	    
		if (count($snortInterfaces) < 1) {
			//log_error("Snort will not start.  You must select an interface for it to listen on.");
			echo "Snort will not start.  You must select an interface for it to listen on.";
			return;
		}
	}
	//print_r($snortInterfaces);
	
	/* create log directory */
	$start  = "/bin/mkdir -p /var/log/snort";

	/* snort advanced features - bpf tuning */
	if($bpfbufsize)
		$start .= ";sysctl net.bpf.bufsize={$bpfbufsize}";
	if($bpfmaxbufsize)
		$start .= ";sysctl net.bpf.maxbufsize={$bpfmaxbufsize}";
	if($bpfmaxinsns)
		$start .= ";sysctl net.bpf.maxinsns={$bpfmaxinsns}";

	/* go ahead and issue bpf changes */
	if($bpfbufsize)
		mwexec_bg("sysctl net.bpf.bufsize={$bpfbufsize}");
	if($bpfmaxbufsize)
		mwexec_bg("sysctl net.bpf.maxbufsize={$bpfmaxbufsize}");
	if($bpfmaxinsns)
		mwexec_bg("sysctl net.bpf.maxinsns={$bpfmaxinsns}");

	/* always stop snort2c before starting snort -gtm */
  $start .= ";/usr/bin/killall snort2c";

  /* start a snort process for each interface -gtm */
  /* Note the sleep delay.  Seems to help getting mult interfaces to start -gtm */
  foreach($snortInterfaces as $snortIf)
  {
  	$start .= ";sleep 8;snort -c /usr/local/etc/snort/snort.conf -l /var/log/snort -D -i {$snortIf} -A fast &";
  }    
    
	/* if block offenders is checked, start snort2c */
	if($_POST['blockoffenders'])
		$start .= ";sleep 8;snort2c -w /var/db/whitelist -a /var/log/snort/alert";
    
	$sample_before = "\nBEFORE_MEM=`top | grep Free | grep Wired | awk '{print \$10}'`\n";
	$sample_after = "\nAFTER_MEM=`top | grep Free | grep Wired | awk '{print \$10}'`\n";
	$sleep_before_final = "\necho \"Sleeping before final memory sampling...\"\nsleep 17";
	$total_free_after = "\nTOTAL_USAGE=`top | grep snort | grep -v grep | awk '{ print \$6 }'`\n";
	$echo_usage = "\necho \"Ram free BEFORE starting Snort: \${BEFORE_MEM} -- Ram free AFTER starting Snort: \${AFTER_MEM}\" -- Mode {$snort_performance} -- Snort memory usage: \$TOTAL_USAGE | logger -p daemon.info -i -t SnortStartup\n";

	/* write out rc.d start/stop file */
	write_rcfile(array(
			"file" => "snort.sh",
			"start" => "{$sample_before}{$start}{$sleep_before_final}{$sample_after}{$echo_usage}",
			"stop" => "/usr/bin/killall snort; killall snort2c"
		)
	);

	/* create snort configuration file */
	create_snort_conf();

	/* start snort service */
	conf_mount_ro();
	start_service("snort");
}

function create_snort_conf() {
	global $config, $g;
	/* write out snort.conf */
	$snort_conf_text = generate_snort_conf();
	conf_mount_rw();
	$conf = fopen("/usr/local/etc/snort/snort.conf", "w");
	if(!$conf) {
		log_error("Could not open /usr/local/etc/snort/snort.conf for writing.");
		exit;
	}
	fwrite($conf, $snort_conf_text);
	fclose($conf);
	conf_mount_ro();
}

function snort_deinstall() {
	$text_ww   = "*/60\t* \t 1\t *\t *\t root\t /usr/bin/nice -n20 /usr/local/pkg/snort_check_for_rule_updates.php";
	$filenamea = "/etc/crontab";
	/* remove auto rules update helper */
	remove_text_from_file($filenamea, $text_ww);
	/* remove custom sysctl */
	remove_text_from_file("/etc/sysctl.conf", "sysctl net.bpf.bufsize=20480");
	/* decrease bpf buffers back to 4096, from 20480 */
	exec("/sbin/sysctl net.bpf.bufsize=4096");
	exec("/usr/bin/killall snort");
	sleep(5);
	exec("/usr/bin/killall -9 snort");
	exec("rm -f /usr/local/etc/rc.d/snort*");
	exec("rm -rf /usr/local/etc/snort*");
	exec("cd /var/db/pkg && pkg_delete `ls | grep snort`");
}

function generate_snort_conf() {
	global $config, $g;
	conf_mount_rw();
	/* obtain external interface */
	/* XXX: make multi wan friendly */
	$snort_ext_int = $config['installedpackages']['snort']['config'][0]['iface_array'][0];

	$snort_config_pass_thru = $config['installedpackages']['snortadvanced']['config'][0]['configpassthru'];

	/* add auto update scripts to /etc/crontab */
	$text_ww   = "*/60\t* \t 1\t *\t *\t root\t /usr/bin/nice -n20 /usr/local/pkg/snort_check_for_rule_updates.php";
	$filenamea = "/etc/crontab";
	remove_text_from_file($filenamea, $text_ww);
	add_text_to_file($filenamea, $text_ww);
	exec("killall -HUP cron");

	/* should we install a automatic update crontab entry? */
	$automaticrulesupdate = $config['installedpackages']['snort']['config'][0]['automaticrulesupdate'];

	/* if user is on pppoe, we really want to use ng0 interface */
	if($config['interfaces'][$snort_ext_int]['ipaddr'] == "pppoe")
		$snort_ext_int = "ng0";

	/* set the snort performance model */
	if($config['installedpackages']['snort']['config'][0]['performance'])
		$snort_performance = $config['installedpackages']['snort']['config'][0]['performance'];
	else
		$snort_performance = "ac-bnfa";

	/* open snort2c's whitelist for writing */
	$whitelist = fopen("/var/db/whitelist", "w");
	if(!$whitelist) {
		log_error("Could not open /var/db/whitelist for writing.");
		return;
	}

	/* build an interface array list */
	$int_array = array('lan');
	for ($j = 1; isset ($config['interfaces']['opt' . $j]); $j++)
		if(isset($config['interfaces']['opt' . $j]['enable']))
			if(!$config['interfaces']['opt' . $j]['gateway'])
				$int_array[] = "opt{$j}";

	/* if user has defined a custom ssh port, use it */
	if($config['system']['ssh']['port'])
		$ssh_port = $config['system']['ssh']['port'];
	else
		$ssh_port = "22";

	/*  iterate through interface list and write out whitelist items
	 *  and also compile a home_net list for snort.
	 */
	foreach($int_array as $int) {
		/* calculate interface subnet information */
		$ifcfg = &$config['interfaces'][$int];
		$subnet = gen_subnet($ifcfg['ipaddr'], $ifcfg['subnet']);
		$subnetmask = gen_subnet_mask($ifcfg['subnet']);
		if($subnet == "pppoe" or $subnet == "dhcp") {
			$subnet = find_interface_ip("ng0");
			if($subnet)
				$home_net .= "{$subnet} ";
		} else {
			if ($subnet)
				if($ifcfg['subnet']) 
					$home_net .= "{$subnet}/{$ifcfg['subnet']} ";
		}
	}

	/* add all WAN ips to the whitelist */
	$wan_if = get_real_wan_interface();
	$ip = find_interface_ip($wan_if);
	if($ip) 
		$home_net .= "{$ip} ";
	
	/* Add Gateway on WAN interface to whitelist (For RRD graphs) */
	$int = convert_friendly_interface_to_real_interface_name("WAN");
	$gw = get_interface_gateway($int);
	if($gw) 
		$home_net .= "{$gw} ";
	
	/* Add DNS server for WAN interface to whitelist */
	$dns_servers = get_dns_servers();
	foreach($dns_servers as $dns) {
		if($dns) 
			$home_net .= "{$dns} ";		
	}
		
	/* Add loopback to whitelist (ftphelper) */
	$home_net .= "127.0.0.1 "; 	
	
	/* iterate all vips and add to whitelist */
	if($config['virtualip'])
		foreach($config['virtualip']['vip'] as $vip)
			if($vip['subnet']) 
				$home_net .= $vip['subnet'] . " ";

	if($config['installedpackages']['snortwhitelist'])
		foreach($config['installedpackages']['snortwhitelist']['config'] as $snort)
			if($snort['ip']) 
				$home_net .= $snort['ip'] . " ";

	/* write out whitelist, convert spaces to carriage returns */
	$whitelist_home_net = str_replace("  ", " ", $home_net);
	$whitelist_home_net = str_replace(" ", "\n", $home_net);

	/* make $home_net presentable to snort */
	$home_net = trim($home_net);
	$home_net = str_replace(" ", ",", $home_net);
	$home_net = "[{$home_net}]";

	/* foreach through whitelist, writing out to file */
	$whitelist_split = split("\n", $whitelist_home_net);
	foreach($whitelist_split as $wl)
		if(trim($wl))
			fwrite($whitelist, trim($wl) . "\n");

	/* should we whitelist vpns? */
	$whitelistvpns = $config['installedpackages']['snort']['config'][0]['whitelistvpns'];

	/* grab a list of vpns and whitelist if user desires */
	if($whitelistvpns) {
		$vpns_list = get_vpns_list();
		$whitelist_vpns = split(" ", $vpns_list);
		foreach($whitelist_split as $wl)
			if(trim($wl))
				fwrite($whitelist, trim($wl) . "\n");
	}

	/* close file */
	fclose($whitelist);

	/* generate rule sections to load */
	$enabled_rulesets = $config['installedpackages']['snort']['rulesets'];
	if($enabled_rulesets) {
		$selected_rules_sections = "";
		$enabled_rulesets_array = split("\|\|", $enabled_rulesets);
		foreach($enabled_rulesets_array as $enabled_item)
			$selected_rules_sections  .= "include \$RULE_PATH/{$enabled_item}\n";
	}

	conf_mount_ro();

	/* build snort configuration file */
	$snort_conf_text = <<<EOD

# snort configuration file
# generated by the pfSense
# package manager system
# see /usr/local/pkg/snort.inc
# for more information

var HOME_NET {$home_net}
var EXTERNAL_NET !\$HOME_NET

var DNS_SERVERS \$HOME_NET
var SMTP_SERVERS \$HOME_NET
var HTTP_SERVERS \$HOME_NET
var SQL_SERVERS \$HOME_NET
var TELNET_SERVERS \$HOME_NET
var SNMP_SERVERS \$HOME_NET
var FTP_SERVERS \$HOME_NET
var SSH_SERVERS \$HOME_NET
var POP_SERVERS \$HOME_NET
var IMAP_SERVERS \$HOME_NET
var RPC_SERVERS \$HOME_NET
var WWW_SERVERS \$HOME_NET
var AIM_SERVERS \
[64.12.24.0/23,64.12.28.0/23,64.12.161.0/24,64.12.163.0/24,64.12.200.0/24,205.188.3.0/24,205.188.5.0/24,205.188.7.0/24,205.188.9.0/24,205.188.153.0/24,205.188.179.0/24,205.188.248.0/24]

portvar HTTP_PORTS 80
portvar SHELLCODE_PORTS !80
portvar ORACLE_PORTS 1521
portvar AUTH_PORTS 113
portvar DNS_PORTS 53
portvar FINGER_PORTS 79
portvar FTP_PORTS 21
portvar IMAP_PORTS 143
portvar IRC_PORTS [6665,6666,6667,6668,6669,7000]
portvar MSSQL_PORTS 1433
portvar NNTP_PORTS 119
portvar POP2_PORTS 109
portvar POP3_PORTS 110
portvar SUNRPC_PORTS [111,32770,32771,32772,32773,32774,32775,32776,32777,32778,32779]
portvar RLOGIN_PORTS 513
portvar RSH_PORTS 514
portvar SMB_PORTS [139,445]
portvar SMTP_PORTS 25
portvar SNMP_PORTS 161
portvar SSH_PORTS {$ssh_port}
portvar TELNET_PORTS 23
portvar MAIL_PORTS [25,143,465,691]
portvar SSL_PORTS [25,443,465,636,993,995]

var RULE_PATH /usr/local/etc/snort/rules

# Configure the snort decoder
config checksum_mode: all
config disable_decode_alerts
config disable_tcpopt_experimental_alerts
config disable_tcpopt_obsolete_alerts
config disable_ttcp_alerts
config disable_tcpopt_alerts
config disable_ipopt_alerts
config disable_decode_drops

#Configure the detection engine
#Use lower memory models
config detection: search-method {$snort_performance}
config detection: max_queue_events 5
config event_queue: max_queue 8 log 3 order_events content_length

#Configure dynamic loaded libraries
dynamicpreprocessor file /usr/local/lib/snort/dynamicpreprocessor/libsf_dcerpc_preproc.so
dynamicpreprocessor file /usr/local/lib/snort/dynamicpreprocessor/libsf_dns_preproc.so
dynamicpreprocessor file /usr/local/lib/snort/dynamicpreprocessor/libsf_ftptelnet_preproc.so
dynamicpreprocessor file /usr/local/lib/snort/dynamicpreprocessor/libsf_smtp_preproc.so
dynamicpreprocessor file /usr/local/lib/snort/dynamicpreprocessor/libsf_ssh_preproc.so

dynamicengine /usr/local/lib/snort/dynamicengine/libsf_engine.so

#Flow and stream

preprocessor frag3_global: max_frags 8192
preprocessor frag3_engine: policy last detect_anomalies
preprocessor stream5_global: max_tcp 8192, track_tcp yes, \
track_udp yes, track_icmp yes
preprocessor stream5_tcp: policy BSD, ports both all, use_static_footprint_sizes
preprocessor stream5_udp
preprocessor stream5_icmp

#HTTP Inspect
preprocessor http_inspect: global iis_unicode_map unicode.map 1252

preprocessor http_inspect_server: server default \
                        ports  { 80 8080 3128 }  \
                        no_alerts \
                        non_strict \
                        non_rfc_char  { 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 }  \
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
preprocessor rpc_decode: 111 32770 32771 32772 32773 32774 32775 32776 32777 32778 32779
preprocessor bo

preprocessor ftp_telnet: global \
inspection_type stateless
preprocessor ftp_telnet_protocol: \
    ftp server default \
    def_max_param_len 100 \
    ports { 21 } \
    ftp_cmds { USER PASS ACCT CWD SDUP SMNT QUIT REIN PORT PASV TYPE STRU MODE } \
    ftp_cmds { RETR STOR STOU APPE ALLO REST RNFR RNTO ABOR DELE RMD MKD PWD } \
    ftp_cmds { LIST NLST SITE SYST STAT HELP NOOP } \
    ftp_cmds { AUTH ADAT PROT PBSZ CONF ENC } \
    ftp_cmds { FEAT OPTS CEL CMD MACB } \
    ftp_cmds { MDTM REST SIZE MLST MLSD } \
    ftp_cmds { XPWD XCWD XCUP XMKD XRMD TEST CLNT } \
    alt_max_param_len 0 { CDUP QUIT REIN PASV STOU ABOR PWD SYST NOOP } \
    alt_max_param_len 100 { MDTM CEL XCWD SITE USER PASS REST DELE RMD SYST TEST STAT MACB EPSV CLNT LPRT } \
    alt_max_param_len 200 { XMKD NLST ALLO STOU APPE RETR STOR CMD RNFR HELP } \
    alt_max_param_len 256 { RNTO CWD } \ 
    alt_max_param_len 400 { PORT } \
    alt_max_param_len 512 { SIZE } \
    chk_str_fmt { USER PASS ACCT CWD SDUP SMNT PORT TYPE STRU MODE } \
    chk_str_fmt { RETR STOR STOU APPE ALLO REST RNFR RNTO DELE RMD MKD } \
    chk_str_fmt { LIST NLST SITE SYST STAT HELP } \
    chk_str_fmt { AUTH ADAT PROT PBSZ CONF ENC } \
    chk_str_fmt { FEAT CEL CMD } \
    chk_str_fmt { MDTM REST SIZE MLST MLSD } \
    chk_str_fmt { XPWD XCWD XCUP XMKD XRMD TEST CLNT } \
    cmd_validity MODE < char ASBCZ > \
    cmd_validity STRU < char FRP > \
    cmd_validity ALLO < int [ char R int ] > \
    cmd_validity TYPE < { char AE [ char NTC ] | char I | char L [ number ] } > \
    cmd_validity MDTM < [ date nnnnnnnnnnnnnn[.n[n[n]]] ] string > \
    cmd_validity PORT < host_port >
preprocessor ftp_telnet_protocol: ftp client default \
   max_resp_len 100

preprocessor SMTP: \
    ports { 25 465 691 } \
    inspection_type stateful \
    normalize cmds \
    valid_cmds { MAIL RCPT HELP HELO ETRN EHLO EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET SEND SAML SOML AUTH TURN ETRN PIPELINING \
CHUNKING DATA DSN RSET QUIT ONEX QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    normalize_cmds { MAIL RCPT HELP HELO ETRN EHLO EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET SEND SAML SOML AUTH TURN ETRN \
PIPELINING CHUNKING DATA DSN RSET QUIT ONEX QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    max_header_line_len 1000 \ 
    max_response_line_len 512 \
    alt_max_command_line_len 260 { MAIL } \
    alt_max_command_line_len 300 { RCPT } \
    alt_max_command_line_len 500 { HELP HELO ETRN EHLO } \
    alt_max_command_line_len 255 { EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET } \
    alt_max_command_line_len 246 { SEND SAML SOML AUTH TURN ETRN PIPELINING CHUNKING DATA DSN RSET QUIT ONEX } \
    alt_max_command_line_len 246 { QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR } \
    alt_max_command_line_len 246 { XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    xlink2state { enable }




#sf Portscan
preprocessor sfportscan: proto { all } \
						scan_type { all } \
						sense_level { low } \
						ignore_scanners { \$HOME_NET }

preprocessor dcerpc: \
    autodetect \
    max_frag_size 3000 \
    memcap 100000

preprocessor dns: ports { 53 } enable_rdata_overflow

#Output plugins
#output database: alert
output alert_syslog: LOG_AUTH LOG_ALERT LOG_CONS LOG_NDELAY LOG_PERROR LOG_PID

output alert_unified: filename alert							
						
#Required files
include /usr/local/etc/snort/classification.config
include /usr/local/etc/snort/reference.config

# Include any thresholding or suppression commands. See threshold.conf in the
# include threshold.conf

# Snort user pass through configuration
{$snort_config_pass_thru}

#Rulesets, all optional
{$selected_rules_sections}

EOD;

	return $snort_conf_text;
}

/*   check downloaded text from snort.org to make sure that an error did not occur
 *   for example, if you are not a premium subscriber you can only download rules
 *   so often, etc.
 */
function check_for_common_errors($filename) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	$contents = file_get_contents($filename);
	if(stristr($contents, "You don't have permission")) {
		if(!$console_mode) {
			update_all_status("An error occured while downloading {$filename}.");
			hide_progress_bar_status();
		} else {
			log_error("An error occured.  Scroll down to inspect it's contents.");
			echo "An error occured.  Scroll down to inspect it's contents.";
		}
		if(!$console_mode) {
			update_output_window(strip_tags("$contents"));
		} else {
			$contents = strip_tags($contents);
			log_error("Error downloading snort rules: {$contents}");
			echo "Error downloading snort rules: {$contents}";
		}
		scroll_down_to_bottom_of_page();
		exit;
	}
}

/* force browser to scroll all the way down */
function scroll_down_to_bottom_of_page() {
	global $snort_filename, $console_mode;
	ob_flush();
	if(!$console_mode)
		echo "\n<script type=\"text/javascript\">parent.scrollTo(0,1500);\n</script>";
}

/* ensure downloaded file looks sane */
function verify_downloaded_file($filename) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(filesize($filename)<9500) {
		if(!$console_mode) {
			update_all_status("Checking {$filename}...");
			check_for_common_errors($filename);
		}
	}
	update_all_status("Verifying {$filename}...");
	if(!file_exists($filename)) {
		if(!$console_mode) {
			update_all_status("Could not fetch snort rules ({$filename}).  Check oinkid key and dns and try again.");
			hide_progress_bar_status();
		} else {
			log_error("Could not fetch snort rules ({$filename}).  Check oinkid key and dns and try again.");
			echo "Could not fetch snort rules ({$filename}).  Check oinkid key and dns and try again.";
		}
		exit;
	}
	update_all_status("Verifyied {$filename}.");
}

/* extract rules */
function extract_snort_rules_md5($tmpfname) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(!$console_mode) {
		$static_output = gettext("Extracting snort rules...");
		update_all_status($static_output);
	}
	mkdir("/usr/local/etc/snort/rules/");
	$cmd = "/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C /usr/local/etc/snort/ rules/";
	$handle = popen("{$cmd} 2>&1", 'r');
	while(!feof($handle)) {
		$buffer = fgets($handle);
		update_output_window($buffer);
	}
	pclose($handle);	

	if(!$console_mode) {
		$static_output = gettext("Snort rules extracted.");
		update_all_status($static_output);
	} else {
		log_error("Snort rules extracted.");
		echo "Snort rules extracted.";
	}
}

/* verify MD5 against downloaded item */
function verify_snort_rules_md5($tmpfname) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(!$console_mode) {
		$static_output = gettext("Verifying md5 signature...");
		update_all_status($static_output);
	}
	$md5 = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
	$file_md5_ondisk = `/sbin/md5 {$tmpfname}/{$snort_filename} | /usr/bin/awk '{ print $4 }'`;
	if($md5 <> $file_md5_ondisk) {
		if(!$console_mode) {
			$static_output = gettext("snort rules: md5 signature of rules mismatch.");
			update_all_status($static_output);
			hide_progress_bar_status();
		} else {
			log_error("snort rules: md5 signature of rules mismatch.");
			echo "snort rules: md5 signature of rules mismatch.";
		}
		exit;
	}
}

/* hide progress bar */
function hide_progress_bar_status() {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(!$console_mode)
		echo "\n<script type=\"text/javascript\">document.progressbar.style.visibility='hidden';\n</script>";
}

/* update both top and bottom text box during an operation */
function update_all_status($status) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(!$console_mode) {
		update_status($status);
		update_output_window($status);
	}
}

/* obtain alert description for an ip address */
function get_snort_alert($ip) {
	global $snort_alert_file_split, $snort_config;
	if(!file_exists("/var/log/snort/alert"))
		return;
	if(!$snort_config)
		$snort_config = read_snort_config_cache();
	if($snort_config[$ip])
		return $snort_config[$ip];
	if(!$snort_alert_file_split)
		$snort_alert_file_split = split("\n", file_get_contents("/var/log/snort/alert"));
	foreach($snort_alert_file_split as $fileline) {
		if (preg_match("/\[\*\*\] (\[.*\]) (.*) (\[\*\*\])/", $fileline, $matches))
			$alert_title = $matches[2];
		if (preg_match("/(\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b)/", $fileline, $matches))
			$alert_ip = $matches[0];
		if($alert_ip == $ip) {
			if(!$snort_config[$ip])
				$snort_config[$ip] = $alert_title;
			return $alert_title;
		}
	}
	return "n/a";
}

function make_clickable($buffer) {
    global $config, $g;
    /* if clickable urls is disabled, simply return buffer back to caller */
    $clickablalerteurls = $config['installedpackages']['snort']['config'][0]['oinkmastercode'];
    if(!$clickablalerteurls)
    	return $buffer;
    $buffer = eregi_replace("(^|[ \n\r\t])((http(s?)://)(www\.)?([a-z0-9_-]+(\.[a-z0-9_-]+)+)(/[^/ \n\r]*)*)","\\1<a href=\"\\2\" target=\"_blank\">\\2</a>", $buffer);
    $buffer = eregi_replace("(^|[ \n\r\t])((ftp://)(www\.)?([a-z0-9_-]+(\.[a-z0-9_-]+)+)(/[^/ \n\r]*)*)","\\1<a href=\"\\2\" target=\"_blank\">\\2</a>", $buffer);
    $buffer = eregi_replace("([a-z_-][a-z0-9\._-]*@[a-z0-9_-]+(\.[a-z0-9_-]+)+)","<a href=\"mailto:\\1\">\\1</a>", $buffer);
    $buffer = eregi_replace("(^|[ \n\r\t])(www\.([a-z0-9_-]+(\.[a-z0-9_-]+)+)(/[^/ \n\r]*)*)","\\1<a href=\"http://\\2\" target=\"_blank\">\\2</a>", $buffer);
    $buffer = eregi_replace("(^|[ \n\r\t])(ftp\.([a-z0-9_-]+(\.[a-z0-9_-]+)+)(/[^/ \n\r]*)*)","\\1<a href=\"ftp://\\2\" target=\"_blank\">\\2</a>", $buffer);

    return $buffer;
}

function read_snort_config_cache() {
	global $g, $config, $snort_config;
	if($snort_config)
		return $snort_config;
	if(file_exists($g['tmp_path'] . '/snort_config.cache')) {
		$snort_config = unserialize(file_get_contents($g['tmp_path'] . '/snort_config.cache'));
		return $snort_config;
	}
	return;
}

function write_snort_config_cache($snort_config) {
	global $g, $config;
	conf_mount_rw();
	$configcache = fopen($g['tmp_path'] . '/snort_config.cache', "w");
	if(!$configcache) {
		log_error("Could not open {$g['tmp_path']}/snort_config.cache  for writing.");
		return false;
	}
	fwrite($configcache, serialize($snort_config));
	fclose($configcache);
	conf_mount_ro();
	return true;
}

function snort_advanced() {
	global $g, $config;
	sync_package_snort();
}

?>
