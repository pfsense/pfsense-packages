<?php
/* $Id$ */
/* ========================================================================== */
/*
    services_unison.php
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

/*
       *************************

	Unison Installation Notes

	To work, unison requires an environment variable UNISON to point at
	a writable directory. Unison keeps information there between syncs to
	speed up the process.

	When a user runs the unison client, it will try to invoke ssh to
	connect to the this server. Giving the local ssh a UNISON environment
	variable without compromising ssh turned out to be non-trivial.
	The solution is to modify the default path found in /etc/login.conf.
	The path is seeded with "UNISON=/mnt" and this updated by the
	/etc/inc/services.inc file.

	Todo:
	* 	Arguably, a full client install could be done too to
	allow FreeNAS to FreeNAS syncing.
	
       *************************
*/

$pgtitle = array(gettext("Services"),
                 gettext("Unison"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['unison'])) {
  $freenas_config['unison'] = array();	
}

$pconfig['enable'] = isset($freenas_config['unison']['enable']);
$pconfig['share'] = $freenas_config['unison']['share'];
$pconfig['workdir'] = isset($freenas_config['unison']['workdir']);
$pconfig['makedir'] = isset($freenas_config['unison']['makedir']);

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  $pconfig = $_POST;
  
  /* input validation */
  $reqdfields = split(" ", "share workdir");
  $reqdfieldsn = split(",", "Share,Working Directory");
  
  do_input_validation_new($_POST, $reqdfields, $reqdfieldsn, &$error_bucket);
  
  $fullpath = "/mnt/{$_POST['share']}/{$_POST['workdir']}";
  
  if (!$_POST['makedir'] && ($fullpath) && (!file_exists($fullpath))) {
    $error_bucket[] = array("error" => gettext("The combination of share and working directory does not exist."),
                            "field" => "workdir");
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
    $freenas_config['unison']['share'] = $_POST['share'];
    $freenas_config['unison']['workdir'] = $_POST['workdir'];
    $freenas_config['unison']['enable'] = $_POST['enable'] ? true : false;
    $freenas_config['unison']['makedir'] = $_POST['makedir'] ? true : false;
    
    write_config();
    
    $retval = 0;
    if (!file_exists($d_sysrebootreqd_path))
    {
      /* nuke the cache file */
      config_lock();
      services_unison_configure();
      /* services_zeroconf_configure(); */
      config_unlock();
    }
    
    $savemsg = get_std_save_message($retval);
  }
}

/* retrieve mounts to build list of share names */
if (!is_array($freenas_config['mounts']['mount']))
  $freenas_config['mounts']['mount'] = array();

mount_sort();

$a_mount = &$freenas_config['mounts']['mount'];

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
  
  document.iform.share.disabled = endis;
  document.iform.workdir.disabled = endis;
  document.iform.makedir.disabled = endis;
  /* color adjustments */
  document.iform.share.style.backgroundColor = color;
  document.iform.workdir.style.backgroundColor = color;
  document.iform.makedir.style.backgroundColor = color;
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
  <form id="iform" name="iform" action="services_unison.php" method="post">
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr>
        <td width="100%" valign="middle" class="listtopic" colspan="2">
          <span style="vertical-align: middle; position: relative; left: 0px;"><?=gettext("Unison File Synchronisation");?></span>
          <span style="vertical-align: middle; position: relative; left: 72%;">
            <input name="enable" type="checkbox" value="yes" <?php if ($pconfig['enable']) echo "checked=\"checked\""; ?> onClick="enable_change(false)" />&nbsp;<?= gettext("Enable"); ?>
          </span>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Share");?></td>
        <td width="78%" class="vtable">
          <select name="share" class="formselect" id="share">
            <?php foreach ($a_mount as $mount): $tmp=$mount['sharename']; ?>
            <option value="<?=$tmp;?>"
            <?php if ($tmp == $pconfig['share']) echo "selected=\"selected\"";?>><?=$tmp?></option>
            <?php endforeach; ?>
          </select>
          <br />
          <?= gettext("You may need enough space to duplicate all files being synced."); ?>.
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Working Directory");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="workdir" type="text" class="formfld file" id="workdir" size="20" value="<?=htmlspecialchars($pconfig['workdir']);?>" />
          <?= gettext("Where the working files will be stored"); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Create");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="makedir" type="checkbox" id="makedir" value="yes" <?php if ($pconfig['makedir']) echo "checked=\"checked\""; ?> />
          <?= gettext("Create work directory if it doesn't exist"); ?>
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
            <strong><?= gettext("Note"); ?>:</strong>
          </span>
          <br />
          <?= gettext("<a href='/system_advanced.php'>SSHD</a> must be enabled for Unison to work, and the <a href='/system_usermanager.php'>user</a> must have Full Shell enabled."); ?>
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
