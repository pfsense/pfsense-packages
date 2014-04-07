<?php
/*
	edit.php
	Copyright (C) 2004, 2005 Scott Ullrich
	Copyright (C) 2013 robi <robreg@zsurob.hu>
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
/*
	pfSense_MODULE:	shell
*/

##|+PRIV
##|*IDENT=page-status-asterisk
##|*NAME=Status: Asterisk config editor page
##|*DESCR=Allow access to the 'Status: Asterisk configuration files' page.
##|*MATCH=asterisk_edit_file.php*
##|-PRIV

$pgtitle = array(gettext("Status"),gettext("Asterisk configuration files"));
require("guiconfig.inc");


$backup_dir = "/conf";
$backup_filename = "asterisk_config.bak.tgz";
$backup_path = "{$backup_dir}/{$backup_filename}";
$files_dir = "/conf/asterisk";
$host = "{$config['system']['hostname']}.{$config['system']['domain']}";
$downname = "asterisk-config-{$host}-".date("YmdHis").".bak.tgz";  //put the date in the filename

if (($_GET['a'] == "download") && $_GET['t'] == "backup") {
	conf_mount_rw();
//	system("cd {$files_dir} && tar czf {$backup_path} *");
	system("cd {$files_dir} && tar czf {$backup_path} --exclude 'dist/*' --exclude dist *");
	conf_mount_ro();
}

if (($_GET['a'] == "download") && file_exists("{$backup_path}")) {
	session_cache_limiter('public');
	$fd = fopen("{$backup_path}", "rb");
	header("Content-Type: application/force-download");
	header("Content-Type: application/octet-stream");
	header("Content-Type: application/download");
	header("Content-Description: File Transfer");
	header("Content-Disposition: attachment; filename=\"{$downname}\"");
	header("Cache-Control: no-cache, must-revalidate"); // HTTP/1.1
	header("Expires: Sat, 26 Jul 1997 05:00:00 GMT"); // Date in the past
	header("Content-Length: " . filesize("{$backup_path}"));
	fpassthru($fd);
	exit;
}

if ($_GET['a'] == "other") {
	if ($_GET['t'] == "restore") {
		//extract files to $files_dir (/conf/asterisk)
		if (file_exists($backup_path)) {
			//echo "The file $filename exists";
			conf_mount_rw();
			exec("tar -xzC {$files_dir} -f {$backup_path} 2>&1", $sysretval);
			$savemsg = "Backup has been restored, please restart Asterisk now " . $sysretval[1];
			system("chmod -R 644 {$files_dir}/*");
			header( 'Location: asterisk_edit_file.php?savemsg=' . $savemsg ) ;
			conf_mount_ro();
		} else {
			header( 'Location: asterisk_edit_file.php?savemsg=Restore+failed.+Backup+file+not+found.' ) ;
		}
		exit;
	}
	if ($_GET['t'] == "factrest") {
		//extract files to $files_dir (/conf/asterisk)
		if (file_exists('/conf.default/asterisk_factory_defaults_config.tgz')) {
			//echo "The file $filename exists";
			conf_mount_rw();
			exec("tar -xzC {$files_dir} -f /conf.default/asterisk_factory_defaults_config.tgz 2>&1", $sysretval);
			$savemsg = "Factory configuration restored, please restart Asterisk now " . $sysretval[1];
			system("chmod -R 644 {$files_dir}/*");
			header( 'Location: asterisk_edit_file.php?savemsg=' . $savemsg ) ;
			conf_mount_ro();
		}
		exit;
	}
	if ($_GET['t'] == "deldist") {
		//delete dist directory from $files_dir/dist (/conf/asterisk/dist)
		if (file_exists($files_dir . "/dist")) {
			conf_mount_rw();
			exec("rm -r {$files_dir}/dist 2>&1", $sysretval);
			$savemsg = "Deleted dist files " . $sysretval[1];
			header( 'Location: asterisk_edit_file.php?savemsg=' . $savemsg ) ;
			conf_mount_ro();
		}
		exit;
	}
}

