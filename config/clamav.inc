<?php
require_once('globals.inc');
require_once('config.inc');
require_once('service-utils.inc');
require_once('pkg-utils.inc');
require_once('pfsense-utils.inc');

function clamav_install_command() {
	global $g;
	mwexec('rm -f /usr/local/etc/rc.d/clamav*');

	$clamav_rundir = "{$g['varrun_path']}/clamav";

	$pidfile = "$clamav_rundir/clamd.pid";
	$rcfile = array();
	$rcfile['file'] = 'clamd.sh';
	$rcfile['start'] = "mkdir -p $clamav_rundir\n\tchown proxy:proxy $clamav_rundir\n\tclamd";
	$rcfile['stop'] = "kill `cat $pidfile`";
	write_rcfile($rcfile);

	$pidfile = "$clamav_rundir/freshclam.pid";
	$rcfile = array();
	$rcfile['file'] = 'freshclam.sh';
	$rcfile['start'] = "mkdir -p $clamav_rundir\n\tchown proxy:proxy $clamav_rundir\n\tfreshclam -d";
	$rcfile['start'] = 'freshclam -d';
	$rcfile['stop'] = "kill `cat $pidfile`";
	write_rcfile($rcfile);

	$dbdir = "{$g['vardb_path']}/clamav";
	make_dirs($dbdir);
	chown($dbdir, 'proxy');
	$conf = <<<EOD
DatabaseDirectory $dbdir
PidFile $pidfile
DatabaseOwner proxy
AllowSupplementaryGroups
DatabaseMirror database.clamav.net

EOD;
	file_put_contents('/usr/local/etc/freshclam.conf', $conf);

	$static_output = "Fetching the virus database. This can take long. You can cancel this at any time by clicking the stop button in your browser.\n";
	update_output_window($static_output);
	$static_output .= system('freshclam');
	update_output_window($static_output);
	$static_output .= "Done!\n";
	update_output_window($static_output);

	start_service('freshclam');
}

function clamav_resync() {
	global $g, $config;

	$settings = $config['installedpackages']['clamav']['config'][0];

	$clamav_rundir = "{$g['varrun_path']}/clamav";
	$logfile = "{$g['varlog_path']}/clamd.log";
	$pidfile = "$clamav_rundir/clamd.pid";
	$tempdir = $g['tmp_path'];
	$dbdir = "{$g['vardb_path']}/clamav";
	$socket = "$clamav_rundir/clamd.socket";

	$conf = <<<EOD
LogFile $logfile
LogTime
PidFile $pidfile
TemporaryDirectory $tempdir
DatabaseDirectory $dbdir
LocalSocket $socket
FixStaleSocket
User proxy
AllowSupplementaryGroups
ScanMail

EOD;

	if ($settings['expert_mode'] == 'on') $conf .= "DisableDefaultScanOptions";
	else {
		$options = array(	'brokenexec'	=> 'DetectBrokenExecutables',
					'scan_pe'	=> 'ScanPE',
					'scan_ole2'	=> 'ScanOLE2',
					'scan_html'	=> 'ScanHTML',
					'scan_archive'	=> 'ScanArchive');
		foreach ($options as $option => $directive) {
			if ($settings[$option] == 'on')
				$conf .= "$directive\n";
		}
	}

	file_put_contents('/usr/local/etc/clamd.conf', $conf);

	restart_service('clamav');

	if ($settings['scan_pop3'] == 'on') {
		require_once('p3scan.inc');
		p3scan_resync();
	}

	if ($settings['scan_smtp'] == 'on') {
		require_once('clamsmtp.inc');
		clamsmtp_resync();
	}

	if ($settings['scan_http'] == 'on') {
		require_once('viralator.inc');
		viralator_resync();
	}
}

