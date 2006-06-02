<?php
require_once('config.inc');

define('VIRALATOR_DOWNLOADDIR', '/usr/local/www/viralator');
define('SQUIRM_CONF', '/usr/local/etc/squirm/squirm.conf');
define('SQUIRM_PATTERNS', '/usr/local/etc/squirm/squirm.patterns');
define('FORBIDDEN_EXTS', 'html htm css php pl js vbs jpg jpeg gif png');

function viralator_install_command() {
	if (!is_dir(VIRALATOR_DOWNLOADDIR)) mkdir(VIRALATOR_DOWNLOADDIR);

	if (!is_file(SQUIRM_CONF)) {
		$conf = <<<EOD
begin
network 0.0.0.0/0
pattern squirm.patterns get
end

EOD;
		file_put_contents(SQUIRM_CONF, $conf);
	}

	if (!is_dir(VIRALATOR_DOWNLOADDIR))
		make_dirs(VIRALATOR_DOWNLOADDIR);
}

function viralator_validate_input($post, $input_errors) {
	if (trim($post['http_otherexts'])) {
		foreach (explode(',', $post['http_otherexts']) as $ext) {
			$ext = trim($ext);
			if (in_array($ext, explode(' ', FORBIDDEN_EXTS)))
				$input_errors[] = "The HTTP virus scanner can't scan *.$ext files.";
			else if (!preg_match('/\w+/', $ext))
				$input_errors[] = "*.ext is not a valid file extension.";
		}
	}
}

function viralator_get_real_lan_ip() {
	global $config;

	$lan_if = $config['interfaces']['lan']['if'];
	$line = trim(shell_exec("ifconfig $lan_if | grep inet | grep -v inet6"));
	list($dummy, $ip) = explode(' ', $line);

	return $ip;
}

function viralator_escape_dots($value) {
	return preg_replace('/\./', '\\.', $value);
}

function viralator_resync() {
	global $config;
	$settings = $config['installedpackages']['clamav']['config'][0];

	$exts = (empty($settings['http_exts']) ? array() : explode(',', $settings['http_exts']));

	if (trim($settings['http_otherexts'])) {
		foreach (explode(',', $settings['http_otherexts']) as $ext)
			$exts[] = trim($ext);
	}

	$towrite = '';

	// TODO: Escape any interface Squid listens on
	$lan_ip = viralator_get_real_lan_ip();
	if (is_ipaddr($lan_ip)) {
		$escaped = viralator_escape_dots($lan_ip);
		$towrite .= "abort regexi (^http://$escaped/.*)\n";
	}

	foreach (explode(' ', FORBIDDEN_EXTS) as $ext)
		$towrite .= "abort .$ext\n";

	foreach ($exts as $ext)
		$towrite .= "regexi (^.*\\.$ext\$) http://$lan_ip/viralator.pl?url=\\1\n";

	file_put_contents(SQUIRM_PATTERNS, $towrite);

	if (is_package_installed('squid')) {
		require_once('squid.inc');
		squid_resync();
	}
}
?>
