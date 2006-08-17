<?php
/* $Id$ */
/*
	disks_manage_edit.php
	part of FreeNAS (http://www.freenas.org)
	Copyright (C) 2005-2006 Olivier Cochard-Labbé <olivier@freenas.org>.
	All rights reserved.
	
	Based on m0n0wall (http://m0n0.ch/wall)
	Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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

$pgtitle = array(gettext("System"),
                 gettext("Disks"),
                 gettext("Mount Point"));

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

  if (is_array($error_bucket))
    foreach($error_bucket as $elem)
      $input_errors[] =& $elem["error"];
  
  /* if this is an AJAX caller then handle via JSON */
  if(isAjax() && is_array($error_bucket)) {
      input_errors2Ajax(NULL, $error_bucket);       
      exit;   
  }
  
  if ($_POST['apply']) {
    $retval = 0;
    
    if (!file_exists($d_sysrebootreqd_path)) {
      config_lock();
      /* reload all components that mount disk */
      disks_mount_all();
      /* reload all components that use mount */
      services_samba_configure();
      services_nfs_configure();
      services_rsyncd_configure();
      services_afpd_configure();
      config_unlock();
    }
    $savemsg = get_std_save_message($retval);
    if ($retval == 0) {
      if (file_exists($d_mountdirty_path))
        unlink($d_mountdirty_path);
    }
  }
}

if ($_GET['act'] == "del")
{
	if ($a_mount[$_GET['id']]) {
		disks_umount_adv($a_mount[$_GET['id']]);
		unset($a_mount[$_GET['id']]);
		write_config();
		touch($d_mountdirty_path);
		pfSenseHeader("disks_mount.php");
		exit;
	}
}

if ($_GET['act'] == "ret")
{
	if ($a_mount[$_GET['id']]) {
		disks_mount($a_mount[$_GET['id']]);
		pfSenseHeader("disks_mount.php");
		exit;
	}
}

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>

<form id="iform" name="iform" action="disks_mount.php" method="post">
<?php if (file_exists($d_diskdirty_path)): ?>
<?php print_info_box_np(gettext("The mount point list has been changed.") . "<br />" .
                        gettext("You must apply the changes in order for them to take effect."));?>
<?php endif; ?>
  <div id="inputerrors"></div>
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr> 
      	<td class="listhdrr"><?=gettext("Disk");?></td>
      	<td class="listhdrr"><?=gettext("Partition");?></td>
        <td class="listhdrr"><?=gettext("File system");?></td>
        <td class="listhdrr"><?=gettext("Share name");?></td>
        <td class="listhdrr"><?=gettext("Description");?></td>
        <td class="listhdrr"><?=gettext("Status");?></td>
      	<td class="list">&nbsp;</td>
      </tr>
      <?php $i = 0; foreach ($a_mount as $mount): ?>
      <tr> 
        <td valign="middle" class="listlr">
          <?=htmlspecialchars($mount['mdisk']);?> &nbsp;
        </td>
        <td valign="middle" class="listr">
          <?=htmlspecialchars($mount['partition']);?>&nbsp;
        </td>
        <td valign="middle" class="listr">
          <?=htmlspecialchars($mount['fstype']);?>&nbsp;
        </td>
         <td valign="middle" class="listr">
          <?=htmlspecialchars($mount['sharename']);?>&nbsp;
        </td>
         <td valign="middle" class="listr">
          <?=htmlspecialchars($mount['desc']);?>&nbsp;
        </td>
       </td>
       <td valign="middle" class="listbg">
       <?php
         if (file_exists($d_mountdirty_path))
           $stat=_CONFIGURING;
         else {
           $stat=disks_mount_status($mount);
           if ($stat == "ERROR")
             echo "ERROR - <a href=\"disks_mount.php?act=ret&id=$i\">retry</a>";
           else
             echo $stat;
        }
      ?>&nbsp;
      </td>
      <td valign="middle" class="list"> 
        <a href="disks_mount_edit.php?id=<?=$i;?>">
          <img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" title="<?=gettext("edit mount");?>" width="17" height="17" border="0" alt="" />
        </a>
        <a href="disks_mount.php?act=del&id=<?=$i;?>">
          <img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" onclick="return confirm('<? gettext("Do you really want to delete this mount point? All elements that still use it will become invalid (e.g. share)!"); ?>');" title="<?=gettext("delete mount");?>" width="17" height="17" border="0" alt="" />
        </a> 
      </td>
      </tr>
      <?php $i++; endforeach; ?>
      <tr>
    	  <td class="list" colspan="6"></td>
    	  <td class="list" nowrap>
    	    <a href="disks_mount_edit.php">
            <img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="<?=gettext("add mount");?>" width="17" height="17" border="0" alt="" />
          </a>
    	  </td>
      </tr>
      <tr>
        <td align="left" valign="top" colspan="7">
            <span class="red">
              <strong>Note:</strong>
            </span>
            <br />
            <span class="vexpl">
              <?= gettext("Second configuration step: Declaring the filesystem used by your"); ?>
            </span>
            <br />
            <span class="vexpl">
              <a href="disk_manage.php">previously configured disk</a>
            </span>
        </td>
      </tr>
    </table>
  </form>
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
</body>
</html>
