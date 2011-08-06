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

//Set no caching
header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
header("Cache-Control: no-store, no-cache, must-revalidate");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");

$new_ruleUUID = genAlphaNumMixFast(7, 8);

$a_interfaces = snortSql_fetchAllInterfaceRules('SnortIfaces', 'snortDB');


	$pgtitle = "Services: Snort 2.9.0.5 pkg v. 2.0";
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

<form id="iform" >

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
			<li class="newtabmenu_active"><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
			<li><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
			<li><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
			<li><a href="/snort/snort_interfaces_rules.php"><span>RulesDB</span></a></li>
			<li><a href="/snort/snort_alerts.php"><span>Alerts</span></a></li>
			<li><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
			<li><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
			<li><a href="/snort/snort_interfaces_suppress.php"><span>Suppress</span></a></li>
			<li><a href="/snort/snort_help_info.php"><span>Help</span></a></li>
			</li>			
		</ul>
		</div>

		</td>
	</tr>
	<tr>
	<td id="tdbggrey">	
	<div style="width:750px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 10px;">
	<!-- START MAIN AREA -->	
			
			<!-- start snortsam -->
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
				<tr id="maintable77" >
					<td colspan="2" valign="top" class="listtopic">SnortSam Status</td>
				</tr>
			</table>
			
			<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
			
				<td class="list" colspan="8"></td>
				<td class="list" valign="middle" nowrap>
										
						<tr id="frheader" >
							<td width="3%" class="list">&nbsp;</td>
							<td width="10%" class="listhdrr2">SnortSam</td>
							<td width="10%" class="listhdrr">Role</td>
							<td width="10%" class="listhdrr">Port</td>
							<td width="10%" class="listhdrr">Pass</td>			
							<td width="10%" class="listhdrr">Log</td>
							<td width="50%" class="listhdr">Description</td>
							<td width="5%" class="list">&nbsp;</td>
							<td width="5%" class="list">&nbsp;</td>


							<tr valign="top" id="fr0">
							<td class="listt">
								<a href="?act=toggle&id=0"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_pass.gif" width="13" height="13" border="0" title="click to toggle start/stop snortsam"></a>
							</td>
								<td class="listbg" id="frd0" ondblclick="document.location='snort_interfaces_edit.php?id=0';">DISABLED</td>
								<td class="listr" id="frd0" ondblclick="document.location='snort_interfaces_edit.php?id=0';">MASTER</td>
								<td class="listr" id="frd0" ondblclick="document.location='snort_interfaces_edit.php?id=0';">3526</td>
								<td class="listr" id="frd0" ondblclick="document.location='snort_interfaces_edit.php?id=0';">ENABLED</td>
								<td class="listr" id="frd0" ondblclick="document.location='snort_interfaces_edit.php?id=0';">DISABLED</td>
								<td class="listbg3" ondblclick="document.location='snort_interfaces_edit.php?id=0';"><font color="#ffffff">Mster IPs&nbsp;</td>
								<td></td>
								<td>
									<a href="snort_interfaces_edit.php?id=0"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0" title="edit rule"></a>
								</td>
								
							</tr>						
						</tr>
				</td>
				<td class="list" colspan="8"></td>				
			</table>
			<!-- stop snortsam -->
<br>
			<!-- start Interface Satus -->
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
				<tr id="maintable77" >
					<td colspan="2" valign="top" class="listtopic2">Interface Status</td>
					<td width="6%" colspan="2" valign="middle" class="listtopic3" >
					<a href="snort_interfaces_edit.php?uuid=<?=$new_ruleUUID;?>">
					<img style="padding-left:3px;" src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0" title="add rule">
					</a>
					</td>
				</tr>
			</table>
