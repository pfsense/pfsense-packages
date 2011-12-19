<?php
require_once('config.inc');
require_once('service-utils.inc');

define('RADDB', '/usr/local/etc/raddb');

function freeradius_deinstall_command() {
	exec("cd /var/db/pkg && pkg_delete `ls | grep freeradius`");
	exec("rm -rf /usr/local/etc/raddb/");
	exec("rm -rf /var/log/raddb/");
	exec("rm -rf /var/log/radacct/");
	exec("rm -rf /var/run/radiusd/");
	exec("rm -f /var/log/radius.log");
	exec("rm -f /var/log/radutmp");
	exec("rm -f /var/log/radwtmp");
}

function freeradius_install_command() {
	global $config;
        conf_mount_rw();
	$handle = opendir(RADDB);
	while (false != ($file = readdir($handle))) {
		if (false != ($pos = strpos($file, '.sample'))) {
			$newfile = substr($file, 0, $pos);
			if (copy(RADDB . "/$file", RADDB . "/$newfile"))
				unlink(RADDB . "/$file");
		}
	}
	
	exec("chown -R root:wheel /usr/local/etc/raddb");
	exec("chown -R root:wheel /usr/local/lib/freeradius-2.1.12");
	
	closedir($handle);

	$rcfile = array();
	$rcfile['file'] = 'radiusd.sh';
	$rcfile['start'] = '/usr/local/etc/rc.d/radiusd onestart';
	$rcfile['stop'] = '/usr/local/etc/rc.d/radiusd onestop';
        conf_mount_rw();
	write_rcfile($rcfile);
        conf_mount_ro();
	start_service("freeradius");
}

