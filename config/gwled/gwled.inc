<?php
require_once("functions.inc");

function gwled_running () {
	return ((int)exec('pgrep -f gwled | wc -l') > 0);
}

function sync_package_gwled() {
	global $config;
	$gwled_config = $config['installedpackages']['gwled']['config'][0];
	led_normalize();
	/* kill all instances of gwled */
	if(gwled_running()) {
		gwled_stop();
	}
	gwled_start();
}

function gwled_start() {
	global $config;
	$gwled_config = $config['installedpackages']['gwled']['config'][0];
	if (!($gwled_config['enable']))
		return;

	// How often to run the check, in seconds.
	$interval = 15;

	if (($gwled_config['enable_led2']) && ($gwled_config['gw_led2'])
	|| ($gwled_config['enable_led3']) && ($gwled_config['gw_led3'])) {
		mwexec("/usr/local/bin/minicron {$interval} /var/run/gwled.pid /usr/local/bin/gwled.php");
		mwexec_bg("/usr/local/bin/gwled.php");
	}
}

function gwled_stop() {
	exec("/bin/pkill -9 -f gwled");
}

function validate_form_gwled($post, &$input_errors) {
	/* Make sure both aren't using the same interface */
	if (($post['gw_led2']) && ($post['gw_led3']) && 
	    (($post['enable_led2']) && ($post['enable_led3'])) &&
	    ($post['gw_led2'] == $post['gw_led3']))
		$input_errors[] = 'You cannot set two LEDs for a single gateway. Please choose seperate interfaces.';
}

function gwled_get_gateways() {
	$gateways = return_gateways_array();

	foreach ($gateways as $gateway) {
		$tmp["name"] = "{$gateway['name']} (Monitor: {$gateway['monitor']})";
		$tmp["value"]  = $gateway['name'];
		$gws[] = $tmp;
	}
	return $gws;
}

function gwled_set_status($gateway, $led) {
	$a_gateways = return_gateways_array();
	$gateways_status = array();
	$gateways_status = return_gateways_status(true);
	if ($gateways_status[$gateway]) {
		$status = $gateways_status[$gateway]['status'];
		if (stristr($status, "down"))
			led_blink($led, "veryfast");
		elseif (stristr($status, "loss"))
			led_blink($led, "fast");
		elseif (stristr($status, "delay"))
			led_blink($led, "medium");
		elseif ($status == "none")
			led_on($led);
	} else
		led_blink($led, "veryfast");
}

?>