if (($_POST['submit'] == "Upload") && is_uploaded_file($_FILES['ulfile']['tmp_name'])) {
	$upfilnam = $_FILES['ulfile']['name'];
	$upfiltim = strtotime(str_replace(".bak.tgz","",end(explode("-",$upfilnam))));
	conf_mount_rw();
	move_uploaded_file($_FILES['ulfile']['tmp_name'], "{$backup_path}");
	$savemsg = "Uploaded ". htmlentities($_FILES['ulfile']['name']) . " file as " . $backup_path . "." ;
	system('chmod -R 644 {$backup_path}');
	if ($upfiltim) {		//take the date from the filename and update modified time accordingly
		touch($backup_path, $upfiltim);
	}
	unset($_POST['txtCommand']);
	conf_mount_ro();
	header( 'Location: asterisk_edit_file.php?savemsg=' . $savemsg ) ;
}

if($_REQUEST['action']) {
	switch($_REQUEST['action']) {
		case 'load':
			if(strlen($_REQUEST['file']) < 1) {
				echo "|5|" . gettext("No file name specified") . ".|";
			} elseif(is_dir($_REQUEST['file'])) {
				echo "|4|" . gettext("Loading a directory is not supported") . ".|";
			} elseif(! is_file($_REQUEST['file'])) {
				echo "|3|" . gettext("File does not exist or is not a regular file") . ".|";
			} else {
				$data = file_get_contents(urldecode($_REQUEST['file']));
				if($data === false) {
					echo "|1|" . gettext("Failed to read file") . ".|";
				} else {
					echo "|0|{$_REQUEST['file']}|{$data}|";	
				}
			}
			exit;
		case 'save':
			if(strlen($_REQUEST['file']) < 1) {
				echo "|" . gettext("No file name specified") . ".|";
			} else {
				conf_mount_rw();
				$_REQUEST['data'] = str_replace("\r", "", base64_decode($_REQUEST['data']));
				$ret = file_put_contents($_REQUEST['file'], $_REQUEST['data']);
				conf_mount_ro();
				if($_REQUEST['file'] == "/conf/config.xml" || $_REQUEST['file'] == "/cf/conf/config.xml") {
					if(file_exists("/tmp/config.cache"))
						unlink("/tmp/config.cache");
					disable_security_checks();
				}
				if($ret === false) {
					echo "|" . gettext("Failed to write file") . ".|";
				} elseif($ret <> strlen($_REQUEST['data'])) {
					echo "|" . gettext("Error while writing file") . ".|";
				} else {
					echo "|" . gettext("File successfully saved") . ".|";
				}
			}
			exit;
	}
	exit;
}
$shortcut_section = "asterisk";
require("head.inc");
outputJavaScriptFileInline("filebrowser/browser.js");
outputJavaScriptFileInline("javascript/base64.js");

?>

<body link="#000000" vlink="#000000" alink="#000000">
<?php include("fbegin.inc"); ?>

<?php
$savemsg = $_GET["savemsg"];
if ($savemsg) {
  print_info_box($savemsg);
}
?>

