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

// unset Session tmp on page load
unset($_SESSION['snort']['tmp']);

// list rules in the default dir
$a_list = snortSql_fetchAllSettings('snortDBrules', 'Snortrules', 'uuid', $rdbuuid);

$snortRuleDir = '/usr/local/etc/snort/snortDBrules/DB/' . $rdbuuid;

	// list rules in the default dir
	$filterDirList = array();
	$filterDirList = snortScanDirFilter($snortRuleDir . '/rules', '\.rules');

	// START read rule file
	if ($_GET['openruleset']) {
		$rulefile = $_GET['openruleset'];
	}else{
		$rulefile = $filterDirList[0];
	}

	// path of rule file
	$workingFile = $snortRuleDir . '/rules/' . $rulefile;
	
function load_rule_file($incoming_file, $splitcontents)
{	
		$pattern = '/(^alert |^# alert )/';
	 	foreach ( $splitcontents  as $val )
	 	{
	 		// remove whitespaces
			$rmWhitespaces = preg_replace('/\s\s+/', ' ', $val);	
			
			// filter none alerts
	 		if (preg_match($pattern, $rmWhitespaces))
	 		{
	 			$splitcontents2[] = $val;			
	 		}
	 		
	 	}
		unset($splitcontents);

	return $splitcontents2;

}
	
	// Load the rule file
	// split the contents of the string file into an array using the delimiter
	// used by rule gui edit and table build code	
	if (filesize($workingFile) > 0) {
	$splitcontents = split_rule_file($workingFile);		
	
		$splitcontents2 = load_rule_file($workingFile, $splitcontents);	
		
		$countSig = count($splitcontents2);
		
		if ($countSig > 0) {
			$newFilterRuleSigArray = newFilterRuleSig($splitcontents2);
		}
	}
		
	/*
	 * SET GLOBAL ARRAY $_SESSION['snort']
	 * Use SESSION instead POST for security because were writing to files.  
	 */
	
	$_SESSION['snort']['tmp']['snort_rules']['dbName'] = 'snortDBrules';
	$_SESSION['snort']['tmp']['snort_rules']['dbTable'] = 'SnortruleSigs';
	$_SESSION['snort']['tmp']['snort_rules']['rdbuuid'] = $rdbuuid;
	$_SESSION['snort']['tmp']['snort_rules']['rulefile'] = $rulefile;
	

// find ./ -name test.txt | xargs grep "^disablesid 127 "

	$pgtitle = "Snort: Category: rule: $rulefile";
	include("/usr/local/pkg/snort/snort_head.inc");

?>

		
	
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

<!-- hidden div -->
<div id="loadingRuleEditGUI">
	
	<div class="loadingRuleEditGUIDiv">
				<form id="iform2" action="">
				<input type="hidden" name="snortSidRuleEdit" value="1" />
				<input type="hidden" name="snortSidRuleDBuuid" value="<?=$rdbuuid;?>" /> <!-- what to do, save -->
				<input type="hidden" name="snortSidRuleFile" value="<?=$rulefile; ?>" /> <!-- what to do, save -->								
				<input type="hidden" name="snortSidNum" value="" /> <!-- what to do, save -->
				<table width="100%" cellpadding="9" cellspacing="9" bgcolor="#eeeeee">
					<tr>
						<td>							
							<input name="save" type="submit" class="formbtn" id="save" value="Save" /> 
							<input type="button" class="formbtn closeRuleEditGUI" value="Close" >
						</td>
					</tr>				
					<tr>
						<td>						
							<textarea id="sidstring" name="sidstring" wrap="off" style="width: 98%; margin: 7px;" rows="1" cols="" ></textarea> <!-- SID to EDIT -->
						</td>
					</tr>
					<tr>
						<td>
							<textarea wrap="off" style="width: 98%; margin: 7px;" rows="<?php if(count($splitcontents) > 24){echo 24;}else{echo count($splitcontents);} ?>" cols="" disabled >
							
							<?php
							
							echo "\n";
							
							foreach ($splitcontents as $sidLineGui)
							
							echo $sidLineGui . "\n";
							
							
							
							?>
							</textarea> <!-- Display rule file -->
						</td>
					</tr>
					</table>
					<table width="100%" cellpadding="9" cellspacing="9" bgcolor="#eeeeee">
					<tr>
						<td>							
							<input name="save" type="submit" class="formbtn" id="save" value="Save" /> 
							<input type="button" class="formbtn closeRuleEditGUI" value="Close" >
						</td>
					</tr>
				</table>
				</form>
	</div>


</div>

