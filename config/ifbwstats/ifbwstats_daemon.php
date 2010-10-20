<?php
/*
	/usr/local/www/ifbwstats_daemon.php

	Contributed - 2010 - Zorac

	interface read code using netstat as identifed below from 
	/usr/local/www/ifstats.php
	Copyright (C) 2005-2006 Scott Ullrich (sullrich@gmail.com)
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

	command to run
	/usr/local/bin/php -q /usr/local/www/ifbwstats_daemon.php &

	command to interupt
	kill -INT pid
	kill -INT `cat /var/run/ifbwstats.lock`

	command to force interface read without quitting daemon
	kill -USR1 pid
	kill -USR1 `cat /var/run/ifbwstats.lock`
*/

//required for SIGINT and SIGUSR1 
declare(ticks = 1);

//allow php to run as a daemon and not time out
set_time_limit(0);

require_once("config.inc");
define( 'LOCK_FILE', "/var/run/ifbwstats.lock" );

function isLocked()
{
	# If lock file exists, check if stale.  If exists and is not stale, return TRUE
	# Else, create lock file and return FALSE.

	if( file_exists( LOCK_FILE ) )
	{
		# check if it's stale
		$lockingPID = trim( file_get_contents( LOCK_FILE ) );

		# Get all active PIDs.
		$pids = explode( "\n", trim( `ps | awk '{print $1}'` ) );

		# If PID is still active, return true
		if( in_array( $lockingPID, $pids ) )  
		{
			return true;
		}

		# Lock-file is stale, so kill it.  Then move on to re-creating it.
		unlink( LOCK_FILE );
	}
	file_put_contents( LOCK_FILE, getmypid() . "\n" );
	return false;
} 

function sig_handler($signo)
{
	global $logfile;
	switch ($signo) 
	{
	case SIGTERM:
		if (isset($logfile)) fwrite($logfile, date('Y-m-d H:i:s')." SIGTERM ".getmypid()." \n");
		break;
	case SIGINT:
		global $_MYDAEMON_SHOULD_STOP;
		$_MYDAEMON_SHOULD_STOP = true;
		if (isset($logfile)) fwrite($logfile, date('Y-m-d H:i:s')." SIGINT ".getmypid()." \n");
		break;
	case SIGUSR1:
		global $_MYDAEMON_SHOULD_QUERY;
		$_MYDAEMON_SHOULD_QUERY = true;
		if (isset($logfile)) fwrite($logfile, date('Y-m-d H:i:s')." SIGUSR1 ".getmypid()." \n");
		break;
	}
}

