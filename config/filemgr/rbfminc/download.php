<?php
include_once("auth.inc");
include "functions.php";
//Set the cache policy
ob_end_clean();
header("Expires: Sat, 26 Jul 1997 05:00:00 GMT");
header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");	
//Gets the parameters
$_GET['file_name'] = urldecode($_GET['file_name']);
$_GET['p'] = urldecode($_GET['p']);
//Check Authentication
$candownload = false;
if (function_exists("session_auth"))
	{//pfSense 2.X
	$candownload = session_auth();}
else
	{//pfSense 1.2.3
	$candownload = htpasswd_backed_basic_auth();}
if ($candownload)
{
	if($_GET['file_name'] and $_GET['p']){
		$filepath = $_GET['p'].$_GET['file_name'];
		if(file_exists($filepath)){
			$type = wp_check_filetype($_GET['file_name']);	
			header('Content-type: ' . $type[$_GET['file_name']]);
			header('Content-Disposition: attachment; filename="'.$_GET['file_name'].'"');
			header('Content-Length: ' . filesize($filepath));
			header('Last-Modified: '.gmdate('D, d M Y H:i:s', filemtime($filepath)).' GMT', true, 200);
			flush();
			readfile($filepath);
			exit;
		}
		else
		{
			echo("File not found");
		}
	}
	else
	{
		echo("File Unknown");
	}
}
else
{
	echo("Session Expired");
}
?>
