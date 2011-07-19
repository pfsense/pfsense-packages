<?php
/* $Id$ */
/*
	status_rrd_graph.php
	Part of pfSense
	Copyright (C) 2011 Jim Pingle <jimp@pfsense.org>
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
/*	
	pfSense_MODULE:	system
*/

##|+PRIV
##|*IDENT=page-status-rrdgraphs
##|*NAME=Status: RRD Graphs page
##|*DESCR=Allow access to the 'Status: RRD Graphs' page.
##|*MATCH=status_rrd_graph.php*
##|-PRIV

require("guiconfig.inc");
require_once("mail_reports.inc");

/* if the rrd graphs are not enabled redirect to settings page */
if(! isset($config['rrd']['enable'])) {
	header("Location: status_rrd_graph_settings.php");
	exit;
}

$graphid = $_GET['graphid'];
if (isset($_POST['graphid']))
	$graphid = $_POST['graphid'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

if (!is_array($config['mailreports']['schedule']))
	$config['mailreports']['schedule'] = array();

$a_mailreports = &$config['mailreports']['schedule'];
if (isset($id) && $a_mailreports[$id]) {
	if (!is_array($a_mailreports[$id]['row']))
		$a_mailreports[$id]['row'] = array();
	$pconfig = $a_mailreports[$id];
	$a_graphs = $a_mailreports[$id]['row'];
} else {
	$pconfig = array();
	$a_graphs = array();
}

if ($_GET['act'] == "del") {
	if ($a_graphs[$graphid]) {
		unset($a_graphs[$graphid]);
		$a_mailreports[$id]['row'] = $a_graphs;
		write_config();
		header("Location: status_mail_report_edit.php?id={$id}");
		exit;
	}
}

$frequencies = array("daily", "weekly", "monthly");
$daysofweek = array(
		"" => "",
		"0" => "sunday",
		"1" => "monday",
		"2" => "tuesday",
		"3" => "wednesday",
		"4" => "thursday",
		"5" => "friday",
		"6" => "saturday");
$dayofmonth = array("", "1", "15");

if ($_POST) {
	unset($_POST['__csrf_magic']);
	$pconfig = $_POST;
	if ($_POST['Submit'] == "Send Now") {
		mwexec_bg("/usr/local/bin/mail_reports_generate.php {$id}");
		header("Location: status_mail_report_edit.php?id={$id}");
		exit;
	}
	$friendly = "";

	// Default to midnight if unset/invalid.
	$pconfig['timeofday'] = isset($pconfig['timeofday']) ? $pconfig['timeofday'] : 0;
	$friendlytime = sprintf("%02d:00", $pconfig['timeofday']);
	$friendly = "Daily at {$friendlytime}";

	// If weekly, check for day of week
	if ($pconfig['frequency'] == "weekly") {
		$pconfig['dayofweek'] = isset($pconfig['dayofweek']) ? $pconfig['dayofweek'] : 0;
		$friendly = "Weekly, on {$daysofweek[$pconfig['dayofweek']]} at {$friendlytime}";
	} else {
		if (isset($pconfig['dayofweek']))
			unset($pconfig['dayofweek']);
	}

	// If monthly, check for day of the month
	if ($pconfig['frequency'] == "monthly") {
		$pconfig['dayofmonth'] = isset($pconfig['dayofmonth']) ? $pconfig['dayofmonth'] : 1;
		$friendly = "Monthly, on day {$pconfig['dayofmonth']} at {$friendlytime}";
	} else {
		if (isset($pconfig['dayofmonth']))
			unset($pconfig['dayofmonth']);
	}

	// Copy graphs back into the schedule.
	$pconfig["row"] = $a_graphs;

	$pconfig['schedule_friendly'] = $friendly;

	if (isset($id) && $a_mailreports[$id])
		$a_mailreports[$id] = $pconfig;
	else
		$a_mailreports[] = $pconfig;

	// Fix up cron job(s)
	set_mail_report_cron_jobs($a_mailreports);

	write_config();
	header("Location: status_mail_report.php");
	exit;
}

$pgtitle = array(gettext("Status"),gettext("Edit Mail Reports"));
include("head.inc");
?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr><td><div id="mainarea">
	<form action="status_mail_report_edit.php" method="post" name="iform" id="iform">
	<table class="tabcont" width="100%" border="0" cellpadding="1" cellspacing="1">
		<tr>
			<td class="listtopic" colspan="4">General Settings</td>
			<td></td>
		</tr>
		<tr>
			<td valign="top" class="vncell"><?=gettext("Description");?></td>
			<td class="vtable" colspan="3">
				<input name="descr" type="text" class="formfld unknown" id="descr" size="60" value="<?=htmlspecialchars($pconfig['descr']);?>">
			</td>
			<td></td>
		</tr>
		<tr>
			<td class="listtopic" colspan="4">Report Schedule</td>
			<td></td>
		</tr>
		<tr>
			<td class="vncellreq" valign="top" colspan="1">Frequency</td>
			<td class="vtable" colspan="3">
			<select name="frequency">
			<?php foreach($frequencies as $freq): ?>
				<option value="<?php echo $freq; ?>" <?php if($pconfig["frequency"] === $freq) echo "selected"; ?>><?php echo ucwords($freq); ?></option>
			<?php endforeach; ?>
			</select>
			<br/>Select the frequency for the report to be sent via e-mail.
			<br/>
			</td>
			<td></td>
		</tr>
		<tr>
			<td class="vncell" valign="top" colspan="1">Day of the Week</td>
			<td class="vtable" colspan="3">
			<select name="dayofweek">
			<?php foreach($daysofweek as $dowi => $dow): ?>
				<option value="<?php echo $dowi; ?>" <?php if($pconfig["dayofweek"] == $dowi) echo "selected"; ?>><?php echo ucwords($dow); ?></option>
			<?php endforeach; ?>
			</select>
			<br/>Select the day of the week to send the report. Only valid for weekly reports.
			<br/>
			</td>
			<td></td>
		</tr>
		<tr>
			<td class="vncell" valign="top" colspan="1">Day of the Month</td>
			<td class="vtable" colspan="3">
			<select name="dayofmonth">
			<?php foreach($dayofmonth as $dom): ?>
				<option value="<?php echo $dom; ?>" <?php if($pconfig["dayofmonth"] === $dom) echo "selected"; ?>><?php echo $dom; ?></option>
			<?php endforeach; ?>
			</select>
			<br/>Select the day of the month to send the report. Only valid for monthly reports.
			<br/>
			</td>
			<td></td>
		</tr>
		<tr>
			<td class="vncell" valign="top" colspan="1">Hour of Day</td>
			<td class="vtable" colspan="3">
			<select name="timeofday">
				<option value="" <?php if($pconfig["timeofday"] == "") echo "selected"; ?>></option>
				<?php for($i=0; $i < 24; $i++): ?>
				<option value="<?php echo $i; ?>" <?php if("{$pconfig['timeofday']}" == "{$i}") echo "selected"; ?>><?php echo $i; ?></option>
				<?php endfor; ?>
			</select>
			<br/>Select the hour of the day when the report should be sent. Be aware that scheduling reports between 1am-3am can cause issues during DST switches in zones that have them. Valid for any type of report.
			<br/>
			</td>
			<td></td>
		</tr>
		<tr>
			<td class="listtopic" colspan="4">Report Graphs</td>
			<td></td>
		</tr>
		<tr>
			<td width="30%" class="listhdr"><?=gettext("Graph");?></td>
			<td width="20%" class="listhdr"><?=gettext("Style");?></td>
			<td width="20%" class="listhdr"><?=gettext("Time Span");?></td>
			<td width="20%" class="listhdr"><?=gettext("Period");?></td>
			<td width="10%" class="list">
			<?php if (isset($id) && $a_mailreports[$id]): ?>
				<a href="status_mail_report_add_graph.php?reportid=<?php echo $id ;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a>
			</td>
			<?php else: ?>
			</td>
				<tr><td colspan="5" align="center"><br/>Save the report first, then you may add graphs.<br/></td></tr>
			<?php endif; ?>
		</tr>
		<?php $i = 0; foreach ($a_graphs as $graph): 
			$optionc = split("-", $graph['graph']);
			$search = array("-", ".rrd", $optionc);
			$replace = array(" :: ", "", $friendly);
			$prettyprint = ucwords(str_replace($search, $replace, $graph['graph']));
		?>
		<tr ondblclick="document.location='status_mail_report_edit.php?id=<?=$i;?>'">
			<td class="listlr"><?php echo $prettyprint; ?></td>
			<td class="listlr"><?php echo $graph['style']; ?></td>
			<td class="listlr"><?php echo $graph['timespan']; ?></td>
			<td class="listlr"><?php echo $graph['period']; ?></td>
			<td valign="middle" nowrap class="list">
				<a href="status_mail_report_add_graph.php?reportid=<?php echo $id ;?>&id=<?=$i;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a>
				&nbsp;
				<a href="status_mail_report_edit.php?act=del&id=<?php echo $id ;?>&graphid=<?=$i;?>" onclick="return confirm('<?=gettext("Do you really want to delete this entry?");?>')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a>
			</td>
		</tr>
		<?php $i++; endforeach; ?>
		<tr>
			<td class="list" colspan="4"></td>
			<td class="list">
			<?php if (isset($id) && $a_mailreports[$id]): ?>
				<a href="status_mail_report_add_graph.php?reportid=<?php echo $id ;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a>
			<?php endif; ?>
			</td>
		</tr>
		<tr>
			<td colspan="4" align="center">
			<input name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>">
			<?php if (isset($id) && $a_mailreports[$id]): ?>
			<input name="Submit" type="submit" class="formbtn" value="<?=gettext("Send Now");?>">
			<?php endif; ?>
			<a href="status_mail_report.php"><input name="cancel" type="button" class="formbtn" value="<?=gettext("Cancel");?>"></a>
			<?php if (isset($id) && $a_mailreports[$id]): ?>
			<input name="id" type="hidden" value="<?=htmlspecialchars($id);?>">
			<?php endif; ?>
			</td>
			<td></td>
		</tr>
		<tr>
			<td colspan="4" class="list"><p class="vexpl">
				<span class="red"><strong><?=gettext("Note:");?><br></strong></span>
				<?=gettext("Click + above to add graphs to this report.");?><br/>
				Configure your SMTP settings under <a href="/system_advanced_notifications.php">System -&gt; Advanced, on the Notifications tab</a>.
			</td>
			<td class="list">&nbsp;</td>
		</tr>
	</table>
	</form>
	</div></td></tr>
</table>

<?php include("fend.inc"); ?>
</body>
</html>
