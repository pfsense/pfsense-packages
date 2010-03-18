<?php
/* $Id$ */
/*
	snort_alerts.php
	part of pfSense

	Copyright (C) 2005 Bill Marquette <bill.marquette@gmail.com>.
	Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
	Copyright (C) 2006 Scott Ullrich
	All rights reserved.
	
	Modified for the Pfsense snort package v. 1.8+
	Copyright (C) 2009 Robert Zelaya Sr. Developer

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

require("globals.inc");
require("guiconfig.inc");
require("/usr/local/pkg/snort/snort.inc");

$snortalertlogt = $config['installedpackages']['snortglobal']['snortalertlogtype'];
$snort_logfile = '/var/log/snort/alert';

$pconfig['arefresh'] = $config['installedpackages']['snortglobal']['alertsblocks']['arefresh'];
$pconfig['alertnumber'] = $config['installedpackages']['snortglobal']['alertsblocks']['alertnumber'];

if ($pconfig['alertnumber'] == '' || $pconfig['alertnumber'] == '0')
{
	$anentries = '250';
}else{
	$anentries = $pconfig['alertnumber'];
}

if ($_POST['save'])
{

	//unset($input_errors);
	//$pconfig = $_POST;

	/* input validation */
	if ($_POST['save'])
	{

	//	if (($_POST['radiusacctport'] && !is_port($_POST['radiusacctport']))) {
	//		$input_errors[] = "A valid port number must be specified. [".$_POST['radiusacctport']."]";
	//	}
	
	}
		
		/* no errors */
		if (!$input_errors)
		{
		
		$config['installedpackages']['snortglobal']['alertsblocks']['arefresh'] = $_POST['arefresh'] ? on : off;
		$config['installedpackages']['snortglobal']['alertsblocks']['alertnumber'] = $_POST['alertnumber'];
		
		conf_mount_rw();
		write_config();
		//conf_mount_ro();
		sleep(2);
			
		header("Location: /snort/snort_alerts.php");
				
		}
		
}


if ($_POST['delete'])
{

	exec("killall syslogd");
	conf_mount_rw();
	if(file_exists("/var/log/snort/alert"))
	{	
		exec('/bin/rm /var/log/snort/*');
		exec('/usr/bin/touch /var/log/snort/alert');
	}
	conf_mount_ro();
	system_syslogd_start();
	//exec("/usr/bin/killall -HUP snort");
		
}

if ($_POST['download'])
{

	ob_start(); //importanr or other post will fail
	$save_date = exec('/bin/date "+%Y-%m-%d-%H-%M-%S"');
	$file_name = "snort_logs_{$save_date}.tar.gz";
	exec("/usr/bin/tar cfz /tmp/snort_logs_{$save_date}.tar.gz /var/log/snort");

	if(file_exists("/tmp/snort_logs_{$save_date}.tar.gz"))
	{
		$file = "/tmp/snort_logs_{$save_date}.tar.gz";
		header("Expires: Mon, 26 Jul 1997 05:00:00 GMT\n");
		header("Pragma: private"); // needed for IE
		header("Cache-Control: private, must-revalidate"); // needed for IE
		header('Content-type: application/force-download');
		header('Content-Transfer-Encoding: Binary');
		header("Content-length: ".filesize($file));
		header("Content-disposition: attachment; filename = {$file_name}");
		readfile("$file");
		exec("/bin/rm /tmp/snort_logs_{$save_date}.tar.gz");
		od_end_clean(); //importanr or other post will fail
	}else{
    echo 'Error no saved file.';
	}

}


/* WARNING: took me forever to figure reg expression, dont lose */
// $fileline = '12/09-18:12:02.086733  [**] [122:6:0] (portscan) TCP Filtered Decoy Portscan [**] [Priority: 3] {PROTO:255} 125.135.214.166 -> 70.61.243.50';

function get_snort_alert_date($fileline)
{
	/* date full date \d+\/\d+-\d+:\d+:\d+\.\d+\s */
	if (preg_match("/\d+\/\d+-\d+:\d+:\d\d/", $fileline, $matches1))
	{
        $alert_date =  "$matches1[0]";
	}

return $alert_date;

}

function get_snort_alert_disc($fileline)
{
	/* disc */
	if (preg_match("/\[\*\*\] (\[.*\]) (.*) (\[\*\*\])/", $fileline, $matches))
	{
        $alert_disc =  "$matches[2]";
	}

return $alert_disc;

}

function get_snort_alert_class($fileline)
{
	/* class */
	if (preg_match('/\[Classification:\s.+[^\d]\]/', $fileline, $matches2))
	{
        $alert_class = "$matches2[0]";
	}

return $alert_class;

}

