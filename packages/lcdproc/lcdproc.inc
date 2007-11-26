<?php
	require_once("config.inc");
	require_once("functions.inc");

	/* LCDproc */

	function lcdproc_notice ($msg) { syslog(LOG_NOTICE, "lcdproc: {$msg}"); }
	function lcdproc_warn ($msg) { syslog(LOG_WARNING, "lcdproc: {$msg}"); }

	function lcdproc_running () {
		if((int)exec('pgrep lcdproc | wc -l') > 0)
			return true;
		return false;
	}	

	function lcdproc_write_config($file, $text) {
		$handle = fopen($file, 'w');
		if(!$handle) {
			lcdproc_warn("Could not open {$file} for writing.");
			exit;
		}
		fwrite($handle, $text);
		fclose($handle);
	}

	function before_form_lcdproc($pkg) {
		global $config;

		config_lock();		
		
		config_unlock();
	}

	function validate_form_lcdproc($post, $input_errors) {
		if($post['comport']) {
			switch($post['comport']) {
				case "none":
					continue;
					break;
				case "com2":
					continue;
					break;
				case "ucom1":
					continue;
					break;
				case "ucom2":
					continue;
					break;
				default:
					$input_errors[] = "The chosen com port is not valid";
					break;
			}
		}
	}

	function sync_package_lcdproc() {
		global $config;
		global $input_errors;

		config_lock();

		$lcdproc_config = $config['installedpackages']['lcdproc']['config'][0];
		$config_file = '/var/etc/lcdd.conf';


		/* since config is written before this file invoked we don't need to read post data */
		if($lcdproc_config['enable'] && ($lcdproc_config['comport'] != "none")) {
			$config_text = "[server]\n";
			$config_text .= "Driver={$lcdproc_config[driver]}\n";

			/* if lcdproc not running start it */
			if(!lcdproc_running()) {
				lcdproc_notice("Starting service lcdproc");
				lcdproc_action('start');	
			}
			/* or restart lcdproc if settings were changed */
			elseif($_POST['comport']) {
				lcdproc_notice("Restarting service lcdproc");
				lcdproc_action('restart');
			}
		}

		if((! $lcdproc_config['comport']) || ($lcdproc_config['comport'] == "none")) {
			/* no parameters user does not want lcdproc running */
			/* lets stop the service and remove the rc file */

			if(file_exists($config_file)) {
				if(!$lcdproc_config['enable']) {
					lcdproc_notice('Stopping service: lcdproc disabled');
				} else {
					lcdproc_notice('Stopping service: no com port selected');
				}
				lcdproc_action('stop');
				unlink($config_file);
			}
		}
		config_unlock();
	}
?>
