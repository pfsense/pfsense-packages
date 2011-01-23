<?php
require_once('globals.inc');
require_once('config.inc');
require_once('service-utils.inc');

function sassassin_install_command() {
	global $g;

	$oldrc = '/usr/local/etc/rc.d/sa-spamd.sh';
	if (file_exists($oldrc)) unlink($oldrc);

	$pidfile = "$rundir/spamd.pid";
	$rcfile = array();
	$rcfile['file'] = 'spamd.sh';
	$rcfile['start'] = "mkdir -p $rundir\n\tspamd -u proxy -c -d -r $pidfile";
	$rcfile['stop'] = "kill `cat $pidfile`";
	write_rcfile($rcfile);
}

function sassassin_resync() {
	global $config;

	$settings = $config['installedpackages']['sassassin']['config'][0];

	$use_bayes = ($settings['bayes'] == 'on' ? 1 : 0);
	$bayes_auto_learn = ($settings['bayes_learn'] == 'on' ? 1 : 0);
	$required_score = $settings['threshold'];
	$rewrite_header = "Subject {$settings['subject']}";

	$report_safe_array = array('donotsend' => 0, 'attachment' => 1, 'plaintext' => 2);
	$report_safe = $report_safe_array[$settings['send_spam']];

	$local_cf = <<<EOD
use_bayes $use_bayes
bayes_auto_learn $bayes_auto_learn
required_score $required_score
rewrite_header $rewrite_header
report_safe $report_safe

EOD;

	foreach (array('whitelist' => 'wl', 'blacklist' => 'bl') as $list => $key) {
		foreach ($config['installedpackages']["sassassin{$key}"]['config'] as $item) {
			if (!empty($item))
				$local_cf .= "{$list}_from {$item['address']}\n";
		}
	}

	file_put_contents('/usr/local/etc/mail/spamassassin/local.cf', $local_cf);

	restart_service('spamassassin');

	if (get_pkg_id('p3scan') != -1) {
		require_once('p3scan.inc');
		p3scan_resync();
	}
}

function sassassin_validate_input($post, $input_errors) {
	if (!is_numeric($post['threshold']))
		$input_errors[] = 'The field \'Spam threshold\' should contain a numeric value.';
}
?>
