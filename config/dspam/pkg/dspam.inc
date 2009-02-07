<?php
/* $Id$ */
/* Copyright (C) 2006 Daniel S. Haischt */

/* include all configuration functions */
require_once ("functions.inc");
require_once ("dspam-config.inc");
require_once ("dspam-guifunc.inc");
require_once ("dspam-pkgfunc.inc");
require_once ("dspam-utilfunc.inc");

/* General TODO(s):
 *
 * - Use quotemeta where applicable
 * - Issue a change message after a successful GET/POST
 * - Issue an error message if something went wrong during a GET/POST
 * - Revise user management (CURENT_USER, SESSION_USER etc.)
 */

$DATA = array();

/*
 * Determine which extensions are available
 */
if ($CONFIG['AUTODETECT'] == 1 || $CONFIG['AUTODETECT'] == "") {
  $CONFIG['PREFERENCES_EXTENSION'] = 0;
  $CONFIG['LARGE_SCALE'] = 0;
  $CONFIG['DOMAIN_SCALE'] = 0;

  $buffer = "";
  $handle = popen ("/usr/local/bin/dspam --version", "r");

  while (!feof($handle)) {
    $buffer .= fgets($handle, 4096);
  }

  pclose($handle);

  if (strpos($buffer, '--enable-preferences-extension') !== false) {
    $CONFIG['PREFERENCES_EXTENSION'] = 1;
  }
  if (strpos($buffer, '--enable-large-scale') !== false) {
    $CONFIG['LARGE_SCALE'] = 1;
  }
  if (strpos($buffer, '--enable-domain-scale') !== false) {
    $CONFIG['DOMAIN_SCALE'] = 1;
  }
}

/*
 * Determine admin status
 */

$CONFIG['ADMIN'] = isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER']);

/*
 * Determine which user should be used to display DSPAm related data
 */
$CURRENT_USER = $HTTP_SERVER_VARS['AUTH_USER'];

if (empty($CONFIG['ADMIN']) &&
    strpos($_SERVER['SCRIPT_NAME'], "wizard.php") !== false) {

    $group = $config['system']['user'][$userindex[$HTTP_SERVER_VARS['AUTH_USER']]]['groupname'];
    $home = "/" . getGroupHomePage($group);
    if ($home == "/") { $home = "/index.php"; }

    if (! headers_sent()) {
        pfSenseHeader($home);
        exit;
    } else {
      $htmlstr = <<< EOD
<html>
  <head>
    <title>Unauthorized Access</title>
    <meta http-equiv="refresh" content="2; URL={$home}">
  </head>
  <body>
    <h3>Unauthorized Access - You will be redirected shortly!</h3>
  </body>
</html>
EOD;
      print $htmlstr;
      exit;
    }
}

/*
 * only DSPAM admins can change the current user without logging out
 * and logging in as another user again.
 */
if ($_POST) {
  if (! empty($_POST['username']) && $CONFIG['ADMIN'] == 1) {
    $CURRENT_USER = $_POST['username'];
  }
}
if ($_GET) {
  if (! empty($_GET['user']) && $CONFIG['ADMIN'] == 1) {
    $CURRENT_USER = $_GET['user'];
  }
}

if ($CURRENT_USER == "") {
  $input_errors[] = "System Error. I was unable to determine your identity.";
}

preg_replace('/%CURRENT_USER%/', $CURRENT_USER, $CONFIG['DSPAM_ARGS']);

/* current store */
$PREF =& GetPrefs($CURRENT_USER);
$CURRENT_STORE = $PREF['localStore'];
if ($CURRENT_STORE == "") {$CURRENT_STORE = $CURRENT_USER; }

$USER = GetPath($CURRENT_STORE);
$MAILBOX = $USER . ".mbox";
$TMPFILE = $USER . ".tmp";

/*
 * Set up initial display variables
 */
CheckQuarantine();
$DATA['REMOTE_USER'] = $CURRENT_USER;

/*
 * Check whether DSPAM is already configured.
 */
