<?php
/* $Id$ */
/* ========================================================================== */
/*
    puppet.inc
    part of the puppet package for pfSense
    Copyright (C) 2014 Frank Wall <fw@moov.de>

    based on the apcupsd package for pfSense
    Copyright (C) 2013 Danilo G. Baio <dbaio@bsd.com.br>

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
require_once("util.inc");
require_once("functions.inc");
require_once("pkg-utils.inc");
require_once('service-utils.inc');
require_once("globals.inc");

// check pfsense version
$pfs_version = substr(trim(file_get_contents("/etc/version")),0,3);
if ($pfs_version > 2.0){
	define('PUPPET_BASE', '/usr/pbi/puppet-' . php_uname("m"));
} else {
	define('PUPPET_BASE', '/usr/local');
}

// define globals
define('FACTER_CMD', PUPPET_BASE . '/bin/facter');
define('PUPPET_AGENTLOG', '/var/log/puppet.log');
define('PUPPET_AGENTPID', '/var/run/puppet/agent.pid');
define('PUPPET_CMD', PUPPET_BASE . '/bin/puppet');
define('PUPPET_CONFFILE', PUPPET_BASE . '/etc/puppet/puppet.conf');
define('PUPPET_DATADIR', '/var/puppet');
define('PUPPET_GROUP', 'puppet');
define('PUPPET_PIDDIR', '/var/run/puppet');
define('PUPPET_RCFILE', PUPPET_BASE . '/etc/rc.d/puppet');
define('PUPPET_RUN_SUMMARY', '/var/puppet/state/last_run_summary.yaml');
define('PUPPET_USER', 'puppet');

function php_install_puppet() {
	global $config, $g;

	conf_mount_rw();
        exec("/usr/bin/sed -i -e 's/puppet_enable=\"NO\"/puppet_enable=\"YES\"/g' " . PUPPET_RCFILE);
	conf_mount_ro();
}

function php_deinstall_puppet() {
	global $config, $g;

	conf_mount_rw();

	// Stop agent and cleanup.
	exec(PUPPET_RCFILE . ' stop');
	exec('/usr/bin/killall puppet');
	unlink_if_exists(PUPPET_RCFILE);
	unlink_if_exists(PUPPET_CONFFILE);
	unlink_if_exists(PUPPET_AGENTLOG);
	unlink_if_exists(PUPPET_AGENTPID);

	// Purging the datadir would reset the agent during upgrades.
	//if (is_dir(PUPPET_DATADIR))
	//	exec('/bin/rm -r ' . PUPPET_DATADIR);

	// Purge obsolete package data
	if (is_dir(PUPPET_PIDDIR))
		exec('/bin/rm -r ' . PUPPET_PIDDIR);

	// Remove user and group
        exec('/usr/sbin/pw userdel ' . PUPPET_USER);
        exec('/usr/sbin/pw groupdel ' . PUPPET_GROUP);

	conf_mount_ro();
}

function validate_input_puppet($post,&$input_errors) {

	if  (isset($post['enable'])){
	
		if ($post['configtimeout'] != '' && !is_numericint($post['configtimeout'])) {
			$input_errors[]='Config Timeout is not numeric.';
		}

	} // agent enabled
}

function sync_package_puppet() {
	global $config, $g;

	conf_mount_rw();

	file_put_contents('/tmp/puppet_test', 'puppet test 123' . PUPPET_BASE, LOCK_EX);
	file_put_contents('/usr/local/etc/puppet/puppet_test', "puppet test: " . PUPPET_CONFFILE, LOCK_EX);

	// puppet settings
	if (is_array($config['installedpackages']['puppet'])) {
		$puppet_config = $config['installedpackages']['puppet']['config'][0];
		if ($puppet_config['enable'] == 'on') {
			$certname = ($puppet_config['certname'] != '' ? "{$puppet_config['certname']}" : "{$config['system']['hostname']}.{$config['system']['domain']}");
			$configtimeout = ($puppet_config['configtimeout'] != '' ? "{$puppet_config['configtimeout']}" : '2');
			$environment = ($puppet_config['environment'] != '' ? "{$puppet_config['environment']}" : 'production');
			$server = ($puppet_config['server'] != '' ? "{$puppet_config['server']}" : 'puppet');
	
			include("/usr/local/pkg/puppet.conf.php");
			file_put_contents(PUPPET_CONFFILE, $puppetconf, LOCK_EX);
		}
	}

	// puppet service
	if (is_array($puppet_config) && $puppet_config['enable'] == 'on') {
		puppet_restart();
	} else {
		puppet_stop();
	}

	conf_mount_ro();
}

// Facter output
function puppet_facts() {
    exec("PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin " . FACTER_CMD . " --puppet 2>&1", $output);
    return implode("\n", $output);
}

// Puppet fingerprint
function puppet_fingerprint() {
    exec("PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin " . PUPPET_CMD . " agent --fingerprint 2>&1", $output);
    return implode("\n", $output);
}

// Puppet run summary
function puppet_run_summary() {
    $checks = array('changed', 'failed', 'failure', 'failed_to_restart', 'out_of_sync', 'scheduled', 'skipped');
    $summary = array();
    foreach ($checks as $check) {
        exec("/usr/bin/awk '/$check:/ {print \$2}' " . PUPPET_RUN_SUMMARY, $output);
        if (empty($output)) {
            $summary[$check] = 'unknown error';
        } else {
            $summary[$check] = implode("\n", $output);
        }
        unset($output);
    }
    return $summary;
}

// Service status
function puppet_servicestatus() {
    if (is_pid_running("PUPPET_AGENTPID")) {
        return true;
    } else {
        return false;
    }
}

// Startup function
function puppet_start() {
	mwexec_bg(PUPPET_RCFILE . ' start');
}

// Shutdown function
function puppet_stop() {
	mwexec_bg(PUPPET_RCFILE . ' onestop');
}

// Restart function
function puppet_restart() {
	mwexec_bg(PUPPET_RCFILE . ' restart');
}

?>
