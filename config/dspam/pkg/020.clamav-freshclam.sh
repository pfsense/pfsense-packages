#!/bin/sh
# This file was automatically generated
# by the pfSense service handler

rc_start() {
	/sbin/mount_fdescfs fdescfs /dev/fd
	/usr/local/bin/freshclam --daemon
}

rc_stop() {
	/usr/bin/killall freshclam
	sleep 2
}

rc_restart() {
	rc_stop
	rc_start
}

case $1 in
	start)
		rc_start
		;;
	stop)
		rc_stop
		;;
	restart)
		rc_restart
		;;
	*)
		echo "Usage: $0 <start|stop|restart>"
		;;
esac
