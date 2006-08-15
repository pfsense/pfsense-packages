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

$pgtitle = array(gettext("Services"),
                 gettext("NFS"));

require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['nfs']))
{
	$freenas_config['nfs'] = array();
}

$pconfig['enable'] = isset($freenas_config['nfs']['enable']);
$pconfig['mapall'] = $freenas_config['nfs']['mapall'];

list($pconfig['network'],$pconfig['network_subnet']) = 
		explode('/', $freenas_config['nfs']['nfsnetwork']);

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  $pconfig = $_POST;

  /* input validation */
	$reqdfields = explode(" ", "network network_subnet");
	$reqdfieldsn = explode(",", "Destination network,Destination network bit count");
	
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
  
	if (($_POST['network'] && !is_ipaddr($_POST['network']))) {
    $error_bucket[] = array("error" => gettext("A valid network must be specified."),
                            "field" => "network");
	}
	
	if (($_POST['network_subnet'] && !is_numeric($_POST['network_subnet']))) {
    $error_bucket[] = array("error" => gettext("A valid network bit count must be specified."),
                            "field" => "network_subnet");

	}
	
	$osn = gen_subnet($_POST['network'], $_POST['network_subnet']) . "/" . $_POST['network_subnet'];

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
		$freenas_config['nfs']['enable'] = $_POST['enable'] ? true : false;
		$freenas_config['nfs']['mapall'] = $_POST['mapall'];
		$freenas_config['nfs']['nfsnetwork'] = $osn;
		write_config();
		
		$retval = 0;
		if (!file_exists($d_sysrebootreqd_path))
		{
			/* nuke the cache file */
			config_lock();
			services_nfs_configure();	
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
  
	document.iform.mapall.disabled = endis;
	document.iform.network.disabled = endis;
  document.iform.network_subnet.disabled = endis;
  /* color adjustments */
	document.iform.mapall.style.backgroundColor = color;
	document.iform.network.style.backgroundColor = color;
  document.iform.network_subnet.style.backgroundColor = color;
}
//-->
</script>

EOD;

$pfSenseHead->addScript($jscriptstr);
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
  <div id="inputerrors"></div>
  <form id="iform" name="iform" action="services_nfs.php" method="post">
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr>
        <td width="100%" valign="middle" class="listtopic" colspan="2">
          <span style="vertical-align: middle; position: relative; left: 0px;"><?=gettext("NFS Server");?></span>
          <span style="vertical-align: middle; position: relative; left: 84%;">
            <input name="enable" type="checkbox" value="yes" <?php if ($pconfig['enable']) echo "checked=\"checked\""; ?> onClick="enable_change(false)" />&nbsp;<?= gettext("Enable"); ?>
          </span>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("map all user to root");?></td>
        <td width="78%" class="vtable">
					<select name="mapall" class="formselect" id="mapall">
            <?php
              $types = explode(",", "Yes,No");
					    $vals = explode(" ", "yes no");
              $j = 0;
              
              for ($j = 0; $j < count($vals); $j++):
            ?>
            <option value="<?=$vals[$j];?>" <?php if ($vals[$j] == $pconfig['mapall']) echo "selected=\"selected\"";?>> 
            <?=htmlspecialchars($types[$j]);?>
            </option>
            <?php endfor; ?>
          </select>
          <br />
          <?= gettext("All users will have the root privilege."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Authorised network");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="network" type="text" class="formfld host" id="network" size="20" value="<?=htmlspecialchars($pconfig['network']);?>" /> 
				  / 
          <select name="network_subnet" class="formselect" id="network_subnet">
            <?php for ($i = 32; $i >= 1; $i--): ?>
              <option value="<?=$i;?>" <?php if ($i == $pconfig['network_subnet']) echo "selected=\"selected\""; ?>>
              <?=$i;?>
              </option>
            <?php endfor; ?>
          </select>
          <br />
          <?= gettext("Network that is authorised to access to NFS share"); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top">&nbsp;</td>
        <td width="78%">
          <input id="submit" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" />
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top">&nbsp;</td>
        <td width="78%">
          <span class="red">
            <strong><?= gettext("WARNING"); ?></strong>
          </span>
          <span class="vexpl">
            <?= gettext("The name of the exported directories are : /mnt/sharename"); ?>
          </span>
        </td>
      </tr>
    </table>
  </form>
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
<script type="text/javascript">
<!--
enable_change(false);
//-->
</script>
</body>
</html>
