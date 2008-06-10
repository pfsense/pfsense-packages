#!/bin/sh
#
# $FreeBSD: ports/emulators/open-vm-tools/files/vmware-guestd.sh.in,v 1.2 2008/05/15 17:24:29 mbr Exp $
#

# PROVIDE: vmware-guestd
# REQUIRE: DAEMON
# BEFORE: LOGIN

PREFIX=/usr/local
. /etc/rc.subr

# Global
checkvm_cmd="${PREFIX}/sbin/vmware-checkvm > /dev/null"

# VMware guest daemon
name="vmware_guestd"
rcvar=`set_rcvar`
start_precmd="${checkvm_cmd}"
unset start_cmd
stop_precmd="${checkvm_cmd}"
unset stop_cmd
command="${PREFIX}/sbin/vmware-guestd"
command_args="--halt-command '/sbin/shutdown -p now' >/dev/null 2>&1"
pidfile="/var/run/${name}.pid"

vmware_guestd_enable="YES"
vmware_guestd_flags="--background ${pidfile}"
run_rc_command "$1"
