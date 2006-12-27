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
                 gettext("Mount Point"),
                 gettext("Edit"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['mounts']['mount']))
	$freenas_config['mounts']['mount'] = array();

mount_sort();

if (!is_array($freenas_config['disks']['disk']))
	$freenas_config['disks']['disk'] = array();
	
disks_sort();

if (!is_array($freenas_config['raid']['vdisk']))
	$freenas_config['raid']['vdisk'] = array();

gvinum_sort();

if (!is_array($freenas_config['gmirror']['vdisk']))
	$freenas_config['gmirror']['vdisk'] = array();

gmirror_sort();

$a_mount = &$freenas_config['mounts']['mount'];

$a_disk = array_merge($freenas_config['disks']['disk'],$freenas_config['raid']['vdisk'],$freenas_config['gmirror']['vdisk']);

/* Load the cfdevice file*/
$filename=$g['varetc_path']."/cfdevice";
if (file_exists($filename))
  $cfdevice = trim(file_get_contents("$filename"));


$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

if (isset($id) && $a_mount[$id]) {
	$pconfig['mdisk'] = $a_mount[$id]['mdisk'];
	$pconfig['partition'] = $a_mount[$id]['partition'];
	$pconfig['fstype'] = $a_mount[$id]['fstype'];
	$pconfig['sharename'] = $a_mount[$id]['sharename'];
	$pconfig['desc'] = $a_mount[$id]['desc'];
}

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  $pconfig = $_POST;

  /* input validation */
  $reqdfields = split(" ", "partition mdisk fstype");
  $reqdfieldsn = split(",", "Partition,Mdisk,Fstype");

  do_input_validation_new($_POST, $reqdfields, $reqdfieldsn, &$error_bucket);
		
	if (($_POST['sharename'] && !is_validsharename($_POST['sharename'])))
	{
    $error_bucket[] = array("error" => gettext("The share name may only consist of the characters a-z, A-Z, 0-9, _ , -."),
                            "field" => "sharename");
	}
	
	
	if (($_POST['desc'] && !is_validdesc($_POST['desc'])))
	{
    $error_bucket[] = array("error" => gettext("The description name contain invalid characters."),
                            "field" => "desc");

	}
	$device=$_POST['mdisk'].$_POST['partition'];
	
	if ($device == $cfdevice )
	{
    $error_bucket[] = array("error" => gettext("Can't mount the system partition 1, the DATA partition is the 2."),
                            "field" => "mdisk");

	}
		
	/* check for name conflicts */
	foreach ($a_mount as $mount)
	{
		if (isset($id) && ($a_mount[$id]) && ($a_mount[$id] === $mount))
			continue;

		/* Remove the duplicate disk use
		if ($mount['mdisk'] == $_POST['mdisk'])
		{
			$input_errors[] = "This device already exists in the mount point list.";
			break;
		}
		*/
		
		if (($_POST['sharename']) && ($mount['sharename'] == $_POST['sharename']))
		{
      $error_bucket[] = array("error" => gettext("Duplicate Share Name."),
                              "field" => "sharename");
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
		$mount = array();
		$mount['mdisk'] = $_POST['mdisk'];
		$mount['partition'] = $_POST['partition'];
		$mount['fstype'] = $_POST['fstype'];
		$mount['desc'] = $_POST['desc'];
		/* if not sharename given, create one */
		if (!$_POST['sharename'])
			$mount['sharename'] = "disk_{$_POST['mdisk']}_part_{$_POST['partition']}";
		else
			$mount['sharename'] = $_POST['sharename'];
		if (isset($id) && $a_mount[$id])
			$a_mount[$id] = $mount;
		else
			$a_mount[] = $mount;
		
		touch($d_mountdirty_path);
		
		write_config();
		
		pfSenseHeader("disks_mount.php");
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
  <form id="iform" name="iform" action="disks_mount_edit.php" method="post">
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Disk");?></td>
        <td width="78%" class="vtable">
      		<select name="mdisk" class="formselect" id="mdisk">
      		  <?php foreach ($a_disk as $disk): ?>
      			<?php if ((strcmp($disk['fstype'],"raid")!=0) | (strcmp($disk['fstype'],"gmirror")!=0)): ?> 	  
      				<option value="<?=$disk['name'];?>" <?php if ($pconfig['mdisk'] == $disk['name']) echo "selected";?>> 
      				<?php echo htmlspecialchars($disk['name'] . ": " .$disk['size'] . " (" . $disk['desc'] . ")");	?>
      				</option>
      			<?php endif; ?>
      		  <?php endforeach; ?>
          </select>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Partition");?></td>
        <td width="78%" class="vtable">
          <select name="partition" class="formselect" id="partition number">
            <option value="s1" <?php if ($pconfig['partition'] == "s1") echo "selected=\"selected\""; ?>>1</option>
            <option value="s2" <?php if ($pconfig['partition'] == "s2") echo "selected\"selected\""; ?>>2</option>
            <option value="s3" <?php if ($pconfig['partition'] == "s3") echo "selected\"selected\""; ?>>3</option>
            <option value="s4" <?php if ($pconfig['partition'] == "s4") echo "selected\"selected\""; ?>>4</option>
            <option value="gmirror" <?php if ($pconfig['partition'] == "gmirror") echo "selected\"selected\""; ?>><?=_SOFTRAID ;?> - gmirror</option>
            <option value="gvinum" <?php if ($pconfig['partition'] == "gvinum") echo "selected\"selected\""; ?>><?=_SOFTRAID ;?> - gvinum</option>
            <option value="p1" <?php if ($pconfig['partition'] == "gpt") echo "selected\"selected\""; ?>>GPT</option>
          </select>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("File system");?></td>
        <td width="78%" class="vtable">
          <select name="fstype" class="formselect" id="fstype">
            <option value="ufs" <?php if ($pconfig['fstype'] == "ufs") echo "selected=\"selected\""; ?>>UFS</option>
            <option value="msdosfs" <?php if ($pconfig['fstype'] == "msdosfs") echo "selected\"selected\""; ?>>FAT</option>
            <option value="ntfs" <?php if ($pconfig['fstype'] == "ntfs") echo "selected\"selected\""; ?>>NTFS (read-only)</option> 
            <option value="ext2fs" <?php if ($pconfig['fstype'] == "ext2fs") echo "selected\"selected\""; ?>>EXT2 FS</option> 
          </select>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Share name");?></td>
        <td width="78%" class="vtable">
          <input name="sharename" type="text" class="formfld unknown" id="sharename" size="20" value="<?=htmlspecialchars($pconfig['sharename']);?>" />
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Description");?></td>
        <td width="78%" class="vtable">
          <input name="desc" type="text" class="formfld unknown" id="desc" size="20" value="<?=htmlspecialchars($pconfig['desc']);?>" /> 
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top">&nbsp;</td>
        <td width="78%">
          <input id="submit" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" />
          <?php if (isset($id) && $a_mount[$id]): ?>
          <input name="id" type="hidden" value="<?=$id;?>" /> 
          <?php endif; ?>
        </td>
      </tr>
      <tr>
        <td width="100%" align="left" valign="top" colspan="2">
          <span class="red">
            <strong><?= gettext("WARNING"); ?>:</strong>
          </span>
          <ol>
           <li>
             <span class="vexpl">
               <?= gettext("You can't mount the partition '"); ?>
               <?php echo htmlspecialchars($cfdevice);?>
               <?= gettext("' where the config file is stored"); ?>
             </span>
           </li>
           <li><span class="vexpl"><?= gettext("FreeBSD NTFS has lots of bugs."); ?></span></li>
          </ol>
        </td>
      </tr>
    </table>
  </form>
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
</body>
</html>
