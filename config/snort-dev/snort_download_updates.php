<?php
/* $Id$ */
/*
 snort_interfaces.php
 part of m0n0wall (http://m0n0.ch/wall)

 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 All rights reserved.
 
 Pfsense snort GUI 
 Copyright (C) 2008-2011 Robert Zelaya.

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


// set page vars

$generalSettings = snortSql_fetchAllSettings('snortDB', 'SnortSettings', 'id', '1');

header("Cache-Control: no-cache, must-revalidate");
header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");


	$pgtitle = 'Services: Snort: Updates';
	include("/usr/local/pkg/snort/snort_head.inc");

?>

		
	
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">


<div id="loadingWaiting">
  <p class="loadingWaitingMessage"><img src="./images/loading.gif" /> <br>Please Wait...</p>
</div>

<div class="pb_div" id="pb3"></div>	

<div id="loadingRuleUpadteGUI">

					<div class="loadingWaitingUpdateGUI" >
					<table>
					<tr>
						<td>Yellow Bar</td>
					</tr>
					<tr>
						<td><span class="progressBar" id="pb2"></span></td>
					</tr>
					<tr>
						<td>Yellow Bar</td>
					</tr>
					</table>
					</div>			
				
				
               <!-- progress bar -->
               <!--
               <table id="progholder" width='800px' style='border-collapse: collapse; border: 1px solid #000000;' cellpadding='2' cellspacing='2' bgcolor="#eeeeee">
                  <tr>
                    <td>
                      <img border='0' src='/themes/<?= $g['theme']; ?>/images/misc/progress_bar.gif' width='280' height='23' name='progressbar' id='progressbar' alt='' />
                    </td>
                  </tr>
                </table>
                <br />
                      
              <table width="800px" cellpadding="9" cellspacing="9" bgcolor="#eeeeee">
                <tr>
                  <td align="center" valign="top">
                      <textarea cols="90" rows="2" name="status" id="status" wrap="hard">
                      <?=gettext("Initializing...");?>
                      </textarea>
                      <textarea cols="90" rows="2" name="output" id="output" wrap="hard">
                      </textarea>
                  </td>
                </tr>
              </table>
              -->


</div>


<?php include("fbegin.inc"); ?>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="newtabmenu" style="margin: 1px 0px; width: 790px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
			<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
			<li><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
			<li class="newtabmenu_active"><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
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
			<li class="newtabmenu_active"><a href="/snort/snort_download_rules.php"><span>Rule Update</span></a></li>
			<li><a href="#"><span>Upload Custom Rules</span></a></li>
			<li><a href="#"><span>Gui Update</span></a></li>
		</ul>
		</div>

		</td>
	</tr>	
	<tr>
	<td id="tdbggrey">	
	<div style="width:780px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 10px;">
	<!-- START MAIN AREA -->
	
	
			<!-- start Interface Satus -->
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
				<tr id="maintable77" >
					<td colspan="2" valign="top" class="listtopic2">
					There are <?=$countSig; ?> rule databases that are ready to be updated. 
					</td>
					<td width="6%" colspan="2" valign="middle" class="listtopic3" >
					</td>
				</tr>
			</table>
<br>

			<!-- start User Interface -->
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
				<tr id="maintable77" >
					<td colspan="2" valign="top" class="listtopic">SIGNATURE RULESET DATABASES:</td>
				</tr>
			</table>
			
			
			<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
			
				<td class="list" ></td>
				<td class="list" valign="middle" >
										
						<tr id="frheader" >
							<td width="1%" class="listhdrr2">On</td>
							<td width="25%" class="listhdrr2">Signature DB Name</td>
							<td width="35%" class="listhdrr2">MD5 Version</td>
							<td width="38%" class="listhdrr2">New Rule DB Available</td>
							<td width="1%" class="listhdrr2">&nbsp;</td>											
						</tr>
						
				<!-- START javascript sid loop here -->
						<tbody class="rulesetloopblock">
						
<tr id="fr0" valign="top">
<td class="odd_ruleset2">
<input class="domecheck" name="filenamcheckbox2[]" value="1292" checked="checked" type="checkbox" disabled="disabled" >
</td>
<td class="odd_ruleset2" id="frd0">SNORT.ORG</td>
<td class="odd_ruleset2" id="frd0">tcp</td>
<td class="listbg" id="frd0"><font color="white">ATTACK-RESPONSES directory listing</font></td>
<td class="odd_ruleset2">
<img src="/themes/pfsense_ng/images/icons/icon_alias_url_reload.gif" title="edit rule" width="17" border="0" height="17">
</td>
</tr>

<tr id="fr0" valign="top">
<td class="odd_ruleset2">
<input class="domecheck" name="filenamcheckbox2[]" value="1292" checked="checked" type="checkbox" disabled="disabled" >
</td>
<td class="odd_ruleset2" id="frd0">EMERGINGTHREATS.NET</td>
<td class="odd_ruleset2" id="frd0">tcp</td>
<td class="listbg" id="frd0"><font color="white">ATTACK-RESPONSES directory listing</font></td>
<td class="odd_ruleset2">
<img src="/themes/pfsense_ng/images/icons/icon_alias_url_reload.gif" title="edit rule" width="17" border="0" height="17">
</td>
</tr>

<tr id="fr0" valign="top">
<td class="odd_ruleset2">
<input class="domecheck" name="filenamcheckbox2[]" value="1292" checked="checked" type="checkbox" disabled="disabled" >
</td>
<td class="odd_ruleset2" id="frd0">PFSENSE.ORG</td>
<td class="odd_ruleset2" id="frd0">tcp</td>
<td class="listbg" id="frd0"><font color="white">ATTACK-RESPONSES directory listing</font></td>
<td class="odd_ruleset2">
<img src="/themes/pfsense_ng/images/icons/icon_alias_url_reload.gif" title="edit rule" width="17" border="0" height="17">
</td>
</tr>						
						
						</tbody>
				<!-- STOP javascript sid loop here -->
						
				</td>
				<td class="list" colspan="8"></td>				
							
			</table>
			<br>
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
			<tr>
				<td>
					<input name="update" type="submit" class="formbtn" value="Update">
				</td>
			</tr>
			</table>
			<br>	
					
			<!-- stop snortsam -->

	<!-- STOP MAIN AREA -->			
	</div>			
	</td>
	</tr>
</table>
</div>

<!-- start info box -->

<br>

<div style="width:790px; background-color: #dddddd;" id="mainarea4">
<div style="width:780px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 10px;">
<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr >
		<td width="10%" valign="middle" >
			<img style="vertical-align: middle;" src="/snort/images/icon_excli.png" width="40" height="32">
		</td>
		<td width="90%" valign="middle" >
			<span class="red"><strong>Note:</strong></span>
			<strong>&nbsp;&nbsp;Snort.org and Emergingthreats.net will go down from time to time. Please be patient.</strong>		
		</td>
	</tr>
</table>
</div>
</div>


<script type="text/javascript">


//prepare the form when the DOM is ready 
jQuery(document).ready(function() {

	jQuery('input[name=update]').live('click', function(){	

		// jQuery("#pb2").progressBar(percent,{width: 404, height: 22, barImage: 'images/pb_orange.png'});
		// console.log(response[0].percent);
		// '/snort/snort_json_get.php?snortGetUpdate=1'
		
		showLoading('#loadingRuleUpadteGUI');

		function callComplete(response) {
			//alert("Response received is: "+response);
			
			while(1)
			{				
				console.log('HELLO: ' + response[0].percent);
				// reconnect to the server
				//connect();

			if(response[0].percent === '100')
			{
				console.log('HELLO: ' + response[0].percent);
				break;
			}
							
			};

			
		};

		function connect() {
			// when the call completes, callComplete() will be called along with
			// the response returned
			jQuery.get('/snort/snort_json_get.php?snortGetUpdate=1', {}, callComplete, 'json');
		};
		 
		connect(); // start loop
		

	}); // end of on click
		
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
