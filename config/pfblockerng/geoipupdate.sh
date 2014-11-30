#!/bin/sh
#
# pfBlockerNG MaxMind GeoLite GeoIP Updater Script - By BBcan177@gmail.com
# Copyright (C) 2014 BBcan177@gmail.com
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License Version 2 as
# published by the Free Software Foundation.  You may not use, modify or
# distribute this program under any other version of the GNU General
# Public License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# The GeoLite databases by MaxMind Inc., are distributed under the Creative Commons 
# Attribution-ShareAlike 3.0 Unported License. The attribution requirement
# may be met by including the following in all advertising and documentation
# mentioning features of or use of this database.


# Folder Locations
pathfetch=/usr/bin/fetch
pathtar=/usr/bin/tar
pathgunzip=/usr/bin/gunzip

# File Locations
pathdb=/var/db/pfblockerng
pathlog=/var/log/pfblockerng
errorlog=$pathlog/geoip.log
pathgeoipdatgz=$pathdb/GeoIP.dat.gz
pathgeoipdatgzv6=$pathdb/GeoIPv6.dat.gz
pathgeoipdat=$pathdb/GeoIP.dat
pathgeoipdatv6=$pathdb/GeoIPv6.dat
pathgeoipcc=$pathdb/country_continent.csv
pathgeoipcsv4=$pathdb/GeoIPCountryCSV.zip
pathgeoipcsvfinal4=$pathdb/GeoIPCountryWhois.csv
pathgeoipcsv6=$pathdb/GeoIPv6.csv.gz
pathgeoipcsvfinal6=$pathdb/GeoIPv6.csv

if [ ! -d $pathdb ]; then mkdir $pathdb; fi
if [ ! -d $pathlog ]; then mkdir $pathlog; fi

# Collect pfSense Version
pfs_version="$(cut -c1-3 /etc/version)"
now=$(date)
echo; echo "$now - Updating pfBlockerNG - Country Database Files"
echo "pfBlockerNG uses GeoLite data created by MaxMind, available from http://www.maxmind.com"
echo "pfSense version: $pfs_version"; echo


##########
#Function to update MaxMind GeoIP Binary (For Reputation Process)
binaryupdate() {

echo " ** Downloading MaxMind GeoLite IPv4 Binary Database (For Reputation/Alerts Processes) **"; echo
URL="http://geolite.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz"

case $pfs_version in
	1.0|2.0|2.1)
		$pathfetch -v -o $pathgeoipdatgz -T 20 $URL
		;;
	2.2)
		$pathfetch -v --no-verify-peer -o $pathgeoipdatgz -T 20 $URL
		;;
	*)
		$pathfetch -v --no-verify-peer -o $pathgeoipdatgz -T 20 $URL
		;;
esac

if [ "$?" -eq "0" ]; then
	$pathgunzip -f $pathgeoipdatgz
	echo; echo " ( MaxMind IPv4 GeoIP.dat has been updated )"; echo
	echo "Current Date/Timestamp:"
	/bin/ls -alh $pathgeoipdat
	echo
else
	echo; echo " => MaxMind IPv4 GeoIP.dat Update [ FAILED ]"; echo
	echo "MaxMind IPV4 Binary Update FAIL [ $now ]" >> $errorlog 
fi


#####

echo; echo " ** Downloading MaxMind GeoLite IPv6 Binary Database (For Reputation/Alerts Processes) **"; echo
URL="http://geolite.maxmind.com/download/geoip/database/GeoIPv6.dat.gz"

case $pfs_version in
	1.0|2.0|2.1)
		$pathfetch -v -o $pathgeoipdatgzv6 -T 20 $URL
		;;
	2.2)
		$pathfetch -v --no-verify-peer -o $pathgeoipdatgzv6 -T 20 $URL
		;;
	*)
		$pathfetch -v --no-verify-peer -o $pathgeoipdatgzv6 -T 20 $URL
		;;
esac

if [ "$?" -eq "0" ]; then
	$pathgunzip -f $pathgeoipdatgzv6
	echo; echo " ( MaxMind IPv6 GeoIPv6.dat has been updated )"; echo
	echo "Current Date/Timestamp:"
	/bin/ls -alh $pathgeoipdatv6
	echo
