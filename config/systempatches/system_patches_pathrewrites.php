<?php
/*
	system_patches.php
	Copyright (C) 2013 PiBa-NL
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
/*
	pfSense_MODULE:	system
*/

##|+PRIV
##|*IDENT=page-system-patches
##|*NAME=System: Patches
##|*DESCR=Allow access to the 'System: Patches' page.
##|*MATCH=system_patches.php*
##|-PRIV

require("guiconfig.inc");
require_once("functions.inc");
require_once("itemid.inc");
require_once("patches.inc");

if (!is_array($config['installedpackages']['patches']['item']))
	$config['installedpackages']['patches']['item'] = array();

$patches_config = &$config['installedpackages']['patches'];

/* if a custom message has been passed along, lets process it */
if ($_GET['savemsg'])
	$savemsg = htmlspecialchars($_GET['savemsg']);

if ($_POST) {
	$pconfig = $_POST;
	if (!empty($_POST['pathrewrites'])) {
		/* Strip DOS style carriage returns from textarea input */
		$patches_config['pathrewrites'] = base64_encode(str_replace("\r", "", $pconfig['pathrewrites']));
	}
	write_config("Patches path rewrite files saved");
}

$closehead = false;
$pgtitle = array(gettext("System"),gettext("Patches"),gettext("Path rewrites"));
include("head.inc");

?>
<script type="text/javascript" src="/javascript/domTT/domLib.js"></script>
<script type="text/javascript" src="/javascript/domTT/domTT.js"></script>
<script type="text/javascript" src="/javascript/domTT/behaviour.js"></script>
<script type="text/javascript" src="/javascript/domTT/fadomatic.js"></script>

<link type="text/css" rel="stylesheet" href="/javascript/chosen/chosen.css" />
</head>
<body link="#000000" vlink="#000000" alink="#000000">
<?php include("fbegin.inc"); ?>
<form action="system_patches_pathrewrites.php" method="post" name="iform">
<script type="text/javascript" language="javascript" src="/javascript/row_toggle.js"></script>
<?php if ($savemsg) print_info_box($savemsg); ?>
	<table width="100%" border="0" cellpadding="0" cellspacing="0" summary="system patches">
	<tr><td class="tabnavtbl">
		<?php
		/* active tabs */
		$tab_array = array();
		$tab_array[] = array("Patches", false, "system_patches.php");
		$tab_array[] = array("Path rewrites", true, "system_patches_pathrewrites.php");
		display_top_tabs($tab_array);
		?>
		</td>
	</tr>
	<tr><td><div id="mainarea">
		<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0" summary="main area">
		<tr><td colspan="8">
			<?php 
			echo gettext("This page allows you to set 'path rewrites' for files inside patches, set the 'rewrite path' checkbox in the patch and fill this memobox with the proper patchpath|realpath values seperated by pipes. Each file should be on a seperate line.");
			echo gettext("<br/><br/>Example:");
			?>
			<br/>
			<b>config/haproxy-devel/haproxy.inc|usr/local/pkg/haproxy.inc<br/>
			config/haproxy-devel/haproxy_global.php|usr/local/www/haproxy_global.php</b><br/>
			<br/><br/>
			</td>
		</tr>
		<tr>
			<td>
				<?=gettext("Path rewrites:"); ?><br/>
				<textarea name="pathrewrites" class="" id="pathrewrites" rows="15" cols="90" wrap="off"><?=htmlspecialchars(base64_decode($patches_config['pathrewrites']));?></textarea>
			</td>
		</tr>
		<tr>
			<td align="center">
				<input name="Submit" type="submit" class="formbtn" value="<?=gettext("Save"); ?>" />
			</td>
		</tr>
		</table>
		</div></td>
	</tr>
	</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
