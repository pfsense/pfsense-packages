<?php
/* $Id$ */
/*
	dspam-settings.php
	Copyright (C) 2006 Daniel S. Haischt
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
  $pgtitle = array(gettext("Services"),
                   gettext("DSPAM"),
                   gettext("Advanced Settings"),
                   gettext("Overview"));

  require("guiconfig.inc");
  include("/usr/local/pkg/dspam.inc");

  if (isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER'])) {

  $pconfig['sectionid'] = $_GET['sectionid'];

  $pconfig['sdriver'] = $config['installedpackages']['dspam']['config'][0]['storage-driver'];
  /* ============================================================================================= */
  /* == MySQL                                                                                   == */
  /* ============================================================================================= */
  $pconfig['msqlserver'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-server'];
  $pconfig['msqlport'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-port'];
  $pconfig['msqluser'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-user'];
  $pconfig['msqlpwd'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-password'];
  $pconfig['msqldb'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-database'];
  $pconfig['msqlcomp'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-compress'];
  $pconfig['msqlsuqt'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-squote'];
  $pconfig['msqlccache'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-ccache'];
  $pconfig['msqluid'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-uid'];
  /* ============================================================================================= */
  /* == SQLite                                                                                  == */
  /* ============================================================================================= */
  $pconfig['slitepr'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['sqlite-pragma'];
  /* ============================================================================================= */
  /* == PostgreSQL                                                                              == */
  /* ============================================================================================= */
  $pconfig['pgserver'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-server'];
  $pconfig['pgport'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-port'];
  $pconfig['pguser'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-user'];
  $pconfig['pgpwd'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-password'];
  $pconfig['pgdb'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-database'];
  $pconfig['pgccache'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-ccache'];
  $pconfig['pguid'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-uid'];
  /* ============================================================================================= */
  /* == Oracle                                                                                  == */
  /* ============================================================================================= */
  $pconfig['oraserver'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['ora-server'];
  $pconfig['orauser'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['ora-user'];
  $pconfig['orapwd'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['ora-password'];
  $pconfig['orasch'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['ora-schema'];
  /* ============================================================================================= */
  /* == Hash                                                                                    == */
  /* ============================================================================================= */
  $pconfig['hsrmax'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-rec-max'];
  $pconfig['hsatex'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-auto-ex'];
  $pconfig['hsmxex'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-max-ext'];
  $pconfig['hsexsz'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-ext-size'];
  $pconfig['hsmxse'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-max-seek'];
  $pconfig['hsccus'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-co-user'];
  $pconfig['hscoca'] = $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-co-cache'];
  /* ============================================================================================= */
  /* == Delivery Settings                                                                       == */
  /* ============================================================================================= */
  $pconfig['dagent'] = $config['installedpackages']['dspam']['config'][0]['tdelivery-agent'];
  $pconfig['dsthinc'] = $config['installedpackages']['dspam']['config'][0]['thin-client'];
  $pconfig['tcpipdel'] = $config['installedpackages']['dspam']['config'][0]['tcpip-delivery'];
  $pconfig['dhost'] = $config['installedpackages']['dspam']['config'][0]['tcpip-delivery-host'];
  $pconfig['dport'] = $config['installedpackages']['dspam']['config'][0]['tcpip-delivery-port'];
  $pconfig['dident'] = $config['installedpackages']['dspam']['config'][0]['tcpip-delivery-ident'];
  $pconfig['delproto'] = $config['installedpackages']['dspam']['config'][0]['tcpip-delivery-proto'];
  $pconfig['onfail'] = $config['installedpackages']['dspam']['config'][0]['delivery-onfail'];
  /* ============================================================================================= */
  /* == DSPAM Debugging Options                                                                 == */
  /* ============================================================================================= */
  $pconfig['enabledbg'] = $config['installedpackages']['dspam']['config'][0]['debug-enable'];
  $pconfig['debug'] = $config['installedpackages']['dspam']['config'][0]['debug-whom'];
  $pconfig['dopt'] = $config['installedpackages']['dspam']['config'][0]['debug-options'];
  /* ============================================================================================= */
  /* == DSPAM Engine Settings                                                                   == */
  /* ============================================================================================= */
  $pconfig['tmode'] = $config['installedpackages']['dspam']['config'][0]['training-mode'];
  $pconfig['testct'] = $config['installedpackages']['dspam']['config'][0]['test-cond-training'];
  $pconfig['pvalue'] = $config['installedpackages']['dspam']['config'][0]['pvalue'];
  $pconfig['ipdrive'] = $config['installedpackages']['dspam']['config'][0]['improbability-drive'];
  /* ============================================================================================= */
  /* == LDAP Settings                                                                           == */
  /* ============================================================================================= */
  $pconfig['enableldap'] = $config['installedpackages']['dspam']['config'][0]['ldap-enable'];
  $pconfig['ldapmode'] = $config['installedpackages']['dspam']['config'][0]['ldap-mode'];
  $pconfig['ldaphost'] = $config['installedpackages']['dspam']['config'][0]['ldap-host'];
  $pconfig['ldapfilter'] = $config['installedpackages']['dspam']['config'][0]['ldap-filter'];
  $pconfig['ldapbase'] = $config['installedpackages']['dspam']['config'][0]['ldap-base'];
  /* ============================================================================================= */
  /* == Miscellaneous Settings                                                                  == */
  /* ============================================================================================= */
  $pconfig['foatt'] = $config['installedpackages']['dspam']['config'][0]['failover-attempts'];
  $pconfig['enablesbl'] = $config['installedpackages']['dspam']['config'][0]['sbl-enable'];
  $pconfig['sblhost'] = $config['installedpackages']['dspam']['config'][0]['sbl-host'];
  $pconfig['enablerbl'] = $config['installedpackages']['dspam']['config'][0]['rbl-inoculate'];
  $pconfig['enablenoti'] = $config['installedpackages']['dspam']['config'][0]['notification-email'];
  $pconfig['dspamdomain'] = $config['installedpackages']['dspam']['config'][0]['dspam-domain'];
  $pconfig['dspamcontact'] = $config['installedpackages']['dspam']['config'][0]['dspam-contact'];
  /* ============================================================================================= */
  /* == Maintainance Settings                                                                   == */
  /* ============================================================================================= */
  $pconfig['psig'] = $config['installedpackages']['dspam']['config'][0]['purge-signatures'];
  $pconfig['pneut'] = $config['installedpackages']['dspam']['config'][0]['purge-neutral'];
  $pconfig['punu'] = $config['installedpackages']['dspam']['config'][0]['purge-unused'];
  $pconfig['phapa'] = $config['installedpackages']['dspam']['config'][0]['purge-hapaxes'];
  $pconfig['pones'] = $config['installedpackages']['dspam']['config'][0]['purge-hits-1s'];
  $pconfig['ponei'] = $config['installedpackages']['dspam']['config'][0]['purge-hits-1i'];
  /* ============================================================================================= */
  /* == System Settings                                                                         == */
  /* ============================================================================================= */
  $pconfig['locmx'] = $config['installedpackages']['dspam']['config'][0]['local-mx'];
  $pconfig['enablesysl'] = $config['installedpackages']['dspam']['config'][0]['system-log'];
  $pconfig['enableusel'] = $config['installedpackages']['dspam']['config'][0]['user-log'];
  $pconfig['optinout'] = $config['installedpackages']['dspam']['config'][0]['filter-opt'];
  $pconfig['enableptoh'] = $config['installedpackages']['dspam']['config'][0]['parse-to-headers'];
  $pconfig['enablecmop'] = $config['installedpackages']['dspam']['config'][0]['change-mode-on-parse'];
  $pconfig['enablecuop'] = $config['installedpackages']['dspam']['config'][0]['change-user-on-parse'];
  $pconfig['enablebmta'] = $config['installedpackages']['dspam']['config'][0]['broken-mta-settings'];
  $pconfig['maxmsgs'] = $config['installedpackages']['dspam']['config'][0]['max-message-size'];
  $pconfig['procbias'] = $config['installedpackages']['dspam']['config'][0]['processor-bias'];
  /* ============================================================================================= */
  /* == ClamAV Engine Settings                                                                  == */
  /* ============================================================================================= */
  $pconfig['enableclam'] = $config['installedpackages']['dspam']['config'][0]['clamav-enable'];
  $pconfig['clamport'] = $config['installedpackages']['dspam']['config'][0]['clamav-port'];
  $pconfig['clamhost'] = $config['installedpackages']['dspam']['config'][0]['clamav-host'];
  $pconfig['clamresp'] = $config['installedpackages']['dspam']['config'][0]['clamav-response'];
  /* ============================================================================================= */
  /* == DSPAM Daemon Settings (Server)                                                          == */
  /* ============================================================================================= */
  $pconfig['dsport'] = $config['installedpackages']['dspam']['config'][0]['dspam-server-port'];
  $pconfig['dsqsize'] = $config['installedpackages']['dspam']['config'][0]['dspam-server-queue-size'];
  $pconfig['dspid'] = $config['installedpackages']['dspam']['config'][0]['dspam-server-pid'];
  $pconfig['dssmode'] = $config['installedpackages']['dspam']['config'][0]['dspam-server-mode'];
  $pconfig['serverparam'] = $config['installedpackages']['dspam']['config'][0]['dspam-server-params'];
  $pconfig['serverid'] = $config['installedpackages']['dspam']['config'][0]['dspam-server-id'];
  $pconfig['serversock'] = $config['installedpackages']['dspam']['config'][0]['dspam-server-socket'];
  /* ============================================================================================= */
  /* == DSPAM Daemon Settings (Client)                                                          == */
  /* ============================================================================================= */
  $pconfig['enabledsclient'] = $config['installedpackages']['dspam']['config'][0]['dspam-client-enable'];
  $pconfig['dsclhost'] = $config['installedpackages']['dspam']['config'][0]['dspam-client-host'];
  $pconfig['dsclport'] = $config['installedpackages']['dspam']['config'][0]['dspam-client-port'];
  $pconfig['dsclident'] = $config['installedpackages']['dspam']['config'][0]['dspam-client-id'];

  if (!is_array($config['installedpackages']['dspam']['config'][0]['tuser'])) {
    $config['installedpackages']['dspam']['config'][0]['tuser'] = array();
  }
  if (!is_array($config['installedpackages']['dspam']['config'][0]['algorithm'])) {
    $config['installedpackages']['dspam']['config'][0]['algorithm'] = array();
  }
  if (!is_array($config['installedpackages']['dspam']['config'][0]['feature'])) {
    $config['installedpackages']['dspam']['config'][0]['feature'] = array();
  }
  if (!is_array($config['installedpackages']['dspam']['config'][0]['preference'])) {
    $config['installedpackages']['dspam']['config'][0]['preference'] = array();
  }
  if (!is_array($config['installedpackages']['dspam']['config'][0]['override'])) {
    $config['installedpackages']['dspam']['config'][0]['override'] = array();
  }
  if (!is_array($config['installedpackages']['dspam']['config'][0]['header'])) {
    $config['installedpackages']['dspam']['config'][0]['header'] = array();
  }
  if (!is_array($config['installedpackages']['dspam']['config'][0]['bmta'])) {
    $config['installedpackages']['dspam']['config'][0]['bmta'] = array();
  }

  $t_users = &$config['installedpackages']['dspam']['config'][0]['tuser'];
  $t_features = &$config['installedpackages']['dspam']['config'][0]['feature'];
  $t_algos = &$config['installedpackages']['dspam']['config'][0]['algorithm'];
  $t_prefs = &$config['installedpackages']['dspam']['config'][0]['preference'];
  $t_overr = &$config['installedpackages']['dspam']['config'][0]['override'];
  $t_headers = &$config['installedpackages']['dspam']['config'][0]['header'];
  $t_bmtas = &$config['installedpackages']['dspam']['config'][0]['bmta'];
  $t_spwds = &$config['installedpackages']['dspam']['config'][0]['server-pwd'];

if ($_POST) {

  /* hash */
  $error_bucket = array();
  /* simple error list */
  unset($input_errors);
  $pconfig = $_POST;

  /* input validation */
  if($_POST['sdriver'] == "mysql") {
    if (! $_POST['msqlserver'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid MySQL server name value.",
                              "field" => "msqlserver");
    } else {
      if (strpos($_POST['msqlserver'], '/') === false) {
        foreach (explode(' ', $_POST['msqlserver']) as $ts) {
          if (!is_domain($ts)) {
            $error_bucket[] = array("error" => "A MySQL server name may only contain the characters a-z, 0-9, '-' and '.'.",
                                    "field" => "msqlserver");
            break;
          }
        }
      }
    }
    /* if we are going to use a TCP/IP base MySQL connection, a port value is required */
    if (! is_port($_POST['msqlport']) && strpos($_POST['msqlserver'], '/') === false) {
      $error_bucket[] = array("error" => "You must specify a valid MySQL port value.",
                              "field" => "msqlport");
    }
    if (! $_POST['msqluser'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid MySQL username value.",
                              "field" => "msqluser");
    }
    if (! $_POST['msqlpwd'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid MySQL password value.",
                              "field" => "msqlpwd");
    }
    if (! $_POST['msqldb'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid MySQL database value.",
                              "field" => "msqldb");
    }
    if ($_POST['msqlccache'] && !is_numericint($_POST['msqlccache'])) {
      $error_bucket[] = array("error" => "You must specify a valid integer value as a connection cache value.",
                              "field" => "msqlccache");
    }
  } else if($_POST['sdriver'] == "sqlite") {
    /* NOP */
  } else if($_POST['sdriver'] == "bdb") {
    /* NOP */
  } else if($_POST['sdriver'] == "pgsql") {
    if (! $_POST['pgserver'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid PostgreSQL server name value.",
                              "field" => "pgserver");
    } else {
      foreach (explode(' ', $_POST['pgserver']) as $ts) {
        if (!is_domain($ts)) {
          $error_bucket[] = array("error" => "A PostgreSQL server name may only contain the characters a-z, 0-9, '-' and '.'.",
                                  "field" => "pgserver");
          break;
        }
      }
    }
    if (! is_port($_POST['pgport'])) {
      $error_bucket[] = array("error" => "You must specify a valid PostgreSQL port value.",
                              "field" => "pgport");
    }
    if (! $_POST['pguser'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid PostgreSQL username value.",
                              "field" => "pguser");
    }
    if (! $_POST['pgpwd'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid PostgreSQL password value.",
                              "field" => "pgpwd");
    }
    if (! $_POST['pgdb'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid PostgreSQL database value.",
                              "field" => "pgdb");
    }
    if ($_POST['pgccache'] && !is_numericint($_POST['pgccache'])) {
      $error_bucket[] = array("error" => "You must specify a valid integer value as a connection cache value.",
                              "field" => "pgccache");
    }
  } else if($_POST['sdriver'] == "oracle") {
    if (! $_POST['oraserver'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid Oracle server connection string.",
                              "field" => "oraserver");
    }
    if (! $_POST['orauser'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid Oracle username value.",
                              "field" => "orauser");
    }
    if (! $_POST['orapwd'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid Oracle password value.",
                              "field" => "orapwd");
    }
    if (! $_POST['orasch'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid Oracle schema value.",
                              "field" => "orasch");
    }
  } else if($_POST['sdriver'] == "hash") {
    if ($_POST['hsrmax'] && !is_numericint($_POST['hsrmax'])) {
      $error_bucket[] = array("error" => "You must specify a valid integer value as a number for the initial records to be created.",
                              "field" => "hsrmax");
    }
    if ($_POST['hsmxex'] && !is_numericint($_POST['hsmxex'])) {
      $error_bucket[] = array("error" => "You must specify a valid integer value as a number for the maximum extends.",
                              "field" => "hsmxex");
    }
    if ($_POST['hsexsz'] && !is_numericint($_POST['hsexsz'])) {
      $error_bucket[] = array("error" => "You must specify a valid integer value as a number for the record size.",
                              "field" => "hsexsz");
    }
    if ($_POST['hsmxse'] && !is_numericint($_POST['hsmxse'])) {
      $error_bucket[] = array("error" => "You must specify a valid integer value as a number for the maximum number of records to seek.",
                              "field" => "hsmxse");
    }
    if ($_POST['hscoca'] && !is_numericint($_POST['hscoca'])) {
      $error_bucket[] = array("error" => "You must specify a valid integer value as a number for hash connection cache.",
                              "field" => "hscoca");
    }
  }

  if ($_POST['tcpipdel'] == "yes") {
    if (! $_POST['dhost'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid server name value for the DSPAM deliver host.",
                              "field" => "dhost");
    } else {
      foreach (explode(' ', $_POST['dhost']) as $ts) {
        if (!is_domain($ts)) {
          $error_bucket[] = array("error" => "A DSPAM delivery host name may only contain the characters a-z, 0-9, '-' and '.'.",
                                  "field" => "dhost");
          break;
        }
      }
    }
    if (! is_port($_POST['dport'])) {
      $error_bucket[] = array("error" => "You must specify a valid port value for the DSPAM delivery host.",
                              "field" => "dport");
    }
    if (! $_POST['dident'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid identification string for the DSPAM delivery host.",
                              "field" => "dident");
    }
  }

  if ($_POST['enabledbg'] == "yes") {
    if (! $_POST['debug'] <> "") {
      $error_bucket[] = array("error" => "You must specify a non-zero value for the debug parameter.",
                              "field" => "debug");
    }
    if (! $_POST['dopt'] <> "") {
      $error_bucket[] = array("error" => "You must specify a non-zero value for the debug options.",
                              "field" => "dopt");
    }
  }

  if ($_POST['enableldap'] == "yes") {
    if (! $_POST['ldaphost'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid server name value for the LDAP host.",
                              "field" => "ldaphost");
    } else {
      foreach (explode(' ', $_POST['ldaphost']) as $ts) {
        if (!is_domain($ts)) {
          $error_bucket[] = array("error" => "A LDAP host name may only contain the characters a-z, 0-9, '-' and '.'.",
                                  "field" => "ldaphost");
          break;
        }
      }
    }
    if (! $_POST['ldapfilter'] <> "") {
      $error_bucket[] = array("error" => "You must specify a non-zero value for the LDAP filter option or you may not be able to get any query result.",
                              "field" => "ldapfilter");
    }
    if (! $_POST['ldapbase'] <> "") {
      $error_bucket[] = array("error" => "You must specify a non-zero value for the LDAP base option or you may not be able to get any query result.",
                              "field" => "ldapbase");
    }
  }

  /* misc settings */
  if ($_POST['foatt'] && !is_numericint($_POST['foatt'])) {
    $error_bucket[] = array("error" => "You must specify a integer based value for the number of failover attempts.",
                            "field" => "foatt");
  }
  if ($_POST['enablesbl'] == "yes") {
    if (! $_POST['sblhost'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid server name value for the SBL host.",
                              "field" => "sblhost");
    } else {
      foreach (explode(' ', $_POST['sblhost']) as $ts) {
        if (!is_domain($ts)) {
          $error_bucket[] = array("error" => "A SBL host name may only contain the characters a-z, 0-9, '-' and '.'.",
                                  "field" => "sblhost");
          break;
        }
      }
    }
  }
  if(isset($_POST['enablenoti'])) {
    if($_POST['dspamcontact'] == "") {
      $error_bucket[] = array("error" => "It is necessary to provide a support contact, if you want DSPAM to send notification messages.",
                              "field" => "dspamcontact");
    }
    if(empty($_POST['whichdomain'])) {
      if ($_POST['dspamdomain'] == "") {
        $error_bucket[] = array("error" => "You must specify a valid domain name that should be used while sending DSPAM related mail messages.",
                                "field" => "dspamdomain");
      } else {
        if (!is_domain($_POST['dspamdomain'])) {
          $error_bucket[] = array("error" => "You must specify a valid domain name that should be used while sending DSPAM related mail messages.",
                                  "field" => "dspamdomain");
        }
      }
    }
  }

  /* Maintanance Settings */
  if (! $_POST['psig'] || $_POST['psig'] == "") {
    $error_bucket[] = array("error" => "You must specify a value for the number of signatures to be purged.",
                            "field" => "psig");
  } else if (! $_POST['psig'] == "off") {
    if (!is_numericint($_POST['psig'])) {
      $error_bucket[] = array("error" => "You must specify a valide integer value for the number of signatures to be purged.",
                              "field" => "psig");
    }
  }
  if (! $_POST['pneut'] || $_POST['pneut'] == "") {
    $error_bucket[] = array("error" => "You must specify a value for the number of neutrals to be purged.",
                            "field" => "pneut");
  } else if (! $_POST['pneut'] == "off") {
    if (!is_numericint($_POST['pneut'])) {
    $error_bucket[] = array("error" => "You must specify a valide integer value for the number of neutrals to be purged.",
                            "field" => "pneut");
    }
  }
  if (! $_POST['punu'] || $_POST['punu'] == "") {
    $error_bucket[] = array("error" => "You must specify a value for the number of unused tokens to be purged.",
                            "field" => "punu");
  } else if (! $_POST['punu'] == "off") {
    if (!is_numericint($_POST['punu'])) {
    $error_bucket[] = array("error" => "You must specify a valide integer value for the number of unused tokens to be purged.",
                            "field" => "punu");
    }
  }
  if (! $_POST['phapa'] || $_POST['phapa'] == "") {
    $input_errors[] = "You must specify a value for the number of hapaxes to be purged.";
    $input_error_fields[] = "phapa";
  } else if (! $_POST['phapa'] == "off") {
    if (!is_numericint($_POST['phapa'])) {
    $error_bucket[] = array("error" => "You must specify a valide integer value for the number of hapaxes to be purged.",
                            "field" => "phapa");
    }
  }
  if (! $_POST['pones'] || $_POST['pones'] == "") {
    $error_bucket[] = array("error" => "You must specify a value for the number of tokens with only 1 spam hit to be purged.",
                            "field" => "pones");
  } else if (! $_POST['pones'] == "off") {
    if (!is_numericint($_POST['pones'])) {
    $error_bucket[] = array("error" => "You must specify a valide integer value for the number of tokens with only 1 spam hit to be purged.",
                            "field" => "pones");
    }
  }
  if (! $_POST['ponei'] || $_POST['ponei'] == "") {
    $error_bucket[] = array("error" => "You must specify a value for the number of  tokens with only 1 innocent hit to be purged.",
                            "field" => "ponei");
  } else if (! $_POST['ponei'] == "off") {
    if (!is_numericint($_POST['ponei'])) {
    $error_bucket[] = array("error" => "You must specify a valide integer value for the number of  tokens with only 1 innocent hit to be purged.",
                            "field" => "ponei");
    }
  }

  /* System Settings */
  if (! is_ipaddr($_POST['locmx'])) {
    $error_bucket[] = array("error" => "You must specify a valid IP address for the local MX parameter.",
                            "field" => "locmx");
  }
  if ($_POST['maxmsgs'] && !is_numericint($_POST['maxmsgs'])) {
    $error_bucket[] = array("error" => "You must specify a integer based value for the maximum message size.",
                            "field" => "maxmsgs");
  }

  /* ClamAV Settings */
  if ($_POST['enableclam'] == "yes") {
    if (! is_port($_POST['clamport'])) {
      $error_bucket[] = array("error" => "You must specify a valid port value for the ClamAV host.",
                              "field" => "clamport");
    }
    if (! $_POST['clamhost'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid server name value for the ClamAV host.",
                              "field" => "clamhost");
    } else {
      foreach (explode(' ', $_POST['clamhost']) as $ts) {
        if (!is_domain($ts)) {
          $error_bucket[] = array("error" => "A ClamAV host name may only contain the characters a-z, 0-9, '-' and '.'.",
                                  "field" => "clamhost");
          break;
        }
      }
    }
  }

  /*                                */
  /* DSPAM Daemon Settings (Server) */
  /*                                */

  /* at least the DSPAM thin client (dspamc)
   * should force the user to configure the
   * DSPAM daemon.
   */
  if (isset($_POST['dsthinc'])) {
    if (! is_port($_POST['dsport'])) {
      $error_bucket[] = array("error" => "You must specify a valid port value for the DSPAM host.",
                              "field" => "dsport");
    }
    if ($_POST['dsqsize'] && !is_numericint($_POST['dsqsize'])) {
      $error_bucket[] = array("error" => "You must specify a valid integer value as a number for the server queue size.",
                              "field" => "dsqsize");
    }
    if (! $_POST['dspid'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid value as PID file for the DSPAM server.",
                              "field" => "dspid");
    }
    if ($_POST['dssmode'] == "standard") {
      if (! $_POST['serverparam'] <> "") {
        $error_bucket[] = array("error" => "You must specify some valid parameters to be passed to the LMTP server.",
                                "field" => "serverparam");
      }
      if (! $_POST['serverid'] <> "") {
        $error_bucket[] = array("error" => "You must specify a valid identification string to be passed to the LMTP server.",
                                "field" => "serverid");
      }
      if ($_POST['serversock'] && $_POST['serversock'] <> "") {
        if (strpos($_POST['serversock'], '/') === false) {
          $error_bucket[] = array("error" => "You must specify a valid value for the location of a Unix domain socket.",
                                  "field" => "serversock");
        }
      }
    }
  }

  /* DSPAM Daemon Settings (Client) */
  if ($_POST['enabledsclient'] == "yes") {
    if (! $_POST['dsclhost'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid server name value for the DSPAM client host.",
                              "field" => "dsclhost");
    } else {
      foreach (explode(' ', $_POST['dsclhost']) as $ts) {
        if (!is_domain($ts)) {
          $error_bucket[] = array("error" => "A DSPAM client host name may only contain the characters a-z, 0-9, '-' and '.'.",
                                  "field" => "dsclhost");
          break;
        }
      }
    }
    if (! is_port($_POST['dsclport'])) {
      $error_bucket[] = array("error" => "You must specify a valid port value for the DSPAM client host.",
                              "field" => "dsclport");
    }
    if (! $_POST['dsclident'] <> "") {
      $error_bucket[] = array("error" => "You must specify a valid value as identification string for the DSPAM client.",
                              "field" => "dsclident");
    }
  }

  if (is_array($error_bucket))
    foreach($error_bucket as $elem)
      $input_errors[] =& $elem["error"];

	/* if this is an AJAX caller then handle via JSON */
	if(isAjax() && is_array($input_errors)) {
	  input_errors2Ajax($input_errors);
		exit;
	}

  if (!$input_errors) {
    $config['installedpackages']['dspam']['config'][0]['storage-driver'] = $_POST['sdriver'];
    unset($config['installedpackages']['dspam']['config'][0]['dbsettings']);

    if($_POST['sdriver'] == "mysql") {
      /* ====================================================================== */
      /* == String and integer values                                        == */
      /* ====================================================================== */
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-server'] = $_POST['msqlserver'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-port'] = $_POST['msqlport'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-user'] = $_POST['msqluser'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-password'] = $_POST['msqlpwd'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-database'] = $_POST['msqldb'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-ccache'] = $_POST['msqlccache'];
      /* ====================================================================== */
      /* == Boolean values                                                   == */
      /* ====================================================================== */
		  if($_POST['msqlcomp'] == "yes")
			  $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-compress'] = $_POST['msqlcomp'];
		  else
			  unset($config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-compress']);
		  if($_POST['msqlsuqt'] == "yes")
			  $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-squote'] = $_POST['msqlsuqt'];
		  else
			  unset($config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-squote']);
		  if($_POST['msqluid'] == "yes")
			  $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-uid'] = $_POST['msqluid'];
		  else
			  unset($config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['mysql-uid']);
    } else if($_POST['sdriver'] == "sqlite") {
      /* ====================================================================== */
      /* == String and integer values                                        == */
      /* ====================================================================== */
    	if ($_POST['slitepr'])
      	$config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['sqlite-pragma'] = $_POST['slitepr'];
    } else if($_POST['sdriver'] == "bdb") {
    	/* NOP */
    } else if($_POST['sdriver'] == "pgsql") {
      /* ====================================================================== */
      /* == String and integer values                                        == */
      /* ====================================================================== */
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-server'] = $_POST['pgserver'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-port'] = $_POST['pgport'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-user'] = $_POST['pguser'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-password'] = $_POST['pgpwd'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-database'] = $_POST['pgdb'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-ccache'] = $_POST['pgccache'];
      /* ====================================================================== */
      /* == Boolean values                                                   == */
      /* ====================================================================== */
		  if($_POST['pguid'] == "yes")
			  $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-uid'] = $_POST['pguid'];
		  else
			  unset($config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['pgsql-uid']);
    } else if($_POST['sdriver'] == "oracle") {
      /* ====================================================================== */
      /* == String and integer values                                        == */
      /* ====================================================================== */
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['ora-server'] = $_POST['oraserver'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['ora-user'] = $_POST['orauser'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['ora-password'] = $_POST['orapwd'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['ora-schema'] = $_POST['orasch'];
    } else if($_POST['sdriver'] == "hash") {
      /* ====================================================================== */
      /* == String and integer values                                        == */
      /* ====================================================================== */
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-rec-max'] = $_POST['hsrmax'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-max-ext'] = $_POST['hsmxex'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-ext-size'] = $_POST['hsexsz'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-max-seek'] = $_POST['hsmxse'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-co-user'] = $_POST['hsccus'];
      $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-co-cache'] = $_POST['hscoca'];
      /* ====================================================================== */
      /* == Boolean values                                                   == */
      /* ====================================================================== */
		  if($_POST['hsatex'] == "yes")
			  $config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-auto-ex'] = $_POST['hsatex'];
		  else
			  unset($config['installedpackages']['dspam']['config'][0]['dbsettings'][0]['hash-auto-ex']);
    }

    $config['installedpackages']['dspam']['config'][0]['tdelivery-agent'] = $_POST['dagent'];
    $config['installedpackages']['dspam']['config'][0]['thin-client'] = $_POST['dsthinc'];

	  if($_POST['tcpipdel'] == "yes") {
		  $config['installedpackages']['dspam']['config'][0]['tcpip-delivery'] = $_POST['tcpipdel'];
		  $config['installedpackages']['dspam']['config'][0]['tcpip-delivery-host'] = $_POST['dhost'];
		  $config['installedpackages']['dspam']['config'][0]['tcpip-delivery-port'] = $_POST['dport'];
		  $config['installedpackages']['dspam']['config'][0]['tcpip-delivery-ident'] = $_POST['dident'];
		  $config['installedpackages']['dspam']['config'][0]['tcpip-delivery-proto'] = $_POST['delproto'];
	  } else {
		  unset($config['installedpackages']['dspam']['config'][0]['tcpip-delivery']);
		  unset($config['installedpackages']['dspam']['config'][0]['tcpip-delivery-host']);
		  unset($config['installedpackages']['dspam']['config'][0]['tcpip-delivery-port']);
		  unset($config['installedpackages']['dspam']['config'][0]['tcpip-delivery-ident']);
		  unset($config['installedpackages']['dspam']['config'][0]['tcpip-delivery-proto']);
		}

    $config['installedpackages']['dspam']['config'][0]['delivery-onfail'] = $_POST['onfail'];

	  if($_POST['enabledbg'] == "yes") {
	    $config['installedpackages']['dspam']['config'][0]['debug-enable'] = $_POST['enabledbg'];
		  $config['installedpackages']['dspam']['config'][0]['debug-whom'] = $_POST['debug'];
		  $config['installedpackages']['dspam']['config'][0]['debug-options'] = $_POST['dopt'];
	  } else {
	    unset($config['installedpackages']['dspam']['config'][0]['debug-enable']);
		  unset($config['installedpackages']['dspam']['config'][0]['debug-whom']);
		  unset($config['installedpackages']['dspam']['config'][0]['debug-options']);
	  }

	  /* DSPAM engine settings */
    $config['installedpackages']['dspam']['config'][0]['training-mode'] = $_POST['tmode'];
	  if($_POST['testct'] == "yes") {
		  $config['installedpackages']['dspam']['config'][0]['test-cond-training'] = $_POST['testct'];
	  } else {
		  unset($config['installedpackages']['dspam']['config'][0]['test-cond-training']);
	  }
	  $config['installedpackages']['dspam']['config'][0]['pvalue'] = $_POST['pvalue'];
	  if($_POST['ipdrive'] == "yes") {
		  $config['installedpackages']['dspam']['config'][0]['improbability-drive'] = $_POST['ipdrive'];
	  } else {
		  unset($config['installedpackages']['dspam']['config'][0]['improbability-drive']);
	  }

	  /* LDAP related settings */
	  if($_POST['enableldap'] == "yes") {
	    $config['installedpackages']['dspam']['config'][0]['ldap-enable'] = $_POST['enableldap'];
		  $config['installedpackages']['dspam']['config'][0]['ldap-mode'] = $_POST['ldapmode'];
		  $config['installedpackages']['dspam']['config'][0]['ldap-host'] = $_POST['ldaphost'];
		  $config['installedpackages']['dspam']['config'][0]['ldap-filter'] = $_POST['ldapfilter'];
		  $config['installedpackages']['dspam']['config'][0]['ldap-base'] = $_POST['ldapbase'];
	  } else {
	    unset($config['installedpackages']['dspam']['config'][0]['ldap-enable']);
		  unset($config['installedpackages']['dspam']['config'][0]['ldap-mode']);
		  unset($config['installedpackages']['dspam']['config'][0]['ldap-host']);
		  unset($config['installedpackages']['dspam']['config'][0]['ldap-filter']);
		  unset($config['installedpackages']['dspam']['config'][0]['ldap-base']);
	  }

	  /* misc settings */
	  $config['installedpackages']['dspam']['config'][0]['failover-attempts'] = $_POST['foatt'];
	  if($_POST['enablesbl'] == "yes") {
	    $config['installedpackages']['dspam']['config'][0]['sbl-enable'] = $_POST['enablesbl'];
	    $config['installedpackages']['dspam']['config'][0]['sbl-host'] = $_POST['sblhost'];
	  } else {
	    unset($config['installedpackages']['dspam']['config'][0]['sbl-enable']);
	    unset($config['installedpackages']['dspam']['config'][0]['sbl-host']);
	  }
	  if($_POST['enablerbl'] == "yes") {
      $config['installedpackages']['dspam']['config'][0]['rbl-inoculate'] = $_POST['enablerbl'];
	  } else {
      unset($config['installedpackages']['dspam']['config'][0]['rbl-inoculate']);
	  }
	  if($_POST['enablenoti'] == "yes") {
      $config['installedpackages']['dspam']['config'][0]['notification-email'] = $_POST['enablenoti'];
      $config['installedpackages']['dspam']['config'][0]['dspam-contact'] = $_POST['dspamcontact'];
	  } else {
      unset($config['installedpackages']['dspam']['config'][0]['notification-email']);
      unset($config['installedpackages']['dspam']['config'][0]['dspam-domain']);
      unset($config['installedpackages']['dspam']['config'][0]['dspam-contact']);
	  }
	  if($_POST['whichdomain'] == "yes") {
      unset($config['installedpackages']['dspam']['config'][0]['dspam-domain']);
	  } else {
	    $config['installedpackages']['dspam']['config'][0]['dspam-domain'] = $_POST['dspamdomain'];
    }

    /* Maintainance Settings */
    $config['installedpackages']['dspam']['config'][0]['purge-signatures'] = $_POST['psig'];
    $config['installedpackages']['dspam']['config'][0]['purge-neutral'] = $_POST['pneut'];
    $config['installedpackages']['dspam']['config'][0]['purge-unused'] = $_POST['punu'];
    $config['installedpackages']['dspam']['config'][0]['purge-hapaxes'] = $_POST['phapa'];
    $config['installedpackages']['dspam']['config'][0]['purge-hits-1s'] = $_POST['pones'];
    $config['installedpackages']['dspam']['config'][0]['purge-hits-1i'] = $_POST['ponei'];

    /* System Settings */
    $config['installedpackages']['dspam']['config'][0]['local-mx'] = $_POST['locmx'];
    $config['installedpackages']['dspam']['config'][0]['local-mx'] = $_POST['locmx'];
	  if($_POST['enablesysl'] == "yes") {
      $config['installedpackages']['dspam']['config'][0]['system-log'] = $_POST['enablesysl'];
	  } else {
      unset($config['installedpackages']['dspam']['config'][0]['system-log']);
	  }
	  if($_POST['enableusel'] == "yes") {
      $config['installedpackages']['dspam']['config'][0]['user-log'] = $_POST['enableusel'];
	  } else {
      unset($config['installedpackages']['dspam']['config'][0]['user-log']);
	  }
	  $config['installedpackages']['dspam']['config'][0]['filter-opt'] = $_POST['optinout'];
	  if($_POST['enableptoh'] == "yes") {
      $config['installedpackages']['dspam']['config'][0]['parse-to-headers'] = $_POST['enableptoh'];
	  } else {
      unset($config['installedpackages']['dspam']['config'][0]['parse-to-headers']);
	  }
	  if($_POST['enablecmop'] == "yes") {
      $config['installedpackages']['dspam']['config'][0]['change-mode-on-parse'] = $_POST['enablecmop'];
	  } else {
      unset($config['installedpackages']['dspam']['config'][0]['change-mode-on-parse']);
	  }
	  if($_POST['enablecuop'] == "yes") {
      $config['installedpackages']['dspam']['config'][0]['change-user-on-parse'] = $_POST['enablecuop'];
	  } else {
      unset($config['installedpackages']['dspam']['config'][0]['change-user-on-parse']);
	  }
	  if($_POST['enablecuop'] == "yes") {
      $config['installedpackages']['dspam']['config'][0]['change-user-on-parse'] = $_POST['enablecuop'];
	  } else {
      unset($config['installedpackages']['dspam']['config'][0]['change-user-on-parse']);
	  }
	  if($_POST['enablebmta'] == "yes") {
      $config['installedpackages']['dspam']['config'][0]['broken-mta-settings'] = $_POST['enablebmta'];
	  } else {
      unset($config['installedpackages']['dspam']['config'][0]['broken-mta-settings']);
	  }
	  $config['installedpackages']['dspam']['config'][0]['max-message-size'] = $_POST['maxmsgs'];
	  if($_POST['procbias'] == "yes") {
      $config['installedpackages']['dspam']['config'][0]['processor-bias'] = $_POST['procbias'];
	  } else {
      unset($config['installedpackages']['dspam']['config'][0]['processor-bias']);
	  }

	  /* ClamAV related settings */
	  if($_POST['enableclam'] == "yes") {
	    $config['installedpackages']['dspam']['config'][0]['clamav-enable'] = $_POST['enableclam'];
      $config['installedpackages']['dspam']['config'][0]['clamav-port'] = $_POST['clamport'];
      $config['installedpackages']['dspam']['config'][0]['clamav-host'] = $_POST['clamhost'];
      $config['installedpackages']['dspam']['config'][0]['clamav-response'] = $_POST['clamresp'];
	  } else {
	    unset($config['installedpackages']['dspam']['config'][0]['clamav-enable']);
      unset($config['installedpackages']['dspam']['config'][0]['clamav-port']);
      unset($config['installedpackages']['dspam']['config'][0]['clamav-host']);
      unset($config['installedpackages']['dspam']['config'][0]['clamav-response']);
	  }

	  /* DSPAM daemon settings */
	  $config['installedpackages']['dspam']['config'][0]['dspam-server-port'] = $_POST['dsport'];
	  $config['installedpackages']['dspam']['config'][0]['dspam-server-queue-size'] = $_POST['dsqsize'];
	  $config['installedpackages']['dspam']['config'][0]['dspam-server-pid'] = $_POST['dspid'];
	  $config['installedpackages']['dspam']['config'][0]['dspam-server-mode'] = $_POST['dssmode'];
	  $config['installedpackages']['dspam']['config'][0]['dspam-server-params'] = $_POST['serverparam'];
	  $config['installedpackages']['dspam']['config'][0]['dspam-server-id'] = $_POST['serverid'];
	  $config['installedpackages']['dspam']['config'][0]['dspam-server-socket'] = $_POST['serversock'];

	  /* DSPAM client settings */
	  if($_POST['enabledsclient'] == "yes") {
	    $config['installedpackages']['dspam']['config'][0]['dspam-client-enable'] = $_POST['enabledsclient'];
      $config['installedpackages']['dspam']['config'][0]['dspam-client-host'] = $_POST['dsclhost'];
      $config['installedpackages']['dspam']['config'][0]['dspam-client-port'] = $_POST['dsclport'];
      $config['installedpackages']['dspam']['config'][0]['dspam-client-id'] = $_POST['dsclident'];
	  } else {
	    unset($config['installedpackages']['dspam']['config'][0]['dspam-client-enable']);
      unset($config['installedpackages']['dspam']['config'][0]['dspam-client-host']);
      unset($config['installedpackages']['dspam']['config'][0]['dspam-client-port']);
      unset($config['installedpackages']['dspam']['config'][0]['dspam-client-id']);
	  }

    write_config();

    $retval = 0;
    conf_mount_rw();
    config_lock();
    $retval = dspam_configure();
    config_unlock();
    $savemsg = get_std_save_message($retval);
    conf_mount_ro();
  }
}

/* did the user send a request to delete an item? */
if ($_GET['act'] == "del") {
	if ($_GET['what'] == "tuser" && $t_users[$_GET['id']]) {
		unset($t_users[$_GET['id']]);
		write_config();
		pfSenseHeader("dspam-settings.php");
		exit;
	} else if ($_GET['what'] == "feat" && $t_features[$_GET['id']]) {
		unset($t_features[$_GET['id']]);
		write_config();
		pfSenseHeader("dspam-settings.php");
		exit;
	} else if ($_GET['what'] == "algo" && $t_algos[$_GET['id']]) {
		unset($t_algos[$_GET['id']]);
		write_config();
		pfSenseHeader("dspam-settings.php");
		exit;
	}	else if ($_GET['what'] == "pref" && $t_prefs[$_GET['id']]) {
		unset($t_prefs[$_GET['id']]);
		write_config();
		pfSenseHeader("dspam-settings.php");
		exit;
	} else if ($_GET['what'] == "overr" && $t_overr[$_GET['id']]) {
		unset($t_overr[$_GET['id']]);
		write_config();
		pfSenseHeader("dspam-settings.php");
		exit;
	} else if ($_GET['what'] == "header" && $t_headers[$_GET['id']]) {
		unset($t_headers[$_GET['id']]);
		write_config();
		pfSenseHeader("dspam-settings.php");
		exit;
  } else if ($_GET['what'] == "bmta" && $t_bmtas[$_GET['id']]) {
		unset($t_bmtas[$_GET['id']]);
		write_config();
		pfSenseHeader("dspam-settings.php");
		exit;
  } else if ($_GET['what'] == "spwd" && $t_spwds[$_GET['id']]) {
		unset($t_spwds[$_GET['id']]);
		write_config();
		pfSenseHeader("dspam-settings.php");
		exit;
	}
}

  /* if ajax is calling, give them an update message */
  if(isAjax())
	  print_info_box_np($savemsg);

  include("head.inc");
  /* put your custom HTML head content here        */
  /* using some of the $pfSenseHead function calls */
  $jscriptstr = <<<EOD
<script type="text/javascript">
<!--

EOD;

  $jscriptstr .= getJScriptFunction(5);
  if (empty($_POST))
    $jscriptstr .= getJScriptFunction(6);
  $jscriptstr .= <<<EOD
//-->
</script>
EOD;

  $pfSenseHead->addScript($jscriptstr);
  echo $pfSenseHead->getHTML();?>

<body link="#000000" vlink="#000000" alink="#000000" <?php if (empty($_POST)) { echo "onLoad='checkDisabledState(document.iform);'"; } ?>>
  <?php include("fbegin.inc"); ?>
  <form action="dspam-settings.php" method="post" name="iform" id="iform">
  <input type="hidden" name="sectionid" id="sectionid" value="<?=$pconfig['sectionid'];?>" />
  <?php if ($input_errors) print_input_errors($input_errors); ?>
  <?php if ($savemsg) print_info_box($savemsg); ?>
  <p>
    <span class="vexpl">
      <span class="red">
        <strong>Note: </strong>
      </span>
      the options on this page are intended for use by advanced users only.
      Any setting found on this page is directly going into <code>dspam.conf</code>.
      Make sure you do not mess with settings, you do not understand.
    </span>
  </p>
  <p>
    <span class="vexpl">If you submit this page, the DSPAM daemon process will be restarted.</span>
  </p>
  <br />
  <table width="99%" border="0" cellpadding="0" cellspacing="0">
    <tr>
      <td>
        <?php
	      $tab_array = array();
	      $tab_array[] = array("System Status",   false, "/dspam-admin.php");
	      $tab_array[] = array("User Statistics", false, "/dspam-admin-stats.php");
	      $tab_array[] = array("Administration",  false, "/dspam-admin-prefs.php");
	      $tab_array[] = array("Settings",        true,  "/dspam-settings.php");
	      $tab_array[] = array("Control Center",  false, "/dspam-perf.php");
	      display_top_tabs($tab_array);
        ?>
      </td>
    </tr>
    <tr>
      <td>
	    <div id="mainarea">
		  <table id="maintable" name="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
		    <tr>
		      <td>
		        <p><strong>Settings</strong></p>
				    <ul style="font-size:0.95em; font-family:Verdana,Arial,sans-serif">
				      <li><a href="#db" class="redlnk">Database Settings<?php if ($pconfig['sectionid'] == "db") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				      <li><a href="#del" class="redlnk">Delivery Settings<?php if ($pconfig['sectionid'] == "del") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				      <li><a href="#priv" class="redlnk">DSPAM Privileges<?php if ($pconfig['sectionid'] == "priv") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				      <li><a href="#dbg" class="redlnk">DSPAM Debugging Options<?php if ($pconfig['sectionid'] == "dbg") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				      <li><a href="#eng" class="redlnk">DSPAM Engine Settings<?php if ($pconfig['sectionid'] == "eng") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				      <?php if (checkForLDAPSupport()): ?>
				      <li><a href="#ldap" class="redlnk">LDAP Settings<?php if ($pconfig['sectionid'] == "ldap") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				      <?php endif; ?>
				      <li><a href="#misc" class="redlnk">Miscellaneous Settings<?php if ($pconfig['sectionid'] == "misc") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				      <li><a href="#main" class="redlnk">Maintainance Settings<?php if ($pconfig['sectionid'] == "main") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				      <li><a href="#sys" class="redlnk">System Settings<?php if ($pconfig['sectionid'] == "sys") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				      <?php if (checkForClamAVSupport()): ?>
				      <li><a href="#clam" class="redlnk">ClamAV Engine Settings<?php if ($pconfig['sectionid'] == "clam") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				      <?php endif; ?>
				      <li><a href="#srv" class="redlnk">DSPAM Daemon Settings (Server)<?php if ($pconfig['sectionid'] == "srv") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				      <li><a href="#cli" class="redlnk">DSPAM Daemon Settings (Client)<?php if ($pconfig['sectionid'] == "cli") echo '<span class="red">&nbsp;&raquo;last modified&laquo;</span>'; ?></a></li>
				    </ul>
					  <br />
			    </td>
			  </tr>
		    <tr>
              <td>
                <table id="sortabletable0" name="sortabletable0" width="100%" border="0" cellpadding="10" cellspacing="0">
                  <tr>
                    <td colspan="2" valign="top" class="listtopic"><a name="db" style="visibility: hidden;">&nbsp;</a>Database Settings</td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Storage Driver</td>
                    <td width="78%" class="vtable">
                      <select name="sdriver" onChange="toggleDBSettings();" class="formselect">
                        <?php if (checkForMySQLSupport()): ?>
                        <option value="mysql" <?php if($pconfig['sdriver'] == "mysql") echo('selected="selected"');?>>mysql</option>
                        <?php endif; ?>
                        <?php if (checkForSQLiteSupport()): ?>
                        <option value="sqlite" <?php if($pconfig['sdriver'] == "sqlite") echo('selected="selected"');?>>sqlite</option>
                        <?php endif; ?>
                        <option value="bdb" <?php if($pconfig['sdriver'] == "bdb") echo('selected="selected"');?>>bdb</option>
                        <?php if (checkForPgSQLSupport()): ?>
                        <option value="pgsql" <?php if($pconfig['sdriver'] == "pgsql") echo('selected="selected"');?>>pgsql</option>
                        <?php endif; ?>
                        <option value="oracle" <?php if($pconfig['sdriver'] == "oracle") echo('selected="selected"');?>>oracle</option>
                        <option value="hash" <?php if($pconfig['sdriver'] == "hash") echo('selected="selected"');?>>hash</option>
                      </select>
                      <strong>Specifies the storage driver backend (library) to use.</strong>
                      <p>
                        <span class="vexpl">
                          IMPORTANT: Switching storage drivers requires more than merely changing this option.
                          If you do not wish to lose all of your data, you will need to migrate it to the new
                          backend before making this change.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <?php if ($pconfig['sdriver'] == "mysql" && checkForMySQLSupport()): ?>
                  <tbody id="DBmysql" style="display: table-row-group;">
                  <?php else: ?>
                  <tbody id="DBmysql" style="display: none;">
                  <?php endif; ?>
                    <tr>
                      <td width="22%" valign="top" class="vncell">MySQL Server</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("msqlserver", "formfld host"); ?> name="msqlserver" id="msqlserver" value="<?=htmlspecialchars($pconfig['msqlserver']);?>" />
                        <strong>
                          Either a reference to a Unix domain socket or a reference to a specific host.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">MySQL Port</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("msqlport", "formfld unknown"); ?> name="msqlport" id="msqlport" value="<?=htmlspecialchars($pconfig['msqlport']);?>" />
                        <strong>
                          Use this variable if you are going to a MySQL server instance using TCP/IP instead of a socket connection.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">MySQL User</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("msqluser", "formfld user"); ?> name="msqluser" id="msqluser" value="<?=htmlspecialchars($pconfig['msqluser']);?>" <?php if ($_POST && $input_error_fields && in_array("msqluser", $input_error_fields)) echo 'style="background-color: red;" onFocus="this.style.backgroundColor = \'white\';" onBlur="this.style.backgroundColor = \'red\';"'; ?>/>
                        <strong>
                          Username, that will be used to connect to a MySQL server instance.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">MySQL Password</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("msqlpwd", "formfld pwd"); ?> name="msqlpwd" id="msqlpwd" value="<?=htmlspecialchars($pconfig['msqlpwd']);?>" <?php if ($_POST && $input_error_fields && in_array("msqlpwd", $input_error_fields)) echo 'style="background-color: red;" onFocus="this.style.backgroundColor = \'white\';" onBlur="this.style.backgroundColor = \'red\';"'; ?>/>
                        <strong>
                          Password, that will be used to connect to a MySQL server instance.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">MySQL Database</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("msqldb", "formfld unknown"); ?> name="msqldb" id="msqldb" value="<?=htmlspecialchars($pconfig['msqldb']);?>" <?php if ($_POST && $input_error_fields && in_array("msqldb", $input_error_fields)) echo 'style="background-color: red;" onFocus="this.style.backgroundColor = \'white\';" onBlur="this.style.backgroundColor = \'red\';"'; ?>/>
                        <strong>
                          Database name, that contains DSPAM data.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">MySQL Compress</td>
                      <td width="78%" class="vtable">
                        <input type="checkbox" class="formfld" name="msqlcomp" id="msqlcomp" value="yes" <?php if (isset($pconfig['msqlcomp'])) echo 'checked="checked"'; ?> />
                        <strong>
                          Indicates whether communication data between DSPAM and MySQL should be compressed.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">MySQL Supress Quote</td>
                      <td width="78%" class="vtable">
                        <input type="checkbox" class="formfld" name="msqlsuqt" id="msqlsuqt" value="yes" <?php if (isset($pconfig['msqlsuqt'])) echo 'checked="checked"'; ?> />
                        <strong>
                          Use this if you have the 4.1 quote bug (see doc/mysql.txt).
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">&nbsp;</td>
                      <td width="78%" class="vtable">
                        <p>
                          <span class="vexpl">
                            If you're running DSPAM in client/server (daemon) mode, uncomment the
                            setting below to override the default connection cache size (the number
                            of connections the server pools between all clients). The connection cache
                            represents the maximum number of database connections *available* and should
                            be set based on the maximum number of concurrent connections you're likely
                            to have. Each connection may be used by only one thread at a time, so all
                            other threads _will block_ until another connection becomes available.
                          </span>
                        </p>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">MySQL Connection Cache</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("msqlccache", "formfld unknown"); ?> name="msqlccache" id="msqlccache" value="<?=htmlspecialchars($pconfig['msqlccache']);?>" <?php if ($_POST && $input_error_fields && in_array("msqlccache", $input_error_fields)) echo 'style="background-color: red;" onFocus="this.style.backgroundColor = \'white\';" onBlur="this.style.backgroundColor = \'red\';"'; ?>/>
                        <strong>
                          Conection cache default set to 10.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">&nbsp;</td>
                      <td width="78%" class="vtable">
                        <p>
                          <span class="vexpl">
                            MySQL supports the insertion of the user id into the DSPAM
                            signature. This allows you to create one single spam or fp alias
                            (pointing to some arbitrary user), and the uid in the signature will
                            switch to the correct user. Result: you need only one spam alias
                          </span>
                        </p>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">MySQL UID In Signature</td>
                      <td width="78%" class="vtable">
                        <input type="checkbox" class="formfld" name="msqluid" id="msqluid" value="yes" <?php if (isset($pconfig['msqluid'])) echo 'checked="checked"'; ?> />
                        <strong>
                          Insert user id into the DSPAM signature.
                        </strong>
                      </td>
                    </tr>
                  </tbody>
                  <?php if ($pconfig['sdriver'] == "sqlite" && checkForSQLiteSupport()): ?>
                  <tbody id="DBsqlite" style="display: table-row-group;">
                  <?php else: ?>
                  <tbody id="DBsqlite" style="display: none;">
                  <?php endif; ?>
                    <tr>
                      <td width="22%" valign="top" class="vncell">SQLite Pragma</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("slitepr", "formfld unknown"); ?> name="slitepr" id="slitepr" value="<?=htmlspecialchars($pconfig['slitepr']);?>" />
                        <strong>
                          A particular SQLite pragma command to be used.
                        </strong>
                        <p>
                          <span class="vexpl">
                          See: <a href="http://sqlite.org/pragma.html" target="_blank">http://sqlite.org/pragma.html</a>
                          </span>
                        </p>
                      </td>
                    </tr>
                  </tbody>
                  <?php if ($pconfig['sdriver'] == "bdb"): ?>
                  <tbody id="DBbdb" style="display: table-row-group;">
                  <?php else: ?>
                  <tbody id="DBbdb" style="display: none;">
                  <?php endif; ?>
                    <tr>
                      <td width="22%" valign="top" class="vncell">&nbsp;</td>
                      <td width="78%" class="vtable">
                        <strong>
                          Nothing to be configured here !
                        </strong>
                      </td>
                    </tr>
                  </tbody>
                  <?php if ($pconfig['sdriver'] == "pgsql" && checkForPgSQLSupport()): ?>
                  <tbody id="DBpgsql" style="display: table-row-group;">
                  <?php else: ?>
                  <tbody id="DBpgsql" style="display: none;">
                  <?php endif; ?>
                    <tr>
                      <td width="22%" valign="top" class="vncell">PostgreSQL Server</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("pgserver", "formfld host"); ?> name="pgserver" id="pgserver" value="<?=htmlspecialchars($pconfig['pgserver']);?>" />
                        <strong>
                          A reference to a specific host that is running a PostgreSQL instance.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">PostgreSQL Port</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("pgport", "formfld unknown"); ?> name="pgport" id="pgport" value="<?=htmlspecialchars($pconfig['pgport']);?>" />
                        <strong>
                          A number that represents the port a specific PostgreSQL instance is listening to.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">PostgreSQL User</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("pguser", "formfld user"); ?> name="pguser" id="pguser" value="<?=htmlspecialchars($pconfig['pguser']);?>" />
                        <strong>
                          Username, that will be used to connect to a PostgreSQL server instance.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">PostgreSQL Password</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("pgpwd", "formfld pwd"); ?> name="pgpwd" id="pgpwd" value="<?=htmlspecialchars($pconfig['pgpwd']);?>"/>
                        <strong>
                          Password, that will be used to connect to a PostgreSQL server instance.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">PostgreSQL Database</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("pgdb", "formfld unknown"); ?> name="pgdb" id="pgdb" value="<?=htmlspecialchars($pconfig['pgdb']);?>" />
                        <strong>
                          Database name, that contains DSPAM data.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">&nbsp;</td>
                      <td width="78%" class="vtable">
                        <p>
                          <span class="vexpl">
                            If you're running DSPAM in client/server (daemon) mode, uncomment the
                            setting below to override the default connection cache size (the number
                            of connections the server pools between all clients). The connection cache
                            represents the maximum number of database connections *available* and should
                            be set based on the maximum number of concurrent connections you're likely
                            to have. Each connection may be used by only one thread at a time, so all
                            other threads _will block_ until another connection becomes available.
                          </span>
                        </p>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">PostgreSQL Connection Cache</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("pgccache", "formfld unknown"); ?> name="pgccache" id="pgccache" value="<?=htmlspecialchars($pconfig['pgccache']);?>" />
                        <strong>
                          Conection cache default set to 3.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">&nbsp;</td>
                      <td width="78%" class="vtable">
                        <p>
                          <span class="vexpl">
                            PostgreSQL supports the insertion of the user id into the DSPAM
                            signature. This allows you to create one single spam or fp alias
                            (pointing to some arbitrary user), and the uid in the signature will
                            switch to the correct user. Result: you need only one spam alias
                          </span>
                        </p>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">PostgreSQL UID In Signature</td>
                      <td width="78%" class="vtable">
                        <input type="checkbox" class="formfld" name="pguid" id="pguid" value="yes" <?php if (isset($pconfig['pguid'])) echo 'checked="checked"'; ?> />
                        <strong>
                          Insert user id into the DSPAM signature.
                        </strong>
                      </td>
                    </tr>
                  </tbody>
                  <?php if ($pconfig['sdriver'] == "oracle"): ?>
                  <tbody id="DBoracle" style="display: table-row-group;">
                  <?php else: ?>
                  <tbody id="DBoracle" style="display: none;">
                  <?php endif; ?>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Attention !</td>
                      <td width="78%" class="vtable">
                        <strong style="color: red;">
                          This feature is currently unsupported !
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Oracle Server</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("oraserver", "formfld host"); ?> name="oraserver" id="oraserver" value="<?=htmlspecialchars($pconfig['oraserver']);?>" />
                        <strong>
                          A reference to a specific host that is running an Oracle database instance.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Oracle User</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("orauser", "formfld user"); ?> name="orauser" id="orauser" value="<?=htmlspecialchars($pconfig['orauser']);?>" />
                        <strong>
                          Username, that will be used to connect to a Oracle database server instance.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Oracle Password</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("orapwd", "formfld pwd"); ?> name="orapwd" id="orapwd" value="<?=htmlspecialchars($pconfig['orapwd']);?>" />
                        <strong>
                          Password, that will be used to connect to a Oracle database server instance.
                        </strong>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Oracle Schema</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("orasch", "formfld unknown"); ?> name="orasch" id="orasch" value="<?=htmlspecialchars($pconfig['orasch']);?>" />
                        <strong>
                          Schema name, that contains DSPAM data.
                        </strong>
                      </td>
                    </tr>
                  </tbody>
                  <?php if ($pconfig['sdriver'] == "hash"): ?>
                  <tbody id="DBhash" style="display: table-row-group;">
                  <?php else: ?>
                  <tbody id="DBhash" style="display: none;">
                  <?php endif; ?>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Hash Rec Max</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("hsrmax", "formfld unknown"); ?> name="hsrmax" id="hsrmax" value="<?=htmlspecialchars($pconfig['hsrmax']);?>" />
                        <strong>
                          Default number of records to create in the initial segment when building hash files.
                        </strong>
                        <p>
                          <span class="vexpl">
                          100,000 yields files 1.6MB in size, but can fill up fast, so be sure to increase this
                          (to a million or more) if you're not using autoextend.
                          </span>
                        </p>
                        <p>
                          <span class="vexpl">
                          Primes List:
                          <pre>
53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613,
393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653,
100663319, 201326611, 402653189, 805306457, 1610612741, 3221225473,
4294967291
                          </pre>
                          </span>
                        </p>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Hash Auto Extend</td>
                      <td width="78%" class="vtable">
                        <input type="checkbox" class="formfld" name="hsatex" id="hsatex" value="yes" <?php if (isset($pconfig['hsatex'])) echo 'checked="checked"'; ?> />
                        <strong>
                        Autoextend hash databases when they fill up. This allows them to continue
                        to train by adding extents (extensions) to the file.
                        </strong>
                        <p>
                          <span class="vexpl">
                          Note: There will be a small delay during the growth process,
                          as everything needs to be closed and remapped.
                          </span>
                        </p>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Hash Max Extents</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("hsmxex", "formfld unknown"); ?> name="hsmxex" id="hsmxex" value="<?=htmlspecialchars($pconfig['hsmxex']);?>" />
                        <strong>
                          The maximum number of extents that may be created in a single hash file.
                        </strong>
                        <p>
                          <span class="vexpl">
                          Note: Set this to zero for unlimited.
                          </span>
                        </p>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Hash Extent Size</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("hsexsz", "formfld unknown"); ?> name="hsexsz" id="hsexsz" value="<?=htmlspecialchars($pconfig['hsexsz']);?>" />
                        <strong>
                          The record size for newly created extents.
                        </strong>
                        <p>
                          <span class="vexpl">
                          Note: Creating this too small could result in many extents
                          being created. Creating this too large could result in
                          excessive disk space usage.
                          </span>
                        </p>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Hash Max Seek</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("hsmxse", "formfld unknown"); ?> name="hsmxse" id="hsmxse" value="<?=htmlspecialchars($pconfig['hsmxse']);?>" />
                        <strong>
                          The maximum number of records to seek to insert a new record
                          before failing or adding a new extent.
                        </strong>
                        <p>
                          <span class="vexpl">
                          Note: Setting this too high will exhaustively scan each segment
                          and kill performance. Typically, a low value is acceptable as
                          even older extents will continue to fill over time.
                          </span>
                        </p>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Hash Concurrent User</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("hsccus", "formfld unknown"); ?> name="hsccus" id="hsccus" value="<?=htmlspecialchars($pconfig['hsccus']);?>" />
                        <strong>
                          If you are using a single, stateful hash database in daemon mode,
                          specifying a concurrent user will cause the user to be permanently
                          mapped into memory and shared via rwlocks.
                        </strong>
                        <p>
                          <span class="vexpl">
                          Note: Leave this field blank, if you do not want to use this option.
                          </span>
                        </p>
                      </td>
                    </tr>
                    <tr>
                      <td width="22%" valign="top" class="vncell">Hash Connection Cache</td>
                      <td width="78%" class="vtable">
                        <input type="text" size="30" <?= checkForErrorClass("hscoca", "formfld unknown"); ?> name="hscoca" id="hscoca" value="<?=htmlspecialchars($pconfig['hscoca']);?>" />
                        <strong>
                          If running in daemon mode, this is the max # of concurrent
                          connections that will be supported.
                        </strong>
                        <p>
                          <span class="vexpl">
                          Note: If you are using HashConcurrentUser, this option is ignored,
                          as all connections are read write locked instead of mutex locked.
                          </span>
                        </p>
                      </td>
                    </tr>
                  </tbody>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'db';" />
                    </td>
                  </tr>
                  <tr>
                    <td colspan="2" class="list" height="12">&nbsp;</td>
                  </tr>
                  <tr>
                    <td colspan="2" valign="top" class="listtopic"><a name="del" style="visibility: hidden;">&nbsp;</a>Delivery Settings</td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Trusted Delivery Agent</td>
                    <td width="78%" class="vtable">
                      <select name="dagent" class="formselect">
                        <option value="procmail" <?php if($pconfig['dagent'] == "procmail") echo('selected="selected"');?>>procmail</option>
                        <option value="mail" <?php if($pconfig['dagent'] == "mail") echo('selected="selected"');?>>mail</option>
                        <option value="mail.local" <?php if($pconfig['dagent'] == "mail.local") echo('selected="selected"');?>>mail.local</option>
                        <option value="deliver" <?php if($pconfig['dagent'] == "deliver") echo('selected="selected"');?>>deliver</option>
                        <option value="maildrop" <?php if($pconfig['dagent'] == "maildrop") echo('selected="selected"');?>>maildrop</option>
                        <option value="exim" <?php if($pconfig['dagent'] == "exim") echo('selected="selected"');?>>exim</option>
                      </select>
                      <strong>Specifies the local delivery agent DSPAM should call when delivering mail as a trusted user.</strong>
                      <p>
                        <span class="vexpl">
                          Note: Use %u to specify the user DSPAM is processing mail for. It is generally a good idea to
                          allow the MTA to specify the pass-through arguments at run-time, but they may also be specified
                          here.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">DSPAM Thin Client</td>
                    <td width="78%" class="vtable">
                      <input type="checkbox" name="dsthinc" id="dsthinc" value="yes" <?php if (isset($pconfig['dsthinc'])) echo 'checked="checked"'; ?> />
                      <strong>Use <code>dspamc</code> instead of the <code>dspam</code> binary.</strong>
                      <p>
                        <span class="vexpl">
                          Note: This requires to enable the dspam daemon as well (section: <i>DSPAM Daemon Settings (Server)</i>).
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">TCP/IP Based Delivery</td>
                    <td width="78%" class="vtable">
                      <input type="checkbox" name="tcpipdel" id="tcpipdel" value="yes" <?php if (isset($pconfig['tcpipdel'])) echo 'checked="checked"'; ?> onClick="enable_change(false, 5);" />
                      <strong>Use TCP/IP based delivery.</strong>
                      <p>
                        <span class="vexpl">
                          Note: This option needs to be ticked if you are going to deliver via LMTP or SMTP.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Deliver Host</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("dhost", "formfld host"); ?> name="dhost" id="dhost" value="<?=htmlspecialchars($pconfig['dhost']);?>" <?php if (! isset($pconfig['tcpipdel'])) echo 'disabled="disabled"'; ?> />
                      <strong>Alternatively, you may wish to use SMTP or LMTP delivery to deliver your message to the mail server.</strong>
                      <p>
                        <span class="vexpl">
                          Note: You will need to configure with <code>--enable-daemon</code> to use host delivery,
                          however you do not need to operate in daemon mode. Specify an IP address or UNIX path to a
                          domain socket below as a host.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Deliver Port</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("dport", "formfld unknown"); ?> name="dport" id="dport" value="<?=htmlspecialchars($pconfig['dport']);?>" <?php if (! isset($pconfig['tcpipdel'])) echo 'disabled="disabled"'; ?> />
                      <strong>Port number of a particular host.</strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Deliver Ident</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("dident", "formfld unknown"); ?> name="dident" id="dident" value="<?=htmlspecialchars($pconfig['dident']);?>" <?php if (! isset($pconfig['tcpipdel'])) echo 'disabled="disabled"'; ?> />
                      <strong>A particular identification string</strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">TCP/IP Delivery Protocol</td>
                    <td width="78%" class="vtable">
                      <select name="delproto" class="formselect" <?php if (! isset($pconfig['tcpipdel'])) echo 'disabled="disabled"'; ?>>
                        <option value="smtp" <?php if($pconfig['delproto'] == "smtp") echo('selected="selected"');?>>smtp</option>
                        <option value="lmtp" <?php if($pconfig['delproto'] == "lmtp") echo('selected="selected"');?>>lmtp</option>
                      </select>
                      <strong>A particular protocol typ. Either <acronym title="Simple Mail Transfer Protocol">SMTP</acronym>
                      or <acronym title="Local Mail Transfer Protocol">LMTP</acronym>.</strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">On Fail</td>
                    <td width="78%" class="vtable">
                      <select name="onfail" class="formselect">
                        <option value="error" <?php if($pconfig['onfail'] == "error") echo('selected="selected"');?>>error</option>
                        <option value="unlearn" <?php if($pconfig['onfail'] == "unlearn") echo('selected="selected"');?>>unlearn</option>
                      </select>
                      <strong>What to do if local delivery or quarantine should fail.</strong>
                      <p>
                        <span class="vexpl">
                        Note: If set to &quot;unlearn&quot;, DSPAM will unlearn the message prior to exiting with an un
                        successful return code. The default option, &quot;error&quot; will not unlearn the message but
                        return the appropriate error code. The unlearn option is use-ful on some systems where local
                        delivery failures will cause the message to be requeued for delivery, and could result in the
                        message being processed multiple times. During a very large failure, however, this could cause
                        a significant load increase.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'del';" />
                    </td>
                  </tr>
                  <tr>
                    <td colspan="2" class="list" height="12">&nbsp;</td>
                  </tr>
                  <tr>
                    <td colspan="2" valign="top" class="listtopic"><a name="priv" style="visibility: hidden;">&nbsp;</a>DSPAM Privileges</td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Trusted Users</td>
                    <td width="78%" class="vtable">
                      <strong>Unix users which are allowed to perform certain actions.</strong>
                      <p>
                        <span class="vexpl">
                        Note: Only the users specified below will be allowed to perform
                        administrative functions in DSPAM such as setting the active user and
                        accessing tools. All other users attempting to run DSPAM will be restricted;
                        their uids will be forced to match the active username and they will not be
                        able to specify delivery agent privileges or use tools.
                        </span>
                      </p>
                      <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                        <tr>
                          <td width="55%" class="listhdrr"><?=gettext("UNIX username");?></td>
                          <td width="35%" class="listhdr"><?=gettext("Description");?></td>
                          <td width="10%" class="list"></td>
		                    </tr>
			                  <?php if(is_array($t_users)): ?>
			                  <?php $i = 0; foreach ($t_users as $user): ?>
			                  <?php if($user['name'] <> ""): ?>

                        <tr>
                          <td class="listlr" ondblclick="document.location='dspam-settings-tuser.php?id=<?=$i;?>&sectionid=priv';">
                            <?=htmlspecialchars($user['name']);?>
                          </td>
				                  <td class="listbg" ondblclick="document.location='dspam-settings-tuser.php?id=<?=$i;?>&sectionid=priv';">
				                    <font color="#FFFFFF"><?=htmlspecialchars($user['descr']);?>&nbsp;</font>
				                  </td>
                          <td valign="middle" nowrap class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-tuser.php?id=<?=$i;?>&sectionid=priv"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
                                <td valign="middle"><a href="dspam-settings.php?act=del&what=tuser&id=<?=$i;?>&sectionid=priv" onclick="return confirm('<?=gettext("Do you really want to delete this mapping?");?>')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>

		                    <?php endif; ?>
		                    <?php $i++; endforeach; ?>
		                    <?php endif; ?>
                        <tr>
                          <td class="list" colspan="3"></td>
                          <td class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-tuser.php?sectionid=priv"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>
                      </table>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'priv';" />
                    </td>
                  </tr>
                  <tr>
                    <td colspan="2" class="list" height="12">&nbsp;</td>
                  </tr>
                  <tr>
                    <td valign="top" class="listtopic"><a name="dbg" style="visibility: hidden;">&nbsp;</a>DSPAM Debugging Options</td>
                    <td align="right" valign="top" class="listtopic">
                      <input type="checkbox" name="enabledbg" id="enabledbg" value="yes" <?php if (isset($pconfig['enabledbg'])) echo 'checked="checked"'; ?> onClick="enable_change(false, 0);" />
                      <strong>Enable</strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Debug</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("debug", "formfld unknown"); ?> name="debug" id="debug" value="<?=htmlspecialchars($pconfig['debug']);?>" <?php if (! isset($pconfig['enabledbg'])) echo 'disabled="disabled"'; ?> />
                      <strong>Enables debugging for some or all users.</strong>
                      <p>
                        <span class="vexpl">
                        IMPORTANT: DSPAM must be compiled with debug support in order to use this option.
                        DSPAM should never be running in production with debug active unless you are
                        troubleshooting problems.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Debug Options</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("dopt", "formfld unknown"); ?> name="dopt" id="dopt" value="<?=htmlspecialchars($pconfig['dopt']);?>" <?php if (! isset($pconfig['enabledbg'])) echo 'disabled="disabled"'; ?> />
                      <strong>One or more of: process, classify, spam, fp, inoculation, corpus</strong>
                      <p>
                        <span class="vexpl">
                        <pre>
process     standard message processing
classify    message classification using --classify
spam        error correction of missed spam
fp          error correction of false positives
inoculation message inoculations (source=inoculation)
corpus      corpusfed messages (source=corpus)
                        </pre>
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'dbg';" />
                    </td>
                  </tr>
                  <tr>
                    <td colspan="2" class="list" height="12">&nbsp;</td>
                  </tr>
                  <tr>
                    <td colspan="2" valign="top" class="listtopic"><a name="eng" style="visibility: hidden;">&nbsp;</a>DSPAM Engine Settings</td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Training Mode</td>
                    <td width="78%" class="vtable">
                      <select name="tmode" class="formselect">
                        <option value="toe" <?php if($pconfig['tmode'] == "toe") echo('selected="selected"');?>>toe</option>
                        <option value="tum" <?php if($pconfig['tmode'] == "tum") echo('selected="selected"');?>>tum</option>
                        <option value="teft" <?php if($pconfig['tmode'] == "teft") echo('selected="selected"');?>>teft</option>
                        <option value="notrain" <?php if($pconfig['tmode'] == "notrain") echo('selected="selected"');?>>notrain</option>
                      </select>
                      <strong>
                        The default training mode to use for all operations, when one has not been
                        specified on the commandline or in the user's preferences.
                      </strong>
                      <p>
                        <span class="vexpl">
                        Acceptable values are: toe, tum, teft, notrain
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Test Conditional Training</td>
                    <td width="78%" class="vtable">
                      <input name="testct" type="checkbox" id="testct" value="yes" <?php if (isset($pconfig['testct'])) echo 'checked="checked"'; ?> />
                      <strong>
                        By default, dspam will retrain certain errors
                        until the condition is no longer met.
                      </strong>
                      <p>
                        <span class="vexpl">
                        Note: This usually accelerates learning. Some people argue that this can increase
                        the risk of errors, however.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Features</td>
                    <td width="78%" class="vtable">
                      <strong>
                        Specify features to activate by default; can also be specified
                        on the commandline. See the documentation for a list of available features.
                        If _any_ features are specified on the commandline, these are ignored.
                      </strong>
                      <p>
                        <span class="vexpl">
                        Note: For standard "CRM114" Markovian weighting, use sbph
                        </span>
                      </p>
                      <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                        <tr>
                          <td width="55%" class="listhdrr"><?=gettext("DSPAM Feature");?></td>
                          <td width="35%" class="listhdr"><?=gettext("Description");?></td>
                          <td width="10%" class="list"></td>
		                    </tr>
			                  <?php if(is_array($t_features)): ?>
			                  <?php $i = 0; foreach ($t_features as $feature): ?>
			                  <?php if($feature['name'] <> ""): ?>

                        <tr>
                          <td class="listlr" ondblclick="document.location='dspam-settings-feat.php?id=<?=$i;?>&sectionid=eng';">
                            <?=htmlspecialchars($feature['name']);?>
                          </td>
				                  <td class="listbg" ondblclick="document.location='dspam-settings-feat.php?id=<?=$i;?>&sectionid=eng';">
				                    <font color="#FFFFFF"><?=htmlspecialchars($feature['descr']);?>&nbsp;</font>
				                  </td>
                          <td valign="middle" nowrap class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-feat.php?id=<?=$i;?>&sectionid=eng"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
                                <td valign="middle"><a href="dspam-settings.php?act=del&what=feat&id=<?=$i;?>&sectionid=eng" onclick="return confirm('<?=gettext("Do you really want to delete this mapping?");?>')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>

		                    <?php endif; ?>
		                    <?php $i++; endforeach; ?>
		                    <?php endif; ?>
                        <tr>
                          <td class="list" colspan="3"></td>
                          <td class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-feat.php?sectionid=eng"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>
                      </table>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Algorithms</td>
                    <td width="78%" class="vtable">
                      <strong>
                        Specify the statistical algorithms to use, overriding any
                        defaults configured in the build.
                      </strong>
                      <p>
                        <span class="vexpl">
                        The options are:
                        <pre>
naive       Naive-Bayesian (All Tokens)
graham      Graham-Bayesian ("A Plan for Spam")
burton      Burton-Bayesian (SpamProbe)
robinson    Robinson's Geometric Mean Test (Obsolete)
chi-square  Fisher-Robinson's Chi-Square Algorithm
                        </pre>
                        </span>
                      </p>
                      <p>
                        <span class="vexpl">
                        You may have multiple algorithms active simultaneously, but it is strongly
                        recommended that you group Bayesian algorithms with other Bayesian
                        algorithms, and any use of Chi-Square remain exclusive.
                      </p>
                      <p>
                        <span class="vexpl">
                        NOTE: For standard &quot;CRM114&quot; Markovian weighting, use &lsquo;naive&rsquo;, or consider
                        using &lsquo;burton&rsquo; for slightly better accuracy.
                        </span>
                      </p>
                      <p>
                        <span class="vexpl">
                        Don't mess with this unless you know what you're doing
                        </span>
                      </p>
                      <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                        <tr>
                          <td width="55%" class="listhdrr"><?=gettext("DSPAM Algorithm");?></td>
                          <td width="35%" class="listhdr"><?=gettext("Description");?></td>
                          <td width="10%" class="list"></td>
		                    </tr>
			                  <?php if(is_array($t_algos)): ?>
			                  <?php $i = 0; foreach ($t_algos as $algo): ?>
			                  <?php if($algo['name'] <> ""): ?>

                        <tr>
                          <td class="listlr" ondblclick="document.location='dspam-settings-algo.php?id=<?=$i;?>&sectionid=eng';">
                            <?=htmlspecialchars($algo['name']);?>
                          </td>
				                  <td class="listbg" ondblclick="document.location='dspam-settings-algo.php?id=<?=$i;?>&sectionid=eng';">
				                    <font color="#FFFFFF"><?=htmlspecialchars($algo['descr']);?>&nbsp;</font>
				                  </td>
                          <td valign="middle" nowrap class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-algo.php?id=<?=$i;?>&sectionid=eng"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
                                <td valign="middle"><a href="dspam-settings.php?act=del&what=algo&id=<?=$i;?>&sectionid=eng" onclick="return confirm('<?=gettext("Do you really want to delete this mapping?");?>')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>

		                    <?php endif; ?>
		                    <?php $i++; endforeach; ?>
		                    <?php endif; ?>
                        <tr>
                          <td class="list" colspan="3"></td>
                          <td class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-algo.php?sectionid=eng"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>
                      </table>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">PValue</td>
                    <td width="78%" class="vtable">
                      <select name="pvalue" class="formselect">
                        <option value="graham" <?php if($pconfig['pvalue'] == "toe") echo('selected="selected"');?>>graham</option>
                        <option value="robinson" <?php if($pconfig['pvalue'] == "toe") echo('selected="selected"');?>>robinson</option>
                        <option value="markov" <?php if($pconfig['pvalue'] == "toe") echo('selected="selected"');?>>markov</option>
                      </select>
                      <strong>
                      Specify the technique used for calculating PValues, overriding any defaults
                      configured in the build.
                      </strong>
                      <p>
                        <span class="vexpl">
                        These options are:
                        <pre>
graham      Graham's Technique (&quot;A Plan for Spam&quot;)
robinson    Robinson's Technique
markov      Markovian Weighted Technique
                        </pre>
                        </span>
                      </p>
                      <p>
                        <span class="vexpl">
                        Unlike algorithms, you may only have one of these defined. Use of the
                        chi-square algorithm automatically changes this to robinson.
                        </span>
                      </p>
                      <p>
                        <span class="vexpl">
                        Don't mess with this unless you know what you're doing.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Improbability Drive</td>
                    <td width="78%" class="vtable">
                      <input name="ipdrive" type="checkbox" id="ipdrive" value="yes" <?php if (isset($pconfig['ipdrive'])) echo 'checked="checked"'; ?> />
                      <strong>
                      Calculate odds-ratios for ham/spam, and add to X-DSPAM-Improbability headers
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Preferences</td>
                    <td width="78%" class="vtable">
                      <strong>
                      Specify any preferences to set by default, unless otherwise
                      overridden by the user (see next section) or a default.prefs file.
                      </strong>
                      <p>
                        <span class="vexpl">
                        Note: If user or default.prefs are found, the user's
                        preferences will override any defaults.
                        </span>
                      </p>
                      <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                        <tr>
                          <td width="55%" class="listhdrr"><?=gettext("DSPAM Preference Value");?></td>
                          <td width="35%" class="listhdr"><?=gettext("Description");?></td>
                          <td width="10%" class="list"></td>
		                    </tr>
			                  <?php if(is_array($t_prefs)): ?>
			                  <?php $i = 0; foreach ($t_prefs as $pref): ?>
			                  <?php if($pref['value'] <> ""): ?>

                        <tr>
                          <td class="listlr" ondblclick="document.location='dspam-settings-prefs.php?id=<?=$i;?>&sectionid=eng';">
                            <?=htmlspecialchars($pref['value']);?>
                          </td>
				                  <td class="listbg" ondblclick="document.location='dspam-settings-prefs.php?id=<?=$i;?>&sectionid=eng';">
				                    <font color="#FFFFFF"><?=htmlspecialchars($pref['descr']);?>&nbsp;</font>
				                  </td>
                          <td valign="middle" nowrap class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-prefs.php?id=<?=$i;?>&sectionid=eng"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
                                <td valign="middle"><a href="dspam-settings.php?act=del&what=pref&id=<?=$i;?>&sectionid=eng" onclick="return confirm('<?=gettext("Do you really want to delete this mapping?");?>')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>

		                    <?php endif; ?>
		                    <?php $i++; endforeach; ?>
		                    <?php endif; ?>
                        <tr>
                          <td class="list" colspan="3"></td>
                          <td class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-prefs.php?sectionid=eng"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>
                      </table>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Overrides</td>
                    <td width="78%" class="vtable">
                      <strong>
                      Specifies the user preferences which may override
                      configuration and commandline defaults.
                      </strong>
                      <p>
                        <span class="vexpl">
                        Note: Any other preferences supplied by an untrusted user will be ignored.
                        </span>
                      </p>
                      <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                        <tr>
                          <td width="55%" class="listhdrr"><?=gettext("DSPAM Override Value");?></td>
                          <td width="35%" class="listhdr"><?=gettext("Description");?></td>
                          <td width="10%" class="list"></td>
		                    </tr>
			                  <?php if(is_array($t_overr)): ?>
			                  <?php $i = 0; foreach ($t_overr as $over): ?>
			                  <?php if($over['value'] <> ""): ?>

                        <tr>
                          <td class="listlr" ondblclick="document.location='dspam-settings-overr.php?id=<?=$i;?>&sectionid=eng';">
                            <?=htmlspecialchars($over['value']);?>
                          </td>
				                  <td class="listbg" ondblclick="document.location='dspam-settings-overr.php?id=<?=$i;?>&sectionid=eng';">
				                    <font color="#FFFFFF"><?=htmlspecialchars($over['descr']);?>&nbsp;</font>
				                  </td>
                          <td valign="middle" nowrap class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-overr.php?id=<?=$i;?>&sectionid=eng"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
                                <td valign="middle"><a href="dspam-settings.php?act=del&what=overr&id=<?=$i;?>&sectionid=eng" onclick="return confirm('<?=gettext("Do you really want to delete this mapping?");?>')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>

		                    <?php endif; ?>
		                    <?php $i++; endforeach; ?>
		                    <?php endif; ?>
                        <tr>
                          <td class="list" colspan="3"></td>
                          <td class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-overr.php?sectionid=eng"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>
                      </table>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'eng';" />
                    </td>
                  </tr>
                  <?php if (checkForLDAPSupport()): ?>
                  <tr>
                    <td colspan="2" class="list" height="12">&nbsp;</td>
                  </tr>
                  <tr>
                    <td valign="top" class="listtopic"><a name="ldap" style="visibility: hidden;">&nbsp;</a>LDAP Settings</td>
                    <td align="right" valign="top" class="listtopic">
                      <input name="enableldap" type="checkbox" id="enableldap" value="yes" <?php if (isset($pconfig['enableldap'])) echo 'checked="checked"'; ?> onClick="enable_change(false, 1);" />
                      <strong>Enable</strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">LDAP Mode</td>
                    <td width="78%" class="vtable">
                      <select name="ldapmode" class="formselect" <?php if (! isset($pconfig['enableldap'])) echo 'disabled="disabled"'; ?>>
                        <option value="verify" selected="selected">verify</option>
                      </select>
                      <strong>
                      Perform various LDAP functions depending on LDAPMode variable.
                      </strong>
                      <p>
                        <span class="vexpl">
                        Note: Presently, the only mode supported is 'verify', which will verify the
                        existence of an unknown user in LDAP prior to creating them as a new user in
                        the system. This is useful on some systems acting as gateway machines.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">LDAP Host</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("hldaphost", "formfld host"); ?> name="ldaphost" id="ldaphost" value="<?=htmlspecialchars($pconfig['ldaphost']);?>" <?php if (! isset($pconfig['enableldap'])) echo 'disabled="disabled"'; ?> />
                      <strong>
                      Hostname of the LDAP directory server.
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">LDAP Filter</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("ldapfilter", "formfld unknown"); ?> name="ldapfilter" id="ldapfilter" value="<?=htmlspecialchars($pconfig['ldapfilter']);?>" <?php if (! isset($pconfig['enableldap'])) echo 'disabled="disabled"'; ?> />
                      <strong>
                      A specific query filter, that should be used while querying the LDAP server.
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">LDAP Base</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("ldapbase", "formfld unknown"); ?> name="ldapbase" id="ldapbase" value="<?=htmlspecialchars($pconfig['ldapbase']);?>" <?php if (! isset($pconfig['enableldap'])) echo 'disabled="disabled"'; ?> />
                      <strong>
                      A particular distinguish name from where to start LDAP queries.
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'ldap';" />
                    </td>
                  </tr>
                  <?php endif; ?>
                  <tr>
                    <td colspan="2" class="list" height="12">&nbsp;</td>
                  </tr>
                  <tr>
                    <td colspan="2" valign="top" class="listtopic"><a name="misc" style="visibility: hidden;">&nbsp;</a>Miscellaneous Settings</td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Failover Attempts</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30"<?= checkForErrorClass("foatt", "formfld unknown"); ?> name="foatt" id="foatt" value="<?=htmlspecialchars($pconfig['foatt']);?>" />
                      <strong>
                      A particular number of attempts.
                      </strong>
                      <p>
                        <span class="vexpl">
                        If the storage fails, the agent will follow each profile's failover up to
                        a maximum number of failover attempts. This should be set to a maximum of
                        the number of profiles you have, otherwise the agent could loop and try
                        the same profile multiple times (unless this is your desired behavior).
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Ignore Headers</td>
                    <td width="78%" class="vtable">
                      <p>
                        <span class="vexpl">
                        If DSPAM is behind other tools which may add a header to
                        incoming emails, it may be beneficial to ignore these headers - especially
                        if they are coming from another spam filter. If you are _not_ using one of
                        these tools, however, leaving the appropriate headers commented out will
                        allow DSPAM to use them as telltale signs of forged email.
                        </span>
                      </p>
                      <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                        <tr>
                          <td width="55%" class="listhdrr"><?=gettext("Header Name");?></td>
                          <td width="35%" class="listhdr"><?=gettext("Description");?></td>
                          <td width="10%" class="list"></td>
		                    </tr>
			                  <?php if(is_array($t_headers)): ?>
			                  <?php $i = 0; foreach ($t_headers as $header): ?>
			                  <?php if($header['name'] <> ""): ?>

                        <tr>
                          <td class="listlr" ondblclick="document.location='dspam-settings-header.php?id=<?=$i;?>&sectionid=misc';">
                            <?=htmlspecialchars($header['name']);?>
                          </td>
				                  <td class="listbg" ondblclick="document.location='dspam-settings-header.php?id=<?=$i;?>&sectionid=misc';">
				                    <font color="#FFFFFF"><?=htmlspecialchars($header['descr']);?>&nbsp;</font>
				                  </td>
                          <td valign="middle" nowrap class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-header.php?id=<?=$i;?>&sectionid=misc"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
                                <td valign="middle"><a href="dspam-settings.php?act=del&what=header&id=<?=$i;?>&sectionid=misc" onclick="return confirm('<?=gettext("Do you really want to delete this mapping?");?>')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>

		                    <?php endif; ?>
		                    <?php $i++; endforeach; ?>
		                    <?php endif; ?>
                        <tr>
                          <td class="list" colspan="3"></td>
                          <td class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-header.php?sectionid=misc"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>
                      </table>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">SBL Lookup</td>
                    <td width="78%" class="vtable">
                      <p>
                        <input type="checkbox" name="enablesbl" id="enablesbl" value="yes" <?php if (isset($pconfig['enablesbl'])) echo 'checked="checked"'; ?> onClick="enable_change(false, 2);" />
                        <strong>
                        Enable checks against a particular <acronym title="Streamlined Blackhole List">SBL</acronym> host.
                        </strong>
                      </p>
                      <p>
                        <input type="text" size="30" c<?= checkForErrorClass("sblhost", "formfld host"); ?> name="sblhost" id="sblhost" value="<?=htmlspecialchars($pconfig['sblhost']);?>" <?php if (! isset($pconfig['enablesbl'])) echo 'disabled="disabled"'; ?> />
                        <strong>
                        A particular SBL hostname.
                        </strong>
                      </p>
                      <p>
                        <span class="vexpl">
                        Perform lookups on streamlined blackhole list servers (see
                        <a href="http://www.nuclearelephant.com/projects/sbl/" target="_blank">http://www.nuclearelephant.com/projects/sbl/</a>).
                        The streamlined blacklist
                        server is machine-automated, unsupervised blacklisting system designed to
                        provide real-time and highly accurate blacklisting based on network spread.
                        When performing a lookup, DSPAM will automatically learn the inbound message
                        as spam if the source IP is listed. Until an official public RABL server is
                        available, this feature is only useful if you are running your own
                        streamlined blackhole list server for internal reporting among multiple mail
                        servers. Provide the name of the lookup zone below to use.
                        </span>
                      </p>
                      <p>
                        <span class="vexpl">
                        This function performs standard reverse-octet.domain lookups, and while it
                        will function with many RBLs, it's strongly discouraged to use those
                        maintained by humans as they're often inaccurate and could hurt filter
                        learning and accuracy.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">RBL Inoculate</td>
                    <td width="78%" class="vtable">
                      <input name="enablerbl" type="checkbox" id="enablerbl" value="yes" <?php if (isset($pconfig['enablerbl'])) echo 'checked="checked"'; ?> />
                      <strong>
                      Enable <acronym title="Realtime Blackhole List">RBL</acronym> inoculation support.
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Enable Notification</td>
                    <td width="78%" class="vtable">
                      <input name="enablenoti" type="checkbox" id="enablenoti" value="yes" <?php if (isset($pconfig['enablenoti'])) echo 'checked="checked"'; ?> onClick="enable_change(false, 2);" />
                      <strong>
                      Enable the sending of notification emails to users (first message, quarantine full, etc.)
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">DSPAM Support Contact</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("dspamcontact", "formfld mail"); ?> name="dspamcontact" id="dspamcontact" value="<?=htmlspecialchars($pconfig['dspamcontact']);?>" <?php if (empty($pconfig['enablenoti'])) echo 'disabled="disabled"'; ?> />
                      <strong>
                      The username of the person who provides DSPAM support for this DSPAM installation
                      </strong>
                      &nbsp;(This is the left most part of an email address before the @ sign).
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Email Domain Name</td>
                    <td width="78%" class="vtable">
                      <input name="whichdomain" type="checkbox" id="whichdomain" value="yes" <?php if (empty($pconfig['dspamdomain'])) echo 'checked="checked"'; if (empty($pconfig['enablenoti'])) echo 'disabled="disabled"'; ?> onClick="toggleDSPAMDomain(false, this);" />
                      <strong>
                      Use global domain settings while trying to send an email message.
                      </strong>
                    </td>
                  </tr>
                  <?php if (isset($pconfig['dspamdomain'])): ?>
                  <tbody id="emailnotitb" style="display: table-row-group;">
                  <?php else: ?>
                  <tbody id="emailnotitb" style="display: none;">
                  <?php endif; ?>
                  <tr>
                    <td width="22%" valign="top" class="vncell">DSPAM Domain Name</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("dspamdomain", "formfld url"); ?> name="dspamdomain" id="dspamdomain" value="<?=htmlspecialchars($pconfig['dspamdomain']);?>" />
                      <strong>
                      Use this domain name while trying to send an email message.
                      </strong>
                    </td>
                  </tr>
                  </tbody>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'misc';" />
                    </td>
                  </tr>
                  <tr>
                    <td colspan="2" class="list" height="12">&nbsp;</td>
                  </tr>
                  <tr>
                    <td colspan="2" valign="top" class="listtopic"><a name="main" style="visibility: hidden;">&nbsp;</a>Maintainance Settings</td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">&nbsp;</td>
                    <td width="78%" class="vtable">
                      <p>
                        <span class="vexpl">
                        Set dspam_clean purge default options, if not
                        otherwise specified on the commandline. You may set some of
                        the below values to <code>off</code>, for instance if you are
                        using a SQL-based database backend for DSPAM. Please consult your
                        DSPAM manual for any details.
                        </span>
                      </p>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Purge Signatures</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("psig", "formfld unknown"); ?> name="psig" id="psig" value="<?=htmlspecialchars($pconfig['psig']);?>" />
                      <strong>
                      Purge stale signatures
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Purge Neutral</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("pneut", "formfld unknown"); ?> name="pneut" id="pneut" value="<?=htmlspecialchars($pconfig['pneut']);?>" />
                      <strong>
                      Purge tokens with neutralish probabilities
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Purge Unused</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("punu", "formfld unknown"); ?> name="punu" id="punu" value="<?=htmlspecialchars($pconfig['punu']);?>" />
                      <strong>
                      Purge unused tokens
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Purge Hapaxes</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("phapa", "formfld unknown"); ?> name="phapa" id="phapa" value="<?=htmlspecialchars($pconfig['phapa']);?>" />
                      <strong>
                      Purge tokens with less than 5 hits (hapaxes)
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Purge Hits 1S</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("pones", "formfld unknown"); ?> name="pones" id="pones" value="<?=htmlspecialchars($pconfig['pones']);?>" />
                      <strong>
                      Purge tokens with only 1 spam hit
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Purge Hits 1I</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("ponei", "formfld unknown"); ?> name="ponei" id="ponei" value="<?=htmlspecialchars($pconfig['ponei']);?>" />
                      <strong>
                      Purge tokens with only 1 innocent hit
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'main';" />
                    </td>
                  </tr>
                  <tr>
                    <td colspan="2" class="list" height="12">&nbsp;</td>
                  </tr>
                  <tr>
                    <td colspan="2" valign="top" class="listtopic"><a name="sys" style="visibility: hidden;">&nbsp;</a>System Settings</td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Local MX</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("locmx", "formfld host"); ?> name="locmx" id="locmx" value="<?=htmlspecialchars($pconfig['locmx']);?>" />
                      <strong>
                      Local Mail Exchangers: Used for source address tracking, tells DSPAM which
                      mail exchangers are local and therefore should be ignored in the Received:
                      header when tracking the source of an email. Note: you should use the address
                      of the host as appears between brackets [ ] in the Received header.
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">&nbsp;</td>
                    <td width="78%" class="vtable">
                      <span class="vexpl">
                      Disabling logging for users will make usage graphs unavailable to
                      them. Disabling system logging will make admin graphs unavailable.
                      </span>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Enable System Log</td>
                    <td width="78%" class="vtable">
                      <input name="enablesysl" type="checkbox" id="enablesysl" value="yes" <?php if (isset($pconfig['enablesysl'])) echo 'checked="checked"'; ?> />
                      <strong>
                      Enable system logging.
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Enable User Log</td>
                    <td width="78%" class="vtable">
                      <input name="enableusel" type="checkbox" id="enableusel" value="yes" <?php if (isset($pconfig['enableusel'])) echo 'checked="checked"'; ?> />
                      <strong>
                      Enable user logging.
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Opt Settings</td>
                    <td width="78%" class="vtable">
                      <select name="optinout" class="formselect">
                        <option value="out" <?php if($pconfig['optinout'] == "out") echo('selected="selected"');?>>out</option>
                        <option value="in" <?php if($pconfig['optinout'] == "in") echo('selected="selected"');?>>in</option>
                      </select>
                      <p>
                        <span class="vexpl">
                          Opt: in or out; determines DSPAM's default filtering behavior. If this value
                          is set to in, users must opt-in to filtering by dropping a .dspam file in
                          <code>/var/dspam/opt-in/user.dspam</code> (or if you have homedirs configured, a .dspam
                          folder in their home directory).  The default is opt-out, which means all
                          users will be filtered unless a <code>.nodspam</code> file is dropped in
                          <code>/var/dspam/opt-out/user.nodspam</code>
                        <span class="vexpl">
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">&nbsp;</td>
                    <td width="78%" class="vtable">
                      <span class="vexpl">
                        In lieu of setting up individual aliases for each user,
                        DSPAM can be configured to automatically parse the To: address for spam and
                        false positive forwards. From there, it can be configured to either set the
                        DSPAM user based on the username specified in the header and/or change the
                        training class and source accordingly. The options below can be used to
                        customize most common types of header parsing behavior to avoid the need for
                        multiple aliases, or if using LMTP, aliases entirely..
                      </span>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Parse To Headers</td>
                    <td width="78%" class="vtable">
                      <input name="enableptoh" type="checkbox" id="enableptoh" value="yes" <?php if (isset($pconfig['enableptoh'])) echo 'checked="checked"'; ?> />
                      <strong>
                      Parse the <i>To:</i> headers of an incoming message.
                      </strong>
                      <p>
                        <span class="vexpl">
                        This must be set to &lsquo;on&rsquo; to use either of the following features.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Change Mode On Parse</td>
                    <td width="78%" class="vtable">
                      <input name="enablecmop" type="checkbox" id="enablecmop" value="yes" <?php if (isset($pconfig['enablecmop'])) echo 'checked="checked"'; ?> />
                      <strong>
                      Automatically change the class (to spam or innocent).
                      </strong>
                      <p>
                        <span class="vexpl">
                        This depends on whether spam- or notspam- was specified, and change
                        the source to &lsquo;error&rsquo;. This is convenient if you're not
                        using aliases at all, but are delivering via LMTP.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Change User On Parse</td>
                    <td width="78%" class="vtable">
                      <input name="enablecuop" type="checkbox" id="enablecuop" value="yes" <?php if (isset($pconfig['enablecuop'])) echo 'checked="checked"'; ?> />
                      <strong>
                      Automatically change the username to match that specified in the <i>To:</i> header.
                      </strong>
                      <p>
                        <span class="vexpl">
                        For example, <code>spam-bob@domain.tld</code> will set the username
                        to bob, ignoring any --user passed in. This may not always be desirable if
                        you are using virtual email addresses as usernames. Options:
                        on or user take the portion before the @ sign only
                        full take everything after the initial {spam,notspam}-.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Broken MTA Settings</td>
                    <td width="78%" class="vtable">
                      <input name="enablebmta" type="checkbox" id="enablebmta" value="yes" <?php if (isset($pconfig['enablebmta'])) echo 'checked="checked"'; ?> />
                      <strong>
                      Enable broken MTA settings.
                      </strong>
                      <p>
                        <span class="vexpl">
                        Broken MTA Options: Some MTAs don't support the proper functionality
                        necessary. In these cases you can activate certain features in DSPAM to
                        compensate. &lsquo;returnCodes&rsquo; causes DSPAM to return an exit code of 99 if
                        the message is spam, 0 if not, or a negative code if an error has occured.
                        Specifying &lsquo;case&rsquo; causes DSPAM to force the input usernames to lowercase.
                        Spceifying &lsquo;lineStripping&rsquo; causes DSPAM to strip &circ;M's from messages passed
                        </span>
                      </p>
                      <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                        <tr>
                          <td width="55%" class="listhdrr"><?=gettext("Broken MTA Option");?></td>
                          <td width="35%" class="listhdr"><?=gettext("Description");?></td>
                          <td width="10%" class="list"></td>
		                    </tr>
			                  <?php if(is_array($t_bmtas)): ?>
			                  <?php $i = 0; foreach ($t_bmtas as $bmta): ?>
			                  <?php if($bmta['name'] <> ""): ?>

                        <tr>
                          <td class="listlr" ondblclick="document.location='dspam-settings-bmta.php?id=<?=$i;?>&sectionid=sys';">
                            <?=htmlspecialchars($bmta['name']);?>
                          </td>
				                  <td class="listbg" ondblclick="document.location='dspam-settings-bmta.php?id=<?=$i;?>&sectionid=sys';">
				                    <font color="#FFFFFF"><?=htmlspecialchars($bmta['descr']);?>&nbsp;</font>
				                  </td>
                          <td valign="middle" nowrap class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-bmta.php?id=<?=$i;?>&sectionid=sys"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
                                <td valign="middle"><a href="dspam-settings.php?act=del&what=bmta&id=<?=$i;?>&sectionid=sys" onclick="return confirm('<?=gettext("Do you really want to delete this mapping?");?>')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>

		                    <?php endif; ?>
		                    <?php $i++; endforeach; ?>
		                    <?php endif; ?>
                        <tr>
                          <td class="list" colspan="3"></td>
                          <td class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-bmta.php?sectionid=sys"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>
                      </table>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Max Message Size</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("maxmsgs", "formfld unknown"); ?> name="maxmsgs" id="maxmsgs" value="<?=htmlspecialchars($pconfig['maxmsgs']);?>" />
                      <strong>
                      You may specify a maximum message size for DSPAM to process.
                      </strong>
                      <p>
                        <span class="vexpl">
                        If the message is larger than the maximum size, it will be delivered
                        without processing. Value is in bytes.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Processor Bias</td>
                    <td width="78%" class="vtable">
                      <input type="checkbox" name="procbias" id="procbias" value="yes" <?php if (isset($pconfig['procbias'])) echo 'checked="checked"'; ?> />
                      <strong>
                      Bias causes the filter to lean more toward &lsquo;innocent&rsquo;, and
                      usually greatly reduces false positives. It is the default behavior of
                      most Bayesian filters (including dspam).
                      </strong>
                      <p>
                        <span class="vexpl">
                        Note: You probably DONT want this if you're using Markovian Weighting,
                        unless you are paranoid about false positives.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'sys';" />
                    </td>
                  </tr>
                  <tr>
                    <td colspan="2" class="list" height="12">&nbsp;</td>
                  </tr>
                  <?php if (checkForClamAVSupport()): ?>
                  <tr>
                    <td valign="top" class="listtopic"><a name="clam" style="visibility: hidden;">&nbsp;</a>ClamAV Engine Settings</td>
                    <td align="right" valign="top" class="listtopic">
                      <input name="enableclam" type="checkbox" id="enableclam" value="yes" <?php if (isset($pconfig['enableclam'])) echo 'checked="checked"'; ?> onClick="enable_change(false, 3);" />
                      <strong>Enable</strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">&nbsp;</td>
                    <td width="78%" class="vtable">
                      <p>
                      <span class="vexpl">
                      If you are running clamd, DSPAM can perform stream-based
                      virus checking using TCP. Uncomment the values below to enable virus
                      checking.
                      </span>
                      </p>
                      <p>
                      <span class="vexpl">
                        ClamAVResponse:
                        <dl>
                          <dt>reject</dt>
                          <dd>(reject or drop the message with a permanent failure)</dd>
                          <dt>accept</dt>
                          <dd>(accept the message and quietly drop the message)</dd>
                          <dt>spam</dt>
                          <dd>(treat as spam and quarantine/tag/whatever)</dd>
                        </dl>
                      </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">ClamAV Port</td>
                    <td width="78%" class="vtable">
                      <p>
                        <input type="text" size="30" <?= checkForErrorClass("clamport", "formfld unknown"); ?> name="clamport" id="clamport" value="<?=htmlspecialchars($pconfig['clamport']);?>" <?php if (! isset($pconfig['enableclam'])) echo 'disabled="disabled"'; ?> />
                        <strong>
                        A number that specifies the port the ClamAV daemon is listening to.
                        </strong>
                      </p>
                      <p>
                        <span class="vexpl">
                        If the message is larger than the maximum size, it will be delivered
                        without processing. Value is in bytes.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">ClamAV Host</td>
                    <td width="78%" class="vtable">
                      <input type="text" size="30" <?= checkForErrorClass("clamhost", "formfld host"); ?> name="clamhost" id="clamhost" value="<?=htmlspecialchars($pconfig['clamhost']);?>" <?php if (! isset($pconfig['enableclam'])) echo 'disabled="disabled"'; ?> />
                      <strong>
                      An IP address that points to the host the ClamAV daemon is running on.
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">ClamAV Response</td>
                    <td width="78%" class="vtable">
                      <select name="clamresp" class="formselect" <?php if (! isset($pconfig['enableclam'])) echo 'disabled="disabled"'; ?>>
                        <option value="reject" <?php if($pconfig['clamresp'] == "reject") echo('selected="selected"');?>>reject</option>
                        <option value="accept" <?php if($pconfig['clamresp'] == "accept") echo('selected="selected"');?>>accept</option>
                        <option value="spam" <?php if($pconfig['clamresp'] == "spam") echo('selected="selected"');?>>spam</option>
                      </select>
                      <strong>
                      The action that should take place, if ClamAV reports a positive.
                      </strong>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'clam';" />
                    </td>
                  </tr>
                  <tr>
                    <td colspan="2" class="list" height="12">&nbsp;</td>
                  </tr>
                  <?php endif; ?>
                  <tr>
                    <td colspan="2" valign="top" class="listtopic"><a name="srv" style="visibility: hidden;">&nbsp;</a>DSPAM Daemon Settings (Server)</td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">&nbsp;</td>
                    <td width="78%" class="vtable">
                      <span class="vexpl">
                        If you are running DSPAM as a daemonized server using
                        <code>--daemon</code>, the following parameters will override the default. Use the
                        ServerPass option to set up accounts for each client machine. The DSPAM
                        server will process and deliver the message based on the parameters
                        specified. If you want the client machine to perform delivery, use
                        the <code>--stdout</code> option in conjunction with a local setup.
                      </span>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Server Port</td>
                    <td width="78%" class="vtable">
                      <p>
                        <input type="text" size="30" <?= checkForErrorClass("dsport", "formfld unknown"); ?> name="dsport" id="dsport" value="<?=htmlspecialchars($pconfig['dsport']);?>" />
                        <strong>
                        A number that specifies the port the DSPAM daemon is listening to.
                        </strong>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Server Queue Size</td>
                    <td width="78%" class="vtable">
                      <p>
                        <input type="text" size="30" <?= checkForErrorClass("dsqsize", "formfld unknown"); ?> name="dsqsize" id="dsqsize" value="<?=htmlspecialchars($pconfig['dsqsize']);?>" />
                        <strong>
                        A number that specifies the server's queue size.
                        </strong>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Server PID</td>
                    <td width="78%" class="vtable">
                      <p>
                        <input type="text" size="30" <?= checkForErrorClass("dspid", "formfld file"); ?> name="dspid" id="dspid" value="<?=htmlspecialchars($pconfig['dspid']);?>" />
                        <strong>
                        Keep this is sync with <code>/usr/local/etc/rc.d/dspam.rc</code> script.
                        </strong>
                      </p>
                      <p>
                        <span class="vexpl">
                          Note: Don't change this value unless you know what you are doing.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Server Mode</td>
                    <td width="78%" class="vtable">
                      <p>
                        <select name="dssmode" class="formselect">
                          <option value="dspam" <?php if($pconfig['dssmode'] == "dspam") echo('selected="selected"');?>>dspam</option>
                          <option value="standard" <?php if($pconfig['dssmode'] == "standard") echo('selected="selected"');?>>standard</option>
                          <option value="auto" <?php if($pconfig['dssmode'] == "auto") echo('selected="selected"');?>>auto</option>
                        </select>
                        <strong>
                        Specifies the type of LMTP server to start.
                        </strong>
                        <p>
                          <span class="vexpl">
                            This can be one of:
                            <dl>
                              <dt>dspam</dt>
                              <dd>DSPAM-proprietary DLMTP server, for communicating with dspamc</dd>
                              <dt>standard</dt>
                              <dd>Standard LMTP server, for communicating with Postfix or other MTA</dd>
                              <dt>auto</dt>
                              <dd>Speak both DLMTP and LMTP; auto-detect by ServerPass.IDENT</dd>
                            </dl>
                          </span>
                        </p>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" rowspan="2" class="vncell">&nbsp;</td>
                    <td width="78%" class="vtable">
                      <p>
                        <span class="vexpl">
                          If supporting DLMTP (dspam) mode, dspam clients will require authentication
                          as they will be passing in parameters. The idents below will be used to
                          determine which clients will be speaking DLMTP, so if you will be using
                          both LMTP and DLMTP from the same host, be sure to use something other
                          than the server's hostname below (which will be sent by the MTA during a
                          standard LMTP LHLO).
                        </span>
                      </p>
                      <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                        <tr>
                          <td width="55%" class="listhdrr"><?=gettext("DLMTP Password Value");?></td>
                          <td width="35%" class="listhdr"><?=gettext("Description");?></td>
                          <td width="10%" class="list"></td>
		                    </tr>
			                  <?php if(is_array($t_spwds)): ?>
			                  <?php $i = 0; foreach ($t_spwds as $spwd): ?>
			                  <?php if($spwd['value'] <> ""): ?>

                        <tr>
                          <td class="listlr" ondblclick="document.location='dspam-settings-spwd.php?id=<?=$i;?>&sectionid=srv';">
                            <?=htmlspecialchars($spwd['value']);?>
                          </td>
				                  <td class="listbg" ondblclick="document.location='dspam-settings-spwd.php?id=<?=$i;?>&sectionid=srv';">
				                    <font color="#FFFFFF"><?=htmlspecialchars($spwd['descr']);?>&nbsp;</font>
				                  </td>
                          <td valign="middle" nowrap class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-spwd.php?id=<?=$i;?>&sectionid=srv"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
                                <td valign="middle"><a href="dspam-settings.php?act=del&what=spwd&id=<?=$i;?>&sectionid=srv" onclick="return confirm('<?=gettext("Do you really want to delete this mapping?");?>')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>

		                    <?php endif; ?>
		                    <?php $i++; endforeach; ?>
		                    <?php endif; ?>
                        <tr>
                          <td class="list" colspan="3"></td>
                          <td class="list">
                            <table border="0" cellspacing="0" cellpadding="1">
                              <tr>
                                <td valign="middle"><a href="dspam-settings-spwd.php?sectionid=srv"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                            </table>
                          </td>
                        </tr>
                      </table>
                    </td>
                  </tr>
                  <tr>
                    <td width="78%" class="vtable">
                      <p>
                        <span class="vexpl">
                          If supporting standard LMTP mode, server parameters will need to be specified
                          here, as they will not be passed in by the mail server. The ServerIdent
                          specifies the 250 response code ident sent back to connecting clients and
                          should be set to the hostname of your server, or an alias.
                        </span>
                      </p>
                      <p>
                        <span class="vexpl">
                          Note: If you specify <code>--user</code> in ServerParameters, the RCPT TO will be used
                          only for delivery, and not set as the active user for processing.
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Server Parameters</td>
                    <td width="78%" class="vtable">
                      <p>
                        <input type="text" size="30" class="formfld unknown" name="serverparam" id="serverparam" value="<?=htmlspecialchars($pconfig['serverparam']);?>" />
                        <strong>
                        Parameters which will be passed to the LMTP server.
                        </strong>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Server Ident</td>
                    <td width="78%" class="vtable">
                      <p>
                        <input type="text" size="30" <?= checkForErrorClass("serverid", "formfld host"); ?> name="serverid" id="serverid" value="<?=htmlspecialchars($pconfig['serverid']);?>" />
                        <strong>
                        An identification string which will be used to be passed to the LMTP server.
                        </strong>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Server Domain Socket Path</td>
                    <td width="78%" class="vtable">
                      <p>
                        <input type="text" size="30" <?= checkForErrorClass("serversock", "formfld file"); ?> name="serversock" id="serversock" value="<?=htmlspecialchars($pconfig['serversock']);?>" />
                        <strong>
                        A local Unix domain socket.
                        </strong>
                      </p>
                      <p>
                        <span class="vexpl">
                        If you wish to use a local domain socket instead of a TCP socket, uncomment
                        the following. It is strongly recommended you use local domain sockets if
                        you are running the client and server on the same machine, as it eliminates
                        much of the bandwidth overhead.
                        </span>
                      </p>
                      <p>
                        <span class="vexpl">
                          Keep this is sync with <code>/usr/local/etc/rd.d/dspam.rc</code> script
                        </span>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'srv';" />
                    </td>
                  </tr>
                  <tr>
                    <td colspan="2" class="list" height="12">&nbsp;</td>
                  </tr>
                  <tr>
                    <td valign="top" class="listtopic"><a name="cli" style="visibility: hidden;">&nbsp;</a>DSPAM Daemon Settings (Client)</td>
                    <td align="right" valign="top" class="listtopic">
                      <input name="enabledsclient" type="checkbox" id="enabledsclient" value="yes" <?php if (isset($pconfig['enabledsclient'])) echo 'checked="checked"'; ?> onClick="enable_change(false, 4);" />
                      <strong>Enable</strong>
                    </td>
                  </tr>

                  <tr>
                    <td width="22%" valign="top" class="vncell">&nbsp;</td>
                    <td width="78%" class="vtable">
                      <p>
                        <span class="vexpl">
                        If you are running DSPAM in client/server mode, uncomment and
                        set these variables. A ClientHost beginning with a <code>/</code>
                        will be treated as a domain socket.
                        </span>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Client Host</td>
                    <td width="78%" class="vtable">
                      <p>
                        <input type="text" size="30" <?= checkForErrorClass("dsclhost", "formfld host"); ?> name="dsclhost" id="dsclhost" value="<?=htmlspecialchars($pconfig['dsclhost']);?>" <?php if (! isset($pconfig['enabledsclient'])) echo 'disabled="disabled"'; ?> />
                        <strong>
                        A IP address or a Unix domain socket.
                        </strong>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Client Port</td>
                    <td width="78%" class="vtable">
                      <p>
                        <input type="text" size="30" <?= checkForErrorClass("dsclhost", "formfld host"); ?> name="dsclport" id="dsclport" value="<?=htmlspecialchars($pconfig['dsclport']);?>" <?php if (! isset($pconfig['enabledsclient'])) echo 'disabled="disabled"'; ?> />
                        <strong>
                        Will be only used if this client uses TCP/IP communication.
                        </strong>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top" class="vncell">Client Ident</td>
                    <td width="78%" class="vtable">
                      <p>
                        <input type="text" size="30" <?= checkForErrorClass("dsclident", "formfld unknown"); ?> name="dsclident" id="dsclident" value="<?=htmlspecialchars($pconfig['dsclident']);?>" <?php if (! isset($pconfig['enabledsclient'])) echo 'disabled="disabled"'; ?> />
                        <strong>
                        A string that will be used to identify the client against a server.
                        </strong>
                      </p>
                    </td>
                  </tr>
                  <tr>
                    <td width="22%" valign="top">&nbsp;</td>
                    <td width="78%">
                      <!-- <input name="Submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)" /> -->
                      <input id="submitt" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onclick="document.iform.sectionid.value = 'cli';" />
                    </td>
                  </tr>
	              </table>
	          </td>
            </tr>
	      </table>
	    </div>
	  </td>
    </tr>
  </table>
  </form>
  <br>
  <?= checkForInputErrors(); ?>
<?
  } else {
?>
<?php
    $input_errors[] = "Access to this particular site was denied. You need DSPAM admin access rights to be able to view it.";

    include("head.inc");
    echo $pfSenseHead->getHTML();
?>
<?php include("fbegin.inc");?>
<?php if ($input_errors) print_input_errors($input_errors);?>
<?php if ($savemsg) print_info_box($savemsg);?>
  <body link="#000000" vlink="#000000" alink="#000000">
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr>
        <td valign="top" class="listtopic">Access denied for: <?=$HTTP_SERVER_VARS['AUTH_USER']?></td>
      </tr>
    </table>
<?php
  } // end of access denied code
?>
<?php include("fend.inc"); ?>
</body>
</html>