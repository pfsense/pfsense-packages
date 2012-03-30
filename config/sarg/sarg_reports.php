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
	if ($_REQUEST['cmd'] =='sarg'){
		
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
		if	($config['installedpackages']['sargrealtime']['config'][0]['realtime_users'] != $_REQUEST['qtype']){
			$config['installedpackages']['sargrealtime']['config'][0]['realtime_users']= $_REQUEST['qtype'];
			$update_config++;
			}
			
		if($update_config > 0){
			write_config;
			#write changes to sarg_file
			$sarg_config=file_get_contents('/usr/local/etc/sarg/sarg.conf');
			$pattern[0]='/realtime_types\s+[A-Z,,]+/';
			$pattern[1]='/realtime_unauthenticated_records\s+\w+/';
			$replace[0]="realtime_types ".$_REQUEST['qshape'];
			$replace[1]="realtime_unauthenticated_records ".$_REQUEST['qtype'];
			file_put_contents('/usr/local/etc/sarg/sarg.conf', preg_replace($pattern,$replace,$sarg_config),LOCK_EX);
			}
		exec("/usr/local/bin/sarg -r", $sarg);
		$patern[0]="/<?(html|head|style)>/";
		$replace[0]="";
		$patern[1]="/header_\w/";
		$replace[1]="listtopic";
		$patern[2]="/class=.data./";
		$replace[2]='class="listlr"';
		$patern[3]="/cellpadding=.\d./";
		$replace[3]='cellpadding="0"';
		$patern[4]="/cellspacing=.\d./";
		$replace[4]='cellspacing="0"';
		$patern[5]="/sarg/";
		$replace[5]='cellspacing="0"';
		
		foreach ($sarg as $line){
			if (preg_match("/<.head>/",$line))
				$print ="ok";
			if ($print =="ok" && !preg_match("/(sarg realtime|Auto Refresh)/i",$line))
				print preg_replace($patern,$replace,$line);
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
	
	$pgtitle = "Status: Sarg Reports";
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
	$tab_array[] = array(gettext("View Report"), true, "/sarg_reports.php");
	$tab_array[] = array(gettext("Realtime"), false, "/sarg_realtime.php");
	$tab_array[] = array(gettext("XMLRPC Sync"), false, "/pkg_edit.php?xml=sarg_sync.xml&id=0");
	$tab_array[] = array(gettext("Help"), false, "/pkg_edit.php?xml=sarg_about.php");
	display_top_tabs($tab_array);
	?>
			</td></tr>
	 		<tr>
	    		<td>
					<div id="mainarea">
						<table class="tabcont" width="100%" border="0" cellpadding="8" cellspacing="0">
						<tr><td></td></tr>
						<tr>
						<td colspan="2" valign="top" class="listtopic"><?=gettext("Sarg Reports"); ?></td></tr>
				</table>
				</div>
				<br>
				<iframe src="/sarg-reports/" frameborder=0 width="100%" height="600"></iframe>
				<div id="file_div"></div>
				
				</td>
			</tr>
			</table>
	</div>
	</form>
	<?php 
	include("fend.inc");
	}
	?>
	</body>
	</html>
