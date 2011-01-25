<?php
/* $Id$ */
/*
	dspam-settings-prefs.php

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
                 gettext("Edit DSPAM Preferences"));

require("guiconfig.inc");
include("/usr/local/pkg/dspam.inc");

if (isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER'])) {

/*
  The following code presumes, that the following XML structure exists or
  if it does not exist, it will be created.

    <preference>
      <name>foo</name>
      <descr>foo desc</descr>
    </preference>
    <preference>
      <name>bar</name>
      <descr>bar desc</descr>
    </preference>
*/

if (!is_array($config['installedpackages']['dspam']['config'][0]['preference'])) {
  $config['installedpackages']['dspam']['config'][0]['preference'] = array();
}

$t_prefs = &$config['installedpackages']['dspam']['config'][0]['preference'];

/* ID is only set if the user wants to edit an existing entry */
$id = $_GET['id'];
$sectionid = $_GET['sectionid'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (isset($_POST['sectionid']))
	$sectionid = $_POST['sectionid'];

if (isset($id) && $t_prefs[$id]) {
        $pconfig['value'] = $t_prefs[$id]['value'];
        $pconfig['descr'] = $t_prefs[$id]['descr'];
} else {
        $pconfig['value'] = $_GET['pvalue'];
        $pconfig['descr'] = $_GET['descr'];
}

if ($_POST) {

	unset($input_errors);
	$pconfig = $_POST;

	/* input validation */
	$reqdfields = explode(" ", "pvalue");
	$reqdfieldsn = explode(",", "DSPAM Algorithm Name");

	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	/* check for overlaps */
	foreach ($t_prefs as $pref) {
		if (isset($id) && ($t_prefs[$id]) && ($t_prefs[$id] === $pref)) {
			continue;
		}
		if ($pref['value'] == $_POST['pvalue']) {
			$input_errors[] = gettext("This preference value already exists.");
			break;
		}
	}

	/* if this is an AJAX caller then handle via JSON */
	if(isAjax() && is_array($input_errors)) {
		input_errors2Ajax($input_errors);
		exit;
	}

	if (!$input_errors) {
		$pref = array();
		$pref['value'] = $_POST['pvalue'];
		$pref['descr'] = $_POST['descr'];

		if (isset($id) && $t_prefs[$id])
			$t_prefs[$id] = $pref;
		else
			$t_prefs[] = $pref;

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
            <form action="dspam-settings-prefs.php" method="post" name="iform" id="iform">
            <div name="inputerrors" id="inputerrors"></div>
              <table width="100%" border="0" cellpadding="6" cellspacing="0">
                <tr>
                  <td width="22%" valign="top" class="vncellreq"><?=gettext("DSPAM Algorithm Name");?></td>
                  <td width="78%" class="vtable">
                    <input name="pvalue" type="text" class="formfld unknown" id="pvalue" size="30" value="<?=htmlspecialchars($pconfig['value']);?>">
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
                    <input id="submit" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>"> <input id="cancelbutton" class="formbtn" type="button" value="<?=gettext("Cancel");?>" onclick="history.back()">
                    <?php if (isset($id) && $t_prefs[$id]): ?>
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
