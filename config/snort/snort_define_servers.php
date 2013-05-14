<?php
/*
 * snort_define_servers.php
 * part of pfSense
 *
 * Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 * Copyright (C) 2008-2009 Robert Zelaya.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

//require_once("globals.inc");
require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort.inc");

global $g, $rebuild_rules;

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (is_null($id)) {
        header("Location: /snort/snort_interfaces.php");
        exit;
}

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

/* NOTE: KEEP IN SYNC WITH SNORT.INC since global do not work quite well with package */
/* define servers and ports snortdefservers */
$snort_servers = array (
"dns_servers" => "\$HOME_NET", "smtp_servers" => "\$HOME_NET", "http_servers" => "\$HOME_NET",
"www_servers" => "\$HOME_NET", "sql_servers" => "\$HOME_NET", "telnet_servers" => "\$HOME_NET",
"snmp_servers" => "\$HOME_NET", "ftp_servers" => "\$HOME_NET", "ssh_servers" => "\$HOME_NET",
"pop_servers" => "\$HOME_NET", "imap_servers" => "\$HOME_NET", "sip_proxy_ip" => "\$HOME_NET",
"sip_servers" => "\$HOME_NET", "rpc_servers" => "\$HOME_NET", "dnp3_server" => "\$HOME_NET", 
"dnp3_client" => "\$HOME_NET", "modbus_server" => "\$HOME_NET", "modbus_client" => "\$HOME_NET",
"enip_server" => "\$HOME_NET", "enip_client" => "\$HOME_NET",
"aim_servers" => "64.12.24.0/23,64.12.28.0/23,64.12.161.0/24,64.12.163.0/24,64.12.200.0/24,205.188.3.0/24,205.188.5.0/24,205.188.7.0/24,205.188.9.0/24,205.188.153.0/24,205.188.179.0/24,205.188.248.0/24"
);

/* if user has defined a custom ssh port, use it */
if(is_array($config['system']['ssh']) && isset($config['system']['ssh']['port']))
        $ssh_port = $config['system']['ssh']['port'];
else
        $ssh_port = "22";
$snort_ports = array(
"dns_ports" => "53", "smtp_ports" => "25", "mail_ports" => "25,143,465,691",
"http_ports" => "80", "oracle_ports" => "1521", "mssql_ports" => "1433",
"telnet_ports" => "23","snmp_ports" => "161", "ftp_ports" => "21",
"ssh_ports" => $ssh_port, "pop2_ports" => "109", "pop3_ports" => "110",
"imap_ports" => "143", "sip_proxy_ports" => "5060:5090,16384:32768",
"sip_ports" => "5060,5061", "auth_ports" => "113", "finger_ports" => "79",
"irc_ports" => "6665,6666,6667,6668,6669,7000", "smb_ports" => "139,445",
"nntp_ports" => "119", "rlogin_ports" => "513", "rsh_ports" => "514",
"ssl_ports" => "443,465,563,636,989,990,992,993,994,995", "GTP_PORTS" => "2123,2152,3386",
"file_data_ports" => "\$HTTP_PORTS,110,143", "shellcode_ports" => "!80",
"sun_rpc_ports" => "111,32770,32771,32772,32773,32774,32775,32776,32777,32778,32779",
"DCERPC_NCACN_IP_TCP" => "139,445", "DCERPC_NCADG_IP_UDP" => "138,1024:",
"DCERPC_NCACN_IP_LONG" => "135,139,445,593,1024:", "DCERPC_NCACN_UDP_LONG" => "135,1024:",
"DCERPC_NCACN_UDP_SHORT" => "135,593,1024:", "DCERPC_NCACN_TCP" => "2103,2105,2107",
"DCERPC_BRIGHTSTORE" => "6503,6504", "DNP3_PORTS" => "20000", "MODBUS_PORTS" => "502"
);

$pconfig = $a_nat[$id];

