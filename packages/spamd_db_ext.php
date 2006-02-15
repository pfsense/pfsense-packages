<?php
/* $Id$ */
/*
	spamd_db_ext.php
	Copyright (C) 2006 Scott Ullrich
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

require("config.inc");

if($_GET['username'])
	$username = $_GET['username'];
if($_GET['password'])
	$password = $_GET['password'];
if($_POST['username'])
	$username = $_POST['username'];
if($_POST['password'])
	$password = $_POST['password'];

foreach($config['installedpackages']['spamdoutlook']['config'] as $outlook) {
	if($outlook['username'] <> $username) {
		echo "550.  INVALID USERNAME {$username}.";
		exit;
	}
	if($outlook['password'] <> $password) {
		echo "550.  INVALID PASSWORD {$password}.";
		exit;
	}
}

exec("echo {$_GET['action']} > /tmp/tmp");

/* handle AJAX operations */
if($_GET['action'] or $_POST['action']) {
	/*    echo back buttonid so it can be turned
	 *    back off when request is completed.
	 */
	if($_GET['action'])
		$action = $_GET['action'];
	if($_POST['action'])
		$action = $_POST['action'];
	if($_GET['srcip'])
		$srcip = $_GET['srcip'];
	if($_POST['srcip'])
		$srcip = $_POST['srcip'];
	/* execute spamdb command */
	if($action == "whitelist") {
		exec("/usr/local/sbin/spamdb -a {$srcip}");
		exit;
	} else if($action == "delete") {
		$fd = fopen("/tmp/execcmds", "w");
		config_lock();
		fwrite($fd, "#!/bin/sh\n");
		fwrite($fd, "/usr/local/sbin/spamdb -d {$srcip}\n");
		fwrite($fd, "/usr/local/sbin/spamdb -T -d \"<{$srcip}>\"\n");
		fwrite($fd, "/usr/local/sbin/spamdb -t -d \"<{$srcip}>\"\n");
		fwrite($fd, "/usr/local/sbin/spamdb | grep {$srcip}\n");
		fclose($fd);
		exec("chmod a+rx /tmp/execcmds");
		system("/bin/sh /tmp/execcmds");
		config_unlock();
		exit;
	} else if($action == "spamtrap") {
		exec("/usr/local/sbin/spamdb -a {$srcip} -T");
		exit;
	} else if($action == "trapped") {
		exec("/usr/local/sbin/spamdb -a {$srcip} -t");
		exit;
	}
	/* signal a reload for real time effect. */
	mwexec("killall -HUP spamlogd");
	exit;
}

/* spam trap e-mail address */
if($_POST['spamtrapemail'] <> "") {
	mwexec("/usr/local/sbin/spamdb -T -a \"<{$_POST['spamtrapemail']}>\"");
	mwexec("killall -HUP spamlogd");
	$savemsg = $_POST['spamtrapemail'] . " added to spam trap database.";
}

if($_GET['getstatus'] <> "") {
	$status = exec("/usr/local/sbin/spamdb | grep \"{$_GET['getstatus']}\"");
	if(stristr($status, "WHITE") == true) {
		echo "WHITE";
	} else if(stristr($status, "TRAPPED") == true) {
		echo "TRAPPED";
	} else if(stristr($status, "GREY") == true) {
		echo "GREY";
	} else if(stristr($status, "SPAMTRAP") == true) {
		echo "SPAMTRAP";
	} else {
		echo "NOT FOUND";	
	}	
	exit;
}

/* spam trap e-mail address */
if($_GET['spamtrapemail'] <> "") {
	$status = exec("spamdb -T -a \"<{$_GET['spamtrapemail']}>\"");
	mwexec("killall -HUP spamlogd");
	if($status)
		echo $status;
	else 
		echo $_POST['spamtrapemail'] . " added to spam trap database.";
	exit;
}

/* spam trap e-mail address */
if($_GET['whitelist'] <> "") {
	$status = exec("spamdb -a \"<{$_GET['spamtrapemail']}>\"");
	mwexec("killall -HUP spamlogd");
	if($status)
		echo $status;
	else 
		echo $_POST['spamtrapemail'] . " added to whitelist database.";
	exit;
}

function basic_auth_prompt(){
	header("WWW-Authenticate: Basic realm=\".\"");
	header("HTTP/1.0 401 Unauthorized");
	echo "You must enter valid credentials to access this resource.";
	exit;
}

exit;

?>