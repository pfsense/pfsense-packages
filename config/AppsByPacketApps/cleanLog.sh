#!/bin/sh
find /usr/local/appsbypacketapps/logs/ -mtime +30 -exec rm -f {} \;