<?php
/* $Id$ */
/* Copyright (C) 2006 Daniel S. Haischt */
require_once("functions.inc");
require_once("dspam.inc");

/* ========================================================================== */
/* = A D M I N   R E L A T E D   F U N C T I O N   C A L L S                = */
/* ========================================================================== */

  /* Status */
  else if (basename($_SERVER['SCRIPT_NAME']) == "dspam-admin.php" && $CONFIG['ADMIN']) {
    $displays_errors =& DisplayStatus();
    $input_errors = array_merge($input_errors, $displays_errors);
  }

  /* User Statistics */
  else if (basename($_SERVER['SCRIPT_NAME']) == "dspam-admin-stats.php" && $CONFIG['ADMIN']) {
    $displayus_errors =& DisplayUserStatistics();
    $input_errors = array_merge($input_errors, $displayus_errors);
  }

  /* Preferences */
  else if (basename($_SERVER['SCRIPT_NAME']) == "dspam-admin-prefs.php" && $CONFIG['ADMIN']) {
    $savemsg = "";
    $displayap_errors =& DisplayAdminPreferences($savemsg);
    $input_errors = array_merge($input_errors, $displayap_errors);
  }
} else {
  $input_errors[] = "The page you did request can't be processed by <code>dspam.inc</code>.";
}

/* ========================================================================== */
/* = A D M I N   S T A T U S   F U N C T I O N S                            = */
/* ========================================================================== */

function DisplayAdminPreferences(&$statusmsg) {
  return DisplayPreferences("admin", $statusmsg);
}

function DisplayUserStatistics() {
  global $CONFIG, $DATA;
  $b = "rowEven";

  $sl_total = 0;
  $il_total = 0;
  $sm_total = 0;
  $fp_total = 0;
  $sc_total = 0;
  $ic_total = 0;
  $mailbox_total = 0;

  $pd = popen("{$CONFIG['DSPAM_STATS']} 2>&1", "r");

  while (!feof($pd)) {
    $line = chop( fgets($pd, 4096) );

    if ($b == "qrowEven") {
      $b = "qrowOdd";
    } else {
      $b = "qrowEven";
    }

    $line = preg_replace('/:/', ' ', $line);

    list($username, , $sl, , $il, , $fp, , $sm, , $sc, , $ic)
         = (preg_split('/\s+/', $line));
    if ($username == "" && $sl == "") {
      /* we do not want to display data that
       * does not bleong to any user
       */
      continue;
    } else if ($sl == "") {
      $line = fgets($pd, 4096);
      $line = preg_replace('/:/', ' ', $line);
      list(, , $sl, , $il, , $fp, , $sm, , $sc, , $ic)
           = (preg_split('/\s+/', $line));
    }

    $PREFS =& GetPrefs($username, GetPath($username).".prefs");
    if ($PREFS['enableBNR'] == "on") { $PREFS['enableBNR'] = "OFF"; }
    if ($PREFS['enableWhitelist'] == "on") { $PREFS['enableWhitelist'] = "OFF"; }
    $PREFS['spamAction'] = ucfirst($PREFS['spamAction']);
    $PREFS['enableBNR'] = strtoupper($PREFS['enableBNR']);
    $PREFS['enableWhitelist'] = strtoupper($PREFS['enableWhitelist']);

    $mailbox = GetPath($username).".mbox";
    if ( file_exists($mailbox) ) {
      $mailbox_size = filesize($mailbox);
      $mailbox_display = sprintf("%2.1f KB", ($mailbox_size / 1024));
      $mailbox_total += $mailbox_size;
    }
    else {
      $mailbox_display = "--";
    }

    $sl_total += $sl;
    $il_total += $il;
    $sm_total += $sm;
    $fp_total += $fp;
    $sc_total += $sc;
    $ic_total += $ic;

    $DATA['TABLE'] .= "<tr><td class=\"{$b}\"><a href=\"{$CONFIG['DSPAM_CGI']}?user={$username}\">{$username}</A></td>" .
                      "  <td class=\"{$b} rowDivider\" align=\"right\" nowrap=\"nowrap\">{$mailbox_display}</td>" .
                      "  <td class=\"{$b} rowDivider\">{$sl}</td>" .
                      "  <td class=\"{$b}\">{$il}</td>" .
                      "  <td class=\"{$b}\">{$fp}</td>" .
                      "  <td class=\"{$b}\">{$sm}</td>" .
                      "  <td class=\"{$b}\">{$sc}</td>" .
                      "  <td class=\"{$b}\">{$ic}</td>" .
                      "  <td class=\"{$b} rowDivider\">{$PREFS['trainingMode']}</td>" .
                      "  <td class=\"{$b}\">{$PREFS['spamAction']}</td>" .
                      "  <td class=\"{$b}\">{$PREFS['enableBNR']}</td>" .
                      "  <td class=\"{$b}\">{$PREFS['enableWhitelist']}</td>" .
                      "  <td class=\"{$b}\">{$PREFS['statisticalSedation']}</td>" .
                      "  <td class=\"{$b}\">{$PREFS['signatureLocation']}</td>" .
                      "</tr>\n";
  }
  pclose($pd);

  $mailbox_total_display = sprintf("%2.1f KB", ($mailbox_total / 1024));

  $b = "listhdrr";
  $DATA['TABLE'] .= "<tr><td class=\"{$b}\">Total</td>".
                    "	<td class=\"{$b} rowDivider\" align=\"right\" nowrap=\"nowrap\">{$mailbox_total_display}</td>".
                    "	<td class=\"{$b} rowDivider\">{$sl_total}</td>".
                    "	<td class=\"{$b}\">{$il_total}</td>".
                    "	<td class=\"{$b}\">{$sm_total}</td>".
                    "	<td class=\"{$b}\">{$fp_total}</td>".
                    "	<td class=\"{$b}\">{$sc_total}</td>".
                    "	<td class=\"{$b}\">{$ic_total}</td>".
                    "	<td class=\"{$b} rowDivider\">&nbsp;</td>".
                    "	<td class=\"{$b}\">&nbsp;</td>".
                    "	<td class=\"{$b}\">&nbsp;</td>".
                    "	<td class=\"{$b}\">&nbsp;</td>".
                    "	<td class=\"{$b}\">&nbsp;</td>".
                    "	<td class=\"{$b}\">&nbsp;</td>".
                   "</tr>\n";
}

function &DisplayStatus() {
  global $CONFIG, $DATA;

  $LOG = "{$CONFIG['DSPAM_HOME']}/system.log";
  $spam_daily = array();
  $nonspam_daily = array();
  $period_daily = array();
  $fp_daily = array();
  $sm_daily = array();
  $inoc_daily = array();
  $whitelist_daily = array();
  $spam_weekly = array();
  $nonspam_weekly = array();
  $period_weekly = array();
  $fp_weekly = array();
  $sm_weekly = array();
  $inoc_weekly = array();
  $$whitelist_weekly = array();
  $msgpersecond = array();
  $classes = array();

  list (, $min, $hour, $mday, $mon, $year, , ,) = (localtime(time()));
  $hmstart = time() - 60;
  $daystart = mktime(0, 0, 0, $mon, $mday, $year);
  $periodstart = $daystart - (3600*24*24); /* 2 Weeks ago */
  $dailystart = time() - (3600*23);
  $c_weekly = 0; /* Cursor to most recent time slot */
  $c_daily = 0;

  if (! file_exists($LOG)) {
    return $input_errors[] = "No historical data is available (log file �{$LOG}� does not exist).";
  }

  /* Initialize each individual time period */

  for ($i = 0; $i <= 23; $i++) {
    $h = To12Hour($hour - (23-$i));
    $period_daily[$i] = $h;
    $spam_daily[$i] = 0;
    $nonspam_daily[$i] = 0;
    $sm_daily[$i] = 0;
    $fp_daily[$i] = 0;
    $inoc_daily[$i] = 0;
  }

  for ($i = 0; $i <= 23; $i++) {
    $d = $daystart - (3600 * 24 * (24 - $i));
    list (, , , $lday, $lmon, $lyear, , ,) = (localtime($d));
    $lmon++;
    $lyear += 1900;
    $period_weekly[$i] = "{$lmon}/{$lday}/{$lyear}";
    $spam_weekly[$i] = 0;
    $nonspam_weekly[$i] = 0;
    $sm_weekly[$i] = 0;
    $fp_weekly[$i] = 0;
    $inoc_weekly[$i] = 0;
  }

  if($fd = @fopen("{$LOG}", "r")) {
    while (!feof($fd)) {
      $line = fgets($fd, 4096);
      list ($t_log, $c_log, , $signature, , $e_log) = preg_split('/\t/', $line);
      if ($t_log > time()) { continue; }

      $last_message = $t_log;

      /* Only Parse Log Data in our Time Period */
      if ($t_log >= $periodstart) {
        list (, $tmin, $thour, $tday, $tmon, $tyear) = (localtime($t_log));
        $tmon++;
        $tyear += 1900;

        /* Weekly Graph */
        $c_weekly = 0;
        while($period_weekly[$c_weekly] <> "{$tmon}/{$tday}/{$tyear}" && $c_weekly < 24) {
          $c_weekly++;
        }

        if ($c_log == "E") {
          if ($classes[$signature] == "S") {
            $spam_weekly[$c_weekly]--;
            if ($spam_weekly[$c_weekly] < 0) { $spam_weekly[$c_weekly] = 0; }
          } else if ($classes[$signature] == "I") {
            $nonspam_weekly[$c_weekly]--;
            if ($nonspam_weekly[$c_weekly] < 0) { $nonspam_weekly[$c_weekly] = 0; }
          } else if ($classes[$signature] == "W") {
            $whitelist_weekly[$c_weekly]--;
            if ($whitelist_weekly[$c_weekly] < 0) { $whitelist_weekly[$c_weekly] = 0; }
          } else if ($classes[$signature] == "F") {
            $spam_weekly[$c_weekly]++; $fp_weekly[$c_weekly]--;
            if ($fp_weekly[$c_weekly] < 0) { $fp_weekly[$c_weekly] = 0; }
          } else if ($classes[$signature] == "M") {
            $sm_weekly[$c_weekly]--; $nonspam_weekly[$c_weekly]++;
            if ($sm_weekly[$c_weekly] < 0) { $sm_weekly[$c_weekly] = 0; }
          } else if ($classes[$signature] == "N") {
            $inoc_weekly[$c_weekly]--;
            if ($inoc_weekly[$c_weekly] < 0) { $inoc_weekly[$c_weekly] = 0; }
          }
        } else {
         $classes[$signature] = $c_log;
        }

        if ($c_log == "S") { $spam_weekly[$c_weekly]++; }
        if ($c_log == "I") { $nonspam_weekly[$c_weekly]++; }
        if ($c_log == "W") { $whitelist_weekly[$c_weekly]++; }
        if ($c_log == "F")
          { $spam_weekly[$c_weekly]--; $fp_weekly[$c_weekly]++;
            if ($spam_weekly[$c_weekly] < 0) { $spam_weekly[$c_weekly] = 0; }
          }
        if ($c_log == "M")
          { $sm_weekly[$c_weekly]++; $nonspam_weekly[$c_weekly]--;
            if ($nonspam_weekly[$c_weekly] < 0) { $nonspam_weekly[$c_weekly] = 0; }
          }
        if ($c_log == "N") { $inoc_weekly[$c_weekly]++; }


        /* Daily Graph */
        if ($t_log >= $dailystart) {
          while($period_daily[$c_daily] <> To12Hour($thour) && $c_daily < 24) {
            $c_daily++;
          }

          if ($c_log == "E") {
            if ($classes[$signature] == "S") {
              $spam_daily[$c_daily]--;
              if ($spam_daily[$c_daily] < 0) { $spam_daily[$c_daily] = 0; }
            } else if ($classes[$signature] == "I") {
              $nonspam_daily[$c_daily]--;
              if ($nonspam_daily[$c_daily] < 0) { $nonspam_daily[$c_daily] = 0; }
            } else if ($classes[$signature] == "W") {
              $whitelist_daily[$c_daily]--;
              if ($whitelist_daily[$c_daily] < 0) { $whitelist_daily[$c_daily] = 0; }
            } else if ($classes[$signature] == "F") {
              $spam_daily[$c_daily]++; $fp_daily[$c_daily]--;
              if ($fp_daily[$c_daily] < 0) { $fp_daily[$c_daily] = 0; }
            } else if ($classes[$signature] == "M") {
              $sm_daily[$c_daily]--; $nonspam_daily[$c_daily]++;
              if ($sm_daily[$c_daily] < 0) { $sm_daily[$c_daily] = 0; }
            } else if ($classes[$signature] == "N") {
              $inoc_daily[$c_daily]--;
              if ($inoc_daily[$c_daily] < 0) { $inoc_daily[$c_daily] = 0; }
            }
          }

          if ($c_log == "S") { $spam_daily[$c_daily]++; }
          if ($c_log == "I") { $nonspam_daily[$c_daily]++; }
          if ($c_log == "W") { $whitelist_daily[$c_daily]++; }
          if ($c_log == "F")
            { $spam_daily[$c_daily]--; $fp_daily[$c_daily]++;
              if ($spam_daily[$c_daily] < 0) { $spam_daily[$c_daily] = 0; }
            }
          if ($c_log == "M")
            { $sm_daily[$c_daily]++; $nonspam_daily[$c_daily]--;
              if ($nonspam_daily[$c_daily] < 0) { $nonspam_daily[$c_daily] = 0; }
            }
          if ($c_log == "N") { $inoc_daily[$c_daily]++; }
        }

        /* Last Half-Minute */
        if ($t_log >= $hmstart) {
          $msgpersecond[$t_log]++;
          $DATA['AVG_PROCESSING_TIME'] += $e_log;
          $keycount_exectime++;
        }
      }
    } // end while

    fclose($fd);
  } else {
    return $input_errors[] = "Unable to open logfile: {$LOG}.";
  } // end if

  /* Calculate Avg. Messages Per Second */
  foreach(array_values($msgpersecond) as $el) {
    $DATA['AVG_MSG_PER_SECOND'] += $el;
  }
  $DATA['AVG_MSG_PER_SECOND'] /= 60;
  $DATA['AVG_MSG_PER_SECOND'] = sprintf("%2.2f", $DATA['AVG_MSG_PER_SECOND']);

  /* Calculate Avg. Processing Time */
  if ($keycount_exectime == 0) {
    $DATA['AVG_PROCESSING_TIME'] = 0;
  } else {
    $DATA['AVG_PROCESSING_TIME'] /= $keycount_exectime;
  }
  $DATA['AVG_PROCESSING_TIME'] = sprintf("%01.6f", $DATA['AVG_PROCESSING_TIME']);

  /* Calculate Number of processes, Uptime and Mail Queue length */
  $pd = popen("{$CONFIG['ALL_PROCS']} | grep dspam | grep -v grep | grep -v cgi | grep -v sock | wc -l", "r");
  $DATA['DSPAM_PROCESSES'] = fgets($pd, 4096);
  pclose($pd);

  $pd = popen("uptime", "r");
  $DATA['UPTIME'] = fgets($pd, 4096);
  pclose($pd);

  $pd = popen("{$CONFIG['MAIL_QUEUE']}", "r");
  $DATA['MAIL_QUEUE'] = fgets($pd, 4096);
  pclose($pd);

  /* Calculate Graphs */
  $DATA['SPAM_TODAY']               = $spam_weekly[24];
  $DATA['NONSPAM_TODAY']            = $nonspam_weekly[24];
  $DATA['SM_TODAY']                 = $sm_weekly[24];
  $DATA['FP_TODAY']                 = $fp_weekly[24];
  $DATA['INOC_TODAY']               = $inoc_weekly[24];
  $DATA['TOTAL_TODAY']              = $DATA['SPAM_TODAY'] + $DATA['NONSPAM_TODAY'] + $DATA['SM_TODAY']
                                      + $DATA['FP_TODAY'] + $DATA['INOC_TODAY'];

  $DATA['SPAM_THIS_HOUR']           = $spam_daily[23];
  $DATA['NONSPAM_THIS_HOUR']        = $nonspam_daily[23];
  $DATA['SM_THIS_HOUR']             = $sm_daily[23];
  $DATA['FP_THIS_HOUR']             = $fp_daily[23];
  $DATA['INOC_THIS_HOUR']           = $inoc_daily[23];
  $DATA['TOTAL_THIS_HOUR']          = $DATA['SPAM_THIS_HOUR'] +
                                      + $DATA['NONSPAM_THIS_HOUR']
                                      + $DATA['SM_THIS_HOUR']
                                      + $DATA['FP_THIS_HOUR']
                                      + $DATA['INOC_THIS_HOUR'];

  if (is_array($spam_daily) &&
      is_array($nonspam_daily) &&
      is_array($sm_daily) &&
      is_array($fp_daily) &&
      is_array($inoc_daily) &&
      is_array($whitelist_daily) &&
      is_array($period_daily)) {
    $DATA['DATA_DAILY'] = join(",", $spam_daily)
                        . "_"
                        . join(",", $nonspam_daily)
                        . "_"
                        . join(",", $sm_daily)
                        . "_"
                        . join(",", $fp_daily)
                        . "_"
                        . join(",", $inoc_daily)
                        . "_"
                        . join(",", $whitelist_daily)
                        . "_"
                        . join(",", $period_daily);

    foreach($spam_daily as $el){ $DATA['TS_DAILY'] += $el; };
    foreach($nonspam_daily as $el){ $DATA['TI_DAILY'] += $el; }
    foreach($sm_daily as $el){ $DATA['SM_DAILY'] += $el; }
    foreach($fp_daily as $el){ $DATA['FP_DAILY'] += $el; }
    foreach($inoc_daily as $el){ $DATA['INOC_DAILY'] += $el; }
    foreach($whitelist_daily as $el){ $DATA['TI_DAILY'] += $el; }
  }

  if (is_array($spam_weekly) &&
      is_array($nonspam_weekly) &&
      is_array($sm_weekly) &&
      is_array($fp_weekly) &&
      is_array($inoc_weekly) &&
      is_array($whitelist_weekly) &&
      is_array($period_weekly)) {
    $DATA['DATA_WEEKLY'] = join(",", $spam_weekly)
                        . "_"
                        . join(",", $nonspam_weekly)
                        . "_"
                        . join(",", $sm_weekly)
                        . "_"
                        . join(",", $fp_weekly)
                        . "_"
                        . join(",", $inoc_weekly)
                        . "_"
                        . join(",", $whitelist_weekly)
                        . "_"
                        . join(",", $period_weekly);

    foreach($spam_weekly as $el){ $DATA['TS_WEEKLY'] += $el; }
    foreach($nonspam_weekly as $el){ $DATA['TI_WEEKLY'] += $el; }
    foreach($sm_weekly as $el){ $DATA['SM_WEEKLY'] += $el; }
    foreach($fp_weekly as $el){ $DATA['FP_WEEKLY'] += $el; }
    foreach($inoc_weekly as $el){ $DATA['INOC_WEEKLY'] += $el; }
    foreach($whitelist_weekly as $el){ $DATA['TI_WEEKLY'] += $el; }
  }
}

