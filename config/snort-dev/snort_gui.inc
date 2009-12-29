<?php
/* $Id$ */
/*
	snort.inc
	Copyright (C) 2006 Scott Ullrich
	Copyright (C) 2006 Robert Zelaya
	part of pfSense
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

function print_info_box_np2($msg) {
        global $g;
        echo "<table height=\"32\" width=\"100%\">\n";
        echo "  <tr>\n";
        echo "   <td>\n";
        echo "      <div style='background-color:#990000' id='redbox'>\n";
        echo "       <table width='100%'><tr><td width='8%'>\n";
        echo "        &nbsp;&nbsp;&nbsp;<img style='vertical-align:middle' src=\"/snort/images/alert.jpg\" width=\"32\" height=\"28\">\n";
        echo "        </td>\n";
        echo "        <td width='70%'><font color='white'><b>{$msg}</b></font>\n";
        echo "        </td>";
        echo "        </tr></table>\n";
        echo "       </div>\n";
        echo "    </td>\n";
        echo "</table>\n";
        echo "<script type=\"text/javascript\">\n";
        echo "NiftyCheck();\n";
        echo "Rounded(\"div#redbox\",\"all\",\"#FFF\",\"#990000\",\"smooth\");\n";
        echo "Rounded(\"td#blackbox\",\"all\",\"#FFF\",\"#000000\",\"smooth\");\n";
        echo "</script>\n";
        echo "\n<br>\n";
}











?>