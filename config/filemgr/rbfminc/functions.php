<?php
/*
	functions.php
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
function RecursiveFolderDelete($folderPath) {
	if (is_dir($folderPath )) {
		foreach (scandir($folderPath) as $value) {
			if ($value != "." && $value != "..") {
				$value = $folderPath . "/" . $value;
				if (is_dir($value)) {
					RecursiveFolderDelete($value);
				} elseif (is_file($value)) {
					@unlink($value);
				}
			}
		}
		if (!@rmdir($folderPath)) {
			return FALSE;
		} else {
			return TRUE;
		}
	} else {
		return FALSE;
	}
}


function GetFilePerms($file) {
	$perms = fileperms($file);
	
	if (($perms & 0xC000) == 0xC000) {
		// Socket
		$info = 's';
	} elseif (($perms & 0xA000) == 0xA000) {
		// Symbolic Link
		$info = 'l';
	} elseif (($perms & 0x8000) == 0x8000) {
		// Regular
		$info = '-';
	} elseif (($perms & 0x6000) == 0x6000) {
		// Block special
		$info = 'b';
	} elseif (($perms & 0x4000) == 0x4000) {
		// Directory
		$info = 'd';
	} elseif (($perms & 0x2000) == 0x2000) {
		// Character special
		$info = 'c';
	} elseif (($perms & 0x1000) == 0x1000) {
		// FIFO pipe
		$info = 'p';
	} else {
		// Unknown
		$info = '?';
	}

	// Owner
	$info .= " " . (($perms & 0x0100) ? 'r' : '-');
	$info .= (($perms & 0x0080) ? 'w' : '-');
	$info .= (($perms & 0x0040) ? (($perms & 0x0800) ? 's' : 'x' ) : (($perms & 0x0800) ? 'S' : '-'));
	// Group
	$info .= " " . (($perms & 0x0020) ? 'r' : '-');
	$info .= (($perms & 0x0010) ? 'w' : '-');
	$info .= (($perms & 0x0008) ? (($perms & 0x0400) ? 's' : 'x' ) : (($perms & 0x0400) ? 'S' : '-'));
	// World
	$info .= " ".(($perms & 0x0004) ? 'r' : '-');
	$info .= (($perms & 0x0002) ? 'w' : '-');
	$info .= (($perms & 0x0001) ? (($perms & 0x0200) ? 't' : 'x' ) : (($perms & 0x0200) ? 'T' : '-'));

	$info = "[" . substr(sprintf('%o', fileperms($file)), -4) . "] " . $info;
	return $info;
}


function merge_filters($tag) {
	global $wp_filter, $merged_filters;

	if (isset($wp_filter['all']) && is_array($wp_filter['all'])) {
		$wp_filter[$tag] = array_merge($wp_filter['all'], (array) $wp_filter[$tag]);
	}

	if (isset($wp_filter[$tag])) {
		reset($wp_filter[$tag]);
		uksort($wp_filter[$tag], "strnatcasecmp");
	}
	$merged_filters[$tag] = true;
}


function apply_filters($tag, $string) {
	global $wp_filter, $merged_filters;

	if (!isset($merged_filters[$tag])) {
		merge_filters($tag);
	}

	if (!isset($wp_filter[$tag])) {
		return $string;
	}

	reset($wp_filter[$tag]);

	$args = func_get_args();

	do {
		foreach ((array)current($wp_filter[$tag]) as $the_) {
			if (!is_null($the_['function'])) {
				$args[1] = $string;
				$string = call_user_func_array($the_['function'], array_slice($args, 1, (int) $the_['accepted_args']));
			}
		}

	} while (next($wp_filter[$tag]) !== false);

	return $string;
} 


function wp_check_filetype($filename, $mimes = null) {
	// Accepted MIME types are set here as PCRE unless provided.
	$mimes = is_array($mimes) ? $mimes : apply_filters('upload_mimes', array(
		'jpg|jpeg|jpe' => 'image/jpeg',
		'gif' => 'image/gif',
		'png' => 'image/png',
		'bmp' => 'image/bmp',
		'tif|tiff' => 'image/tiff',
		'ico' => 'image/x-icon',
		'asf|asx|wax|wmv|wmx' => 'video/asf',
		'avi' => 'video/avi',
		'mov|qt' => 'video/quicktime',
		'mpeg|mpg|mpe' => 'video/mpeg',
		'txt|c|cc|h' => 'text/plain',
		'rtx' => 'text/richtext',
		'css' => 'text/css',
		'htm|html' => 'text/html',
		'php|php3|' => 'application/php',
		'mp3|mp4' => 'audio/mpeg',
		'ra|ram' => 'audio/x-realaudio',
		'wav' => 'audio/wav',
		'ogg' => 'audio/ogg',
		'mid|midi' => 'audio/midi',
		'wma' => 'audio/wma',
		'rtf' => 'application/rtf',
		'js' => 'application/javascript',
		'pdf' => 'application/pdf',
		'doc' => 'application/msword',
		'pot|pps|ppt' => 'application/vnd.ms-powerpoint',
		'wri' => 'application/vnd.ms-write',
		'xla|xls|xlt|xlw' => 'application/vnd.ms-excel',
		'mdb' => 'application/vnd.ms-access',
		'mpp' => 'application/vnd.ms-project',
		'swf' => 'application/x-shockwave-flash',
		'class' => 'application/java',
		'tar' => 'application/x-tar',
		'zip' => 'application/zip',
		'gz|gzip' => 'application/x-gzip',
		'exe' => 'application/x-msdownload',
		// openoffice formats
		'odt' => 'application/vnd.oasis.opendocument.text',
		'odp' => 'application/vnd.oasis.opendocument.presentation',
		'ods' => 'application/vnd.oasis.opendocument.spreadsheet',
		'odg' => 'application/vnd.oasis.opendocument.graphics',
		'odc' => 'application/vnd.oasis.opendocument.chart',
		'odb' => 'application/vnd.oasis.opendocument.database',
		'odf' => 'application/vnd.oasis.opendocument.formula',

	));

	$type = false;
	$ext = false;

	foreach ($mimes as $ext_preg => $mime_match) {
		$ext_preg = '!\.(' . $ext_preg . ')$!i';
		if (preg_match($ext_preg, $filename, $ext_matches)) {
			$type = $mime_match;
			$ext = $ext_matches[1];
			break;
		}
	}

	return compact('ext', 'type');
}



function roundsize($size) {
	$i=0;
	$iec = array("B", "Kb", "Mb", "Gb", "Tb");
	while (($size/1024)>1) {
		$size=$size/1024;
		$i++;
	}
	return(round($size, 1) . " " . $iec[$i]);
}

?>