function &DisplayInfos() {
  global $CONFIG, $DATA, $g;
  $validity = "valide";

  $pd = @popen("{$CONFIG['DSPAM']} --version", "r");
  fgets($pd, 4096); // ignore 1st line
  $DATA['DSPAM_VERSION'] = fgets($pd, 4096);
  fgets($pd, 4096); // ignore next line
  $DATA['DSPAM_COPYRIGHT'] = fgets($pd, 4096);
  $DATA['DSPAM_WEBSITE'] = fgets($pd, 4096);
  fgets($pd, 4096); // ignore next line
  $DATA['DSPAM_COPYRIGHT_TEXT'] = fgets($pd, 4096);
  $DATA['DSPAM_COPYRIGHT_TEXT'] .= " " . fgets($pd, 4096);
  fgets($pd, 4096); // ignore next line
  $DATA['DSPAM_CONFIGURE_ARGS'] = str_replace("Configuration parameters: ", "", fgets($pd, 4096));

  pclose($pd);

  if (file_exists("{$g['conf_path']}/nione.lic") &&
      file_exists("{$g['conf_path']}/nione.lic.sha1")) {
    if ($fd = @fopen("{$g['conf_path']}/nione.lic", "r")) {
      $owneru = str_replace("Licensed User: ", "", fgets($fd, 4096));
      if ($owneru == "") { $validity = "invalide (owner data not found)"; }
      $ownerc = str_replace("Company: ", "", fgets($fd, 4096));
      if ($ownerc == "") { $validity = "invalide (company not found)"; }
      $lkey = str_replace("License Key: ", "", fgets($fd, 4096));
      if ($lkey == "") { $validity = "invalide (license key not found)"; }
      $pdate = strtotime( str_replace("Purchase Date: ", "", fgets($fd, 4096)) );
      if ($pdate == "") { $validity = "invalide (purchase date not found)"; }

      fclose($fd);
    } else {
      $validity = "invalide (license files not found)";
    }

    require_once ("knione");
    $gen_lkey = getNIONEKey(trim($owneru), trim($ownerc));
    if (trim($lkey) <> $gen_lkey) { $validity = "invalide (wrong license key)"; }

    $cdate = mktime(0, 0, 0, date("m"),
                             date("d"),
                             date("Y"));

    $edate = mktime(0, 0, 0, date("m", $pdate),
                             date("d", $pdate),
                             date("Y", $pdate)+1);

    if ($edate < $cdate ||
        $pdate > $cdate) {
      $validity = "invalide (license expired)";
    }

    if ($fd = @fopen("{$g['conf_path']}/nione.lic.sha1", "r")) {
      $chksum = str_replace("SHA1 (nione.lic) = ", "", fgets($fd, 4096));
      $chksum_new = sha1_file("{$g['conf_path']}/nione.lic");

      if (trim($chksum) !== trim($chksum_new)) {
        $validity = "invalide (wrong license file checksum)";
      }

      fclose($fd);
    } else {
      $validity = "invalide (license files not found)";
    }
  } else {
    $validity = "invalide (license files not found)";
  }

  $DATA['OWNER'] = $owneru;
  $DATA['COMPANY'] = $ownerc;
  $DATA['LICENSE_KEY'] = $lkey;
  $DATA['LICENSE_VALIDITY'] = $validity;
  $DATA['PURCHASE_DATE'] = date("F j, Y", $pdate);
  $DATA['EXPIRY_DATE'] = date("F j, Y", $edate);
}

/* ========================================================================== */
/* = H I S T O R Y   F U N C T I O N S                                      = */
/* ========================================================================== */

function &DisplayFragment($sigID = "",
                         $from = "",
                         $subject = "",
                         $info = "",
                         $time = "") {
  global $DATA, $USER;

  $sigID = preg_replace('/\/', '///', $sigID);

  $DATA['FROM'] = $from;
  $DATA['SUBJECT'] = $subject;
  $DATA['INFO'] = $info;
  $DATA['TIME'] = $time;

  if($fd = @fopen("{$USER}.frag/{$sigID}.frag", "r")) {
    while (!feof($fd)) {
      /* sanitize HTML markup */
      $line = preg_replace("/</e", "'&lt;'", fgets($fd, 4096));
      $line = preg_replace("/>/e", "'&gt;'", $line);
      $DATA['MESSAGE'] .= $line;
    }
    fclose($fd);
  } else {
    return $input_errors[] = "Unable to open file {$USER}.frag/{$sigID}.frag.";
  }
}

