<?php
/*
	tftp.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2008 Mark J Crane
	Copyright (C) 2011 Jim Pingle
	Copyright (C) 2015 ESF, LLC
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
function tftp_byte_convert($bytes) {
	if ($bytes <= 0) {
		return '0 Byte';
	}
	$convention = 1000;
	$s = array('B', 'kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB');
	$e = floor(log($bytes, $convention));
	return round($bytes/pow($convention, $e), 2) . ' ' . $s[$e];
}

function tftp_install_command() {
	global $config;

	$tftpdir = "/tftpboot";
	$tftpbackup = "/root/backup/tftp.bak.tgz";

	// Create the directories if required
	safe_mkdir("{$tftpdir}", 0777);
	safe_mkdir("/root/backup/");

	// Restore backup if it exists
	if (file_exists($tftpbackup)) {
		mwexec("/usr/bin/tar xvpfz {$tftpbackup} -C /");
		mwexec("/bin/chmod -R 0744 {$tftpdir}/*");
	}
	unset($tftpdir, $tftpbackup);
}

function tftp_deinstall_command() {
	unlink_if_exists("/usr/local/etc/rc.d/tftp.sh");
	unlink_if_exists("/tmp/pkg_mgr_tftp.log");
}

function tftp_generate_rules($type) {
	global $config, $FilterIflist;

	if ($type != "nat") {
		return;
	}

	// Add tftpd daemon to inetd
	$inetd_fd = fopen("/var/etc/inetd.conf", "a+");
	fwrite($inetd_fd, "tftp\t\tdgram\tudp\twait\t\troot\t/usr/libexec/tftpd\ttftpd /tftpboot\n");
	fclose($inetd_fd);

	if (!empty($config['installedpackages']['tftpd']['config'][0]['tftpdinterface'])) {
		$tftpifs = explode(",", $config['installedpackages']['tftpd']['config'][0]['tftpdinterface']);
		foreach ($tftpifs as $tftpif) {
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
