<?php
/*
	phpservice_php.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2008 Mark J Crane
	Copyright (C) 2015 ESF, LLC
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
require("/usr/local/pkg/phpservice.inc");

$a_phpservice = &$config['installedpackages']['phpservice']['config'];

if ($_GET['act'] == "del") {
	if ($_GET['type'] == 'php') {
		if ($a_phpservice[$_GET['id']]) {
			unset($a_phpservice[$_GET['id']]);
			write_config();
			header("Location: phpservice_php.php");
			exit;
		}
	}
}

$pgtitle = "PHP Service";
include("head.inc");

?>


<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabnavtbl">
<?php
			
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/packages/phpservice/phpservice_php.php");
 	display_top_tabs($tab_array);
 	
?>
</td></tr>
</table>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabcont" >
	<form action="phpservice_php.php" method="post" name="iform" id="iform">
<?php 
if ($config_change == 1) {
	write_config();
	$config_change = 0;
}
?>
	<table width="100%" border="0" cellpadding="6" cellspacing="0">              
	<tr><td>
		<div>
		The custom PHP code that is defined below is run over and over again inside a continuous loop. There are many possible uses such as monitoring CPU, Memory, File System Space, interacting with Snort, and many others uses that are yet to be discovered.<br />
		It can send events to the sylog that will can be viewed from the system log or remote syslog server. Example: <em>exec("logger This is a test");</em><br /><br />
		For more information see: <a href='https://doc.pfsense.org/index.php/PHPService'>https://doc.pfsense.org/index.php/PHPService</a>
		</div>
	</td></tr>
	</table>
	<br />

	<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td width="30%" class="listhdrr">Name</td>
		<td width="20%" class="listhdrr">Enabled</td>
		<td width="40%" class="listhdr">Description</td>
		<td width="10%" class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			<tr>
				<td width="17"></td>
				<td valign="middle"><a href="phpservice_php_edit.php"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0" alt="" /></a></td>
			</tr>
			</table>
		</td>
	</tr>

	<?php
        $i = 0;
	if (count($a_phpservice) > 0) {
		foreach ($a_phpservice as $ent) {
	?>
        <tr>
		<td class="listr" ondblclick="document.location='phpservice_php_edit.php?id=<?=$i;?>';"><?=$ent['name'];?>&nbsp;</td>
		<td class="listr" ondblclick="document.location='phpservice_php_edit.php?id=<?=$i;?>';"><?=$ent['enabled'];?>&nbsp;</td>
		<td class="listbg" ondblclick="document.location='phpservice_php_edit.php?id=<?=$i;?>';"><?=htmlspecialchars($ent['description']);?>&nbsp;</td>
		<td valign="middle" nowrap class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			<tr>
				<td valign="middle"><a href="phpservice_php_edit.php?id=<?=$i;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0" alt="" /></a></td>
				<td><a href="phpservice_php_edit.php?type=php&amp;act=del&amp;id=<?=$i;?>" onclick="return confirm('Do you really want to delete this?')"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0" alt="" /></a></td>
			</tr>
			</table>
		</td>
	</tr>
	<?php $i++;
		}
	}
	?>

	<tr>
		<td class="list" colspan="3"></td>
		<td class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			<tr>
				<td width="17"></td>
				<td valign="middle"><a href="phpservice_php_edit.php"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0" alt="" /></a></td>
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
<br />

</td></tr>
</table>
</div>

<?php include("fend.inc"); ?>
</body>
</html>