function freeradius_settings_resync() {
	global $config;
	$conf = '';
	
	// Definition variables for freeradiussettings
	$varsettings = $config['installedpackages']['freeradiussettings']['config'][0];
	$varsettingsmaxrequesttime = $varsettings['varsettingsmaxrequesttime'];
	$varsettingscleanupdelay = $varsettings['varsettingscleanupdelay'];
	$varsettingsmaxrequests = $varsettings['varsettingsmaxrequests'];
	$varsettingslogdir = $varsettings['varsettingslogdir'];
	$varsettingsstrippednames = $varsettings['varsettingsstrippednames'];
	$varsettingsauth = $varsettings['varsettingsauth'];
	$varsettingsauthbadpass = $varsettings['varsettingsauthbadpass'];
	$varsettingsauthgoodpass = $varsettings['varsettingsauthgoodpass'];
	$varsettingshostnamelookups = $varsettings['varsettingshostnamelookups'];
	$varsettingsallowcoredumps = $varsettings['varsettingsallowcoredumps'];
	$varsettingsregularexpressions = $varsettings['varsettingsregularexpressions'];
	$varsettingsextendedexpressions = $varsettings['varsettingsextendedexpressions'];
	$varsettingsmaxattributes = $varsettings['varsettingsmaxattributes'];
	$varsettingsrejectdelay = $varsettings['varsettingsrejectdelay'];
	$varsettingsstartservers = $varsettings['varsettingsstartservers'];
	$varsettingsmaxservers = $varsettings['varsettingsmaxservers'];
	$varsettingsminspareservers = $varsettings['varsettingsminspareservers'];
	$varsettingsmaxspareservers = $varsettings['varsettingsmaxspareservers'];
	$varsettingsmaxqueuesize = $varsettings['varsettingsmaxqueuesize'];
	$varsettingsmaxrequestsperserver = $varsettings['varsettingsmaxrequestsperserver'];

	$conf = <<<EOD
prefix = /usr/local
exec_prefix = \${prefix}
sysconfdir = \${prefix}/etc
localstatedir = /var
sbindir = \${exec_prefix}/sbin
logdir = \${localstatedir}/log
raddbdir = \${sysconfdir}/raddb
radacctdir = \${logdir}/radacct
confdir = \${raddbdir}
run_dir = \${localstatedir}/run
libdir = \${exec_prefix}/lib/freeradius-2.1.12
pidfile = \${run_dir}/radiusd.pid
db_dir = \${raddbdir}
name = radiusd
#chroot = /path/to/chroot/directory
#user = freeradius
#group = freeradius

###############################################################################
### Is not present in freeradius 2.x radiusd.conf anymore but it was in 1.x ###
### delete_blocked_requests = no                                            ###
### usercollide = no                                                        ###
### lower_user = no                                                         ###
### lower_pass = no                                                         ###
### nospace_user = no                                                       ###
### nospace_pass = no                                                       ###
###############################################################################

max_request_time = $varsettingsmaxrequesttime
cleanup_delay = $varsettingscleanupdelay
max_requests = $varsettingsmaxrequests
hostname_lookups = $varsettingshostnamelookups
allow_core_dumps = $varsettingsallowcoredumps
regular_expressions = $varsettingsregularexpressions
extended_expressions = $varsettingsextendedexpressions

EOD;

$arrinterfaces = $config['installedpackages']['freeradiusinterfaces']['config'];
	if (is_array($arrinterfaces)) {
		foreach ($arrinterfaces as $item) {
			$varinterfaceip = $item['varinterfaceip'];
			$varinterfaceport = $item['varinterfaceport'];
			$varinterfacetype = $item['varinterfacetype'];
			$varinterfaceipversion = $item['varinterfaceipversion'];
			$conf .= <<<EOD

listen {
		type = $varinterfacetype
		$varinterfaceipversion = $varinterfaceip
		port = $varinterfaceport
		### the following two lines are only neccessary for interface type detail
		filename = ${radacctdir}/detail
		load_factor = 10
}

EOD;
		}	// end foreach
	}		// end if
$conf .= <<<EOD

log {
	destination = $varsettingslogdir
	file = \${logdir}/radius.log
	syslog_facility = daemon
	stripped_names = $varsettingsstrippednames
	auth = $varsettingsauth
	auth_badpass = $varsettingsauthbadpass
	auth_goodpass = $varsettingsauthgoodpass
	###msg_goodpass = ""
	###msg_badpass = ""
}

checkrad = \${sbindir}/checkrad
security {
	max_attributes = $varsettingsmaxattributes
	reject_delay = $varsettingsrejectdelay
	status_server = no  		### Needs additional config in raddb/sites-available/status
}

### disbale proxy module. In most environments we do not need to proxy requests to another RADIUS PROXY server
#proxy_requests = yes
#\$INCLUDE  proxy.conf
\$INCLUDE  clients.conf
thread pool {
	start_servers = $varsettingsstartservers
	max_servers = $varsettingsmaxservers
	min_spare_servers = $varsettingsminspareservers
	max_spare_servers = $varsettingsmaxspareservers
	max_queue_size = $varsettingsmaxqueuesize
	max_requests_per_server = $varsettingsmaxrequestsperserver
}

modules {
	\$INCLUDE \${confdir}/modules/
	\$INCLUDE eap.conf
	#\$INCLUDE sql.conf
	#\$INCLUDE sql/mysql/counter.conf
	#\$INCLUDE sqlippool.conf
}

instantiate {

	exec
	expr
	#daily
	expiration
	logintime
	#redundant redundant_sql {
	#	sql1
	#	sql2
	#}
}
\$INCLUDE policy.conf
\$INCLUDE sites-enabled/


EOD;

	conf_mount_rw();
	file_put_contents(RADDB . '/radiusd.conf', $conf);
	conf_mount_ro();
	restart_service("freeradius");
}

