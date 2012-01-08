<?php
/* copyright */
/* ========================================================================== */
/*
	freeradius.inc
	part of pfSense (http://www.pfSense.com)
	Copyright (C) 2011 - 2012 Alexander Wilke <nachtfalkeaw@web.de>
	All rights reserved.

	Based on m0n0wall (http://m0n0.ch/wall)
	Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
	All rights reserved.
									      */
/* ========================================================================== */
/*
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
/* ========================================================================== */

require_once('config.inc');
require_once('service-utils.inc');
require_once("util.inc");
require_once("functions.inc");
require_once("pkg-utils.inc");
require_once("globals.inc");
require_once("filter.inc");
require_once("services.inc");

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
	
	closedir($handle);
	
	exec("chown -R root:wheel /usr/local/etc/raddb");
	exec("chown -R root:wheel /usr/local/lib/freeradius-2.1.12");
	
	// creating a backup file of the original policy.conf no matter if user checked this or not
	if (!file_exists("/usr/local/etc/raddb/policy.conf.backup")) {
		log_error("FreeRADIUS: Creating backup of the original file to {$filepolicyconfbackup}");
		copy("/usr/local/etc/raddb/policy.conf", "/usr/local/etc/raddb/policy.conf.backup");
	}
	
	// creating a backup file of the original /modules/files no matter if user checked this or not
	if (!file_exists("/usr/local/etc/raddb/files.backup")) {
		log_error("FreeRADIUS: Creating backup of the original file to {$filemodulesfilesbackup}");
		copy("/usr/local/etc/raddb/modules/files", "/usr/local/etc/raddb/files.backup");
	}
	
	// Disable virtual-server we do not need by default
	unlink("/usr/local/etc/raddb/sites-enabled/control-socket");
	unlink("/usr/local/etc/raddb/sites-enabled/inner-tunnel");

	$rcfile = array();
	$rcfile['file'] = 'radiusd.sh';
	$rcfile['start'] = '/usr/local/etc/rc.d/radiusd onestart';
	$rcfile['stop'] = '/usr/local/etc/rc.d/radiusd onestop';
        conf_mount_rw();
	write_rcfile($rcfile);
        conf_mount_ro();
	start_service("radiusd");
}

function freeradius_settings_resync() {
	global $config;
	$conf = '';
	
	$varsettings = $config['installedpackages']['freeradiussettings']['config'][0];
	
	// Variables: General configuration
	$varsettingsmaxrequests = ($varsettings['varsettingsmaxrequests']?$varsettings['varsettingsmaxrequests']:'1024');
	$varsettingsmaxrequesttime = ($varsettings['varsettingsmaxrequesttime']?$varsettings['varsettingsmaxrequesttime']:'30');
	$varsettingscleanupdelay = ($varsettings['varsettingscleanupdelay']?$varsettings['varsettingscleanupdelay']:'5');
	$varsettingshostnamelookups = ($varsettings['varsettingshostnamelookups']?$varsettings['varsettingshostnamelookups']:'no');
	$varsettingsallowcoredumps = ($varsettings['varsettingsallowcoredumps']?$varsettings['varsettingsallowcoredumps']:'no');
	$varsettingsregularexpressions = ($varsettings['varsettingsregularexpressions']?$varsettings['varsettingsregularexpressions']:'yes');
	$varsettingsextendedexpressions = ($varsettings['varsettingsextendedexpressions']?$varsettings['varsettingsextendedexpressions']:'yes');
	
	// Variables: Logging options
	$varsettingslogdir = ($varsettings['varsettingslogdir']?$varsettings['varsettingslogdir']:'syslog');
	$varsettingsauth = ($varsettings['varsettingsauth']?$varsettings['varsettingsauth']:'yes');
	$varsettingsauthbadpass = ($varsettings['varsettingsauthbadpass']?$varsettings['varsettingsauthbadpass']:'no');
	$varsettingsauthgoodpass = ($varsettings['varsettingsauthgoodpass']?$varsettings['varsettingsauthgoodpass']:'no');
	$varsettingsstrippednames = ($varsettings['varsettingsstrippednames']?$varsettings['varsettingsstrippednames']:'no');
	
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
	
	// For more details look at "freeradius_sqlconf_resync"
	$sqlconf = $config['installedpackages']['freeradiussqlconf']['config'][0];
	$varsqlconfincludeenable = ($sqlconf['varsqlconfincludeenable']?$sqlconf['varsqlconfincludeenable']:'Disable');
		
	// Dis-/Enable SQL in "instatiate" section in "freeradius_settings_resync" and radiusd.conf
	if ($sqlconf['varsqlconfincludeenable'] == 'Enable') {
		$varsqlconfinclude = '$INCLUDE sql.conf';
		$varsqlconfincludecounter = '$INCLUDE sql/mysql/counter.conf';
		$varsqlconfinstantiate = 'sql';
	}
	else {
		$varsqlconfinclude = '#$INCLUDE sql.conf';
		$varsqlconfincludecounter = '#$INCLUDE sql/mysql/counter.conf';
		$varsqlconfinstantiate = '#sql';
	}


	$conf .= <<<EOD
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

// Deletes virtual-server coa by default. Will be re-enabled if there is an interface-type "coa"
exec("rm -f /usr/local/etc/raddb/sites-enabled/coa");

$arrinterfaces = $config['installedpackages']['freeradiusinterfaces']['config'];
	if (is_array($arrinterfaces)  && !empty($arrinterfaces)) {
		foreach ($arrinterfaces as $item) {
			$varinterfaceip = $item['varinterfaceip'];
			$varinterfaceport = $item['varinterfaceport'];
			$varinterfacetype = $item['varinterfacetype'];
			$varinterfaceipversion = $item['varinterfaceipversion'];

			// Begin "if" for interface-type = detail
			if ($item['varinterfacetype'] == 'detail') {
			$conf .= <<<EOD
listen {
		type = $varinterfacetype
		$varinterfaceipversion = $varinterfaceip
		port = $varinterfaceport
		filename = \${radacctdir}/detail-%Y%m%d:%H
		load_factor = 10
}

EOD;
			}	// End "if" for interface-type = detail

			// Begin "if" for interface-type = coa
			if ($item['varinterfacetype'] == 'coa') {
			// Enables virtual-server coa because interface-type is coa
			exec("ln -s /usr/local/etc/raddb/sites-available/coa /usr/local/etc/raddb/sites-enabled/");
			$conf .= <<<EOD
listen {
		type = $varinterfacetype
		$varinterfaceipversion = $varinterfaceip
		port = $varinterfaceport
		server = coa
}

EOD;
			}	// End "if" for interface-type = coa

			// Begin "if" for interface-type = auth, acct, proxy, status
			if (($item['varinterfacetype'] == 'auth') || ($item['varinterfacetype'] == 'acct') || ($item['varinterfacetype'] == 'proxy') || ($item['varinterfacetype'] == 'status')) {
			$conf .= <<<EOD
listen {
		type = $varinterfacetype
		$varinterfaceipversion = $varinterfaceip
		port = $varinterfaceport
}

EOD;
			}	// End "if" for interface-type = auth, acct, proxy, status
		}	// end foreach
	}	// end if array


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
	status_server = no
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
	### Dis-/Enable sql.conf INCLUDE
	$varsqlconfinclude
	
	### Dis-/Enable sql/mysql/counter.conf INCLUDE
	$varsqlconfincludecounter
		
	#\$INCLUDE sqlippool.conf
}

instantiate {

	exec
	expr
	daily
	weekly
	monthly
	forever
	expiration
	logintime
	### Dis-/Enable sql instatiate
	$varsqlconfinstantiate
}
\$INCLUDE policy.conf
\$INCLUDE sites-enabled/


EOD;

	conf_mount_rw();
	file_put_contents(RADDB . '/radiusd.conf', $conf);
	conf_mount_ro();
	
	// "freeradius_sqlconf_resync" is pointing to this function because we need to run "freeradius_serverdefault_resync" and after that restart freeradius.
	freeradius_plainmacauth_resync();
	freeradius_serverdefault_resync();
	freeradius_modulescounter_resync();
	freeradius_modulesmschap_resync();
	freeradius_modulesrealm_resync();
	restart_service("radiusd");
}

function freeradius_users_resync() {
global $config;

$conf = '';

$arrusers = $config['installedpackages']['freeradius']['config'];

if (is_array($arrusers) && !empty($arrusers)) {
	foreach ($arrusers as $users) {
	
	// Variables for users file defined parameters
	$varusersusername = $users['varusersusername'];
	$varuserspassword = $users['varuserspassword'];
	$varuserssimultaneousconnect = ($users['varuserssimultaneousconnect']?$users['varuserssimultaneousconnect']:'1');
	$varusersframedipaddress = $users['varusersframedipaddress'];
	$varusersframedipnetmask = $users['varusersframedipnetmask'];
	$varusersframedroute = $users['varusersframedroute'];
	$varusersexpiration=$users['varusersexpiration'];
	$varuserssessiontimeout=$users['varuserssessiontimeout'];
	$varuserslogintime=$users['varuserslogintime'];
	$varusersvlanid=$users['varusersvlanid'];

	// Clear variables for next user foreach additional options TOP
	$varuserstopadditionaloptions = '';
	$varusersadditionaloptionstop = '';
	
	if(!empty($users['varuserstopadditionaloptions'])) {
		$varuserstopadditionaloptions = explode("|", ($users['varuserstopadditionaloptions']));
		foreach ($varuserstopadditionaloptions as $toptmp) {
			$varusersadditionaloptionstop .= $toptmp . "\n";
		}
	}
	
	// Clear variables for next user foreach additional options: CHECK-ITEMS
	$varuserscheckitemsadditionaloptions = '';
	$varusersadditionaloptionscheckitems = '';
	
	if(!empty($users['varuserscheckitemsadditionaloptions'])) {
		$varuserscheckitemsadditionaloptions = explode("|", ($users['varuserscheckitemsadditionaloptions']));
		$varusersadditionaloptionscheckitems .= '';
		foreach ($varuserscheckitemsadditionaloptions as $checkitemtmp) {
			$varusersadditionaloptionscheckitems .= $checkitemtmp;
		}
	}
		
	// Clear variables for next user foreach additional options: REPLY-ITEMS
	$varusersreplyitemsadditionaloptions = '';
	$varusersadditionaloptionsreplyitems = '';
	
	if(!empty($users['varusersreplyitemsadditionaloptions'])) {
		$varusersreplyitemsadditionaloptions = explode("|", ($users['varusersreplyitemsadditionaloptions']));
		$varusersadditionaloptionsreplyitems .= '';
		foreach ($varusersreplyitemsadditionaloptions as $replyitemtmp) {
			$varusersadditionaloptionsreplyitems .= $replyitemtmp . "\n\t";
		}
	}

	// Empty variable
	$varuserscheckitem = '';
	$varusersreplyitem = '';
	
	// Add the user attributes to each user.
	$varuserscheckitem = '"' . $varusersusername . '"' . " Cleartext-Password := " . '"' . $varuserspassword .'"';
	
	// Add additional CHECK-ITEMS here. Different formatting in "users" file needed.
	if ($varuserssimultaneousconnect != '') {
	$varuserscheckitem .= ", Simultaneous-Use := " . '"' . $varuserssimultaneousconnect . '"';
	}
	if ($varusersexpiration != '') {
	$varuserscheckitem .= ", Expiration := " . '"' . $varusersexpiration . '"';
	}
	if ($varuserslogintime != '') {
	$varuserscheckitem .= ", Login-Time := " . '"' . $varuserslogintime . '"';
	}
	if ($varusersadditionaloptionscheckitems != '') {
	$varuserscheckitem .= ", $varusersadditionaloptionscheckitems";
	}
	
		
	// Add additional REPLY-ITEMS here. Different formatting in "users" file needed.
	if ($varusersframedipaddress != '') {
		if ($varusersreplyitem != '') { $varusersreplyitem .=","; }
		$varusersreplyitem .= "\n\tFramed-IP-Address = $varusersframedipaddress";
	}
	if ($varusersframedipnetmask != '') {
		if ($varusersreplyitem != '') { $varusersreplyitem .=","; }
		$varusersreplyitem .= "\n\tFramed-IP-Netmask = $varusersframedipnetmask";
	}
	if ($varusersframedroute != '') {
		if ($varusersreplyitem != '') { $varusersreplyitem .=","; }
		$varusersreplyitem .= "\n\tFramed-Route = " . '"' . $varusersframedroute . '"';
	}
	if ($varuserssessiontimeout != '') {
		if ($varusersreplyitem != '') { $varusersreplyitem .=","; }
		$varusersreplyitem .= "\n\tSession-Timeout := $varuserssessiontimeout";
	}
	if ($varusersvlanid != '') {
		if ($varusersreplyitem != '') { $varusersreplyitem .=","; }
		$varusersreplyitem .= "\n\tTunnel-Type = VLAN,\n\tTunnel-Medium-Type = IEEE-802,\n\tTunnel-Private-Group-ID = " . '"' . $varusersvlanid . '"';
	}
	if ($varusersadditionaloptionsreplyitems != '') {
		if ($varusersreplyitem != '') { $varusersreplyitem .=","; }
		$varusersreplyitem .= "\n\t$varusersadditionaloptionsreplyitems";
	}
	
	// Cosmetic fix - This is just to make a blank new line after each user entry
	$varusersreplyitem .= "\n\n";
	
	
	$conf .= <<<EOD
$varusersadditionaloptionstop
$varuserscheckitem
	$varusersreplyitem
EOD;
	} //end foreach
} // end if

	$filename = RADDB . '/users';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();
	
	freeradius_sync_on_changes();
	restart_service('radiusd');
}


