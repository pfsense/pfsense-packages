<?php
/*
	snort_interfaces_global.php
	part of m0n0wall (http://m0n0.ch/wall)

	Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
	Copyright (C) 2003-2006 Robert Zelaya
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

$pgtitle = "Services:[Snort][Global Settings]";
require("guiconfig.inc");

/* make things short  */
$pconfig['snortdownload'] = $config['installedpackages']['snortglobal']['snortdownload'];
$pconfig['oinkmastercode'] = $config['installedpackages']['snortglobal']['oinkmastercode'];
$pconfig['emergingthreats'] = $config['installedpackages']['snortglobal']['emergingthreats'];
$pconfig['rm_blocked'] = $config['installedpackages']['snortglobal']['rm_blocked'];
$pconfig['autorulesupdate7'] = $config['installedpackages']['snortglobal']['autorulesupdate7'];
$pconfig['whitelistvpns'] = $config['installedpackages']['snortglobal']['whitelistvpns'];
$pconfig['clickablalerteurls'] = $config['installedpackages']['snortglobal']['clickablalerteurls'];
$pconfig['associatealertip'] = $config['installedpackages']['snortglobal']['associatealertip'];

if ($_POST) {

	unset($input_errors);
	$pconfig = $_POST;

	/* input validation */
	if ($_POST['enable']) {
		$reqdfields = explode(" ", "interface");
		$reqdfieldsn = explode(",", "Interface");

		do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

		if ($_POST['httpslogin_enable']) {
		 	if (!$_POST['cert'] || !$_POST['key']) {
				$input_errors[] = "Certificate and key must be specified for HTTPS login.";
			} else {
				if (!strstr($_POST['cert'], "BEGIN CERTIFICATE") || !strstr($_POST['cert'], "END CERTIFICATE"))
					$input_errors[] = "This certificate does not appear to be valid.";
				if (!strstr($_POST['key'], "BEGIN RSA PRIVATE KEY") || !strstr($_POST['key'], "END RSA PRIVATE KEY"))
					$input_errors[] = "This key does not appear to be valid.";
			}

			if (!$_POST['httpsname'] || !is_domain($_POST['httpsname'])) {
				$input_errors[] = "The HTTPS server name must be specified for HTTPS login.";
			}
		}
	}

	if ($_POST['timeout'] && (!is_numeric($_POST['timeout']) || ($_POST['timeout'] < 1))) {
		$input_errors[] = "The timeout must be at least 1 minute.";
	}
	if ($_POST['idletimeout'] && (!is_numeric($_POST['idletimeout']) || ($_POST['idletimeout'] < 1))) {
		$input_errors[] = "The idle timeout must be at least 1 minute.";
	}
	if (($_POST['radiusip'] && !is_ipaddr($_POST['radiusip']))) {
		$input_errors[] = "A valid IP address must be specified. [".$_POST['radiusip']."]";
	}
	if (($_POST['radiusip2'] && !is_ipaddr($_POST['radiusip2']))) {
		$input_errors[] = "A valid IP address must be specified. [".$_POST['radiusip2']."]";
	}
	if (($_POST['radiusport'] && !is_port($_POST['radiusport']))) {
		$input_errors[] = "A valid port number must be specified. [".$_POST['radiusport']."]";
	}
	if (($_POST['radiusport2'] && !is_port($_POST['radiusport2']))) {
		$input_errors[] = "A valid port number must be specified. [".$_POST['radiusport2']."]";
	}
	if (($_POST['radiusacctport'] && !is_port($_POST['radiusacctport']))) {
		$input_errors[] = "A valid port number must be specified. [".$_POST['radiusacctport']."]";
	}
	if ($_POST['maxproc'] && (!is_numeric($_POST['maxproc']) || ($_POST['maxproc'] < 4) || ($_POST['maxproc'] > 100))) {
		$input_errors[] = "The total maximum number of concurrent connections must be between 4 and 100.";
	}
	$mymaxproc = $_POST['maxproc'] ? $_POST['maxproc'] : 16;
	if ($_POST['maxprocperip'] && (!is_numeric($_POST['maxprocperip']) || ($_POST['maxprocperip'] > $mymaxproc))) {
		$input_errors[] = "The maximum number of concurrent connections per client IP address may not be larger than the global maximum.";
	}

	if (!$input_errors) {

		$config['installedpackages']['snortglobal']['snortdownload'] = $_POST['snortdownload'];
		$config['installedpackages']['snortglobal']['oinkmastercode'] = $_POST['oinkmastercode'];
		$config['installedpackages']['snortglobal']['emergingthreats'] = $_POST['emergingthreats'] ? on : off;
		$config['installedpackages']['snortglobal']['rm_blocked'] = $_POST['rm_blocked'];
		$config['installedpackages']['snortglobal']['autorulesupdate7'] = $_POST['autorulesupdate7'];
		$config['installedpackages']['snortglobal']['whitelistvpns'] = $_POST['whitelistvpns'] ? on : off;
		$config['installedpackages']['snortglobal']['clickablalerteurls'] = $_POST['clickablalerteurls'] ? on : off;
		$config['installedpackages']['snortglobal']['associatealertip'] = $_POST['associatealertip'] ? on : off;

		write_config();

		$retval = 0;

		config_lock();
		$retval = captiveportal_configure();
		config_unlock();

		$savemsg = get_std_save_message($retval);
	}
}
include("head.inc");
?>
<?php include("fbegin.inc"); ?>
<script language="JavaScript">
<!--

