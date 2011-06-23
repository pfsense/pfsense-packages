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

$a_list = snortSql_fetchAllSettings('snortDB', 'SnortSuppress', 'uuid', $uuid);


// $a_list returns empty use defaults
if ($a_list == '')
{
  
  $a_list = array(
      'id' => '',
      'date' => date(U),
      'uuid' => $uuid,
      'filename' => '',
      'description' => '',
      'suppresspassthru' => ''

  );
  
}




	$pgtitle = 'Services: Snort: Suppression: Edit: ' . $uuid;
	include('/usr/local/pkg/snort/snort_head.inc');

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
			<li><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
			<li class="newtabmenu_active"><a href="/snort/snort_interfaces_suppress.php"><span>Suppress</span></a></li>
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
		
		<!-- table point -->
		<input name="snortSaveSuppresslist" type="hidden" value="1" />
		<input name="ifaceTab" type="hidden" value="snort_interfaces_suppress_edit" />
		<input type="hidden" name="dbName" value="snortDB" /> <!-- what db -->
        <input type="hidden" name="dbTable" value="SnortSuppress" /> <!-- what db table -->
		<input name="date" type="hidden" value="<?=$a_list['date'];?>" />
		<input name="uuid" type="hidden" value="<?=$a_list['uuid'];?>" />		
	
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
			<tr>
				<td colspan="2" valign="top" class="listtopic">Add the name anddescription of the file.</td>
			</tr>
			<tr>
				<td valign="top" class="vncellreq2">Name</td>
				<td class="vtable">
				<input class="formfld2" name="filename" type="text" id="filename" size="40" value="<?=$a_list['filename'] ?>" /> <br />
				<span class="vexpl"> The list name may only consist of the characters a-z, A-Z and 0-9. <span class="red">Note: </span> No Spaces. </span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Description</td>
				<td width="78%" class="vtable">
				<input class="formfld2" name="description" type="text" id="description" size="40" value="<?=$a_list['description'] ?>" /> <br />
				<span class="vexpl"> You may enter a description here for your reference (not parsed). </span>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic"> 
				Examples:
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="vncell2">
				<b>Example 1;</b> suppress gen_id 1, sig_id 1852, track by_src, ip 10.1.1.54<br>
				<b>Example 2;</b> event_filter gen_id 1, sig_id 1851, type limit,track by_src, count 1, seconds 60<br>
				<b>Example 3;</b> rate_filter gen_id 135, sig_id 1, track by_src, count 100, seconds 1, new_action log, timeout 10
				</td>
			</tr>
		</table>
		<table width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td colspan="2" valign="top" class="listtopic"> 
				Apply suppression or filters to rules. Valid keywords are 'suppress', 'event_filter' and 'rate_filter'.
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="vncelltextbox">
				<textarea wrap="off" name="suppresspassthru" cols="101" rows="28" id="suppresspassthru" class="formfld2"><?=base64_decode($a_list['suppresspassthru']); ?></textarea>
				</td>
			</tr>
		</table>
			<tr>
				<td style="padding-left: 160px;">
				<input name="Submit" type="submit" class="formbtn" value="Save">
				<input id="cancel" type="button" class="formbtn" value="Cancel">
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
