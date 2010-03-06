<?php
/*
    status_ospfd.php
    Copyright (C) 2010 Nick Buraglio; nick@buraglio.com
	Copyright (C) 2010 Scott Ullrich <sullrich@pfsense.org>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INClUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

require("guiconfig.inc");

$pgtitle = "OpenOSPFD: Status";
include("head.inc");

?>

<html>
	<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
		<?php include("fbegin.inc"); ?>
		<p class="pgtitle"><?=$pgtitle?></font></p>
		<?php if ($savemsg) print_info_box($savemsg); ?>
		<div id="mainlevel">
			<table width="100%" border="0" cellpadding="0" cellspacing="0">
			<?php
				$tab_array = array();
				$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=openospfd.xml&id=0");
				$tab_array[] = array(gettext("Interfaces"), false, "/pkg.php?xml=openospfd_interfaces.xml");
				$tab_array[] = array(gettext("Status"), true, "/status_ospfd.php");
				display_top_tabs($tab_array);
			?>
			</table>
			<table width="100%" border="0" cellpadding="0" cellspacing="0">
			   <tr>
			     <td class="tabcont" >
			      <form action="status_ospfd.php" method="post">
			    </form>
			    </td>
			   </tr>
			   <tr>
			    <td class="tabcont" >
					<?php
						defCmdT("OpenOSPFD Summary","/usr/local/sbin/ospfctl show summary"); 
						defCmdT("OpenOSPFD Neighbors","/usr/local/sbin/ospfctl show neighbor"); 
						defCmdT("OpenOSPFD FIB","/usr/local/sbin/ospfctl show fib");
						defCmdT("OpenOSPFD RIB","/usr/local/sbin/ospfctl show rib"); 
						defCmdT("OpenOSPFD Interfaces","/usr/local/sbin/ospfctl show interfaces"); 
						defCmdT("OpenOSPFD Database","/usr/local/sbin/ospfctl show database"); 
					?>
					<div id="cmdspace" style="width:100%">
						<?php listCmds(); ?>		
						<?php execCmds(); ?>
					</div>
				</td>
			   </tr>
			</table>
		</div>
		<?php include("fend.inc"); ?>
	</body>
</html>
