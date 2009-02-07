#!/bin/sh

rc_start() {
	/usr/local/sbin/darkstat --detach
}

rc_stop() {
   killall darkstat
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
	    rc_start
	    ;;
esac