function get_snort_alert_priority($fileline)
{
	/* Priority */
	if (preg_match('/Priority:\s\d/', $fileline, $matches3))
	{
        $alert_priority = "$matches3[0]";
	}

return $alert_priority;

}

function get_snort_alert_proto($fileline)
{
	/* Priority */
	if (preg_match('/\{.+\}/', $fileline, $matches3))
	{
        $alert_proto = "$matches3[0]";
	}

return $alert_proto;

}

function get_snort_alert_proto_full($fileline)
{
		/* Protocal full */
		if (preg_match('/.+\sTTL/', $fileline, $matches2))
		{
			$alert_proto_full = "$matches2[0]";
		}

return $alert_proto_full;

}

function get_snort_alert_ip_src($fileline)
{
	/* SRC IP */
	$re1='.*?';   # Non-greedy match on filler
	$re2='((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))(?![\\d])'; # IPv4 IP Address 1

	if ($c=preg_match_all ("/".$re1.$re2."/is", $fileline, $matches4))
	{
      $alert_ip_src = $matches4[1][0];
	}

return $alert_ip_src;

}

function get_snort_alert_src_p($fileline)
{
	/* source port */
	if (preg_match('/:\d+\s-/', $fileline, $matches5))
	{
        $alert_src_p = "$matches5[0]";
	}

return $alert_src_p;

}

function get_snort_alert_flow($fileline)
{
	/* source port */
	if (preg_match('/(->|<-)/', $fileline, $matches5))
	{
        $alert_flow = "$matches5[0]";
	}

return $alert_flow;

}

function get_snort_alert_ip_dst($fileline)
{
	/* DST IP */
	$re1dp='.*?';   # Non-greedy match on filler
	$re2dp='(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(?![\\d])';   # Uninteresting: ipaddress
	$re3dp='.*?';   # Non-greedy match on filler
	$re4dp='((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))(?![\\d])'; # IPv4 IP Address 1

	if ($c=preg_match_all ("/".$re1dp.$re2dp.$re3dp.$re4dp."/is", $fileline, $matches6))
	{
      $alert_ip_dst = $matches6[1][0];
	}
  
return $alert_ip_dst;

}
  
function get_snort_alert_dst_p($fileline)
{ 
	/* dst port */
	if (preg_match('/:\d+$/', $fileline, $matches7))
	{
        $alert_dst_p = "$matches7[0]";
	}

return $alert_dst_p;

}

function get_snort_alert_dst_p_full($fileline)
{ 
	/* dst port full */
	if (preg_match('/:\d+\n[A-Z]+\sTTL/', $fileline, $matches7))
	{
        $alert_dst_p = "$matches7[0]";
	}

return $alert_dst_p;

}

function get_snort_alert_sid($fileline)
{ 
	/* SID */
	if (preg_match('/\[\d+:\d+:\d+\]/', $fileline, $matches8))
	{
        $alert_sid = "$matches8[0]";
	}

return $alert_sid;

}

//

$pgtitle = "Services: Snort: Snort Alerts";
include("head.inc");

?>

<link rel="stylesheet" href="/snort/css/style.css" type="text/css" media="all">
<script type="text/javascript" src="/snort/javascript/mootools.js"></script>
<script type="text/javascript" src="/snort/javascript/sortableTable.js"></script>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php 

include("fbegin.inc");

echo "<p class=\"pgtitle\">";
if($pfsense_stable == 'yes'){echo $pgtitle;}
echo "</p>\n";

/* refresh every 60 secs */
if ($pconfig['arefresh'] == 'on' || $pconfig['arefresh'] == '')
{
	echo "<meta http-equiv=\"refresh\" content=\"60;url=/snort/snort_alerts.php\" />\n";
}
?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array("Snort Interfaces", false, "/snort/snort_interfaces.php");
	$tab_array[] = array("Global Settings", false, "/snort/snort_interfaces_global.php");
	$tab_array[] = array("Rule Updates", false, "/snort/snort_download_rules.php");
	$tab_array[] = array("Alerts", true, "/snort/snort_alerts.php");
    $tab_array[] = array("Blocked", false, "/snort/snort_blocked.php");
	$tab_array[] = array("Whitelists", false, "/pkg.php?xml=/snort/snort_whitelist.xml");
	$tab_array[] = array("Help & Info", false, "/snort/snort_help_info.php");
	display_top_tabs($tab_array);
