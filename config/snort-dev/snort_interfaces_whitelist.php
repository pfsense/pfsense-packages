<?php
/* $Id$ */
/*
 snort_interfaces.php
 part of m0n0wall (http://m0n0.ch/wall)

 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 All rights reserved.
 
 Modified for the Snaort Package By 
 Copyright (C) 2008-2011 Robert Zelaya.
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


$a_whitelist = snortSql_fetchAllWhitelistTypes('SnortWhitelist');

  if ($a_whitelist == 'Error') {
    echo 'Error';
    exit(0);
  }

	$pgtitle = "Services: Snort: Whitelist";
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
			<li><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
			<li class="newtabmenu_active"><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
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
						
			<tr  id="maintable" data-options='{"pagetable":"SnortWhitelist"}'> <!-- db to lookup -->
				<td width="20%" class="listhdrr">File Name</td>
				<td width="45%" class="listhdrr">Values</td>
				<td width="35%" class="listhdr">Description</td>
				<td width="10%" class="list"></td>
			</tr>
			<?php $i = 1; foreach ($a_whitelist as $list): ?>
			<tr id="rowlist_<?=$i;?>" class="icon_xrow" data-options='{"rowuuid":"<?=$list['uuid'];?>"}' >
				<td id="rowfilename<?=$i;?>" class="listlr" ondblclick="document.location='snort_interfaces_whitelist_edit.php?id=<?=$i;?>';"><?=$list['filename'];?></td>
				<td class="listr" ondblclick="document.location='snort_interfaces_whitelist_edit.php?id=<?=$i;?>';">
					<?php
            $a = 0;
            $countList = count($list['list']);
            foreach ($list['list'] as $value) 
            {
            
              $a++;
              
              if ($a != $countList || $countList == 1) 
              {
                echo $value['ip'];
              }
              
               if ($a > 0 && $a != $countList) 
              {
                echo ',' . ' ';
              }else{
                echo ' ';
              }
              
            } // end foreach
            
            if ($a > 3)
            {
              echo '...';
            }         
					?>	
				</td>
				<td class="listbg" ondblclick="document.location='snort_interfaces_whitelist_edit.php?id=<?=$i;?>';">
				<font color="#FFFFFF"> <?=htmlspecialchars($list['description']);?>&nbsp;
				</td>
				<td valign="middle" nowrap class="list">
				<table border="0" cellspacing="0" cellpadding="1">
					<tr>
						<td valign="middle">
						<a href="snort_interfaces_whitelist_edit.php?uuid=<?=$list['uuid'];?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif"width="17" height="17" border="0" title="edit whitelist"></a>
						</td>
						<td>
						<img id="icon_x_<?=$i;?>" class="icon_click icon_x" src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0" title="delete list" >
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
				<td valign="middle"><a href="snort_interfaces_whitelist_edit.php?uuid=<?=genAlphaNumMixFast(28, 28);?> "><img src="/themes/nervecenter/images/icons/icon_plus.gif" width="17" height="17" border="0" title="add a new list"></a></td>
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
</div>


<!-- footer do not touch below -->
<?php 
include("fend.inc"); 
echo $snort_custom_rnd_box;
?>


</body>
</html>
