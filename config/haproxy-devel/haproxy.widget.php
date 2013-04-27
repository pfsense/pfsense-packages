<?php 
/*
        Copyright 2011 Thomas Schaefer - Tomschaefer.org
        Copyright 2011 Marcello Coutinho
        Part of pfSense widgets (www.pfsense.com)

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
/*
	Some mods made from pfBlocker widget to make this for HAProxy on Pfsense
	Copyleft 2012 by jvorhees
*/
require_once("guiconfig.inc");
require_once("pfsense-utils.inc");
require_once("functions.inc");
#Retrieve parameters
require_once("haproxy_socketinfo.inc");
require_once("/usr/local/www/widgets/include/haproxy.widget.inc");
?><div id='HAProxy'><?php

#Backends/Servers Actions if asked
if(!empty($_GET['act']) and !empty($_GET['be']) and !empty($_GET['srv'])) {
	$backend = $_GET['be'];
	$server =  $_GET['srv'];
	$enable = $_GET['act'] == 'start' ? true : false;
	haproxy_set_server_enabled($backend, $server, $enable);
}

$out="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_down.gif'>";
$in="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_up.gif'>";
$running="<img src ='/themes/{$g['theme']}/images/icons/icon_pass.gif'>";
$stopped="<img src ='/themes/{$g['theme']}/images/icons/icon_block.gif'>";
$log="<img src ='/themes/{$g['theme']}/images/icons/icon_log.gif'>";
$start="<img src ='/themes/{$g['theme']}/images/icons/icon_service_start.gif' title='Enable this backend/server'>";
$stop="<img src ='/themes/{$g['theme']}/images/icons/icon_service_stop.gif' title='Disable this backend/server'>";

$clients=array();
$clientstraffic=array();

$statistics = haproxy_get_statistics();
$frontends = $statistics['frontends'];
$backends = $statistics['backends'];
$servers = $statistics['servers'];

if ($show_clients == "YES") {
	$clients = haproxy_get_clients();
}

echo "<table style=\"padding-top:0px; padding-bottom:0px; padding-left:0px; padding-right:0px\" width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"";
#Frontends
if ($show_frontends == "YES") {
	print "<tr><td class=\"widgetsubheader\" colspan=\"4\"><strong>FrontEnd(s)</strong></td></tr>";
		print "<tr><td class=\"listlr\"><strong>Name</strong></td>";
		print "<td class=\"listlr\"><strong>Sessions</strong><br>(cur/max)</td>";
		print "<td class=\"listlr\" colspan=\"2\"><strong><center>Status</center></strong></td></tr>"; 

	foreach ($frontends as $fe => $fedata){
		print "<tr><td class=\"listlr\">".$fedata['pxname']."</td>";
		print "<td class=\"listlr\">".$fedata['scur']." / ".$fedata['slim']."</td>";
		if ($fedata['status'] == "OPEN") {
			$fedata['status'] = $running." ".$fedata['status'];
		} else {
			$fedata['status'] = $stopped." ".$fedata['status'];
		}
		print "<td class=\"listlr\" colspan=\"2\"><center>".$fedata['status']."</center></td></tr>";      
	}

	print "<tr height=\"6\"><td colspan=\"4\"></td></tr>";
}

#Backends/Servers w/o clients
print "<tr><td class=\"widgetsubheader\" colspan=\"4\"><strong>Backend(s)/Server(s)</strong></td></tr>";
print "<tr><td class=\"listlr\"><strong>Backend(s)</strong><br>&nbsp;Server(s)";
if ($show_clients == "YES") {
	print "<br>&nbsp;&nbsp;<font color=\"blue\"><i>Client(s) addr:port</i></font>";
}
print "</td>";
print "<td class=\"listlr\"><strong>Sessions</strong><br>(cur/max)<br>";
if ($show_clients == "YES" and $show_clients_traffic != "YES") {
	print "<font color=\"blue\">age/id</font>";
} elseif ($show_clients == "YES" and $show_clients_traffic == "YES") {
	print "<font color=\"blue\">age/traffic i/o</font>";
}
print "</td>";
print "<td class=\"listlr\" colspan=\"2\"><strong><center>Status<br>/<br>Actions</center></strong></td>";

