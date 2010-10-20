<?php
/*
	/usr/local/www/ifbwstats_daily.php

	Contributed - 2010 - Zorac

	
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

	// determine which month to displayed

	if (isset($_SERVER['REQUEST_URI'])) 
	{
        $url = $_SERVER['REQUEST_URI'];
    } 
	else 
	{
        $url = $_SERVER['SCRIPT_NAME'];
		$url .= (!empty($_SERVER['QUERY_STRING'])) ? '?' . $_SERVER['QUERY_STRING'] : '';
    }

	$parsed_url = parse_url($url);
	if (isset($parsed_url['query']))
    {
		$queryparts = explode("&", $parsed_url['query']);
	}

require_once("guiconfig.inc");
require_once("ifbwstats.inc");

usr1_php_script();

$interfacename = str_replace('.data', '', $queryparts[0]);
$interfacename = str_replace('/tmp/ifbwstats-', '', $interfacename);
$interfacename = str_replace('/cf/conf/ifbwstats-', '', $interfacename);

$pgtitle = "ifBWStats: Archived Daily Statistics For ".$interfacename; 
include("head.inc");

echo '<body>';
include("fbegin.inc");
echo '<p class="pgtitle">ifBWStats: Daily Statistics For '.$interfacename.'</p>';
echo '<b>Month Ending: '.str_replace('%20', ' ', $queryparts[3]).'</b><br><br>';

//display tabs
echo '<table width="100%" border="0" cellpadding="0" cellspacing="0">';
echo '<tr><td class="tabnavtbl">';
$tab_array[0] = array ("Daily", false, "ifbwstats_cur.php");
$tab_array[1] = array ("Monthly", false, "ifbwstats_disp.php");
$tab_array[2] = array ("Settings", false, "pkg_edit.php?xml=ifbwstats.xml");
$tab_array[3] = array ("Archive", true, $url);
display_top_tabs($tab_array);
echo '</td></tr>';
echo '<tr><td>';
echo '<div id="mainarea">';

$foundfile = 'null';
if (file_exists($queryparts[0])) $foundfile = $queryparts[0];
if ($foundfile == 'null')
{
	echo 'Sorry, no data file found.';
}
else
{
	$fp = fopen($foundfile,"r") or die("Error Reading File");
	$data = fread($fp, filesize($foundfile));
	fclose($fp);
	$wandataall = explode("\n", $data);
	$n = count($wandataall);

	$monthintotal = 0;
	$monthouttotal = 0;
	$monthdaystart = 0;

	echo '<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">';
	echo '<tr>';
	echo '<td width="175px" class="listhdrr">Date</td>';
	echo '<td width="175px" class="listhdrr">Downloaded</b></font></td>';
	echo '<td width="175px" class="listhdrr">Uploaded</b></font></td>';
	echo '<td width="175px" class="listhdrr">Total Transfered</td>';
	echo '</tr>';

	for ($i=$queryparts[1] ; $i < $queryparts[2]+1 ; $i++ ) 
	{
		$dataset=explode("|", $wandataall[$i]);
		$dateset=explode("-", $dataset[0]);
		$monthintotal = $monthintotal + $dataset[1];
		$monthouttotal = $monthouttotal + $dataset[2];
		$total = round((($dataset[1]+$dataset[2])/1024/1024/1024),2);
		$dataset[1] = round(($dataset[1]/1024/1024),2);
		$dataset[2] = round(($dataset[2]/1024/1024),2);
		
		echo '<tr>';
		echo '<td width="175px" class="listlr">'.$dataset[0].'</td>';
		echo '<td width="175px" class="listlr">'.$dataset[1].'MB</td>';
		echo '<td width="175px" class="listlr">'.$dataset[2].'MB</td>';
		echo '<td width="175px" class="listlr">'.$total.'GB</td>';
		echo '</tr>';
	}

	$total = round((($monthintotal + $monthouttotal)/1024/1024/1024),2);
	$monthintotal = round(($monthintotal/1024/1024/1024),2);
	$monthouttotal = round(($monthouttotal/1024/1024/1024),2);

	echo '<tr>';
	echo '<td width="175px" class="listlr"><b>Month Totals</b></td>';
	echo '<td width="175px" class="listlr"><b>&#8595;'.$monthintotal.'GB</b></td>';
	echo '<td width="175px" class="listlr"><b>&#8593;'.$monthouttotal.'GB</b></td>';
	echo '<td width="175px" class="listlr"><b>&#8597;'.$total.'GB</b></td>';
	echo '</tr>';
	echo '</table>';
}

echo '</div>';
echo '</tr></td>';
echo '</table>';

include("fend.inc");
echo '</body>';
echo '</html>';

?>