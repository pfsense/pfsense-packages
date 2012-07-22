<?php
/*
    snort_alerts.widget.php
    Copyright (C) 2009 Jim Pingle
    mod 19-07-2012

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
global $config, $g;
$snort_alerts_title = "Snort Alerts";
$snort_alerts_title_link = "snort/snort_alerts.php";

/* retrieve snort variables */
require_once("/usr/local/pkg/snort/snort.inc");

if (!is_array($config['installedpackages']['snortglobal']['rule']))
	$config['installedpackages']['snortglobal']['rule'] = array();
$a_instance = &$config['installedpackages']['snortglobal']['rule'];

/* read log file(s) */
$snort_alerts = array();
$tmpblocked = array_flip(snort_get_blocked_ips());
foreach ($a_instance as $instanceid => $instance) {
	if ($instance['enable'] != 'on')
		continue;

	/* make sure alert file exists */
	if (file_exists("/var/log/snort/snort_{$if_real}{$snort_uuid}/alert")) {
		$snort_uuid = $instance['uuid'];
		$if_real = snort_get_real_interface($instance['interface']);
		$tmpfile = "{$g['tmp_path']}/.widget_alert_{$snort_uuid}";
		if (isset($config['syslog']['reverse']))
			exec("tail -10 /var/log/snort/snort_{$if_real}{$snort_uuid}/alert | sort -r > {$tmpfile}");
		else
			exec("tail -10 /var/log/snort/snort_{$if_real}{$snort_uuid}/alert > {$tmpfile}");
		if (file_exists($tmpfile)) {
			/*                 0         1           2      3      4    5    6    7      8     9    10    11             12    */
			/* File format timestamp,sig_generator,sig_id,sig_rev,msg,proto,src,srcport,dst,dstport,id,classification,priority */
			$fd = fopen($tmpfile, "r");
			while (($fileline = @fgets($fd))) {
				if (empty($fileline))
					continue;
				$fields = explode(",", $fileline);

				$snort_alert = array();
				$snort_alert[]['instanceid'] = snort_get_friendly_interface($instance['interface']);
				$snort_alert[]['timestamp'] = $fields[0];
				$snort_alert[]['timeonly'] = substr($fields[0], 6, -8);
				$snort_alert[]['dateonly'] = substr($fields[0], 0, -17);
				$snort_alert[]['src'] = $fields[6];
				$snort_alert[]['srcport'] = $fields[7];
				$snort_alert[]['dst'] = $fields[8];
				$snort_alert[]['dstport'] = $fields[9];
				$snort_alert[]['priority'] = $fields[12];
				$snort_alert[]['category'] = $fields[11];
				$snort_alerts[] = $snort_alert;
			}
			fclose($fd);
			@unlink($tmpfile);
		}
	}
}

if ($_GET['evalScripts']) {
	/* AJAX specific handlers */
        $new_rules = "";
	foreach($snort_alerts as $log_row)
		$new_rules .= "{$log_row['time']}||{$log_row['priority']}||{$log_row['category']}||{$log_row['src']}||{$log_row['dst']}||{$log_row['timestamp']}||{$log_row['timeonly']}||{$log_row['dateonly']}\n";

	echo $new_rules;
} else {
/* display the result */
?>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
	<tbody>
		<tr class="snort-alert-header">
		  <td width="30%" class="widgetsubheader" >IF/Date</td>
			<td width="40%" class="widgetsubheader">Src/Dst</td>
			<td width="40%" class="widgetsubheader">Details</td>
		</tr>
<?php
foreach ($snort_alerts as $counter => $alert) {
	echo("	<tr class='snort-alert-entry'" . $activerow . ">
			<td width='30%' class='listr'>{$alert['instanceid']}<br/>{$alert['timeonly']} {$alert['dateonly']}</td>		
			<td width='40%' class='listr'>{$alert['src']}:{$alert['srcport']}<br/>{$alert['dst']}:{$alert['dstport']}</td>
			<td width='40%' class='listr'>Pri : {$alert['priority']}<br/>Cat : {$alert['category']}</td>
		</tr>");
}
?>
	</tbody>
</table>
<?php } ?>