foreach ($backends as $be => $bedata) {
	if ($bedata['status'] == "UP") {
		$statusicon = $in;
		$besess = $bedata['scur']." / ".$bedata['slim'];
		$bename = $bedata['pxname'];
	} else {
		$statusicon = $out;
		$besess = "<strong><font color=\"red\">".$bedata['status']."</font></strong>";
		$bename = "<font color=\"red\">".$bedata['pxname']."</font>";
	}
	$icondetails = " onmouseover=\"this.title='".$bedata['status']."'\"";
	print "<tr height=\"4\"><td bgcolor=\"#B1B1B1\" colspan=\"4\"></td></tr>";
        print "<tr><td class=\"listlr\"><strong>".$bename."</strong></td>";
        print "<td class=\"listlr\">".$besess."</td>";
        print "<td class=\"listlr\"$icondetails><center>".$statusicon."</center></td>";
	print "<td class=\"listlr\">&nbsp;</td></tr>";

	foreach ($servers as $srv => $srvdata) {
		if ($srvdata['pxname'] == $bedata['pxname']) {
			if ($srvdata['status'] == "UP") {
				$nextaction = "stop";
				$statusicon = $in;
				$acticon = $stop;
				$srvname = $srvdata['svname'];
			} elseif ($srvdata['status'] == "no check") {
				$nextaction = "stop";
				$statusicon = $in;
				$acticon = $stop;
				$srvname = $srvdata['svname'];
				$srvdata['scur'] = "<font color=\"blue\">no check</font>";
			} elseif ($srvdata['status'] == "MAINT") {
				$nextaction = "start";
				$statusicon = $out;
				$acticon = $start;
				$srvname = "<font color=\"blue\">".$srvdata['svname']."</font>";
				$srvdata['scur'] = "<font color=\"blue\">".$srvdata['status']."</font>";
			} else {
				$nextaction = "stop";
				$statusicon = $out;
				$acticon = $stop;
				$srvname = "<font color=\"red\">".$srvdata['svname']."</font>";
				$srvdata['scur'] = "<font color=\"red\">".$srvdata['status']."</font>";
			}
			$icondetails = " onmouseover=\"this.title='".$srvdata['status']."'\"";
			print "<tr><td class=\"listlr\">&nbsp;".$srvname."</td>";
			print "<td class=\"listlr\">".$srvdata['scur']."</td>";
			print "<td class=\"listlr\"$icondetails><center>".$statusicon."</center></td>";
			print "<td class=\"listlr\"><center><a  onclick=\"control_haproxy('".$nextaction."','".$bedata['pxname']."','".$srvdata['svname']."');\">".$acticon."</a></center></td></tr>";

			if ($show_clients == "YES") {
				foreach ($clients as $cli => $clidata) {
					if ($clidata['be'] == $bedata['pxname'] && $clidata['srv'] == $srvdata['svname']) {
						print "<tr><td class=\"listlr\">&nbsp;&nbsp;<font color=\"blue\"><i>".$clidata['src']."</i></font>&nbsp;<a href=\"diag_dns.php?host=".$clidata['srcip']."\" title=\"Reverse Resolve with DNS\">".$log."</a></td>";
						if ($show_clients_traffic == "YES") {
							$clisessoutput=exec("/bin/echo 'show sess ".$clidata['sessid']."' | /usr/local/bin/socat unix-connect:/tmp/haproxy.socket stdio | /usr/bin/grep 'total=' | /usr/bin/awk '{print $5}'",$clidebug);
							$i=0;
							foreach($clidebug as $cliline) {
								$clibytes = explode("=", $cliline);
								if ($clibytes[1] >= 1024 and $clibytes[1] <= 1048576) { 
									$clibytes = (int)($clibytes[1]/1024)."Kb";
								} elseif ($clibytes[1] >= 1048576 and $clibytes[1] <= 10485760) {
									$clibytes = round(($clibytes[1]/1048576),2)."Mb";
								} elseif ($clibytes[1] >= 10485760) {
									$clibytes = round(($clibytes[1]/1048576),1)."Mb";
								} else {
									$clibytes = $clibytes[1]."B";
								}
								$clientstraffic[$i] = $clibytes;
								$i++;
							}
							$clidebug="";
							print "<td class=\"listlr\" colspan=\"3\"><font color=\"blue\">".$clidata['age']." / ".$clientstraffic[0]." / ".$clientstraffic[1]."</font></td></tr>";
						} else {
							print "<td class=\"listlr\" colspan=\"3\"><font color=\"blue\">".$clidata['age']." / ".$clidata['sessid']."</font></td></tr>";
						}
					}
				}
			}
		}
	}
}

echo"</table></div>";
?>
<script type="text/javascript">
	function getstatusgetupdate() {
		var url = "/widgets/widgets/haproxy.widget.php";
		var pars = 'getupdatestatus=yes';
		var myAjax = new Ajax.Request(
			url,
			{
					method: 'get',
					parameters: pars,
					onComplete: activitycallback_haproxy
			});
	}
	function getstatus_haproxy() {
		getstatusgetupdate();
		setTimeout('getstatus_haproxy()', <?= $refresh_rate ?>);
	}
	function activitycallback_haproxy(transport) {
			$('HAProxy').innerHTML = transport.responseText;
	}
	getstatus_haproxy();
</script>
<script type="text/javascript">
        function control_haproxy(act,be,srv) {
                var url = "/widgets/widgets/haproxy.widget.php";
                var pars = 'act='+act+'&be='+be+'&srv='+srv;
                var myAjax = new Ajax.Request(
                        url,
                        {
                                method: 'get',
                                parameters: pars,
                                //onComplete: activitycallback_haproxy
								onComplete: getstatusgetupdate
                        });
        }
</script>
