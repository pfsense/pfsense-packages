<?php
/*
	dansguardian_about.php
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

require_once("guiconfig.inc");

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

$pgtitle = "About: Dansguardian Package";
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
	$tab_array[] = array(gettext("Daemon"), false, "/pkg_edit.php?xml=dansguardian.xml&id=0");
	$tab_array[] = array(gettext("General"), false, "/pkg_edit.php?xml=dansguardian_config.xml&id=0");
	$tab_array[] = array(gettext("Limits"), false, "/pkg_edit.php?xml=dansguardian_limits.xml&id=0");
	$tab_array[] = array(gettext("Blacklist"), false, "/pkg_edit.php?xml=dansguardian_blacklist.xml&id=0");
	$tab_array[] = array(gettext("ACLs"), false, "/pkg.php?xml=dansguardian_site_acl.xml");
	$tab_array[] = array(gettext("LDAP"), false, "/pkg.php?xml=dansguardian_ldap.xml&id=0");
	$tab_array[] = array(gettext("Groups"), false, "/pkg.php?xml=dansguardian_groups.xml&id=0");
	$tab_array[] = array(gettext("Users"), false, "/pkg_edit.php?xml=dansguardian_users.xml&id=0");
	$tab_array[] = array(gettext("IPs"), false, "/pkg_edit.php?xml=dansguardian_ips.xml&id=0");
	$tab_array[] = array(gettext("Report and Log"), false, "/pkg_edit.php?xml=dansguardian_log.xml&id=0");
	$tab_array[] = array(gettext("Sync"), false, "/pkg_edit.php?xml=dansguardian_sync.xml&id=0");
	$tab_array[] = array(gettext("Help"), true, "/dansguardian_about.php");
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
                        <td width="22%" valign="top" class="vncell"><?=gettext("Copyright");?></td>
						<td width="78%" class="vtable"><?=gettext("<a target=_new href='http://dansguardian.org/?page=copyright2'>Copyright and licensing for Dansguardian 2</a><br><br>");?>
                        </tr>
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Blacklists");?></td>
						<td width="78%" class="vtable"><?=gettext("<a target=_new href='http://www.squidguard.org/blacklists.html'>Dansguardian Blacklists</a><br><br>");?>
                        </tr>
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Whatis");?></td>
						<td width="78%" class="vtable"><?=gettext("<a target=_new href='http://dansguardian.org/?page=whatisdg'>What is Dansguardian</a><br><br>");?>
                        </tr>
                        <tr>
						<td colspan="2" valign="top" class="listtopic"><?=gettext("About dansguardian package"); ?></td>
						</tr>
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Credits ");?></td>
                        <td width="78%" class="vtable"><?=gettext("Package Created by <a target=_new href='https://forum.pfsense.org/index.php?action=profile;u=4710'>Marcello Coutinho</a><br><br>");?></td>
                        </tr>
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Donations ");?></td>
                        <td width="78%" class="vtable"><?=gettext("If you like this package, please <a target=_new href='https://www.pfsense.org/index.php?option=com_content&task=view&id=47&Itemid=77'>donate to the pfSense project</a>.<br><br>
								 If you want your donation to go to this package developer, make a note on the donation forwarding it to me.<br><br>");?></td>
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
