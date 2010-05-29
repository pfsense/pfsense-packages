<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
	ipblocklist.inc v2
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


function pkg_is_service_running($servicename) 
{
	exec("/bin/ps ax | awk '{ print $5 }'", $psout);
	array_shift($psout);
	foreach($psout as $line) {
		$ps[] = trim(array_pop(explode(' ', array_pop(explode('/', $line)))));
	}
	if(is_service_running($servicename, $ps) or is_process_running($servicename) ) {
		return true;
	}
	else {
		return false;
	}
}
*/
function byte_convert( $bytes ) {
	if ($bytes<=0)
		return '0 Byte';

	$convention=1000; //[1000->10^x|1024->2^x]
	$s=array('B', 'kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB');
	$e=floor(log($bytes,$convention));
	return round($bytes/pow($convention,$e),2).' '.$s[$e];
}

 


function php_sync_package() 
{
	global $config;
}


function php_install_command() 
{

	global $config;
	conf_mount_rw();
	config_lock();

	if (!is_dir('/usr/local/www/packages/')) {
		exec("mkdir /usr/local/www/packages/");
	}

	if (!is_dir('/usr/local/www/packages/ipblocklist/')) {
		exec("mkdir /usr/local/www/packages/ipblocklist/");
	}

	
	//install PERL
	exec("pkg_add -r ftp://ftp.freebsd.org/pub/FreeBSD/ports/i386/packages-7.2-release/All/perl-5.8.9_2.tbz");
	exec("pkg_add -r ftp://ftp.freebsd.org/pub/FreeBSD/ports/i386/packages-7.2-release/All/p5-Net-CIDR-0.13.tbz");

	//rename PHP files from .tmp to .php
	exec("cp /tmp/ipblocklist.tmp /usr/local/www/packages/ipblocklist/ipblocklist.php");
	unlink_if_exists("/tmp/ipblocklist.tmp");
	exec("cp /tmp/ipblocklist_list.tmp /usr/local/www/packages/ipblocklist/ipblocklist_list.php");
	unlink_if_exists("/tmp/ipblocklist_list.tmp");
	exec("cp /tmp/purge.tmp /usr/local/www/packages/ipblocklist/purge.php");
	unlink_if_exists("/tmp/purge.tmp");
	exec("cp /tmp/index.tmp /usr/local/www/packages/ipblocklist/index.php");
	unlink_if_exists("/tmp/index.tmp");
	exec("cp /tmp/whitelist.tmp /usr/local/www/packages/ipblocklist/whitelist.php");
	unlink_if_exists("/tmp/whitelist.tmp");
	exec("cp /tmp/purgeip.tmp /usr/local/www/packages/ipblocklist/purgeip.php");
	unlink_if_exists("/tmp/purgeip.tmp");
	//Get scripts
	exec("cp /tmp/convert.pl /usr/local/www/packages/ipblocklist/convert.pl");
	unlink_if_exists("/tmp/convert.pl");
	exec("cp /tmp/convert-execute.sh /usr/local/www/packages/ipblocklist/convert-execute.sh");
	unlink_if_exists("/tmp/convert-execute.sh");
	exec("cp /tmp/IP-Blocklist.sh /usr/local/etc/rc.d/IP-Blocklist.sh");
	unlink_if_exists("/tmp/IP-Blocklist.sh");
	
	

	conf_mount_ro();
	config_unlock();
}


function deinstall_command() 
{
	conf_mount_rw();
	$handle = popen("/usr/local/etc/rc.d/ipblocklist.sh stop", "r");
	unlink_if_exists("/usr/local/pkg/ipblocklist.xml");
	unlink_if_exists("/usr/local/pkg/ipblocklist.inc");
	exec("rm -R /usr/local/www/packages/ipblocklist/ipblocklists");
	exec("rm -R /usr/local/www/packages/ipblocklist");
	exec("rm /usr/local/etc/rc.d/IP-Blocklist.sh");
	exec("pfctl -t ipblocklist -T kill");
	exec("ipfw -f flush");
	conf_mount_ro();
}

?>