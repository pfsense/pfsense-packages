<?php
/*
	/usr/local/pkg/ifbwstats.inc

	Contributed - 2010 - Zorac
	
	cron restart code as identifed below from 
	/usr/local/pkg/cron.inc
	Copyright (C) 2008 Mark J Crane
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

//require_once("/usr/local/www/guiconfig.inc");

define( 'LOCK_FILE', "/var/run/ifbwstats.lock" );

function ifbwstats_add_cron()
{
	global $config;

	$i = count($config['cron']['item']);
	$cif = -1; //cron interval entry
	for ($n=0; $n<$i; $n++)
	{
		//find cron interval run
		if ((strstr($config['cron']['item'][$n]['hour'], '*/3')) && (strstr($config['cron']['item'][$n]['command'], 'ifbwstats_daemon.php')))
		{
			$cif = $n;
		}
	}

	//daily restart
	if ($cif < 0) //if entry not found, add it
	{
		$config['cron']['item'][$i]['minute'] = '0';
		$config['cron']['item'][$i]['hour'] = '1';
		$config['cron']['item'][$i]['mday'] = '*';
		$config['cron']['item'][$i]['month'] = '*';
		$config['cron']['item'][$i]['wday'] = '*';
		$config['cron']['item'][$i]['who'] = 'root';
		$config['cron']['item'][$i]['command'] = 'kill -INT `cat /var/run/ifbwstats.lock` ; sleep 5 ; /usr/local/etc/rc.d/ifbwstats.sh start';
		$i++;
	}

	//restart cron so changes are running
	//----------start insert from /usr/local/pkg/cron.inc----------
	configure_cron();
	$handle = popen("/usr/local/etc/rc.d/cron.sh stop", "r");
	pclose($handle);
	$handle = popen("/usr/local/etc/rc.d/cron.sh start", "r");
	pclose($handle);
	//----------end insert from /usr/local/pkg/cron.inc----------

	write_config();
}

function ifbwstats_remove_cron()
{
	global $config;

	//find and erase cron entries
	$i = count($config['cron']['item']);
	for ($n=0; $n<$i; $n++)
	{
		//find cron interval run
		if ((strstr($config['cron']['item'][$n]['hour'], '1')) && (strstr($config['cron']['item'][$n]['command'], 'ifbwstats'))) unset ($config['cron']['item'][$n]);
	}

	//restart cron so changes are running
	//----------start insert from /usr/local/pkg/cron.inc----------
	configure_cron();
	$handle = popen("/usr/local/etc/rc.d/cron.sh stop", "r");
	pclose($handle);
	$handle = popen("/usr/local/etc/rc.d/cron.sh start", "r");
	pclose($handle);
	//----------end insert from /usr/local/pkg/cron.inc----------

	write_config();
}

function ifbwstats_set_var()
{
	global $config;

	$config['installedpackages']['ifbwstats']['config'][0]['firstday'] = 1;
	$config['installedpackages']['ifbwstats']['config'][0]['intervalrun'] = 280;
	$config['installedpackages']['ifbwstats']['config'][0]['logging'] = 'no';
	$config['installedpackages']['ifbwstats']['config'][0]['ifmon'] = 'wan';
	write_config();
}

function ifbwstats_remove_var()
{
	global $config;

	unset ($config['installedpackages']['ifbwstats']);
	write_config();
}

function start_php_script()
{
	$handle = popen("/usr/local/etc/rc.d/ifbwstats.sh start", "r");
	sleep (1);
	pclose($handle);
}

function usr1_php_script()
{
	if( file_exists( LOCK_FILE ) )
	{
		//force interface read via daemon
		$handle = popen("kill -USR1 `cat /var/run/ifbwstats.lock`", "r");
		//while (!feof($handle)) sleep (1);
		sleep (1);
		pclose($handle);
	}
}

