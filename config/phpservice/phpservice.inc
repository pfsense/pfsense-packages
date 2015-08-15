<?php
/*
	phpservice.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2008 Mark J Crane
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
function phpservice_sync_package() {
	global $config; 
	
	if ($config['installedpackages']['phpservice']['config'] != "") {
	
		conf_mount_rw();
		$tmp = <<<EOF
<?php

// Set time limit to indefinite execution
set_time_limit (0);

//run this program as long as the pid file exists
$filename = '/tmp/phpmonitor.pid';
$fp = fopen($filename, 'w');
fwrite($fp, "If this file is deleted then phpmonitor will stop.");
fclose($fp);
unset($filename);

function send_to_syslog($syslogaddress, $syslogport, $syslogmsg) {
	
	$syslogaddress = "127.0.0.1";
	$syslogport = 514;
	
	$fp = fsockopen("udp://".$syslogaddress, $syslogport, $errno, $errstr);
	if (!$fp) {
		return;
	} else {
		fwrite($fp, $syslogmsg);
		fclose($fp);
	}
}

$x = 0;
while ($x == 0) {

// If the pid file does not exist then close the program.
if (!file_exists('/tmp/phpmonitor.pid')) {
	return;
}

EOF;
	      
		foreach ($config['installedpackages']['phpservice']['config'] as $rowhelper) {
			if ($rowhelper['enabled'] != "false") {
				$tmp_php = base64_decode($rowhelper['php']);
				if (strlen($tmp_php) > 0) {
					$tmp .= "// name: ".$rowhelper['name']." \n";
					$tmp .= "// description: ".$rowhelper['description']." \n\n";
					$tmp .= base64_decode($rowhelper['php']);
					$tmp .= "\n";
				}
			}
		}

		$tmp .= <<< EOF

sleep(1);

}
?>

EOF;

		$fout = fopen("/usr/local/pkg/phpservice.php", "w");
		fwrite($fout, $tmp);
		unset($tmp);
		fclose($fout);

		restart_service("phpservice");
		conf_mount_ro();
	} else {
		stop_service("phpservice");
	}
}

function phpservice_install_command() {
	conf_mount_rw();
	write_rcfile(array(
		"file" => "phpservice.sh",
		"start" => "/usr/local/bin/php /usr/local/pkg/phpservice.php >> /var/log/phpservice.log &",
		"stop" => "/bin/rm -f /tmp/phpmonitor.pid; sleep 3"
		)
	);
	phpservice_sync_package();
	conf_mount_ro();
}


function phpservice_deinstall_command() {
	conf_mount_rw();
	stop_service("phpservice");
	unlink_if_exists("/usr/local/etc/rc.d/phpservice.sh");	
	conf_mount_ro();
}

?>
