<?php
/* $Id$ */
/* ========================================================================== */
/*
    system_usermanager.php
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2007 Daniel S. Haischt <me@daniel.stefan.haischt.name>
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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

require("guiconfig.inc");
// The page title for non-admins
$pgtitle = getUsermanagerPagetitle();

include("head.inc");

$effectStyle = '
    <style type="text/css">
        .popup_nousers {
            background: #000000;
            opacity: 0.2;
        }
    </style>
';
foreach(getWindowJSScriptRefs() as $jscript){
    $pfSenseHead->addScript($jscript);
}
foreach(getWindowJSStyleRefs() as $style){
    $pfSenseHead->addStyle($style);
}
$pfSenseHead->addStyle($effectStyle);
echo $pfSenseHead->getHTML();
?>
<body link="#000000" vlink="#000000" alink="#000000">
<?php include("fbegin.inc");?>
<p class="pgtitle"><?= gentitle($pgtitle); ?></p>
<form action="system_usermanager.php" method="post" name="iform" id="iform">
<?php if ($input_errors) print_input_errors($input_errors);?>
<?php if ($savemsg) print_info_box($savemsg);?>
<?php
    if (! gotNoUsers()) {
        if ($userPeer->isSystemAdmin($HTTP_SERVER_VARS['AUTH_USER'])) {
            processUserManagerAdminPostVars();
            require_once("system_usermanager_admin.inc");
        } else {
            processUserManagerPostVars();
            require_once("system_usermanager_user.inc");
        }
    }
?>
</form>
<div id="popupanchor">&#160;</div>
<?= openNoUserDefsDialog("popup_nousers"); ?>
<?php include("fend.inc");?>
</body>
</html>
