<?php
/* $Id$ */
/*
  dspam-train.php

  Copyright (C) 2006 Daniel S. Haischt.
  All rights reserved.
*/

$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("User Statistics"));

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
<form action="dspam-admin-stats.php" method="post" name="iform" id="iform">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<p>
  <span class="vexpl">
    The following table shows the number of messages processed for each user
    along with their current preference settings.
  </span>
</p>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
<?php
  $tab_array = array();
  $tab_array[] = array("System Status",   false,  "/dspam-admin.php?user={$CURRENT_USER}");
  $tab_array[] = array("User Statistics", true,  "/dspam-admin-stats.php?user={$CURRENT_USER}");
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
          <td align="left" valign="top" class="listhdrr" colspan="14">Statistical SPAM Protection for...</td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">Username</td>
          <td width="78%" class="vtable" colspan="13">
            <strong><?= $CURRENT_USER ?></strong>
          </td>
        </tr>
        <tr>
          <td colspan="14" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td align="left" valign="top" class="listtopic">Name</td>
          <td align="left" valign="top" class="listtopic">Q.Size</td>
          <td align="left" valign="top" class="listtopic">TP</td>
          <td align="left" valign="top" class="listtopic">TN</td>
          <td align="left" valign="top" class="listtopic">FP</td>
          <td align="left" valign="top" class="listtopic">FN</td>
          <td align="left" valign="top" class="listtopic">SC</td>
          <td align="left" valign="top" class="listtopic">IC</td>
          <td align="left" valign="top" class="listtopic">Mode</td>
          <td align="left" valign="top" class="listtopic">On Spam</td>
          <td align="left" valign="top" class="listtopic">BNR</td>
          <td align="left" valign="top" class="listtopic">Whitelist</td>
          <td align="left" valign="top" class="listtopic">Sed</td>
          <td align="left" valign="top" class="listtopic">Sig Loc</td>
        </tr>
        <?= $DATA['TABLE']; ?>
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