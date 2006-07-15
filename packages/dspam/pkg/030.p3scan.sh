#!/bin/sh
# This file was automatically generated
# by the pfSense service handler

rc_start() {
test_p3scan_user=`cat /etc/passwd | grep p3scan`
test_p3scan_group=`cat /etc/passwd | grep p3scan`

if [ -z "${test_p3scan_group}" ]; then
  pw groupadd p3scan -g 108
fi

if [ -z "${test_p3scan_user}" ]; then
  pw useradd p3scan -u 108 -g p3scan -d /var/spool/p3scan -s /sbin/nologin -c 'P3Scan Daemon'
fi

if [ ! -d "/var/spool/p3scan" ]; then
  mkdir /var/spool/p3scan && chown p3scan:p3scan /var/spool/p3scan
fi

if [ ! -d "/var/spool/p3scan/children" ]; then
  mkdir /var/spool/p3scan/children && chown p3scan:p3scan /var/spool/p3scan/children
fi

if [ ! -d "/var/spool/p3scannotify" ]; then
  mkdir /var/spool/p3scannotify && chown p3scan:p3scan /var/spool/p3scannotify
fi

if [ ! -d "/var/run/p3scan" ]; then
  mkdir /var/run/p3scan && chown p3scan:p3scan /var/run/p3scan
fi

	/sbin/mount_fdescfs fdescfs /dev/fd
	/usr/local/sbin/p3scan --configfile=/usr/local/etc/p3scan/p3scan.conf &
}

rc_stop() {
	/usr/bin/killall p3scan
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
