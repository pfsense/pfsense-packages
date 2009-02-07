<?php
/* $Id$ */
/*
  dspam-train.php

  Copyright (C) 2006 Daniel S. Haischt.
  All rights reserved.
*/

$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("Train Filter"));

/* omit no-cache headers because it confuses IE with file downloads */
$omit_nocacheheaders = true;
require("guiconfig.inc");
include("/usr/local/pkg/dspam.inc");

function prepare_compressed_mbox_data($type) {
  global $CURRENT_USER;
  /* prepare directories */
  mwexec("mkdir -m 0755 -p /tmp/dspam-data/{$CURRENT_USER}/mbox");
  mwexec("mkdir -m 0755 -p /tmp/dspam-data/{$CURRENT_USER}/mdir");

  switch ($type) {
    case 0:
      move_uploaded_file($_FILES['archfile']['tmp_name'], "/tmp/" . $_FILES['archfile']['name']);
      mwexec("/usr/local/bin/unzip /tmp/{$_FILES['archfile']['name']} -d /tmp/dspam-data/{$CURRENT_USER}/mbox");
      unlink("/tmp/{$_FILES['archfile']['name']}");
      mwexec("/usr/local/bin/mb2md -s /tmp/dspam-data/{$CURRENT_USER}/mbox -R -d /tmp/dspam-data/{$CURRENT_USER}/mdir");
      break;
    case 1:
      move_uploaded_file($_FILES['archfile']['tmp_name'], "/tmp/dspam-data/" . $_SESSION['Username'] . "/mbox/" . $_FILES['archfile']['name']);
      mwexec("/usr/bin/gunzip /tmp/dspam-data/{$CURRENT_USER}/mbox/{$_FILES['archfile']['name']}");
      mwexec("/usr/local/bin/mb2md -s /tmp/dspam-data/{$CURRENT_USER}/mbox -R -d /tmp/dspam-data/{$CURRENT_USER}/mdir");
      break;
    case 2:
      move_uploaded_file($_FILES['archfile']['tmp_name'], "/tmp/dspam-data/" . $_SESSION['Username'] . "/mbox/" . $_FILES['archfile']['name']);
      mwexec("/usr/bin/bunzip2 /tmp/dspam-data/{$CURRENT_USER}/mbox/{$_FILES['archfile']['name']}");
      mwexec("/usr/local/bin/mb2md -s /tmp/dspam-data/{$CURRENT_USER}/mbox -R -d /tmp/dspam-data/{$CURRENT_USER}/mdir");
      break;
  }
}

function prepare_compressed_mdir_data($type) {
  global $CURRENT_USER;
  /* prepare directories */
  mwexec("mkdir -m 0755 -p /tmp/dspam-data/{$CURRENT_USER}/mdir");

  switch ($type) {
    case 0:
      move_uploaded_file($_FILES['archfile']['tmp_name'], "/tmp/" . $_FILES['archfile']['name']);
      mwexec("/usr/local/bin/unzip /tmp/{$_FILES['archfile']['name']} -d /tmp/dspam-data/{$CURRENT_USER}/mdir");
      unlink("/tmp/{$_FILES['archfile']['name']}");
      break;
    case 1:
      move_uploaded_file($_FILES['archfile']['tmp_name'], "/tmp/dspam-data/" . $_SESSION['Username'] . "/mdir/" . $_FILES['archfile']['name']);
      mwexec("/usr/bin/gunzip /tmp/dspam-data/{$CURRENT_USER}/mdir/{$_FILES['archfile']['name']}");
      break;
    case 2:
      move_uploaded_file($_FILES['archfile']['tmp_name'], "/tmp/dspam-data/" . $_SESSION['Username'] . "/mdir/" . $_FILES['archfile']['name']);
      mwexec("/usr/bin/bunzip2 /tmp/dspam-data/{$CURRENT_USER}/mdir/{$_FILES['archfile']['name']}");
      break;
  }
}

function prepare_compressed_data($type) {
  if ($_POST['archformat'] == "mbox") {
    prepare_compressed_mbox_data($type);
  } else {
    prepare_compressed_mdir_data($type);
  }
}

