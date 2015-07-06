<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
	tftp_inc.php
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

function tftp_guid() {
	if (function_exists('com_create_guid')){
		return com_create_guid();
	}else{
		mt_srand((double)microtime()*10000);//optional for php 4.2.0 and up.
		$charid = strtoupper(md5(uniqid(rand(), true)));
		$hyphen = chr(45);// "-"
		$uuid = chr(123)// "{"
				.substr($charid, 0, 8).$hyphen
				.substr($charid, 8, 4).$hyphen
				.substr($charid,12, 4).$hyphen
				.substr($charid,16, 4).$hyphen
				.substr($charid,20,12)
				.chr(125);// "}"
		return $uuid;
	}
}

function tftp_pkg_is_service_running($servicename) {
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

function tftp_byte_convert( $bytes ) {

	if ($bytes<=0)
		return '0 Byte';

	$convention=1000; //[1000->10^x|1024->2^x]
	$s=array('B', 'kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB');
	$e=floor(log($bytes,$convention));
	return round($bytes/pow($convention,$e),2).' '.$s[$e];
}

function tftp_install_command() {
	global $config;
	conf_mount_rw();

	if (!is_dir('/tftpboot')) {
		// Create the directory
		exec("mkdir /tftpboot");

		//Set the directory permissions
		exec("chmod -R 777 /tftpboot");
	}

	if (!is_dir('/root/backup/')) {
		// Create the backup directory
		exec("mkdir /root/backup/");
	}

	// if backup file exists restore it
	$filename = 'tftp.bak.tgz';

	//extract a specific directory to /usr/local/freeswitch
	if (file_exists('/root/backup/'.$filename)) {
		system('cd /; tar xvpfz /root/backup/tftp.bak.tgz');
		system('chmod -R 744 /tftpboot/*');
		unset($filename);
	}
}

function tftp_deinstall_command() {

	//exec("rm -R /tftpboot");
	unlink_if_exists("/usr/local/etc/rc.d/tftp.sh");
	unlink_if_exists("/tmp/pkg_mgr_tftp.log");
}

function tftp_generate_rules($type) {
	global $config, $FilterIflist;
	if ($type != "nat")
		return;
	// Open inetd.conf write handle
	$inetd_fd = fopen("/var/etc/inetd.conf","a+");
	/* add tftp daemon */
	fwrite($inetd_fd, "tftp\t\tdgram\tudp\twait\t\troot\t/usr/libexec/tftpd\ttftpd /tftpboot\n");
	fclose($inetd_fd);		// Close file handle

	if (!empty($config['installedpackages']['tftpd']['config'][0]['tftpdinterface'])) {
		$tftpifs = explode(",", $config['installedpackages']['tftpd']['config'][0]['tftpdinterface']);
		foreach($tftpifs as $tftpif) {
			if ($FilterIflist[$tftpif]) {
				log_error("Adding TFTP nat rules");
				$natrules .= "rdr pass on {$FilterIflist[$tftpif]['if']} proto udp from any to {$FilterIflist[$tftpif]['ip']} port 69 -> 127.0.0.1 port 69\n";
				$natrules .= "nat on {$FilterIflist[$tftpif]['if']} from 127.0.0.1 to any -> {$FilterIflist[$tftpif]['ip']} port 1024:65535 \n";
			}
		}
	}
	return $natrules;
}
?>
