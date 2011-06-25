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


$a_suppress = snortSql_fetchAllWhitelistTypes('SnortSuppress', '');

	if (!is_array($a_suppress))
	{
		$a_suppress = array();
	}


	if ($a_suppress == 'Error') 
	{
		echo 'Error';
		exit(0);
	}

	$pgtitle = "Services: Snort: Suppression";
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
						
			<tr> <!-- db to lookup -->
				<td width="30%" class="listhdrr">File Name</td>
				<td width="70%" class="listhdr">Description</td>
				<td width="10%" class="list"></td>
			</tr>
			<?php foreach ($a_suppress as $list): ?>
			<tr id="maintable_<?=$list['uuid']?>" data-options='{"pagetable":"SnortSuppress", "pagedb":"snortDB", "DoPOST":"true"}' >
				<td class="listlr" ondblclick="document.location='snort_interfaces_suppress_edit.php?uuid=<?=$list['uuid'];?>'"><?=$list['filename'];?></td>
				<td class="listbg" ondblclick="document.location='snort_interfaces_suppress_edit.php?uuid=<?=$list['uuid'];?>'">
				<font color="#FFFFFF"> <?=htmlspecialchars($list['description']);?>&nbsp;
				</td>
				<td></td>
				<td valign="middle" nowrap class="list">
				<table border="0" cellspacing="0" cellpadding="1">
					<tr>
						<td valign="middle">
						<a href="snort_interfaces_suppress_edit.php?uuid=<?=$list['uuid'];?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif"width="17" height="17" border="0" title="edit suppress list"></a>
						</td>
						<td>
						<img id="icon_x_<?=$list['uuid'];?>" class="icon_click icon_x" src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0" title="delete list" >
						</a>
						</td>
					</tr>
				</table>
				</td>
			</tr>
			<?php $i++; endforeach; ?>
			<tr>
				<td class="list" colspan="3"></td>
				<td class="list">
				<table border="0" cellspacing="0" cellpadding="1">
			<tr>
				<td valign="middle" width="17">&nbsp;</td>
				<td valign="middle"><a href="snort_interfaces_suppress_edit.php?uuid=<?=genAlphaNumMixFast(28, 28);?> "><img src="/themes/nervecenter/images/icons/icon_plus.gif" width="17" height="17" border="0" title="add a new list"></a></td>
					</tr>
				</table>
				</td>
			</tr>
				</table>				
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

<!-- 2nd box note -->
<br>
<div id=mainarea4>
<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
	<td width="100%">
	<span class="vexpl">
	<span class="red"><strong>Note:</strong></span>
	<p><span class="vexpl">
		Here you can create event filtering and suppression for your snort package rules.<br>
		Please note that you must restart a running rule so that changes can take effect.<br>
	</span></p>
	</td>
</table>
</div>

</div>


<!-- footer do not touch below -->
<?php 
include("fend.inc"); 
echo $snort_custom_rnd_box;
?>


</body>
</html>
