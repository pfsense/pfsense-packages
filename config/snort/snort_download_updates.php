<?php
/* $Id$ */
/*
 halt.php
 part of pfSense
 Copyright (C) 2004 Scott Ullrich
 All rights reserved.

 part of m0n0wall as reboot.php (http://m0n0.ch/wall)
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

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");
require_once("/usr/local/pkg/snort/snort.inc");

global $g;

/* load only javascript that is needed */
$snort_load_jquery = 'yes';
$snort_load_jquery_colorbox = 'yes';


/* quick md5s chk */
if(file_exists('/usr/local/etc/snort/snortrules-snapshot-2860.tar.gz.md5'))
{
	$snort_org_sig_chk_local = exec('/bin/cat /usr/local/etc/snort/snortrules-snapshot-2860.tar.gz.md5');
}else{
	$snort_org_sig_chk_local = 'N/A';
}

if(file_exists('/usr/local/etc/snort/version.txt'))
{
	$emergingt_net_sig_chk_local = exec('/bin/cat /usr/local/etc/snort/version.txt');
}else{
	$emergingt_net_sig_chk_local = 'N/A';
}

if(file_exists('/usr/local/etc/snort/pfsense_rules.tar.gz.md5'))
{
	$pfsense_org_sig_chk_local = exec('/bin/cat /usr/local/etc/snort/pfsense_rules.tar.gz.md5');
}else{
	$pfsense_org_sig_chk_local = 'N/A';
}

/* define checks */
$oinkid = $config['installedpackages']['snortglobal']['oinkmastercode'];
$snortdownload = $config['installedpackages']['snortglobal']['snortdownload'];
$emergingthreats = $config['installedpackages']['snortglobal']['emergingthreats'];

if ($snortdownload != 'on' && $emergingthreats != 'on')
{
	$snort_emrging_info = 'stop';
}

if ($oinkid == '' && $snortdownload != 'off')
{
	$snort_oinkid_info = 'stop';
}

if ($snort_emrging_info == 'stop' || $snort_oinkid_info == 'stop') {
	$error_stop = 'true';
}


/* check if main rule directory is empty */
$if_mrule_dir = "/usr/local/etc/snort/rules";
$mfolder_chk = (count(glob("$if_mrule_dir/*")) === 0) ? 'empty' : 'full';

/* check for logfile */
if(file_exists('/usr/local/etc/snort/snort_update.log'))
{
	$update_logfile_chk = 'yes';
}else{
	$update_logfile_chk = 'no';
}

header("snort_help_info.php");
header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
header( "Cache-Control: no-cache, must-revalidate" );
header( "Pragma: no-cache" );


$pgtitle = "Services: Snort: Updates";
include_once("/usr/local/pkg/snort/snort_head.inc");

?>

<body
	link="#000000" vlink="#000000" alink="#000000">

<script>
			jQuery(document).ready(function(){
			
				//Examples of how to assign the ColorBox event to elements
				jQuery(".example7").colorbox({width:"900px", height:"400px", iframe:true, overlayClose:false});
				jQuery(".example8").colorbox({width:"820px", height:"700px", iframe:true, overlayClose:false});
				jQuery(".example9").colorbox({width:"90%", height:"65%", iframe:true, overlayClose:false});
				
			});
		</script>

<?php
echo "{$snort_general_css}\n";
echo "$snort_interfaces_css\n";
?>

<?php include("fbegin.inc"); ?>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img
	src="./images/transparent.gif" border="0"></img></a></div>

<?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>

<noscript>
<div class="alert" ALIGN=CENTER><img
	src="../themes/<?php echo $g['theme']; ?>/images/icons/icon_alert.gif" /><strong>Please
