<?php
/* $Id$ */
/*
	snort_interfaces.php
	part of m0n0wall (http://m0n0.ch/wall)

	Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
	Copyright (C) 2008-2009 Robert Zelaya.
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

require("guiconfig.inc");
include_once("/usr/local/pkg/snort/snort.inc");

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}
//nat_rules_sort();
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

if (isset($_GET['dup'])) {
        $id = $_GET['dup'];
        $after = $_GET['dup'];
}

if (isset($id) && $a_nat[$id]) {

	$pconfig['enable'] = $a_nat[$id]['enable'];
	$pconfig['interface'] = $a_nat[$id]['interface'];
	$pconfig['descr'] = $a_nat[$id]['descr'];
	$pconfig['performance'] = $a_nat[$id]['performance'];
	$pconfig['blockoffenders7'] = $a_nat[$id]['blockoffenders7'];
	$pconfig['snortalertlogtype'] = $a_nat[$id]['snortalertlogtype'];
	$pconfig['alertsystemlog'] = $a_nat[$id]['alertsystemlog'];
	$pconfig['tcpdumplog'] = $a_nat[$id]['tcpdumplog'];
	$pconfig['snortunifiedlog'] = $a_nat[$id]['snortunifiedlog'];
	$pconfig['flow_depth'] = $a_nat[$id]['flow_depth'];
	$pconfig['barnyard_enable'] = $a_nat[$id]['barnyard_enable'];
	$pconfig['barnyard_mysql'] = $a_nat[$id]['barnyard_mysql'];
		
	if (!$pconfig['interface'])
		$pconfig['interface'] = "wan";
} else {
	$pconfig['interface'] = "wan";
}

if (isset($_GET['dup']))
	unset($id);
	
/* convert fake interfaces to real */
$if_real = convert_friendly_interface_to_real_interface_name($pconfig['interface']);

if ($_POST) {

	/* input validation */
//	if(strtoupper($_POST['proto']) == "TCP" or strtoupper($_POST['proto']) == "UDP" or strtoupper($_POST['proto']) == "TCP/UDP") {
//		$reqdfields = explode(" ", "interface proto beginport endport localip localbeginport");
//		$reqdfieldsn = explode(",", "Interface,Protocol,External port from,External port to,NAT IP,Local port");
//	} else {
//		$reqdfields = explode(" ", "interface proto localip");
//		$reqdfieldsn = explode(",", "Interface,Protocol,NAT IP");
//	}

//	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

//	if (($_POST['localip'] && !is_ipaddroralias($_POST['localip']))) {
//		$input_errors[] = "\"{$_POST['localip']}\" is not valid NAT IP address or host alias.";
//	}

	/* only validate the ports if the protocol is TCP, UDP or TCP/UDP */
//	if(strtoupper($_POST['proto']) == "TCP" or strtoupper($_POST['proto']) == "UDP" or strtoupper($_POST['proto']) == "TCP/UDP") {

//		if (($_POST['beginport'] && !is_ipaddroralias($_POST['beginport']) && !is_port($_POST['beginport']))) {
//			$input_errors[] = "The start port must be an integer between 1 and 65535.";
//		}

//		if (($_POST['endport'] && !is_ipaddroralias($_POST['endport']) && !is_port($_POST['endport']))) {
//			$input_errors[] = "The end port must be an integer between 1 and 65535.";
//		}

//		if (($_POST['localbeginport'] && !is_ipaddroralias($_POST['localbeginport']) && !is_port($_POST['localbeginport']))) {
//			$input_errors[] = "The local port must be an integer between 1 and 65535.";
//		}

//		if ($_POST['beginport'] > $_POST['endport']) {
			/* swap */
//			$tmp = $_POST['endport'];
//			$_POST['endport'] = $_POST['beginport'];
//			$_POST['beginport'] = $tmp;
//		}

//		if (!$input_errors) {
//			if (($_POST['endport'] - $_POST['beginport'] + $_POST['localbeginport']) > 65535)
//				$input_errors[] = "The target port range must be an integer between 1 and 65535.";
//		}

//	}

	/* check for overlaps */
	foreach ($a_nat as $natent) {
		if (isset($id) && ($a_nat[$id]) && ($a_nat[$id] === $natent))
			continue;
		if ($natent['interface'] != $_POST['interface'])
			continue;
	}

/* if no errors write to conf */
	if (!$input_errors) {
		$natent = array();

		/* write to conf for 1st time or rewrite the answer */
		$natent['interface'] = $_POST['interface'] ? $_POST['interface'] : $pconfig['interface'];
		/* if post write to conf or rewite the answer */
		$natent['enable'] = $_POST['enable'] ? on : off;
		$natent['descr'] = $_POST['descr'] ? $_POST['descr'] : $pconfig['descr'];
		$natent['performance'] = $_POST['performance'] ? $_POST['performance'] : $pconfig['performance'];
		/* if post = on use on off or rewrite the conf */
		if ($_POST['blockoffenders7'] == "on") { $natent['blockoffenders7'] = on; }else{ $natent['blockoffenders7'] = off; } if ($_POST['enable'] == "") { $natent['blockoffenders7'] = $pconfig['blockoffenders7']; }
		$natent['snortalertlogtype'] = $_POST['snortalertlogtype'] ? $_POST['snortalertlogtype'] : $pconfig['snortalertlogtype'];
		if ($_POST['alertsystemlog'] == "on") { $natent['alertsystemlog'] = on; }else{ $natent['alertsystemlog'] = off; } if ($_POST['enable'] == "") { $natent['alertsystemlog'] = $pconfig['alertsystemlog']; }
		if ($_POST['tcpdumplog'] == "on") { $natent['tcpdumplog'] = on; }else{ $natent['tcpdumplog'] = off; } if ($_POST['enable'] == "") { $natent['tcpdumplog'] = $pconfig['tcpdumplog']; }
		if ($_POST['snortunifiedlog'] == "on") { $natent['snortunifiedlog'] = on; }else{ $natent['snortunifiedlog'] = off; } if ($_POST['enable'] == "") { $natent['snortunifiedlog'] = $pconfig['snortunifiedlog']; }
		/* if optiion = 0 then the old descr way will not work */
		if ($_POST['flow_depth'] != "") { $natent['flow_depth'] = $_POST['flow_depth']; }else{ $natent['flow_depth'] = $pconfig['flow_depth']; }
		/* rewrite the options that are not in post */
		$natent['barnyard_enable'] = $pconfig['barnyard_enable'];
		$natent['barnyard_mysql'] = $pconfig['barnyard_mysql'];

		if (isset($id) && $a_nat[$id])
			$a_nat[$id] = $natent;
		else {
			if (is_numeric($after))
				array_splice($a_nat, $after+1, 0, array($natent));
			else
				$a_nat[] = $natent;
		}

		touch($d_natconfdirty_path);

		write_config();
		// stop_service("snort");
		//create_snort_conf();
		//create_barnyard2_conf();
		sync_package_snort();
		// sleep(2);
		// start_service("snort");

		header("Location: snort_interfaces.php");
		exit;
	}
}

