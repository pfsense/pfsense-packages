<?php
/*
	config.php
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
// Initial folder
$initial_folder = "/";
// 0 = you can browse all files on the server;
// 1= you can browse only the $initial_folder and below
$only_below = 0;

// Login info {Please change the initial username and password}
$username = 'admin';
$password = 'admin';

/* ==================================== */
/* BEGIN Protect against GLOBALS tricks */
if (isset($_POST['GLOBALS']) || isset($_FILES['GLOBALS']) || isset($_GET['GLOBALS']) || isset($_COOKIE['GLOBALS'])) {
	die("Hacking attempt");
}

if (isset($_SESSION) && !is_array($_SESSION)) {
	die("Hacking attempt");
}

if (@ini_get('register_globals') == '1' || strtolower(@ini_get('register_globals')) == 'on') {
	$not_unset = array('_GET', '_POST', '_COOKIE', 'HTTP_SERVER_VARS', '_SESSION', 'HTTP_ENV_VARS', '_FILES');

	if (!isset($_SESSION) || !is_array($_SESSION)) {
		$_SESSION = array();
	}
	$input = array_merge($_GET, $_POST, $_COOKIE, $HTTP_SERVER_VARS, $_SESSION, $HTTP_ENV_VARS, $_FILES);

	unset($input['input']);
	unset($input['not_unset']);

	while (list($var,) = @each($input)) {
		if (in_array($var, $not_unset)) {
			die('Hacking attempt!');
		}
		unset($$var);
	}

	unset($input);
}

if (!get_magic_quotes_gpc()) {
	if (is_array($_GET)) {
		while (list($k, $v) = each($_GET)) {
			if (is_array($_GET[$k])) {
				while (list($k2, $v2) = each($_GET[$k])) {
					$_GET[$k][$k2] = addslashes($v2);
				}
				@reset($_GET[$k]);
			} else {
				$_GET[$k] = addslashes($v);
			}
		}
		@reset($_GET);
	}

	if (is_array($_POST)) {
		while (list($k, $v) = each($_POST)) {
			if (is_array($_POST[$k])) {
				while (list($k2, $v2) = each($_POST[$k])) {
					$_POST[$k][$k2] = addslashes($v2);
				}
				@reset($_POST[$k]);
			} else {
				$_POST[$k] = addslashes($v);
			}
		}
		@reset($_POST);
	}

	if (is_array($_COOKIE)) {
		while (list($k, $v) = each($_COOKIE)) {
			if (is_array($_COOKIE[$k])) {
				while (list($k2, $v2) = each($_COOKIE[$k])) {
					$_COOKIE[$k][$k2] = addslashes($v2);
				}
				@reset($_COOKIE[$k]);
			} else {
				$_COOKIE[$k] = addslashes($v);
			}
		}
		@reset($_COOKIE);
	}
}
/*  END Protect against GLOBALS tricks  */
/* ==================================== */

/*
if ($username == 'admin' and $password == 'admin') {
	$security_issues = "<div align=\"center\" style=\"color: red;\"><strong>Security issue</strong>: Please change your username or password</div>";
}
*/
$security_issues = "<br />";
?>
