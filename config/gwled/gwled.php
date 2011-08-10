#!/usr/local/bin/php -q
<?php
require_once("config.inc");
require_once("functions.inc");
require_once("gwled.inc");
require_once("led.inc");
require_once("gwlb.inc");

global $config;
$gwled_config = $config['installedpackages']['gwled']['config'][0];

if (($gwled_config['enable_led2']) && ($gwled_config['gw_led2'])) {
	gwled_set_status($gwled_config['gw_led2'], 2);
}
if (($gwled_config['enable_led3']) && ($gwled_config['gw_led3'])) {
	gwled_set_status($gwled_config['gw_led3'], 3);
}
?>