function freeradius_authorizedmacs_resync() {
global $config;

$conf = '';

$arrmacs = $config['installedpackages']['freeradiusauthorizedmacs']['config'];

if (is_array($arrmacs) && !empty($arrmacs)) {
	foreach ($arrmacs as $macs) {
	
	// Variables for macs file defined parameters
	$varmacaddress = $macs['varmacaddress'];
	$varmacaddress = "$varmacaddress" . "\n";
	
	$conf .= <<<EOD
$varmacaddress
EOD;
	} //end foreach
} // end if

	$filename = RADDB . '/authorized_macs';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();
	
	freeradius_sync_on_changes();
	restart_service('radiusd');
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
	proto = $varclientproto
	secret = $varclientsharedsecret
	require_message_authenticator = $varrequiremessageauthenticator
	max_connections = $varclientmaxconnections
	shortname = $varclientshortname
	nastype = $varclientnastype
	#login = !root
	#password = someadminpas
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
	
	freeradius_sync_on_changes();
	restart_service("radiusd");
}



function freeradius_eapconf_resync() {
	global $config;
	$conf = '';

	$eapconf = $config['installedpackages']['freeradiuseapconf']['config'][0];
	
	// Choose pfsense Cert-Manager or freeradius Cert-Manager
	$vareapconfchoosecertmanager = ($eapconf['vareapconfchoosecertmanager']?$eapconf['vareapconfchoosecertmanager']:'radiuscertmgr');
	
	// Variables: EAP
	$vareapconfdefaulteaptype = ($eapconf['vareapconfdefaulteaptype']?$eapconf['vareapconfdefaulteaptype']:'md5');
	$vareapconftimerexpire = ($eapconf['vareapconftimerexpire']?$eapconf['vareapconftimerexpire']:'60');
	$vareapconfignoreunknowneaptypes = ($eapconf['vareapconfignoreunknowneaptypes']?$eapconf['vareapconfignoreunknowneaptypes']:'no');
	$vareapconfciscoaccountingusernamebug = ($eapconf['vareapconfciscoaccountingusernamebug']?$eapconf['vareapconfciscoaccountingusernamebug']:'no');
	$vareapconfmaxsessions = ($eapconf['vareapconfmaxsessions']?$eapconf['vareapconfmaxsessions']:'4096');
	
	// Variables: EAP-TLS and EAP-TLS with OCSP support
	$vareapconfprivatekeypassword = ($eapconf['vareapconfprivatekeypassword']?$eapconf['vareapconfprivatekeypassword']:'whatever');
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
	$vareapconfpeapsohenable = ($eapconf['vareapconfpeapsohenable']?$eapconf['vareapconfpeapsohenable']:'Disable');
	
	// This is for enable/disbable MS SoH in EAP-PEAP and the virtuial-server "soh-server"
	if ($eapconf['vareapconfpeapsohenable'] == 'Enable') {
		$vareapconfpeapsoh = 'soh = yes' . "\n\t\t\tsoh_virtual_server = " . '"' . "soh-server" . '"';
		exec("ln -s /usr/local/etc/raddb/sites-available/soh /usr/local/etc/raddb/sites-enabled/");
	}
	else {
		$vareapconfpeapsoh = '### MS SoH Server is disabled ###';
			if (file_exists("/usr/local/etc/raddb/sites-enabled/soh")) {
			exec("rm -f /usr/local/etc/raddb/sites-enabled/soh");
			}
	}
		

// The filenames of pfsense cert manager are different from freeradius cert manager so it is possible to store both in the same folder at any time.
// This is for the pfsense cert manager
// Depends on "freeradius_get_server_certs" and "freeradius_get_ca_certs"
if ($vareapconfchoosecertmanager == 'pfsensecertmgr') {
		
		$ca_cert = lookup_ca($eapconf["ssl_ca_cert"]);
		if ($ca_cert != false) {
			if(base64_decode($ca_cert['prv'])) {
				file_put_contents(RADDB . "/certs/ca_key.pem", 
					base64_decode($ca_cert['prv']));
				$conf['ssl_ca_key'] = RADDB . '/certs/ca_key.pem';
			}


			if(base64_decode($ca_cert['crt'])) {
				file_put_contents(RADDB . "/certs/ca_cert.pem", 
					base64_decode($ca_cert['crt']));
				$conf['ssl_ca_cert'] = RADDB . "/certs/ca_cert.pem";
			}


			$svr_cert = lookup_cert($eapconf["ssl_server_cert"]);
			if ($svr_cert != false) {
				if(base64_decode($svr_cert['prv'])) {
					file_put_contents(RADDB . "/certs/server_key.pem", 
						base64_decode($svr_cert['prv']));
					$conf['ssl_key'] = RADDB . '/certs/server_key.pem';
				}
			}


			if(base64_decode($svr_cert['crt'])) {
				file_put_contents(RADDB . "/certs/server_cert.pem", 
					base64_decode($svr_cert['crt']));
				$conf['ssl_server_cert'] = RADDB . "/certs/server_cert.pem";
			}


			$conf['ssl_cert_dir'] = RADDB . '/certs';
		}

	$vareapconfprivatekeyfile = 'server_key.pem';
	$vareapconfcertificatefile = 'server_cert.pem';
	$vareapconfcafile = 'ca_cert.pem';
	
	// generate new DH and RANDOM file
	log_error("freeRADIUS: Switched to pfSense Cert-Manager. Creating new DH and random file in /usr/local/etc/raddb/certs");
	exec("cd /usr/local/etc/raddb/certs && openssl dhparam -out dh 1024");
	exec("cd /usr/local/etc/raddb/certs && dd if=/dev/urandom of=./random count=10");
}

// This is for freeradius cert manager
if ($vareapconfchoosecertmanager == 'radiuscertmgr') {

	$vareapconfprivatekeyfile = 'server.pem';
	$vareapconfcertificatefile = 'server.pem';
	$vareapconfcafile = 'ca.pem';

}

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
			dh_file = \${certdir}/dh
			random_file = \${certdir}/random
		#	fragment_size = 1024
		#	include_length = yes
		#	check_crl = yes
			CA_path = \${cadir}
		#   check_cert_issuer = "/C=GB/ST=Berkshire/L=Newbury/O=My Company Ltd"
		#	check_cert_cn = %{User-Name}
			cipher_list = "DEFAULT"
		###	we make this from Certificate tab on GUI at startup
		#	make_cert_command = "\${certdir}/bootstrap"
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
		#	include_length = yes
		}	### end ttls	
	
	### EAP-PEAP with MSCHAPv2
		peap {
			default_eap_type = $vareapconfpeapdefaulteaptype
			copy_request_to_tunnel = $vareapconfpeapcopyrequesttotunnel
			use_tunneled_reply = $vareapconfpeapusetunneledreply
		#	proxy_tunneled_request_as_eap = yes
			$vareapconfpeapsoh
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

	restart_service('radiusd');
}

// Gets started from freeradiuseapconf.xml
function freeradius_get_ca_certs() {
	global $config;
	$ca_arr = array();
	$ca_arr[] = array('refid' => 'none', 'descr' => 'none');

	foreach ($config['ca'] as $ca) {
		$ca_arr[] = array('refid' => $ca['refid'], 'descr' => $ca['descr']);
	}
	return $ca_arr;
}

// Gets started from freeradiuseapconf.xml
function freeradius_get_server_certs() {
	global $config;
	$cert_arr = array();
	$cert_arr[] = array('refid' => 'none', 'descr' => 'none');

	foreach ($config['cert'] as $cert) {
		$cert_arr[] = array('refid' => $cert['refid'], 'descr' => $cert['descr']);
	}
	return $cert_arr;
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
	
	// Additional changes were made in "freeradius_settings_resync"

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
	
	// We don't need a restart at this time because there are additional changes needed in:
	// "freeradius_settings_resync" and "freeradius_serverdefault_resync".
	// restart_service('radiusd');
	freeradius_settings_resync();
}

