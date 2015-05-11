<?php
/*
	e2guardian_about.php
	Copyright (C) 2015 Marcello Coutinho
	part of pfSense (http://www.pfSense.com)
   	All rights reserved.
                                                                              */
/* ========================================================================== */
/*
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
/* ========================================================================== */

require_once("guiconfig.inc");

$pgtitle = "About: E2guardian Package";
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
	$tab_array[] = array(gettext("Daemon"), false, "/pkg_edit.php?xml=e2guardian.xml&id=0");
	$tab_array[] = array(gettext("General"), false, "/pkg_edit.php?xml=e2guardian_config.xml&id=0");
	$tab_array[] = array(gettext("Limits"), false, "/pkg_edit.php?xml=e2guardian_limits.xml&id=0");
	$tab_array[] = array(gettext("Blacklist"), false, "/pkg_edit.php?xml=e2guardian_blacklist.xml&id=0");
	$tab_array[] = array(gettext("ACLs"), false, "/pkg.php?xml=e2guardian_site_acl.xml");
	$tab_array[] = array(gettext("LDAP"), false, "/pkg.php?xml=e2guardian_ldap.xml&id=0");
	$tab_array[] = array(gettext("Groups"), false, "/pkg.php?xml=e2guardian_groups.xml&id=0");
	$tab_array[] = array(gettext("Users"), false, "/pkg_edit.php?xml=e2guardian_users.xml&id=0");
	$tab_array[] = array(gettext("IPs"), false, "/pkg_edit.php?xml=e2guardian_ips.xml&id=0");
	$tab_array[] = array(gettext("Report and Log"), false, "/pkg_edit.php?xml=e2guardian_log.xml&id=0");
	$tab_array[] = array(gettext("Sync"), false, "/pkg_edit.php?xml=e2guardian_sync.xml&id=0");
	$tab_array[] = array(gettext("Help"), true, "/e2guardian_about.php");
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
                        <td width="22%" valign="top" class="vncell"><?=gettext("Blacklists");?></td>
						<td width="78%" class="vtable"><?=gettext("<a target=_new href='http://www.squidguard.org/blacklists.html'>E2guardian Blacklists</a><br><br>");?>
                        </tr>
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Whatis");?></td>
						<td width="78%" class="vtable"><?=gettext("<a target=_new href='http://e2guardian.org/'>What is E2guardian</a><br><br>");?>
                        </tr>
                        <tr>
						<td colspan="2" valign="top" class="listtopic"><?=gettext("About e2guardian pfSense package"); ?></td>
						</tr>
						<tr>
                        <td width="22%" valign="top" class="vncell"><?=gettext("Credits ");?></td>
                        <td width="78%" class="vtable"><?=gettext("Package Created by <a target=_new href='http://forum.pfsense.org/index.php?action=profile;u=4710'>Marcello Coutinho</a><br><br>");?></td>
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
