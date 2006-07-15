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
                 gettext("View Message"));

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
<form action="dspam-quarantine.php" method="post" name="iform" id="iform">
<input type="hidden" name="command" value="processFalsePositive" />
<input type="hidden" name="signatureID" value="<?= $DATA['MESSAGE_ID']; ?>" />
<input type="hidden" name="qpage" value="<?= $DATA['QPAGE']; ?>" />
<input type="hidden" name="sortby" value="<?= $DATA['SORTBY']; ?>" >
<input type="hidden" name="qperpage" value="<?= $CONFIG['QUARANTINE_PER_PAGE']; ?>" >
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<p>
  <span class="vexpl">
    The contents of the message in the quarantine is shown below.
  </span>
</p>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
<?php
  $querystr = "?user=?{$CURRENT_USER}&page={$DATA['QPAGE']}&sortby={$DATA['SORTBY']}&qperpage={$CONFIG['QUARANTINE_PER_PAGE']}";

  $tab_array = array();
  $tab_array[] = array("Info",         false, "/dspam.php?{$CURRENT_USER}");
  $tab_array[] = array("Performance",  false, "/dspam-perf.php?{$CURRENT_USER}");
  $tab_array[] = array("Preferences",  false, "/dspam-prefs.php?{$CURRENT_USER}");
  $tab_array[] = array("Alerts",       false, "/pkg.php?xml=dspam_alerts.xml&user={$CURRENT_USER}");
  $tab_array[] = array("Quarantine (View)",   true,  "/dspam-quarantine.php{$querystr}");
  $tab_array[] = array("Analysis",     false, "/dspam-analysis.php?{$CURRENT_USER}");
  $tab_array[] = array("History",      false, "/dspam-history.php?{$CURRENT_USER}");
  $tab_array[] = array("Train Filter", false, "/dspam-train.php?{$CURRENT_USER}");
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
          <td align="left" valign="top" class="listhdrr" colspan="3">Statistical SPAM Protection for...</td>
        </tr>
        <tr>
          <td width="15%" valign="baseline" class="vncell">Username</td>
          <td width="85%" class="vtable" colspan="2">
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
          <td class="list" height="12" colspan="3">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="middle" class="qnavtd" colspan="3">
          <input type="submit" class="formbtn" title="Deliver Message" value="Deliver Message" name="delmsg" id="delmsg" />
          <label for="delmsg">&nbsp;because it is <strong>not</strong> SPAM</label>
          </td>
        </tr>
        <tr>
          <td class="list" height="12" colspan="3">&nbsp;</td>
        </tr>
        <?php if(! extension_loaded( 'mailparse' ) && $CONFIG['USE_MAILPARSE']): ?>
        <?php if(! @dl( 'mailparse.so' )): ?>
        <tr>
          <td align="left" valign="top" class="listtopic" colspan="3">Mail Message</td>
        </tr>
        <tr>
          <td align="center" valign="top" colspan="3" class="vncell">
            <textarea rows="36" cols="87" readonly="readonly">
            <?= $DATA['MESSAGE']; ?>
            </textarea>
          </td>
        </tr>
        <?php else: ?>
        <?= getLayoutedMessage($DATA['MESSAGE'], $DATA['MESSAGE_ID'], $DATA['SHOWPART'], $DATA['CONTENT_TYPE']); ?>
        <?php endif; ?>
        <?php else: ?>
        <tr>
          <td align="left" valign="top" class="listtopic" colspan="3">Mail Message</td>
        </tr>
        <tr>
          <td align="center" valign="top" colspan="3" class="vncell">
            <textarea rows="36" cols="87" readonly="readonly">
            <?= $DATA['MESSAGE']; ?>
            </textarea>
          </td>
        </tr>
        <?php endif; ?>
      </table>
      </div>
    </td>
  </tr>
</table>
</form>

<?php include("fend.inc"); ?>
</body>
</html>