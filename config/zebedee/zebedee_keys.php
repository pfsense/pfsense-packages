<?php
/*
	zebedee_keys.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2010 Scott Ullrich <sullrich@gmail.com>
	Copyright (C) 2010 Marcello Coutinho
	Copyright (C) 2010 Jorge Lustosa
	
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



require("guiconfig.inc");

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

$pgtitle = "Zebedee Tunneling";
include("head.inc");

error_reporting(0); 
?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<?php endif; ?>

<?php if ($savemsg) print_info_box($savemsg); ?>

<form action="varnishstat_view_config.php" method="post">
	
<div id="mainlevel">
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr><td>
		
		
		
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=zebedee.xml&amp;id=0");
	$tab_array[] = array(gettext("Tunnels"), false, "/pkg_edit.php?xml=zebedee_tunnels.xml&amp;id=0");
	$tab_array[] = array(gettext("Keys"), true, "/zebedee_keys.php");
	$tab_array[] = array(gettext("XMLRPC Sync"), false, "/pkg_edit.php?xml=zebedee_sync.xml&amp;id=0");
	$tab_array[] = array(gettext("View Configuration"), false, "/zebedee_view_config.php");
	$tab_array[] = array(gettext("View log files"), false, "/zebedee_log.php");
	display_top_tabs($tab_array);
	
	$zebede_keys = $config['installedpackages']['zebedeekeys']['config'] ; 
	$next_row = sizeof($zebede_keys) ;
	if($next_row == 1 && !array_key_exists("config", $config['installedpackages']["zebedeekeys"]))$next_row =0 ;
	
	//echo "<pre>" ;  
	//print_r($config['installedpackages']); 
?>
		</td>
		</tr>
 		<tr>
    		<td>
				<div id="mainarea">
				
				<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr> 
                  <td class="listhdrr"><?=gettext("Identifier"); ?></td>
                  <td class="listhdr"><?=gettext("Public key"); ?></td>
                  <td class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			    <tr>
			    	<td width="20" heigth="17"></td>
			        <td width="20" heigth="17"></td>
			        <td width="20" heigth="17"></td>
			    
				<td align="left"><a href="pkg_edit.php?xml=zebedee_key_details.xml&id=<?php echo $next_row?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="<?=gettext("add key"); ?>" width="17" height="17" border="0"></a></td>
			    </tr>
			</table>
		  </td>
		</tr>
			  <?php $i = 0; foreach ($zebede_keys as $key): ?>
		<tr>
		<td class="listlr">
			<?=htmlspecialchars($key['ident']);?>
		</td>
		<td class="listr">
			<?=htmlspecialchars($key['public_key']);?>
		</td>
		<td class="list" nowrap>
		<a href="pkg_edit.php?xml=zebedee_key_details.xml&id=<?php echo $i?>">
		<img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" title="<?=gettext("edit key"); ?>" width="17" height="17" border="0"></a>
		<a href="/zebedee_del_key.php?id=<?php echo $i?>"><img height="17" border="0" width="17" src="./themes/pfsense_ng/images/icons/icon_x.gif"></a>
		<a alt="Download client.zbd file"  href="/zebedee_get_key.php?id=<?php echo $i?>" target="_blank"><img height="17" border="0" width="17" src="./themes/pfsense_ng/images/icons/icon_right.gif" alt="Download client.zbd file"></a>
		</td>
				</tr>
			  <?php $i++; endforeach; ?>

			  
                <tr> 
                  <td class="list" colspan="2"></td>
                  <td class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			    <tr>
			        <td width="20" heigth="17"></td>
			        <td width="20" heigth="17"></td>
			        <td width="20" heigth="17"></td>
			        
				<td><a href="pkg_edit.php?xml=zebedee_key_details.xml&id=<?php echo $next_row?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="<?=gettext("add key"); ?>" width="17" height="17" border="0"></a></td>
			    </tr>
			</table>
		  </td>
		</tr>
              </table>
				

				</div>
			</td>
		</tr>
	</table>
</div>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
