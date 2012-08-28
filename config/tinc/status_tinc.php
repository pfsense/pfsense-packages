<?php

$pgtitle = array(gettext("Status"), "tinc");
require("guiconfig.inc");
require_once("tinc.inc");

include("head.inc"); ?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?=$jsevents["body"]["onload"];?>">
<?php include("fbegin.inc"); ?>


1:<BR>
<pre>
<?php print tinc_status_1(); ?>
</pre>
<BR>
2:<BR>
<pre>
<?php print tinc_status_2(); ?>
</pre>


<?php include("fend.inc"); ?>