<br>
			<!-- start User Interface -->
			<?php
			foreach ($a_interfaces as $list)
			{
				// make caps
				$list['interface'] = strtoupper($list['interface']);
				$list['performance'] = strtoupper($list['performance']);
				
				// rename for GUI iface
				$ifaceStat = ($list['enable'] == 'on' ? 'ENABLED' : 'DISABLED');
				$blockStat = ($list['blockoffenders7'] == 'on' ? 'ENABLED' : 'DISABLED');
				$logStat = ($list['snortunifiedlog'] == 'on' ? 'ENABLED' : 'DISABLED');
				$barnyard2Stat = ($list['barnyard_enable'] == 'on' ? 'ENABLED' : 'DISABLED');
				
				
				echo "
				<div id=\"maintable_{$list['uuid']}\" data-options='{\"pagetable\":\"SnortIfaces\", \"pagedb\":\"snortDB\", \"DoPOST\":\"true\"}'>
					";
				echo '
				<table width="100%" border="0" cellpadding="0" cellspacing="0">			
					<tr id="maintable77" >
					';
				echo "
						<td width=\"100%\" colspan=\"2\" valign=\"top\" class=\"listtopic\" >{$list['interface']} Interface Status &nbsp; ({$list['uuid']})</td>
					";
				echo '
					</tr>
				</table>
				
				<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
				
					<td class="list" colspan="8"></td>
					<td class="list" valign="middle" nowrap>
											
							<tr id="frheader" >
								<td width="3%" class="list">&nbsp;</td>
								<td width="11%" class="listhdrr2">Snort</td>
								<td width="10%" class="listhdrr">If</td>
								<td width="10%" class="listhdrr">Performance</td>
								<td width="10%" class="listhdrr">Block</td>			
								<td width="10%" class="listhdrr">Log</td>
								<td width="50%" class="listhdr">Description</td>
								<td width="5%" class="list">&nbsp;</td>
								<td width="5%" class="list">&nbsp;</td>
	
								<tr valign="top" id="fr0">
								<td class="listt">
						';
						echo "
									<a href=\"?act=toggle&id=0\"><img src=\"/themes/{$g['theme']}/images/icons/icon_pass.gif\" width=\"13\" height=\"13\" border=\"0\" title=\"click to toggle start/stop snort\"></a>
	
								</td>
									<td class=\"listbg\" id=\"frd0\" ondblclick=\"document.location='snort_interfaces_edit.php?uuid={$list['uuid']}';\">{$ifaceStat}</td>
									<td class=\"listr\" id=\"frd0\" ondblclick=\"document.location='snort_interfaces_edit.php?uuid={$list['uuid']}';\">{$list['interface']}</td>
									<td class=\"listr\" id=\"frd0\" ondblclick=\"document.location='snort_interfaces_edit.php?uuid={$list['uuid']}';\">{$list['performance']}</td>
									<td class=\"listr\" id=\"frd0\" ondblclick=\"document.location='snort_interfaces_edit.php?uuid={$list['uuid']}';\">{$blockStat}</td>
									<td class=\"listr\" id=\"frd0\" ondblclick=\"document.location='snort_interfaces_edit.php?uuid={$list['uuid']}';\">{$logStat}</td>
									<td class=\"listbg3\" ondblclick=\"document.location='snort_interfaces_edit.php?uuid={$list['uuid']}';\"><font color=\"#ffffff\">{$list['descr']}</td>
									<td></td>
									<td>
										<a href=\"snort_interfaces_edit.php?uuid={$list['uuid']}\"><img src=\"/themes/{$g['theme']}/images/icons/icon_e.gif\" width=\"17\" height=\"17\" border=\"0\" title=\"edit rule\"></a>
							";
						echo '
									</td>
									
								</tr>						
							</tr>
					</td>
					<td class="list" colspan="8"></td>				
				</table>
				<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
				
					<td class="list" colspan="8"></td>
					<td class="list" valign="middle" nowrap>
											
							<tr id="frheader" >
								<td width="3%" class="list">&nbsp;</td>
								<td width="10%" class="listhdrr2">Barnyard2</td>
								<td width="10%" class="listhdrr">If</td>
								<td width="10%" class="listhdrr">Sensor</td>
								<td width="10%" class="listhdrr">Type</td>			
								<td width="10%" class="listhdrr">Log</td>
								<td width="50%" class="listhdr">Description</td>
								<td width="5%" class="list">&nbsp;</td>
								<td width="5%" class="list">&nbsp;</td>
	
	
								<tr valign="top" id="fr0">
								<td class="listt">
								';
							echo "
									<a href=\"?act=toggle&id=0\"><img src=\"/themes/{$g['theme']}/images/icons/icon_pass.gif\" width=\"13\" height=\"13\" border=\"0\" title=\"click to toggle start/stop barnyard2\"></a>
								</td>
									<td class=\"listbg\" id=\"frd0\" ondblclick=\"document.location='snort_interfaces_edit.php?id=0';\">{$barnyard2Stat}</td>
									<td class=\"listr\" id=\"frd0\" ondblclick=\"document.location='snort_interfaces_edit.php?id=0';\">{$list['interface']}</td>
									<td class=\"listr\" id=\"frd0\" ondblclick=\"document.location='snort_interfaces_edit.php?id=0';\">{$list['uuid']}_{$list['interface']}</td>
									<td class=\"listr\" id=\"frd0\" ondblclick=\"document.location='snort_interfaces_edit.php?id=0';\">unified2</td>
									<td class=\"listr\" id=\"frd0\" ondblclick=\"document.location='snort_interfaces_edit.php?id=0';\">{$barnyard2Stat}</td>
									<td class=\"listbg3\" ondblclick=\"document.location='snort_interfaces_edit.php?id=0';\"><font color=\"#ffffff\">Mster IPs&nbsp;</td>
									<td></td>
									<td>
									<img id=\"icon_x_{$list['uuid']}\" class=\"icon_click icon_x\" src=\"/themes/{$g['theme']}/images/icons/icon_x.gif\" width=\"17\" height=\"17\" border=\"0\" title=\"delete rule\">
								";
							echo '		
									</td>
									
								</tr>						
							</tr>
					</td>
					<td class="list" colspan="8"></td>				
				</table>
				<br>
				</div>';
			} // end of foreach main
			?>		
			<!-- stop User Interface -->
			
			<!-- stop Interface Sat -->

	<!-- STOP MAIN AREA -->			
	</div>			
	</td>
	</tr>
