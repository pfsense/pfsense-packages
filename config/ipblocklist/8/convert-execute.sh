#!/bin/sh

#check if ipblocklist running
#export resultr=`pfctl -s rules | grep -c ipblocklist`

#echo $resultr
#if [ "$resultr" -gt "0" ]; then
#	echo running
#	exit 1
#else
#	echo not running
#	/usr/bin/logger -s "IP-Blocklist was found not running"
#	echo "IP-Blocklist not running" | /usr/local/bin/php /usr/local/www/packages/ipblocklist/email_send.php
#fi


#kill tables to elminate dups
/sbin/pfctl -t ipblocklist -T kill
/sbin/pfctl -t ipblocklistW -T kill
/usr/bin/sed -i -e '/ipblocklist/d' /tmp/rules.debug
/usr/bin/sed -i -e '/ipblocklistW/d' /tmp/rules.debug

#Generate lists to process
ls /usr/local/www/packages/ipblocklist/lists > /usr/local/www/packages/ipblocklist/file_list.txt
ls /usr/local/www/packages/ipblocklist/Wlists > /usr/local/www/packages/ipblocklist/file_Wlist.txt
filelist="/usr/local/www/packages/ipblocklist/file_list.txt"
Wfilelist="/usr/local/www/packages/ipblocklist/file_Wlist.txt"

#READ contents in file_list.txt and process as file
for fileline in $(cat $filelist); do
iplist="/usr/local/www/packages/ipblocklist/lists/$fileline"
iplistout="/usr/local/www/packages/ipblocklist/lists/ipfw.ipfw"
if [ "$iplist" != "/usr/local/www/packages/ipblocklist/lists/ipfw.ipfw" ]; then
	/usr/bin/perl /usr/local/www/packages/ipblocklist/convert.pl $iplist $iplistout
	#echo "THIS JUST RAN"
fi
done

#Whitelist
for Wfileline in $(cat $Wfilelist); do
Wiplist="/usr/local/www/packages/ipblocklist/Wlists/$Wfileline"
Wiplistout="/usr/local/www/packages/ipblocklist/Wlists/whitelist"
/usr/bin/perl convert.pl $Wiplist $Wiplistout
done
#echo "ipfw made"

#clean up ipfw.ipfw (duplicates)
rm /usr/local/www/packages/ipblocklist/lists/ipfw.ipfwTEMP
/usr/bin/sort lists/ipfw.ipfw | uniq -u >> /usr/local/www/packages/ipblocklist/lists/ipfw.ipfwTEMP
mv /usr/local/www/packages/ipblocklist/lists/ipfw.ipfwTEMP /usr/local/www/packages/ipblocklist/lists/ipfw.ipfw
#echo "ipfw clean"

#clean up whitelist (duplicates)
rm Wlists/whitelistTEMP
/usr/bin/sort Wlists/whitelist | uniq -u >> Wlists/whitelistTEMP
mv Wlists/whitelistTEMP Wlists/whitelist
#echo "whitelist clean"



#Now edit /tmp/rules.debug

#find my line for table
export i=`grep -n 'block quick from any to <snort2c>' /tmp/rules.debug | grep -o '[0-9]\{2,4\}'`
export t=`grep -n 'User Aliases' /tmp/rules.debug |grep -o '[0-9]\{1,2\}'`

i=$(($i+'1'))
t=$(($t+'1'))
#echo $i
#echo $t

rm /tmp/rules.debug.tmp

#Insert table-entry limit 
/usr/bin/sed -i -e '/900000/d' /tmp/rules.debug
while read line
	do a=$(($a+1)); 
	#echo $a;
	if [ "$a" = "$t" ]; then
		echo "" >> /tmp/rules.debug.tmp
		echo "set limit table-entries 900000" >> /tmp/rules.debug.tmp
	fi
	echo $line >> /tmp/rules.debug.tmp
done < "/tmp/rules.debug"

mv /tmp/rules.debug /tmp/rules.debug.old
mv /tmp/rules.debug.tmp /tmp/rules.debug

/sbin/pfctl -o basic -f /tmp/rules.debug > /usr/local/www/packages/ipblocklist/errorOUT.txt 2>&1

rm /tmp/rules.debug.tmp
#Insert ipblocklist rules
a="0"
echo $a
while read line
	do a=$(($a+1));
	echo $a; 
	if [ "$a" = "$i" ]; then
		echo "" >> /tmp/rules.debug.tmp
		echo "#ipblocklist" >> /tmp/rules.debug.tmp
		echo "table <ipblocklist> persist file '/usr/local/www/packages/ipblocklist/lists/ipfw.ipfw'" >> /tmp/rules.debug.tmp
		echo "table <ipblocklistW> persist file '/usr/local/www/packages/ipblocklist/Wlists/whitelist'" >> /tmp/rules.debug.tmp
		
		for i in $(cat /usr/local/www/packages/ipblocklist/interfaces.txt); do
			echo "pass quick from <ipblocklistW> to any label 'IP-Blocklist'" >> /tmp/rules.debug.tmp
			echo "pass quick from $i to <ipblocklistW> label 'IP-Blocklist'" >> /tmp/rules.debug.tmp
			if [ -f /usr/local/www/packages/ipblocklist/logging ]; then
				echo "block log quick from <ipblocklist> to $i label 'IP-Blocklist'" >> /tmp/rules.debug.tmp
			else
				echo "block quick from <ipblocklist> to $i label 'IP-Blocklist'" >> /tmp/rules.debug.tmp
			fi
			if [ -f /usr/local/www/packages/ipblocklist/OUTBOUND ]; then
				echo "block quick from $i to <ipblocklist> label 'IP-Blocklist'" >> /tmp/rules.debug.tmp
			fi
		done
	fi
	echo $line >> /tmp/rules.debug.tmp
done < "/tmp/rules.debug"

mv /tmp/rules.debug /tmp/rules.debug.old
mv /tmp/rules.debug.tmp /tmp/rules.debug

#Now execute the ipfw list (Take a long time in old version)
#sh lists/ipfw.ipfw (Version 0.1.4)
rm /usr/local/www/packages/ipblocklist/errorOUT.txt
/sbin/pfctl -o basic -f /tmp/rules.debug > /usr/local/www/packages/ipblocklist/errorOUT.txt 2>&1
