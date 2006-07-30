#!/bin/sh

# Squid monitor 1.0
# Written for pfSense's Squid Package
# by Scott Ullrich
#
# This file is released under the BSD style
#

set -e

LOOP_SLEEP=5

if [ -f /var/run/squid_alarm ]; then
	rm /var/run/squid_alarm
fi

# Squid monitor 1.0
while [ /bin/true ]; do
        if [  ! -f /var/run/squid_alarm ]; then
                NUM_PROCS=`ps awux | grep "squid -D" | grep -v "grep" | wc -l | awk '{ print $1 }'`
                if [ $NUM_PROCS -lt 1 ]; then
                        # squid is down
                        echo "Squid has exited.  Reconfiguring filter." | \
                                logger -p daemon.info -i -t Squid_Alarm
                        echo "Attempting restart..." | logger -p daemon.info -i -t Squid_Alarm
                        /usr/local/etc/rc.d/squid.sh start
                        sleep 3
                        echo "Reconfiguring filter..." | logger -p daemon.info -i -t Squid_Alarm
                        /etc/rc.filter_configure
                        touch /var/run/squid_alarm
                fi
        fi
        NUM_PROCS=`ps awux | grep "squid -D" | grep -v "grep" | wc -l | awk '{ print $1 }'`
        if [ $NUM_PROCS -gt 0 ]; then
                if [ -f /var/run/squid_alarm ]; then
                        echo "Squid has resumed. Reconfiguring filter." | \
                                logger -p daemon.info -i -t Squid_Alarm
			/etc/rc.filter_configure
                        rm /var/run/squid_alarm
                fi
        fi
        sleep $LOOP_SLEEP
done