</table>
</form>
</div>

<!-- start info box -->

<br>

<div style="background-color: #dddddd;" id="mainarea4">
<div style="width:750px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 10px;">
<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>&nbsp;&nbsp;&nbsp;</td>
	</tr>
	<tr >
	<td width="100%">
		<span class="red"><strong>Note:</strong></span> <br>
		This is the <strong>Snort Menu</strong> where you can see an over view of all your interface settings.
		Please edit the <strong>Global Settings</strong> tab before adding an interface. 
		<br>
		<br>
		<span class="red"><strong>Warning:</strong></span> 
		<br>
		<strong>New settings will not take effect until interface restart.</strong>
		<br>
		<br>
			<table>
			<tr>
			<td>
				<strong>Click</strong> on the 
				<img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0" title="Add Icon">
				 icon to add a interface.
			</td>
			<td>
				<strong>Click</strong> on the 
				<img src="/themes/<?= $g['theme']; ?>/images/icons/icon_pass.gif" width="13" height="13" border="0" title="Start Icon"> 
				icon to <strong>start</strong> snort or barnyard2.
			</td>
			</tr>
			<tr>
			<td>
				<strong>Click</strong> on the 
				<img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0" title="Edit Icon"> icon to edit a
				interface and settings.
			</td>
			<td>
				<strong>Click</strong> on the 
				<img src="/themes/<?= $g['theme']; ?>/images/icons/icon_block.gif" width="13" height="13" border="0" title="Stop Icon"> 
				icon to <strong>stop</strong> snort or barnyard2.
			</td>
			</tr>
			<tr>
			<td>
				<strong> Click</strong> on the 
				<img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0" title="Delete Icon"> 
				icon to delete a interface and settings.
			</td>
			</tr>
			<tr>
			<td>&nbsp;&nbsp;&nbsp;</td>
			</tr>
			</table>		
	</td>
	</tr>
</table>
</div>
</div>

<!-- stop info box -->

<!-- start snort footer -->

<br>

<div style="background-color: #dddddd;" id="mainarea6">
<div style="width:750px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 10px;">
<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>&nbsp;&nbsp;&nbsp;</td>
	</tr>
	<tr >
	<td width="100%">
		<div id="footer2">
		<table>
		<tr>
		<td style="padding-top: 40px;">		
			SNORT registered &#174; by Sourcefire, Inc, Barnyard2 registered &#174; by securixlive.com, Orion registered &#174; by Robert Zelaya,
			Emergingthreats registered &#174; by emergingthreats.net, Mysql registered &#174; by Mysql.com
		</td>
		</tr>			
		</table>
		</div>	
	</td>
	</tr>
	<tr>
		<td>&nbsp;&nbsp;&nbsp;</td>
	</tr>	
</table>
</div>
</div>

<!-- stop snort footer -->

<!-- footer do not touch below -->
<?php 
include("fend.inc"); 
echo $snort_custom_rnd_box;
?>


</body>
</html>
