<?php 
/*
        Copyright 2013 Marcello Coutinho
        Part of bind package for pfSense(www.pfsense.org)

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

$uname=posix_uname();
if ($uname['machine']=='amd64')
        ini_set('memory_limit', '250M');
        
function open_table(){
	echo "<table style=\"padding-top:0px; padding-bottom:0px; padding-left:0px; padding-right:0px\" width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">";
	echo"  <tr>";
}
function close_table(){
	echo"  </tr>";
	echo"</table>";
	
}

$pfb_table=array();
$img['Sick']="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_down.gif'>";
$img['Healthy']="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_up.gif'>";


#var_dump($pfb_table);
#exit;
?><div id='bind'><?php 
global $config;
$rndc_bin="/usr/local/sbin/rndc";

if (file_exists($rndc_bin))
	exec("$rndc_bin status",$status);

open_table();
foreach($status as $line){
	$fields=explode(":",$line);
	print "<tr><td class=\"vncellt\"width=50%><strong>".ucfirst($fields[0])."</strong></td>\n";
	print "<td class=\"listlr\">{$fields[1]}</td>\n</tr>";
	}
close_table();
echo"</div>";

?>
<script type="text/javascript">
	function getstatus_bind() {
		var url = "/widgets/widgets/bind.widget.php";
		var pars = 'getupdatestatus=yes';
		var myAjax = new Ajax.Request(
			url,
			{
				method: 'get',
				parameters: pars,
				onComplete: activitycallback_bind
			});
		}
	function activitycallback_bind(transport) {
		$('bind').innerHTML = transport.responseText;
		setTimeout('getstatus_postfix()', 5000);
	}
	getstatus_bind();
</script>