function &DisplayHistory($command = "",
                        $sigID= "",
                        $retrainChecked = array(),
                        $username = "",
                        $retrainParam = "",
                        $currentPage = 1,
                        $hPerPage = 0) {
  global $CONFIG, $USER, $CURRENT_USER, $DATA;

  $buffer = array();
  $history = array();
  $rec = array();
  $rowclass = "rowEven";

  if ($command == "retrainChecked" && count ($retrainChecked) > 0) {
    foreach($retrainChecked as $el) {
      list ($retrain, $signature) = split(":", $el);
      if ($retrain == "innocent") {
        ProcessFalsePositive(quotemeta($signature));
      } else if ($retrain == "innocent" || $retrain == "spam") {
         system("{$CONFIG['DSPAM']} --source=error --class=" . quotemeta($retrain) .
                " --signature=" . quotemeta($signature) .
                " --user " . quotemeta($CURRENT_USER));
      }
    }
    // TODO: Do we need the other params which were submited during the current
    //       POS/GET request?
    pfSenseHeader("/dspam-history.php?user={$username}&page={$currentPage}&hperpage={$hPerPage}");
  } else {
    if ($retrainParam <> "") {
      if ($retrainParam == "innocent") {
        ProcessFalsePositive();
      } else {
        system("{$CONFIG['DSPAM']} --source=error --class=" . quotemeta($retrainParam) .
               " --signature=" . quotemeta($sigID) .
               " --user " . quotemeta($CURRENT_USER));
      }
      // TODO: Do we need the other params which were submited during the current
      //       POS/GET request?
      pfSenseHeader("/dspam-history.php?user={$username}&page={$currentPage}&hperpage={$hPerPage}");
    }
  }

  $LOG = "{$USER}.log";
  if (! file_exists($LOG)) {
    return $input_errors[] = "No historical data is available (log file �{$USER}.log� does not exist).";
  }

  /* Preseed retraining information and delivery errors */

  $fd = fopen($LOG, "r");
  while (!feof($fd)) {
    /* TODO: If the subject line contains a <TAB>, the below array would
     * contain more then seven elements and thus would be invalide.
     *
     * The below code is some kind of a bug fix.
     */
    $cline = fgets($fd, 4096);
    $log_columns = preg_split("/\t/", $cline);
    list($time, $class, $from, $signature, $subject, $info, $messageid) = $log_columns;
    if (count ($log_columns) > 7) {
      /* get values from the array beginning */
      $time = array_shift($log_columns);
      $class = array_shift($log_columns);
      $from = array_shift($log_columns);
      $signature = array_shift($log_columns);
      /* get msgid and info from the array end */
      $messageid = array_pop($log_columns);
      $info = array_pop($log_columns);
      /* the remaining parts are belonging to the subject */
      $subject = implode(" ", $log_columns);
    }

    if ($signature == "") { continue; }
    if ($class == "M" || $class == "F" || $class == "E") {
      if ($class == "E") {
        $rec[$signature]['info'] = $info;
      } else if ($class == "F" || $class == "M") {
        $rec[$signature]['class'] = $class;
        $rec[$signature]['count']++;
        if ($rec[$signature]['info'] == "")
          { $rec[$signature]['info'] = $info; }
      }
      /* filter out resents if there are any.  Since it's the same
       * message we only allow retraining on the 1st occurence of it.
       */
    } else if ($messageid == "" ||
               $rec[$signature]['messageid'] != $messageid ||
               $CONFIG['HISTORY_DUPLICATES'] <> "no") {
      $rec[$signature]['time'] = $time;
      $rec[$signature]['class'] = $class;
      $rec[$signature]['from'] = $from;
      $rec[$signature]['signature'] = $signature;
      $rec[$signature]['subject'] = $subject;
      $rec[$signature]['info'] = $info;
      $rec[$signature]['messageid'] = $messageid;

      array_unshift($buffer, $rec[$signature]);
    }
  } // end while
  fclose($fd);

  /* if the page size wasn't specified, set a default one */
  if ($CONFIG['HISTORY_PER_PAGE'] == 0) {
    $CONFIG['HISTORY_PER_PAGE'] = 50;
  }

  if (isset($hPerPage) && $hPerPage > 0) {
    $CONFIG['HISTORY_PER_PAGE'] = $hPerPage;
  }

  if (isset($currentPage) && isset($CONFIG['HISTORY_PER_PAGE'])) {
    $pages = ceil( (count ($buffer) / $CONFIG['HISTORY_PER_PAGE']) );
    $begin = (($currentPage - 1) * $CONFIG['HISTORY_PER_PAGE']);
    $ranges = ceil ($pages / $CONFIG['HNAV_BUTTONS']);

    /* Now lets just keep the information that we really need. */
    $buffer = array_splice($buffer, $begin, $CONFIG['HISTORY_PER_PAGE']);
  }

  $retrain_checked_msg_no = 0;
  while ($rec = array_pop($buffer)) {
    $time = $rec['time'];
    $class = $rec['class'];
    $from = $rec['from'];
    $signature = $rec['signature'];
    $subject = $rec['subject'];
    $info = $rec['info'];
    $messageid = $rec['messageid'];

    if ($signature == "") { continue; }
    if ($rec[$signature]['displayed'] <> "") { continue; }
    if ($class == "E") { continue; }
    $rec[$signature]['displayed'] = 1;

    /* Resends of retrained messages will need the original from/subject line */
    if ($messageid <> "") {
      if ($from == "<None Specified>") { $from = $rec[$messageid]['from']; }
      if ($subject == "<None Specified>") { $subject = $rec[$messageid]['subject']; }


      if ($rec[$messageid]['from'] == "") { $rec[$messageid]['from'] = $from; }
      if ($rec[$messageid]['subject'] == "") { $rec[$messageid]['subject'] = $subject; }
    }

    if ($from == "") { $from = "<None Specified>"; }
    if ($subject == "") { $subject = "<None Specified>"; }

    $ctime = "";
    if(isset($CONFIG["DATE_FORMAT"])) {
      $ctime = strftime($CONFIG["DATE_FORMAT"], localtime($time));
    } else {
      /* date format was taken from ctime.pl */
      $ctime = date ("D M d H:i:s TY",$time);
      $ttmp = preg_split("/\s+/", $ctime);
      $t = preg_split("/\:/", $ttmp[3]);
      $xtmp = preg_split("/\s+/", $ctime);
      $x = $xtmp[0];
      $m = "a";
      if ($t[0] > 12) { $t[0] -= 12; $m = "p"; }
      if ($t[0] == 0) { $t[0] = 12; }
      $ctime = "{$x} {$t[0]}:{$t[1]}{$m}";
    }

    /* Set the appropriate type and label for this message */

    $cl = "";
    $cllabel = "";
    if ($rec[$signature]['class'] <> "") { $class = $rec[$signature]['class']; }
    if ($class == "S") { $cl = "spam"; $cllabel="SPAM"; }
    else if ($class == "I") { $cl = "innocent"; $cllabel="Good"; }
    else if ($class == "F") {
      if (fmod($rec[$signature]['count'], 2) != 0) {
        $cl = "false"; $cllabel="Miss";
      } else {
        $cl = "innocent"; $cllabel="Good";
      }
    }
    else if ($class == "M") {
      if (fmod($rec[$signature]['count'], 2) != 0) {
          $cl = "missed"; $cllabel="Miss";
      } else {
          $cl = "spam"; $cllabel="SPAM";
      }
    }
    else if ($class == "N") { $cl = "inoculation"; $cllabel="Spam"; }
    else if ($class == "C") { $cl = "blacklisted"; $cllabel="RBL"; }
    else if ($class == "W") { $cl = "whitelisted"; $cllabel="Whitelist"; }
    if ($messageid <> "") {
      if ($rec[$messageid]['resend'] <> "") {
        $cl = "relay";
        $cllabel = "Resend";
      }
      $rec[$messageid]['resend'] = $signature;
    }

    if ($rec[$signature]['info'] <> "") { $info = $rec[$signature]['info']; }

    /* sanitize HTML markup */
    $from = preg_replace("/</e", "'&lt;'", $from);
    $from = preg_replace("/>/e", "'&gt;'", $from);
    $subject = preg_replace('/</e', "'&lt;'", $subject);
    $subject = preg_replace('/>/e', "'&gt;'", $subject);

    if (strlen($from) > $CONFIG['MAX_COL_LEN']) { $from = substr($from, 0, $CONFIG['MAX_COL_LEN']) . "..."; }
    if (strlen($subject) > $CONFIG['MAX_COL_LEN']) { $subject = substr($subject, 0, $CONFIG['MAX_COL_LEN']) . "..."; }

    $rclass = "";
    if ($class == "I" || $class == "W" || $class == "F") { $rclass = "spam"; }
    if ($class == "S" || $class == "M") { $rclass = "innocent"; }

    $retrain = "";
    if (preg_match('/^(M|F)$/', $rec[$signature]['class']) > 0 &&
        fmod($rec[$signature]['count'], 2) != 0) {
      $retrain = "<b>Retrained</b>";
    }

    if ($retrain == "") {
      $retrain = "<A HREF=\"/dspam-history.php?page={$currentPage}&hperpage={$hPerPage}&user={$username}&retrain={$rclass}&signatureID={$signature}\">As " . ucfirst($rclass) . "</A>";
    } else {
      $retrain .= "(<A HREF=\"/dspam-history.php?page={$currentPage}&hperpage={$hPerPage}&user={$username}&retrain={$rclass}&signatureID={$signature}\">Undo</A>)";
    }

    $path = "{$USER}.frag/{$signature}.frag";
    if (file_exists($path)) {
      $pairs = array();
      $pairs['template'] = "fragment";
      $pairs['signatureID'] = $signature;
      $sub = $subject;
      $sub = preg_replace('/#/e', '//', $sub);
      $sub = preg_replace("/(['])/e", '/\\$1/', $sub);
      $pairs['subject'] = $sub;
      $pairs['from'] = $from;
      $pairs['info'] = $info;
      $pairs['time'] = $ctime;
      $pairs['user'] = $username;
      $pairs['page'] = $currentPage;
      $pairs['hperpage'] = $hPerPage;
      $url = SafeVars($pairs);
      $from = "<a href=\"javascript:openwin(580,400,1,'/dspam-hfragment.php?{$url}')\">{$from}</a>";
    }

    $entry = <<<EOD
        <tr>
          <td align="left" valign="top" class="{$rowclass} {$cl}" nowrap="true"><small>{$cllabel}</small></td>
          <td align="left" valign="top" class="{$rowclass}" nowrap="true">
            <small>
              <input class="formfld" name="msgid{$retrain_checked_msg_no}" id="msgid{$retrain_checked_msg_no}" title="Check/Uncheck" type="checkbox" value="{$rclass}:{$signature}">
              <label for="msgid{$retrain_checked_msg_no}">{$retrain}</label>
            </small>
          </td>
          <td align="left" valign="top" class="{$rowclass}" nowrap="true"><small>{$ctime}</small></td>
          <td align="left" valign="top" class="{$rowclass}" nowrap="true"><small>{$from}</small></td>
          <td align="left" valign="top" class="{$rowclass}" nowrap="true"><small>{$subject}</small></td>
          <td align="left" valign="top" class="{$rowclass}" nowrap="true"><small>{$info}</small></td>
        </tr>

EOD;

    $retrain_checked_msg_no++;
    array_push($history, $entry);

    if ($rowclass == "qrowEven") {
      $rowclass = "qrowOdd";
    } else {
      $rowclass = "qrowEven";
    }
    $hurtz++;
  } // end while

  $entry = <<<EOD
        <input name="history_page" type="hidden" value="{$history_page}">

EOD;
  array_push($history, $entry);

  while($line = array_pop($history)) { $DATA['HISTORY'] .= $line; }

  if ($CONFIG['HISTORY_PER_PAGE'] > 0) {
    /* prepare quarantine navbar */
    if (($currentPage - 1) >= 1) { $previousPage = $currentPage - 1; }
    else { $previousPage = 1; }

    if (($currentPage + 1) <= $pages) { $nextPage = $currentPage + 1; }
    else { $nextPage = $pages; }

    $historyFooterBegin = <<<EOD
          <tr>
            <td align="right" valign="middle" class="listtopic" colspan="6">
              <span class="qnavbtn" onmouseover="this.style.backgroundColor='#A5B9E1;'" onmouseout="this.style.backgroundColor='#507DCD;'" title="First Page (1)"><a href="/dspam-history.php?page=1&qperpage={$CONFIG['HISTORY_PER_PAGE']}" title="First Page (1)">|&lt;</a></span>&nbsp;
              <span class="qnavbtn" onmouseover="this.style.backgroundColor='#A5B9E1;'" onmouseout="this.style.backgroundColor='#507DCD;'" title="Page {$previousPage}"><a href="/dspam-history.php?page={$previousPage}&qperpage={$CONFIG['HISTORY_PER_PAGE']}" title="Page {$previousPage}">&lt;</a></span>&nbsp;

EOD;

    $historyFooterEnd = <<<EOD
              <span class="qnavbtn" onmouseover="this.style.backgroundColor='#A5B9E1;'" onmouseout="this.style.backgroundColor='#507DCD;'" title="Page {$nextPage}"><a href="/dspam-history.php?page={$nextPage}&qperpage={$CONFIG['HISTORY_PER_PAGE']}" title="Page {$nextPage}">&gt;</a></span>&nbsp;
              <span class="qnavbtn" onmouseover="this.style.backgroundColor='#A5B9E1;'" onmouseout="this.style.backgroundColor='#507DCD;'" title="Last Page ({$pages})"><a href="/dspam-history.php?page={$pages}&qperpage={$CONFIG['HISTORY_PER_PAGE']}" title="Last Page ({$pages})">&gt;|</a></span>
            </td>
          </tr>

EOD;

    $ranges_array = array();
    $rpages = $pages;
    for ($i = 0; $i < $ranges; $i++) {
      $range = array();
      $range['start'] = (($i + 1)* $CONFIG['HNAV_BUTTONS']) - ($CONFIG['HNAV_BUTTONS'] - 1);

      if (($i + 1) == $ranges) {
        $range['end'] = ($range['start'] + $rpages) - 1;
      } else {
        $range['end'] = (($i + 1)* $CONFIG['HNAV_BUTTONS']);
        $rpages -= $CONFIG['HNAV_BUTTONS'];
      }

      $ranges_array[$i] = $range;
    }

    /* generate nav buttons */
    foreach($ranges_array as $range){
      if ($currentPage >= $range['start'] && $currentPage <= $range['end']) {
        for ($i = $range['start']; ; $i++) {
          if ($i > $range['end']) {
            break;
          } else {
            if ($i == $currentPage) {
              $historyFooter .= "<span class=\"qnavbtnhl\">{$i}</span>&nbsp\n";
            } else {
              $historyFooter .= "<span class=\"qnavbtn\" onmouseover=\"this.style.backgroundColor='#A5B9E1;'\" onmouseout=\"this.style.backgroundColor='#507DCD;'\" title=\"Page {$i}\"><a href=\"/dspam-history.php?page={$i}&hperpage={$CONFIG['HISTORY_PER_PAGE']}\" title=\"Page {$i}\">{$i}</a></span>&nbsp\n";
            }
          }
        }
      }
    }

    $DATA['HISTORY_FOOTER'] = $historyFooterBegin . $historyFooter .$historyFooterEnd;
    $DATA['HPAGES'] = $pages;
    $DATA['HPAGE'] = $currentPage;

/*
    $DATA['HISTORY'] .= "<tr><td align=\"left\" valign=\"top\" colspan=\"6\"><center>[";
    if (($history_pages > 1) && ($history_page > 1)) {
      $i = $history_page - 1;
      $DATA['HISTORY'] .= "<a href=\"/dspam-history.php?user={$username}&history_page={$i}\">&nbsp;&lt;&nbsp;</a>";
    }
    for($i = 1; $i <= $history_pages; $i++) {
      if ($i == $history_page) {
        $DATA['HISTORY'] .= "<a href=\"/dspam-history.php?user={$username}&history_page={$i}\"><big><strong>&nbsp;$i&nbsp;</strong></big></a>";
      } else {
       $DATA['HISTORY'] .= "<a href=\"/dspam-history.php?user={$username}&history_page={$i}\">&nbsp;{$i}&nbsp;</a>";
      }
    }
    if (($history_pages > 1) && ($history_page < $history_pages)) {
      $i = $history_page + 1;
      $DATA['HISTORY'] .= "<a href=\"/dspam-history.php?user={$username}&history_page={$i}\">&nbsp;&gt;&nbsp;</a>";
    }
    $DATA['HISTORY'] .= "]</center></td></tr>";
*/
  } // end if
}