<script type="text/javascript">	
	function loadFile() {
		$("fileStatus").innerHTML = "<?=gettext("Loading file"); ?> ...";
		Effect.Appear("fileStatusBox", { duration: 0.5 });

		new Ajax.Request(
			"<?=$_SERVER['SCRIPT_NAME'];?>", {
				method:     "post",
				postBody:   "action=load&file=" + $("fbTarget").value,
				onComplete: loadComplete
			}
		);
	}

	function loadComplete(req) {
		Element.show("fileContent")
		var values = req.responseText.split("|");
		values.shift(); values.pop();

		if(values.shift() == "0") {
			var file = values.shift();
			$("fileStatus").innerHTML = "<?=gettext("File successfully loaded"); ?>.";
			$("fileContent").value    = values.join("|");

			var lang = "none";
				 if(file.indexOf(".php") > 0) lang = "php";
			else if(file.indexOf(".inc") > 0) lang = "php";
			else if(file.indexOf(".xml") > 0) lang = "xml";
			else if(file.indexOf(".js" ) > 0) lang = "js";
			else if(file.indexOf(".css") > 0) lang = "css";

		}
		else {
			$("fileStatus").innerHTML = values[0];
			$("fileContent").value = "";
		}
		new Effect.Appear("fileContent");
	}

	function saveFile(file) {
		$("fileStatus").innerHTML = "<?=gettext("Saving file"); ?> ...";
		Effect.Appear("fileStatusBox", { duration: 0.5 });
		
		var fileContent = Base64.encode($("fileContent").value);
		fileContent = fileContent.replace(/\+/g,"%2B");
		
		new Ajax.Request(
			"<?=$_SERVER['SCRIPT_NAME'];?>", {
				method:     "post",
				postBody:   "action=save&file=" + $("fbTarget").value +
							"&data=" + fileContent,
				onComplete: function(req) {
					var values = req.responseText.split("|");
					$("fileStatus").innerHTML = values[1];
				}
			}
		);
	}

	

	function ckrest() {
		if(document.getElementById('ckrest').checked==true) {
			document.getElementById('restfactdef').disabled=false;
		} else {
			document.getElementById('restfactdef').disabled=true;
		}
	}

	function ckdist() {
		if(document.getElementById('ckdist').checked==true) {
			document.getElementById('deldistdire').disabled=false;
		} else {
			document.getElementById('deldistdire').disabled=true;
		}
	}
 	
	
