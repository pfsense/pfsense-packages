<?php
/*
	download.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2010 Tom Schaefer <tom@tomschaefer.org>
	Copyright (C) 2015 ESF, LLC
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
include_once("auth.inc");
include("functions.php");

// Set the cache policy
ob_end_clean();
header("Expires: Sat, 26 Jul 1997 05:00:00 GMT");
header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");

// Gets the parameters
$_GET['file_name'] = urldecode($_GET['file_name']);
$_GET['p'] = urldecode($_GET['p']);

// Check Authentication
$candownload = false;
$candownload = session_auth();
if ($candownload) {
	if (($_GET['file_name']) && ($_GET['p'])) {
		$filepath = $_GET['p'].$_GET['file_name'];
		if (file_exists($filepath)) {
			$type = wp_check_filetype($_GET['file_name']);
			header('Content-type: ' . $type[$_GET['file_name']]);
			header('Content-Disposition: attachment; filename="'.$_GET['file_name'].'"');
			header('Content-Length: ' . filesize($filepath));
			header('Last-Modified: '.gmdate('D, d M Y H:i:s', filemtime($filepath)).' GMT', true, 200);
			flush();
			readfile($filepath);
			exit;
		} else {
			echo "File not found";
		}
	} else {
		echo "File Unknown";
	}
} else {
	echo "Session Expired";
}

?>
