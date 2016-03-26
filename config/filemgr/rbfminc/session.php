<?php
/*
	session.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2010 Tom Schaefer <tom@tomschaefer.org>
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
session_name("file_manager_session");

// Create session
session_start();
if (($_POST['login'] == 'login') && ($_POST['username']) && ($_POST['password'])) {
	$_SESSION = array();
	$_SESSION['username'] = $_POST['username'];
	$_SESSION['password'] = $_POST['password'];
}

if ($_GET['logout'] == "logout") {
	setcookie('url_field', '', time()-3600);
	setcookie('current_folder', '', time()-3600);
	$_SESSION = array();
	session_destroy();
	session_unset();
	header("Location: file_manager.php");
}


if (($_SESSION['username']) && ($_SESSION['password'])) {
	if (($_SESSION['username'] == $username) && ($_SESSION['password'] == $password)) {
		$user_login = 'ok';
	} else {
		$error_message = "Incorrect username or password!";
	}
}

?>
