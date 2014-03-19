<?php
/*
    suricata_alerts.widget.php
    Copyright (C) 2009 Jim Pingle
    mod 24-07-2012

    Copyright (C) 2014 Bill Meeks
    mod 03-Mar-2014 adapted for use with Suricata by Bill Meeks

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
require_once("/usr/local/www/widgets/include/widget-suricata.inc");

global $config, $g;

/* Retrieve Suricata configuration */
if (!is_array($config['installedpackages']['suricata']['rule']))
	$config['installedpackages']['suricata']['rule'] = array();
$a_instance = &$config['installedpackages']['suricata']['rule'];

/* array sorting */
function sksort(&$array, $subkey="id", $sort_ascending=false) {
        /* an empty array causes sksort to fail - this test alleviates the error */
	if(empty($array))
	        return false;
	if (count($array)){
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

/* check if suricata widget variable is set */
$suri_nentries = $config['widgets']['widget_suricata_display_lines'];
if (!isset($suri_nentries) || $suri_nentries < 0)
	$suri_nentries = 5;

// Called by Ajax to update alerts table contents
if (isset($_GET['getNewAlerts'])) {
	$response = "";
	$suri_alerts = suricata_widget_get_alerts();
	$counter = 0;
	foreach ($suri_alerts as $a) {
		$response .= $a['instanceid'] . " " . $a['dateonly'] . "||" . $a['timeonly'] . "||" . $a['src'] . "||";
		$response .= $a['dst'] . "||" . $a['priority'] . "||" . $a['category'] . "\n";
		$counter++;
		if($counter >= $suri_nentries)
			break;
	}
	echo $response;
	return;
}

if(isset($_POST['widget_suricata_display_lines'])) {
	$config['widgets']['widget_suricata_display_lines'] = $_POST['widget_suricata_display_lines'];
	write_config("Saved Suricata Alerts Widget Displayed Lines Parameter via Dashboard");
	header("Location: ../../index.php");
}

// Read "$suri_nentries" worth of alerts from the top of the alerts.log file
function suricata_widget_get_alerts() {

	global $config, $a_instance, $suri_nentries;
	$suricata_alerts = array();

	/* read log file(s) */
	$counter=0;
	foreach ($a_instance as $instanceid => $instance) {
		$suricata_uuid = $a_instance[$instanceid]['uuid'];
		$if_real = get_real_interface($a_instance[$instanceid]['interface']);

		// make sure alert file exists, then grab the most recent {$suri_nentries} from it
		// and write them to a temp file.
		if (file_exists("/var/log/suricata/suricata_{$if_real}{$suricata_uuid}/alerts.log")) {
			exec("tail -{$suri_nentries} -r /var/log/suricata/suricata_{$if_real}{$suricata_uuid}/alerts.log > /tmp/surialerts_{$suricata_uuid}");
			if (file_exists("/tmp/surialerts_{$suricata_uuid}")) {

				/*              0         1      2             3      4       5   6              7        8     9   10      11  12      */
				/* File format: timestamp,action,sig_generator,sig_id,sig_rev,msg,classification,priority,proto,src,srcport,dst,dstport */
				$fd = fopen("/tmp/surialerts_{$suricata_uuid}", "r");
				while (($fields = fgetcsv($fd, 1000, ',', '"')) !== FALSE) {
					if(count($fields) < 13)
						continue;

					// Create a DateTime object from the event timestamp that
					// we can use to easily manipulate output formats.
					$event_tm = date_create_from_format("m/d/Y-H:i:s.u", $fields[0]);

					// Check the 'CATEGORY' field for the text "(null)" and
					// substitute "No classtype defined".
					if ($fields[6] == "(null)")
						$fields[6] = "No classtype assigned";

					$suricata_alerts[$counter]['instanceid'] = strtoupper($a_instance[$instanceid]['interface']);
					$suricata_alerts[$counter]['timestamp'] = strval(date_timestamp_get($event_tm));
					$suricata_alerts[$counter]['timeonly'] = date_format($event_tm, "H:i:s");
					$suricata_alerts[$counter]['dateonly'] = date_format($event_tm, "M d");
					// Add square brackets around any IPv6 address
					if (is_ipaddrv6($fields[9]))
						$suricata_alerts[$counter]['src'] = "[" . $fields[9] . "]";
					else
						$suricata_alerts[$counter]['src'] = $fields[9];
					// Add the SRC PORT if not null
					if (!empty($fields[10]))					
						$suricata_alerts[$counter]['src'] .= ":" . $fields[10];
					// Add square brackets around any IPv6 address
					if (is_ipaddrv6($fields[11]))
						$suricata_alerts[$counter]['dst'] = "[" . $fields[11] . "]";
					else
						$suricata_alerts[$counter]['dst'] = $fields[11];
					// Add the SRC PORT if not null
					if (!empty($fields[12]))
						$suricata_alerts[$counter]['dst'] .= ":" . $fields[12];
					$suricata_alerts[$counter]['priority'] = $fields[7];
					$suricata_alerts[$counter]['category'] = $fields[6];
					$counter++;
				};
				fclose($fd);
				@unlink("/tmp/surialerts_{$suricata_uuid}");
			};
		};
	};

	// Sort the alerts array
	if (isset($config['syslog']['reverse'])) {
		sksort($suricata_alerts, 'timestamp', false);
	} else {
		sksort($suricata_alerts, 'timestamp', true);
	}

	return $suricata_alerts;
}

/* display the result */
?>

<input type="hidden" id="suricata_alerts-config" name="suricata_alerts-config" value=""/>
<div id="suricata_alerts-settings" class="widgetconfigdiv" style="display:none;">
	<form action="/widgets/widgets/suricata_alerts.widget.php" method="post" name="iformd">
		Enter number of recent alerts to display (default is 5)<br/>
		<input type="text" size="5" name="widget_suricata_display_lines" class="formfld unknown" id="widget_suricata_display_lines" value="<?= $config['widgets']['widget_suricata_display_lines'] ?>" />
		&nbsp;&nbsp;<input id="submitd" name="submitd" type="submit" class="formbtn" value="Save" />
    </form>
</div>

<table width="100%" border="0" cellspacing="0" cellpadding="0" style="table-layout: fixed;">
	<colgroup>
		<col style="width: 24%;" />
		<col style="width: 38%;" />
		<col style="width: 38%;" />
	</colgroup>
	<thead>
		<tr>
			<th class="listhdrr"><?=gettext("IF/Date");?></th>
			<th class="listhdrr"><?=gettext("Src/Dst Address");?></th>
			<th class="listhdrr"><?=gettext("Classification");?></th>
		</tr>
	</thead>
	<tbody id="suricata-alert-entries">
	<?php
		$suricata_alerts = suricata_widget_get_alerts($suri_nentries);
		$counter=0;
		if (is_array($suricata_alerts)) {
			foreach ($suricata_alerts as $alert) {
				$evenRowClass = $counter % 2 ? " listMReven" : " listMRodd";
				echo("	<tr class='" . $evenRowClass . "'>
				<td class='listMRr'>" . $alert['instanceid'] . " " . $alert['dateonly'] . "<br/>" . $alert['timeonly'] . "</td>		
				<td class='listMRr ellipsis' nowrap><div style='display:inline;' title='" . $alert['src'] . "'>" . $alert['src'] . "</div><br/><div style='display:inline;' title='" . $alert['dst'] . "'>" . $alert['dst'] . "</div></td>
				<td class='listMRr'>Pri: " . $alert['priority'] . " " . $alert['category'] . "</td></tr>");
				$counter++;
				if($counter >= $suri_nentries)
					break;
			}
		}
	?>
	</tbody>
</table>

<script type="text/javascript">
//<![CDATA[
	var suricataupdateDelay = 10000; // update every 10 seconds
	var suri_nentries = <?php echo $suri_nentries; ?>; // default is 5

<!-- needed to display the widget settings menu -->
	selectIntLink = "suricata_alerts-configure";
	textlink = document.getElementById(selectIntLink);
	textlink.style.display = "inline";
//]]>
</script>

