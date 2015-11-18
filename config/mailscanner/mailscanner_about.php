<?php
/*
	mailscanner_about.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2011 Marcello Coutinho <marcellocoutinho@gmail.com>
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

require("guiconfig.inc");

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

$pgtitle = "About: Mailscanner Package";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<?php endif; ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>


<div id="mainlevel">
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr><td>
		<?php
	$tab_array = array();
	$tab_array[] = array(gettext("General"), false, "/pkg_edit.php?xml=mailscanner.xml&id=0");
	$tab_array[] = array(gettext("Attachments"), false, "/pkg_edit.php?xml=mailscanner_attachments.xml&id=0");
	$tab_array[] = array(gettext("Antivirus"), false, "/pkg_edit.php?xml=mailscanner_antivirus.xml&id=0");
	$tab_array[] = array(gettext("Content"), false, "/pkg_edit.php?xml=mailscanner_content.xml&id=0");
	$tab_array[] = array(gettext("AntiSpam"), false, "/pkg_edit.php?xml=mailscanner_antispam.xml&id=0");
	$tab_array[] = array(gettext("Alerts"), false, "/pkg_edit.php?xml=mailscanner_alerts.xml&id=0");
	$tab_array[] = array(gettext("Reporting"), false, "/pkg_edit.php?xml=mailscanner_report.xml&id=0");
	$tab_array[] = array(gettext("XMLRPC Sync"), false, "/pkg_edit.php?xml=mailscanner_sync.xml&id=0");
	$tab_array[] = array(gettext("Help"), true, "/mailscanner_about.php");
	display_top_tabs($tab_array);
?>
		</td></tr>
 		<tr>
 		
    		<td>
				<div id="mainarea">
					<table class="tabcont" width="100%" border="0" cellpadding="8" cellspacing="0">
					<tr><td></td></tr>
						<tr>
						<td colspan="2" valign="top" class="listtopic"><?=gettext("Help docs"); ?></td>
						</tr>
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("FAQ ");?></td>
						<td width="78%" class="vtable"><?=gettext("<a target=_new href='http://www.mailscanner.info/wiki/doku.php?id=maq:index'>Most Asked Questions</a><br><br>");?>
                        </tr>
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Help Index");?></td>
						<td width="78%" class="vtable"><?=gettext("<a target=_new href='http://www.mailscanner.info/MailScanner.conf.index.html'>Mailscanner Configuration Index</a><br><br>");?>
                        </tr>
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Getting the best");?></td>
						<td width="78%" class="vtable"><?=gettext("<a target=_new href='http://www.mailscanner.info/gettingthebest.html'>Getting The Best Out Of MailScanner</a><br><br>");?>
                        </tr>
                        
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Performance ");?></td>
                        <td width="78%" class="vtable"><?=gettext("<a target=_new href='http://wiki.apache.org/spamassassin/FasterPerformance'>How do I get SpamAssassin to run faster?</a><br><br>");?></td>
                        </tr>
                        <tr>
						<td colspan="2" valign="top" class="listtopic"><?=gettext("About Mailscanner package"); ?></td>
						</tr>
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Credits ");?></td>
                        <td width="78%" class="vtable"><?=gettext("Package Created by <a target=_new href='https://forum.pfsense.org/index.php?action=profile;u=4710'>Marcello Coutinho</a><br><br>");?></td>
                        </tr>
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Donatios ");?></td>
                        <td width="78%" class="vtable"><?=gettext("If you like this package, please <a target=_new href='https://www.pfsense.org/index.php?option=com_content&task=view&id=47&Itemid=77'>donate to pfSense project</a>.<br><br>
								 If you want that your donation goes to this package developer, make a note on donation forwarding it to me.<br><br>");?></td>
                        </tr>
						</table>
						
				</div>
			</td>
		</tr>
	

	</table>
	<br>
	<div id="search_results"></div>
</div>
<?php include("fend.inc"); ?>
</body>
</html>