/* ========================================================================== */
/* = A N A L Y S I S   F U N C T I O N S                                    = */
/* ========================================================================== */

function &DisplayAnalysis() {
  global $USER, $CURRENT_USER, $CONFIG, $DATA;
  $LOG = "{$USER}.log";

  $Stats = array(
             "daily"  => array(),
             "weekly" => array()
           );

  list(, $min, $hour, $mday, $mon, $year, , ,) = (localtime(time()));
  $daystart = mktime(0, 0, 0, $mon, $mday, $year);
  $periodstart = $daystart - (3600 * 24 * 13); /* 2 Weeks ago */
  $dailystart = time() - (3600 * 23);

  /* TODO: There's an issue that the Perl timelocal returns
   *       different values compared to PHP's mktime. There's a
   *       difference of 2678400, which will be added manually below.
   */
  $daystart += 2678400;
  $periodstart += 2678400;

  if (file_exists($LOG)) {
    if ($fd = @fopen($LOG, "r")) {
      $scount = 0;
      $icount = 0;
      $wcount = 0;
      $fcount = 0;
      $mcount = 0;

      while(!feof($fd)) {
        $buffer = fgets($fd, 4096);
        /* drop blank lines */
        if (strlen($buffer) == 0) { continue; }
        list($t_log, $c_log) = preg_split("/\t/", $buffer);

        /* Only Parse Log Data in our Time Period */
        /* TODO: The below if should evaluate to true at least for some data */
        if ($t_log >= $periodstart) {
          list(, $tmin, $thour, $tday, $tmon, , , ,) = (localtime($t_log));
          $tmon++;

          foreach (array('weekly', 'daily') as $period) {
            $idx = 0;
            if ($period == "weekly") {
              $idx= "{$tmon}/{$tday}";
            } else {
              if ($t_log <= $dailystart) { continue; }
              $idx = To12Hour($thour);
            }
            if (is_array($Stats[$period]) && ! array_key_exists ($idx, $Stats[$period])) {
              $Stats[$period][$idx] = array(
                'nonspam' => 0,
                'spam'    => 0,
                'title'   => $idx,
                'idx'     => $t_log);
            }
            /* TODO: Is passing by reference here correct? */
            $hr =& $Stats[$period][$idx];
            /* S => spam */
            if ($c_log== "S") {
              $hr['spam']++;
              $scount++;
            }
            /* I => innocent W => whitelisted */
            if ($c_log == "I" || $c_log == "W") {
              $hr['nonspam']++;
              if ($c_log == "I") { $icount++; }
              else { $wcount++; }
            }
            /* F => false positive */
            if ($c_log == "F") {
              $hr['spam']--;
              if ($hr['spam'] < 0) { $hr['spam'] = 0; }
              $hr['nonspam']++;
              $fcount++;
            }
            /* M => spam miss */
            if ($c_log == "M") {
              $hr['nonspam']--;
              if ($hr['nonspam'] < 0) { $hr['nonspam'] = 0; }
              $hr['spam']++;
              $mcount++;
            }
          }
        }
      }

      fclose($fd);
    } else {
      return $input_errors[] = "Unable to open log file: {$LOG}.";
    }

    usort ((array_values ($Stats[$period])), "cmpArrayValues");

    foreach (array('weekly', 'daily') as $period) {
      $uc_period = strtoupper($period);
      $hk = "DATA_{$uc_period}";
      $lst = array();

      foreach (array_values($Stats[$period]) as $hr) {
        foreach (array('spam', 'nonspam', 'title') as $type ) {

          if (empty($lst[$type])) {
            $lst[$type] = array();
          }

          /* populate (newly) created array */
          $lst[$type][] = $hr[$type];

          $totk="";

          if ($type == "spam") { $totk="S"; }
          else if ($type == "nonspam") { $totk="I"; }

          if ($totk == "") { continue; }

          $sk="T{$totk}_{$uc_period}";
          if (empty($DATA[$sk])) { $DATA[$sk] = 0; }

          $DATA[$sk] += $hr[$type];
        }
      } // end foreach
      $DATA[$hk] =
        @join(",",$lst['spam']) . "_" .
        @join(",",$lst['nonspam']) . "_" .
        @join(",",$lst['title']);
    } // end foreach
  } else {
    return $input_errors[] = "No historical data is available (log file �{$LOG}� does not exist).";
  } // end if
}

function cmpArrayValues($a, $b) {
  if ($a['idx'] == $b['idx'])
    return 0;

  return ($a['idx'] < $b['idx']) ? -1 : 1;
}

/* ========================================================================== */
/* = P E R E F E R E N C E S   F U N C T I O N S                            = */
/* ========================================================================== */

function &DisplayPreferences($mode = "", &$statusmsg){
  global $USER, $CURRENT_USER, $CONFIG, $DATA;
  $FILE = "{$USER}.prefs";
  $username = $CURRENT_USER;

  if ($_POST) {
    $pconfig = $_POST;

    if ($pconfig['chk_feature_nr'] <> "on") {
      $pconfig['chk_feature_nr'] = "off";
    }

    if ($pconfig['chk_feature_optin'] <> "on") {
      $pconfig['chk_feature_optin'] = "off";
    }

    if ($pconfig['chk_feature_optout'] <> "on") {
      $pconfig['chk_feature_optout'] = "off";
    }

    if ($pconfig['chk_feature_at'] <> "on") {
      $pconfig['chk_feature_at'] = "off";
    }

    if ($pconfig['chk_feature_aw'] <> "on") {
      $pconfig['chk_feature_aw'] = "off";
    }

    if ($CONFIG['PREFERENCES_EXTENSION'] == 1) {
      if ($pconfig['msgtag'] == "") {
        $pconfig['msgtag'] = "''";
      } else {
        $pconfig['msgtag'] = quotemeta($pconfig['msgtag']);
      }

      exec("{$CONFIG['DSPAM_BIN']}/dspam_admin ch pref " . quotemeta($username) .
        " trainingMode " . quotemeta($pconfig['rad_train']) . " > /dev/null");
      exec("{$CONFIG['DSPAM_BIN']}/dspam_admin ch pref " . quotemeta($username) .
        " spamAction " . quotemeta($pconfig['rad_train_action']) . " > /dev/null");
      exec("{$CONFIG['DSPAM_BIN']}/dspam_admin ch pref " . quotemeta($username) .
        " signatureLocation " . quotemeta($pconfig['signatureLocation']) . " > /dev/null");
      exec("{$CONFIG['DSPAM_BIN']}/dspam_admin ch pref " . quotemeta($username) .
        " spamSubject " . quotemeta($pconfig['msgtag']) . " > /dev/null");
      exec("{$CONFIG['DSPAM_BIN']}/dspam_admin ch pref " . quotemeta($username) .
        " statisticalSedation " . quotemeta($pconfig['rad_filter_sens']) . " > /dev/null");
      exec("{$CONFIG['DSPAM_BIN']}/dspam_admin ch pref " . quotemeta($username) .
        " enableBNR " . quotemeta($pconfig['chk_feature_nr']) . " > /dev/null");
      exec("{$CONFIG['DSPAM_BIN']}/dspam_admin ch pref " . quotemeta($username) .
        " optOut " . quotemeta($pconfig['chk_feature_optout']) . " >/dev/null");
      exec("{$CONFIG['DSPAM_BIN']}/dspam_admin ch pref " . quotemeta($username) .
        " optIn " . quotemeta($pconfig['chk_feature_optin']) . " >/dev/null");
      exec("{$CONFIG['DSPAM_BIN']}/dspam_admin ch pref " . quotemeta($username) .
        " showFactors " . quotemeta($pconfig['chk_feature_at']) . " > /dev/null");
      exec("{$CONFIG['DSPAM_BIN']}/dspam_admin ch pref " . quotemeta($username) .
        " enableWhitelist " . quotemeta($pconfig['chk_feature_aw']) . " > /dev/null");
    } else {
      $prefsstr = <<<EOD
trainingMode={$pconfig['rad_train']}
spamAction={$pconfig['rad_ident_action']}
spamSubject={$pconfig['msgtag']}
statisticalSedation={$pconfig['rad_filter_sens']}
enableBNR={$pconfig['chk_feature_nr']}
optIn={$pconfig['chk_feature_optin']}
optOut={$pconfig['chk_feature_optout']}
showFactors={$pconfig['chk_feature_at']}
enableWhitelist={$pconfig['chk_feature_aw']}
signatureLocation={$pconfig['rad_train_action']}

EOD;

      if ($fd = @fopen("{$FILE}","w")) {
        fwrite($fd, $prefsstr);
        fclose($fd);
      } else {
        return $input_errors[] = "Unable to write preferences to file: {$FILE}";
      }
    }

    $statusmsg = "DSPAM preferences have been written to: {$FILE}.";
  }

  $PREFS =& GetPrefs($username);

  $DATA["SEDATION_{$PREFS['statisticalSedation']}"] = 'checked="checked"';
  $DATA["S_{$PREFS['trainingMode']}"] = 'checked="checked"';
  $DATA["S_ACTION_" . strtoupper($PREFS['spamAction'])] = 'checked="checked"';
  $DATA["S_LOC_" . strtoupper($PREFS['signatureLocation'])] = 'checked="checked"';
  $DATA["SPAM_SUBJECT"] = $PREFS['spamSubject'];
  if ($PREFS['optIn'] == "on") {
    $DATA['C_OPTIN'] = 'checked="checked"';
  }
  if ($PREFS['optOut'] == "on") {
    $DATA['C_OPTOUT'] = 'checked="checked"';
  }
  if ($PREFS['enableBNR'] == "on") {
    $DATA['C_BNR'] = 'checked="checked"';
  }
  if ($PREFS['showFactors'] == "on") {
    $DATA['C_FACTORS'] = 'checked="checked"';
  }
  if ($PREFS['enableWhitelist'] == "on") {
    $DATA['C_WHITELIST'] = 'checked="checked"';
  }
}

function sortByRating($a, $b){
  if ($a['rating'] == $b['rating']) {
    return 0;
  }

  return ($a['rating'] < $b['rating']) ? -1 : 1;
}

/* ========================================================================== */
/* = Q U A R A N T I N E   F U N C T I O N S                                = */
/* ========================================================================== */

function &ProcessQuarantine($signatures = array(),
                           $action = "None",
                           $sortBy = "Rating",
                           $currentPage = 1,
                           $qPerPage = 0){
  switch($action){
    case "None":
      $input_errors =& DisplayQuarantine($sortBy, $currentPage, $qPerPage);
      break;
    case "manyNotSpam":
      $input_errors =& QuarantineManyNotSpam($signatures, $sortBy, $currentPage, $qPerPage);
      break;
    case "deleteAll":
      QuarantineDeleteSpam($action, $signatures, $sortBy, $currentPage, $qPerPage);
      break;
    default:
      QuarantineDeleteSpam();
  } // switch

  CheckQuarantine();

  return $input_errors;
}

