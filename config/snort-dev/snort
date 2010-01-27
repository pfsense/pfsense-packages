#!/bin/sh
# $FreeBSD: ports/security/snort/files/snort.sh.in,v 1.4 2009/10/29 01:27:53 clsung Exp $

# PROVIDE: snort
# REQUIRE: DAEMON
# BEFORE: LOGIN
# KEYWORD: shutdown

. /etc/rc.subr
. /var/etc/rc.snort

name="snort"
rcvar=`set_rcvar`
start_cmd="snort_start"
stop_cmd="snort_stop"

snort_bin="/usr/local/bin/snort"
barnyard_bin="/usr/local/bin/barnyard2"

[ -z "$snort_enable" ]    && snort_enable="YES"
[ -z "$snort_flags" ]     && snort_flags="-u snort -g snort -D -q -l /var/log/snort"
[ -z "$barnyard_flags" ]     && barnyard_flags="-u snort -g snort -d /var/log/snort"

snort_start()             
{                       
        echo -n 'Starting snort:'
        for _s in ${snort_list}
	do
		echo -n " ${_s}"

		eval _conf=\"\$snort_${_s}_conf\"
		eval _name=\"\$snort_${_s}_name\"
		eval _id=\"\$snort_${_s}_id\"
		eval _iface=\"\$snort_${_s}_interface\"
		eval _enable=\"\$snort_${_s}_enable\"
		eval _barnyard=\"\$snort_${_s}_barnyard\"
		_confdir=${_conf%/*}

		_enable="${_enable:-YES}"
		if ! checkyesno _enable; then
			continue;
		fi

		if [ -f /var/run/snort_${_iface}${_name}.pid ]; then
			if pgrep -F /var/run/snort_${_iface}${_name}.pid snort; then
				echo -n " [snort ${_s} already running]"
				continue;
			fi
		fi
		${snort_bin} ${snort_flags} -G ${_id} -R ${_name} -c ${_conf} -i ${_iface}

		_barnyard="${_barnyard:-NO}"
		if checkyesno _barnyard; then
			${barnyard_bin} ${snort_flags} -R ${_name} -c ${_confdir}/barnyard2.conf \
				-f snort.u2_${_name} -w ${_confdir}/barnyard2.waldo
		fi
	done
	echo
}

snort_stop()             
{                       
        echo -n 'Stopping snort:'
	_pidlist=''
        for _s in ${snort_list}
	do
		echo -n " ${_s}"

		eval _conf=\"\$snort_${_s}_conf\"
		eval _name=\"\$snort_${_s}_name\"
		eval _iface=\"\$snort_${_s}_interface\"

		if [ -f /var/run/snort_${_iface}${_name}.pid ]; then
			_pid=$(pgrep -F /var/run/snort_${_iface}${_name}.pid snort)
			if [ -n "${_pid}" ]; then
				kill ${_pid}
				_pidlist="${_pidlist} ${_pid}"
			fi
		fi
		if [ -f /var/run/barnyard_${_iface}${_name}.pid ]; then
			_pid=$(pgrep -F /var/run/barnyard_${_iface}${_name}.pid barnyard2)
			if [ -n "${_pid}" ]; then
				kill ${_pid}
				_pidlist="${_pidlist} ${_pid}"
			fi
		fi
	done
	echo
	wait_for_pids ${_pidlist}
}

cmd="$1"
if [ $# -gt 0 ]; then
	shift
fi
if [ -n "$*" ]; then
	snort_list="$*"
fi
run_rc_command "${cmd}"
