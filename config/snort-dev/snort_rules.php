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

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_new.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");

//Set no caching
header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
header("Cache-Control: no-store, no-cache, must-revalidate");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");

// set page vars

if (isset($_GET['uuid']) && isset($_GET['rdbuuid'])) {
	echo 'Error: more than one uuid';
	exit(0);
}

if (isset($_GET['uuid'])) {
	$uuid = $_GET['uuid'];
}

if (isset($_GET['rdbuuid'])) {
	$rdbuuid = $_GET['rdbuuid'];
}else{
	$ruledbname_pre1 = snortSql_fetchAllSettings('snortDB', 'SnortIfaces', 'uuid', $uuid);
	$rdbuuid = $ruledbname_pre1['ruledbname'];
}

// unset Session tmp on page load
unset($_SESSION['snort']['tmp']);

// list rules in the default dir
$a_list = snortSql_fetchAllSettings('snortDBrules', 'Snortrules', 'uuid', $rdbuuid);

$snortRuleDir = '/usr/local/etc/snort/snortDBrules/DB/' . $rdbuuid;

	// list rules in the default dir
	$filterDirList = array();
	$filterDirList = snortScanDirFilter($snortRuleDir . '/rules', '\.rules');

	// START read rule file
	if ($_GET['openruleset']) {
		$rulefile = $_GET['openruleset'];
	}else{
		$rulefile = $filterDirList[0];
	}

	// path of rule file
	$workingFile = $snortRuleDir . '/rules/' . $rulefile;
	
function load_rule_file($incoming_file, $splitcontents)
{	
		$pattern = '/(^alert |^# alert )/';
	 	foreach ( $splitcontents  as $val )
	 	{
	 		// remove whitespaces
			$rmWhitespaces = preg_replace('/\s\s+/', ' ', $val);	
			
			// filter none alerts
	 		if (preg_match($pattern, $rmWhitespaces))
	 		{
	 			$splitcontents2[] = $val;			
	 		}
	 		
	 	}
		unset($splitcontents);

	return $splitcontents2;

}
	
	// Load the rule file
	// split the contents of the string file into an array using the delimiter
	// used by rule gui edit and table build code	
	if (filesize($workingFile) > 0) {
	$splitcontents = split_rule_file($workingFile);		
	
		$splitcontents2 = load_rule_file($workingFile, $splitcontents);	
		
		$countSig = count($splitcontents2);
		
		if ($countSig > 0) {
			$newFilterRuleSigArray = newFilterRuleSig($splitcontents2);
		}
	}
		
	/*
	 * SET GLOBAL ARRAY $_SESSION['snort']
	 * Use SESSION instead POST for security because were writing to files.  
	 */
	
	$_SESSION['snort']['tmp']['snort_rules']['dbName'] = 'snortDBrules';
	$_SESSION['snort']['tmp']['snort_rules']['dbTable'] = 'SnortruleSigs';
	$_SESSION['snort']['tmp']['snort_rules']['rdbuuid'] = $rdbuuid;
	$_SESSION['snort']['tmp']['snort_rules']['rulefile'] = $rulefile;
	

// find ./ -name test.txt | xargs grep "^disablesid 127 "

	$pgtitle = "Snort: Category: rule: $rulefile";
	include("/usr/local/pkg/snort/snort_head.inc");

?>

		
	
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<!-- loading msg -->
<div id="loadingWaiting">
	<div class="snortModal" style="top: 200px; left: 700px;">
		<div class="snortModalTop">
			<!-- <div class="snortModalTopClose"><a href="javascript:hideLoading('#loadingWaiting');"><img src="/snort/images/close_9x9.gif" border="0" height="9" width="9"></a></div> -->
		</div>
		<div class="snortModalTitle">
	  		<p><img src="./images/loading.gif" /><br><br>Please Wait...</p>
	  	</div>
		<div>
		<p class="loadingWaitingMessage"></p>
	  	</div>
	</div>  
</div>

