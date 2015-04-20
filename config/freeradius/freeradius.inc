<?php
require_once('config.inc');
require_once('service-utils.inc');

define('RADDB', '/usr/local/etc/raddb');

function freeradius_deinstall_command() {
	exec("cd /var/db/pkg && pkg_delete `ls | grep freeradius`");
	exec("cd /var/db/pkg && pkg_delete `ls | grep libltdl`");
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
	closedir($handle);

	freeradius_settings_resync();

	$rcfile = array();
	$rcfile['file'] = 'radiusd.sh';
	$rcfile['start'] = 'logger -f /var/log/system.log "freeRADIUS rc_start: killing all existing radiusd processes" && killall -9 radiusd  ; sleep 5 && logger -f /var/log/system.log "freeRADIUS rc_start: starting radiusd " ; radiusd -s &';
	$rcfile['stop'] = 'logger -f /var/log/system.log "freeRADIUS rc_stop: killing all existing radiusd processes" && killall -9 radiusd  ; sleep 5 && logger -f /var/log/system.log "freeRADIUS rc_stop: radiusd has quit"';
        conf_mount_rw();
	write_rcfile($rcfile);
        conf_mount_ro();
	start_service("freeradius");
}

function freeradius_settings_resync() {
	global $config;
	$settings = $config['installedpackages']['freeradiussettings']['config'][0];
	$iface = ($settings['interface'] ? $settings['interface'] : 'LAN');
	$iface = convert_friendly_interface_to_real_interface_name($iface);
	$iface_ip = find_interface_ip($iface);
	$interface_ip = $settings['interface_ip'];
	$port = ($settings['port'] != '' ? $settings['port'] : 0);
	$radiuslogging = $settings['radiuslogging'];
	$radiuslogbadpass = $settings['radiuslogbadpass'];
	$radiusloggoodpass = $settings['radiusloggoodpass'];
	$max_requests_var = $settings['max_requests_var'];
	$max_request_time_var = $settings['max_request_time_var'];
	$cleanup_delay_var = $settings['cleanup_delay_var'];
	$logdir_var = $settings['logdir_var'];

	// FreeRADIUS's configuration is huge
	// This is the standard default config file, trimmed down a bit. Somebody might want to implement more options. It should be as simple as editing this, then also providing the settings in each file that was included here (or maybe just put the config inlined here).
	$conf = <<<EOD
prefix = /usr/local
exec_prefix = \${prefix}
sysconfdir = \${prefix}/etc
localstatedir = /var
sbindir = \${exec_prefix}/sbin
logdir = $logdir_var
raddbdir = \${sysconfdir}/raddb
radacctdir = \${logdir}/radacct
confdir = \${raddbdir}
run_dir = \${localstatedir}/run/radiusd
log_file = \${logdir}/radius.log
libdir = \${exec_prefix}/lib
pidfile = \${run_dir}/radiusd.pid
#user = nobody
#group = nobody
max_request_time = $max_request_time_var
delete_blocked_requests = no
cleanup_delay = $cleanup_delay_var
max_requests = $max_requests_var
bind_address = $interface_ip
port = $port
hostname_lookups = no
allow_core_dumps = no
regular_expressions	= yes
extended_expressions	= yes
log_stripped_names = no
log_auth = $radiuslogging
log_auth_badpass = $radiuslogbadpass
log_auth_goodpass = $radiusloggoodpass
usercollide = no
lower_user = no
lower_pass = no
nospace_user = no
nospace_pass = no
checkrad = \${sbindir}/checkrad
 
security {
	max_attributes = 200
	reject_delay = 1
	status_server = no
}
 
proxy_requests = yes
\$INCLUDE  \${confdir}/proxy.conf
 
\$INCLUDE  \${confdir}/clients.conf
 
snmp = no
\$INCLUDE  \${confdir}/snmp.conf
 
thread pool {
	start_servers = 5
	max_servers = 32
	min_spare_servers = 3
	max_spare_servers = 10
	max_requests_per_server = 0
}
 
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
		delimiter = "\\"
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
	$clients = $config['installedpackages']['freeradiusclients']['config'];
	if (is_array($clients) && !empty($clients)) {
		foreach ($clients as $item) {
			$client = $item['client'];
			$secret = $item['sharedsecret'];
			$shortname = $item['shortname'];
			$conf .= <<<EOD
client $client {
	secret = $secret
	shortname = $shortname
}

EOD;
		}
	}
	else {
		$conf .= <<<EOD
client 127.0.0.1 {
	secret = pfsense
	shortname = localhost
}

EOD;
	}

        conf_mount_rw();
	file_put_contents(RADDB . '/clients.conf', $conf);
        conf_mount_ro();
	restart_service("freeradius");
}
?>
