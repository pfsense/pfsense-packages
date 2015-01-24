<?php
/* $Id$ */
/*
        lcdproc.inc
        Copyright (C) 2007 Seth Mos <seth.mos@dds.nl>
        All rights reserved.

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions are met:

        1. Redistributions of source code must retain the above copyright notice,
           this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright
           notice, this list of conditions and the following disclaimer in the
           documentation and/or other materials provided with the distribution.

        THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
        INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
        AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
        AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
        OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
        SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
        CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
        ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
        POSSIBILITY OF SUCH DAMAGE.
*/

	require_once("config.inc");
	require_once("functions.inc");

	/* LCDproc */
	define('LCDPROC_RCFILE', '/usr/local/etc/rc.d/lcdproc.sh');
	define('LCDPROC_CLIENT', '/tmp/lcdclient.sh');
	define('LCDPROC_CONFIG','/usr/local/etc/LCDd.conf');
	define('LCDPROC_HOST','localhost');
	define('LCDPROC_PORT','13666');

	function lcdproc_notice ($msg) { syslog(LOG_NOTICE, "lcdproc: {$msg}"); }
	function lcdproc_warn ($msg) { syslog(LOG_WARNING, "lcdproc: {$msg}"); }

	function lcdproc_action ($action) {
		if (file_exists(LCDPROC_RCFILE))
			mwexec(LCDPROC_RCFILE.' '.$action);
	}

	function lcdproc_running () {
		if((int)exec('pgrep LCDd | wc -l') > 0)
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

	function lcdproc_write_script($file, $text) {
		$handle = fopen($file, 'wx');
		if(!$handle) {
			lcdproc_warn("Could not open {$file} for writing.");
			exit;
		}
		fwrite($handle, $text);
		fclose($handle);
		chmod($file, 0755);
	}

	function before_form_lcdproc(&$pkg) {
		global $config;

		config_lock();		
		
		config_unlock();
	}

	function before_form_lcdproc_screens(&$pkg) {
		global $config;

		config_lock();		
		
		config_unlock();
	}

	function validate_form_lcdproc($post, &$input_errors) {
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
				case "lpt1":
					continue;
					break;
				case "ugen0.2":
					continue;
					break;
				case "ugen1.2":
					continue;
					break;
				case "ugen2.2":
					continue;
					break;
				default:
					$input_errors[] = "The chosen com port is not valid";
					break;
			}
		}
		if($post['size']) {
			switch($post['size']) {
				case "12x1":
					continue;
					break;
				case "12x2":
					continue;
					break;
				case "12x4":
					continue;
					break;
				case "16x1":
					continue;
					break;
				case "16x2":
					continue;
					break;
				case "16x4":
					continue;
					break;
				case "20x1":
					continue;
					break;
				case "20x2":
					continue;
					break;
				case "20x4":
					continue;
					break;
				case "42x7":
					continue;
					break;
				default:
					$input_errors[] = "The chosen display size is not valid";
					break;
			}
		}
	}
	function validate_form_lcdproc_screens($post, &$input_errors) {
		// No validation required ?!
	}

	function sync_package_lcdproc_screens() {
		sync_package_lcdproc();
	}
	function sync_package_lcdproc() {
		global $g;
		global $config;
		global $input_errors;

		config_lock();

		$lcdproc_config = $config['installedpackages']['lcdproc']['config'][0];
		$lcdproc_screens_config = $config['installedpackages']['lcdprocscreens']['config'][0];

		/* since config is written before this file invoked we don't need to read post data */
		if($lcdproc_config['enable'] && ($lcdproc_config['comport'] != "none")) {
			switch($lcdproc_config['comport']) {
				case "com2":
					$realport = "/dev/cua1";
					break;
				case "ucom1":
					$realport = "/dev/cuaU0";
					break;
				case "ucom2":
					$realport = "/dev/cuaU1";
					break;
				case "lpt1":
					$realport = "/dev/lpt0";
					break;
				case "ugen0.2":
					$realport = "/dev/ugen0.2";
					break;
				case "ugen1.2":
					$realport = "/dev/ugen1.2";
					break;
				case "ugen2.2":
					$realport = "/dev/ugen2.2";
					break;
				default:
					lcdproc_warn("The selected com port is not valid!");
					return;
			}

			$config_text = "[server]\n";
			$config_text .= "Driver={$lcdproc_config[driver]}\n";
			$config_text .= "Bind=127.0.0.1\n";
			$config_text .= "Port=13666\n";
			$config_text .= "ReportLevel=3\n";
			$config_text .= "ReportToSyslog=yes\n";
			$config_text .= "WaitTime=5\n";
			$config_text .= "User=nobody\n";
			$config_text .= "ServerScreen=no\n";
			$config_text .= "Foreground=no\n";
			$config_text .= "DriverPath=/usr/local/lib/lcdproc/\n";
			$config_text .= "GoodBye=\"Thanks for using\"\n";
			$config_text .= "GoodBye=\"    {$g['product_name']}     \"\n";
			/* FIXME: Specific to the pyramid project */
			$config_text .= "ToggleRotateKey=Enter\n";
			$config_text .= "PrevScreenKey=Left\n";
			$config_text .= "NextScreenKey=Right\n";
			$config_text .= "ScrollUpKey=Up\n";
			$config_text .= "ScrollDownKey=Down\n";
			/* FIXME: pyramid test menu */
			$config_text .= "[menu]\n";
			$config_text .= "MenuKey=Escape\n";
			$config_text .= "EnterKey=Enter\n";
			$config_text .= "UpKey=Up\n";
			$config_text .= "DownKey=Down\n";

			/* lcdproc default driver definitions */
			switch($lcdproc_config[driver]) {
				case "SureElec":
					$config_text .= "[{$lcdproc_config['driver']}]\n";
					$config_text .= "driverpath =/usr/local/lib/lcdproc/\n";
					$config_text .= "Device={$realport}\n";
					$config_text .= "Size={$lcdproc_config['size']}\n";
					$config_text .= "Edition=2\n";					
					$config_text .= "Contrast=200\n";
					$config_text .= "Brightness=480\n";	
					$config_text .= "Speed=19200\n";
					break;			
				case "nexcom":
					$config_text .= "[{$lcdproc_config['driver']}]\n";
					$config_text .= "driverpath =/usr/local/lib/lcdproc/\n";
					$config_text .= "Device={$realport}\n";
					$config_text .= "Size={$lcdproc_config['size']}\n";
					break;
				case "bayrad":
					$config_text .= "[{$lcdproc_config['driver']}]\n";
					$config_text .= "Device={$realport}\n";
					$config_text .= "Speed=9600\n";
					break;
				case "picolcd":
					 $config_text .= "[{$lcdproc_config['driver']}]\n";
					 $config_text .= "driverpath=/usr/local/lib/lcdproc/\n";
					 $config_text .= "Device={$realport}\n";
					 $config_text .= "Size={$lcdproc_config['size']}\n";
					 $config_text .= "KeyTimeout=500\n";
					 $config_text .= "Brightness=1000\n";
					 $config_text .= "Blacklight_Timer=60\n";
					 $config_text .= "Contrast=1000\n";
					 $config_text .= "Keylights=on\n";
					 $config_text .= "Key0Light=on\n";
					 $config_text .= "Key1Light=off\n";
					 $config_text .= "Key2Light=off\n";
					 $config_text .= "Key3Light=off\n";
					 $config_text .= "Key4Light=off\n";
					 $config_text .= "Key5Light=off\n";
					 break;					
				case "CFontz":
					$config_text .= "[{$lcdproc_config['driver']}]\n";
					$config_text .= "Device={$realport}\n";
					$config_text .= "Size={$lcdproc_config['size']}\n";
					$config_text .= "Contrast=350\n";
					$config_text .= "Brightness=1000\n";
					$config_text .= "OffBrightness=50\n";
					$config_text .= "Speed=9600\n";
					$config_text .= "NewFirmware=no\n";
					$config_text .= "Reboot=no\n";
					break;
				case "CFontz633":
					$config_text .= "[{$lcdproc_config['driver']}]\n";
					$config_text .= "Device={$realport}\n";
					$config_text .= "Size={$lcdproc_config['size']}\n";
					$config_text .= "Contrast=350\n";
					$config_text .= "Brightness=1000\n";
					$config_text .= "OffBrightness=50\n";
					$config_text .= "Speed=19200\n";
					$config_text .= "NewFirmware=yes\n";
					$config_text .= "Reboot=yes\n";
					break;
				case "CFontzPacket":
					$config_text .= "[{$lcdproc_config['driver']}]\n";
					$config_text .= "Device={$realport}\n";
					$config_text .= "Model=635\n";
					$config_text .= "Size={$lcdproc_config['size']}\n";
					$config_text .= "Contrast=350\n";
					$config_text .= "Brightness=1000\n";
					$config_text .= "OffBrightness=50\n";
					$config_text .= "Speed=115200\n";
					$config_text .= "NewFirmware=yes\n";
					$config_text .= "Reboot=yes\n";
					break;
				case "curses":
					$config_text .= "[{$lcdproc_config['driver']}]\n";
					$config_text .= "Foreground=blue\n";
					$config_text .= "Background=cyan\n";
					$config_text .= "Backlight=red\n";
					$config_text .= "Size={$lcdproc_config['size']}\n";
					$config_text .= "TopLeftX=7\n";
					$config_text .= "TopLeftY=7\n";
					$config_text .= "UseACS=no\n";
					break;
				case "CwLynx":
					$config_text .= "[{$lcdproc_config['driver']}]\n";
					$config_text .= "Model=12232\n";
					$config_text .= "Device={$realport}\n";
					$config_text .= "Size={$lcdproc_config['size']}\n";
					$config_text .= "Speed=19200\n";
					$config_text .= "Reboot=no\n";
					break;
				case "pyramid":
					$config_text .= "[{$lcdproc_config['driver']}]\n";
					$config_text .= "Device={$realport}\n";
					$config_text .= "Size={$lcdproc_config['size']}\n";
					break;
				case "ea65":
					$config_text .= "[{$lcdproc_config['driver']}]\n";
					$config_text .= "Device={$realport}\n";
					$config_text .= "OffBrightness=0\n";
					$config_text .= "Brightness=500\n";
					break;
				case "RhinoOSLI":
					$config_text .= "[{$lcdproc_config['driver']}]\n";
					$config_text .= "Device={$realport}\n";
					break;
				default:
					lcdproc_warn("The chosen lcdproc driver is not a valid choice");
					unset($lcdproc_config[driver]);
			}

			/* generate rc file start and stop */
			$client_script = <<<EOD
#!/bin/sh

# script starts a lcd client and always keeps it active.
counter=1
while [ "\$counter" -ne 0 ]
do
	# loop the client to drive the display
	/usr/local/bin/php -f /usr/local/pkg/lcdproc_client.php
	sleep 1
done

EOD;

			/* generate rc file start and stop */
			$stop = <<<EOD

if [ `ps auxw |awk '/LCD[d]/ {print $2}'| wc -l` != 0  ]; then
	ps auxw |awk '/LCD[d]/ {print $2}'|xargs kill 
	sleep 1	
fi
if [ `ps auxw |awk '/lcdclient.s[h]/ {print $2}'| wc -l` != 0  ]; then
	ps auxw |awk '/lcdclient.s[h]/ {print $2}'|xargs kill
	sleep 1	
fi

EOD;
			$start = $stop ."\n";
			$start .= "\t/usr/local/sbin/LCDd -c ". LCDPROC_CONFIG ."\n";
			$start .= "\t". LCDPROC_CLIENT ." &\n";

			/* write out the configuration */
			conf_mount_rw();
			lcdproc_write_script(LCDPROC_CLIENT, $client_script);
			lcdproc_write_config(LCDPROC_CONFIG, $config_text);
			write_rcfile(array(
					'file' => 'lcdproc.sh',
					'start' => $start,
					'stop' => $stop
					));
			conf_mount_ro();

			/* or restart lcdproc if settings were changed */
			if(lcdproc_running($_POST['comport'])) {
				lcdproc_notice("Restarting service lcdproc");
				lcdproc_action('restart');
			}
		}

		if((! $lcdproc_config['driver']) || ($lcdproc_config['comport'] == "none")) {
			/* no parameters user does not want lcdproc running */
			/* lets stop the service and remove the rc file */

			if(file_exists(LCDPROC_RCFILE)) {
				if(!$lcdproc_config['enable']) {
					lcdproc_notice('Stopping service: lcdproc disabled');
				} else {
					lcdproc_notice('Stopping service: no com port selected');
				}
				lcdproc_action('stop');
				conf_mount_rw();
				unlink(LCDPROC_RCFILE);
				unlink(LCDPROC_CLIENT);
				unlink(LCDPROC_CONFIG);
				conf_mount_ro();
			}
		}
		config_unlock();
	}

?>
