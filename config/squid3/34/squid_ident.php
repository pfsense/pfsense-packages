#!/usr/bin/php
#http://blog.dataforce.org.uk/2010/03/Ident-Server
<?php
        /**
         * Simple PHP-Based inetd ident server, version 0.1.
         * Copyright (c) 2010 - Shane "Dataforce" Mc Cormack
         * This code is licensed under the MIT License, of which a copy can be found
         * at http://www.opensource.org/licenses/mit-license.php
         *
         * The latest version of the code can be found at
         * http://blog.dataforce.org.uk/index.php?p=news&id=135
         *
         * This should be run from inetd, it will take input on stdin and write to stdout.
         *
         * By default users can spoof ident by having a .ident file in /home/<username>/.ident
         * If this is present, it will be read.
         * It should be a file with a format like so:
         *
         * <pid> <ident>
         * <local host>:<local port>:<target host>:<target port> <ident>
         *
         * The first line that matches is used, any bit can be a * and it will always match,
         * so "* user" is valid. In future more sophisticated matches will be permitted
         * (eg 127.*) but for now its either all or nothing.
         *
         * Its worth noting that <target host> is the host that requests the ident, so if this
         * is likely to be different than the host that was connected to, then "STRICT_HOST" will
         * need to be set to false.
         *
         * At the moment <local host> is ignored, in future versions this might be changed, so
         * it is still required.
         *
         * Lines with a ':' in them are assumed to be of the second format, and must contain
         * all 4 sections or they will be ignored.
         *
         * Lines starting with a # are ignored.
         *
         * There are some special values that can be used as idents:
         *    ! = Send an error instead.
         *    * = Send the default ident.
         *    ? = Send a random ident (In future a 3rd parameter will specify the format,
         *        # for a number, @ for a letter, ? for either, but this is not implemented yet)
         *
         * In future there will also be support for /home/user/.ident.d/ directories, where
         * every file will be read for the ident response untill one matches.
         * This will allow multiple processes to create files rather than needing to
         * lock and edit .ident
         */

        // Allow spoofing idents.
        define('ALLOW_SPOOF', true);

        // Requesting host must be the same as the host that was connected to.
        define('STRICT_HOST', true);

        // Error to send when '!' is used as an ident.
        define('HIDE_ERROR', 'UNKNOWN-ERROR');

        openlog('simpleIdent', LOG_PID | LOG_ODELAY, LOG_DAEMON);

        $result = 'ERROR : UNKNOWN-ERROR' . "\n";

        $host = $_SERVER['REMOTE_HOST'];

        syslog(LOG_INFO, 'Connection from: '.$host);

        // Red in the line from the socket.
        $fh = @fopen('php://stdin', 'r');
        if ($fh) {
                $input = @fgets($fh);
                $line = trim($input);
                if ($input !== FALSE && !empty($line)) {
                        $result = trim($input) . ' : ' . $result;
                        // Get the data from it.
                        $bits = explode(',', $line);
                        $source = trim($bits[0]);
                        $dest = isset($bits[1]) ? trim($bits[1]) : '';

                        // Check if it is valid
                        if (preg_match('/^[0-9]+$/', $source) && preg_match('/^[0-9]+$/', $dest)) {
                                // Now actually look for this!
                                $match = STRICT_HOST ? ":$source .*$host:$dest " : ":$source.*:$dest";

                                $output = `netstat -napW 2>&1 | grep '$match' | awk '{print \$7}'`;

                                $bits = explode('/', $output);
                                $pid = $bits[0];

                                if (preg_match('/^[0-9]+$/', $pid)) {
                                        $user = `ps -o ruser=SOME-REALLY-WIDE-USERNAMES-ARE-PERMITTED-HERE $pid | tail -n 1`;

                                        $senduser = trim($user);

                                        // Look for special ident file: /home/user/.ident this is an ini-format file.
                                        $file = '/home/'.trim($user).'/.ident';

                                        if (file_exists($file)) {
                                                $config = file($file, FILE_SKIP_EMPTY_LINES | FILE_IGNORE_NEW_LINES | FILE_TEXT);
                                                foreach ($config as $line) {
                                                        // Ignore comments.
                                                        $line = trim($line);
                                                        if (substr($line, 1) == '#') { continue; }

                                                        // Make sure line is valid.
                                                        $bits = explode(' ', $line);
                                                        if (count($bits) == 1) { continue; }

                                                        // Check type of line
                                                        if (strpos($bits[0], ':') !== FALSE) {
                                                                // LocalHost:LocalPort:RemoteHost:RemotePort
                                                                $match = explode(':', $bits[0]);
                                                                if (count($match) != 4) { continue; }

                                                                if (($match[1] == '*' || $match[1] == $source) &&
                                                                    ($match[2] == '*' || $match[2] == $host) &&
                                                                    ($match[3] == '*' || $match[3] == $dest)) {
                                                                        syslog(LOG_INFO, 'Spoof for '.$senduser.': '.$line);
                                                                        $senduser = $bits[1];
                                                                        break;
                                                                }
                                                        } else if ($bits[0] == '*' || $bits[0] == $pid) {
                                                                syslog(LOG_INFO, 'Spoof for '.$senduser.': '.$line);
                                                                $senduser = $bits[1];
                                                        }
                                                }

                                                if ($senduser == "*") {
                                                        $senduser = trim(user);
                                                } else if ($senduser == "?") {
                                                        $senduser = 'user'.rand(1000,9999);
                                                }
                                        }

                                        if ($senduser != "!") {
                                                $result = $source . ', ' . $dest . ' : USERID : UNIX : ' . trim($senduser);
                                        } else {
                                                $result = $source . ', ' . $dest . ' : ERROR : ' . HIDE_ERROR;
                                        }
                                }
                        }
                }
        }

        echo $result;
        syslog(LOG_INFO, 'Result: '.$result);
        closelog();
        exit(0);
?>