/* make shure all the settings exist or function hide will not work */
function enable_change(enable_change) {
	var endis, radius_endis;
	endis = !(document.iform.enable.checked || enable_change);
//	radius_endis = !((!endis && document.iform.auth_method[2].checked) || enable_change);

	document.iform.snortdownload[0].disabled = endis;
	document.iform.snortdownload[1].disabled = endis;
	document.iform.snortdownload[2].disabled = endis;
	document.iform.oinkmastercode.disabled = endis;
	document.iform.emergingthreats.disabled = endis;
	document.iform.rm_blocked.disabled = endis;
	document.iform.autorulesupdate7.disabled = endis;
	document.iform.whitelistvpns.disabled = endis;
	document.iform.clickablalerteurls.disabled = endis;
	document.iform.associatealertip.disabled = endis;
}
//-->
</script>
<p class="pgtitle"><?=$pgtitle?></p>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<form action="snort_interfaces_global.php" method="post" enctype="multipart/form-data" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
<?php
	$tab_array = array();
	$tab_array[] = array("Snort Inertfaces", false, "snort_interfaces.php");
	$tab_array[] = array("Global Settings", true, "snort_interfaces_global.php");
	$tab_array[] = array("Rule Updates", false, "services_captiveportal_ip.php");
	$tab_array[] = array("Alerts", false, "services_captiveportal_users.php");
    $tab_array[] = array("Blocked", false, "services_captiveportal_filemanager.php");
	$tab_array[] = array("Whitelists", false, "services_captiveportal_users.php");
	$tab_array[] = array("Help & Info", false, "services_captiveportal_filemanager.php");
	display_top_tabs($tab_array);
