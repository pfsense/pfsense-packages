<?php
/*
    snort_alerts.widget.php
    Copyright (C) 2009 Jim Pingle
    mod 24-07-2012

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

require_once("guiconfig.inc");
require_once("/usr/local/www/widgets/include/widget-snort.inc");

global $config, $g;

/* array sorting */
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

/* check if firewall widget variable is set */
$nentries = $config['widgets']['widget_snort_display_lines'];
if (!isset($nentries) || $nentries < 0) $nentries = 5;

if(isset($_POST['widget_snort_display_lines'])) {
	$config['widgets']['widget_snort_display_lines'] = $_POST['widget_snort_display_lines'];
	write_config("Saved Snort Alerts Widget Displayed Lines Parameter via Dashboard");
	header("Location: ../../index.php");
}

/* check if Snort include file exists before we use it */
if (file_exists("/usr/local/pkg/snort/snort.inc")) {
	require_once("/usr/local/pkg/snort/snort.inc");

	/* retrieve snort variables */
	if (!is_array($config['installedpackages']['snortglobal']['rule']))
		$config['installedpackages']['snortglobal']['rule'] = array();
	$a_instance = &$config['installedpackages']['snortglobal']['rule'];

	/* read log file(s) */
	$counter=0;
	foreach ($a_instance as $instanceid => $instance) {
		$snort_uuid = $a_instance[$instanceid]['uuid'];
		$if_real = snort_get_real_interface($a_instance[$instanceid]['interface']);

		/* make sure alert file exists */
		if (file_exists("/var/log/snort/snort_{$if_real}{$snort_uuid}/alert")) {
			exec("tail -n{$nentries} /var/log/snort/snort_{$if_real}{$snort_uuid}/alert > /tmp/alert_{$snort_uuid}");
			if (file_exists("/tmp/alert_{$snort_uuid}")) {
				$tmpblocked = array_flip(snort_get_blocked_ips());

				/*                 0         1           2      3      4    5    6    7      8     9    10    11             12    */
				/* File format timestamp,sig_generator,sig_id,sig_rev,msg,proto,src,srcport,dst,dstport,id,classification,priority */
				$fd = fopen("/tmp/alert_{$snort_uuid}", "r");
				while (($fields = fgetcsv($fd, 1000, ',', '"')) !== FALSE) {
					if(count($fields) < 11)
						continue;

					$snort_alerts[$counter]['instanceid'] = $a_instance[$instanceid]['interface'];
					// fields[0] is the timestamp.  Reverse its date order to YY/MM/DD for proper sorting
					$tmp = substr($fields[0],6,2) . '/' . substr($fields[0],0,2) . '/' . substr($fields[0],3,2);
					$snort_alerts[$counter]['timestamp'] = str_replace(substr($fields[0],0,8),$tmp,$fields[0]);
					$snort_alerts[$counter]['timeonly'] = substr($fields[0], strpos($fields[0], '-')+1, -8);
					$snort_alerts[$counter]['dateonly'] = substr($fields[0], 0, strpos($fields[0], '-'));
					$snort_alerts[$counter]['src'] = $fields[6];
					$snort_alerts[$counter]['srcport'] = $fields[7];
					$snort_alerts[$counter]['dst'] = $fields[8];
					$snort_alerts[$counter]['dstport'] = $fields[9];
					$snort_alerts[$counter]['priority'] = $fields[12];
					$snort_alerts[$counter]['category'] = $fields[11];
					$counter++;
				};
				fclose($fd);
				@unlink("/tmp/alert_{$snort_uuid}");
			};
		};
	};

	/* sort the array */
	if (isset($config['syslog']['reverse'])) {
		sksort($snort_alerts, 'timestamp', false);
	} else {
		sksort($snort_alerts, 'timestamp', true);
	};
} else {
	$msg = gettext("The Snort package is not installed.");
}

/* display the result */
?>

<input type="hidden" id="snort_alerts-config" name="snort_alerts-config" value="" />
<div id="snort_alerts-settings" class="widgetconfigdiv" style="display:none;">
	<form action="/widgets/widgets/snort_alerts.widget.php" method="post" name="iformd">
		Enter number of recent alerts to display (default is 5)<br/>
		<input type="text" size="5" name="widget_snort_display_lines" class="formfld unknown" id="widget_snort_display_lines" value="<?= $config['widgets']['widget_snort_display_lines'] ?>" />
		&nbsp;&nbsp;<input id="submitd" name="submitd" type="submit" class="formbtn" value="Save" />
    </form>
</div>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
	<tbody>
		<tr class="snort-alert-header">
		  <td width="30%" class="widgetsubheader" >IF/Date</td>
			<td width="40%" class="widgetsubheader">Src/Dst</td>
			<td width="40%" class="widgetsubheader">Details</td>
		</tr>
<?php
$counter=0;
if (is_array($snort_alerts)) {
	foreach ($snort_alerts as $alert) {
		echo("	<tr class='snort-alert-entry'" . $activerow . ">
				<td width='30%' class='listr'>" . $alert['instanceid'] . "<br>" . $alert['timeonly'] . " " . $alert['dateonly'] . "</td>		
				<td width='40%' class='listr'>" . $alert['src'] . ":" . $alert['srcport'] . "<br>" . $alert['dst'] . ":" . $alert['dstport'] . "</td>
				<td width='40%' class='listr'>Pri : " . $alert['priority'] . "<br>Cat : " . $alert['category'] . "</td>
			</tr>");
		$counter++;
		if($counter >= $nentries) break;
	}
} else {
	if (!empty($msg)) {
		echo ("  <tr class=\"snort-alert-entry\">
				<td colspan=\"3\" align=\"center\"><br>{$msg}</br></td>
			 </tr>");
	}
}
?>
	</tbody>
</table>

<!-- needed to display the widget settings menu -->
<script type="text/javascript">
//<![CDATA[
	selectIntLink = "snort_alerts-configure";
	textlink = document.getElementById(selectIntLink);
	textlink.style.display = "inline";
//]]>
</script>

