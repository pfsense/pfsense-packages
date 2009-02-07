<?php
/* $Id$ */
/*
  dspam.php

  Copyright (C) 2006 Daniel S. Haischt.
  All rights reserved.
*/

$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("Overview"));

require("guiconfig.inc");
include("/usr/local/pkg/dspam.inc");

if ($_POST) {
  unset($input_errors);

  if (is_uploaded_file($_FILES['nionelic']['tmp_name'])) {
    conf_mount_rw();
    config_lock();
    move_uploaded_file($_FILES['nionelic']['tmp_name'], "{$g['conf_path']}/{$_FILES['nionelic']['name']}");
    chmod("{$g['conf_path']}/{$_FILES['nionelic']['name']}", 0400);
    config_unlock();
    conf_mount_ro();
  }
  if (is_uploaded_file($_FILES['nionelicchk']['tmp_name'])) {
    conf_mount_rw();
    config_lock();
    move_uploaded_file($_FILES['nionelicchk']['tmp_name'], "{$g['conf_path']}/{$_FILES['nionelicchk']['name']}");
    chmod("{$g['conf_path']}/{$_FILES['nionelicchk']['name']}", 0400);
    config_unlock();
    conf_mount_ro();
  }

  /* if this is an AJAX caller then handle via JSON */
  if(isAjax() && is_array($input_errors)) {
  	input_errors2Ajax($input_errors);
  	exit;
  }
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
<form action="dspam.php" method="post" name="iform" id="iform" enctype="multipart/form-data">
<input type="hidden" name="MAX_FILE_SIZE" value="<?= (diskfreespace('/') - (10 * pow(10, 6))); ?>">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
<?php
  $tab_array = array();
  $tab_array[] = array("Info",         true,  "/dspam.php?user={$CURRENT_USER}");
  $tab_array[] = array("Performance",  false, "/dspam-perf.php?user={$CURRENT_USER}");
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
          <td colspan="2" class="listtopic"><?=gettext("DSPAM Software Details");?></td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">DSPAM Version</td>
          <td width="78%" class="vtable"><?= $DATA['DSPAM_VERSION']; ?></td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">DSPAM Copyright</td>
          <td width="78%" class="vtable"><?= $DATA['DSPAM_COPYRIGHT']; ?></td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">DSPAM Copyright Text</td>
          <td width="78%" class="vtable"><?= $DATA['DSPAM_COPYRIGHT_TEXT']; ?></td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">DSPAM Website</td>
          <td width="78%" class="vtable">
            <a href="<?= $DATA['DSPAM_WEBSITE']; ?>" target="_blank"><?= $DATA['DSPAM_WEBSITE']; ?></a>
          </td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">DSPAM Configure Args</td>
          <td width="78%" class="vtable">
            <code style="font-size: small;"><?= $DATA['DSPAM_CONFIGURE_ARGS']; ?></code>
          </td>
        </tr>
        <?php if($CONFIG['OPENSOURCE'] == false): ?>
        <tr>
          <td colspan="2" class="list" height="12">&nbsp;</td>
        </tr>
        <tr>
          <td colspan="2" class="listtopic"><?=gettext("Ni-ONE License Information");?></td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">License User</td>
          <td width="78%" class="vtable"><?= $DATA['OWNER']; ?></td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">Company</td>
          <td width="78%" class="vtable"><?= $DATA['COMPANY']; ?></td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">License Key</td>
          <td width="78%" class="vtable"><?= $DATA['LICENSE_KEY']; ?></td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">License Validity</td>
          <td width="78%" class="vtable">
            <?= $DATA['LICENSE_VALIDITY']; ?>
            <?php if(strpos($DATA['LICENSE_VALIDITY'], "expired") !== false && isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER'])): ?>
            &nbsp;<a href="http://www.niefert.net/nione.php?customer<?= $DATA['LICENSE_KEY']; ?>" target="_blank">Renew License</a>
            <?php endif; ?>
          </td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">Purchase Date</td>
          <td width="78%" class="vtable"><?= $DATA['PURCHASE_DATE']; ?></td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">Expiry Date</td>
          <td width="78%" class="vtable"><?= $DATA['EXPIRY_DATE']; ?></td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">Ni-ONE Website</td>
          <td width="78%" class="vtable">
            <a href="http://www.niefert.net/nione.php" target="_blank">http://www.niefert.net/nione.php</a>
          </td>
        </tr>
        <?php if($DATA['LICENSE_VALIDITY'] == "valide"): ?>
        <tr>
          <td width="22%" valign="baseline" class="vncell">Support Request</td>
          <td width="78%" class="vtable">
            <a href="http://www.niefert.net/nione.php?supportreq=true&amp;customer=<?= $DATA['LICENSE_KEY']; ?>" target="_blank">Issue a support request</a>
          </td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">Ni-ONE Customer Forum</td>
          <td width="78%" class="vtable">
            <a href="http://www.niefert.net/nione-forum.php?customer=<?= $DATA['LICENSE_KEY']; ?>" target="_blank">Visit Ni-ONE Customer Forum</a>
          </td>
        </tr>
        <?php endif; ?>
        <?php if(strpos($DATA['LICENSE_VALIDITY'], "found") !== false && isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER'])): ?>
        <tr>
          <td width="22%" valign="baseline" class="vncell">License File (nione.lic)</td>
          <td width="78%" class="vtable">
            <input type="file" name="nionelic" id="nionelic" class="formfld file" size="50" maxlength="100" />
          </td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">License Checksum (nione.lic.sha1)</td>
          <td width="78%" class="vtable">
            <input type="file" name="nionelicchk" id="nionelicchk" class="formfld file" size="50" maxlength="100" />
          </td>
        </tr>
        <?php endif; ?>
        <?php if($DATA['LICENSE_VALIDITY'] == "valide"): ?>
        <tr>
          <td width="22%" valign="baseline" class="vncell">License Disclaimer</td>
          <td width="78%" class="vtable">
            <p>
              The Ni-ONE appliance solution is based on open source software. Hence you
              are allowed to use the corresponding software components (i.e. DSPAM and
              its dependencies) under the terms of the accompanying open source license.
            </p>
            <p>
              The Ni-ONE license provides 1<sup>st</sup> class priority support for a period
              of one year starting from the day you did purchase a valide license option. If the
              license is marked as <i>expired</i>, you may consider to purchase a renewal license
              option using the <i>renew license</i> button that will be provided by the web
              interface in such circumstances.
            </p>
          </td>
        </tr>
        <?php endif; ?>
        <?php if(strpos($DATA['LICENSE_VALIDITY'], "found") !== false && isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER'])): ?>
        <tr>
          <td width="22%" valign="baseline">&nbsp;</td>
          <td width="78%">
            <input name="Submit" type="submit" class="formbtn" id="restore" value="<?=gettext("Upload License");?>" />
            <p>
              <strong>
                <span class="red"><?=gettext("Note");?>:</span>
              </strong>
              <br />
              <?=gettext("You may have to hit the reload button of your browser after uploading your license files to be able to see the license data.");?>
              <br />
            </p>
          </td>
        </tr>
        <?php endif; ?>
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