<?php
/*
	surftool.inc
	Copyright (C) 2015 H-T Reimers <reimers@mail.de>
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

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('filter.inc');
require_once('service-utils.inc');

if (file_exists('surftool.inc')) {
  require_once('surftool.inc');
}
else surftool_update_log("File 'surftool.inc' not found.");

global $pfs_version;
$pfs_version = substr(trim(file_get_contents("/etc/version")),0,3);
switch ($pfs_version) {
	case "2.1":
		break;
	case "2.2":
		break;
	default:
		break;
}

define ('CMD_PKGDELETE', 'pkg_delete surftool-0.01');

// enable GUI debug
define('SURFTOOL_GUI_DEBUG', 'on');
define('SURFTOOL_LOG_FILE',  '/tmp/surftool_gui.log');

// configuration settings !-- CHECK THIS --!
define('SURFTOOL_WWWPATH',			'/usr/local/www/surftool/');
define('SURFTOOL_WWWCONFIGFILE',		SURFTOOL_WWWPATH.'surftool.ini');
define('SURFTOOL_STARTUP_SCRIPT', '/usr/local/etc/rc.d/surftool.sh');

// xml variables
define('SURFTOOL_LANGUAGE',				'surftool_lang');
define('SURFTOOL_REFRESH_TIME',				'surftool_refresh_time');
define('SURFTOOL_SQUID_BINARY_PATH',			'surftool_squid_binary');
define('SURFTOOL_SQUIDGUARD_BINARY_PATH',	'surftool_sg_binary');
define('SURFTOOL_SQUIDGUARD_CONFIGFILE',	'surftool_sg_config_file');
define('SURFTOOL_SQUIDGUARD_DATABASE',		'surftool_sg_db');
define('SURFTOOL_GROUP_ONPLUS',			'surftool_group_onplus_name');
define('SURFTOOL_GROUP_ONLY',				'surftool_group_only_name');
define('SURFTOOL_GROUP_ADMINFREE',				'surftool_group_adminfree_name');
define('SURFTOOL_DEBUGLEVEL',				'surftool_debuglevel');
define('SURFTOOL_LOGFILE',					'surftool_logfile');
define('SURFTOOL_CMD_PATH',					'surftool_command_path');
define('SURFTOOL_LOGOUT_TARGET',					'surftool_logout_target');
define('SURFTOOL_REDIRECT_SECRET',					'surftool_redirect_secret');
define('SURFTOOL_USER_NAME',					'surftool_user_name');
define('SURFTOOL_USER_PASSWORD',					'surftool_user_password');
define('SURFTOOL_DAEMON_EXECUTABLE',					'surftool_daemon_executable');
define('SURFTOOL_WORKER_EXECUTABLE',					'surftool_worker_executable');
define('SURFTOOL_CUSTOM_OPTIONS',					'surftool_custom_options');
define('SURFTOOL_PHP',					'surftool_php');


function surftool_install() {
	global $pfs_version;
	global $g, $config;
	$SURFTOOL_STARTUP_SCRIPT=SURFTOOL_STARTUP_SCRIPT;
	surftool_update_log("surftool_install: started" );
	//Write rc.d script
	surftool_write_config_file();
	surftool_write_rcfile();
	//mwexec(SURFTOOL_STARTUP_SCRIPT."  start");
	//mwexec("/usr/local/etc/rc.d/surftool.sh  start");
	surftool_update_log("Starting daemon... One moment please...");
	sleep(2);
	//Start service
	mwexec("/usr/local/etc/rc.d/surftool.sh  restart");
	surftool_update_log("surftool_install: finish");
	
}

function surftool_deinstall() {
	global $g, $config;
	surftool_update_log("surftool_deinstall: started");
	//exec(SURFTOOL_STARTUP_SCRIPT . " stop");
	// delete cron task's
	//ls_setup_cron("lightsquid_squid_rotate", "", "", false);
	//ls_setup_cron("lightsquid_parser", "", "", false);


	surftool_update_log("surftool_deinstall: stopped");
}



function surftool_resync() {
	global $config, $pfs_version;
	surftool_write_config_file();
	surftool_write_rcfile();
}


function surftool_validate(&$post, &$input_errors){
	global $config;
	$cfg = $config['installedpackages']['surftool']['config'][0];

	$surftool_refresh_time = trim($cfg[SURFTOOL_REFRESH_TIME]);
	$surftool_squid_binary = $cfg[SURFTOOL_SQUID_BINARY_PATH];
	$surftool_sg_binary = $cfg[SURFTOOL_SQUIDGUARD_BINARY_PATH];
	$surftool_sg_config_file = $cfg[SURFTOOL_SQUIDGUARD_CONFIGFILE];
	$surftool_sg_db = $cfg[SURFTOOL_SQUIDGUARD_DATABASE];
	$surftool_group_onplus_name = $cfg[SURFTOOL_GROUP_ONPLUS];
	$surftool_group_only_name = $cfg[SURFTOOL_GROUP_ONLY];
	$surftool_group_adminfree_name = $cfg[SURFTOOL_GROUP_ADMINFREE];
	$surftool_debuglevel = $cfg[SURFTOOL_DEBUGLEVEL];
	$surftool_logfile = $cfg[SURFTOOL_LOGFILE];
  $surftool_command_path = $cfg[SURFTOOL_CMD_PATH];

//	if(!is_numeric($surftool_refresh_time)) $input_errors[] = "Refresh time: '$surftool_refresh_time' is not numeric";
//	if(!file_exists($surftool_squid_binary)) $input_errors[] = "The file: '$surftool_squid_binary' do not exists";
}


function surftool_write_config_file(){
	global $config;
	$cfg = $config['installedpackages']['surftool']['config'][0];


	$surftool_lang = $cfg[SURFTOOL_LANGUAGE];
	$surftool_refresh_time = $cfg[SURFTOOL_REFRESH_TIME];
	$surftool_squid_binary = $cfg[SURFTOOL_SQUID_BINARY_PATH];
	$surftool_sg_binary = $cfg[SURFTOOL_SQUIDGUARD_BINARY_PATH];
	$surftool_sg_config_file = $cfg[SURFTOOL_SQUIDGUARD_CONFIGFILE];
	$surftool_sg_db = $cfg[SURFTOOL_SQUIDGUARD_DATABASE];
	$surftool_group_onplus_name = $cfg[SURFTOOL_GROUP_ONPLUS];
	$surftool_group_only_name = $cfg[SURFTOOL_GROUP_ONLY];
	$surftool_group_adminfree_name = $cfg[SURFTOOL_GROUP_ADMINFREE];
	$surftool_debuglevel = $cfg[SURFTOOL_DEBUGLEVEL];
	$surftool_logfile = $cfg[SURFTOOL_LOGFILE];
	$surftool_command_path = $cfg[SURFTOOL_CMD_PATH];
	$surftool_logout_target = $cfg[SURFTOOL_LOGOUT_TARGET];
	$surftool_redirect_secret = $cfg[SURFTOOL_REDIRECT_SECRET];
	$surftool_user_name = $cfg[SURFTOOL_USER_NAME];
	$surftool_user_password = $cfg[SURFTOOL_USER_PASSWORD];
	$surftool_daemon_executable = $cfg[SURFTOOL_DAEMON_EXECUTABLE];
	$surftool_worker_executable = $cfg[SURFTOOL_WORKER_EXECUTABLE];
  $surftool_custom_options = $cfg[SURFTOOL_CUSTOM_OPTIONS];
  $surftool_custom_options = str_replace(";", "\n\t", $surftool_custom_options);
	$surftool_php = $cfg[SURFTOOL_PHP];



	$stc="; Surftool configuration
	language = \"".$surftool_lang."\"
	refresh_time = \"".$surftool_refresh_time."\"
	squid_binary_path = \"".$surftool_squid_binary ."\"
	squidGuard_binary_path = \"".$surftool_sg_binary."\"
	squidGuard_configfile = \"".$surftool_sg_config_file."\"
	squidGuard_database = \"".$surftool_sg_db."\"
	group_onplus = \"".$surftool_group_onplus_name."\"
	group_only = \"".$surftool_group_only_name."\"
	group_adminfree = \"".$surftool_group_adminfree_name."\"
	command_path = \"".$surftool_command_path."\"  
	user_name = \"".$surftool_user_name."\"  
	user_password = \"".$surftool_user_password."\"  
	debuglevel  = \"".$surftool_debuglevel."\"
	logfile= \"".$surftool_logfile."\"
	logout_target= \"".$surftool_logout_target."\"
	redirect_secret = \"".$surftool_redirect_secret."\"
	daemon_executable = \"".$surftool_daemon_executable."\"
	worker_executable = \"".$surftool_worker_executable."\"
	php_executable = \"".$surftool_php."\"
	$surftool_custom_options \n";

	file_put_contents(SURFTOOL_WWWCONFIGFILE,$stc);



}


// setup cron tasks
// original source from '/etc/inc/pfsense-utils.inc' function 'tdr_install_cron'
// this function safe for other tasks
// *****************************************************************************
// - $task_name: cron task name (for config identification) /for searching my cron tasks/
// - $options:   array=[0:minute][1:hour][2:mday][3:month][4:wday][5:who][6:cmd]
// - $task_key:  cron command key for searching
// - $on_off:	true-'on task', false-'off' task
// required: $task_nameand $on_off
// *****************************************************************************


/*
define('FIELD_TASKNAME', 'task_name');

function ls_setup_cron($task_name, $options, $task_key, $on_off) {
	global $config;
	surftool_update_log("ls_setup_cron: start task_name=$task_name, task_key=$task_key, on_off=$on_off");

	// check input params
	if(!$task_name) {
		surftool_update_log("ls_setup_cron: exit - uncomplete input params.");
		return;
	}

	// delete old task(s)
	if (is_array($config['cron']['item'])) {
		foreach ($config['cron']['item'] as $key => $item) {
		# unset crontask by name
		if (!empty($task_name) && ($item[FIELD_TASKNAME] == $task_name)) {
			unset($config['cron']['item'][$key]);
		} else
		# unset crontask by cmd
		if ($options[6] && (strpos($item['command'], $options[6]) !== false)) {
			unset($config['cron']['item'][$key]);
		}
		}
	}

	# install cron task
	if ($on_off) {
		if ($task_key) {
		if (is_array($options)) {
			# add new
			$cron_item = array();
			$cron_item[FIELD_TASKNAME]	= $task_name;
			$cron_item['minute']		= $options[0];
			$cron_item['hour']		= $options[1];
			$cron_item['mday']		= $options[2];
			$cron_item['month']		= $options[3];
			$cron_item['wday']		= $options[4];
			$cron_item['who']		= $options[5];
			$cron_item['command']		= $options[6];

			# check options
			if (!$cron_item['who']) $cron_item['who'] = "nobody";

			$config['cron']['item'][] = $cron_item;
			surftool_update_log("ls_setup_cron: add cron task '{$task_name}'='{$cron_item['command']}'");
		}
		} else
		// log
		surftool_update_log("ls_setup_cron: input prm 'task_key' not defined");
	}

	write_config("Installed cron task '$task_name' for 'lightsquid' package");
	configure_cron();
	surftool_update_log("ls_setup_cron: Apply new cron settings.");
}
*/
    
