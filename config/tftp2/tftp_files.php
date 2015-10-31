<?php
/*
	tftp_files.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2008 Mark J Crane
	Copyright (C) 2011 Jim Pingle
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
require_once("guiconfig.inc");
require_once("util.inc");
require_once("/usr/local/pkg/tftp.inc");

$pconfig['tftpdinterface'] = explode(",", $config['installedpackages']['tftpd']['config'][0]['tftpdinterface']);
$backup_dir = "/root/backup";
$backup_filename = "tftp.bak.tgz";
$backup_path = "{$backup_dir}/{$backup_filename}";
$files_dir = "/tftpboot";
$filename = $_GET['filename'];
$download_dir = $files_dir;

if (($_GET['a'] == "download") && $_GET['t'] == "backup") {
	conf_mount_rw();
	$filename = $backup_filename;
	$download_dir = $backup_dir;
	mwexec("/usr/bin/tar -czC / -f {$backup_path} tftpboot");
	conf_mount_ro();
}

if (($_GET['a'] == "download") && file_exists("{$download_dir}/{$filename}")) {

	session_cache_limiter('public');
	$fd = fopen("{$download_dir}/{$filename}", "rb");
	header("Content-Type: application/force-download");
	header("Content-Type: application/octet-stream");
	header("Content-Type: application/download");
	header("Content-Description: File Transfer");
	header("Content-Disposition: attachment; filename=\"{$filename}\"");
	/* HTTP/1.1 */
	header("Cache-Control: no-cache, must-revalidate");
	/* Date in the past */
	header("Expires: Sat, 26 Jul 1997 05:00:00 GMT");
	header("Content-Length: " . filesize("{$download_dir}/{$filename}"));
	fpassthru($fd);
	exit;
}


if ($_GET['a'] == "other") {
	if ($_GET['t'] == "restore") {

		//extract a specific directory to /tftpboot
		if (file_exists($backup_path)) {
			//echo "The file $filename exists";
			conf_mount_rw();
			mwexec("/usr/bin/tar -xpzC / -f {$backup_path}");
			mwexec("/bin/chmod -R 744 {$files_dir}/*");
			header( 'Location: tftp_files.php?savemsg=Backup+has+been+restored.' ) ;
			conf_mount_ro();
		} else {
			header( 'Location: tftp_files.php?savemsg=Restore+failed.+Backup+file+not+found.' ) ;
		}
		exit;
	}
}
if ($_POST['submit'] == "Save") {
	if (($_POST['tftpdinterface']) != "") {
		$config['installedpackages']['tftpd']['config'][0]['tftpdinterface'] = implode(",", $_POST['tftpdinterface']);
		$pconfig['tftpdinterface'] = $_POST['tftpdinterface'];
		write_config();
		send_event("filter reload");
	} else {
		unset($config['installedpackages']['tftpd']['config'][0]['tftpdinterface']);
		write_config();
		send_event("filter reload");
	}
}

if (($_POST['submit'] == "Upload") && is_uploaded_file($_FILES['ulfile']['tmp_name'])) {
	conf_mount_rw();
	move_uploaded_file($_FILES['ulfile']['tmp_name'], "{$files_dir}/{$_FILES['ulfile']['name']}");
	$savemsg = "Uploaded file to {$files_dir}/" . htmlentities($_FILES['ulfile']['name']);
	mwexec('/bin/chmod -R 744 {$files_dir}/*');
	unset($_POST['txtCommand']);
	conf_mount_ro();
}


if ($_GET['act'] == "del") {
	if ($_GET['type'] == 'tftp') {
		conf_mount_rw();
		unlink_if_exists("{$files_dir}/".$_GET['filename']);
		conf_mount_ro();
		header("Location: tftp_files.php");
		exit;
	}
}

include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle">TFTP: Settings/Files</p>

<?php
$savemsg = $_GET["savemsg"];
if ($savemsg) {
	print_info_box($savemsg);
}
?>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabnavtbl">
<?php

	$tab_array = array();
	$tab_array[] = array(gettext("TFTP"), false, "tftp_files.php");
	display_top_tabs($tab_array);

