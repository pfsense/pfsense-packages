<?php
/* $Id$ */
/* Copyright (C) 2006 Daniel S. Haischt */
require_once("functions.inc");
require_once("dspam.inc");

function &GetPrefs($user) {
  global $CONFIG, $USER;
  $prefs = array();
  $FILE = "{$USER}.prefs";

  if ($CONFIG['PREFERENCES_EXTENSION'] == 1) {
    $handle = popen ("{$CONFIG['DSPAM_BIN']}/dspam_admin agg pref " . quotemeta($user));

    while (!feof($handle)) {
      $buffer = chop(fgets($handle, 4096));
      list($key, $value) = split("=", $buffer);
      $prefs[$key] = $value;
    }

    pclose($handle);

  }

  if (count(array_keys($prefs)) == 0 || $CONFIG['PREFERENCES_EXTENSION'] != 1) {

    /* This step is only required if the user
     * wants to use the legacy DSPAm CGI app.
     */
    if (! file_exists("{$CONFIG['DSPAM_WWW']}/default.prefs")) {
      /* try to copy the sample file */
      if (file_exists("{$CONFIG['DSPAM_WWW']}/default.prefs.sample")) {
       copy("{$CONFIG['DSPAM_WWW']}/default.prefs.sample",
            "{$CONFIG['DSPAM_WWW']}/default.prefs");
      }
    }
    $handle = fopen ("{$CONFIG['DSPAM_WWW']}/default.prefs", "r");

    while (!feof($handle)) {
      $buffer = chop(fgets($handle, 4096));
      list($key, $value) = split("=", $buffer);
      $prefs[$key] = $value;
    }

    fclose($handle);

    if(file_exists($FILE)) {
      $handle = fopen ($FILE, "r");

      while (!feof($handle)) {
        $buffer = chop(fgets($handle, 4096));
        list($key, $value) = split("=", $buffer);
        $prefs[$key] = $value;
      }
    }
  }
  return $prefs;
}

function CheckQuarantine() {
  global $MAILBOX, $DATA;
  $f = 0;

  if (file_exists($MAILBOX)) {
    $handle = fopen($MAILBOX, "r");

    while (!feof($handle)) {
        $buffer = fgets($handle, 4096);
        if (preg_match('/^From /', $buffer) <= 0) {continue;}
        $f++;
    }

    fclose ($handle);
  }

  if ($f == 0) {
    $f = "Empty";
  }

  $DATA['TOTAL_QUARANTINED_MESSAGES'] = $f;
}

function http_parse_query( $array = NULL, $convention = '%s' ) {
  if( count( $array ) == 0 ) {
    return '';
  } else {
    if( function_exists( 'http_build_query' ) ){
      $query = http_build_query( $array );
    } else {
      $query = '';

      foreach( $array as $key => $value ) {
        if( is_array( $value ) ){
          $new_convention = sprintf( $convention, $key ) . '[%s]';
          $query .= http_parse_query( $value, $new_convention );
        } else {
          $key = urlencode( $key );
          $value = urlencode( $value );
          $query .= sprintf( $convention, $key ) . "=$value&";
        }
      }
    }

    return $query;
  }
}

/* just a wrapper function */
function SafeVars($PAIRS) {
  $url = http_parse_query($PAIRS);
  return $url;
}

function To12Hour($h) {
  if ($h < 0) { $h += 24; }
  if ($h > 11) { if ($h > 12) { $h -= 12; } $h .= "p"; }
  else { if ($h == 0) { $h = "12"; } $h .= "a"; }
  return $h;
}

function GetPath($store) {
  global $CONFIG, $USER;
  $PATH = "";

  /* Domain-scalen */
  if ($CONFIG['DOMAIN_SCALE'] == 1) {
    $splittmp = (split('@', $store));
    $VPOPUSERNAME = $splittmp[0];
    $VPOPDOMAIN = $splittmp[1];
    if ($VPOPDOMAIN == "") {$VPOPDOMAIN = "local";}

    $PATH = "{$CONFIG['DSPAM_HOME']}/data/{$VPOPDOMAIN}/{$VPOPUSERNAME}/" .
            "{$VPOPUSERNAME}";
    return $PATH;

  /* Normal scale */
  } else if ($CONFIG['LARGE_SCALE'] == 0) {
    $PATH = "{$CONFIG['DSPAM_HOME']}/data/{$USER}/{$USER}";
    return $PATH;

  /* Large-scale */
  } else {
    if (strlen($USER) > 1) {
      $PATH = "{$CONFIG['DSPAM_HOME']}/data/" . substr($USER, 0, 1) .
        "/". substr($USER, 1, 1) . "/{$USER}/{$USER}";
    } else {
      $PATH = "{$CONFIG['DSPAM_HOME']}/data/{$USER}/{$USER}";
    }
    return $PATH;
  }
}