/* convert fake interfaces to real */
$if_real = snort_get_real_interface($pconfig['interface']);
$snort_uuid = $config['installedpackages']['snortglobal']['rule'][$id]['uuid'];

/* alert file */
$d_snortconfdirty_path = "/var/run/snort_conf_{$snort_uuid}_{$if_real}.dirty";

if ($_POST) {

	$natent = array();
	$natent = $pconfig;

	foreach ($snort_servers as $key => $server) {
		if ($_POST["def_{$key}"] && !is_alias($_POST["def_{$key}"]))
			$input_errors[] = "Only aliases are allowed";
	}
	foreach ($snort_ports as $key => $server) {
		if ($_POST["def_{$key}"] && !is_alias($_POST["def_{$key}"]))
			$input_errors[] = "Only aliases are allowed";
	}
	/* if no errors write to conf */
	if (!$input_errors) {
		/* post new options */
		foreach ($snort_servers as $key => $server) {
			if ($_POST["def_{$key}"])
				$natent["def_{$key}"] = $_POST["def_{$key}"];
			else
				unset($natent["def_{$key}"]);
		}
		foreach ($snort_ports as $key => $server) {
			if ($_POST["def_{$key}"])
				$natent["def_{$key}"] = $_POST["def_{$key}"];
			else
				unset($natent["def_{$key}"]);
		}

		$a_nat[$id] = $natent;

		write_config();

		/* Update the snort conf file for this interface. */
		$rebuild_rules = "off";
		snort_generate_conf($a_nat[$id]);

		/* after click go to this page */
		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		header("Location: snort_define_servers.php?id=$id");
		exit;
	}
}

$if_friendly = snort_get_friendly_interface($pconfig['interface']);
$pgtitle = "Snort: Interface {$if_friendly} Define Servers";
include_once("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<?php 
include("fbegin.inc"); 
if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}
/* Display Alert message */
if ($input_errors)
	print_input_errors($input_errors); // TODO: add checks
if ($savemsg)
	print_info_box($savemsg);
?>

<script type="text/javascript" src="/javascript/autosuggest.js">
</script>
<script type="text/javascript" src="/javascript/suggestions.js">
</script>
<form action="snort_define_servers.php" method="post" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
		$tab_array = array();
		$tab_array[0] = array(gettext("Snort Interfaces"), true, "/snort/snort_interfaces.php");
		$tab_array[1] = array(gettext("Global Settings"), false, "/snort/snort_interfaces_global.php");
		$tab_array[2] = array(gettext("Updates"), false, "/snort/snort_download_updates.php");
		$tab_array[3] = array(gettext("Alerts"), false, "/snort/snort_alerts.php");
		$tab_array[4] = array(gettext("Blocked"), false, "/snort/snort_blocked.php");
		$tab_array[5] = array(gettext("Whitelists"), false, "/snort/snort_interfaces_whitelist.php");
		$tab_array[6] = array(gettext("Suppress"), false, "/snort/snort_interfaces_suppress.php");
		$tab_array[7] = array(gettext("Sync"), false, "/pkg_edit.php?xml=snort/snort_sync.xml");
		display_top_tabs($tab_array);
		echo '</td></tr>';
		echo '<tr><td class="tabnavtbl">';
		$menu_iface=($if_friendly?substr($if_friendly,0,5)." ":"Iface ");
        $tab_array = array();
        $tab_array[] = array($menu_iface . gettext(" Settings"), false, "/snort/snort_interfaces_edit.php?id={$id}");
        $tab_array[] = array($menu_iface . gettext("Categories"), false, "/snort/snort_rulesets.php?id={$id}");
        $tab_array[] = array($menu_iface . gettext("Rules"), false, "/snort/snort_rules.php?id={$id}");
        $tab_array[] = array($menu_iface . gettext("Variables"), true, "/snort/snort_define_servers.php?id={$id}");
        $tab_array[] = array($menu_iface . gettext("Preprocessors"), false, "/snort/snort_preprocessors.php?id={$id}");
        $tab_array[] = array($menu_iface . gettext("Barnyard2"), false, "/snort/snort_barnyard.php?id={$id}");
        display_top_tabs($tab_array);
