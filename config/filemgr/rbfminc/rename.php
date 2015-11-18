<?php
/*
	rename.php
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
include("config.php");
include("session.php");

if ($user_login == 'ok') {
	include("functions.php");
?>

<html>
<head>
<title>Rename</title>
</head>
<body>
<script type="text/javascript">
//<![CDATA[
<?php
if ($_POST['o'] != $_POST['n']) {
	if (@rename($_POST['cf'].$_POST['o'], $_POST['cf'].$_POST['n'])) {
		if ($_POST['t'] == 'd') {
			echo "alert('Directory successfuly renamed from \"{$_POST['o']}\" to \"{$_POST['n']}\"');";
		} else {
			echo "alert('File successfuly renamed from \"{$_POST['o']}\" to \"{$_POST['n']}\"');";
		}
	} else {
		echo <<<EOD
		alert('Rename error');
		window.parent.location.href = window.parent.location.href;
EOD;
	}
}

?>
//]]>
</script>
</body>
</html>

<?php
}
?>
