<?php
/* $Id$ */
/*
  dspam-train.php

  Copyright (C) 2006 Daniel S. Haischt.
  All rights reserved.
*/

$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("Performance"));

require("guiconfig.inc");
include("/usr/local/pkg/dspam.inc");

/* if this is an AJAX caller then handle via JSON */
if(isAjax() && is_array($input_errors)) {
	input_errors2Ajax($input_errors);
	exit;
}

/* if ajax is calling, give them an update message */
if(isAjax())
	print_info_box_np($savemsg);

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */

  $jscriptstr = <<<EOD
<script type="text/javascript">
<!--

EOD;

  $jscriptstr .= getJScriptFunction(0);
  $jscriptstr .= <<<EOD
//-->
</script>
EOD;

$pfSenseHead->addScript($jscriptstr);
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="dspam-perf.php" method="post" name="iform" id="iform">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<p>
  <span class="vexpl">
    If you receive a message in your e-mail application that was not caught by
    the filter, please forward it to <strong><?= $DATA['SPAM_ALIAS']; ?></strong>
    so that it can be analyzed and learned as <acronym title="">SPAM</acronym>.
    This will improve the filter's accuracy in the future.
  </span>
</p>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
<?php
  $tab_array = array();
  $tab_array[] = array("Info",         false, "/dspam.php?{$CURRENT_USER}");
  $tab_array[] = array("Performance",  true,  "/dspam-perf.php?user={$CURRENT_USER}");
  $tab_array[] = array("Preferences",  false, "/dspam-prefs.php?user={$CURRENT_USER}");
  $tab_array[] = array("Alerts",       false, "/pkg.php?xml=dspam_alerts.xml&user={$CURRENT_USER}");
  $tab_array[] = array("Quarantine ({$DATA['TOTAL_QUARANTINED_MESSAGES']})",   false, "/dspam-quarantine.php?user={$CURRENT_USER}");
  $tab_array[] = array("Analysis",     false, "/dspam-analysis.php?user={$CURRENT_USER}");
  $tab_array[] = array("History",      false, "/dspam-history.php?user={$CURRENT_USER}");
  $tab_array[] = array("Train Filter", false, "/dspam-train.php?user={$CURRENT_USER}");
  if (isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER'])) {
    $tab_array[] = array("Admin Suite",  false, "/dspam-admin.php");
  }
  display_top_tabs($tab_array);
