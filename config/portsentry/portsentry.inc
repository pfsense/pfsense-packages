<?php

function portsentry_custom_php_deinstall_command() {
	conf_mount_rw();
	exec("killall portsentry");
	exec("rm -rf /usr/local/etc/portsentry*");
}

function portsentry_custom_php_install_command() {
	global $config;

	if($config['installedpackages']['portsentry']['config'][0]['blocktcp'])
		$blocktcp = "1";
	else 
		$blocktcp = "0";

	if($config['installedpackages']['portsentry']['config'][0]['blockudp'])
		$blockudp = "1";
	else 
		$blockudp = "0";

	if($config['installedpackages']['portsentry']['config'][0]['portbanner'])
		$portbanner = $config['installedpackages']['portsentry']['config'][0]['portbanner'];
	else 
		$portbanner = "You have connected to an invalid port.   Your connection has been logged.";

	if($config['installedpackages']['portsentry']['config'][0]['scantrigger'])
		$scantrigger = $config['installedpackages']['portsentry']['config'][0]['scantrigger'];
	else 
		$scantrigger = "0";

	$isfirst = true;
	$ports = "";
	foreach($config['installedpackages']['portsentry']['config'][0]['row'] as $ps) {
		if(!$isfirst) 
			$ports .= ",";
		if($ps['listenport'])
			$ports .= $ps['listenport'];
		$isfirst = false;
	}
	$tcp_ports = $ports;
	$udp_ports = $ports;
	
	$config = <<<EOF
# PortSentry Configuration

#######################
# Port Configurations #
#######################
#
#
# Some example port configs for classic and basic Stealth modes
#
# I like to always keep some ports at the "low" end of the spectrum.
# This will detect a sequential port sweep really quickly and usually
# these ports are not in use (i.e. tcpmux port 1)
#
# ** X-Windows Users **: If you are running X on your box, you need to be sure
# you are not binding PortSentry to port 6000 (or port 2000 for OpenWindows users). 
# Doing so will prevent the X-client from starting properly. 
#
# These port bindings are *ignored* for Advanced Stealth Scan Detection Mode.
#

# Un-comment these if you are really anal:
TCP_PORTS="$tcp_ports"
UDP_PORTS="$udp_ports"

###########################################
# Advanced Stealth Scan Detection Options #
###########################################
#
# This is the number of ports you want PortSentry to monitor in Advanced mode.
# Any port *below* this number will be monitored. Right now it watches 
# everything below 1024. 
# 
# On many Linux systems you cannot bind above port 61000. This is because
# these ports are used as part of IP masquerading. I don't recommend you
# bind over this number of ports. Realistically: I DON'T RECOMMEND YOU MONITOR 
# OVER 1024 PORTS AS YOUR FALSE ALARM RATE WILL ALMOST CERTAINLY RISE. You've been
# warned! Don't write me if you have have a problem because I'll only tell
# you to RTFM and don't run above the first 1024 ports.
#
#
#ADVANCED_PORTS_TCP="1024"
#ADVANCED_PORTS_UDP="1024"
#
# This field tells PortSentry what ports (besides listening daemons) to
# ignore. This is helpful for services like ident that services such 
# as FTP, SMTP, and wrappers look for but you may not run (and probably 
# *shouldn't* IMHO). 
#
# By specifying ports here PortSentry will simply not respond to
# incoming requests, in effect PortSentry treats them as if they are
# actual bound daemons. The default ports are ones reported as 
# problematic false alarms and should probably be left alone for
# all but the most isolated systems/networks.
#
# Default TCP ident and NetBIOS service
ADVANCED_EXCLUDE_TCP="113,139"
# Default UDP route (RIP), NetBIOS, bootp broadcasts.
ADVANCED_EXCLUDE_UDP="520,138,137,67"


######################
# Configuration Files#
######################
#
# Hosts to ignore
IGNORE_FILE="/usr/local/etc/portsentry.ignore"
# Hosts that have been denied (running history)
HISTORY_FILE="/var/db/portsentry.history"
# Hosts that have been denied this session only (temporary until next restart)
BLOCKED_FILE="/var/db/portsentry.blocked"

##############################
# Misc. Configuration Options#
##############################
#
# DNS Name resolution - Setting this to "1" will turn on DNS lookups
# for attacking hosts. Setting it to "0" (or any other value) will shut
# it off.
RESOLVE_HOST = "1"

###################
# Response Options#
###################
# Options to dispose of attacker. Each is an action that will 
# be run if an attack is detected. If you don't want a particular
# option then comment it out and it will be skipped.
#
# The variable $TARGET$ will be substituted with the target attacking
# host when an attack is detected. The variable $PORT$ will be substituted
# with the port that was scanned. 
#
##################
# Ignore Options #
##################
# These options allow you to enable automatic response
# options for UDP/TCP. This is useful if you just want
# warnings for connections, but don't want to react for  
# a particular protocol (i.e. you want to block TCP, but
# not UDP). To prevent a possible Denial of service attack
# against UDP and stealth scan detection for TCP, you may 
# want to disable blocking, but leave the warning enabled. 
# I personally would wait for this to become a problem before
# doing though as most attackers really aren't doing this.
# The third option allows you to run just the external command
# in case of a scan to have a pager script or such execute
# but not drop the route. This may be useful for some admins
# who want to block TCP, but only want pager/e-mail warnings
# on UDP, etc.
#
# 
# 0 = Do not block UDP/TCP scans.
# 1 = Block UDP/TCP scans.
# 2 = Run external command only (KILL_RUN_CMD)

BLOCK_UDP="$block_udp"
BLOCK_TCP="$block_tcp"

###############
# TCP Wrappers#
###############
# This text will be dropped into the hosts.deny file for wrappers
# to use. There are two formats for TCP wrappers:
#
# Format One: Old Style - The default when extended host processing
# options are not enabled.
#
KILL_HOSTS_DENY="ALL: \$TARGET\$"

# Format Two: New Style - The format used when extended option
# processing is enabled. You can drop in extended processing
# options, but be sure you escape all '%' symbols with a backslash
# to prevent problems writing out (i.e. \%c \%h )
#
#KILL_HOSTS_DENY="ALL: \$TARGET\$ : DENY"

###################
# External Command#
###################
# This is a command that is run when a host connects, it can be whatever
# you want it to be (pager, etc.). This command is executed before the 
# route is dropped or after depending on the KILL_RUN_CMD_FIRST option below
#
#
# I NEVER RECOMMEND YOU PUT IN RETALIATORY ACTIONS AGAINST THE HOST SCANNING 
# YOU!
#
# TCP/IP is an *unauthenticated protocol* and people can make scans appear out 
# of thin air. The only time it is reasonably safe (and I *never* think it is 
# reasonable) to run reverse probe scripts is when using the "classic" -tcp mode. 
# This mode requires a full connect and is very hard to spoof.
#
# The KILL_RUN_CMD_FIRST value should be set to "1" to force the command 
# to run *before* the blocking occurs and should be set to "0" to make the 
# command run *after* the blocking has occurred. 
#
#KILL_RUN_CMD_FIRST = "0"
#
# \$PORT\$
KILL_RUN_CMD="pfctl -k \$TARGET\$ ; pfctl -t virusprot -T add \$TARGET\$"

#####################
# Scan trigger value#
#####################
# Enter in the number of port connects you will allow before an 
# alarm is given. The default is 0 which will react immediately.
# A value of 1 or 2 will reduce false alarms. Anything higher is 
# probably not necessary. This value must always be specified, but
# generally can be left at 0. 
#
# NOTE: If you are using the advanced detection option you need to
# be careful that you don't make a hair trigger situation. Because
# Advanced mode will react for *any* host connecting to a non-used
# below your specified range, you have the opportunity to really 
# break things. (i.e someone innocently tries to connect to you via 
# SSL [TCP port 443] and you immediately block them). Some of you
# may even want this though. Just be careful.
#
SCAN_TRIGGER="$scan_trigger"

######################
# Port Banner Section#
######################
#
# Enter text in here you want displayed to a person tripping the PortSentry.
# I *don't* recommend taunting the person as this will aggravate them.
# Leave this commented out to disable the feature
#
# Stealth scan detection modes don't use this feature
#
PORT_BANNER="$port_banner"

EOF

	conf_mount_rw();
	// Write out configuration
	$fd = fopen("/usr/local/etc/portsentry.conf", "w");
	fwrite($fd, $config);
	fclose($fd);
	
	$svscan = <<<EOD
#!/bin/sh

# PROVIDE: portsentry
# REQUIRE: LOGIN
# KEYWORD: FreeBSD

. /etc/rc.subr

name="portsentry"
rcvar=`set_rcvar`
command="/usr/local/bin/portsentry"
portsentry_enable=\${portsentry_enable-"YES"}

start_cmd="portsentry_start"
stop_postcmd="portsentry_stop_post"

load_rc_config \$name

portsentry_start () {
        echo "Starting svscan."
        /usr/bin/env \
        PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
		portsentry 
}

portsentry_stop_post () {
        echo "Stopping portsentry."
        killall portsentry
}

run_rc_command "\$1"

EOD;

	$fd = fopen("/usr/local/etc/rc.d/portsentry.sh", "w");
	fwrite($fd, $svscan);
	fclose($fd);
	exec("chmod a+rx /usr/local/etc/rc.d/portsentry.sh");
	conf_mount_ro();
}

?>