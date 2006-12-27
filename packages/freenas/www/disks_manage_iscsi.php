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
                 gettext("iSCSI Initiator"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (! is_array($freenas_config['iscsi']))
{
	$freenas_config['iscsi'] = array();
}

$pconfig['enable'] = isset($freenas_config['iscsi']['enable']);
$pconfig['targetaddress'] = $freenas_config['iscsi']['targetaddress'];
$pconfig['targetname'] = $freenas_config['iscsi']['targetname'];

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  unset($do_format);
  $pconfig = $_POST;

  /* input validation */
	if ($_POST['enable'])
	{
		$reqdfields = array_merge($reqdfields, explode(" ", "targetaddress targetname"));
		$reqdfieldsn = array_merge($reqdfieldsn, explode(",", "targetaddress,targetname"));
	}
	
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
	
	if ($_POST['enable'] && !is_ipaddr($_POST['targetaddress'])){
      $error_bucket[] = array("error" => gettext("A valid IP address must be specified."),
                              "field" => "targetaddress");
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
		$freenas_config['iscsi']['enable'] = $_POST['enable'] ? true : false;
		$freenas_config['iscsi']['targetaddress'] = $_POST['targetaddress'];
		$freenas_config['iscsi']['targetname'] = $_POST['targetname'];
		
		write_config();
		
		$retval = 0;
		if (! file_exists($d_sysrebootreqd_path))
		{
			/* nuke the cache file */
			config_lock();
			services_iscsi_configure();
			config_unlock();
		}
		$savemsg = get_std_save_message($retval);
	}
}

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */

$jscriptstr = <<<EOD
<script type="text/javascript">
<!--
function enable_change(enable_change) {
	var endis;
	
	endis = !(document.iform.enable.checked || enable_change);
  endis ? color = '#D4D0C8' : color = '#FFFFFF';
  
	document.iform.targetname.disabled = endis;
	document.iform.targetaddress.disabled = endis;
  /* adjust colors */
	document.iform.targetname.style.backgroundColor = color;
	document.iform.targetaddress.style.backgroundColor = color;
}
//-->
</script>

EOD;

$pfSenseHead->addScript($jscriptstr);
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
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
	$tab_array[1] = array(gettext("Format"),          false, "disks_manage_init.php");
	$tab_array[2] = array(gettext("iSCSI Initiator"), true,  "disks_manage_iscsi.php");
  display_top_tabs($tab_array);
?>
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
      <form id="iform" name="iform" action="disks_manage_iscsi.php" method="post">
      <table class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
          <tr>
            <td width="100%" valign="middle" class="listtopic" colspan="2">
              <span style="vertical-align: middle; position: relative; left: 0px;"><?=gettext("iSCSI Initiator");?></span>
              <span style="vertical-align: middle; position: relative; left: 81%;">
                <input name="enable" type="checkbox" value="yes" <?php if ($pconfig['enable']) echo "checked=\"checked\""; ?> onClick="enable_change(false)" />&nbsp;<?= gettext("Enable"); ?>
              </span>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?= gettext("Target IP address"); ?></td>
            <td width="78%" class="vtable">
              <input name="targetaddress" type="text" class="formfld unknown" id="targetaddress" size="20" value="<?=htmlspecialchars($pconfig['targetaddress']);?>" />
              <br /><?= gettext("Target IP address"); ?>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?= gettext("targetname"); ?></td>
            <td width="78%" class="vtable">
              <input name="targetname" type="text" class="formfld unknown" id="targetname" size="20" value="<?=htmlspecialchars($pconfig['targetname']);?>" />
              <br /><?= gettext("targetname"); ?>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top">&nbsp;</td>
            <td width="78%">
              <input id="submit" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" />
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
<script type="text/javascript">
<!--
enable_change(false);
//-->
</script>
</body>
</html>
