<?php

	require_once("guiconfig.inc");

echo '
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Help & Info</title>
<base target="main">
<script src="./javascript/tabs.js" type="text/javascript"></script>
<link href="./css/style2.css" rel="stylesheet" type="text/css" />
</head>

<body>

<style type="text/css">
</style>

<div id="container">
	<div id="header">
	<IMG SRC="./images/logo.jpg" width="780px" height="76" ALT="Snort Package">
	</div>
	<div class="navigation" id="navigation">
		<ul>
			<li><a href="#item1" target="_self">Home</a></li>
			<li><a href="#item2" target="_self">About Me</a></li>
			<li><a href="#item3" target="_self">Services</a></li>
			<li><a href="#item4" target="_self">Change Log</a></li>
			<li><a href="#item7" target="_self">Faq</a></li>
			<li><a href="#item6" target="_self">Heros</a></li>
			<li><a href="#item5" target="_self">Developers</a></li>
		</ul>
	</div>
	<br>
<div class="content" id="item1">
	<p>
	<font size="5"><strong>Snort Package</strong></font> is a GUI based front-end for Sourcefire\'s Snort ® IDS/IPS software. The Snort Package goal is to be
	the best open-source GUI to manage multiple snort sensors and multiple rule snapshots. The project other goal is to be a highly competitive GUI for
	network monitoring for both private and enterprise use. Lastly, this project software development should bring programmers and users together to create 
	software.
	</p>
	<p>
	<font size="5"><strong>What is Snort ?</strong></font> Used by fortune 500 companies and goverments Snort is the most widely deployed IDS/IPS technology worldwide. It features rules based logging and
    can perform content searching/matching in addition to being used to detect a variety of other attacks and probes, such as buffer overflows, stealth port
    scans, CGI attacks, SMB probes, and much more.
	</p>
	<p>
	<font size="5"><strong>Requirements :</strong></font><br>
	Minimum requirement 256 mb ram, 500 MHz CPU.<br>
	Recommended 500 mb ram, 1 Ghz CPU.<br>
    The more rules you run the more memory you need.<br>
    The more interfaces you select the more memory you need.<br><br>
    Development is done on a Alix 2D3 system (500 MHz AMD Geode LX800 CPU 256MB DDR DRAM).
	</p>
</div>
<div class="content" id="item2">
    <p>
About Me<br><br>
Coming soon............

</p>
</div>
<div class="content" id="item3">
    <p>
Services<br><br>
Coming soon............
</p>
</div>
<div class="content" id="item4">
<p>
Change Log<br><br>
Coming soon............
</p>
</div>
<div class="content" id="item5">
<p>
<font size="5"><strong>PfSense</strong></font> is brought to you by a dedicated group of developers who are security and network professionals by trade. The following people are active developers of the pfSense project. 
Username is listed in parenthesis (generally also the person\'s forum username, IRC nickname, etc.).<br><br>

<font size="5"><strong>Main Snort-dev Package Developer</strong></font><br>
Robert Zelaya<br><br>

<font size="5"><strong>Founders</strong></font><br>
In alphabetical order<br><br>

Chris Buechler (cmb)<br>
Scott Ullrich (sullrich)<br><br>

<font size="5"><strong>Active Developers</strong></font><br>
Listed in order of seniority along with date of first contribution.<br><br>

Bill Marquette (billm) - February 2005<br>
Holger Bauer (hoba) - May 2005<br>
Erik Kristensen (ekristen) - August 2005<br>
Seth Mos (smos) - November 2005<br>
Scott Dale (sdale) - December 2006<br>
Martin Fuchs (mfuchs) - June 2007<br>
Ermal Luçi (ermal) - January 2008<br>
Matthew Grooms (mgrooms) - July 2008<br>
Mark Crane (mcrane) - October 2008<br>
Jim Pingle (jim-p) - February 2009<br>
Rob Zelaya (robiscool) - March 2009<br>
Renato Botelho (rbgarga) - May 2009<br><br>

<font size="5"><strong>FreeBSD Developer Assistance</strong></font><br>
We would like to thank the following FreeBSD developers for their assistance.<br><br>

Max Laier (mlaier)<br>
Christian S.J. Peron (csjp)<br>
Andrew Thompson (thompsa)<br>
Bjoern A. Zeeb (bz)<br><br>

among many others who help us directly, and everyone who contributes to FreeBSD.<br><br>

<font size="5"><strong>Inactive Developers</strong></font><br>
The following individuals are no longer active contributors, having moved on because of other commitments, or employers forbidding contributions. We thank them for their past contributions.<br><br>

Daniel Berlin (dberlin)<br>
Daniel Haischt (dsh)<br>
Espen Johansen (lsf)<br>
Scott Kamp (dingo)<br>
Bachman Kharazmi (bkw)<br>
Fernando Tarlá Cardoso Lemos (fernando)<br>
Kyle Mott (kyle)<br>
Colin Smith (colin)<br>
</p>
</div>
<div class="content" id="item6">
<p>
Heros<br><br>
Coming soon............
</p>
</div>
<div class="content" id="item7">
<p>
=========================<br>

Q: Do you have a quick install tutorial and tabs explanation.<br>

A: Yes.<br>
    
    http://doc.pfsense.org/index.php/Setup_Snort_Package<br>

=========================<br>

Q: What interfaces can snort listen on ?<br>

A: Right now all WAN interfaces and LAN interfaces. But if you select a LAN interface you may need to adjust the snort rules to use the LAN interface.<br>
    
==========================<br>

Q: What logs does the snort package keep. ?<br>

A: Most of the snort logs are keept in the /var/log/snort.<br>
    Snorts syslogs\' are saved to the /var/log/snort/snort_sys_0ng0.<br>
    
==========================<br>

Q: What is the best Performance setting ? or Snort is using 90% cpu and all my memory.<br>

A: Depends how much memory you have and how many rules you want to run.; lowmem for systems with less than 256 mb memory, ac-bnfa for systems<br>
   with over 256 mb of memory. The other options are; ac high memory, best performance, ac-std moderate memory, high performance,acs small<br>
   memory, moderate performance,ac-banded small memory,moderate performance,ac-sparsebands small memory, high performance.<br>

   Short version: For most people ac-bnfa is the best setting.<br>

=========================<br>

Q: What is the Oinkmaster code ? How do I get the code ?<br>

A: The Oinkmaster code is your personal password in order to download snort rules.<br>
    You get a Oinkmaster code when you register with snort.org. It is free to register.<br>
    Goto https://www.snort.org/signup to get your personal code.<br>
    
=========================<br>

Q: What is the Snort.org subscriber option? How do I become a  Snort.org subscriber?<br>

A: Snort.org subscribers get the the latest rule updates 30 days faster than registered users.<br>
    Goto http://www.snort.org/vrt/buy-a-subscription/.
    It is highly suggested that you get a paid subscription so that you can always have the latest rules.<br>
    
=========================<br>

Q: When did you start working on the snort package.<br>

A: I started working on the snort package in May 2009.<br>
</p>
</div>
</div>
</body>
</html>
';
?>