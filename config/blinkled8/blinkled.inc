<?php
require_once("functions.inc");

function blinkled_rcfile() {
	global $config;
	$blinkled_config = $config['installedpackages']['blinkled']['config'][0];
	$stop = <<<EOD
if [ `/bin/pgrep blinkled | /usr/bin/wc -l` != 0 ]; then
		/usr/bin/killall -9 blinkled
		while [ `/bin/pgrep blinkled | /usr/bin/wc -l` != 0 ]; do
			sleep 1
		done
	fi
EOD;

	$start = "{$stop}\n";
	if (($blinkled_config['enable_led2']) && ($blinkled_config['iface_led2']))
		$start .= "\t" . blinkled_launch(convert_friendly_interface_to_real_interface_name($blinkled_config['iface_led2']), 2, true) . "\n";
	if (($blinkled_config['enable_led3']) && ($blinkled_config['iface_led3']))
		$start .= "\t" . blinkled_launch(convert_friendly_interface_to_real_interface_name($blinkled_config['iface_led3']), 3, true) . "\n";

	conf_mount_rw();
	write_rcfile(array(
		'file' => 'blinkled.sh',
		'start' => $start,
		'stop' => $stop)
	);
	conf_mount_ro();
}

function blinkled_running () {
	return ((int)exec('pgrep blinkled | wc -l') > 0);
}
function sync_package_blinkled() {
	global $config;
	$blinkled_config = $config['installedpackages']['blinkled']['config'][0];
	/* kill all instances of blinkled */
	if(blinkled_running()) {
		blinkled_stop();
	}
	/* if blinkled not running start it */
	if(!blinkled_running()) {
		blinkled_start();
	}
	blinkled_rcfile();
}

function blinkled_launch($int, $led, $return = false) {
	$cmd = "/usr/local/bin/blinkled -i " . escapeshellarg($int) . " -l " . escapeshellarg("/dev/led/led{$led}");
	if ($return)
		return $cmd;
	else
		mwexec($cmd);
}

function blinkled_start() {
	global $config;
	$blinkled_config = $config['installedpackages']['blinkled']['config'][0];
	if (!($blinkled_config['enable']))
		return;

	if (($blinkled_config['enable_led2']) && ($blinkled_config['iface_led2']))
		blinkled_launch(convert_friendly_interface_to_real_interface_name($blinkled_config['iface_led2']), 2);
	if (($blinkled_config['enable_led3']) && ($blinkled_config['iface_led3']))
		blinkled_launch(convert_friendly_interface_to_real_interface_name($blinkled_config['iface_led3']), 3);
}

function blinkled_stop() {
	mwexec("/usr/bin/killall -9 blinkled");
}

function validate_form_blinkled($post, $input_errors) {
	/* Make sure both aren't using the same interface */
	if (($post['iface_led2']) && ($post['iface_led3']) && 
	    (($post['enable_led2']) && ($post['enable_led3'])) &&
	    ($post['iface_led2'] == $post['iface_led3']))
		$input_errors[] = 'You cannot set two LEDs for a single interface. Please choose seperate interfaces.';
}

?>
