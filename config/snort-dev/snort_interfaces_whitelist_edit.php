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

require_once('guiconfig.inc');
require_once('/usr/local/pkg/snort/snort_new.inc');
require_once('/usr/local/pkg/snort/snort_gui.inc');

//$GLOBALS['csrf']['rewrite-js'] = false;

$uuid = $_GET['uuid'];
if (isset($_POST['uuid']))
$uuid = $_POST['uuid'];

if ($uuid == '') {
	echo 'error: no uuid';
	exit(0);
}

$a_list = snortSql_fetchAllSettings('snortDB', 'SnortWhitelist', 'uuid', $uuid);

// $a_list returns empty use defaults
if ($a_list == '')
{
  
  $a_list = array(
      'id' => '',
      'date' => date(U),
      'uuid' => $uuid,
      'filename' => '',
      'snortlisttype' => 'whitelist',
      'description' => '',
      'wanips' => 'on',
      'wangateips' => 'on',
      'wandnsips' => 'on',
      'vips' => 'on',
      'vpnips' => 'on'
  ); 
  
}

$listFilename = $a_list['filename'];

$a_list['list'] = snortSql_fetchAllSettingsList('SnortWhitelistips', $listFilename);

$wanips_chk = $a_list['wanips'];
$wanips_on = ($wanips_chk == 'on' ? 'checked' : '');

$wangateips_chk = $a_list['wangateips'];
$wangateips_on = ($wangateips_chk == 'on' ? 'checked' : '');

$wandnsips_chk = $a_list['wandnsips'];
$wandnsips_on = ($wandnsips_chk == 'on' ? 'checked' : '');

$vips_chk = $a_list['vips'];
$vips_on = ($vips_chk == 'on' ? 'checked' : '');

$vpnips_chk = $a_list['vpnips'];
$vpnips_on = ($vpnips_chk == 'on' ? 'checked' : '');



	$pgtitle = "Services: Snort: Whitelist Edit";
	include("/usr/local/pkg/snort/snort_head.inc");

?>
	
	
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

