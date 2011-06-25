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
if (isset($_GET['uuid'])) {
	$uuid = $_GET['uuid'];
}

if (isset($_GET['rdbuuid'])) {
	$rdbuuid = $_GET['rdbuuid'];
}else{
	$ruledbname_pre1 = snortSql_fetchAllSettings('snortDB', 'SnortIfaces', 'uuid', $uuid);
	$rdbuuid = $ruledbname_pre1['ruledbname'];
}

$a_list = snortSql_fetchAllSettings('snortDBrules', 'Snortrules', 'uuid', $rdbuuid);


// $a_list returns empty use defaults
if ($a_list == '')
{
  
  $a_list = array(
      'id' => '',
      'date' => date(U),
      'uuid' => $rdbuuid,
      'ruledbnamename' => '',
      'description' => ''

  );
  
}




	$pgtitle = 'Services: Snort: Rules: Edit: ' . $rdbuuid;
	include('/usr/local/pkg/snort/snort_head.inc');

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

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>
		<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
			<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
			<li><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
			<li><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
			<li class="newtabmenu_active"><a href="/snort/snort_interfaces_rules.php"><span>RulesDB</span></a></li>
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
		<li class="hide_newtabmenu newtabmenu_active"><a href="/snort/snort_interfaces_rules.php?rdbuuid=<?=$rdbuuid;?>"><span>Rules DB Edit</span></a></li>
		<li class="hide_newtabmenu"><a href="/snort/snort_rulesets.php?rdbuuid=<?=$rdbuuid;?>"><span>Categories</span></a></li>
		<li class="hide_newtabmenu"><a href="/snort/snort_rules.php?rdbuuid=<?=$rdbuuid;?>"><span>Rules</span></a></li>
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
		<form id="iform">
		<input name="snortSaveSettings" type="hidden" value="1" />
		<input name="ifaceTab" type="hidden" value="snort_interfaces_rules_edit" />
		<input type="hidden" name="dbName" value="snortDBrules" /> <!-- what db -->
        <input type="hidden" name="dbTable" value="Snortrules" /> <!-- what db table -->
		<input name="date" type="hidden" value="<?=$a_list['date'];?>" />
		<input name="uuid" type="hidden" value="<?=$a_list['uuid'];?>" />		
	
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
			<tr>
				<td colspan="2" valign="top" class="listtopic">Add the name and description of the rule DB</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncellreq2">RuleDB</td>
				<td width="22%" valign="top" class="vtable">
					&nbsp; 
					<input name="enable" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['enable'] == 'on' || $a_list['enable'] == '' ? 'checked' : '';?> ">
					&nbsp;&nbsp;<span class="vexpl">Enable or Disable</span>
				</td>
			</tr>			
			<tr>
				<td valign="top" class="vncellreq2">Name</td>
				<td class="vtable">
				<input class="formfld2" name="ruledbname" type="text" id="ruledbname" size="40" value="<?=$a_list['ruledbname'] ?>" /> <br />
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
				<span class="red"><b>NOTE: </b></span>Rule DB will not be active until snort sensor restart. <br>
				</td>
			</tr>
		</table>
			<tr>
				<td style="padding-left: 10px;">
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
