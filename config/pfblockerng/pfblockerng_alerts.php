<?php
/*
	pfBlockerNG_Alerts.php

	pfBlockerNG
	Copyright (C) 2014 BBcan177@gmail.com
	All rights reserved.

	Portions of this code are based on original work done for
	pfSense from the following contributors:

	Parts based on works from Snort_alerts.php
	Copyright (C) 2014 Bill Meeks
	All rights reserved.

	Javascript Hostname Lookup modifications by J. Nieuwenhuizen

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

require_once("util.inc");
require_once("guiconfig.inc");
require_once("globals.inc");
require_once("filter_log.inc");
require_once("/usr/local/pkg/pfblockerng/pfblockerng.inc");

pfb_global();

// Application Paths
$pathgeoip	= "/usr/pbi/pfblockerng-" . php_uname("m") . "/bin/geoiplookup";
$pathgeoip6	= "/usr/pbi/pfblockerng-" . php_uname("m") . "/bin/geoiplookup6";

// Define File Locations
$filter_logfile = "{$g['varlog_path']}/filter.log";
$pathgeoipdat   = "{$pfb['dbdir']}/GeoIP.dat";
$pathgeoipdat6  = "{$pfb['dbdir']}/GeoIPv6.dat";

// Emerging Threats IQRisk Header Name Reference
$pfb['et_header'] = TRUE;
$et_header = $config['installedpackages']['pfblockerngreputation']['config'][0]['et_header'];
if (empty($et_header))
	$pfb['et_header'] = FALSE;

// Collect pfBlockerNGSuppress Alias and Create pfbsuppression.txt
if ($pfb['supp'] == "on")
	pfb_create_suppression_file();

// Collect Number of Suppressed Hosts
if (file_exists("{$pfb['supptxt']}")) {
	$pfbsupp_cnt = exec ("/usr/bin/grep -c ^ {$pfb['supptxt']}");
} else {
	$pfbsupp_cnt = 0;
}

// Collect pfBlockerNG Rule Names and Number
$rule_list = array();
$results = array();
$data = exec ("/sbin/pfctl -vv -sr | grep 'pfB_'", $results);

if (empty($config['installedpackages']['pfblockerngglobal']['pfbdenycnt']))
	$config['installedpackages']['pfblockerngglobal']['pfbdenycnt']		= '25';
if (empty($config['installedpackages']['pfblockerngglobal']['pfbpermitcnt']))
	$config['installedpackages']['pfblockerngglobal']['pfbpermitcnt']	= '5';
if (empty($config['installedpackages']['pfblockerngglobal']['pfbmatchcnt']))
	$config['installedpackages']['pfblockerngglobal']['pfbmatchcnt']	= '5';
if (empty($config['installedpackages']['pfblockerngglobal']['alertrefresh']))
	$config['installedpackages']['pfblockerngglobal']['alertrefresh']	= 'off';
if (empty($config['installedpackages']['pfblockerngglobal']['hostlookup']))
	$config['installedpackages']['pfblockerngglobal']['hostlookup']		= 'off';

if (isset($_POST['save'])) {
	if (!is_array($config['installedpackages']['pfblockerngglobal']))
		$config['installedpackages']['pfblockerngglobal'] = array();
	$config['installedpackages']['pfblockerngglobal']['alertrefresh']	= $_POST['alertrefresh'] ? 'on' : 'off';
	$config['installedpackages']['pfblockerngglobal']['hostlookup']		= $_POST['hostlookup'] ? 'on' : 'off';
	if (is_numeric($_POST['pfbdenycnt']))
		$config['installedpackages']['pfblockerngglobal']['pfbdenycnt']		= $_POST['pfbdenycnt'];
	if (is_numeric($_POST['pfbpermitcnt']))
		$config['installedpackages']['pfblockerngglobal']['pfbpermitcnt']	= $_POST['pfbpermitcnt'];
	if (is_numeric($_POST['pfbmatchcnt']))
		$config['installedpackages']['pfblockerngglobal']['pfbmatchcnt']	= $_POST['pfbmatchcnt'];

	write_config("pfBlockerNG pkg: updated ALERTS tab settings.");
	header("Location: " . $_SERVER['PHP_SELF']);
	exit;
}

if (is_array($config['installedpackages']['pfblockerngglobal'])) {
	$alertrefresh	= $config['installedpackages']['pfblockerngglobal']['alertrefresh'];
	$hostlookup	= $config['installedpackages']['pfblockerngglobal']['hostlookup'];
	$pfbdenycnt	= $config['installedpackages']['pfblockerngglobal']['pfbdenycnt'];
	$pfbpermitcnt	= $config['installedpackages']['pfblockerngglobal']['pfbpermitcnt'];
	$pfbmatchcnt	= $config['installedpackages']['pfblockerngglobal']['pfbmatchcnt'];
}

// Collect pfBlockerNG Firewall Rules
if (is_array($results)) {
	foreach ($results as $result) {

		# Find Rule Descriptions
		$descr = "";
		if (preg_match("/USER_RULE: (\w+)/",$result,$desc))
			$descr = $desc[1];

		if ($pfb['pfsenseversion'] >= '2.2') {
			preg_match ("/@(\d+)\(/",$result, $rule);
		} else {
			preg_match ("/@(\d+)\s/",$result, $rule);
		}

		$id = $rule[1];
		# Create array of Rule Description and pfctl Rule Number
		$rule_list['id'][] = $id;
		$rule_list[$id]['name'] = $descr;
	}
}

// Add IP to the Suppression Alias
if (isset($_POST['addsuppress'])) {
	$ip = "";
	if (isset($_POST['ip'])) {
		$ip = $_POST['ip'];
		$table = $_POST['table'];
		$descr = $_POST['descr'];
		if (empty($descr))
			$descr = sprintf(gettext("Entry added %s"), date('r'));
		$cidr = $_POST['cidr'];
		if (is_ipaddr($ip)) {

			$savemsg1 = "Host IP address {$ip}";
			if (is_ipaddrv4($ip)) {
				$iptrim1 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '$1.$2.$3.0/24', $ip);
				$iptrim2 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '$1.$2.$3.', $ip);
				$iptrim3 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '$4', $ip);

				if ($cidr == "32") {
					$pfb_pfctl = exec ("/sbin/pfctl -t {$table} -T show | grep {$iptrim1} 2>&1");

					if ($pfb_pfctl == "") {
						$savemsg2 = " : Removed /32 entry";
						exec ("/sbin/pfctl -t {$table} -T delete {$ip}");
					} else {
						$savemsg2 = " : Removed /24 entry, added 254 addr";
						exec ("/sbin/pfctl -t {$table} -T delete {$iptrim1}");
						for ($add_ip=0; $add_ip <= 255; $add_ip++){
							if ($add_ip != $iptrim3) {
								exec ("/sbin/pfctl -t {$table} -T add {$iptrim2}{$add_ip}");
							}
						}
					}
				} else {
					$cidr = 24;
					$savemsg2 = " : Removed /24 entry";
					exec ("/sbin/pfctl -t {$table} -T delete {$iptrim1} 2>&1", $pfb_pfctl);
					if (!preg_grep("/1\/1 addresses deleted/", $pfb_pfctl)) {
						$savemsg2 = " : Removed all entries";
						// Remove 0-255 IP Address from Alias Table
						for ($del_ip=0; $del_ip <= 255; $del_ip++){
							exec ("/sbin/pfctl -t {$table} -T delete {$iptrim2}{$del_ip}");
						}
					}
				}
			}

			// Collect pfBlockerNGSuppress Alias Contents
			$pfb_sup_list = array();
			$pfb_sup_array = array();
			$pfb['found'] = FALSE;
			$pfb['update'] = FALSE;
			if (is_array($config['aliases']['alias'])) {
				foreach ($config['aliases']['alias'] as $alias) {
					if ($alias['name'] == "pfBlockerNGSuppress") {
						$data = $alias['address'];
						$data2 = $alias['detail'];
						$arr1 = explode(" ",$data);
						$arr2 = explode("||",$data2);

						if (!empty($data)) {
							$row = 0;
							foreach ($arr1 as $host) {
								$pfb_sup_list[] = $host;
								$pfb_sup_array[$row]['host'] = $host;
								$row++;
							}
							$row = 0;
							foreach ($arr2 as $detail) {
								$pfb_sup_array[$row]['detail'] = $detail;
								$row++;
							}
						}
						$pfb['found'] = TRUE;
					}
				}
			}

			// Call Function to Create Suppression Alias if not found.
			if (!$pfb['found'])
				pfb_create_suppression_alias();

			// Save New Suppress IP to pfBlockerNGSuppress Alias
			if (in_array($ip . '/' . $cidr, $pfb_sup_list)) {
				$savemsg = gettext("Host IP address {$ip} already exists in the pfBlockerNG Suppress Table.");
			} else {
				if (!$pfb['found'] && empty($pfb_sup_list)) {
					$next_id = 0;
				} else {
					$next_id = count($pfb_sup_list);
				}
				$pfb_sup_array[$next_id]['host'] = $ip . '/' . $cidr;
				$pfb_sup_array[$next_id]['detail'] = $descr;

				$address = "";
				$detail = "";
				foreach ($pfb_sup_array as $pfb_sup) {
					$address .= $pfb_sup['host'] . " ";
					$detail .= $pfb_sup['detail'] . "||";
				}

				// Find pfBlockerNGSuppress Array ID Number
				if (is_array($config['aliases']['alias'])) {
					$pfb_id = 0;
					foreach ($config['aliases']['alias'] as $alias) {
						if ($alias['name'] == "pfBlockerNGSuppress") {
							break;
						}
						$pfb_id++;
					}
				}

				$config['aliases']['alias'][$pfb_id]['address']	= rtrim($address, " ");
				$config['aliases']['alias'][$pfb_id]['detail']	= rtrim($detail, "||");
				$savemsg = gettext($savemsg1) . gettext($savemsg2) . gettext(" and added Host to the pfBlockerNG Suppress Table.");
				$pfb['update'] = TRUE;
			}

			if ($pfb['found'] || $pfb['update']) {
				// Save all Changes to pfsense config file
				write_config();
			}
		}
	}
}

// Auto-Resolve Hostnames
if (isset($_REQUEST['getpfhostname'])) {
	$getpfhostname = htmlspecialchars($_REQUEST['getpfhostname']);
	$hostname = htmlspecialchars(gethostbyaddr($getpfhostname), ENT_QUOTES);
	if ($hostname == $getpfhostname) {
		$hostname = 'unknown';
	}
	echo $hostname;
	die;
}


// Host Resolve Function lookup
function getpfbhostname($type = 'src', $hostip, $countme = 0) {
	$hostnames['src'] = '';
	$hostnames['dst'] = '';
	$hostnames[$type] = '<div id="gethostname_' . $countme . '" name="' . $hostip . '"></div>';
	return $hostnames;
}


// Determine if Alert Host 'Dest' is within the Local Lan IP Range.
function check_lan_dest($lan_ip,$lan_mask,$dest_ip,$dest_mask="32") {
	$result = check_subnets_overlap($lan_ip, $lan_mask, $dest_ip, $dest_mask);
	return $result;
}


$pgtitle = gettext("pfBlockerNG: Alerts");
include_once("head.inc");
?>
<body link="#000000" vlink="#0000CC" alink="#000000">
<form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="post">
<input type="hidden" name="ip" id="ip" value=""/>
<input type="hidden" name="table" id="table" value=""/>
<input type="hidden" name="descr" id="descr" value=""/>
<input type="hidden" name="cidr" id="cidr" value=""/>
<?php

include_once("fbegin.inc");

/* refresh every 60 secs */
if ($alertrefresh == 'on')
	echo "<meta http-equiv=\"refresh\" content=\"60;url={$_SERVER['PHP_SELF']}\" />\n";