function GetUserDir($store) {
  global $CONFIG, $USER;
  $PATH = "";

  /* Domain-scalen */
  if ($CONFIG['DOMAIN_SCALE'] == 1) {
    $splittmp = (split('@', $store));
    $VPOPUSERNAME = $splittmp[0];
    $VPOPDOMAIN = $splittmp[1];
    if ($VPOPDOMAIN == "") {$VPOPDOMAIN = "local";}

    $PATH = "{$CONFIG['DSPAM_HOME']}/data/{$VPOPDOMAIN}/{$VPOPUSERNAME}";
    return $PATH;

  /* Normal scale */
  } else if ($CONFIG['LARGE_SCALE'] == 0) {
    $PATH = "{$CONFIG['DSPAM_HOME']}/data/{$USER}/";
    return $PATH;

  /* Large-scale */
  } else {
    if (strlen($USER) > 1) {
      $PATH = "{$CONFIG['DSPAM_HOME']}/data/" . substr($USER, 0, 1) .
        "/". substr($USER, 1, 1) . "/{$USER}/{$USER}";
    } else {
      $PATH = "{$CONFIG['DSPAM_HOME']}/data/{$USER}";
    }
    return $PATH;
  }
}

function GetDomain($store) {
  global $CONFIG, $USER;
  $PATH = "";

  /* Domain-scalen */
  if ($CONFIG['DOMAIN_SCALE'] == 1) {
    $splittmp = (split('@', $store));
    $VPOPUSERNAME = $splittmp[0];
    $VPOPDOMAIN = $splittmp[1];
    if ($VPOPDOMAIN == "") {$VPOPDOMAIN = "local";}

    return $VPOPDOMAIN;
  }
}

function isDSPAMAdmin($username = "") {
  global $config, $CONFIG;

  $groupindex = index_groups();
  $userindex = index_users();

  if ($username == "") { return 0; }

  $gname = $config['system']['group'][$groupindex[$config['system']['user'][$userindex[$username]]['groupname']]]['name'];

  if (isset($gname)) {
    return ($gname === $CONFIG['DSPAM_ADMIN_GROUP']);
  }

  return 0;
}

function createUserNotificationMessages() {
        global $config, $CONFIG, $USER, $CURRENT_USER, $CURRENT_STORE;

        $firstrun = "{$USER}.firstrun";
        $firstspam = "{$USER}.firstspam";
        $quarantinefull = "{$USER}.quarantinefull";
        $savemsg = "";

        if (isset($config['installedpackages']['dspam']['config'][0]['dspam-domain']))
          $domain = $config['installedpackages']['dspam']['config'][0]['dspam-domain'];
        else
          $domain = $config['system']['domain'];

        if (GetDomain($CURRENT_STORE) <> "")
          $user_domain = GetDomain($CURRENT_STORE);
        else
          $user_domain = $config['system']['domain'];

        $userdir = GetUserDir($CURRENT_STORE);
        $hostname = $config['system']['hostname'];
        $support_user = $config['installedpackages']['dspam']['config'][0]['dspam-contact'];

        /* return if there are no sample files */
        if (! file_exists("{$CONFIG['DSPAM_HOME']}/firstrun.txt.sample") ||
            ! file_exists("{$CONFIG['DSPAM_HOME']}/firstspam.txt.sample") ||
            ! file_exists("{$CONFIG['DSPAM_HOME']}/quarantinefull.txt.sample") ||
            ! file_exists($userdir)) {
          return;
        }

        /* create firstrun.txt */
        if (! file_exists("{$firstrun}")) {
          $sample_msg = file_get_contents("{$CONFIG['DSPAM_HOME']}/firstrun.txt.sample");

          $sample_msg = str_replace("support", $support_user, $sample_msg);
          $sample_msg = str_replace("configureme.com", $user_domain, $sample_msg);
          $sample_msg = str_replace("http://www.yourdomain.com/dspam/",
                                    "http://{$hostname}.{$domain}/dspam.php",
                                    $sample_msg);

          @file_put_contents("{$firstrun}", $sample_msg);
          $savemsg .= " »{$firstrun}«, ";
        }

        /* create firstspam.txt */
        if (! file_exists("{$firstspam}")) {
          $sample_msg = file_get_contents("{$CONFIG['DSPAM_HOME']}/firstspam.txt.sample");

          $sample_msg = str_replace("support", $support_user, $sample_msg);
          $sample_msg = str_replace("configureme.com", $user_domain, $sample_msg);
          $sample_msg = str_replace("http://www.yourdomain.com/dspam/",
                                    "http://{$hostname}.{$domain}/dspam.php",
                                    $sample_msg);

          @file_put_contents("{$firstspam}", $sample_msg);
          $savemsg .= " »{$firstspam}«, ";
        }

        /* create quarantinefull.txt */
        if (! file_exists("{$quarantinefull}")) {
          $sample_msg = file_get_contents("{$CONFIG['DSPAM_HOME']}/quarantinefull.txt.sample");

          $sample_msg = str_replace("support", $support_user, $sample_msg);
          $sample_msg = str_replace("configureme.com", $user_domain, $sample_msg);
          $sample_msg = str_replace("http://www.yourdomain.com/dspam/",
                                    "http://{$hostname}.{$domain}/dspam.php",
                                    $sample_msg);

          @file_put_contents("{$quarantinefull}", $sample_msg);
          $savemsg .= " »{$quarantinefull}«.";
        }

        if ($savemsg <> "") {
          $savemsg = gettext("The following files were created: ") . $savemsg;
          return $savemsg;
        }

        return;
}