?>
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
      <table class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
        <tr>
          <td align="left" valign="top" class="listhdrr" colspan="2">Statistical SPAM Protection for...</td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">Username</td>
          <td width="78%" class="vtable">
          <?php if(isset($HTTP_SERVER_VARS['AUTH_USER'])): ?>
            <input type="text" name="username" id="username" value="<?= $CURRENT_USER ?>" class="formfld user"<?php if (! isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER'])) { echo " readonly=\"readonly\""; } ?> />
          <?php else: ?>
            <input type="text" name="username" id="username" value="Please provide a username" class="formfld user" onFocus="this.value='';" />
          <?php endif; ?>
            &nbsp;
            <?php
              if (! isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER']))
                $action = "onClick=\"changeuser();\"";
              else
                $action = "onClick=\"document.iform.submit();\"";
            ?>
            <input type="button" name="change_user" id="change_user" class="formbtn" value="Change" <?= $action ?> />
          </td>
        </tr>
        <tr>
          <td colspan="2" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="top" class="listtopic" colspan="2">
            <strong>Performance Statistics</strong> &ndash; <?= date("l dS of F Y h:i:s A"); ?>
          </td>
        </tr>
        <tr>
          <td align="left" valign="top" colspan="2" class="vncell">
            <table align="center" width="100%" border="0" cellpadding="0" cellspacing="0">
              <tr>
                <!- beginn left info pane -->
                <td align="left" valign="top">
                  <table border="0" cellpadding="0" cellspacing="0" summary="left info pane">
                    <tr>
                      <td align="left" valign="top" class="listhdrr" colspan="2">Metric</td>
                      <td align="left" valign="top" class="listhdrr">Calculated as</td>
                      <td align="left" valign="top" class="list"></td>
                    </tr>
                    <tr>
                      <td align="left" valign="top" class="listr">
                        <nobr>Overall accuracy (since last reset)</nobr>
                      </td>
                      <td align="left" valign="top" class="listr">
                        <strong><?= $DATA['OVERALL_ACCURACY']; ?>%</strong>
                      </td>
                      <td align="left" valign="top" class="listr">(SPAM messages caught + Good messages delivered) / Total number of messages</td>
                      <td align="left" valign="top" class="none">&nbsp;</td>
                    </tr>
                    <tr>
                      <td align="left" valign="top" class="listr">
                        <nobr>Spam identification (since last reset)</nobr>
                      </td>
                      <td align="left" valign="top" class="listr">
                        <strong><?= $DATA['SPAM_ACCURACY']; ?>%</strong>
                      </td>
                      <td align="left" valign="top" class="listr">(Spam catch rate only)</td>
                      <td align="left" valign="top" class="none">&nbsp;</td>
                    </tr>
                    <tr>
                      <td align="left" valign="top" class="listr">
                        <nobr>Spam ratio (of total processed)</nobr>
                      </td>
                      <td align="left" valign="top" class="listr">
                        <strong><?= $DATA['SPAM_RATIO']; ?>%</strong>
                      </td>
                      <td align="left" valign="top" class="listr">Total SPAM messages (both caught & missed) / Total number of messages</td>
                      <td align="left" valign="top" class="none">&nbsp;</td>
                    </tr>
                  </table>
                </td>
                <!-- spacer td -->
                <td align="left" valign="top" class="none">&nbsp;</td>
                <!-- begin right info pane -->
                <td align="left" valign="top">
                  <table border="0" cellpadding="0" cellspacing="0" summary="right info pane">
                    <tr id="frheader">
                      <td class="list">&nbsp;</td>
                      <td class="listhdrr">SPAM messages</td>
                      <td class="listhdrr">Good messages</td>
                    </tr>
                    <tr>
                      <td align="left" valign="top" class="listhdrr">Since last reset</td>
                      <td align="left" valign="top" class="listr">
                        <nobr><?= $DATA['TOTAL_SPAM_MISSED']; ?> missed</nobr><br />
                        <nobr><?= $DATA['TOTAL_SPAM_CAUGHT']; ?> caught</nobr><br />
                        <nobr><?= $DATA['SPAM_ACCURACY']; ?>% caught</nobr><br />
                      </td>
                      <td align="left" valign="top" class="listr">
                        <nobr><?= $DATA['TOTAL_NONSPAM_MISSED']; ?> missed</nobr><br />
                        <nobr><?= $DATA['TOTAL_NONSPAM_CAUGHT']; ?> delivered</nobr><br />
                        <nobr><?= $DATA['NONSPAM_ERROR_RATE']; ?>% missed</nobr><br />
                      </td>
                    </tr>
                    <tr>
                      <td align="left" valign="top" class="listhdrr">Total processed by filter</td>
                      <td align="left" valign="top" class="listr">
                        <nobr><?= $DATA['TOTAL_SPAM_LEARNED']; ?> missed</nobr><br />
                        <nobr><?= $DATA['TOTAL_SPAM_SCANNED']; ?> caught</nobr><br />
                      </td>
                      <td align="left" valign="top" class="listr">
                        <nobr><?= $DATA['TOTAL_NONSPAM_LEARNED']; ?> missed</nobr><br />
                        <nobr><?= $DATA['TOTAL_NONSPAM_SCANNED']; ?> delivered</nobr><br />
                      </td>
                    </tr>
                    <tr>
                      <td align="left" valign="top" class="listhdrr">From corpus</td>
                      <td align="left" valign="top" class="listr">
                        <nobr><?= $DATA['TOTAL_SPAM_CORPUSFED']; ?> feed</nobr><br />
                      </td>
                      <td align="left" valign="top" class="listr">
                        <nobr><?= $DATA['TOTAL_NONSPAM_CORPUSFED']; ?> feed</nobr><br />
                      </td>
                    </tr>
                  </table>
                </td>
            </table>
          </td>
        </tr>
        <tr>
          <td align="left" valign="top" colspan="2">
            <p>
              <a href="/dspam-perf.php?user=<?= $CURRENT_USER ?>&command=resetStats">Reset</a>&nbsp;|&nbsp;<a href="/dspam-perf.php?user=<?= $CURRENT_USER ?>&command=tweak">Tweak -1</a>
            </p>
          </td>
        </tr>
      </table>
      </div>
    </td>
  </tr>
</table>
</form>

<?php include("fend.inc"); ?>
</body>
</html>