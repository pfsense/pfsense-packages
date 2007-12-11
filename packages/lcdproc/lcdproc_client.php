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

	require_once("config.inc");
	require_once("functions.inc");
	require_once("/usr/local/pkg/lcdproc.inc");

	/* Define functions */
	function send_lcd_commands($lcd, $lcd_cmds) {
		if(!is_array($lcd_cmds) || (empty($lcd_cmds))) {
			lcdproc_warn("Failed to interpret lcd commands");
			return;
		}
		foreach($lcd_cmds as $lcd_cmd) {
			$cmd_output = "";
			fwrite($lcd, $lcd_cmd);
			$cmd_output .= fgets($lcd, 128);
			lcdproc_notice("LCDd output for cmd $lcd_cmd is: $cmd_output");
			sleep(1);
		}
	}

	function loop_status($lcd) {
		/* keep a counter to see how many times we can loop */
		$i = 0;
		while(1) {
			$time = time();
			$lcd_cmds = array();
			$lcd_cmds[] = "client_set -name \"Parenttest\"\n";
			$lcd_cmds[] = "screen_add status\n";
			$lcd_cmds[] = "screen_set status -heartbeat off\n";
			$lcd_cmds[] = "widget_add status title title\n";
			$lcd_cmds[] = "widget_add status date scroller\n";
			$lcd_cmds[] = "widget_set status title $i\n";
			$lcd_cmds[] = "widget_set status date left right h 1 $time\n";
			send_lcd_commands($lcd, $lcd_cmds);
			$i++;
		}
	}

	function send_hello($lcd) {
		$lcd_cmds = array();
		$lcd_cmds[] = "hello\n";
		send_lcd_commands($lcd, $lcd_cmds);
	}

	/* Connect to the LCDd port and interface with the LCD */
	$lcd = fsockopen(LCDPROC_HOST, LCDPROC_PORT, $errno, $errstr, 10);
	if (!$lcd) {
		lcdproc_warn("Failed to connect to LCDd process $errstr ($errno)");
	} else {
		send_hello($lcd);
		loop_status($lcd);
		/* loop exited? Close fd and wait for the script to kick in again */
		sleep(1);
	}
	fclose($lcd);
?>
