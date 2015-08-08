<?php
/*
	vnstati.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2009 PerryMason
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
require_once("guiconfig.inc");
global $config;
include("head.inc");
echo '<body link="#0000CC" vlink="#0000CC" alink="#0000CC">';
include("fbegin.inc");
$aaaa = $config['installedpackages']['vnstat2']['config'][0]['vnstat_interface'];
$cccc = convert_real_interface_to_friendly_descr($aaaa);
$pgtitle = gettext("Vnstati info for $cccc ($aaaa)");
?>
<p style="text-align: center"><img src="vnstat2_img.php?image=newpicture1.png" alt="" style="border:1px solid black; center;" /></p>
<p style="text-align: center"><img src="vnstat2_img.php?image=newpicture2.png" alt="" style="border:1px solid black; center;" /></p>
<p style="text-align: center"><img src="vnstat2_img.php?image=newpicture3.png" alt="" style="border:1px solid black; center;" /></p>
<p style="text-align: center"><img src="vnstat2_img.php?image=newpicture4.png" alt="" style="border:1px solid black; center;" /></p>
<?php include("fend.inc"); ?>
</body>
</html>
