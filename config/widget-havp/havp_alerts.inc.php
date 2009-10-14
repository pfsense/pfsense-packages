<?
function get_havp_alerts($havp_alerts, $nentries, $tail = 20) {
	global $config, $g;
	$logarr = "";
	/* Always do a reverse tail, to be sure we're grabbing the 'end' of the alerts. */
	exec("/usr/bin/tail -r -n {$tail} {$havp_alerts}", $logarr);
	
	$havpalerts = array();
	
	$counter = 0;
	
	foreach ($logarr as $logent) {
		if($counter >= $nentries)
			break;

		$alert = parse_havp_alert_line($logent);
		if ($alert != "") {
			$counter++;
			$havpalerts[] = $alert;
		}

	}
	/* Since the rules are in reverse order, flip them around if needed based on the user's preference */
	return isset($config['syslog']['reverse']) ? $havpalerts : array_reverse($havpalerts);
}




function parse_havp_alert_line($line) {
	$log_split = "";
	
	preg_match("/^(\d+\/\d+\/\d+)\s+(\d+:\d+:\d+)\s+(\d+.\d+.\d+.\d+)\s+\w+\s+\d+\s+(https?:\/\/([0-9a-z-]+\.)+([a-z]{2,3}|aero|coop|jobs|mobi|museum|name|travel)(:[0-9]{1,5})?(\/[^ ]*)?)\s+[0-9+]+\s+\w+\s+\w+:\s+([\S]+)$/U", $line, $log_split);

  list($all, $alert['date'], $alert['time'], $alert['lanip'], $alert['url'], $alert['dontcare1'], $alert['dontcare2'], $alert['dontcare3'], $alert['query'],
    $alert['virusname']) = $log_split;

	$usableline = true;

	if(trim($alert['url']) == "")
		$usableline = false;
	if(trim($alert['virusname']) == "")
		$usableline = false;

	if($usableline == true) {
		return $alert;
	} else {
		if($g['debug']) {
			log_error("There was a error parsing line: $line.   Please report to mailing list or forum.");
		}
		return "";
	}
}

/* AJAX specific handlers */
function handle_havp_ajax($havp_alerts_logfile, $nentries = 5, $tail = 50) {
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
		$time_regex = "";
		
		$havp_alerts = get_havp_alerts($havp_alerts_logfile, $nentries);
		foreach($havp_alerts as $log_row) {
		  preg_match("/^([0-9][0-9])\/([0-9][0-9])\/([0-9][0-9][0-9][0-9])$/U",$log_row['date'] , $time_regex);
#			$time_regex = "";"/^([0-9][0-9])\/([0-9][0-9])\/([0-9][0-9][0-9][0-9])\s+([0-9][0-9]:[0-9][0-9]:[0-9][0-9])$/U"
	//		preg_match("/.*([0-9][0-9]:[0-9][0-9]:[0-9][0-9]).*/", $log_row['date'] . " " .  $log_row['time'], $time_regex);
			$row_time = strtotime($time_regex[2] . "/" . $time_regex[1] . "/" . $time_regex[3] . " " . $log_row['time']);
     // $myfile = "/testfile.txt";
     // $fh = fopen($myfile,'a') or die("can't open file");
     // $stringdata = $lastsawtime . "-" . $row_time . "\n";
    //  fwrite($fh, $stringdata);
    //  fclose($fh);

			if($row_time > $lastsawtime  and $lastsawtime > 0) {
			  
				$new_rules .= "{$log_row['url']}||{$log_row['virusname']}||" . time() . "||{$log_row['date']}||{$log_row['time']}||" . "\n";
			}
		}
		echo $new_rules;
		exit;
	}
}
?>