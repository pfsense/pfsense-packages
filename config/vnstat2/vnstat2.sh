#!/bin/sh


/etc/rc.conf_mount_rw
/usr/local/bin/vnstat -u
sleep 0.2
/etc/rc.conf_mount_ro
