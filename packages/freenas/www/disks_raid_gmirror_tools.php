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
                 gettext("GEOM Mirror"),
                 gettext("Tools"));

require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);

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
	$tab_array[0] = array(gettext("Geom Mirror"),           true,  "disks_raid_gmirror.php");
	$tab_array[1] = array(gettext("Geom Vinum (unstable)"), false, "disks_raid_gvinum.php");
	display_top_tabs($tab_array);
?>  
    </td>
  </tr>
  <tr>
    <td class="tabnavtbl">
<?php
	$tab_array = array();
	$tab_array[0] = array(gettext("Manage RAID"), false, "disks_raid_gmirror.php");
	$tab_array[1] = array(gettext("Format RAID"), false, "disks_raid_gmirror_init.php");
	$tab_array[2] = array(gettext("Tools"),       true,  "disks_raid_gmirror_tools.php");
  $tab_array[3] = array(gettext("Information"), false, "disks_raid_gmirror_infos.php");
	display_top_tabs($tab_array);
?>  
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
      <form action="disks_raid_gmirror_tools.php" method="post" name="iform" id="iform">
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
              <option value="rebuild" <?php if ($action == "rebuild") echo "selected=\"selected\""; ?>>rebuild</option>
              <option value="list" <?php if ($action == "list") echo "selected=\"selected\""; ?>>list</option>
              <option value="status" <?php if ($action == "status") echo "selected=\"selected\""; ?>>status</option>
              <option value="remove" <?php if ($action == "remove") echo "selected=\"selected\""; ?>>remove</option>
              <option value="activate" <?php if ($action == "activate") echo "selected=\"selected\""; ?>>activate</option>
              <option value="deactivate" <?php if ($action == "deactivate") echo "selected=\"selected\""; ?>>deactivate</option>
              <option value="forget" <?php if ($action == "forget") echo "selected=\"selected\""; ?>>forget</option>
              <option value="clear" <?php if ($action == "clear") echo "selected=\"selected\""; ?>>clear</option>
              <option value="stop" <?php if ($action == "stop") echo "selected=\"selected\""; ?>>stop</option>
             </select>
          </td>
        </tr>
        <tr> 
          <td width="22%" valign="top">&nbsp;</td>
          <td width="78%">
            <input name="Submit" type="submit" class="formbtn" value="Send Command!" /> 
            <?php if (isset($id) && $a_raid[$id]): ?>
            <input name="id" type="hidden" value="<?=$id;?>" /> 
            <?php endif; ?>
          </td>
        </tr>
				<tr>
  				<td valign="top" colspan="2">
    				<?
              if ($do_action) {
    					  echo("<strong>" . gettext("GMIRROR command output:") . "</strong><br />");
    					  echo('<pre>');
    					  ob_end_flush();
    					
    					  system("/sbin/gmirror $action " . escapeshellarg($object));
    				
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
