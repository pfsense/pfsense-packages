<?php
/* ========================================================================== */
/*
	asterisk.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2012-2013 Marcello Coutinho
    Copyright (C) 2012-2013 robi <robreg@zsurob.hu>
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

define('ASTERISK_CONF_DIR', '/conf/asterisk');
//Check pfsense version
$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version > 2.0)
	define('ASTERISK_LOCALBASE', '/usr/pbi/asterisk-' . php_uname("m"));
else
	define('ASTERISK_LOCALBASE','/usr/local');

function asterisk_install() {
	sync_package_asterisk();
}

function asterisk_deinstall() {
	$script='/usr/local/etc/rc.d/asterisk';
	if (file_exists($script)){
		mwexec("$script stop");
		chmod ($script,0444);
		}
}

function sync_package_asterisk() {
	global $config, $g;

	#mount filesystem writeable
	conf_mount_rw();
	
  	//for NanoBSD compatibility, move the /etc/asterisk configuration directory to /conf, and symlink it back
  	$dist_dir=ASTERISK_CONF_DIR ."/dist";
  	if (!is_dir($dist_dir))
  		mkdir($dist_dir,0755,TRUE);

  	if(file_exists (ASTERISK_LOCALBASE."/etc/asterisk") && !is_link(ASTERISK_LOCALBASE."/etc/asterisk")){
  		$dist_files= scandir(ASTERISK_LOCALBASE."/etc/asterisk");
  		foreach ($dist_files as $dist){
  			if (preg_match("/-dist/",$dist))
  				rename (ASTERISK_LOCALBASE."/etc/asterisk"."/$dist", "$dist_dir/$dist");
  			elseif (preg_match("/\w+/",$dist))
  				rename (ASTERISK_LOCALBASE."/etc/asterisk"."/$dist", ASTERISK_CONF_DIR."/$dist");
  			}
  		rmdir(ASTERISK_LOCALBASE. "/etc/asterisk");
  		symlink (ASTERISK_CONF_DIR , ASTERISK_LOCALBASE. "/etc/asterisk");
  		}
	
	//fix asterisk options for nanobsd: logging, db and calls log in /tmp
	//	if ($g['platform'] == "nanobsd"){
		$script='/conf/asterisk/logger.conf';
		if (file_exists($script)){
			$script_file=file_get_contents($script);
			$pattern[0]='@;rotatestrategy@';
			$replace[0]='rotatestrategy = rotate ;by pfSense ;';
			$script_file=preg_replace($pattern,$replace,$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
		}
		
		$script='/conf/asterisk/asterisk.conf';
		if (file_exists($script)){
		//point to the /var subdirs in the writable area in RAM
			$script_file=file_get_contents($script);
			$pattern[0]='/(\Wdirectories\W)\S+/';
			$replace[0]='$1';
			$pattern[1]='/astetcdir => \S+/';
			$replace[1]='astetcdir => /conf/asterisk';
			$pattern[2]='/astdbdir => \S+/';
			$replace[2]='astdbdir => /var/db/asterisk';
			$pattern[3]='/astspooldir => \S+/';
			$replace[3]='astspooldir => /var/spool/asterisk';
			$pattern[4]='/astrundir => \S+/';
			$replace[4]='astrundir => /var/run/asterisk';
			$pattern[5]='/astlogdir => \S+/';
			$replace[5]='astlogdir => /var/log/asterisk';
			$script_file=preg_replace($pattern,$replace,$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
		}
//	}

	//add modules settings, for disabling those not required on pfSense	
        $script='/conf/asterisk/modules.conf';
		if (file_exists($script)){
			$script_file=file_get_contents($script);
			if (strpos($script_file,'pfSense') === false) {  //first check if already added...
				$add_modules_settings=<<<EOF
;The following modules settings work out of the box on pfSense (note: do not remove this comment line).
;Should you need any disabled module, check for its functionality individually in your environment.
;For more information check asterisk's online documentation.
noload => res_ael_share.so
noload => res_adsi.so
;noload => res_agi.so
noload => res_calendar.so
noload => res_crypto.so
;noload => res_fax.so
noload => res_jabber.so
noload => res_monitor.so
;noload => res_stun_monitor.so
noload => res_smdi.so
noload => res_speech.so
noload => res_odbc.so
noload => res_musiconhold.so
noload => app_celgenuserevent.so
;noload => app_confbridge.so
;noload => app_minivm.so
;noload => app_originate.so
;noload => app_playtones.so
;noload => app_readexten.so
;noload => app_waituntil.so
;noload => bridge_builtin_features.so
;noload => bridge_multiplexed.so
;noload => bridge_simple.so
;noload => bridge_softmix.so
noload => cdr_adaptive_odbc.so
noload => chan_jingle.so
;noload => chan_bridge.so
noload => chan_unistim.so
;noload => codec_g722.so
;noload => format_g719.so
noload => format_sln16.so
noload => format_siren14.so
noload => format_siren7.so
;noload => func_aes.so
;noload => func_audiohookinherit.so
;noload => func_blacklist.so
;noload => func_config.so
;noload => func_devstate.so
;noload => func_dialgroup.so
;noload => func_dialplan.so
;noload => func_extstate.so
;noload => func_iconv.so
;noload => func_lock.so
;noload => func_module.so
;noload => func_shell.so
;noload => func_speex.so
;noload => func_sprintf.so
;noload => func_sysinfo.so
;noload => func_version.so
;noload => res_curl.so
noload => func_vmcount.so
noload => func_volume.so
noload => res_clialiases.so
noload => res_config_curl.so
noload => res_config_ldap.so
noload => res_config_sqlite.so
;noload => res_limit.so
;noload => res_phoneprov.so
noload => res_realtime.so
noload => res_timing_pthread.so
;noload => app_adsiprog.so
;noload => app_alarmreceiver.so
;noload => app_amd.so
;noload => app_authenticate.so
;noload => app_cdr.so
;noload => app_chanisavail.so
;noload => app_channelredirect.so
;noload => app_chanspy.so
;noload => app_controlplayback.so
noload => app_db.so
;noload => app_dial.so
;noload => app_dictate.so
;noload => app_directed_pickup.so
;noload => app_directory.so
;noload => app_disa.so
;noload => app_dumpchan.so
;noload => app_echo.so
;noload => app_exec.so
;noload => app_externalivr.so
;noload => app_festival.so
;noload => app_followme.so
;noload => app_forkcdr.so
;noload => app_getcpeid.so
;noload => app_ices.so
;noload => app_image.so
;noload => app_macro.so
;noload => app_milliwatt.so
;noload => app_mixmonitor.so
;noload => app_mp3.so
;noload => app_morsecode.so
;noload => app_nbscat.so
;noload => app_parkandannounce.so
;noload => app_playback.so
;noload => app_privacy.so
;noload => app_queue.so
;noload => app_read.so
;noload => app_readfile.so
;noload => app_record.so
;noload => app_sayunixtime.so
;noload => app_senddtmf.so
;noload => app_sendtext.so
;noload => app_setcallerid.so
;noload => app_sms.so
;noload => app_softhangup.so
noload => app_speech_utils.so
;noload => app_stack.so
;noload => app_system.so
;noload => app_talkdetect.so
;noload => app_test.so
;noload => app_transfer.so
;noload => app_url.so
;noload => app_userevent.so
;noload => app_verbose.so
;noload => app_voicemail.so
;noload => app_waitforring.so
;noload => app_waitforsilence.so
;noload => app_while.so
;noload => app_zapateller.so
;noload => cdr_csv.so
noload => cdr_custom.so
;noload => cdr_manager.so
noload => cdr_pgsql.so
noload => cdr_radius.so
noload => cdr_sqlite.so
noload => cdr_sqlite3_custom.so
noload => cdr_syslog.so
;noload => cel_custom.so
;noload => cel_manager.so
noload => cel_odbc.so
noload => cel_pgsql.so
noload => cel_radius.so
noload => cel_sqlite3_custom.so
noload => cel_tds.so
;noload => chan_agent.so
noload => chan_dahdi.so
noload => chan_gtalk.so
noload => chan_iax2.so
;noload => chan_local.so
noload => chan_mgcp.so
;noload => chan_multicast_rtp.so
noload => chan_oss.so
;noload => chan_sip.so
noload => chan_skinny.so
;noload => codec_a_mu.so
;noload => codec_adpcm.so
;noload => codec_alaw.so
noload => codec_dahdi.so
;noload => codec_g726.so
;noload => codec_gsm.so
;noload => codec_lpc10.so
;noload => codec_speex.so
;noload => codec_ulaw.so
;noload => format_g723.so
;noload => format_g726.so
;noload => format_g729.so
;noload => format_gsm.so
;noload => format_h263.so
;noload => format_h264.so
;noload => format_ilbc.so
noload => format_jpeg.so
;noload => format_ogg_vorbis.so
;noload => format_pcm.so
;noload => format_sln.so
;noload => format_vox.so
;noload => format_wav.so
;noload => format_wav_gsm.so
;noload => func_base64.so
;noload => func_callcompletion.so
;noload => func_callerid.so
;noload => func_cdr.so
;noload => func_channel.so
;noload => func_curl.so
;noload => func_cut.so
noload => func_db.so
;noload => func_enum.so
;noload => func_env.so
;noload => func_frame_trace.so
;noload => func_global.so
;noload => func_groupcount.so
;noload => func_logic.so
;noload => func_math.so
;noload => func_md5.so
noload => func_odbc.so
;noload => func_pitchshift.so
;noload => func_rand.so
;noload => func_realtime.so
;noload => func_sha1.so
;noload => func_srv.so
;noload => func_strings.so
;noload => func_timeout.so
;noload => func_uri.so
noload => pbx_ael.so
;noload => pbx_config.so
noload => pbx_dundi.so
;noload => pbx_loopback.so
;noload => pbx_realtime.so
;noload => pbx_spool.so
;noload => res_clioriginate.so
noload => res_config_pgsql.so
;noload => res_convert.so
;noload => res_mutestream.so
;noload => res_rtp_asterisk.so
;noload => res_rtp_multicast.so
;noload => res_security_log.so
;noload => res_snmp.so
noload => cdr_odbc.so
noload => cdr_tds.so
noload => chan_h323.so
noload => res_config_odbc.so

EOF;
				$script_file .= $add_modules_settings;
				file_put_contents($script, $script_file, LOCK_EX);
			}
		}

	//replace general SIP settings as a newbie hint to start configuration
		$script='/conf/asterisk/sip.conf';
		if (file_exists($script)){
			$script_file=file_get_contents($script);
			//strenghten a couple of security settings, and predefine codecs in the default SIP configuration
			if (strpos($script_file,'pfSense') === false) {  //first check if already added...
				$pattern[0]='/;allowguest\S+/';
				$replace[0]='allowguest=no ;by pfSense ;';
				$pattern[1]='/;alwaysauthreject/';
				$replace[1]='alwaysauthreject=yes ;by pfSense ;';
				$pattern[2]='/; jbenable/';
				$replace[2]='jbenable=yes ;by pfSense ;';
				$pattern[3]='/(First disallow all codecs)/';
				$replace[3]="$1\n;The following general settings usually work on pfSense boxes (note: please do not remove this comment line).\ndisallow=all ;by pfSense\nallow=g729\nallow=gsm\nallow=ulaw\nallow=alaw\n\n";
				$script_file=preg_replace($pattern,$replace,$script_file);
				file_put_contents($script, $script_file, LOCK_EX);
			}
	
			if (strpos($script_file,'demo extension for pfSense') === false) {  //first check if already added...
				$add_demo_extension = <<<EOF
				
[301]
;demo extension for pfSense
type=friend
defaultuser=301
insecure=port,invite
secret=1234
regexten=301
host=dynamic
context=default

[302]
;demo extension for pfSense
type=friend
defaultuser=302
insecure=port,invite
secret=1234
regexten=302
host=dynamic
context=default

EOF;
				$script_file .= $add_demo_extension;
				file_put_contents($script, $script_file, LOCK_EX);
			}
		}

	$script='/usr/local/etc/rc.d/asterisk';
	if (file_exists($script)){
		$script_file=file_get_contents($script);
		if (preg_match('/NO/',$script_file)){
			$script_file=preg_replace("/NO/","YES",$script_file);
			$script_file=preg_replace("/core stop now'/","core stop now'\n  killall \$name",$script_file);
			
			//create the /var subdirs for the writable area in RAM
			if (strpos($script_file,'pfSense') === false) {  //first check if already added...
				$writable_area = <<< EOF
# Create paths in pfSense's volatile area if they don't exist (note: please do not remove this comment line)
for a in db spool run log 
do
if [ ! -d /var/\$a/asterisk ]; then
  mkdir -p /var/\$a/asterisk
  chown -R asterisk:asterisk /var/\$a/asterisk
  chmod -R g+w /var/\$a/asterisk
fi
done

if [ ! -d /var/log/asterisk/cdr-csv ]; then
  mkdir -p /var/log/asterisk/cdr-csv
  chown -R asterisk:asterisk /var/log/asterisk/cdr-csv
  chmod -R g+w /var/log/asterisk/cdr-csv
fi

EOF;
				$script_file .= $writable_area;
			}
			file_put_contents($script, $script_file, LOCK_EX);
		}
		chmod ($script,0755);
		mwexec("$script stop");			
		mwexec_bg("$script start");
	}
	
	//prepare backup for factory defaults restoring feature
	if (!file_exists("/conf.default/asterisk_factory_defaults_config.tgz")) {
		system("cd /conf/asterisk/ && tar czf /conf.default/asterisk_factory_defaults_config.tgz *");
	}
	
	//mount filesystem readonly
	conf_mount_ro();
}

?>
