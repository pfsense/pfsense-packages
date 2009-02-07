/* $Id$ */
/* ========================================================================== */
/*
    authng_wizard.js
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

function openInfoDialog() {
  Dialog.info("Reloading table data ...",
               {className:"alphacube", width:200, height:75, top:100, showProgress: true});
}

/**
 *
 * @access public
 * @return void
 **/
function getFirstTD(node, index){
    var firstTD = node.childNodes[index];

    if (firstTD.tagName == 'TD') {
        return firstTD;
    } else {
        return getFirstTD(node, ++index);
    }
}

/**
 *
 * @access public
 * @return void
 **/
function getTableRow(nodes, index){
    var row = nodes[index];

    if (row.tagName == 'TR') {
        return row;
    } else {
        return getTableRow(nodes, ++index);
    }
}

/**
 *
 * @access public
 * @return void
 **/
function emtyDhcpLeaseTable(table) {
    var tbody = table.childNodes[1];
    var trNodes = tbody.childNodes;

    for (i = 0; i < trNodes.length; i++) {
        var currentRow = getTableRow(trNodes, i);
        var firstTd = getFirstTD(currentRow, 0);
        var classAttrib = firstTd.className;

        if (classAttrib == 'listhdrr') {
            continue;
        } else if (currentRow.tagName == 'TR') {
            tbody.removeChild(currentRow);
        }
    }
}

/**
 * TODO: data[if], $fspans, $fspane,
 *
 **/
