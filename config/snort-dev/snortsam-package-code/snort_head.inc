<?php
/* $Id$ */
/*

 part of pfSense
 All rights reserved.

 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 All rights reserved.

 Pfsense Old snort GUI 
 Copyright (C) 2006 Scott Ullrich.
 
 Pfsense snort GUI 
 Copyright (C) 2008-2012 Robert Zelaya.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. Neither the name of the pfSense nor the names of its contributors 
 may be used to endorse or promote products derived from this software without 
 specific prior written permission.

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

/*
	pfSense_MODULE:	header
*/

/* 
 *   if user has selected a custom template, use it.
 *   otherwise default to pfsense tempalte
 */
if (($g["disablethemeselection"] === true) && !empty($g["default_theme"]) && (is_dir($g["www_path"].'/themes/'.$g["default_theme"])))
	$g['theme'] = $g["default_theme"];
elseif($config['theme'] <> "" && (is_dir($g["www_path"].'/themes/'.$config['theme'])))
	$g['theme'] = $config['theme'];
else
	$g['theme'] = "pfsense";

/*
 *  If this device is an apple ipod/iphone
 *  switch the theme to one that works with it.
 */
$lowres_ua = array("iPhone","iPod", "iPad", "Android");
foreach($lowres_ua as $useragent)
	if(strstr($_SERVER['HTTP_USER_AGENT'], $useragent))
		$g['theme'] = empty($g['theme_lowres']) ? "pfsense" : $g['theme_lowres'];

$pagetitle = gentitle( $pgtitle );

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
	<title><?php echo($config['system']['hostname'] . "." . $config['system']['domain'] . " - " . $pagetitle); ?></title>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<link rel="apple-touch-icon" href="/themes/<?php echo $g['theme']; ?>/apple-touch-icon.png"/>
	
        <?php if (strpos($_SERVER["SCRIPT_FILENAME"], "wizard.php") !== false &&
                  file_exists("{$g['www_path']}/themes/{$g['theme']}/wizard.css")): ?>
					<?php echo "<style type=\"text/css\" src=\"/themes/{$g['theme']}/wizard.css\"></style>"; ?>
        <?php else: ?>
        	<link rel="stylesheet" href="/themes/<?php echo $g['theme']; ?>/all.css" media="all" />
        <?php endif; ?>
		<link rel="stylesheet" type="text/css" href="/niftycssCode.css">
		<link rel="stylesheet" type="text/css" href="/niftycssprintCode.css" media="print">
		<link rel="stylesheet" type="text/css" href="/themes/<?=$g['theme']?>/new_tab_menu.css" media="all">
		<script type="text/javascript" src="/javascript/niftyjsCode.js"></script>
		<script type="text/javascript">
			var theme = "<?php echo $g['theme']; ?>";
		</script>
		<?php echo "\t<script type=\"text/javascript\" src=\"/themes/{$g['theme']}/loader.js\"></script>\n"; ?>	
		
<?php
	//<!-- snort custom javascript and css -->
	echo "\n";
	include('/usr/local/pkg/snort/snort_headbase.inc');
	echo "\n";
	//<!-- snort custom javascript and css -->
?>		
		
<?php
		if($_GET['enablefirebuglite']) {
			echo "\t<script type=\"text/javascript\" src=\"/javascript/pi.js\"></script>\n";
			echo "\t<script type=\"text/javascript\" src=\"/javascript/firebug-lite.js\"></script>\n";
		}
		echo "\t<script type=\"text/javascript\" src=\"/javascript/scriptaculous/prototype.js\"></script>\n";
		echo "\t<script type=\"text/javascript\" src=\"/javascript/scriptaculous/scriptaculous.js\"></script>\n";
		echo "\t<script type=\"text/javascript\" src=\"/javascript/scriptaculous/effects.js\"></script>\n";
		echo "\t<script type=\"text/javascript\" src=\"/javascript/scriptaculous/dragdrop.js\"></script>\n";
		if(file_exists("{$g['www_path']}/javascript/global.js"))
			echo "\t<script type=\"text/javascript\" src=\"/javascript/global.js\"></script>\n";
	/*
	 *	Find all javascript files that need to be included
	 *	for this page ... from the arrays ... :)
	 *	Coded by: Erik Kristensen
	 */

	$dir  = trim(basename($_SERVER["SCRIPT_FILENAME"], '.php'));
	$path = "{$g['www_path']}/javascript/" . $dir . "/";
	if (is_dir($path)) {
		if ($dh = opendir($path)) {
			while (($file = readdir($dh)) !== false) {
		   		if (is_dir($file)) 
					continue;
				echo "\t<script type=\"text/javascript\" src=\"/javascript/{$dir}/{$file}\"></script>\n";
			}
			closedir($dh);
		}
	}
	
	
if (!isset($closehead))
	echo "</head>";

/*  If this page is being remotely managed then do not allow the loading of the contents. */
if($config['remote_managed_pages']['item']) {
	foreach($config['remote_managed_pages']['item'] as $rmp) {
		if($rmp == $_SERVER['SCRIPT_NAME']) {
			include("fbegin.inc");
			print_info_box_np("This page is currently being managed by a remote machine.");
			include("fend.inc");
			exit;
		}
	}	
}

?>
