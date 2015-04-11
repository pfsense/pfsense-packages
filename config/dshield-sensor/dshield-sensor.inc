<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
    dshield-sensor.inc
    Copyright (C) 2015 Robert Nelson
    All rights reserved.
*/

require_once("config.inc");
require_once("services.inc");

global $dshield_sensor_cron_command;
$dshield_sensor_cron_command = "/usr/bin/nice -n20 /usr/local/pkg/dshield-sensor/cronjob";

function dshield_sensor_warn ($msg)
{
	 log_error("dshield-sensor: {$msg}");
}

function dshield_sensor_resync_config_command()
{
	global $config;
	global $input_errors;
	global $dshield_sensor_cron_command;

	$dshield_config = $config['installedpackages']['dshieldsensor']['config'][0];
	$config_file = '/var/etc/dshield.cnf';
	$sample_config_file = '/usr/pbi/dshield-sensor-'.php_uname("m").'/local/etc/dshield-sensor/dshield.cnf.sample';

	$configuration = "";
	$config_size = 0;
	$config_handle = fopen($config_file, "r");
	if ($config_handle !== FALSE) {
		$config_size = filesize($config_file);
	} else {
		$config_handle = fopen ($sample_config_file, "r");
		if ($config_handle !== FALSE) {
			$config_size = filesize($sample_config_file);
		}
	}
	if ($config_handle !== FALSE) {
		if ($config_size > 0) {
			$configuration = fread($config_handle, $config_size);
		} else {
			dshield_sensor_warn("Empty configuration file {$config_file}, reinstall");
		}
		fclose($config_handle);
	} else {
		dshield_sensor_warn("Couldn't open {$config_file} or {$sample_config_file}, reinstall");
	}

	$enabled = !empty($dshield_config['enable']);
	$userid = "0";
	if (!empty($dshield_config['userid'])) {
		$userid = $dshield_config['userid'];
	}
	$replyto = $config['notifications']['smtp']['notifyemailaddress'];
	$cc = "";
	if (!empty($dshield_config['ccreport'])) {
		$cc = $replyto;
	}
	$obfuscate = !empty($dshield_config['obfuscate']) ? "Y" : "N";

	$patterns = array('/^userid=.*$/m', '/^cc=.*$/m', '/^replyto=.*$/m', '/^obfus=.*$/m');
	$replacements = array('userid='.$userid, 'cc='.$cc, 'replyto='.$replyto, 'obfus='.$obfuscate);
	if ($config_size > 0) {
		$configuration = preg_replace($patterns, $replacements, $configuration);
	}
	$config_handle = fopen($config_file, "wb");
	if ($config_handle !== FALSE) {
		fwrite($config_handle, $configuration);
		fclose($config_handle);
	}
	if ($enabled) {
		$frequency = intval($dshield_config['frequency']);
		if ($frequency > 1) {
			$start_hour = mt_rand(0, $frequency - 1);
		} else {
			$start_hour = 0;
		}
		$start_minute = mt_rand(0, 59);

		install_cron_job($dshield_sensor_cron_command, TRUE, $start_minute, $start_hour."-23/".$frequency);
	} else {
		install_cron_job($dshield_sensor_cron_command, FALSE);
	}
}


function dshield_sensor_install_command()
{
	copy('/usr/pbi/dshield-sensor-'.php_uname("m").'/local/etc/dshield-sensor/dshield.cnf', '/var/etc/dshield.cnf');
	chmod('/var/etc/dshield-sensor.cnf', 0644);
}


function dshield_sensor_deinstall_command()
{
	global $dshield_sensor_cron_command;

	install_cron_job($dshield_sensor_cron_command, FALSE);
	unlink_if_exists('/var/etc/dshield.cnf');
}

?>

