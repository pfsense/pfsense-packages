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

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_new.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");

// set page vars

$generalSettings = snortSql_fetchAllSettings('SnortSettings', 'id', '1');

$snortdownload_off = ($generalSettings['snortdownload'] == 'off' ? 'checked' : '');
$snortdownload_on = ($generalSettings['snortdownload'] == 'on' ? 'checked' : '');

$emergingthreats_on = ($generalSettings['emergingthreats'] == 'on' ? 'checked' : '');

$updaterules = $generalSettings['updaterules'];

$oinkmastercode = $generalSettings['oinkmastercode'];

$rm_blocked = $generalSettings['rm_blocked'];

$snortloglimit_off = ($generalSettings['snortloglimit'] == 'off' ? 'checked' : '');
$snortloglimit_on = ($generalSettings['snortloglimit']  == 'on' ? 'checked' : '');

$snortloglimitsize = $generalSettings['snortloglimitsize'];

$snortalertlogtype = $generalSettings['snortalertlogtype'];

$forcekeepsettings_on = ($generalSettings['forcekeepsettings'] == 'on' ? 'checked' : '');

$snortlogCurrentDSKsize = round(exec('df -k /var | grep -v "Filesystem" | awk \'{print $4}\'') / 1024);


	$pgtitle = "Services: Snort: Global Settings";
	include("/usr/local/pkg/snort/snort_head.inc");

?>
		
	
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<script type="text/javascript">

	// set the page resets watchThis
	function resetSnortDefaults(general0) {
	
		if (general0.snortdownload == 'on') {
			jQuery("#snortdownloadon").attr("checked", "checked");
		}else{
			jQuery("#snortdownloadoff").attr("checked", "checked");
		}
		
		jQuery("#oinkmastercode").val(general0.oinkmastercode);
		
		if (general0.emergingthreats == 'on') {
			jQuery("#emergingthreats").attr("checked", "checked");
		}else{
			jQuery("#emergingthreats").removeAttr('checked');
		}
		
		jQuery("#updaterules").val(general0.updaterules);
		
		if (general0.snortloglimit == 'on') {
			jQuery("#snortloglimiton").attr("checked", "checked");
		}else{
			jQuery("#snortloglimitoff").attr("checked", "checked");
		}
		
		jQuery("#snortloglimitsize").val(general0.snortloglimitsize);
		
		jQuery("#rm_blocked").val(general0.rm_blocked);
		
		jQuery("#snortalertlogtype").val(general0.snortalertlogtype);
		
		if (general0.forcekeepsettings == 'on') {
			jQuery("#forcekeepsettings").attr("checked", "checked");
		}else{
			jQuery("#forcekeepsettings").removeAttr('checked');
		}
		
	}
	
</script>

<div id="loadingWaiting">
  <p class="loadingWaitingMessage"><img src="./images/loading.gif" /> <br>Please Wait...</p>
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

<? //if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}
echo '<p class="pgtitle">' . $pgtitle . '</p>';
?>

