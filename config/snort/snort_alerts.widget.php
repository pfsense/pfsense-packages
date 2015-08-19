<?php
/*
    snort_alerts.widget.php
    Copyright (C) 2009 Jim Pingle
    mod 24-07-2012
    mod 28-02-2014 by Bill Meeks

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INClUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

$nocsrf = true;

require_once("guiconfig.inc");
require_once("/usr/local/www/widgets/include/widget-snort.inc");

global $config, $g;

/* retrieve snort variables */
if (!is_array($config['installedpackages']['snortglobal']['rule']))
	$config['installedpackages']['snortglobal']['rule'] = array();
$a_instance = &$config['installedpackages']['snortglobal']['rule'];

// Set some CSS class variables
$alertRowEvenClass = "listMReven";
$alertRowOddClass = "listMRodd";
$alertColClass = "listMRr";

/* check if Snort widget alert display lines value is set */
$snort_nentries = $config['widgets']['widget_snort_display_lines'];
if (!isset($snort_nentries) || $snort_nentries <= 0)
	$snort_nentries = 5;

/* array sorting of the alerts */
function sksort(&$array, $subkey="id", $sort_ascending=false) {
        /* an empty array causes sksort to fail - this test alleviates the error */
	if(empty($array))
	        return false;
	if (count($array)) {
		$temp_array[key($array)] = array_shift($array);
	};
	foreach ($array as $key => $val){
		$offset = 0;
		$found = false;
		foreach ($temp_array as $tmp_key => $tmp_val) {
			if (!$found and strtolower($val[$subkey]) > strtolower($tmp_val[$subkey])) {
				$temp_array = array_merge((array)array_slice($temp_array,0,$offset), array($key => $val), array_slice($temp_array,$offset));
				$found = true;
			};
			$offset++;
		};
		if (!$found) $temp_array = array_merge($temp_array, array($key => $val));
	};

	if ($sort_ascending) {
		$array = array_reverse($temp_array);
	} else $array = $temp_array;
        /* below is the complement for empty array test */
        return true; 
};

// Called by Ajax to update the "snort-alert-entries" <tbody> table element's contents
if (isset($_GET['getNewAlerts'])) {
	$response = "";
	$s_alerts = snort_widget_get_alerts();
	$counter = 0;
	foreach ($s_alerts as $a) {
		$response .= $a['instanceid'] . " " . $a['dateonly'] . "||" . $a['timeonly'] . "||" . $a['src'] . "||";
		$response .= $a['dst'] . "||" . $a['msg'] . "\n";
		$counter++;
		if($counter >= $snort_nentries)
			break;
	}
	echo $response;
	return;
}

// See if saving new display line count value
if(isset($_POST['widget_snort_display_lines'])) {
	if($_POST['widget_snort_display_lines'] == "") {
		unset($config['widgets']['widget_snort_display_lines']);
	} else {
		$config['widgets']['widget_snort_display_lines'] = max(intval($_POST['widget_snort_display_lines']), 1);
	}
	write_config("Saved Snort Alerts Widget Displayed Lines Parameter via Dashboard");
	header("Location: ../../index.php");
}

// Read "$snort_nentries" worth of alerts from the top of the alert.log file
// of each configured interface, and then return the most recent '$snort_entries'
// alerts in a sorted array (most recent alert first).
function snort_widget_get_alerts() {

	global $config, $a_instance, $snort_nentries;
	$snort_alerts = array();
	/* read log file(s) */
	$counter=0;
	foreach ($a_instance as $instanceid => $instance) {
		$snort_uuid = $a_instance[$instanceid]['uuid'];
		$if_real = get_real_interface($a_instance[$instanceid]['interface']);

		/* make sure alert file exists, then "tail" the last '$snort_nentries' from it */
		if (file_exists("/var/log/snort/snort_{$if_real}{$snort_uuid}/alert")) {
			exec("tail -{$snort_nentries} -r /var/log/snort/snort_{$if_real}{$snort_uuid}/alert > /tmp/alert_snort{$snort_uuid}");

			if (file_exists("/tmp/alert_snort{$snort_uuid}")) {

				/*              0         1            2      3       4   5     6   7       8   9       10 11             12       */
				/* File format: timestamp,generator_id,sig_id,sig_rev,msg,proto,src,srcport,dst,dstport,id,classification,priority */
				if (!$fd = fopen("/tmp/alert_snort{$snort_uuid}", "r")) {
					log_error(gettext("[Snort Widget] Failed to open file /tmp/alert_snort{$snort_uuid}"));
					continue;
				}
				while (($fields = fgetcsv($fd, 1000, ',', '"')) !== FALSE) {
					if(count($fields) < 13)
						continue;

					// Get the Snort interface this alert was received from
					$snort_alerts[$counter]['instanceid'] = strtoupper($a_instance[$instanceid]['interface']);

					// "fields[0]" is the complete timestamp in ASCII form. Convert
					// to a UNIX timestamp so we can use it for various date and
					// time formatting.  Also extract the MM/DD/YY component and
					// reverse its order to YY/MM/DD for proper sorting.
					$fields[0] = trim($fields[0]); // remove trailing space before comma delimiter
					$tstamp = strtotime(str_replace("-", " ", $fields[0])); // remove "-" between date and time components
					$tmp = substr($fields[0],6,2) . '/' . substr($fields[0],0,2) . '/' . substr($fields[0],3,2);
					$snort_alerts[$counter]['timestamp'] = str_replace(substr($fields[0],0,8),$tmp,$fields[0]);

					$snort_alerts[$counter]['timeonly'] = date("H:i:s", $tstamp);
					$snort_alerts[$counter]['dateonly'] = date("M d", $tstamp);
					// Add square brackets around any any IPv6 address
					if (strpos($fields[6], ":") === FALSE)
						$snort_alerts[$counter]['src'] = trim($fields[6]);
					else
						$snort_alerts[$counter]['src'] = "[" . trim($fields[6]) . "]";
					// Add the SRC PORT if not null
					if (!empty($fields[7]))
						$snort_alerts[$counter]['src'] .= ":" . trim($fields[7]);
					// Add square brackets around any any IPv6 address
					if (strpos($fields[8], ":") === FALSE)
						$snort_alerts[$counter]['dst'] = trim($fields[8]);
					else
						$snort_alerts[$counter]['dst'] = "[" . trim($fields[8]) . "]";
					// Add the DST PORT if not null
					if (!empty($fields[9]))
						$snort_alerts[$counter]['dst'] .= ":" . trim($fields[9]);
					$snort_alerts[$counter]['msg'] = trim($fields[4]);
					$counter++;
				};
				fclose($fd);
				@unlink("/tmp/alert_snort{$snort_uuid}");
			};
		};
	};

	/* sort the alerts array */
	if (isset($config['syslog']['reverse'])) {
		sksort($snort_alerts, 'timestamp', false);
	} else {
		sksort($snort_alerts, 'timestamp', true);
	};

	return $snort_alerts;
}
?>

