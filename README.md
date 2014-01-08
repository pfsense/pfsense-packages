pfsense-packages
================
lcdproc-0.5.6 pkg v. 0.9.7

Patch to fix the 4 status LEDs 

This is in the pages for this Package
Enable Output LEDs	 
Enable the Output LEDs present on some LCD panels. This feature is currently supported by the CFontz633 driver only. 
Each LED can be off or show two colors: RED (alarm) or GREEN (everything ok) and shows: 
LED1: NICs status (green: ok, red: at least one nic down);
LED2: CARP status (green: master, red: backup, off: CARP not implemented);
LED3: CPU status (green < 50, red > 50%);
LED4: Gateway status (green: ok, red: at least one gateway not responding, off: no gateway configured).

This does not work on CFonts Displays properly.  I have fixed it to make this work.  There is one thing that I don't 
know if I got going and that is LED1.  


pfSense packages repository
