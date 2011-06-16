<?php
/* $Id$ */
/*
 snort_interfaces.php
 part of m0n0wall (http://m0n0.ch/wall)

 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 Copyright (C) 2008-2009 Robert Zelaya.
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
require_once("/usr/local/pkg/snort/snort_new.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");

if (isset($_GET['uuid']) && isset($_GET['rdbuuid'])) {
	echo 'Error: more than one uuid';
	exit(0);
}

// set page vars
if (isset($_GET['uuid'])) {
	$uuid = $_GET['uuid'];
}

if (isset($_GET['rdbuuid'])) {
	$rdbuuid = $_GET['rdbuuid'];
}else{
	$ruledbname_pre1 = snortSql_fetchAllSettings('snortDB', 'SnortIfaces', 'uuid', $uuid);
	$rdbuuid = $ruledbname_pre1['ruledbname'];
}

//$a_list = snortSql_fetchAllSettings('snortDBrules', 'SnortIfaces', 'uuid', $uuid);

	// list rules in the default dir
	$filterDirList = array();
	$filterDirList = snortScanDirFilter('/usr/local/etc/snort/snortDBrules/DB/' . $rdbuuid . '/rules', '\.rules');
	
	// list rules in db that are on in a array
	$listOnRules = array();
	$listOnRules = snortSql_fetchAllSettings('snortDBrules', 'SnortRuleSets', 'rdbuuid', $rdbuuid);
	
	if (!empty($listOnRules)) {
		foreach ( $listOnRules as $val2 )
		{
			if ($val2['enable'] == 'on') {
				$rulesetOn[] = $val2['rulesetname'];
			}			
		}
		unset($listOnRules);
	}
	
	$pgtitle = "Snort: Interface Rule Categories";
	include("/usr/local/pkg/snort/snort_head.inc");

?>




<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<script type="text/javascript">

//prepare the form when the DOM is ready 
jQuery(document).ready(function() { 	

	<?php 
			/* 
			 * NOTE: I could have used a php loop to build the table but off loading to client is faster 
			 * use jQuery jason parse, make sure its in one line
			 */
			if (!empty($filterDirList))
			{
				$countDirList = count($filterDirList);
				
				echo "\n";
				
				echo 'var snortObjlist = jQuery.parseJSON(\' { "ruleSets": [ ';	
						$i = 0;
						foreach ($filterDirList as $val3)
						{
					
							$i++;
							
							// if list ruleset is in the db ON mark it checked
							$rulesetOnChecked = 'off';
							if(!empty($rulesetOn))
							{
								if (in_array($val3, $rulesetOn))
								{
									$rulesetOnChecked = 'on';
								}
							}		
							
							if ( $i !== $countDirList )
							{
								echo '{"rule": ' . '"' . $val3 . '", ' . '"enable": ' . '"' . $rulesetOnChecked . '"' . '}, ';
							}else{
								echo '{"rule": "' . $val3 . '", ' . '"enable": ' . '"' . $rulesetOnChecked . '"' . '} ';
							}
						}
						
				echo ' ]}\');' . "\n";
			}	

	
			
	?>
	
	// loop through object, dont use .each in jQuery as its slow
	if(snortObjlist.ruleSets.length > 0)
	{
		for (var i = 0; i < snortObjlist.ruleSets.length; i++)
		{
	
			if (isEven(i) === true)
			{
				var rowIsEvenOdd = 'even_ruleset';
			}else{
				var rowIsEvenOdd = 'odd_ruleset';
			}
	
			if (snortObjlist.ruleSets[i].enable === 'on')
			{
				var rulesetChecked = 'checked'; 
			}else{
				var rulesetChecked = '';
			}
			
			jQuery('.rulesetloopblock').append(
					"\n" + '<tr>' + "\n" +
					'<td class="' + rowIsEvenOdd + '" align="center" valign="top" width="9%">' + "\n" +
					'	<input class="domecheck" name="filenamcheckbox[]" value="' + snortObjlist.ruleSets[i].rule + '" type="checkbox" ' + rulesetChecked + ' >' + "\n" +
					'</td>' + "\n" +
					'<td class="' + rowIsEvenOdd + '">' + "\n" +
					'	<a href="/snort/snort_rules.php?openruleset=' + snortObjlist.ruleSets[i].rule + '<?php if(isset($uuid)){echo "&uuid=$uuid";}else{echo "&rdbuuid=$rdbuuid";}?>' + '">' + snortObjlist.ruleSets[i].rule + '</a>' + "\n" + 
					'</td>' + "\n" +
					'</tr>' + "\n\n"			
			);	
		};
	}

		
}); // end of document ready	

</script>

<div id="loadingWaiting">
  <p class="loadingWaitingMessage"><img src="./images/loading.gif" /> <br>Please Wait...</p>
