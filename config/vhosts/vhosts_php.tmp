<?php
/* $Id$ */
/*
	vhosts_php.php
	Copyright (C) 2008 Mark J Crane
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
require("/usr/local/pkg/vhosts.inc");

$a_vhosts = &$config['installedpackages']['vhosts']['config'];


include("head.inc");

?>


<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle">vHosts: Web Server</p>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabnavtbl">
<?php

	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), true, "/packages/vhosts/vhosts_php.php");
	display_top_tabs($tab_array);

?>
</td></tr>
</table>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
<td class="tabcont" >

	<form action="vhosts_php.php" method="post" name="iform" id="iform">
	<?php 

	if ($config_change == 1) {
		write_config();
		$config_change = 0;  
	}

	//if ($savemsg) print_info_box($savemsg); 
	//if (file_exists($d_hostsdirty_path)): echo"<p>";
	//print_info_box_np("This is an info box.");
	//echo"<br />";
	//endif; 

	?>
		<table width="100%" border="0" cellpadding="6" cellspacing="0">              
			<tr>
				<td><p><!--<span class="vexpl"><span class="red"><strong>PHP Service<br></strong></span>-->
				vHosts is a web server package that can host HTML, Javascript, CSS, and PHP. It creates another instance of the lighttpd web server 
				that is already installed. It uses PHP5 in FastCGI mode and has access to PHP Data Ojbects and PDO SQLite. To use SFTP enable SSH from 
				System -> Advanced -> Enable Secure Shell. Then SFTP can be used to access the files at /usr/local/vhosts.
				After adding or updating an entry make sure to restart the <a href='/status_services.php'>service</a> to apply the settings.
				<br /><br />
				For more information see: <a href='https://doc.pfsense.org/index.php/vhosts'>https://doc.pfsense.org/index.php/vhosts</a>            
				</p></td>
			</tr>
		</table>
		<br />

		<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td width="30%" class="listhdrr">Host</td>
			<td width="20" class="listhdrr">Port</td>
			<td width="20" class="listhdrr">SSL</td>
			<td width="20" class="listhdrr">Enabled</td>
			<td width="40%" class="listhdr">Description</td>
			<td width="10%" class="list">
				<table border="0" cellspacing="0" cellpadding="1">
				  <tr>
					<td width="17"></td>
					<td valign="middle"><a href="vhosts_php_edit.php"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
				  </tr>
				</table>
			</td>
		</tr>

		<?php 
		$i = 0;
		if (count($a_vhosts) > 0) {
			//sort array
				if (!function_exists('sort_host')) {
					function sort_host($a, $b){
						return strcmp($a["host"], $b["host"]);
					}
				}
				//disable for now because it throws off the edit and delete
				//if (count($a_vhosts) > 1) {
				//	usort($a_vhosts, 'sort_host');
				//}
			foreach ($a_vhosts as $ent) {
				$host = $ent['host'];
				$port = $ent['port'];
				if (strlen($ent['certificate']) == 0) { $http_protocol = 'http'; } else { $http_protocol = 'https'; }
				if ($http_protocol == 'http' && $port == '80') { $port = ''; }
				if ($http_protocol == 'https' && $port == '443') { $port = ''; }
				if (strlen($port) > 0) { $port = ':'.$port; }
				$vhost_url = $http_protocol.'://'.$host.$port;
				?>
				<tr>
					<td class="listr" ondblclick="document.location='vhosts_php_edit.php?id=<?=$i;?>';">
					  <a href='<?=$vhost_url;?>' target='_blank'><?=$ent['host'];?></a>&nbsp;
					</td>
					<td class="listr" ondblclick="document.location='vhosts_php_edit.php?id=<?=$i;?>';">
					  <a href='<?=$vhost_url;?>' target='_blank'><?=$ent['port'];?></a>&nbsp;
					</td>
					<td class="listr" ondblclick="document.location='vhosts_php_edit.php?id=<?=$i;?>';">
						<div align='center'>
						<?php
							if ($http_protocol == "https") {
								echo "x";
							}
							else {
								echo "&nbsp;";
							}
						?>
						</div>
					</td>
					<td class="listr" ondblclick="document.location='vhosts_php_edit.php?id=<?=$i;?>'; align='center' ">
						<?php echo $ent['enabled']; ?>
					</td>
					<td class="listbg" ondblclick="document.location='vhosts_php_edit.php?id=<?=$i;?>';">
					  <font color="#FFFFFF"><?=htmlspecialchars($ent['description']);?>&nbsp;
					</td>
					<td valign="middle" nowrap class="list">
					  <table border="0" cellspacing="0" cellpadding="1">
						<tr>
						  <td valign="middle"><a href="vhosts_php_edit.php?id=<?=$i;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
						  <td><a href="vhosts_php_edit.php?type=php&act=del&id=<?=$i;?>" onclick="return confirm('Do you really want to delete this?')"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
						</tr>
					 </table>
					</td>
				  </tr>
		<?php
				$i++; 
			}
		}
		?>

		<tr>
			<td class="list" colspan="5"></td>
			<td class="list">          
				<table border="0" cellspacing="0" cellpadding="1">
				<tr>
					<td width="17"></td>
					<td valign="middle"><a href="vhosts_php_edit.php"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
				</tr>
				</table>
			</td>
		</tr>

		<tr>
			<td class="list" colspan="3"></td>
			<td class="list"></td>
		</tr>
		</table>

	</form>


	<br>
	<br>
	<br>
	<br>
	<br>
	<br>
	<br>
	<br>

</td>
</tr>
</table>

</div>


<?php include("fend.inc"); ?>
</body>
</html>