function createNotificationMessages() {
        global $config, $CONFIG, $USER, $CURRENT_USER;

        $firstrun = "{$CONFIG['DSPAM_HOME']}/firstrun.txt";
        $firstspam = "{$CONFIG['DSPAM_HOME']}/firstspam.txt";
        $quarantinefull = "{$CONFIG['DSPAM_HOME']}/quarantinefull.txt";
        $savemsg = "";

        if (isset($config['installedpackages']['dspam']['config'][0]['dspam-domain']))
          $domain = $config['installedpackages']['dspam']['config'][0]['dspam-domain'];
        else
          $domain = $config['system']['domain'];

        $hostname = $config['system']['hostname'];
        $support_user = $config['installedpackages']['dspam']['config'][0]['dspam-contact'];

        /* return if there are no sample files */
        if (! file_exists("{$CONFIG['DSPAM_HOME']}/firstrun.txt.sample") ||
            ! file_exists("{$CONFIG['DSPAM_HOME']}/firstspam.txt.sample") ||
            ! file_exists("{$CONFIG['DSPAM_HOME']}/quarantinefull.txt.sample")) {
          return;
        }

        /* create firstrun.txt */
        if (! file_exists("{$CONFIG['DSPAM_HOME']}/firstrun.txt")) {
          $sample_msg = file_get_contents("{$CONFIG['DSPAM_HOME']}/firstrun.txt.sample");

          $sample_msg = str_replace("support", $support_user, $sample_msg);
          $sample_msg = str_replace("configureme.com", $domain, $sample_msg);
          $sample_msg = str_replace("http://www.yourdomain.com/dspam/",
                                    "http://{$hostname}.{$domain}/dspam.php",
                                    $sample_msg);

          @file_put_contents("{$firstrun}", $sample_msg);
          $savemsg .= " »{$firstrun}«, ";
        }

        /* create firstspam.txt */
        if (! file_exists("{$CONFIG['DSPAM_HOME']}/firstspam.txt")) {
          $sample_msg = file_get_contents("{$CONFIG['DSPAM_HOME']}/firstspam.txt.sample");

          $sample_msg = str_replace("support", $support_user, $sample_msg);
          $sample_msg = str_replace("configureme.com", $domain, $sample_msg);
          $sample_msg = str_replace("http://www.yourdomain.com/dspam/",
                                    "http://{$hostname}.{$domain}/dspam.php",
                                    $sample_msg);

          @file_put_contents("{$firstspam}", $sample_msg);
          $savemsg .= " »{$firstspam}«, ";
        }

        /* create quarantinefull.txt */
        if (! file_exists("{$CONFIG['DSPAM_HOME']}/quarantinefull.txt")) {
          $sample_msg = file_get_contents("{$CONFIG['DSPAM_HOME']}/quarantinefull.txt.sample");

          $sample_msg = str_replace("support", $support_user, $sample_msg);
          $sample_msg = str_replace("configureme.com", $domain, $sample_msg);
          $sample_msg = str_replace("http://www.yourdomain.com/dspam/",
                                    "http://{$hostname}.{$domain}/dspam.php",
                                    $sample_msg);

          @file_put_contents("{$quarantinefull}", $sample_msg);
          $savemsg .= " »{$quarantinefull}«.";
        }

        if ($savemsg <> "") {
          $savemsg = gettext("The following files were created: ") . $savemsg;
          return $savemsg;
        }

        return;
}