function kill_php_script()
{
	//if lock file exists, process must be running, therefore kill it and remove lock file
	if( file_exists( LOCK_FILE ) )
	{
		$handle = popen("kill -INT `cat /var/run/ifbwstats.lock`", "r");
		//while (!feof($handle)) sleep (1);
		sleep (1);
		pclose($handle);
	}
}
/*
function file_put_contents($filename, $data, $file_append = false) 
{
	$fp = fopen($filename, (!$file_append ? 'w+' : 'a+'));
		if(!$fp) 
		{
			trigger_error('file_put_contents cannot write in file.', E_USER_ERROR);
			return;
		}
	fputs($fp, $data);
	fclose($fp);
}
*/

function ifbwstats_add_rc()
{
	//add shutdown entry
	$fp = fopen('/etc/rc.shutdown',"r") or die("Error Reading File");
	$data = fread($fp, filesize('/etc/rc.shutdown'));
	fclose($fp);
	$fpdata = explode("\n", $data);
	$fpwrite = '';
	$i = count($fpdata);
	$fndentry = -1; //found shutdown entry
	for ($n=0; $n<$i; $n++) if (strstr($fpdata[$n], '# ifBWStats:')) $fndentry = $n;
	if ($fndentry < 0)
	{
		for ($n=0; $n<$i; $n++) if (strstr($fpdata[$n], 'HOME=/; export HOME')) $fndentry = $n-2;
		for ($n=0; $n<$fndentry+1; $n++) $fpwrite .= $fpdata[$n]."\n";
		$fpwrite .= "\n".'# ifBWStats: shut down script'."\n".'kill -INT `cat /var/run/ifbwstats.lock`'."\n".'sleep 1'."\n".'# ifBWStats: remove last reading as some platforms may not erase tmp files'."\n".'rm /tmp/ifbwstats-*.last'."\n";
		for ($n=$fndentry+1; $n<$i-1; $n++) $fpwrite .= $fpdata[$n]."\n";
		$fpwrite .= $fpdata[$i-1];
		$fp = fopen('/etc/rc.shutdown',"w") or die("Error Reading File");
		fwrite($fp, $fpwrite);
		fclose($fp);
	}

	//add reboot entry
	$fp = fopen('/etc/rc.reboot',"r") or die("Error Reading File");
	$data = fread($fp, filesize('/etc/rc.reboot'));
	fclose($fp);
	$fpdata = explode("\n", $data);
	$fpwrite = '';
	$i = count($fpdata);
	$fndentry = -1; //found reboot entry
	for ($n=0; $n<$i; $n++) if (strstr($fpdata[$n], '# ifBWStats:')) $fndentry = $n;
	if ($fndentry < 0)
	{
		for ($n=0; $n<$i; $n++) if (strstr($fpdata[$n], '# If PLATFORM is pfSense then remove')) $fndentry = $n-2;
		for ($n=0; $n<$fndentry+1; $n++) $fpwrite .= $fpdata[$n]."\n";
		$fpwrite .= "\n".'# ifBWStats: shut down script'."\n".'kill -INT `cat /var/run/ifbwstats.lock`'."\n".'sleep 1'."\n".'# ifBWStats: remove last reading as some platforms may not erase tmp files'."\n".'rm /tmp/ifbwstats-*.last'."\n";
		for ($n=$fndentry+1; $n<$i-1; $n++) $fpwrite .= $fpdata[$n]."\n";
		$fpwrite .= $fpdata[$i-1];
		$fp = fopen('/etc/rc.reboot',"w") or die("Error Reading File");
		fwrite($fp, $fpwrite);
		fclose($fp);
	}

	//add halt entry
	$fp = fopen('/etc/rc.halt',"r") or die("Error Reading File");
	$data = fread($fp, filesize('/etc/rc.halt'));
	fclose($fp);
	$fpdata = explode("\n", $data);
	$fpwrite = '';
	$i = count($fpdata);
	$fndentry = -1; //found halt entry
	for ($n=0; $n<$i; $n++) if (strstr($fpdata[$n], '# ifBWStats:')) $fndentry = $n;
	if ($fndentry < 0)
	{
		for ($n=0; $n<$i; $n++) if (strstr($fpdata[$n], 'sleep 1')) $fndentry = $n-2;
		for ($n=0; $n<$fndentry+1; $n++) $fpwrite .= $fpdata[$n]."\n";
		$fpwrite .= "\n".'# ifBWStats: shut down script'."\n".'kill -INT `cat /var/run/ifbwstats.lock`'."\n".'sleep 1'."\n".'# ifBWStats: remove last reading as some platforms may not erase tmp files'."\n".'rm /tmp/ifbwstats-*.last'."\n";
		for ($n=$fndentry+1; $n<$i-1; $n++) $fpwrite .= $fpdata[$n]."\n";
		$fpwrite .= $fpdata[$i-1];
		$fp = fopen('/etc/rc.halt',"w") or die("Error Reading File");
		fwrite($fp, $fpwrite);
		fclose($fp);
	}
}

