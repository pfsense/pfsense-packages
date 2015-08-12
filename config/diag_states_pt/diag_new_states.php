<?php
/*
	diag_new_states.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2002 Paul Taylor
	Copyright (C) 2015 ESF, LLC
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
global $config;

function displayIP($ip, $col) {

	global $viewPassThru;

	switch ($col) {
		case 'srcip':
			if ($_GET['sfilter']) {
				if ($_GET['sfilter'] == $ip)
					return $ip;
			}
			else {
				return '<a href="?sfilter='.$ip.$viewPassThru.'">'. $ip .'</a>';
			}
			break;

		case 'dstip':
			if ($_GET['dfilter']) {
				if ($_GET['dfilter'] == $ip) {
					return $ip;
				}
			} else {
				return '<a href="?dfilter='.$ip.$viewPassThru.'">'. $ip .'</a>';
			}
			break;
	}

}

function sortOrder($column) {

	if ($_GET['order'] == $column) {
		if ($_GET['sort'] == 'des') {
			return "&amp;sort=asc";
		}
		return "&amp;sort=des";
	} else {
		return "&amp;sort=asc";
	}
}

// FIXME: Needs changes to handle IPv6 addresses properly
function stripPort($ip, $showPort = false) {
	if (!$showPort) {
		if (strpos($ip, ':') > 0) {
			return substr($ip, 0, strpos($ip, ":"));
		} else {
			return ($ip);
		}
	} else {
		if (strpos($ip, ':') > 0) {
			return substr($ip, (strpos($ip, ":")+1));
		} else {
			return "&nbsp;";
		}
	}
}

// sfilter and dfilter allow setting of source and dest IP filters
// on the output. $filterPassThru allows these source and dest
// filters to be passed on in the column sorting links.
if (($_GET['sfilter']) or ($_GET['dfilter'])) {

	$filter = '';
	if ($_GET['sfilter']) {
		if (is_ipaddr($_GET['sfilter'])) {
			$sfilter = $_GET['sfilter'];
			$filterPassThru = '&amp;sfilter=' . $_GET['sfilter'];
		} else {
			unset ($_GET['sfilter']);
		}
	}
	if ($_GET['dfilter']) {
		if (is_ipaddr($_GET['dfilter'])) {
			$dfilter = $_GET['dfilter'];
			$filterPassThru = '&amp;dfilter=' . $_GET['dfilter'];
		} else {
			unset ($_GET['dfilter']);
		}
	}
}

$dataRows = 300;

$rawdata = array();

/* get our states */
//         1         2         3         4         5         6         7         8
//12345678901234567890123456789012345678901234567890123456789012345678901234567890
//    [3] => PR    D SRC                   DEST                 STATE   AGE   EXP  PKTS BYTES
//    [4] => icmp  O 192.168.112.94:16734  192.168.112.1:0       0:0     12     2    10   840
//    [5] => tcp   I 192.168.111.99:61221  192.168.111.150:22    4:4    710 86399   726  242K
// -w 132 sets width of data to 132
// $dataRows defaults to 300 for embedded hardware
exec("echo q | /usr/local/sbin/pftop -w 132 $dataRows", $rawdata);

// exporting TERM set to nothing gets you a "dumb" term. echo q to pftop makes it
// quit out after displaying the first page of data.

// Get top line with total state data
$topDataLine = $rawdata[2];
//pfTop: Up State 1-5/5, View: default, Order: none
$slashPos = strpos($topDataLine, '/') + 1;
$commaPos = strpos($topDataLine, ',');
if (($slashPos > 1) and ($commaPos > 1)) {
	$totalStates = substr($topDataLine, $slashPos, ($commaPos - $slashPos));
} else {
	$totalStates = 0;
}

// Get rid of the header data
unset($rawdata[0], $rawdata[1], $rawdata[2], $rawdata[3]);