if ($_POST) {
  unset($input_errors);

  if(! extension_loaded( 'fileinfo' )) {
    /* fileinfo pecl extension unavailable? */
    if(! @dl( 'fileinfo.so' )) {
      if ($_POST['cotype'] == "zip") {
        prepare_compressed_data(0);
      } else if ($_POST['cotype'] == "gzip") {
        prepare_compressed_data(1);
      } else if ($_POST['cotype'] == "bzip") {
        prepare_compressed_data(2);
      } else {
        $input_errors[] = "unable to determine compression type.";
      }
    } else {
      if (is_uploaded_file($_FILES['archfile']['tmp_name'])) {
        $info = finfo_open( FILEINFO_MIME, '/usr/share/misc/magic' );
        $type = finfo_file( $info, $_FILES['archfile']['tmp_name'] );

        if ($type == "application/x-zip") {
          prepare_compressed_data(0);
        } else if ($type == "application/x-gzip") {
          prepare_compressed_data(1);
        } else if ($type == "application/x-bzip2") {
          prepare_compressed_data(2);
        } else {
          $input_errors[] = "unable to determine compression type.";
        }
      }
    }
  }

  /* tell DSPAM to train the messages contained within the maildir */
  if ($_POST['msgtype'] == "spam") {
    mwexec("find /tmp/dspam-data/{$CURRENT_USER}/mdir -name '*' -exec /usr/local/bin/dspam_spamfeed {$CURRENT_USER} {} \\;");
  } else if ($_POST['msgtype'] == "ham") {
    mwexec("find /tmp/dspam-data/{$CURRENT_USER}/mdir -name '*' -exec /usr/local/bin/dspam_innocentfeed {$CURRENT_USER} {} \\;");
    mwexec("rm -rf /tmp/dspam-data");
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
<form action="dspam-train.php" method="post" name="iform" id="iform" enctype="multipart/form-data">
<input type="hidden" name="MAX_FILE_SIZE" value="<?= (diskfreespace('/') - (10 * pow(10, 6))); ?>">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
<?php
  $tab_array = array();
  $tab_array[] = array("Info",         false, "/dspam.php?user={$CURRENT_USER}");
  $tab_array[] = array("Performance",  false, "/dspam-perf.php?user={$CURRENT_USER}");
  $tab_array[] = array("Preferences",  false, "/dspam-prefs.php?user={$CURRENT_USER}");
  $tab_array[] = array("Alerts",       false, "/pkg.php?xml=dspam_alerts.xml&user={$CURRENT_USER}");
  $tab_array[] = array("Quarantine ({$DATA['TOTAL_QUARANTINED_MESSAGES']})",   false, "/dspam-quarantine.php?user={$CURRENT_USER}");
  $tab_array[] = array("Analysis",     false, "/dspam-analysis.php?user={$CURRENT_USER}");
  $tab_array[] = array("History",      false, "/dspam-history.php?user={$CURRENT_USER}");
  $tab_array[] = array("Train Filter", true,  "/dspam-train.php?user={$CURRENT_USER}");
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
          <td colspan="2" class="listtopic"><?=gettext("Upload Message Archive");?></td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">Archive Type</td>
          <td width="78%" class="vtable">
          <?php if($config['installedpackages']['mb2md'] || file_exists('/usr/local/bin/mb2md')): ?>
            <input type="radio" name="archformat" id="mbxformat" value="mbox" class="formfld" />
            &nbsp;<a href="http://en.wikipedia.org/wiki/Mbox" target="_blank">Mailbox</a> format (like it is used for example by Mozilla Thunderbird)
            <br />
          <?php endif; ?>
            <input type="radio" name="archformat" id="mdirformat" value="mdir" class="formfld" checked="checked"/>
            &nbsp;<a href="http://en.wikipedia.org/wiki/Maildir" target="_blank">Maildir</a> format (like it was initially introduced by qmail)
            <p>
              <strong>
                <span class="red"><?=gettext("Note");?>:</span>
              </strong>
              <br />
              <?=gettext("DSPAM is only able to handle Maildir message archives natively. Mailbox message archives need to be converted (the conversion from mbx to maildir will be done on the fly while you are uploading a message archive).");?>
              <br />
            </p>
          </td>
        </tr>
        <?php if(! extension_loaded( 'fileinfo' )): ?>
        <?php if(! @dl( 'fileinfo.so' )): ?>
        <tr>
          <td width="22%" valign="baseline" class="vncell">Compression Type</td>
          <td width="78%" class="vtable">
            <input type="radio" name="cotype" id="ziptype" value="zip" class="formfld" />
            &nbsp;Archive was compressed using a ZIP algorithm.
            <br />
            <input type="radio" name="cotype" id="gziptype" value="gzip" class="formfld" />
            &nbsp;Archive was compressed using a GNU ZIP algorithm.
            <br />
            <input type="radio" name="cotype" id="bziptype" value="bzip" class="formfld" checked="checked" />
            &nbsp;Archive was compressed using a bzip2 algorithm
            <br />
          </td>
        </tr>
        <?php endif; ?>
        <?php endif; ?>
        <tr>
          <td width="22%" valign="baseline" class="vncell">Message Type</td>
          <td width="78%" class="vtable">
            <input type="radio" name="msgtype" id="spamtype" value="spam" class="formfld" checked="checked" />
            &nbsp;Archive to be uploaded contains Spam messages.
            <br />
            <input type="radio" name="msgtype" id="hamtype" value="ham" class="formfld" />
            &nbsp;Archive to be uploaded contains Ham messages.
            <br />
          </td>
        </tr>
        <tr>
          <td width="22%" valign="baseline" class="vncell">&nbsp;</td>
          <td width="78%" class="vtable">
            <p>
              <?=gettext("Open a Ham or Spam message archive (please either zip, gzip or bzip your files).");?>
            </p>
            <p>
              <input name="archfile" type="file" class="formfld file" id="archfile" size="40" maxlength="<?= (diskfreespace('/') - (10 * pow(10, 6))); ?>" />
            </p>
            <p>
              <input name="Submit" type="submit" class="formbtn" id="restore" value="<?=gettext("Upload Message Archive");?>" />
            </p>
            <p>
              <strong>
                <span class="red"><?=gettext("Note");?>:</span>
              </strong>
              <br />
              <?=gettext("It may take a long time until the filter stops training, if you are going to upload a huge archive. Therefore the the allowed filesize to be uploaded has been set to " . ((diskfreespace('/') - (10 * pow(10, 6))) / pow(10, 6)) . " MByte (available space minus 10 MByte).");?>
              <br />
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