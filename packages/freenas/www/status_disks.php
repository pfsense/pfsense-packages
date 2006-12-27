<?php
/* $Id$ */
/*
	disks_manage_edit.php
	part of FreeNAS (http://www.freenas.org)
	Copyright (C) 2005-2006 Olivier Cochard-Labbé <olivier@freenas.org>.
	All rights reserved.
	
	Based on m0n0wall (http://m0n0.ch/wall)
	Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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

$pgtitle = array(gettext("Status"),
                 gettext("Disks"));

require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['disks']['disk']))
	$freenas_config['disks']['disk'] = array();
	
disks_sort();

$raidstatus=get_sraid_disks_list();

$a_disk_conf = &$freenas_config['disks']['disk'];

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php include("fbegin.inc"); ?>

<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
  <div id="inputerrors"></div>
  <form id="iform" name="iform" action="status_disks.php" method="post">
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
        <tr>
          <td width="5%" class="listhdrr">Disk</td>
          <td width="5%" class="listhdrr">Size</td>
          <td width="60%" class="listhdrr">Description</td>
          <td width="10%" class="listhdr">Status</td>
				</tr>
			  <?php foreach ($a_disk_conf as $disk): ?>
        <tr>
          <td class="listr">
            <?=htmlspecialchars($disk['name']);?>
          </td>
          <td class="listr">
            <?=htmlspecialchars($disk['size']);?>
          </td>
          <td class="listr">
            <?=htmlspecialchars($disk['desc']);?>&nbsp;
          </td>
           <td class="listr">
            <?php
            $stat=disks_status($disk);
            echo $stat;?>&nbsp;
          </td>
				</tr>
			  <?php endforeach; ?>
			  <?php if (isset($raidstatus)): ?>
				<?php foreach ($raidstatus as $diskk => $diskv): ?>
        <tr>
          <td class="listr">
            <?=htmlspecialchars($diskk);?>
          </td>
          <td class="listr">
            <?=htmlspecialchars($diskv['size']);?>
          </td>
          <td class="listr">
          
           <?=htmlspecialchars("Software RAID volume");?>&nbsp;
          </td>
           <td class="listr">
              <?=htmlspecialchars($diskv['desc']);?>&nbsp;
          </td>
				</tr>
				<?php endforeach; ?>
			  <?php endif; ?>
    </table>
  </form>
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
<script type="text/javascript">
<!--
enable_change(false);
//-->
</script>
</body>
</html>
