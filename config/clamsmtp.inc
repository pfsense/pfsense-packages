<?php
require_once('globals.inc');
require_once('config.inc');
require_once('service-utils.inc');
require_once('filter.inc');

function clamsmtp_install_command() {
	global $g;

	$clamav_rundir = "{$g['varrun_path']}/clamav";
	$pidfile = "$clamav_rundir/clamsmtpd.pid";
	$rcfile = array();
	$rcfile['file'] = 'clamsmtp.sh';
	$rcfile['start'] = "mkdir -p $clamav_rundir\n\tchown proxy:proxy $clamav_rundir\n\tclamsmtpd";
	$rcfile['stop'] = "kill `cat $pidfile`";
	write_rcfile($rcfile);

	clamsmtp_resync();
}

function clamsmtp_validate_input($post, $input_errors) {
	if (!is_ipaddr(trim($post['smtp_server'])))
		$input_errors[] = 'The field \'SMTP server\' must contain a valid IP address.';
	$port = trim($post['smtp_port']);
	if (!is_numeric($port) || ($port < 0) || ($port > 65535))
	$input_errors[] = 'The field \'SMTP server\' must contain a port ranging from 0 to 65535';
}

function clamsmtp_resync() {
	global $g, $config;

	$avconf = $config['installedpackages']['clamav']['config'][0];
	if ($avconf['scan_smtp'] == 'on')
		$outaddress = "{$avconf['smtp_server']}:{$avconf['smtp_port']}";
	else
		$outaddress = '127.0.0.1:25';
	$clamav_rundir = "{$g['varrun_path']}/clamav";
	$clamsocket = "$clamav_rundir/clamd.socket";
	$pidfile = "$clamav_rundir/clamsmtpd.pid";
	$conf = <<<EOD
Listen: 127.0.0.1:10025
OutAddress: $outaddress
ClamAddress: $clamsocket
User: proxy
PidFile: $pidfile

EOD;

	file_put_contents('/usr/local/etc/clamsmtpd.conf', $conf);

	restart_service('clamsmtp');
	filter_configure();
}
?>
