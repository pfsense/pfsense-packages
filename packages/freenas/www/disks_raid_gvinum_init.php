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
                 gettext("Initialize"));

require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);

	$reqdfields = explode(" ", "disk");
	$reqdfieldsn = explode(",", "Disk");
  
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
  
  if (is_array($error_bucket))
    foreach($error_bucket as $elem)
      $input_errors[] =& $elem["error"];
  
  /* if this is an AJAX caller then handle via JSON */
  if(isAjax() && is_array($error_bucket)) {
      input_errors2Ajax(NULL, $error_bucket);       
      exit;   
  }
  
	if (! $input_errors) {
		$do_format = true;
		$disk = $_POST['disk'];
	}
}

if (! isset($do_format)) {
	$do_format = false;
	$disk = '';
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
	$tab_array[0] = array(gettext("Manage RAID"), false, "disks_raid_gvinum.php");
	$tab_array[1] = array(gettext("Format RAID"), true,  "disks_raid_gvinum_init.php");
	$tab_array[2] = array(gettext("Tools"),       false, "disks_raid_gvinum_tools.php");
  $tab_array[3] = array(gettext("Information"), false, "disks_raid_gvinum_infos.php");
	display_top_tabs($tab_array);
?>  
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
      <form action="disks_raid_gvinum_init.php" method="post" name="iform" id="iform">
      <table class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
        <tr>
          <td width="22%" valign="top" class="vncellreq"><?=gettext("Volume name");?></td>
          <td width="78%" class="vtable">
            <input name="disk" type="text" class="formfld" id="disk" size="20" value="<?=htmlspecialchars($disk);?>" />
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
				<tr>
  				<td valign="top" colspan="2">
    				<?
              if ($do_format) {
      					echo("<strong>_DISKSRAIDINITPHP_INFO</strong><br />");
      					echo('<pre>');
      					ob_end_flush();
      					
      					/* Create filesystem */
      					system("/sbin/newfs -U /dev/gvinum/" . escapeshellarg($disk));
      					/* Do it twice for test the RAID5 bug at reboot*/
      					system("/sbin/newfs -U /dev/gvinum/" . escapeshellarg($disk));
      										
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
            <span class="vexpl">
            <?= gettext("This step will format the RAID volume in Unix FileSystem (UFS)."); ?>
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
</body>
</html>
