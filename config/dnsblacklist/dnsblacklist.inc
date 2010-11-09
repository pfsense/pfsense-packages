<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
	dnsblacklist.inc
	Copyright (C) 2009 Mark J Crane
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

function byte_convert( $bytes ) {
	if ($bytes<=0)
		return '0 Byte';

	$convention=1000; //[1000->10^x|1024->2^x]
	$s=array('B', 'kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB');
	$e=floor(log($bytes,$convention));
	return round($bytes/pow($convention,$e),2).' '.$s[$e];
}

 


function dnsblacklist_sync_package()
{
	global $config;
}


function dnsblacklist_install_command()
{

	global $config;
	conf_mount_rw();
	config_lock();

	if (!is_dir('/usr/local/www/packages/')) {
		exec("mkdir /usr/local/www/packages/");
	}

	if (!is_dir('/usr/local/www/packages/dnsblacklist/')) {
		exec("mkdir /usr/local/www/packages/dnsblacklist/");
	}

	//extract the blacklist
	exec("cp /tmp/blacklists.tar.gz /usr/local/www/packages/dnsblacklist/blacklists.tar.gz");
	chdir("/usr/local/www/packages/dnsblacklist/");
	exec("tar xvf blacklists.tar.gz");
	exec("rm /tmp/blacklists.tar.gz");
	exec("rm /usr/local/www/packages/dnsblacklist/blacklists.tar.gz");

	//rename PHP files from .tmp to .php
	exec("cp /tmp/dnsblacklist.tmp /usr/local/www/packages/dnsblacklist/dnsblacklist.php");
	unlink_if_exists("/tmp/dnsblacklist.tmp");

	//write_rcfile(array(
	//	"file" => "phpservice.sh",
	//	"start" => "/usr/local/bin/php /usr/local/pkg/phpservice.php >> /var/log/phpservice.log &",
	//	"stop" => "rm /tmp/phpmonitor.pid"
	//	)
	//);

	//dnsblacklist_sync_package();
	//$handle = popen("/usr/local/etc/rc.d/phpservice.sh start", "r");
	//pclose($handle);

	//if (pkg_is_service_running('phpservice')) {  
		//documentation purposes
	//}

	conf_mount_ro();
	config_unlock();
}


function dnsblacklist_deinstall_command()
{
	conf_mount_rw();
	$handle = popen("/usr/local/etc/rc.d/dnsblacklist.sh stop", "r");
	unlink_if_exists("/usr/local/pkg/dnsblacklist.xml");
	unlink_if_exists("/usr/local/pkg/dnsblacklist.inc");
	exec("rm -R /usr/local/www/packages/dnsblacklist/blacklists");
	exec("rm -R /usr/local/www/packages/dnsblacklist");
	conf_mount_ro();
}

?>