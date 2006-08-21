<?php
/*
	guiconfig.inc
	part of m0n0wall (http://m0n0.ch/wall)
	
	Copyright (C) 2003-2005 Manuel Kasper <mk@neon1.net>.
	All rights reserved.

	Modified for FreeNAS (http://freenas.org) by Olivier Cochard <cochard@gmail.com>

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

$d_mountdirty_path = $g['varrun_path'] . "/mount.dirty";
$d_diskdirty_path = $g['varrun_path'] . "/disk.dirty";
$d_raidconfdirty_path = $g['varrun_path'] . "/raid.conf.dirty";
$d_userconfdirty_path = $g['varrun_path'] . "/user.conf.dirty";
$d_groupconfdirty_path = $g['varrun_path'] . "/group.conf.dirty";

$freenas_config =& $config['installedpackages']['freenas']['config'][0];

function mount_sort() {
	global $g, $freenas_config;

	function mountcmp($a, $b) {
		return strcmp($a['mdisk'], $b['mdisk']);
	}

	usort($freenas_config['mounts']['mount'], "mountcmp");
}

function disks_sort() {
	global $g, $freenas_config;

	function diskcmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($freenas_config['disks']['disk'], "diskcmp");
}

function gvinum_sort() {
	global $g, $freenas_config;

	function gvinumcmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($freenas_config['raid']['vdisk'], "gvinumcmp");
	
}

function gmirror_sort() {
	global $g, $freenas_config;

	function gmirrorcmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($freenas_config['gmirror']['vdisk'], "gmirrorcmp");
	
}

/* TODO: This needs to be changed */
function users_sort() {
	global $g, $config;

	function userscmp($a, $b) {
		return strcmp($a['login'], $b['login']);
	}

	usort($config['system']['user'], "userscmp");
}

/* TODO: This needs to be changed */
function groups_sort() {
	global $g, $config;

	function groupscmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($config['system']['group'], "groupscmp");
}
?>