?>
</td></tr>
<tr>
		<td class="tabcont">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Define Servers"); ?></td>
		</tr>
<?php
		foreach ($snort_servers as $key => $server):
			if (strlen($server) > 40)
				$server = substr($server, 0, 40) . "...";
			$label = strtoupper($key);
			$value = "";
			if (!empty($pconfig["def_{$key}"]))
				$value = htmlspecialchars($pconfig["def_{$key}"]);
?>
			<tr>
				<td width='22%' valign='top' class='vncell'><?php echo gettext("Define"); ?> <?=$label;?></td>
				<td width="78%" class="vtable">
					<input name="def_<?=$key;?>" size="40"
					type="text" autocomplete="off" class="formfldalias" id="def_<?=$key;?>"
					value="<?=$value;?>"> <br/>
				<span class="vexpl"><?php echo gettext("Default value:"); ?> "<?=$server;?>" <br/><?php echo gettext("Leave " .
				"blank for default value."); ?></span>
				</td>
			</tr>
<?php		endforeach; ?>
		<tr>
			<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Define Ports"); ?></td>
		</tr>
<?php
		foreach ($snort_ports as $key => $server):
			$server = substr($server, 0, 20);
			$label = strtoupper($key);
			$value = "";
			if (!empty($pconfig["def_{$key}"]))
				$value = htmlspecialchars($pconfig["def_{$key}"]);
?>
			<tr>
				<td width='22%' valign='top' class='vncell'><?php echo gettext("Define"); ?> <?=$label;?></td>
				<td width="78%" class="vtable">
					<input name="def_<?=$key;?>" type="text" size="40" autocomplete="off"  class="formfldalias" id="def_<?=$key;?>"
					value="<?=$value;?>"> <br/>
				<span class="vexpl"><?php echo gettext("Default value:"); ?> "<?=$server;?>" <br/> <?php echo gettext("Leave " .
				"blank for default value."); ?></span>
				</td>
			</tr>
<?php		endforeach; ?>
		<tr>
			<td width="22%" valign="top">&nbsp;</td>
			<td width="78%">
				<input name="Submit" type="submit" class="formbtn" value="Save">
				<input name="id" type="hidden" value="<?=$id;?>">
			</td>
		</tr>
	</table>
</td></tr>
</table>
</form>
<script type="text/javascript">
<?php
        $isfirst = 0;
        $aliases = "";
        $addrisfirst = 0;
        $portisfirst = 0;
        $aliasesaddr = "";
        $aliasesports = "";
        if(isset($config['aliases']['alias']) && is_array($config['aliases']['alias']))
                foreach($config['aliases']['alias'] as $alias_name) {
                        if ($alias_name['type'] == "host" || $alias_name['type'] == "network") {
				if($addrisfirst == 1) $aliasesaddr .= ",";
				$aliasesaddr .= "'" . $alias_name['name'] . "'";
				$addrisfirst = 1;
			} else if ($alias_name['type'] == "port") {
				if($portisfirst == 1) $aliasesports .= ",";
				$aliasesports .= "'" . $alias_name['name'] . "'";
				$portisfirst = 1;
			}
                }
?>

        var addressarray=new Array(<?php echo $aliasesaddr; ?>);
        var portsarray=new Array(<?php echo $aliasesports; ?>);

function createAutoSuggest() {
<?php
	foreach ($snort_servers as $key => $server)
		echo "objAlias{$key} = new AutoSuggestControl(document.getElementById('def_{$key}'), new StateSuggestions(addressarray));\n";
	foreach ($snort_ports as $key => $server)
		echo "pobjAlias{$key} = new AutoSuggestControl(document.getElementById('def_{$key}'), new StateSuggestions(portsarray));\n";
?>
}

setTimeout("createAutoSuggest();", 500);

</script>

<?php include("fend.inc"); ?>
</body>
</html>