$install_errors =& check_dspam_installation();
$input_errors = array_merge($input_errors, $install_errors);

/*
 * Check whether we should generate notification messages
 */
if (isset($config['installedpackages']['dspam']['config'][0]['notification-email'])) {
  $tmpmsg = createNotificationMessages();
  if (! empty($tmpmsg))
    $tmpmsg .= "\n" . createUserNotificationMessages();
  else
    $tmpmsg = createUserNotificationMessages();

  $savemsg =& $tmpmsg;
}

/*
 * Process Commands
 */
if (isset($pgtitle) && is_array($pgtitle)) {
  /* Performance */
  if (basename($_SERVER['SCRIPT_NAME']) == "dspam-perf.php") {
    if ($_GET) {
      if ($GET['command'] == "resetStats") {
        ResetStats();
      } else if ($GET['command'] == "tweak") {
        Tweak();
      }
    }

    $display_errors =& DisplayIndex();
    $input_errors = array_merge($input_errors, $display_errors);
  }

  /* Preferences */
  else if (basename($_SERVER['SCRIPT_NAME']) == "dspam-prefs.php") {
    $savemsg = "";
    $prefs_errors =& DisplayPreferences(NULL, $savemsg);
    $input_errors = array_merge($input_errors, $prefs_errors);
  }

  /* Quarantine */
  else if (basename($_SERVER['SCRIPT_NAME']) == "dspam-quarantine.php") {
    if ($_GET) {
      if ($_GET['command'] == "viewMessage") {
        $showpart = 0;
        $ctype = 0;
        $sortby = "Rating";
        $currentPage = 1;
        $qPerPage = 0;

        if (isset($_GET['showpart'])) { $showpart = $_GET['showpart']; }
        if (isset($_GET['ctype'])) { $ctype = intval($_GET['ctype']); }
        if (isset($_GET['sortby'])) { $sortby = $_GET['sortby']; }
        if (isset($_GET['page'])) { $currentPage = $_GET['page']; }
        if (isset($_GET['qperpage'])) { $qPerPage = $_GET['qperpage']; }

        $quarantine_errors =& QuarantineViewMessage($_GET['signatureID'],
                                                    $showpart,
                                                    $ctype,
                                                    $sortby,
                                                    $currentPage,
                                                    $qPerPage);
        $input_errors = array_merge($input_errors, $quarantine_errors);
      } else if (isset($_GET['sortby'])) {
        $qperpage = $CONFIG['QUARANTINE_PER_PAGE'];
        if (isset($_GET['qperpage'])) { $qperpage = $_GET['qperpage']; }
        $page = 1;
        if (isset($_GET['page'])) { $page = $_GET['page']; }

        $quarantine_errors =& DisplayQuarantine($_GET['sortby'],
                                                intval($page),
                                                intval($qperpage));
        $input_errors = array_merge($input_errors, $quarantine_errors);
      } else if (isset($_GET['qperpage'])) {
        if (isset($_GET['page'])) {
          $quarantine_errors =& DisplayQuarantine($CONFIG['SORT_DEFAULT'],
                                                  intval($_GET['page']),
                                                  intval($_GET['qperpage']));
          $input_errors = array_merge($input_errors, $quarantine_errors);
        } else {
          $quarantine_errors =& DisplayQuarantine($CONFIG['SORT_DEFAULT'],
                                                  1,
                                                  intval($_GET['qperpage']));
          $input_errors = array_merge($input_errors, $quarantine_errors);
        }
      } else {
        $displayq_errors =& DisplayQuarantine($CONFIG['SORT_DEFAULT'], 1);
        $input_errors = array_merge($input_errors, $displayq_errors);
      }
    } else if ($_POST) {
      if ($_POST['command'] == "processQuarantine") {
        $signatures = array();

        while (list($key, $val) = each($_POST)) {
          if (strpos($key, "chkmsg") !== false) {
            $signatures[$key] = $val;
          }
        }
        if ($_POST['processAction']) {
          $processq_errors =& ProcessQuarantine($signatures,
                                                $_POST['processAction'],
                                                $_POST['sortby'],
                                                intval($_POST['qpage']),
                                                intval($_POST['qperpage']));
          $input_errors = array_merge($input_errors, $processq_errors);
        } else {
          $processq_errors =& ProcessQuarantine($signatures,
                                                "None",
                                                $_POST['sortby'],
                                                intval($_POST['qpage']),
                                                intval($_POST['qperpage']));
          $input_errors = array_merge($input_errors, $processq_errors);
        }
      } else if ($_POST['command'] == "processFalsePositive") {
        $processfp_errors =& ProcessFalsePositive($_POST['signatureID'],
                                                  $_POST['sortby'],
                                                  intval($_POST['qpage']),
                                                  intval($_POST['qperpage']));
        $input_errors = array_merge($input_errors, $processfp_errors);
        $displayq_errors =& DisplayQuarantine($_POST['sortby'],
                                              intval($_POST['qpage']),
                                              intval($_POST['qperpage']));
        $input_errors = array_merge($input_errors, $displayq_errors);
      }
    } else {
      /* usually this particular line of code should not be hit because
       * the user always triggers a GET or POST action. Unfortunatly at
       * the time, the alerts page is implemented in a way that neither
       * triggers a GET nor a POST action if initially accessed via the
       * tab. Hence this final catch all else.
       */
      $displayq_errors =& DisplayQuarantine($CONFIG['SORT_DEFAULT'], 1);
      $input_errors = array_merge($input_errors, $displayq_errors);
    }
  }

  /* Analysis */
  else if (basename($_SERVER['SCRIPT_NAME']) == "dspam-analysis.php") {
    $displaya_errors =& DisplayAnalysis();
    $input_errors = array_merge($input_errors, $displaya_errors);
  }

  /* History */
  else if (in_array(gettext("History"), $pgtitle)) {
    if ($_POST) {
      /* Form fields that may be submited via POST:
       * - command
       * - username
       * - msgid$retrain_checked_msg_no (checkbox, multible times)
       */
      $checked = array();

      while (list($key, $val) = each($_POST)) {
        if (strpos($key, "msgid") !== false) {
          $checked[] = $val;
        }
      }

      $displayh_errors =& DisplayHistory($_POST['command'],
                                         "",
                                         $checked,
                                         $_POST['username'],
                                         "",
                                         $_POST['hpage'],
                                         $_POST['hperpage']);
      $input_errors = array_merge($input_errors, $displayh_errors);
    } else if ($_GET) {
      /* Form fields that may be submited via GET:
       * - history_page
       * - command
       * - signatureID
       * - retrain
       * - user
       */
       $displayh_errors =& DisplayHistory($_GET['command'],
                                          $_GET['signatureID'],
                                          NULL,
                                          $_GET['user'],
                                          $_GET['retrain'],
                                          isset($_GET['page']) ? intval($_GET['page']) : 1,
                                          isset($_GET['hperpage']) ? $_GET['hperpage'] : $CONFIG['QUARANTINE_PER_PAGE']);
       $input_errors = array_merge($input_errors, $displayh_errors);
    } else {
      $displayh_errors =& DisplayHistory("",    // command
                                         "",    // signature ID
                                         NULL,  // signatures to be retrained
                                         $CURRENT_USER);
      $input_errors = array_merge($input_errors, $displayh_errors);
    }
  } else if (in_array(gettext("Fragment"), $pgtitle)) {
    if ($_GET) {
      $displayf_errors =& DisplayFragment($_GET['signatureID'],
                                          $_GET['from'],
                                          $_GET['subject'],
                                          $_GET['info'],
                                          $_GET['time']);
      $input_errors = array_merge($input_errors, $displayf_errors);
    }
  }

  /* Info Page */
  else if (basename($_SERVER['SCRIPT_NAME']) == "dspam.php") {
    $info_errors =& DisplayInfos();
    $input_errors = array_merge($input_errors, $info_errors);
  }
  
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

?>