?>
</td></tr>
</table>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<div id="mainarea">
	<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
	<tr>
		<td  colspan="2" class="listtopic">TFTP Daemon Interfaces</td>
	</tr>
	<tr>
		<td width="22%" class="vncell">Select TFTP Daemon Interface(s).</td>
		<td width="78%" class="vtable">
			<form action="tftp_files.php" method="post" enctype="multipart/form-data" name="frmInterfaces" onsubmit="">
			<select name="tftpdinterface[]" multiple="multiple" class="formselect" size="3">
			<?php
				$ifdescs = get_configured_interface_with_descr();
				foreach ($ifdescs as $ifent => $ifdesc) {
					$selected = "";
					if (in_array($ifent, $pconfig['tftpdinterface'])) {
						$selected = " selected=\"selected\"";
					}
					echo "\t\t\t<option value=\"{$ifent}\"{$selected}>" . $ifdesc . "</option>\n";
				}
			?>
			</select><br /><br />
			<span class="vexpl">
			Choose the interfaces where you want the TFTP Daemon to accept connections.<br/>
			<strong>Hint:</strong> If you simply need to (re)start tftpd/inetd, just use the Save button without making any changes here.<br /><br />
			</span>
			<input name="submit" type="submit" class="formbtns" id="save" value="Save" title="Save settings" />
			</form>
		</td>
	</tr>

	<tr>
		<td colspan="2" class="listtopic">TFTP Files Upload</td>
	</tr>
	<tr>
		<td width="22%" class="vncell">Use the file upload to add files to the /tftpboot directory.</td>
		<td width="78%" class="vtable">
			<form action="tftp_files.php" method="post" enctype="multipart/form-data" name="frmUpload" onsubmit="">
			<span class="vexpl">1. Select file to upload:&nbsp;</span>
			<input name="ulfile" type="file" class="formbtns" id="ulfile" title="Select file to upload" />
			<br />
			<span class="vexpl">2. Upload the selected file:&nbsp;</span>
			<input name="submit" type="submit" class="formbtns" id="upload" value="Upload" title="Upload file" />
			</form>
		</td>
	</tr>

	<tr>
		<td colspan="3" class="listtopic">TFTP Files Download</td>
	</tr>
	<tr>
		<td colspan="3" class="vexpl">Click on the file from the file list below to download it.</td>
	</tr>
	<tr>
		<td colspan="3">
			<table width="100%" border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width="25%" class="listhdrr">File Name (download)</td>
					<td width="50%" class="listhdr">Last Modified</td>
					<td width="50%" class="listhdr">Size</td>
				</tr>

<?php
if ($handle = opendir('/tftpboot')) {
	while (false !== ($file = readdir($handle))) {
		if ($file != "." && $file != "..") {
			$tftp_filesize = filesize("/tftpboot/{$file}");
			$tftp_filesize = tftp_byte_convert($tftp_filesize);
			echo "<tr>\n";
			echo "<td class=\"listlr\" ondblclick=\"\">";
			echo "<a href=\"tftp_files.php?a=download&amp;filename={$file}\">{$file}</a>";
			echo "</td>\n";
			echo "<td class=\"listlr\" ondblclick=\"\">";
			echo "\t\t" . date("F d Y H:i:s", filemtime('/tftpboot/'. $file));
			echo "</td>\n";
			echo "<td class=\"listlr\" ondblclick=\"\">";
			echo "\t\t{$tftp_filesize}";
			echo "</td>\n";
			echo "<td valign=\"middle\" nowrap=\"nowrap\" class=\"list\">\n";
			echo "<table border=\"0\" cellspacing=\"0\" cellpadding=\"1\">\n";
			echo "<tr>\n";
			echo "<td valign=\"middle\">\n";
			echo "\t<form method=\"post\" action=\"/edit.php\" target=\"_blank\"><input type=\"hidden\" name=\"savetopath\" value=\"/tftpboot/{$file}\" />\n";
			echo "\t\t<input type=\"hidden\" name=\"submit\" value=\"Load\" />\n";
			echo "\t\t<input type='image' src=\"/themes/" . $g['theme'] . "/images/icons/icon_e.gif\" />\n";
			echo "\t</form>\n";
			echo "</td>\n";
			echo "<td><a href=\"tftp_files.php?type=tftp&amp;act=del&amp;filename={$file}\" onclick=\"return confirm('Do you really want to delete this file?')\">";
			echo "<img src=\"/themes/" . $g['theme'] . "/images/icons/icon_x.gif\" alt=\"\" width=\"17\" height=\"17\" border=\"0\" /></a>";
			echo "</td>\n";
			echo "</tr>\n";
			echo "</table>\n";
			echo "</td>\n";
			echo "</tr>\n";
		}
	}
	closedir($handle);
}
?>

				<tr>
					<td class="list" colspan="3"></td>
					<td class="list"></td>
				</tr>
			</table>
		</td>
	</tr>

	<tr>
		<td colspan="2" class="listtopic">TFTP Backup / Restore</td>
	</tr>
	<tr>
		<td width="22%" class="vncell">Use the file upload to add files to the /tftpboot directory.</td>
		<td width="78%" class="vtable">
			<span class="vexpl"><strong>Backup:</strong> 'Backup' button compresses /tftpboot/ to /root/backup/tftp.bak.tgz; after that it presents the backup for download.<br />
			<strong>Restore:</strong> If the backup file does not exist in /root/backup/tftp.bak.tgz then the 'Restore' button will be hidden.
			To 'Restore' a previously downloaded backup, use <a href="diag_backup.php" title="Upload file">Diagnostics -> Command Prompt -> File to upload</a> to browse to the file and then click on upload.<br />
			After that, backup will be ready to be restored.<br /><br />
			</span>
			<input type="button" value="Backup" title="Create backup" onclick="document.location.href='tftp_files.php?a=download&amp;t=backup'" />
			<?php
				if (file_exists('/root/backup/tftp.bak.tgz')) {
					echo "<input type='button' value='Restore' title='Restore backup' onclick=\"document.location.href='tftp_files.php?a=other&amp;t=restore';\" />\n";
				}
			?>
		</td>
	</tr>
	</table>

</div>
</td></tr>
</table>
</div>

<?php include("fend.inc"); ?>
</body>
</html>
