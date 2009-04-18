#!/bin/sh
echo "Runnig the After Filter Change reset_states script" | logger
sleep 60
/sbin/pfctl -F state
sleep 40
/sbin/pfctl -F state
echo "States has been reset" | logger
