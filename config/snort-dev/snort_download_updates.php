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

// disable csrf for downloads, progressbar did not work because of this
$nocsrf = true;

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");
require_once("/usr/local/pkg/snort/snort_download_rules.inc");

// set page vars
if (isset($_GET['updatenow'])) {
	$updatenow = $_GET['updatenow'];
}

header("Cache-Control: no-cache, must-revalidate");
header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");

// get dates of md5s

$tmpSettingsSnort = 'N/A';
$tmpSettingsSnortChk = snortSql_fetchAllSettings2('snortDBtemp', 'SnortDownloads', 'filename', 'snortrules-snapshot-2905.tar.gz');
if (!empty($tmpSettingsSnortChk)) {
	$tmpSettingsSnort = date('l jS \of F Y h:i:s A', $tmpSettingsSnortChk[date]);
}

$tmpSettingsEmerging = 'N/A';
$tmpSettingsEmergingChk = snortSql_fetchAllSettings2('snortDBtemp', 'SnortDownloads', 'filename', 'emerging.rules.tar.gz');
if (!empty($tmpSettingsEmergingChk)) {
	$tmpSettingsEmerging = date('l jS \of F Y h:i:s A', $tmpSettingsEmergingChk[date]);
}

$tmpSettingsPfsense = 'N/A';
$tmpSettingsPfsenseChk = snortSql_fetchAllSettings2('snortDBtemp', 'SnortDownloads', 'filename', 'pfsense_rules.tar.gz');
if (!empty($tmpSettingsPfsenseChk)) {
	$tmpSettingsPfsense = date('l jS \of F Y h:i:s A', $tmpSettingsPfsenseChk[date]);
}

// get rule on stats
$generalSettings = snortSql_fetchAllSettings2('snortDB', 'SnortSettings', 'id', '1');

$snortMd5CurrentChk = @file_get_contents('/usr/local/etc/snort/snortDBrules/snort_rules/snortrules-snapshot-2905.tar.gz.md5');

$snortDownlodChkMark = '';
if ($generalSettings[snortdownload] === 'on') {
	$snortDownlodChkMark = 'checked="checked"';
}

$snortMd5Current = 'N/A';
if (!empty($snortMd5CurrentChk)) {	
	preg_match('/^\".*\"/', $snortMd5CurrentChk, $snortMd5Current);	
	if (!empty($snortMd5Current[0])) {
		$snortMd5Current = preg_replace('/\"/', '', $snortMd5Current[0]);
	}
}

$emergingMd5CurrentChk = @file_get_contents('/usr/local/etc/snort/snortDBrules/emerging_rules/emerging.rules.tar.gz.md5');

$emerginDownlodChkMark = '';
if ($generalSettings[emergingthreatsdownload] !== 'off') {
	$emerginDownlodChkMark = 'checked="checked"';
}

$emergingMd5Current = 'N/A';
if (!empty($emergingMd5CurrentChk)) {
	$emergingMd5Current = $emergingMd5CurrentChk;
}

$pfsenseMd5CurrentChk = @file_get_contents('/usr/local/etc/snort/snortDBrules/pfsense_rules/pfsense_rules.tar.gz.md5');

$pfsenseMd5Current = 'N/A';
if (!empty($pfsenseMd5CurrentChk)) {	
	preg_match('/^\".*\"/', $pfsenseMd5CurrentChk, $pfsenseMd5Current);	
	if (!empty($pfsenseMd5Current[0])) {
		$pfsenseMd5Current = preg_replace('/\"/', '', $pfsenseMd5Current[0]);
	}
}

	$pgtitle = 'Services: Snort: Updates';
	include("/usr/local/pkg/snort/snort_head.inc");

?>

		
	
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<!-- loading update msg -->
<div id="loadingRuleUpadteGUI">

	<div class="snortModalUpdate">
		<div class="snortModalTopUpdate">
			<div class="snortModalTopClose">
			<!-- <a href="javascript:hideLoading('#loadingRuleUpadteGUI');"><img src="/snort/images/close_9x9.gif" border="0" height="9" width="9"></a> -->
			</div>
		</div>
	  			<p id="UpdateMsg1" class="snortModalTitleUpdate snortModalTitleUpdateMsg1">
				</p>
		<div class="snortModalTitleUpdate snortModalTitleUpdateBar">
				<table width="600px" height="43px" border="0" cellpadding="0" cellspacing="0">
					<tr><td><span class="progressBar" id="pb4"></span></td></tr>
				</table>
	  	</div>
	  			<p id="UpdateMsg2" class="snortModalTitleUpdate snortModalTitleUpdateMsg2">
	  			</p>  	
	</div> 

</div>


<?php include("fbegin.inc"); ?>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="newtabmenu" style="margin: 1px 0px; width: 790px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
			<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
			<li><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
			<li class="newtabmenu_active"><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
			<li><a href="/snort/snort_interfaces_rules.php"><span>RulesDB</span></a></li>
			<li><a href="/snort/snort_alerts.php"><span>Alerts</span></a></li>
			<li><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
			<li><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
			<li><a href="/snort/snort_interfaces_suppress.php"><span>Suppress</span></a></li>
			<li><a href="/snort/snort_help_info.php"><span>Help</span></a></li>
		</ul>					
		</div>

		</td>
	</tr>	
	<tr>
		<td>

		<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
			<li class="newtabmenu_active"><a href="/snort/snort_download_rules.php"><span>Rule Update</span></a></li>
			<!-- <li><a href="#"><span>Upload Custom Rules</span></a></li> -->
			<!-- <li><a href="#"><span>Gui Update</span></a></li> -->
		</ul>
		</div>

		</td>
	</tr>	
	<tr>
	<td id="tdbggrey">	
	<div style="width:780px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 10px;">
	<!-- START MAIN AREA -->
	
	
			<!-- start Interface Satus -->
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
				<tr id="maintable77" >
					<td colspan="2" valign="top" class="listtopic2">
					Rule databases that are ready to be updated. 
					</td>
					<td width="6%" colspan="2" valign="middle" class="listtopic3" >
					</td>
				</tr>
			</table>
