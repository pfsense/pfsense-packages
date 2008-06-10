#!/bin/sh
#
# $FreeBSD: ports/emulators/open-vm-tools/files/vmware-kmod.sh.in,v 1.1 2008/05/15 17:24:29 mbr Exp $
#

# PROVIDE: vmware-kmod
# REQUIRE: FILESYSTEMS
# BEFORE: netif

PREFIX=/usr/local
. /etc/rc.subr

# Global
checkvm_cmd="${PREFIX}/sbin/vmware-checkvm > /dev/null"

# Functions
vmware_guest_vmmemctl_start()
{
	echo 'Loading vmmemctl kernel module.'
	kldload ${PREFIX}/lib/vmware-tools/modules/drivers/vmmemctl.ko >/dev/null 2>&1
}
vmware_guest_vmxnet_start()
{
	echo 'Loading vmxnet kernel module.'
	kldload ${PREFIX}/lib/vmware-tools/modules/drivers/vmxnet.ko >/dev/null 2>&1
}
vmware_guest_vmblock_start()
{
	echo 'Loading vmblock kernel module.'
	kldload ${PREFIX}/lib/vmware-tools/modules/drivers/vmblock.ko >/dev/null 2>&1
}
vmware_guest_vmhgfs_start()
{
	echo 'Loading vmhgfs kernel module.'
	kldload ${PREFIX}/lib/vmware-tools/modules/drivers/vmhgfs.ko >/dev/null 2>&1
}

# VMware kernel module: vmmemctl
name="vmware_guest_vmmemctl"
rcvar=`set_rcvar`
start_precmd="${checkvm_cmd}"
start_cmd="vmware_guest_vmmemctl_start"
stop_precmd="${checkvm_cmd}"
stop_cmd=":"

vmware_guest_vmmemctl_enable="YES"
vmware_guest_kmod_enable="YES"
run_rc_command "$1"

# VMware kernel module: vmxnet
name="vmware_guest_vmxnet"
rcvar=`set_rcvar`
start_precmd="${checkvm_cmd}"
start_cmd="vmware_guest_vmxnet_start"
stop_precmd="${checkvm_cmd}"
stop_cmd=":"

vmware_guest_vmxnet_enable="YES"
run_rc_command "$1"

# VMware kernel module: vmblock
name="vmware_guest_vmblock"
rcvar=`set_rcvar`
start_precmd="${checkvm_cmd}"
start_cmd="vmware_guest_vmblock_start"
stop_precmd="${checkvm_cmd}"
stop_cmd=":"

vmware_guest_vmblock_enable="YES"
run_rc_command "$1"

# VMware kernel module: vmhgfs
name="vmware_guest_vmhgfs"
rcvar=`set_rcvar`
start_precmd="${checkvm_cmd}"
start_cmd="vmware_guest_vmhgfs_start"
stop_precmd="${checkvm_cmd}"
stop_cmd=":"

vmware_guest_vmhgfs_enable="YES"
run_rc_command "$1"