function freeradius_serverdefault_resync() {
	global $config;
	$conf = '';
	
	
	// Get Variables from freeradiusmodulesldap.xml
	$arrmodulesldap = $config['installedpackages']['freeradiusmodulesldap']['config'][0];
	
	// If unchecked then disable authorize
	if (!$arrmodulesldap['varmodulesldapenableauthorize']) {
	$varmodulesldapenableauthorize = '### ldap ###';
	}
	else {
	$varmodulesldapenableauthorize = 'ldap';
	}
	
	// If unchecked then disable authenticate
	if (!$arrmodulesldap['varmodulesldapenableauthenticate']) {
	$varmodulesldapenableauthenticate = "#Auth-Type LDAP {" . "\n\t\t\t#ldap" . "\n\t#}";
	}
	else {
	$varmodulesldapenableauthenticate = "Auth-Type LDAP {" . "\n\t\t\tldap" . "\n\t}";
	}
		
	// Get Variables from freeradiussqlconf.xml
	$sqlconf = $config['installedpackages']['freeradiussqlconf']['config'][0];
	$varsqlconfenableauthorize = ($sqlconf['varsqlconfenableauthorize']?$sqlconf['varsqlconfenableauthorize']:'Disable');
	$varsqlconfenableaccounting = ($sqlconf['varsqlconfenableaccounting']?$sqlconf['varsqlconfenableaccounting']:'Disable');
	$varsqlconfenablesession = ($sqlconf['varsqlconfenablesession']?$sqlconf['varsqlconfenablesession']:'Disable');
	$varsqlconfenablepostauth = ($sqlconf['varsqlconfenablepostauth']?$sqlconf['varsqlconfenablepostauth']:'Disable');	

	// authorize section
	if (($sqlconf['varsqlconfincludeenable'] == 'Enable') && ($sqlconf['varsqlconfenableauthorize'] == 'Enable')) {
	$varsqlconfauthorize = 'sql';
	}
	else {
	$varsqlconfauthorize = '#sql';
	}
	
	// accounting section
	if (($sqlconf['varsqlconfincludeenable'] == 'Enable') && ($sqlconf['varsqlconfenableaccounting'] == 'Enable')) {
	$varsqlconfaccounting = 'sql';
	}
	else {
	$varsqlconfaccounting = '#sql';
	}
	
	// session section
	if (($sqlconf['varsqlconfincludeenable'] == 'Enable') && ($sqlconf['varsqlconfenablesession'] == 'Enable')) {
	$varsqlconfsession = 'sql';
	}
	else {
	$varsqlconfsession = 'radutmp';
	}

	// post-auth section
	if (($sqlconf['varsqlconfincludeenable'] == 'Enable') && ($sqlconf['varsqlconfenablepostauth'] == 'Enable')) {
	$varsqlconfpostauth = 'sql';
	}
	else {
	$varsqlconfpostauth = '#sql';
	}
	
	// Changing authorize section for plain mac auth
	// Variables: If not using 802.1x, mac address must be known
	$varsettings = $config['installedpackages']['freeradiussettings']['config'][0];
	
	// If unchecked we need the normal EAP section.
	if (!$varsettings['varsettingsenablemacauth']) {
		$varplainmacauthenable = '';
		$varplainmacauthenable .= "eap {";
		$varplainmacauthenable .= "\n\tok = return";
		$varplainmacauthenable .= "\n\t}";

		$varplainmacpreacctenable = '';
		$varplainmacpreacctenable .= '##### ACCOUNTING FOR PLAIN MAC-AUTH DISABLED #####';
	}
	// If checked we need to check if it is plain mac or eap
	else {
		$varplainmacauthenable = '';
		$varplainmacauthenable .= "\t### FIRST check MAC address in authorized_macs and if that fails proceed with other checks below in else-section ###";
		$varplainmacauthenable .= "\n\t# if cleaning up the Calling-Station-Id...";
		$varplainmacauthenable .= "\n\trewrite_calling_station_id";
		$varplainmacauthenable .= "\n\t# now check against the authorized_macs file";
		$varplainmacauthenable .= "\n\tauthorized_macs";
		$varplainmacauthenable .= "\n\tif (ok) {";
		$varplainmacauthenable .= "\n\t\t\tupdate control {";
		$varplainmacauthenable .= "\n\t\t\tAuth-Type := Accept";
		$varplainmacauthenable .= "\n\t\t}";
		$varplainmacauthenable .= "\n\t}";
		$varplainmacauthenable .= "\n\t### Here we have to place all other authorize modules which should be check when MAC fails ###";

		$varplainmacpreacctenable = '';
		$varplainmacpreacctenable .= '##### ACCOUNTING FOR PLAIN MAC-AUTH ENABLED #####';
		$varplainmacpreacctenable .= "\n\trewrite_calling_station_id";
	}
	
	$conf .= <<<EOD

######################################################################
#
#	As of 2.0.0, FreeRADIUS supports virtual hosts using the
#	"server" section, and configuration directives.
#
#	Virtual hosts should be put into the "sites-available"
#	directory.  Soft links should be created in the "sites-enabled"
#	directory to these files.  This is done in a normal installation.
#
#	If you are using 802.1X (EAP) authentication, please see also
#	the "inner-tunnel" virtual server.  You wll likely have to edit
#	that, too, for authentication to work.
#
#	$Id$
#
######################################################################
#
#	Read "man radiusd" before editing this file.  See the section
#	titled DEBUGGING.  It outlines a method where you can quickly
#	obtain the configuration you want, without running into
#	trouble.  See also "man unlang", which documents the format
#	of this file.
#
#	This configuration is designed to work in the widest possible
#	set of circumstances, with the widest possible number of
#	authentication methods.  This means that in general, you should
#	need to make very few changes to this file.
#
#	The best way to configure the server for your local system
#	is to CAREFULLY edit this file.  Most attempts to make large
#	edits to this file will BREAK THE SERVER.  Any edits should
#	be small, and tested by running the server with "radiusd -X".
#	Once the edits have been verified to work, save a copy of these
#	configuration files somewhere.  (e.g. as a "tar" file).  Then,
#	make more edits, and test, as above.
#
#	There are many "commented out" references to modules such
#	as ldap, sql, etc.  These references serve as place-holders.
#	If you need the functionality of that module, then configure
#	it in radiusd.conf, and un-comment the references to it in
#	this file.  In most cases, those small changes will result
#	in the server being able to connect to the DB, and to
#	authenticate users.
#
######################################################################

#
#	In 1.x, the "authorize", etc. sections were global in
#	radiusd.conf.  As of 2.0, they SHOULD be in a server section.
#
#	The server section with no virtual server name is the "default"
#	section.  It is used when no server name is specified.
#
#	We don't indent the rest of this file, because doing so
#	would make it harder to read.
#

#  Authorization. First preprocess (hints and huntgroups files),
#  then realms, and finally look in the "users" file.
#
#  Any changes made here should also be made to the "inner-tunnel"
#  virtual server.
#
#  The order of the realm modules will determine the order that
#  we try to find a matching realm.
#
#  Make *sure* that 'preprocess' comes before any realm if you 
#  need to setup hints for the remote radius server
authorize {
	#
	#  Security settings.  Take a User-Name, and do some simple
	#  checks on it, for spaces and other invalid characters.  If
	#  it looks like the user is trying to play games, reject it.
	#
	#  This should probably be enabled by default.
	#
	#  See policy.conf for the definition of the filter_username policy.
	#
#	filter_username

	#
	#  The preprocess module takes care of sanitizing some bizarre
	#  attributes in the request, and turning them into attributes
	#  which are more standard.
	#
	#  It takes care of processing the 'raddb/hints' and the
	#  'raddb/huntgroups' files.
	preprocess
	
	#
	#  If you want to have a log of authentication requests,
	#  un-comment the following line, and the 'detail auth_log'
	#  section, above.
#	auth_log

	#
	#  The chap module will set 'Auth-Type := CHAP' if we are
	#  handling a CHAP request and Auth-Type has not already been set
	chap

	#
	#  If the users are logging in with an MS-CHAP-Challenge
	#  attribute for authentication, the mschap module will find
	#  the MS-CHAP-Challenge attribute, and add 'Auth-Type := MS-CHAP'
	#  to the request, which will cause the server to then use
	#  the mschap module for authentication.
	mschap

	#
	#  If you have a Cisco SIP server authenticating against
	#  FreeRADIUS, uncomment the following line, and the 'digest'
	#  line in the 'authenticate' section.
	digest

	#
	#  The WiMAX specification says that the Calling-Station-Id
	#  is 6 octets of the MAC.  This definition conflicts with
	#  RFC 3580, and all common RADIUS practices.  Un-commenting
	#  the "wimax" module here means that it will fix the
	#  Calling-Station-Id attribute to the normal format as
	#  specified in RFC 3580 Section 3.21
#	wimax

	#
	#  Look for IPASS style 'realm/', and if not found, look for
	#  '@realm', and decide whether or not to proxy, based on
	#  that.
#	IPASS

	#
	#  If you are using multiple kinds of realms, you probably
	#  want to set "ignore_null = yes" for all of them.
	#  Otherwise, when the first style of realm doesn't match,
	#  the other styles won't be checked.
	#
	
	suffix
	ntdomain
	
	#
	#  This module takes care of EAP-MD5, EAP-TLS, and EAP-LEAP
	#  authentication.
	#
	#  It also sets the EAP-Type attribute in the request
	#  attribute list to the EAP type from the packet.
	#
	#  As of 2.0, the EAP module returns "ok" in the authorize stage
	#  for TTLS and PEAP.  In 1.x, it never returned "ok" here, so
	#  this change is compatible with older configurations.
	#
	#  The example below uses module failover to avoid querying all
	#  of the following modules if the EAP module returns "ok".
	#  Therefore, your LDAP and/or SQL servers will not be queried
	#  for the many packets that go back and forth to set up TTLS
	#  or PEAP.  The load on those servers will therefore be reduced.
	#
	
	$varplainmacauthenable
	
	#
	#  Pull crypt'd passwords from /etc/passwd or /etc/shadow,
	#  using the system API's to get the password.  If you want
	#  to read /etc/passwd or /etc/shadow directly, see the
	#  passwd module in radiusd.conf.
	#
#	unix

	#
	#  Read the 'users' file
	files

	#
	#  Look in an SQL database.  The schema of the database
	#  is meant to mirror the "users" file.
	#
	#  See "Authorization Queries" in sql.conf
	$varsqlconfauthorize

	#
	#  If you are using /etc/smbpasswd, and are also doing
	#  mschap authentication, the un-comment this line, and
	#  configure the 'etc_smbpasswd' module, above.
#	etc_smbpasswd

	#
	#  The ldap module will set Auth-Type to LDAP if it has not
	#  already been set
	$varmodulesldapenableauthorize

	#
	#  Enforce daily limits on time spent logged in.
	daily
	weekly
	monthly
	forever

	#
	# Use the checkval module
	checkval

	expiration
	logintime

	#
	#  If no other module has claimed responsibility for
	#  authentication, then try to use PAP.  This allows the
	#  other modules listed above to add a "known good" password
	#  to the request, and to do nothing else.  The PAP module
	#  will then see that password, and use it to do PAP
	#  authentication.
	#
	#  This module should be listed last, so that the other modules
	#  get a chance to set Auth-Type for themselves.
	#
	pap
	
	#
	#  If "status_server = yes", then Status-Server messages are passed
	#  through the following section, and ONLY the following section.
	#  This permits you to do DB queries, for example.  If the modules
	#  listed here return "fail", then NO response is sent.
	#
	Autz-Type Status-Server {

	}
}


#  Authentication.
#
#
#  This section lists which modules are available for authentication.
#  Note that it does NOT mean 'try each module in order'.  It means
#  that a module from the 'authorize' section adds a configuration
#  attribute 'Auth-Type := FOO'.  That authentication type is then
#  used to pick the apropriate module from the list below.
#

#  In general, you SHOULD NOT set the Auth-Type attribute.  The server
#  will figure it out on its own, and will do the right thing.  The
#  most common side effect of erroneously setting the Auth-Type
#  attribute is that one authentication method will work, but the
#  others will not.
#
#  The common reasons to set the Auth-Type attribute by hand
#  is to either forcibly reject the user (Auth-Type := Reject),
#  or to or forcibly accept the user (Auth-Type := Accept).
#
#  Note that Auth-Type := Accept will NOT work with EAP.
#
#  Please do not put "unlang" configurations into the "authenticate"
#  section.  Put them in the "post-auth" section instead.  That's what
#  the post-auth section is for.
#
authenticate {
	#
	#  PAP authentication, when a back-end database listed
	#  in the 'authorize' section supplies a password.  The
	#  password can be clear-text, or encrypted.
	Auth-Type PAP {
		pap
	}

	#
	#  Most people want CHAP authentication
	#  A back-end database listed in the 'authorize' section
	#  MUST supply a CLEAR TEXT password.  Encrypted passwords
	#  won't work.
	Auth-Type CHAP {
		chap
	}

	#
	#  MSCHAP authentication.
	Auth-Type MS-CHAP {
		mschap
	}

	#
	#  If you have a Cisco SIP server authenticating against
	#  FreeRADIUS, uncomment the following line, and the 'digest'
	#  line in the 'authorize' section.
	digest

	#
	#  Pluggable Authentication Modules.
#	pam

	#
	#  See 'man getpwent' for information on how the 'unix'
	#  module checks the users password.  Note that packets
	#  containing CHAP-Password attributes CANNOT be authenticated
	#  against /etc/passwd!  See the FAQ for details.
	#
	#  For normal "crypt" authentication, the "pap" module should
	#  be used instead of the "unix" module.  The "unix" module should
	#  be used for authentication ONLY for compatibility with legacy
	#  FreeRADIUS configurations.
	#
	unix

	# Uncomment it if you want to use ldap for authentication
	#
	# Note that this means "check plain-text password against
	# the ldap database", which means that EAP won't work,
	# as it does not supply a plain-text password.
	$varmodulesldapenableauthenticate

	#
	#  Allow EAP authentication.
	eap

	#
	#  The older configurations sent a number of attributes in
	#  Access-Challenge packets, which wasn't strictly correct.
	#  If you want to filter out these attributes, uncomment
	#  the following lines.
	#
#	Auth-Type eap {
#		eap {
#			handled = 1  
#		}
#		if (handled && (Response-Packet-Type == Access-Challenge)) {
#			attr_filter.access_challenge.post-auth
#			handled  # override the "updated" code from attr_filter
#		}
#	}
}


#
#  Pre-accounting.  Decide which accounting type to use.
#
preacct {
	preprocess
	
	$varplainmacpreacctenable
	
	#
	#  Session start times are *implied* in RADIUS.
	#  The NAS never sends a "start time".  Instead, it sends
	#  a start packet, *possibly* with an Acct-Delay-Time.
	#  The server is supposed to conclude that the start time
	#  was "Acct-Delay-Time" seconds in the past.
	#
	#  The code below creates an explicit start time, which can
	#  then be used in other modules.
	#
	#  The start time is: NOW - delay - session_length
	#

#	  update request {
#	  	FreeRADIUS-Acct-Session-Start-Time = "%{expr: %l - %{%{Acct-Session-Time}:-0} - %{%{Acct-Delay-Time}:-0}}"
#	}


	#
	#  Ensure that we have a semi-unique identifier for every
	#  request, and many NAS boxes are broken.
	acct_unique

	#
	#  Look for IPASS-style 'realm/', and if not found, look for
	#  '@realm', and decide whether or not to proxy, based on
	#  that.
	#
	#  Accounting requests are generally proxied to the same
	#  home server as authentication requests.
#	IPASS
	suffix
	ntdomain

	#
	#  Read the 'acct_users' file
	files
}

#
#  Accounting.  Log the accounting data.
#
accounting {
	#
	#  Create a 'detail'ed log of the packets.
	#  Note that accounting requests which are proxied
	#  are also logged in the detail file.
	detail
	daily
	weekly
	monthly
	forever

	#  Update the wtmp file
	#
	#  If you don't use "radlast", you can delete this line.
	unix

	#
	#  For Simultaneous-Use tracking.
	#
	#  Due to packet losses in the network, the data here
	#  may be incorrect.  There is little we can do about it.
	radutmp
#	sradutmp

	#  Return an address to the IP Pool when we see a stop record.
#	main_pool

	#
	#  Log traffic to an SQL database.
	#
	#  See "Accounting queries" in sql.conf
	$varsqlconfaccounting

	#
	#  If you receive stop packets with zero session length,
	#  they will NOT be logged in the database.  The SQL module
	#  will print a message (only in debugging mode), and will
	#  return "noop".
	#
	#  You can ignore these packets by uncommenting the following
	#  three lines.  Otherwise, the server will not respond to the
	#  accounting request, and the NAS will retransmit.
	#
#	if (noop) {
#		ok
#	}

	#
	#  Instead of sending the query to the SQL server,
	#  write it into a log file.
	#
#	sql_log

	#  Cisco VoIP specific bulk accounting
#	pgsql-voip

	# For Exec-Program and Exec-Program-Wait
	exec

	#  Filter attributes from the accounting response.
	attr_filter.accounting_response

	#
	#  See "Autz-Type Status-Server" for how this works.
	#
	Acct-Type Status-Server {

	}
}


#  Session database, used for checking Simultaneous-Use. Either the radutmp 
#  or rlm_sql module can handle this.
#  The rlm_sql module is *much* faster
session {
	### choose radutmp or sql
	$varsqlconfsession
}


#  Post-Authentication
#  Once we KNOW that the user has been authenticated, there are
#  additional steps we can take.
post-auth {
	#  Get an address from the IP Pool.
#	main_pool

	#
	#  If you want to have a log of authentication replies,
	#  un-comment the following line, and the 'detail reply_log'
	#  section, above.
#	reply_log

	#
	#  After authenticating the user, do another SQL query.
	#
	#  See "Authentication Logging Queries" in sql.conf
	$varsqlconfpostauth

	#
	#  Instead of sending the query to the SQL server,
	#  write it into a log file.
	#
#	sql_log

	#
	#  Un-comment the following if you have set
	#  'edir_account_policy_check = yes' in the ldap module sub-section of
	#  the 'modules' section.
	#
#	ldap

	# For Exec-Program and Exec-Program-Wait
	exec

	#
	#  Calculate the various WiMAX keys.  In order for this to work,
	#  you will need to define the WiMAX NAI, usually via
	#
	#	update request {
	#	       WiMAX-MN-NAI = "%{User-Name}"
	#	}
	#
	#  If you want various keys to be calculated, you will need to
	#  update the reply with "template" values.  The module will see
	#  this, and replace the template values with the correct ones
	#  taken from the cryptographic calculations.  e.g.
	#
	# 	update reply {
	#		WiMAX-FA-RK-Key = 0x00
	#		WiMAX-MSK = "%{EAP-MSK}"
	#	}
	#
	#  You may want to delete the MS-MPPE-*-Keys from the reply,
	#  as some WiMAX clients behave badly when those attributes
	#  are included.  See "raddb/modules/wimax", configuration
	#  entry "delete_mppe_keys" for more information.
	#
#	wimax

	#  If there is a client certificate (EAP-TLS, sometimes PEAP
	#  and TTLS), then some attributes are filled out after the
	#  certificate verification has been performed.  These fields
	#  MAY be available during the authentication, or they may be
	#  available only in the "post-auth" section.
	#
	#  The first set of attributes contains information about the
	#  issuing certificate which is being used.  The second
	#  contains information about the client certificate (if
	#  available).
#
#	update reply {
#	       Reply-Message += "%{TLS-Cert-Serial}"
#	       Reply-Message += "%{TLS-Cert-Expiration}"
#	       Reply-Message += "%{TLS-Cert-Subject}"
#	       Reply-Message += "%{TLS-Cert-Issuer}"
#	       Reply-Message += "%{TLS-Cert-Common-Name}"
#
#	       Reply-Message += "%{TLS-Client-Cert-Serial}"
#	       Reply-Message += "%{TLS-Client-Cert-Expiration}"
#	       Reply-Message += "%{TLS-Client-Cert-Subject}"
#	       Reply-Message += "%{TLS-Client-Cert-Issuer}"
#	       Reply-Message += "%{TLS-Client-Cert-Common-Name}"
#	}


	#  If the WiMAX module did it's work, you may want to do more
	#  things here, like delete the MS-MPPE-*-Key attributes.
	#
	#	if (updated) {
	#		update reply {
	#			MS-MPPE-Recv-Key !* 0x00
	#			MS-MPPE-Send-Key !* 0x00
	#		}
	#	}

	#
	#  Access-Reject packets are sent through the REJECT sub-section of the
	#  post-auth section.
	#
	#  Add the ldap module name (or instance) if you have set 
	#  'edir_account_policy_check = yes' in the ldap module configuration
	#
	Post-Auth-Type REJECT {
		# log failed authentications in SQL, too.
#		sql
		attr_filter.access_reject
	}
}

#
#  When the server decides to proxy a request to a home server,
#  the proxied request is first passed through the pre-proxy
#  stage.  This stage can re-write the request, or decide to
#  cancel the proxy.
#
#  Only a few modules currently have this method.
#
pre-proxy {
#	attr_rewrite

	#  Uncomment the following line if you want to change attributes
	#  as defined in the preproxy_users file.
#	files

	#  Uncomment the following line if you want to filter requests
	#  sent to remote servers based on the rules defined in the
	#  'attrs.pre-proxy' file.
	attr_filter.pre-proxy

	#  If you want to have a log of packets proxied to a home
	#  server, un-comment the following line, and the
	#  'detail pre_proxy_log' section, above.
#	pre_proxy_log
}

#
#  When the server receives a reply to a request it proxied
#  to a home server, the request may be massaged here, in the
#  post-proxy stage.
#
post-proxy {

	#  If you want to have a log of replies from a home server,
	#  un-comment the following line, and the 'detail post_proxy_log'
	#  section, above.
#	post_proxy_log

#	attr_rewrite

	#  Uncomment the following line if you want to filter replies from
	#  remote proxies based on the rules defined in the 'attrs' file.
	attr_filter.post-proxy

	#
	#  If you are proxying LEAP, you MUST configure the EAP
	#  module, and you MUST list it here, in the post-proxy
	#  stage.
	#
	#  You MUST also use the 'nostrip' option in the 'realm'
	#  configuration.  Otherwise, the User-Name attribute
	#  in the proxied request will not match the user name
	#  hidden inside of the EAP packet, and the end server will
	#  reject the EAP request.
	#
	eap

	#
	#  If the server tries to proxy a request and fails, then the
	#  request is processed through the modules in this section.
	#
	#  The main use of this section is to permit robust proxying
	#  of accounting packets.  The server can be configured to
	#  proxy accounting packets as part of normal processing.
	#  Then, if the home server goes down, accounting packets can
	#  be logged to a local "detail" file, for processing with
	#  radrelay.  When the home server comes back up, radrelay
	#  will read the detail file, and send the packets to the
	#  home server.
	#
	#  With this configuration, the server always responds to
	#  Accounting-Requests from the NAS, but only writes
	#  accounting packets to disk if the home server is down.
	#
#	Post-Proxy-Type Fail {
#			detail
#	}
}

EOD;

	$filename = RADDB . '/sites-available/default';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();
	
	// No need to restart here because the restart of the service will be done in "freeradius_settings_resync"
	// restart_service('radiusd');
}

