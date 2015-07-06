<?php
require_once("config.inc");
require_once("functions.inc");

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
}

function blinkled_launch($int, $led) {
	mwexec("/usr/local/bin/blinkled -i " . escapeshellarg($int) . " -l /dev/led/led" . escapeshellarg($led));
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

function validate_form_blinkled($post, &$input_errors) {
	/* Make sure both aren't using the same interface */
	if (($post['iface_led2']) && ($post['iface_led3']) && 
	    (($post['enable_led2']) && ($post['enable_led3'])) &&
	    ($post['iface_led2'] == $post['iface_led3']))
		$input_errors[] = 'You cannot set two LEDs for a single interface. Please choose seperate interfaces.';
}

?>
