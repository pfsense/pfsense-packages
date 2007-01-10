<?php
/* $Id$ */
/* ========================================================================== */
/*
    disks_mount_tools.php
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
                 gettext("Mount Point"),
                 gettext("Tools"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['mounts']['mount']))
  $freenas_config['mounts']['mount'] = array();

mount_sort();

$a_mount = &$freenas_config['mounts']['mount'];

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  unset($do_action);
  
  /* input validation */
  $reqdfields = explode(" ", "fullname action");
  $reqdfieldsn = explode(",", "Fullname,Action");
  do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

  if (is_array($error_bucket))
    foreach($error_bucket as $elem)
      $input_errors[] =& $elem["error"];
  
  /* if this is an AJAX caller then handle via JSON */
  if(isAjax() && is_array($error_bucket)) {
      input_errors2Ajax(NULL, $error_bucket);       
      exit;   
  }
  
  if(!$input_errors)
  {
    $do_action = true;
    $fullname = $_POST['fullname'];
    $action = $_POST['action'];
  }
}

if(!isset($do_action))
{
  $do_action = false;
  $fullname = '';
  $action = '';
}

// URL GET from the disks_manage_init.php page:
// we get the $disk value, must found the $fullname now
if(isset($_GET['disk'])) {
  $disk = $_GET['disk'];
  $id = array_search_ex($disk, $a_mount, "mdisk");
  
  $fullname = $a_mount[$id]['fullname'];
}
if(isset($_GET['action'])) {
  $action = $_GET['action'];
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

<form action="disks_mount_tools.php" method="post" name="iform" id="iform">
<div id="inputerrors"></div>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
<?php
  $tab_array = array();
  $tab_array[0] = array(gettext("Manage"), false, "disks_mount.php");
  $tab_array[1] = array(gettext("Tools"),  true,  "disks_mount_tools.php");
  display_top_tabs($tab_array);
?>
  </td></tr>
  <tr> 
    <td>
      <div id="mainarea">
      <?php if ($input_errors) print_input_errors($input_errors); ?>
        <table class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
          <tr> 
            <td valign="top" class="vncellreq"><?= gettext("Share Name"); ?></td>
            <td class="vtable">
              <select name="fullname" class="formselect" id="fullname">
                <?php foreach ($a_mount as $mountv): ?>
                <option value="<?=$mountv['fullname'];?>"<?php if ($mountv['fullname'] == $fullname) echo "selected";?>>
                <?php echo htmlspecialchars($mountv['sharename'] . " (" . gettext("Disk") . ": " . $mountv['mdisk'] . " " . gettext("Partition") . ": " . $mountv['partition'] . ")");?>
                <?php endforeach; ?>
                </option>
              </select>
            </td>
          </tr>
          <tr>
            <td valign="top" class="vncellreq"><?= gettext("Command"); ?></td>
            <td class="vtable"> 
              <select name="action" class="formselect" id="action">
                <option value="mount" <?php if ($action == "mount") echo "selected"; ?>>mount</option>
                <option value="umount" <?php if ($action == "umount") echo "selected"; ?>>umount</option>
               </select>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top">&nbsp;</td>
            <td width="78%">
              <input name="Submit" type="submit" class="formbtn" value="<?= gettext("Send Command!"); ?>" />
            </td>
          </tr>
          <tr>
            <td valign="top" colspan="2">
            <?php if($do_action)
            {
              echo("<strong>" . gettext("Command output") . ": </strong><br />");
              echo('<pre>');
              ob_end_flush();
  
              /* Get the id of the mount array entry. */
              $id = array_search_ex($fullname, $a_mount, "fullname");
              /* Get the mount data. */
              $mount = $a_mount[$id];
  
              switch($action)
              {
                case "mount":
                  echo(gettext("Mounting '{$fullname}'...") . "<br />");
                  $result = disks_mount_fullname($fullname);
                  break;
                case "umount":
                  echo(gettext("Umounting '{$fullname}'...") . "<br />");
                  $result = disks_umount_fullname($fullname);
                  break;
              }
  
              /* Display result */
              echo((0 == $result) ? gettext("Successful") : gettext("Failed"));
  
              echo('</pre>');
            }
            ?>
            </td>
          </tr>
        </table>
      </div>
    </td>
  </tr>
  </table>
  </form>
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
</body>
</html>
