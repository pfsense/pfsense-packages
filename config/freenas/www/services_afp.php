<?php
/* $Id$ */
/* ========================================================================== */
/*
    services_afp.php
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
                 gettext("AFP"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['afp'])) {
  $freenas_config['afp'] = array();	
}


$pconfig['enable'] = isset($freenas_config['afp']['enable']);
$pconfig['afpname'] = $freenas_config['afp']['afpname'];
$pconfig['guest'] = isset($freenas_config['afp']['guest']);
$pconfig['local'] = isset($freenas_config['afp']['local']);

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  $pconfig = $_POST;
  
  /* input validation */
  $reqdfields = split(" ", "afpname");
  $reqdfieldsn = split(",", "Afpname");
  
  do_input_validation_new($_POST, $reqdfields, $reqdfieldsn, &$error_bucket);
  
  if ($_POST['enable'] && !$_POST['guest'])
  {
    if (!$_POST['local'])
      $error_bucket[] = array("error" => gettext("You must select at least one authentication method."),
                              "field" => "local");
  }
  if ($_POST['enable'] && !$_POST['local'])
  {
    if (!$_POST['guest'])
      $error_bucket[] = array("error" => gettext("You must select at least one authentication method."),
                              "field" => "name");
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
    $freenas_config['afp']['enable'] = $_POST['enable'] ? true : false;
    $freenas_config['afp']['guest'] = $_POST['guest'] ? true : false;
    $freenas_config['afp']['local'] = $_POST['local'] ? true : false;
    $freenas_config['afp']['afpname'] = $_POST['afpname'];
    
    write_config();
    
    $retval = 0;
    if (!file_exists($d_sysrebootreqd_path))
    {
      /* nuke the cache file */
      config_lock();
      services_afpd_configure();
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
  
  document.iform.guest.disabled = endis;
  document.iform.local.disabled = endis;
  document.iform.afpname.disabled = endis;
  /* color adjustments */
  document.iform.guest.style.backgroundColor = color;
  document.iform.local.style.backgroundColor = color;
  document.iform.afpname.style.backgroundColor = color;
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
  <form id="iform" name="iform" action="services_afp.php" method="post">
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr>
        <td width="100%" valign="middle" class="listtopic" colspan="2">
          <span style="vertical-align: middle; position: relative; left: 0px;"><?=gettext("AFP Server");?></span>
          <span style="vertical-align: middle; position: relative; left: 84%;">
            <input name="enable" type="checkbox" value="yes" <?php if ($pconfig['enable']) echo "checked=\"checked\""; ?> onClick="enable_change(false)" />&nbsp;<?= gettext("Enable"); ?>
          </span>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Server Name");?></td>
        <td width="78%" class="vtable">
          <input name="afpname" type="text" class="formfld unknown" id="afpname" size="20" value="<?=htmlspecialchars($pconfig['afpname']);?>" />
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Authentication");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="guest" id="guest" type="checkbox" value="yes" <?php if ($pconfig['guest']) echo "checked=\"checked\""; ?> />
          <?= gettext("Enable guest access."); ?><br />
          <input name="local" id="local" type="checkbox" value="yes" <?php if ($pconfig['local']) echo "checked=\"checked\""; ?> />
          <?= gettext("Enable local user authentication."); ?>
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
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
<script type="text/javascript">
<!--
enable_change(false);
//-->
</script>
</body>
</html>
