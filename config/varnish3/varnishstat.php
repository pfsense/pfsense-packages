<?php
/*
	varnishstat.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2006 Scott Ullrich <sullrich@gmail.com>
	Copyright (C) 2014 Marcello Coutinho
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

require("guiconfig.inc");
require("varnish.inc");
function open_table(){
	echo "<table style=\"padding-top:0px; padding-bottom:0px; padding-left:0px; padding-right:0px\" width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">";
	echo"  <tr>";
}
function close_table(){
	echo"  </tr>";
	echo"</table>";
	echo "<br>";
}
if($_REQUEST['getactivity']) {
	//$varnishstatlogs = `varnishstat -1`;
	$backends=exec("varnishstat -1",$debug);
	echo "<h2>VarnishSTAT Server logs as of " . date("D M j G:i:s T Y")  . "</h2>";
	open_table();
	print "<td class=\"vncellt\" width=30%><strong>Description</strong></td>";
	print "<td class=\"vncellt\" width=15%><strong>Count</strong></td>";
	print "<td class=\"vncellt\" width=15%><strong></strong></td>";
	print "<td width=20%></td><td width=20%></td></tr>";
	foreach ($debug as $line){
		if (!preg_match("/^VBE/",$line,$lm) &&
        	preg_match("/(\S+)\s+(\S+)\s+(\S+)\s+(.*)/",$line,$lm))
           $varnish_stats[]=$lm;
	}
	unset($debug);
	if (is_array($varnish_stats)){
		foreach ($varnish_stats as $v){
   	      print "<td class=\"listlr\"> $v[4] ($v[1])</td>";
   	      print "<td class=\"listlr\" align=\"Right\">". @number_format($v[2]) ."</td>";
          print "<td class=\"listlr\" align=\"Right\">$v[3]</td>";
          print "<td>&nbsp;</td><td>&nbsp;</td></tr>";
		}
	}
	else{
		print "<td class=\"listlr\">No traffic</td><td class=\"listlr\"></td><td class=\"listlr\"></td></tr>";
	}

close_table();
	//echo $varnishstatlogs;
	exit;
}

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

$pgtitle = "Varnish: VarnishSTAT";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
	<script type="text/javascript">
		function getlogactivity() {
			var url = "/varnishstat.php";
			var pars = 'getactivity=yes';
			var myAjax = new Ajax.Request(
				url,
				{
					method: 'post',
					parameters: pars,
					onComplete: activitycallback
				});
		}
		function activitycallback(transport) {
			$('varnishstatlogs').innerHTML = '<font face="Courier"><pre>' + transport.responseText  + '</pre></font>';
			setTimeout('getlogactivity()', 2500);
		}
		setTimeout('getlogactivity()', 1000);
	</script>
<?php include("fbegin.inc"); ?>

<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<?php endif; ?>

<?php varnish_check_config();if ($savemsg) print_info_box($savemsg); ?>

<div id="mainlevel">
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Backends"), false, "/pkg.php?xml=varnish_backends.xml");
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=varnish_settings.xml&id=0");
	$tab_array[] = array(gettext("Custom VCL"), false, "/pkg_edit.php?xml=varnish_custom_vcl.xml&id=0");
	$tab_array[] = array(gettext("LB Directors"), false, "/pkg.php?xml=varnish_lb_directors.xml");
	$tab_array[] = array(gettext("XMLRPC Sync"), false, "/pkg_edit.php?xml=varnish_sync.xml&id=0");
	$tab_array[] = array(gettext("View Configuration"), false, "/varnish_view_config.php");
	$tab_array[] = array(gettext("VarnishSTAT"), true, "/varnishstat.php");
	display_top_tabs($tab_array);
?>
		</td></tr>
 		<tr>
    		<td>
				<div id="mainarea">
					<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
						<tr>
     						<td class="tabcont" >
      							<form action="varnishstat.php" method="post">
								<div id="varnishstatlogs">
									<pre>One moment please, loading VarnishSTAT...</pre>
								</div>
     						</td>
						</tr>
					</table>
				</div>
			</td>
		</tr>
	</table>
</div>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
