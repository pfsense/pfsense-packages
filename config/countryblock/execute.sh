pfctl -t countryblock -T kill
sed -i -e '/countryblock/d' /tmp/rules.debug

#Now edit /tmp/rules.debug

#find my line for table
export i=`grep -n 'block quick from any to <snort2c>' /tmp/rules.debug | grep -o '[0-9]\{2,4\}'`
export t=`grep -n 'User Aliases' /tmp/rules.debug |grep -o '[0-9]'`

i=$(($i+'1'))
t=$(($t+'1'))
#echo $i
#echo $t


rm /tmp/rules.debug.tmp

#Insert table-entry limit 
sed -i -e '/900000/d' /tmp/rules.debug
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

pfctl -o basic -f /tmp/rules.debug > errorOUT.txt 2>&1

rm /tmp/rules.debug.tmp

#Insert countryblock rules
a="0"
echo $a
while read line
	do a=$(($a+1));
	echo $a; 
	if [ "$a" = "$i" ]; then
		echo "" >> /tmp/rules.debug.tmp
		echo "#countryblock" >> /tmp/rules.debug.tmp
		echo "table <countryblock> persist file '/usr/local/www/packages/countryblock/lists/countries.txt'" >> /tmp/rules.debug.tmp
		echo "block quick from <countryblock> to any label 'countryblock'" >> /tmp/rules.debug.tmp
		if [ -f OUTBOUND ]; then
			echo "block quick from any to <countryblock> label 'countryblock'" >> /tmp/rules.debug.tmp
		fi
	fi
	echo $line >> /tmp/rules.debug.tmp
done < "/tmp/rules.debug"

mv /tmp/rules.debug /tmp/rules.debug.old
mv /tmp/rules.debug.tmp /tmp/rules.debug

rm errorOUT.txt
pfctl -o basic -f /tmp/rules.debug > errorOUT.txt 2>&1