?>
</td>
</tr>
	<tr>
	<td>
	<div id="mainarea">
		<table class="tabcont" width="100%" border="1" cellspacing="0" cellpadding="0">
		  <tr>
			<td width="22%" colspan="0" class="listtopic">
			Last <?=$anentries;?> Alert Entries.
			</td>
			<td width="78%" class="listtopic">
			Latest Alert Entries Are Listed First.
			</td>
		  </tr>
    <tr>
        <td width="22%" class="vncell">Save or Remove Logs</td>
		<td width="78%" class="vtable">
		<form action="/snort/snort_alerts.php" method="post">
        <input name="download" type="submit" class="formbtn" value="Download">
        All log files will be saved.
		<input name="delete" type="submit" class="formbtn" value="Clear">
		<span class="red"><strong>Warning:</strong></span> all log files will be deleted.
		</form>
		</td>
    </tr>
    <tr>
        <td width="22%" class="vncell">Auto Refresh and Log View</td>
		<td width="78%" class="vtable">
		<form action="/snort/snort_alerts.php" method="post">
		<input name="save" type="submit" class="formbtn" value="Save">
		Refresh
        <input name="arefresh" type="checkbox" value="on" <?php if ($config['installedpackages']['snortglobal']['alertsblocks']['arefresh']=="on") echo "checked"; ?>>
        <strong>Default</strong> is <strong>ON</strong>.
		<input name="alertnumber" type="text" class="formfld" id="alertnumber" size="5" value="<?=htmlspecialchars($anentries);?>">
		Enter the number of log entries to view. <strong>Default</strong> is <strong>250</strong>.
		</form>
		</td>
    </tr>
		</table>
	</div>
	</td>
  </tr>
</table>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
	  <td width="100%">
	  <br>
			<div class="tableFilter">
		  		<form id="tableFilter" onsubmit="myTable.filter(this.id); return false;">Filter: 
					<select id="column">
						<option value="1">PRIORITY</option>
						<option value="2">PROTO</option>
						<option value="3">DESCRIPTION</option>
						<option value="4">CLASS</option>
						<option value="5">SRC</option>
						<option value="6">SRC PORT</option>
						<option value="7">FLOW</option>
						<option value="8">DST</option>
						<option value="9">DST PORT</option>
						<option value="10">SID</option>
						<option value="11">Date</option>
					</select>
					<input type="text" id="keyword" />
					<input type="submit" value="Submit" />
					<input type="reset" value="Clear" />
				</form>
		  </div>
<table class="allRow" id="myTable" width="100%" border="2" cellpadding="1" cellspacing="1">
		  	<thead>
				<th axis="number">#</th>
				<th axis="string">PRI</th>
				<th axis="string">PROTO</th>
				<th axis="string">DESCRIPTION</th>
				<th axis="string">CLASS</th>
				<th axis="string">SRC</th>
				<th axis="string">SPORT</th>
				<th axis="string">FLOW</th>
				<th axis="string">DST</th>
				<th axis="string">DPORT</th>
				<th axis="string">SID</th>
				<th axis="date">Date</th>
			</thead>
			<tbody>
<?php

	/* make sure alert file exists */	
	if(!file_exists('/var/log/snort/alert'))
	{
		conf_mount_rw();
		exec('/usr/bin/touch /var/log/snort/alert');
		conf_mount_ro();
	}

	$logent = $anentries;
	
	/* detect the alert file type */
	if ($snortalertlogt == 'full')
	{
		$alerts_array = array_reverse(array_filter(explode("\n\n", file_get_contents('/var/log/snort/alert'))));
	}else{
		$alerts_array = array_reverse(array_filter(split("\n", file_get_contents('/var/log/snort/alert'))));
	}


	
