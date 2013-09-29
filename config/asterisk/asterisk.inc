<?php
/* ========================================================================== */
/*
	asterisk.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2012 Marcello Coutinho
    Copyright (C) 2013 robi <robreg@zsurob.hu>
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
	if (file_exists("/usr/pbi/asterisk-i386/etc/asterisk/")) {
	//this should occur only on i386 systems v2.1 and up
		system("mv -f /usr/pbi/asterisk-i386/etc/asterisk/ /conf/asterisk/ && ln -s /conf/asterisk /usr/pbi/asterisk-i386/etc/asterisk");
	}
	if (file_exists("/usr/pbi/asterisk-amd64/etc/asterisk/")) {
	//this should occur only on amd64 systems v2.1 and up
		system("mv -f /usr/pbi/asterisk-amd64/etc/asterisk/ /conf/asterisk/ && ln -s /conf/asterisk /usr/pbi/asterisk-amd64/etc/asterisk");
	}
	if (file_exists("/conf/asterisk/")) {
		if (file_exists("/usr/local/etc/asterisk/")) {
			system("mv -f /usr/local/etc/asterisk /usr/local/etc/asterisk.bak");
		}
		system("ln -s /conf/asterisk /usr/local/etc/asterisk");
		system("cd /conf/asterisk && mkdir dist && mv *-dist dist");
	} else {
	//should reach here only on non-pbi installs (2.0.x)
		system("mv -f /usr/local/etc/asterisk/ /conf/asterisk/ && ln -s /conf/asterisk /usr/local/etc/asterisk");
	}
		$script='/conf/asterisk/logger.conf';
		if (file_exists($script)){
			$script_file=file_get_contents($script);
			$pattern[0]='@\\;rotatestrategy@';
			$replace[0]='rotatestrategy = rotate ;by pfSense ;';
			$script_file=preg_replace($pattern,$replace,$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
		}
		
		$script='/conf/asterisk/asterisk.conf';
		if (file_exists($script)){
		//point to the /var subdirs in the writable area in RAM
			$script_file=file_get_contents($script);
			$pattern[0]='@\\[directories\\]\\(!\\)@';
			$replace[0]='[directories]';
			$pattern[1]='@astetcdir => [A-Z,a-z,0-9,\\_,\\-,/]+@';
			$replace[1]='astetcdir => /conf/asterisk';
			$pattern[2]='@astdbdir => [A-Z,a-z,0-9,\\_,\\-,/]+@';
			$replace[2]='astdbdir => /var/db/asterisk';
			$pattern[3]='@astspooldir => [A-Z,a-z,0-9,\\_,\\-,/]+@';
			$replace[3]='astspooldir => /var/spool/asterisk';
			$pattern[4]='@astrundir => [A-Z,a-z,0-9,\\_,\\-,/]+@';
			$replace[4]='astrundir => /var/run/asterisk';
			$pattern[5]='@astlogdir => [A-Z,a-z,0-9,\\_,\\-,/]+@';
			$replace[5]='astlogdir => /var/log/asterisk';
			$script_file=preg_replace($pattern,$replace,$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
		}

	//add modules settings, for disabling those not required on pfSense	
        $script='/conf/asterisk/modules.conf';
		if (file_exists($script)){
			$script_file=file_get_contents($script);
			if (strpos($script_file,'pfSense') === false) {  //first check if already added...
				$add_modules_settings = "\n";
				$add_modules_settings .= ";The following modules settings work out of the box on pfSense (note: do not remove this comment line).\n";
				$add_modules_settings .= ";Should you need any disabled module, check for its functionality individually in your environment.\n";
				$add_modules_settings .= ";For more information check asterisk's online documentation.\n";
				$add_modules_settings .= "noload => res_ael_share.so\n";
				$add_modules_settings .= "noload => res_adsi.so\n";
				$add_modules_settings .= ";noload => res_agi.so\n";
				$add_modules_settings .= "noload => res_calendar.so\n";
				$add_modules_settings .= "noload => res_crypto.so\n";
				$add_modules_settings .= ";noload => res_fax.so\n";
				$add_modules_settings .= "noload => res_jabber.so\n";
				$add_modules_settings .= "noload => res_monitor.so\n";
				$add_modules_settings .= ";noload => res_stun_monitor.so\n";
				$add_modules_settings .= "noload => res_smdi.so\n";
				$add_modules_settings .= "noload => res_speech.so\n";
				$add_modules_settings .= "noload => res_odbc.so\n";
				$add_modules_settings .= "noload => res_musiconhold.so\n";
				$add_modules_settings .= "noload => app_celgenuserevent.so\n";
				$add_modules_settings .= ";noload => app_confbridge.so\n";
				$add_modules_settings .= ";noload => app_minivm.so\n";
				$add_modules_settings .= ";noload => app_originate.so\n";
				$add_modules_settings .= ";noload => app_playtones.so\n";
				$add_modules_settings .= ";noload => app_readexten.so\n";
				$add_modules_settings .= ";noload => app_waituntil.so\n";
				$add_modules_settings .= ";noload => bridge_builtin_features.so\n";
				$add_modules_settings .= ";noload => bridge_multiplexed.so\n";
				$add_modules_settings .= ";noload => bridge_simple.so\n";
				$add_modules_settings .= ";noload => bridge_softmix.so\n";
				$add_modules_settings .= "noload => cdr_adaptive_odbc.so\n";
				$add_modules_settings .= "noload => chan_jingle.so\n";
				$add_modules_settings .= ";noload => chan_bridge.so\n";
				$add_modules_settings .= "noload => chan_unistim.so\n";
				$add_modules_settings .= ";noload => codec_g722.so\n";
				$add_modules_settings .= ";noload => format_g719.so\n";
				$add_modules_settings .= "noload => format_sln16.so\n";
				$add_modules_settings .= "noload => format_siren14.so\n";
				$add_modules_settings .= "noload => format_siren7.so\n";
				$add_modules_settings .= ";noload => func_aes.so\n";
				$add_modules_settings .= ";noload => func_audiohookinherit.so\n";
				$add_modules_settings .= ";noload => func_blacklist.so\n";
				$add_modules_settings .= ";noload => func_config.so\n";
				$add_modules_settings .= ";noload => func_devstate.so\n";
				$add_modules_settings .= ";noload => func_dialgroup.so\n";
				$add_modules_settings .= ";noload => func_dialplan.so\n";
				$add_modules_settings .= ";noload => func_extstate.so\n";
				$add_modules_settings .= ";noload => func_iconv.so\n";
				$add_modules_settings .= ";noload => func_lock.so\n";
				$add_modules_settings .= ";noload => func_module.so\n";
				$add_modules_settings .= ";noload => func_shell.so\n";
				$add_modules_settings .= ";noload => func_speex.so\n";
				$add_modules_settings .= ";noload => func_sprintf.so\n";
				$add_modules_settings .= ";noload => func_sysinfo.so\n";
				$add_modules_settings .= ";noload => func_version.so\n";
				$add_modules_settings .= ";noload => res_curl.so\n";
				$add_modules_settings .= "noload => func_vmcount.so\n";
				$add_modules_settings .= "noload => func_volume.so\n";
				$add_modules_settings .= "noload => res_clialiases.so\n";
				$add_modules_settings .= "noload => res_config_curl.so\n";
				$add_modules_settings .= "noload => res_config_ldap.so\n";
				$add_modules_settings .= "noload => res_config_sqlite.so\n";
				$add_modules_settings .= ";noload => res_limit.so\n";
				$add_modules_settings .= ";noload => res_phoneprov.so\n";
				$add_modules_settings .= "noload => res_realtime.so\n";
				$add_modules_settings .= "noload => res_timing_pthread.so\n";
				$add_modules_settings .= ";noload => app_adsiprog.so\n";
				$add_modules_settings .= ";noload => app_alarmreceiver.so\n";
				$add_modules_settings .= ";noload => app_amd.so\n";
				$add_modules_settings .= ";noload => app_authenticate.so\n";
				$add_modules_settings .= ";noload => app_cdr.so\n";
				$add_modules_settings .= ";noload => app_chanisavail.so\n";
				$add_modules_settings .= ";noload => app_channelredirect.so\n";
				$add_modules_settings .= ";noload => app_chanspy.so\n";
				$add_modules_settings .= ";noload => app_controlplayback.so\n";
				$add_modules_settings .= "noload => app_db.so\n";
				$add_modules_settings .= ";noload => app_dial.so\n";
				$add_modules_settings .= ";noload => app_dictate.so\n";
				$add_modules_settings .= ";noload => app_directed_pickup.so\n";
				$add_modules_settings .= ";noload => app_directory.so\n";
				$add_modules_settings .= ";noload => app_disa.so\n";
				$add_modules_settings .= ";noload => app_dumpchan.so\n";
				$add_modules_settings .= ";noload => app_echo.so\n";
				$add_modules_settings .= ";noload => app_exec.so\n";
				$add_modules_settings .= ";noload => app_externalivr.so\n";
				$add_modules_settings .= ";noload => app_festival.so\n";
				$add_modules_settings .= ";noload => app_followme.so\n";
				$add_modules_settings .= ";noload => app_forkcdr.so\n";
				$add_modules_settings .= ";noload => app_getcpeid.so\n";
				$add_modules_settings .= ";noload => app_ices.so\n";
				$add_modules_settings .= ";noload => app_image.so\n";
				$add_modules_settings .= ";noload => app_macro.so\n";
				$add_modules_settings .= ";noload => app_milliwatt.so\n";
				$add_modules_settings .= ";noload => app_mixmonitor.so\n";
				$add_modules_settings .= ";noload => app_mp3.so\n";
				$add_modules_settings .= ";noload => app_morsecode.so\n";
				$add_modules_settings .= ";noload => app_nbscat.so\n";
				$add_modules_settings .= ";noload => app_parkandannounce.so\n";
				$add_modules_settings .= ";noload => app_playback.so\n";
				$add_modules_settings .= ";noload => app_privacy.so\n";
				$add_modules_settings .= ";noload => app_queue.so\n";
				$add_modules_settings .= ";noload => app_read.so\n";
				$add_modules_settings .= ";noload => app_readfile.so\n";
				$add_modules_settings .= ";noload => app_record.so\n";
				$add_modules_settings .= ";noload => app_sayunixtime.so\n";
				$add_modules_settings .= ";noload => app_senddtmf.so\n";
				$add_modules_settings .= ";noload => app_sendtext.so\n";
				$add_modules_settings .= ";noload => app_setcallerid.so\n";
				$add_modules_settings .= ";noload => app_sms.so\n";
				$add_modules_settings .= ";noload => app_softhangup.so\n";
				$add_modules_settings .= "noload => app_speech_utils.so\n";
				$add_modules_settings .= ";noload => app_stack.so\n";
				$add_modules_settings .= ";noload => app_system.so\n";
				$add_modules_settings .= ";noload => app_talkdetect.so\n";
				$add_modules_settings .= ";noload => app_test.so\n";
				$add_modules_settings .= ";noload => app_transfer.so\n";
				$add_modules_settings .= ";noload => app_url.so\n";
				$add_modules_settings .= ";noload => app_userevent.so\n";
				$add_modules_settings .= ";noload => app_verbose.so\n";
				$add_modules_settings .= ";noload => app_voicemail.so\n";
				$add_modules_settings .= ";noload => app_waitforring.so\n";
				$add_modules_settings .= ";noload => app_waitforsilence.so\n";
				$add_modules_settings .= ";noload => app_while.so\n";
				$add_modules_settings .= ";noload => app_zapateller.so\n";
				$add_modules_settings .= ";noload => cdr_csv.so\n";
				$add_modules_settings .= "noload => cdr_custom.so\n";
				$add_modules_settings .= ";noload => cdr_manager.so\n";
				$add_modules_settings .= "noload => cdr_pgsql.so\n";
				$add_modules_settings .= "noload => cdr_radius.so\n";
				$add_modules_settings .= "noload => cdr_sqlite.so\n";
				$add_modules_settings .= "noload => cdr_sqlite3_custom.so\n";
				$add_modules_settings .= "noload => cdr_syslog.so\n";
				$add_modules_settings .= ";noload => cel_custom.so\n";
				$add_modules_settings .= ";noload => cel_manager.so\n";
				$add_modules_settings .= "noload => cel_odbc.so\n";
				$add_modules_settings .= "noload => cel_pgsql.so\n";
				$add_modules_settings .= "noload => cel_radius.so\n";
				$add_modules_settings .= "noload => cel_sqlite3_custom.so\n";
				$add_modules_settings .= "noload => cel_tds.so\n";
				$add_modules_settings .= ";noload => chan_agent.so\n";
				$add_modules_settings .= "noload => chan_dahdi.so\n";
				$add_modules_settings .= "noload => chan_gtalk.so\n";
				$add_modules_settings .= "noload => chan_iax2.so\n";
				$add_modules_settings .= ";noload => chan_local.so\n";
				$add_modules_settings .= "noload => chan_mgcp.so\n";
				$add_modules_settings .= ";noload => chan_multicast_rtp.so\n";
				$add_modules_settings .= "noload => chan_oss.so\n";
				$add_modules_settings .= ";noload => chan_sip.so\n";
				$add_modules_settings .= "noload => chan_skinny.so\n";
				$add_modules_settings .= ";noload => codec_a_mu.so\n";
				$add_modules_settings .= ";noload => codec_adpcm.so\n";
				$add_modules_settings .= ";noload => codec_alaw.so\n";
				$add_modules_settings .= "noload => codec_dahdi.so\n";
				$add_modules_settings .= ";noload => codec_g726.so\n";
				$add_modules_settings .= ";noload => codec_gsm.so\n";
				$add_modules_settings .= ";noload => codec_lpc10.so\n";
				$add_modules_settings .= ";noload => codec_speex.so\n";
				$add_modules_settings .= ";noload => codec_ulaw.so\n";
				$add_modules_settings .= ";noload => format_g723.so\n";
				$add_modules_settings .= ";noload => format_g726.so\n";
				$add_modules_settings .= ";noload => format_g729.so\n";
				$add_modules_settings .= ";noload => format_gsm.so\n";
				$add_modules_settings .= ";noload => format_h263.so\n";
				$add_modules_settings .= ";noload => format_h264.so\n";
				$add_modules_settings .= ";noload => format_ilbc.so\n";
				$add_modules_settings .= "noload => format_jpeg.so\n";
				$add_modules_settings .= ";noload => format_ogg_vorbis.so\n";
				$add_modules_settings .= ";noload => format_pcm.so\n";
				$add_modules_settings .= ";noload => format_sln.so\n";
				$add_modules_settings .= ";noload => format_vox.so\n";
				$add_modules_settings .= ";noload => format_wav.so\n";
				$add_modules_settings .= ";noload => format_wav_gsm.so\n";
				$add_modules_settings .= ";noload => func_base64.so\n";
				$add_modules_settings .= ";noload => func_callcompletion.so\n";
				$add_modules_settings .= ";noload => func_callerid.so\n";
				$add_modules_settings .= ";noload => func_cdr.so\n";
				$add_modules_settings .= ";noload => func_channel.so\n";
				$add_modules_settings .= ";noload => func_curl.so\n";
				$add_modules_settings .= ";noload => func_cut.so\n";
				$add_modules_settings .= "noload => func_db.so\n";
				$add_modules_settings .= ";noload => func_enum.so\n";
				$add_modules_settings .= ";noload => func_env.so\n";
				$add_modules_settings .= ";noload => func_frame_trace.so\n";
				$add_modules_settings .= ";noload => func_global.so\n";
				$add_modules_settings .= ";noload => func_groupcount.so\n";
				$add_modules_settings .= ";noload => func_logic.so\n";
				$add_modules_settings .= ";noload => func_math.so\n";
				$add_modules_settings .= ";noload => func_md5.so\n";
				$add_modules_settings .= "noload => func_odbc.so\n";
				$add_modules_settings .= ";noload => func_pitchshift.so\n";
				$add_modules_settings .= ";noload => func_rand.so\n";
				$add_modules_settings .= ";noload => func_realtime.so\n";
				$add_modules_settings .= ";noload => func_sha1.so\n";
				$add_modules_settings .= ";noload => func_srv.so\n";
				$add_modules_settings .= ";noload => func_strings.so\n";
				$add_modules_settings .= ";noload => func_timeout.so\n";
				$add_modules_settings .= ";noload => func_uri.so\n";
				$add_modules_settings .= "noload => pbx_ael.so\n";
				$add_modules_settings .= ";noload => pbx_config.so\n";
				$add_modules_settings .= "noload => pbx_dundi.so\n";
				$add_modules_settings .= ";noload => pbx_loopback.so\n";
				$add_modules_settings .= ";noload => pbx_realtime.so\n";
				$add_modules_settings .= ";noload => pbx_spool.so\n";
				$add_modules_settings .= ";noload => res_clioriginate.so\n";
				$add_modules_settings .= "noload => res_config_pgsql.so\n";
				$add_modules_settings .= ";noload => res_convert.so\n";
				$add_modules_settings .= ";noload => res_mutestream.so\n";
				$add_modules_settings .= ";noload => res_rtp_asterisk.so\n";
				$add_modules_settings .= ";noload => res_rtp_multicast.so\n";
				$add_modules_settings .= ";noload => res_security_log.so\n";
				$add_modules_settings .= ";noload => res_snmp.so\n";
				$add_modules_settings .= "noload => cdr_odbc.so\n";
				$add_modules_settings .= "noload => cdr_tds.so\n";
				$add_modules_settings .= "noload => chan_h323.so\n";
				$add_modules_settings .= "noload => res_config_odbc.so\n";
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
				$pattern[0]='@\\;allowguest@';
				$replace[0]='allowguest=no ;by pfSense ;';
				$pattern[1]='@\\;alwaysauthreject@';
				$replace[1]='alwaysauthreject=yes ;by pfSense ;';
				$pattern[2]='@\\; jbenable@';
				$replace[2]='jbenable=yes ;by pfSense ;';
				$pattern[3]='@\\[general\\]\n@';
				$replace[3]='[general]\n;The following general settings usually work on pfSense boxes (note: please do not remove this comment line).\ndisallow=all ;by pfSense\nallow=g729\nallow=ulaw\nallow=alaw\n\n';
				$script_file=preg_replace($pattern,$replace,$script_file);
				file_put_contents($script, $script_file, LOCK_EX);
			}
	
			if (strpos($script_file,'demo extension for pfSense') === false) {  //first check if already added...
				$add_demo_extension = "\n\n";
				$add_demo_extension .= "[301]\n";
				$add_demo_extension .= ";demo extension for pfSense\n";
				$add_demo_extension .= "type=friend\n";
				$add_demo_extension .= "defaultuser=301\n";
				$add_demo_extension .= "insecure=port,invite\n";
				$add_demo_extension .= "secret=1234\n";
				$add_demo_extension .= "regexten=301\n";
				$add_demo_extension .= "host=dynamic\n";
				$add_demo_extension .= "context=default\n\n";
				$add_demo_extension .= "[302]\n";
				$add_demo_extension .= ";demo extension for pfSense\n";
				$add_demo_extension .= "type=friend\n";
				$add_demo_extension .= "defaultuser=302\n";
				$add_demo_extension .= "insecure=port,invite\n";
				$add_demo_extension .= "secret=1234\n";
				$add_demo_extension .= "regexten=302\n";
				$add_demo_extension .= "host=dynamic\n";
				$add_demo_extension .= "context=default\n\n";
				$script_file .= $add_demo_extension;
				file_put_contents($script, $script_file, LOCK_EX);
			}
		}

	$script='/usr/local/etc/rc.d/asterisk';
	if (file_exists($script)){
		$script_file=file_get_contents($script);
		if (preg_match('/NO/',$script_file)){
			$pattern[0]="@NO@";
			$replace[0]="YES";
			$pattern[1]="@\\-nqrx 'core stop now'@";
			$replace[1]="-nqrx 'core stop now'\n  killall " . '$name';
			$script_file=preg_replace($pattern,$replace,$script_file);			}
			//create the /var subdirs for the writable area in RAM
			if (strpos($script_file,'pfSense') === false) {  //first check if already added...
				$writable_area = "\n\n# Create paths in pfSense's volatile area if they don't exist (note: please do not remove this comment line)\n";
				$writable_area .= "if [ ! -d /var/db/asterisk ]; then\n";
				$writable_area .= "  mkdir -p /var/db/asterisk\n";
				$writable_area .= "  chown -R asterisk:asterisk /var/db/asterisk\n";
				$writable_area .= "  chmod -R g+w /var/db/asterisk\n";
				$writable_area .= "fi\n\n";
				$writable_area .= "if [ ! -d /var/spool/asterisk ]; then\n";
				$writable_area .= "  mkdir -p /var/spool/asterisk\n";
				$writable_area .= "  chown -R asterisk:asterisk /var/spool/asterisk\n";
				$writable_area .= "  chmod -R g+w /var/spool/asterisk\n";
				$writable_area .= "fi\n\n";
				$writable_area .= "if [ ! -d /var/run/asterisk ]; then\n";
				$writable_area .= "  mkdir -p /var/run/asterisk\n";
				$writable_area .= "  chown -R asterisk:asterisk /var/run/asterisk\n";
				$writable_area .= "  chmod -R g+w /var/run/asterisk\n";
				$writable_area .= "fi\n\n";
				$writable_area .= "if [ ! -d /var/log/asterisk ]; then\n";
				$writable_area .= "  mkdir -p /var/log/asterisk\n";
				$writable_area .= "  chown -R asterisk:asterisk /var/log/asterisk\n";
				$writable_area .= "  chmod -R g+w /var/log/asterisk\n";
				$writable_area .= "fi\n\n";
				$writable_area .= "if [ ! -d /var/log/asterisk/cdr-csv ]; then\n";
				$writable_area .= "  mkdir -p /var/log/asterisk/cdr-csv\n";
				$writable_area .= "  chown -R asterisk:asterisk /var/log/asterisk/cdr-csv\n";
				$writable_area .= "  chmod -R g+w /var/log/asterisk/cdr-csv\n";
				$writable_area .= "fi\n\n";
				$script_file .= $writable_area;
			}
			file_put_contents($script, $script_file, LOCK_EX);

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
