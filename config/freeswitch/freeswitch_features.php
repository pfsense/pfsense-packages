<?php
/* $Id$ */
/*
	freeswitch_extensions.php
  Copyright (C) 2008 Mark J Crane
  All rights reserved.
  
  FreeSWITCH (TM)
  http://www.freeswitch.org/

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

require("guiconfig.inc");
require("/usr/local/pkg/freeswitch.inc");

$a_extensions 	   = &$config['installedpackages']['freeswitchextensions']['config'];


if ($_GET['act'] == "del") {
    if ($_GET['type'] == 'extensions') {
        if ($a_extensions[$_GET['id']]) {
            unset($a_extensions[$_GET['id']]);
            write_config();
            header("Location: freeswitch_extensions.php");
            exit;
        }
    }
}

include("head.inc");

?>


<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle">FreeSWITCH: Extensions</p>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabnavtbl">
<?php
			
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=freeswitch.xml&amp;id=0");
	$tab_array[] = array(gettext("Dialplan"), false, "/freeswitch/freeswitch_dialplan_includes.php");
	$tab_array[] = array(gettext("Extensions"), false, "/freeswitch/freeswitch_extensions.php");
	$tab_array[] = array(gettext("External"), false, "/pkg_edit.php?xml=freeswitch_external.xml&amp;id=0");
	$tab_array[] = array(gettext("Features"), true, "/freeswitch/freeswitch_features.php");	
	$tab_array[] = array(gettext("Gateways"), false, "/freeswitch/freeswitch_gateways.php");
	$tab_array[] = array(gettext("Internal"), false, "/pkg_edit.php?xml=freeswitch_internal.xml&amp;id=0");
	$tab_array[] = array(gettext("Public"), false, "/freeswitch/freeswitch_public_includes.php");	
	$tab_array[] = array(gettext("Status"), false, "/freeswitch/freeswitch_status.php");
	$tab_array[] = array(gettext("Vars"), false, "/pkg_edit.php?xml=freeswitch_vars.xml&amp;id=0");
 	display_top_tabs($tab_array);
 	
?>
</td></tr>
</table>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
   <tr>
     <td class="tabcont" >

  	<table width="100%" border="0" cellpadding="6" cellspacing="0">              
      <tr>
        <td><p><span class="vexpl"><span class="red"><strong>Features<br>
            </strong></span>
            List of a few of the features.
            </p></td>
      </tr>
    </table>
    <br />
		
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
    <tr>
      <td class="listtopic" colspan='2'>Auto Attendant</td>
	</tr>
    <tr>
		<td width='10%' class="vncell"><a href='/freeswitch/freeswitch_ivr.php'>Open</a></td>
		<td class="vtable">          
			An interactive voice response (IVR) often refered to as an Auto Attendant. 
			It associates a recording to multiple options that can be used to direct 
			calls to extensions, voicemail, queues, other IVR applications, and external 
			phone numbers. 
		</td>
     </tr>
     </table>

    <br />
    <br />

    <table width="100%" border="0" cellpadding="6" cellspacing="0">
    <tr>
      <td class="listtopic" colspan='2'>Modules</td>
	</tr>
    <tr>
		<td width='10%' class="vncell"><a href='/pkg_edit.php?xml=freeswitch_modules.xml&amp;id=0'>Open</a></td>
		<td class="vtable">          
			Modules add additional features and can be enabled or disabled to provide the desired features.
		</td>
     </tr>
     </table>
	 
    <br />
    <br />
	
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
    <tr>
      <td class="listtopic" colspan='2'>Music on Hold</td>
	</tr>
    <tr>
		<td width='10%' class="vncell"><a href='/freeswitch/freeswitch_recordings.php'>Open</a></td>
		<td class="vtable">          
			Music on hold can be in WAV or MP3 format. To play MP3 you must have mod_shout enabled 
			on the 'Modules' tab. For best performance upload 16bit 8khz/16khz Mono WAV files. 
		</td>
     </tr>
     </table>
	 
    <br />
    <br />
	
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
	<tr>
	  <td class="listtopic" colspan='2'>Recordings</td>
	</tr>
	<tr>
		<td width='10%' class="vncell"><a href='/freeswitch/freeswitch_recordings.php'>Open</a></td>
		<td class="vtable">          
			To make a recording dial extension 732673 (record) or you can make a 16bit 8khz/16khz 
			Mono WAV file then copy it to the following directory then refresh the page to play 
			it back. Click on the 'Filename' to download it or the 'Recording Name' to play the audio. 
		</td>
	 </tr>
	 </table>	 
	 
    <br />
    <br />

<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>

</td>
</tr>
</table>

</div>


<?php include("fend.inc"); ?>
</body>
</html>
