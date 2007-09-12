<?php
/* $Id$ */
/* ========================================================================== */
/*
    authng.inc
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

require_once("authng_authmethods.inc");
require_once("authng_backends.inc");
require_once("authng_peers.inc");

// TODO: Define user- and groupindex array

// get principal store type from config.xml
// TODO: needs to be defined in config.xml
//$principalStore = $config['system']['webgui']['principal_store'];
$principalStore = "xml";
// get PeerFactory instance
$peerFactory =& PeerFactory::getInstance();
// get the actual UserPeer that holds the user index
$userPeer =& $peerFactory->getUserPeerByPrincipalStore($principalStore);
// get the actual GroupPeer that holds the user index
$groupPeer =& $peerFactory->getGroupPeerByPrincipalStore($principalStore);
// get AuthMethodFactory instance
$authMethodFactory =& AuthMethodFactory::getInstance();
// get BackendFactory instance
$backendFactory =& BackendFactory::getInstance();
// get the actual auth method
$authMethod =& $authMethodFactory->getAuthMethodByName($config['system']['webgui']['auth_method']);
// get the actual backend
$backend =& $backendFactory->getBackendByName($config['system']['webgui']['backing_method']);

function syncPackageAuthNG() {
}

function installPackageAuthNG() {
    mwexec("cd / && /usr/bin/patch < /usr/local/pkg/authng-pfSenseHead.diff");
    mwexec("cd / && /usr/bin/patch < /usr/local/pkg/authng-fbegin.inc.diff");
    mwexec("cd / && /usr/bin/patch < /usr/local/pkg/authng-guiconfig.inc.diff");
    mwexec("cd / && /usr/bin/patch < /usr/local/pkg/authng-globals.inc.diff");
}

function deinstallPackageAuthNG() {
    mwexec("cd / && /usr/bin/patch -R < /usr/local/pkg/authng-pfSenseHead.diff");
    mwexec("cd / && /usr/bin/patch -R < /usr/local/pkg/authng-fbegin.inc.diff");
    mwexec("cd / && /usr/bin/patch -R < /usr/local/pkg/authng-guiconfig.inc.diff");
    mwexec("cd / && /usr/bin/patch -R < /usr/local/pkg/authng-globals.inc.diff");
}
?>