if (isset($rawdata)) {
	$count = 0;
	foreach ($rawdata as $line) {
		if (!strlen(trim($line)) < 70) {
//PR        DIR SRC                          DEST                                 STATE                AGE       EXP     PKTS    BYTES
//tcp       Out 192.168.111.99:62831         66.84.12.81:110              FIN_WAIT_2:FIN_WAIT_2   00:01:20  00:00:11       28     1933
//  0        1            2                          3                               4                  5         6         7        8
			$split = preg_split("/\s+/", trim($line));

			$data[$count]['protocol'] = $split[0];
			$data[$count]['dir'] =  strtolower($split[1]);
			$srcTmp = $split[2];
			$data[$count]['srcip'] = stripPort($srcTmp);
			$data[$count]['srcport'] = stripPort($srcTmp, true);
			$dstTmp = $split[3];
			$data[$count]['expire'] = $split[6];
			$data[$count]['dstip'] = stripPort($dstTmp);
			$data[$count]['dstport'] = stripPort($dstTmp, true);
			$data[$count]['packets'] = $split[7];
			$data[$count]['bytes'] = $split[8];
			$count++;
		}
	}
	// Clear the statistics snapshot files, which track the packets and bytes of connections
	if (isset($_GET['clear'])) {
		if (file_exists('/tmp/packets')) {
			unlink('/tmp/packets');
		}
		if (file_exists('/tmp/bytes')) {
			unlink('/tmp/bytes');
		}

		// Redirect so we don't hit "clear" every time we refresh the screen.
		header("Location: diag_new_states.php?".$filterPassThru);
		exit;
	}

	// Create a new set of stats snapshot files
	if (isset($_GET['new'])) {
		$packets = array();
		$bytes = array();

		// Create variables to let us later quickly access this data
		if (is_array($data)) {
			foreach ($data as $row) {
				$packets[$row['srcip']][$row['srcport']][$row['dstip']][$row['dstport']][$row['protocol']] = $row['packets'];
				$bytes[$row['srcip']][$row['srcport']][$row['dstip']][$row['dstport']][$row['protocol']] = $row['bytes'];
			}
		}

		// Write the files out
		writeStats("packets", $packets);
		writeStats("bytes", $bytes);

		// If we're in view mode, pass that on.
		if (isset($_GET['view'])) {
			$filterPassThru .= "&amp;view=1";
		}

		// Redirect so we don't hit "new" every time we refresh the screen.
		header("Location: diag_new_states.php?&amp;order=bytes&amp;sort=des".$filterPassThru);
		exit;
	}

	// View the delta from the last snapshot against the current data.
	if (isset($_GET['view'])) {

		// Read the stats data files
		readStats("packets", $packets);
		readStats("bytes", $bytes);

		if (is_array($data)) {
			foreach ($data as $key => $row) {
				if (isset($packets[$row['srcip']][$row['srcport']][$row['dstip']][$row['dstport']][$row['protocol']])) {
					if (isset($bytes[$row['srcip']][$row['srcport']][$row['dstip']][$row['dstport']][$row['protocol']])) {
						$tempPackets = $data[$key]['packets'] - $packets[$row['srcip']][$row['srcport']][$row['dstip']][$row['dstport']][$row['protocol']];
						$tempBytes = $data[$key]['bytes'] - $bytes[$row['srcip']][$row['srcport']][$row['dstip']][$row['dstport']][$row['protocol']];
						if (($tempPackets > -1) && ($tempBytes > -1)) {
							$data[$key]['packets'] = $tempPackets;
							$data[$key]['bytes'] = $tempBytes;
						}
					}
				}

			}
		}

		$filterPassThru .= "&amp;view=1";
		$viewPassThru = "&amp;view=1";
	}

	// Sort it by the selected order
	if ($_GET['order']) {
		natsort2d($data, $_GET['order']);
		if ($_GET['sort']) {
			if ($_GET['sort'] == "des") {
				$data = array_reverse($data);
			}
		}
	}
}

function natsort2d( &$arrIn, $index = null ) {

	$arrTemp = array();
	$arrOut = array();

	if (is_array($arrIn)) {
		foreach ( $arrIn as $key=>$value ) {
			reset($value);
			$arrTemp[$key] = is_null($index) ? current($value) : $value[$index];
		}
	}

	natsort($arrTemp);

	foreach ( $arrTemp as $key=>$value ) {
		$arrOut[$key] = $arrIn[$key];
	}

	$arrIn = $arrOut;

}

function writeStats($fname, &$data) {
	$fname = "/tmp/" . $fname;
	if (file_exists($fname)) {
		unlink($fname);
	}
	$file = fopen($fname, 'a');
	fwrite($file, serialize($data));
	fclose($file);
}

function readStats($fname, &$data) {
	$fname = "/tmp/" . $fname;
	if (file_exists($fname)) {
		$file = fopen($fname,'r');
		$data = unserialize(fread($file, filesize($fname)));
		fclose($file);
	}
}