function interface_query($if)
{
	global $config;
	global $g;
	
	//set data files for appropriate interface
	$wandatalastfile = '/tmp/ifbwstats-'.$if.'.last';
	$wandataallfile = '/tmp/ifbwstats-'.$if.'.data';
	$wandatabackupfile = '/cf/conf/ifbwstats-'.$if.'.data';
	
	//assume max is 4GB because of the 32bit counter wrap
	$maxbytesin = 4294967296;
	$maxbytesout = 4294967296;

	//create (or clear if already used)  variables
	$wandatacurrent = array();
	$wandatalast = array();
	$wandataall = array();

	//----------start modified code insert from ifstats.php----------
	$ifinfo = array();

	$ifinfo['hwif'] = $config['interfaces'][$if]['if'];
	if(!$ifinfo['hwif'])
	$ifinfo['hwif'] = $if;

	$ifinfo['if'] = $ifinfo['hwif'];	

	/* run netstat to determine link info */
	$linkinfo = "";
	unset($linkinfo);
	exec("/usr/bin/netstat -I " . $ifinfo['hwif'] . " -nWb -f link", $linkinfo);
	$linkinfo = preg_split("/\s+/", $linkinfo[1]);
	if (preg_match("/\*$/", $linkinfo[0])) {
		$ifinfo['status'] = "down";
	} else {
		$ifinfo['status'] = "up";
	}

	if(preg_match("/^enc|^tun/i", $ifinfo['if'])) {
		$ifinfo['inbytes'] = $linkinfo[5];
		$ifinfo['outbytes'] = $linkinfo[8];
	} else {
		$ifinfo['inbytes'] = $linkinfo[6];
		$ifinfo['outbytes'] = $linkinfo[9];
	}
	//----------end modified code insert from ifstats.php----------

	//check for errors
	if ((file_exists($wandatalastfile)) && ($ifinfo['inbytes'] == 0) && ($ifinfo['outbytes'] == 0)) 
	{
		$ifinfo['status'] = "down";
	}

	if (is_NaN($ifinfo['inbytes']) || is_NaN($outinfo['inbytes'])) 
	{
		$ifinfo['status'] = "down";
	}

	if ($ifinfo['status'] == "up")
	{
		$wandatacurrent[0] = $ifinfo['inbytes'];
		$wandatacurrent[1] = $ifinfo['outbytes'];

		if (file_exists($wandatalastfile))
		{
			//read last read file
			$wandatalast = explode("|", file_get_contents($wandatalastfile));
		}
		else 
		{
			$wandatalast = $wandatacurrent;
		}
		
		if (!is_numeric($wandatalast[0]) || is_NaN($wandatalast[1]))
		{
			$wandatalast = $wandatacurrent;
		}

		$fp = fopen($wandatalastfile,"w") or die("Error Reading File");
		fwrite($fp, $wandatacurrent[0].'|'.$wandatacurrent[1]);
		fclose($fp);

		//account for 4gig counter reset
		if ($wandatacurrent[0] < $wandatalast[0]) $inbytes = ((4294967296 - $wandatalast[0]) + $wandatacurrent[0]);
		else $inbytes = $wandatacurrent[0] - $wandatalast[0];
		if ($wandatacurrent[1] < $wandatalast[1]) $outbytes = ((4294967296 - $wandatalast[1]) + $wandatacurrent[1]);
		else $outbytes = $wandatacurrent[1] - $wandatalast[1];

		//check to make sure inbytes and outbytes are possible, if not, 0 both and erase last data as it may be corrupt
		if (($inbytes < 0) || ($inbytes > $maxbytesin) || ($outbytes < 0) || ($outbytes > $maxbytesout))
		{
			$inbytes = 0;
			$outbytes = 0;
			if (file_exists($wandatalastfile)) unlink ($wandatalastfile);
		}

		$foundfile = 'null';
		if (file_exists($wandataallfile)) $foundfile = $wandataallfile;
		else
		{
			if (file_exists($wandatabackupfile)) $foundfile = $wandatabackupfile;
		}

		//if no file is found, create new data, else read file and add to existing data
		if ($foundfile == 'null')
		{
			$wanwritedata = date("Y-m-d").'|'.$inbytes.'|'.$outbytes;
		}
		else
		{
			//read data file
			$wandataall = explode("\n", file_get_contents($foundfile));
			$n = count($wandataall);

			//if last line of data date matchs current date, add to totals, else add new line
			$dataset = explode("|", $wandataall[$n-1]);
			if ($dataset[0] == date("Y-m-d"))
			{
				$dataset[1]=$dataset[1]+$inbytes;
				$dataset[2]=$dataset[2]+$outbytes;
				$wandataall[$n-1]=$dataset[0].'|'.$dataset[1].'|'.$dataset[2];
			}
			else 
			{
				$wandataall[$n] = date("Y-m-d").'|'.$inbytes.'|'.$outbytes;
			}

			//number of data entries (days)
			$n = count($wandataall);

			//if more than three years worth of data, trim data to 4 years (1460 days)
			$start = 0;
			if ($n > 1460) $start = $n - 1460;

			//generate file data to write
			for ($i=$start ; $i < ($n-1) ; $i++ ) $wanwritedata = $wanwritedata.$wandataall[$i]."\n";
			$wanwritedata = $wanwritedata.$wandataall[$n-1];
		}

		//write data file
		$fp = fopen($wandataallfile,"w") or die("Error Reading File");
		fwrite($fp, $wanwritedata);
		fclose($fp);
	}
	else 
	{
		if (file_exists($wandatalastfile)) unlink ($wandatalastfile);
	}
}

pcntl_signal(SIGTERM, 'sig_handler');
pcntl_signal(SIGINT, 'sig_handler');
pcntl_signal(SIGUSR1, 'sig_handler');

