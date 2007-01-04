<?php 
/* $Id$ */
/* ========================================================================== */
/*
    disks_manage_tools.php
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
                 gettext("Tools"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['disks']['disk']))
  $freenas_config['disks']['disk'] = array();

disks_sort();

$a_disk_conf = &$freenas_config['disks']['disk'];

if ($_POST) {
  unset($input_errors);
  unset($do_action);
  
  /* input validation */
  $reqdfields = explode(" ", "disk action");
  $reqdfieldsn = explode(",", "Disk,Action");
  do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
  
  if (!$input_errors)
  {
    $do_action = true;
    $disk = $_POST['disk'];
    $action = $_POST['action'];
    $partition = $_POST['partition'];
    $umount = $_POST['umount'];
  }
}
  
if (!isset($do_action))
{
  $do_action = false;
  $disk = '';
  $action = '';
  $partition = '';
  $umount = false;
}

/* if ajax is calling, give them an update message */
if(isAjax())
  print_info_box_np($savemsg);

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
$pfSenseHead->setCloseHead(false);
echo $pfSenseHead->getHTML();

?>
<script type="text/javascript">
<!--
function disk_change() {
  var next = null;
  // Remove all entries from partition combobox.
  document.iform.partition.length = 0;
  // Insert entries for partition combobox.
  switch(document.iform.disk.value)
  {
    <?php foreach ($a_disk as $diskv): ?>
    case "<?=$diskv['name'];?>":
      <?php $partinfo = disks_get_partition_info($diskv['name']);?>
      <?php foreach($partinfo as $partinfon => $partinfov): ?>
        if(document.all) // MS IE workaround.
          next = document.iform.partition.length;
        document.iform.partition.add(new Option("<?=$partinfon;?>","s<?=$partinfon;?>",false,<?php if("s{$partinfon}"==$partition){echo "true";}else{echo "false";};?>), next);
      <?php endforeach; ?>
      break;
    <?php endforeach; ?>
  }
}
// -->
</script>
</head>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>

<form action="disks_manage_tools.php" method="post" name="iform" id="iform">
<div id="inputerrors"></div>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
<?php
  $tab_array = array();
  $tab_array[0] = array(gettext("Manage"),          false, "disks_manage.php");
  $tab_array[1] = array(gettext("Format"),          false, "disks_manage_init.php");
  $tab_array[2] = array(gettext("Tools"),           true,  "disks_manage_tools.php");
  $tab_array[3] = array(gettext("iSCSI Initiator"), false, "disks_manage_iscsi.php");
  display_top_tabs($tab_array);
?>  
  </td></tr>
  <tr> 
    <td>
      <div id="mainarea">
      <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
        <tr>
          <td width="22%" valign="top" class="vncellreq"><?=gettext("Disk");?></td>
          <td width="78%" class="vtable">
            <select name="disk" class="formfld" id="disk" onchange="disk_change()">
              <?php foreach ($a_disk as $diskn): ?>
              <option value="<?=$diskn['name'];?>"<?php if ($diskn['name'] == $disk) echo "selected";?>>
              <?php echo htmlspecialchars($diskn['name'] . ": " .$diskn['size'] . " (" . $diskn['desc'] . ")");?>
              <?php endforeach; ?>
              </option>
            </select>
          </td>
        </tr>
        <tr>
          <td width="22%" valign="top" class="vncellreq"><?=gettext("Partition");?></td>
          <td width="78%" class="vtable">
            <td valign="top" class="vncellreq"><?=_PARTITION;?></td>
            <td class="vtable"> 
              <select name="partition" class="formfld" id="partition"></select>
            </td>
          </td>
        </tr>
        <tr>
          <td width="22%" valign="top" class="vncellreq"><?=gettext("Command");?></td>
          <td width="78%" class="vtable">
            <td valign="top" class="vncellreq"><?=_PARTITION;?></td>
            <td class="vtable"> 
              <select name="action" class="formfld" id="action">
                <option value="fsck" <?php if ($action == "fsck") echo "selected"; ?>>fsck</option>
               </select>
            </td>
          </td>
        </tr>
        <tr>
          <td width="22%" valign="top" class="vncellreq"></td>
          <td width="78%" class="vtable">
            <td valign="top" class="vncellreq"><?=_PARTITION;?></td>
            <td class="vtable"> 
              <input name="umount" type="checkbox" id="umount" value="yes" <?php if ($umount) echo "checked"; ?> />
              <strong>
                <?= gettext("Unmount disk/partition"); ?>
              </strong>
              <span class="vexpl">
                <br />
                <?= gettext("If the selected disk/partition is mounted it will be unmounted temporary to perform selected command, otherwise the commands work in read-only mode."); ?>
              </span>
            </td>
          </td>
        </tr>
        <tr>
          <td width="22%" valign="top" class="vncellreq">&nbsp;</td>
          <td width="78%" class="vtable">
            <td valign="top" class="vncellreq"><?=_PARTITION;?></td>
            <td class="vtable"> 
              <input name="Submit" type="submit" class="formbtn" value="<?= gettext("Send Command!"); ?>">
            </td>
          </td>
        </tr>
        <tr>
          <td width="22%" valign="top" class="vncellreq">&nbsp;</td>
          <td width="78%" class="vtable">
            <td valign="top" class="vncellreq"><?=_PARTITION;?></td>
            <td class="vtable"> 
            <?php
            if($do_action)
            {
              echo("<strong>" . gettext("Command output:") . "</strong><br>");
              echo('<pre>');
              ob_end_flush();
  
              switch($action)
              {
                case "fsck":
                  /* Get the id of the disk. */
                  $id = array_search_ex($disk, $a_disk, "name");
                  /* Get the filesystem type of the disk. */ 
                  $type = $a_disk[$id]['fstype'];
                  /* Check if disk is mounted. */
                  $ismounted = disks_check_mount($disk,$partition);
  
                  /* Umount disk if necessary. */
                  if($umount && $ismounted) {
                    echo("<strong class='red'>" . gettext("Note") . ":</strong> " . gettext("The disk is currently mounted! The mount point will be removed temporary to perform selected command.") . "<br><br>");
                    disks_umount_ex($disk,$partition);
                  }
  
                  switch($type)
                  {
                    case "":
                    case "ufs":
                    case "ufs_no_su":
                    case "ufsgpt":
                    case "ufsgpt_no_su":
                      system("/sbin/fsck_ufs -y -f /dev/" . escapeshellarg($disk . $partition));
                      break;
                    case "gmirror":
                    case "gvinum":
                    case "graid5":
                      $infomsg = sprintf(gettext("Use <a href='%s'>RAID tools</a> for this disk!"), "disks_raid_{$type}_tools.php");
                      print_info_box_np($infomsg);
                      break;
                    case "msdos":
                      system("/sbin/fsck_msdosfs -y -f /dev/" . escapeshellarg($disk . $partition));
                      break;
                  }
  
                  /* Mount disk if necessary. */
                  if($umount && $ismounted) {
                    disks_mount_ex($disk,$partition);
                  }
  
                  break;
              }
              echo('</pre>');
            }
            ?>
            </td>
          </td>
        </tr>
      </table>
      </div>
    </td>
  </tr>
</table>
</form>
<script type="text/javascript">
<!--
  disk_change();
//-->
</script>
<?php include("fend.inc"); ?>
</body>
</html>