</div>

<?php include("fbegin.inc"); ?>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0" alt="transgif" ></img></a></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<?php
	if (!empty($uuid)) { 
		echo '
		<tr>
			<td>
			<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
			<ul class="newtabmenu">
					<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
					<li><a href="/snort/snort_interfaces_edit.php?uuid=' . $uuid . '"><span>If Settings</span></a></li>
					<li class="newtabmenu_active"><a href="/snort/snort_rulesets.php?uuid=' . $uuid . '"><span>Categories</span></a></li>
					<li><a href="/snort/snort_rules.php?uuid=' . $uuid . '"><span>Rules</span></a></li>
					<li><a href="/snort/snort_define_servers.php?uuid=' . $uuid . '"><span>Servers</span></a></li>
					<li><a href="/snort/snort_preprocessors.php?uuid=' . $uuid . '"><span>Preprocessors</span></a></li>
					<li><a href="/snort/snort_barnyard.php?uuid=' . $uuid . '"><span>Barnyard2</span></a></li>			
			</ul>
			</div>
			</td>
		</tr>
		';
	}else{
		echo ' 
		<tr>
			<td>
			<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
			<ul class="newtabmenu">
				<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
				<li><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
				<li><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
				<li class="newtabmenu_active"><a href="/snort/snort_interfaces_rules.php"><span>RulesDB</span></a></li>
				<li><a href="/snort/snort_alerts.php"><span>Alerts</span></a></li>
				<li><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
				<li><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
				<li><a href="/snort/snort_interfaces_suppress.php"><span>Suppress</span></a></li>
				<li><a href="/snort/snort_help_info.php"><span>Help</span></a></li>
			</ul>
			</div>
			</td>
		</tr>
		<tr>
			<td>
			<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
			<ul class="newtabmenu">
			<li class="hide_newtabmenu"><a href="/snort/snort_interfaces_rules_edit.php?rdbuuid=' . $rdbuuid . '"><span>Rules DB Edit</span></a></li>
			<li class="hide_newtabmenu newtabmenu_active"><a href="/snort/snort_rulesets.php?rdbuuid=' . $rdbuuid . '"><span>Categories</span></a></li>
			<li class="hide_newtabmenu"><a href="/snort/snort_rules.php?rdbuuid=' . $rdbuuid . '"><span>Rules</span></a></li>
			</ul>
			</div>
			</td>
		</tr>	
			';
	}
	?>
	<tr>
		<td id="tdbggrey">		
		<table width="100%" border="0" cellpadding="10px" cellspacing="0">
		<tr>
		<td class="tabnavtbl">
		<table width="100%" border="0" cellpadding="6" cellspacing="0" >
		<!-- START MAIN AREA -->


		
		<table width="100%" border="0" cellpadding="0" cellspacing="0" >
		<tr>
			<td>
			</td>
			<td>
				<input id="select_all" type="button" class="formbtn" value="Select All"  >
				<input id="deselect_all" type="button" class="formbtn" value="Deselect All" >
			</td>
		</tr>
		</table>
	
	<div id="checkboxdo" style="width:750px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 0px;">	
	<form id="iform" action="" >		
		<input type="hidden" name="snortSaveRuleSets" value="1" /> <!-- what to do, save -->
		<input type="hidden" name="dbName" value="snortDBrules" /> <!-- what db-->
		<input type="hidden" name="dbTable" value="SnortruleSets" /> <!-- what db table-->
		<input type="hidden" name="ifaceTab" value="snort_rulesets" /> <!-- what interface tab -->
		<input type="hidden" name="rdbuuid" value="<?=$rdbuuid;?>" /> <!-- what interface to save for -->
			
		<table width="100%" border="0" cellpadding="0" cellspacing="0">
		
		<tr >
			<td width="5%" class="listtopic">Enabled</td>
			<td class="listtopic">Ruleset: Rules that end with "so.rules" are shared object rules.</td>
		</tr>
			<table class="rulesetbkg" width="100%">	
				
			<tbody class="rulesetloopblock" >
			<!-- javscript loop table build here -->
			</tbody>
			
			</table>			
			<table class="vncell1" width="100%" border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td class="listtopic" >Check the rulesets that you would like Snort to load at startup.</td>
				</tr>
			</table>
		<tr>
			<td>
				<input name="Submit" type="submit" class="formbtn" value="Save">
				<input id="cancel" type="button" class="formbtn" value="Cancel">
			</td>
		</tr>
			<tr>
				<td width="78%">
					<span class="vexpl"><span class="red"><strong>Note:</strong></span>
					Please save your settings before you click start.</span>
				</td>
			</tr>
			
		</table>
		</form>			
	</div>	
		
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

