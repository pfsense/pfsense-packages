<?php
/* $Id$ */
/*
 snort_chk_log_dir_size.php
 part of pfSense

 Modified for the Pfsense snort package v. 1.8+
 Copyright (C) 2009-2010 Robert Zelaya Developer
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

require_once("/usr/local/pkg/snort/snort.inc");

//        'B' => 1,
//        'KB' => 1024,
//        'MB' => 1024 * 1024,
//        'GB' => 1024 * 1024 * 1024,
//        'TB' => 1024 * 1024 * 1024 * 1024,
//        'PB' => 1024 * 1024 * 1024 * 1024 * 1024,


/* chk if snort log dir is full if so clear it */
$snortloglimit = $config['installedpackages']['snortglobal']['snortloglimit'];
$snortloglimitsize = $config['installedpackages']['snortglobal']['snortloglimitsize'];

if ($g['booting']==true) {
	exit(0);
}

if ($snortloglimit == 'off') {
	exit(0);
}

$snortloglimitDSKsize = exec('/bin/df -k /var | grep -v "Filesystem" | awk \'{print $4}\'');

$snortlogAlertsizeKB = snort_Getdirsize('/var/log/snort/alert');
$snortloglimitAlertsizeKB = round($snortlogAlertsizeKB * .70);
$snortloglimitsizeKB = round($snortloglimitsize * 1024);

/* do I need HUP kill ? */
if (snort_Getdirsize('/var/log/snort/') >= $snortloglimitsizeKB ) {

	conf_mount_rw();
	if(file_exists('/var/log/snort/alert')) {
		if ($snortlogAlertsizeKB >= $snortloglimitAlertsizeKB) {
			exec('/bin/echo "" > /var/log/snort/alert');
		}
		post_delete_logs();
		exec('/usr/sbin/chown snort:snort /var/log/snort/*');
		exec('/bin/chmod 660 /var/log/snort/*');
		//sleep(2);
		//exec('/usr/bin/killall -HUP snort');
	}
	conf_mount_ro();

}


?>
