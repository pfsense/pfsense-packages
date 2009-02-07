<?php
/* $Id$ */
/* ========================================================================== */
/*
    diag_fn_logs_ftp.php
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

$pgtitle = array(gettext("Diagnostics"),
                 gettext("System logs"),
                 gettext("FTP"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

$nentries = $config['syslog']['nentries'];
if (!$nentries) { $nentries = 50; }

if ($_POST['clear'])
{
  exec("/usr/sbin/clog -i -s 262144 /var/log/ftp.log");
  /* redirect to avoid reposting form data on refresh */
  pfSenseHeader("diag_fn_logs_ftp.php");
  exit;
}

/* if ajax is calling, give them an update message */
if(isAjax())
  print_info_box_np($savemsg);

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
echo $pfSenseHead->getHTML();

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php include("fbegin.inc"); ?>
<form action="diag_fn_logs_ftp.php" method="post" name="iform" id="iform">
<div id="inputerrors"></div>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td>
<?php
  $tab_array = array();
  $tab_array[] = array(gettext("Samba"),    false, "diag_fn_logs_samba.php");
  $tab_array[] = array(gettext("FTP"),      true,  "diag_fn_logs_ftp.php");
  $tab_array[] = array(gettext("RSYNCD"),   false, "diag_fn_logs_rsyncd.php");
  $tab_array[] = array(gettext("SSHD"),     false, "diag_fn_logs_sshd.php");
  $tab_array[] = array(gettext("SMARTD"),   false, "diag_fn_logs_smartd.php");
  $tab_array[] = array(gettext("Daemon"),   false, "diag_fn_logs_daemon.php");
  $tab_array[] = array(gettext("Settings"), false, "diag_fn_logs_settings.php");
  display_top_tabs($tab_array);?>
  </td></tr>
  <tr>
    <td>
      <div id="mainarea">
        <table class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
          <tr> 
            <td class="listtopic"> 
              Last <?=$nentries;?> FTP log entries
            </td>
          </tr>
          <?php dump_clog("/var/log/ftp.log", $nentries); ?>
          <tr>
            <td align="left" valign="top" height="12">&nbsp;</td>
          </tr>
          <tr>
            <td align="left" valign="top">
              <input name="clear" type="submit" class="formbtn" value="Clear log" />
            </td>
          </tr>
        </table>
      </div>
    </td>
  </tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