<!-- hidden div -->
<div id="loadingRuleEditGUI">
	
	<div class="loadingRuleEditGUIDiv">
				<form id="iform2" action="">
				<input type="hidden" name="snortSidRuleEdit" value="1" />
				<input type="hidden" name="snortSidRuleDBuuid" value="<?=$rdbuuid;?>" /> <!-- what to do, save -->
				<input type="hidden" name="snortSidRuleFile" value="<?=$rulefile; ?>" /> <!-- what to do, save -->								
				<input type="hidden" name="snortSidNum" value="" /> <!-- what to do, save -->
				<table width="100%" cellpadding="9" cellspacing="9" bgcolor="#eeeeee">
					<tr>
						<td>							
							<input name="save" type="submit" class="formbtn" id="save" value="Save" /> 
							<input type="button" class="formbtn closeRuleEditGUI" value="Close" >
						</td>
					</tr>				
					<tr>
						<td>						
							<textarea id="sidstring" name="sidstring" wrap="off" style="width: 98%; margin: 7px;" rows="1" cols="" ></textarea> <!-- SID to EDIT -->
						</td>
					</tr>
					<tr>
						<td>
							<textarea wrap="off" style="width: 98%; margin: 7px;" rows="<?php if(count($splitcontents) > 24){echo 24;}else{echo count($splitcontents);} ?>" cols="" disabled >
							
							<?php
							
							echo "\n";
							
							foreach ($splitcontents as $sidLineGui)
							
							echo $sidLineGui . "\n";
							
							
							
							?>
							</textarea> <!-- Display rule file -->
						</td>
					</tr>
					</table>
					<table width="100%" cellpadding="9" cellspacing="9" bgcolor="#eeeeee">
					<tr>
						<td>							
							<input name="save" type="submit" class="formbtn" id="save" value="Save" /> 
							<input type="button" class="formbtn closeRuleEditGUI" value="Close" >
						</td>
					</tr>
				</table>
				</form>
	</div>


</div>

<?php include("fbegin.inc"); ?>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<?php
	if (!empty($uuid)) { 
		echo '
		<tr>
			<td>
			<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
			<ul class="newtabmenu">
					<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
					<li><a href="/snort/snort_interfaces_edit.php?uuid=' . $uuid . '"><span>If Settings</span></a></li>
					<li><a href="/snort/snort_rulesets.php?uuid=' . $uuid . '"><span>Categories</span></a></li>
					<li class="newtabmenu_active"><a href="/snort/snort_rules.php?uuid=' . $uuid . '"><span>Rules</span></a></li>
					<li><a href="/snort/snort_rulesets_ips.php?uuid=' . $uuid . '"><span>Ruleset Ips</span></a></li>
					<li><a href="/snort/snort_define_servers.php?uuid=' . $uuid . '"><span>Servers</span></a></li>
					<li><a href="/snort/snort_preprocessors.php?uuid=' . $uuid . '"><span>Preprocessors</span></a></li>
					<li><a href="/snort/snort_barnyard.php?uuid=' . $uuid . '"><span>Barnyard2</span></a></li>			
			</ul>
			</div>
			</td>
		</tr>
		';
	}else{
		echo ' 
		<tr>
			<td>
			<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
			<ul class="newtabmenu">
				<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
				<li><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
				<li><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
				<li class="newtabmenu_active"><a href="/snort/snort_interfaces_rules.php"><span>RulesDB</span></a></li>
				<li><a href="/snort/snort_alerts.php"><span>Alerts</span></a></li>
				<li><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
				<li><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
				<li><a href="/snort/snort_interfaces_suppress.php"><span>Suppress</span></a></li>
				<li><a href="/snort/snort_help_info.php"><span>Help</span></a></li>
			</ul>
			</div>
			</td>
		</tr>
		<tr>
			<td>
			<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
			<ul class="newtabmenu">
			<li class="hide_newtabmenu"><a href="/snort/snort_interfaces_rules_edit.php?rdbuuid=' . $rdbuuid . '"><span>Rules DB Edit</span></a></li>
			<li class="hide_newtabmenu"><a href="/snort/snort_rulesets.php?rdbuuid=' . $rdbuuid . '"><span>Categories</span></a></li>
			<li class="hide_newtabmenu newtabmenu_active"><a href="/snort/snort_rules.php?rdbuuid=' . $rdbuuid . '"><span>Rules</span></a></li>
			<li><a href="/snort/snort_rulesets_ips.php?rdbuuid=' . $rdbuuid . '"><span>Ruleset Ips</span></a></li>
			</ul>
			</div>
			</td>
		</tr>	
			';
	}
	?>
	<tr>
	<td id="tdbggrey">	
	<div style="width:780px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 10px;">
	<!-- START MAIN AREA -->
	
	
			<!-- start Interface Satus -->
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
				<tr id="maintable77" >
					<td colspan="2" valign="top" class="listtopic2">
					Category:
			<select name="selectbox" class="formfld" >
				<?php
				if(isset($_GET['uuid'])) {
					$urlUuid = "&uuid=$uuid";
				}
				
				if(isset($_GET['rdbuuid'])) {
					$urlUuid = "&rdbuuid=$rdbuuid";
				}
				
				$i=0;
				foreach ($filterDirList as $value)
				{
					$selectedruleset = '';
					if ($value === $rulefile) {
						$selectedruleset = 'selected';
					}
					
					echo "\n" . '<option value="?&openruleset=' . $ruledir . $value . $urlUuid . '" ' . $selectedruleset . ' >' . $value . '</option>' . "\r";

				$i++;

				}
				?>
			</select>				
					There are <?=$countSig; ?> rules in this category. 
					</td>
					<td width="6%" colspan="2" valign="middle" class="listtopic3" >
					<a href="snort_interfaces_edit.php?uuid=<?=$new_ruleUUID;?>">
					<img style="padding-left:3px;" src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0" title="add rule">
					</a>
					</td>
				</tr>
			</table>
