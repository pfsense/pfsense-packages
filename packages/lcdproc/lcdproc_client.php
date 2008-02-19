<?php
/* $Id$ */
/*
        lcdproc_client.php
        Copyright (C) 2007 Seth Mos <seth.mos@xs4all.nl>
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

	// require_once("config.inc");
	// require_once("functions.inc");
	require_once("/usr/local/pkg/lcdproc.inc");
	require_once("/usr/local/www/includes/functions.inc.php");

	/* Define functions */
	function send_lcd_commands($lcd, $lcd_cmds) {
		if(!is_array($lcd_cmds) || (empty($lcd_cmds))) {
			lcdproc_warn("Failed to interpret lcd commands");
			return;
		}
		foreach($lcd_cmds as $lcd_cmd) {
			$cmd_output = "";
			if(! fwrite($lcd, "$lcd_cmd\n")) {
				lcdproc_warn("Connection to LCDd process lost $errstr ($errno)");
				die();
			}
			$cmd_output = fgets($lcd, 4096);
			// FIXME: add support for interpreting menu commands here.
			if(preg_match("/^huh?/", $cmd_output)) {
				lcdproc_notice("LCDd output: \"$cmd_output\". Executed \"$lcd_cmd\"");
			}
			// sleep(1);
		}
	}

	function build_interface($lcd) {
		global $g;
		global $config;
		$lcdproc_screens_config = $config['installedpackages']['lcdprocscreens']['config'][0];

		$lcd_cmds = array();
		$lcd_cmds[] = "hello";
		$lcd_cmds[] = "client_set name pfSense";
		$lcd_cmds[] = "screen_add welcome_scr";
		$lcd_cmds[] = "screen_set welcome_scr heartbeat off";
		$lcd_cmds[] = "screen_set welcome_scr name welcome";
		$lcd_cmds[] = "widget_add welcome_scr title_wdgt title";
		$lcd_cmds[] = "widget_add welcome_scr text_wdgt scroller";

		/* process screens to display */
		if(is_array($lcdproc_screens_config)) {
			foreach($lcdproc_screens_config as $name => $screen) {
				if($screen == "on") {
					switch($name) {
						case "scr_time":
							$lcd_cmds[] = "screen_add $name";
							$lcd_cmds[] = "screen_set $name heartbeat off";
							$lcd_cmds[] = "screen_set $name name $name";
							$lcd_cmds[] = "widget_add $name title_wdgt string";
							$lcd_cmds[] = "widget_add $name text_wdgt scroller";
							break;
						case "scr_uptime":
							$lcd_cmds[] = "screen_add $name";
							$lcd_cmds[] = "screen_set $name heartbeat off";
							$lcd_cmds[] = "screen_set $name name $name";
							$lcd_cmds[] = "widget_add $name title_wdgt string";
							$lcd_cmds[] = "widget_add $name text_wdgt scroller";
							break;
						case "scr_hostname":
							$lcd_cmds[] = "screen_add $name";
							$lcd_cmds[] = "screen_set $name heartbeat off";
							$lcd_cmds[] = "screen_set $name name $name";
							$lcd_cmds[] = "widget_add $name title_wdgt string";
							$lcd_cmds[] = "widget_add $name text_wdgt scroller";
							break;
						case "scr_cpu":
							$lcd_cmds[] = "screen_add $name";
							$lcd_cmds[] = "screen_set $name heartbeat off";
							$lcd_cmds[] = "screen_set $name name $name";
							$lcd_cmds[] = "widget_add $name title_wdgt string";
							$lcd_cmds[] = "widget_add $name text_wdgt scroller";
							break;
						case "scr_memory":
							$lcd_cmds[] = "screen_add $name";
							$lcd_cmds[] = "screen_set $name heartbeat off";
							$lcd_cmds[] = "screen_set $name name $name";
							$lcd_cmds[] = "widget_add $name title_wdgt string";
							$lcd_cmds[] = "widget_add $name text_wdgt scroller";
							break;
						case "scr_load":
							$lcd_cmds[] = "screen_add $name";
							$lcd_cmds[] = "screen_set $name heartbeat off";
							$lcd_cmds[] = "screen_set $name name $name";
							$lcd_cmds[] = "widget_add $name title_wdgt string";
							$lcd_cmds[] = "widget_add $name text_wdgt scroller";
							break;
						case "scr_states":
							$lcd_cmds[] = "screen_add $name";
							$lcd_cmds[] = "screen_set $name heartbeat off";
							$lcd_cmds[] = "screen_set $name name $name";
							$lcd_cmds[] = "widget_add $name title_wdgt string";
							$lcd_cmds[] = "widget_add $name text_wdgt scroller";
							break;
					}
				}
			}
		}
		send_lcd_commands($lcd, $lcd_cmds);
	}

	function loop_status($lcd) {
		global $g;
		global $config;
		$lcdproc_screens_config = $config['installedpackages']['lcdprocscreens']['config'][0];
		if(empty($g['product_name'])) {
			$g['product_name'] = "pfSense";
		}
		$version = @file_get_contents("/etc/version");
		/* keep a counter to see how many times we can loop */
		$i = 1;
		while($i) {
			$lcd_cmds = array();
			$lcd_cmds[] = "widget_set welcome_scr title_wdgt \"Welcome to\"";
			$lcd_cmds[] = "widget_set welcome_scr text_wdgt 1 2 16 2 h 4 \"{$g['product_name']} $version\"";

			/* process screens to display */
			if(is_array($lcdproc_screens_config)) {
				foreach($lcdproc_screens_config as $name => $screen) {
					if($screen == "on") {
						switch($name) {
							case "scr_time":
								$time = date ("n/j/Y H:i");
								$lcd_cmds[] = "widget_set $name title_wdgt 1 1 \"+ System Time\"";
								$lcd_cmds[] = "widget_set $name text_wdgt 1 2 16 2 h 2 \"$time\"";
								break;
							case "scr_uptime":
								exec("/usr/bin/uptime", $output, $ret);
								$temp = explode(" ", $output[0]);
								$uptime = "$temp[3] $temp[4] $temp[5] $temp[6] $temp[7] ". substr($temp[8], 0, -1);
								$lcd_cmds[] = "widget_set $name title_wdgt 1 1 \"+ System Uptime\"";
								$lcd_cmds[] = "widget_set $name text_wdgt 1 2 16 2 h 2 \"$uptime\"";
								break;
							case "scr_hostname":
								exec("/bin/hostname", $output, $ret);
								$hostname = $output[0];
								$lcd_cmds[] = "widget_set $name title_wdgt 1 1 \"+ System Name\"";
								$lcd_cmds[] = "widget_set $name text_wdgt 1 2 16 2 h 2 \"$hostname\"";
								break;
							case "scr_cpu":
								$processor = cpu_usage();
								$lcd_cmds[] = "widget_set $name title_wdgt 1 1 \"+ Processor Use\"";
								$lcd_cmds[] = "widget_set $name text_wdgt 1 2 16 2 h 4 \"$processor Percent\"";
								break;
							case "scr_memory":
								$memory = mem_usage();
								$lcd_cmds[] = "widget_set $name title_wdgt 1 1 \"+ Memory Use\"";
								$lcd_cmds[] = "widget_set $name text_wdgt 1 2 16 2 h 4 \"$memory Percent\"";
								break;
							case "scr_load":
								exec("/usr/bin/uptime", $output, $ret);
								$temp = explode(" ", $output[0]);
								$loadavg = "$temp[11] $temp[12] $temp[13]";
								$lcd_cmds[] = "widget_set $name title_wdgt 1 1 \"+ Load Averages\"";
								$lcd_cmds[] = "widget_set $name text_wdgt 1 2 16 2 h 2 \"$loadavg\"";
								break;
							case "scr_states":
								$states = get_pfstate();
								$lcd_cmds[] = "widget_set $name title_wdgt 1 1 \"+ Traffic States\"";
								$lcd_cmds[] = "widget_set $name text_wdgt 1 2 16 2 h 2 \"$states States\"";
								break;
						}
					}
				}
			}

			send_lcd_commands($lcd, $lcd_cmds);
			sleep(5);
			$i++;
		}
	}

	/* Connect to the LCDd port and interface with the LCD */
	$lcd = fsockopen(LCDPROC_HOST, LCDPROC_PORT, $errno, $errstr, 10);
	if (!$lcd) {
		lcdproc_warn("Failed to connect to LCDd process $errstr ($errno)");
	} else {
		build_interface($lcd);
		loop_status($lcd);
		/* loop exited? Close fd and wait for the script to kick in again */
		fclose($lcd);
	}
?>
