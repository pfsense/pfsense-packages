<?php
/*
	postfix_view_config.php
	part of pfSense (http://www.pfsense.com/)
	Copyright (C) 2011 Marcello Coutinho <marcellocoutinho@gmail.com>
	based on varnish_view_config.
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

require("guiconfig.inc");
function get_cmd(){
	global $config,$g;
	#print $_REQUEST['type'];
	if ($_REQUEST['cmd'] =='sarg'){
		$update_config=0;
		#Check report xml info
		if (!is_array($config['installedpackages']['sargrealtime'])){
			$config['installedpackages']['sargrealtime']['config'][0]['realtime_types']= "";
			$config['installedpackages']['sargrealtime']['config'][0]['realtime_users']= "";
		}
		#Check report http actions to show
		if	($config['installedpackages']['sargrealtime']['config'][0]['realtime_types'] != $_REQUEST['qshape']){
			$config['installedpackages']['sargrealtime']['config'][0]['realtime_types']= $_REQUEST['qshape'];
			$update_config++;
			}

		#Check report users show
		if	($config['installedpackages']['sargrealtime']['config'][0]['realtime_users'] != $_REQUEST['type']){
			$config['installedpackages']['sargrealtime']['config'][0]['realtime_users']= $_REQUEST['type'];
			$update_config++;
			}
			
		if($update_config > 0){
			write_config();
			#write changes to sarg_file
			$sarg_config=file_get_contents('/usr/local/etc/sarg/sarg.conf');
			$pattern[0]='/realtime_types\s+[A-Z,,]+/';
			$replace[0]="realtime_types ".$_REQUEST['qshape'];
			$pattern[1]='/realtime_unauthenticated_records\s+\w+/';
			$replace[1]="realtime_unauthenticated_records ".$_REQUEST['type'];
			file_put_contents('/usr/local/etc/sarg/sarg.conf', preg_replace($pattern,$replace,$sarg_config),LOCK_EX);
			}
		exec("/usr/local/bin/sarg -r", $sarg);
		$pattern[0]="/<?(html|head|style)>/";
		$replace[0]="";
		$pattern[1]="/header_\w/";
		$replace[1]="listtopic";
		$pattern[2]="/class=.data./";
		$replace[2]='class="listlr"';
		$pattern[3]="/cellpadding=.\d./";
		$replace[3]='cellpadding="0"';
		$pattern[4]="/cellspacing=.\d./";
		$replace[4]='cellspacing="0"';
		$pattern[5]="/sarg/";
		$replace[5]='cellspacing="0"';
		
		foreach ($sarg as $line){
			if (preg_match("/<.head>/",$line))
				$print ="ok";
			if ($print =="ok" && !preg_match("/(sarg realtime|Auto Refresh)/i",$line))
				print preg_replace($pattern,$replace,$line);
		}
	}
}

if ($_REQUEST['cmd']!=""){
	get_cmd();
	}
else{
	$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
	if(strstr($pfSversion, "1.2"))
		$one_two = true;
	
	$pgtitle = "Status: Postfix Mail Queue";
	include("head.inc");
	
	?>
	<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
	<?php include("fbegin.inc"); ?>
	
	<?php if($one_two): ?>
	<p class="pgtitle"><?=$pgtitle?></font></p>
	<?php endif; ?>
	
	<?php if ($savemsg) print_info_box($savemsg); ?>
	
	<form action="postfix_view_config.php" method="post">
		
	<div id="mainlevel">
		<table width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr><td>
	<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=sarg.xml&id=0");
	$tab_array[] = array(gettext("Schedule"), false, "/pkg.php?xml=sarg_schedule.xml");
	$tab_array[] = array(gettext("View Report"), false, "/sarg-reports/");
	$tab_array[] = array(gettext("Realtime"), true, "/sarg_realtime.php");
	$tab_array[] = array(gettext("XMLRPC Sync"), false, "/pkg_edit.php?xml=sarg_sync.xml&id=0");
	$tab_array[] = array(gettext("Help"), false, "/sarg_about.php");
	display_top_tabs($tab_array);
	?>
			</td></tr>
	 		<tr>
	    		<td>
					<div id="mainarea">
						<table class="tabcont" width="100%" border="0" cellpadding="8" cellspacing="0">
						<tr><td></td></tr>
						<tr>
						<td colspan="2" valign="top" class="listtopic"><?=gettext("Postfix Queue"); ?></td></tr>
					<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Log command: ");?></td>
                        <td width="78%" class="vtable">
                        <select name="drop3" id="cmd">
                        	<option value="sarg" selected="selected">Sarg Realtime</option>
						</select><br><?=gettext("Select queue command to run.");?></td>
					</tr>
					<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("update frequency: ");?></td>
                        <td width="78%" class="vtable">
                        <select name="drop3" id="updatef">
                        	<option value="1">01 second</option>
                            <option value="3" selected="selected">03 seconds</option>
                        	<option value="5">05 seconds</option>
                        	<option value="15">15 Seconds</option>
							<option value="30">30 Seconds</option>
							<option value="60">One minute</option>
							<option value="1">Never</option>
						</select><br><?=gettext("Select how often queue cmd will run.");?></td>
					</tr>
					<tr>					
                        <td width="22%" valign="top" class="vncell"><?=gettext("Report Types: ");?></td>
                        <td width="78%" class="vtable">
                        <select name="drop3" id="qshape" multiple="multiple" size="5">
                        	<option value="GET" selected="selected">GET</option>
							<option value="PUT" selected="selected">PUT</option>
							<option value="CONNECT" selected="selected">CONNECT</option>
							<option value="ICP_QUERY">ICP_QUERY</option>
							<option value="POST">POST</option>
						</select><br><?=gettext("Which records must be in realtime report.");?></td>
					</tr>
					<tr>					
                        <td width="22%" valign="top" class="vncell"><?=gettext("unauthenticated_records: ");?></td>
                        <td width="78%" class="vtable">
                        <select name="drop3" id="qtype">
							<option value="show" selected>show</option>
							<option value="hide">hide</option>
						</select><br><?=gettext("What to do with unauthenticated records in realtime report.");?></td>
					</tr>

					<tr>
							<td width="22%" valign="top"></td>
                        <td width="78%"><input name="Submit" type="button" class="formbtn" id="run" value="<?=gettext("show log");?>" onclick="get_queue('mailq')"><div id="search_help"></div></td>
				</table>
				</div>
				</td>
			</tr>
			</table>
			<br>
				<div>
				<table class="tabcont" width="100%" border="0" cellpadding="8" cellspacing="0">
								<tr>
	     						<td class="tabcont" >
	     						<div id="file_div"></div>
									
								</td>
							</tr>
						</table>
					</div>
	</div>
	</form>
	<script type="text/javascript">
	function loopSelected(id)
	{
	  var selectedArray = new Array();
	  var selObj = document.getElementById(id);
	  var i;
	  var count = 0;
	  for (i=0; i<selObj.options.length; i++) {
	    if (selObj.options[i].selected) {
	      selectedArray[count] = selObj.options[i].value;
	      count++;
	    }
	  }
	  return(selectedArray);
	}
		
	function get_queue(loop) {
			//prevent multiple instances
			if ($('run').value=="show log" || loop== 'running'){
				$('run').value="running...";
				$('search_help').innerHTML ="<br><strong>You can change options while running.<br>To Stop seach, change update frequency to Never.</strong>";
				var q_args=loopSelected('qshape');
				var pars = 'cmd='+$('cmd').options[$('cmd').selectedIndex].value;
				var pars = pars + '&qshape='+q_args;
				var pars = pars + '&type='+$('qtype').options[$('qtype').selectedIndex].value;
				var url = "/sarg_queue.php";
				var myAjax = new Ajax.Request(
					url,
					{
						method: 'post',
						parameters: pars,
						onComplete: activitycallback_queue_file
					});
				}
			}
		function activitycallback_queue_file(transport) {
			$('file_div').innerHTML = transport.responseText;
			var update=$('updatef').options[$('updatef').selectedIndex].value * 1000;
			if (update > 999){
				setTimeout('get_queue("running")', update);
				}
			else{
				$('run').value="show log";
				$('search_help').innerHTML ="";
			}
		}
	</script>
	<?php 
	include("fend.inc");
	}
	?>
	</body>
	</html>
