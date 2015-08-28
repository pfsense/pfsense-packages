<?php
/*
	nrpe2.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2009 Erik Kristensen <erik@erikkristensen.com>
	Copyright (C) 2011-2013 Jim Pingle
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
require_once('filter.inc');

global $pfs_version;
$pfs_version = substr(trim(file_get_contents("/etc/version")), 0, 3);

if ($pfs_version == "2.1" || $pfs_version == "2.2") {
	define('NRPE_BASE', '/usr/pbi/nrpe-' . php_uname("m"));
} else {
	define('NRPE_BASE', '/usr/local');
}

if ($pfs_version == "2.1") {
	define('NRPE_BINARY', NRPE_BASE . "/sbin/nrpe2");
} else {
	define('NRPE_BINARY', "/usr/local/sbin/nrpe2");
}

define('NRPE_CONFIG_DIR', NRPE_BASE . '/etc');
define('NRPE_RCFILE', '/usr/local/etc/rc.d/nrpe2.sh');


function nrpe2_custom_php_install_command() {
	global $g, $config;
	$NRPE_BASE = NRPE_BASE;
	$NRPE_CONFIG_DIR = NRPE_CONFIG_DIR;

	$ip = $config['interfaces']['lan']['ipaddr'];

	if (!is_array($config['installedpackages']['nrpe2'])) {
		$config['installedpackages']['nrpe2']['config'][0]['enabled'] = "on";
		$config['installedpackages']['nrpe2']['config'][0]['server_address'] = $ip;
		$config['installedpackages']['nrpe2']['config'][0]['server_port'] = 5666;
		$config['installedpackages']['nrpe2']['config'][0]['allowed_hosts'] = "127.0.0.1";
		$config['installedpackages']['nrpe2']['config'][0]['dont_blame_nrpe'] = "on";
	}

	if (!is_array($config['installedpackages']['nrpe2']['config'][0]['row'])) {
		$config['installedpackages']['nrpe2']['config'][0]['row'] = array(
			0 => array(
				'name' => 'check_users',
				'command' => 'check_users',
				'warning' => '5',
				'critical' => '10'
			),
			1 => array(
				'name' => 'check_load',
				'command' => 'check_load',
				'warning' => '15,10,5',
				'critical' => '30,25,20',
			),
			2 => array(
				'name' => 'check_root',
				'command' => 'check_disk',
				'warning' => '20%',
				'critical' => '10%',
				'extra' => '-p /'
			),
			3 => array(
				'name' => 'check_var',
				'command' => 'check_disk',
				'warning' => '20%',
				'critical' => '10%',
				'extra' => '-p /var/run'
			),
			4 => array(
				'name' => 'check_zombie_procs',
				'command' => 'check_procs',
				'warning' => '5',
				'critical' => '10',
				'extra' => '-s Z'
			),
			5 => array(
				'name' => 'check_total_procs',
				'command' => 'check_procs',
				'warning' => '150',
				'critical' => '200'
			)
		);
	}
	unlink_if_exists(NRPE_CONFIG_DIR . '/rc.d/nrpe2');
	$nrpe2_binary = NRPE_BINARY;
	$fd = fopen(NRPE_RCFILE, 'w');
	$rc_file = <<<EOD
#!/bin/sh
#
# $FreeBSD: ports/net-mgmt/nrpe2/files/nrpe2.in,v 1.3 2008/12/25 20:24:43 miwi Exp $
#

# PROVIDE: nrpe2
# REQUIRE: NETWORKING
# KEYWORD: shutdown

#
# Add the following lines to /etc/rc.conf to enable nrpe2:
# nrpe2_enable (bool):    Set to "NO" by default.
#                         Set it to "YES" to enable nrpe2.
# nrpe2_flags (str):      Set to "" by default.
# nrpe2_configfile (str): Set to "{$NRPE_CONFIG_DIR}/nrpe.cfg" by default.
# nrpe2_pidfile (str):    Set to "/var/spool/nagios/nrpe2.pid" by default.
#

. /etc/rc.subr

nrpe2_enable=\${nrpe2_enable-"YES"}
name="nrpe2"
rcvar="\${name}_enable"

command="{$nrpe2_binary}"
command_args="-d"
extra_commands="reload"

sig_reload=HUP

[ -z "\${nrpe2_flags}" ] && nrpe2_flags=""
[ -z "\${nrpe2_configfile}" ] && nrpe2_configfile="{$NRPE_CONFIG_DIR}/nrpe.cfg"
[ -z "\${nrpe2_pidfile}" ] && nrpe2_pidfile="/var/run/nrpe2.pid"

load_rc_config "\${name}"

pidfile="\${nrpe2_pidfile}"

required_files="\${nrpe2_configfile}"
command_args="\${command_args} -c \${nrpe2_configfile}"

run_rc_command "$1"
EOD;
	fwrite($fd, $rc_file);
	fclose($fd);
	chmod(NRPE_RCFILE, 0755);

}

function nrpe2_custom_php_write_config() {
	global $g, $config;
	$nagios_check_path = NRPE_BASE . "/libexec/nagios";

	conf_mount_rw();
	$cmds = array();
	foreach ($config['installedpackages']['nrpe2']['config'][0]['row'] as $cmd) {
		$sudo_bin = "/usr/local/bin/sudo";
		$sudo = (isset($cmd['sudo']) && is_executable($sudo_bin)) ? "{$sudo_bin} " : "";
		$wcmd = !empty($cmd['warning']) ? "-w {$cmd['warning']}" : "";
		$ccmd = !empty($cmd['critical']) ? "-c {$cmd['critical']}" : "";
		if (is_executable("{$nagios_check_path}/{$cmd['command']}"))
			$cmds[] = "command[{$cmd['name']}]={$sudo}{$nagios_check_path}/{$cmd['command']} {$wcmd} {$ccmd} {$cmd['extra']}\n";
	}
	$commands = implode($cmds);

	$server_port = $config['installedpackages']['nrpe2']['config'][0]['server_port'];
	$allowed_hosts = $config['installedpackages']['nrpe2']['config'][0]['allowed_hosts'];
	$dont_blame_nrpe = $config['installedpackages']['nrpe2']['config'][0]['dont_blame_nrpe'];
	if ($config['installedpackages']['nrpe2']['config'][0]['dont_blame_nrpe'] == "on") {
		$dont_blame_nrpe = 1;
	} else {
		$dont_blame_nrpe = 0;
	}

	$fd = fopen(NRPE_CONFIG_DIR . '/nrpe.cfg', 'w');
	$nrpe_cfg = <<<EOD
log_facility=daemon
pid_file=/var/run/nrpe2.pid
server_port={$server_port}
nrpe_user=nagios
nrpe_group=nagios
allowed_hosts={$allowed_hosts}
dont_blame_nrpe={$dont_blame_nrpe}
debug=0
command_timeout=60
connection_timeout=300
{$commands}
EOD;
	if (defined($config['installedpackages']['nrpe2']['config'][0]['server_address'])) {
		$server_address = $config['installedpackages']['nrpe2']['config'][0]['server_address'];
		$nrpe_cfg .= "server_address={$server_address}";
	}
	fwrite($fd, $nrpe_cfg);
	fclose($fd);
	conf_mount_ro();
}

function nrpe2_custom_php_service() {
	global $config;

	if ($config['installedpackages']['nrpe2']['config'][0]['enabled'] == "on") {
		restart_service("nrpe2");
	} else {
		stop_service("nrpe2");
	}
}

function nrpe2_get_commands() {
	$nagios_check_path = NRPE_BASE . "/libexec/nagios";
	$commands = glob("{$nagios_check_path}/check_*");
	$cmdarr = array();
	foreach ($commands as $cmd)
		$cmdarr[]["command"] = basename($cmd);
	return $cmdarr;
}
?>
