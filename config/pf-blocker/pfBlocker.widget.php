<?php 
/*
        Copyright 2011 Thomas Schaefer - Tomschaefer.org
        Copyright 2011 Marcello Coutinho
        Part of pfSense widgets (www.pfsense.org)

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
@require_once("guiconfig.inc");
@require_once("pfsense-utils.inc");
@require_once("functions.inc");
?><div id='pfBlocker'><?php 
echo "<table style=\"padding-top:0px; padding-bottom:0px; padding-left:0px; padding-right:0px\" width=\"100%\" border=\"0\" cellpadding=\"0\" 
cellspacing=\"0\"";
echo"  <tr>";

$pfb_table=array();
$out="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_down.gif'>";
$in="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_up.gif'>";
if (is_array($config['aliases']['alias']))
foreach ($config['aliases']['alias'] as $cbalias){
	if (preg_match("/pfBlocker/",$cbalias['name'])){
	
		if (file_exists('/var/db/aliastables/'.$cbalias['name'].'.txt')){
				preg_match("/(\d+)/",exec("/usr/bin/wc -l /var/db/aliastables/".$cbalias['name'].".txt"),$matches);
				$pfb_table[$cbalias['name']]=array("count" => $matches[1],
													"img"=> $out);
				}
		}
	}

#check rule count
#(label, evaluations,packets total, bytes total, packets in, bytes in,packets out, bytes out)
$packets=exec("/sbin/pfctl -s labels",$debug);
foreach ($debug as $line){
		#USER_RULE: pfBlocker Outbound rule 1656 0 0 0 0 0 0
		if (preg_match("/USER_RULE: (\w+).*\s+\d+\s+(\d+)\s+\d+\s+\d+\s+\d+\s+\d+\s+\d+/",$line,$matches))
			${$matches[1]}+=$matches[2];
}
	
$rules=$config['filter']['rule'];
#echo "<pre>";
foreach($rules as $rule){
	if (preg_match("/pfBlocker/",$rule['source']['address']))
		$pfb_table[$rule['source']['address']]["img"]=$in;
		
	if (preg_match("/pfBlocker/",$rule['destination']['address']))
		$pfb_table[$rule['destination']['address']]["img"]=$in;
}
print "<pre>";
#var_dump($pfb_table);
#exit;
	print "<td class=\"listlr\"><strong>Alias</strong></td>";
	print "<td class=\"listlr\"><strong>CIDRs</strong></td>";
	print "<td class=\"listlr\"><strong>Packets</strong></td>";
	print "<td class=\"listlr\"><strong>Status</strong></td></tr>";	
foreach ($pfb_table as $alias => $values){
	print "<td class=\"listlr\">".$alias ."</td>";
	print "<td class=\"listlr\">".$values["count"]."</td>";
	print "<td class=\"listlr\">".${$alias}."</td>";
	print "<td class=\"listlr\">".$values["img"]."</td></tr>";	
}
echo"  </tr>";
echo"</table></div>";
?>
<script type="text/javascript">
	function getstatus_pfblocker() {
		var url = "/widgets/widgets/pfBlocker.widget.php";
		var pars = 'getupdatestatus=yes';
		var myAjax = new Ajax.Request(
			url,
			{
				method: 'get',
				parameters: pars,
				onComplete: activitycallback_pfblocker
			});
		//I know it's ugly but works.
		setTimeout('getstatus_pfblocker()', 10000);
		}
	function activitycallback_pfblocker(transport) {
		$('pfBlocker').innerHTML = transport.responseText;
	}
	getstatus_pfblocker();
</script>
