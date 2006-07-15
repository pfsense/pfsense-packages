<?php
/* $Id$ */
/*
  dspam-train.php

  Copyright (C) 2006 Daniel S. Haischt.
  All rights reserved.
*/

$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("Quarantine"),
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
  $jscriptstr .= getJScriptFunction(1);
  $jscriptstr .= getJScriptFunction(2);
  $jscriptstr .= getJScriptFunction(3);
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
<form action="dspam-quarantine.php" method="post" name="iform" id="iform">
<input type="hidden" name="command" value="processQuarantine" />
<input type="hidden" name="processAction" value="None" />
<input type="hidden" name="qpage" value="<?= $DATA['QPAGE']; ?>" />
<input type="hidden" name="sortby" value="<?= $DATA['SORTBY']; ?>" >
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<p>
  <span class="vexpl">
    The messages below have not been delivered to your normal e-mail application
    because they are believed to be spam. Click on the Subject line to view the
    message or choose a sort option to change how messages are sorted. Use the
    checkboxes and <strong>Deliver Checked</strong> to deliver messages you want
    to read, or use <strong>Delete All</strong> to empty the quarantine.
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
  $tab_array[] = array("Quarantine ({$DATA['TOTAL_QUARANTINED_MESSAGES']})",   true,  "/dspam-quarantine.php?user={$CURRENT_USER}");
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
          <td align="left" valign="top" class="listhdrr" colspan="5">Statistical SPAM Protection for...</td>
        </tr>
        <tr>
          <td width="10%" valign="baseline" class="vncell" colspan="2">Username</td>
          <td width="90%" class="vtable" colspan="3">
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
          <td colspan="5" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="middle" colspan="4" class="qnavtdl">
            <nobr>
              <input type="button" class="formbtn" title="Deliver Checked" value="Deliver Checked" name="delichk" id="delichk" onclick="processmsg(0);" />&nbsp;
              <input type="button" class="formbtn" title="Delete Checked" value="Delete Checked" name="delchk" id="delchk" onclick="processmsg(1);" />&nbsp;
              <input type="button" class="formbtn" title="Delete All" value="Delete All" name="delall" id="delall" onclick="processmsg(2);" />
            </nobr>
          </td>
          <td align="right" valign="middle" class="qnavtdr">
            <label for="qperpage-top">Records per page:&nbsp;</label>
            <select class="formselect" id="qperpage-top" name="qperpage" onchange="changeQPerPage(this);">
              <option value="25"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 25) echo ' selected="selected"'; ?>>25</option>
              <option value="50"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 50) echo ' selected="selected"'; ?>>50</option>
              <option value="75"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 75) echo ' selected="selected"'; ?>>75</option>
              <option value="100"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 100) echo ' selected="selected"'; ?>>100</option>
              <option value="125"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 125) echo ' selected="selected"'; ?>>125</option>
              <option value="150"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 150) echo ' selected="selected"'; ?>>150</option>
            </select>
          </td>
        </tr>
        <tr>
          <td colspan="5" class="list" height="12">&nbsp;</td>
        </tr>
        <?= $DATA['SORT_SELECTOR']; ?>
        <?= $DATA['QUARANTINE']; ?>
        <?= $DATA['QUARANTINE_FOOTER']; ?>
        <tr>
          <td colspan="5" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="middle" colspan="4" class="qnavtdl">
            <nobr>
              <input type="button" class="formbtn" title="Deliver Checked" value="Deliver Checked" name="delichk" id="delichk" onclick="processmsg(0);" />&nbsp;
              <input type="button" class="formbtn" title="Delete Checked" value="Delete Checked" name="delchk" id="delchk" onclick="processmsg(1);" />&nbsp;
              <input type="button" class="formbtn" title="Delete All" value="Delete All" name="delall" id="delall" onclick="processmsg(2);" />
            </nobr>
          </td>
          <td align="right" valign="middle" class="qnavtdr">
            <label for="qperpage-bottom">Records per page:&nbsp;</label>
            <select class="formselect" id="qperpage-bottom" name="qperpage" onchange="changeQPerPage(this);">
              <option value="25"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 25) echo ' selected="selected"'; ?>>25</option>
              <option value="50"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 50) echo ' selected="selected"'; ?>>50</option>
              <option value="75"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 75) echo ' selected="selected"'; ?>>75</option>
              <option value="100"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 100) echo ' selected="selected"'; ?>>100</option>
              <option value="125"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 125) echo ' selected="selected"'; ?>>125</option>
              <option value="150"<?php if ($CONFIG['QUARANTINE_PER_PAGE'] == 150) echo ' selected="selected"'; ?>>150</option>
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