enable JavaScript to view this content
</CENTER></div>
</noscript>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="snorttabs" style="margin: 1px 10px; width: 760px;"><!-- Tabbed bar code-->
		<ul class="snorttabs">
			<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
			<li><a href="/snort/snort_interfaces_global.php"><span>Global
			Settings</span></a></li>
			<li class="snorttabs_active"><a
				href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
			<li><a href="/snort/snort_alerts.php"><span>Alerts</span></a></li>
			<li><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
			<li><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
			<li><a href="/snort/snort_interfaces_suppress.php"><span>Suppress</span></a></li>
			<li><a class="example8" href="/snort/help_and_info.php?id_d=up"><span>Help</span></a></li>
		</ul>
		</div>

		</td>
	</tr>

	<tr>
		<td>

		<div class="snorttabs" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="snorttabs">

			<li class="snorttabs_active"><a
				href="/snort/snort_download_rules.php"><span>Rule Update</span></a></li>
			<li><a href="#"><span>Upload Custom Rules</span></a></li>
			<li><a href="#"><span>Gui Update</span></a></li>

		</ul>
		</div>

		</td>
	</tr>

	<tr>
		<td>
		<div id="mainarea3">
		<table id="maintable4" class="tabcont" width="100%" border="0"
			cellpadding="0" cellspacing="0">
			<tr>
				<td><!-- grey line -->
				<table height="12px" width="725px" border="0" cellpadding="5px"
					cellspacing="0">
					<tr>
						<td style='background-color: #eeeeee'>
						<div height="12px" width="725px" style='background-color: #dddddd'>
						</div>
						</td>
					</tr>
				</table>

				<br>

				<table id="download_rules" height="32px" width="725px" border="0"
					cellpadding="5px" cellspacing="0">
					<tr>
						<td id="download_rules_td" style="background-color: #eeeeee">
						<div height="32" width="725px" style="background-color: #eeeeee">

						<font color="#777777" size="1.5px"><b>INSTALLED SIGNATURE RULESET</b></font><br>
						<br>
						<p style="text-align: left; margin-left: 225px;"><font
							color="#FF850A" size="1px"><b>SNORT.ORG >>></b></font><font
							size="1px" color="#000000">&nbsp;&nbsp;<? echo $snort_org_sig_chk_local; ?></font><br>
						<font color="#FF850A" size="1px"><b>EMERGINGTHREATS.NET >>></b></font><font
							size="1px" color="#000000">&nbsp;&nbsp;<? echo $emergingt_net_sig_chk_local; ?></font><br>
						<font color="#FF850A" size="1px"><b>PFSENSE.ORG >>></b></font><font
							size="1px" color="#000000">&nbsp;&nbsp;<? echo $pfsense_org_sig_chk_local; ?></font><br>
						</p>

						</div>
						</td>
					</tr>
				</table>

				<br>

				<!-- grey line -->
				<table height="12px" width="725px" border="0" cellpadding="5px"
					cellspacing="0">
					<tr>
						<td style='background-color: #eeeeee'>
						<div height="12px" width="725px" style='background-color: #eeeeee'>
						</div>
						</td>
					</tr>
				</table>

				<br>

				<table id="download_rules" height="32px" width="725px" border="0"
					cellpadding="5px" cellspacing="0">
					<tr>
						<td id="download_rules_td" style='background-color: #eeeeee'>
						<div height="32" width="725px" style='background-color: #eeeeee'>

						<font color='#777777' size='1.5px'><b>UPDATE YOUR RULES</b></font><br>
						<br>

						<?php

						if ($error_stop == 'true') {

							echo '
		
			<button class="sexybutton disabled" disabled="disabled" href="/snort/snort_download_rules.php?$id_d=up"><span class="download">Update Rules&nbsp;&nbsp;&nbsp;&nbsp;</span></button><br>
			<p style="text-align:left; margin-left:150px;">
			<font color="#fc3608" size="2px"><b>WARNING:</b></font><font size="1px" color="#000000">&nbsp;&nbsp;No rule types have been selected for download. "Global Settings Tab"</font><br>
		';

							if ($mfolder_chk == 'empty') {

								echo '
			<font color="#fc3608" size="2px"><b>WARNING:</b></font><font size="1px" color="#000000">&nbsp;&nbsp;The main rules directory is empty. /usr/local/etc/snort/rules</font>
			' . "\n";
							}

							echo '</p>' . "\n";

						}else{

							echo '
		
		<button class="sexybutton sexysimple example7" href="/snort/snort_download_rules.php?$id_d=up"><span class="download">Update Rules&nbsp;&nbsp;&nbsp;&nbsp;</span></button>
		
		' . "\n";

							if ($mfolder_chk == 'empty') {

								echo '
			<p style="text-align:left; margin-left:150px;">
			<font color="#fc3608" size="2px"><b>WARNING:</b></font><font size="1px" color="#000000">&nbsp;&nbsp;The main rules directory is empty. /usr/local/etc/snort/rules</font>
			</p>
			';
							}

						}

						?> <br>

						</div>
						</td>
					</tr>
				</table>

				<br>

				<table id="download_rules" height="32px" width="725px" border="0"
					cellpadding="5px" cellspacing="0">
					<tr>
						<td id="download_rules_td" style='background-color: #eeeeee'>
						<div height="32" width="725px" style='background-color: #eeeeee'>

						<font color='#777777' size='1.5px'><b>VIEW UPDATE LOG</b></font><br>
						<br>

						<?php

						if ($update_logfile_chk == 'yes') {
							echo '
				<button class="sexybutton sexysimple example9" href="/snort/snort_rules_edit.php?openruleset=/usr/local/etc/snort/snort_update.log"><span class="pwhitetxt">Update Log&nbsp;&nbsp;&nbsp;&nbsp;</span></button>' . "\n";
						}else{
							echo '
				<button class="sexybutton disabled" disabled="disabled" href="/snort/snort_rules_edit.php?openruleset=/usr/local/etc/snort/snort_update.log"><span class="pwhitetxt">Update Log&nbsp;&nbsp;&nbsp;&nbsp;</span></button>' . "\n";
						}
							
						?> <br>
						<br>

						</div>
						</td>
					</tr>
				</table>

				<br>

				<table height="12px" width="725px" border="0" cellpadding="5px"
					cellspacing="0">
					<tr>
						<td style='background-color: #eeeeee'>
						<div height="12px" width="725px" style='background-color: #eeeeee'>
						</div>
						</td>
					</tr>
				</table>

				<br>

				<table id="download_rules" height="32px" width="725px" border="0"
					cellpadding="5px" cellspacing="0">
					<tr>
						<td id="download_rules_td" style='background-color: #eeeeee'>
						<div height="32" width="725px" style='background-color: #eeeeee'>

						<img style='vertical-align: middle'
							src="/snort/images/icon_excli.png" width="40" height="32"> <font
							color='#FF850A' size='1px'><b>NOTE:</b></font><font size='1px'
							color='#000000'>&nbsp;&nbsp;Snort.org and Emergingthreats.net
						will go down from time to time. Please be patient.</font></div>
						</td>
					</tr>
				</table>

				<br>

				<table height="12px" width="725px" border="0" cellpadding="5px"
					cellspacing="0">
					<tr>
						<td style='background-color: #eeeeee'>
						<div height="12px" width="725px" style='background-color: #eeeeee'>
						</div>
						</td>
					</tr>
				</table>

				</td>
			</tr>
		</table>
		</div>





		<br>
		</td>
	</tr>
</table>
<!-- end of final table --></div>

						<?php //include("fend.inc"); ?>

</div>
<!-- Right DIV -->
</div>
<!-- Content DIV -->
<div id="footer"><a target="_blank"
	href="<?=$g['product_website_footer']?>" class="redlnk"><?=$g['product_name']?></a>
is &copy; <?=$g['product_copyright_years']?> by <a
	href="<?=$g['product_copyright_url']?>" class="tblnk"><?=$g['product_copyright']?></a>.
All Rights Reserved. [<a href="/license.php" class="tblnk">view license</a>]
<br />
[<a target="_blank" href="https://portal.pfsense.org/?guilead=true"
	class="tblnk">Commercial Support Available</a>]</div>
<!-- Footer DIV -->
</div>
<!-- Wrapper Div -->
<script type="text/javascript"
	src="/themes/<?php echo $g['theme']; ?>/bottom-loader.js"></script>

<?php echo "$snort_custom_rnd_box\n"; ?>

</body>
</html>
