<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
	notes.inc
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

function notes_sync_package() {

	//global $config;

}


function notes_install_command() {

	global $config;
	conf_mount_rw();
	config_lock();

	if (!is_dir('/usr/local/www/packages/')) {
		exec("mkdir /usr/local/www/packages/");
	}

	//if (!is_dir('/usr/local/www/packages/notes/')) {
	//   exec("mkdir /usr/local/www/packages/notes/");
	//}

	//rename PHP files from .tmp to .php
	//exec("cp /tmp/notes.tmp /usr/local/www/packages/phpservice/notes.php");
	//unlink_if_exists("/tmp/notes.tmp");

	//write_config();

	//write_rcfile(array(
	//	"file" => "notes.sh",
	//	"start" => "/usr/local/bin/php /usr/local/pkg/notes.php",
	//	"stop" => "rm /tmp/phpmonitor.pid"
	//	)
	//);

	notes_sync_package();

	//if (pkg_is_service_running('notes')) {
		//documentation purposes
	//}

	conf_mount_ro();
	config_unlock();

}


function notes_deinstall_command() {

	conf_mount_rw();
	config_lock();
	unlink_if_exists("/usr/local/pkg/notes.xml");
	unlink_if_exists("/usr/local/pkg/notes.inc");
	conf_mount_ro();
	config_unlock();

}

?>