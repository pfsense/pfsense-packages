<?php
/* $Id$ */
/* ========================================================================== */
/*
    services_samba.php
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
                 gettext("CIFS"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['samba']))
{
  $freenas_config['samba'] = array();
}

if (!is_array($freenas_config['mounts']['mount']))
  $freenas_config['mounts']['mount'] = array();

mount_sort();

$a_mount = &$freenas_config['mounts']['mount'];

$pconfig['netbiosname'] = $freenas_config['samba']['netbiosname'];
$pconfig['workgroup'] = $freenas_config['samba']['workgroup'];
$pconfig['serverdesc'] = $freenas_config['samba']['serverdesc'];
$pconfig['security'] = $freenas_config['samba']['security'];
$pconfig['localmaster'] = $freenas_config['samba']['localmaster'];
$pconfig['winssrv'] = $freenas_config['samba']['winssrv'];
/* $pconfig['hidemount'] = $freenas_config['samba']['hidemount']; */
$pconfig['timesrv'] = $freenas_config['samba']['timesrv'];
$pconfig['unixcharset'] = $freenas_config['samba']['unixcharset'];
$pconfig['doscharset'] = $freenas_config['samba']['doscharset'];
$pconfig['loglevel'] = $freenas_config['samba']['loglevel'];
$pconfig['sndbuf'] = $freenas_config['samba']['sndbuf'];
$pconfig['rcvbuf'] = $freenas_config['samba']['rcvbuf'];
$pconfig['enable'] = isset($freenas_config['samba']['enable']);
$pconfig['recyclebin'] = isset($freenas_config['samba']['recyclebin']);

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  $pconfig = $_POST;
  
  /* input validation */
  if ($_POST['enable']) {
    $reqdfields = array_merge($reqdfields, explode(" ", "netbiosname workgroup security localmaster"));
    $reqdfieldsn = array_merge($reqdfieldsn, explode(",", "Netbiosname,Workgroup,Security, Localmaster"));
  }
  
  do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
  
  if (($_POST['netbiosname'] && !is_domain($_POST['netbiosname']))) {
    $error_bucket[] = array("error" => gettext("The Netbios name contains invalid characters."),
                            "field" => "netbiosname");
  }
  if (($_POST['workgroup'] && !is_domain($_POST['workgroup']))) {
    $error_bucket[] = array("error" => gettext("The Workgroup name contains invalid characters."),
                            "field" => "workgroup");
  }
  if (($_POST['winssrv'] && !is_ipaddr($_POST['winssrv']))) {
    $error_bucket[] = array("error" => gettext("The WINS server must be an IP address."),
                            "field" => "winssrv");
  }
  
  if (!is_numericint($_POST['sndbuf'])) {
    $error_bucket[] = array("error" => gettext("PediaXThe SND Buffer value must be a number."),
                            "field" => "sndbuf");
  }
  
  if (!is_numericint($_POST['rcvbuf'])) {
    $error_bucket[] = array("error" => gettext("The RCV Buffer value must be a number."),
                            "field" => "rcvbuf");
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
    $freenas_config['samba']['netbiosname'] = $_POST['netbiosname'];	
    $freenas_config['samba']['workgroup'] = $_POST['workgroup'];
    $freenas_config['samba']['serverdesc'] = $_POST['serverdesc'];
    $freenas_config['samba']['security'] = $_POST['security'];
    $freenas_config['samba']['localmaster'] = $_POST['localmaster'];
    $freenas_config['samba']['winssrv'] = $_POST['winssrv'];
    /* $freenas_config['samba']['hidemount'] = $_POST['hidemount']; */
    $freenas_config['samba']['timesrv'] = $_POST['timesrv'];
    $freenas_config['samba']['doscharset'] = $_POST['doscharset'];
    $freenas_config['samba']['unixcharset'] = $_POST['unixcharset'];
    $freenas_config['samba']['loglevel'] = $_POST['loglevel'];
    $freenas_config['samba']['sndbuf'] = $_POST['sndbuf'];
    $freenas_config['samba']['rcvbuf'] = $_POST['rcvbuf'];
    $freenas_config['samba']['recyclebin'] = $_POST['recyclebin'] ? true : false;
    $freenas_config['samba']['enable'] = $_POST['enable'] ? true : false;
    
    write_config();
    
    $retval = 0;
    if (!file_exists($d_sysrebootreqd_path)) {
      /* nuke the cache file */
      config_lock();
      services_samba_configure();
      services_zeroconf_configure();
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
  
  document.iform.netbiosname.disabled = endis;
  document.iform.workgroup.disabled = endis;
  document.iform.localmaster.disabled = endis;
  document.iform.winssrv.disabled = endis;
  document.iform.timesrv.disabled = endis;
  document.iform.serverdesc.disabled = endis;
  document.iform.doscharset.disabled = endis;
  document.iform.unixcharset.disabled = endis;
  document.iform.loglevel.disabled = endis;
  document.iform.sndbuf.disabled = endis;
  document.iform.rcvbuf.disabled = endis;
  document.iform.recyclebin.disabled = endis;
  document.iform.security.disabled = endis;
  /* color adjustments */
  document.iform.netbiosname.style.backgroundColor = color;
  document.iform.workgroup.style.backgroundColor = color;
  document.iform.localmaster.style.backgroundColor = color;
  document.iform.winssrv.style.backgroundColor = color;
  document.iform.timesrv.style.backgroundColor = color;
  document.iform.serverdesc.style.backgroundColor = color;
  document.iform.doscharset.style.backgroundColor = color;
  document.iform.unixcharset.style.backgroundColor = color;
  document.iform.loglevel.style.backgroundColor = color;
  document.iform.sndbuf.style.backgroundColor = color;
  document.iform.rcvbuf.style.backgroundColor = color;
  document.iform.recyclebin.style.backgroundColor = color;
  document.iform.security.style.backgroundColor = color;
}
//-->
</script>

EOD;

$pfSenseHead->addScript($jscriptstr);
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php include("fbegin.inc"); ?>

<form id="iform" name="iform" action="services_samba.php" method="post">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<div id="inputerrors"></div>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td class="tabnavtbl">
<?php
  $tab_array = array();
  $tab_array[0] = array(gettext("Settings"), true,  "services_samba.php");
  $tab_array[1] = array(gettext("Shares"),   false, "services_samba_share.php");
  display_top_tabs($tab_array);
?>  
    </td>
  </tr>
  <tr> 
    <td>
    <div id="mainarea">
    <table class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
      <?php if ($input_errors) print_input_errors($input_errors); ?>
      <?php if ($savemsg) print_info_box($savemsg); ?>
      <tr>
        <td width="100%" valign="middle" class="listtopic" colspan="2">
          <span style="vertical-align: middle; position: relative; left: 0px;"><?=gettext("CIFS share");?></span>
          <span style="vertical-align: middle; position: relative; left: 84%;">
            <input name="enable" type="checkbox" value="yes" <?php if ($pconfig['enable']) echo "checked=\"checked\""; ?> onClick="enable_change(false)" />&nbsp;<?= gettext("Enable"); ?>
          </span>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Authentication");?></td>
        <td width="78%" class="vtable">
          <select name="security" class="formselect" id="security">
            <?php
              $types = explode(",", "Anonymous,Local User,Domain");
              $vals = explode(" ", "share user domain");
              $j = 0;
              
              for ($j = 0; $j < count($vals); $j++):
            ?>
            <option value="<?=$vals[$j];?>" <?php if ($vals[$j] == $pconfig['security']) echo "selected=\"selected\"";?>> 
            <?=htmlspecialchars($types[$j]);?>
            </option>
            <?php endfor; ?>
          </select>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("NetBios name");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="netbiosname" type="text" class="formfld unknown" id="netbiosname" size="20" value="<?=htmlspecialchars($pconfig['netbiosname']);?>" /> 
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Workgroup");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="workgroup" type="text" class="formfld unknown" id="workgroup" size="20" value="<?=htmlspecialchars($pconfig['workgroup']);?>" />
          <br />
          <?= gettext("Workgroup to be member of.");?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Description");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="serverdesc" type="text" class="formfld unknown" id="serverdesc" size="30" value="<?=htmlspecialchars($pconfig['serverdesc']);?>" />
          <br />
          <?= gettext("Server description. This can usually be left blank.");?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Dos charset");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <select name="doscharset" class="formselect" id="doscharset">
            <?php
              $types = explode(",", "CP850,CP852,CP437,ASCII");
              $vals = explode(" ", "CP850 CP852 CP437 ASCII");
              $j = 0;
              
              for ($j = 0; $j < count($vals); $j++):
            ?>
            <option value="<?=$vals[$j];?>" <?php if ($vals[$j] == $pconfig['doscharset']) echo "selected=\"selected\"";?>> 
            <?=htmlspecialchars($types[$j]);?>
            </option>
            <?php endfor; ?>
          </select>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Unix charset");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <select name="unixcharset" class="formselect" id="unixcharset">     
            <?php
              $types = explode(",", "UTF-8,iso-8859-1,iso-8859-15,gb2312,ASCII");
              $vals = explode(" ", "UTF-8 iso-8859-1 iso-8859-15 gb2312 ASCII");      
              $j = 0;
              
              for ($j = 0; $j < count($vals); $j++):
            ?>
            <option value="<?=$vals[$j];?>" <?php if ($vals[$j] == $pconfig['unixcharset']) echo "selected=\"selected\"";?>> 
            <?=htmlspecialchars($types[$j]);?>
            </option>
            <?php endfor; ?>
          </select>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Log level");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <select name="loglevel" class="formselect" id="loglevel">
            <?php
              $types = explode(",", "Minimum,Normal,Full,Debug");
              $vals = explode(" ", "1 2 3 10");
              $j = 0;
              
              for ($j = 0; $j < count($vals); $j++):
            ?>
            <option value="<?=$vals[$j];?>" <?php if ($vals[$j] == $pconfig['loglevel']) echo "selected=\"selected\"";?>> 
            <?=htmlspecialchars($types[$j]);?>
            </option>
            <?php endfor; ?>
          </select>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Local Master Browser");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <select name="localmaster" class="formselect" id="localmaster">
            <?php
              $types = explode(",", "Yes,No");
              $vals = explode(" ", "yes no");
              $j = 0;
              
              for ($j = 0; $j < count($vals); $j++):
            ?>
            <option value="<?=$vals[$j];?>" <?php if ($vals[$j] == $pconfig['localmaster']) echo "selected=\"selected\"";?>> 
            <?=htmlspecialchars($types[$j]);?>
            </option>
            <?php endfor; ?>
          </select>
          <br />
          <?= gettext("Allows FreeNAS to try and become a local master browser."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Time Server");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <select name="timesrv" class="formselect" id="timesrv">
            <?php
              $types = explode(",", "Yes,No");
              $vals = explode(" ", "yes no");
              $j = 0;
              
              for ($j = 0; $j < count($vals); $j++):
            ?>
            <option value="<?=$vals[$j];?>" <?php if ($vals[$j] == $pconfig['timesrv']) echo "selected=\"selected\"";?>> 
            <?=htmlspecialchars($types[$j]);?>
            </option>
            <?php endfor; ?>
          </select>
          <br />
          <?= gettext("FreeNAS advertises itself as a time server to Windows clients."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("WINS server");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="winssrv" type="text" class="formfld host" id="winssrv" size="30" value="<?=htmlspecialchars($pconfig['winssrv']);?>" />
          <br />
          <?= gettext("WINS Server IP address."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Recycle Bin");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="recyclebin" type="checkbox" id="recyclebin" value="yes" <?php if ($pconfig['recyclebin']) echo "checked=\"checked\""; ?> />
          <?= gettext("Enable Recycle bin"); ?><br />
          <?= gettext("This will create a recycle bin on the CIFS shares"); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Send Buffer Size");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="sndbuf" type="text" class="formfld unknown" id="sndbuf" size="30" value="<?=htmlspecialchars($pconfig['sndbuf']);?>" />
          <br />
          <?= gettext("Size of send buffer (16384 by default)."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Receive Buffer Size");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="rcvbuf" type="text" class="formfld unknown" id="rcvbuf" size="30" value="<?=htmlspecialchars($pconfig['rcvbuf']);?>" />
          <br />
          <?= gettext("Size of receive buffer (16384 by default)."); ?>
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
