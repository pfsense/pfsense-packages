<?php
/*
    snort_alerts.widget.php
    Copyright (C) 2009 Jim Pingle

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
global $config, $g;

?>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
	<tbody>
		<tr class="snort-alert-header">
			<td width="5%"  class="widgetsubheader">Pri</td>
			<td width="5%"  class="widgetsubheader">Category</td>
			<td width="45%" class="widgetsubheader">Src</td>
			<td width="45%" class="widgetsubheader">Dst</td>
		</tr>
<?php   $counter=0;
 	foreach ($snort_alerts as $alert) { ?>

	<?php
		if(isset($config['syslog']['reverse'])) {
			/* honour reverse logging setting */
			if($counter == 0)
				$activerow = " id=\"snort-firstrow\"";
			else
				$activerow = "";

		} else {
			/* non-reverse logging */
			if($counter == count($snort_alerts) - 1)
				$activerow = " id=\"snort-firstrow\"";
			else
				$activerow = "";
		}
	?>

		<tr class="snort-alert-entry" <?php echo $activerow; ?>>
			<td width="5%"  class="listr"><?= $alert["priority"] ?></td>
			<td width="5%"  class="listr"><?= $alert["category"] ?></td>
			<td width="45%" class="listr"><?= $alert["src"] ?></td>
			<td width="45%" class="listr"><?= $alert["dst"] ?></td>
		</tr>
<?php 		$counter++;
	} ?>
	</tbody>
</table>