if ($savemsg) {
	print_info_box($savemsg);
}

?>
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>
			<?php
				$tab_array = array();
				$tab_array[] = array(gettext("General"), false, "/pkg_edit.php?xml=pfblockerng.xml&amp;id=0");
				$tab_array[] = array(gettext("Update"), false, "/pfblockerng/pfblockerng_update.php");
				$tab_array[] = array(gettext("Alerts"), true, "/pfblockerng/pfblockerng_alerts.php");
				$tab_array[] = array(gettext("Reputation"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_reputation.xml&id=0");
				$tab_array[] = array(gettext("IPv4"), false, "/pkg.php?xml=/pfblockerng/pfblockerng_v4lists.xml");
				$tab_array[] = array(gettext("IPv6"), false, "/pkg.php?xml=/pfblockerng/pfblockerng_v6lists.xml");
				$tab_array[] = array(gettext("Top 20"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_top20.xml&id=0");
				$tab_array[] = array(gettext("Africa"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_Africa.xml&id=0");
				$tab_array[] = array(gettext("Asia"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_Asia.xml&id=0");
				$tab_array[] = array(gettext("Europe"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_Europe.xml&id=0");
				$tab_array[] = array(gettext("N.A."), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_NorthAmerica.xml&id=0");
				$tab_array[] = array(gettext("Oceania"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_Oceania.xml&id=0");
				$tab_array[] = array(gettext("S.A."), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_SouthAmerica.xml&id=0");
				$tab_array[] = array(gettext("Logs"), false, "/pfblockerng/pfblockerng_log.php");
				$tab_array[] = array(gettext("Sync"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_sync.xml&id=0");
				display_top_tabs($tab_array, true);
			?>
		</td>
	</tr>
	<tr>
	<td><div id="mainarea">
		<table id="maintable" class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="6">
			<tr>
				<td colspan="3" class="vncell" align="left"><?php echo gettext("LINKS :"); ?> &nbsp;
				<a href='/firewall_aliases.php' target="_blank"><?php echo gettext("Firewall Alias"); ?></a> &nbsp;
				<a href='/firewall_rules.php' target="_blank"><?php echo gettext("Firewall Rules"); ?></a> &nbsp;
				<a href='/diag_logs_filter.php' target="_blank"><?php echo gettext("Firewall Logs"); ?></a><br/></td>
			</tr>
			<tr>
			<td width="10%" class="vncell"><?php echo gettext('Alert Settings'); ?></td>
			<td width="90%" class="vtable">
				<input name="pfbdenycnt" type="text" class="formfld unknown" id="pdbdenycnt" size="1" title="Enter the number of 'Deny' Alerts to Show"  value="<?=htmlspecialchars($pfbdenycnt);?>"/>
				<?php printf(gettext('%sDeny%s.&nbsp;&nbsp;') , '<strong>', '</strong>'); ?>
				<input name="pfbpermitcnt" type="text" class="formfld unknown" id="pdbpermitcnt" size="1" title="Enter the number of 'Permit' Alerts to Show" value="<?=htmlspecialchars($pfbpermitcnt);?>"/>
				<?php printf(gettext('%sPermit%s.&nbsp;&nbsp;'), '<strong>', '</strong>'); ?>
				<input name="pfbmatchcnt" type="text" class="formfld unknown" id="pdbmatchcnt" size="1" title="Enter the number of 'Match' Alerts to Show" value="<?=htmlspecialchars($pfbmatchcnt); ?>"/>
				<?php printf(gettext('%sMatch%s.'), '<strong>', '</strong>'); ?>

				<?php echo gettext('&nbsp;&nbsp;&nbsp;&nbsp;Click to Auto-Refresh');?>&nbsp;&nbsp;<input name="alertrefresh" type="checkbox" value="on" title="Click to enable Auto-Refresh of this Tab once per minute"
				<?php if ($config['installedpackages']['pfblockerngglobal']['alertrefresh']=="on") echo "checked"; ?>/>&nbsp;

				<?php echo gettext('&nbsp;Click to Auto-Resolve');?>&nbsp;&nbsp;<input name="hostlookup" type="checkbox" value="on" title="Click to enable Auto-Resolve of Hostnames. Country Blocks/Permit/Match Lists will not auto-resolve"
				<?php if ($config['installedpackages']['pfblockerngglobal']['hostlookup']=="on") echo "checked"; ?>/>&nbsp;&nbsp;&nbsp;
				<input name="save" type="submit" class="formbtns" value="Save" title="<?=gettext('Save settings');?>"/><br>

				<?php printf(gettext('Enter number of log entries to view.')); ?>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<?php printf(gettext("Currently Suppressing &nbsp; %s$pfbsupp_cnt%s &nbsp; Hosts."), '<strong>', '</strong>');?>
			</td>
			</tr>
<!--Create Three Output Windows 'Deny', 'Permit' and 'Match'-->
<?php foreach (array ("Deny" => $pfb['denydir'], "Permit" => $pfb['permitdir'], "Match" => $pfb['matchdir']) as $type => $pfbfolder ):
	switch($type) {
		case "Deny":
			$rtype = "block";
			$pfbentries = "{$pfbdenycnt}";
			break;
		case "Permit":
			$rtype = "pass";
			$pfbentries = "{$pfbpermitcnt}";
			break;
		case "Match":
			if ($pfb['pfsenseversion'] >= '2.2') {
				$rtype = "unkn(%u)";
			} else {
				$rtype = "unkn(11)";
			}
			$pfbentries = "{$pfbmatchcnt}";
			break;
	}

?>
			<table id="maintable" class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="6">
			<tr>
				<!--Print Table Info-->
				<td colspan="2" class="listtopic"><?php printf(gettext("&nbsp;{$type}&nbsp;&nbsp; - &nbsp; Last %s Alert Entries."), "{$pfbentries}"); ?>
					<?php if ($pfb['pfsenseversion'] >= '2.2'): ?>
						<?php if (!array_key_exists("reverse", $config['syslog'])): ?>
							&nbsp;&nbsp;<?php echo gettext("Firewall Logs must be in Reverse Order."); ?>
						<?php endif; ?>
					<?php else: ?>
						&nbsp;&nbsp;<?php echo gettext("Firewall Rule changes can unsync these Alerts."); ?>
						<?php if (!array_key_exists("reverse", $config['syslog'])): ?>
							&nbsp;&nbsp;<?php echo gettext("Firewall Logs must be in Reverse Order."); ?>
						<?php endif; ?>
					<?php endif; ?>
				</td>
			</tr>

<td width="100%" colspan="2">
<table id="pfbAlertsTable" style="table-layout: fixed;" width="100%" class="sortable" border="0" cellpadding="0" cellspacing="0">
	<colgroup>
		<col width="8%" align="center" axis="date">
		<col width="6%" align="center" axis="string">
		<col width="17%" align="center" axis="string">
		<col width="6%" align="center" axis="string">
		<col width="20%" align="center" axis="string">
		<col width="20%" align="center" axis="string">
		<col width="3%" align="center" axis="string">
		<col width="12%" align="center" axis="string">
	</colgroup>
	<thead>
		<tr class="sortableHeaderRowIdentifier">
			<th class="listhdrr" axis="date"><?php echo gettext("Date"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("IF"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("Rule"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("Proto"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("Source"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("Destination"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("CC"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("List"); ?></th>
		</tr>
	</thead>
	<tbody>
<?php

$pfb['runonce'] = TRUE;
if (isset($pfb['load']))
	$pfb['runonce'] = FALSE;

// Execute the following once per refresh
if ($pfb['runonce']) {
	$pfb['load'] = TRUE;
	$fields_array = array();

	// pfSense versions below 2.2 have the Logfiles in two lines.
	if ($pfb['pfsenseversion'] >= '2.2') {
		$pfblines = exec("/usr/bin/grep -c ^ {$filter_logfile}");
	} else {
		$pfblines = (exec("/usr/bin/grep -c ^ {$filter_logfile}") /2 );
	}
	$fields_array = conv_log_filter($filter_logfile, $pfblines, $pfblines);

	$continents = array('pfB_Africa','pfB_Antartica','pfB_Asia','pfB_Europe','pfB_NAmerica','pfB_Oceania','pfB_SAmerica','pfB_Top');

	$supp_ip_txt .= "Clicking this Suppression Icon, will immediately remove the Block.\n\nSuppressing a /32 CIDR is better than Suppressing the full /24";
	$supp_ip_txt .= " CIDR.\nThe Host will be added to the pfBlockerNG Suppress Alias Table.\n\nOnly 32 or 24 CIDR IPs can be Suppressed with the '+' Icon.";
	$supp_ip_txt .= "\nTo manually add Host(s), edit the 'pfBlockerNGSuppress' Alias in the Alias Tab.\nManual entries will not remove existing Blocked Hosts";

	// Array of all Local IPs for Alert Analysis
	$pfb_local = array();

	// Collect Gateway IP Addresses for Inbound/Outbound List matching
	$int_gateway = get_interfaces_with_gateway();
	if (is_array($int_gateway)) {
		foreach ($int_gateway as $gateway) {
			$convert = get_interface_ip($gateway);
			$pfb_local[] = $convert;
		}
	}

	// Collect Virtual IP Aliases for Inbound/Outbound List Matching
	if (is_array($config['virtualip']['vip'])) {
		foreach ($config['virtualip']['vip'] as $list) {
			$pfb_local[] = $list['subnet'];
		}
	}
	// Collect NAT IP Addresses for Inbound/Outbound List Matching
	if (is_array($config['nat']['rule'])) {
		foreach ($config['nat']['rule'] as $natent) {
			$pfb_local[] = $natent['target'];
		}
	}

	// Collect 1:1 NAT IP Addresses for Inbound/Outbound List Matching
	if(is_array($config['nat']['onetoone'])) {
		foreach ($config['nat']['onetoone'] as $onetoone) {
			$pfb_local[] = $onetoone['source']['address'];
		}
	}

	// Convert any 'Firewall Aliases' to IP Address Format
	if (is_array($config['aliases']['alias'])) {
		for ($cnt = 0; $cnt <= count($pfb_local); $cnt++) {
			foreach ($config['aliases']['alias'] as $i=> $alias) {
				if (isset($alias['name']) && isset($pfb_local[$cnt])) {
					if ($alias['name'] == $pfb_local[$cnt]) {
						$pfb_local[$cnt] = $alias['address'];
					}
				}
			}
		}
	}
	// Remove any Duplicate IPs
	$pfb_local = array_unique($pfb_local);

	// Determine Lan IP Address and Mask
	if (is_array($config['interfaces']['lan'])) {
		$lan_ip = $config['interfaces']['lan']['ipaddr'];
		$lan_mask = $config['interfaces']['lan']['subnet'];
	}
}

$counter = 0;
// Process Fields_array and generate Output
if (isset($fields_array)) {
	foreach ($fields_array as $fields) {
		$rulenum	= "";
		$alert_ip	= "";
		$supp_ip	= "";
		$pfb_query	= "";

		$rulenum = $fields['rulenum'];
		if ($fields['act'] == $rtype && !empty($rule_list) && in_array($rulenum, $rule_list['id']) && $counter < $pfbentries) {

			// Skip Repeated Events
			if (($fields['dstip'] . $fields['dstport']) == $previous_dstip || ($fields['srcip'] . $fields['srcport']) == $previous_srcip) {
				continue;
			}

			$proto = str_replace("TCP", "TCP-", $fields['proto']) . $fields['tcpflags'];

			// Cleanup Port Output
			if ($fields['proto'] == "ICMP") {
				$srcport = $fields['srcport'];
				$dstport = $fields['dstport'];
			} else {
				$srcport = " :" . $fields['srcport'];
				$dstport = " :" . $fields['dstport'];
			}

			// Don't add Suppress Icon to Country Block Lines
			if (in_array(substr($rule_list[$rulenum]['name'], 0, -3), $continents)) {
				$pfb_query = "Country";
			}

			// Add DNS Resolve and Suppression Icons to External IPs only. GeoIP Code to External IPs only.
			if (in_array($fields['dstip'], $pfb_local) || check_lan_dest($lan_ip,$lan_mask,$fields['dstip'],"32")) {
				// Destination is Gateway/NAT/VIP
				$rule = $rule_list[$rulenum]['name'] . "<br>(" . $rulenum .")";
				$host = $fields['srcip'];

				if (is_ipaddrv4($host)) {
					$country = substr(exec("$pathgeoip -f $pathgeoipdat $host"),23,2);
				} else {
					$country = substr(exec("$pathgeoip6 -f $pathgeoipdat6 $host"),26,2);
				}

				$alert_ip .= "<a href='/pfblockerng/pfblockerng_diag_dns.php?host={$host}' title=\" " . gettext("Resolve host via Rev. DNS lookup");
				$alert_ip .= "\"> <img src=\"/themes/{$g['theme']}/images/icons/icon_log.gif\" width=\"11\" height=\"11\" border=\"0\" ";
				$alert_ip .= "alt=\"Icon Reverse Resolve with DNS\" style=\"cursor: pointer;\"/></a>";

				if ($pfb_query != "Country" && $rtype == "block" && $pfb['supp'] == "on") {
					$supp_ip .= "<input type='image' name='addsuppress[]' onclick=\"hostruleid('{$host}','{$rule_list[$rulenum]['name']}');\" ";
					$supp_ip .= "src=\"../themes/{$g['theme']}/images/icons/icon_plus.gif\" title=\"";
					$supp_ip .= gettext($supp_ip_txt) . "\" border=\"0\" width='11' height='11'>";
				}

				if ($pfb_query != "Country" && $rtype == "block" && $hostlookup == "on") {
					$hostname = getpfbhostname('src', $fields['srcip'], $counter);
				} else {
					$hostname = "";
				}
		
				$src_icons	= $alert_ip . "&nbsp;" . $supp_ip . "&nbsp;";
				$dst_icons	= "";
				$scc		= $country;
				$dcc		= "";
			} else {
				// Outbound
				$rule = $rule_list[$rulenum]['name'] . "<br>(" . $rulenum .")";
				$host = $fields['dstip'];

				if (is_ipaddrv4($host)) {
					$country = substr(exec("$pathgeoip -f $pathgeoipdat $host"),23,2);
				} else {
					$country = substr(exec("$pathgeoip6 -f $pathgeoipdat6 $host"),26,2);
				}

				$alert_ip .= "<a href='/pfblockerng/pfblockerng_diag_dns.php?host={$host}' title=\"" . gettext("Resolve host via Rev. DNS lookup");
				$alert_ip .= "\"> <img src=\"/themes/{$g['theme']}/images/icons/icon_log.gif\" width=\"11\" height=\"11\" border=\"0\" ";
				$alert_ip .= "alt=\"Icon Reverse Resolve with DNS\" style=\"cursor: pointer;\"/></a>";

				if ($pfb_query != "Country" && $rtype == "block" && $pfb['supp'] == "on") {
					$supp_ip .= "<input type='image' name='addsuppress[]' onclick=\"hostruleid('{$host}','{$rule_list[$rulenum]['name']}');\" ";
					$supp_ip .= "src=\"../themes/{$g['theme']}/images/icons/icon_plus.gif\" title=\"";
					$supp_ip .= gettext($supp_ip_txt) . "\" border=\"0\" width='11' height='11'>";
				}

				if ($pfb_query != "Country" && $rtype == "block" && $hostlookup == "on") {
					$hostname = getpfbhostname('dst', $fields['dstip'], $counter);
				} else {
					$hostname = "";
				}

				$src_icons	= "";
				$dst_icons	= $alert_ip . "&nbsp;" . $supp_ip . "&nbsp;";
				$scc		= "";
				$dcc		= $country; 
			}

			# IP Query Grep Exclusion
			$pfb_ex1 = "grep -v 'pfB\_\|\_v6\.txt'";
			$pfb_ex2 = "grep -v 'pfB\_\|/32\|/24\|\_v6\.txt' | grep -m1 '/'";

			// Find List which contains Blocked IP Host
			if ($pfb_query == "Country") {
				# Skip
			} else {
				// Search for exact IP Match
				$host1 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '\'$1\.$2\.$3\.$4\'', $host);
				$pfb_query = exec("grep -Hm1 {$host1} {$pfbfolder}/* | sed -e 's/^.*[a-zA-Z]\///' -e 's/:.*//' -e 's/\..*/ /' | {$pfb_ex1}");
				// Search for IP in /24 CIDR
				if (empty($pfb_query)) {
					$host1 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '\'$1\.$2\.$3\.0/24\'', $host);
					$pfb_query = exec("grep -Hm1 {$host1} {$pfbfolder}/* | sed -e 's/^.*[a-zA-Z]\///' -e 's/\.txt:/ /' | {$pfb_ex1}");
				}
				// Search for First Two IP Octets in CIDR Matches Only. Skip any pfB (Country Lists) or /32,/24 Addresses.
				if (empty($pfb_query)) {
					$host1 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '\'^$1\.$2\.\'', $host);
					$pfb_query = exec("grep -H {$host1} {$pfbfolder}/* | sed -e 's/^.*[a-zA-Z]\///' -e 's/\.txt:/ /' | {$pfb_ex2}");
				}
				// Search for First Two IP Octets in CIDR Matches Only (Subtract 1 from second Octet on each loop).
				// Skip (Country Lists) or /32,/24 Addresses.
				if (empty($pfb_query)) {
					$host1 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '\'^$1\.', $host);
					$host2 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '$2', $host);
					for ($cnt = 1; $cnt <= 5; $cnt++) {
						$host3 = $host2 - $cnt . '\'';
						$pfb_query = exec("grep -H {$host1}{$host3} {$pfbfolder}/* | sed -e 's/^.*[a-zA-Z]\///' -e 's/\.txt:/ /' | {$pfb_ex2}");
						// Break out of loop if found.
						if (!empty($pfb_query))
							$cnt = 6;
					}
				}
				// Search for First Three Octets
				if (empty($pfb_query)) {
					$host1 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '\'^$1\.$2\.$3\.\'', $host);
					$pfb_query = exec("grep -H {$host1} {$pfbfolder}/* | sed -e 's/^.*[a-zA-Z]\///' -e 's/\.txt:/ /' | {$pfb_ex2}");
				}
				// Search for First Two Octets
				if (empty($pfb_query)) {
					$host1 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '\'^$1\.$2\.\'', $host);
					$pfb_query = exec("grep -H {$host1} {$pfbfolder}/* | sed -e 's/^.*[a-zA-Z]\///' -e 's/\.txt:/ /' | {$pfb_ex2}");
				}
				// Report Specific ET IQRisk Details
				if ($pfb['et_header'] && preg_match("/{$et_header}/", $pfb_query)) {
					$host1 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '\'$1\.$2\.$3\.$4\'', $host);
					$pfb_query = exec("grep -Hm1 {$host1} {$pfb['etdir']}/* | sed -e 's/^.*[a-zA-Z]\///' -e 's/:.*//' -e 's/\..*/ /' -e 's/ET_/ET IPrep /' ");
					if (empty($pfb_query)) {
						$host1 = preg_replace("/(\d{1,3})\.(\d{1,3}).(\d{1,3}).(\d{1,3})/", '\'$1.$2.$3.0/24\'', $host);
						$pfb_query = exec("grep -Hm1 {$host1} {$pfbfolder}/* | sed -e 's/^.*[a-zA-Z]\///' -e 's/\.txt:/ /' | {$pfb_ex1}");
					}
				}
				// Default to "No Match" if not found.
				if (empty($pfb_query))
					$pfb_query = "No Match";
			}

			# Split List Column into Two lines.
			unset ($pfb_match);
			if ($pfb_query == "No Match") {
				$pfb_match[1] = "{$pfb_query}";
				$pfb_match[2] = "";
			} else {
				preg_match ("/(.*)\s(.*)/", $pfb_query, $pfb_match);
				if ($pfb_match[1] == "") {
					$pfb_match[1] = "{$pfb_query}";
					$pfb_match[2] = "";
				}
			}

			// Print Alternating Line Shading 
			if ($pfb['pfsenseversion'] > '2.0') {
				$alertRowEvenClass = "listMReven";
				$alertRowOddClass = "listMRodd";
			} else {
				$alertRowEvenClass = "listr";
				$alertRowOddClass = "listr";
			}

			// Collect Details for Repeated Alert Comparison
			$previous_srcip = $fields['srcip'] . $fields['srcport'];
			$previous_dstip = $fields['dstip'] . $fields['dstport'];
			$countrycode = trim($scc . $dcc);

			$alertRowClass = $counter % 2 ? $alertRowEvenClass : $alertRowOddClass;
			echo "<tr class='{$alertRowClass}'>
				<td class='listMRr' align='center'>{$fields['time']}</td>
				<td class='listMRr' align='center'>{$fields['interface']}</td>
				<td class='listMRr' align='center' title='The pfBlockerNG Rule that Blocked this Host.'>{$rule}</td>
				<td class='listMRr' align='center'>{$proto}</td>
				<td nowrap class='listMRr' align='center' style='sorttable_customkey:{$fields['srcip']};' sorttable_customkey='{$fields['srcip']}'>{$src_icons}{$fields['srcip']}{$srcport}<br><small>{$hostname['src']}</small></td>
				<td nowrap class='listMRr' align='center' style='sorttable_customkey:{$fields['dstip']};' sorttable_customkey='{$fields['dstip']}'>{$dst_icons}{$fields['dstip']}{$dstport}<br><small>{$hostname['dst']}</small></td>
				<td class='listMRr' align='center'>{$countrycode}</td>
				<td class='listbg' align='center' title='Country Block Rules cannot be suppressed.\n\nTo allow a particular Country IP, either remove the particular Country or add the Host\nto a Permit Alias in the Firewall Tab.\n\nIf the IP is not listed beside the List, this means that the Block is a /32 entry.\nOnly /32 or /24 CIDR Hosts can be suppressed.\n\nIf (Duplication) Checking is not enabled. You may see /24 and /32 CIDR Blocks for a given blocked Host' style=\"font-size: 10px word-wrap:break-word;\">{$pfb_match[1]}<br>{$pfb_match[2]}</td></tr>";
			$counter++;
			if ($counter > 0 && $rtype == "block") {
				$mycounter = $counter;
			}
		}
	}
}
?>
	</tbody>
	</table>
	</table>
<?php endforeach; ?>	<!--End - Create Three Output Windows 'Deny', 'Permit' and 'Match'-->
</td></tr>
</table>

</div>
</td>

<script type="text/javascript">

// This function stuffs the passed HOST, Table values into hidden Form Fields for postback.
function hostruleid(host,table) {
	document.getElementById("ip").value = host;
	document.getElementById("table").value = table;

	var description = prompt("Please enter Suppression Description");
	document.getElementById("descr").value = description;

	var cidr = prompt("Please enter CIDR [ 32 or 24 CIDR only supported ]","32");
	document.getElementById("cidr").value = cidr;
}

// Auto-Resolve of Alerted Hostnames
function findhostnames(counter) {
	getip = jQuery('#gethostname_' + counter).attr('name');
	geturl = "<?php echo $_SERVER['PHP_SELF']; ?>";
	jQuery.get( geturl, { "getpfhostname": getip } )
	.done(function( data ) {
			jQuery('#gethostname_' + counter).prop('title' , data );
			var str = data;
			if(str.length > 32) str = str.substring(0,29)+"...";
			jQuery('#gethostname_' + counter).html( str );
		}
	)
}

	var lines = <?php echo $mycounter; ?>;
	for (i = 0; i < lines; i++) {
		findhostnames(i);
	}

</script>
<?php include("fend.inc"); ?>
</form>
</body>
</html>