function surftool_update_log($log) {
	if (SURFTOOL_GUI_DEBUG === 'on') {
		$t_st_log = '';
		if (file_exists(SURFTOOL_LOG_FILE))
			$t_st_log = file_get_contents(SURFTOOL_LOG_FILE);
		$t_st_log .= "\n$log";
		file_put_contents(SURFTOOL_LOG_FILE, $t_st_log);
	}
}

function refresh_now() {
//	$cmd = CRONTAB_LS_TEMPLATE . " today";
	surftool_update_log("surftool refresh_now: execute command '$cmd'");
	// create lightsquid report catalog

/*	if (!file_exists(LS_REPORTPATH)) {
		surftool_update_log("lightsquid_install: Create report dir " . LS_REPORTPATH);
		mwexec("mkdir -p " . LS_REPORTPATH);
	}
	mwexec_bg($cmd);
*/
}

//check to see if the service is installed if it is return the id
	function get_service_id ($service_array, $fieldname, $fieldvalue) {
		$x = 0;
		$id = '';
		foreach($service_array as $rowhelper) {
			if ($rowhelper[$fieldname] == $fieldvalue) {
				$id = $x; //return the id
			}
			$x++;
		}
		if (strlen($id) > 0) {
			return ($id);
		}
		else {
			return false;
		}
	}



  