<input type="hidden" id="snort_alerts-config" name="snort_alerts-config" value="" />
<div id="snort_alerts-settings" class="widgetconfigdiv" style="display:none;">
	<form action="/widgets/widgets/snort_alerts.widget.php" method="post" name="iformd">
		Enter number of recent alerts to display (default is 5)<br/>
		<input type="text" size="5" name="widget_snort_display_lines" class="formfld unknown" id="widget_snort_display_lines" value="<?= $config['widgets']['widget_snort_display_lines'] ?>" />
		&nbsp;&nbsp;<input id="submitd" name="submitd" type="submit" class="formbtn" value="Save" />
    </form>
</div>

<table id="snort-alert-tbl" width="100%" border="0" cellspacing="0" cellpadding="0" style="table-layout: fixed;">
	<colgroup>
		<col style="width: 24%;" />
		<col style="width: 38%;" />
		<col style="width: 38%;" />
	</colgroup>
	<thead>
		<tr>
			<th class="widgetsubheader"><?=gettext("IF/Date");?></th>
			<th class="widgetsubheader"><?=gettext("Src/Dst Address");?></th>
			<th class="widgetsubheader"><?=gettext("Description");?></th>
		</tr>
	</thead>
	<tbody id="snort-alert-entries">
	<?php
		$snort_alerts = snort_widget_get_alerts();
		$counter=0;
		if (is_array($snort_alerts)) {
			foreach ($snort_alerts as $alert) {
				$alertRowClass = $counter % 2 ? $alertRowEvenClass : $alertRowOddClass;
				echo("	<tr class='" . $alertRowClass . "'>
				<td class='listMRr'>" . $alert['instanceid'] . "&nbsp;" . $alert['dateonly'] . "<br/>" . $alert['timeonly'] . "</td>
				<td class='listMRr' style='overflow: hidden; text-overflow: ellipsis;' nowrap><div style='display:inline;' title='" . $alert['src'] . "'>" . $alert['src'] . "</div><br/><div style='display:inline;' title='" . $alert['dst'] . "'>" . $alert['dst'] . "</div></td>
				<td class='listMRr'><div style='display: fixed; display: -webkit-box; -webkit-line-clamp: 2; -webkit-box-orient: vertical; line-height: 1.2em; max-height: 2.4em; overflow: hidden; text-overflow: ellipsis;' title='{$alert['msg']}'>" . $alert['msg'] . "</div></td></tr>");
				$counter++;
				if($counter >= $snort_nentries)
					break;
			}
		}
	?>
	</tbody>
</table>

<script type="text/javascript">
//<![CDATA[
<!-- needed in the snort_alerts.js file code -->
	var snortupdateDelay = 10000; // update every 10 seconds
	var snort_nentries = <?=$snort_nentries;?>; // number of alerts to display (5 is default)
	var snortWidgetRowEvenClass = "<?=$alertRowEvenClass;?>"; // allows alternating background
	var snortWidgetRowOddClass = "<?=$alertRowOddClass;?>"; // allows alternating background

<!-- needed to display the widget settings menu -->
	selectIntLink = "snort_alerts-configure";
	textlink = document.getElementById(selectIntLink);
	textlink.style.display = "inline";
//]]>
</script>

