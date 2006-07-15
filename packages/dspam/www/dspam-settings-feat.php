<?php
/* $Id$ */
/*
	dspam-settings-feat.php

	Copyright (C) 2006 Daniel S. Haischt.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("Advanced Settings"),
                 gettext("Edit DSPAM Feature"));

require("guiconfig.inc");
include("/usr/local/pkg/dspam.inc");

if (isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER'])) {

/*
  The following code presumes, that the following XML structure exists or
  if it does not exist, it will be created.

    <feature>
      <name>foo</name>
      <descr>foo desc</descr>
    </feature>
    <feature>
      <name>bar</name>
      <descr>bar desc</descr>
    </feature>
*/

if (!is_array($config['installedpackages']['dspam']['config'][0]['feature'])) {
  $config['installedpackages']['dspam']['config'][0]['feature'] = array();
}

$t_features = &$config['installedpackages']['dspam']['config'][0]['feature'];

/* ID is only set if the user wants to edit an existing entry */
$id = $_GET['id'];
$sectionid = $_GET['sectionid'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (isset($_POST['sectionid']))
	$sectionid = $_POST['sectionid'];

if (isset($id) && $t_features[$id]) {
        $pconfig['name'] = $t_features[$id]['name'];
        $pconfig['descr'] = $t_features[$id]['descr'];
} else {
        $pconfig['name'] = $_GET['fname'];
        $pconfig['descr'] = $_GET['descr'];
}

if ($_POST) {

	unset($input_errors);
	$pconfig = $_POST;

	/* input validation */
	$reqdfields = explode(" ", "fname");
	$reqdfieldsn = explode(",", "DSPAM Feature Name");

	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	/* check for overlaps */
	foreach ($t_features as $feature) {
		if (isset($id) && ($t_features[$id]) && ($t_features[$id] === $feature)) {
			continue;
		}
		if ($feature['name'] == $_POST['fname']) {
			$input_errors[] = gettext("This feature name already exists.");
			break;
		}
	}

	/* if this is an AJAX caller then handle via JSON */
	if(isAjax() && is_array($input_errors)) {
		input_errors2Ajax($input_errors);
		exit;
	}

	if (!$input_errors) {
		$feature = array();
		$feature['name'] = $_POST['fname'];
		$feature['descr'] = $_POST['descr'];

		if (isset($id) && $t_features[$id])
			$t_features[$id] = $feature;
		else
			$t_features[] = $feature;

		write_config();

    $retval = 0;
    config_lock();
    $retval = dspam_configure();
    config_unlock();

    $savemsg = get_std_save_message($retval);

    isset($sectionid) ? $header = "dspam-settings.php?sectionid={$sectionid}" : $header = "dspam-settings.php";
		pfSenseHeader($header);
		exit;
  }
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
<?php if ($input_errors) print_input_errors($input_errors); ?>
            <form action="dspam-settings-feat.php" method="post" name="iform" id="iform">
            <div name="inputerrors" id="inputerrors"></div>
              <table width="100%" border="0" cellpadding="6" cellspacing="0">
                <tr>
                  <td width="22%" valign="top" class="vncellreq"><?=gettext("DSPAM Feature Name");?></td>
                  <td width="78%" class="vtable">
                    <!-- <input name="fname" type="text" class="formfld" id="fname" size="30" value="<?=htmlspecialchars($pconfig['name']);?>"> -->
                    <select name="fname" id="fname" class="formselect">
                      <option value="sbph" <?php if($pconfig['name'] == "sbph") echo('selected=\"selected\"');?>>sbph</option>
                      <option value="noise" <?php if($pconfig['name'] == "noise") echo('selected=\"selected\"');?>>noise</option>
                      <option value="chained" <?php if($pconfig['name'] == "chained") echo('selected=\"selected\"');?>>chained</option>
                      <option value="whitelist" <?php if($pconfig['name'] == "whitelist") echo('selected=\"selected\"');?>>whitelist</option>
                    </select>
                  </td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell"><?=gettext("Description");?></td>
                  <td width="78%" class="vtable">
                    <input name="descr" type="text" class="formfld unknown" id="descr" size="40" value="<?=htmlspecialchars($pconfig['descr']);?>">
                    <br> <span class="vexpl"><?=gettext("You may enter a description here
                    for your reference (not parsed).");?></span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top">&nbsp;</td>
                  <td width="78%">
                    <input id="submit"  name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>"> <input id="cancelbutton" class="formbtn" type="button" value="<?=gettext("Cancel");?>" onclick="history.back()">
                    <?php if (isset($id) && $t_features[$id]): ?>
                    <input name="id" type="hidden" value="<?=$id;?>">
                    <?php endif; ?>
                    <?php if (isset($sectionid)): ?>
                    <input name="sectionid" type="hidden" value="<?=$sectionid;?>">
                    <?php endif; ?>
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
