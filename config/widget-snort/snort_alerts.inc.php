<?
function get_snort_alerts($snort_alerts, $nentries, $tail = 20) {
	global $config, $g;
	$logarr = "";
	/* Always do a reverse tail, to be sure we're grabbing the 'end' of the alerts. */
	exec("/usr/bin/tail -r -n {$tail} {$snort_alerts}", $logarr);
	
	$snortalerts = array();
	
	$counter = 0;
	
	foreach ($logarr as $logent) {
		if($counter >= $nentries)
			break;

		$alert = parse_snort_alert_line($logent);
		if ($alert != "") {
			$counter++;
			$snortalerts[] = $alert;
		}

	}
	/* Since the rules are in reverse order, flip them around if needed based on the user's preference */
	return isset($config['syslog']['reverse']) ? $snortalerts : array_reverse($snortalerts);
}

function parse_snort_alert_line($line) {
	$log_split = "";
	$datesplit = "";
	preg_match("/^(.*)\s+\[\*\*\]\s+\[(\d+\:\d+:\d+)\]\s(.*)\s(.*)\s+\[\*\*\].*\s+\[Priority:\s(\d+)\]\s{(.*)}\s+(.*)\s->\s(.*)$/U", $line, $log_split);
	
	list($all, $alert['time'], $alert['rule'], $alert['category'], $alert['descr'], 
	  $alert['priority'], $alert['proto'], $alert['src'], $alert['dst']) = $log_split;

	$usableline = true;

	if(trim($alert['src']) == "")
		$usableline = false;
	if(trim($alert['dst']) == "")
		$usableline = false;

	if($usableline == true) {
	  preg_match("/^(\d+)\/(\d+)-(\d+\:\d+\:\d+).\d+$/U", $alert['time'], $datesplit);
    $alert['dateonly'] = $datesplit[2] . "/"  . $datesplit[1];
    $alert['timeonly'] = $datesplit[3];
    $alert['category'] = strtoupper( substr($alert["category"],0 , 1) ) . strtolower( substr($alert["category"],1 ) );
		return $alert;
	} else {
		if($g['debug']) {
			log_error("There was a error parsing line: $line.   Please report to mailing list or forum.");
		}
		return "";
	}
}

/* AJAX specific handlers */
function handle_snort_ajax($snort_alerts_logfile, $nentries = 5, $tail = 50) {
	if($_GET['lastsawtime'] or $_POST['lastsawtime']) {
		if($_GET['lastsawtime'])
			$lastsawtime = $_GET['lastsawtime'];
		if($_POST['lastsawtime'])
			$lastsawtime = $_POST['lastsawtime'];
		/*  compare lastsawrule's time stamp to alert logs.
		 *  afterwards return the newer records so that client
                 *  can update AJAX interface screen.
		 */
		$new_rules = "";
		$snort_alerts = get_snort_alerts($snort_alerts_logfile, $nentries);
		foreach($snort_alerts as $log_row) {
			$time_regex = "";
			preg_match("/.*([0-9][0-9]:[0-9][0-9]:[0-9][0-9]).*/", $log_row['time'], $time_regex);
			$row_time = strtotime($time_regex[1]);
			if($row_time > $lastsawtime) {
				$new_rules .= "{$log_row['time']}||{$log_row['priority']}||{$log_row['category']}||{$log_row['src']}||{$log_row['dst']}||" . time() . "||{$log_row['timeonly']}||{$log_row['dateonly']}" . "||\n";
			}
		}
		echo $new_rules;
		exit;
	}
}
?>