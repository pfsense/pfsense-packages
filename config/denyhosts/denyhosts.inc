<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
    denyhosts.inc
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

function denyhosts_sync_package()
{
  
	//global $config; 

}
    
// bounty: https://forum.pfsense.org/index.php/topic,15791.0/topicseen.html
// pkg_add -r denyhosts
// python /usr/local/share/denyhosts/denyhosts.py –file=/var/log/auth.log
// /var/run/denyhosts.pid
// http://en.wikipedia.org/wiki/DenyHosts


function denyhosts_install_command()
{

	global $config;
	conf_mount_rw();
	config_lock();
	
	if (!is_dir('/usr/local/www/packages/')) {
	   exec("mkdir /usr/local/www/packages/");
	}
	
	if (!is_dir('/usr/local/www/packages/denyhosts/')) {
	   exec("mkdir /usr/local/www/packages/denyhosts/");
	}

	exec("pkg_add -r https://files.pfsense.org/packages/security/denyhosts-2.5.tbz");

	//misc files
	if (!is_dir('/usr/local/www/edit_area/')) {
		chdir('/tmp/');
		exec("cd /tmp/;fetch ".$download_path."edit_area.tgz");
		chdir('/usr/local/www');
		system('tar xvpfz /tmp/edit_area.tgz edit_area');
		unlink_if_exists("/tmp/edit_area.tgz");
	}

	//backup the original hosts.allow file
	exec ("cp /etc/hosts.allow /etc/hosts.allow.bak");

	$hosts_allow = "#\n";
	$hosts_allow .= "# hosts.allow access control file for \"tcp wrapped\" applications.\n";
	$hosts_allow .= "#\n";
	$hosts_allow .= "sshd : /etc/hosts.deniedssh : deny\n";
	$hosts_allow .= "sshd : ALL : allow\n";
	$hosts_allow .= "ALL : ALL : allow\n";
	$fout = fopen("/etc/hosts.allow","w");
	fwrite($fout, $hosts_allow);
	unset($hosts_allow);
	fclose($fout);

	if(!is_file("/etc/hosts.deniedssh")) {
		exec("touch /etc/hosts.deniedssh");
		exec("chmod 755 /etc/hosts.deniedssh");
	}

	if(!is_file("/var/log/denyhosts")) {
		exec ('touch /var/log/denyhosts');
	}

	$download_path = 'https://packages.pfsense.org/packages/config/denyhosts/';

	//rename PHP files from .tmp to .php
	chdir('/tmp/');
	exec("cd /tmp/;fetch ".$download_path."denyhosts_log.tmp");
	exec("cp /tmp/denyhosts_log.tmp /usr/local/www/packages/denyhosts/denyhosts_log.php");
	unlink_if_exists("/tmp/denyhosts_log.tmp");


	//$denyhosts_sh = "\n";
	//$denyhosts_sh .= "name=\"denyhosts\"\n";
	//$denyhosts_sh .= "rcvar=\${name}_enable\n";
	//$denyhosts_sh .= "\n";
	//$denyhosts_sh .= "command=\"/usr/local/bin/denyhosts.py\"\n";
	//$denyhosts_sh .= "command_interpreter=\"/usr/local/bin/python2.5\"\n";
	//$denyhosts_sh .= "command_args=\"--config /usr/local/etc/denyhosts.conf --daemon\"\n";
	//$denyhosts_sh .= "pidfile=\"/var/run/\${name}.pid\"\n";
	//$denyhosts_sh .= "\n";
	//$denyhosts_sh .= "load_rc_config \$name\n";
	//$denyhosts_sh .= "\n";
	//$denyhosts_sh .= ": \${denyhosts_enable=\"YES\"}\n";
	//$denyhosts_sh .= "\n";
	//$denyhosts_sh .= "run_rc_command \"\$1\"\n";
	//$fout = fopen("/usr/local/etc/rc.d/denyhosts.sh","w");
	//fwrite($fout, $denyhosts_sh);
	//unset($denyhosts_sh);
	//fclose($fout);
	//exec("chmod 755 /usr/local/etc/rc.d/denyhosts.sh");

	$filename = "/usr/local/etc/denyhosts.conf";
	$handle = fopen($filename,"rb");
	$denyhosts_conf = fread($handle, filesize($filename));	
	fclose($handle);	
	$denyhosts_conf = str_replace("SECURE_LOG = /var/log/auth.log", "SECURE_LOG = /var/log/system.log", $denyhosts_conf);
	$denyhosts_conf = str_replace("#BLOCK_SERVICE  = sshd", "BLOCK_SERVICE  = sshd", $denyhosts_conf);
	$denyhosts_conf = str_replace("#SYNC_SERVER = http://xmlrpc.denyhosts.net:9911", "SYNC_SERVER = http://xmlrpc.denyhosts.net:9911", $denyhosts_conf);
	$fout = fopen($filename,"w");
	fwrite($fout, $denyhosts_conf);
	unset($filename);
	fclose($fout);

	$filename = "/usr/local/etc/denyhosts.conf-dist";
	$fout = fopen($filename,"w");
	fwrite($fout, $denyhosts_conf);
	unset($filename);
	unset($denyhosts_conf);
	fclose($fout);

	write_rcfile(array(
		"file" => "denyhosts.sh",
		"start" => "/usr/local/bin/python2.5 /usr/local/bin/denyhosts.py --config /usr/local/etc/denyhosts.conf --daemon",
		"stop" => "rm /var/run/denyhosts.pid"
		)
	);
   
	denyhosts_sync_package();

	//start denyhosts
	exec("/usr/local/etc/rc.d/denyhosts.sh start");

	//if (pkg_is_service_running('notes')) {  
		//documentation purposes
	//}

	conf_mount_ro();
	config_unlock();
  
}


function denyhosts_deinstall_command()
{

	conf_mount_rw();
	config_lock();

	exec("pkg_delete denyhosts");
	exec("rm -R /usr/local/www/packages/denyhosts/");
	
	//restore original hosts.allow file
		if (file_exists('/usr/local/www/exec.php')) {
			exec ("rm /etc/hosts.allow");
			exec ("cp /etc/hosts.allow.bak /etc/hosts.allow");
		}

	//remove the configuration
		exec ("rm /usr/local/etc/denyhosts*");

	//create a new hosts.allow file
		//$hosts_allow = "#\n";
		//$hosts_allow .= "# hosts.allow access control file for \"tcp wrapped\" applications.\n";
		//$hosts_allow .= "#\n";
		//$hosts_allow .= "ALL : ALL : allow\n";
		//$fout = fopen("/etc/hosts.allow","w");
		//fwrite($fout, $tmp);
		//unset($tmp);
		//fclose($fout);

	conf_mount_ro();
	config_unlock();
}

?>