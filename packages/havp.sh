#!/bin/sh
# HAVP Init script
# 6/23/06 - Gary Buckmaster

pidfile=/var/run/havp/havp.pid
required_dirs=/var/tmp/havp
required_files=/usr/local/etc/havp/havp.config


rc_start()
{
	if [ ! -f $required_files ]
	then
		echo "FATAL: Missing HAVP config file: $required_files"
		return
	fi
	if [ ! -d $required_dirs ]
	then
		echo "FATAL: Missing HAVP working director: $required_dirs"
		return
	fi
	if [ -f $pidfile ]
	then
		pid=$(sed 's/ //g' $pidfile)
		echo "FATAL: HAVP already running? pid: $pid"	
		return
	else
		echo "Starting HAVP Antivirus HTTP Proxy"
		/usr/local/sbin/havp &
		sleep 4 
		if [ -f $pidfile ]
		then
			pid=$(sed 's/ //g' $pidfile)
			echo "Started pid: $pid"
		else
			echo "An error occurred starting HAVP"
			return
		fi
	fi
}	

rc_stop()
{
	pid=$(sed 's/ //g' $pidfile)
	if [ ! -f $pidfile ]
	then
		echo "FATAL: HAVP already running pid: $pid"
		return
	else
		echo "Stopping HAVP pid: $pid"
		kill $pid
	fi
}

case $1 in
	start)
		rc_start
		;;
	stop)
		rc_stop
		;;
	restart) 
		rc_stop 
		sleep 5
		rc_start
		;;
esac

