<?php
/* $Id$ */
/*
  dspam-train.php

  Copyright (C) 2006 Daniel S. Haischt.
  All rights reserved.
*/

$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("Admin Preferences"));

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
<form action="dspam-admin-prefs.php" method="post" name="iform" id="iform">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<p>
  <span class="vexpl">
    This page lets you configure how the filter will handle your messages.
  </span>
</p>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
<?php
  $tab_array = array();
  $tab_array[] = array("System Status",   false, "/dspam-admin.php?user={$CURRENT_USER}");
  $tab_array[] = array("User Statistics", false, "/dspam-admin-stats.php?user={$CURRENT_USER}");
  $tab_array[] = array("Administration",  true,  "/dspam-admin-prefs.php?user={$CURRENT_USER}");
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
            <strong>Training</strong> &ndash; Configure how the filter learns as it processes messages
          </td>
        </tr>
        <tr>
          <td align="left" valign="top" class="vncell" width="40%">
            <p>DSPAM should train:</p>
            <input <?= $DATA["S_TEFT"]; ?> value="TEFT" type="radio" class="formfld" title="On every new message scanned by the filter" alt="On every new message scanned by the filter" name="rad_train" id="rad_train_one" />
            <label for="rad_train_one">&nbsp;On every new message scanned by the filter (TEFT)</label>
            <br />
            <input <?= $DATA["S_TOE"]; ?> value="TOE" type="radio" class="formfld" title="Only when the filter makes a mistake" alt="Only when the filter makes a mistake" name="rad_train" id="rad_train_two" />
            <label for="rad_train_two">&nbsp;Only when the filter makes a mistake (TOE)</label>
            <br />
            <input <?= $DATA["S_TUM"]; ?> value="TUM" type="radio" class="formfld" title=";Only with new data or if the filter makes a mistake" alt=";Only with new data or if the filter makes a mistake" name="rad_train" id="rad_train_three" />
            <label for="rad_train_three">&nbsp;Only with new data or if the filter makes a mistake (TUM)</label>
          </td>
          <td align="left" valign="top" class="vncell" width="60%">
            <p>When I train DSPAM, I prefer:</p>
            <input value="message" <?= $DATA["S_LOC_MESSAGE"]; ?> value="message" type="radio" class="formfld" title="To forward my spams (signature appears in message body)" alt="To forward my spams (signature appears in message body)" name="rad_train_action" id="rad_train_action_one" />
            <label for="rad_train_action_one">&nbsp;To <u>forward</u> my spams (signature appears in message body)</label>
            <br />
            <input <?= $DATA["S_LOC_HEADERS"]; ?> value="headers" type="radio" class="formfld" title="To bounce my spams (signature appears in message headers)" alt="To bounce my spams (signature appears in message headers)" name="rad_train_action" id="rad_train_action_two" />
            <label value="headers" for="rad_train_action_two">&nbsp;To <u>bounce</u> my spams (signature appears in message headers)</label>
          </td>
        </tr>
        <tr>
          <td align="left" valign="middle" class="vncell" colspan="2">
            <p>
              Filter sensitivity <strong>during</strong> the training period:
            </p>
            <p align="center">
              <nobr>
                <span>
                  Catch SPAM (More in Quarantine)&nbsp;
                  <input value="0" type="radio" class="formfld" title="-5" alt="-5" name="rad_filter_sens" <?= $DATA["SEDATION_0"]; ?> />
                  <input value="1" type="radio" class="formfld" title="-4" alt="-4" name="rad_filter_sens" <?= $DATA["SEDATION_1"]; ?> />
                  <input value="2" type="radio" class="formfld" title="-3" alt="-3" name="rad_filter_sens" <?= $DATA["SEDATION_2"]; ?> />
                  <input value="3" type="radio" class="formfld" title="-2" alt="-2" name="rad_filter_sens" <?= $DATA["SEDATION_3"]; ?> />
                  <input value="4" type="radio" class="formfld" title="-1" alt="-1" name="rad_filter_sens" <?= $DATA["SEDATION_4"]; ?> />
                  <strong style="font-size: larger;">&raquo;</strong>
                  <input value="5" type="radio" class="formfld" title="0" alt="0" name="rad_filter_sens" <?= $DATA["SEDATION_5"]; ?> />
                  <strong style="font-size: larger;">&laquo;</strong>
                  <input value="6" type="radio" class="formfld" title="1" alt="1" name="rad_filter_sens" <?= $DATA["SEDATION_6"]; ?> />
                  <input value="7" type="radio" class="formfld" title="2" alt="2" name="rad_filter_sens" <?= $DATA["SEDATION_7"]; ?> />
                  <input value="8" type="radio" class="formfld" title="3" alt="3" name="rad_filter_sens" <?= $DATA["SEDATION_8"]; ?> />
                  <input value="9" type="radio" class="formfld" title="4" alt="4" name="rad_filter_sens" <?= $DATA["SEDATION_9"]; ?> />
                  <input value="10" type="radio" class="formfld" title="5" alt="5" name="rad_filter_sens" <?= $DATA["SEDATION_10"]; ?> />
                  &nbsp;Assume Good (Fewer in Quarantine)
                </span>
              </nobr>
            </p>
          </td>
        </tr>
        <tr>
          <td colspan="2" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="top" class="listtopic" colspan="2">
            <strong>Message Handling</strong> &ndash; Configure how SPAM is handled
          </td>
        </tr>
        <tr>
          <td align="left" valign="top" class="vncell" colspan="2">
            <p>When a SPAM message is identified:</p>
            <p>
              <input value="quarantine" <?= $DATA["S_ACTION_QUARANTINE"]; ?> type="radio" class="formfld" title="Quarantine the message" alt="Quarantine the message" name="rad_ident_action" id="rad_ident_action_one" />
              <label for="rad_ident_action_one">&nbsp;Quarantine the message</label>
              <br />
              <input value="tag" <?= $DATA["S_ACTION_TAG"]; ?> type="radio" class="formfld" title="Tag the Subject header with" alt="Tag the Subject header with" name="rad_ident_action" id="rad_ident_action_two" />
              <label for="rad_ident_action_two">Tag the Subject header with&nbsp;</label>
              <input size="35" type="text" class="formfld mail" title="message tag" alt="message tag" value="<?= $DATA["SPAM_SUBJECT"]; ?>" name="msgtag" />
              <br />
              <input value="deliver" <?= $DATA["S_ACTION_DELIVER"]; ?> type="radio" class="formfld" title="Deliver the message normally with a X-DSPAM-Result header" alt="Deliver the message normally with a X-DSPAM-Result header" name="rad_ident_action" id="rad_ident_action_three" />
              <label for="rad_ident_action_three">&nbsp;Deliver the message normally with a X-DSPAM-Result header</label>
            </p>
          </td>
        </tr>
        <tr>
          <td colspan="2" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="top" class="listtopic" colspan="2">
            <strong>Features</strong> &ndash; Tuning SPAM filtering
          </td>
        </tr>
        <tr>
          <td align="left" valign="top" class="vncell" colspan="2">
            <p>
              <input <?= $DATA["C_BNR"]; ?> type="checkbox" class="formbtn" title="Enable noise reduction, which usually improves filtering accuracy" alt="Enable noise reduction, which usually improves filtering accuracy" name="chk_feature_nr" id="chk_feature_nr" />
              <label for="chk_feature_nr">&nbsp;Enable noise reduction, which usually improves filtering accuracy</label>
              <br />
              <input <?= $DATA["C_WHITELIST"]; ?> type="checkbox" class="formbtn" title="Enable automatic whitelisting to record frequent correspondence" alt="Enable automatic whitelisting to record frequent correspondence" name="chk_feature_aw" id="chk_feature_aw" />
              <label for="chk_feature_aw">&nbsp;Enable automatic whitelisting to record frequent correspondence</label>
              <br />
              <input <?= $DATA["C_FACTORS"]; ?> type="checkbox" class="formbtn" title="Add the factoring tokens in each email into the message's full headers" alt="Add the factoring tokens in each email into the message's full headers" name="chk_feature_at" id="chk_feature_at" />
              <label for="chk_feature_at">&nbsp;Add the factoring tokens in each email into the message's full headers</label>
              <!--
              <input type="checkbox" class="formbtn" title="Add the factoring tokens in each email into the message's full headers" alt="Add the factoring tokens in each email into the message's full headers" name="chk_feature_at" id="chk_feature_at" />
              <label for="chk_feature_at">&nbsp;Add the factoring tokens in each email into the message's full headers</label>
              <br />
              <input type="checkbox" class="formbtn" title="opt in of DSPAM filtering" alt="opt in of DSPAM filtering" name="chk_feature_optin" id="chk_feature_optin" />
              <label for="chk_feature_optin">&nbsp;opt in of DSPAM filtering</label>
              <br />
              <input type="checkbox" class="formbtn" title="opt out of DSPAM filtering" alt="opt out of DSPAM filtering" name="chk_feature_optout" id="chk_feature_optout" />
              <label for="chk_feature_optout">&nbsp;opt out of DSPAM filtering</label>
              -->
            </p>
          </td>
        </tr>
        <tr>
          <td colspan="2" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td class="list">&nbsp;</td>
          <td class="list">
            <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" />
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