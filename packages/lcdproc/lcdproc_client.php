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
			if(preg_match("/^huh?/", $cmd_output)) {
				lcdproc_notice("LCDd output: \"$cmd_output\". Executed \"$lcd_cmd\"");
			}
			// sleep(1);
		}
	}

	function loop_status($lcd) {
		global $g;
		global $config;
		/* keep a counter to see how many times we can loop */
		$i = 1;
		while($i) {
			$time = date ("l dS of F Y h:i:s A");
			$lcd_cmds = array();
			$lcd_cmds[] = "widget_set welcome_scr title_wdgt \"$i test \"";
			// $lcd_cmds[] = "widget_set welcome_scr text_wdgt 1 1 \"$time test\"";
			$lcd_cmds[] = "widget_set welcome_scr text_wdgt 1 2 20 2 h 4 \"$i $time test\"";
			// $lcd_cmds[] = "output on 1 \"";
			send_lcd_commands($lcd, $lcd_cmds);
			sleep(10);
			$i++;
		}
	}

	function build_interface($lcd) {
		$lcd_cmds = array();
		$lcd_cmds[] = "hello";
		$lcd_cmds[] = "client_set name pfSense";
		$lcd_cmds[] = "screen_add welcome_scr";
		$lcd_cmds[] = "screen_set welcome_scr heartbeat off";
		// $lcd_cmds[] = "screen_set welcome_scr duration 32";
		$lcd_cmds[] = "screen_set welcome_scr name welcome";
		$lcd_cmds[] = "widget_add welcome_scr title_wdgt title";
		$lcd_cmds[] = "widget_add welcome_scr text_wdgt scroller";
		send_lcd_commands($lcd, $lcd_cmds);
	}

	/* Connect to the LCDd port and interface with the LCD */
	$lcd = fsockopen(LCDPROC_HOST, LCDPROC_PORT, $errno, $errstr, 10);
	if (!$lcd) {
		lcdproc_warn("Failed to connect to LCDd process $errstr ($errno)");
	} else {
		build_interface($lcd);
		loop_status($lcd);
		/* loop exited? Close fd and wait for the script to kick in again */
		// sleep(1);
		fclose($lcd);
	}
?>
