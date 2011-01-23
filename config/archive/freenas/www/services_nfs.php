<?php
/* $Id$ */
/* ========================================================================== */
/*
    services_nfs.php
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
                 gettext("NFS"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['nfs']))
{
  $freenas_config['nfs'] = array();
}

$pconfig['enable'] = isset($freenas_config['nfs']['enable']);
$pconfig['bindto'] = $freenas_config['nfs']['bindto'];
$pconfig['serveudp'] = isset($freenas_config['nfs']['serveudp']);
$pconfig['servetcp'] = isset($freenas_config['nfs']['servetcp']);

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  $pconfig = $_POST;
  
  /* input validation */
  $reqdfields = explode(" ", "bindto");
  $reqdfieldsn = explode(",", "IP address to bind to");
  
  do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
  
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
    $freenas_config['nfs']['bindto'] = $_POST['bindto'];
    if (isset($_POST['servetcp'])) {
      $freenas_config['nfs']['servetcp'] = $_POST['servetcp'];
    } else {
      unset($freenas_config['nfs']['servetcp']);
    }
    if (isset($_POST['serveudp'])) {
      $freenas_config['nfs']['serveudp'] = $_POST['serveudp'];
    } else {
      unset($freenas_config['nfs']['serveudp']);
    }
    
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

/* if ajax is calling, give them an update message */
if(isAjax())
  print_info_box_np($savemsg);

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
  
  document.iform.servetcp.disabled = endis;
  document.iform.serveudp.disabled = endis;
  document.iform.bindto.disabled = endis;
  /* color adjustments */
  document.iform.servetcp.style.backgroundColor = color;
  document.iform.serveudp.style.backgroundColor = color;
  document.iform.bindto.style.backgroundColor = color;
}
//-->
</script>

EOD;

$pfSenseHead->addScript($jscriptstr);
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php include("fbegin.inc"); ?>

<form id="iform" name="iform" action="services_nfs.php" method="post">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<div id="inputerrors"></div>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td class="tabnavtbl">
<?php
  $tab_array = array();
  $tab_array[0] = array(gettext("Settings"), true,  "services_nfs.php");
  $tab_array[1] = array(gettext("Exports"),   false, "services_nfs_export.php");
  display_top_tabs($tab_array);
?>  
    </td>
  </tr>
  <tr> 
    <td>
    <div id="mainarea">
    <table class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr>
        <td width="100%" valign="middle" class="listtopic" colspan="2">
          <span style="vertical-align: middle; position: relative; left: 0px;"><?=gettext("NFS Server");?></span>
          <span style="vertical-align: middle; position: relative; left: 84%;">
            <input name="enable" type="checkbox" value="yes" <?php if ($pconfig['enable']) echo "checked=\"checked\""; ?> onClick="enable_change(false)" />&nbsp;<?= gettext("Enable"); ?>
          </span>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Bind to IP address");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <select name="bindto" id="bindto" class="formselect">
            <option value="(ANY)"<?php if ($pconfig['bindto'] == "(ANY)") { echo " selected=\"selected\""; } ?>>
              (ANY)
            </option>
            <option value="<?= $config['interfaces']['wan'][ipaddr] ?>"<?php if ($pconfig['bindto'] == $config['interfaces']['wan'][ipaddr]) { echo " selected=\"selected\""; } ?>>
              <?= $config['interfaces']['wan'][ipaddr] ?>
            </option>
            <option value="<?= $config['interfaces']['lan'][ipaddr] ?>"<?php if ($pconfig['bindto'] == $config['interfaces']['lan'][ipaddr]) { echo " selected=\"selected\""; } ?>>
              <?= $config['interfaces']['lan'][ipaddr] ?>
            </option>
            <?php
              for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++):
            ?>
            <option value="<?= $config['interfaces']['opt' . $i][ipaddr] ?>"<?php if ($pconfig['bindto'] == $config['interfaces']['opt' . $i][ipaddr]) { echo " selected=\"selected\""; } ?>>
              <?= $config['interfaces']['opt' . $i][ipaddr] ?>
            </option>
            <?php endfor; ?>
          </select>
          <br />
          <?= gettext("Use an address from the list to make nfsd and rpcbind bind to a specific address."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Client Types");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <p>
            <input type="checkbox" name="servetcp" id="servetcp" value="on"<?php if ($pconfig['servetcp']) { echo " checked=\"checked\""; } ?>/>
            <label for="servetcp"><?= gettext("Serve TCP NFS clients"); ?></label>
          </p>
          <p>
            <input type="checkbox" name="serveudp" id="serveudp" value="on"<?php if ($pconfig['serveudp']) { echo " checked=\"checked\""; } ?>/>
            <label for="serveudp"><?= gettext("Serve UDP NFS clients"); ?></label>
          </p>
          <p>
            <span class="red"><strong><?= gettext("Note"); ?>: </strong></span>
            <span class="vexpl">
              <?= gettext("Usually it's save to enable support for both, UDP and TCP client types."); ?>
            </span>
          </p>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top">&nbsp;</td>
        <td width="78%">
          <input id="submit" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" />
        </td>
      </tr>
    </table>
    </div>
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
