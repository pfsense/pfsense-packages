<?php
/*
	sarg_frame.php
	part of pfSense (http://www.pfsense.com/)
	Copyright (C) 2012 Marcello Coutinho <marcellocoutinho@gmail.com>
	based on varnish_view_config.
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

if(preg_match("/(\S+)\W(\w+.html)/",$_REQUEST['file'],$matches)){
	#https://192.168.1.1/sarg_reports.php?file=2012Mar30-2012Mar30/index.html
	$url=$matches[2];
	$prefix=$matches[1];
	}
else{
	$url="index.html";
	$prefix="";
	}
$url=($_REQUEST['file'] == ""?"index.html":$_REQUEST['file']);
if (file_exists("/usr/local/www/sarg-reports/".$url))
	{
	$report=file_get_contents("/usr/local/www/sarg-reports/".$url);
	$pattern[0]="/href=\W(\S+html)\W/";
	$replace[0]="href=/sarg_frame.php?file=$prefix/$1";
	$pattern[1]='/img src="(\w+\.\w+)/';
	$replace[1]='img src="/sarg-reports'.$prefix.'/$1';
	$pattern[2]='@img src="([.a-z/]+)/(\w+\.\w+)@';
	$replace[2]='img src="/sarg-reports'.$prefix.'/$1/$2';
	$pattern[3]='/<head>/';
	$replace[3]='<head><META HTTP-EQUIV="CACHE-CONTROL" CONTENT="NO-CACHE"><META HTTP-EQUIV="PRAGMA" CONTENT="NO-CACHE">';
	print preg_replace($pattern,$replace,$report);
	}
else{
	print "<pre>Error: Could not find report index file.<br>Check sarg settings and try to force sarg schedule.";
	}		

?>