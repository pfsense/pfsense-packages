<?php
require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('service-utils.inc');

if(!function_exists("filter_configure")) 
	requre_once("filter.inc");

define('VIRUSDIR', '/var/tmp/p3scan');

function p3scan_install_command() {
	global $g;

	$pidfile = "{$g['varrun_path']}/p3scan.pid";
	$rcfile = array();
	$rcfile['file'] = 'p3scan.sh';
	$rcfile['start'] = "p3scan -f /usr/local/etc/p3scan.conf -l $pidfile";
	$rcfile['stop'] = "kill `cat $pidfile`\nrm $pidfile";
	write_rcfile($rcfile);

	make_dirs(VIRUSDIR);
	chown(VIRUSDIR, 'proxy');
	chmod(VIRUSDIR, 0700);
	make_dirs(VIRUSDIR . '/children');
	chown(VIRUSDIR, 'proxy');
	chmod(VIRUSDIR, 0700);

	p3scan_resync();
}

function p3scan_resync() {
	global $g, $config;

	$virusdir = VIRUSDIR;
	$conf = <<<EOD
ip = 127.0.0.1
user = proxy
justdelete
virusregexp = .*: (.*) FOUND
virusdir = $virusdir
spamcheck = /usr/local/bin/spamc
template = /usr/local/etc/p3scan.mail
scannertype = basic

EOD;

	if ($config['installedpackages']['sassassin']['config'][0]['enable'] == 'on')
		$conf .= "checkspam\n";

	$virusen = $config['installedpackages']['clamav']['config'][0]['scan_pop3'];
	if ($virusen == 'on')
		$scanner = '/usr/local/bin/clamdscan --no-summary';
	else
		$scanner = 'echo';
	$conf .= "scanner = $scanner\n";

	file_put_contents('/usr/local/etc/p3scan.conf', $conf);

	restart_service('p3scan');
	filter_configure();
}
?>
