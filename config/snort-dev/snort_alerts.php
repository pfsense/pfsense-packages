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

$generalSettings = snortSql_fetchAllSettings('snortDB', 'SnortSettings', 'id', '1');

$alertnumber = $generalSettings['alertnumber'];

$arefresh_on = ($generalSettings['arefresh']  == 'on' ? 'checked' : '');

	$pgtitle = "Services: Snort: Alerts";
	include("/usr/local/pkg/snort/snort_head.inc");

?>
			
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">


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

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
			<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
			<li><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
			<li><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
			<li class="newtabmenu_active"><a href="/snort/snort_alerts.php"><span>Alerts</span></a></li>
			<li><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
			<li><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
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
			
			<tr id="maintable" data-options='{"pagetable":"SnortSettings"}'> <!-- db to lookup -->
				<td colspan="2" valign="top" class="listtopic" width="21%">Last 255 Alert Entries</td>
				<td colspan="2" valign="top" class="listtopic">Latest Alert Entries Are Listed First</td>
			</tr>
			<table width="100%" border="0" cellpadding="6" cellspacing="0">
			<tr>
				<td class="vncell2" valign="center" width="21%"><span class="vexpl">Save or Remove Logs</span></td>
				<td class="vtable" width="40%">
					<form id="iform" >
					<input name="snortlogsdownload"  type="submit" class="formbtn" value="Download" >
					<input type="hidden" name="snortlogsdownload" value="1" />
					<span class="vexpl">Save All Log Files.</span>
					</form>
				</td>
				<td class="vtable">
					<form id="iform2" >
					<input name="snortlogsdelete"  type="submit" class="formbtn" value="Clear" onclick="return confirm('Do you really want to remove all your logs ? All Snort Logs will be removed !')" >
					<input type="hidden" name="snortlogsdelete" value="1" />
					<span class="vexpl red"><strong>Warning:</strong></span><span class="vexpl"> all logs will be deleted.</span>
					</form>
				</td>
					<div class="hiddendownloadlink"></div>				
			</tr>
			<tr>
				<td class="vncell2" valign="center"><span class="vexpl">Auto Refresh and Log View</span></td>
				<td class="vtable">
					<form id="iform3" >
					<input name="save" type="submit" class="formbtn" value="Save">
					<input id="cancel" type="button" class="formbtn" value="Cancel">
					<input name="arefresh" id="arefresh" type="checkbox" value="on" <?=htmlspecialchars($arefresh_on);?> >
					<span class="vexpl">Auto Refresh</span>
					<span class="vexpl"><strong>Default ON</strong>.</span> 
				</td>
				<td class="vtable">
					<input name="alertnumber" type="text" class="formfld2" id="alertnumber" size="5" value="<?=htmlspecialchars($alertnumber);?>" >
					<span class="vexpl">Limit entries to view. <strong>Default 250</strong>.</span>

          <input type="hidden" name="snortSaveSettings" value="1" /> <!-- what to do, save -->
          <input type="hidden" name="dbName" value="snortDB" /> <!-- what db -->
          <input type="hidden" name="dbTable" value="SnortSettings" /> <!-- what db table -->
          <input type="hidden" name="ifaceTab" value="snort_alerts" /> <!-- what interface tab -->

					</form>
				</td>				
			</tr>			
			</table>			

			
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
