<?php
/* $Id$ */
/*
  dspam-train.php

  Copyright (C) 2006 Daniel S. Haischt.
  All rights reserved.
*/

$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("System Status"),
                 gettext("Overview"));

require("guiconfig.inc");
include("/usr/local/pkg/dspam.inc");

if (isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER'])) {

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
  echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="dspam-admin.php" method="post" name="iform" id="iform">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<p>
  <span class="vexpl">
     The following graphs and tables summarize the processing done by the filter.
  </span>
</p>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
<?php
  $tab_array = array();
  $tab_array[] = array("System Status",   true,  "/dspam-admin.php?user={$CURRENT_USER}");
  $tab_array[] = array("User Statistics", false, "/dspam-admin-stats.php?user={$CURRENT_USER}");
  $tab_array[] = array("Administration",  false, "/dspam-admin-prefs.php?user={$CURRENT_USER}");
  $tab_array[] = array("Settings",        false, "/dspam-settings.php?user={$CURRENT_USER}");
  $tab_array[] = array("Control Center",  false, "/dspam.php?user={$CURRENT_USER}");
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
            <strong><?= $CURRENT_USER ?></strong>
          </td>
        </tr>
        <tr>
          <td colspan="2" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td colspan="2" align="left" valign="top" class="listtopic">
            <strong>Overview</strong>
          </td>
        </tr>
        <tr>
          <td colspan="2" align="center" valign="top" class="vncell">
            <table align="center" width="100%" border="0" cellpadding="0" cellspacing="0">
              <tr>
                <td align="left" valign="top" class="listhdrr">Messages</td>
                <td align="left" valign="top" class="listhdrr">Today</td>
                <td align="left" valign="top" class="listhdrr">This Hour</td>
                <td align="left" valign="top" class="list">&nbsp;</td>
                <td align="left" valign="top" class="listhdrr">Status</td>
                <td align="left" valign="top" class="listhdrr">Current Value</td>
              </tr>
              <tr>
                <td align="left" valign="top" class="listr">Spam</td>
                <td align="left" valign="top" class="listr"><?= $DATA['SPAM_TODAY']; ?></td>
                <td align="left" valign="top" class="listr"><?= $DATA['SPAM_THIS_HOUR']; ?></td>
                <td align="left" valign="top" class="list">&nbsp;</td>
                <td align="left" valign="top" class="listr">Average message processing time</td>
                <td align="left" valign="top" class="listr"><?= $DATA['AVG_PROCESSING_TIME']; ?> sec.</td>
              </tr>
              <tr>
                <td align="left" valign="top" class="listr">Good</td>
                <td align="left" valign="top" class="listr"><?= $DATA['NONSPAM_TODAY']; ?></td>
                <td align="left" valign="top" class="listr"><?= $DATA['NONSPAM_THIS_HOUR']; ?></td>
                <td align="left" valign="top" class="list">&nbsp;</td>
                <td align="left" valign="top" class="listr">Average throughput</td>
                <td align="left" valign="top" class="listr"><?= $DATA['AVG_MSG_PER_SECOND']; ?> messages/sec.</td>
              </tr>
              <tr>
                <td align="left" valign="top" class="listr">Spam Misses</td>
                <td align="left" valign="top" class="listr"><?= $DATA['SM_TODAY']; ?></td>
                <td align="left" valign="top" class="listr"><?= $DATA['SM_THIS_HOUR']; ?></td>
                <td align="left" valign="top" class="list">&nbsp;</td>
                <td align="left" valign="top" class="listr">DSPAM instances</td>
                <td align="left" valign="top" class="listr"><?= $DATA['DSPAM_PROCESSES']; ?> process(es)</td>
              </tr>
              <tr>
                <td align="left" valign="top" class="listr">False Positives</td>
                <td align="left" valign="top" class="listr"><?= $DATA['FP_TODAY']; ?></td>
                <td align="left" valign="top" class="listr"><?= $DATA['FP_THIS_HOUR']; ?></td>
                <td align="left" valign="top" class="list">&nbsp;</td>
                <td align="left" valign="top" class="listr">System uptime</td>
                <td align="left" valign="top" class="listr"><?= $DATA['UPTIME']; ?></td>
              </tr>
              <tr>
                <td align="left" valign="top" class="listr">Inoculations</td>
                <td align="left" valign="top" class="listr"><?= $DATA['INOC_TODAY']; ?></td>
                <td align="left" valign="top" class="listr"><?= $DATA['INOC_THIS_HOUR']; ?></td>
                <td align="left" valign="top" class="list">&nbsp;</td>
                <td align="left" valign="top" class="listr">Mail queue length</td>
                <td align="left" valign="top" class="listr"><?= $DATA['MAIL_QUEUE']; ?> messages</td>
              </tr>
              <tr>
                <td align="left" valign="top" class="listbgns"><strong>Total</strong></td>
                <td align="left" valign="top" class="listbgns"><strong><?= $DATA['TOTAL_TODAY']; ?></strong></td>
                <td align="left" valign="top" class="listbgns"><strong><?= $DATA['TOTAL_THIS_HOUR']; ?></strong></td>
                <td align="left" valign="top" class="list">&nbsp;</td>
                <td align="left" valign="top" class="list">&nbsp;</td>
                <td align="left" valign="top" class="list">&nbsp;</td>
              </tr>
            </table>
          </td>
        </tr>
        <tr>
          <td colspan="2" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td colspan="2" align="left" valign="top" class="listtopic">
            <strong>24 Hour Activity</strong> &ndash; 125 SPAM, 601 Good, 2 Spam Misses, 0 False Positives, 2 Inoculations
          </td>
        </tr>
        <tr>
          <td colspan="2" align="center" valign="top" class="vncell">
            <?php if(isset($_GET['test'])): ?>
            <img src="/dspam-admin-graph.php?data=1,2,5,6,2,6,3,1,3,9,5,2,4,8,9,6,9,2,6,8,3,3,5,2_4,22,12,9,11,10,10,8,2,9,9,27,18,26,20,20,11,14,27,69,51,108,86,43_0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0_0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0_0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,2,0,0,0,0_2,8,3,9,7,4,3,2,2,,4,7,6,3,4,6,2,7,4,17,5,3,10,1_10:00pm,11:00pm,12:00am,1:00am,2:00am,3:00am,4:00am,5:00am,6:00am,7:00am,8:00am,9:00am,10:00am,11:00am,12:00pm,1:00pm,2:00pm,3:00pm,4:00pm,5:00pm,6:00pm,7:00pm,8:00pm,9:00pm&x_label=Hour+of+the+day&offset=35" alt="24 Hour Activity" border="0" />
            <?php else: ?>
            <img src="/dspam-admin-graph.php?data=<?= $DATA['DATA_DAILY']; ?>&x_label=<?= urlencode("Hour of the day"); ?>&offset=20" alt="24 Hour Activity" border="0" />
            <?php endif; ?>
          </td>
        </tr>
        <tr>
          <td colspan="2" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td colspan="2" align="left" valign="top" class="listtopic">
            <strong>Daily Activity</strong> &ndash; 2457 SPAM, 10772 Good, 35 Spam Misses, 0 False Positives, 33 Inoculations
          </td>
        </tr>
        <tr>
          <td colspan="2" align="center" valign="top" class="vncell">
            <?php if(isset($_GET['test'])): ?>
            <img src="/dspam-admin-graph.php?data=105,98,54,104,85,94,93,103,115,122,109,94,77,103,116,105,112,103,97,83,87,99,97,126,107_368,339,326,395,367,166,176,325,376,382,458,305,149,134,335,396,388,368,403,220,142,534,312,595,600_0,2,0,2,1,3,0,1,4,1,0,0,0,1,2,1,2,1,2,3,4,1,1,2,0_0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0_1,1,2,1,4,1,0,3,1,0,2,0,2,1,2,1,1,5,0,1,0,0,0,2,4_129,142,76,184,139,55,51,94,107,139,168,130,70,63,123,140,118,96,108,88,46,110,133,143,109_5/29/2006,5/30/2006,5/31/2006,6/1/2006,6/2/2006,6/3/2006,6/4/2006,6/5/2006,6/6/2006,6/7/2006,6/8/2006,6/9/2006,6/10/2006,6/11/2006,6/12/2006,6/13/2006,6/14/2006,6/15/2006,6/16/2006,6/17/2006,6/18/2006,6/19/2006,6/20/2006,6/21/2006,6/22/2006&graph=period&x_label=Date&offset=45" border="0" />
            <?php else: ?>
            <img src="/dspam-admin-graph.php?data=<?= $DATA['DATA_WEEKLY']; ?>&graph=period&x_label=Date&offset=45" border="0" />
            <?php endif; ?>
          </td>
        </tr>
      </table>
      </div>
    </td>
  </tr>
</table>
</form>
<?
} else {
?>
<?php
  $input_errors[] = "Access to this particular site was denied. You need DSPAM admin access rights to be able to view it.";

  include("head.inc");
  echo $pfSenseHead->getHTML();
?>
<?php include("fbegin.inc");?>
<?php if ($input_errors) print_input_errors($input_errors);?>
<?php if ($savemsg) print_info_box($savemsg);?>
  <body link="#000000" vlink="#000000" alink="#000000">
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr>
        <td valign="top" class="listtopic">Access denied for: <?=$HTTP_SERVER_VARS['AUTH_USER']?></td>
      </tr>
    </table>
<?php
} // end of access denied code
?>
<?php include("fend.inc"); ?>
</body>
</html>