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
	
	$varsettings = $config['installedpackages']['freeradiussettings']['config'][0];
	
	// Variables: General configuration
	$varsettingsmaxrequesttime = ($varsettings['varsettingsmaxrequesttime']?$varsettings['varsettingsmaxrequesttime']:'1024');
	$varsettingscleanupdelay = ($varsettings['varsettingscleanupdelay']?$varsettings['varsettingscleanupdelay']:'30');
	$varsettingsmaxrequests = ($varsettings['varsettingsmaxrequests']?$varsettings['varsettingsmaxrequests']:'5');
	$varsettingslogdir = ($varsettings['varsettingslogdir']?$varsettings['varsettingslogdir']:'no');
	$varsettingsstrippednames = ($varsettings['varsettingsstrippednames']?$varsettings['varsettingsstrippednames']:'no');
	
	// Variables: Logging options
	$varsettingsauth = ($varsettings['varsettingsauth']?$varsettings['varsettingsauth']:'yes');
	$varsettingsauthbadpass = ($varsettings['varsettingsauthbadpass']?$varsettings['varsettingsauthbadpass']:'yes');
	$varsettingsauthgoodpass = ($varsettings['varsettingsauthgoodpass']?$varsettings['varsettingsauthgoodpass']:'files');
	$varsettingshostnamelookups = ($varsettings['varsettingshostnamelookups']?$varsettings['varsettingshostnamelookups']:'no');
	$varsettingsallowcoredumps = ($varsettings['varsettingsallowcoredumps']?$varsettings['varsettingsallowcoredumps']:'no');
	$varsettingsregularexpressions = ($varsettings['varsettingsregularexpressions']?$varsettings['varsettingsregularexpressions']:'no');
	$varsettingsextendedexpressions = ($varsettings['varsettingsextendedexpressions']?$varsettings['varsettingsextendedexpressions']:'no');
	
	// Variables: Security
	$varsettingsmaxattributes = ($varsettings['varsettingsmaxattributes']?$varsettings['varsettingsmaxattributes']:'200');
	$varsettingsrejectdelay = ($varsettings['varsettingsrejectdelay']?$varsettings['varsettingsrejectdelay']:'1');
	
	// Variables: Thread Pool
	$varsettingsstartservers = ($varsettings['varsettingsstartservers']?$varsettings['varsettingsstartservers']:'5');
	$varsettingsmaxservers = ($varsettings['varsettingsmaxservers']?$varsettings['varsettingsmaxservers']:'32');
	$varsettingsminspareservers = ($varsettings['varsettingsminspareservers']?$varsettings['varsettingsminspareservers']:'3');
	$varsettingsmaxspareservers = ($varsettings['varsettingsmaxspareservers']?$varsettings['varsettingsmaxspareservers']:'10');
	$varsettingsmaxqueuesize = ($varsettings['varsettingsmaxqueuesize']?$varsettings['varsettingsmaxqueuesize']:'65536');
	$varsettingsmaxrequestsperserver = ($varsettings['varsettingsmaxrequestsperserver']?$varsettings['varsettingsmaxrequestsperserver']:'0');
	
	// These lines are uncommented in fuction "freeradius_settings_resync" to INCLUDE / enable eap.conf
	$sqlconf = $config['installedpackages']['freeradiussqlconf']['config'][0];
	$varsqlconfenable = ($sqlconf['varsqlconfenable']?$sqlconf['varsqlconfenable']:'#\$INCLUDE sql.conf');
	$varsqlconfenablecounter = ($sqlconf['varsqlconfenablecounter']?$sqlconf['varsqlconfenablecounter']:'#\$INCLUDE sql/mysql/counter.conf');
	
	

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
	if (is_array($arrinterfaces)  && !empty($arrinterfaces)) {
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
	
	### Original line
	### Enable sql.conf INCLUDE
	###\$INCLUDE sql.conf
	$varsqlconfenable
	
	
	### Original line
	### Enable sql/mysql/counter.conf INCLUDE
	#\$INCLUDE sql/mysql/counter.conf
	$varsqlconfenablecounter
	
	
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

$usersoutput = $config['installedpackages']['freeradius']['config'][0]['usersoutput']=base64_encode(file_get_contents('/usr/local/etc/raddb/users'));

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
	$vareapconfdefaulteaptype = ($eapconf['vareapconfdefaulteaptype']?$eapconf['vareapconfdefaulteaptype']:'md5');
	$vareapconftimerexpire = ($eapconf['vareapconftimerexpire']?$eapconf['vareapconftimerexpire']:'60');
	$vareapconfignoreunknowneaptypes = ($eapconf['vareapconfignoreunknowneaptypes']?$eapconf['vareapconfignoreunknowneaptypes']:'no');
	$vareapconfciscoaccountingusernamebug = ($eapconf['vareapconfciscoaccountingusernamebug']?$eapconf['vareapconfciscoaccountingusernamebug']:'no');
	$vareapconfmaxsessions = ($eapconf['vareapconfmaxsessions']?$eapconf['vareapconfmaxsessions']:'4096');
	
	// Variables: EAP-TLS and EAP-TLS with OCSP support
	$vareapconfprivatekeypassword = ($eapconf['vareapconfprivatekeypassword']?$eapconf['vareapconfprivatekeypassword']:'whatever');
	$vareapconfprivatekeyfile = ($eapconf['vareapconfprivatekeyfile']?$eapconf['vareapconfprivatekeyfile']:'server.pem');
	$vareapconfcertificatefile = ($eapconf['vareapconfcertificatefile']?$eapconf['vareapconfcertificatefile']:'server.pem');
	$vareapconfcafile = ($eapconf['vareapconfcafile']?$eapconf['vareapconfcafile']:'ca.pem');
	$vareapconfdhfile = ($eapconf['vareapconfdhfile']?$eapconf['vareapconfdhfile']:'dh');
	$vareapconfrandomfile = ($eapconf['vareapconfrandomfile']?$eapconf['vareapconfrandomfile']:'random');
	$vareapconfocspenable = ($eapconf['vareapconfocspenable']?$eapconf['vareapconfocspenable']:'no');
	$vareapconfocspoverridecerturl = ($eapconf['vareapconfocspoverridecerturl']?$eapconf['vareapconfocspoverridecerturl']:'no');
	$vareapconfocspurl = ($eapconf['vareapconfocspurl']?$eapconf['vareapconfocspurl']:'http://127.0.0.1/ocsp/');
	
	// Variables: EAP-TTLS
	$vareapconfttlsdefaulteaptype = ($eapconf['vareapconfttlsdefaulteaptype']?$eapconf['vareapconfttlsdefaulteaptype']:'md5');
	$vareapconfttlscopyrequesttotunnel = ($eapconf['vareapconfttlscopyrequesttotunnel']?$eapconf['vareapconfttlscopyrequesttotunnel']:'no');
	$vareapconfttlsusetunneledreply = ($eapconf['vareapconfttlsusetunneledreply']?$eapconf['vareapconfttlsusetunneledreply']:'no');
	
	// Variables: EAP-PEAP with MSCHAPv2
	$vareapconfpeapdefaulteaptype = ($eapconf['vareapconfpeapdefaulteaptype']?$eapconf['vareapconfpeapdefaulteaptype']:'mschapv2');
	$vareapconfpeapcopyrequesttotunnel = ($eapconf['vareapconfpeapcopyrequesttotunnel']?$eapconf['vareapconfpeapcopyrequesttotunnel']:'no');
	$vareapconfpeapusetunneledreply = ($eapconf['vareapconfpeapusetunneledreply']?$eapconf['vareapconfpeapusetunneledreply']:'no');
	
	
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

function freeradius_sqlconf_resync() {
	global $config;
	$conf = '';

	$sqlconf = $config['installedpackages']['freeradiussqlconf']['config'][0];
	
	// Variables: SQL
	$varsqlconfdatabase = ($sqlconf['varsqlconfdatabase']?$sqlconf['varsqlconfdatabase']:'mysql');
	$varsqlconfserver = ($sqlconf['varsqlconfserver']?$sqlconf['varsqlconfserver']:'localhost');
	$varsqlconfport = ($sqlconf['varsqlconfport']?$sqlconf['varsqlconfport']:'3306');
	$varsqlconflogin = ($sqlconf['varsqlconflogin']?$sqlconf['varsqlconflogin']:'radius');
	$varsqlconfpassword = ($sqlconf['varsqlconfpassword']?$sqlconf['varsqlconfpassword']:'radpass');
	$varsqlconfradiusdb = ($sqlconf['varsqlconfradiusdb']?$sqlconf['varsqlconfradiusdb']:'radius');
	$varsqlconfaccttable1 = ($sqlconf['varsqlconfaccttable1']?$sqlconf['varsqlconfaccttable1']:'radacct');
	$varsqlconfaccttable2 = ($sqlconf['varsqlconfaccttable2']?$sqlconf['varsqlconfaccttable2']:'radacct');
	$varsqlconfpostauthtable = ($sqlconf['varsqlconfpostauthtable']?$sqlconf['varsqlconfpostauthtable']:'radpostauth');
	$varsqlconfauthchecktable = ($sqlconf['varsqlconfauthchecktable']?$sqlconf['varsqlconfauthchecktable']:'radcheck');
	$varsqlconfauthreplytable = ($sqlconf['varsqlconfauthreplytable']?$sqlconf['varsqlconfauthreplytable']:'radreply');
	$varsqlconfgroupchecktable = ($sqlconf['varsqlconfgroupchecktable']?$sqlconf['varsqlconfgroupchecktable']:'radgroupcheck');
	$varsqlconfgroupreplytable = ($sqlconf['varsqlconfgroupreplytable']?$sqlconf['varsqlconfgroupreplytable']:'radgroupreply');
	$varsqlconfusergrouptable = ($sqlconf['varsqlconfusergrouptable']?$sqlconf['varsqlconfusergrouptable']:'radusergroup');
	$varsqlconfreadgroups = ($sqlconf['varsqlconfreadgroups']?$sqlconf['varsqlconfreadgroups']:'yes');
	$varsqlconfdeletestalesessions = ($sqlconf['varsqlconfdeletestalesessions']?$sqlconf['varsqlconfdeletestalesessions']:'yes');
	$varsqlconfsqltrace = ($sqlconf['varsqlconfsqltrace']?$sqlconf['varsqlconfsqltrace']:'no');
	$varsqlconfnumsqlsocks = ($sqlconf['varsqlconfnumsqlsocks']?$sqlconf['varsqlconfnumsqlsocks']:'5');
	$varsqlconfconnectfailureretrydelay = ($sqlconf['varsqlconfconnectfailureretrydelay']?$sqlconf['varsqlconfconnectfailureretrydelay']:'60');
	$varsqlconflifetime = ($sqlconf['varsqlconflifetime']?$sqlconf['varsqlconflifetime']:'0');
	$varsqlconfmaxqueries = ($sqlconf['varsqlconfmaxqueries']?$sqlconf['varsqlconfmaxqueries']:'0');
	$varsqlconfreadclients = ($sqlconf['varsqlconfreadclients']?$sqlconf['varsqlconfreadclients']:'yes');
	$varsqlconfnastable = ($sqlconf['varsqlconfnastable']?$sqlconf['varsqlconfnastable']:'nas');
	
	// These lines are uncommented in fuction "freeradius_settings_resync" to INCLUDE / enable eap.conf
	// $sqlconf = $config['installedpackages']['freeradiussqlconf']['config'][0];
	// $varsqlconfenable = ($sqlconf['varsqlconfenable']?$sqlconf['varsqlconfenable']:'#\$INCLUDE sql.conf');
	// $varsqlconfenablecounter = ($sqlconf['varsqlconfenablecounter']?$sqlconf['varsqlconfenablecounter']:'#\$INCLUDE sql/mysql/counter.conf');
	

	$conf .= <<<EOD

sql {
	database = "$varsqlconfdatabase"
	driver = "rlm_sql_\${database}"
	server = "$varsqlconfserver"
	port = $varsqlconfport
	login = "$varsqlconflogin"
	password = "$varsqlconfpassword"
	radius_db = "$varsqlconfradiusdb"
	acct_table1 = "$varsqlconfaccttable1"
	acct_table2 = "$varsqlconfaccttable2"
	postauth_table = "$varsqlconfpostauthtable"
	authcheck_table = "$varsqlconfauthchecktable"
	authreply_table = "$varsqlconfauthreplytable"
	groupcheck_table = "$varsqlconfgroupchecktable"
	groupreply_table = "$varsqlconfgroupreplytable"
	usergroup_table = "$varsqlconfusergrouptable"
	read_groups = $varsqlconfreadgroups
	deletestalesessions = $varsqlconfdeletestalesessions
	sqltrace = $varsqlconfsqltrace
	sqltracefile = \${logdir}/sqltrace.sql
	num_sql_socks = $varsqlconfnumsqlsocks
	connect_failure_retry_delay = $varsqlconfconnectfailureretrydelay
	lifetime = $varsqlconflifetime
	max_queries = $varsqlconfmaxqueries
	readclients = $varsqlconfreadclients
	nas_table = "$varsqlconfnastable"
	\$INCLUDE sql/\${database}/dialup.conf
}

EOD;

	$filename = RADDB . '/sql.conf';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();

	restart_service('freeradius');
}
?>