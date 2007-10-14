<?php
/* $Id$ */
/* ========================================================================== */
/*
    dyntables.inc
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

require_once("dyntables_classdefs.inc");

function syncPackageDyntables() {

}

function installPackageDyntables() {

}

function deinstallPackageDyntables() {

}

function checkForExtension($alertClass) {
    if(! extension_loaded( 'json' )) {
        if(! @dl( 'json.so' )) {
            return openNoExtDialog($alertClass);
        }
    }
}

/**
 * openNoExtDialog()
 *
 * @param mixed $effectClass
 * @return
 */
function openNoExtDialog($effectClass) {
    $alertMessage = gettext("PHP extension json.so was not found or could not be loaded. Please check you PHP installation.");
    $dialogScript = "
        <script type='text/javascript'>
            var anchor = document.getElementById('popupanchor');

            function openNoExtDialog(html) {
                var effect = new PopupEffect(html, {className: '${effectClass}'});
                Dialog.alert('${alertMessage}',{className:'alphacube', top:150, width:400, height:null, showEffect:effect.show.bind(effect), hideEffect:effect.hide.bind(effect)});
            }

            openNoExtDialog(anchor);
        </script>
    ";

    return $dialogScript;
}

/**
* getWindowJSScriptRefs()
*
* @return an array of &lt;script /&gt; elements
*/
function getWindowJSScriptRefs(){
  $result = array('<script type="text/javascript" src="/javascript/windows-js/javascript/window.js"></script>',
                  '<script type="text/javascript" src="/javascript/windows-js/javascript/window_effects.js"></script>',
                  '<script type="text/javascript" src="/javascript/windows-js/javascript/debug.js"></script>');

  return $result;
}

/**
* getWindowJSStyleRefs()
*
* @return an array of &lt;style /&gt; tags
*/
function getWindowJSStyleRefs(){
  $result = array('<link href="/javascript/windows-js/themes/default.css" rel="stylesheet" type="text/css" />',
                  '<link href="/javascript/windows-js/themes/alert.css" rel="stylesheet" type="text/css" />',
                  '<link href="/javascript/windows-js/themes/alphacube.css" rel="stylesheet" type="text/css" />');

  return $result;
}

?>