<br>

			<!-- start User Interface -->
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
				<tr id="maintable77" >
					<td colspan="2" valign="top" class="listtopic">SIGNATURE RULESET DATABASES:</td>
				</tr>
			</table>
			
			
			<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
			
				<td class="list" ></td>
				<td class="list" valign="middle" >
										
						<tr id="frheader" >
							<td width="1%" class="listhdrr2">On</td>
							<td width="25%" class="listhdrr2">Signature DB Name</td>
							<td width="35%" class="listhdrr2">MD5 Version</td>
							<td width="38%" class="listhdrr2">Last Rule DB Date</td>
							<td width="1%" class="listhdrr2">&nbsp;</td>											
						</tr>
						
				<!-- START javascript sid loop here -->
						<tbody class="rulesetloopblock">
						
<tr id="fr0" valign="top">
<td class="odd_ruleset2">
<input class="domecheck" name="filenamcheckbox2[]" value="1292" <?=$snortDownlodChkMark;?> type="checkbox" disabled="disabled" >
</td>
<td class="odd_ruleset2" id="frd0">SNORT.ORG</td>
<td class="odd_ruleset2" id="frd0"><?=$snortMd5Current;?></td>
<td class="listbg" id="frd0"><font color="white"><?=$tmpSettingsSnort;?></font></td>
<td class="odd_ruleset2">
<img src="/themes/pfsense_ng/images/icons/icon_alias_url_reload.gif" title="edit rule" width="17" border="0" height="17">
</td>
</tr>

<tr id="fr0" valign="top">
<td class="odd_ruleset2">
<input class="domecheck" name="filenamcheckbox2[]" value="1292" <?=$emerginDownlodChkMark;?> type="checkbox" disabled="disabled" >
</td>
<td class="odd_ruleset2" id="frd0">EMERGINGTHREATS.NET</td>
<td class="odd_ruleset2" id="frd0"><?=$emergingMd5Current;?></td>
<td class="listbg" id="frd0"><font color="white"><?=$tmpSettingsEmerging; ?></font></td>
<td class="odd_ruleset2">
<img src="/themes/pfsense_ng/images/icons/icon_alias_url_reload.gif" title="edit rule" width="17" border="0" height="17">
</td>
</tr>

<tr id="fr0" valign="top">
<td class="odd_ruleset2">
<input class="domecheck" name="filenamcheckbox2[]" value="1292" checked="checked" type="checkbox" disabled="disabled" >
</td>
<td class="odd_ruleset2" id="frd0">PFSENSE.ORG</td>
<td class="odd_ruleset2" id="frd0"><?=$pfsenseMd5Current;?></td>
<td class="listbg" id="frd0"><font color="white"><?=$tmpSettingsPfsense;?></font></td>
<td class="odd_ruleset2">
<img src="/themes/pfsense_ng/images/icons/icon_alias_url_reload.gif" title="edit rule" width="17" border="0" height="17">
</td>
</tr>						
						
						</tbody>
				<!-- STOP javascript sid loop here -->
						
				</td>
				<td class="list" colspan="8"></td>				
							
			</table>
			<br>
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
			<tr>
				<td>
					<input id="openupdatebox" type="submit" class="formbtn" value="Update">
				</td>
			</tr>
			</table>
			<br>	
					
			<!-- stop snortsam -->

	<!-- STOP MAIN AREA -->			
	</div>			
	</td>
	</tr>
</table>
</div>

<!-- start info box -->

<br>

<div style="width:790px; background-color: #dddddd;" id="mainarea4">
<div style="width:780px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 10px;">
<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr >
		<td width="10%" valign="middle" >
			<img style="vertical-align: middle;" src="/snort/images/icon_excli.png" width="40" height="32">
		</td>
		<td width="90%" valign="middle" >
			<span class="red"><strong>Note:</strong></span>
			<strong>&nbsp;&nbsp;Snort.org and Emergingthreats.net will go down from time to time. Please be patient.</strong>		
		</td>
	</tr>
</table>
</div>
</div>


<script type="text/javascript">


//prepare the form when the DOM is ready 
jQuery(document).ready(function() {

	jQuery('.closeupdatebox').live('click', function(){
		var url = '/snort/snort_download_updates.php';
		window.location = url;
	});	

	jQuery('#openupdatebox').live('click', function(){
		var url = '/snort/snort_download_updates.php?updatenow=1';
		window.location = url;
	});	
		
}); // end of document ready

</script>

<?php

if ($updatenow == 1) {
	sendUpdateSnortLogDownload('');	// start main function
	echo '
	<script type="text/javascript">
		jQuery(\'.snortModalTopClose\').append(\'<img class="icon_click closeupdatebox" src="/snort/images/close_9x9.gif" border="0" height="9" width="9">\'); 
	</script>
	';
}

?>


<!-- stop info box -->

<!-- footer do not touch below -->
<?php 
include("fend.inc"); 
echo $snort_custom_rnd_box;
?>


</body>
</html>
