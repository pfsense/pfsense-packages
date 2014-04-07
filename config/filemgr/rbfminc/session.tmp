<?php
session_name("file_manager_session");

session_start();

/*************************************************************************************************/
//create session
if($_POST['login'] == 'login' and $_POST['username'] and $_POST['password']){
	$_SESSION = array();
	$_SESSION['username']=$_POST['username'];
	$_SESSION['password']=$_POST['password'];
}

if($_GET['logout'] == "logout"){
	setcookie('url_field', '', time()-3600);
	setcookie('current_folder', '', time()-3600);
	$_SESSION = array();
	session_destroy();
	session_unset();
	header("Location: file_manager.php");
}


if($_SESSION['username'] and $_SESSION['password']){
	if($_SESSION['username'] == $username and $_SESSION['password'] == $password){
		$user_login = 'ok';
	}else{
		$error_message = "Incorect username or password!";
	}
}

?>