function &ProcessFalsePositive($sigID = "",
                              $sortBy = "Rating",
                              $currentPage = 1,
                              $qPerPage = 0) {
  global $MAILBOX, $CONFIG, $TMPFILE, $CURRENT_USER;
  $buffer = array();
  $head = array();
  $singatures = array();
  $found = 0;
  $error = false;

  if ($sigID == "") {
    return $input_errors[] = "No Message ID Specified.";
  }

  /* read the user's mailbox line by line into a buffer */
  $fd = fopen("{$MAILBOX}", "r");
  while (!feof($fd)) {
      $line = chop(fgets($fd, 4096));
      array_push($buffer, $line);
  }
  fclose ($fd);

  /* iterate over the mailbox buffer */
  reset($buffer);
  $i = 0;
  while ($i < count($buffer)) {
    $temp = array();
    $head = array();
    $mode = 0;
    $buff = "";

    /* this while tries to iterate over one single mesage including
     * the message header and the message body.
     */
    while((preg_match('/^From /', $buff) == 0) && ($i < count($buffer))) {
      $buff = $buffer[0];

      /* switch mode if we are hitting DSPAMs
       * pseudo From QUARANTINE line (without a
       * colon after the From).
       */
      if (preg_match('/^From /', $buff) > 0) {
        if ($mode == 0) { $mode = 1; }
        else { continue; }
      }

      $buff = array_shift($buffer);
      if (preg_match('/^From /', $buff) == 0) {
        array_push($temp, $buff);
      }

      continue;
    }

    foreach($temp as $tempel) {
      if ($tempel == "") { break; }
      list($key, $val) = preg_split('/\: ?/', $tempel, 2);
      $head[$key] = $val;
    }
    if ($head['X-DSPAM-Signature'] == $sigID) {
      $found = 1;
      $old_erep = error_reporting(E_ALL);
      if ($pd = @popen("|{$CONFIG['DSPAM']} {$CONFIG['DSPAM_ARGS']}  >{$TMPFILE} 2>&1", "w")) {
        $pdresult = fread($handle, 2096);

        foreach($temp as $tempel) {
          fwrite($pd, "{$tempel}\n");
        }

        pclose($pd);
        error_reporting($old_erep);
      } else {
        $error = true;
        $input_errors[] = "Unable to ope process pipe in function <code>ProcessFalsePositive</code>.";
      }
    }
  }

  /* Couldn't find the message, so just retrain on signature */
  if (!$found) {
    system("$CONFIG{'DSPAM'} --source=error --class=innocent --signature=" . quotemeta($sigID) .
           " --user " . quotemeta($CURRENT_USER));
  }

  if ($error) {
    $log = array();
    $fd = fopen("{$TMPFILE}", "r");
    while (!feof($handle)) {
      $log .= fgets($fd, 4096);
    }
    fclose($fd);
    unlink("{$TMPFILE}");
    return $input_errors[] = $log;
  }

  unlink("{$TMPFILE}");
  $signatures[$sigID] = "on";
  return QuarantineDeleteSpam("", $signatures, $sortBy, $currentPage, $qPerPage);
}

function &QuarantineManyNotSpam($signatures = array(), $sortBy = "Rating", $currentPage = 1, $qPerPage = 0){
  global $MAILBOX, $USER;
  $buffer = array();
  $errors = array();

  /* read the user's mailbox line by line into a buffer */
  $fd = fopen("{$MAILBOX}", "r");
  while (!feof($fd)) {
      $line = chop(fgets($fd, 4096));
      array_push($buffer, $line);
  }
  fclose ($fd);

  if ($fd_FILE = @fopen("{$MAILBOX}", "w")) {
    $fd_RETRAIN = fopen("{$USER}.retrain.log", "a");

    /* iterate over the mailbox buffer */
    reset($buffer);
    $i = 0;
    while ($i < count($buffer)) {
      $temp = array();
      $head = array();
      $mode = 0;
      $buff = "";

      /* this while tries to iterate over one single mesage including
       * the message header and the message body.
       */
      while((preg_match('/^From /', $buff) == 0) && ($i < count($buffer))) {
        $buff = $buffer[0];

        /* switch mode if we are hitting DSPAMs
         * pseudo From QUARANTINE line (without a
         * colon after the From).
         */
        if (preg_match('/^From /', $buff) > 0) {
          if ($mode == 0) {
            $mode = 1;
            $buff = array_shift($buffer);
            array_push($temp, $buff);
            $buff = "";
            continue;
          } else {
            continue;
          }
        }

        $buff = array_shift($buffer);
        array_push($temp, $buff);

        continue;
      }

      /* populate the header array with header fields */
      foreach($temp as $tempel) {
        if ($tempel == "") { break; }
        list($key, $val) = preg_split('/\: ?/', $tempel, 2);
        $head[$key] = $val;
      }

      $delivered = 0;
      if ($signatures["chkmsg-{$head['X-DSPAM-Signature']}"] <> "") {
        $err = Deliver($temp);
        if ($err == "") {
          $delivered = 1;
        } else {
          array_push($errors, $err);
        }
      }
      if (!$delivered) {
        foreach($temp as $tempel) {
          fwrite($fd_FILE, "{$tempel}\n");
        }
      } else {
        fwrite($fd_RETRAIN, strval(time()) . "\t{$head['X-DSPAM-Signature']}\tinnocent\n");
      }

      $i++;
    } // end while

    fclose($fd_FILE);
    fclose($fd_RETRAIN);
  } else {
    return $input_errors[] = "Unable to open mailbox file: {$MAILBOX}.";
  }

  if (count($errors) > 0) {
    return $errors;
  }

  return DisplayQuarantine($sortBy, $currentPage, $qPerPage);
}

function Deliver($temp = array()) {
  global $CONFIG;

  if (! file_exists("/tmp/dspam-error-output.txt")) {
    touch("/tmp/dspam-error-output.txt");
  }
  $descriptorspec = array(
     0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
     1 => array("pipe", "w"),  // stdout is a pipe that the child will write to
     2 => array("file", "/tmp/dspam-error-output.txt", "a") // stderr is a file to write to
  );

  list($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$sizeb,
       $atime,$mtimeb,$ctime,$blksize,$blocks)
       = stat("/tmp/dspam-error-output.txt");
  clearstatcache();

  $cwd = '/tmp';
  $process = @proc_open("{$CONFIG['DSPAM']} {$CONFIG['DSPAM_ARGS']}",
                       $descriptorspec,
                       $pipes);

  if (is_resource($process)) {
    foreach($temp as $tempel) {
      if (! @fwrite($pipes[0], "{$tempel}\n")) {
        return "error while writting to pipe.";
      }
    }

    fclose($pipes[0]);
    fclose($pipes[1]);
    $return_value = proc_close($process);

    /* this isn't an elegant solution to determine whether
     * DSPAM did report some errors, but it works for now
     */
    list($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$sizea,
         $atime,$mtimea,$ctime,$blksize,$blocks)
         = stat("/tmp/dspam-error-output.txt");

    if ($mtimeb <> $mtimea) { return "DSPAM did report some errors to /tmp/dspam-error-output.txt.\n" .
                                   "Please check this particular file."; }
  } else {
    return "process is not a resource type.";
  }

  return "";
}

function getLayoutedMessage($msgbuffer = "", $sigID = "", $showpart = 0, $ctype = 0){

  if ($msgbuffer == "") {
    return '<span class="errmsg">An error occured while parsing the message (no message).</span>';
  }
  if ($sigID == "") {
    return '<span class="errmsg">An error occured while parsing the message (no signature).</span>';
  }

  if( extension_loaded( 'mailparse' ) ) {
    $msgdate = "N/A";
    $msgfrom = "N/A";
    $msgsub = "N/A";
    $msgto = "N/A";

    $mime = mailparse_msg_create();
    mailparse_msg_parse($mime, $msgbuffer);
    /* return an array of message parts - this contsists of the
     * names of the parts only.
     */
    $struct = mailparse_msg_get_structure($mime);
    $htmlstr = <<<EOD
        <tr>
          <td align="left" valign="top" class="listtopic" colspan="3">Message Infos</td>
        </tr>
        <tr>
          <td valign="baseline" class="vncell" colspan="1">Date:</td>
          <td class="vtable" colspan="2">%MSGDATE%</td>
        </tr>
        <tr>
          <td valign="baseline" class="vncell" colspan="1">From:</td>
          <td class="vtable" colspan="2">%MSGFROM%</td>
        </tr>
        <tr>
          <td valign="baseline" class="vncell" colspan="1">Subject:</td>
          <td class="vtable" colspan="2">%MSGSUB%</td>
        </tr>
        <tr>
          <td valign="baseline" class="vncell" colspan="1">To:</td>
          <td class="vtable" colspan="2">%MSGTO%</td>
        </tr>
        <tr>
          <td class="list" height="12" colspan="3">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="top" class="listtopic">Message Part</td>
          <td align="left" valign="top" class="listtopic">Part Type</td>
          <td align="left" valign="top" class="listtopic">Part Encoding</td>
        </tr>
        <tr>

EOD;

    /* print a choice of sections */
    foreach($struct as $st) {

      /* get a handle on the message resource for a subsection */
      $section = mailparse_msg_get_part($mime, $st);
      /* get content-type, encoding and header information for that section */
      $info = mailparse_msg_get_part_data($section);

      /* replace placeholder with real data */
      if ($info['headers']['date'] <> "") {
        $htmlstr = str_replace("%MSGDATE%", $info['headers']['date'], $htmlstr);
      }
      if ($info['headers']['from'] <> "") {
        $htmlstr = str_replace("%MSGFROM%", $info['headers']['from'], $htmlstr);
      }
      if ($info['headers']['subject'] <> "") {
        $htmlstr = str_replace("%MSGSUB%", $info['headers']['subject'], $htmlstr);
      }
      if ($info['headers']['to'] <> "") {
        $htmlstr = str_replace("%MSGTO%", $info['headers']['to'], $htmlstr);
      }

      $fontStyle = "";
      if ($showpart && $showpart == $st) { $fontStyle = " style=\"font-weight: bolder;\""; }

      if ($info["content-type"] == "text/html") {
        $htmlstr .= "<td align=\"left\" valign=\"top\" class=\"vncell\"{$fontStyle}>" .
                   "<a href=\"/dspam-viewmsg.php?ctype=1&showpart={$st}&signatureID={$sigID}&command=viewMessage\">{$st}</a>" .
                   "</td>\n";
      } else {
        $htmlstr .= "<td align=\"left\" valign=\"top\" class=\"vncell\"{$fontStyle}>" .
                   "<a href=\"/dspam-viewmsg.php?showpart={$st}&signatureID={$sigID}&command=viewMessage\">{$st}</a>" .
                   "</td>\n";
      }

      $htmlstr .= <<<EOD
          <td align="left" valign="top" class="vtable"{$fontStyle}>{$info["content-type"]}</td>
          <td align="left" valign="top" class="vtable"{$fontStyle}>{$info["charset"]}</td>
        </tr>

EOD;
    } // end foreach

    /* if we were called to display a part, do so now */
    if ($showpart)  {
      /* get a handle on the message resource for the desired part */
      $sec = mailparse_msg_get_part($mime, $showpart);

      $htmlstr .= <<<EOD
        <tr>
          <td class="list" height="12" colspan="3">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="top" class="listtopic" colspan="3">Section {$showpart}</td>
        </tr>
        <tr>
          <td align="top" valign="left" colspan="3" class="vncell">
EOD;

      ob_start();
      mailparse_msg_extract_part($sec, $msgbuffer);
      $contents = ob_get_contents();
      ob_end_clean();
      $contents = wordwrap( str_replace("&gt;", "&gt;\n", $contents), 100, "\n" );
      /* quote the message for safe display in a browser */
      if ($ctype = 1) {
       /* a html email message */
       $htmlstr .= "<pre style=\"font-size: 1.4em;\">" . $contents . "</pre></td>\n</tr>\n";
      } else {
       /* an ASCII (text) email message */
       $htmlstr .= htmlentities($contents) . "</td>\n</tr>\n";
      }
    }

    return $htmlstr;
  } else {
    return '<span class="errmsg">Could not load mailparse extension.</span>';
  }
}

function &QuarantineViewMessage($sigID = "",
                               $showpart = 0,
                               $ctype = 0,
                               $sortBy = "Rating",
                               $currentPage = 1,
                               $qPerPage = 0) {
  global $MAILBOX, $DATA;
  $buffer = array(); // mailbox buffer

  if ($sigID == "") {
    return $input_errors[] = "No Message ID Specified.";
  }

  /* save data to be displayed as HTML form data */
  $DATA['MESSAGE_ID'] = $sigID;
  $DATA['SHOWPART'] = $showpart;
  $DATA['CONTENT_TYPE'] = $ctype;
  $DATA['QPAGE'] = $currentPage;
  $DATA['SORTBY'] = $sortBy;

  if ($qPerPage > 0) { $CONFIG['QUARANTINE_PER_PAGE'] = $qPerPage; }

  /* read the user's mailbox line by line into a buffer */
  $fd = fopen("{$MAILBOX}", "r");
  while (!feof($fd)) {
      $line = chop(fgets($fd, 4096));
      array_push($buffer, $line);
  }
  fclose ($fd);

  /* iterate over the mailbox buffer */
  reset($buffer);
  $i = 0;
  while ($i < count($buffer)) {
    $temp = array();
    $head = array();
    $mode = 0;
    $buff = "";

    /* this while tries to iterate over one single mesage including
     * the message header and the message body.
     */
    while((preg_match('/^From /', $buff) == 0) && ($i < count($buffer))) {
      $buff = $buffer[0];

      /* switch mode if we are hitting DSPAMs
       * pseudo From QUARANTINE line (without a
       * colon after the From).
       */
      if (preg_match('/^From /', $buff) > 0) {
        if ($mode == 0) { $mode = 1; }
        else { continue; }
      }

      $buff = array_shift($buffer);
      if (preg_match('/^From /', $buff) == 0) {
        array_push($temp, $buff);
      }

      continue;
    }

    /* populate the header array with header fields */
    foreach($temp as $tempel) {
      if ($tempel == "") { break; }
      list($key, $val) = preg_split('/\: ?/', $tempel, 2);
      $head[$key] = $val;
    }
    if ($head['X-DSPAM-Signature'] == $sigID) {
      foreach($temp as $tempel) {
        $tempel = preg_replace("/</e", "'&lt;'", $tempel);
        $tempel = preg_replace("/>/e", "'&gt;'", $tempel);
        $DATA['MESSAGE'] .= "{$tempel}\n";
      }
    }

    $i ++;
  } // end while
}

