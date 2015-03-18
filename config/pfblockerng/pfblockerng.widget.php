<?php
/*
	pfBlockerNG.widget.php

	pfBlockerNG
	Copyright (C) 2015 BBcan177@gmail.com
	All rights reserved.

	Based Upon pfblocker :
	Copyright 2011 Thomas Schaefer - Tomschaefer.org
	Copyright 2011 Marcello Coutinho
	Part of pfSense widgets (www.pfsense.org)

	Adapted From:
	snort_alerts.widget.php
	Copyright (C) 2009 Jim Pingle
	mod 24-07-2012
	mod 28-02-2014 by Bill Meeks

	Javascript and Integration modifications by J. Nieuwenhuizen

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

@require_once("/usr/local/www/widgets/include/widget-pfblockerng.inc");
@require_once("/usr/local/pkg/pfblockerng/pfblockerng.inc");
@require_once("guiconfig.inc");
@require_once("globals.inc");
@require_once("pfsense-utils.inc");
@require_once("functions.inc");

pfb_global();

// Ackwnowlege Failed Downloads
if (isset($_POST['pfblockerngack'])) {
	$clear = exec("/usr/bin/sed -i '' 's/FAIL/Fail/g' {$pfb['errlog']}");
	header("Location: ../../index.php");
}

// This function will create the counts
function pfBlockerNG_get_counts() {
	global $config, $g, $pfb;

	// Collect Alias Count and Update Date/Time
	$pfb_table = array();
	$out = "<img src ='/themes/{$g['theme']}/images/icons/icon_interface_down.gif' title=\"No Rules are Defined using this Alias\" alt=\"\" />";
	$in = "<img src ='/themes/{$g['theme']}/images/icons/icon_interface_up.gif' title=\"Rules are Defined using this Alias\" alt=\"\" />";
	if (is_array($config['aliases']['alias'])) {
		foreach ($config['aliases']['alias'] as $cbalias) {
			if (preg_match("/pfB_/", $cbalias['name'])) {
				if (file_exists("{$pfb['aliasdir']}/{$cbalias['name']}.txt")) {	
					preg_match("/(\d+)/", exec("/usr/bin/grep -cv \"^1\.1\.1\.1\" {$pfb['aliasdir']}/{$cbalias['name']}.txt"), $matches);
					$pfb_table[$cbalias['name']] = array("count" => $matches[1], "img" => $out);
					$updates = exec("ls -ld {$pfb['aliasdir']}/{$cbalias['name']}.txt | awk '{ print $6,$7,$8 }'", $update);
					$pfb_table[$cbalias['name']]['up'] = $updates;
				}
			}
		}
	}

	// Collect if Rules are defined using pfBlockerNG Aliases.
	if (is_array($config['filter']['rule'])) {
		foreach ($config['filter']['rule'] as $rule) {
			if (preg_match("/pfB_/",$rule['source']['address']) || preg_match("/pfb_/",$rule['source']['address'])) {
				$pfb_table[$rule['source']['address']]['img'] = $in;
			}
			if (preg_match("/pfB_/",$rule['destination']['address']) || preg_match("/pfb_/",$rule['destination']['address'])) {
				$pfb_table[$rule['destination']['address']]['img'] = $in;
			}
		}
		return $pfb_table;
	}
}

// Status Indicator if pfBlockerNG is Enabled/Disabled
if ("{$pfb['enable']}" == "on") {
	$pfb_status = "/themes/{$g['theme']}/images/icons/icon_pass.gif";
	$pfb_msg = "pfBlockerNG is Active.";
} else {
	$pfb_status = "/themes/{$g['theme']}/images/icons/icon_block.gif";
	$pfb_msg = "pfBlockerNG is Disabled.";
}

// Collect Total IP/Cidr Counts
$dcount = exec("cat {$pfb['denydir']}/*.txt | grep -cv '^#\|^$\|^1\.1\.1\.1'");
$pcount = exec("cat {$pfb['permitdir']}/*.txt | grep -cv '^#\|^$\|^1\.1\.1\.1'");
$mcount = exec("cat {$pfb['matchdir']}/*.txt | grep -cv '^#\|^$\|^1\.1\.1\.1'");
$ncount = exec("cat {$pfb['nativedir']}/*.txt | grep -cv '^#\|^$\|^1\.1\.1\.1'");

// Collect Number of Suppressed Hosts
if (file_exists("{$pfb['supptxt']}")) {
	$pfbsupp_cnt = exec ("/usr/bin/grep -c ^ {$pfb['supptxt']}");
} else {
	$pfbsupp_cnt = 0;
}

#check rule count
#(label, evaluations,packets total, bytes total, packets in, bytes in,packets out, bytes out)
$packets = exec("/sbin/pfctl -s labels", $debug);
if (!empty($debug)) {
	foreach ($debug as $line) {
		// Auto-Rules start with 'pfB_', Alias Rules should start with 'pfb_' and exact spelling of Alias Name.
		$line = str_replace("pfb_","pfB_",$line);
		if ("{$pfb['pfsenseversion']}" >= '2.2') {
			#USER_RULE: pfB_Top auto rule 8494 17 900 17 900 0 0 0
			if (preg_match("/USER_RULE: (\w+).*\s+\d+\s+(\d+)\s+\d+\s+\d+\s+\d+\s+\d+\s+\d+\s+\d+/", $line, $matches)) {
				if (isset($matches)) {
					${$matches[1]}+=$matches[2];
				} else {
					${$matches[1]} = 'Err';
				}
			}
		} else {
			#USER_RULE: pfB_Top auto rule 1656 0 0 0 0 0 0
			if (preg_match("/USER_RULE: (\w+).*\s+\d+\s+(\d+)\s+\d+\s+\d+\s+\d+\s+\d+\s+\d+/", $line, $matches)) {
				if (isset($matches)) {
					${$matches[1]}+=$matches[2];
				} else {
					${$matches[1]} = 'Err';
				}
			}
		}
	}
}

// Called by Ajax to update alerts table contents
if (isset($_GET['getNewCounts'])) {
	$response = "";
	$pfb_table = pfBlockerNG_get_counts();
	if (!empty($pfb_table)) {
		foreach ($pfb_table as $alias => $values){
			if (!isset(${$alias})) { ${$alias} = "-";}
			$response .= $alias . "||" . $values['count'] . "||" . ${$alias} . "||" . $values['up'] . "||" . $values['img'] . "\n";
		}
		echo $response;
		return;
	}
}

// Report any Failed Downloads
$results = array();
$fails = exec("grep $(date +%m/%d/%y) {$pfb['errlog']} | grep 'FAIL'", $results);

// Print widget Status Bar Items
?>
	<div class="marinarea">
	<table border="0" cellspacing="0" cellpadding="0">
		<thead>
		<tr>
			<td valign="middle">&nbsp;<img src="<?= $pfb_status ?>" width="13" height="13" border="0" title="<?=gettext($pfb_msg) ?>" alt="" /></td>
			<td valign="middle">&nbsp;&nbsp;</td>
			<td valign="middle" p style="font-size:10px">
									<?php if ($dcount != 0): ?>
										<?=gettext("Deny:"); echo("&nbsp;<strong>" . $dcount . "</strong>") ?>
									<?php endif; ?>
									<?php if ($pcount != 0): ?>
										<?=gettext("&nbsp;Permit:"); echo("&nbsp;<strong>" . $pcount . "</strong>") ?>
									<?php endif; ?>
									<?php if ($mcount != 0): ?>
										<?=gettext("&nbsp;Match:"); echo("&nbsp;<strong>" . $mcount . "</strong>"); ?>
									<?php endif; ?>
									<?php if ($ncount != 0): ?>
										<?=gettext("&nbsp;Native:"); echo("&nbsp;<strong>" . $ncount . "</strong>"); ?>
									<?php endif; ?>
									<?php if ($pfbsupp_cnt != 0): ?>
										<?=gettext("&nbsp;Supp:"); echo("&nbsp;<strong>" . $pfbsupp_cnt . "</strong>"); ?>
									<?php endif; ?></td>
			<td valign="middle">&nbsp;&nbsp;</td>
			<td valign="top"><a href="pfblockerng/pfblockerng_log.php"><img src="/themes/<?=$g['theme']; ?>/images/icons/icon_logs.gif" width="13" height="13" border="0" title="<?=gettext("View pfBlockerNG Logs TAB") ?>" alt="" /></a>&nbsp;
			<td valign="top">
				<?php if (!empty($results)): ?>		<!--Hide "Ack" Button when Failed Downloads are Empty-->
					<form action="/widgets/widgets/pfblockerng.widget.php" method="post" name="widget_pfblockerng_ack">
						<input type="hidden" value="clearack" name="pfblockerngack" />
						<input class="vexpl" type="image" name="pfblockerng_ackbutton" src="/themes/<?=$g['theme']; ?>/images/icons/icon_x.gif" width="14" height="14" border="0" title="<?=gettext("Clear Failed Downloads") ?>"/>
					</form>
				<?php endif; ?>
			</td>
		</tr>
		</thead>
	</table>
	</div>

	<table id="pfb-tblfails" width="100%" border="0" cellspacing="0" cellpadding="0">
	<tbody id="pfb-fails">
<?php

if ("{$pfb['pfsenseversion']}" > '2.0') {
	$alertRowEvenClass = "listMReven";
	$alertRowOddClass = "listMRodd";
	$alertColClass = "listMRr";
} else {
	$alertRowEvenClass = "listr";
	$alertRowOddClass = "listr";
	$alertColClass = "listr";
}

# Last errors first
$results = array_reverse($results);

$counter = 0;
# Max errors to display
$maxfailcount = 3;
if (!empty($results)) {
	foreach ($results as $result) {
		$alertRowClass = $counter % 2 ? $alertRowEvenClass : $alertRowOddClass;
		if (!isset(${$alias})) { ${$alias} = "-";}
		echo(" <tr class='" . $alertRowClass . "'><td class='" . $alertColClass .  "'>" . $result . "</td><tr>");
		$counter++;
		if ($counter > $maxfailcount) {
			# To many errors stop displaying
			echo(" <tr class='" . $alertRowClass . "'><td class='" . $alertColClass .  "'>" . (count($results) - $maxfailcount) . " more error(s)...</td><tr>");
			break;
		}
	}
}

// Print Main Table Header
?>
	</tbody>
	</table>
	<table id="pfb-tbl" width="100%" border="0" cellspacing="0" cellpadding="0">
	<thead>
		<tr>
			<th class="widgetsubheader" align="center"><?=gettext("Alias");?></th>
			<th title="The count can be a mixture of Single IPs or CIDR values" class="widgetsubheader" align="center"><?=gettext("Count");?></th>
			<th title="Packet Counts can be cleared by the pfSense filter_configure() function. Make sure Rule Descriptions start with 'pfB_'" class="widgetsubheader" align="center"><?=gettext("Packets");?></th>
			<th title="Last Update (Date/Time) of the Alias " class="widgetsubheader" align="center"><?=gettext("Updated");?></th>
			<th class="widgetsubheader" align="center"><?php echo $out; ?><?php echo $in; ?></th>
		</tr>
	</thead>
	<tbody id="pfbNG-entries">
<?php
// Print Main Table Body
$pfb_table = pfBlockerNG_get_counts();
$counter=0;
if (is_array($pfb_table)) {
	foreach ($pfb_table as $alias => $values) {
		$evenRowClass = $counter % 2 ? " listMReven" : " listMRodd";
		if (!isset(${$alias})) { ${$alias} = "-";}
		echo("	<tr class='" . $evenRowClass . "'>
				<td class='listMRr ellipsis'>{$alias}</td>
				<td class='listMRr' align='center'>{$values['count']}</td>
				<td class='listMRr' align='center'>{${$alias}}</td>
				<td class='listMRr' align='center'>{$values['up']}</td>
				<td class='listMRr' align='center'>{$values['img']}</td>
			</tr>");
		$counter++;
	}
}

?>
</tbody>
</table>

<script type="text/javascript">
//<![CDATA[
	var pfBlockerNGupdateDelay = 10000; // update every 10000 ms
//]]>
</script>