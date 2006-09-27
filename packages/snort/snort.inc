<?php
/* $Id$ */
/*
	snort.inc
	Copyright (C) 2006 Scott Ullrich
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

$snort_conf_file = "/usr/local/etc/snort/snort.conf";

/* define oinkid */
if($config['installedpackages']['snort'])
	$oinkid = $config['installedpackages']['snort']['config'][0]['oinkmastercode'];

function sync_package_snort() {
	global $config, $g, $snort_conf_file;
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
	/* generate if list */
	$iflist = array("lan" => "LAN");
	for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++)
		$iflist['opt' . $i] = "opt{$i}";
	foreach($_POST['iface_array'] as $iface) {
		$if = convert_friendly_interface_to_real_interface_name($iface);
		if($config['interfaces'][$iface]['ipaddr'] == "pppoe")
			$if = "ng0";
		if($if) {
			$ifaces_final .= " -i " . $if;
			$first = 1;
		}
	}

	/* create log directory */
	$start  = "/bin/mkdir -p /var/log/snort";

	/* start snort */
	$start .= ";snort -c {$snort_conf_file} -l /var/log/snort {$ifaces_final} -A full -D";

	/* if block offenders is checked, start snort2c */
	if($_POST['blockoffenders'])
		$start .= ";sleep 1;snort2c -w /var/db/whitelist -a /var/log/snort/alert";

	write_rcfile(array(
			"file" => "snort.sh",
			"start" => $start,
			"stop" => "/usr/bin/killall snort; killall snort2c"
		)
	);

	/* create snort configuration file */
	create_snort_conf();

	/* start snort service */
	start_service("snort");
}

function create_snort_conf() {
	global $config, $g, $snort_conf_file;
	/* write out snort.conf */
	$snort_conf_text = generate_snort_conf();
	$conf = fopen($snort_conf_file, "w");
	if(!$conf) {
		log_error("Could not open {$snort_conf_file} for writing.");
		exit;
	}
	fwrite($conf, $snort_conf_text);
	fclose($conf);
}

