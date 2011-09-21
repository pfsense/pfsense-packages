<?php
/* $Id$ */
/*

 part of pfSense
 All rights reserved.

 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 All rights reserved.

 Pfsense Old snort GUI 
 Copyright (C) 2006 Scott Ullrich.
 
 Pfsense snort GUI 
 Copyright (C) 2008-2012 Robert Zelaya.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. Neither the name of the pfSense nor the names of its contributors 
 may be used to endorse or promote products derived from this software without 
 specific prior written permission.

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
require_once("/usr/local/pkg/snort/snort_new.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");

//Set no caching
header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
header("Cache-Control: no-store, no-cache, must-revalidate");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");

// set page vars

$uuid = $_GET['uuid'];
if (isset($_POST['uuid']))
$uuid = $_POST['uuid'];

if ($uuid == '') {
	echo 'error: no uuid';
	exit(0);
}


$a_list = snortSql_fetchAllSettings('snortDB', 'SnortIfaces', 'uuid', $uuid);

	if (!is_array($a_list))
	{
		$a_list = array();
	}



	$pgtitle = "Snort: Interface: Barnyard2 Edit";
	include("/usr/local/pkg/snort/snort_head.inc");

?>


<!-- START page custom script -->
<script language="JavaScript">

// start a jQuery sand box
jQuery(document).ready(function() { 

	// START disable option for snort_interfaces_edit.php
	endis = !(jQuery('input[name=barnyard_enable]:checked').val());
		
	disableInputs=new Array(
			"barnyard_mysql",
			"barnconfigpassthru",
			"dce_rpc",
			"dns_preprocessor",
			"ftp_preprocessor",
			"http_inspect",
			"other_preprocs",
			"perform_stat",
			"sf_portscan",
			"smtp_preprocessor"
			);


	jQuery('[name=interface]').attr('disabled', 'true');
	
	
	if (endis)
	{
		for (var i = 0; i < disableInputs.length; i++)
		{
		jQuery('[name=' + disableInputs[i] + ']').attr('disabled', 'true');
		}
	}

	jQuery("input[name=barnyard_enable]").live('click', function() {

		endis = !(jQuery('input[name=barnyard_enable]:checked').val());

		if (endis)
		{
			for (var i = 0; i < disableInputs.length; i++)
			{
			jQuery('[name=' + disableInputs[i] + ']').attr('disabled', 'true');
			}
		}else{
			for (var i = 0; i < disableInputs.length; i++)
			{
			jQuery('[name=' + disableInputs[i] + ']').removeAttr('disabled');
			}
		}

		
	}); 
	// STOP disable option for snort_interfaces_edit.php
	
	
}); // end of on ready

</script>




<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<!-- loading msg -->
<div id="loadingWaiting">
	<div class="snortModal" style="top: 200px; left: 700px;">
		<div class="snortModalTop">
			<!-- <div class="snortModalTopClose"><a href="javascript:hideLoading('#loadingWaiting');"><img src="/snort/images/close_9x9.gif" border="0" height="9" width="9"></a></div> -->
		</div>
		<div class="snortModalTitle">
	  		<p><img src="./images/loading.gif" /><br><br>Please Wait...</p>
	  	</div>
		<div>
		<p class="loadingWaitingMessage"></p>
	  	</div>
	</div>  
</div>

<?php include("fbegin.inc"); ?>
<!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2">
<a href="../index.php" id="status-link2">
<img src="./images/transparent.gif" border="0"></img>
</a>
</div>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
				<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
				<li><a href="/snort/snort_interfaces_edit.php?uuid=<?=$uuid;?>"><span>If Settings</span></a></li>
				<li><a href="/snort/snort_rulesets.php?uuid=<?=$uuid;?>"><span>Categories</span></a></li>
				<li><a href="/snort/snort_rules.php?uuid=<?=$uuid;?>"><span>Rules</span></a></li>
				<li><a href="/snort/snort_rulesets_ips.php?uuid=<?=$uuid;?>"><span>Ruleset Ips</span></a></li>
				<li><a href="/snort/snort_define_servers.php?uuid=<?=$uuid;?>"><span>Servers</span></a></li>
				<li><a href="/snort/snort_preprocessors.php?uuid=<?=$uuid;?>"><span>Preprocessors</span></a></li>
				<li class="newtabmenu_active"><a href="/snort/snort_barnyard.php?uuid=<?=$uuid;?>"><span>Barnyard2</span></a></li>			
		</ul>
		</div>

		</td>
	</tr>
	<tr>
		<td id="tdbggrey">		
		<table width="100%" border="0" cellpadding="10px" cellspacing="0">
		<tr>
		<td class="tabnavtbl">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<!-- START MAIN AREA -->
		
		<form id="iform" >
		<input type="hidden" name="snortSaveSettings" value="1" /> <!-- what to do, save -->
		<input type="hidden" name="dbName" value="snortDB" /> <!-- what db-->
		<input type="hidden" name="dbTable" value="SnortIfaces" /> <!-- what db table-->
		<input type="hidden" name="ifaceTab" value="snort_barnyard" /> <!-- what interface tab -->
		<input name="uuid" type="hidden" value="<?=$uuid; ?>">


			<tr>
				<td colspan="2" valign="top" class="listtopic">General Barnyard2 Settings</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncellreq2">Enable</td>
				<td width="78%" class="vtable">
					<input name="barnyard_enable" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['barnyard_enable'] == 'on' || $a_list['barnyard_enable'] == '' ? 'checked' : '';?> >
					<span class="vexpl"><strong>Enable Barnyard2 on this Interface</strong><br>
					This will enable barnyard2 for this interface. You will also have to set the database credentials.</span>
				</td>			
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Interface</td>
				<td width="78%" class="vtable">
					<select name="interface" class="formfld" >
						<option value="wan" selected><?=strtoupper($a_list['interface']); ?></option>
					</select>
					<br>
					<span class="vexpl">Choose which interface this rule applies to.<br>
					Hint: in most cases, you'll want to use WAN here.</span></span>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Mysql Settings</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Log to a Mysql Database</td>
				<td width="78%" class="vtable">
					<input name="barnyard_mysql" type="text" class="formfld" id="barnyard_mysql" size="100" value="<?=$a_list['barnyard_mysql']; ?>"> 
					<br>
					<span class="vexpl">Example: output database: alert, mysql, dbname=snort user=snort host=localhost password=xyz<br>
					Example: output database: log, mysql, dbname=snort user=snort host=localhost password=xyz</span>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Advanced Settings</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Advanced configuration pass through</td>
				<td width="78%" class="vtable">
					<textarea name="barnconfigpassthru" cols="75" rows="12" id="barnconfigpassthru" class="formpre2"><?=$a_list['barnconfigpassthru']; ?></textarea>
					<br>
					<span class="vexpl">Arguments here will be automatically inserted into the running barnyard2 configuration.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%">
					<input name="Submit" type="submit" class="formbtn" value="Save">
					<input type="button" class="formbtn" value="Cancel" >
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%">
					<span class="vexpl"><span class="red"><strong>Note:</strong></span>
					Please save your settings befor you click start.</span>
				</td>
			</tr>
			
		
		</form>
		<!-- STOP MAIN AREA -->
		</table>
		</td>
		</tr>			
		</table>
	</td>
	</tr>
</table>
</div>


<!-- footer do not touch below -->
<?php 
include("fend.inc"); 
echo $snort_custom_rnd_box;
?>


</body>
</html>