if (is_array($alerts_array))
{

	$counter = 0;
	foreach($alerts_array as $fileline)
	{
	
	if($logent <= $counter)
    continue;

		$counter++;
	
		/* Date */
		$alert_date_str = get_snort_alert_date($fileline);
		
			if($alert_date_str != '')
			{
				$alert_date = $alert_date_str;
			}else{			
				$alert_date = 'empty';
			}

		/* Discription */
		$alert_disc_str = get_snort_alert_disc($fileline);
		
			if($alert_disc_str != '')
			{
				$alert_disc = $alert_disc_str;
			}else{			
				$alert_disc = 'empty';
			}		
		
		/* Classification */
		$alert_class_str = get_snort_alert_class($fileline);
		
			if($alert_class_str != '')
			{

				$alert_class_match = array('[Classification:',']'); 
				$alert_class = str_replace($alert_class_match, '', "$alert_class_str");
			}else{			
				$alert_class = 'Prep';
			}
			
		/* Priority */
		$alert_priority_str = get_snort_alert_priority($fileline);
		
			if($alert_priority_str != '')
			{
				$alert_priority_match = array('Priority: ',']'); 
				$alert_priority = str_replace($alert_priority_match, '', "$alert_priority_str");
			}else{		
				$alert_priority = 'empty';
			}

		/* Protocol */
		/* Detect alert file type */
	if ($snortalertlogt == 'full')
	{
		$alert_proto_str = get_snort_alert_proto_full($fileline);
	}else{
		$alert_proto_str = get_snort_alert_proto($fileline);
	}

			if($alert_proto_str != '')
			{
				$alert_proto_match = array(" TTL",'{','}'); 
				$alert_proto = str_replace($alert_proto_match, '', "$alert_proto_str");
			}else{
				$alert_proto = 'empty';
			}
			
		/* IP SRC */
		$alert_ip_src_str = get_snort_alert_ip_src($fileline);
		
		if($alert_ip_src_str != '')
		{
			$alert_ip_src = $alert_ip_src_str;
		}else{
			$alert_ip_src = 'empty';
		}	
			
		/* IP SRC Port */
		$alert_src_p_str = get_snort_alert_src_p($fileline);
			
			if($alert_src_p_str != '')
			{
				$alert_src_p_match = array(' -',':'); 
				$alert_src_p = str_replace($alert_src_p_match, '', "$alert_src_p_str");
			}else{			
				$alert_src_p = 'empty';
			}			
		
		/* Flow */
		$alert_flow_str = get_snort_alert_flow($fileline);
		
			if($alert_flow_str != '')
			{
				$alert_flow = $alert_flow_str;
			}else{			
				$alert_flow = 'empty';
			}		
		
		/* IP Destination */
		$alert_ip_dst_str = get_snort_alert_ip_dst($fileline);
		
		if($alert_ip_dst_str != '')
		{
			$alert_ip_dst = $alert_ip_dst_str;
		}else{
			$alert_ip_dst = 'empty';
		}
				
		/* IP DST Port */
	if ($snortalertlogt == 'full')
	{
		$alert_dst_p_str = get_snort_alert_dst_p_full($fileline);
	}else{
		$alert_dst_p_str = get_snort_alert_dst_p($fileline);
	}
	
			if($alert_dst_p_str != '')
			{
				$alert_dst_p_match = array(':',"\n"," TTL");
				$alert_dst_p_str2 = str_replace($alert_dst_p_match, '', "$alert_dst_p_str");
				$alert_dst_p_match2 = array('/[A-Z]/');
				$alert_dst_p = preg_replace($alert_dst_p_match2, '', "$alert_dst_p_str2");
			}else{
				$alert_dst_p = 'empty';
			}

		/* SID */
		$alert_sid_str = get_snort_alert_sid($fileline);
		
			if($alert_sid_str != '')
			{
				$alert_sid_match = array('[',']'); 
				$alert_sid = str_replace($alert_sid_match, '', "$alert_sid_str");
			}else{
				$alert_sid_str = 'empty';	
			}
		
		/* NOTE: using one echo improves performance by 2x */
		if ($alert_disc != 'empty')
		{
			echo "<tr id=\"{$counter}\">
				<td class=\"centerAlign\">{$counter}</td>
				<td class=\"centerAlign\">{$alert_priority}</td>
				<td class=\"centerAlign\">{$alert_proto}</td>
				<td>{$alert_disc}</td>
				<td class=\"centerAlign\">{$alert_class}</td>
				<td>{$alert_ip_src}</td>
				<td class=\"centerAlign\">{$alert_src_p}</td>
				<td class=\"centerAlign\">{$alert_flow}</td>
				<td>{$alert_ip_dst}</td>
				<td class=\"centerAlign\">{$alert_dst_p}</td>
				<td class=\"centerAlign\">{$alert_sid}</td>
				<td>{$alert_date}</td>
				</tr>\n";
		}
		
//		<script type="text/javascript">
//			var myTable = {};
//			window.addEvent('domready', function(){
//				myTable = new sortableTable('myTable', {overCls: 'over', onClick: function(){alert(this.id)}});
//			});
//		</script>
		
	}
}

?>
			</tbody>	
		</table>
	</td>
</table>

<?php include("fend.inc"); ?>

		<script type="text/javascript">
			var myTable = {};
			window.addEvent('domready', function(){
				myTable = new sortableTable('myTable', {overCls: 'over'});
			});
		</script>

</body>
</html>
