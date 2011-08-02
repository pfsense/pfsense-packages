<?php
/* $Id$ */
/*

 part of pfSense
 All rights reserved.

 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 All rights reserved.

 Pfsense Old snort GUI 
 Copyright (C) 2006 Scott Ullrich.
 
 Pfsense snort GUI 
 Copyright (C) 2008-2012 Robert Zelaya.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. Neither the name of the pfSense nor the names of its contributors 
 may be used to endorse or promote products derived from this software without 
 specific prior written permission.

 THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 */

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");

	$pgtitle = 'Snort: Help and Info';
	include("/usr/local/pkg/snort/snort_head.inc");

?>		

<style type="text/css">

h1 {
font-size: 3em; 
margin: 20px 0;
}

.container2 {
width: 765px; 
margin: 0px auto;
}
ul.tabs {
	margin: 0;
	padding: 0;
	float: left;
	list-style: none;
	height: 25px;
	border-bottom: 1px solid #999;
	border-left: 1px solid #999;
	width: 100%;
}
ul.tabs li {
	float: left;
	margin: 0;
	padding: 0;
	height: 24px;
	line-height: 24px;
	border: 1px solid #000000;
	border-left: none;
	margin-bottom: -1px;
	background: #ffffff;
	overflow: hidden;
	position: relative;
}
ul.tabs li a {
	text-decoration: none;
	color: #000000;
	display: block;
	font-size: 1.2em;
	padding: 0 20px;
	border: 1px solid #fff;
	outline: none;
}
ul.tabs li a:hover {
	background: #eeeeee;
}
	
html ul.tabs li.active, html ul.tabs li.active a:hover  {
	background: #fff;
	border-bottom: 1px solid #fff;
	color: #000000;
}
.tab_container {
	border: 1px solid #999;
	border-top: none;
	clear: both;
	float: left; 
	width: 100%;
	background: #fff;
	-moz-border-radius-bottomright: 5px;
	-khtml-border-radius-bottomright: 5px;
	-webkit-border-bottom-right-radius: 5px;
	-moz-border-radius-bottomleft: 5px;
	-khtml-border-radius-bottomleft: 5px;
	-webkit-border-bottom-left-radius: 5px;
}
.tab_content {
	padding: 0px 20px;
	font-size: 1.2em;
}
.tab_content h2 {
	font-weight: normal;
	padding-bottom: 10px;
	border-bottom: 1px dashed #ddd;
	font-size: 1.8em;
}
.tab_content h3 a{
	color: #254588;
}
.tab_content img {
	position:relative;
	left:-19px;
	margin: 0 0px 0px 0;
	border: 1px solid #ddd;
	padding: 5px;
}
</style>

<script type="text/javascript">

jQuery(document).ready(function() {

	//Default Action
	jQuery(".tab_content").hide(); //Hide all content
	jQuery("ul.tabs li:first").addClass("active").show(); //Activate first tab
	jQuery(".tab_content:first").show(); //Show first tab content
	
	//On Click Event
	jQuery("ul.tabs li").click(function() {
		jQuery("ul.tabs li").removeClass("active"); //Remove any "active" class
		jQuery(this).addClass("active"); //Add "active" class to selected tab
		jQuery(".tab_content").hide(); //Hide all tab content
		var activeTab = jQuery(this).find("a").attr("href"); //Find the rel attribute value to identify the active tab + content
		jQuery(activeTab).fadeIn(); //Fade in the active content
		return false;
	});

});

</script>
	
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<noscript>
<div class="alert" ALIGN=CENTER><img src="../themes/<?php echo $g['theme']; ?>/images/icons/icon_alert.gif" /><strong> Please enable JavaScript to view this content</strong></div>
</noscript>

<!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td class="tabnavtbl">

		<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
			<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
			<li><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
			<li><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
			<li><a href="/snort/snort_interfaces_rules.php"><span>RulesDB</span></a></li>
			<li><a href="/snort/snort_alerts.php"><span>Alerts</span></a></li>
			<li><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
			<li><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
			<li><a href="/snort/snort_interfaces_suppress.php"><span>Suppress</span></a></li>
			<li class="newtabmenu_active"><a href="/snort/snort_help_info.php"><span>Help</span></a></li>			
		</ul>
		</div>

		</td>
	</tr>
	<tr>
		<td id="tdbggrey">		
		<table width="100%" border="0" cellpadding="10px" cellspacing="0">
		<tr>
		<td class="tabnavtbl">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<!-- START MAIN AREA -->

	<div class="container2">
    <ul class="tabs">
        <li><a href="#tab1">Home</a></li>
        <li><a href="#tab2">Change Log</a></li>
        <li><a href="#tab3">Getting Help</a></li>
        <li><a href="#tab4">Heros</a></li>

    </ul>
    <div class="tab_container">
        <div id="tab1" class="tab_content">
        <h2><a href="#"> <img src="./images/logo.jpg" width="750px" height="76" ALT="Snort Package" /></a></h2>
			
    <p>           
	<font size="5"><strong>Snort Package</strong></font> is a GUI based front-end for Sourcefire's Snort ® IDS/IPS software. The Snort Package goal is to be
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
		
        <div id="tab2" class="tab_content">
            <h2><a href="#"> <img src="./images/logo.jpg" width="750px" height="76" ALT="Snort Package" /></a></h2>
			
			<p><font size="5"><strong>Change Log</strong></font></p>
            
            <p>Changes to this package can be viewed by following <a href="https://github.com/bsdperimeter/pfsense-packages/commits/master" target="_blank"><font size="2" color="#990000"><strong>pfSense packages repository</strong></font></a></p>

		</div>
		
        <div id="tab3" class="tab_content">
            <h2><a href="#"> <img src="./images/logo.jpg" width="750px" height="76" ALT="Snort Package" /></a></h2>
			
            <p><font size="5"><strong>Getting Help</strong></font></p>
            
