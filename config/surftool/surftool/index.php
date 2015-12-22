<?php
/*
	index.php
	Copyright (C) 2015 H-T Reimers <reimers@mail.de>
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

	session_start();
	//Read configuration
	$config=parse_ini_file ("surftool.ini");		


	if(isset($_POST["logout"])){
		session_unset();
		$logout=true;
	}
	if(isset($_SESSION["login"])) $login=$_SESSION["login"];
	else $login=false;
	
	if( isset($_GET["secret"])  ){
		if($_GET["secret"]==$config["redirect_secret"] ){
			$login=true;
			$_SESSION["login"]=true;
		}	
	}
	if( isset($_POST["loginname"]) AND isset($_POST["passwd"]) ){
		$user=$_POST["loginname"];
		$passwd=$_POST["passwd"];
		if($user==$config["user_name"] AND $passwd==$config["user_password"]){
			$login=true;
			$_SESSION["login"]=true;
		}
	}
	
	if($logout){
		header("Location: ".$config["logout_target"]);
		$meta="<meta http-equiv='refresh' content='0; URL=".$config["logout_target"]."'>"; //When redirect with header is not supported
	}
	else if($login){
		header("Location: set.php");
		$meta="<meta http-equiv='refresh' content='0; URL=set.php'>"; //When redirect with header is not supported
	}
	else $meta="";
	echo "<!DOCTYPE html><html><head>$meta<title>Surftool</title></head><body>";
	
	if($login){
		echo "<a href='set.php'> weiter </a>"; //When redirect with header and meta is not supported
		echo "<form action='index.php' method='post'>
			<input type='submit' name='logout' value='logout'>
		</form> ";
	}
	else if(!$login){
		echo "
		<form action='' method='post'>
			Loginname:<input type='text' name='loginname'><br>
			Password:<input type='password' name='passwd'><br>
			<input type='submit' value='login'>
		</form> ";
	}
	
	
	echo "</body></html>";
?>