function generate_snort_conf() {
	global $config, $g, $snort_conf_file;
	/* obtain external interface */
	/* XXX: make multi wan friendly */
	$snort_ext_int = $config['installedpackages']['snort']['config'][0]['iface_array'][0];

	$text = "*/60\t* \t 1\t *\t *\t root\t /usr/bin/nice -n20 /usr/local/pkg/snort_check_for_rule_updates.php";
	$file = "/etc/crontab";
	add_text_to_file($file, $text);
	remove_text_from_file($file, $text);
	exec("killall -HUP cron");

	/* should we install a automatic update crontab entry? */
	$automaticrulesupdate = $config['installedpackages']['snort']['config'][0]['automaticrulesupdate'];

	if($config['interfaces'][$snort_ext_int]['ipaddr'] == "pppoe")
		$snort_ext_int = "ng0";

	/* set the snort performance model */
	if($config['installedpackages']['snort']['config'][0]['performance'])
		$snort_performance = $config['installedpackages']['snort']['config'][0]['performance'];
	else
		$snort_performance = "lowmem";

	/* open snort2c's whitelist for writing */
	$whitelist = fopen("/var/db/whitelist", "w");
	if(!$whitelist) {
		log_error("Could not open /var/db/whitelist for writing.");
		exit;
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

	/* iterate through interface list and write out whitelist items
	 * and also compile a home_net list for snort.
	 */
	foreach($int_array as $int) {
		/* calculate interface subnet information */
		$ifcfg = &$config['interfaces'][$int];
		$subnet = gen_subnet($ifcfg['ipaddr'], $ifcfg['subnet']);
		$subnetmask = gen_subnet_mask($ifcfg['subnet']);
		$home_net .= "{$subnet}/{$ifcfg['subnet']} ";
	}

	/* add all local ips to the whitelist */
	$wan_if = get_real_wan_interface();
	$ip = find_interface_ip($wan_if);
	$home_net .= "{$ip} ";

	/* iterate all interfaces and add to whitelist */
	foreach($config['interfaces'] as $interface)
        $home_net .= "{$interface['ipaddr']} ";

	/* iterate all vips and add to whitelist */
	if($config['virtualip'])
		foreach($config['virtualip']['vip'] as $vip)
			$home_net .= $vip['subnet'] . " ";

	if($config['installedpackages']['snortwhitelist'])
		foreach($config['installedpackages']['snortwhitelist']['config'] as $snort)
			$home_net .= $snort['ip'] . " ";

	/* write out whitelist, convert spaces to carriage returns */
	$whitelist_home_net = str_replace(" ", "\n", $home_net);

	fwrite($whitelist, $whitelist_home_net);

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

	/* build snort configuration file */
	$snort_conf_text = <<<EOD

#snort configuration file
#generated by the pfSense
#package manager system
#see /usr/local/pkg/snort.inc
#for more information

var AIM_SERVERS [64.12.24.0/24,64.12.25.0/24,64.12.26.14/24,64.12.28.0/24,64.12.29.0/24,64.12.161.0/24,64.12.163.0/24,205.188.5.0/24,205.188.9.0/24]
var HTTP_PORTS 80
var SHELLCODE_PORTS !\$HTTP_PORTS
var ORACLE_PORTS 1521
var HOME_NET {$home_net}
var TELNET_SERVERS \$HOME_NET
var SQL_SERVERS \$HOME_NET
var HTTP_SERVERS \$HOME_NET
var SMTP_SERVERS \$HOME_NET
var DNS_SERVERS \$HOME_NET
var EXTERNAL_NET !\$HOME_NET
var SSH_PORTS {$ssh_port}
var RULE_PATH /usr/local/etc/snort/rules

#Use lower memory models
config detection: search-method {$snort_performance}

#Output plugins
#output database: alert
output alert_syslog: LOG_AUTH LOG_ALERT LOG_CONS LOG_NDELAY LOG_PERROR LOG_PID

output alert_unified: filename alert

#Flow and stream
preprocessor flow: stats_interval 0 hash 2
preprocessor frag2
preprocessor stream4: disable_evasion_alerts,detect_scans

preprocessor stream4_reassemble: both, ports all

#XLink2State mini proc
#preprocessor xlink2state: ports { 25 691 }

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
        server-watchnet \$HOME_NET \
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

EOD;

	return $snort_conf_text;
}

function check_for_common_errors($filename) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	$contents = file_get_contents($filename);
	if(stristr($contents, "You don't have permission")) {
		if(!$console_mode) {
			update_all_status("An error occured.  Scroll down to inspect it's contents.");
			hide_progress_bar_status();
		}
		echo "
      <center>
        <div id='error' style='background:white;width:90%'>
        <!-- TODO: The below paragraphs are kind of stupid. Use CSS instead -->
        <p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>
        <p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>
        <p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>
        <p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>
        <p>The following error occured while downloading the snort rules file from snort.org:</p>
		    {$contents}
        <p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>
		    </div>
      </center>
    ";
		scroll_down_to_bottom_of_page();
		exit;
	}
}

function scroll_down_to_bottom_of_page() {
	global $snort_filename, $console_mode;
	ob_flush();
	if(!$console_mode)
		echo "\n<script type=\"text/javascript\">parent.scrollTo(0,1500);\n</script>";
}

function verify_downloaded_file($filename) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(filesize($filename)<99500) {
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
		}
		exit;
	}
	update_all_status("Verifyied {$filename}.");
}

function extract_snort_rules_md5($tmpfname) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(!$console_mode) {
		$static_output = gettext("Extracting snort rules...");
		update_all_status($static_output);
	}
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C /usr/local/etc/snort/");
	if(!$console_mode) {
		$static_output = gettext("Snort rules extracted.");
		update_all_status($static_output);
	}
}

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
			$static_output = gettext("md5 signature of rules mismatch.");
			update_all_status($static_output);
			hide_progress_bar_status();
		}
		exit;
	}
}

function hide_progress_bar_status() {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(!$console_mode)
		echo "\n<script type=\"text/javascript\">document.progressbar.style.visibility='hidden';\n</script>";
}

function update_all_status($status) {
	global $snort_filename, $snort_filename_md5;
	ob_flush();
	if(!$console_mode) {
		update_status($status);
		update_output_window($status);
	}
}

?>