</script>

	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td>
				<?php
					$tab_array = array();
					$tab_array[0] = array(gettext("Commands"), false, "asterisk_cmd.php");
					$tab_array[1] = array(gettext("Calls"), false, "asterisk_calls.php");
					$tab_array[2] = array(gettext("Log"), false, "asterisk_log.php");
					$tab_array[3] = array(gettext("Edit configuration"), true, "asterisk_edit_file.php");
					display_top_tabs($tab_array);
				?>
			</td>
		</tr>
		<tr>
			<td>
				<div id="mainarea">

					<!-- backup options -->
					<div style="background:#eeeeee;">
						<div class="vexpl" style="padding-left:15px;">
						<br />
						<table width='98%' cellpadding='0' cellspacing='0' border='0'>
						<tr>
						<td width='80%'>
						<b>Backup / Restore</b>
						The 'Backup' button will tar gzip asterisk configuration files to <? echo $backup_path; ?> it then offers it to download.<br>
						The 'Restore' button will be visible only if the <? echo $backup_path; ?> backup file exists.<br>
						You can upload a backup file to the system, if one already exists at <? echo $backup_path; ?>, it will be overwritten.
						<br />
						</td>
						<td width='20%' valign='middle' align='right'>
						<?php
						echo "  <input type='button' value='Backup' onclick=\"document.location.href='asterisk_edit_file.php?a=download&t=backup';\" />\n";
						if (file_exists($backup_path)) {
							echo "  <input type='button' value='Restore' onclick=\"document.location.href='asterisk_edit_file.php?a=other&t=restore';\" />\n";
						}
						?>
						</td>
						</tr></table><br>
						<table width='98%' cellpadding='0' cellspacing='0' border='0'>
						<tr>
						<td width='20%' valign='middle' align='left'>
						<?php
						if (file_exists($backup_path)) {
							echo $backup_filename . " date:<br>" . date ("Y F d H:i:s.", filemtime($backup_path));
						}
						?>
						</td>
						<td width='80%' valign='middle' align='right'>
						<form action="asterisk_edit_file.php" method="POST" enctype="multipart/form-data" name="frmUpload" onSubmit="">
						Upload backup file:
						<input name="ulfile" type="file" class="button" id="ulfile">
						<input name="submit" type="submit"  class="button" id="upload" value="Upload">
						</form>
						</td>
						</tr>
						</table><br />					
						</div>
					</div>


									
					<table width="100%" border="0" cellpadding="0" cellspacing="0">
						<tr>
							<td class="tabcont" align="center">

					<!-- controls -->
					<table width="100%" cellpadding="9" cellspacing="9">
						<tr>
							<td align="center" class="list">
								<?=gettext("Configuration files stored in"); ?>:
								<input type="text"   class="formfld file" id="fbTarget"         value="<?=gettext($files_dir);?>" size="45" />
								<input type="button" class="formbtn"      id="fbOpen"           value="<?=gettext('Browse');?>" />
					<!--			<input type="button" class="formbtn"      onclick="loadFile();" value="<?=gettext('Load');?>" /> -->
								<input type="button" class="formbtn"      onclick="saveFile();" value="<?=gettext('Save');?>" />
								<br />
							</td>
						</tr>
					</table>

					
					
					<!-- file status box -->
					<div style="display:none; background:#eeeeee;" id="fileStatusBox">
						<div class="vexpl" style="padding-left:15px;">
							<strong id="fileStatus"></strong>
						</div>
					</div>
					
					
					<!-- filebrowser -->
					<div id="fbBrowser" style="display:none; border:1px dashed gray; width:98%;"></div>

					<!-- file viewer/editor -->
					<table width="100%">
						<tr>
							<td valign="top" class="label">
								<div style="background:#eeeeee;" id="fileOutput">
									<textarea id="fileContent" name="fileContent" style="width:100%;" rows="30" wrap="off"></textarea>
								</div>
							</td>
						</tr>
					</table>

							</td>
						</tr>
					</table>

					<script type="text/javascript">
						Event.observe(
							window, "load",
							function() {
								$("fbTarget").focus();

								NiftyCheck();
								Rounded("div#fileStatusBox", "all", "#ffffff", "#eeeeee", "smooth");
							}
						);

						<?php if($_GET['action'] == "load"): ?>
							Event.observe(
								window, "load",
								function() {
									$("fbTarget").value = "<?=$_GET['path'];?>";
									loadFile();
								}
							);
						<?php endif; ?>
					</script>

					
					<div style="background:#eeeeee;">
						<div class="vexpl" style="padding-left:15px;">
						<table width='98%' cellpadding='0' cellspacing='0' border='0'>
						<tr>
						<td width='80%' valign='middle' align='right'><br />
						<?php
							if (file_exists($files_dir . "/dist")) {
								echo "<input name='ckdist' id='ckdist' type='checkbox' onclick='return ckdist();' style='vertical-align:-3px;'>enable <input type='button' value='Delete dist files' name='deldistdire' id='deldistdire' disabled='disabled' onclick=\"document.location.href='asterisk_edit_file.php?a=other&t=deldist';\" />&nbsp;&nbsp;\n";
							}
							if (file_exists("/conf.default/asterisk_factory_defaults_config.tgz")) {
								echo "<input name='ckrest' id='ckrest' type='checkbox' onclick='return ckrest();' style='vertical-align:-3px;'>enable <input type='button' value='Restore to factory defaults' name='restfactdef' id='restfactdef' disabled='disabled' onclick=\"document.location.href='asterisk_edit_file.php?a=other&t=factrest';\" />\n";
							}
						?>
						<br /></td>
						</tr>
						</table><br />			
						</div>
					</div>
					
					
				</div>
			</td>
		</tr>
	</table>

<p/>

<span class="vexpl">
	<span class="red">
		<strong><?=gettext("Note:");?><br /></strong>
	</span>
	<?=gettext("Please back up your Asterisk configuration regularly.");?><br>
	<?=gettext("It's worth to preserve the automatically generated filename of the downloaded backup file. It contains the backup creation date, which is used when uploading it back to the system.");?>
	<?php
	$sipconf=$files_dir . "/sip.conf";
	if (file_exists($sipconf)){
		$sipconf_file=file_get_contents($sipconf);
		if (strpos($sipconf_file,"demo extension for pfSense") !== false) {
			?><br />
			<?=gettext("This Asterisk configuration on pfSense contains two demo SIP accounts, 301 and 302 with password 1234, for you to test functionality. Check sip.conf for more details. These accounts can be safely removed at any time.");?>
			<?php
		}
	}	
	?>
	
</span>
	
<?php include("fend.inc"); ?>
</body>
</html>
