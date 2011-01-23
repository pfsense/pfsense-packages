<?php
/* $Id$ */
/* ========================================================================== */
/*
    disks_manage_edit.php
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
                 gettext("Management"),
                 gettext("Edit"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

$id = $_GET['id'];
if (isset($_POST['id']))
  $id = $_POST['id'];
  
/* get disk list (without CDROM) */
$disklist = get_physical_disks_list();

if (!is_array($freenas_config['disks']['disk']))
  $freenas_config['disks']['disk'] = array();

disks_sort();

$a_disk = &$freenas_config['disks']['disk'];

if (isset($id) && $a_disk[$id])
{
  $pconfig['name'] = $a_disk[$id]['name'];
  $pconfig['harddiskstandby'] = $a_disk[$id]['harddiskstandby'];
  $pconfig['acoustic'] = $a_disk[$id]['acoustic'];
  $pconfig['fstype'] = $a_disk[$id]['fstype'];
  $pconfig['apm'] = $a_disk[$id]['apm'];
  $pconfig['udma'] = $a_disk[$id]['udma'];
  $pconfig['fullname'] = $a_disk[$id]['fullname'];
}

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  $pconfig = $_POST;

  /* input validation */
  $reqdfields = split(" ", "name");
  $reqdfieldsn = split(",", "Name");

  do_input_validation_new($_POST, $reqdfields, $reqdfieldsn, &$error_bucket);
  $pconfig = $_POST;
    
  /* check for name conflicts */
  foreach ($a_disk as $disk)
  {
    if (isset($id) && ($a_disk[$id]) && ($a_disk[$id] === $disk)) { continue; }
  
    if ($disk['name'] == $_POST['name'])
    {
      $error_bucket[] = array("error" => gettext("This disk already exists in the disk list."),
                              "field" => "name");
      break;
    }
  }
  
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
    $disks = array();
    
    $devname = $_POST['name'];
    $devharddiskstandby = $_POST['harddiskstandby'];
    $harddiskacoustic = $_POST['acoustic'];
    $harddiskapm  = $_POST['apm'];
    $harddiskudma  = $_POST['udma'];
    $harddiskfstype = $_POST['fstype'];
    
    $disks['name'] = $devname;
    $disks['fullname'] = "/dev/$devname";
    $disks['harddiskstandby'] = $devharddiskstandby ;
    $disks['acoustic'] = $harddiskacoustic ;
    if ($harddiskfstype) { $disks['fstype'] = $harddiskfstype; }
    $disks['apm'] = $harddiskapm ;
    $disks['udma'] = $harddiskudma ;
    $disks['type'] = $disklist[$devname]['type'];
    $disks['desc'] = $disklist[$devname]['desc'];
    $disks['size'] = $disklist[$devname]['size'];
    
    if (isset($id) && $a_disk[$id]) {
      $a_disk[$id] = $disks;
    } else {
      $a_disk[] = $disks;
    }
    
    touch($d_diskdirty_path);
    
    disks_set_ataidle();
    write_config();
    
    pfSenseHeader("disks_manage.php");
    exit;
  }
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
  <form id="iform" name="iform" action="disks_manage_edit.php" method="post">
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Disk");?></td>
        <td width="78%" class="vtable">
          <select name="name" class="formselect" id="name">
            <?php foreach ($disklist as $diski => $diskv): ?>
            <option value="<?=$diski;?>" <?php if ($diski == $pconfig['name']) echo "selected=\"selected\"";?>> 
            <?php echo htmlspecialchars($diski . ": " .$diskv['size'] . " (" . $diskv['desc'] . ")");?>
            </option>
            <?php endforeach; ?>
          </select>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("UDMA mode");?></td>
        <td width="78%" class="vtable">
          <select name="udma" class="formselect" id="udma">
            <?php
              $types = explode(",", "Auto,UDMA-33,UDMA-66,UDMA-100,UDMA-133");
              $vals = explode(" ", "auto UDMA2 UDMA4 UDMA5 UDMA6");
              $j = 0;
              
              for ($j = 0; $j < count($vals); $j++):
            ?>
            <option value="<?=$vals[$j];?>" <?php if ($vals[$j] == $pconfig['udma']) echo "selected=\"selected\"";?>> 
            <?=htmlspecialchars($types[$j]);?>
            </option>
            <?php endfor; ?>
          </select>
          <br />
          <?= gettext("You can force UDMA mode if you have \"UDMA_ERROR.... LBA\" message with your hard drive."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Hard disk standby time");?></td>
        <td width="78%" class="vtable">
          <select name="harddiskstandby" class="formselect">
            <?php
              $sbvals = array(0=>"Always on",5=>"5 minutes",10=>"10 minutes",20=>"20 minutes",30=>"30 minutes",60=>"60 minutes");
            ?>
            <?php foreach ($sbvals as $sbval => $sbname): ?>
            <option value="<?=$sbval;?>" <?php if($pconfig['harddiskstandby'] == $sbval) echo 'selected="selected"';?>><?=htmlspecialchars($sbname);?></option>
            <?php endforeach; ?>
          </select>
          <br />
          <?= gettext("Puts the hard disk into standby mode when the selected amount of time after the last
                       access has elapsed. <em>Do not set this for CF cards.</em>"); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Advanced Power Management");?></td>
        <td width="78%" class="vtable">
          <select name="apm" class="formselect">
            <?php
              $apmvals = array(0=>"Disabled",1=>"Minimum power usage with Standby",64=>"Medium power usage with Standby",128=>"Minimum power usage without Standby",192=>"Medium power usage without Standby",254=>"Maximum performance, maximum power usage");
            ?>
            <?php foreach ($apmvals as $apmval => $apmname): ?>
            <option value="<?=$apmval;?>" <?php if($pconfig['apm'] == $apmval) echo 'selected="selected"';?>><?=htmlspecialchars($apmname);?></option>
            <?php endforeach; ?>
          </select>
          <br />
          <?= gettext("This allows  you  to lower the power consumption of the drive, at the expense of performance.<em>Do not set this for CF cards.</em>"); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("acoustic level");?></td>
        <td width="78%" class="vtable">
          <select name="acoustic" class="formselect">
            <?php
              $acvals = array(0=>"Disabled",1=>"Minimum performance, Minimum acoustic output",64=>"Medium acoustic output",127=>"Maximum performance, maximum acoustic output");
            ?>
            <?php foreach ($acvals as $acval => $acname): ?>
            <option value="<?=$acval;?>" <?php if($pconfig['acoustic'] == $acval) echo 'selected';?>><?=htmlspecialchars($acname);?></option>
            <?php endforeach; ?>
          </select>
          <br />
          <?= gettext("This allows you to set how loud the drive is while it\'s  operating.<em>Do not set this for CF cards.</em>"); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("preformated FS");?></td>
        <td width="78%" class="vtable">
          <select name="fstype" class="formselect">
            <?php $fstlist = get_fstype_list(); ?>
            <?php foreach ($fstlist as $fstval => $fstname): ?>
            <option value="<?=$fstval;?>" <?php if($pconfig['fstype'] == $fstval) echo 'selected';?>><?=htmlspecialchars($fstname);?></option>
            <?php endforeach; ?>
          </select>
          <br />
          <?= gettext("This allows you to set FS type for preformated disk with data.<br />
                      <em>Leave \"unformated\" for unformated disk and then use Format menu.</em>"); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top">&nbsp;</td>
        <td width="78%">
          <input id="submit" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" />
          <?php if (isset($id) && $a_disk[$id]): ?>
          <input name="id" type="hidden" value="<?=$id;?>"> 
          <?php endif; ?>
        </td>
      </tr>
    </table>
  </form>
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
</body>
</html>
