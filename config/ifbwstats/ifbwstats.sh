#!/bin/sh

. /etc/rc.subr

name="ifbwstats"
start_cmd="${name}_start"
stop_cmd="${name}_stop"
restart_cmd="${name}_restart"

# called by pfSense by rc.start_packages on startup
ifbwstats_start()
{
# ifBWStats: initialize ifbwstats_daemon.php script
/usr/local/bin/php -q /usr/local/www/ifbwstats_daemon.php & 2>/dev/null
}

ifbwstats_stop()
{
################################################################
# pfSense does not call rc.stop_packages so this is not called 
################################################################
kill -INT `cat /var/run/ifbwstats.lock`
}

ifbwstats_restart()
{
kill -INT `cat /var/run/ifbwstats.lock`
sleep 2
/usr/local/bin/php -q /usr/local/www/ifbwstats_daemon.php & 2>/dev/null
}

load_rc_config $name
run_rc_command "$1"