function &check_dspam_installation() {
  global $config;

  /* create RC files if necessary */
  if (! file_exists("/usr/local/etc/rc.d")) {
    @mkdir("/usr/local/etc/rc.d");
  }
  if (! file_exists("/usr/local/etc/rc.d/000.mysql.sh")) {
    @copy("/usr/local/pkg/000.mysql.sh", "/usr/local/etc/rc.d/000.mysql.sh");
    @chmod("/usr/local/etc/rc.d/000.mysql.sh", 0755);
  }
  if (! file_exists("/usr/local/etc/rc.d/010.clamav-clamd.sh")) {
    @copy("/usr/local/pkg/010.clamav-clamd.sh",
          "/usr/local/etc/rc.d/010.clamav-clamd.sh");
    @chmod("/usr/local/etc/rc.d/010.clamav-clamd.sh", 0755);
  }
  if (! file_exists("/usr/local/etc/rc.d/020.clamav-freshclam.sh")) {
    @copy("/usr/local/pkg/020.clamav-freshclam.sh",
          "/usr/local/etc/rc.d/020.clamav-freshclam.sh");
    @chmod("/usr/local/etc/rc.d/020.clamav-freshclam.sh", 0755);
  }
  if (! file_exists("/usr/local/etc/rc.d/030.p3scan.sh")) {
    @copy("/usr/local/pkg/030.p3scan.sh",
          "/usr/local/etc/rc.d/030.p3scan.sh");
    @chmod("/usr/local/etc/rc.d/030.p3scan.sh", 0755);
  }

  /* create conf files for those packages DSPAM depends on */
  if (! file_exists("/usr/local/etc/clamd.conf")) {
    @copy("/usr/local/pkg/clamd.conf",
          "/usr/local/etc/clamd.conf");
  }
  if (! file_exists("/usr/local/etc/freshclam.conf")) {
    @copy("/usr/local/pkg/freshclam.conf",
          "/usr/local/etc/freshclam.conf");
  }

  if (! is_service_running("dspam")) {
    $input_errors[] = "The DSPAM daemon process is not running. " .
                      "If you are certain that you did configure DSPAM " .
                      "appropriatly, you can start the corresponding process " .
                      "using the service control panel which is part of the " .
                      "»Status« menu item.";
  }
  if (! is_service_running("p3scan")) {
    $input_errors[] = "The POP3 proxy process is not running. " .
                      "If you are certain that you did configure the POP3 proxy " .
                      "appropriatly, you can start the corresponding process " .
                      "using the service control panel which is part of the " .
                      "»Status« menu item.";
  }
  if (! is_service_running("clamd")) {
    $input_errors[] = "The ClamAV daemon process is not running. " .
                      "If you are certain that you did configure DSPAM " .
                      "appropriatly, you can start the corresponding process " .
                      "using the service control panel which is part of the " .
                      "»Status« menu item (ClamAV does not need any configuration).";
  }
  if (! is_service_running("freshclam")) {
    $input_errors[] = "The freshclam daemon process is not running. " .
                      "If you are certain that you did configure DSPAM " .
                      "appropriatly, you can start the corresponding process " .
                      "using the service control panel which is part of the " .
                      "»Status« menu item (freshclam does not need any configuration).";
  }
  if (! is_service_running("mysql")) {
    $input_errors[] = "The MySQL daemon process is not running. " .
                      "If you are certain that you did configure DSPAM " .
                      "appropriatly, you can start the corresponding process " .
                      "using the service control panel which is part of the " .
                      "»Status« menu item (MySQL does not need any configuration).";
  }

  if (empty($config['installedpackages']['dspam']['config']) ||
      empty($config['installedpackages']['dspam']['config'][0]['storage-driver']) ||
      (file_exists('/usr/local/etc/dspam.conf') == false)) {
      /* create default config files etc. */
    sync_package_dspam();

    $input_errors[] = "It seems that you are using DSPAM for the very first time. " .
                      "Please run the DSPAM wizard from the »Services« menu " .
                      "if you would like to configure DSPAM right now. " .
                      "If you are not eligible to administer DSPAM you may contact " .
                      "your local DSPAM admin personal to solve this issue.";
  }

  return $input_errors;
}

?>
