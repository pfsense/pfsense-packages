<?php
/* $Id$ */
/*
	sg_log.inc
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

$pgtitle = "Proxy filter SquidGuard: Log page";

require_once('globals.inc');
require_once('config.inc');
require_once("guiconfig.inc");
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('service-utils.inc');

include("head.inc");

if (file_exists("/usr/local/pkg/squidguard.inc")) {
   require_once("/usr/local/pkg/squidguard.inc");
}

$mode     = $_GET["mode"];
$selfpath = "/squidGuard/squidguard_log.php";

if (!in_array( $mode, array("blocked", "fgui", "flog", "pconf", "fconf"))) $mode = "blocked";

?>

<!-- HTML -->
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="sg_log.php" method="post">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<!-- Tabs -->
  <tr>
    <td>
<?php
        $tab_array = array();
        $tab_array[] = array(gettext("General settings"), false, "/pkg_edit.php?xml=squidguard.xml&amp;id=0");
        $tab_array[] = array(gettext("Default"),          false, "/pkg_edit.php?xml=squidguard_default.xml&amp;id=0");
        $tab_array[] = array(gettext("ACL"),              false, "/pkg.php?xml=squidguard_acl.xml");
        $tab_array[] = array(gettext("Destinations"),     false, "/pkg.php?xml=squidguard_dest.xml");
        $tab_array[] = array(gettext("Times"),            false, "/pkg.php?xml=squidguard_time.xml");
        $tab_array[] = array(gettext("Rewrites"),         false, "/pkg.php?xml=squidguard_rewr.xml");
        $tab_array[] = array(gettext("Log"),              true,  "$selfpath");
        display_top_tabs($tab_array);
?>
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
        <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
          <tr>
            <td>

<?php
        # Subtabs
        $mode = $mode ? $mode : "blocked";
        $tab_array = array();
        $tab_array[] = array(gettext("Blocked"),        ($mode == "blocked"), "{$selfpath}?mode=blocked");
        $tab_array[] = array(gettext("Filter GUI log"), ($mode == "fgui"),    "{$selfpath}?mode=fgui");
        $tab_array[] = array(gettext("Filter log"),     ($mode == "flog"),    "{$selfpath}?mode=flog");
        $tab_array[] = array(gettext("Proxy config"),   ($mode == "pconf"),   "{$selfpath}?mode=pconf");
        $tab_array[] = array(gettext("Filter config"),  ($mode == "fconf"),   "{$selfpath}?mode=fconf");

        echo "<big>| ";
        foreach ($tab_array as $ta) {
            $bl = $ta[1] ? "<b>"  : "";
            $br = $ta[1] ? "</b>" : "";
            echo "<a href=\"{$ta[2]}\">$bl{$ta[0]}$br</a> | ";
        }
        echo "</big>";
?>
            </td>
          </tr>
          <tr>
            <td>
<?php
    $cont = "";
    switch( $mode ) {
        case "pconf":   if (function_exists("squidguard_squid_conflist")) 
                             $cont = squidguard_squid_conflist( );
                        else $cont = "Function 'squidguard_squid_conflist' not found.";
                        echo "<br><center><textarea rows='30' cols='70' name='pconf' wrap='hard' readonly>{$cont}</textarea></center>" ; 
                        break;
        case "fconf":   if (function_exists("squidguard_conflist")) 
                             $cont = squidguard_conflist( );
                        else $cont = "Function 'squidguard_conflist' not found.";
                        echo "<br><center><textarea rows='30' cols='70' name='pconf' wrap='hard' readonly>{$cont}</textarea></center>" ; 
                        break;
        case "flog":    if (function_exists( "squidguard_logrep" )) echo squidguard_logrep( SQUIDGUARD_LOGDIR . '/squidGuard.log', 50, false );
                        break;
        case "fgui":    if (function_exists( "squidguard_logrep" )) echo squidguard_logrep( SQUIDGUARD_LOGDIR . SQUIDGUARD_CONFLOGFILE, 50, false );
                        break;
        case "blocked":
        default:        if (function_exists( "squidguard_logrep" )) echo squidguard_logrep( SQUIDGUARD_LOGDIR . '/' . SQUIDGUARD_LOGFILE, 50, false );                        
                        break;
    }
?>              
            </td>
          </tr>
        </table>
      </div>
    </td>
  </tr>
</table>
</form>

<?php include("fend.inc"); ?>

<script type="text/javascript"> 
  NiftyCheck(); 
  Rounded("div#mainarea","bl br","#FFF","#eeeeee","smooth");
</script>
</body>
</html>