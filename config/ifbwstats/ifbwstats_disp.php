<?php
/*
	/usr/local/www/ifbwstats_disp.php

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

require_once("guiconfig.inc");
require_once("ifbwstats.inc");
$pgtitle = "ifBWStats: Monthly Statistics";
include("head.inc");

usr1_php_script();

//set first day of billing period from config, if hasn't been configed, assume the 1st
if(!$config['installedpackages']['ifbwstats']['config'][0]['firstday']) $firstday = 1;
else $firstday = $config['installedpackages']['ifbwstats']['config'][0]['firstday'];

echo '<body>';
include("fbegin.inc");
echo '<p class="pgtitle">ifBWStats: Monthly Statistics</p>';

//find all valid data files for active and inactive interfaces
//assume monitoring all interfaces
$datafilestores = array();
$n=0;

//if only monitoring one inteface
if ($config['installedpackages']['ifbwstats']['config'][0]['ifmon'] != 'all') 
{
	//dont check conf directory, as if only one interface is being monitored, it must be used and therefore in the tmp dir
	if (file_exists('/tmp/ifbwstats-'.$config['installedpackages']['ifbwstats']['config'][0]['ifmon'].'.data'))
	{
		$datafilestores[$n] = '/tmp/ifbwstats-'.$config['installedpackages']['ifbwstats']['config'][0]['ifmon'].'.data';
		cleanup_data_file ($datafilestores[$n], $datafilestores[$n]);
		$n++;
	}
}
else
{
	if ($handle = opendir('/tmp')) 
	{
	    while (false !== ($file = readdir($handle))) 
		{
			if ((preg_match ("/ifbwstats/", $file))&&(preg_match ("/.data/", $file)))
			{
				$datafilestores[$n] = '/tmp/'.$file;
				cleanup_data_file ($datafilestores[$n], $datafilestores[$n]);
				$n++;
			}
	    }
	}

	if ($handle = opendir('/cf/conf')) 
	{
	    while (false !== ($file = readdir($handle))) 
		{
	        $filefound = 'no';
			if ((preg_match ("/ifbwstats-/", $file))&&(preg_match ("/.data/", $file)))
			{
				for ($i = 0; $i < $n; $i++) if (preg_match ("/$file/", $datafilestores[$i])) $filefound = 'yes';
				if ($filefound == 'no')
				{
					cleanup_data_file ('/cf/conf/'.$file, '/tmp/'.$file);
					$datafilestores[$n] = '/tmp/'.$file;
					$n++;
				}
			}
	    }
	}
}

//display tabs
echo '<table width="100%" border="0" cellpadding="0" cellspacing="0">';
echo '<tr><td class="tabnavtbl">';
$tab_array[0] = array ("Daily", false, "ifbwstats_cur.php");
$tab_array[1] = array ("Monthly", true, "ifbwstats_disp.php");
$tab_array[2] = array ("Settings", false, "pkg_edit.php?xml=ifbwstats.xml");
display_top_tabs($tab_array);
echo '</td></tr>';
echo '<tr><td>';

//cycle through all valid data files found
foreach ($datafilestores as $wandataallfile)
{
//----------------------------------------begin file statistics monthly display----------------------------------------
	//read data file
	$fp = fopen($wandataallfile,"r") or die("Error Reading File");
	$data = fread($fp, filesize($wandataallfile));
	fclose($fp);
	$wandataall = explode("\n", $data);
	$n = count($wandataall);

	$monthintotal = 0;
	$monthouttotal = 0;
	$monthdaystart = 0;
	
	$interfacename = str_replace('.data', '', $wandataallfile);
	$interfacename = str_replace('/tmp/ifbwstats-', '', $interfacename);
	$interfacename = str_replace('/cf/conf/ifbwstats-', '', $interfacename);
		
	echo '<div id="mainarea">';
	echo '<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">';
	echo '<tr><td><font size=+1><b>'.$interfacename.' Statistics Summary</b></font></<td></tr>';
	echo '<tr><td>';
	echo '<table width="700px" border="0" cellpadding="0" cellspacing="0">';
	echo '<tr>';
	echo '<td width="175px" class="listhdrr">Month Ending</td>';
	echo '<td width="175px" class="listhdrr">Downloaded</b></td>';
	echo '<td width="175px" class="listhdrr">Uploaded</b></td>';
	echo '<td width="175px" class="listhdrr">Total Transfered</td>';
	echo '</tr>';
	echo '</table>';
	echo '</td></tr><tr><td>';
	echo '<div>';
	//echo '<div style="overflow: auto; height: 400px"; width: 620px>';
	echo '<table width="700px" border="0" cellpadding="0" cellspacing="0">';

	for ($i=0 ; $i < $n ; $i++ ) 
	{
		$dataset=explode("|", $wandataall[$i]);
		$dateset=explode("-", $dataset[0]);
		$monthintotal = $monthintotal + $dataset[1];
		$monthouttotal = $monthouttotal + $dataset[2];
		$total = round((($dataset[1]+$dataset[2])/1024/1024/1024),2);
		$dataset[1] = round(($dataset[1]/1024/1024),2);
		$dataset[2] = round(($dataset[2]/1024/1024),2);
		
		//show daily stats
		/*echo '<tr>';
		echo '<td width="175px" class="listlr">'.$dataset[0].'</td>';
		echo '<td width="175px" class="listlr">'.$dataset[1].'MB</td>';
		echo '<td width="175px" class="listlr">'.$dataset[2].'MB</td>';
		echo '<td width="175px" class="listlr">'.$total.'GB</td>';
		echo '</tr>';*/
		
		//as data file saved by day, determin if a monthy total is required to be shown based on setting 'first day of billing period'
		$showmonth = '0';
		
		//if at the end of the billing cycle show month totals, 1 to indicate that a full billing period has passed and to show a 'month end' total
		if ($dateset[2] == ($firstday-1)) $showmonth = '1';
		//if there is no more data, show month totals, 2 to show 'current month'
		if (($n-1) == $i) $showmonth = '2';
		//if the billing cycle starts on the first day of the month, figure out the last day of the previous month, and if appropriate show month totals
		if ($firstday == '1')
		{
			//find the last day of the month
			$maxday = date("t", strtotime($dateset[0]."-".$dateset[1]."-".$dateset[2]));
			if ($dateset[2]==$maxday) $showmonth = '1';
		}

		if (($showmonth == '1')||($showmonth == '2'))
		{
			$total = round((($monthintotal + $monthouttotal)/1024/1024/1024),2);
			$monthintotal = round(($monthintotal/1024/1024/1024),2);
			$monthouttotal = round(($monthouttotal/1024/1024/1024),2);
			if ($showmonth == '2') $dataset[0]='Current%20Month';

			echo '<tr>';
			echo '<td width="175px" class="listlr"><a href="http://'.$_SERVER['SERVER_NAME'].'/ifbwstats_daily.php?'.$wandataallfile.'&'.$monthdaystart.'&'.$i.'&'.$dataset[0].'">'.str_replace('%20', ' ', $dataset[0]).'</a></td>';
			echo '<td width="175px" class="listlr">&#8595;'.$monthintotal.'GB</td>';
			echo '<td width="175px" class="listlr">&#8593;'.$monthouttotal.'GB</td>';
			echo '<td width="175px" class="listlr">&#8597;'.$total.'GB</td>';
			echo '</tr>';
			$monthdaystart = $i + 1;
			$monthintotal = 0;
			$monthouttotal = 0;
		}
	}
	echo '</table>';
	echo '</div>';
	echo '</td></tr></table>';
	echo '</div>';
	unset ($writedata);
//----------------------------------------end file statistics monthly display----------------------------------------
}
//end foreach loop
echo '</tr></td>';
echo '</table>';

include("fend.inc");
echo '</body>';
echo '</html>';
?>