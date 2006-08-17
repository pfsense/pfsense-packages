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
                 gettext("GEOM Vinum"),
                 gettext("RAID"),
                 gettext("Edit"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['raid']['vdisk']))
	$freenas_config['raid']['vdisk'] = array();

gvinum_sort();

if (!is_array($freenas_config['disks']['disk']))
	$nodisk_errors[] = _DISKSRAIDEDITPHP_MSGADDDISKFIRST;
else
	disks_sort();

$a_raid = &$freenas_config['raid']['vdisk'];

$a_disk = &$freenas_config['disks']['disk'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

if (isset($id) && $a_raid[$id]) {
	$pconfig['name'] = $a_raid[$id]['name'];
	$pconfig['type'] = $a_raid[$id]['type'];
	$pconfig['diskr'] = $a_raid[$id]['diskr'];
}

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);

	$reqdfields = explode(" ", "name type");
	$reqdfieldsn = explode(",", "Name,Type");
	
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
  
	if (($_POST['name'] && !is_validaliasname($_POST['name'])))
	{
    $error_bucket[] = array("error" => gettext("The device name may only consist of the characters a-z, A-Z, 0-9."),
                            "field" => "name");
	}
		
	/* check for name conflicts */
	foreach ($a_raid as $raid)
	{
		if (isset($id) && ($a_raid[$id]) && ($a_raid[$id] === $raid))
			continue;

		if ($raid['name'] == $_POST['name'])
		{
      $error_bucket[] = array("error" => gettext("This device already exists in the raid volume list."),
                              "field" => "name");
			break;
		}
	}
	
	/* check the number of RAID disk for volume */

	/* check the number of RAID disk for volume */
	
	switch ($_POST['type'])
	{
		case 0:
			if (count($_POST['diskr']) < 2)
        $error_bucket[] = array("error" => gettext("There must be a minimum of 2 disks in a RAID 0 volume."),
                                "field" => "diskr");
			break;
		case 1:
			if (count($_POST['diskr']) != 2)
        $error_bucket[] = array("error" => gettext("There must be 2 disks in a RAID 1 volume."),
                                "field" => "diskr");
			break;
		case 5:
			if (count($_POST['diskr']) < 3)
        $error_bucket[] = array("error" => gettext("There must be a minimum of 3 disks in a RAID 5 volume."),
                                "field" => "diskr");
			break;
	}
  
  if (is_array($error_bucket))
    foreach($error_bucket as $elem)
      $input_errors[] =& $elem["error"];
  
  /* if this is an AJAX caller then handle via JSON */
  if(isAjax() && is_array($error_bucket)) {
      input_errors2Ajax(NULL, $error_bucket);       
      exit;   
  }
  
	if (!$input_errors) {
		$raid = array();
		$raid['name'] = $_POST['name'];
		$raid['type'] = $_POST['type'];
		$raid['diskr'] = $_POST['diskr'];
		$raid['desc'] = "Software RAID {$_POST['type']}";
		
		if (isset($id) && $a_raid[$id])
			$a_raid[$id] = $raid;
		else
			$a_raid[] = $raid;
		
         	$fd = @fopen("$d_raidconfdirty_path", "a");
         	if (!$fd) {
         		echo "_DISKSRAIDEDITPHP_MSGERROR";
         		exit(0);
         	}
         	fwrite($fd, "$raid[name]\n");
         	fclose($fd);
		
		write_config();
		
		pfSenseHeader("disks_raid_gvinum.php");
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

<div id="inputerrors"></div>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td class="tabnavtbl">
<?php
	$tab_array = array();
	$tab_array[0] = array(gettext("Geom Mirror"),           false, "disks_raid_gmirror.php");
	$tab_array[1] = array(gettext("Geom Vinum (unstable)"), true,  "disks_raid_gvinum.php");
	display_top_tabs($tab_array);
?>  
    </td>
  </tr>
  <tr>
    <td class="tabnavtbl">
<?php
	$tab_array = array();
	$tab_array[0] = array(gettext("Manage RAID"), true,  "disks_raid_gvinum.php");
	$tab_array[1] = array(gettext("Format RAID"), false, "disks_raid_gvinum_init.php");
	$tab_array[2] = array(gettext("Tools"),       false, "disks_raid_gvinum_tools.php");
  $tab_array[3] = array(gettext("Information"), false, "disks_raid_gvinum_infos.php");
	display_top_tabs($tab_array);
?>  
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
      <form action="disks_raid_gvinum_edit.php" method="post" name="iform" id="iform">
      <table class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
        <tr>
          <td width="22%" valign="top" class="vncellreq"><?=gettext("Volume name");?></td>
          <td width="78%" class="vtable">
            <input name="name" type="text" class="formfld unknown" id="name" size="20" value="<?=htmlspecialchars($pconfig['name']);?>" />
          </td>
        </tr>
        <tr> 
          <td valign="top" class="vncellreq"><?= gettext("Type"); ?></td>
          <td class="vtable">
            <select name="type" class="formselect" id="type">
              <option value="0" <?php if ($pconfig['type'] == 0) echo "selected=\"selected\""; ?>>RAID 0 (<?= gettext("striping"); ?>)</option>
              <option value="1" <?php if ($pconfig['type'] == 1) echo "selected=\"selected\""; ?>>RAID 1 (<?= gettext("mirroring"); ?>)</option>
              <option value="5" <?php if ($pconfig['type'] == 5) echo "selected=\"selected\""; ?>>RAID 5 (<?= gettext("rotated block-interleaved parity"); ?>)</option>

            </select>
          </td>
        </tr>        
        <tr> 
          <td width="22%" valign="top" class="vncellreq"><?= gettext("Members of this volume"); ?></td>
          <td width="78%" class="vtable">
            <?
              $i=0;
              $disable_script="";
              
              foreach ($a_disk as $diskv) {
                $r_name="";
                
                if (strcmp($diskv['fstype'],"raid")==0) {
                  foreach($a_raid as $raid) {
                    if (in_array($diskv['name'],$raid['diskr'])) {
                      $r_name=$raid['name'];
                      
                      if ($r_name!=$pconfig['name'])
                        $disable_script.="document.getElementById($i).disabled=1;\n";
                      break;
                    }
                  }
                  echo "<input name='diskr[]' id='$i' type='checkbox' value='$diskv[name]'".
                       ((is_array($pconfig['diskr']) && in_array($diskv['name'], $pconfig['diskr'])) ? " checked=\"checked\"" : "").
                       " />$diskv[name] ($diskv[size], $diskv[desc])" . (($r_name) ? " - assigned to $r_name" : "") . "<br>\n";
                }
                $i++;
              }
              if ($disable_script) echo "<script type='text/javascript'><!--\n$disable_script--></script>\n";
            ?>
          </td>
        </tr>
        <tr> 
          <td width="22%" valign="top">&nbsp;</td>
          <td width="78%">
            <input name="Submit" type="submit" class="formbtn" value="Save" /> 
            <?php if (isset($id) && $a_raid[$id]): ?>
            <input name="id" type="hidden" value="<?=$id;?>" /> 
            <?php endif; ?>
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