<form id="iform">

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
			<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
			<li><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
			<li><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
			<li><a href="/snort/snort_alerts.php"><span>Alerts</span></a></li>
			<li><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
			<li class="newtabmenu_active"><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
			<li><a href="/snort/snort_interfaces_suppress.php"><span>Suppress</span></a></li>
			<li><a href="/snort/snort_help_info.php"><span>Help</span></a></li>	
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
		
      <!-- table point -->
		<input name="snortSaveWhitelist" type="hidden" value="1" />
		<input name="ifaceTab" type="hidden" value="snort_interfaces_whitelist_edit" />
		<input type="hidden" name="dbName" value="snortDB" /> <!-- what db -->
        <input type="hidden" name="dbTable" value="SnortWhitelist" /> <!-- what db table -->  
		<input name="date" type="hidden" value="<?=$a_list['date'];?>" />
		<input name="uuid" type="hidden" value="<?=$a_list['uuid'];?>" />
						
			<tr>
				<td colspan="2" valign="top" class="listtopic">Add the name and description of the file.</td>

			</tr>
			<tr id="filename" data-options='{"filename":"<?=$listFilename; ?>"}' >
				<td valign="top" class="vncellreq2">Name</td>
				<td class="vtable">
				<input class="formfld2" name="filename" type="text" id="name" size="40" value="<?=$listFilename; ?>" /> <br />
				<span class="vexpl"> The list name may only consist of the characters a-z, A-Z and 0-9. <span class="red">Note: </span> No Spaces. </span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Description</td>
				<td width="78%" class="vtable">
				<input class="formfld2" name="description" type="text" id="descr" size="40" value="<?=$a_list['description']; ?>" /> <br />
				<span class="vexpl"> You may enter a description here for your reference (not parsed). </span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">List Type</td>
				<td width="78%" class="vtable">
				<div style="padding: 5px; margin-top: 16px; margin-bottom: 16px; border: 1px dashed #ff3333; background-color: #eee; color: #000; font-size: 8pt;"id="itemhelp">
				<strong>WHITELIST:</strong>&nbsp;&nbsp;&nbsp;This list specifies addresses that Snort Package should not block.<br><br>
				<strong>NETLIST:</strong>&nbsp;&nbsp;&nbsp;This list is for defining addresses as $HOME_NET or $EXTERNAL_NET in the snort.conf file.
				</div>
				<select name="snortlisttype" class="formfld2" id="snortlisttype">
				<?php
				$updateDaysList = array('whitelist' => 'WHITELIST', 'netlist' => 'NETLIST'); 
				snortDropDownList($updateDaysList, $a_list['snortlisttype']); 
				?>
				</select> 
				<span class="vexpl"> &nbsp;&nbsp;&nbsp;Choose the type of list you will like see in your <span class="red">Interface Edit Tab</span>.</span>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Add auto generated ips.</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">WAN IPs</td>
				<td width="78%" class="vtable">
				<input name="wanips" type="checkbox" id="wanips" size="40" value="on" <?=$wanips_on; ?> />
				<span class="vexpl"> Add WAN IPs to the list. </span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Wan Gateways</td>
				<td width="78%" class="vtable">
				<input name="wangateips" type="checkbox" id="wangateips" size="40" value="on" <?=$wangateips_on; ?> />
				<span class="vexpl"> Add WAN Gateways to the list. </span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Wan DNS servers</td>
				<td width="78%" class="vtable">
				<input name="wandnsips" type="checkbox" id="wandnsips" size="40" value="on" <?=$wandnsips_on; ?> />
				<span class="vexpl"> Add WAN DNS servers to the list. </span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Virtual IP Addresses</td>
				<td width="78%" class="vtable">
				<input name="vips" type="checkbox" id="vips" size="40" value="on" <?=$vips_on; ?> />
				<span class="vexpl"> Add Virtual IP Addresses to the list. </span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">VPNs</td>
				<td width="78%" class="vtable">
				<input name="vpnips" type="checkbox" id="vpnips" size="40" value="on" <?=$vpnips_on; ?> />
				<span class="vexpl"> Add VPN Addresses to the list. </span>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Add your own custom ips.</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncellreq2">
				<div id="addressnetworkport">IP or CIDR items</div>
				</td>
				<td width="78%" class="vtable">
				<table >
					<tbody class="insertrow">
						<tr>
							<td colspan="4">
							<div style="width:550px; padding: 5px; margin-top: 16px; margin-bottom: 16px; border: 1px dashed #ff3333; background-color: #eee; color: #000; font-size: 8pt;"id="itemhelp">
							For <strong>WHITELIST's</strong> enter <strong>ONLY IPs not CIDRs</strong>. Example: 192.168.4.1<br><br>
							For <strong>NETLIST's</strong> you may enter <strong>IPs and CIDRs</strong>. Example: 192.168.4.1 or 192.168.4.0/24
							</div>
							</td>
						</tr>
						<tr>
							<td>
							<div id="onecolumn" style="width:175px;"><span class="vexpl">IP or CIDR</span></div>
							</td>
							<td>
   							<div id="threecolumn"><span class="vexpl">Add a Description or leave blank and a date will be added.</span></div>
							</td>
						</tr>
						</tbody>
						<!-- Start of js loop -->
						<tbody id="listloopblock" class="insertrow">
						<?php echo "\r"; $i = 0; foreach ($a_list['list'] as $list): ?>
						<tr id="maintable_<?=$list['uuid']?>" data-options='{"pagetable":"SnortWhitelist", "pagedb":"snortDB", "DoPOST":"false"}' >
							<td>
							<input class="formfld2" name="list[<?=$i; ?>][ip]" type="text" id="address" size="30" value="<?=$list['ip']; ?>" />
							</td>
							<td>
							<input class="formfld2" name="list[<?=$i; ?>][description]" type="text" id="detail" size="50" value="<?=$list['description'] ?>" />
							</td>
							<td>
							<img id="icon_x_<?=$list['uuid'];?>" class="icon_click icon_x" src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0" title="delete list" >
							</td>
							<input name="list[<?=$i; ?>][uuid]" type="hidden" value="<?=$list['uuid'];?>" />
						</tr>
						<?php echo "\r"; $i++; endforeach; ?>
					</tbody>
					<!-- End of js loop -->
					<tbody>
						<tr>
							<td>
							</td>
							<td>
							</td>
							<td>				
							<img id="iconplus_<?=$i;?>" class="icon_click icon_plus" src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0" title="add list" >
							</td>
						</tr>
					</tbody>
				</table>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%">
				<input id="submit" name="submit" type="submit" class="formbtn" value="Save" /> 
				<input id="cancel" name="cancel" type="button" class="formbtn" value="Cancel">
				</td>
			</tr>
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