function surftool_write_rcfile() {
	global $config;
	$cfg = $config['installedpackages']['surftool']['config'][0];

	$inifile = $cfg[SURFTOOL_WWWCONFIGFILE];
	$daemon = $cfg[SURFTOOL_DAEMON_EXECUTABLE];
	$logfile = $cfg[SURFTOOL_LOGFILE];
	
	//if not set -> set to default 
	if($inifile=="") $inifile="/usr/local/www/surftool/surftool.ini";
	if($daemon=="") $worker="/usr/local/share/surftool/surftooldaemon.sh";
	if($logfile=="") $logfile="/var/log/surftool.log";

	$rc = array();
	$SURFTOOL_LOCALBASE = "/usr/local/share/surftool/" ; //SURFTOOL_LOCALBASE;
	$rc['file'] = 'surftool.sh';
	$rc['start'] = <<<EOD
	
	INIFILE='$inifile'
	STDAEMON='$daemon'
	LOGFILE='$logfile'


        if ! [ -f "\$STDAEMON" ]; then
		echo "Error surftooldaemon executable: file '\$STDAEMON' does not exist "
        fi

        if ! [ -f "\$INIFILE" ]; then
		echo "Error surftooldaemon config file '\$INIFILE' does not exist "
        fi

	#start daemon
	\$STDAEMON start \$INIFILE >> \$LOGFILE 2>&1 &
	
	
EOD;
	$rc['stop'] = <<<EOD
	
	INIFILE='{$inifile}'
	STDAEMON='{$daemon}'
	LOGFILE='{$logfile}'
	
	\$STDAEMON stop \$INIFILE 

EOD;
	$rc['restart'] = <<<EOD
	rc_stop
	rc_start
EOD;
	conf_mount_rw();
	write_rcfile($rc);
	conf_mount_ro();

}


?>
