<?
/* $Id$ */
/*
	part of pfSense (http://www.pfsense.org/)

	Copyright (C) 2006 Bill Marquette - bill.marquette@gmail.com.
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

function nmap_custom_add_php_command() {
	$nmap_options = "";
	switch($_POST['scanmethod']) {
		case 'syn':
			$nmap_options .= " -sS";
			break;
		case 'connect':
			$nmap_options .= " -sT";
			break;
		case 'icmp':
			$nmap_options .= " -sP";
			break;
		case 'udp':
			$nmap_options .= " -sU";
			break;
	}
	
	if($_POST['noping']) $nmap_options .= " -P0";
	if($_POST['servicever']) $nmap_options .= " -sV";
	if($_POST['osdetect']) $nmap_options .= " -O";
	$nmap_options .= " " . $_POST['hostname'];
	system("/usr/local/bin/nmap" . $nmap_options);
}

?>