function dhcpLeaseTableToHTML(table, json, theme) {
    var tbody = table.childNodes[1];

    if (json && json.length > 0) {
        for (i = 0; i < json.length; i++) {
            /* text nodes */
            var newActTxt = document.createTextNode(json[i]['act']);
            var newEndTxt = document.createTextNode(json[i]['end']);
            var newStartTxt = document.createTextNode(json[i]['start']);
            var newHostnameTxt = document.createTextNode(json[i]['hostname']);
            var newIPTxt = document.createTextNode(json[i]['ip']);
            var newMACTxt = document.createTextNode(json[i]['mac']);
            var newOnlineTxt = document.createTextNode(json[i]['online']);
            var newTypeTxt = document.createTextNode(json[i]['type']);

            var newTR = document.createElement("tr");
            var newTRIdAttrib = document.createAttribute("id");
            newTRIdAttrib.nodeValue = "dhcpRow" + i;
            newTR.setAttributeNode(newTRIdAttrib);

            /* IP td element */
            var newIPTD = document.createElement("td");
            /* TD attributes */
            var newIPTDClassAttrib = document.createAttribute("class");
            newIPTDClassAttrib.nodeValue = "listlr";
            /* assign attribs */
            newIPTD.setAttributeNode(newIPTDClassAttrib);

            /* MAC td element */
            var newMACTD = document.createElement("td");
            /* TD attributes */
            var newMACTDClassAttrib = document.createAttribute("class");
            newMACTDClassAttrib.nodeValue = "listr";
            /* assign attribs */
            newMACTD.setAttributeNode(newMACTDClassAttrib);

            /* Hostname td element */
            var newHostnameTD = document.createElement("td");
            /* TD attributes */
            var newHostnameTDClassAttrib = document.createAttribute("class");
            newHostnameTDClassAttrib.nodeValue = "listr";
            /* assign attribs */
            newHostnameTD.setAttributeNode(newHostnameTDClassAttrib);

            /* Start td element */
            var newStartTD = document.createElement("td");
            /* TD attributes */
            var newStartTDClassAttrib = document.createAttribute("class");
            newStartTDClassAttrib.nodeValue = "listr";
            /* assign attribs */
            newStartTD.setAttributeNode(newStartTDClassAttrib);

            /* End td element */
            var newEndTD = document.createElement("td");
            /* TD attributes */
            var newEndTDClassAttrib = document.createAttribute("class");
            newEndTDClassAttrib.nodeValue = "listr";
            /* assign attribs */
            newEndTD.setAttributeNode(newEndTDClassAttrib);

            /* Online td element */
            var newOnlineTD = document.createElement("td");
            /* TD attributes */
            var newOnlineTDClassAttrib = document.createAttribute("class");
            newOnlineTDClassAttrib.nodeValue = "listr";
            /* assign attribs */
            newOnlineTD.setAttributeNode(newOnlineTDClassAttrib);

            /* Lease td element */
            var newLeaseTD = document.createElement("td");
            /* TD attributes */
            var newLeaseTDClassAttrib = document.createAttribute("class");
            newLeaseTDClassAttrib.nodeValue = "listr";
            /* assign attribs */
            newLeaseTD.setAttributeNode(newLeaseTDClassAttrib);

            /* Mapping td element */
            var newMappingTD = document.createElement("td");
            /* TD attributes */
            var newMappingTDClassAttrib = document.createAttribute("class");
            newMappingTDClassAttrib.nodeValue = "list";
            var newMappingTDValignAttrib = document.createAttribute("valign");
            newMappingTDValignAttrib.nodeValue = "middle";
            /* assign attribs */
            newMappingTD.setAttributeNode(newMappingTDClassAttrib);
            newMappingTD.setAttributeNode(newMappingTDValignAttrib);

            /* WOL td element */
            var newWOLTD = document.createElement("td");
            /* TD attributes */
            var newWOLTDValignAttrib = document.createAttribute("valign");
            newWOLTDValignAttrib.nodeValue = "middle";
            /* assign attribs */
            newWOLTD.setAttributeNode(newWOLTDValignAttrib);

            /* Mapping anchor */
            var newMappingAnchor = document.createElement("a");
            /* Anchor attribs */
            var newMappingAnchorHrefAttrib = document.createAttribute("href");
            newMappingAnchorHrefAttrib.nodeValue = "services_dhcp_edit.php?if=lan&mac=" + json[i]['mac'] + "&hostname=" + json[i]['hostname'];
            /* assign attribs */
            newMappingAnchor.setAttributeNode(newMappingAnchorHrefAttrib);

            /* Mapping button */
            var newMappingButton = document.createElement("img");
            /* Button attribs */
            var newMappingButtonSrcAttrib = document.createAttribute("src");
            newMappingButtonSrcAttrib.nodeValue = "/themes/" + theme + "/images/icons/icon_plus.gif";
            var newMappingButtonWidthAttrib = document.createAttribute("width");
            newMappingButtonWidthAttrib.nodeValue = "17";
            var newMappingButtonHeightAttrib = document.createAttribute("height");
            newMappingButtonHeightAttrib.nodeValue = "17";
            var newMappingButtonBorderAttrib = document.createAttribute("border");
            newMappingButtonBorderAttrib.nodeValue = "0";
            var newMappingButtonTitleAttrib = document.createAttribute("title");
            newMappingButtonTitleAttrib.nodeValue = "add a static mapping for this MAC address";
            var newMappingButtonAltAttrib = document.createAttribute("alt");
            newMappingButtonAltAttrib.nodeValue = "add a static mapping for this MAC address";
            /* assign attribs */
            newMappingButton.setAttributeNode(newMappingButtonSrcAttrib);
            newMappingButton.setAttributeNode(newMappingButtonWidthAttrib);
            newMappingButton.setAttributeNode(newMappingButtonHeightAttrib);
            newMappingButton.setAttributeNode(newMappingButtonBorderAttrib);
            newMappingButton.setAttributeNode(newMappingButtonTitleAttrib);
            newMappingButton.setAttributeNode(newMappingButtonAltAttrib);

            /* WOL anchor */
            var newWOLAnchor = document.createElement("a");
            /* Anchor attribs */
            var newWOLAnchorHrefAttrib = document.createAttribute("href");
            newWOLAnchorHrefAttrib.nodeValue = "services_wol_edit.php?if=lan&mac=" + json[i]['mac'] + "&descr=pfSense";
            /* assign attribs */
            newWOLAnchor.setAttributeNode(newWOLAnchorHrefAttrib);

            /* WOL button */
            var newWOLButton = document.createElement("img");
            /* Button attribs */
            var newWOLButtonSrcAttrib = document.createAttribute("src");
            newWOLButtonSrcAttrib.nodeValue = "/themes/" + theme + "/images/icons/icon_wol_all.gif";
            var newWOLButtonWidthAttrib = document.createAttribute("width");
            newWOLButtonWidthAttrib.nodeValue = "17";
            var newWOLButtonHeightAttrib = document.createAttribute("height");
            newWOLButtonHeightAttrib.nodeValue = "17";
            var newWOLButtonBorderAttrib = document.createAttribute("border");
            newWOLButtonBorderAttrib.nodeValue = "0";
            var newWOLButtonTitleAttrib = document.createAttribute("title");
            newWOLButtonTitleAttrib.nodeValue = "add a Wake on Lan mapping for this MAC address";
            var newWOLButtonAltAttrib = document.createAttribute("alt");
            newWOLButtonAltAttrib.nodeValue = "add a Wake on Lan mapping for this MAC address";
            /* assign attribs */
            newWOLButton.setAttributeNode(newWOLButtonSrcAttrib);
            newWOLButton.setAttributeNode(newWOLButtonWidthAttrib);
            newWOLButton.setAttributeNode(newWOLButtonHeightAttrib);
            newWOLButton.setAttributeNode(newWOLButtonBorderAttrib);
            newWOLButton.setAttributeNode(newWOLButtonTitleAttrib);
            newWOLButton.setAttributeNode(newWOLButtonAltAttrib);

            /* assign buttons to anchor elements */
            newMappingAnchor.appendChild(newMappingButton);
            newWOLAnchor.appendChild(newWOLButton);

            /* assign anchors to TD elements */
            newMappingTD.appendChild(newMappingAnchor);
            newWOLTD.appendChild(newWOLAnchor);

            /* assign text nodes to TD elements */
            newIPTD.appendChild(newIPTxt);
            newMACTD.appendChild(newMACTxt);
            newHostnameTD.appendChild(newHostnameTxt);
            newStartTD.appendChild(newStartTxt);
            newEndTD.appendChild(newEndTxt);
            newOnlineTD.appendChild(newOnlineTxt);
            newLeaseTD.appendChild(newActTxt);

            /* populate table body */
            newTR.appendChild(newIPTD);
            newTR.appendChild(newMACTD);
            newTR.appendChild(newHostnameTD);
            newTR.appendChild(newStartTD);
            newTR.appendChild(newEndTD);
            newTR.appendChild(newOnlineTD);
            newTR.appendChild(newLeaseTD);
            newTR.appendChild(newMappingTD);
            newTR.appendChild(newWOLTD);

            tbody.appendChild(newTR);
        }
    }
}