function freeradius_cacertcnf_resync() {
	global $config;
	$conf = '';

	$arrcerts = $config['installedpackages']['freeradiuscerts']['config'][0];
	
	// General variables: CA, Server, Client
	$varcertsdefaultdays = ($arrcerts['varcertsdefaultdays']?$arrcerts['varcertsdefaultdays']:'3650');
	$varcertsdefaultmd = ($arrcerts['varcertsdefaultmd']?$arrcerts['varcertsdefaultmd']:'md5');
	$varcertsdefaultbits = ($arrcerts['varcertsdefaultbits']?$arrcerts['varcertsdefaultbits']:'2048');
	$varcertspassword = ($arrcerts['varcertspassword']?$arrcerts['varcertspassword']:'whatever');
	$varcertscountryname = ($arrcerts['varcertscountryname']?$arrcerts['varcertscountryname']:'US');
	$varcertsstateorprovincename = ($arrcerts['varcertsstateorprovincename']?$arrcerts['varcertsstateorprovincename']:'Texas');
	$varcertslocalityname = ($arrcerts['varcertslocalityname']?$arrcerts['varcertslocalityname']:'Austin');
	$varcertsorganizationname = ($arrcerts['varcertsorganizationname']?$arrcerts['varcertsorganizationname']:'My Company Inc');

	// Variables: Only for CA
	$varcertscaemailaddress = ($arrcerts['varcertscaemailaddress']?$arrcerts['varcertscaemailaddress']:'admin@mycompany.com');
	$varcertscacommonname = ($arrcerts['varcertscacommonname']?$arrcerts['varcertscacommonname']:'internal-ca');
	
	
	

	$conf .= <<<EOD
[ ca ]
default_ca		= CA_default

[ CA_default ]
dir			= ./
certs			= \$dir
crl_dir			= \$dir/crl
database		= \$dir/index.txt
new_certs_dir		= \$dir
certificate		= \$dir/ca.pem
serial			= \$dir/serial
crl			= \$dir/crl.pem
private_key		= \$dir/ca.key
RANDFILE		= \$dir/.rand
name_opt		= ca_default
cert_opt		= ca_default
default_days		= $varcertsdefaultdays
default_crl_days	= 30
default_md		= $varcertsdefaultmd
preserve		= no
policy			= policy_match

[ policy_match ]
countryName		= match
stateOrProvinceName	= match
organizationName	= match
organizationalUnitName	= optional
commonName		= supplied
emailAddress		= optional

[ policy_anything ]
countryName		= optional
stateOrProvinceName	= optional
localityName		= optional
organizationName	= optional
organizationalUnitName	= optional
commonName		= supplied
emailAddress		= optional

[ req ]
prompt			= no
distinguished_name	= certificate_authority
default_bits		= $varcertsdefaultbits
input_password		= $varcertspassword
output_password		= $varcertspassword
x509_extensions		= v3_ca

[certificate_authority]
countryName		= $varcertscountryname
stateOrProvinceName	= $varcertsstateorprovincename
localityName		= $varcertslocalityname
organizationName	= $varcertsorganizationname
emailAddress		= $varcertscaemailaddress
commonName		= "$varcertscacommonname"

[v3_ca]
subjectKeyIdentifier	= hash
authorityKeyIdentifier	= keyid:always,issuer:always
basicConstraints	= CA:true

EOD;

	$filename = RADDB . '/certs/ca.cnf';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();

}