function clamav_before_form($pkg) {
	global $config;

	if (is_package_installed('p3scan')) {
		$field = array();
		$field['fieldname'] = 'scan_pop3';
		$field['fielddescr'] = 'POP3 scanning';
		$field['description'] = 'Enable POP3 scanning.';
		$field['type'] = 'checkbox';
		$field['required'] = true;
		$field['enablefields'] = 'pop3_ifaces[]';
		$pkg['fields']['field'][] = $field;

		$field = array();
		$field['fieldname'] = 'pop3_ifaces';
		$field['fielddescr'] = 'POP3 interfaces';
		$field['description'] = 'The POP3 proxy will bind to the selected interfaces';
		$field['default_value'] = 'ALL';
		$field['type'] = 'interfaces_selection';
		$field['multiple'] = 'yes';
		$pkg['fields']['field'][] = $field;
	}

	if (is_package_installed('clamsmtp')) {
		$field = array();
		$field['fieldname'] = 'scan_smtp';
		$field['fielddescr'] = 'SMTP scanning';
		$field['description'] = 'Enable SMTP scanning.';
		$field['required'] = true;
		$field['type'] = 'checkbox';
		$field['enablefields'] = 'smtp_ifaces[],smtp_server,smtp_port';
		$pkg['fields']['field'][] = $field;

		$field = array();
		$field['fieldname'] = 'smtp_ifaces';
		$field['fielddescr'] = 'SMTP interfaces';
		$field['description'] = 'The SMTP proxy will bind to the selected interfaces';
		$field['default_value'] = 'ALL';
		$field['type'] = 'interfaces_selection';
		$field['multiple'] = 'yes';
		$pkg['fields']['field'][] = $field;

		$field = array();
		$field['fieldname'] = 'smtp_server';
		$field['fielddescr'] = 'SMTP server address';
		$field['description'] = 'Enter the IP address of the local SMTP server.';
		$field['type'] = 'input';
		$pkg['fields']['field'][] = $field;

		$field = array();
		$field['fieldname'] = 'smtp_port';
		$field['fielddescr'] = 'SMTP server port';
		$field['description'] = 'Enter the port of the local SMTP server.';
		$field['type'] = 'input';
		$pkg['fields']['field'][] = $field;
	}

	if (is_package_installed('viralator')) {
		$field = array();
		$field['fieldname'] = 'scan_http';
		$field['fielddescr'] = 'HTTP scanning';
		$field['description'] = 'Enable HTTP scanning. Note that this filtering is triggered by the proxy server. Therefore, to transparently filter out HTTP viruses, you need to set up the proxy server in transparent mode.';
		$field['required'] = 'yes';
		$field['type'] = 'checkbox';
		$field['enablefields'] = 'http_exts[],http_otherexts';
		$pkg['fields']['field'][] = $field;

		$field = array();
		$field['fieldname'] = 'http_exts';
		$field['fielddescr'] = 'Extensions to scan';
		$field['description'] = 'Extensions to be scanned by the HTTP virus scanner';
		$field['type'] = 'select';
		$exts = array();
		$exts['zip'] = 'Archives in the ZIP format';
		$exts['rar'] = 'Archives in the RAR format';
		$exts['arj'] = 'Archives in the ARJ format';
		$exts['gz'] = 'Files compressed in the GZ format';
		$exts['bz2'] = 'Files compressed in the BZIP2 format';
		$exts['exe'] = 'Windows/DOS PE (EXE) executables';
		$exts['com'] = 'DOS COM executables';
		$exts['bat'] = 'DOS Batch files';
		foreach ($exts as $ext => $desc)
			$field['options']['option'][] = array('name' => "$desc (*.$ext)", 'value' => $ext);
		$field['multiple'] = 'yes';
		$pkg['fields']['field'][] = $field;

		$field = array();
		$field['fieldname'] = 'http_otherexts';
		$field['fielddescr'] = 'Other extensions to scan';
		$field['description'] = 'Comma-separated list of extensions to be scanned by the HTTP virus scanner (e.g.: txt,ppt,doc,wmv).';
		$field['type'] = 'input';
		$pkg['fields']['field'][] = $field;
	}
}

function clamav_validate_input($post, $input_errors) {
	if ($post['scan_smtp'] == 'on') {
		require_once('clamsmtp.inc');
		clamsmtp_validate_input($post, &$input_errors);
	}

	if (is_package_installed('viralator')) {
		require_once('viralator.inc');
		viralator_validate_input($post, &$input_errors);
	}
}

function clamav_generate_rules($type) {
	global $config;

	if (!is_service_running('clamav')) return;

	$rules = '';
	$clamav_conf = $config['installedpackages']['clamav']['config'][0];
	$sassassin_conf = $config['installedpackages']['sassassin']['config'][0];

	$p3scan_enabled = (($clamav_conf['scan_pop3'] == 'on') || ($sassassin_conf['enable'] == 'on'));
	if ($p3scan_enabled && is_service_running('p3scan')) {
		$ifaces = explode(',', $clamav_conf['pop3_ifaces']);
		$ifaces = array_map('convert_friendly_interface_to_real_interface_name', $ifaces);

		switch($type) {
		case 'nat':
			foreach ($ifaces as $iface)
				$rules .= "rdr on $iface proto tcp to port pop3 -> 127.0.0.1 port 8110\n";
			break;
		case 'filter':
			foreach ($ifaces as $iface)
				$rules .= "pass quick on $iface proto tcp to port pop3 flags S/SA keep state\n";
			break;
		default:
			break;
		}
	}

	if ($clamav_conf['scan_smtp'] && is_service_running('clamsmtp')) {
		$ifaces = explode(',', $clamav_conf['pop3_ifaces']);
		$ifaces = array_map('convert_friendly_interface_to_real_interface_name', $ifaces);

		if ($type == 'nat') {
			foreach ($ifaces as $iface)
				$rules .= "rdr on $iface proto tcp to ($iface) port smtp -> 127.0.0.1 port 10025\n";
		}

		else {
			foreach ($ifaces as $iface)
				$rules .= "pass quick on $iface proto tcp to ($iface) port smtp flags S/SA keep state\n";
		}
	}

	return ($rules);
}
?>
