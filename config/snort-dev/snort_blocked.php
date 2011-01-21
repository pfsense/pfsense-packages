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


$generalSettings = snortSql_fetchAllSettings('SnortSettings', 'id', '1');

$blertnumber = $generalSettings['blertnumber'];

$brefresh_on = ($generalSettings['brefresh'] == 'on' ? 'checked' : '');

	$pgtitle = "Services: Snort Blocked Hosts";
	include("/usr/local/pkg/snort/snort_head.inc");

?>
		
	
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<script type="text/javascript">

	// set the page resets watchThis
	function resetSnortDefaults(data) {
		
		jQuery("#blertnumber").val(data.blertnumber);
		
		if (data.brefresh == 'on') {
			jQuery("#brefresh").attr("checked", "checked");
		}else{
			jQuery("#brefresh").removeAttr('checked');
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

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
			<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
			<li><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
			<li><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
			<li><a href="/snort/snort_alerts.php"><span>Alerts</span></a></li>
			<li class="newtabmenu_active"><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
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
			
			<tr id="maintable" data-options='{"pagetable":"SnortSettings"}'> <!-- db to lookup -->
				<td width="22%" colspan="0" class="listtopic">Last 500 Blocked.</td>
				<td class="listtopic">This page lists hosts that have been blocked by Snort.&nbsp;&nbsp;Hosts are removed every <strong>hour</strong>.</td>
			</tr>
			<table width="100%" border="0" cellpadding="6" cellspacing="0">
			<tr>
				<td class="vncell2" valign="center" width="22%"><span class="vexpl">Save or Remove Hosts</span></td>
				<td width="40%" class="vtable">
				<form id="iform" action="./snort_json_post.php" method="post">
				<input name="snortblockedlogsdownload"  type="submit" class="formbtn" value="Download" >
				<input type="hidden" name="snortblockedlogsdownload" value="1" />
				<span class="vexpl">Save All Blocked Hosts</span>
				</form>
				</td>
				<td class="vtable">
				<form id="iform2" action="./snort_json_post.php" method="post">
				<input name="remove" type="submit" class="formbtn" value="Clear" onclick="return confirm('Do you really want to remove all blocked hosts ? All Blocked Hosts will be removed !')" > 
				<input type="hidden" name="snortflushpftable" value="1" />
				<span class="vexpl red"><strong>Warning:</strong></span><span class="vexpl"> all hosts will be removed.</span>
				</form>
				</td>
				<div class="hiddendownloadlink"></div>
			</tr>
			<tr>
				<td class="vncell2" valign="center"><span class="vexpl">Auto Refresh and Log View</span></td>
				<td class="vtable">
				<form id="iform3" action="./snort_json_post.php" method="post">
				<input name="save" type="submit" class="formbtn" value="Save"> 
				<input id="cancel" type="button" class="formbtn" value="Cancel">
				<span class="vexpl">Auto Refresh</span> 
				<input name="brefresh" id="brefresh" type="checkbox" value="on" <?=$brefresh_on; ?> >
				<span class="vexpl"><strong>Default ON</strong>.</span>
				</td>
				<td class="vtable">
				<input name="blertnumber" type="text" class="formfld" id="blertnumber" size="5" value="<?=$blertnumber;?>" > 
				<span class="vexpl">Limit entries to view. <strong>Default 500</strong>.</span>
				
        <input type="hidden" name="snortSaveSettings" value="1" /> <!-- what to do, save -->
        <input type="hidden" name="dbTable" value="SnortSettings" /> <!-- what db-->
        <input type="hidden" name="ifaceTab" value="snort_alerts_blocked" /> <!-- what interface tab -->
				
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
