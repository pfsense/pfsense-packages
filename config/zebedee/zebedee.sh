#!/bin/sh
if [ ! -d "/usr/local/etc/zebedee"  ];  then 
mkdir /usr/local/etc/zebedee
fi 
killall zebedee 
/usr/local/bin/zebedee -t -f /usr/local/etc/server.zbd &
