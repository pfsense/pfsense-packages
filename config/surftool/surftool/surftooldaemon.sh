#!/bin/sh


DAEMON_EXECUTABLE=$0
INIFILE=$2

surftool_read_configfile(){
	#check logfile
	if ! [ -f "$INIFILE" ]; then
		echo "Error surftooldaemon inifile '$INIFILE' does not exist "
		exit 1
	fi
	
	
	WORKER_EXECUTABLE=$(awk -F "=" '/worker_executable/ {print $2}' $INIFILE | tr -d ' '  | tr -d '"')
	REFRESHTIME=$(awk -F "=" '/refresh_time/ {print $2}' $INIFILE | tr -d ' ' | tr -d '"')
	LOGFILE=$(awk -F "=" '/logfile/ {print $2}' $INIFILE | tr -d ' ' | tr -d '"')
	COMMANDPATH=$(awk -F "=" '/command_path/ {print $2}' $INIFILE | tr -d ' '  | tr -d '"') 
	PHP=$(awk -F "=" '/php_executable/ {print $2}' $INIFILE | tr -d ' ' | tr -d '"')
	STOPPFILE="$COMMANDPATH/stoppdeamon"
	
	
	#echo "DAEMON_EXECUTABLE:$DAEMON_EXECUTABLE  CMD:$1 INIFILE:$INIFILE WORKER_EXECUTABLE:$WORKER_EXECUTABLE REFRESHTIME:$REFRESHTIME LOGFILE:$LOGFILE COMMANDPATH:$COMMANDPATH PHP:$PHP STOPPFILE:$STOPPFILE"
}

surftool_start() {
	DATE=`date +%Y-%m-%d_%H:%M:%S`
	echo "$DATE surftool deamon starts with DAEMON_EXECUTABLE:$DAEMON_EXECUTABLE  CMD:$1 INIFILE:$INIFILE WORKER_EXECUTABLE:$WORKER_EXECUTABLE REFRESHTIME:$REFRESHTIME LOGFILE:$LOGFILE COMMANDPATH:$COMMANDPATH PHP:$PHP STOPPFILE:$STOPPFILE"
	rm -f "$STOPPFILE"

	###  daemon loop
	while [ ! -f "$STOPPFILE" ]
	do
		$PHP --no-header $WORKER_EXECUTABLE >> $LOGFILE
		sleep $REFRESHTIME
	done

	DATE=`date +%Y-%m-%d_%H:%M:%S`
	echo "$DATE surftool deamon stopps" 
}

surftool_stop() {
	#set stopp signal
	touch "$STOPPFILE"
	sleep $((REFRESHTIME=REFRESHTIME+1))
	#kill the rest - there should be nothing
	ps ax | grep $DAEMON_EXECUTABLE | grep -v grep | awk '{print $1}' | xargs kill
	ps ax | grep $WORKER_EXECUTABLE | grep -v grep | awk '{print $1}' | xargs kill
}


surftool_status() {
	#DAEMON_EXECUTABLE
	RESULT=`ps ax | grep $DAEMON_EXECUTABLE | grep -v grep | grep -v ' status ' | grep -v rc.d | wc -l`

	if [ $RESULT -gt 0 ]; then
  		echo 'Surftool runs'
	else
  		echo 'Surftool stopped'
	fi
}

surftool_check() {
	#check CommandPath
	if ! [ -d "$COMMANDPATH" ]; then
		mkdir -p $COMMANDPATH
	 fi
	if ! [ -d "$COMMANDPATH" ]; then
		echo "Error surftooldaemon commandpath: Directory '$COMMANDPATH' does not exist "
	fi
	
	#check logfile
	if ! [ -f "$LOGFILE" ]; then
		touch $LOGFILE
	fi
	if ! [ -f "$LOGFILE" ]; then
		echo "Error surftooldaemon logfile: Logfile '$LOGFILE' does not exist "
	fi
	
	#check worker
	if ! [ -f "$WORKER_EXECUTABLE" ]; then
		echo "Error surftooldaemon worker: file '$WORKER_EXECUTABLE' does not exist "
	fi
	
	#check worker
	if ! [ -f "$PHP" ]; then
		echo "Error surftooldaemon php: file '$PHP' does not exist "
	fi
}

case $1 in
	start)
		surftool_read_configfile
		surftool_check
		surftool_start
		;;
	stop)
		surftool_read_configfile
		surftool_stop
		;;
	status)
		surftool_read_configfile
		surftool_status
		;;
	check)
		surftool_read_configfile
		surftool_check
		;;
	restart)
		surftool_stop
		surftool_start
		;;
	*)
		echo "usage [start|stop|status|check|restart] + path to config file "
		;;
esac





