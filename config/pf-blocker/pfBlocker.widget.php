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
@require_once("guiconfig.inc");
@require_once("pfsense-utils.inc");
@require_once("functions.inc");

echo "<table style=\"padding-top:0px; padding-bottom:0px; padding-left:0px; padding-right:0px\" width=\"100%\" border=\"0\" cellpadding=\"0\" 
cellspacing=\"0\"";
echo"  <tr>";

$in="";
$out="";
$white="";
$rules=$config['filter']['rule'];
#echo "<pre>";
foreach($rules as $rule){
	if ($rule['destination']['address'] == 'pfBlockerOutbound' && $out == ""){
		#print_r($rule);
		$out="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_up.gif'>";
	}
		
	if ($rule['source']['address']== 'pfBlockerInbound' && $in == "")
		$in="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_up.gif'>";
		
	if ($rule['source']['address']== 'pfBlockerWL' && $white == "")
		$white="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_up.gif'>";
		
	if ($rule['destination']['address']== 'pfBlockerWL' && $white == "")
		$white="<img src ='/themes/{$g['theme']}/images/icons/icon_interface_up.gif'>";
}

$in=($in != ""?$in:"<img src ='/themes/{$g['theme']}/images/icons/icon_interface_down.gif'>");
$out=($out != ""?$out:"<img src ='/themes/{$g['theme']}/images/icons/icon_interface_down.gif'>");
$white=($white != ""?$white:"<img src ='/themes/{$g['theme']}/images/icons/icon_interface_down.gif'>");

echo "    <td class=\"listhdrr\">pfBlockerInbound".$in."</td>";
echo "    <td class=\"listhdrr\">pfBlockerOutbound".$out."</td>";
echo "    <td class=\"listhdrr\">pfBlockerWL".$white."</td>";

echo"  </tr>";
echo"  <tr>";
if (file_exists("/usr/local/pkg/pfb_in.txt")) {
         $resultsIP = preg_match_all("/\//",file_get_contents("/usr/local/pkg/pfb_in.txt"),$matches);
         echo "    <td class=\"listlr\">". count($matches[0])." Networks</td>";
}
if (file_exists("/usr/local/pkg/pfb_out.txt")) {
         $resultsIP = preg_match_all("/\//",file_get_contents("/usr/local/pkg/pfb_out.txt"),$matches);
         echo "    <td class=\"listlr\">" . count($matches[0])." Networks</td>";
}
if (file_exists("/usr/local/pkg/pfb_w.txt")) {
         $resultsIP = preg_match_all("/\//",file_get_contents("/usr/local/pkg/pfb_w.txt"),$matches);
         echo "    <td class=\"listlr\">" . count($matches[0])." Networks</td>";}

echo"  </tr>";
echo"</table>";
?>