function QuarantineDeleteSpam($deleteAll = "", $signatures = array(), $sortBy = "Rating", $currentPage = 1, $qPerPage = 0){
  global $USER, $MAILBOX;
  $buffer = array();

  /* this is the most easiest operation: If the user wants
   * to completly delete any quarantined message, simply
   * open his mailbox in write mode, which empties the
   * user's mailbox file.
   */
  if ($deleteAll <> "") {

    list($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
         $atime,$mtime,$ctime,$blksize,$blocks) = stat("{$USER}.mbox");

    $fd = fopen("{$USER}.mbox.size", "r");
    $sz = chop(fgets($fd, 4096));
    fclose($fd);

    if ($sz == $size) {
      $fd = fopen("{$MAILBOX}", "w");
      fclose($fd);
      unlink("{$USER}.mbox.size");
      unlink("{$USER}.mboxwarn");
    } else {
      return DisplayQuarantine($sortBy, $currentPage, $qPerPage);
    }

    //$FORM{'template'} = "performance";
    CheckQuarantine();
    return DisplayIndex();
  }

  /* iterate over the user's mailbox and store its contents in a buffer */
  $fd = fopen("{$MAILBOX}", "r");
  while (!feof($fd)) {
    $line = chop(fgets($fd, 4096));
    array_push($buffer, $line);
  }
  fclose($fd);

  /* open the user's mailbox in write mode. This empties the mailbox! */
  fopen("{$MAILBOX}", "w");

  /* iterate over the mailbox buffer */
  reset($buffer);
  $i = 0;
  while ($i < count($buffer)) {
    $temp = array();
    $head = array();
    $mode = 0;

    /* this while tries to iterate over one single mesage including
     * the message header and the message body.
     */
    while((preg_match('/^From /', $buff) == 0) && ($i < count($buffer))) {
      $buff = $buffer[0];

      /* switch mode if we are hitting DSPAMs
       * pseude From QUARANTINE line (without a
       * colon after the From).
       */
      if (preg_match('/^From /', $buff) > 0) {
        if ($mode == 0) {
          $mode = 1;
          $buff = array_shift($buffer);
          array_push($temp, $buff);
          $buff = "";
          continue;
        } else {
          continue;
        }
      }
      $buff = array_shift($buffer);
      array_push($temp, $buff);

      continue;
    }

    /* populate the header array with header fields */
    foreach($temp as $tempel) {
      if ($tempel == "") { break; }
      list($key, $val) = preg_split('/\: ?/', $tempel, 2);
      $head[$key] = $val;
    }

    /* if the current DSPAM signature wasn't selected by the
     * user to be deleted, write it back to the user's mailbox.
     */
    if ($signatures["chkmsg-{$head['X-DSPAM-Signature']}"] == "") {
      foreach($temp as $tempel) {
        fwrite($fd, "{$tempel}\n");
      }
    }

    $i++;
  } // end while
  fclose($fd);

  return;
}

function sortBySubject($a, $b){
  $lca = strtolower ($a['Subject']);
  $lcb = strtolower ($b['Subject']);

  return strcmp($lca, $lcb);
}

function sortByFrom($a, $b){
  $lca = strtolower ($a['From']);
  $lcb = strtolower ($b['From']);

  return strcmp($lca, $lcb);
}

function &DisplayQuarantine($sortBy = "Rating", $currentPage = 1, $qPerPage = 0) {
  global $USER, $CURRENT_USER, $CONFIG, $DATA, $MAILBOX;
  $alertcfg = &$config['installedpackages']['dspamalerts']['config'];
  $alerts = array();

  if (file_exists("{$USER}.mbox")) {
    list($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
         $atime,$mtime,$ctime,$blksize,$blocks) = stat("{$USER}.mbox");

    $fd = fopen("{$USER}.mbox.size", "w");
    fwrite($fd, "{$size}");
    fclose($fd);

    $fd = fopen("{$MAILBOX}.stamp", "a+");
    fclose($fd);
    chmod("{$MAILBOX}.stamp", 0660);

    /* process alert names */
    if (is_array($alertcfg)) {
      $alert_counter = 0;

      foreach ($alertcfg as $alert) {
        $alerts[$alert_counter] = $alert['alertname'];
        $alert_counter++;
      }
    }

    $mode = "";
    $buffer = array();
    $headings = array();
    $rowclass = "qrowEven";
    $fd = fopen("{$MAILBOX}", "r");

    while (!feof($fd)) {
      $mbxline = chop(fgets($fd, 4096));

      if ($mbxline <> "") {
        if (($mode == "")) {
          if (preg_match('/^From /', $mbxline) > 0) {
            $mode = 1;
          } else {
            continue;
          }
        }

        array_push($buffer, $mbxline);
        continue;
      }

      if ($mode == "") { continue; }

      $alert = 0;
      $new = array();

      foreach($buffer as $buf_element){
        foreach($alerts as $al){
          if (preg_match("/{$al}/i", $buf_element) > 0) {
            $alert = 1;
          }
        }

        if (preg_match('/^From /', $buf_element) > 0) {
          $a = split(' ', $buf_element);
          $x = 2;

          for ($i = 0; $i < count ($a); $i++) {
            if ((preg_match('/\@|>/', $a[$i]) > 0) && $i > $x) {
              $x = $i + 1;
            }
          }

          for ($i = 1; $i < $x; $i++) { array_shift($a); }
          $start = join(" ", $a);
        } else {
          list($key, $val) = preg_split('/\: ?/', $buf_element, 2);
          $new[$key] = $val;
        }
      }

      if ($rowclass == "qrowEven") {
        $rowclass = "qrowOdd";
      } else {
        $rowclass = "qrowEven";
      }

      $new['alert'] = $alert;

      if ($alert) { $rowclass="qrowAlert"; }

      $new['Sub2'] = $new['X-DSPAM-Signature'];
      if (strlen($new['Subject']) > $CONFIG['MAX_COL_LEN']) {
        $new['Subject'] = substr($new['Subject'], 0, $CONFIG['MAX_COL_LEN']) . "...";
      }

      if (strlen($new['From']) > $CONFIG['MAX_COL_LEN']) {
        $new['From'] = substr($new['From'], 0, $CONFIG['MAX_COL_LEN']) . "...";
      }

      if ($new['Subject'] == "") {
        $new['Subject'] = "<None Specified>";
      }

      //$new->{'rating'} = $new->{'X-DSPAM-Probability'} * $new->{'X-DSPAM-Confidence'};
      $new['rating'] = $new['X-DSPAM-Confidence'];

      foreach(array_keys($new) as $key) {
        if ($key == "X-DSPAM-Signature") { continue; }
        preg_replace('/</', '/\&lt\;/', $new[$key]);
        preg_replace('/>/', '/\&gt\;/', $new[$key]);
      }

      array_push($headings, $new);

      $buffer = array();
      $mode = "";
      continue;
    } // end while (!feof($fd))

    if (! isset($sortBy) || $sortBy == "") {
      $sortBy = $CONFIG['SORT_DEFAULT'];
    }
    if ($sortBy == "Rating") {
      usort($headings, "sortByRating");
    }
    if ($sortBy == "Subject") {
      usort($headings, "sortBySubject");
    }
    if ($sortBy == "From") {
      usort($headings, "sortByFrom");
    }
    if ($sortBy == "Date") {
      array_reverse ($headings);
    }

  /*

          <tr>
            <td align="left" valign="top" class="listtopic" width="8%">&nbsp;</td>
            <td align="left" valign="top" class="listtopic" width="5%" onclick="sortmsg('Rating');" style="cursor: pointer;">Rating</td>
            <td align="left" valign="top" class="listtopic" width="20%" onclick="sortmsg('Date');" style="cursor: pointer;">Date</td>
            <td align="left" valign="top" class="listtopic" width="25%" onclick="sortmsg('From');" style="cursor: pointer;">From</td>
            <td align="left" valign="top" class="listtopic" width="42%" onclick="sortmsg('Subject');" style="cursor: pointer;">Subject</td>
          </tr>

  */

    $DATA['SORTBY'] = $sortBy;
    $DATA['SORT_SELECTOR'] .= <<<EOD
          <tr>
            <td align="left" valign="top" class="listtopic" width="10%">&nbsp;</td>

EOD;

    if ($sortBy == "Rating") {
      $DATA{'SORT_SELECTOR'} .= "<td align=\"left\" valign=\"top\" class=\"listtopic\" width=\"5%\" onclick=\"sortmsg('Rating');\" style=\"cursor: pointer;\">Rating&nbsp;&nbsp;&loz;</td>";
    } else {
      $DATA{'SORT_SELECTOR'} .= "<td align=\"left\" valign=\"top\" class=\"listtopic\" width=\"5%\" onclick=\"sortmsg('Rating');\" style=\"cursor: pointer;\">Rating</td>";
    }
    if ($sortBy == "Date") {
      $DATA{'SORT_SELECTOR'} .= "<td align=\"left\" valign=\"top\" class=\"listtopic\" width=\"20%\" onclick=\"sortmsg('Date');\" style=\"cursor: pointer;\">Date&nbsp;&nbsp;&loz;</td>";
    } else {
      $DATA{'SORT_SELECTOR'} .= "<td align=\"left\" valign=\"top\" class=\"listtopic\" width=\"20%\" onclick=\"sortmsg('Date');\" style=\"cursor: pointer;\">Date</td>";
    }
    if ($sortBy == "Subject") {
      $DATA{'SORT_SELECTOR'} .= "<td align=\"left\" valign=\"top\" class=\"listtopic\" width=\"40%\" onclick=\"sortmsg('Subject');\" style=\"cursor: pointer;\">Subject&nbsp;&nbsp;&loz;</td>";
    } else {
      $DATA{'SORT_SELECTOR'} .= "<td align=\"left\" valign=\"top\" class=\"listtopic\" width=\"40%\" onclick=\"sortmsg('Subject');\" style=\"cursor: pointer;\">Subject</td>";
    }
    if ($sortBy == "From") {
      $DATA{'SORT_SELECTOR'} .= "<td align=\"left\" valign=\"top\" class=\"listtopic\" width=\"25%\" onclick=\"sortmsg('From');\" style=\"cursor: pointer;\">From&nbsp;&nbsp;&loz;</td>";
    } else {
      $DATA{'SORT_SELECTOR'} .= "<td align=\"left\" valign=\"top\" class=\"listtopic\" width=\"25%\" onclick=\"sortmsg('From');\" style=\"cursor: pointer;\">From</td>";
    }

    $DATA{'SORT_SELECTOR'} .=  "\n        </tr>";

    if (isset($qPerPage) && $qPerPage > 0) {
      $CONFIG['QUARANTINE_PER_PAGE'] = $qPerPage;
    }

    if (isset($currentPage) && isset($CONFIG['QUARANTINE_PER_PAGE'])) {
      $pages = ceil( (count ($headings) / $CONFIG['QUARANTINE_PER_PAGE']) );
      $begin = (($currentPage - 1) * $CONFIG['QUARANTINE_PER_PAGE']);
      $ranges = ceil ($pages / $CONFIG['QNAV_BUTTONS']);

      /* Now lets just keep the information that we really need. */
      $headings = array_splice ($headings, $begin, $CONFIG['QUARANTINE_PER_PAGE']);
    }

    $rowclass = "qrowEven";
    foreach ($headings as $row) {
      $rating = sprintf("%3.0f%%", $row['rating'] * 100.0);
      if ($row['rating'] > 0.8) {
        $markclass = "high";
      } else {
        if ($row['rating'] < 0.7) {
          $markclass = "low";
        } else {
          $markclass = "medium";
        }
      }

      $PAIRS = array();

      $PAIRS['signatureID'] = $row['X-DSPAM-Signature'];
      $PAIRS['command']  = "viewMessage";
      $PAIRS['user'] = $CURRENT_USER;
      $PAIRS['page'] = $currentPage;
      $PAIRS['sortby'] = $sortBy;
      $PAIRS['qperpage'] = $CONFIG['QUARANTINE_PER_PAGE'];
      // $PAIRS['template'] = "quarantine";

      $url = SafeVars($PAIRS);
      $sender = htmlentities ($row['From']);
      $rsubject = htmlentities ($row['Subject']);

      if ($row['alert']) {
        $outclass = "qrowAlert";
      } else {
        $outclass = $rowclass;
      }

      $ptfields = preg_split('/\s+/', $row['X-DSPAM-Processed']);
      $times = preg_split('/\:/', $ptfields[3]);
      $ptime = "";
      if($CONFIG["DATE_FORMAT"]) {
        $month = array();
        $month['Jan'] = 0;
        $month['Feb'] = 1;
        $month['Mar'] = 2;
        $month['Apr'] = 3;
        $month['May'] = 4;
        $month['Jun'] = 5;
        $month['Jul'] = 6;
        $month['Aug'] = 7;
        $month['Sep'] = 8;
        $month['Oct'] = 9;
        $month['Nov'] = 10;
        $month['Dec'] = 11;
        $ptime = strftime($CONFIG["DATE_FORMAT"],
                          mktime($times[2],
                                 $times[1],
                                 $times[0],
                                 $ptfields[2],
                                $month[$ptfields[1]],
                                $ptfields[4] - 1900));
      } else {
        $mer = "a";
        if ($times[0] > 12) { $times[0] -= 12; $mer = "p"; }
        if ($times[0] == 0) { $times[0] = "12"; }
        $ptime = "{$ptfields[1]} {$ptfields[2]} {$times[0]}:{$times[1]}{$mer}";
      }

  /*

          <tr>
            <td align="left" valign="top" class="vncell">
              <input type="checkbox" class="formfld" title="check" alt="check" name="chkmsg" id="" />
            </td>
            <td align="left" valign="top" class="vncell">
              <span style="color: darkblue; font-weight: bold;">50%</span>
            </td>
            <td align="left" valign="top" class="vncell">Apr 1 05:59a</td>
            <td align="left" valign="top" class="vncell">Mar 30 11:08a</td>
            <td align="left" valign="top" class="vncell">
              <u>Last chance to register for Frankfurt, 4 April BEA...</u>
            </td>
          </tr>

          <tr>
            <td align="left" valign="top" class="{$outclass}" nowrap="true">
              <input type="checkbox" class="formfld" title="check" alt="check" name="{$row['X-DSPAM-Signature']}" id="{$row['X-DSPAM-Signature']}" />
            </td>
            <td align="left" valign="top" class="{$outclass} {$markclass}" nowrap="true">
              {$rating}
            </td>
            <td align="left" valign="top" class="{$outclass}" nowrap="true">{$ptime}</td>
            <td align="left" valign="top" class="{$outclass}" nowrap="true">{$row['From']}</td>
            <td align="left" valign="top" class="{$outclass}" nowrap="true">
              <a href="{$CONFIG['ME']}?{$url}">{$row['Subject']}</a>
            </td>
          </tr>
  */

      $DATA['QUARANTINE'] .= <<<EOD
          <tr>
            <td align="left" valign="top" class="{$outclass}" nowrap="true">
              <input type="checkbox" class="formfld" title="check" alt="check" name="chkmsg-{$row['X-DSPAM-Signature']}" id="{$row['X-DSPAM-Signature']}" />
            </td>
            <td align="left" valign="top" class="{$outclass} {$markclass}" nowrap="true">
              {$rating}
            </td>
            <td align="left" valign="top" class="{$outclass}" nowrap="true">{$ptime}</td>
            <td align="left" valign="top" class="{$outclass}" nowrap="true">{$sender}</td>
            <td align="left" valign="top" class="{$outclass}" nowrap="true">
              <a href="/dspam-viewmsg.php?{$url}">{$rsubject}</a>
            </td>
          </tr>

EOD;

      if ($rowclass == "qrowEven") {
        $rowclass = "qrowOdd";
      } else {
        $rowclass = "qrowEven";
      }
    } // end foreach ($headings as $row)

    /* prepare quarantine navbar */
    if (($currentPage - 1) >= 1) { $previousPage = $currentPage - 1; }
    else { $previousPage = 1; }

    if (($currentPage + 1) <= $pages) { $nextPage = $currentPage + 1; }
    else { $nextPage = $pages; }

    $quarantineFooterBegin = <<<EOD
          <tr>
            <td align="left" valign="top" class="listtopic">
              <input type="checkbox" class="formfld" title="check all" alt="check all" name="checkall" id="checkall" onClick="checkallmsgs();" />
              <label for="checkall">All</label>
            </td>
            <td align="right" valign="middle" class="listtopic" colspan="4">
              <span class="qnavbtn" onmouseover="this.style.backgroundColor='#A5B9E1;'" onmouseout="this.style.backgroundColor='#507DCD;'" title="First Page (1)"><a href="/dspam-quarantine.php?page=1&qperpage={$CONFIG['QUARANTINE_PER_PAGE']}" title="First Page (1)">|&lt;</a></span>&nbsp;
              <span class="qnavbtn" onmouseover="this.style.backgroundColor='#A5B9E1;'" onmouseout="this.style.backgroundColor='#507DCD;'" title="Page {$previousPage}"><a href="/dspam-quarantine.php?page={$previousPage}&qperpage={$CONFIG['QUARANTINE_PER_PAGE']}" title="Page {$previousPage}">&lt;</a></span>&nbsp;

EOD;

    $quarantineFooterEnd = <<<EOD
              <span class="qnavbtn" onmouseover="this.style.backgroundColor='#A5B9E1;'" onmouseout="this.style.backgroundColor='#507DCD;'" title="Page {$nextPage}"><a href="/dspam-quarantine.php?page={$nextPage}&qperpage={$CONFIG['QUARANTINE_PER_PAGE']}" title="Page {$nextPage}">&gt;</a></span>&nbsp;
              <span class="qnavbtn" onmouseover="this.style.backgroundColor='#A5B9E1;'" onmouseout="this.style.backgroundColor='#507DCD;'" title="Last Page ({$pages})"><a href="/dspam-quarantine.php?page={$pages}&qperpage={$CONFIG['QUARANTINE_PER_PAGE']}" title="Last Page ({$pages})">&gt;|</a></span>
            </td>
          </tr>

EOD;

    $ranges_array = array();
    $rpages = $pages;
    for ($i = 0; $i < $ranges; $i++) {
      $range = array();
      $range['start'] = (($i + 1)* $CONFIG['QNAV_BUTTONS']) - ($CONFIG['QNAV_BUTTONS'] - 1);

      if (($i + 1) == $ranges) {
        $range['end'] = ($range['start'] + $rpages) - 1;
      } else {
        $range['end'] = (($i + 1)* $CONFIG['QNAV_BUTTONS']);
        $rpages -= $CONFIG['QNAV_BUTTONS'];
      }

      $ranges_array[$i] = $range;
    }

    /* generate nav buttons */
    foreach($ranges_array as $range){
      if ($currentPage >= $range['start'] && $currentPage <= $range['end']) {
        for ($i = $range['start']; ; $i++) {
          if ($i > $range['end']) {
            break;
          } else {
            if ($i == $currentPage) {
              $quarantineFooter .= "<span class=\"qnavbtnhl\">{$i}</span>&nbsp\n";
            } else {
              $quarantineFooter .= "<span class=\"qnavbtn\" onmouseover=\"this.style.backgroundColor='#A5B9E1;'\" onmouseout=\"this.style.backgroundColor='#507DCD;'\" title=\"Page {$i}\"><a href=\"/dspam-quarantine.php?page={$i}&qperpage={$CONFIG['QUARANTINE_PER_PAGE']}\" title=\"Page {$i}\">{$i}</a></span>&nbsp\n";
            }
          }
        }
      }
    }

    $DATA['QUARANTINE_FOOTER'] = $quarantineFooterBegin . $quarantineFooter .$quarantineFooterEnd;
    $DATA['QPAGES'] = $pages;
    $DATA['QPAGE'] = $currentPage;
  } else {
    $input_errors[] = "Unable to open DSPAM quarantine mailbox at �{$USER}.mbox�. " .
                      "If you are a DSPAM admin user you can savely " .
                      "ignore this error because such users usually do not " .
                      "have a DSPAM mailbox/quarantine.";
  }

  return $input_errors;
}