else
	echo; echo " => MaxMind IPv6 GeoIPv6.dat Update [ FAILED ]"; echo
	echo "MaxMind IPv6 Binary Update FAIL [ $now ]" >> $errorlog
fi

}


##########
#Function to update MaxMind Country Code Files
csvupdate() {

# Download Part 1 - CSV IPv4 Database

echo; echo " ** Downloading MaxMind GeoLite IPv4 CSV Database **"; echo
URL="http://geolite.maxmind.com/download/geoip/database/GeoIPCountryCSV.zip"
case $pfs_version in
	1.0|2.0|2.1)
		$pathfetch -v -o $pathgeoipcsv4 -T 20 $URL
		;;
	2.2)
		$pathfetch -v --no-verify-peer -o $pathgeoipcsv4 -T 20 $URL
		;;
	*)
		$pathfetch -v --no-verify-peer -o $pathgeoipcsv4 -T 20 $URL
		;;
esac

if [ "$?" -eq "0" ]; then
	$pathtar -zxvf $pathgeoipcsv4 -C $pathdb
	if [ "$?" -eq "0" ]; then
		echo; echo " ( MaxMind GeoIPCountryWhois has been updated )"; echo
		echo "Current Date/Timestamp:"
		/bin/ls -alh $pathgeoipcsvfinal4
		echo
	else
		echo; echo " => MaxMind IPv4 GeoIPCountryWhois [ FAILED ]"; echo
		echo "MaxMind CSV Database Update FAIL - Tar extract [ $now ]" >> $errorlog
	fi
else
	echo; echo " => MaxMind IPv4 CSV Download [ FAILED ]"; echo
	echo "MaxMind CSV Database Update FAIL [ $now ]" >> $errorlog
fi


# Download Part 2 - Country Definitions

echo; echo " ** Downloading MaxMind GeoLite Database Country Definition File **"; echo
URL="http://dev.maxmind.com/static/csv/codes/country_continent.csv"
case $pfs_version in
	1.0|2.0|2.1)
		$pathfetch -v -o $pathgeoipcc -T 20 $URL
		;;
	2.2)
		$pathfetch -v --no-verify-peer -o $pathgeoipcc -T 20 $URL
		;;
	*)
		$pathfetch -v --no-verify-peer -o $pathgeoipcc -T 20 $URL
		;;
esac

if [ "$?" -eq "0" ]; then
	echo; echo " ( MaxMind ISO 3166 Country Codes has been updated. )"; echo
	echo "Current Date/Timestamp:"
	/bin/ls -alh $pathgeoipcc
	echo
else
	echo; echo " => MaxMind ISO 3166 Country Codes Update [ FAILED ]"; echo
	echo "MaxMind ISO 3166 Country Code Update FAIL [ $now ]" >> $errorlog
fi

# Download Part 3 - Country Definitions IPV6

echo " ** Downloading MaxMind GeoLite IPv6 CSV Database **"; echo
URL="http://geolite.maxmind.com/download/geoip/database/GeoIPv6.csv.gz"
case $pfs_version in
	1.0|2.0|2.1)
		$pathfetch -v -o $pathgeoipcsv6 -T 20 $URL
		;;
	2.2)
		$pathfetch -v --no-verify-peer -o $pathgeoipcsv6 -T 20 $URL
		;;
	*)
		$pathfetch -v --no-verify-peer -o $pathgeoipcsv6 -T 20 $URL
		;;
esac

if [ "$?" -eq "0" ]; then
	$pathgunzip -f $pathgeoipcsv6 
	echo; echo " ( MaxMind GeoIPv6.csv has been updated )"; echo
	echo "Current Date/Timestamp:"
	/bin/ls -alh $pathgeoipcsvfinal6
	echo
else
	echo; echo " => MaxMind GeoLite IPv6 Update [ FAILED ]"; echo
	echo "MaxMind GeoLite IPv6 Update FAIL [ $now ]" >> $errorlog
fi

}

##########
# CALL APPROPRIATE PROCESSES using Script Argument $1

case $1 in
	bu)
		binaryupdate
		;;
	cu)
		csvupdate
		;;
	all)
		binaryupdate
		csvupdate
		;;
	*)
		exit
		;;
esac
exit