function freeradius_users_resync() {
global $config;

$conf = '';
$users = $config['installedpackages']['freeradius']['config'];
if (is_array($users)) {
foreach ($users as $user) {
$username = $user['username'];
$password = $user['password'];
$multiconnect = $user['multiconnect'];
$ip = $user['ip'];
$subnetmask = $user['subnetmask'];
$gateway = $user['gateway'];
$userexpiration=$user['expiration'];
$sessiontime=$user['sessiontime'];
$onlinetime=$user['onlinetime'];
$vlanid=$user['vlanid'];
$additionaloptions=$user['additionaloptions'];
$atrib='';
$head="$username Cleartext-Password := ".'"'.$password.'"';
      if ($multiconnect <> '') {
        $head .=", Simultaneous-Use := $multiconnect";
       }
      if ($userexpiration <> '') {
        $head .=", Expiration := ".'"'.$userexpiration.'"';
       }
      if ($subnetmask<> '') {
        $head .=", Framed-IP-Netmask = $subnetmask";
       }
      if ($gateway<> '') {
        $head .=", Framed-Route = $gateway";
       }
      if ($onlinetime <> '') {
        $head .=", Login-Time := ". '"' . $onlinetime .'"';
       }
      if ($ip <> '') {
        if ($atrib <> '') { $atrib .=","; }
        $atrib .="\r\n\tFramed-IP-Address = $ip";
       }
      if ($sessiontime <> '') {
        if ($atrib <> '') { $atrib .=","; }
        $atrib .="\r\n\tSession-Timeout := $sessiontime";
       }
      if ($vlanid <> '') {
        if ($atrib <> '') { $atrib .=","; }
        $atrib .="\r\n\tTunnel-Type = VLAN,\r\n\tTunnel-Medium-Type = IEEE-802,\r\n\tTunnel-Private-Group-ID = \"$vlanid\"";
       }
      if ($additionaloptions <> '') {
        if ($atrib <> '') { $atrib .=","; }
        $atrib .="\r\n\t$additionaloptions";
       }
       
      $conf .= <<<EOD
$head
$atrib

EOD;
}
}
	$filename = RADDB . '/users';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();

	restart_service('freeradius');
}

function freeradius_clients_resync() {
	global $config;

	$conf = '';
	$arrclients = $config['installedpackages']['freeradiusclients']['config'];
	if (is_array($arrclients) && !empty($arrclients)) {
		foreach ($arrclients as $item) {
			$varclientip = $item['varclientip'];
			$varclientsharedsecret = $item['varclientsharedsecret'];
			$varclientipversion = $item['varclientipversion'];
			$varclientshortname = $item['varclientshortname'];
			$varclientproto = $item['varclientproto'];
			$varrequiremessageauthenticator = $item['varrequiremessageauthenticator'];
			$varclientnastype = $item['varclientnastype'];
			$varclientmaxconnections = $item['varclientmaxconnections'];
			$conf .= <<<EOD

client $varclientshortname {
	$varclientipversion = $varclientip

		### udp or tcp - udp is default
	proto = $varclientproto
	secret = $varclientsharedsecret

		### RFC5080: User Message-Authenticator in Access-Request. But older sqitches, accesspoints, NAS do not include that. Default: no
	require_message_authenticator = $varrequiremessageauthenticator

		### Takes only effect if you use TCP as protocol. This is the mirror of "max_requests" from "Settings" tab. Default 16
	max_connections = $varclientmaxconnections
	shortname = $varclientshortname

		### Optional: Used by checkrad.pl for simultaneous use checks. Default: other
	nastype = $varclientnastype

		### Optional: will be used in future releases
	#login = !root
	#password = someadminpas
							
		### Additional configuration needed. See: raddb/sites-available/originate-coa
	#virtual_server = home1
	#coa_server = coa
}

EOD;
		}
	}
	else {
		$conf .= <<<EOD
client pfsense {
	ipaddr = 127.0.0.1
	secret = pfsense
	shortname = pfsense
}

EOD;
	}

	conf_mount_rw();
	file_put_contents(RADDB . '/clients.conf', $conf);
	conf_mount_ro();
	restart_service("freeradius");
}



