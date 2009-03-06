#!/bin/sh
sleep 60
/sbin/pfctl -F state
sleep 40
/sbin/pfctl -F state