function ifbwstats_remove_rc()
{
	//remove shutdown entry
	$fp = fopen('/etc/rc.shutdown',"r") or die("Error Reading File");
	$data = fread($fp, filesize('/etc/rc.shutdown'));
	fclose($fp);
	$fpdata = explode("\n", $data);
	$fpwrite = '';
	$i = count($fpdata);
	$fndentry = -1; //found shutdown entry
	for ($n=0; $n<$i; $n++) if ((strstr($fpdata[$n], '# ifBWStats:'))&&($fndentry < 0)) $fndentry = $n;
	if ($fndentry >= 0)
	{
		for ($n=0; $n<$fndentry; $n++) $fpwrite .= $fpdata[$n]."\n";
		for ($n=$fndentry+6; $n<$i-1; $n++) $fpwrite .= $fpdata[$n]."\n";
		$fpwrite .= $fpdata[$i-1];
		$fp = fopen('/etc/rc.shutdown',"w") or die("Error Reading File");
		fwrite($fp, $fpwrite);
		fclose($fp);
	}

	//remove reboot entry
	$fp = fopen('/etc/rc.reboot',"r") or die("Error Reading File");
	$data = fread($fp, filesize('/etc/rc.reboot'));
	fclose($fp);
	$fpdata = explode("\n", $data);
	$fpwrite = '';
	$i = count($fpdata);
	$fndentry = -1; //found reboot entry
	for ($n=0; $n<$i; $n++) if ((strstr($fpdata[$n], '# ifBWStats:'))&&($fndentry < 0)) $fndentry = $n;
	if ($fndentry >= 0)
	{
		for ($n=0; $n<$fndentry; $n++) $fpwrite .= $fpdata[$n]."\n";
		for ($n=$fndentry+6; $n<$i-1; $n++) $fpwrite .= $fpdata[$n]."\n";
		$fpwrite .= $fpdata[$i-1];
		$fp = fopen('/etc/rc.reboot',"w") or die("Error Reading File");
		fwrite($fp, $fpwrite);
		fclose($fp);
	}

	//remove halt entry
	$fp = fopen('/etc/rc.halt',"r") or die("Error Reading File");
	$data = fread($fp, filesize('/etc/rc.halt'));
	fclose($fp);
	$fpdata = explode("\n", $data);
	$fpwrite = '';
	$i = count($fpdata);
	$fndentry = -1; //found halt entry
	for ($n=0; $n<$i; $n++) if ((strstr($fpdata[$n], '# ifBWStats:'))&&($fndentry < 0)) $fndentry = $n;
	if ($fndentry >= 0)
	{
		for ($n=0; $n<$fndentry; $n++) $fpwrite .= $fpdata[$n]."\n";
		for ($n=$fndentry+6; $n<$i-1; $n++) $fpwrite .= $fpdata[$n]."\n";
		$fpwrite .= $fpdata[$i-1];
		$fp = fopen('/etc/rc.halt',"w") or die("Error Reading File");
		fwrite($fp, $fpwrite);
		fclose($fp);
	}
}

