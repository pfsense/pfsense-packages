<?php
/* $Id$ */
/*
	backup.php
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
require("/usr/local/pkg/backup.inc");

$a_backup = &$config['installedpackages']['backup']['config'];

if ($_GET['act'] == "del") {
	if ($_GET['type'] == 'backup') {
		if ($a_backup[$_GET['id']]) {
			conf_mount_rw();
			unset($a_backup[$_GET['id']]);
			write_config();
			header("Location: backup.php");
			conf_mount_ro();
			exit;
		}
	}
}

if ($_GET['a'] == "download") {
	if ($_GET['t'] == "backup") {
		conf_mount_rw();

		$tmp = '/root/backup/';
		$filename = 'pfsense.bak.tgz';
		//system('cd /usr/local/;tar cvzf /root/backup/pfsense.bak.tgz freeswitch');

		$i = 0;
		if (count($a_backup) > 0) {
			$backup_cmd = 'tar --create --verbose --gzip --file '.$tmp.$filename.' --directory / ';
			foreach ($a_backup as $ent) {
				if ($ent['enabled'] == "true") {
					//htmlspecialchars($ent['name']);
					//htmlspecialchars($ent['path']);
					//htmlspecialchars($ent['description']);
					$backup_cmd .= htmlspecialchars($ent['path']).' ';
				}
				$i++;
			}
			//echo $backup_cmd; //exit;
			system($backup_cmd);
		}

		session_cache_limiter('public');
		$fd = fopen($tmp.$filename, "rb");
		header("Content-Type: binary/octet-stream");
		header("Content-Length: " . filesize($tmp.$filename));
		header('Content-Disposition: attachment; filename="' . $filename . '"');
		fpassthru($fd);

		conf_mount_ro();
		exit;
	}
}

if ($_GET['a'] == "other") {
	if ($_GET['t'] == "restore") {
		conf_mount_rw();
		$tmp = '/root/backup/';
		$filename = 'pfsense.bak.tgz';

		//extract the tgz file
		if (file_exists('/root/backup/'.$filename)) {
			//echo "The file $filename exists";
			system('cd /; tar xvpfz /root/backup/'.$filename.' ');
			header( 'Location: backup.php?savemsg=Backup+has+been+restored.' ) ;
		} else {
			header( 'Location: backup.php?savemsg=Restore+failed.+Backup+file+not+found.' ) ;
		}
		conf_mount_ro();
		exit;
	}
}

if (($_POST['submit'] == "Upload") && is_uploaded_file($_FILES['ulfile']['tmp_name'])) {
	conf_mount_rw();
	$filename = 'pfsense.bak.tgz';
	move_uploaded_file($_FILES['ulfile']['tmp_name'], "/root/backup/" . $filename);
	$savemsg = "Uploaded file to /root/backup/" . htmlentities($_FILES['ulfile']['name']);
	system('cd /; tar xvpfz /root/backup/'.$filename.' ');
	conf_mount_ro();
}

include("head.inc");

?>


<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle">Backup: Files & Directories</p>

<?php
if ($_GET["savemsg"]) {
	print_info_box($_GET["savemsg"]);
}
?>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabnavtbl">
<?php

	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), true, "/packages/backup/backup.php");
 	display_top_tabs($tab_array);

?>
</td></tr>
</table>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
	<td class="tabcont" >

	<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>
			<p>
			Use this to tool to backup files and directories. The following directories
			are recommended for backup.

			<table>
			<tr><td><strong>pfSense Config</strong></td><td>/cf/conf</td></tr>
			<tr><td><strong>RRD Graph Data Files</strong></td><td>/var/db/rrd</td></tr>
			</table>
			</p>
		</td>
	</tr>
	</table>

	<br/>
	<br/>

	<div id="niftyOutter">

	<form action="backup.php" method="POST" enctype="multipart/form-data" name="frmUpload" onSubmit="">
		<table width='100%' width='690' cellpadding='0' cellspacing='0' border='0'>
		<tr><td align='left' colspan='4'><strong>Upload and Restore</strong></td></tr>
		<tr>
			<td colspan='2'>Use this to upload and restore your backup file.</td>
			<td align="right">File to upload:</td>
			<td width='50%' valign="top" align='right' class="label">
				<input name="ulfile" type="file" class="button" id="ulfile">
			</td>
			<td valign="top" class="label">
				<input name="submit" type="submit"  class="button" id="upload" value="Upload">
			</td>
		</tr>
		</table>
		<br />
		<br />
	</div>
	</form>


<?php
	echo "<table width='690' cellpadding='0' cellspacing='0' border='0'>\n";
	echo "<tr>\n";
	echo "<td width='80%'>\n";
	echo "<b>Backup / Restore</b><br />\n";
	echo "The 'backup' button will tar gzip the directories that are listed below to /root/backup/pfsense.bak.tgz it then presents a file to download. \n";
	echo "If the backup file does not exist in /root/backup/pfsense.bak.tgz then the 'restore' button will be hidden. \n";
	echo "<br /><br />\n";
	echo "</td>\n";
	echo "<td width='20%' valign='middle' align='right'>\n";
	echo "  <input type='button' value='backup' onclick=\"document.location.href='backup.php?a=download&t=backup';\" />\n";
	if (file_exists('/root/backup/pfsense.bak.tgz')) {
	  echo "  <input type='button' value='restore' onclick=\"document.location.href='backup.php?a=other&t=restore';\" />\n";
	}
	echo "</td>\n";
	echo "</tr>\n";
	echo "</table>\n";
	echo "<br /><br />\n\n";


echo "  <form action='backup.php' method='post' name='iform' id='iform'>\n";


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

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td width="30%" class="listhdrr">Name</td>
		<td width="20%" class="listhdrr">Enabled</td>
		<td width="40%" class="listhdr">Description</td>
		<td width="10%" class="list">
			<table border="0" cellspacing="0" cellpadding="1">
				<tr>
					<td width="17"></td>
					<td valign="middle"><a href="backup_edit.php"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
				</tr>
			</table>
		</td>
	</tr>

	<?php

	$i = 0;
	if (count($a_backup) > 0) {

		foreach ($a_backup as $ent) {

	?>
	<tr>
		<td class="listr" ondblclick="document.location='backup_edit.php?id=<?=$i;?>';">
			<?=$ent['name'];?>&nbsp;
		</td>
		<td class="listr" ondblclick="document.location='backup_edit.php?id=<?=$i;?>';">
			<?=$ent['enabled'];?>&nbsp;
		</td>
		<td class="listbg" ondblclick="document.location='backup_edit.php?id=<?=$i;?>';">
			<font color="#FFFFFF"><?=htmlspecialchars($ent['description']);?>&nbsp;
		</td>
		<td valign="middle" nowrap class="list">
			<table border="0" cellspacing="0" cellpadding="1">
				<tr>
					<td valign="middle"><a href="backup_edit.php?id=<?=$i;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
					<td><a href="backup_edit.php?type=backup&act=del&id=<?=$i;?>" onclick="return confirm('Do you really want to delete this?')"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
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
		<td class="list" colspan="3"></td>
		<td class="list">
			<table border="0" cellspacing="0" cellpadding="1">
				<tr>
					<td width="17"></td>
					<td valign="middle"><a href="backup_edit.php"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
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
