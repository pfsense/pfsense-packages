<?php
/* $Id$ */
/* ========================================================================== */
/*
    services_ftp.php
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
                 gettext("FTP"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['ftp']))
{
  $freenas_config['ftp'] = array();
}

$pconfig['enable'] = isset($freenas_config['ftp']['enable']);
$pconfig['port'] = $freenas_config['ftp']['port'];
$pconfig['authbackend'] = $freenas_config['ftp']['authentication_backend'];
$pconfig['numberclients'] = $freenas_config['ftp']['numberclients'];
$pconfig['maxconperip'] = $freenas_config['ftp']['maxconperip'];
$pconfig['timeout'] = $freenas_config['ftp']['timeout'];
$pconfig['anonymous'] = isset($freenas_config['ftp']['anonymous']);
$pconfig['localuser'] = isset($freenas_config['ftp']['localuser']);
$pconfig['pasv_max_port'] = $freenas_config['ftp']['pasv_max_port'];
$pconfig['pasv_min_port'] = $freenas_config['ftp']['pasv_min_port'];
$pconfig['pasv_address'] = $freenas_config['ftp']['pasv_address'];
$pconfig['banner'] = $freenas_config['ftp']['banner'];
$pconfig['natmode'] = isset($freenas_config['ftp']['natmode']);
$pconfig['passiveip'] = $freenas_config['ftp']['passiveip'];
$pconfig['fxp'] = isset($freenas_config['ftp']['fxp']);

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  $pconfig = $_POST;
  
  /* input validation */
  if ($_POST['enable']) {
    $reqdfields = array_merge($reqdfields, explode(" ", "numberclients maxconperip timeout port"));
    $reqdfieldsn = array_merge($reqdfieldsn, explode(",", "Numberclients,Maxconperip,Timeout,Port"));
  }
  
  do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
  
  if ($_POST['enable'] && !is_port($_POST['port']))
  {
    $error_bucket[] = array("error" => gettext("The TCP port must be a valid port number."),
                            "field" => "port");
  }
  if ($_POST['enable'] && !is_numericint($_POST['numberclients'])) {
    $error_bucket[] = array("error" => gettext("The maximum Number of client must be a number."),
                            "field" => "numberclients");
  }
  
  if ($_POST['enable'] && !is_numericint($_POST['maxconperip'])) {
    $error_bucket[] = array("error" => gettext("The max con per ip must be a number."),
                            "field" => "maxconperip");
  }
  if ($_POST['enable'] && !is_numericint($_POST['timeout'])) {
    $error_bucket[] = array("error" => gettext("The maximum idle time be a number."),
                            "field" => "timeout");
  }
  
  if ($_POST['enable'] && ($_POST['pasv_address']))
  {
    if (!is_ipaddr($_POST['pasv_address']))
      $error_bucket[] = array("error" => gettext("The pasv address must be a public IP address."),
                              "field" => "pasv_address");
  
  }
  
  if ($_POST['enable'] && ($_POST['pasv_max_port']))
  {
    if (!is_port($_POST['pasv_max_port']))
      $error_bucket[] = array("error" => gettext("The pasv_max_port port must be a valid port number."),
                              "field" => "pasv_max_port");
  }
  
  if ($_POST['enable'] && ($_POST['pasv_min_port']))
  {
    if (!is_port($_POST['pasv_min_port']))
      $error_bucket[] = array("error" => gettext("The pasv_min_port port must be a valid port number."),
                              "field" => "pasv_min_port");
  
  }
  
  if (($_POST['passiveip'] && !is_ipaddr($_POST['passiveip']))) {
    $error_bucket[] = array("error" => gettext("A valid IP address must be specified."),
                            "field" => "passiveip");
  
  }
  
  if (!($_POST['anonymous']) && !($_POST['localuser'])) {
    $input_errors[] = _SRVFTP_MSGVALIDAUTH;
    $error_bucket[] = array("error" => gettext("You must select at minium anonymous or/and local user authentication."),
                            "field" => "localuser");
  
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
    $freenas_config['ftp']['numberclients'] = $_POST['numberclients'];	
    $freenas_config['ftp']['maxconperip'] = $_POST['maxconperip'];
    $freenas_config['ftp']['timeout'] = $_POST['timeout'];
    $freenas_config['ftp']['port'] = $_POST['port'];
    $freenas_config['ftp']['authentication_backend'] = $_POST['authbackend'];
    $freenas_config['ftp']['anonymous'] = $_POST['anonymous'] ? true : false;
    $freenas_config['ftp']['localuser'] = $_POST['localuser'] ? true : false;
    $freenas_config['ftp']['pasv_max_port'] = $_POST['pasv_max_port'];
    $freenas_config['ftp']['pasv_min_port'] = $_POST['pasv_min_port'];
    $freenas_config['ftp']['pasv_address'] = $_POST['pasv_address'];
    $freenas_config['ftp']['banner'] = $_POST['banner'];
    $freenas_config['ftp']['passiveip'] = $_POST['passiveip'];
    $freenas_config['ftp']['fxp'] = $_POST['fxp'] ? true : false;
    $freenas_config['ftp']['natmode'] = $_POST['natmode'] ? true : false;
    $freenas_config['ftp']['enable'] = $_POST['enable'] ? true : false;
    
    write_config();
    
    $retval = 0;
    if (!file_exists($d_sysrebootreqd_path)) {
      /* nuke the cache file */
      config_lock();
      services_wzdftpd_configure();
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
  
  document.iform.port.disabled = endis;
  document.iform.timeout.disabled = endis;
  document.iform.numberclients.disabled = endis;
  document.iform.maxconperip.disabled = endis;
  document.iform.anonymous.disabled = endis;
  document.iform.localuser.disabled = endis;
  document.iform.banner.disabled = endis;
  document.iform.fxp.disabled = endis;
  document.iform.natmode.disabled = endis;
  document.iform.passiveip.disabled = endis;
  document.iform.pasv_max_port.disabled = endis;
  document.iform.pasv_min_port.disabled = endis;
  /* color adjustments */
  document.iform.port.style.backgroundColor = color;
  document.iform.timeout.style.backgroundColor = color;
  document.iform.numberclients.style.backgroundColor = color;
  document.iform.maxconperip.style.backgroundColor = color;
  document.iform.anonymous.style.backgroundColor = color;
  document.iform.localuser.style.backgroundColor = color;
  document.iform.banner.style.backgroundColor = color;
  document.iform.fxp.style.backgroundColor = color;
  document.iform.natmode.style.backgroundColor = color;
  document.iform.passiveip.style.backgroundColor = color;
  document.iform.pasv_max_port.style.backgroundColor = color;
  document.iform.pasv_min_port.style.backgroundColor = color;
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
  <form id="iform" name="iform" action="services_ftp.php" method="post">
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr>
        <td width="100%" valign="middle" class="listtopic" colspan="2">
          <span style="vertical-align: middle; position: relative; left: 0px;"><?=gettext("FTP Server");?></span>
          <span style="vertical-align: middle; position: relative; left: 84%;">
            <input name="enable" type="checkbox" value="yes" <?php if ($pconfig['enable']) echo "checked=\"checked\""; ?> onClick="enable_change(false)" />&nbsp;<?= gettext("Enable"); ?>
          </span>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("TCP port");?></td>
        <td width="78%" class="vtable">
          <input name="port" type="text" class="formfld unknown" id="port" size="20" value="<?=htmlspecialchars($pconfig['port']);?>" />
        </td>
      </tr>
      <?php
        if (file_exists("/usr/local/sbin/wzdftpd")) {
          $a_backends = array();

          $dh  = opendir("/usr/local/share/wzdftpd/backends");
          while (false !== ($filename = readdir($dh))) {
            if (preg_match("/\.so$/", $filename)) {
              $lastslash = strrpos($filename, "/");
              $dot = strrpos($filename, ".");

              $backend_name = str_replace("libwzd",
                                          "",
                                          substr($filename,
                                                 $lastslash,
                                                 $dot - $lastslash));
              $a_backends[] = $backend_name;
            }
          }
        }
      ?>
      <?php if (is_array($a_backends)) : ?>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Authentication Backend");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <select name="authbackend" id="authbackend" class="formselect">
            <?php foreach ($a_backends as $backend) : ?>
              <option value="<?= $backend ?>"><?= $backend ?></option>
            <?php endforeach; ?>
          </select><br />
        <?= gettext("Choose a particular backend, that will be used to authenticate FTP users."); ?>
        </td>
      </tr>
      <?php endif; ?>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Number of clients");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="numberclients" type="text" class="formfld unknown" id="numberclients" size="20" value="<?=htmlspecialchars($pconfig['numberclients']);?>" />
          <br />
          <?= gettext("Maximum number of simultaneous clients."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Max conn per ip");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="maxconperip" type="text" class="formfld unknown" id="maxconperip" size="20" value="<?=htmlspecialchars($pconfig['maxconperip']);?>" />
          <br />
          <?= gettext("Maximum connection per IP address."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Timeout");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="timeout" type="text" class="formfld unknown" id="timeout" size="20" value="<?=htmlspecialchars($pconfig['timeout']);?>" />
          <br />
          <?= gettext("Maximum idle time in seconds."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Anonymous login");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="anonymous" type="checkbox" id="anonymous" value="yes" <?php if ($pconfig['anonymous']) echo "checked=\"checked\""; ?> />
          <?= gettext("Enable Anonymous login"); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Local User");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="localuser" type="checkbox" id="localuser" value="yes" <?php if ($pconfig['localuser']) echo "checked=\"checked\""; ?> />
          <?= gettext("Enable local User login"); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Banner");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <textarea name="banner" cols="65" rows="7" id="banner" class="formpre"><?=htmlspecialchars($pconfig['banner']);?></textarea>
          <br />
          <?= gettext("Greeting banner displayed by FTP when a connection first comes in."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("FXP");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="fxp" type="checkbox" id="fxp" value="yes" <?php if ($pconfig['fxp']) echo "checked=\"checked\""; ?> />
          <?= gettext("Enable FXP protocol."); ?>
          <br />
          <?= gettext("FXP allows transfers between two remote servers without any file data going to the client asking for the transfer (insecure!)."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("NAT mode");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="natmode" type="checkbox" id="natmode" value="yes" <?php if ($pconfig['natmode']) echo "checked=\"checked\""; ?> />
          <?= gettext("Force NAT mode"); ?>
          <br />
          <?= gettext("Enable it if your FTP server is behind a NAT box that doesn't support applicative FTP proxying"); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("Passive IP address");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="passiveip" type="text" class="formfld unknown" id="passiveip" size="30" value="<?=htmlspecialchars($pconfig['passiveip']);?>" />
          <?= gettext("Use this option to override the IP address that FTP daemon will advertise in response to the PASV command."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("pasv_min_port");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="pasv_min_port" type="text" class="formfld unknown" id="pasv_min_port" size="20" value="<?=htmlspecialchars($pconfig['pasv_min_port']);?>" />
          <?= gettext("The minimum port to allocate for PASV style data connections."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?=gettext("pasv_max_port");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <input name="pasv_max_port" type="text" class="formfld unknown" id="pasv_max_port" size="20" value="<?=htmlspecialchars($pconfig['pasv_max_port']);?>" /> 
          <?= gettext("The maximum port to allocate for PASV style data connections."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top">&nbsp;</td>
        <td width="78%">
          <input id="submitt" name="Submitt" type="submit" class="formbtn" value="<?=gettext("Save");?>" />
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
