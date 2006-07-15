#!/bin/sh
# This file was automatically generated
# by the pfSense service handler

rc_start() {
test_mysql_user=`cat /etc/passwd | grep mysql`
test_mysql_group=`cat /etc/group | grep mysql`
mysql_user="mysql"
mysql_limits_args="-e -U ${mysql_user}"
pidfile="/var/db/mysql/`/bin/hostname`.pid"
command="/usr/local/bin/mysqld_safe"
command_args="--user=${mysql_user} --datadir=/var/db/mysql --pid-file=${pidfile} --bind-address=127.0.0.1 --set-variable=max_connections=500"
procname="/usr/local/libexec/mysqld"
mysql_install_db="/usr/local/bin/mysql_install_db"
mysql_install_db_args="--ldata=/var/db/mysql"

/sbin/mount_fdescfs fdescfs /dev/fd

if [ -z "${test_mysql_group}" ]; then
  pw groupadd mysql -g 88
fi

if [ -z "${test_mysql_user}" ]; then
  pw useradd mysql -u 88 -g 88 -d /nonexistent -s /sbin/nologin -c 'MySQL Daemon'
fi

if [ ! -d "/var/db/mysql" ]; then
  mkdir /var/db/mysql && chown mysql:mysql /var/db/mysql
fi

if [ ! -d "/var/db/mysql/mysql/." ]; then
	eval $mysql_install_db $mysql_install_db_args >/dev/null
	[ $? -eq 0 ] && chown -R ${mysql_user}:${mysql_user} /var/db/mysql
fi

#if checkyesno mysql_limits; then
#	eval `/usr/bin/limits ${mysql_limits_args}` 2>/dev/null
#else
#	return 0
#fi

${command} ${command_args} > /dev/null &
}

rc_stop() {
/usr/bin/killall mysqld
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
