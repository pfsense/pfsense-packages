#!/bin/sh
# This file was automatically generated
# by the pfSense service handler

rc_start() {
test_clamav_group=`cat /etc/group | grep clam`
test_clamav_user=`cat /etc/passwd | grep clam`

if [ -z "${test_clamav_group}" ]; then
  pw groupadd clamav -g 106
fi

if [ -z "${test_clamav_user}" ]; then
  pw useradd clamav -u 106 -g 106 -d /nonexistent -s /sbin/nologin -c 'Clam Antivirus'
fi

if [ ! -d "/usr/local/share/clamav" ]; then
  mkdir /usr/local/share/clamav && chown clamav:clamav /usr/local/share/clamav
fi

if [ ! -d "/var/log/clamav" ]; then
  mkdir /var/log/clamav && chown clamav:clamav /var/log/clamav
fi

if [ ! -d "/var/run/clamav" ]; then
  mkdir /var/run/clamav && chown clamav:clamav /var/run/clamav
fi

	/sbin/mount_fdescfs fdescfs /dev/fd
	/usr/local/sbin/clamd
}

rc_stop() {
	/usr/bin/killall clamd
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