<?php include("fbegin.inc"); ?>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

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
					<li><a href="/snort/snort_rulesets.php?uuid=' . $uuid . '"><span>Categories</span></a></li>
					<li class="newtabmenu_active"><a href="/snort/snort_rules.php?uuid=' . $uuid . '"><span>Rules</span></a></li>
					<li><a href="/snort/snort_rulesets_ips.php?uuid=' . $uuid . '"><span>Ruleset Ips</span></a></li>
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
			<li class="hide_newtabmenu"><a href="/snort/snort_rulesets.php?rdbuuid=' . $rdbuuid . '"><span>Categories</span></a></li>
			<li class="hide_newtabmenu newtabmenu_active"><a href="/snort/snort_rules.php?rdbuuid=' . $rdbuuid . '"><span>Rules</span></a></li>
			<li><a href="/snort/snort_rulesets_ips.php?rdbuuid=' . $rdbuuid . '"><span>Ruleset Ips</span></a></li>
			</ul>
			</div>
			</td>
		</tr>	
			';
	}
	?>
	<tr>
	<td id="tdbggrey">	
	<div style="width:780px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 10px;">
	<!-- START MAIN AREA -->
	
	
			<!-- start Interface Satus -->
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
				<tr id="maintable77" >
					<td colspan="2" valign="top" class="listtopic2">
					Category:
			<select name="selectbox" class="formfld" >
				<?php
				if(isset($_GET['uuid'])) {
					$urlUuid = "&uuid=$uuid";
				}
				
				if(isset($_GET['rdbuuid'])) {
					$urlUuid = "&rdbuuid=$rdbuuid";
				}
				
				$i=0;
				foreach ($filterDirList as $value)
				{
					$selectedruleset = '';
					if ($value === $rulefile) {
						$selectedruleset = 'selected';
					}
					
					echo "\n" . '<option value="?&openruleset=' . $ruledir . $value . $urlUuid . '" ' . $selectedruleset . ' >' . $value . '</option>' . "\r";

				$i++;

				}
				?>
			</select>				
					There are <?=$countSig; ?> rules in this category. 
					</td>
					<td width="6%" colspan="2" valign="middle" class="listtopic3" >
					<a href="snort_interfaces_edit.php?uuid=<?=$new_ruleUUID;?>">
					<img style="padding-left:3px;" src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0" title="add rule">
					</a>
					</td>
				</tr>
			</table>
<br>

			<!-- Save all inputs -->
			<table width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td>
					<input id="select_all" type="button" class="formbtn" value="Select All"  >
					<input id="deselect_all" type="button" class="formbtn" value="Deselect All" >
				</td>
			</tr>
			</table>

<br>

			<!-- start User Interface -->
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
				<tr id="maintable77" >
					<td colspan="2" valign="top" class="listtopic">Snort Signatures:</td>
				</tr>
			</table>
			
			<form id="iform" action="">
			<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
			
				<td class="list" colspan="8"></td>
				<td class="list" valign="middle" >
										
						<tr id="frheader" >
							<td width="1%" class="listhdrr2">On</td>
							<td width="1%" class="listhdrr2">Sid</td>
							<td width="1%" class="listhdrr2">Proto</td>
							<td width="1%" class="listhdrr2">Src</td>
							<td width="1%" class="listhdrr2">Port</td>			
							<td width="1%" class="listhdrr2">Dst</td>
							<td width="1%" class="listhdrr2">Port</td>
							<td width="20%" class="listhdrr2">Message</td>
							<td width="1%" class="listhdrr2">&nbsp;</td>												
						</tr>
						<form id="iform" action="" >		
						<input type="hidden" name="snortSaveRuleSets" value="1" /> <!-- what to do, save -->
						<input type="hidden" name="ifaceTab" value="snort_rules" /> <!-- what interface tab -->
						
				<!-- START javascript sid loop here -->
						<tbody class="rulesetloopblock">
						
						
						
						</tbody>
				<!-- STOP javascript sid loop here -->
						
				</td>
				<td class="list" colspan="8"></td>				
							
			</table>
			<br>
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
			<tr>
				<td>
					<input name="Submit" type="submit" class="formbtn" value="Save">
					<input id="cancel" type="button" class="formbtn" value="Cancel">
				</td>
			</tr>
			</table>
			<br>
			</form>			
					
			<!-- stop snortsam -->

	<!-- STOP MAIN AREA -->			
	</div>			
	</td>
	</tr>
</table>
</form>
</div>

<!-- start info box -->

<br>

<div style="width:790px; background-color: #dddddd;" id="mainarea4">
<div style="width:780px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 10px;">
<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>&nbsp;&nbsp;&nbsp;</td>
	</tr>
	<tr >
	<td width="100%">
		<span class="red"><strong>Note:</strong></span> <br>
		This is the <strong>Snort Rule Signature Viewer</strong>.
		Please make sure not to add a <strong>whitespace</strong> before <strong>alert</strong> or <strong>#alert</strong>. 
		<br>
		<br>
		<span class="red"><strong>Warning:</strong></span> 
		<br>
		<strong>New settings will not take effect until interface restart.</strong>
		<br><br>		
	</td>
	</tr>
</table>
</div>
</div>


<script type="text/javascript">