function freeradius_servercertcnf_resync() {
	global $config;
	$conf = '';

	$arrcerts = $config['installedpackages']['freeradiuscerts']['config'][0];
	
	// General variables: CA, Server, Client
	$varcertsdefaultdays = ($arrcerts['varcertsdefaultdays']?$arrcerts['varcertsdefaultdays']:'3650');
	$varcertsdefaultmd = ($arrcerts['varcertsdefaultmd']?$arrcerts['varcertsdefaultmd']:'md5');
	$varcertsdefaultbits = ($arrcerts['varcertsdefaultbits']?$arrcerts['varcertsdefaultbits']:'2048');
	$varcertspassword = ($arrcerts['varcertspassword']?$arrcerts['varcertspassword']:'whatever');
	$varcertscountryname = ($arrcerts['varcertscountryname']?$arrcerts['varcertscountryname']:'US');
	$varcertsstateorprovincename = ($arrcerts['varcertsstateorprovincename']?$arrcerts['varcertsstateorprovincename']:'Texas');
	$varcertslocalityname = ($arrcerts['varcertslocalityname']?$arrcerts['varcertslocalityname']:'Austin');
	$varcertsorganizationname = ($arrcerts['varcertsorganizationname']?$arrcerts['varcertsorganizationname']:'My Company Inc');

	// Variables: Only for Server
	$varcertsserveremailaddress = ($arrcerts['varcertsserveremailaddress']?$arrcerts['varcertsserveremailaddress']:'webadmin@mycompany.com');
	$varcertsservercommonname = ($arrcerts['varcertsservercommonname']?$arrcerts['varcertsservercommonname']:'server-cert');
	

	$conf .= <<<EOD
[ ca ]
default_ca		= CA_default

[ CA_default ]
dir			= ./
certs			= \$dir
crl_dir			= \$dir/crl
database		= \$dir/index.txt
new_certs_dir		= \$dir
certificate		= \$dir/server.pem
serial			= \$dir/serial
crl			= \$dir/crl.pem
private_key		= \$dir/server.key
RANDFILE		= \$dir/.rand
name_opt		= ca_default
cert_opt		= ca_default
default_days		= $varcertsdefaultdays
default_crl_days	= 30
default_md		= $varcertsdefaultmd
preserve		= no
policy			= policy_match

[ policy_match ]
countryName		= match
stateOrProvinceName	= match
organizationName	= match
organizationalUnitName	= optional
commonName		= supplied
emailAddress		= optional

[ policy_anything ]
countryName		= optional
stateOrProvinceName	= optional
localityName		= optional
organizationName	= optional
organizationalUnitName	= optional
commonName		= supplied
emailAddress		= optional

[ req ]
prompt			= no
distinguished_name	= server
default_bits		= $varcertsdefaultbits
input_password		= $varcertspassword
output_password		= $varcertspassword

[server]
countryName		= $varcertscountryname
stateOrProvinceName	= $varcertsstateorprovincename
localityName		= $varcertslocalityname
organizationName	= $varcertsorganizationname
emailAddress		= $varcertsserveremailaddress
commonName		= "$varcertsservercommonname"

EOD;

	$filename = RADDB . '/certs/server.cnf';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();

}

function freeradius_clientcertcnf_resync() {
	global $config;
	$conf = '';

	$arrcerts = $config['installedpackages']['freeradiuscerts']['config'][0];
	
	// General variables: CA, Server, Client
	$varcertsdefaultdays = ($arrcerts['varcertsdefaultdays']?$arrcerts['varcertsdefaultdays']:'3650');
	$varcertsdefaultmd = ($arrcerts['varcertsdefaultmd']?$arrcerts['varcertsdefaultmd']:'md5');
	$varcertsdefaultbits = ($arrcerts['varcertsdefaultbits']?$arrcerts['varcertsdefaultbits']:'2048');
	$varcertspassword = ($arrcerts['varcertspassword']?$arrcerts['varcertspassword']:'whatever');
	$varcertscountryname = ($arrcerts['varcertscountryname']?$arrcerts['varcertscountryname']:'US');
	$varcertsstateorprovincename = ($arrcerts['varcertsstateorprovincename']?$arrcerts['varcertsstateorprovincename']:'Texas');
	$varcertslocalityname = ($arrcerts['varcertslocalityname']?$arrcerts['varcertslocalityname']:'Austin');
	$varcertsorganizationname = ($arrcerts['varcertsorganizationname']?$arrcerts['varcertsorganizationname']:'My Company Inc');

	// Variables: Only for Client
	$varcertsclientemailaddress = ($arrcerts['varcertsclientemailaddress']?$arrcerts['varcertsclientemailaddress']:'user@mycompany.com');
	$varcertsclientcommonname = ($arrcerts['varcertsclientcommonname']?$arrcerts['varcertsclientcommonname']:'client-cert');
	

	$conf .= <<<EOD
[ ca ]
default_ca		= CA_default

[ CA_default ]
dir			= ./
certs			= \$dir
crl_dir			= \$dir/crl
database		= \$dir/index.txt
new_certs_dir		= \$dir
certificate		= \$dir/server.pem
serial			= \$dir/serial
crl			= \$dir/crl.pem
private_key		= \$dir/server.key
RANDFILE		= \$dir/.rand
name_opt		= ca_default
cert_opt		= ca_default
default_days		= $varcertsdefaultdays
default_crl_days	= 30
default_md		= $varcertsdefaultmd
preserve		= no
policy			= policy_match

[ policy_match ]
countryName		= match
stateOrProvinceName	= match
organizationName	= match
organizationalUnitName	= optional
commonName		= supplied
emailAddress		= optional

[ policy_anything ]
countryName		= optional
stateOrProvinceName	= optional
localityName		= optional
organizationName	= optional
organizationalUnitName	= optional
commonName		= supplied
emailAddress		= optional

[ req ]
prompt			= no
distinguished_name	= client
default_bits		= $varcertsdefaultbits
input_password		= $varcertspassword
output_password		= $varcertspassword

[client]
countryName		= $varcertscountryname
stateOrProvinceName	= $varcertsstateorprovincename
localityName		= $varcertslocalityname
organizationName	= $varcertsorganizationname
emailAddress		= $varcertsclientemailaddress
commonName		= "$varcertsclientcommonname"

EOD;

	$filename = RADDB . '/certs/client.cnf';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();

}

function freeradius_allcertcnf_resync() {
	global $config;

	$arrcerts = $config['installedpackages']['freeradiuscerts']['config'][0];
	
	// General variable for deleting and generation of further Client-Cert
	$varcertscreateclient = ($arrcerts['varcertscreateclient']?$arrcerts['varcertscreateclient']:'no');
	
	// General variables for deleting: CA, Server, Client
	$varcertsdeleteall = ($arrcerts['varcertsdeleteall']?$arrcerts['varcertsdeleteall']:'no');
		
	
		if ($arrcerts['varcertscreateclient'] == 'yes') {
		
		// delete all old certificates and keys
		log_error("freeRADIUS: deleting all client.csr .crt .key .pem .tar in /usr/local/etc/raddb/certs");
		exec("rm -f /usr/local/etc/raddb/certs/client.csr");
		exec("rm -f /usr/local/etc/raddb/certs/client.crt");
		exec("rm -f /usr/local/etc/raddb/certs/client.key");
		exec("rm -f /usr/local/etc/raddb/certs/client.pem");
		exec("rm -f /usr/local/etc/raddb/certs/client.tar");
				
			
		// run fuction to create ONLY new client.cnf files based on user input from freeradiuscert.xml
		freeradius_clientcertcnf_resync();
	
	
		// make bootstrap executable and run to create cert based on client.cnf files
		exec("chmod 0770 /usr/local/etc/raddb/certs/bootstrap");
		exec("/usr/local/etc/raddb/certs/bootstrap");
			
		// rename client generated XX.pem to client.pem // use regex to replace spaces and so on.
		$varserial = preg_replace("/\s/","",file_get_contents('/usr/local/etc/raddb/certs/serial.old'));
		if (file_exists("/usr/local/etc/raddb/certs/$varserial.pem"))
		rename("/usr/local/etc/raddb/certs/$varserial.pem","/usr/local/etc/raddb/certs/client.pem");
		
		
		// tar client-cert files
		exec("cd /usr/local/etc/raddb/certs && tar -cf client.tar client.crt client.csr client.key ca.der client.pem");
		
		// Make all files in certs folder read/write only for root
		exec("chmod -R 0600 /usr/local/etc/raddb/certs/");
		log_error("freeRADIUS: Created new client.csr .crt .key .pem and added them together with ca.der in /usr/local/etc/raddb/certs/client.tar");
		}

		
		if ($arrcerts['varcertsdeleteall'] == 'yes') {
	
		// delete all old certificates and keys - deletes certs from pfsense cert-manager IN THIS FOLDER, too.
		log_error("freeRADIUS: deleting all CA, Server and Client certs, DH, random and database files in /usr/local/etc/raddb/certs");
		exec("rm -f /usr/local/etc/raddb/certs/*.pem");
		exec("rm -f /usr/local/etc/raddb/certs/*.der");
		exec("rm -f /usr/local/etc/raddb/certs/*.csr");
		exec("rm -f /usr/local/etc/raddb/certs/*.crt");
		exec("rm -f /usr/local/etc/raddb/certs/*.key");
		exec("rm -f /usr/local/etc/raddb/certs/*.p12");
		exec("rm -f /usr/local/etc/raddb/certs/serial*");
		exec("rm -f /usr/local/etc/raddb/certs/index*");
		exec("rm -f /usr/local/etc/raddb/certs/dh");
		exec("rm -f /usr/local/etc/raddb/certs/random");
		exec("rm -f /usr/local/etc/raddb/certs/client.tar");
		
	
		// run fuctions to create new .cnf files based on user input from freeradiuscert.xml
		freeradius_cacertcnf_resync();
		freeradius_servercertcnf_resync();
		freeradius_clientcertcnf_resync();
		
		// generate new DH and RANDOM file
		log_error("freeRADIUS: Creating new DH and random file in /usr/local/etc/raddb/certs");
		exec("cd /usr/local/etc/raddb/certs && openssl dhparam -out dh 1024");
		exec("cd /usr/local/etc/raddb/certs && dd if=/dev/urandom of=./random count=10");
	
		log_error("freeRADIUS: Creating new CA, Server and Client certs in /usr/local/etc/raddb/certs");
		// make bootstrap executable and run to create certs based on .cnf files
		exec("chmod 0770 /usr/local/etc/raddb/certs/bootstrap");
		exec("/usr/local/etc/raddb/certs/bootstrap");
		
		// rename client generated 02.pem to client.pem
		if (file_exists("/usr/local/etc/raddb/certs/02.pem"))
		rename("/usr/local/etc/raddb/certs/02.pem","/usr/local/etc/raddb/certs/client.pem");
		
		// tar client-cert files
		exec("cd /usr/local/etc/raddb/certs && tar -cf client.tar client.crt client.csr client.key ca.der client.pem");
		exec("chmod -R 0600 /usr/local/etc/raddb/certs/");
		log_error("freeRADIUS: Added client.csr .crt .key .pem together with ca.der in /usr/local/etc/raddb/certs/client.tar");
		
		// If there were changes on the certificates we need to restart freeradius
		restart_service('radiusd');
		}
}