<br>

			<!-- Save all inputs -->
			<table width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td>
					<input id="select_all" type="button" class="formbtn" value="Select All"  >
					<input id="deselect_all" type="button" class="formbtn" value="Deselect All" >
				</td>
			</tr>
			</table>

<br>

			<!-- start User Interface -->

			
			<form id="iform" action="">
			<input type="hidden" name="snortSaveRuleSets" value="1" /> <!-- what to do, save -->
			<input type="hidden" name="ifaceTab" value="snort_rules" /> <!-- what interface tab -->
			
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
				<tr id="maintable77" >
					<td colspan="2" valign="top" class="listtopic">Snort Signatures:</td>
				</tr>
			</table>			
			
			<table id="mainCreateTable" width="100%" border="0" cellpadding="0" cellspacing="0">			
										
			<tr id="frheader" >
				<td class="listhdrr2">On</td>
				<td class="listhdrr2">Sid</td>
				<td class="listhdrr2">Proto</td>
				<td class="listhdrr2">Src</td>
				<td class="listhdrr2">Port</td>			
				<td class="listhdrr2">Dst</td>
				<td class="listhdrr2">Port</td>
				<td class="listhdrr2">Message</td>
				<td class="listhdrr2">&nbsp;</td>												
			</tr>
			<tr>
			<!-- START javascript sid loop here -->
			<tbody class="rulesetloopblock">
			
			
			
			</tbody>
			<!-- STOP javascript sid loop here -->
			</tr>				
							
			</table>
			<br>
			<table width="100%" border="0" cellpadding="0" cellspacing="0">			
			<tr>
				<td>
					<input name="Submit" type="submit" class="formbtn" value="Save">
					<input id="cancel" type="button" class="formbtn" value="Cancel">
				</td>
			</tr>
			</table>
			</form>			
			<br>			
					
			<!-- stop snortsam -->

	<!-- STOP MAIN AREA -->			
	</div>			
	</td>
	</tr>
</table>
</form>
</div>

<!-- start info box -->

<br>

<div style="width:790px; background-color: #dddddd;" id="mainarea4">
<div style="width:780px; margin-left: auto ; margin-right: auto ; padding-top: 10px; padding-bottom: 10px;">
<table class="vncell2" width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>&nbsp;&nbsp;&nbsp;</td>
	</tr>
	<tr >
	<td width="100%">
		<span class="red"><strong>Note:</strong></span> <br>
		This is the <strong>Snort Rule Signature Viewer</strong>.
		Please make sure not to add a <strong>whitespace</strong> before <strong>alert</strong> or <strong>#alert</strong>. 
		<br>
		<br>
		<span class="red"><strong>Warning:</strong></span> 
		<br>
		<strong>New settings will not take effect until interface restart.</strong>
		<br><br>		
	</td>
	</tr>
</table>
</div>
</div>


<script type="text/javascript">


