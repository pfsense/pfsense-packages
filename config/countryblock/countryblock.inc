<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
	countryblock.inc
	Copyright (C) 2010 Tom Schaefer
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



function php_sync_package_cb() 
{
	global $config;
}


function php_install_command_cb() 
{

	global $config;
	conf_mount_rw();
	config_lock();

	if (!is_dir('/usr/local/www/packages/')) {
		exec("mkdir /usr/local/www/packages/");
	}

	if (!is_dir('/usr/local/www/packages/countryblock/')) {
		exec("mkdir /usr/local/www/packages/countryblock/");
	}

	
	
	//rename PHP files from .tmp to .php
	exec("cp /tmp/class.phpmailer.tmp /usr/local/www/packages/countryblock/class.phpmailer.php");
	unlink_if_exists("/tmp/class.phpmailer.tmp ");
	exec("cp /tmp/class.smtp.tmp /usr/local/www/packages/countryblock/class.smtp.php");
	unlink_if_exists("/tmp/class.smtp.tmp");
	exec("cp /tmp/email.tmp /usr/local/www/packages/countryblock/email.php");
	unlink_if_exists("/tmp/email.tmp");
	exec("cp /tmp/countryblock.tmp /usr/local/www/packages/countryblock/countryblock.php");
	unlink_if_exists("/tmp/countryblock.tmp");
	exec("cp /tmp/countryblock_if.tmp /usr/local/www/packages/countryblock/countryblock_if.php");
	unlink_if_exists("/tmp/countryblock_if.tmp");
	exec("cp /tmp/index.tmp /usr/local/www/packages/countryblock/index.php");
	unlink_if_exists("/tmp/index.tmp");
	exec("cp /tmp/firewall_shaper.tmp /usr/local/www/packages/countryblock/firewall_shaper.php");
	unlink_if_exists("/tmp/firewall_shaper.tmp");
	exec("cp /tmp/help.tmp /usr/local/www/packages/countryblock/help.php");
	unlink_if_exists("/tmp/help.tmp");
	exec("cp /tmp/settings.tmp /usr/local/www/packages/countryblock/settings.php");
	unlink_if_exists("/tmp/settings.tmp");
	//Get scripts
	exec("cp /tmp/countryblock.sh /usr/local/etc/rc.d/countryblock.sh");
	unlink_if_exists("/tmp/countryblock.sh");
	exec("cp /tmp/execute.sh /usr/local/www/packages/countryblock/execute.sh");
	unlink_if_exists("/tmp/execute.sh");
	exec("cp /tmp/jquery.min.js /usr/local/www/packages/countryblock/jquery.min.js");
	unlink_if_exists("/tmp/jquery.min.js");
	exec("cp /tmp/ddaccordion.js /usr/local/www/packages/countryblock/ddaccordion.js");
	unlink_if_exists("/tmp/ddaccordion.js");
	exec("cp /tmp/public_smo_scripts.js /usr/local/www/packages/countryblock/public_smo_scripts.js");
	unlink_if_exists("/tmp/public_smo_scripts.js");
	exec("cp /tmp/titlebar.png /usr/local/www/packages/countryblock/titlebar.png");
	unlink_if_exists("/tmp/titlebar.png");
	exec("cp /tmp/titlebar-active.png /usr/local/www/packages/countryblock/titlebar-active.png");
	unlink_if_exists("/tmp/titlebar-active.png");
	exec("cp /tmp/purge.tmp /usr/local/www/packages/countryblock/purge.php");
	unlink_if_exists("/tmp/purge.tmp");
	exec("cp /tmp/whitelist.tmp /usr/local/www/packages/countryblock/whitelist.php");
	unlink_if_exists("/tmp/whitelist.tmp");
	exec("cp /tmp/interfaces.txt /usr/local/www/packages/countryblock/interfaces.txt");
	unlink_if_exists("/tmp/interfaces.txt");
	
	

	conf_mount_ro();
	config_unlock();
}


function deinstall_command_cb() 
{
	conf_mount_rw();
	$handle = popen("/usr/local/etc/rc.d/countryblock.sh stop", "r");
	unlink_if_exists("/usr/local/pkg/countryblock.xml");
	unlink_if_exists("/usr/local/pkg/countryblock.inc");
	exec("rm -R /usr/local/www/packages/countryblock/countryblocks");
	exec("rm -R /usr/local/www/packages/countryblock");
	exec("rm /usr/local/etc/rc.d/countryblock.sh");
	exec("pfctl -t countryblock -T kill");
	exec("sed -i -e '/countryblock/d' /tmp/rules.debug");
	exec("pfctl -o basic -f /tmp/rules.debug");
	conf_mount_ro();
}

?>
