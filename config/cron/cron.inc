<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
    cron.inc
    Copyright (C) 2008 Mark J Crane
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

require_once("services.inc");

function cron_pkg_is_service_running($servicename)
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

function cron_sync_package()
{

	global $config;   
	//synch cron tab
	configure_cron();
	$handle = popen("/usr/local/etc/rc.d/cron.sh stop", "r");
	pclose($handle);
	$handle = popen("/usr/local/etc/rc.d/cron.sh start", "r");
	pclose($handle);

}
    

function cron_install_command()
{

	global $config;
	conf_mount_rw();
	config_lock();

	if (!is_dir('/usr/local/www/packages/cron/')) {
		exec("mkdir -p /usr/local/www/packages/cron/");
	}

	//Copy PHP files from .tmp to .php - don't move or put them in /tmp or the pkg system will miss them if it gets cleared.
	exec("cp /usr/local/pkg/cron.tmp /usr/local/www/packages/cron/cron.php");
	exec("cp /usr/local/pkg/cron_edit.tmp /usr/local/www/packages/cron/cron_edit.php");

	//write_config();

	write_rcfile(array(
		"file" => "cron.sh",
		"start" => "[ `/bin/pgrep -f 'cron -s' | wc -l` -eq 0 ] && /usr/sbin/cron -s &",
		"stop" => "[ -f \"/var/run/cron.pid\" ] && kill -9 `cat /var/run/cron.pid`; rm -f /var/run/cron.pid; /bin/pkill -f 'cron -s'"
		)
	);

	cron_sync_package();

	//if (pkg_is_service_running('cron')) {
		//documentation purposes
	//}

	conf_mount_ro();
	config_unlock();

}


function cron_deinstall_command()
{
	conf_mount_rw();
	config_lock();
	$handle = popen("/usr/local/etc/rc.d/cron.sh stop", "r");
	unlink_if_exists("/usr/local/pkg/cron.xml");
	unlink_if_exists("/usr/local/www/cron.inc");
	exec("rm -R /usr/local/www/packages/cron");
	//unlink_if_exists("/usr/local/etc/rc.d/cron.sh");
	conf_mount_ro();
	config_unlock();
}

?>
