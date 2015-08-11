<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
	strikeback.inc
	Copyright (C) 2011 Tom Schaefer TomSchaefer.org
	All rights reserved.
																			*/
/* ========================================================================== */
/*
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



function php_sync_package_sb() 
{
	global $config;
}


function php_install_command_sb() 
{

	global $config;
	conf_mount_rw();

	if (!is_dir('/usr/local/www/packages/')) {
		exec("mkdir /usr/local/www/packages/");
	}

	if (!is_dir('/usr/local/www/packages/strikeback/')) {
		exec("mkdir /usr/local/www/packages/strikeback/");
	}

	if (!is_dir('/var/run/iplog/')) {
		exec("mkdir /var/run/iplog/");
	}


	
	
	//rename PHP files from .tmp to .php
	//exec("cp /tmp/class.phpmailer.tmp /usr/local/www/packages/strikeback/class.phpmailer.php");
	//unlink_if_exists("/tmp/class.phpmailer.tmp ");
	//exec("cp /tmp/class.smtp.tmp /usr/local/www/packages/strikeback/class.smtp.php");
	//unlink_if_exists("/tmp/class.smtp.tmp");
	//exec("cp /tmp/email.tmp /usr/local/www/packages/strikeback/email.php");
	//unlink_if_exists("/tmp/email.tmp");
	exec("cp /tmp/strikeback.tmp /usr/local/www/packages/strikeback/strikeback.php");
	unlink_if_exists("/tmp/strikeback.tmp");
	
	exec("cp /tmp/index.tmp /usr/local/www/packages/strikeback/index.php");
	unlink_if_exists("/tmp/index.tmp");
	exec("cp /tmp/firewall_shaper.tmp /usr/local/www/packages/strikeback/firewall_shaper.php");
	unlink_if_exists("/tmp/firewall_shaper.tmp");
	exec("cp /tmp/help.tmp /usr/local/www/packages/strikeback/help.php");
	unlink_if_exists("/tmp/help.tmp");
	exec("cp /tmp/settings.tmp /usr/local/www/packages/strikeback/settings.php");
	unlink_if_exists("/tmp/settings.tmp");
	exec("cp /tmp/parse.tmp /usr/local/www/packages/strikeback/parse.php");
	unlink_if_exists("/tmp/parse.tmp");
	exec("cp /tmp/strikeback.db /var/db/strikeback.db");
	unlink_if_exists("/tmp/strikeback.db");
	exec("cp /tmp/jscolor.js /usr/local/www/packages/strikeback/jscolor.js");
	unlink_if_exists("/tmp/jscolor.js");


	//Get scripts
	
	//exec("cp /tmp/execute.sh /usr/local/www/packages/strikeback/execute.sh");
	//unlink_if_exists("/tmp/execute.sh");
	
		
	exec("pkg_add -r iplog");
	//unlink_if_exists("/tmp/iplog-2.2.3_3.tbz");

	exec("pkg_add -r nmap");
	//unlink_if_exists("/tmp/nmap.tbz");
	
	exec("/usr/bin/sed -i -e 's/iplog_enable="."NO"."/iplog_enable="."YES"."/g' /usr/local/etc/rc.d/iplog");

	exec("fetch -o /usr/local/www/packages/strikeback http://www.tutorialhero.com/uploads/43132.png");
	exec("mv /usr/local/www/packages/strikeback/43132.png /usr/local/www/packages/strikeback/color.png");

	conf_mount_ro();
}


function deinstall_command_sb() 
{
	conf_mount_rw();
	
	unlink_if_exists("/usr/local/pkg/strikeback.xml");
	unlink_if_exists("/usr/local/pkg/strikeback.inc");
	exec("rm -R /usr/local/www/packages/strikeback");
	exec("rm /usr/local/etc/rc.d/strikeback.sh");
	exec("pkg_delete -f iplog-2.2.3_3");
	exec("pkg_delete -f nmap-5.21_1");
	exec("rm /var/log/iplog");
	conf_mount_ro();
}

?>
