<?php

/* ========================================================================== */
/*
    bacula-client.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2012 Marcio Carlos Braga Antao
    Copyright (C) 2012-2013 Marcello Coutinho
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
    All rights reserved.
                                                                              */
/* ========================================================================== */
/*
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code MUST retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form MUST reproduce the above copyright
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
/* ========================================================================== */
 require_once("config.inc");
 require_once("util.inc");
 
 $pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version > 2.0)
	define('BACULA_LOCALBASE', '/usr/pbi/bacula-' . php_uname("m"));
else
  define('BACULA_LOCALBASE','/usr/local');
  
function baculaclient_custom_php_install_command(){
	global $g, $config;
	baculaclient_custom_php_write_config();	
}

function baculaclient_custom_php_deinstall_command(){
	global $g, $config;

	conf_mount_rw();

	// 1. Delete our config file
	unlink_if_exists(BACULA_LOCALBASE."/etc/bacula-fd.conf");

	// 2. Re-run sshd config generation script
	exec("/usr/local/etc/rc.d/bacula-fd.sh stop");
	conf_mount_ro();
}

function baculaclient_custom_php_write_config(){
	global $g, $config;
	conf_mount_rw();
	//check config_file
	$startup_file="/usr/local/etc/rc.d/bacula-fd";
	if (file_exists($startup_file)){
		$startup_script=file_get_contents($startup_file);
		$startup_script=preg_replace("/NO/","YES",$startup_script);
		$startup_script=preg_replace("@/usr/local/etc/bacula-fd.conf@",BACULA_LOCALBASE."/etc/bacula-fd.conf",$startup_script);
		file_put_contents("{$startup_file}.sh",$startup_script,LOCK_EX);
		// Ensure bacula-fd has a+rx
		exec("chmod a+rx {$startup_file}.sh");
		}

	//check config
	if (is_array($config['installedpackages']['baculaclient']['config'])){
		$baculaclient_conf="";
		foreach ($config['installedpackages']['baculaclient']['config'] as $bc) {
			// create Director
			switch ($bc['type']){
			case "Director":
				$baculaclient_conf .= "Director { \n\t Name = {$bc['director']}-dir #{$bc['description']}\n\t Password = \"{$bc['password']}\"\n\t}\n"; 
				Break;
			case "Monitor":
				$baculaclient_conf .= "Director { \n\t Name = {$bc['director']}-mon #{$bc['description']}\n\t Password = \"{$bc['password']}\"\n\t Monitor = yes\n\t}\n";
				break;
			case "Local":
				$baculaclient_conf .= "Director { \n\t Name = {$bc['director']}-dir #{$bc['description']}\n\t Password = \"{$bc['password']}\"\n\t}\n";
				$baculaclient_conf .= "Director { \n\t Name = {$bc['director']}-mon #{$bc['description']}\n\t Password = \"{$bc['password']}\"\n\t Monitor = yes\n\t}\n";
				$LocalDirector = $bc['director'];
				}
				
			}
		
		// create Messages
		$baculaclient_conf .= "Messages { \n\t Name = Standard \n\t director  = {$LocalDirector}-dir = all, !skipped, !restored\n\t}\n";
		// create FielDaemon
			
		if (is_array($config['installedpackages']['baculaclientfd']['config'])){
			$port = $config['installedpackages']['baculaclientfd']['config'][0]['port'];
			$jobs = $config['installedpackages']['baculaclientfd']['config'][0]['jobs'];
			}
		else{
			$port="9102";
			$jobs="20";	
			}
		$baculaclient_conf .= "FileDaemon { \n\t Name = {$LocalDirector}-fd #\n\t FDport = {$port}\n\t WorkingDirectory = /var/db/bacula\n\t Pid Directory = /var/run\n\tMaximum Concurrent Jobs = {$jobs}\n\t}\n";
		file_put_contents(BACULA_LOCALBASE."/etc/bacula-fd.conf",$baculaclient_conf,LOCK_EX);
		exec("/usr/local/etc/rc.d/bacula-fd.sh restart");
		// Mount Read-only
		conf_mount_ro();
		}
	}

	?>