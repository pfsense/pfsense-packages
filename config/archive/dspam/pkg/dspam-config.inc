<?php
/* $Id$ */
/* Copyright (C) 2006 Daniel S. Haischt */

$CONFIG = array('DSPAM_HOME'          => '/var/db/dspam',
                'DSPAM_BIN'           => '/usr/local/bin',
                'DSPAM'               => '/usr/local/bin/dspam',
                'DSPAM_STATS'         => '/usr/local/bin/dspam_stats',
                'DSPAM_WWW'           => '/usr/local/pkg',
                'DSPAM_ARGS'          => '--deliver=innocent --class=innocent ' .
                                         '--source=error --user %CURRENT_USER% -d %u',
                'DSPAM_ADMIN_GROUP'   => 'dspam_admins',
                'ALL_PROCS'           => 'ps auxw',
                'MAIL_QUEUE'          => 'mailq | grep \'^[0-9,A-F]\' | wc -l',
                'HISTORY_SIZE'        => 799,
                'HISTORY_PER_PAGE'    => 100,
                'QUARANTINE_PER_PAGE' => 100,
                'HISTORY_DUPLICATES'  => 'yes',
                'MAX_COL_LEN'         => 26,
                'QNAV_BUTTONS'        => 20,
                'HNAV_BUTTONS'        => 22,
                'SORT_DEFAULT'        => 'Rating',
                '3D_GRAPHS'           => 1,
                'USE_MAILPARSE'       => 1,
                'OPTMODE'             => 'NONE',
                'LOCAL_DOMAIN'        => 'localhost',
                'AUTODETECT'          => 1,
                'OPENSOURCE'          => 0,
                /* Is there a website which provides dedicated infos? */
                'PACKAGE_WEBSITE'     => 'https://www.pfsense.org/',
                /* Is there a forum which provides dedicated infos? */
                'PACKAGE_FORUM'       => 'https://www.pfsense.org/',
                /*
                 * Is there a issue tracker which allows to fill a
                 * support request or a bug report?
                 */
                'PACKAGE_TRACKER'     => 'https://www.pfsense.org/',
                /* 'DATE_FORMAT'        => '%d.%m.%Y %H:%M' */
                'DATE_FORMAT'         => '%b %d %H:%M'
               );
               
?>
