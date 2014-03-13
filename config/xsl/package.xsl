<?xml version="1.0" encoding="utf-8"?>
<!--
/* $Id$ */
/* ========================================================================== */
/*
    package.xsl
    part of pfSense (https://www.pfsense.org)
    Copyright (C) 2004-2014 Electric Sheep Fencing, LLC
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
-->
<xsl:stylesheet
    version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns="http://www.w3.org/1999/xhtml"
>
    <xsl:output
        method="xml"
        doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"
        doctype-public="-//W3C//DTD XHTML 1.1//EN"
    />
 
    <xsl:template match="/packagegui">
        <html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
            <head>
                <meta name="DC.title">
                    <xsl:attribute name="content"><xsl:value-of select="//name"/></xsl:attribute>
                </meta>
                <meta name="DC.creator" content="Daniel S. Haischt" />
                <meta name="DC.subject" content="Meta Data" />
                <meta name="DC.description" content="Next gen user and group manager for pfSense" />
                <meta name="DC.publisher" content="pfSense" />
                <meta name="DC.date" content="2007-08-28T21:00:00+02:00" scheme="DCTERMS.W3CDTF" />
                <meta name="DC.type" content="Text" scheme="DCTERMS.DCMIType" />
                <meta name="DC.format" content="text/html" scheme="DCTERMS.IMT" />
                <meta name="DC.language" content="en" scheme="DCTERMS.RFC3066" />
                <meta name="DC.relation" content="http://dublincore.org/" scheme="DCTERMS.URI" />
                <meta name="DC.coverage" content="Munich" scheme="DCTERMS.TGN" />
                <meta name="DC.rights" content="All rights reserved" />
                <meta http-equiv="Keywords" content="bsd license, altq, traffic shaping, packet, rule, Linux, OpenBSD, DragonFlyBSD, freebsd 5.3, vpn, stateful failover, carp, packet filter, m0n0wall, firewall" />
                <style type="text/css">
                </style>
                <script type="text/javascript" language="utf-8">
                //<![CDATA[
                    function toggleContentItem(whichItem) {
                        var element = document.getElementById(whichItem);

                        element.style.visibility = 'visible';
                        element.style.display = 'block';
                        element.style.top = '0';
                        element.className = 'highLight';

                        if (whichItem != 'info-div') {
                            document.getElementById('info-div').style.visibility = 'hidden';
                            document.getElementById('info-div').style.display = 'none';
                            document.getElementById('info-div').className = '';
                        }
                        if (whichItem != 'license-div') {
                            document.getElementById('license-div').style.visibility = 'hidden';
                            document.getElementById('license-div').style.display = 'none';
                            document.getElementById('license-div').className = '';
                        }
                        if (whichItem != 'desc-div') {
                            document.getElementById('desc-div').style.visibility = 'hidden';
                            document.getElementById('desc-div').style.display = 'none';
                            document.getElementById('desc-div').className = '';
                        }
                        if (whichItem != 'req-div') {
                            document.getElementById('req-div').style.visibility = 'hidden';
                            document.getElementById('req-div').style.display = 'none';
                            document.getElementById('req-div').className = '';
                        }
                        if (whichItem != 'faq-div') {
                            document.getElementById('faq-div').style.visibility = 'hidden';
                            document.getElementById('faq-div').style.display = 'none';
                            document.getElementById('faq-div').className = '';
                        }
                        if (whichItem != 'files-div') {
                            document.getElementById('files-div').style.visibility = 'hidden';
                            document.getElementById('files-div').style.display = 'none';
                            document.getElementById('files-div').className = '';
                        }
                        if (whichItem != 'menu-div') {
                            document.getElementById('menu-div').style.visibility = 'hidden';
                            document.getElementById('menu-div').style.display = 'none';
                            document.getElementById('menu-div').className = '';
                        }
                        if (whichItem != 'tab-div') {
                            document.getElementById('tab-div').style.visibility = 'hidden';
                            document.getElementById('tab-div').style.display = 'none';
                            document.getElementById('tab-div').className = '';
                        }
                        if (whichItem != 'service-div') {
                            document.getElementById('service-div').style.visibility = 'hidden';
                            document.getElementById('service-div').style.display = 'none';
                            document.getElementById('service-div').className = '';
                        }
                        if (whichItem != 'rsync-div') {
                            document.getElementById('rsync-div').style.visibility = 'hidden';
                            document.getElementById('rsync-div').style.display = 'none';
                            document.getElementById('rsync-div').className = '';
                        }
                        if (whichItem != 'install-div') {
                            document.getElementById('install-div').style.visibility = 'hidden';
                            document.getElementById('install-div').style.display = 'none';
                            document.getElementById('install-div').className = '';
                        }
                        if (whichItem != 'deinstall-div') {
                            document.getElementById('deinstall-div').style.visibility = 'hidden';
                            document.getElementById('deinstall-div').style.display = 'none';
                            document.getElementById('deinstall-div').className = '';
                        }
                    }
                ]]>
                </script>
            </head>
            <body style="color: rgb(0, 0, 0); background-color: rgb(51, 51, 51);" alink="#cc0000" link="#cc0000" vlink="#cc0000">
                <table style="width: 802px; text-align: left; margin-left: auto; margin-right: auto;" border="0" cellpadding="0" cellspacing="0">
                    <tbody>
                        <tr>
                        </tr>
                        <tr>
                                <font color="#ffffff"><span class="headers"></span></font>
                            </td>
                        </tr>
                        <tr>
                            <td>
                                <table style="text-align: left; width: 802px;" border="0" cellpadding="0" cellspacing="0">
                                    <tbody>
                                        <tr>
                                            <td style="width: 200px; text-align: center; vertical-align: top;">
                                                <table style="width: 90%; text-align: left; margin-left: auto; margin-right: auto;" border="0" cellpadding="0" cellspacing="1">
                                                    <tbody>
                                                        <tr>
                                                            <td class="navigationHead" style="background-color: rgb(153, 0, 0);">Navigation</td>
                                                        </tr>
                                                        <tr style="padding: 0px; margin: 0px;">
                                                            <td height="100%" align="left" valign="top" class="navigation" style="padding: 0px; margin: 0px;">
                                                                <br />
                                                                <a href='#' id="infoa" onclick="toggleContentItem('info-div');">Info</a>
                                                                <a href='#' id="licensea" onclick="toggleContentItem('license-div');">License</a>
                                                                <a href='#' id="desca" onclick="toggleContentItem('desc-div');">Description</a>
                                                                <a href='#' id="reqa" onclick="toggleContentItem('req-div');">Minimum requirements</a>
                                                                <a href='#' id="faqa" onclick="toggleContentItem('faq-div');">FAQ</a>
                                                                <a href='#' id="addfilea" onclick="toggleContentItem('files-div');">Additional Files Being Installed</a>
                                                                <a href='#' id="addmenua" onclick="toggleContentItem('menu-div');">Menu Items Being Installed</a>
                                                                <a href='#' id="addtaba" onclick="toggleContentItem('tab-div');">Tabs Being Installed</a>
                                                                <a href='#' id="servicesa" onclick="toggleContentItem('service-div');">Services Being Installed</a>
                                                                <a href='#' id="rsynca" onclick="toggleContentItem('rsync-div');">custom_php_resync_config_command</a>
                                                                <a href='#' id="installa" onclick="toggleContentItem('install-div');">custom_php_install_command</a>
                                                                <a href='#' id="deinstalla" onclick="toggleContentItem('deinstall-div');">custom_php_deinstall_command</a>
                                                            </td>
                                                        </tr>
                                                    </tbody>
                                                </table>
                                            </td>
                                            <td style="text-align: left; vertical-align: top;">
                                                <div id="info-div" style="padding-left: 6px; width: 500px;">
                                                    <h1>
                                                        <xsl:text>Info for package: </xsl:text> 
                                                        <xsl:value-of select="//name"/>
                                                    </h1>
                                                    <h2>Version</h2>
                                                    <p>
                                                        <xsl:value-of select="//version" />
                                                    </p>
                                                    <h2>Title</h2>
                                                    <p>
                                                        <xsl:value-of select="//title" />
                                                    </p>
                                                    <h2>Include File</h2>
                                                    <p>
                                                        <xsl:value-of select="//include_file" />
                                                    </p>
                                                    <h2>Configpath</h2>
                                                    <p>
                                                        <code>
                                                            <xsl:value-of select="//configpath" />
                                                        </code>
                                                    </p>
                                                </div>
                                                <div id="license-div" style="padding-left: 6px; width: 500px; display: none; visibility: hidden;">
                                                    <h1>License</h1>
                                                    <p>
                                                        <pre style="font-size: 0.95em;">
                                                            <xsl:value-of select="//copyright" />
                                                        </pre>
                                                    </p>
                                                </div>
                                                <div id="desc-div" style="padding-left: 6px; width: 500px; display: none; visibility: hidden;">
                                                    <h1>Package Description</h1>
                                                    <xsl:value-of select="//description" />
                                                </div>
                                                <div id="req-div" style="padding-left: 6px; width: 500px; display: none; visibility: hidden;">
                                                    <h1>Requirements</h1>
                                                    <xsl:value-of select="//requirements" />
                                                </div>
                                                <div id="faq-div" style="padding-left: 6px; width: 500px; display: none; visibility: hidden;">
                                                    <h1>Frequently Asked Question</h1>
                                                    <xsl:value-of select="//faq" />
                                                </div>
                                                <div id="files-div" style="padding-left: 6px; width: 500px; display: none; visibility: hidden;">
                                                    <h1>Additional Files Being Installed</h1>
                                                    <xsl:for-each select="//additional_files_needed">
                                                        <p>
                                                            <b><xsl:text>Prefix: </xsl:text></b><xsl:value-of select="prefix" /><br />
                                                            <b><xsl:text>Chmod: </xsl:text></b><xsl:value-of select="chmod" /><br />
                                                            <b><xsl:text>Item: </xsl:text></b><xsl:value-of select="item" /><br />
                                                        </p>
                                                    </xsl:for-each>
                                                </div>
                                                <div id="menu-div" style="padding-left: 6px; width: 500px; display: none; visibility: hidden;">
                                                    <h1>Menu Items Being Installed</h1>
                                                    <xsl:for-each select="//menu">
                                                        <p>
                                                            <b><xsl:text>Name: </xsl:text></b><xsl:value-of select="name" /><br />
                                                            <b><xsl:text>Section: </xsl:text></b><xsl:value-of select="section" /><br />
                                                            <b><xsl:text>URL: </xsl:text></b><xsl:value-of select="url" /><br />
                                                        </p>
                                                    </xsl:for-each>
                                                </div> 
                                                <div id="tab-div" style="padding-left: 6px; width: 500px; display: none; visibility: hidden;">
                                                    <h1>Tabs Being Installed</h1>
                                                    <xsl:for-each select="//tabs/tab">
                                                        <p>
                                                            <b><xsl:text>Text: </xsl:text></b><xsl:value-of select="text" /><br />
                                                            <b><xsl:text>URL: </xsl:text></b><xsl:value-of select="url" /><br />
                                                            <xsl:if test="active">
                                                                <b><xsl:text>Active: </xsl:text></b><xsl:text>YES</xsl:text><br />
                                                            </xsl:if>
                                                        </p>
                                                    </xsl:for-each>
                                                </div>
                                                <div id="service-div" style="padding-left: 6px; width: 500px; display: none; visibility: hidden;">
                                                    <h1>Services Being Installed</h1>
                                                    <xsl:for-each select="//service">
                                                        <p>
                                                            <b><xsl:text>Name: </xsl:text></b><xsl:value-of select="name" /><br />
                                                            <b><xsl:text>RC File: </xsl:text></b><xsl:value-of select="rcfile" /><br />
                                                        </p>
                                                    </xsl:for-each>
                                                </div>
                                                <div id="rsync-div" style="padding-left: 6px; width: 500px; display: none; visibility: hidden;">
                                                    <h1>custom_php_install_command</h1>
                                                    <p>
                                                        <pre><xsl:value-of select="custom_php_install_command" /></pre>
                                                    </p>
                                                </div>
                                                <div id="install-div" style="padding-left: 6px; width: 500px; display: none; visibility: hidden;">
                                                    <h1>custom_php_install_command</h1>
                                                    <p>
                                                        <pre><xsl:value-of select="custom_php_deinstall_command" /></pre>
                                                    </p>
                                                </div>
                                                <div id="deinstall-div" style="padding-left: 6px; width: 500px; display: none; visibility: hidden;">
                                                    <h1>custom_php_deinstall_command</h1>
                                                    <p>
                                                        <pre><xsl:value-of select="custom_php_deinstall_command" /></pre>
                                                    </p>
                                                </div>
                                            </td>
                                            <td width="30px"></td>
                                        </tr>
                                    </tbody>
                                </table>
                            </td>
                        </tr>
                        <tr style="color: rgb(255, 255, 255);">
                                pfSense is Copyright 2004-2014 Electric Sheep Fencing LLC. All Rights Reserved.
                                <br />
                            </td>
                        </tr>
                    </tbody>
                </table>
            </body>
        </html>
    </xsl:template>
</xsl:stylesheet>

