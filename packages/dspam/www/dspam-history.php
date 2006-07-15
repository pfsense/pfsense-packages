<?php
/* $Id$ */
/*
  dspam-train.php

  Copyright (C) 2006 Daniel S. Haischt.
  All rights reserved.
*/

$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("History"),
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
  $jscriptstr .= getJScriptFunction(4);
  $jscriptstr .= <<<EOD
//-->
</script>
EOD;

$pfSenseHead->addScript($jscriptstr);
$pfSenseHead->addLink("<link rel=\"stylesheet\" type=\"text/css\" href=\"/themes/" . $g['theme'] . "/dspam.css\" media=\"all\" />\n");
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="dspam-history.php" method="post" name="iform" id="iform">
<input type="hidden" name="command" value="retrainChecked" />
<input type="hidden" name="hpage" value="<?= $DATA['HPAGE']; ?>" />
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<p>
  <span class="vexpl">
    The messages that have been processed by the filter are shown below. The
    most recent messages are shown first. Use the retrain options to correct
    errors and deliver any false positives that are still in your quarantine.
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
  $tab_array[] = array("Analysis",     false, "/dspam-analysis.php?user={$CURRENT_USER}");
  $tab_array[] = array("History",      true,  "/dspam-history.php?user={$CURRENT_USER}");
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
          <td align="left" valign="top" class="listhdrr" colspan="6">Statistical SPAM Protection for...</td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell" colspan="2">Username</td>
          <td width="78%" class="vtable" colspan="4">
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
          <td colspan="6" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="middle" class="qnavtdl" colspan="4">
            <input type="submit" class="formbtn" title="Retrain Checked" value="Retrain Checked" name="retrain_checked" id="retrain_checked_top" />
            <label for="retrain_checked_top">&nbsp;because those messages have<strong>n't</strong> been correctly classified.</label>
          </td>
          <td align="right" valign="middle" class="qnavtdr" colspan="2">
            <label for="hperpage-top">Records per page:&nbsp;</label>
            <select class="formselect" id="hperpage-top" name="hperpage" onchange="changeQPerPage(this);">
              <option value="25"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 25) echo ' selected="selected"'; ?>>25</option>
              <option value="50"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 50) echo ' selected="selected"'; ?>>50</option>
              <option value="75"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 75) echo ' selected="selected"'; ?>>75</option>
              <option value="100"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 100) echo ' selected="selected"'; ?>>100</option>
              <option value="125"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 125) echo ' selected="selected"'; ?>>125</option>
              <option value="150"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 150) echo ' selected="selected"'; ?>>150</option>
            </select>
          </td>
        </tr>
        <tr>
          <td class="list" height="12" colspan="6">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="top" class="listtopic" width="10%">Type</td>
          <td align="left" valign="top" class="listtopic" width="10%">Action</td>
          <td align="left" valign="top" class="listtopic" width="10%">Day/Time</td>
          <td align="left" valign="top" class="listtopic" width="25%">From</td>
          <td align="left" valign="top" class="listtopic" width="25%">Subject</td>
          <td align="left" valign="top" class="listtopic" width="20%">Additional Info</td>
        </tr>
        <?= $DATA['HISTORY']; ?>
        <?= $DATA['HISTORY_FOOTER']; ?>
        <tr>
          <td colspan="6" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="middle" class="qnavtdl" colspan="4">
            <input type="submit" class="formbtn" title="Retrain Checked" value="Retrain Checked" name="retrain_checked" id="retrain_checked_bottom" />
            <label for="retrain_checked_bottom">&nbsp;because those messages have<strong>n't</strong> correctly been classified.</label>
          </td>
          <td align="right" valign="middle" class="qnavtdr" colspan="2">
            <label for="hperpage-bottom">Records per page:&nbsp;</label>
            <select class="formselect" id="hperpage-bottom" name="hperpage" onchange="changeQPerPage(this);">
              <option value="25"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 25) echo ' selected="selected"'; ?>>25</option>
              <option value="50"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 50) echo ' selected="selected"'; ?>>50</option>
              <option value="75"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 75) echo ' selected="selected"'; ?>>75</option>
              <option value="100"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 100) echo ' selected="selected"'; ?>>100</option>
              <option value="125"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 125) echo ' selected="selected"'; ?>>125</option>
              <option value="150"<?php if ($CONFIG['HISTORY_PER_PAGE'] == 150) echo ' selected="selected"'; ?>>150</option>
            </select>
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