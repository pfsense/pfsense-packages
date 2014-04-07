<?php
/*
	suricata_logs_browser.php

	Copyright (C) 2014 Bill Meeks
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
require_once("/usr/local/pkg/suricata/suricata.inc");

if (isset($_POST['instance']) && is_numericint($_POST['instance']))
	$instanceid = $_POST['instance'];
elseif (isset($_GET['instance']) && is_numericint($_GET['instance']))
	$instanceid = htmlspecialchars($_GET['instance']);
if (empty($instanceid))
	$instanceid = 0;

if (!is_array($config['installedpackages']['suricata']['rule']))
	$config['installedpackages']['suricata']['rule'] = array();
$a_instance = $config['installedpackages']['suricata']['rule'];
$suricata_uuid = $a_instance[$instanceid]['uuid'];
$if_real = get_real_interface($a_instance[$instanceid]['interface']);

// Construct a pointer to the instance's logging subdirectory
$suricatalogdir = SURICATALOGDIR . "suricata_{$if_real}{$suricata_uuid}";

$logfile = $_POST['file'];

if ($_POST['action'] == 'load') {
	if(!is_file($_POST['file'])) {
		echo "|3|" . gettext("Log file does not exist or that logging feature is not enabled") . ".|";
	}
	else {
		$data = file_get_contents($_POST['file']);
		if($data === false) {
			echo "|1|" . gettext("Failed to read log file") . ".|";
		} else {
			$data = base64_encode($data);
			echo "|0|{$_POST['file']}|{$data}|";	
		}
	}
	exit;
}

$pgtitle = gettext("Suricata: Logs Browser");
include_once("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">

<?php
include_once("fbegin.inc");
if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}
if ($input_errors) {
	print_input_errors($input_errors);
}

?>
<script type="text/javascript" src="/javascript/base64.js"></script>
<script type="text/javascript">	
	function loadFile() {
		jQuery("#fileStatus").html("<?=gettext("Loading file"); ?> ...");
		jQuery("#fileStatusBox").show(500);
		jQuery("#filePathBox").show(500);
		jQuery("#fbTarget").html("");

		jQuery.ajax(
			"<?=$_SERVER['SCRIPT_NAME'];?>", {
				type: 'POST',
				data: "action=load&file=" + jQuery("#logFile").val(),
				complete: loadComplete
			}
		);
	}

	function loadComplete(req) {
		jQuery("#fileContent").show(1000);
		var values = req.responseText.split("|");
		values.shift(); values.pop();

		if(values.shift() == "0") {
			var file = values.shift();
			var fileContent = Base64.decode(values.join("|"));
			jQuery("#fileStatus").html("<?=gettext("File successfully loaded"); ?>.");
			jQuery("#fbTarget").html(file);
			jQuery("#fileContent").val(fileContent);
		}
		else {
			jQuery("#fileStatus").html(values[0]);
			jQuery("#fbTarget").html("");
			jQuery("#fileContent").val("");
		}
		jQuery("#fileContent").show(1000);
	}

</script>

<form action="/suricata/suricata_logs_browser.php" method="post" id="formbrowse">
<input type="hidden" id="instance" value="<?=$instanceid;?>"/>
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr><td>
	<?php
        	$tab_array = array();
		$tab_array[] = array(gettext("Suricata Interfaces"), false, "/suricata/suricata_interfaces.php");
		$tab_array[] = array(gettext("Global Settings"), false, "/suricata/suricata_global.php");
		$tab_array[] = array(gettext("Update Rules"), false, "/suricata/suricata_download_updates.php");
		$tab_array[] = array(gettext("Alerts"), false, "/suricata/suricata_alerts.php?instance={$instanceid}");
		$tab_array[] = array(gettext("Suppress"), false, "/suricata/suricata_suppress.php");
		$tab_array[] = array(gettext("Logs Browser"), true, "/suricata/suricata_logs_browser.php");
		$tab_array[] = array(gettext("Logs Mgmt"), false, "/suricata/suricata_logs_mgmt.php");
		display_top_tabs($tab_array);
	?>
	</td>
	</tr>
	<tr>
	<td><div id="mainarea">
		<table id="maintable" class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="6">
			<tr>
				<td colspan="2" class="listtopic"><?php echo gettext("Logs Browser Selections"); ?></td>
			</tr>
			<tr>
				<td width="22%" class="vncell"><?php echo gettext('Instance to View'); ?></td>
				<td width="78%" class="vtable">
					<select name="instance" id="instance" class="formselect" onChange="document.getElementById('formbrowse').method='post';document.getElementById('formbrowse').submit()">
			<?php
				foreach ($a_instance as $id => $instance) {
					$selected = "";
					if ($id == $instanceid)
						$selected = "selected";
					echo "<option value='{$id}' {$selected}> (" . convert_friendly_interface_to_friendly_descr($instance['interface']) . ") {$instance['descr']}</option>\n";
				}
			?>
					</select>&nbsp;&nbsp;<?php echo gettext('Choose which instance logs you want to view.'); ?>
				</td>
			</tr>
			<tr>
				<td width="22%" class="vncell"><?php echo gettext('Log File to View'); ?></td>
				<td width="78%" class="vtable">
					<select name="logFile" id="logFile" class="formselect" onChange="loadFile();">
			<?php
				$logs = array( "alerts.log", "files-json.log", "http.log", "stats.log", "suricata.log", "tls.log" );
				foreach ($logs as $log) {
					$selected = "";
					if ($log == basename($logfile))
						$selected = "selected";
					echo "<option value='{$suricatalogdir}/{$log}' {$selected}>" . $log . "</option>\n";
				}
			?>
					</select>&nbsp;&nbsp;<?php echo gettext('Choose which log you want to view.'); ?>
				</td>
			</tr>
			<tr>
				<td colspan="2" class="listtopic"><?php echo gettext("Log Contents"); ?></td>
			</tr>
			<tr>
				<td colspan="2">
					<div style="display:none; " id="fileStatusBox">
						<div class="list" style="padding-left:15px;">
							<strong id="fileStatus"></strong>
						</div>
					</div>
					<div style="padding-left:15px; display:none;" id="filePathBox">
						<strong><?=gettext("Log File Path"); ?>:</strong>
						<div class="list" style="display:inline;" id="fbTarget"></div>
					</div>
				</td>
			</tr>
			<tr>
				<td colspan="2">
					<table width="100%">
						<tr>
							<td valign="top" class="label">
							<div style="background:#eeeeee;" id="fileOutput">
							<textarea id="fileContent" name="fileContent" style="width:100%;" rows="30" wrap="off"></textarea>
							</div>
							</td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
	</div>
	</td>
	</tr>
</table>
</form>

<?php if(empty($logfile)): ?>
<script type="text/javascript">
	document.getElementById("logFile").selectedIndex=-1;
</script>
<?php endif; ?>

<?php include("fend.inc"); ?>
</body>
</html>
