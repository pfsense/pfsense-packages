<?php
/* ========================================================================== */
/*
	asterisk.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2012 Marcello Coutinho
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
/* ========================================================================== */


function asterisk_install() {
	sync_package_asterisk();
}

function asterisk_deinstall() {
	$script='/usr/local/etc/rc.d/asterisk';
	if (file_exists($script)){
		mwexec("$script stop");
		chmod ($script,0444);
		}
}

function sync_package_asterisk() {
	global $config, $g;

	#mount filesystem writeable
	conf_mount_rw();
	
	#fix asterisk options for nanobsd
	if ($g['platform'] == "nanobsd"){
		$script='/usr/local/etc/asterisk/logger.conf';
		if (file_exists($script)){
			$script_file=file_get_contents($script);
			$pattern[0]='/messages =/';
			$replace[0]='/tmp/asterisk.log =';
			$script_file=preg_replace($pattern,$replace,$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
		}
		
		$script='/usr/local/etc/asterisk/asterisk.conf';
		if (file_exists($script)){
			$script_file=file_get_contents($script);
			$pattern[0]='@astdbdir => [a-z,A-Z,/]+@';
			$replace[0]='astdbdir => /tmp';
			$pattern[1]='@astspooldir => [a-z,A-Z,/]+@';
			$replace[1]='astspooldir => /tmp';
			$script_file=preg_replace($pattern,$replace,$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
		}
		
	}
	
	$script='/usr/local/etc/rc.d/asterisk';
	if (file_exists($script)){
		$script_file=file_get_contents($script);
		if (preg_match('/NO/',$script_file)){
			$script_file=preg_replace("/NO/","YES",$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
			}
		chmod ($script,0755);
		mwexec("$script stop");			
		mwexec_bg("$script start");
	}
	#mount filesystem readonly
	conf_mount_ro();
	
}

?>
