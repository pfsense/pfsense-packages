#!/usr/local/bin/php -f

<?php

require_once("/usr/local/pkg/snort/snort.inc");
require_once("/etc/inc/config.inc");

if (empty($argv) || file_exists("/tmp/snort_startstop.php.pid")) {
        exit();
}

if (!empty($_GET[snortstart]) && !empty($_GET[snortstop]) || empty($_GET[snortstart]) && empty($_GET[snortstop]) ) {
        exit();
}

        // make shure there are no dup starts
        exec("/bin/echo 'Starting snort_startstop.php' > /tmp/snort_startstop.php.pid");

        // wait until boot is done
        $snort_bootupWait = function() use(&$_GET, &$g) {
                $i = 0;
                exec("/bin/echo {$i} > /tmp/snort_testing.sh.pid");
                while(isset($g['booting']) || file_exists("{$g['varrun_path']}/booting")) {
                        $i++;
                        exec("/usr/bin/logger -p daemon.info -i -t SnortBoot 'Snort Boot count...{$i}'");
                        exec("/bin/echo {$i} > /tmp/snort_testing.sh.pid"); // remove when finnished testing
                        sleep(2);
                }
        };
        $snort_bootupWait();


        $snort_bootupCleanStartStop = function($type) use(&$_GET, &$g) {

                $snortstartArray = explode(',', $_GET[$type]);

                foreach($snortstartArray as $iface_pre) {
                
                        if (!empty($iface_pre)) {
                                $iface = explode('_', $iface_pre);

                                if( !empty($iface[0]) && !empty($iface[1]) && is_numeric($iface[2]) ) {
                                	
                                        if($type === 'snortstart') { Running_Start($iface[0], $iface[1], $iface[2]); }

                                        if($type === 'snortstop') { Running_Stop($iface[0], $iface[1], $iface[2]); }
                                        
                                }
                        }
                }
        };


        if (!empty($_GET[snortstart])) {
                $snort_bootupCleanStartStop('snortstart');
        }
        if (!empty($_GET[snortstop])) {
                $snort_bootupCleanStartStop('snortstop');
        }

        // important
        @exec("/bin/rm /tmp/snort_startstop.php.pid");
        exit();

?>
