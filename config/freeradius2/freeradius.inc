<?php
require_once('config.inc');
require_once('service-utils.inc');

define('RADDB', '/usr/local/etc/raddb');

function freeradius_deinstall_command() {
	exec("cd /var/db/pkg && pkg_delete `ls | grep freeradius`");
	exec("rm -rf /usr/local/etc/raddb/");
	exec("rm -rf /var/log/raddb/");
	exec("rm -rf /var/log/radacct/");
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
	exec("chown -R root:wheel /var/log/raddb");
	exec("chown -R root:wheel /var/log/radacct");
	
	closedir($handle);

	freeradius_settings_resync();

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
			$description = $item['description'];
			$conf .= <<<EOD

listen {
		type = $varinterfacetype
		$varinterfaceipversion = $varinterfaceip
		port = $varinterfaceport
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
security 	{
			###max_attributes = 200
			###reject_delay = 1
			###status_server = no  ###raddb/sites-available/status ###wohl nur fuer Experten - erstmal weglassen
			}
			###proxy_requests = yes   ###auf "yes" lassen. Sorgt fuer weniger Probleme und kostet wenig/nichts (RAM)
\$INCLUDE  \${confdir}/proxy.conf
\$INCLUDE  \${confdir}/clients.conf   ###Jegliche Konfiguration wird in der clients.conf durchgeführt
thread pool {
			###start_servers = 5
			###max_servers = 32
			###min_spare_servers = 3
			###max_spare_servers = 10
			###max_queue_size = 65536
			###max_requests_per_server = 0
			}


#snmp = no
#\$INCLUDE  \${confdir}/snmp.conf

 
modules {
	pap {
		encryption_scheme = crypt
	}
 
	chap {
		authtype = CHAP
	}
 
	pam {
		pam_auth = radiusd
	}
 
	unix {
		cache = no
		cache_reload = 600
		radwtmp = \${logdir}/radwtmp
	}
 
	\$INCLUDE \${confdir}/eap.conf
 
	mschap {
		authtype = MS-CHAP
		#use_mppe = no
		#require_encryption = yes
		#require_strong = yes
		#with_ntdomain_hack = no
		#ntlm_auth = "/path/to/ntlm_auth --request-nt-key --username=%{Stripped-User-Name:-%{User-Name:-None}} --challenge=%{mschap:Challenge:-00} --nt-response=%{mschap:NT-Response:-00}"
	}
 
	ldap {
		server = "ldap.your.domain"
		basedn = "o=My Org,c=UA"
		filter = "(uid=%{Stripped-User-Name:-%{User-Name}})"
		#base_filter = "(objectclass=radiusprofile)"
		start_tls = no
		#tls_cacertfile = /path/to/cacert.pem
		#tls_cacertdir = /path/to/ca/dir/
		#tls_certfile = /path/to/radius.crt
		#tls_keyfile = /path/to/radius.key
		#tls_randfile = /path/to/rnd
		#tls_require_cert = "demand"
		access_attr = "dialupAccess"
		dictionary_mapping = \${raddbdir}/ldap.attrmap
		ldap_connections_number = 5
		#groupname_attribute = cn
		#groupmembership_filter = "(|(&(objectClass=GroupOfNames)(member=%{Ldap-UserDn}))(&(objectClass=GroupOfUniqueNames)(uniquemember=%{Ldap-UserDn})))"
		#groupmembership_attribute = radiusGroupName
		timeout = 4
		timelimit = 3
		net_timeout = 1
		#compare_check_items = yes
		#do_xlat = yes
		#access_attr_used_for_allow = yes
	}
 
	realm IPASS {
		format = prefix
		delimiter = "/"
		ignore_default = no
		ignore_null = no
	}
 
	realm suffix {
		format = suffix
		delimiter = "@"
		ignore_default = no
		ignore_null = no
	}
 
	realm realmpercent {
		format = suffix
		delimiter = "%"
		ignore_default = no
		ignore_null = no
	}
 
	realm ntdomain {
		format = prefix
		### There is "\\\" in freeradius.inc file and output is "\\" in radiusd.conf
		delimiter = "\\\"
		ignore_default = no
		ignore_null = no
	}
 
	checkval {
		item-name = Calling-Station-Id
		check-name = Calling-Station-Id
		data-type = string
		#notfound-reject = no
	}
 
	preprocess {
		huntgroups = \${confdir}/huntgroups
		hints = \${confdir}/hints
		with_ascend_hack = no
		ascend_channels_per_line = 23
		with_ntdomain_hack = no
		with_specialix_jetstream_hack = no
		with_cisco_vsa_hack = no
	}
 
	files {
		usersfile = \${confdir}/users
		acctusersfile = \${confdir}/acct_users
		preproxy_usersfile = \${confdir}/preproxy_users
		compat = no
	}
 
	detail {
		detailfile = \${radacctdir}/%{Client-IP-Address}/detail-%Y%m%d
		detailperm = 0600
	}
 
	acct_unique {
		key = "User-Name, Acct-Session-Id, NAS-IP-Address, Client-IP-Address, NAS-Port"
	}
 
	\$INCLUDE  \${confdir}/sql.conf
 
	radutmp {
		filename = \${logdir}/radutmp
		username = %{User-Name}
		case_sensitive = yes
		check_with_nas = yes
		perm = 0600
		callerid = "yes"
	}
 
	radutmp sradutmp {
		filename = \${logdir}/sradutmp
		perm = 0644
		callerid = "no"
	}
 
	attr_filter {
		attrsfile = \${confdir}/attrs
	}
 
	counter daily {
		filename = \${raddbdir}/db.daily
		key = User-Name
		count-attribute = Acct-Session-Time
		reset = daily
		counter-name = Daily-Session-Time
		check-name = Max-Daily-Session
		allowed-servicetype = Framed-User
		cache-size = 5000
	}
	
	counter weekly {
		filename = \${raddbdir}/db.weekly
		key = User-Name
		count-attribute = Acct-Session-Time
		reset = weekly
		counter-name = Weekly-Session-Time
		check-name = Max-Weekly-Session
		cache-size = 5000
	}
	
	counter monthly {
		filename = \${raddbdir}/db.monthly
		key = User-Name
		count-attribute = Acct-Session-Time
		reset = monthly
		counter-name = Monthly-Session-Time
		check-name = Max-Monthly-Session
		cache-size = 5000
	}

	counter forever {
		filename = \${raddbdir}/db.forever
		key = User-Name
		count-attribute = Acct-Session-Time
		reset = never
		counter-name = Forever-Session-Time
		check-name = Max-Forever-Session
		cache-size = 5000
	}
 
	always fail {
		rcode = fail
	}
	always reject {
		rcode = reject
	}
	always ok {
		rcode = ok
		simulcount = 0
		mpp = no
	}
 
	expr {
	}
 
	digest {
	}
 
	exec {
		wait = yes
		input_pairs = request
	}
 
	exec echo {
		wait = yes
		program = "/bin/echo %{User-Name}"
		input_pairs = request
		output_pairs = reply
		#packet_type = Access-Accept
	}
 
	ippool main_pool {
		range-start = 192.168.1.1
		range-stop = 192.168.3.254
		netmask = 255.255.255.0
		cache-size = 800
		session-db = \${raddbdir}/db.ippool
		ip-index = \${raddbdir}/db.ipindex
		override = no
		maximum-timeout = 0
	}
}
 
instantiate {
	exec
	expr
	daily
	weekly
	monthly
	forever
}
 
authorize {
	preprocess
	#auth_log
	#attr_filter
	chap
	mschap
	#digest
	#IPASS
	suffix
	#ntdomain
	eap
	files
	#sql
	#etc_smbpasswd
	#ldap
	daily
	weekly
	monthly
	forever
	#checkval
}
 
authenticate {
	Auth-Type PAP {
		pap
	}
	Auth-Type CHAP {
		chap
	}
	Auth-Type MS-CHAP {
		mschap
	}
	#digest
	#pam
	unix
	#Auth-Type LDAP {
	#	ldap
	#}
	eap
}
 
preacct {
	preprocess
	acct_unique
	#IPASS
	suffix
	#ntdomain
	files
}
 
accounting {
	detail
	daily
	weekly
	monthly
	forever
	unix
	radutmp
	#sradutmp
	#main_pool
	#sql
	#pgsql-voip
}
 
session {
	radutmp
	#sql
}
 
post-auth {
	#main_pool
	#reply_log
	#sql
	#ldap
	#Post-Auth-Type REJECT {
	#	insert-module-name-here
	#}
}
 
pre-proxy {
	#attr_rewrite
	#files
	#pre_proxy_log
}
 
post-proxy {
	#post_proxy_log
	#attr_rewrite
	#attr_filter
	eap
}

EOD;
	exec("chown -R root:wheel /var/log/raddb");
	exec("chown -R root:wheel /var/log/radacct");

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
		$head="$username User-Password == ".'"'.$password.'"';
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
	nastype     = $varclientnastype

		### Optional: will be used in future releases
	#login       = !root
	#password    = someadminpas
							
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
?>