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
function open_table(){
	echo "<table style=\"padding-top:0px; padding-bottom:0px; padding-left:0px; padding-right:0px\" width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">";
	echo"  <tr>";
}
function close_table(){
	echo"  </tr>";
	echo"</table>";
	echo "<br>";
}

$pfb_table=array();
$img['Sick']="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_down.gif'>";
$img['Healthy']="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_up.gif'>";


#var_dump($pfb_table);
#exit;
?><div id='varnish'><?php 
open_table();

if ($config['installedpackages']['varnishsettings']['config'][0])
	$mgm=$config['installedpackages']['varnishsettings']['config'][0]['managment'];
if ($mgm != ""){
	print "<pre>";
	print "<td class=\"vncellt\"width=30%><strong>Cache hits</strong></td>";
	print "<td class=\"vncellt\"width=30%><strong>Cache hits pass</strong></td>";	
	print "<td class=\"vncellt\"width=30%><strong>Cache Missed</strong></td></tr>";
	
	$backends=exec("varnishadm -T " . escapeshellarg($mgm) . " stats",$debug);	
	foreach ($debug as $line){
		if (preg_match("/(\d+)\s+Cache\s+(hits.for|hits|misses)/",$line,$matches))
				$cache[preg_replace("/\s+/","",$matches[2])]=$matches[1];
		if (preg_match("/(\d+)\s+Client\s+(\w+)/",$line,$matches))
				$client[$matches[2]]=$matches[1];	
		}
	print "<td class=\"listlr\">".$cache['hits'] ."</td>";
	print "<td class=\"listlr\">".$cache['hitsfor'] ."</td>";
	print "<td class=\"listlr\">".$cache['misses']."</td></tr>";	
 	close_table();
 	
 	open_table();
	print "<td class=\"vncellt\" width=30%><strong>Conn. Accepted</strong></td>";	
	print "<td class=\"vncellt\" width=30%><strong>Req. received</strong></td>";
	print "<td class=\"vncellt\" width=30%><strong>Uptime</strong></td></tr>";
	print "<td class=\"listlr\">".$client['connections'] ."</td>";
	print "<td class=\"listlr\">".$client['requests'] ."</td>";
	print "<td class=\"listlr\">".$client['uptime']."</td></tr>";
 	close_table();
 	
 	open_table();
	print "<td class=\"vncellt\" width=30%><strong>Backend</strong></td>";
	print "<td class=\"vncellt\" width=30%><strong>LB applied</strong></td>";
	print "<td class=\"vncellt\" width=30%><strong>Status</strong></td></tr>";
	if (is_array($config['installedpackages']['varnishlbdirectors']['config']))
		foreach($config['installedpackages']['varnishlbdirectors']['config'] as $lb){
			foreach ($lb['row'] as $lb_backend){
				${$lb_backend['backendname']}++;
				}
			}	
	$backends=exec("varnishadm -T " . escapeshellarg($mgm) . " debug.health",$debug);	
	foreach ($debug as $line){
		if (preg_match("/Backend (.*) is (\w+)/",$line,$matches)){
			$backend=preg_replace("/BACKEND$/","",$matches[1]);
			print "<td class=\"listlr\">". $backend ."</td>";
			print "<td class=\"listlr\">". ${$backend} ."</td>";
			print "<td class=\"listlr\">".$img[$matches[2]]."</td></tr>";	
			}
		}
	}
else{
	print "<td class=\"listlr\">Varnish Managment interface not set in config.</td></tr>";
}	
echo"  </tr>";
echo"</table></div>";

?>
<script type="text/javascript">
	function getstatus_varnish() {
		var url = "/widgets/widgets/varnish.widget.php";
		var pars = 'getupdatestatus=yes';
		var myAjax = new Ajax.Request(
			url,
			{
				method: 'get',
				parameters: pars,
				onComplete: activitycallback_varnish
			});
		//I know it's ugly but works.
		setTimeout('getstatus_varnish()', 10000);
		}
	function activitycallback_varnish(transport) {
		$('varnish').innerHTML = transport.responseText;
	}
	getstatus_varnish();
</script>
