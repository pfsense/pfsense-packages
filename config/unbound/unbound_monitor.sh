#!/bin/sh
# $Id$ */
#
#	unbound.sh
#	Copyright (C) 2011 Warren Baker
#	All rights reserved.
#
#	Redistribution and use in source and binary forms, with or without
#	modification, are permitted provided that the following conditions are met:
#
#	1. Redistributions of source code must retain the above copyright notice,
#	   this list of conditions and the following disclaimer.
#
#	2. Redistributions in binary form must reproduce the above copyright
#	   notice, this list of conditions and the following disclaimer in the
#	   documentation and/or other materials provided with the distribution.
#
#	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
#	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
#	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
#	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#	POSSIBILITY OF SUCH DAMAGE.
#

set -e

LOOP_SLEEP=5

if [ -f /var/run/unbound_alarm ]; then
	rm /var/run/unbound_alarm
fi

# Sleep 5 seconds on startup not to mangle with existing boot scripts.
sleep 5

while [ /bin/true ]; do
        if [  ! -f /var/run/unbound_alarm ]; then
                NUM_PROCS=`/bin/pgrep unbound | wc -l | awk '{print $1}'`
                if [ $NUM_PROCS -lt 1 ]; then
                        # Unbound is not running
                        echo "Unbound has exited." | logger -p daemon.info -i -t Unbound_Alarm
                        echo "Attempting restart..." | logger -p daemon.info -i -t Unbound_Alarm
                        /usr/local/etc/rc.d/unbound.sh start
                        sleep 3
                        touch /var/run/unbound_alarm
                fi
        fi
        NUM_PROCS=`/bin/pgrep unbound | wc -l | awk '{print $1}'`
        if [ $NUM_PROCS -gt 0 ]; then
                if [ -f /var/run/unbound_alarm ]; then
                        echo "Unbound has resumed." | logger -p daemon.info -i -t Unbound_Alarm
                        rm /var/run/unbound_alarm
                fi
        fi
        sleep $LOOP_SLEEP
done

if [ -f /var/run/unbound_alarm ]; then
	rm /var/run/unbound_alarm
fi