function freeradius_eapconf_resync() {
	global $config;
	$conf = '';

	$eapconf = $config['installedpackages']['freeradiuseapconf']['config'][0];
	
	// Variables: EAP
	$vareapconfdefaulteaptype = $eapconf['vareapconfdefaulteaptype'];
	$vareapconftimerexpire = $eapconf['vareapconftimerexpire'];
	$vareapconfignoreunknowneaptypes = $eapconf['vareapconfignoreunknowneaptypes'];
	$vareapconfciscoaccountingusernamebug = $eapconf['vareapconfciscoaccountingusernamebug'];
	$vareapconfmaxsessions = $eapconf['vareapconfmaxsessions'];
	
	// Variables: EAP-TLS and EAP-TLS with OCSP support
	$vareapconfprivatekeypassword = $eapconf['vareapconfprivatekeypassword'];
	$vareapconfprivatekeyfile = $eapconf['vareapconfprivatekeyfile'];
	$vareapconfcertificatefile = $eapconf['vareapconfcertificatefile'];
	$vareapconfcafile = $eapconf['vareapconfcafile'];
	$vareapconfdhfile = $eapconf['vareapconfdhfile'];
	$vareapconfrandomfile = $eapconf['vareapconfrandomfile'];
	$vareapconfocspenable = $eapconf['vareapconfocspenable'];
	$vareapconfocspoverridecerturl = $eapconf['vareapconfocspoverridecerturl'];
	$vareapconfocspurl = $eapconf['vareapconfocspurl'];
	
	// Variables: EAP-TTLS
	$vareapconfttlsdefaulteaptype = $eapconf['vareapconfttlsdefaulteaptype'];
	$vareapconfttlscopyrequesttotunnel = $eapconf['vareapconfttlscopyrequesttotunnel'];
	$vareapconfttlsusetunneledreply = $eapconf['vareapconfttlsusetunneledreply'];
	
	// Variables: EAP-PEAP with MSCHAPv2
	$vareapconfpeapdefaulteaptype = $eapconf['vareapconfpeapdefaulteaptype'];
	$vareapconfpeapcopyrequesttotunnel = $eapconf['vareapconfpeapcopyrequesttotunnel'];
	$vareapconfpeapusetunneledreply = $eapconf['vareapconfpeapusetunneledreply'];
	
	
	$conf .= <<<EOD

	### EAP
	eap {
		default_eap_type = $vareapconfdefaulteaptype
		timer_expire     = $vareapconftimerexpire
		ignore_unknown_eap_types = $vareapconfignoreunknowneaptypes
		cisco_accounting_username_bug = $vareapconfciscoaccountingusernamebug
		max_sessions = $vareapconfmaxsessions
		
		md5 {
		}
		leap {
		}
		gtc {
			#challenge = "Password: "
			auth_type = PAP
		}
		
		
	### EAP-TLS and EAP-TLS with OCSP support
		tls {
			certdir = \${confdir}/certs
			cadir = \${confdir}/certs
			private_key_password = $vareapconfprivatekeypassword
			private_key_file = \${certdir}/$vareapconfprivatekeyfile
			certificate_file = \${certdir}/$vareapconfcertificatefile
			CA_file = \${cadir}/$vareapconfcafile
			dh_file = \${certdir}/$vareapconfdhfile
			random_file = \${certdir}/$vareapconfrandomfile
		#	fragment_size = 1024
		#	include_length = yes
		#	check_crl = yes
			CA_path = \${cadir}
		#   check_cert_issuer = "/C=GB/ST=Berkshire/L=Newbury/O=My Company Ltd"
		#	check_cert_cn = %{User-Name}
			cipher_list = "DEFAULT"
			make_cert_command = "\${certdir}/bootstrap"
			ecdh_curve = "prime256v1"
			cache {
			      enable = no
			      lifetime = 24 # hours
			      max_entries = 255
			}
			verify {
		#     		tmpdir = /tmp/radiusd
		#    		client = "/path/to/openssl verify -CApath ${CA_path} %{TLS-Client-Cert-Filename}"
			}
			ocsp {
			      enable = $vareapconfocspenable
			      override_cert_url = $vareapconfocspoverridecerturl
			      url = "$vareapconfocspurl"
			}
		}	### end tls
	
	### EAP-TTLS
		ttls {
			default_eap_type = $vareapconfttlsdefaulteaptype
			copy_request_to_tunnel = $vareapconfttlscopyrequesttotunnel
			use_tunneled_reply = $vareapconfttlsusetunneledreply
		### if disabled this will be processed by the virtual server called "default"
		#	virtual_server = "inner-tunnel"
		#	include_length = yes
		}	### end ttls	
	
	### EAP-PEAP with MSCHAPv2
		peap {
			default_eap_type = $vareapconfpeapdefaulteaptype
			copy_request_to_tunnel = $vareapconfpeapcopyrequesttotunnel
			use_tunneled_reply = $vareapconfpeapusetunneledreply
		#	proxy_tunneled_request_as_eap = yes
		### if disabled this will be processed by the virtual server called "default"
		#	virtual_server = "inner-tunnel"
		#	soh = yes
		#	soh_virtual_server = "soh-server"
		}
		mschapv2 {
		#	send_error = no
		}	
	}	### end eap


EOD;

	$filename = RADDB . '/eap.conf';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();

	restart_service('freeradius');
}
?>