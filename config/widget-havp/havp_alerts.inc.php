<?php
/*
	havp_alerts.inc.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2009 Jim Pingle
	Copyright (C) 2015 ESF, LLC
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
function get_havp_alerts($havp_alerts, $nentries, $tail = 20) {
	global $config, $g;
	$logarr = "";
	/* Always do a reverse tail, to be sure we're grabbing the 'end' of the alerts. */
	exec("/usr/bin/tail -r -n {$tail} {$havp_alerts}", $logarr);

	$havpalerts = array();
	$counter = 0;

	foreach ($logarr as $logent) {
		if ($counter >= $nentries) {
			break;
		}
		$alert = parse_havp_alert_line($logent);
		if ($alert != "") {
			$counter++;
			$havpalerts[] = $alert;
		}
	}
	/* Since the rules are in reverse order, flip them around if needed, based on the user's preference */
	return isset($config['syslog']['reverse']) ? $havpalerts : array_reverse($havpalerts);
}

function parse_havp_alert_line($line) {
	global $g;
	$log_split = "";

	// FIXME: Obviously incomplete TLD list at the moment, plus the whole thing is completely whacky...
	preg_match("/^(\d+\/\d+\/\d+)\s+(\d+:\d+:\d+)\s+(\d+.\d+.\d+.\d+)\s+\w+\s+\d+\s+(https?:\/\/([0-9a-z-]+\.)+([a-z]{2,3}|aero|coop|jobs|mobi|museum|name|travel)(:[0-9]{1,5})?(\/[^ ]*)?)\s+[0-9+]+\s+\w+\s+\w+:\s+([\S]+)$/U", $line, $log_split);

	list($all, $alert['date'], $alert['time'], $alert['lanip'], $alert['url'], $alert['dontcare1'], $alert['dontcare2'], $alert['dontcare3'], $alert['query'], $alert['virusname']) = $log_split;
	$usableline = true;

	if (trim($alert['url']) == "") {
		$usableline = false;
	}
	if (trim($alert['virusname']) == "") {
		$usableline = false;
	}
	if ($usableline == true) {
		return $alert;
	} else {
		if ($g['debug']) {
			log_error("There was a error parsing line: $line.");
		}
		return "";
	}
}

/* AJAX specific handlers */
function handle_havp_ajax($havp_alerts_logfile, $nentries = 5, $tail = 50) {
	if ($_GET['lastsawtime'] or $_POST['lastsawtime']) {
		if ($_GET['lastsawtime']) {
			$lastsawtime = $_GET['lastsawtime'];
		}
		if ($_POST['lastsawtime']) {
			$lastsawtime = $_POST['lastsawtime'];
		}
		// Compare last seen rule's time stamp with alert logs.
		// Afterwards, return the newer records so that client can update AJAX interface screen.
		$new_rules = "";
		$time_regex = "";

		$havp_alerts = get_havp_alerts($havp_alerts_logfile, $nentries);
		foreach($havp_alerts as $log_row) {
			preg_match("/^([0-9][0-9])\/([0-9][0-9])\/([0-9][0-9][0-9][0-9])$/U", $log_row['date'], $time_regex);
			$row_time = strtotime($time_regex[2] . "/" . $time_regex[1] . "/" . $time_regex[3] . " " . $log_row['time']);

			if ($row_time > $lastsawtime and $lastsawtime > 0) {
				$new_rules .= "{$log_row['url']}||{$log_row['virusname']}||" . time() . "||{$log_row['date']}||{$log_row['time']}||" . "\n";
			}
		}
		echo $new_rules;
		exit;
	}
}

?>
