<?php

$pgtitle = array(gettext("Status"), "tinc");
require("guiconfig.inc");

function tinc_status_1() {
        exec("/usr/local/sbin/tincd --config=/usr/local/etc/tinc -kUSR1");
	usleep(500000);
        exec("/usr/sbin/clog /var/log/tinc.log | sed -e 's/.*tinc\[.*\]: //'",$result);
        $i=0;
        foreach($result as $line)
        {
                if(preg_match("/Connections:/",$line))
                        $begin=$i;
                if(preg_match("/End of connections./",$line))
                        $end=$i;
                $i++;
        }
        $output="";
        $i=0;
        foreach($result as $line)
        {
                if($i >= $begin && $i<= $end)
                        $output .= $line . "\n";
                $i++;
        }
        return $output;
}

function tinc_status_2() {
        exec("/usr/local/sbin/tincd --config=/usr/local/etc/tinc -kUSR2");
	usleep(500000);
        exec("/usr/sbin/clog /var/log/tinc.log | sed -e 's/.*tinc\[.*\]: //'",$result);
        $i=0;
        foreach($result as $line)
        {
                if(preg_match("/Statistics for Generic BSD tun device/",$line))
                        $begin=$i;
                if(preg_match("/End of subnet list./",$line))
                        $end=$i;
                $i++;
        }
        $output="";
        $i=0;
        foreach($result as $line)
        {
                if($i >= $begin && $i<= $end)
                        $output .= $line . "\n";
                $i++;
        }
        return $output;
}

$shortcut_section = "tinc";
include("head.inc"); ?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?=$jsevents["body"]["onload"];?>">
<?php include("fbegin.inc"); ?>

Connection list:<BR>
<pre>
<?php print tinc_status_1(); ?>
</pre>
<BR>
Virtual network device statistics, all known nodes, edges and subnets:<BR>
<pre>
<?php print tinc_status_2(); ?>
</pre>

<?php include("fend.inc"); ?>