//prepare the form when the DOM is ready 
jQuery(document).ready(function() {

	// NOTE: needs to be watched
	// change url on selected dropdown rule	
	jQuery('select[name=selectbox]').change(function() {
		window.location.replace(jQuery(this).val());
	});		
	
<?php

	/* 
	 * NOTE:
	 * I could have used a php loop to build the table but I wanted to see if off loading to client is faster.
	 * Seems to be faster on embeded systems with low specs. On higher end systems there is no difference that I can see.
	 * WARNING:
	 * If Json string is to long browsers start asking to terminate javascript.
	 * FIX: 
	 * Use julienlecomte()net/blog/2007/10/28/, the more reading I do about this subject it seems that off loading to a client is not recomended.
	 */
	if (!empty($newFilterRuleSigArray))
	{
		$countSigList = count($newFilterRuleSigArray);
		
		echo "\n";
		
		echo 'var snortObjlist = [';	
				$i = 0;
				foreach ($newFilterRuleSigArray as $val3)
				{
					
					$i++;
					
					// NOTE: escapeJsonString; foward slash has added spaces on each side, ie and chrome were giving issues with tablw widths
					if( $i !== $countSigList ) {		 
						echo '{"sid":"' . $val3['sid'] . '","enable":"' . $val3['enable'] . '","proto":"' . $val3['proto'] . '","src":"' . $val3['src'] . '","srcport":"' . $val3['srcport'] . '","dst":"' . $val3['dst'] . '", "dstport":"' . $val3['dstport'] . '","msg":"' . escapeJsonString($val3['msg']) . '"},'; 
					}else{
						echo '{"sid":"' . $val3['sid'] . '","enable":"' . $val3['enable'] . '","proto":"' . $val3['proto'] . '","src":"' . $val3['src'] . '","srcport":"' . $val3['srcport'] . '","dst":"' . $val3['dst'] . '", "dstport":"' . $val3['dstport'] . '","msg":"' . escapeJsonString($val3['msg']) . '"}'; 
					}
				}
				
		echo '];' . "\n";
	}	
			

	
	if (!empty($countSig)) {
		echo 'var countRowAppend = ' . $countSig . ';' . "\n";	
	}else{
		echo 'var countRowAppend = 0;' . "\n";		
	}	
	
?>

if(typeof escapeHtmlEntities == 'undefined') {
	  escapeHtmlEntities = function (text) {
	    return text.replace(/[\u00A0-\u2666<>\&]/g, function(c) { return '&' + 
	      escapeHtmlEntities.entityTable[c.charCodeAt(0)] || '#'+c.charCodeAt(0) + ';'; });
	  };

	  // all HTML4 entities as defined here: http://www.w3.org/TR/html4/sgml/entities.html
	  // added: amp, lt, gt, quot and apos
	  escapeHtmlEntities.entityTable = { 34 : 'quot', 38 : 'amp', 39 : 'apos', 47 : 'slash', 60 : 'lt', 62 : 'gt', 160 : 'nbsp', 161 : 'iexcl', 162 : 'cent', 163 : 'pound', 164 : 'curren', 165 : 'yen', 166 : 'brvbar', 167 : 'sect', 168 : 'uml', 169 : 'copy', 170 : 'ordf', 171 : 'laquo', 172 : 'not', 173 : 'shy', 174 : 'reg', 175 : 'macr', 176 : 'deg', 177 : 'plusmn', 178 : 'sup2', 179 : 'sup3', 180 : 'acute', 181 : 'micro', 182 : 'para', 183 : 'middot', 184 : 'cedil', 185 : 'sup1', 186 : 'ordm', 187 : 'raquo', 188 : 'frac14', 189 : 'frac12', 190 : 'frac34', 191 : 'iquest', 192 : 'Agrave', 193 : 'Aacute', 194 : 'Acirc', 195 : 'Atilde', 196 : 'Auml', 197 : 'Aring', 198 : 'AElig', 199 : 'Ccedil', 200 : 'Egrave', 201 : 'Eacute', 202 : 'Ecirc', 203 : 'Euml', 204 : 'Igrave', 205 : 'Iacute', 206 : 'Icirc', 207 : 'Iuml', 208 : 'ETH', 209 : 'Ntilde', 210 : 'Ograve', 211 : 'Oacute', 212 : 'Ocirc', 213 : 'Otilde', 214 : 'Ouml', 215 : 'times', 216 : 'Oslash', 217 : 'Ugrave', 218 : 'Uacute', 219 : 'Ucirc', 220 : 'Uuml', 221 : 'Yacute', 222 : 'THORN', 223 : 'szlig', 224 : 'agrave', 225 : 'aacute', 226 : 'acirc', 227 : 'atilde', 228 : 'auml', 229 : 'aring', 230 : 'aelig', 231 : 'ccedil', 232 : 'egrave', 233 : 'eacute', 234 : 'ecirc', 235 : 'euml', 236 : 'igrave', 237 : 'iacute', 238 : 'icirc', 239 : 'iuml', 240 : 'eth', 241 : 'ntilde', 242 : 'ograve', 243 : 'oacute', 244 : 'ocirc', 245 : 'otilde', 246 : 'ouml', 247 : 'divide', 248 : 'oslash', 249 : 'ugrave', 250 : 'uacute', 251 : 'ucirc', 252 : 'uuml', 253 : 'yacute', 254 : 'thorn', 255 : 'yuml', 402 : 'fnof', 913 : 'Alpha', 914 : 'Beta', 915 : 'Gamma', 916 : 'Delta', 917 : 'Epsilon', 918 : 'Zeta', 919 : 'Eta', 920 : 'Theta', 921 : 'Iota', 922 : 'Kappa', 923 : 'Lambda', 924 : 'Mu', 925 : 'Nu', 926 : 'Xi', 927 : 'Omicron', 928 : 'Pi', 929 : 'Rho', 931 : 'Sigma', 932 : 'Tau', 933 : 'Upsilon', 934 : 'Phi', 935 : 'Chi', 936 : 'Psi', 937 : 'Omega', 945 : 'alpha', 946 : 'beta', 947 : 'gamma', 948 : 'delta', 949 : 'epsilon', 950 : 'zeta', 951 : 'eta', 952 : 'theta', 953 : 'iota', 954 : 'kappa', 955 : 'lambda', 956 : 'mu', 957 : 'nu', 958 : 'xi', 959 : 'omicron', 960 : 'pi', 961 : 'rho', 962 : 'sigmaf', 963 : 'sigma', 964 : 'tau', 965 : 'upsilon', 966 : 'phi', 967 : 'chi', 968 : 'psi', 969 : 'omega', 977 : 'thetasym', 978 : 'upsih', 982 : 'piv', 8226 : 'bull', 8230 : 'hellip', 8242 : 'prime', 8243 : 'Prime', 8254 : 'oline', 8260 : 'frasl', 8472 : 'weierp', 8465 : 'image', 8476 : 'real', 8482 : 'trade', 8501 : 'alefsym', 8592 : 'larr', 8593 : 'uarr', 8594 : 'rarr', 8595 : 'darr', 8596 : 'harr', 8629 : 'crarr', 8656 : 'lArr', 8657 : 'uArr', 8658 : 'rArr', 8659 : 'dArr', 8660 : 'hArr', 8704 : 'forall', 8706 : 'part', 8707 : 'exist', 8709 : 'empty', 8711 : 'nabla', 8712 : 'isin', 8713 : 'notin', 8715 : 'ni', 8719 : 'prod', 8721 : 'sum', 8722 : 'minus', 8727 : 'lowast', 8730 : 'radic', 8733 : 'prop', 8734 : 'infin', 8736 : 'ang', 8743 : 'and', 8744 : 'or', 8745 : 'cap', 8746 : 'cup', 8747 : 'int', 8756 : 'there4', 8764 : 'sim', 8773 : 'cong', 8776 : 'asymp', 8800 : 'ne', 8801 : 'equiv', 8804 : 'le', 8805 : 'ge', 8834 : 'sub', 8835 : 'sup', 8836 : 'nsub', 8838 : 'sube', 8839 : 'supe', 8853 : 'oplus', 8855 : 'otimes', 8869 : 'perp', 8901 : 'sdot', 8968 : 'lceil', 8969 : 'rceil', 8970 : 'lfloor', 8971 : 'rfloor', 9001 : 'lang', 9002 : 'rang', 9674 : 'loz', 9824 : 'spades', 9827 : 'clubs', 9829 : 'hearts', 9830 : 'diams', 34 : 'quot', 38 : 'amp', 60 : 'lt', 62 : 'gt', 338 : 'OElig', 339 : 'oelig', 352 : 'Scaron', 353 : 'scaron', 376 : 'Yuml', 710 : 'circ', 732 : 'tilde', 8194 : 'ensp', 8195 : 'emsp', 8201 : 'thinsp', 8204 : 'zwnj', 8205 : 'zwj', 8206 : 'lrm', 8207 : 'rlm', 8211 : 'ndash', 8212 : 'mdash', 8216 : 'lsquo', 8217 : 'rsquo', 8218 : 'sbquo', 8220 : 'ldquo', 8221 : 'rdquo', 8222 : 'bdquo', 8224 : 'dagger', 8225 : 'Dagger', 8240 : 'permil', 8249 : 'lsaquo', 8250 : 'rsaquo', 8364 : 'euro' };
}

	// if rowcount is not empty do this
	if (countRowAppend > 0){

		// if rowcount is more than 300
		if (countRowAppend > 200){		
			// call to please wait	
			showLoading('#loadingWaiting');
		}
	
	
		// Break up append row adds by chunks of 300
		// NOTE: ie9 is still giving me issues on deleted.rules 6000 sigs. I should break up the json code above into smaller parts.
		incrementallyProcess(function (i){
		  // loop code goes in here
		  //console.log('loop: ', i);
	
			if (isEven(i) === true){
				var rowIsEvenOdd = 'odd_ruleset2';
			}else{ 
				var rowIsEvenOdd = 'even_ruleset2';
			}
			
			if (snortObjlist[i].enable === 'on'){
				var rulesetChecked = 'checked'; 
			}else{
				var rulesetChecked = '';
			}
		
			jQuery('.rulesetloopblock').append(
	
					"\n" + '<tr valign="top" id="fr0">' + "\n" +
					'<td class="' + rowIsEvenOdd + '">' + "\n" +
						'<input class="domecheck" type="checkbox" name="filenamcheckbox2[]" value="' + snortObjlist[i].sid + '" ' + rulesetChecked + ' >' + "\n" +
					'</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].sid + '</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].proto + '</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].src + '</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].srcport + '</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].dst + '</td>' + "\n" +
						'<td class="' + rowIsEvenOdd + '" id="frd0" >' + snortObjlist[i].dstport + '</td>' + "\n" +
						'<td class="listbg" id="frd0" ><font color="white">' + escapeHtmlEntities(snortObjlist[i].msg) + '</font></td>' + "\n" +
						'<td class="' + rowIsEvenOdd+ '">' + "\n" +
							'<img id="' + snortObjlist[i].sid + '" class="icon_click showeditrulegui" src="/themes/<?=$g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0" title="edit rule">' + "\n" +
						'</td>' + "\n" +						
					'</tr>' + "\n"
					
			);
		  
		}, 
		snortObjlist,  // Object to work with the case Json object
		500, // chunk size
		200, // how many secs to wait
		function (){
		// things that happen after the processing is done go here
		// console.log('done!');
		
		// if rowcount is more than 300
		if (countRowAppend > 200){		
			// call to please wait	
			hideLoading('#loadingWaiting');
		}	
		
		});
	} // end of if stopRowAppend

	
	// On click show rule edit GUI
	jQuery('.showeditrulegui').live('click', function(){
	
		// Get sid
		jQuery.getJSON('/snort/snort_json_get.php',
			{
			"snortGetSidString": "1",
			"snortIface": "<?=$uuid . '_' . $a_list['interface']; ?>",
			"snortRuleFile": "<?=$rulefile; ?>",
			"sid": jQuery(this).attr('id')
			},
			function(data){
				jQuery("textarea#sidstring").val(data.sidstring); // add string to textarea
				jQuery("input[name=snortSidNum]").val(data.sid); // add sid to input
				showLoading('#loadingRuleEditGUI');						
			});	
		});
	
	jQuery('.closeRuleEditGUI').live('click', function(){	
		hideLoading('#loadingRuleEditGUI');
	});	
	

}); // end of document ready

</script>


<!-- stop info box -->

<!-- footer do not touch below -->
<?php 
include("fend.inc"); 
echo $snort_custom_rnd_box;
?>


</body>
</html>