/* ========================================================================== */
/* = P E R F O R M A N C E   F U N C T I O N S                              = */
/* ========================================================================== */

function ResetStats() {
  global $USER;

  $fd = fopen("{$USER}.stats", "r");
  $ts = chop(fgets($fd, 4096));
  $group = chop(fgets($fd, 4096));
  fclose($fd);
  list($ts, $ti, $tm, $fp, $sc, $ic) = split(",", $ts);

  if ($group <> "") {
    $GROUP = GetPath($group) . ".stats";
    $fd = fopen("{$GROUP}", "r");
    $gts = chop(fgets($fd, 4096));
    fclose($fd);
    list ($gts, $gti, $gtm, $gfp, $gsc, $gic) = split(",", $gts);
    $ts       -= $gts;
    $ti       -= $gti;
    $tm       -= $gtm;
    $fp       -= $gfp;
    $sc       -= $gsc;
    $ic       -= $gic;
  }

  $fd = fopen("{$USER}.rstats", "w");
  fputs($fd, "{$ts}" . "," . "{$ti}" . "," . "{$tm}" . "," .
             "{$fp}" . "," . "{$sc}" . "," . "{$ic}\n");
  fclose($fd);
}

function Tweak() {
  global $USER;

  $fd = fopen("{$USER}.rstats", "r");
  $ts = chop(fgets($fd, 4096));
  $group = chop($fgets($fd, 4096));
  fclose($fd);
  list($ts, $ti, $tm, $fp, $sc, $ic) = split(",", $ts);
  $tm++;

  $fd = fopen("{$USER}.rstats", "w");
  fputs($fd, "{$ts},{$ti},{$tm},{$fp},{$sc},{$ic}\n");
  fclose($fd);
}

function &DisplayIndex() {
  global $USER, $CONFIG, $DATA, $CURRENT_STORE, $CURRENT_USER;

  if (strpos ($CURRENT_USER, "@") === false) {
    if (GetDomain($CURRENT_STORE) <> "")
      $domain = GetDomain($CURRENT_STORE);
    else
      $domain = $config['system']['domain'];

    $spamalias = "spam-{$CURRENT_USER}@{$domain}";
  } else {
    $spamalias = "spam-{$CURRENT_USER}";
  }

  if ($handle = @fopen ("{$USER}.stats", "r")) {
    $spam .= chop(fgets($handle, 4096));
    $group .= chop(fgets($handle, 4096));
    fclose($handle);
    list($spam, $innocent, $misses, $fp, $sc, $ic) = split(",", $spam);

    if ($group <> "") {
      $GROUP = GetPath($group) . ".stats";
      $fd = fopen("{$GROUP}", "r");
      $gspam = chop(fgets($fd, 4096));
      fclose($fd);

      list($gspam, $ginnocent, $gfp, $gmisses, $gsc, $gic) = preg_split('/\,/', $gspam);
      $spam     -= $gspam;
      $innocent -= $ginnocent;
      $misses   -= $gmisses;
      $fp       -= $gfp;
      $sc       -= $gsc;
      $ic       -= $gic;
    }

    if ($spam + $innocent > 0) {
      $ratio = sprintf("%2.3f",
                       (($spam+$misses)/($spam+$misses+$fp+$innocent)*100));
    } else {
      $ratio = 0;
    }

    if (file_exists("{$USER}.rstats")) {
      $handle = fopen ("{$USER}.rstats", "r");
      $buffer = chop(fgets($handle, 4096));

      fclose ($handle);

      list($rts, $rti, $rtm, $rfp) = split(",", $buffer);

      $real_missed = $misses - $rtm;
      $real_caught = $spam - $rts;
      $real_fp = $fp - $rfp;

      if ($real_fp < 0) { $real_fp = 0; }

      $real_innocent = $innocent - $rti;

      if (($spam - $rts > 0) && ($spam - $rts + $misses - $rtm != 0) &&
          ($real_caught + $real_missed > 0) && ($real_fp + $real_innocent > 0)) {
        $monthly = sprintf("%2.3f",
                           (100.0-(($real_missed)/($real_caught+$real_missed))*100.0));
        $overall = sprintf("%2.3f",
                           (100-((($real_missed+$real_fp) /
                           ($real_fp+$real_innocent+$real_caught+$real_missed))*100)));
      } else {
        if ($real_caught == 0 && $real_missed > 0) {
          $monthly = 0;
          $overall = 0;
        } else {
          $monthly = 100;
          $overall = 100;
        }
      }

      if ($real_fp + $real_innocent > 0) {
        $fpratio = sprintf("%2.3f", ($real_fp/($real_fp+$real_innocent)*100));
      } else {
        $fpratio = 0;
      }

    } else {
      $rts = $spam + $misses;
      $rti = $innocent;
      $rtm = $misses;
      $rfp = $fp;

      $handle = fopen ("{$USER}.rstats", "w");
      fwrite("{$rts},{$rti},{$rtm},{$rfp}\n");
      fclose($handle);

      $monthly = "N/A";
      $fpratio = "N/A";
      $overall = "N/A";
    }

    $DATA['TIME'] = $time;
    $DATA['TOTAL_SPAM_SCANNED'] = $spam;
    $DATA['TOTAL_SPAM_LEARNED'] = $misses;
    $DATA['TOTAL_NONSPAM_SCANNED'] = $innocent;
    $DATA['TOTAL_NONSPAM_LEARNED'] = $fp;
    $DATA['SPAM_RATIO'] = $ratio;
    $DATA['SPAM_ACCURACY'] = $monthly;
    $DATA['NONSPAM_ERROR_RATE'] = $fpratio;
    $DATA['OVERALL_ACCURACY'] = $overall;
    $DATA['TOTAL_SPAM_CORPUSFED'] = $sc;
    $DATA['TOTAL_NONSPAM_CORPUSFED'] = $ic;
    $DATA['TOTAL_SPAM_MISSED'] = $real_missed;
    $DATA['TOTAL_SPAM_CAUGHT'] = $real_caught;
    $DATA['TOTAL_NONSPAM_MISSED'] = $real_fp;
    $DATA['TOTAL_NONSPAM_CAUGHT'] = $real_innocent;
    $DATA['SPAM_ALIAS'] = $spamalias;

    $DATA['LOCAL_DOMAIN'] = $CONFIG['LOCAL_DOMAIN'];
  } else {
    $DATA['SPAM_ACCURACY'] = "N/A";
    $DATA['NONSPAM_ERROR_RATE'] = "N/A";
    $DATA['OVERALL_ACCURACY'] = "N/A";
    $DATA['SPAM_RATIO'] = "N/A";
    $DATA['TOTAL_SPAM_MISSED'] = 0;
    $DATA['TOTAL_SPAM_CAUGHT'] = 0;
    $DATA['SPAM_RATIO'] = "N/A";
    $DATA['TOTAL_NONSPAM_MISSED'] = 0;
    $DATA['TOTAL_NONSPAM_CAUGHT'] = 0;
    $DATA['NONSPAM_ERROR_RATE'] = "N/A";
    $DATA['TOTAL_SPAM_LEARNED'] = 0;
    $DATA['TOTAL_SPAM_SCANNED'] = 0;
    $DATA['TOTAL_NONSPAM_LEARNED'] = 0;
    $DATA['TOTAL_NONSPAM_SCANNED'] = 0;
    $DATA['TOTAL_SPAM_CORPUSFED'] = 0;
    $DATA['TOTAL_NONSPAM_CORPUSFED'] = 0;
    $DATA['SPAM_ALIAS'] = $spamalias;

    $input_errors[] = "Unable to open DSPAM stats at �{$USER}.stats�. " .
                      "If you are a DSPAM admin user you can savely " .
                      "ignore this error because such users usually do not " .
                      "have a DSPAM mailbox/quarantine.";
  }

  return $input_errors;
}