function cleanup_data_file ($readfile, $writefile)
{
	//----------------------------------------maintain file incase of errors in the file----------------------------------------
	$wandataall = array();
	$wandataarray = array();

	//read data file
	$fp = fopen($readfile,"r") or die("Error Reading File");
	$data = fread($fp, filesize($readfile));
	fclose($fp);
	$wandataall = explode("\n", $data);
	$n = count($wandataall);

	//expode data into two dim. array
	for ($x=0; $x<$n; $x++)
	{
		$line = explode('|',$wandataall[$x]);

		// save it by coulmns otherwise it will saved like rows
		for ($i=0; $i<=2; $i++) 
		{
			if ($i==0) $wandataarray[$x][$i] = date("Y-m-d", strtotime($line[$i]));
			else $wandataarray[$x][$i] = $line[$i];
		}
	}

	//sort array into chronological order
	sort ($wandataarray);

	$m=$n;
	//check for multiple entries, and if so add them
	for ($x=0; $x<$n; $x++)
	{
		if (($wandataarray[$x][0] == $wandataarray[$x+1][0]) && ($x+1<$m))
		{
			$wandataarray[$x][1] = $wandataarray[$x][1] + $wandataarray[$x+1][1];
			$wandataarray[$x][2] = $wandataarray[$x][2] + $wandataarray[$x+1][2];
			for ($i = $x+1; $i<$n; $i++)
			{
				if (!is_null($wandataarray[$i+1][0])) $wandataarray[$i][0]=$wandataarray[$i+1][0];
				if (!is_null($wandataarray[$i+1][1])) $wandataarray[$i][1]=$wandataarray[$i+1][1];
				if (!is_null($wandataarray[$i+1][2])) $wandataarray[$i][2]=$wandataarray[$i+1][2];
			}
			$x--;
			$m--;
		}
	}

	//check for missing entries
	$nextday=$wandataarray[0][0];
	$x=0;
	while ($x<$m)
	{
		//missing entry between days
		if ($nextday != $wandataarray[$x][0])
		{
			$m++;
			for ($i=$m; $i>$x; $i--)
			{
				
				$wandataarray[$i][0] = $wandataarray[$i-1][0];
				$wandataarray[$i][1] = $wandataarray[$i-1][1];
				$wandataarray[$i][2] = $wandataarray[$i-1][2];
				$wandataarray[$i][3] = $wandataarray[$i-1][3];
				$wandataarray[$i][4] = $wandataarray[$i-1][4];
			}
			//fill in 0 bytes transfered for missing days
			$wandataarray[$x][0] = $nextday;
			$wandataarray[$x][1] = 0;
			$wandataarray[$x][2] = 0;
		}
		$x++;
		
		//add one day to the date
		$nextday = date("Y-m-d", strtotime(date("Y-m-d", strtotime($nextday)) . " +1 day"));
		
		//echo 'x: '.$x.' m: '.$m.' array date: '.strtotime($wandataarray[$x-1][0]).' current date: '.strtotime(date("Y-m-d")).'<br>';
		//add missing entries at end of file if required
		if (($x==$m)&&(strtotime($wandataarray[$x-1][0] ) < strtotime(date("Y-m-d"))))
		{
			$wandataarray[$x][0] = $nextday;
			$wandataarray[$x][1] = 0;
			$wandataarray[$x][2] = 0;
			$m++;
		}
	}

	//put array data into variable for wirting
	$writedata = '';
	for ($x=0; $x<$m-1; $x++) $writedata .= $wandataarray[$x][0].'|'.$wandataarray[$x][1].'|'.$wandataarray[$x][2]."\n";
	$writedata .= $wandataarray[$m-1][0].'|'.$wandataarray[$m-1][1].'|'.$wandataarray[$m-1][2];

	//write data file
	$fp = fopen($writefile,"w") or die("Error Reading File");
	fwrite($fp, $writedata);
	fclose($fp);
	//----------------------------------------end of file maintaince----------------------------------------
}


?>