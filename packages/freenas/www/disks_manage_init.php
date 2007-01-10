<?php
/* $Id$ */
/* ========================================================================== */
/*
    disks_manage_init.php
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
                 gettext("Initialize"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

define("DISK_DETAILS_PARA", "
              <p style='font-size: small;'>
                <strong>Disk initialization details</strong> (use the toggle icon to unveil detailed infos):
              </p>
       ");

function create_format_output($disk, $type, $notinitmbr) {
  $ddetails = DISK_DETAILS_PARA;
  
  ob_end_flush();
  
  $retvalue =<<<EOD
{$ddetails}

EOD;

  // Erase MBR if not checked
  if (!$notinitmbr) {
    $button = create_toggle_button("Erasing MBR and all paritions", "mbr_out");
    $cmd = "dd if=/dev/zero of=" . escapeshellarg($disk) . " bs=32k count=640";
    $out = create_cmdout_container("mbr_out", $cmd);
    $retvalue .= assemble_cmdout($button, $out);
  } else {
    $diskinit_str = "Keeping the MBR and all partitions";
    $retvalue .=<<<EOD
                <div id="mbr_out" style="font-family: Courier, monospace; font-size: small;">
                <pre style="font-family: Courier, monospace; font-size: small; font-style: italic;">{$diskinit_str}</pre>
                </div>
                
EOD;
  } // end if

  switch ($type) {
    case "ufs":
      $button = create_toggle_button("Creating one partition", "ufs_fdisk_out");
      /* Initialize disk */
      $cmd = "/sbin/fdisk -I -b /boot/mbr " . escapeshellarg($disk);
      $out = create_cmdout_container("ufs_fdisk_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      $button = create_toggle_button("Initializing partition", "ufs_dd_out");
      /* Initialise the partition (optional) */
      $cmd = "/bin/dd if=/dev/zero of=" . escapeshellarg($disk) . "s1 bs=32k count=16";
      $out = create_cmdout_container("ufs_dd_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      $button = create_toggle_button("Creating BSD label", "ufs_label_out");
      /* Create s1 label */
      $cmd = "/sbin/bsdlabel -w " . escapeshellarg($disk) . "s1 auto";
      $out = create_cmdout_container("ufs_label_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      $button = create_toggle_button("Creating Filesystem", "ufs_newfs_out");
      /* Create filesystem */
      $cmd = "/sbin/newfs -U " . escapeshellarg($disk) . "s1";
      $out = create_cmdout_container("ufs_newfs_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      break; // end case "ufs":
    case "ufs_no_su":
      $button = create_toggle_button("Creating one partition", "ufsn_fdisk_out");
      /* Initialize disk */
      $cmd = "/sbin/fdisk -I -b /boot/mbr " . escapeshellarg($disk);
      $out = create_cmdout_container("ufsn_fdisk_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);
      
      $button = create_toggle_button("Initializing partition", "ufsn_dd_out");
      /* Initialise the partition (optional) */
      $cmd = "/bin/dd if=/dev/zero of=" . escapeshellarg($disk) . "s1 bs=32k count=16";
      $out = create_cmdout_container("ufsn_dd_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);
    
      $button = create_toggle_button("Creating BSD label", "ufsn_label_out");
      /* Create s1 label */
      $cmd = "/sbin/bsdlabel -w " . escapeshellarg($disk) . "s1 auto";
      $out = create_cmdout_container("ufsn_label_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);
      
      $button = create_toggle_button("Creating Filesystem", "ufsn_newfs_out");
      /* Create filesystem */
      $cmd = "/sbin/newfs -m 0 " . escapeshellarg($disk) . "s1";
      $out = create_cmdout_container("ufsn_newfs_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);
      
      break; // end ufs_no_su
    case "ufsgpt":
      $button = create_toggle_button("Destroying old GTP information", "ufsg_gptd_out");
      /* Destroy GPT partition table */
      $cmd = "/sbin/gpt destroy " . escapeshellarg($disk);
      $out = create_cmdout_container("ufsg_gptd_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      $button = create_toggle_button("Creating GPT partition", "ufsg_gptc_out");
      /* Create GPT partition table */
      $cmd = array("/sbin/gpt create -f " . escapeshellarg($disk),
                   "/sbin/gpt add -t ufs " . escapeshellarg($disk));
      $out = create_cmdout_container("ufsg_gptc_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      $button = create_toggle_button("Creating Filesystem with Soft Updates", "ufsg_newfs_out");
      /* Create filesystem */
      $cmd = "/sbin/newfs -U " . escapeshellarg($disk) . "p1";
      $out = create_cmdout_container("ufsg_newfs_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      break; // end case "ufsgpt":
    case "ufsgpt_no_su":
      $button = create_toggle_button("Destroying old GTP information", "ufsgn_gpt_out");
      /* Destroy GPT partition table */
      $cmd = "/sbin/gpt destroy " . escapeshellarg($disk);
      $out = create_cmdout_container("ufsgn_gpt_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      $button = create_toggle_button("Creating GPT partition", "ufsgn_gptc_out");
      /* Create GPT partition table */
      $cmd = array("/sbin/gpt create -f " . escapeshellarg($disk),
                   "/sbin/gpt add -t ufs " . escapeshellarg($disk));
      $out = create_cmdout_container("ufsgn_gptc_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      $button = create_toggle_button("Creating Filesystem without Soft Updates", "ufsgn_newfs_out");
      /* Create filesystem */
      $cmd = "/sbin/newfs -m 0 " . escapeshellarg($disk) . "p1";
      $out = create_cmdout_container("ufsgn_newfs_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      break; // end case "ufsgpt_no_su":
    case "softraid":
      $button = create_toggle_button("Initializing disk", "softr_fdisk_out");
      /* Initialize disk */
      $cmd = "/sbin/fdisk -I -b /boot/mbr " . escapeshellarg($disk);
      $out = create_cmdout_container("softr_fdisk_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      $button = create_toggle_button("Initializing partition", "softr_dd_out");
      /* Initialise the partition (optional) */
      $cmd = "/bin/dd if=/dev/zero of=" . escapeshellarg($disk) . "s1 bs=32k count=16";
      $out = create_cmdout_container("softr_dd_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);
      
      $button = create_toggle_button("Delete old gmirror information", "softr_dd_out");
      /* Delete old gmirror information */
      $cmd = "/sbin/gmirror clear " . escapeshellarg($disk);
      $out = create_cmdout_container("softr_dd_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      break; // end case "softraid":
    case "msdos":
      $button = create_toggle_button("Initialize disk", "dos_fdisk_out");
      /* Initialize disk */
      $cmd = "/sbin/fdisk -I -b /boot/mbr " . escapeshellarg($disk);
      $out = create_cmdout_container("dos_fdisk_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      $button = create_toggle_button("Initialize partion", "dos_dd_out");
      /* Initialise the partition (optional) */
      $cmd = "/bin/dd if=/dev/zero of=" . escapeshellarg($disk) . "s1 bs=32k count=16";
      $out = create_cmdout_container("dos_dd_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      $button = create_toggle_button("Creating BSD label", "dos_label_out");
      /* Initialise the partition (optional) */
      $cmd = "/sbin/bsdlabel -w " . escapeshellarg($disk) . "s1 auto";
      $out = create_cmdout_container("dos_label_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out);

      $button = create_toggle_button("Creating Filesystem", "dos_newfs_out");
      /* Initialise the partition (optional) */
      $a_out = "/sbin/newfs_msdos -F 32 " . escapeshellarg($disk) . "s1";
      $out = create_cmdout_container("dos_newfs_out", $cmd);
      $retvalue .= assemble_cmdout($button, $out, true);
      
    break; // end case "msdos":
  } // end switch
  
  return $retvalue;
}

if (!is_array($freenas_config['disks']['disk']))
  $freenas_config['disks']['disk'] = array();

disks_sort();

if (!is_array($freenas_config['gconcat']['vdisk']))
  $freenas_config['gconcat']['vdisk'] = array();

gconcat_sort();

if (!is_array($freenas_config['gmirror']['vdisk']))
  $freenas_config['gmirror']['vdisk'] = array();

gmirror_sort();

if (!is_array($freenas_config['graid5']['vdisk']))
  $freenas_config['graid5']['vdisk'] = array();

graid5_sort();

if (!is_array($freenas_config['gstripe']['vdisk']))
  $freenas_config['gstripe']['vdisk'] = array();

gstripe_sort();

if (!is_array($freenas_config['gvinum']['vdisk']))
  $freenas_config['gvinum']['vdisk'] = array();

gvinum_sort();

// Get all fstype supported by FreeNAS
$a_fst = get_fstype_list();
// Remove NTFS: can't format on NTFS under FreeNAS
unset($a_fst['ntfs']);
// Remove the first blank line 'unknown'
$a_fst = array_slice($a_fst, 1);

$a_disk = &$freenas_config['disks']['disk'];
$a_gconcat = &$freenas_config['gconcat']['vdisk'];
$a_gmirror = &$freenas_config['gmirror']['vdisk'];
$a_gstripe = &$freenas_config['gstripe']['vdisk'];
$a_graid5 = &$freenas_config['graid5']['vdisk'];
$a_gvinum = &$freenas_config['gvinum']['vdisk'];
$all_disk = array_merge($a_disk,$a_gconcat,$a_gmirror,$a_gstripe,$a_graid5,$a_gvinum);

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  unset($do_format);
  $pconfig = $_POST;

  /* input validation */
  $reqdfields = explode(" ", "disk type");
  $reqdfieldsn = explode(",", "Disk,Type");

  do_input_validation_new($_POST, $reqdfields, $reqdfieldsn, &$error_bucket);

  if (is_array($error_bucket))
    foreach($error_bucket as $elem)
      $input_errors[] =& $elem["error"];
  
  /* if this is an AJAX caller then handle via JSON */
  if(isAjax() && is_array($error_bucket)) {
    input_errors2Ajax(NULL, $error_bucket);       
    exit;   
  }
  
  if (!$input_errors)
  {
    $do_format = true;
    $disk = $_POST['disk'];
    $type = $_POST['type'];
    $notinitmbr= $_POST['notinitmbr'];
    
    /* Check if disk is mounted. */ 
    if(isAjax() && disks_check_mount_fullname($disk)) {
      $statustxt = sprintf(gettext("The disk is currently mounted! <a href=%s>Unmount</a> this disk first before proceeding."), "disks_mount_tools.php?disk={$disk}&amp;action=umount");

      $divcontents = "<div style='background:#990000'><table>";
      $divcontents .= "<tr><td>";
      $divcontents .= "<img src='/themes/{$g['theme']}/images/icons/icon_error.gif' width='28' height='32'>";
      $divcontents .= "</td><td><font color='white'><br>&nbsp;&nbsp;The following errors have occured:<p><ul>";
      $divcontents .= "<font color='white'><li> " . $statustxt . "</li>";
      $divcontents .= "</ul></td></table></div><br />";
  
      header("HTTP/1.0 500 Internal Server Error");
      header("Status: 500 Internal Server Error. {$statustxt}");
      echo $divcontents;
      exit;
    } else if (disks_check_mount_fullname($disk)) {
      $errormsg = sprintf(gettext("The disk is currently mounted! <a href=%s>Unmount</a> this disk first before proceeding."), "disks_mount_tools.php?disk={$disk}&action=umount");
      $do_format = false;
    }

    if($do_format) {
      /* Get the id of the disk array entry. */
      $NotFound = 1;
      $id = array_search_ex($disk, $a_disk, "fullname");
  
      if ($id) {
        /* Set new filesystem type. */
        $a_disk[$id]['fstype'] = $type;
        $NotFound = 0;
      }
      else {
        $id = array_search_ex($disk, $a_gmirror, "fullname");
      }
      if (($id !== false) && $NotFound) {
        /* Set new filesystem type. */
        $a_gmirror[$id]['fstype'] = $type;
        $NotFound = 0;
      }
      else {
        $id = array_search_ex($disk, $a_gstripe, "fullname");
      }
      if (($id !== false) && $NotFound) {
        /* Set new filesystem type. */
        $a_gstripe[$id]['fstype'] = $type;
        $NotFound = 0;
      }
      else {
        $id = array_search_ex($disk, $a_gconcat, "fullname");
      }
      if (($id !== false) && $NotFound) {
        /* Set new filesystem type. */
        $a_gconcat[$id]['fstype'] = $type;
        $NotFound = 0;
      }
      else {
        $id = array_search_ex($disk, $a_graid5, "fullname");
      }
      if (($id !== false) && $NotFound) {
        /* Set new filesystem type. */
        $a_graid5[$id]['fstype'] = $type;
        $NotFound = 0;
      }
      else {
        $id = array_search_ex($disk, $a_gvinum, "fullname");
      }
      if (($id !== false) && $NotFound) {
        /* Set new filesystem type. */
        $a_gvinum[$id]['fstype'] = $type;
        $NotFound = 0;
      }
      
      write_config();
      
      echo create_format_output($disk, $type, $notinitmbr);
      exit; // cause of Ajax
    }
  }
}

if (!isset($do_format))
{
  $do_format = false;
  $disk = '';
  $type = '';
}

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
$pfSenseHead->setCloseHead(false);
echo $pfSenseHead->getHTML();

?>
<script type="text/javascript">
<!--
function disk_change() {
  switch(document.iform.disk.value)
  {
    <?php foreach ($a_disk as $diskv): ?>
    case "<?=$diskv['fullname'];?>":
      <?php $i = 0;?>
      <?php foreach ($a_fst as $fstval => $fstname): ?>
        document.iform.type.options[<?=$i++;?>].selected = <?php if($diskv['fstype'] == $fstval){echo "true";}else{echo "false";};?>;
      <?php endforeach; ?>
      break;
    <?php endforeach; ?>
  }
}

<?= CMDOUT_TOGGLE_FUNC ?>
// -->
</script>
</head>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php include("fbegin.inc"); ?>

<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if($errormsg) print_error_box($errormsg);?>
<div id="inputerrors"></div>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
<?php
  $tab_array = array();
  $tab_array[0] = array(gettext("Manage"),          false, "disks_manage.php");
  $tab_array[1] = array(gettext("Format"),          true,  "disks_manage_init.php");
  $tab_array[2] = array(gettext("Tools"),           false,  "disks_manage_tools.php");
  $tab_array[3] = array(gettext("iSCSI Initiator"), false, "disks_manage_iscsi.php");
  display_top_tabs($tab_array);
?>
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
      <form id="iform" name="iform" action="disks_manage_init.php" method="post">
      <?= CMDOUT_AJAX_SCRIPT ?>
      <table class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?=gettext("Disk");?></td>
            <td width="78%" class="vtable">
              <select name="disk" class="formselect" id="disk" onchange="disk_change();">
                <?php foreach ($all_disk as $diskv): ?>
                <option value="<?=$diskv['fullname'];?>" <?php if ($diskv['name'] == $disk) echo "selected=\selected\"";?>>
                <?php echo htmlspecialchars($diskv['name'] . ": " .$diskv['size'] . " (" . $diskv['desc'] . ")");?>
                <?php endforeach; ?>
                </option>
              </select>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?=gettext("File system");?></td>
            <td width="78%" class="vtable">
              <select name="type" class="formselect" id="type">
                <?php foreach ($a_fst as $fstval => $fstname): ?>
                <option value="<?=$fstval;?>" <?php if($type == $fstval) echo 'selected="selected"';?>><?=htmlspecialchars($fstname);?></option>
                <?php endforeach; ?>
             </select>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?=gettext("Don't Erase MBR");?></td>
            <td width="78%" class="vtable">
              <input name="notinitmbr" id="notinitmbr" type="checkbox" value="yes" /><br />
              <?= gettext("don't erase the MBR (useful for some RAID controller cards)"); ?>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top">&nbsp;</td>
            <td width="78%">
              <input id="doCMDSubmit" name="doCMDSubmit" type="button" class="formbtn" value="<?=gettext("Format disk!");?>" onclick="execCMD();" />
            </td>
          </tr>
          <tr>
            <!-- Format Output Container - Do Not Delete -->
            <td id="cmdOutputTD" valign="top" colspan="2" style="visibility: hidden; border: solid 1px silver; vertical-align: middle; width: 100%"></td>
          </tr>
          <tr>
            <td align="left" valign="top" colspan="2">
              <span class="red">
                <strong>WARNING:</strong>
              </span>
              <br />
              <span class="vexpl">
                <?= gettext("This step will erase all your partition, create partition number 1 and format the hard drive with the file system specified."); ?>
              </span>
            </td>
          </tr>
        </table>
        </form>
      </div>
    </td>
  </tr>
</table>
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
</body>
</html>