// ##### The following part is based on the code of pfblocker #####

/* Uses XMLRPC to synchronize the changes to a remote node */
function freeradius_sync_on_changes() {
	global $config, $g;
	$varsyncenablexmlrpc = $config['installedpackages']['freeradiussync']['config'][0]['varsyncenablexmlrpc'];	
	
	// if checkbox is NOT checked do nothing
	if(!$varsyncenablexmlrpc) {
		return;
	}
	
	log_error("freeRADIUS is starting XMLRPC process (freeradius_do_xmlrpc_sync).");	
	
	// if checkbox is checked get IP and password of the destination hosts
	foreach ($config['installedpackages']['freeradiussync']['config'] as $rs ){
		foreach($rs['row'] as $sh){
		$sync_to_ip = $sh['varsyncipaddress'];
		$password   = $sh['varsyncpassword'];
		if($password && $sync_to_ip)
			freeradius_do_xmlrpc_sync($sync_to_ip, $password);
		}
	}
	log_error("freeRADIUS has finished XMLRPC process (freeradius_do_xmlrpc_sync).");
}

/* Do the actual XMLRPC sync */
function freeradius_do_xmlrpc_sync($sync_to_ip, $password) {
	global $config, $g;

	if(!$password)
		return;

	if(!$sync_to_ip)
		return;
	
	// Check and choose correct protocol type, port number and IP address
	$xmlrpc_sync_neighbor = $sync_to_ip;
    if($config['system']['webgui']['protocol'] != "") {
		$synchronizetoip = $config['system']['webgui']['protocol'];
		$synchronizetoip .= "://";
    }
    $port = $config['system']['webgui']['port'];
    /* if port is empty lets rely on the protocol selection */
    if($port == "") {
		if($config['system']['webgui']['protocol'] == "http") 
			$port = "80";
		else 
			$port = "443";
    }
	$synchronizetoip .= $sync_to_ip;

	/* xml will hold the sections to sync */
	$xml = array();
	$xml['freeradius'] = $config['installedpackages']['freeradius'];
	$xml['freeradiusclients'] = $config['installedpackages']['freeradiusclients'];
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Beginning freeRADIUS XMLRPC sync with {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
		if($g['debug'])
			$cli->setDebug(1);
		/* send our XMLRPC message and timeout after 250 seconds */
		$resp = $cli->send($msg, "250");
		if(!$resp) {
			$error = "A communications error occurred while freeRADIUS was attempting XMLRPC sync with {$url}:{$port}.";
			log_error($error);
			file_notice("sync_settings", $error, "freeradius Settings Sync", "");
		} elseif($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, "250");
			$error = "An error code was received while freeRADIUS XMLRPC was attempting to sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "freeradius Settings Sync", "");
		} else {
			log_error("freeRADIUS XMLRPC has synced data successfully with {$url}:{$port}.");
		}
	
	/* tell freeradius to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/freeradius.inc');\n";
	// pfblocker just needed one fuction to reload after XMLRPC. freeRADIUS needs more so we point to a fuction below which contains all fuctions
	$execcmd .= "freeradius_all_after_XMLRPC_resync();";
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);

	log_error("freeRADIUS XMLRPC is reloading data on {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
		$resp = $cli->send($msg, "250");
		if(!$resp) {
			$error = "A communications error occurred while freeRADIUS was attempting XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
			log_error($error);
			file_notice("sync_settings", $error, "freeradius Settings Sync", "");
		} elseif($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, "250");
			$error = "An error code was received while freeRADIUS XMLRPC was attempting to sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "freeradius Settings Sync", "");
		} else {
			log_error("freeRADIUS XMLRPC has reloaded data successfully on {$url}:{$port} (pfsense.exec_php).");
		}

}

// ##### The part above is based on the code of pfblocker #####

// This function restarts all other needed functions after XMLRPC so that the content of .XML + .INC will be written in the files (clients.conf, users)
// Adding more functions will increase the to sync
function freeradius_all_after_XMLRPC_resync() {
	
	freeradius_users_resync();
	freeradius_sync_on_changes();
	freeradius_clients_resync();
	
	log_error("freeRADIUS has finished XMLRPC process. It should be OK. For more information look at the host which started sync.");
	
	exec("/usr/local/etc/rc.d/radiusd onerestart");
}

function freeradius_modulescounter_resync() {
	global $config;
	$conf = '';

	$conf .= <<<EOD
# -*- text -*-
#
#  $Id$

#  counter module:
#  This module takes an attribute (count-attribute).
#  It also takes a key, and creates a counter for each unique
#  key.  The count is incremented when accounting packets are
#  received by the server.  The value of the increment depends
#  on the attribute type.
#  If the attribute is Acct-Session-Time or of an integer type we add
#  the value of the attribute. If it is anything else we increase the
#  counter by one.
#
#  The 'reset' parameter defines when the counters are all reset to
#  zero.  It can be hourly, daily, weekly, monthly or never.
#
#  hourly: Reset on 00:00 of every hour
#  daily: Reset on 00:00:00 every day
#  weekly: Reset on 00:00:00 on sunday
#  monthly: Reset on 00:00:00 of the first day of each month
#
#  It can also be user defined. It should be of the form:
#  num[hdwm] where:
#  h: hours, d: days, w: weeks, m: months
#  If the letter is ommited days will be assumed. In example:
#  reset = 10h (reset every 10 hours)
#  reset = 12  (reset every 12 days)
#
#
#  The check-name attribute defines an attribute which will be
#  registered by the counter module and can be used to set the
#  maximum allowed value for the counter after which the user
#  is rejected.
#  Something like:
#
#  DEFAULT Max-Daily-Session := 36000
#          Fall-Through = 1
#
#  You should add the counter module in the instantiate
#  section so that it registers check-name before the files
#  module reads the users file.
#
#  If check-name is set and the user is to be rejected then we
#  send back a Reply-Message and we log a Failure-Message in
#  the radius.log
#
#  If the count attribute is Acct-Session-Time then on each
#  login we send back the remaining online time as a
#  Session-Timeout attribute ELSE and if the reply-name is
#  set, we send back that attribute.  The reply-name attribute
#  MUST be of an integer type.
#
#  The counter-name can also be used instead of using the check-name
#  like below:
#
#  DEFAULT  Daily-Session-Time > 3600, Auth-Type = Reject
#      Reply-Message = "You've used up more than one hour today"
#
#  The allowed-servicetype attribute can be used to only take
#  into account specific sessions. For example if a user first
#  logs in through a login menu and then selects ppp there will
#  be two sessions. One for Login-User and one for Framed-User
#  service type. We only need to take into account the second one.
#
#  The module should be added in the instantiate, authorize and
#  accounting sections.  Make sure that in the authorize
#  section it comes after any module which sets the
#  'check-name' attribute.
#
counter daily {
	filename = \${raddbdir}/db.daily
	key = User-Name
	count-attribute = Acct-Session-Time
	reset = daily
	counter-name = Daily-Session-Time
	check-name = Max-Daily-Session
	reply-name = Session-Timeout
	cache-size = 5000
}

counter weekly {
	filename = \${raddbdir}/db.weekly
	key = User-Name
	count-attribute = Acct-Session-Time
	reset = weekly
	counter-name = Weekly-Session-Time
	check-name = Max-Weekly-Session
	reply-name = Session-Timeout
	cache-size = 5000
}

counter monthly {
	filename = \${raddbdir}/db.monthly
	key = User-Name
	count-attribute = Acct-Session-Time
	reset = monthly
	counter-name = Monthly-Session-Time
	check-name = Max-Monthly-Session
	reply-name = Session-Timeout
	cache-size = 5000
}

counter forever {
	filename = \${raddbdir}/db.forever
	key = User-Name
	count-attribute = Acct-Session-Time
	reset = never
	counter-name = Forever-Session-Time
	check-name = Max-Forever-Session
	reply-name = Session-Timeout
	cache-size = 5000
}

EOD;

	$filename = RADDB . '/modules/counter';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();

}

function freeradius_modulesmschap_resync() {
	global $config;
	$conf = '';

	$conf .= <<<EOD
# -*- text -*-
#
#  $Id$

# Microsoft CHAP authentication
#
#  This module supports MS-CHAP and MS-CHAPv2 authentication.
#  It also enforces the SMB-Account-Ctrl attribute.
#
mschap {
        #
        #  If you are using /etc/smbpasswd, see the 'passwd'
        #  module for an example of how to use /etc/smbpasswd

        # if use_mppe is not set to no mschap will
        # add MS-CHAP-MPPE-Keys for MS-CHAPv1 and
        # MS-MPPE-Recv-Key/MS-MPPE-Send-Key for MS-CHAPv2
        #
#       use_mppe = no

        # if mppe is enabled require_encryption makes
        # encryption moderate
        #
#       require_encryption = yes

        # require_strong always requires 128 bit key
        # encryption
        #
#       require_strong = yes

        # Windows sends us a username in the form of
        # DOMAIN\user, but sends the challenge response
        # based on only the user portion.  This hack
        # corrects for that incorrect behavior.
        #
		with_ntdomain_hack = yes

        # The module can perform authentication itself, OR
        # use a Windows Domain Controller.  This configuration
        # directive tells the module to call the ntlm_auth
        # program, which will do the authentication, and return
        # the NT-Key.  Note that you MUST have "winbindd" and
        # "nmbd" running on the local machine for ntlm_auth
        # to work.  See the ntlm_auth program documentation
        # for details.
        #
        # If ntlm_auth is configured below, then the mschap
        # module will call ntlm_auth for every MS-CHAP
        # authentication request.  If there is a cleartext
        # or NT hashed password available, you can set
        # "MS-CHAP-Use-NTLM-Auth := No" in the control items,
        # and the mschap module will do the authentication itself,
        # without calling ntlm_auth.
        #
        # Be VERY careful when editing the following line!
        #
        # You can also try setting the user name as:
        #
        #       ... --username=%{mschap:User-Name} ...
        #
        # In that case, the mschap module will look at the User-Name
        # attribute, and do prefix/suffix checks in order to obtain
        # the "best" user name for the request.
        #
#       ntlm_auth = "/path/to/ntlm_auth --request-nt-key --username=%{%{Stripped-User-Name}:-%{%{User-Name}:-None}} --challenge=%{%{mschap:Challenge}:-00} --
nt-response=%{%{mschap:NT-Response}:-00}"

        # For Apple Server, when running on the same machine as
        # Open Directory.  It has no effect on other systems.
        #
#       use_open_directory = yes

        # On failure, set (or not) the MS-CHAP error code saying
        # "retries allowed".
#       allow_retry = yes

        # An optional retry message.
#       retry_msg = "Re-enter (or reset) the password"
}

EOD;

	$filename = RADDB . '/modules/mschap';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();

}

function freeradius_modulesrealm_resync() {
	global $config;
	$conf = '';

	$conf .= <<<EOD
#  'realm/username'
#  Using this entry, IPASS users have their realm set to "IPASS".
realm IPASS {
		format = prefix
		delimiter = "/"
		ignore_null = yes
		ignore_default = no
}
#  'username@realm'
realm suffix {
		format = suffix
		delimiter = "@"
		ignore_null = yes
		ignore_default = no
}
#  'username%realm'
realm realmpercent {
		format = suffix
		delimiter = "%"
		ignore_null = yes
		ignore_default = no
}
#  'domain\user'
realm ntdomain {
		format = prefix
		### 3 backslash in .inc will be 2 backslash in file and after starting radiusd just only one
		delimiter = "\\\"
		ignore_null = yes
		ignore_default = no
}
EOD;

	$filename = RADDB . '/modules/realm';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();

}

function freeradius_modulesldap_resync() {
	global $config;
	$conf = '';
	
	$arrmodulesldap = $config['installedpackages']['freeradiusmodulesldap']['config'][0];
	
	// Enable and Disable LDAP for "authorize" and "authenticate" will be done in "freeradius_serverdefault_resync"
	
	
	// Variables for General Configuration
	$varmodulesldapserver = ($arrmodulesldap['varmodulesldapserver']?$arrmodulesldap['varmodulesldapserver']:'ldap.your.domain');
	$varmodulesldapidentity = ($arrmodulesldap['varmodulesldapidentity']?$arrmodulesldap['varmodulesldapidentity']:'cn=admin,o=My Org,c=UA');
	$varmodulesldappassword = ($arrmodulesldap['varmodulesldappassword']?$arrmodulesldap['varmodulesldappassword']:'mypass');
	$varmodulesldapbasedn = ($arrmodulesldap['varmodulesldapbasedn']?$arrmodulesldap['varmodulesldapbasedn']:'o=My Org,c=UA');
	$varmodulesldapfilter = ($arrmodulesldap['varmodulesldapfilter']?$arrmodulesldap['varmodulesldapfilter']:'(uid=%{%{Stripped-User-Name}:-%{User-Name}})');
	$varmodulesldapbasefilter = ($arrmodulesldap['varmodulesldapbasefilter']?$arrmodulesldap['varmodulesldapbasefilter']:'(objectclass=radiusprofile)');
	$varmodulesldapldapconnectionsnumber = ($arrmodulesldap['varmodulesldapldapconnectionsnumber']?$arrmodulesldap['varmodulesldapldapconnectionsnumber']:'5');
	$varmodulesldaptimeout = ($arrmodulesldap['varmodulesldaptimeout']?$arrmodulesldap['varmodulesldaptimeout']:'4');
	$varmodulesldaptimelimit = ($arrmodulesldap['varmodulesldaptimelimit']?$arrmodulesldap['varmodulesldaptimelimit']:'3');
	$varmodulesldapnettimeout = ($arrmodulesldap['varmodulesldapnettimeout']?$arrmodulesldap['varmodulesldapnettimeout']:'1');
	
	// Variables for TLS / Certificates - will be added later


	// Miscellaneous Configuration + MS Active Directory Compatibility
	$varmodulesldapmsadcompatibilityenable = ($arrmodulesldap['varmodulesldapmsadcompatibilityenable']?$arrmodulesldap['varmodulesldapmsadcompatibilityenable']:'Disable');
	if ($arrmodulesldap['varmodulesldapmsadcompatibilityenable'] == 'Disable') {
		$varmodulesldapmsadcompatibility = '### MS Active Directory Compatibility is disabled ###';
	}
	else {
		$varmodulesldapmsadcompatibility = 'chase_referrals = yes' . "\n\trebind = yes";
	}

	// When disabled we put this in the file but commented (#) like in the default installation
	if (!$arrmodulesldap['varmodulesldapdmiscenable']) {
		$varmodulesldapdefaultprofile = '### default_profile = "cn=radprofile,ou=dialup,o=My Org,c=UA" ###';
		$varmodulesldapprofileattribute = '### profile_attribute = "radiusProfileDn" ###';
		$varmodulesldapaccessattr = '### access_attr = "dialupAccess" ###';
	}
	// When enabled we put in the default values so there is no empty entry if there is not input from GUI
	else {
		$varmodulesldapdefaultprofile = ($arrmodulesldap['varmodulesldapdefaultprofile']?$arrmodulesldap['varmodulesldapdefaultprofile']:'cn=radprofile,ou=dialup,o=My Org,c=UA');
		$varmodulesldapdefaultprofile = "default_profile = " . '"' . "$varmodulesldapdefaultprofile" . '"';
		$varmodulesldapprofileattribute = ($arrmodulesldap['varmodulesldapprofileattribute']?$arrmodulesldap['varmodulesldapprofileattribute']:'radiusProfileDn');
		$varmodulesldapprofileattribute = "profile_attribute = " . '"' . "$varmodulesldapprofileattribute" . '"';
		$varmodulesldapaccessattr = ($arrmodulesldap['varmodulesldapaccessattr']?$arrmodulesldap['varmodulesldapaccessattr']:'dialupAccess');
		$varmodulesldapaccessattr = "access_attr = " . '"' . "$varmodulesldapaccessattr" . '"';
	}

	// Group membership checking
	// When disabled we put this in the file but commented (#) like in the default installation
	if (!$arrmodulesldap['varmodulesldapgroupenable']) {
		$varmodulesldapgroupnameattribute = '### groupname_attribute = cn ###';
		$varmodulesldapgroupmembershipfilter = '### groupmembership_filter = "(|(&(objectClass=GroupOfNames)(member=%{control:Ldap-UserDn}))(&(objectClass=GroupOfUniqueNames)(uniquemember=%{control:Ldap-UserDn})))" ###';
		$varmodulesldapgroupmembershipattribute = '### groupmembership_attribute = radiusGroupName ###';
		$varmodulesldapcomparecheckitems = '### compare_check_items = yes ###';
		$varmodulesldapdoxlat = '### do_xlat = yes ###';
		$varmodulesldapaccessattrusedforallow = '### access_attr_used_for_allow = yes ###';
	}

	// When enabled we put in the default values so there is no empty entry if there is not input from GUI
	else {
		$varmodulesldapgroupnameattribute = ($arrmodulesldap['varmodulesldapgroupnameattribute']?$arrmodulesldap['varmodulesldapgroupnameattribute']:'cn');
		$varmodulesldapgroupnameattribute = "groupname_attribute = $varmodulesldapgroupnameattribute";
		$varmodulesldapgroupmembershipfilter = ($arrmodulesldap['varmodulesldapgroupmembershipfilter']?$arrmodulesldap['varmodulesldapgroupmembershipfilter']:'(|(&(objectClass=GroupOfNames)(member=%{control:Ldap-UserDn}))(&(objectClass=GroupOfUniqueNames)(uniquemember=%{control:Ldap-UserDn})))');
		$varmodulesldapgroupmembershipfilter = "groupmembership_filter = " . '"' . "$varmodulesldapgroupmembershipfilter" . '"';
		$varmodulesldapgroupmembershipattribute = ($arrmodulesldap['varmodulesldapgroupmembershipattribute']?$arrmodulesldap['varmodulesldapgroupmembershipattribute']:'radiusGroupName');
		$varmodulesldapgroupmembershipattribute = "groupmembership_attribute = $varmodulesldapgroupmembershipattribute";
		
		$varmodulesldapcomparecheckitems = ($arrmodulesldap['varmodulesldapcomparecheckitems']?$arrmodulesldap['varmodulesldapcomparecheckitems']:'yes');
		$varmodulesldapcomparecheckitems = "compare_check_items = $varmodulesldapcomparecheckitems";
		$varmodulesldapdoxlat = ($arrmodulesldap['varmodulesldapdoxlat']?$arrmodulesldap['varmodulesldapdoxlat']:'yes');
		$varmodulesldapdoxlat = "do_xlat = $varmodulesldapdoxlat";
		$varmodulesldapaccessattrusedforallow = ($arrmodulesldap['varmodulesldapaccessattrusedforallow']?$arrmodulesldap['varmodulesldapaccessattrusedforallow']:'yes');	
		$varmodulesldapaccessattrusedforallow = "access_attr_used_for_allow = $varmodulesldapaccessattrusedforallow";
	}

	// Keepalive variables
	$varmodulesldapkeepaliveidle = ($arrmodulesldap['varmodulesldapkeepaliveidle']?$arrmodulesldap['varmodulesldapkeepaliveidle']:'60');
	$varmodulesldapkeepaliveprobes = ($arrmodulesldap['varmodulesldapkeepaliveprobes']?$arrmodulesldap['varmodulesldapkeepaliveprobes']:'3');
	$varmodulesldapkeepaliveinterval = ($arrmodulesldap['varmodulesldapkeepaliveinterval']?$arrmodulesldap['varmodulesldapkeepaliveinterval']:'3');

$conf .= <<<EOD

# -*- text -*-
#
#  $Id$

# Lightweight Directory Access Protocol (LDAP)
#
#  This module definition allows you to use LDAP for
#  authorization and authentication.
#
#  See raddb/sites-available/default for reference to the
#  ldap module in the authorize and authenticate sections.
#
#  However, LDAP can be used for authentication ONLY when the
#  Access-Request packet contains a clear-text User-Password
#  attribute.  LDAP authentication will NOT work for any other
#  authentication method.
#
#  This means that LDAP servers don't understand EAP.  If you
#  force "Auth-Type = LDAP", and then send the server a
#  request containing EAP authentication, then authentication
#  WILL NOT WORK.
#
#  The solution is to use the default configuration, which does
#  work.
#
#  Setting "Auth-Type = LDAP" is ALMOST ALWAYS WRONG.  We
#  really can't emphasize this enough.
#	
ldap {
	#
	#  Note that this needs to match the name in the LDAP
	#  server certificate, if you're using ldaps.
	server = "$varmodulesldapserver"
	identity = "$varmodulesldapidentity"
	password = $varmodulesldappassword
	basedn = "$varmodulesldapbasedn"
	filter = "$varmodulesldapfilter"
	base_filter = "$varmodulesldapbasefilter"

	#  How many connections to keep open to the LDAP server.
	#  This saves time over opening a new LDAP socket for
	#  every authentication request.
	ldap_connections_number = $varmodulesldapldapconnectionsnumber

	# seconds to wait for LDAP query to finish. default: 20
	timeout = $varmodulesldaptimeout

	#  seconds LDAP server has to process the query (server-side
	#  time limit). default: 20
	#
	#  LDAP_OPT_TIMELIMIT is set to this value.
	timelimit = $varmodulesldaptimelimit

	#
	#  seconds to wait for response of the server. (network
	#   failures) default: 10
	#
	#  LDAP_OPT_NETWORK_TIMEOUT is set to this value.
	net_timeout = $varmodulesldapnettimeout

	#
	#  This subsection configures the tls related items
	#  that control how FreeRADIUS connects to an LDAP
	#  server.  It contains all of the "tls_*" configuration
	#  entries used in older versions of FreeRADIUS.  Those
	#  configuration entries can still be used, but we recommend
	#  using these.
	#
	tls {
		# Set this to 'yes' to use TLS encrypted connections
		# to the LDAP database by using the StartTLS extended
		# operation.
		#			
		# The StartTLS operation is supposed to be
		# used with normal ldap connections instead of
		# using ldaps (port 689) connections
		start_tls = no

		# cacertfile	= /path/to/cacert.pem
		# cacertdir		= /path/to/ca/dir/
		# certfile		= /path/to/radius.crt
		# keyfile		= /path/to/radius.key
		# randfile		= /path/to/rnd

		#  Certificate Verification requirements.  Can be:
		#    "never" (don't even bother trying)
		#    "allow" (try, but don't fail if the cerificate
		#		can't be verified)
		#    "demand" (fail if the certificate doesn't verify.)
		#
		#	The default is "allow"
		# require_cert	= "demand"
	}

	$varmodulesldapdefaultprofile
	$varmodulesldapprofileattribute
	$varmodulesldapaccessattr

	# Mapping of RADIUS dictionary attributes to LDAP
	# directory attributes.
	dictionary_mapping = \${confdir}/ldap.attrmap
	################## THE BELOW IS NOT COMPILED WITH FREERADIUS #################################
	#  Set password_attribute = nspmPassword to get the
	#  user's password from a Novell eDirectory
	#  backend. This will work ONLY IF FreeRADIUS has been
	#  built with the --with-edir configure option.
	#
	#  See also the following links:
	#
	#  http://www.novell.com/coolsolutions/appnote/16745.html
	#  https://secure-support.novell.com/KanisaPlatform/Publishing/558/3009668_f.SAL_Public.html
	#
	#  Novell may require TLS encrypted sessions before returning
	#  the user's password.
	#
	# password_attribute = userPassword

	#  Un-comment the following to disable Novell
	#  eDirectory account policy check and intruder
	#  detection. This will work *only if* FreeRADIUS is
	#  configured to build with --with-edir option.
	#
	edir_account_policy_check = no
	################## THE ABOVE IS NOT COMPILED WITH FREERADIUS #################################
	#
	#  Group membership checking.  Disabled by default.
	#
	$varmodulesldapgroupnameattribute
	$varmodulesldapgroupmembershipfilter
	$varmodulesldapgroupmembershipattribute

	$varmodulesldapcomparecheckitems
	$varmodulesldapdoxlat
	$varmodulesldapaccessattrusedforallow

	#
	#  The following two configuration items are for Active Directory
	#  compatibility.  If you see the helpful "operations error"
	#  being returned to the LDAP module, uncomment the next
	#  two lines.
	#

	$varmodulesldapmsadcompatibility

	#
	#  By default, if the packet contains a User-Password,
	#  and no other module is configured to handle the
	#  authentication, the LDAP module sets itself to do
	#  LDAP bind for authentication.
	#
	#  THIS WILL ONLY WORK FOR PAP AUTHENTICATION.
	#
	#  THIS WILL NOT WORK FOR CHAP, MS-CHAP, or 802.1x (EAP). 
	#
	#  You can disable this behavior by setting the following
	#  configuration entry to "no".
	#
	#  allowed values: {no, yes}
	# set_auth_type = yes

	#  ldap_debug: debug flag for LDAP SDK
	#  (see OpenLDAP documentation).  Set this to enable
	#  huge amounts of LDAP debugging on the screen.
	#  You should only use this if you are an LDAP expert.
	#
	#	default: 0x0000 (no debugging messages)
	#	Example:(LDAP_DEBUG_FILTER+LDAP_DEBUG_CONNS)
	#ldap_debug = 0x0028 

	#
	#  Keepalive configuration.  This MAY NOT be supported by your
	#  LDAP library.  If these configuration entries appear in the
	#  output of "radiusd -X", then they are supported.  Otherwise,
	#  they are unsupported, and changing them will do nothing.
	#
	keepalive {
		# LDAP_OPT_X_KEEPALIVE_IDLE
		idle = $varmodulesldapkeepaliveidle

		# LDAP_OPT_X_KEEPALIVE_PROBES
		probes = $varmodulesldapkeepaliveprobes

		# LDAP_OPT_X_KEEPALIVE_INTERVAL
		interval = $varmodulesldapkeepaliveinterval
	}
}
EOD;

	$filename = RADDB . '/modules/ldap';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();
	
	// We need to rebuild "freeradius_serverdefault_resync" before restart service
	// "freeradius_serverdefault_resync" needs to restart other dependencies so we are pointing directly to "freeradius_settings_resync()"
	freeradius_settings_resync();
	
}

function freeradius_plainmacauth_resync() {
	global $config;
	$conf = '';
	
	// Variables: If not using 802.1x, mac address must be known
	$varsettings = $config['installedpackages']['freeradiussettings']['config'][0];
	
	// defining variables with filename path
	$filepolicyconf = '/usr/local/etc/raddb/policy.conf';
	$filepolicyconfbackup = '/usr/local/etc/raddb/policy.conf.backup';
	$filemodulesfiles = '/usr/local/etc/raddb/modules/files';
	$filemodulesfilesbackup = '/usr/local/etc/raddb/files.backup';
	
	// If unchecked then plain mac auth is disabled and backups of the original files will be restored
	if (!$varsettings['varsettingsenablemacauth']) {
		log_error("FreeRADIUS: Restoring the original file from {$filepolicyconfbackup} and {$filemodulesfilesbackup}");
		copy($filepolicyconfbackup, $filepolicyconf);
		copy($filemodulesfilesbackup, $filemodulesfiles);
	}
	// If checked then plain mac auth is enabled
	else {
		freeradius_modulesfiles_resync();
		freeradius_policyconf_resync();
	}
}

function freeradius_modulesfiles_resync() {
	global $config;
	$conf = '';

	$conf .= <<<EOD
# -*- text -*-
#
#  \$Id\$

# Livingston-style 'users' file
#
files {
	# The default key attribute to use for matches.  The content
	# of this attribute is used to match the "name" of the
	# entry.
	#key = "%{Stripped-User-Name:-%{User-Name}}"

	usersfile = \${confdir}/users
	acctusersfile = \${confdir}/acct_users
	preproxy_usersfile = \${confdir}/preproxy_users

	#  If you want to use the old Cistron 'users' file
	#  with FreeRADIUS, you should change the next line
	#  to 'compat = cistron'.  You can the copy your 'users'
	#  file from Cistron.
	compat = no
}

#  An example which defines a second instance of the "files" module.
#  This instance is named "second_files".  In order for it to be used
#  in a virtual server, it needs to be listed as "second_files"
#  inside of the "authorize" section (or other section).  If you just
#  list "files", that will refer to the configuration defined above.
#

#  The two names here mean:
#	"files" - this is a configuration for the "rlm_files" module
#	"second_files" - this is a named configuration, which isn't
#			the default configuration.
files authorized_macs {
        # The default key attribute to use for matches.  The content
        # of this attribute is used to match the "name" of the
        # entry.
        key = "%{Calling-Station-ID}"

        usersfile = \${confdir}/authorized_macs

        #  If you want to use the old Cistron 'users' file
        #  with FreeRADIUS, you should change the next line
        #  to 'compat = cistron'.  You can the copy your 'users'
        #  file from Cistron.
        compat = no
}
EOD;

	$filename = RADDB . '/modules/files';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();

}

function freeradius_policyconf_resync() {
	global $config;
	$conf = '';

	$conf .= <<<EOD
# -*- text -*-
##
## policy.conf	-- FreeRADIUS server configuration file.
##
##	http://www.freeradius.org/
##	\$Id\$
##

#
#  Policies are virtual modules, similar to those defined in the
#  "instantate" section of radiusd.conf.
#
#  Defining a policy here means that it can be referenced in multiple
#  places as a *name*, rather than as a series of conditions to match,
#  and actions to take.
#
#  Policies are something like subroutines in a normal language, but
#  they cannot be called recursively.  They MUST be defined in order.
#  If policy A calls policy B, then B MUST be defined before A.
#
policy {
	#
	#	Forbid all EAP types.
	#
	forbid_eap {
		if (EAP-Message) {
			reject
		}
	}

	#
	#	Forbid all non-EAP types outside of an EAP tunnel.
	#
	permit_only_eap {
		if (!EAP-Message) {
			#  We MAY be inside of a TTLS tunnel.
			#  PEAP and EAP-FAST require EAP inside of
			#  the tunnel, so this check is OK.
			#  If so, then there MUST be an outer EAP message.
			if (!"%{outer.request:EAP-Message}") {
				reject
			}
		}
	}

	#
	#	Forbid all attempts to login via realms.
	#
	deny_realms {
		if (User-Name =~ /@|\\\\/) {
			reject
		}
	}

	#
	#	If you want the server to pretend that it is dead,
	#	then use the "do_not_respond" policy.
	#
	do_not_respond {
		update control {
			Response-Packet-Type := Do-Not-Respond
		}

		handled
	}

	#
	#  Force some sanity on User-Name.  This helps to avoid issues
	#  issues where the back-end database is "forgiving" about
	#  what constitutes a user name.
	#
	filter_username {
		# spaces at the start: reject
		if (User-Name =~ /^ /) {
			reject
		}

		# spaces at the end: reject
		if (User-Name =~ / \$\$/) {
			reject
		}

		# Mixed case: reject
		if (User-Name != "%{tolower:%{User-Name}}") {
			reject
		}
	}


	#	
	#  The following policies are for the Chargeable-User-Identity
	#  (CUI) configuration.
	#

	#
	#  The client indicates it can do CUI by sending a CUI attribute	
	#  containing one zero byte
	#
	cui_authorize {
		update request {
			Chargeable-User-Identity:='\\\\000'
		}
	}

	#
	#  Add a CUI attribute based on the User-Name, and a secret key
	#  known only to this server.
	#
	cui_postauth {
		if (FreeRadius-Proxied-To == 127.0.0.1) {
			if (outer.request:Chargeable-User-Identity) {
				update outer.reply {
					Chargeable-User-Identity:="%{md5:%{config:cui_hash_key}%{User-Name}}"
				}
			}
		}
		else {
			if (Chargeable-User-Identity) {
				update reply {
					Chargeable-User-Identity="%{md5:%{config:cui_hash_key}%{User-Name}}"
				}
			}
		}
	}

	#
	#  If there is a CUI attribute in the reply, add it to the DB.
	#
	cui_updatedb {
		if (reply:Chargeable-User-Identity) {
			cui
		}
	}

	#
	#  If we had stored a CUI for the User, add it to the request.
	#
	cui_accounting {
		#
		#  If the CUI isn't in the packet, see if we can find it
		#  in the DB.
		#
		if (!Chargeable-User-Identity) {
			update control {
				Chargable-User-Identity := "%{cui: SELECT cui FROM cui WHERE clientipaddress = '%{Client-IP-Address}' AND callingstationid = '%{Calling-Station-Id}' AND username = '%{User-Name}'}"
			}
		}

		#
		#  If it exists now, then write out when we last saw
		#  this CUI.
		#
		if (Chargeable-User-Identity && (Chargeable-User-Identity != "")) {
			cui
		}
	}

	#
	#  Normalize the MAC Addresses in the Calling/Called-Station-Id
	#
	mac-addr = ([0-9a-f]{2})[^0-9a-f]?([0-9a-f]{2})[^0-9a-f]?([0-9a-f]{2})[^0-9a-f]?([0-9a-f]{2})[^0-9a-f]?([0-9a-f]{2})[^0-9a-f]?([0-9a-f]{2})

	#  Add "rewrite.called_station_id" in the "authorize" and "preacct"
	#  sections.
	rewrite.called_station_id {
		if((Called-Station-Id) && "%{Called-Station-Id}" =~ /^%{config:policy.mac-addr}(:(.+))?\$/i) {
			update request {
				Called-Station-Id := "%{tolower:%{1}-%{2}-%{3}-%{4}-%{5}-%{6}}"
			}

			# SSID component?
			if ("%{8}") {
				update request {
					Called-Station-Id := "%{Called-Station-Id}:%{8}"
				}
			}
			updated
		}
		else {
			noop
		}
	}

	#  Add "rewrite.calling_station_id" in the "authorize" and "preacct"
	#  sections.
	#rewrite.calling_station_id {
	#	if((Calling-Station-Id) && "%{Calling-Station-Id}" =~ /^%{config:policy.mac-addr}\$/i) {
	#		update request {
	#			Calling-Station-Id := "%{tolower:%{1}-%{2}-%{3}-%{4}-%{5}-%{6}}"
	#		}
	#		updated
	#	}
	#	else {
	#		noop
	#	}
	#}
	
	#####  MODIFIED FOR http://wiki.freeradius.org/Mac-Auth#Mac-Auth+or+802.1x #####
	#  Add "rewrite_calling_station_id" in the "authorize" and "preacct"
	#  sections.
	rewrite_calling_station_id {
        if (Calling-Station-Id =~ /([0-9a-f]{2})[-:]?([0-9a-f]{2})[-:]?([0-9a-f]{2})[-:]?([0-9a-f]{2})[-:]?([0-9a-f]{2})[-:]?([0-9a-f]{2})/i){
                update request {
                        Calling-Station-Id := "%{tolower:%{1}-%{2}-%{3}-%{4}-%{5}-%{6}}"
                }
        }
        else {
            noop
		}
	}
}
	
EOD;

	$filename = RADDB . '/policy.conf';
	conf_mount_rw();
	file_put_contents($filename, $conf);
	chmod($filename, 0600);
	conf_mount_ro();

}

?>