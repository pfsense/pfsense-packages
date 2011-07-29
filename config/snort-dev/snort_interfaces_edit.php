<?php
/* $Id$ */
/*

 part of pfSense
 All rights reserved.

 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 All rights reserved.

 Pfsense Old snort GUI 
 Copyright (C) 2006 Scott Ullrich.
 
 Pfsense snort GUI 
 Copyright (C) 2008-2012 Robert Zelaya.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. Neither the name of the pfSense nor the names of its contributors 
 may be used to endorse or promote products derived from this software without 
 specific prior written permission.

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

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_new.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");

// set page vars

$uuid = $_GET['uuid'];
if (isset($_POST['uuid']))
$uuid = $_POST['uuid'];

if ($uuid == '') {
	echo 'error: no uuid';
	exit(0);
}



$a_list = snortSql_fetchAllSettings('snortDB', 'SnortIfaces', 'uuid', $uuid);

$a_rules = snortSql_fetchAllSettings('snortDBrules', 'Snortrules', 'All', '');

if (!is_array($a_list)) {
	$a_list = array();
}

$a_whitelist = snortSql_fetchAllWhitelistTypes('SnortWhitelist', 'SnortWhitelistips');

if (!is_array($a_whitelist)) {
	$a_whitelist = array();
}
	
$a_suppresslist = snortSql_fetchAllWhitelistTypes('SnortSuppress', '');

if (!is_array($a_suppresslist)) {
	$a_suppresslist = array();
}	
	

	$pgtitle = "Services: Snort: Interface Edit:";
	include("/usr/local/pkg/snort/snort_head.inc");

?>

<!-- START page custom script -->
<script language="JavaScript">

// start a jQuery sand box
jQuery(document).ready(function() { 

	// misc call after a good save
	jQuery.fn.miscTabCall = function () {
		jQuery('.hide_newtabmenu').show();
		jQuery('#interface').attr("disabled", true);
	};	

	// START disable option for snort_interfaces_edit.php
	endis = !(jQuery('input[name=enable]:checked').val());
		
	disableInputs=new Array(
			"descr",
			"performance",
			"blockoffenders7",
			"alertsystemlog",
			"externallistname",
			"homelistname",
			"suppresslistname",
			"tcpdumplog",
			"snortunifiedlog",
			"configpassthru"
			);
	<?php 
	
	if ($a_list['interface'] != '') {
		echo '	
			jQuery(\'[name=interface]\').attr(\'disabled\', \'true\');
		';
	}
	
	// disable tabs if nothing in database
	if ($a_list['uuid'] == '') {
		echo '
			jQuery(\'.hide_newtabmenu\').hide();	
		';
	}	
	
	?>
	
	if (endis) {
		for (var i = 0; i < disableInputs.length; i++)
		{
		jQuery('[name=' + disableInputs[i] + ']').attr('disabled', 'true');
		}
	}

	jQuery("input[name=enable]").live('click', function() {

		endis = !(jQuery('input[name=enable]:checked').val());

		if (endis) {
			for (var i = 0; i < disableInputs.length; i++)
			{
			jQuery('[name=' + disableInputs[i] + ']').attr('disabled', 'true');
			}
		}else{
			for (var i = 0; i < disableInputs.length; i++)
			{
			jQuery('[name=' + disableInputs[i] + ']').removeAttr('disabled');
			}
		}

		
	}); 
	// STOP disable option for snort_interfaces_edit.php
	
	
}); // end of on ready

</script>


<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<!-- loading msg -->
<div id="loadingWaiting">
	<div class="snortModal" style="top: 200px; left: 700px;">
		<div class="snortModalTop">
			<!-- <div class="snortModalTopClose"><a href="javascript:hideLoading('#loadingWaiting');"><img src="/snort/images/close_9x9.gif" border="0" height="9" width="9"></a></div> -->
		</div>
		<div class="snortModalTitle">
	  		<p><img src="./images/loading.gif" /><br><br>Please Wait...</p>
	  	</div>
		<div>
		<p class="loadingWaitingMessage"></p>
	  	</div>
	</div>  
</div>

<?php include("fbegin.inc"); ?>
<!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2">
<a href="../index.php" id="status-link2">
<img src="./images/transparent.gif" border="0"></img>
</a>
</div>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>
			<div class="newtabmenu" style="margin: 1px 0px; width: 790px;"><!-- Tabbed bar code-->
			<ul class="newtabmenu">			
				<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
				<li class="newtabmenu_active"><a href="/snort/snort_interfaces_edit.php?uuid=<?=$uuid;?>"><span>If Settings</span></a></li>
				<li class="hide_newtabmenu"><a href="/snort/snort_rulesets.php?uuid=<?=$uuid;?>"><span>Categories</span></a></li>
				<li class="hide_newtabmenu"><a href="/snort/snort_rules.php?uuid=<?=$uuid;?>"><span>Rules</span></a></li>
				<li class="hide_newtabmenu"><a href="/snort/snort_rulesets_ips.php?uuid=<?=$uuid;?>"><span>Ruleset Ips</span></a></li>
				<li class="hide_newtabmenu"><a href="/snort/snort_define_servers.php?uuid=<?=$uuid;?>"><span>Servers</span></a></li>
				<li class="hide_newtabmenu"><a href="/snort/snort_preprocessors.php?uuid=<?=$uuid;?>"><span>Preprocessors</span></a></li>
				<li class="hide_newtabmenu"><a href="/snort/snort_barnyard.php?uuid=<?=$uuid;?>"><span>Barnyard2</span></a></li>			
			</ul>		
			</div>
		</td>
	</tr>
	<tr>
		<td id="tdbggrey">		
		<table width="100%" border="0" cellpadding="10px" cellspacing="0">
		<tr>
		<td class="tabnavtbl">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<!-- START MAIN AREA -->
		
		<form id="iform" name="iform" >
		<input type="hidden" name="snortSaveSettings" value="1" /> <!-- what to do, save -->
		<input type="hidden" name="dbName" value="snortDB" /> <!-- what db-->
		<input type="hidden" name="dbTable" value="SnortIfaces" /> <!-- what db table-->
		<input type="hidden" name="ifaceTab" value="snort_interfaces_edit" /> <!-- what interface tab -->
		<input name="uuid" type="hidden" value="<?=$uuid; ?>" > 

		<table width="100%" border="0" cellpadding="6" cellspacing="0">
			<tr>
				<td colspan="2" valign="top" class="listtopic">General Settings</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncellreq2">Interface</td>
				<td width="22%" valign="top" class="vtable">
					&nbsp; 
					<input name="enable" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['enable'] == 'on' || $a_list['enable'] == '' ? 'checked' : '';?> ">
					&nbsp;&nbsp;<span class="vexpl">Enable or Disable</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncellreq2">Interface</td>
				<td width="78%" class="vtable">
					<select id="interface" name="interface" class="formfld">
						
				<?php 					
					/* add group interfaces */
					/* needs to be watched, dont know if new interfces will work */
					if (is_array($config['ifgroups']['ifgroupentry']))
						foreach($config['ifgroups']['ifgroupentry'] as $ifgen)
							if (have_ruleint_access($ifgen['ifname']))
								$interfaces[$ifgen['ifname']] = $ifgen['ifname'];
					$ifdescs = get_configured_interface_with_descr();
					foreach ($ifdescs as $ifent => $ifdesc)
	        				if(have_ruleint_access($ifent))
								$interfaces[$ifent] = $ifdesc;
						if ($config['l2tp']['mode'] == "server")
							if(have_ruleint_access("l2tp"))
								$interfaces['l2tp'] = "L2TP VPN";
						if ($config['pptpd']['mode'] == "server")
							if(have_ruleint_access("pptp")) 
								$interfaces['pptp'] = "PPTP VPN";
						
						if (is_pppoe_server_enabled() && have_ruleint_access("pppoe"))
							$interfaces['pppoe'] = "PPPoE VPN";
						/* add ipsec interfaces */
						if (isset($config['ipsec']['enable']) || isset($config['ipsec']['client']['enable']))
							if(have_ruleint_access("enc0")) 
								$interfaces["enc0"] = "IPsec";
						/* add openvpn/tun interfaces */
						if  ($config['openvpn']["openvpn-server"] || $config['openvpn']["openvpn-client"])
							$interfaces["openvpn"] = "OpenVPN";
						$selected_interfaces = explode(",", $pconfig['interface']);
						foreach ($interfaces as $iface => $ifacename)
						{ 
							echo "\n" . "<option value=\"$iface\"";  
							if ($a_list['interface'] == strtolower($ifacename)){echo " selected ";} 
							echo '>' . $ifacename . '</option>' . "\r";
						}
					?>						
					</select>
					<br>
					<span class="vexpl">Choose which interface this rule applies to.<br>
					Hint: in most cases, you'll want to use WAN here.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncellreq2">Description</td>
				<td width="78%" class="vtable">
					<input name="descr" type="text" class="formfld" id="descr" size="40" value="<?=$a_list['descr']?>"> 
					<br>
					<span class="vexpl">You may enter a description here for your reference (not parsed).</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Memory Performance</td>
				<td width="78%" class="vtable">
					<select name="performance" class="formfld" id="performance">
	
					<?php 					
					$memoryPerfList = array('ac-bnfa' => 'AC-BNFA', 'lowmem' => 'LOWMEM', 'aclowmem-std' => 'AC-STD', 'ac' => 'AC', 'ac-banded' => 'AC-BANDED', 'ac-sparsebands' => 'AC-SPARSEBANDS', 'acs' => 'ACS');					
					snortDropDownList($memoryPerfList, $a_list['performance']);					
					?>						
						
					</select>
					<br>
					<span class="vexpl">Lowmem and ac-bnfa are recommended for low end systems, Ac: high memory, best performance, ac-std: moderate
					memory,high performance, acs: small memory, moderateperformance, ac-banded: small memory,moderate performance, ac-sparsebands: small memory, high performance.</span>
					<br>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Choose the rule DB snort should use.</td>
			</tr>
			
			<tr>
				<td width="22%" valign="top" class="vncell2">Rule DB</td>
				<td width="78%" class="vtable">
					<select name="ruledbname" class="formfld" id="ruledbname">
						
					<?php
					// find ruleDB names and value by uuid
						$selected = '';
						if ($a_list['ruledbname'] == 'default') {
							$selected = 'selected';
						}
						echo  "\n" . '<option value="default" ' . $selected . ' >DEFAULT</option>' . "\r";		
						foreach ($a_rules as $value)
						{
							$selected = '';
							if ($value['uuid'] == $a_list['ruledbname']) {
								$selected = 'selected';
							}
								
							echo "\n" . '<option value="' . $value['uuid'] . '" ' .  $selected . ' >' . strtoupper($value['ruledbname']) . '</option>' . "\r";
						}
					?>
						
					</select>
					<br>
					<span class="vexpl">Choose the rule database to use. &nbsp;<span class="red">Note:</span>&nbsp;Cahnges to this database are global.
					<br>
					<span class="red">WARNING:</span>&nbsp;Never change this when snort is running.</span>
				</td>
			</tr>			
					
			<tr>
				<td colspan="2" valign="top" class="listtopic">Choose the networks snort should inspect and whitelist.</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Home net</td>
				<td width="78%" class="vtable">
					<select name="homelistname" class="formfld" id="homelistname">
						
					<?php
					/* find homelist names and filter by type */
						$selected = '';
						if ($a_list['homelistname'] == 'default'){$selected = 'selected';}
						echo  "\n" . '<option value="default" ' . $selected . ' >DEFAULT</option>' . "\r";		
						foreach ($a_whitelist as $value)
						{
							$selected = '';
							if ($value['filename'] == $a_list['homelistname']){$selected = 'selected';};
							if ($value['snortlisttype'] == 'netlist') // filter
							{
								
								echo "\n" . '<option value="' . $value['filename'] . '" ' .  $selected . ' >' . strtoupper($value['filename']) . '</option>' . "\r";
	
							}
						}
					?>
						
					</select>
					<br>
					<span class="vexpl">Choose the home net you will like this rule to use. &nbsp;<span class="red">Note:</span>&nbsp;Default homenet adds only local networks.
					<br>
					<span class="red">Hint:</span>&nbsp;Most users add a list offriendly ips that the firewall cant see.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">External net</td>
				<td width="78%" class="vtable">
					<select name="externallistname" class="formfld" id="externallistname">
						
					<?php
					/* find externallist names and filter by type */
						$selected = '';
						if ($a_list['externallistname'] == 'default'){$selected = 'selected';}
						echo  "\n" . '<option value="default" ' . $selected . ' >DEFAULT</option>' . "\r";		
						foreach ($a_whitelist as $value)
						{
							$selected = '';
							if ($value['filename'] == $a_list['externallistname']){$selected = 'selected';}
							if ($value['snortlisttype'] == 'netlist') // filter
							{
								
								echo "\n" . '<option value="' . $value['filename'] . '" ' .  $selected . ' >' . strtoupper($value['filename']) . '</option>' . "\r";
	
							}
						}
					?>																		
						
					</select>
					<br>
					<span class="vexpl">Choose the external net you will like this rule to use.&nbsp;<span class="red">Note:</span>&nbsp;Default external net, networks that are not home net.
					<br>
					<span class="red">Hint:</span>&nbsp;Most users should leave this setting at default.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Block offenders</td>
				<td width="78%" class="vtable">
					<input name="blockoffenders7" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['blockoffenders7'] == 'on' ? 'checked' : '';?> >
					<br>
					<span class="vexpl">Checking this option will automatically block hosts that generate a Snort alerts with SnortSam.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Suppression and filtering</td>
				<td width="78%" class="vtable">
					<select name="suppresslistname" class="formfld" id="suppresslistname">
						
					<?php
					/* find suppresslist names and filter by type */
						$selected = '';
						if ($a_list['suppresslistname'] == 'default'){$selected = 'selected';}
						
						echo  "\n" . '<option value="default" ' . $selected . ' >DEFAULT</option>' . "\r";
							
						foreach ($a_suppresslist as $value)
						{
							$selected = '';
							if ($value['filename'] == $a_list['suppresslistname']){$selected = 'selected';}
						
							echo "\n" . '<option value="' . $value['filename'] . '" ' .  $selected . ' >' . strtoupper($value['filename']) . '</option>' . "\r";
						}
					?>						
						
					</select>
					<br>
					<span class="vexpl">Choose the suppression or filtering file you will like this rule to use.&nbsp;<span class="red">
					Note:</span>&nbsp;Default option disables suppression and filtering.</span>
				</td>
			</tr>			
			<tr>
				<td colspan="2" valign="top" class="listtopic">Choose the types of logs snort should create.</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Type of Unified Logging</td>
				<td width="78%" class="vtable">
					<select name="snortalertlogtype" class="formfld" id="snortalertlogtype">
	
					<?php 					
					$snortalertlogtypePerfList = array('full' => 'FULL', 'fast' => 'FAST', 'disable' => 'DISABLE');					
					snortDropDownList($snortalertlogtypePerfList, $a_list['snortalertlogtype']);					
					?>						
						
					</select>
					<br>
					<span class="vexpl">Snort will log Alerts to a file in the UNIFIED format. Full is a requirement for the snort wigdet.</span>
				</td>
			</tr>			
			<tr>
				<td width="22%" valign="top" class="vncell2">Send alerts to mainSystem logs</td>
				<td width="78%" class="vtable">
					<input name="alertsystemlog" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['alertsystemlog'] == 'on' ? 'checked' : '';?> >
					<br>
					<span class="vexpl">Snort will send Alerts to the Pfsense system logs.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Log to a Tcpdump file</td>
				<td width="78%" class="vtable">
					<input name="tcpdumplog" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['tcpdumplog'] == 'on' ? 'checked' : '';?> >
					<br>
					<span class="vexpl">Snort will log packets to a tcpdump-formatted file. The file then can be analyzed by an application such as Wireshark which understands pcap file formats. 
					<span class="red"><strong>WARNING:</strong></span> File may become large.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Log Alerts to a snort unified2 file</td>
				<td width="78%" class="vtable">
					<input name="snortunifiedlog" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['snortunifiedlog'] == 'on' ? 'checked' : '';?> >
					<br>
					<span class="vexpl">Snort will log Alerts to a file in the UNIFIED2 format. This is a requirement for barnyard2.</span>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Arguments here will be automatically inserted into the snort configuration.</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Advanced configuration pass through</td>
				<td width="78%" class="vtable">
					<textarea wrap="off" name="configpassthru" cols="75" rows="12" id="configpassthru" class="formpre2"><?=base64_decode($a_list['configpassthru']); ?></textarea>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top"></td>
				<td width="78%">
					<input name="Submit" type="submit" class="formbtn" value="Save">
					<input name="Submit2" type="submit" class="formbtn" value="Start"> 
					<input id="cancel" type="button" class="formbtn" value="Cancel">
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%">
					<span class="vexpl"><span class="red"><strong>Note:</strong></span>
					Please save your settings before you click start.</span>
				</td>
			</tr>
		</table>
		</form>
		
		<!-- STOP MAIN AREA -->
		</table>
		</td>
		</tr>			
		</table>
	</td>
	</tr>
</table>
</div>


<!-- footer do not touch below -->
<?php 
include("fend.inc"); 
echo $snort_custom_rnd_box;
?>


</body>
</html>
