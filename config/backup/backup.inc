<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
    backup.inc
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


function byte_convert( $bytes ) {

    if ($bytes<=0)
        return '0 Byte';

    $convention=1000; //[1000->10^x|1024->2^x]
    $s=array('B', 'kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB');
    $e=floor(log($bytes,$convention));
    return round($bytes/pow($convention,$e),2).' '.$s[$e];
}

 
function backup_sync_package_php()
{

  global $config; 
  if($config['installedpackages']['backup']['config'] != "") {

    conf_mount_rw();
    config_unlock();
				      
		foreach($config['installedpackages']['backup']['config'] as $rowhelper) {
			if ($rowhelper['enabled'] != "false") {
				//$tmp_php = base64_decode($rowhelper['php']);
				if (strlen($tmp_php) > 0) {
					$tmp .= "// name: ".$rowhelper['name']." \n";
					$tmp .= "// description: ".$rowhelper['description']." \n\n";				      
					$tmp .= base64_decode($rowhelper['php']);
					$tmp .= "\n";
				}
			}

		}
		          
		conf_mount_ro();

  }
}


function php_sync_package() 
{
  
	global $config;   
	backup_sync_package_php();

}
    

function php_install_command() 
{

	global $config;
	conf_mount_rw();
	config_lock();

	if (!is_dir('/usr/local/www/packages/')) {
	   exec("mkdir /usr/local/www/packages/");
	}
	
	if (!is_dir('/usr/local/www/packages/backup/')) {
	   exec("mkdir /usr/local/www/packages/backup/");
	}

	
	//rename PHP files from .tmp to .php
	exec("cp /tmp/backup.tmp /usr/local/www/packages/backup/backup.php");
	unlink_if_exists("/tmp/backup.tmp");
	
	exec("cp /tmp/backup_edit.tmp /usr/local/www/packages/backup/backup_edit.php");
	unlink_if_exists("/tmp/backup_edit.tmp");
		
  //write_config();
	  	

		  	       
	php_sync_package();

	conf_mount_ro();
	config_unlock();
  
}


function deinstall_command() 
{

	conf_mount_rw();
	config_lock();
	unlink_if_exists("/usr/local/pkg/backup.xml");
	unlink_if_exists("/usr/local/pkg/backup.inc");
	exec("rm /usr/local/www/packages/backup/");
	conf_mount_ro();
	config_unlock();
	
}

?>