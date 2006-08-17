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
                 gettext("Initialize"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['disks']['disk']))
	$freenas_config['disks']['disk'] = array();

disks_sort();

$a_disk = &$freenas_config['disks']['disk'];

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
		$diskid = $_POST['id'];
		$notinitmbr= $_POST['notinitmbr'];
		
		/* found the name in the config: Must be a better way for did that */

		$id=0;
		$i=0;
		foreach ($a_disk as $disks)
		{
			$diskname=$disks['name'];
			if (strcmp($diskname,$disk)==0)
				$id=$i;
			$i++;
		}
		
		if ($type == "ufs" || $type == "ufsgpt" || $type == "ufs_no_su" || $type == "ufsgpt_no_su")
			$a_disk[$id]['fstype'] = "ufs";
		else
			$a_disk[$id]['fstype'] = $type;
		write_config();
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
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<div id="inputerrors"></div> 
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
<?php
  $tab_array = array();
	$tab_array[0] = array(gettext("Manage"),          false, "disks_manage.php");
	$tab_array[1] = array(gettext("Format"),          true,  "disks_manage_init.php");
	$tab_array[2] = array(gettext("iSCSI Initiator"), false, "disks_manage_iscsi.php");
  display_top_tabs($tab_array);
?>
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
      <form id="iform" name="iform" action="disks_manage_init.php" method="post">
      <table class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?=gettext("Disk");?></td>
            <td width="78%" class="vtable">
          		<select name="disk" class="formselect" id="disk">
          		  <?php foreach ($a_disk as $diskn): ?>
          		  <option value="<?=$diskn['name'];?>"<?php if ($diskn['name'] == $disk) echo "selected=\"selected\"";?>> 
          		  <?php echo htmlspecialchars($diskn['name'] . ": " .$diskn['size'] . " (" . $diskn['desc'] . ")");?>
          		  </option>
          		  <?php endforeach; ?>
              </select>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?=gettext("File system");?></td>
            <td width="78%" class="vtable">
              <select name="type" class="formselect" id="type">
                <option value="ufs" <?php if ($type == "ufs") echo "selected=\"selected\""; ?>>UFS with Soft Updates (use 8% space disk)</option>
                <option value="ufs_no_su" <?php if ($type == "ufs_no_su") echo "selected=\"selected\""; ?>>UFS</option>
                <option value="ufsgpt" <?php if ($type == "ufsgpt") echo "selected=\"selected\""; ?>>UFS (EFI/GPT) with Soft Updates (use 8% space disk)</option>
                <option value="ufsgpt_no_su" <?php if ($type == "ufsgpt_no_su") echo "selected=\"selected\""; ?>>UFS (EFI/GPT)</option>
                <option value="msdos" <?php if ($type == "msdos") echo "selected=\"selected\""; ?>>FAT32</option>
                <option value="gmirror" <?php if ($type == "gmirror") echo "selected=\"selected\""; ?>>Software RAID: Geom mirror</option>
                <option value="raid" <?php if ($type == "raid") echo "selected=\"selected\""; ?>>Software RAID: Geom Vinum</option>
               </select>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?=gettext("File system");?></td>
            <td width="78%" class="vtable">
              <input name="notinitmbr" id="notinitmbr" type="checkbox" value="yes" /><br />
              <?= gettext("don't erase the MBR (useful for some RAID controller cards)"); ?>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top">&nbsp;</td>
            <td width="78%">
              <input id="submit" name="Submit" type="submit" class="formbtn" value="<?=gettext("Format disk!");?>" />
            </td>
          </tr>
  				<tr>
    				<td valign="top" colspan="2">
    				<? if ($do_format)
    				{
    					echo(_DISKSMANAGEINITPHP_INITTEXT);
    					echo('<pre>');
    					ob_end_flush();
    					
    					/* Erase MBR if not checked*/
    					
    					if (!$notinitmbr) {
    						echo "Erasing MBR\n";
    						system("dd if=/dev/zero of=/dev/" . escapeshellarg($disk) . " bs=32k count=640");
    						
    					}
    					else
    						echo "Keeping the MBR\n";
    					
    					switch ($type)
    					{
    					case "ufs":
    						/* Initialize disk */
    						system("/sbin/fdisk -I -b /boot/mbr " . escapeshellarg($disk));
    						echo "\"fdisk: Geom not found\"is not an error message!\n";
    						/* Initialise the partition (optional) */
    						system("/bin/dd if=/dev/zero of=/dev/" . escapeshellarg($disk) . "s1 bs=32k count=16");
    						/* Create s1 label */
    						system("/sbin/bsdlabel -w " . escapeshellarg($disk) . "s1 auto");			
    						// Create filesystem	
    						system("/sbin/newfs -U /dev/" . escapeshellarg($disk) . "s1");
    						break;
    					case "ufs_no_su":
    						/* Initialize disk */
    						system("/sbin/fdisk -I -b /boot/mbr " . escapeshellarg($disk));
    						/* Initialise the partition (optional) */
    						system("/bin/dd if=/dev/zero of=/dev/" . escapeshellarg($disk) . "s1 bs=32k count=16");
    						/* Create s1 label */
    						system("/sbin/bsdlabel -w " . escapeshellarg($disk) . "s1 auto");			
    						// Create filesystem	
    						system("/sbin/newfs -m 0 /dev/" . escapeshellarg($disk) . "s1");
    						break;
    					case "ufsgpt":
    						/* Create GPT partition table */
    						system("/sbin/gpt destroy " . escapeshellarg($disk));
    						system("/sbin/gpt create -f " . escapeshellarg($disk));
    						system("/sbin/gpt add -t ufs " . escapeshellarg($disk));
    						// Create filesystem
    						system("/sbin/newfs -U /dev/" . escapeshellarg($disk) . "p1");
    						break;
    					case "ufsgpt_no_su":
    						/* Create GPT partition table */
    						system("/sbin/gpt destroy " . escapeshellarg($disk));
    						system("/sbin/gpt create -f " . escapeshellarg($disk));
    						system("/sbin/gpt add -t ufs " . escapeshellarg($disk));
    						// Create filesystem
    						system("/sbin/newfs -m 0 /dev/" . escapeshellarg($disk) . "p1");
    						break;
    					case "gmirror":
    						/* Initialize disk */
    						system("/sbin/fdisk -I -b /boot/mbr " . escapeshellarg($disk));
    						/* Initialise the partition (optional) */
    						system("/bin/dd if=/dev/zero of=/dev/" . escapeshellarg($disk) . "s1 bs=32k count=16");
    						/* Create s1 label */
    						//system("/sbin/bsdlabel -w " . escapeshellarg($disk) . "s1 auto");
    						/* Delete old gmirror information */
    						system("/sbin/gmirror clear /dev/" . escapeshellarg($disk));
    						break;
    					case "raid":
    						/* Initialize disk */
    						system("/sbin/fdisk -I -b /boot/mbr " . escapeshellarg($disk));
    						echo "\"fdisk: Geom not found\"is not an error message!\n";
    						/* Initialise the partition (optional) */
    						system("/bin/dd if=/dev/zero of=/dev/" . escapeshellarg($disk) . "s1 bs=32k count=16");
    						/* Create s1 label */
    						system("/sbin/bsdlabel -w " . escapeshellarg($disk) . "s1 auto");
    						break;
    					case "msdos":
    						/* Initialize disk */
    						system("/sbin/fdisk -I -b /boot/mbr " . escapeshellarg($disk));
    						echo "\"fdisk: Geom not found\"is not an error message!\n";
    						/* Initialise the partition (optional) */
    						system("/bin/dd if=/dev/zero of=/dev/" . escapeshellarg($disk) . "s1 bs=32k count=16");
    						/* Create s1 label */
    						system("/sbin/bsdlabel -w " . escapeshellarg($disk) . "s1 auto");
    						// Create filesystem
    						system("/sbin/newfs_msdos -F 32 /dev/" . escapeshellarg($disk) . "s1");
    						break;		
    					}
    					
    					echo('</pre>');
    				}
    				?>
    				</td>
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