// Get timestamp of snapshot file, if it exists, for display later.
if (!(file_exists('/tmp/packets'))) {
	$lastSnapshot = "Never";
} else {
	$lastSnapshot = strftime("%m/%d/%y %H:%M:%S",filectime('/tmp/packets'));
}
// The next include must be here because we use redirection above
$pgtitle = "Diagnostics: Show States";
include("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
<script src="/javascript/scriptaculous/scriptaculous.js" type="text/javascript"></script>
<script src="/javascript/sorttable.js" type="text/javascript"></script>

<?php include("fbegin.inc"); ?>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td class="listhdrr" colspan="9">Statistics snapshot control</td>
	</tr>
	<tr>
	<?php if (($lastSnapshot != 'Never') && (!isset($_GET['view']))) :?>
		<td class="listlr"><a href="?view=1&amp;order=bytes&amp;sort=des<?=$filterPassThru;?>">View delta</a></td>
		<td class="listr"><a href="?new=1<?=$filterPassThru;?>">Start new</a></td>
		<td class="listr"><a href="?clear=1">Clear snapshot</a></td>
		<td class="listr" colspan="6" align="right">Last statistics snapshot: <?=$lastSnapshot;?></td>
	<?php endif; ?>
	<?php if (($lastSnapshot != 'Never') && (isset($_GET['view']))) :?>
		<td class="listlr"><a href="?new=1<?=$filterPassThru;?>">Start new</a></td>
		<td class="listr"><a href="?clear=1">Clear</a></td>
		<td class="listr" colspan="7" align="right"><span class="red">Viewing delta of statistics snapshot: <?=$lastSnapshot;?></span></td>
	<?php endif; ?>
	<?php if ($lastSnapshot == 'Never') :?>
		<td class="listlr"><a href="?new=1<?=$filterPassThru;?>">Start new</a></td>
		<td class="listr" colspan="8" align="right">Last statistics snapshot: <?=$lastSnapshot;?></td>
	<?php endif; ?>
	</tr>
	<tr>
		<td colspan="8">&nbsp;</td>
	</tr>
	<tr>
		<td class="listhdrr"><a href="?order=srcip<?=sortOrder('srcip');echo $filterPassThru;?>">Source</a></td>
		<td class="listhdrr"><a href="?order=srcport<?=sortOrder('srcport');echo $filterPassThru;?>">Port</a></td>
		<td class="listhdrr"><a href="?order=dir<?=sortOrder('dir');echo $filterPassThru;?>">Dir</a></td>
		<td class="listhdrr"><a href="?order=dstip<?=sortOrder('dstip');echo $filterPassThru;?>">Destination</a></td>
		<td class="listhdrr"><a href="?order=dstport<?=sortOrder('dstport');echo $filterPassThru;?>">Port</a></td>
		<td class="listhdrr"><a href="?order=protocol<?=sortOrder('protocol');echo $filterPassThru;?>">Protocol</a></td>
		<td class="listhdrr" align="right"><a href="?order=packets<?=sortOrder('packets');echo $filterPassThru;?>">Packets</a></td>
		<td class="listhdrr" align="right"><a href="?order=bytes<?=sortOrder('bytes');echo $filterPassThru;?>">Bytes</a></td>
		<td class="listhdr" align="right"><a href="?order=expire<?=sortOrder('expire');echo $filterPassThru;?>">Expires</a></td>
		<td class="list"></td>
	</tr>
	<?php
		$count = 0;
		if (is_array($data)) {
			foreach ($data as $entry) {
				if ((!isset($sfilter) && (!isset($dfilter))) ||
				    ((isset($sfilter)) && ($entry['srcip'] == $sfilter)) ||
				    ((isset($dfilter)) && ($entry['dstip'] == $dfilter))) {
		?>
	<tr>
		<td class="listlr"><?=displayIP($entry['srcip'],'srcip');?></td>
		<td class="listr"><?=$entry['srcport'];?></td>
		<td class="listr"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_<?=$entry['dir'];?>.gif" width="11" height="11" style="margin-top: 2px" alt="" /></td>
		<td class="listr"><?=displayIP($entry['dstip'],'dstip');?></td>
		<td class="listr"><?=$entry['dstport'];?></td>
		<td class="listr"><?=$entry['protocol'];?></td>
		<td class="listr" align="right"><?=$entry['packets'];?></td>
		<td class="listr" align="right"><?=$entry['bytes'];?></td>
		<td class="listr" align="right"><?=$entry['expire'];?></td>
	</tr>
	<?php
				$count++;
				}
			}
		}
	?>
</table>
<br /><strong>Firewall connection states displayed: <?=$count;?>/<?=$totalStates;?></strong>
<?php if ($filterPassThru): ?>
<div>
	<form action="diag_new_states.php" method="get">
		<input type="hidden" name="order" value="bytes" />
		<input type="hidden" name="sort" value="des" /><br />
		<input type="submit" class="formbtn" value="Unfilter View" />
	</form>
</div>
<?php endif; ?>
<?php include("fend.inc"); ?>
</body>
</html>
