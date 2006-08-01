<?php
/*
	diag_ping.php
	part of m0n0wall (http://m0n0.ch/wall)

	Copyright (C) 2003-2005 Bob Zoller (bob@kludgebox.com) and Manuel Kasper <mk@neon1.net>.
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

$pgtitle = array("Diagnostics", "Shell");
require("guiconfig.inc");

if ($_POST) {
	unset($input_errors);
  /* NOP */
}

include("head.inc"); ?>
<body link="#000000" vlink="#000000" alink="#000000">
<? include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></p>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
        <tr>
                <td>
<?php if ($input_errors) print_input_errors($input_errors); ?>
			<form action="diag_ping.php" method="post" name="iform" id="iform">
			  <table width="100%" border="0" cellpadding="6" cellspacing="0">
        <tr>
                <td>
<?php if ($input_errors) print_input_errors($input_errors); ?>
			<form action="diag_ping.php" method="post" name="iform" id="iform">
			  <table width="100%" border="0" cellpadding="6" cellspacing="0">
				<tr>
				  <td width="10%" valign="top" class="vncellreq">SSH Terminal:</td>
				  <td align="left" valign="top" width="90%">
            <applet width="640"
                    height="480"
                    archive="SSHTermApplet-signed.jar,SSHTermApplet-jdkbug-workaround-signed.jar"
                    code="com.sshtools.sshterm.SshTermApplet"
                    codebase="java"
                    style="border-style: solid; border-width: 1; padding-left: 4; padding-right: 4; padding-top: 1; padding-bottom: 1">
              <param name="sshapps.connection.host" value="<?= $config['interfaces']['lan']['ipaddr'] ?>">
              <param name="sshapps.connection.userName" value="root">
              <param name="sshapps.connection.authenticationMethod" value="password">
              <param name="sshapps.connection.connectImmediately" value="true">
              <param name="sshapps.connection.sshapps.connection.showConnectionDialog" value="false">
              <param name="sshterm.ui.scrollBar" value="true">
              <param name="sshapps.ui.toolBar" value="false">
              <param name="sshapps.ui.menuBar" value="true">
              <param name="sshapps.ui.statusBar" value="true">
              <param name="sshapps.ui.disabledActions" value="Open,About">

            </applet>
    </applet>
				 </td>
				</tr>		
			</table>
</form>
</td></tr></table>
<?php include("fend.inc"); ?>
