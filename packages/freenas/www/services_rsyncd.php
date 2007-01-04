<?php
/* $Id$ */
/* ========================================================================== */
/*
    services_rsyncd.php
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

$pgtitle = array(gettext("Services"),
                 gettext("RSYNCD"),
                 gettext("Server"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

/* TODO: use pfSense users/groups. */
if (!is_array($freenas_config['system']['user']))
  $freenas_config['system']['user'] = array();

users_sort();

$a_user = &$freenas_config['system']['user'];

if (!is_array($freenas_config['rsync']))
{
  $freenas_config['rsync'] = array();	
}

$pconfig['readonly'] = $freenas_config['rsyncd']['readonly'];
$pconfig['port'] = $freenas_config['rsyncd']['port'];
$pconfig['motd'] = $freenas_config['rsyncd']['motd'];
$pconfig['maxcon'] = $freenas_config['rsyncd']['maxcon'];
$pconfig['rsyncd_user'] = $freenas_config['rsyncd']['rsyncd_user'];
$pconfig['enable'] = isset($freenas_config['rsyncd']['enable']);

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
    $reqdfields = array_merge($reqdfields, explode(" ", "readonly port"));
    $reqdfieldsn = array_merge($reqdfieldsn, explode(",", "Readonly,Port"));
  }
  
  do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
  
  if ($_POST['enable']) {
    if (!is_port($_POST['port']))
      $error_bucket[] = array("error" => gettext("The TCP port must be a valid port number."),
                              "field" => "port");
    else if (!is_numericint($_POST['maxcon']))
      $error_bucket[] = array("error" => gettext("The value provided by the maximum connections field is not a number"),
                              "field" => "maxcon");
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
    $freenas_config['rsyncd']['readonly'] = $_POST['readonly'];	
    $freenas_config['rsyncd']['port'] = $_POST['port'];
    $freenas_config['rsyncd']['motd'] = $_POST['motd'];
    $freenas_config['rsyncd']['maxcon'] = $_POST['maxcon'];
    $freenas_config['rsyncd']['enable'] = $_POST['enable'] ? true : false;
    $freenas_config['rsyncd']['rsyncd_user'] = $_POST['rsyncd_user'];
    
    write_config();
    
    $retval = 0;
    if (!file_exists($d_sysrebootreqd_path))
    {
      /* nuke the cache file */
      config_lock();
      services_rsyncd_configure();
      services_zeroconf_configure();
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
  
  document.iform.readonly.disabled = endis;
  document.iform.port.disabled = endis;
  document.iform.motd.disabled = endis;
  document.iform.maxcon.disabled = endis;
  document.iform.rsyncd_user.disabled = endis;
  /* adjust colors */
  document.iform.readonly.style.backgroundColor = color;
  document.iform.port.style.backgroundColor = color;
  document.iform.motd.style.backgroundColor = color;
  document.iform.maxcon.style.backgroundColor = color;
  document.iform.rsyncd_user.style.backgroundColor = color;
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
  $tab_array[0] = array(gettext("Server"), true, "services_rsyncd.php");
  $tab_array[1] = array(gettext("Client"), false, "services_rsyncd_client.php");
  $tab_array[2] = array(gettext("Local"), false, "services_rsyncd_local.php");
  display_top_tabs($tab_array);
  ?>
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
      <form id="iform" name="iform" action="services_rsyncd.php" method="post">
      <table class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
          <tr>
            <td width="100%" valign="middle" class="listtopic" colspan="2">
              <span style="vertical-align: middle; position: relative; left: 0px;"><?=gettext("Rsync Daemon");?></span>
              <span style="vertical-align: middle; position: relative; left: 81%;">
                <input name="enable" type="checkbox" value="yes" <?php if ($pconfig['enable']) echo "checked=\"checked\""; ?> onClick="enable_change(false)" />&nbsp;<?= gettext("Enable"); ?>
              </span>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?= gettext("Read only"); ?></td>
            <td width="78%" class="vtable">
              <select name="readonly" class="formselect" id="readonly">
                <?php
                  $types = explode(",", "Yes,No");
                  $vals = explode(" ", "yes no");                  
                  $j = 0;
                  
                  for ($j = 0; $j < count($vals); $j++):
                ?>
                <option value="<?=$vals[$j];?>" <?php if ($vals[$j] == $pconfig['readonly']) echo "selected=\"selected\"";?>> 
                <?=htmlspecialchars($types[$j]);?>
                </option>
                <?php endfor; ?>
              </select>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?= gettext("Map to user"); ?></td>
            <td width="78%" class="vtable">
              <select name="rsyncd_user" class="formselect" id="rsyncd_user">
                <option value="ftp"<?php if ($pconfig['rsyncd_user'] == "ftp") echo "selected";?>> 
                <?php echo htmlspecialchars("guest"); ?>
                <?php foreach ($a_user as $user): ?>
                <option value="<?=$user['name'];?>"<?php if ($user['name'] == $pconfig['rsyncd_user']) echo "selected";?>> 
                <?php echo htmlspecialchars($user['name']); ?>
                </option>
                <?php endforeach; ?>
              </select>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?= gettext("TCP port"); ?></td>
            <td width="78%" class="vtable">
              <input name="port" type="text" class="formfld" id="port" size="20" value="<?=htmlspecialchars($pconfig['port']);?>" /> 
              <br /><?= gettext("Alternate TCP port."); ?>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top" class="vncellreq"><?= gettext("Maximum connections"); ?></td>
            <td width="78%" class="vtable">
              <input name="maxcon" type="text" class="formfld" id="maxcon" size="20" value="<?=htmlspecialchars($pconfig['maxcon']);?>" /> 
              <br /><?= gettext("Maximum number of simultaneous connections. Default is 0 (unlimited)"); ?>
            </td>
          </tr>
          <tr>
            <td width="22%" valign="top" class="vncell"><?= gettext("MOTD"); ?></td>
            <td width="78%" class="vtable">
              <textarea name="motd" cols="65" rows="7" id="motd" class="formpre"><?=htmlspecialchars($pconfig['motd']);?></textarea>
              <br /> 
              <?= gettext("message of the day");?>
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
