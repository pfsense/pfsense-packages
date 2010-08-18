<?php
/* $Id$ */
/*
	snort.inc
	Copyright (C) 2006 Scott Ullrich
	Copyright (C) 2006 Robert Zelaya
	part of pfSense
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.

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

include_once("/usr/local/pkg/snort/snort.inc");

function print_info_box_np2($msg) {
	global $config, $g;

	echo "<table height=\"32\" width=\"100%\">\n";
	echo "  <tr>\n";
	echo "   <td>\n";
	echo "      <div style='background-color:#990000' id='redbox'>\n";
	echo "       <table width='100%'><tr><td width='8%'>\n";
	echo "        &nbsp;&nbsp;&nbsp;<img style='vertical-align:middle' src=\"/snort/images/alert.jpg\" width=\"32\" height=\"28\">\n";
	echo "        </td>\n";
	echo "        <td width='70%'><font color='white'><b>{$msg}</b></font>\n";
	echo "        </td>";
	if(stristr($msg, "apply") == true) {
		echo "         <td>";
		echo "           <input name=\"apply\" type=\"submit\" class=\"formbtn\" id=\"apply\" value=\"Apply changes\">\n";
		echo "         </td>";
	}
	echo "        </tr></table>\n";
	echo "       </div>\n";
	echo "    </td>\n";
	echo "</table>\n";
	echo "<script type=\"text/javascript\">\n";
	echo "NiftyCheck();\n";
	echo "Rounded(\"div#redbox\",\"all\",\"#FFF\",\"#990000\",\"smooth\");\n";
	echo "Rounded(\"td#blackbox\",\"all\",\"#FFF\",\"#000000\",\"smooth\");\n";
	echo "</script>\n";
	echo "\n<br>\n";


}


/* makes boxes round */
/* load at bottom */

$snort_custom_rnd_box = '
<script type="text/javascript">
<!--

	NiftyCheck();
	Rounded("div#mainarea2","bl br tr","#FFF","#dddddd","smooth");
	Rounded("div#mainarea3","bl br tr","#FFF","#dddddd","smooth");
	Rounded("div#mainarea4","all","#FFF","#dddddd","smooth");
	Rounded("div#mainarea5","all","#eeeeee","#dddddd","smooth");

//-->
</script>' . "\n";

/* general css code */
$snort_general_css = '

<style type="text/css">

.alert {
 position:absolute;
 top:10px;
 left:0px;
 width:94%;
 height:90%;
 
background:#FCE9C0;
background-position: 15px; 
border-top:2px solid #DBAC48;
border-bottom:2px solid #DBAC48;
padding: 15px 10px 85% 50px;
}

.formpre {
font-family:arial;
font-size: 1.1em;
}

#download_rules {
font-family: arial;
font-size: 13px;
font-weight: bold;
text-align: center
}

#download_rules_td {
font-family: arial;
font-size: 13px;
font-weight: bold;
text-align: center
}

body2 {
font-family:arial;
font-size:12px;
}

.tabcont {
background-color: #dddddd;
padding-right: 12px;
padding-left: 12px;
padding-top: 12px;
padding-bottom: 12px;
}

.tabcont2 {
background-color: #eeeeee;
padding-right: 12px;
padding-left: 12px;
padding-top: 12px;
padding-bottom: 12px;
}

.vncell2 {
	background-color: #eeeeee;
	padding-right: 20px;
	padding-left: 8px;
	border-bottom: 1px solid #999999;
}

.vncellreq2 {
background-color: #eeeeee;
padding-right: 20px;
padding-left: 8px;
font-weight: bold;
border-bottom-width: 1px;
border-bottom-style: solid;
border-bottom-color: #999999;
}

</style> ' . "\n";


/* general css code for snort_interface.php */
$snort_interfaces_css = '

<style type="text/css">

.listbg2 {
	border-right: 1px solid #999999;
	border-bottom: 1px solid #999999;
	font-size: 11px;
	background-color: #090;
	color: #000;	
	padding-right: 16px;
	padding-left: 6px;
	padding-top: 4px;
	padding-bottom: 4px;
}

.listbg3 {
	border-right: 1px solid #999999;
	border-bottom: 1px solid #999999;
	font-size: 11px;
	background-color: #777777;
	color: #000;	
	padding-right: 16px;
	padding-left: 6px;
	padding-top: 4px;
	padding-bottom: 4px;
}

</style>' . "\n";

?>