<form id="iform" action="./snort_json_post.php" method="post">

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
			<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
			<li class="newtabmenu_active"><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
			<li><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
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
		<table width="100%" border="0" cellpadding="10px" cellspacing="0">
		<tr>
		<td class="tabnavtbl">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<!-- START MAIN AREA -->
		
		<input type="hidden" name="snortSaveSettings" value="1" /> <!-- what to do, save -->
		<input type="hidden" name="dbTable" value="SnortSettings" /> <!-- what db-->
		<input type="hidden" name="ifaceTab" value="snort_interfaces_global" /> <!-- what interface tab -->
			
			<tr id="maintable" data-options='{"pagetable":"SnortSettings"}'> <!-- db to lookup -->
				<td colspan="2" valign="top" class="listtopic">Please Choose The Type Of Rules You Wish To Download</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Install Snort.org rules</td>
				<td width="78%" class="vtable">
				<table cellpadding="0" cellspacing="0">
					<tr>
						<td colspan="2">
						<input name="snortdownload" type="radio" id="snortdownloadoff" value="off" <?=$snortdownload_off;?> >
						<span class="vexpl">Do <strong>NOT</strong> Install</span>
						</td>
					</tr>
					<tr>
						<td colspan="2">
						<input name="snortdownload" type="radio" id="snortdownloadon" value="on" <?=$snortdownload_on;?> > 
						<span class="vexpl">Install Basic Rules or Premium rules</span> <br>
						&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
						<a href="https://www.snort.org/signup" target="_blank">
						Sign Up for a Basic Rule Account
						</a><br>
						&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
						<a href="http://www.snort.org/vrt/buy-a-subscription" target="_blank">
						Sign Up for Sourcefire VRT Certified Premium Rules. This Is Highly Recommended
						</a>
						</td>
					</tr>
					<tr>
						<td>&nbsp;</td>
					</tr>
				</table>
				<table width="100%" border="0" cellpadding="6" cellspacing="0">
					<tr>
						<td colspan="2" valign="top"><span class="vexpl">Oinkmaster code</span></td>
					</tr>
					<tr>
						<td class="vncell2" valign="top"><span class="vexpl">Code</span></td>
						<td class="vtable">
						<input name="oinkmastercode" type="text"class="formfld" id="oinkmastercode" size="52" value="<?=$oinkmastercode;?>" > <br>
						<span class="vexpl">Obtain a snort.org Oinkmaster code and paste here.</span>
						</td>				
				</table>			
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2"><span>Install <strong>Emergingthreats</strong> rules</span></td>
				<td width="78%" class="vtable">
				<input name="emergingthreats" id="emergingthreats" type="checkbox" value="on" <?=$emergingthreats_on; ?>> <br>
				<span class="vexpl">Emerging Threats is an open source community that produces fastest moving and diverse Snort Rules.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2"><span>Update rules automatically</span></td>
				<td width="78%" class="vtable">
				<select name="updaterules" class="formfld" id="updaterules">
					<?php 					
					$updateDaysList = array('never' => 'NEVER', '6h_up' => '6 HOURS', '12h_up' => '12 HOURS', '1d_up' => '1 DAY', '4d_up' => '4 DAYS', '7d_up' => '7 DAYS', '28d_up' => '28 DAYS');					
					snortDropDownList($updateDaysList, $updaterules);					
					?>
				</select><br>
				<span class="vexpl">
				Please select the update times for rules.<br> Hint: in most cases, every 12 hours is a good choice.
				</span>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic"><span>General Settings</span></td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2"><span>Log Directory SizeLimit</span><br>
				<br><br><br><br><br>
				<span class="red"><strong>Note:</strong><br>Available space is <strong><?=$snortlogCurrentDSKsize; ?>MB</strong></span>
				</td>
				<td width="78%" class="vtable">
				<table cellpadding="0" cellspacing="0">
					<tr>
					<td colspan="2">
						<input name="snortloglimit" type="radio" id="snortloglimiton" value="on"  <?=$snortloglimit_on;?> > 
						<span class="vexpl"><strong>Enable</strong> directory size limit (Default)</span>
					</td>
					</tr>
					<tr>
						<td colspan="2">
						<input name="snortloglimit" type="radio" id="snortloglimitoff" value="off" <?=$snortloglimit_off ?> >  
						<span class="vexpl"><strong>Disable </strong>directory size limit</span><br><br>
						<span class="vexpl red"><strong>Warning:</strong> Pfsense Nanobsd should use no more than 10MB of space.</span>
						</td>
					</tr>
					<tr>
						<td>&nbsp;</td>
					</tr>
				</table>
				<table width="100%" border="0" cellpadding="6" cellspacing="0">
					<tr>
						<td class="vncell3"><span>Size in <strong>MB</strong></span></td>
						<td class="vtable">
						<input name="snortloglimitsize" type="text" class="formfld" id="snortloglimitsize" size="7" value="<?=$snortloglimitsize;?>">
						<span class="vexpl">Default is <strong>20%</strong> of available space.</span>
						</td>				
				</table>			
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2"><span>Remove blocked hosts every</span></td>
				<td width="78%" class="vtable">								
					<select name="rm_blocked" class="formfld" id="rm_blocked">
					<?php 
						$BlockTimeReset = array('never' => 'NEVER', '1h_b' => '1 HOUR', '3h_b' => '3 HOURS', '6h_b' => '6 HOURS', '12h_b' => '12 HOURS', '1d_b' => '1 DAY', '4d_b' => '4 DAYS', '7d_b' => '7 DAYS', '28d_b' => '28 DAYS');
						snortDropDownList($BlockTimeReset, $rm_blocked);				
					?>
					</select><br>					
					<span class="vexpl">Please select the amount of time you would likehosts to be blocked for.<br>Hint: in most cases, 1 hour is a good choice.</span>
					</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2"><span>Alerts file descriptiontype</span></td>
				<td width="78%" class="vtable">
				<select name="snortalertlogtype" class="formfld" id="snortalertlogtype">
					<?php
						// TODO: make this option a check box with all log types
						$alertLogTypeList = array('full' => 'FULL', 'fast' => 'SHORT');
						snortDropDownList($alertLogTypeList, $snortalertlogtype)
					?>
				</select><br>
				<span class="vexpl">Please choose the type of Alert logging you will like see in your alert file.<br> Hint: Best pratice is to chose full logging.</span>&nbsp;
				<span class="red"><strong>WARNING:</strong></span>&nbsp;<strong>On change, alert file will be cleared.</strong>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2"><span>Keep snort settings after deinstall</span></td>
				<td width="22%" class="vtable">
				<input name="forcekeepsettings" id="forcekeepsettings" type="checkbox" value="on" <?=$forcekeepsettings_on;?> >
				<span class="vexpl">Settings will not be removed during deinstall.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2"><span>Save Settings</span></td>
				<td width="30%" class="vtable">
				<input name="Submit" type="submit" class="formbtn" value="Save">
				<input id="cancel" type="button" class="formbtn" value="Cancel">
				</form>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">
				<form id="iform2" action="./snort_json_post.php" method="post">
				<input name="Reset" type="submit" class="formbtn" value="Reset" onclick="return confirm('Do you really want to remove all your settings ? All Snort Settings will be reset !')" >
				<input type="hidden" name="reset_snortgeneralsettings" value="1" />
				<span class="vexpl red"><strong>&nbsp;WARNING:</strong><br> This will reset all global and interface settings.</span>
				</form>
				</td>
				<td class="vtable">
				<span class="vexpl red"><strong>Note:</strong></span><br> 
				<span class="vexpl">Changing any settings on this page will affect all interfaces. Please, double check if your oink code is correct and the type of snort.org account you hold.</span>
				</td>	
			</tr>

		
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