function getJScriptFunction($whichOne = 0) {
        $changeuser_msg = gettext("Do you realy want to change the current user?") . "\\n" .
                          gettext("This requires a logout followed by a login.");

        switch ($whichOne) {
        case 0:
                $scriptstr = '

                /* applicable for almost any dspam related page */
                function changeuser() {
                        check = confirm("' . $changeuser_msg . '");

                        if (check == true)
                        window.location.href = "/index.php?logout=true";
                }
                ';

                break;
        case 1:
                $scriptstr = '

                /* applicable for dspam-quarantine.php */
                function checkallmsgs(enable) {
                        var endis = (document.iform.checkall.checked || enable);
                        var elem = document.iform.elements.length;

                        for (i = 0; i < elem; i++) {
                                if (document.iform.elements[i].name.indexOf("chkmsg") >= 0) {
                                        document.iform.elements[i].checked = endis;
                                }
                        }
                }
                ';

                break;
        case 2:
                $scriptstr = '

                /* applicable for dspam-quarantine.php */
                function sortmsg(criterion) {
                        var baseURL = "/dspam-quarantine.php?page=" + document.iform.qpage.value;
                        var qperpage = document.getElementsByName("qperpage")[0].value;

                        window.location.href = baseURL + "&qperpage=" + qperpage + "&sortby=" + criterion;
                }
                ';

                break;
        case 3:
                $scriptstr = '

                /* applicable for dspam-quarantine.php */
                function processmsg(what) {
                        var elem = document.iform.elements.length;
                        var checked = false;

                        switch (what) {
                        case 0:
                        for (i = 0; i < elem; i++) {
                          if (document.iform.elements[i].name.indexOf("chkmsg") >= 0 &&
                              document.iform.elements[i].checked == true) {
                            checked = true;
                            break;
                          }
                        }

                        if (checked) {
                          document.iform.processAction.value = "manyNotSpam";
                        } else {
                          alert("You did not select any message that should be processed as not beeing Spam.");
                          return false;
                        }
                        break;
                        case 1:
                        for (i = 0; i < elem; i++) {
                          if (document.iform.elements[i].name.indexOf("chkmsg") >= 0 &&
                              document.iform.elements[i].checked == true) {
                            checked = true;
                            break;
                          }
                        }

                        if (checked) {
                          if (confirm("Are you sure you want to delete SELECTED messages in quarantine?") == false) {
                            return false;
                          } else {
                            document.iform.processAction.value = "manySpam";
                          }
                        } else {
                          alert("You did not select any message that should be processed as beeing Spam.");
                          return false;
                        }
                        break;
                        case 2:
                        if (confirm("Are you sure you want to delete ALL messages in quarantine?") == false) {
                         return false;
                        } else {
                          document.iform.processAction.value = "deleteAll";
                        }
                        break;
                        }

                        document.iform.submit();
                }
                ';

                break;
        case 4:
                $scriptstr = '

                /* applicable for dspam-quarantine.php nad dspam-history.php */
                function changeQPerPage(originator) {
                        var elementName = "";
                        var baseURL = "";

                        if (originator.name == "qperpage") {
                                baseURL = "/dspam-quarantine.php?page=" + document.iform.qpage.value;
                                elementName = "qperpage";
                        } else {
                                baseURL = "/dspam-history.php?page=" + document.iform.hpage.value;
                                elementName = "hperpage";
                        }

                        for (var i = 0; i < 2; i++) {
                                document.getElementsByName(elementName)[i].value = originator.value;
                        }

                        if (originator.name == "qperpage")
                                window.location.href = baseURL + "&qperpage=" + originator.value;
                        else
                                window.location.href = baseURL + "&hperpage=" + originator.value;
                }
                ';

                break;
        case 5:
                $scriptstr = '

                /* applicable for dspam-settings.php */

                function fadeTableRow(rowid, fadeType, opts){
                    if(!opts){
                        opts = {};
                    }

                    var row  = $(rowid);
                    var cells= row.childNodes;
                    for(i=0;i<cells.length;i++){
                        if(cells[i].tagName == "TD"){
                            if (fadeType == 0)
                                new Effect.Fade(cells[i],opts);
                            else
                                new Effect.Appear(cells[i],opts);
                        }
                    }
                    if (fadeType == 0)
                        new Effect.Fade(row,opts);
                    else
                        new Effect.Appear(row,opts);
                }

                function toggleDSPAMDomain(enable_over, originator) {
                        var endis = !(originator.checked || enable_over);

                        if (endis) {
                          fadeTableRow("emailnotitb", 1);
                        } else {
                          fadeTableRow("emailnotitb", 0);
                        }
                }

                function toggleDBSettings(idx) {
                        if (idx)
                                idx = idx;
                        else
                                idx = document.iform.sdriver.selectedIndex;

                        switch (idx) {
                        case 0:
                          fadeTableRow("DBmysql", 1);
                          fadeTableRow("DBsqlite", 0);
                          fadeTableRow("DBbdb", 0);
                          fadeTableRow("DBpgsql", 0);
                          fadeTableRow("DBoracle", 0);
                          fadeTableRow("DBhash", 0);
                          break;
                        case 1:
                          fadeTableRow("DBmysql", 0);
                          fadeTableRow("DBsqlite", 1);
                          fadeTableRow("DBbdb", 0);
                          fadeTableRow("DBpgsql", 0);
                          fadeTableRow("DBoracle", 0);
                          fadeTableRow("DBhash", 0);
                          break;
                        case 2:
                          fadeTableRow("DBmysql", 0);
                          fadeTableRow("DBsqlite", 0);
                          fadeTableRow("DBbdb", 1);
                          fadeTableRow("DBpgsql", 0);
                          fadeTableRow("DBoracle", 0);
                          fadeTableRow("DBhash", 0);
                          break;
                        case 3:
                          fadeTableRow("DBmysql", 0);
                          fadeTableRow("DBsqlite", 0);
                          fadeTableRow("DBbdb", 0);
                          fadeTableRow("DBpgsql", 1);
                          fadeTableRow("DBoracle", 0);
                          fadeTableRow("DBhash", 0);
                          break;
                        case 4:
                          fadeTableRow("DBmysql", 0);
                          fadeTableRow("DBsqlite", 0);
                          fadeTableRow("DBbdb", 0);
                          fadeTableRow("DBpgsql", 0);
                          fadeTableRow("DBoracle", 1);
                          fadeTableRow("DBhash", 0);
                          break;
                        case 5:
                          fadeTableRow("DBmysql", 0);
                          fadeTableRow("DBsqlite", 0);
                          fadeTableRow("DBbdb", 0);
                          fadeTableRow("DBpgsql", 0);
                          fadeTableRow("DBoracle", 0);
                          fadeTableRow("DBhash", 1);
                          break;
                        }
                }

                function enable_change(enable_over, originator) {
                        var endis;

                        switch (originator) {
                        case 0:
                                endis = !(document.iform.enabledbg.checked || enable_over);
                                endis ? document.iform.debug.style.backgroundColor = "#D4D0C8" : document.iform.debug.style.backgroundColor = "#FFFFFF";
                                document.iform.debug.disabled = endis;
                                endis ? document.iform.dopt.style.backgroundColor = "#D4D0C8" : document.iform.dopt.style.backgroundColor = "#FFFFFF";
                                document.iform.dopt.disabled = endis;
                        case 1:
                                endis = !(document.iform.enableldap.checked || enable_over);
                                document.iform.ldapmode.disabled = endis;
                                endis ? document.iform.ldaphost.style.backgroundColor = "#D4D0C8" : document.iform.ldaphost.style.backgroundColor = "#FFFFFF";
                                document.iform.ldaphost.disabled = endis;
                                endis ? document.iform.ldapfilter.style.backgroundColor = "#D4D0C8" : document.iform.ldapfilter.style.backgroundColor = "#FFFFFF";
                                document.iform.ldapfilter.disabled = endis;
                                endis ? document.iform.ldapbase.style.backgroundColor = "#D4D0C8" : document.iform.ldapbase.style.backgroundColor = "#FFFFFF";
                                document.iform.ldapbase.disabled = endis;
                        case 2:
                                endis = !(document.iform.enablesbl.checked || enable_over);
                                endis ? document.iform.sblhost.style.backgroundColor = "#D4D0C8" : document.iform.sblhost.style.backgroundColor = "#FFFFFF";
                                document.iform.sblhost.disabled = endis;
                        case 3:
                                endis = !(document.iform.enableclam.checked || enable_over);
                                endis ? document.iform.clamport.style.backgroundColor = "#D4D0C8" : document.iform.clamport.style.backgroundColor = "#FFFFFF";
                                document.iform.clamport.disabled = endis;
                                endis ? document.iform.clamhost.style.backgroundColor = "#D4D0C8" : document.iform.clamhost.style.backgroundColor = "#FFFFFF";
                                document.iform.clamhost.disabled = endis;
                                document.iform.clamresp.disabled = endis;
                        case 4:
                                endis = !(document.iform.enabledsclient.checked || enable_over);
                                endis ? document.iform.dsclhost.style.backgroundColor = "#D4D0C8" : document.iform.dsclhost.style.backgroundColor = "#FFFFFF";
                                document.iform.dsclhost.disabled = endis;
                                endis ? document.iform.dsclport.style.backgroundColor = "#D4D0C8" : document.iform.dsclport.style.backgroundColor = "#FFFFFF";
                                document.iform.dsclport.disabled = endis;
                                endis ? document.iform.dsclident.style.backgroundColor = "#D4D0C8" : document.iform.dsclident.style.backgroundColor = "#FFFFFF";
                                document.iform.dsclident.disabled = endis;
                        case 5:
                                endis = !(document.iform.tcpipdel.checked || enable_over);
                                endis ? document.iform.dhost.style.backgroundColor = "#D4D0C8" : document.iform.dhost.style.backgroundColor = "#FFFFFF";
                                document.iform.dhost.disabled = endis;
                                endis ? document.iform.dport.style.backgroundColor = "#D4D0C8" : document.iform.dport.style.backgroundColor = "#FFFFFF";
                                document.iform.dport.disabled = endis;
                                endis ? document.iform.dident.style.backgroundColor = "#D4D0C8" : document.iform.dident.style.backgroundColor = "#FFFFFF";
                                document.iform.dident.disabled = endis;
                                document.iform.delproto.disabled = endis;
                        case 6:
                                endis = !(document.iform.enablenoti.checked || enable_over);
                                document.iform.whichdomain.disabled = endis;
                                endis ? document.iform.dspamdomain.style.backgroundColor = "#D4D0C8" : document.iform.dspamdomain.style.backgroundColor = "#FFFFFF";
                                document.iform.dspamdomain.disabled = endis;
                                endis ? document.iform.dspamcontact.style.backgroundColor = "#D4D0C8" : document.iform.dspamcontact.style.backgroundColor = "#FFFFFF";
                                document.iform.dspamcontact.disabled = endis;
                        }
                }
                ';

                break;
        case 6:
                $scriptstr = '

                function checkDisabledState(form) {
                  for (i = 0; i < form.elements.length; i++) {
                    if (form.elements[i].disabled && form.elements[i].type == "text")
                      form.elements[i].style.backgroundColor = "#D4D0C8";
                    else if (form.elements[i].type == "text")
                      form.elements[i].style.backgroundColor = "#FFFFFF";
                  }
                }
                ';

                break;
        }

        return $scriptstr;
}

?>
