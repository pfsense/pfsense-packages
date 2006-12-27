<?php
/* $Id$ */
/*
	diag_logs_settings.php
	Copyright (C) 2004 Scott Ullrich
	All rights reserved.

	originially part of m0n0wall (http://m0n0.ch/wall)
	Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
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

$pgtitle = array(gettext("Diagnostics"),
                 gettext("System logs"),
                 gettext("Settings"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

$pconfig['reverse'] = isset($config['syslog']['reverse']);
$pconfig['nentries'] = $config['syslog']['nentries'];
$pconfig['remoteserver'] = $config['syslog']['remoteserver'];
$pconfig['filter'] = isset($config['syslog']['filter']);
$pconfig['dhcp'] = isset($config['syslog']['dhcp']);
$pconfig['portalauth'] = isset($config['syslog']['portalauth']);
$pconfig['vpn'] = isset($config['syslog']['vpn']);
$pconfig['system'] = isset($config['syslog']['system']);
$pconfig['enable'] = isset($config['syslog']['enable']);
$pconfig['logdefaultblock'] = !isset($config['syslog']['nologdefaultblock']);
$pconfig['rawfilter'] = isset($config['syslog']['rawfilter']);
$pconfig['disablelocallogging'] = isset($config['syslog']['disablelocallogging']);
$pconfig['webservlogs'] = isset($config['syslog']['webservlogs']);

$pconfig['sshd'] = isset($config['syslog']['sshd']);
$pconfig['ftp'] = isset($config['syslog']['ftp']);
$pconfig['rsyncd'] = isset($config['syslog']['rsyncd']);
$pconfig['smartd'] = isset($config['syslog']['smartd']);
$pconfig['daemon'] = isset($config['syslog']['daemon']);

if (!$pconfig['nentries'])
	$pconfig['nentries'] = 50;

if ($_POST) {

	unset($input_errors);
	$pconfig = $_POST;

	/* input validation */
	if ($_POST['enable'] && !is_ipaddr($_POST['remoteserver'])) {
		$input_errors[] = gettext("A valid IP address must be specified.");
	}
	if (($_POST['nentries'] < 5) || ($_POST['nentries'] > 2000)) {
		$input_errors[] = gettext("Number of log entries to show must be between 5 and 2000.");
	}

	if (!$input_errors) {
		$config['syslog']['reverse'] = $_POST['reverse'] ? true : false;
		$config['syslog']['nentries'] = (int)$_POST['nentries'];
		$config['syslog']['remoteserver'] = $_POST['remoteserver'];
		$config['syslog']['filter'] = $_POST['filter'] ? true : false;
		$config['syslog']['dhcp'] = $_POST['dhcp'] ? true : false;
		$config['syslog']['portalauth'] = $_POST['portalauth'] ? true : false;
		$config['syslog']['vpn'] = $_POST['vpn'] ? true : false;
		$config['syslog']['system'] = $_POST['system'] ? true : false;
		$config['syslog']['disablelocallogging'] = $_POST['disablelocallogging'] ? true : false;
		$config['syslog']['enable'] = $_POST['enable'] ? true : false;
		$oldnologdefaultblock = isset($config['syslog']['nologdefaultblock']);
		$config['syslog']['nologdefaultblock'] = $_POST['logdefaultblock'] ? false : true;
		$config['syslog']['rawfilter'] = $_POST['rawfilter'] ? true : false;
		$config['syslog']['webservlogs'] = $_POST['webservlogs'] ? true : false;
		$config['syslog']['sshd'] = $_POST['sshd'] ? true : false;
		$config['syslog']['ftp'] = $_POST['ftp'] ? true : false;
		$config['syslog']['rsyncd'] = $_POST['rsyncd'] ? true : false;
		$config['syslog']['smartd'] = $_POST['smartd'] ? true : false;
		$config['syslog']['daemon'] = $_POST['daemon'] ? true : false;
		if($config['syslog']['enable'] == false) 
			unset($config['syslog']['remoteserver']);
		
		write_config();

		$retval = 0;
		config_lock();
		$retval = system_syslogd_start();
		if ($oldnologdefaultblock !== isset($config['syslog']['nologdefaultblock']))
			$retval |= filter_configure();
		config_unlock();
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
<script language="JavaScript" type="text/javascript">
<!--
function enable_change(enable_over) {
  endis = !(document.iform.enable.checked || enable_over);
  endis ? color = '#D4D0C8' : color = '#FFFFFF';
  
	if (document.iform.enable.checked || enable_over) {
		document.iform.remoteserver.disabled = 0;
		document.iform.filter.disabled = 0;
		document.iform.dhcp.disabled = 0;
		document.iform.portalauth.disabled = 0;
		document.iform.vpn.disabled = 0;
    document.iform.system.disabled = 0;
		document.iform.webservlogs.disabled = 0;
    document.iform.sshd.disabled = 0;
    document.iform.ftp.disabled = 0;
    document.iform.rsyncd.disabled = 0;
    document.iform.smartd.disabled = 0;
    document.iform.daemon.disabled = 0;
		
	} else {
		document.iform.remoteserver.disabled = 1;
		document.iform.filter.disabled = 1;
		document.iform.dhcp.disabled = 1;
		document.iform.portalauth.disabled = 1;
		document.iform.vpn.disabled = 1;
		document.iform.system.disabled = 1;
		document.iform.webservlogs.disabled = 1;
    document.iform.sshd.disabled = 1;
    document.iform.ftp.disabled = 1;
    document.iform.rsyncd.disabled = 1;
    document.iform.smartd.disabled = 1;
    document.iform.daemon.disabled = 1;
	}
  
  /* color adjustments */
  document.iform.remoteserver.style.backgrounColor = color;
  document.iform.filter.style.backgrounColor = color;
  document.iform.dhcp.style.backgrounColor = color;
  document.iform.portalauth.style.backgrounColor = color;
  document.iform.vpn.style.backgrounColor = color;
  document.iform.system.style.backgrounColor = color;
  document.iform.webservlogs.style.backgrounColor = color;
  document.iform.sshd.style.backgrounColor = color;
  document.iform.ftp.style.backgrounColor = color;
  document.iform.rsyncd.style.backgrounColor = color;
  document.iform.smartd.style.backgrounColor = color;
  document.iform.daemon.style.backgrounColor = color;
}
// -->
</script>

EOD;

$pfSenseHead->addScript($jscriptstr);
echo $pfSenseHead->getHTML();

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php include("fbegin.inc"); ?>
<form action="diag_fn_logs_settings.php" method="post" name="iform" id="iform">
<div id="inputerrors"></div>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Samba"),    false, "diag_fn_logs_samba.php");
	$tab_array[] = array(gettext("FTP"),      false, "diag_fn_logs_ftp.php");
	$tab_array[] = array(gettext("RSYNCD"),   false, "diag_fn_logs_rsyncd.php");
  $tab_array[] = array(gettext("SSHD"),     false, "diag_fn_logs_sshd.php");
	$tab_array[] = array(gettext("SMARTD"),   false, "diag_fn_logs_smartd.php");
	$tab_array[] = array(gettext("Daemon"),   false, "diag_fn_logs_daemon.php");
	$tab_array[] = array(gettext("Settings"), true,  "diag_fn_logs_settings.php");
	display_top_tabs($tab_array);?>
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
	  <table class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
                      <tr>
                        <td width="22%" valign="top" class="vtable">&nbsp;</td>
                        <td width="78%" class="vtable"> <input name="reverse" class="formfld" type="checkbox" id="reverse" value="yes" <?php if ($pconfig['reverse']) echo "checked"; ?> />
                          <strong><?=gettext("Show log entries in reverse order (newest entries on top)");?></strong></td>
                      </tr>
                      <tr>
                        <td width="22%" valign="top" class="vtable">&nbsp;</td>
                        <td width="78%" class="vtable"><?=gettext("Number of log entries to show");?>:
                          <input name="nentries" id="nentries" type="text" class="formfld unknown" size="4" value="<?=htmlspecialchars($pconfig['nentries']);?>" /></td>
                      </tr>
                      <tr>
                        <td valign="top" class="vtable">&nbsp;</td>
                        <td class="vtable"> <input name="logdefaultblock" class="formfld" type="checkbox" id="logdefaultblock" value="yes" <?php if ($pconfig['logdefaultblock']) echo "checked"; ?> />
                          <strong><?=gettext("Log packets blocked by the default rule");?></strong><br />
                          <?=gettext("Hint: packets that are blocked by the
                          implicit default block rule will not be logged anymore
                          if you uncheck this option. Per-rule logging options are not affected.");?></td>
                      </tr>
                      <tr>
                        <td valign="top" class="vtable">&nbsp;</td>
                        <td class="vtable"> <input name="rawfilter" class="formfld" type="checkbox" id="rawfilter" value="yes" <?php if ($pconfig['rawfilter']) echo "checked"; ?> />
                          <strong><?=gettext("Show raw filter logs");?></strong><br />
                          <?=gettext("Hint: If this is checked, filter logs are shown as generated by the packet filter, without any formatting. This will reveal more detailed information.");?> </td>
                      </tr>
                      <tr>
                        <td width="22%" valign="top" class="vtable">&nbsp;</td>
                        <td width="78%" class="vtable"> <input name="disablelocallogging" class="formfld" type="checkbox" id="disablelocallogging" value="yes" <?php if ($pconfig['disablelocallogging']) echo "checked"; ?> onclick="enable_change(false)" />
                          <strong><?=gettext("Disable writing log files to the local disk");?></strong></td>
                       </tr>		      
                      <tr>
                        <td width="22%" valign="top" class="vtable">&nbsp;</td>
                        <td width="78%" class="vtable"> <input name="enable" class="formfld" type="checkbox" id="enable" value="yes" <?php if ($pconfig['enable']) echo "checked"; ?> onclick="enable_change(false)" />
                          <strong><?=gettext("Enable syslog'ing to remote syslog server");?></strong></td>
                      </tr>
                      <tr>
                        <td width="22%" valign="top" class="vncell">Remote syslog
                          server</td>
                        <td width="78%" class="vtable">
                          <input name="remoteserver" id="remoteserver" type="text" class="formfld unknown" size="20" value="<?=htmlspecialchars($pconfig['remoteserver']);?>" />
                          <br /><?=gettext("IP address of remote syslog server");?>
                          <br /><br />
                          <input name="system" id="system" class="formfld" type="checkbox" value="yes" onclick="enable_change(false)" <?php if ($pconfig['system']) echo "checked=\"checked\""; ?> />
                          <?=gettext("system events");?><br />
                          <input name="filter" id="filter" class="formfld" type="checkbox" value="yes" <?php if ($pconfig['filter']) echo "checked=\"checked\""; ?> />
                          <?=gettext("firewall events");?><br />
                          <input name="dhcp" id="dhcp" class="formfld" type="checkbox" value="yes" <?php if ($pconfig['dhcp']) echo "checked=\"checked\""; ?> />
                          <?=gettext("DHCP service events");?><br />
                          <input name="portalauth" id="portalauth" class="formfld" type="checkbox" value="yes" <?php if ($pconfig['portalauth']) echo "checked=\"checked\""; ?> />
                          <?=gettext("Portal Auth");?><br />
                          <input name="vpn" id="vpn" class="formfld" type="checkbox" value="yes" <?php if ($pconfig['vpn']) echo "checked=\"checked\""; ?> />
                          <?=gettext("PPTP VPN events");?><br />
						              <input name="webservlogs" id="webservlogs" class="formfld" type="checkbox" value="yes" <?php if ($pconfig['webservlogs']) echo "checked=\"checked\""; ?> />  
                          <?=gettext("webConfigurator Logs");?><br />
						              <input name="sshd" id="sshd" class="formfld" type="checkbox" value="yes" <?php if ($pconfig['sshd']) echo "checked=\"checked\""; ?> />  
                          <?=gettext("SSHD events");?><br />
						              <input name="ftp" id="ftp" class="formfld" type="checkbox" value="yes" <?php if ($pconfig['ftp']) echo "checked=\"checked\""; ?> />  
                          <?=gettext("FTP events");?><br />
						              <input name="rsyncd" id="rsyncd" class="formfld" type="checkbox" value="yes" <?php if ($pconfig['rsyncd']) echo "checked=\"checked\""; ?> />  
                          <?=gettext("RSYNCD events");?><br />
						              <input name="smartd" id="smartd" class="formfld" type="checkbox" value="yes" <?php if ($pconfig['smartd']) echo "checked=\"checked\""; ?> />  
                          <?=gettext("SMARTD events");?><br />
						              <input name="daemon" id="daemon" class="formfld" type="checkbox" value="yes" <?php if ($pconfig['daemon']) echo "checked=\"checked\""; ?> />  
                          <?=gettext("Daemon events");?><br />
						</td>
                      </tr>
                      <tr>
                        <td width="22%" valign="top">&nbsp;</td>
                        <td width="78%"> <input id="submit" name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" />
                        </td>
                      </tr>
                      <tr>
                        <td width="22%" height="53" valign="top">&nbsp;</td>
                        <td width="78%"><strong><span class="red"><?=gettext("Note");?>:</span></strong><br />
                          <?=gettext("
                          syslog sends UDP datagrams to port 514 on the specified
                          remote syslog server. Be sure to set syslogd on the
                          remote server to accept syslog messages from {$g['product_name']}.
                          ");?>
                        </td>
                      </tr>
                    </table>
	</div>
    </td>
  </tr>
</table>
</form>
<script language="JavaScript" type="text/javascript">
<!--
enable_change(false);
//-->
</script>
<?php include("fend.inc"); ?>
</body>
</html>
