<?php
/* $Id$ */
/*
	spamd_db_ext.php
	Copyright (C) 2008 Scott Ullrich
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

/* this script is no longer supported */
exit;

if($_GET['loginname'])
	$loginname = " Username: " . $_GET['loginname'];
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

/* handle AJAX operations */
if($_GET['action'] or $_POST['action']) {
	if($_GET['action'])
		$action = escapeshellarg(trim($_GET['action']));
	if($_POST['action'])
		$action = escapeshellarg(trim($_POST['action']));
	if($_GET['srcip'])
		$srcip = escapeshellarg(trim($_GET['srcip']));
	if($_POST['srcip'])
		$srcip = escapeshellarg(trim($_POST['srcip']));
	if($_POST['email'])
		$email = escapeshellarg(trim($_POST['email']));
	if($_GET['email'])
		$email = escapeshellarg(trim($_GET['email']));
	/* execute spamdb command */
	if($action == "whitelist") {
		delete_from_spamd_db($srcip);
		usleep(100);
		exec("/usr/local/sbin/spamdb -a {$srcip}");
		mwexec("/sbin/pfctl -q -t blacklist -T replace -f /var/db/blacklist.txt");
		delete_from_blacklist($srcip);
		log_error("spamd: {$srcip} has been whitelisted by {$_SERVER['REMOTE_ADDR']} {$loginname}");
		hup_spamd();
		exit;
	} else if($action == "delete") {
		delete_from_spamd_db($srcip);
		usleep(100);
		hup_spamd();
		mwexec("/sbin/pfctl -q -t spamd -T delete $srcip");
		mwexec("/sbin/pfctl -q -t blacklist -T replace -f /var/db/blacklist.txt");
		delete_from_blacklist($srcip);
		delete_from_whitelist($srcip);
		log_error("spamd: {$srcip} has been deleted by {$_SERVER['REMOTE_ADDR']} {$loginname}");
		exit;
	} else if($action == "spamtrap") {
		delete_from_spamd_db($email);
		delete_from_whitelist($srcip);
		usleep(100);
		exec("/usr/local/sbin/spamdb -a \"{$email}\" -T");
		hup_spamd();
		mwexec("/sbin/pfctl -q -t blacklist -T add -f /var/db/blacklist.txt");
		log_error("spamd: {$srcip} has been blacklisted by {$_SERVER['REMOTE_ADDR']} {$loginname}");
		exit;
	} else if($action == "trapped") {
		delete_from_spamd_db($srcip);
		delete_from_whitelist($srcip);
		usleep(100);
		exec("/usr/local/sbin/spamdb -a {$srcip} -t");
		add_to_blacklist($srcip);
		log_error("spamd: {$srcip} has been trapped by {$_SERVER['REMOTE_ADDR']} {$loginname}");
		hup_spamd();
		exit;
	}
	/* signal a reload for real time effect. */
	hup_spamd();
	exit;
}

/* spam trap e-mail address */
if($_POST['spamtrapemail'] <> "") {
	$spamtrapemail = escapeshellarg($_POST['spamtrapemail']);
	exec("/usr/local/sbin/spamdb -d {$spamtrapemail}");
	exec("/usr/local/sbin/spamdb -d -T \"{$spamtrapemail}\"");
	exec("/usr/local/sbin/spamdb -d -t \"{$spamtrapemail}\"");
	mwexec("/usr/local/sbin/spamdb -T -a \"{$spamtrapemail}\"");
	mwexec("killall -HUP spamlogd");
	$savemsg = htmlentities($_POST['spamtrapemail']) . " added to spam trap database.";
}

if($_GET['getstatus'] <> "") {
	$getstatus = escapeshellarg($_GET['getstatus']);
	$status = exec("/usr/local/sbin/spamdb | grep \"{$getstatus}\"");
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
	$spamtrapemail = escapeshellarg($_GET['spamtrapemail']);
	$status = exec("spamdb -T -a \"{$spamtrapemail}\"");
	mwexec("killall -HUP spamlogd");
	if($status)
		echo $status;
	else 
		echo htmlentities($_POST['spamtrapemail']) . " added to spam trap database.";
	exit;
}

/* spam trap e-mail address */
if($_GET['whitelist'] <> "") {
	$spamtrapemail = escapeshellarg($_GET['spamtrapemail']);
	$status = exec("spamdb -a \"{$spamtrapemail}\"");
	mwexec("killall -HUP spamlogd");
	if($status)
		echo $status;
	else 
		echo htmlentities($_POST['spamtrapemail']) . " added to whitelist database.";
	exit;
}

function delete_from_spamd_db($srcip) {
	config_lock();
	$fd = fopen("/tmp/execcmds", "w");
	fwrite($fd, "#!/bin/sh\n");	
	fwrite($fd, "/usr/local/sbin/spamdb -d {$srcip}\n");
	fwrite($fd, "/usr/local/sbin/spamdb -d {$srcip} -T\n");
	fwrite($fd, "/usr/local/sbin/spamdb -d {$srcip} -t\n");
	fwrite($fd, "/usr/local/sbin/spamdb -d \"{$srcip}\" -t\n");
	fwrite($fd, "/usr/local/sbin/spamdb -d \"{$srcip}\" -T\n");	
	fclose($fd);
	exec("/bin/chmod a+rx /tmp/execcmds");
	system("/bin/sh /tmp/execcmds");
	mwexec("/usr/bin/killall -HUP spamlogd");
	mwexec("/sbin/pfctl -q -t blacklist -T replace -f /var/db/blacklist.txt");
	config_unlock();
}

function basic_auth_prompt(){
	header("WWW-Authenticate: Basic realm=\".\"");
	header("HTTP/1.0 401 Unauthorized");
	echo "You must enter valid credentials to access this resource.";
	exit;
}

function add_to_blacklist($srcip) {
	config_lock();
	$fd = fopen("/var/db/blacklist.txt", "a");
	fwrite($fd, "{$srcip}\n");
	fclose($fd);
	mwexec("/sbin/pfctl -q -t spamd -T add -f /var/db/blacklist.txt");
	mwexec("/sbin/pfctl -q -t blacklist -T add -f /var/db/blacklist.txt");
	config_unlock();
}

function delete_from_blacklist($srcip) {
	config_lock();
	$blacklist = split("\n", file_get_contents("/var/db/blacklist.txt"));
	$fd = fopen("/var/db/blacklist.txt", "w");
	foreach($blacklist as $bl) {
		if($bl <> "")
			if(!stristr($bl, $srcip))
				fwrite($fd, "{$bl}\n");
	}
	fclose($fd);
	mwexec("/sbin/pfctl -q -t spamd -T delete $srcip");
	mwexec("/sbin/pfctl -q -t blacklist -T replace -f /var/db/blacklist.txt");
	config_unlock();
}

function delete_from_whitelist($srcip) {
	config_lock();
	$whitelist = split("\n", file_get_contents("/var/db/whitelist.txt"));
	$fd = fopen("/var/db/whitelist.txt", "w");
	foreach($whitelist as $wl) {
		if($wl <> "")
			if(!stristr($wl, $srcip))
				fwrite($fd, "{$wl}\n");
	}
	fclose($fd);
	mwexec("/sbin/pfctl -q -t spamd -T delete $srcip");
	mwexec("/sbin/pfctl -q -t whitelist -T replace -f /var/db/whitelist.txt");
	config_unlock();
}

function hup_spamd() {
	mwexec("killall -HUP spamlogd");	
}

exit;

?>