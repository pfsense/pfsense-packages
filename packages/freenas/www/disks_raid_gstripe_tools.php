<?php
/*
    disks_raid_stripe_tools.php
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2006 Daniel S. Haischt <me@daniel.stefan.haischt.name>
    All rights reserved.

    Based on FreeNAS (http://www.freenas.org)
    Copyright (C) 2005-2006 Olivier Cochard-Labbé <olivier@freenas.org>.
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
    All rights reserved.
                                                                              */
/* ========================================================================== */
/*
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
/* ========================================================================== */

$pgtitle = array(gettext("System"),
                 gettext("Disks"),
                 gettext("GEOM Stripe"),
                 gettext("Tools"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  unset($do_action);
  
  $reqdfields = explode(" ", "action object");
  $reqdfieldsn = explode(",", "Action,Object");
  
  do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
  
  if (is_array($error_bucket))
    foreach($error_bucket as $elem)
      $input_errors[] =& $elem["error"];
  
  /* if this is an AJAX caller then handle via JSON */
  if(isAjax() && is_array($error_bucket)) {
      input_errors2Ajax(NULL, $error_bucket);       
      exit;   
  }
  
  if (!$input_errors) {
    $do_action = true;
    $action = $_POST['action'];
    $object = $_POST['object'];
  }
  }
  if (!isset($do_action)) {
  $do_action = false;
  $action = '';
  $object = '';
}

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>

<div id="inputerrors"></div>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td class="tabnavtbl">
<?php
  $tab_array = array();
  $tab_array[0] = array(gettext("Geom Mirror"), false, "disks_raid_gmirror.php");
  $tab_array[1] = array(gettext("Geom Concat"), false, "disks_raid_gconcat.php");
  $tab_array[2] = array(gettext("Geom Stripe"), true,  "disks_raid_gstripe.php");
  $tab_array[3] = array(gettext("Geom RAID5"),  false, "disks_raid_graid5.php");
  $tab_array[4] = array(gettext("Geom Vinum"),  false, "disks_raid_gvinum.php");
  display_top_tabs($tab_array);
?>  
    </td>
  </tr>
  <tr>
    <td class="tabnavtbl">
<?php
  $tab_array = array();
  $tab_array[0] = array(gettext("Manage RAID"), false, "disks_raid_gstripe.php");
  /* $tab_array[1] = array(gettext("Format RAID"), false, "disks_raid_gstripe_init.php"); */
  $tab_array[1] = array(gettext("Tools"),       true,  "disks_raid_gstripe_tools.php");
  $tab_array[2] = array(gettext("Information"), false, "disks_raid_gstripe_infos.php");
  display_top_tabs($tab_array);
?> 
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
      <form action="disks_raid_gstripe_tools.php" method="post" name="iform" id="iform">
      <table class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
        <tr>
          <td width="22%" valign="top" class="vncellreq"><?=gettext("Object name");?></td>
          <td width="78%" class="vtable">
            <input name="object" type="text" class="formfld unknown" id="object" size="20" value="<?=htmlspecialchars($disk);?>" />
          </td>
        </tr>
        <tr>
          <td width="22%" valign="top" class="vncellreq"><?=gettext("Object name");?></td>
          <td width="78%" class="vtable">
            <select name="action" class="formselect" id="action">
              <option value="list" <?php if ($action == "list") echo "selected"; ?>>list</option>
              <option value="status" <?php if ($action == "status") echo "selected"; ?>>status</option>
              <option value="clear" <?php if ($action == "clear") echo "selected"; ?>>clear</option>
              <option value="stop" <?php if ($action == "stop") echo "selected"; ?>>stop</option>
             </select>
          </td>
        </tr>
        <tr> 
          <td width="22%" valign="top">&nbsp;</td>
          <td width="78%">
            <input name="Submit" type="submit" class="formbtn" value="Send Command!" /> 
          </td>
        </tr>
        <tr>
          <td valign="top" colspan="2">
            <?
              if ($do_action) {
                echo("<strong>" . gettext("GSTRIPE command output:") . "</strong><br />");
                echo('<pre>');
                ob_end_flush();
              
                system("/sbin/gstripe $action " . escapeshellarg($object));
            
                echo('</pre>');
              }
            ?>
          </td>
        </tr>
        <tr>
          <td align="left" valign="top" colspan="2">
            <span class="red">
              <strong>WARNING:</strong><br />
            </span>
            <ol>
              <li><span class="vexpl"><?= gettext("Use these specials actions for debugging only!"); ?></span></li>
              <li><span class="vexpl"><?= gettext("There is no need of using this menu for start a RAID volume (start automaticaly)."); ?></span></li>
            </ol>
          </td>
        </tr>
      </table>
      </form>
      </div>
    </td>
  </tr>
</table>
<?php include("fend.inc"); ?>
</body>
</html>
