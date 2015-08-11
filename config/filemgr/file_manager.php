<?php
include "rbfminc/config.php";
//include "rbfminc/session.php";
require_once('config.inc');
require("guiconfig.inc");

$closehead = false;
include("head.inc");

global $config;
if('ok' == 'ok'){
	set_time_limit(1800); //30 min
	include "rbfminc/functions.php";

	clearstatcache ();
	$_GET['p'] = urldecode($_GET['p']);

	if($_COOKIE['current_folder']){
		$initial_folder_cookie = $_COOKIE['current_folder'];
	}

	if($_GET['url_field']){
		setcookie('url_field', $_GET['url_field']);
		$url_field = $_GET['url_field'];
	}else{
		$url_field = $_COOKIE['url_field'];
	}

	if($_GET['p']){
		if(substr($_GET['p'], -1) != "/"){$_GET['p'] = $_GET['p']."/";}
		$current_folder = $_GET['p'];
	}elseif($initial_folder_cookie and file_exists($initial_folder_cookie)){
		if(substr($initial_folder_cookie, -1) != "/"){$initial_folder_cookie = $initial_folder_cookie."/";}
		$current_folder = $initial_folder_cookie;
	}elseif($initial_folder and file_exists($initial_folder)){
		if(substr($initial_folder, -1) != "/"){$initial_folder = $initial_folder."/";}
		$current_folder = $initial_folder;
	}else{
		$current_folder = $_SERVER['DOCUMENT_ROOT']."/";
	}


	if($only_below and strlen($current_folder) < strlen($initial_folder)){
		setcookie('current_folder', '', time()-3600);
		header("Location: file_manager.php");
		exit();
	}


	//setcookie('current_folder', $current_folder);

	if(substr($current_folder, 0, strlen($_SERVER['DOCUMENT_ROOT'])) == $_SERVER['DOCUMENT_ROOT']){
		$url_path = "http://".$_SERVER['HTTP_HOST']."/".substr($current_folder, strlen($_SERVER['DOCUMENT_ROOT']));
	}


	if($_POST['save_file'] == 'save_file' and $_GET['f']){
		if($handle = fopen($current_folder.$_GET['f'] , 'w')){
			if (fwrite($handle, stripslashes($_POST['file_content'])) === FALSE) {
				$alert_info .= "Cannot write to file ({$current_folder}{$_GET['f']})";
			}else{
				$alert_info .= "File ({$current_folder}{$_GET['f']}) successfully saved";
				$redirect = "file_manager.php?p=".urlencode($current_folder);
			}
		}else{
			$alert_info .= "Invalid file!!!";
		}
	}


	if($_POST['upload_file'] == 'upload_file'){
		if($_FILES['file']['error'] == 8){
			$alert_info .= "File upload stopped by extension!!!";
		}
		if($_FILES['file']['error'] == 7){
			$alert_info .= "Failed to write file to disk!!!";
		}
		if($_FILES['file']['error'] == 6){
			$alert_info .= "Missing a temporary folder!!!";
		}
		if($_FILES['file']['error'] == 4){
			$alert_info .= "No image was uploaded!!!";
		}
		if($_FILES['file']['error'] == 3){
			$alert_info .= "The uploaded file was only partially uploaded!!!";
		}
		if($_FILES['file']['error'] == 2){
			$alert_info .= "The uploaded file exceeds the MAX_FILE_SIZE!!!";
		}
		if($_FILES['file']['error'] == 1){
			$alert_info .= "The uploaded file exceeds the upload_max_filesize!!!";
		}


		if(!$alert_info){
			if(file_exists($current_folder.$_FILES['file']['name']) and !$_POST['replace_file']){
				$alert_info .= "A file with the same name already exist in this folder\\nTo replace this file check \"Replace existing file\" in upload form!";
				$redirect = "file_manager.php?p=".urlencode($current_folder);
			}else{
				if(!@move_uploaded_file($_FILES["file"]["tmp_name"], $current_folder.$_FILES['file']['name'])){
					$alert_info .= "Failed to upload file!!!";
				}else{
					$alert_info .= "File successfully uploaded!";
					$redirect = "file_manager.php?p=".urlencode($current_folder);
				}
			}
		}
	}

	if($_GET['do'] == 'delete' and $_GET['file'] and $_GET['type'] == 'file'){
		if(file_exists($current_folder.$_GET['file'])){
			if(!@unlink($current_folder.$_GET['file'])){
				$alert_info = "You cannot delete this file\\nThe relevant permissions must permit this.";
			}else{
				$alert_info = "File deleted";
				$redirect = "file_manager.php?p=".urlencode($current_folder);
			}
		}else{
			$alert_info = "You cannot delete this file\\nInvalid file";
		}
	}

	if($_GET['do'] == 'delete' and $_GET['file'] and $_GET['type'] == 'directory'){
		if(file_exists($current_folder.$_GET['file'])){
			if(!RecursiveFolderDelete($current_folder.$_GET['file'])){
				$alert_info = "You cannot delete this directory\\nThe relevant permissions must permit this.";
			}else{
				$alert_info = "Folder deleted";
				$redirect = "file_manager.php?p=".urlencode($current_folder);
			}
		}else{
			$alert_info = "You cannot delete this directory\\nInvalid directory";
		}
	}

	if($_POST['create_folder'] == "Create folder"){
		if(@mkdir($current_folder.$_POST['folder_name'])){
			$alert_info = "Folder created successfully!";
		}else{
			$alert_info = "Invalid folder bane!";
		}
	}


	preg_match_all("/\//", $current_folder, $m);
	if(count($m[0]) > 1){
		$up_one_level = " ondblclick=\"document.location='{$_SERVER['PHP_SELF']}?p=".urlencode(substr($current_folder, 0, strrpos(substr($current_folder, 0, -1), "/"))."/")."'\"";
	}


	if ($handle = @opendir($current_folder)) {
		while (false !== ($folder_content = readdir($handle))) {
			if(is_dir($current_folder.'/'.$folder_content) and $folder_content!='.' and $folder_content!='..'){
				$folders[] = $folder_content;
			}elseif(!is_dir($current_folder.'/'.$folder_content) and $folder!='.' and $folder_content!='..'){
				$files[] = $folder_content;
			}
		}
		closedir($handle);
	}else{
		$error = "<h1 style=\"color:red\" align=\"center\">Invalid directory</h1>";
	}

	$container .= "
<table border=\"0\" cellspacing=\"1\" cellpadding=\"1\" class=\"list\" width=\"100%\" summary=\"file manager\">
	<tr>
		<th style=\"padding:0;width:18px\">&nbsp;</th>
		<th>Name</th>
		<th colspan=\"5\">&nbsp;</th>
		<th>Ext.</th>
		<th>Size</th>
		<th>Date</th>
		<th>Attributes</th>
	</tr>
	<tr>
		<td style=\"padding:0;width:18px\" title=\"UP one level\"><img width=\"16\" height=\"16\" src=\"rbfmimg/folder.png\" alt=\"F\" {$up_one_level} /></td>
		<td colspan=\"11\"><b title=\"UP one level\"{$up_one_level}>[..]</b></td>
	</tr>
";

	$id = 1;

	if(is_array($folders)){
		array_multisort($folders, SORT_ASC, SORT_REGULAR);
		foreach($folders as $v){
			if($v){

				$vf = $v.'/';

				$last_updated_time = date("Y.m.d H:i:s", filemtime($current_folder.$v));

				$fileperms = GetFilePerms($current_folder.$v);

				if($url_path){
					$browser = "<a href=\"{$url_path}{$v}\" target=\"_blank\"><img src=\"rbfmimg/ico_open_as_web.png\" border=\"0\" width=\"16\" height=\"16\" alt=\"W\" title=\"Open as web page\" /></a>";
					if($url_field){
						$use_url = "<img src=\"rbfmimg/ico_use_file.png\" border=\"0\" width=\"16\" height=\"16\" alt=\"U\" title=\"Use URL ({$url_path}{$v})\" onclick=\"window.opener.document.getElementById('{$url_field}').value='{$url_path}{$v}'; window.close()\" style=\"cursor: pointer\" />";
					}else{
						$use_url = "<img src=\"rbfmimg/ico_use_file_inactive.png\" border=\"0\" width=\"16\" height=\"16\" alt=\"U\" title=\"Use URL (Inactive!!!)\" />";
					}
				}else{
					$browser = "&nbsp;";
					$use_url = "<img src=\"rbfmimg/ico_use_file_inactive.png\" border=\"0\" width=\"16\" height=\"16\" alt=\"U\" title=\"Use URL (Inactive!!!)\" />";
				}


				$container .= "
			<tr>
				<td style=\"padding:0;width:18px\">
					<img width=\"16\" height=\"16\"
						src=\"rbfmimg/folder.png\"
						alt=\"Folder\"
						ondblclick=\"document.location='{$_SERVER['PHP_SELF']}?p=".urlencode($current_folder.$vf)."'\"
					/>
				</td>
				<td>
					
					<div style=\"padding-top:2px;\"
						id=\"f{$id}\"
						ondblclick=\"document.location='{$_SERVER['PHP_SELF']}?p=".urlencode($current_folder.$vf)."'\"
					>
						{$v}
					</div>
					
					<form 
						class=\"rename_field\"
						id=\"r{$id}\"
						name=\"r{$id}\"
						method=\"post\"
						action=\"rbfminc/rename.php\"
						target=\"results\"
						onsubmit=\"
							this.n.blur();
							return false
						\"
					>
						
						<input 
							class=\"input_name rename_input\"
							name=\"n\"
							type=\"text\"
							value=\"{$v}\"
							id=\"rf{$id}\"
							onblur=\"
								document.form{$id}.submit();
								document.getElementById('f{$id}').style.display = 'block';
								document.getElementById('r{$id}').style.display = 'none';
								document.getElementById('f{$id}').innerHTML = this.value;
								document.form{$id}.o.value = this.value;
							\"
						/>
						
						<input name=\"cf\" type=\"hidden\" value=\"{$current_folder}\" />
						<input name=\"o\" type=\"hidden\" value=\"{$v}\" />
						<input name=\"t\" type=\"hidden\" value=\"d\" />
						<input name=\"submitS\" type=\"submit\" value=\"submitS\" style='display: none; width:0;height:0' />
					</form>
				</td>
				<!--<td>{$use_url}</td>-->
				<td>{$browser}</td>
				<td>&nbsp;</td>
				<td>
					<img width=\"16\" height=\"16\"
						src=\"rbfmimg/ico_rename.png\"
						alt=\"Rename\" title=\"Rename\"
						onclick=\"
							document.getElementById('r{$id}').style.display = 'block';
							document.getElementById('f{$id}').style.display = 'none';
							document.getElementById('rf{$id}').focus();
							document.getElementById('rf{$id}').select()
						\"
					/>
				</td>
				<td>&nbsp;</td>
				<td>
					<img width=\"16\" height=\"16\"
						src=\"rbfmimg/ico_delete.png\"
						alt=\"D\"
						title=\"Delete\"
						onclick=\"
							if(
								confirm('Delete folder &quot;{$v}&quot;?') &amp;&amp;
								confirm('You cannot undo this operation!!!') &amp;&amp;
								confirm('To delete this folder &quot;{$v}&quot; press OK\\nTo cancel this operation press CANCEL')
							){
								document.location = 'file_manager.php?p=".urlencode($current_folder)."&amp;do=delete&amp;file=".urlencode($v)."&amp;type=directory'
							}
						\"
					/>
				</td>
				<td class=\"srow\">&nbsp;</td>
				<td><b>&lt;DIR&gt;</b></td>
				<td class=\"srow\">{$last_updated_time}</td>
				<td class=\"fileperms\">{$fileperms}</td>
			</tr>
			";
				$id++;
			}
		}
	}
	if(is_array($files)){
		array_multisort($files, SORT_ASC, SORT_REGULAR);
		foreach($files as $v){
			if($v){

				$extension = substr(strrchr($v, "."), 1);

				$file_image = "ico_file.png";
				if($extension == 'php' or $extension == 'php3'){$file_image = "ico_php.png";}
				if(
				$extension == 'htm' or
				$extension == 'HTM' or
				$extension == 'html' or
				$extension == 'HTML'
				){$file_image = "ico_html.png";}
				if(
				$extension == 'jpg' or
				$extension == 'JPG' or
				$extension == 'jpeg' or
				$extension == 'JPEG' or
				$extension == 'gif' or
				$extension == 'GIF' or
				$extension == 'png' or
				$extension == 'PNG'
				){$file_image = "ico_picture.png";}

				$last_updated_time = date("Y.m.d H:i:s", filemtime($current_folder.$v));

				$file_size = roundsize(filesize($current_folder.$v));

				if(
				$extension == 'txt' or
				$extension == 'inc' or
				$extension == 'sh' or
				$extension == 'js' or
				$extension == 'xml' or
				$extension == 'conf' or
				$extension == 'config' or
				$extension == 'ini' or
				$extension == 'php' or
				$extension == 'php3' or
				$extension == 'htm' or
				$extension == 'HTM' or
				$extension == 'html' or
				$extension == 'HTML' or
				$extension == 'css' or
				$extension == 'CSS'
				){
					$edit_file_content = "<a href=\"file_manager.php?p=".urlencode($current_folder)."&amp;f=".urlencode($v)."&amp;do=edit#file_edit\"><img width=\"16\" height=\"16\" src=\"rbfmimg/ico_script_edit.png\" alt=\"Edit\" title=\"View/Edit\" border=\"0\" /></a>";
				}else{
					$edit_file_content = "&nbsp;";
				}

				$fileperms = GetFilePerms($current_folder.$v);

				if($url_path){
					$browser = "<a href=\"{$url_path}{$v}\" target=\"_blank\"><img src=\"rbfmimg/ico_open_as_web.png\" border=\"0\" width=\"16\" height=\"16\" alt=\"W\" title=\"Open as web page\" /></a>";
					if($url_field){
						$use_url = "<img src=\"rbfmimg/ico_use_file.png\" border=\"0\" width=\"16\" height=\"16\" alt=\"U\" title=\"Use URL ({$url_path}{$v})\" onclick=\"window.opener.document.getElementById('{$url_field}').value='{$url_path}{$v}'; window.close()\" style=\"cursor: pointer\" />";
					}else{
						$use_url = "<img src=\"rbfmimg/ico_use_file_inactive.png\" border=\"0\" width=\"16\" height=\"16\" alt=\"U\" title=\"Use URL (Inactive!!!)\" />";
					}
				}else{
					$browser = "&nbsp;";
					$use_url = "<img src=\"rbfmimg/ico_use_file_inactive.png\" border=\"0\" width=\"16\" height=\"16\" alt=\"U\" title=\"Use URL (Inactive!!!)\" />";
				}


				$container .= "
			<tr>
				<td style=\"padding:0;width:18px\">
					<img width=\"16\" height=\"16\" src=\"rbfmimg/{$file_image}\" alt=\"File\" ondblclick=\"document.location = 'rbfminc/download.php?p=".urlencode($current_folder)."&amp;file_name=".urlencode($v)."'\" />
				</td>
				<td>
					<div style=\"padding-top:2px;\"
						id=\"f{$id}\"
						ondblclick=\"document.location = 'rbfminc/download.php?p=".urlencode($current_folder)."&amp;file_name=".urlencode($v)."'\"
					>
						{$v}
					</div>
					
					<form
						class=\"rename_field\"
						id=\"r{$id}\"
						name=\"r{$id}\"
						method=\"post\"
						action=\"rbfminc/rename.php\"
						target=\"results\"
						onsubmit=\"this.n.blur(); return false\"
					>
						<input name=\"cf\" type=\"hidden\" value=\"{$current_folder}\" />
						<input name=\"o\" type=\"hidden\" value=\"{$v}\" />
						<input name=\"t\" type=\"hidden\" value=\"f\" />
						<input
							class=\"input_name\"
							name=\"n\"
							type=\"text\"
							value=\"{$v}\"
							id=\"rf{$id}\"
							onblur=\"
								document.form{$id}.submit();
								document.getElementById('f{$id}').style.display = 'block';
								document.getElementById('r{$id}').style.display = 'none';
								document.getElementById('f{$id}').innerHTML = this.value;
								document.form{$id}.o.value = this.value;
							\"
						/>
						<input name=\"submitS\" type=\"submit\" value=\"submitS\" style=\"display: none; width:0;height:0\" />
					</form>
				</td>
				<!--<td>{$use_url}</td>-->
				<td>{$browser}</td>
				<td>
					<a href=\"rbfminc/download.php?p=".urlencode($current_folder)."&amp;file_name=".urlencode($v)."\"><img width=\"16\" height=\"16\"
						src=\"rbfmimg/ico_download.png\"
						alt=\"Download\"
						title=\"Download\"
						border=\"0\"
					/></a>
				</td>
				<td>
					<img width=\"16\" height=\"16\"
						src=\"rbfmimg/ico_rename.png\"
						alt=\"Rename\"
						title=\"Rename\"
						onclick=\"document.getElementById('f{$id}').style.display = 'none';
						document.getElementById('r{$id}').style.display = 'block';
						document.getElementById('rf{$id}').focus();
						document.getElementById('rf{$id}').select()\"
					/>
				</td>
				<td>{$edit_file_content}</td>
				<td>
					<img width=\"16\" height=\"16\"
						src=\"rbfmimg/ico_delete.png\"
						alt=\"D\"
						title=\"Delete\"
						onclick=\"
							if(
								confirm('Delete file &quot;{$v}&quot;?') &amp;&amp;
								confirm('You cannot undo this operation!!!') &amp;&amp;
								confirm('To delete this file &quot;{$v}&quot; press OK\\nTo cancel this operation press CANCEL')
							){
								document.location = 'file_manager.php?p=".urlencode($current_folder)."&amp;do=delete&amp;file=".urlencode($v)."&amp;type=file'
							}
						\"
					/>
				</td>
				<td class=\"srow\">{$extension}</td>
				<td>{$file_size}</td>
				<td class=\"srow\">{$last_updated_time}</td>
				<td class=\"fileperms\">{$fileperms}</td>
			</tr>
			";
				$id++;
			}
	}
		}

	$container .= "</table>";

	$container = preg_replace("/\s+/m", " ", $container);

?>

<link href="rbfminc/file_editor_style.css" rel="stylesheet" type="text/css" />
</head>

<body>


<?php include("fbegin.inc"); ?>

<?php echo $security_issues; ?>
<div class="file_editor">
	<div class="header">
		
	</div>
	<form id="path" name="path" method="get" action="" class="path">
		<input name="p" type="text" id="location" value="<?php echo $current_folder; ?>" />
		<input name="go" type="image" id="go" value="Go" src="rbfmimg/go.png" style="width:35;height:18" />
	</form>
	<div class="url_path"><br />URL path: <a href="/<?php echo $url_path; ?>" target="_blank"><?php echo $url_path; ?></a></div>
	<div class="container"> <?php echo $container; ?> <?php echo $error; ?> </div>
	<form action="" method="post" enctype="multipart/form-data" name="form_upload" id="form_upload" class="form_upload">
		Upload a file in current folder:
		<input type="file" name="file" id="file" />
		&nbsp;  &nbsp;
		<input name="replace_file" type="checkbox" value="1" />
		Replace existing file &nbsp;  &nbsp;
		<input type="submit" name="upload" id="upload" value="Upload" />
		<input name="upload_file" type="hidden" id="upload_file" value="upload_file" />
	</form>
	<form action="" method="post" enctype="multipart/form-data" name="form_create" id="form_create" class="form_create">
		&nbsp;&nbsp;&nbsp;&nbsp;Create new folder here;&nbsp;Folder name:
		<input name="folder_name" type="text" style="width:290" />
		<input type="submit" name="create_folder" id="create_folder" value="Create folder" />
	</form>
	<iframe name="results" frameborder="0" scrolling="auto" class='results'></iframe>
	<div align="center" style="margin-top:5px"> [ <img src="rbfmimg/ico_open_as_web.png" width="16" height="16" align="middle" alt="open" /> OPEN IN BROWSER ] &nbsp; &nbsp; 
		[ <img src="rbfmimg/ico_download.png" width="16" height="16" align="middle" alt="download" /> DOWNLOAD ] &nbsp; &nbsp; 
		[ <img src="rbfmimg/ico_rename.png" width="16" height="16" align="middle" alt="rename" /> RENAME ] &nbsp; &nbsp; 
		[ <img src="rbfmimg/ico_script_edit.png" width="16" height="16" align="middle" alt="view" /> VIEW/EDIT ] &nbsp; &nbsp; 
		[ <img src="rbfmimg/ico_delete.png" width="16" height="16" align="middle" alt="delete" /> DELETE ] </div>
	<?php
	if($_GET['do'] == 'edit'){

		$file_content = file_get_contents($current_folder.$_GET['f']);
		echo "
<form id=\"form_edit\" name=\"form_edit\" method=\"post\" action=\"\" style='width: 670px;margin: 10px auto 0;border-top: 1px #999999 solid'>
	<a name=\"file_edit\"></a>
	File: <b>{$current_folder}{$_GET['f']}</b><br />
	<textarea name=\"file_content\" id=\"file_content\" cols=\"1\" rows=\"1\" style=\"width: 99%; height: 400px\">".htmlentities ($file_content)."</textarea><br />
	<input name=\"save\" type=\"submit\" value=\"Save\" />
	<input name=\"close\" type=\"button\" value=\"Close file editor\" onclick=\"document.location = 'file_manager.php?f=".urlencode($current_folder)."'\" />
	<input name=\"save_file\" type=\"hidden\" value=\"save_file\" />
</form>
";
	}

?>
	<div class="footer"></div>
</div>
<small>Created by <a href="http://www.tomschaefer.org/pfsense">TomSchaefer</a></small>
<?php
if($alert_info){
	echo "
<script type=\"text/javascript\">
//<![CDATA[
	alert('{$alert_info}');
//]]>
</script>
	";
}
if($redirect){
	echo "
<script type=\"text/javascript\">
//<![CDATA[
	document.location = '{$redirect}';
//]]>
</script>
	";
}
?>
<?php include("fend.inc"); ?>
</body>
</html>
<?php
}else{
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Login</title>
<style type="text/css">
/*<![CDATA[*/
body,td,th,input {
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}
body {
	background-color: #EEEEEE;
}
/*]]>*/
</style></head>
<body><br /><br /><br /><br />
<div class="login">
	<div style="color:red" align="center"><?php echo $error_message; ?></div>
	<form id="login_form" name="login_form" method="post" action="">
		<table border="0" align="center" cellpadding="4" cellspacing="0" bgcolor="#FFFFFF" style="border:1px solid #999999; padding:10px" summary="login">
			<tr>
				<td align="right">Username:</td>
				<td><input type="text" name="username" id="username" class="login_input" style="width:230px" /></td>
			</tr>
			<tr>
				<td align="right">Password:</td>
				<td><input type="password" name="password" id="password" class="login_input" style="width:100px" /> 
				</td>
			</tr>
			<tr>
				<td colspan="2" align="right"><input type="submit" name="button" id="button" value="Login &raquo;" /></td>
			</tr>
		</table>
		<input name="login" type="hidden" value="login" />
	</form>
</div>
</body>
</html>


<?php
}
?>
