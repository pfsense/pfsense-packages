<?php
/* ========================================================================== */
/*
	apache_view_logs.php
	part of pfSense (http://www.pfSense.com)
	Copyright (C) 2009, 2010 Scott Ullrich <sullrich@gmail.com>
	Copyright (C) 2012 Marcello Coutinho
	Copyright (C) 2012 Carlos Cesario
	All rights reserved.
                                                                              */
/* ========================================================================== */
/*
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	 1. Redistributions of source code MUST retain the above copyright notice,
		this list of conditions and the following disclaimer.

	 2. Redistributions in binary form MUST reproduce the above copyright
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
/* ========================================================================== */

require_once("/etc/inc/util.inc");
require_once("/etc/inc/functions.inc");
require_once("/etc/inc/pkg-utils.inc");
require_once("/etc/inc/globals.inc");
require_once("guiconfig.inc");
require_once("apache_mod_security.inc");

$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
if(strstr($pfSversion, "1.2"))
        $one_two = true;

$pgtitle = "Apache reverse proxy: Apache VirtualHost Location";

$virtualhost_id = $_GET['virtualhost_id'];
if (isset($_POST['virtualhost_id']))        
	$virtualhost_id = $_POST['virtualhost_id'];

$backend_id = $_GET['backend_id'];
if (isset($_POST['backend_id']))        
	$backend_id = $_POST['backend_id'];

if (is_array($config['installedpackages']['apachevirtualhost']['config']) && is_array($config['installedpackages']['apachevirtualhost']['config'][$virtualhost_id])) 
	$virtualhost = &$config['installedpackages']['apachevirtualhost']['config'][$virtualhost_id];
if (is_array($virtualhost['row']) && is_array($virtualhost['row'][$backend_id]))
	$backend = &$virtualhost['row'][$backend_id];

/*
 * Not having a virtualhost->backend entry means we can't do this.
 */
if (! $backend) {
	$input_errors[] = gettext("Requested VirtualHost (ID={$virtualhost_id}) or Backend (ID={$backend_id}) does not exist.");
}


if ($_POST) {
	unset($input_errors);

	/*
	 * Check for a valid expirationdate if one is set at all (valid means,
	 * DateTime puts out a time stamp so any DateTime compatible time
	 * format may be used. to keep it simple for the enduser, we only
	 * claim to accept MM/DD/YYYY as inputs. Advanced users may use inputs
	 * like "+1 day", which will be converted to MM/DD/YYYY based on "now".
	 * Otherwhise such an entry would lead to an invalid expiration data.
	 */
	if ($_POST['expires']) {
		try {
			$expdate = new DateTime($_POST['expires']);
			//convert from any DateTime compatible date to MM/DD/YYYY
			$_POST['expires'] = $expdate->format("m/d/Y");
		} catch ( Exception $ex ) {
			$input_errors[] = gettext("Invalid expiration date format; use MM/DD/YYYY instead.");
		}
	}

	/* if this is an AJAX caller then handle via JSON */
	if (isAjax() && is_array($input_errors)) {
		input_errors2Ajax($input_errors);
		exit;
	}

	if (!$input_errors) {
		if ($_POST['custom'])
			$backend['custom'] = base64_encode($_POST['custom']);
		else
			unset($backend['custom']);

		write_config("Saved Location Custom Settings for location {$backend['sitepath']} on virtual host '{$virtualhost['primarysitehostname']}'");
		apache_mod_security_resync();
		pfSenseHeader("apache_edit_virtualhost_location.php?virtualhost_id={$virtualhost_id}&backend_id={$backend_id}");
	}
}

include("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if($one_two): ?>

	<p class="pgtitle"><?=$pgtitle?></font></p>

<?php endif; ?>

<?php
	if ($input_errors)
		print_input_errors($input_errors);
	if ($savemsg)
		print_info_box($savemsg);
?>

<div id="mainlevel">
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr><td>
		<?php
			$tab_array = array();
			$tab_array[] = array(gettext("Apache"), true, "/pkg_edit.php?xml=apache_settings.xml&amp;id=0");
			$tab_array[] = array(gettext("ModSecurity"), false, "/pkg_edit.php?xml=apache_mod_security_settings.xml");
			$tab_array[] = array(gettext("Sync"), false, "/pkg_edit.php?xml=apache_mod_security_sync.xml");
			display_top_tabs($tab_array);
		?>
		</td></tr>
		<tr><td>
			<?php 
			unset ($tab_array);
			$tab_array[] = array(gettext("Daemon Options"), false, "pkg_edit.php?xml=apache_settings.xml");
			$tab_array[] = array(gettext("Backends / Balancers"), false, "/pkg.php?xml=apache_balancer.xml");
			$tab_array[] = array(gettext("Virtual Hosts"), true, "/pkg.php?xml=apache_virtualhost.xml");
			$tab_array[] = array(gettext("Logs"), false, "/apache_view_logs.php");
			display_top_tabs($tab_array);
		?>
		</td></tr>
		<tr><td>
		<div id="mainarea" style="padding-top: 0px; padding-bottom: 0px; ">
			<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="6"><tbody>
				<form action="apache_edit_virtualhost_location.php" id="paramsForm" name="paramsForm" method="post">
					<tr>
						<td width="22%" valign="top" class="vncellreq">Primary Site Hostname</td>
						<td width="78%" class="vtable">
						<span class="vexpl">
							<?=base64_decode($virtualhost['primarysitehostname']);?>
						</span>
						</td>
					</tr>
					<tr>
						<td width="22%" valign="top" class="vncellreq">Current Site Path</td>
						<td width="78%" class="vtable">
						<span class="vexpl">
							<?=$backend['sitepath'];?>
						</span>
						</td>
					</tr>
					<tr>
						<td width="22%" valign="top" class="vncellreq"><?=gettext("Location Custom Settings");?></td>
						<td width="78%" class="vtable">
							<textarea name='custom' rows='10' cols='65' id='custom'><?=base64_decode($backend['custom']);?></textarea>
							<br/>
							<span class="vexpl">
								<?=gettext("Pass extra Apache config for this Location. This is useful for SSLRequire rules for example.");?>
							</span>
						</td>
					</tr>
					<tr>
						<td width="22%" valign="top">&nbsp;</td>
						<td width="78%">
<?php if (isset($virtualhost_id)): ?>
							<input name="virtualhost_id" type="hidden" value="<?=$virtualhost_id;?>" />
<?php endif;?>
<?php if (isset($backend_id)): ?>
							<input name="backend_id" type="hidden" value="<?=$backend_id;?>" />
<?php endif;?>
							<input id="submit" name="save" type="submit" class="formbtn" value="<?=gettext("Save");?>" />
							<input id="cancel" name="cancel" type="button" class="formbtn" value="<?=gettext("Cancel"); ?>" onclick="history.back()" />
						</td>
					</tr>
				</form>
			</tbody></table>
		</div>
		</td></tr>
	</table>
</div>


<?php
include("fend.inc");
?>

</body>
</html>