global $config;
global $g;
global $_MYDAEMON_SHOULD_STOP;
global $_MYDAEMON_SHOULD_QUERY;

//logging -> yes or no
$logging = $config['installedpackages']['ifbwstats']['config'][0]['logging'];

if ($logging == 'yes')
{
	global $logfile;
	$logfile = fopen("/tmp/ifbwstats-daemon.log","a") or die("Error Reading File");
	$parentpid = getmypid();
	fwrite($logfile, date('Y-m-d H:i:s')." Startup - Parent PID is: ".$parentpid."\n");
}

if ($g['platform'] == 'cdrom') 
{
	fwrite($logfile, date('Y-m-d H:i:s')." Daemon will not run on CD Rom platform, exiting... \n");
	fclose ($logfile);
	exit ();
}
if (isLocked()) 
{
	if ($logging == 'yes') fwrite($logfile, date('Y-m-d H:i:s')." Daemon is already running, exiting second instance (".getmypid().") \n");
	fclose ($logfile);
	exit ();
}

// endless loop
while (1) 
{
	if (isset($_MYDAEMON_SHOULD_STOP) AND $_MYDAEMON_SHOULD_STOP) break;
	$pidA = pcntl_fork();
	if($pidA) 
	{
		// parent process runs here
		// wait until the child has finished processing then end the script
		pcntl_waitpid($pid, $status, WUNTRACED);
		//calc seconds to midnight
		$sleeptime = (mktime(23, 59, 55, date ('m, d, Y'))) - time();
		//use whichever is less, seconds to midnight or interval, running script just prior to midnight insure accurate daily reporting as standard timing interval could be as great at 50min
		if (($sleeptime > $config['installedpackages']['ifbwstats']['config'][0]['intervalrun']) || ($sleeptime < 5)) $sleeptime = $config['installedpackages']['ifbwstats']['config'][0]['intervalrun'];
		if ($logging == 'yes') fwrite($logfile, date('Y-m-d H:i:s')." Parent (".$parentpid.") Sleep for: ".$sleeptime."\n");
		for ($i=0; $i<$sleeptime; $i++)
		{
			if ((isset($_MYDAEMON_SHOULD_QUERY) AND $_MYDAEMON_SHOULD_QUERY) || (isset($_MYDAEMON_SHOULD_STOP) AND $_MYDAEMON_SHOULD_STOP))
			{
				$_MYDAEMON_SHOULD_QUERY = false;
				break;
			}
			else sleep (1);
		}
	}
	else 
	{
		//child process runs here
		if ($logging == 'yes') fwrite($logfile, date('Y-m-d H:i:s')." Child Process ". getmypid(). " Reading Interfaces... (Parent: ".$parentpid.")\n");
		if ($config['installedpackages']['ifbwstats']['config'][0]['ifmon'] != 'all') interface_query($config['installedpackages']['ifbwstats']['config'][0]['ifmon']);
		else
		{
			foreach ($config[interfaces] as $if => $value)
			{
				interface_query($if);
			}
		}
		exit (0);
	}
}

//run query one last time
if ($logging == 'yes') fwrite($logfile, date('Y-m-d H:i:s')." Parent Process ". getmypid(). " Reading Interfaces One Last Time... \n");
if ($config['installedpackages']['ifbwstats']['config'][0]['ifmon'] != 'all') interface_query($config['installedpackages']['ifbwstats']['config'][0]['ifmon']);
else
{
	foreach ($config[interfaces] as $if => $value)
	{
		interface_query($if);
	}
}
// backup data files to conf dir on exit
if ($g['platform'] != 'pfSense') exec ('/etc/rc.conf_mount_rw');
exec('cp /tmp/ifbwstats-*.data /cf/conf/');
if ($g['platform'] != 'pfSense') exec ('/etc/rc.conf_mount_ro');

if ($logging == 'yes') 
{
	fwrite($logfile, date('Y-m-d H:i:s')." Shutdown Parent ".$parentpid." \n");
	fclose ($logfile);
}

if( file_exists( LOCK_FILE ) ) unlink( LOCK_FILE );

exit (0);

?>