?>    </td></tr>
  <tr>
  <td class="tabcont">
  <table width="100%" border="0" cellpadding="6" cellspacing="0">
    <tr>
      <td width="22%" valign="top" class="vncell">Install Snort.org rules</td>
      <td width="78%" class="vtable">
        <table cellpadding="0" cellspacing="0">
        <tr>
          <td colspan="2"><input name="snortdownload" type="radio" id="snortdownload" value="off" onClick="enable_change(false)" <?php if($pconfig['snortdownload']!="premium" && $pconfig['snortdownload']!="basic") echo "checked"; ?>>
  Do <strong>NOT</strong> install</td>
          </tr>
        <tr>
          <td colspan="2"><input name="snortdownload" type="radio" id="snortdownload" value="premium" onClick="enable_change(false)" <?php if($pconfig['snortdownload']=="premium") echo "checked"; ?>>
  Premium rules <a href="http://forum.pfsense.org/index.php/topic,16847.0.html" target="_blank">HIGHLY RECOMMENDED</a></td>
          </tr>
        <tr>
          <td colspan="2"><input name="snortdownload" type="radio" id="snortdownload" value="basic" onClick="enable_change(false)" <?php if($pconfig['snortdownload']=="basic") echo "checked"; ?>>
  Basic Rules</td>
          </tr>
          <tr>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          </tr>
        </table>
        <table width="100%" border="0" cellpadding="6" cellspacing="0">
            <tr>
                <td colspan="2" valign="top" class="optsect_t2">Oinkmaster code</td>
            </tr>
            <tr>
                <td class="vncell" valign="top">Code</td>
                <td class="vtable"><input name="oinkmastercode" type="text" class="formfld" id="oinkmastercode" size="52" value="<?=htmlspecialchars($pconfig['oinkmastercode']);?>"><br>
                Obtain a snort.org Oinkmaster code and paste here.</td>
    </td>
  </table>
    </tr>
    <tr>
      <td width="22%" valign="top" class="vncell">Install <strong>Emergingthreats</strong> rules</td>
      <td width="78%" class="vtable">
        <input name="emergingthreats" type="checkbox" value="yes" <?php if ($config['installedpackages']['snortglobal']['emergingthreats']=="on") echo "checked"; ?> onClick="enable_change(false)"><br>
        Emerging Threats is an open source community that produces fastest moving and diverse Snort Rules.</td>
    </tr>
    <tr>
        <td width="22%" valign="top" class="vncell">Remove blocked hosts every</td>
        <td width="78%" class="vtable">
        <select name="rm_blocked" class="formfld" id="rm_blocked">
               <?php
                  $interfaces3 = array('never_b' => 'NEVER', '1h_b' => '1 HOUR', '3h_b' => '3 HOURS', '6h_b' => '6 HOURS', '12h_b' => '12 HOURS', '1d_b' => '1 DAY', '4d_b' => '4 DAYS', '7d_b' => '7 DAYS', '28d_b' => '28 DAYS');
                  foreach ($interfaces3 as $iface3 => $ifacename3): ?>
                  <option value="<?=$iface3;?>" <?php if ($iface3 == $pconfig['rm_blocked']) echo "selected"; ?>>
                  <?=htmlspecialchars($ifacename3);?>
                  </option>
               <?php endforeach; ?>
         </select><br>
         <span class="vexpl">Please select the amount of time you would like hosts to be blocked for.<br>
         Hint: in most cases, 1 hour is a good choice.</span></td>
    </tr>
    <tr>
        <td width="22%" valign="top" class="vncell">Update rules automatically</td>
        <td width="78%" class="vtable">
        <select name="autorulesupdate7" class="formfld" id="autorulesupdate7">
               <?php
                  $interfaces3 = array('never_up' => 'NEVER', '6h_up' => '6 HOURS', '12h_up' => '12 HOURS', '1d_up' => '1 DAY', '4d_up' => '4 DAYS', '7d_up' => '7 DAYS', '28d_up' => '28 DAYS');
                  foreach ($interfaces3 as $iface3 => $ifacename3): ?>
                  <option value="<?=$iface3;?>" <?php if ($iface3 == $pconfig['autorulesupdate7']) echo "selected"; ?>>
                  <?=htmlspecialchars($ifacename3);?>
                  </option>
               <?php endforeach; ?>
         </select><br>
         <span class="vexpl">Please select the update times for rules.<br>
         Hint: in most cases, every 12 hours is a good choice.</span></td>
    </tr>
    <tr>
      <td width="22%" valign="top" class="vncell">Whitelist VPNs automatically</td>
      <td width="78%" class="vtable">
        <input name="whitelistvpns" type="checkbox" value="yes" <?php if ($config['installedpackages']['snortglobal']['whitelistvpns'] == "on") echo "checked"; ?> onClick="enable_change(false)"><br>
        Checking this option will install whitelists for all VPNs.</td>
    </tr>
    <tr>
      <td width="22%" valign="top" class="vncell">Convert Snort alerts urls to clickable links</td>
      <td width="78%" class="vtable">
        <input name="clickablalerteurls" type="checkbox" value="yes" <?php if ($config['installedpackages']['snortglobal']['clickablalerteurls'] == "on") echo "checked"; ?> onClick="enable_change(false)"><br>
        Checking this option will automatically convert URLs in the Snort alerts tab to clickable links.</td>
    </tr>
    <tr>
      <td width="22%" valign="top" class="vncell">Associate events on Blocked tab</td>
      <td width="78%" class="vtable">
        <input name="associatealertip" type="checkbox" value="yes" <?php if ($config['installedpackages']['snortglobal']['associatealertip'] == "on") echo "checked"; ?> onClick="enable_change(false)"><br>
        Checking this option will automatically associate the blocked reason from the snort alerts file.</td>
    </tr>
	<tr>
	  <td width="22%" valign="top">&nbsp;</td>
	  <td width="78%">
		<input name="Submit" type="submit" class="formbtn" value="Save" onClick="enable_change(true)">
	  </td>
	</tr>
	<tr>
	  <td width="22%" valign="top">&nbsp;</td>
	  <td width="78%"><span class="vexpl"><span class="red"><strong>Note:<br>
		</strong></span>Changing any settings on this page will disconnect all clients! Don't forget to enable the DHCP server on your captive portal interface! Make sure that the default/maximum DHCP lease time is higher than the timeout entered on this page. Also, the DNS forwarder needs to be enabled for DNS lookups by unauthenticated clients to work. </span></td>
	</tr>
  </table>
  </td>
  </tr>
  </table>
</form>
<script language="JavaScript">
<!--
enable_change(false);
//-->
</script>
<?php include("fend.inc"); ?>
</body>
</html>