$pgtitle = "Snort: Interface: $id$if_real Settings Edit";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php 
include("fbegin.inc");
?>
<style type="text/css">
.alert {
 position:absolute;
 top:10px;
 left:0px;
 width:94%;
background:#FCE9C0;
background-position: 15px; 
border-top:2px solid #DBAC48;
border-bottom:2px solid #DBAC48;
padding: 15px 10px 85% 50px;
}
</style> 
<noscript><div class="alert" ALIGN=CENTER><img src="/themes/nervecenter/images/icons/icon_alert.gif"/><strong>Please enable JavaScript to view this content</CENTER></div></noscript>
<script language="JavaScript">
<!--

function enable_change(enable_change) {
	endis = !(document.iform.enable.checked || enable_change);
	// make shure a default answer is called if this is envoked.
	endis2 = (document.iform.enable);

<?php
/* make shure all the settings exist or function hide will not work */
/* if $id is emty allow if and discr to be open */
if($id != "") 
{
echo "	
	document.iform.interface.disabled = endis2;
	document.iform.descr.disabled = endis;\n";
}
?>
    document.iform.flow_depth.disabled = endis;
	document.iform.performance.disabled = endis;
	document.iform.blockoffenders7.disabled = endis;
	document.iform.snortalertlogtype.disabled = endis;
	document.iform.alertsystemlog.disabled = endis;
	document.iform.tcpdumplog.disabled = endis;
	document.iform.snortunifiedlog.disabled = endis;
}
//-->
</script>
<p class="pgtitle"><?=$pgtitle?></p>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<form action="snort_interfaces_edit.php" method="post" enctype="multipart/form-data" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
<?php
if($id != "") 
{

	 /* get the interface name */
		$first = 0;
        $snortInterfaces = array(); /* -gtm  */

        $if_list = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
        $if_array = split(',', $if_list);
        //print_r($if_array);
        if($if_array) {
                foreach($if_array as $iface2) {
                        $if2 = convert_friendly_interface_to_real_interface_name($iface2);

                        if($config['interfaces'][$iface2]['ipaddr'] == "pppoe") {
                                $if2 = "ng0";
                        }

                        /* build a list of user specified interfaces -gtm */
                        if($if2){
                          array_push($snortInterfaces, $if2);
                          $first = 1;
                        }
                }

                if (count($snortInterfaces) < 1) {
                        log_error("Snort will not start.  You must select an interface for it to listen on.");
                        return;
                }
        }
		
		/* do for the selected interface */
		foreach($snortInterfaces as $snortIf)
		{
		
		/* if base directories dont exist create them */
		if(!file_exists("/usr/local/pkg/snort/snort_{$snortIf}_{$id}/"))
			{
			exec("/bin/mkdir -p /usr/local/pkg/snort/snort_{$snortIf}_{$id}/");
			if(!file_exists("/usr/local/www/snort/snort_{$snortIf}_{$id}/"))
			exec("/bin/mkdir -p /usr/local/www/snort/snort_{$snortIf}_{$id}/");
			}

    $tab_array = array();
    $tab_array[] = array("Snort Interfaces", false, "/snort/snort_interfaces.php");
    $tab_array[] = array("If Settings", true, "/snort/snort_interfaces_edit.php?id={$id}");
    $tab_array[] = array("Categories", false, "/snort/snort_rulesets.php?id={$id}");
    $tab_array[] = array("Rules", false, "/snort/snort_rules.php?id={$id}");
    $tab_array[] = array("Servers", false, "/snort/snort_define_servers.php?id={$id}");
    $tab_array[] = array("Preprocessors", false, "/snort/snort_preprocessors.php?id={$id}");
    $tab_array[] = array("Barnyard2", false, "/snort/snort_barnyard.php?id={$id}");
    display_top_tabs($tab_array);	
		}
}
?>
</td>
</tr>
				<tr>
				<td class="tabcont">
				<table width="100%" border="0" cellpadding="6" cellspacing="0">
				<?php
				if($id == "") 
				{
				echo "
				<tr>
				<td width=\"22%\" valign=\"top\">&nbsp;</td>
				<td width=\"78%\"><span class=\"vexpl\"><span class=\"red\"><strong>Note:</strong></span><br>
					You will be redirected to the Snort Interfaces Menu to aprove changes.<br>
					After approval, interface options will be made available.
					<br><br>
					Please select a interface and a description.
				</td>
				</tr>\n";
				}
				?>
				<tr>
				<td width="22%" valign="top" class="vtable">&nbsp;</td>
				<td width="78%" class="vtable">
					<?php
					// <input name="enable" type="checkbox" value="yes" checked onClick="enable_change(false)">
					// care with spaces
					if ($pconfig['enable'] == "on")
					$checked = checked;
					if($id != "") 
					{
					$onclick_enable = "onClick=\"enable_change(false)\">";
					}
					echo "
					<input name=\"enable\" type=\"checkbox\" value=\"on\" $checked $onclick_enable
					<strong>Enable Interface</strong></td>\n\n";
					?>
				</tr>
				<tr>
                  <td width="22%" valign="top" class="vncellreq">Interface</td>
                  <td width="78%" class="vtable">
					<select name="interface" class="formfld">
						<?php
						$interfaces = array('wan' => 'WAN', 'lan' => 'LAN', 'pptp' => 'PPTP', 'pppoe' => 'PPPOE');
						for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++) {
							$interfaces['opt' . $i] = $config['interfaces']['opt' . $i]['descr'];
						}
						foreach ($interfaces as $iface => $ifacename): ?>
						<option value="<?=$iface;?>" <?php if ($iface == $pconfig['interface']) echo "selected"; ?>>
						<?=htmlspecialchars($ifacename);?>
						</option>
						<?php endforeach; ?>
					</select><br>
                     <span class="vexpl">Choose which interface this rule applies to.<br>
                     Hint: in most cases, you'll want to use WAN here.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncellreq">Description</td>
                  <td width="78%" class="vtable">
                    <input name="descr" type="text" class="formfld" id="descr" size="40" value="<?=htmlspecialchars($pconfig['descr']);?>">
                    <br> <span class="vexpl">You may enter a description here
                    for your reference (not parsed).</span></td>
                </tr>
				<tr>
					<td width="22%" valign="top" class="vncell">Memory Performance</td>
					<td width="78%" class="vtable">
					<select name="performance" class="formfld" id="performance">
						<?php
							$interfaces2 = array('ac-bnfa' => 'AC-BNFA', 'lowmem' => 'LOWMEM', 'ac-std' => 'AC-STD', 'ac' => 'AC', 'ac-banded' => 'AC-BANDED', 'ac-sparsebands' => 'AC-SPARSEBANDS', 'acs' => 'ACS');
							foreach ($interfaces2 as $iface2 => $ifacename2): ?>
							<option value="<?=$iface2;?>" <?php if ($iface2 == $pconfig['performance']) echo "selected"; ?>>
							<?=htmlspecialchars($ifacename2);?>
							</option>
						<?php endforeach; ?>
					</select><br>
					<span class="vexpl">Lowmem and ac-bnfa are recommended for low end systems, Ac: high memory, best performance, ac-std: moderate memory,high performance, acs: small memory, moderateperformance, ac-banded: small memory,moderate performance, ac-sparsebands: small memory, high performance.<br>
					Hint: in most cases, you'll want to use WAN here.</span></td>
				</tr>
				<tr>
				<td width="22%" valign="top" class="vncell">Block offenders</td>
				<td width="78%" class="vtable">
					<input name="blockoffenders7" type="checkbox" value="on" <?php if ($pconfig['blockoffenders7'] == "on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Checking this option will automatically block hosts that generate a snort alert.</td>
				</tr>
				<tr>
				<td width="22%" valign="top" class="vncell">Alerts Tab description type</td>
				<td width="78%" class="vtable">
					<select name="snortalertlogtype" class="formfld" id="snortalertlogtype">
						<?php
							$interfaces4 = array('fast' => 'SHORT', 'full' => 'FULL');
							foreach ($interfaces4 as $iface4 => $ifacename4): ?>
							<option value="<?=$iface4;?>" <?php if ($iface4 == $pconfig['snortalertlogtype']) echo "selected"; ?>>
							<?=htmlspecialchars($ifacename4);?>
							</option>
						<?php endforeach; ?>
					</select><br>
					<span class="vexpl">Please choose the type of Alert logging you will like see in the Alerts Tab.<br>
					Hint: in most cases, short descriptions are best.</span></td>
				</tr>
				<tr>
				<td width="22%" valign="top" class="vncell">Send alerts to main System logs</td>
				<td width="78%" class="vtable">
					<input name="alertsystemlog" type="checkbox" value="on" <?php if ($pconfig['alertsystemlog'] == "on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Snort will send Alerts to the Pfsense system logs.</td>
				</tr>
				<tr>
				<td width="22%" valign="top" class="vncell">Log to a Tcpdump file</td>
				<td width="78%" class="vtable">
					<input name="tcpdumplog" type="checkbox" value="on" <?php if ($pconfig['tcpdumplog'] == "on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Snort will log packets to a tcpdump-formatted file. The file then can be analyzed by a wireshark type of application. WARNING: File may become large.</td>
				</tr>
				<tr>
				<td width="22%" valign="top" class="vncell">Log Alerts to a snort unified2 file</td>
				<td width="78%" class="vtable">
					<input name="snortunifiedlog" type="checkbox" value="on" <?php if ($pconfig['snortunifiedlog'] == "on") echo "checked"; ?> onClick="enable_change(false)"><br>
					Snort will log Alerts to a file in the UNIFIED2 format. This is a requirement for barnyard2.</td>
				</tr>
				<tr>
				<td valign="top" class="vncell">HTTP server flow depth</td>
				<td class="vtable">
					<table cellpadding="0" cellspacing="0">
					<tr>
                    <td><input name="flow_depth" type="text" class="formfld" id="flow_depth" size="5" value="<?=htmlspecialchars($pconfig['flow_depth']);?>"> <strong>-1</strong> to <strong>1460</strong> (<strong>-1</strong> disables HTTP inspect, <strong>0</strong> enables all HTTP inspect)</td>
					</tr>
					</table>
						Amount of HTTP server response payload to inspect. Snort's performance may increase by ajusting this value.<br>
						Setting this value too low may cause false negatives. Value above 0 is in bytes.<br>
						<strong>Default value is 0</strong></td>
				</tr>
                <tr>
                  <td width="22%" valign="top">&nbsp;</td>
                  <td width="78%">
                    <input name="Submit" type="submit" class="formbtn" value="Save"> <input name="Submit2" type="submit" class="formbtn" value="Start" onClick="enable_change(true)"> <input type="button" class="formbtn" value="Cancel" onclick="history.back()">
                    <?php if (isset($id) && $a_nat[$id]): ?>
                    <input name="id" type="hidden" value="<?=$id;?>">
                    <?php endif; ?>
                  </td>
                </tr>
	<tr>
	  <td width="22%" valign="top">&nbsp;</td>
	  <td width="78%"><span class="vexpl"><span class="red"><strong>Note:</strong></span>
	  <br>
		Please save your settings befor you click start. </td>
	</tr>
  </table>
  </table>
</form>

<script language="JavaScript">
<!--
enable_change(false);
//-->
</script>
<?php include("fend.inc"); ?>
</body>
</html>