<p>
<font size="2"><strong>Obtaining Support</strong></font><br>

We provide several means of obtaining support for pfSense.
</p>

<p>
<font color="#990000" size="4"><strong>Free Options</strong></font><br>
Our free options include our <a href="http://forum.pfsense.org/" target="_blank"><font color="#990000"><strong>forum</strong></font></a>, <a href="http://www.pfsense.org/index.php?option=com_content&task=view&id=66&Itemid=71" target="_blank"><font color="#990000"><strong>mailing list</strong></font></a> , and <a href="http://www.pfsense.org/index.php?option=com_content&task=view&id=64&Itemid=72" target="_blank"><font color="#990000"><strong>IRC channel</strong></font></a>. Before using any of these resources, please review the Project Rules below.
</p>

<p>
<font color="#990000" size="4"><strong>Commercial Support</strong></font><br>

<a href="https://portal.pfsense.org/index.php/support-subscription" target="_blank"><font color="#990000"><strong>Commercial support</strong></font></a> is available from the company founded by the founders of the pfSense project, <a href="http://www.bsdperimeter.com/" target="_blank"><font color="#990000"><strong>BSD Perimeter</strong></font></a>. Phone and email support is available for <a href="https://portal.pfsense.org/index.php/support-subscription" target="_blank"><font color="#990000"><strong>support subscribers</strong></font></a> only.
</p>

<p>
<font color="#990000" size="4"><strong>Project Rules</strong></font><br>
To keep things orderly, and be fair to everyone, we must enforce these rules. 
</p>

<p>
Please do not post support questions to the blog comments. The comments are for discussion of the post, and letting people ask questions there would make a mess of the purpose of those comments. Any support questions will not be moderator approved.
</p>

<p>
Please do not cross post questions between the forum and mailing list, unless your inquiry has gone unanswered for at least 24 hours. Do not bump your mailing list or forum posts for at least 24 hours. If you have not received a reply after more than 24 hours, you are welcome to bump your thread.
</p>

<p>
Please do not email individuals, the coreteam address, or private message people on the forum to ask questions. We provide a wide variety of means for obtaining help in a public forum, where it helps others who have the same questions in the future. We don't have enough time to answer all the questions our users post in the public forums, much less via email and private messages. Since we cannot possibly reply to everyone's email and private messages, to be fair we will not reply to anyone. Individual attention via phone and email support is available for commercial support customers. 
</p>          
        </div>
		
        <div id="tab4" class="tab_content">
            <h2><a href="#"> <img src="./images/logo.jpg" width="750px" height="76" ALT="Snort Package" /></a></h2>

			
            <p><font size="5"><strong>Heros</strong></font></p>
			
            <p>Pfsense Snort Package users who have cared enough to donate to this project. I can't thank you enough for all your help. With-out your support I would have stoped long time ago.</p>
			
			<p>If your not on this list PM me and I will add you. If you would like to be removed pm me and I will remove you.</p>
			
			<p><font size="5"><strong>Names</strong></font></p>
			
			<p>sandro tavella</p>
			<p>João Kemp Filho</p>

			<p>Julio Fumoso</p>
			<p>Rolland Hart</p>
			<p>DiMarco Technology Solutions Inc.</p>
			<p>Brett Burley</p>
			<p>Tomasz Iskra</p>
			<p>Bruno Buchschacher</p>

			<p>Marco Pannetto</p>
			<p>Christopher Weakland</p>
			<p>Antonio Riveros</p>
			<p>DigitalJer</p>
			<p>Serialdie</p>
			<p>Dlawley</p>

			<p>Onhel</p>
			<p>Jerrygoldsmith</p>

 
		</div>
    </div>
</div>
			
		<!-- STOP MAIN AREA -->
		</table>
		</td>
		</tr>			
		</table>
	</td>
	</tr>
</table>

</div>


<!-- footer do not touch below -->
<?php 
include("fend.inc"); 
echo $snort_custom_rnd_box;
?>


</body>
</html>