//prepare the form when the DOM is ready 
jQuery(document).ready(function() {

	// NOTE: needs to be watched
	// change url on selected dropdown rule	
	jQuery('select[name=selectbox]').change(function() {
		window.location.replace(jQuery(this).val());
	});		
	
<?php

			/* 
			 * NOTE:
			 * I could have used a php loop to build the table but I wanted to see if off loading to client is faster.
			 * Seems to be faster on embeded systems with low specs. On higher end systems there is no difference that I can see.
			 * WARNING:
			 * If Json string is to long browsers start asking to terminate javascript.
			 * FIX: 
			 * Use julienlecomte()net/blog/2007/10/28/, the more reading I do about this subject it seems that off loading to a client is not recomended.
			 */
			if (!empty($newFilterRuleSigArray))
			{
				$countSigList = count($newFilterRuleSigArray);
				
				echo "\n";
				
				echo 'var snortObjlist = [';	
						$i = 0;
						foreach ($newFilterRuleSigArray as $val3)
						{
					
							$i++;	
							
							if ( $i !== $countSigList )
							{//		 
								echo '{"sid":"' . $val3['sid'] . '","enable":"' . $val3['enable'] . '","proto":"' . $val3['proto'] . '","src":"' . $val3['src'] . '","srcport":"' . $val3['srcport'] . '","dst":"' . $val3['dst'] . '", "dstport":"' . $val3['dstport'] . '","msg":"' . escapeJsonString($val3['msg']) . '"},'; 
							}else{
								echo '{"sid":"' . $val3['sid'] . '","enable":"' . $val3['enable'] . '","proto":"' . $val3['proto'] . '","src":"' . $val3['src'] . '","srcport":"' . $val3['srcport'] . '","dst":"' . $val3['dst'] . '", "dstport":"' . $val3['dstport'] . '","msg":"' . escapeJsonString($val3['msg']) . '"}'; 
							}
						}
						
				echo '];' . "\n";
			}	
			
?>

	// disable Row Append if row count is less than 0
	var countRowAppend = <?=$countSig; ?>;

	// if rowcount is not empty do this
	if (countRowAppend > 0){

		// if rowcount is more than 300
		if (countRowAppend > 200){		
			// call to please wait	
			showLoading('#loadingWaiting');
		}
	
	
		// Break up append row adds by chunks of 300
		// NOTE: ie9 is still giving me issues on deleted.rules 6000 sigs. I should break up the json code above into smaller parts.
		incrementallyProcess(function (i){
		  // loop code goes in here
		  //console.log('loop: ', i);
	
			if (isEven(i) === true){
				var rowIsEvenOdd = 'odd_ruleset2';
			}else{ 
				var rowIsEvenOdd = 'even_ruleset2';
			}
			
			if (snortObjlist[i].enable === 'on'){
				var rulesetChecked = 'checked'; 
			}else{
				var rulesetChecked = '';
			}
		
			jQuery('.rulesetloopblock').append(
	
					"\n" + '<tr valign="top" id="fr0">' + "\n" +
					'<td class="' + rowIsEvenOdd + '">' + "\n" +
						'<input class="domecheck" type="checkbox" name="filenamcheckbox2[]" value="' + snortObjlist[i].sid + '" ' + rulesetChecked + ' >' + "\n" +
					'</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].sid + '</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].proto + '</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].src + '</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].srcport + '</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].dst + '</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].dstport + '</td>' + "\n" +
						'<td class="listbg" id="frd0" ><font color="white">' + snortObjlist[i].msg + '</font></td>' + "\n" +
						'<td class="' + rowIsEvenOdd+ '">' + "\n" +
							'<img id="' + snortObjlist[i].sid + '" class="icon_click showeditrulegui" src="/themes/<?=$g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0" title="edit rule">' + "\n" +
						'</td>' + "\n" +						
					'</tr>' + "\n"
					
			);
		  
		}, 
		snortObjlist,  // Object to work with the case Json object
		500, // chunk size
		200, // how many secs to wait
		function (){
		// things that happen after the processing is done go here
		// console.log('done!');
		
		// if rowcount is more than 300
		if (countRowAppend > 200){		
			// call to please wait	
			hideLoading('#loadingWaiting');
		}	
		
		});
	} // end of if stopRowAppend

	// On click show rule edit GUI
	jQuery('.showeditrulegui').live('click', function(){
	
		// Get sid
		jQuery.getJSON('/snort/snort_json_get.php',
			{
			"snortGetSidString": "1",
			"snortIface": "<?=$uuid . '_' . $a_list['interface']; ?>",
			"snortRuleFile": "<?=$rulefile; ?>",
			"sid": jQuery(this).attr('id')
			},
			function(data){
				jQuery("textarea#sidstring").val(data.sidstring); // add string to textarea
				jQuery("input[name=snortSidNum]").val(data.sid); // add sid to input
				showLoading('#loadingRuleEditGUI');						
			});	
		});
	
	jQuery('.closeRuleEditGUI').live('click', function(){	
		hideLoading('#loadingRuleEditGUI');
	});	
	

}); // end of document ready

</script>


<!-- stop info box -->

<!-- footer do not touch below -->
<?php 
include("fend.inc"); 
echo $snort_custom_rnd_box;
?>


</body>
</html>
