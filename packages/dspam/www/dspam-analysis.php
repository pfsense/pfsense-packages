<?php
/* $Id$ */
/*
  dspam-analysis.php

  Copyright (C) 2006 Daniel S. Haischt.
  All rights reserved.
*/

$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("Analysis"),
                 gettext("Overview"));

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
<form action="dspam-analysis.php" method="post" name="iform" id="iform">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<p>
  <span class="vexpl">
     Graphs showing the number of messages that have been processed are shown below.
  </span>
</p>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
<?php
  $tab_array = array();
  $tab_array[] = array("Info",         false, "/dspam.php?{$CURRENT_USER}");
  $tab_array[] = array("Performance",  false, "/dspam-perf.php?user={$CURRENT_USER}");
  $tab_array[] = array("Preferences",  false, "/dspam-prefs.php?user={$CURRENT_USER}");
  $tab_array[] = array("Alerts",       false, "/pkg.php?xml=dspam_alerts.xml&user={$CURRENT_USER}");
  $tab_array[] = array("Quarantine ({$DATA['TOTAL_QUARANTINED_MESSAGES']})",   false, "/dspam-quarantine.php?user={$CURRENT_USER}");
  $tab_array[] = array("Analysis",     true,  "/dspam-analysis.php?user={$CURRENT_USER}");
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
          <td width="10%" valign="baseline" class="vncell">Username</td>
          <td width="90%" class="vtable">
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
            <strong>24 Hour Activity</strong> &ndash; <?= $DATA['TS_DAILY']; ?> SPAM, <?= $DATA['TI_DAILY']; ?> Good
          </td>
        </tr>
        <tr>
          <td align="center" valign="top" class="vncell" colspan="2">
            <?php if(isset($_GET['test'])): ?>
            <img src="/dspam-analysis-graph.php?data=0,0,1,0,1_1,4,0,1,0_4p,6p,7a,11a,2p&x_label=Hour+of+the+day" alt="24 Hour Activity" border="0" />
            <?php else: ?>
            <img src="/dspam-analysis-graph.php?data=<?= $DATA['DATA_DAILY']; ?>&x_label=<?= urlencode("Hour of the day"); ?>" alt="24 Hour Activity" border="0" />
            <?php endif; ?>
          </td>
        </tr>
        <tr>
          <td colspan="2" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="top" class="listtopic" colspan="2">
            <strong>14 Day Activity</strong> &ndash; <?= $DATA['TS_WEEKLY']; ?> SPAM, <?= $DATA['TI_WEEKLY']; ?> Good
          </td>
        </tr>
        <tr>
          <td align="center" valign="top" class="vncell" colspan="2">
            <?php if(isset($_GET['test'])): ?>
            <img src="/dspam-analysis-graph.php?data=1,2,0,2,1,2,2,1,4,0,0,2,0,2_5,2,3,5,12,20,7,9,9,8,7,12,6,1_6/9,6/10,6/11,6/12,6/13,6/14,6/15,6/16,6/17,6/18,6/19,6/20,6/21,6/22&graph=period&x_label=Date" alt="24 Hour Activity" border="0" />
            <?php else: ?>
            <img src="/dspam-analysis-graph.php?data=<?= $DATA['DATA_WEEKLY']; ?>&graph=period&x_label=Date" alt="24 Hour Activity" border="0" />
            <?php endif; ?>
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