<?php

exec("/sbin/pfctl -s state", $states);

$srcipinfo = array();

$row = 0;
if(count($states) > 0) {
	foreach($states as $line) {
		$line_split = preg_split("/\s+/", $line);
		$type  = array_shift($line_split);
		$proto = array_shift($line_split);
		$state = array_pop($line_split);
		$info  = implode(" ", $line_split);

		/* break up info and extract $srcip and $dstip */
		$ends = preg_split("/\<?-\>?/", $info);
		$parts = split(":", $ends[0]);
		$srcip = trim($parts[0]);
		$srcport = trim($parts[1]);

		$parts = split(":", $ends[count($ends) - 1]);
		$dstip = trim($parts[0]);
		$dstport = trim($parts[1]);
		
		$srcipinfo[$srcip]['seen']++;
		$srcipinfo[$srcip]['protos'][$proto]['seen']++;
		if (!empty($srcport)) {
			$srcipinfo[$srcip]['protos'][$proto]['srcports'][$srcport]++;
		}
		if (!empty($dstport)) {
			$srcipinfo[$srcip]['protos'][$proto]['dstports'][$dstport]++;
		}
	}
}

function sort_by_ip($a, $b) {
	return sprintf("%u", ip2long($a)) < sprintf("%u", ip2long($b)) ? -1 : 1;
}

$pgtitle = "Diagnostics: State Table Summary";
require_once("guiconfig.inc");
include("head.inc");
include("fbegin.inc");
?>
<p class="pgtitle"><?=$pgtitle?></font></p>

<table align="center" width="80%">
	<tr>
		<th>IP</th>
		<th># States</th>
		<th>Proto</th>
		<th># States</th>
		<th>Src Ports</th>
		<th>Dst Ports</th>
	</tr>
<?php   uksort($srcipinfo, "sort_by_ip");
	foreach($srcipinfo as $ip => $ipinfo) { ?>
	<tr>
		<td><?php echo $ip; ?></td>
		<td align="center"><?php echo $ipinfo['seen']; ?></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
	</tr>
	<?php foreach($ipinfo['protos'] as $proto => $protoinfo) { ?>
	<tr>
		<td>&nbsp;</td>
		<td>&nbsp;</td>
		<td><?php echo $proto; ?></td>
		<td align="center"><?php echo $protoinfo['seen']; ?></td>
		<td align="center"><?php echo count($protoinfo['srcports']); ?></td>
		<td align="center"><?php echo count($protoinfo['dstports']); ?></td>
	</tr>
	<?php } ?>
<?php } ?>

</table>

<?php include("fend.inc"); ?>
