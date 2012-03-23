<?php
/* ========================================================================== */
/*
	asterisk.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2012 Marcello Coutinho
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
	
	//fix asterisk options for nanobsd: logging, db and calls log in /tmp
	if ($g['platform'] == "nanobsd"){
		$script='/usr/local/etc/asterisk/logger.conf';
		if (file_exists($script)){
			$script_file=file_get_contents($script);
			$pattern[0]='/messages =/';
			$replace[0]='/tmp/asterisk.log =';
			$script_file=preg_replace($pattern,$replace,$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
		}
		
		$script='/usr/local/etc/asterisk/asterisk.conf';
		if (file_exists($script)){
			$script_file=file_get_contents($script);
			$pattern[0]='@astdbdir => [a-z,A-Z,/]+@';
			$replace[0]='astdbdir => /tmp';
			$pattern[1]='@astspooldir => [a-z,A-Z,/]+@';
			$replace[1]='astspooldir => /tmp';
			$script_file=preg_replace($pattern,$replace,$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
		}

		mkdir("/var/log/asterisk");
		symlink("/tmp","/var/log/asterisk/cdr-csv");		
	}

	//add modules settings, for disabling those not required on pfSense	
        $script='/usr/local/etc/asterisk/modules.conf';
        if (file_exists($script)){
		$add_modules_settings = "\n";
		$add_modules_settings .= "noload => res_ael_share.so\n";
		$add_modules_settings .= "noload => res_adsi.so\n";
		$add_modules_settings .= "noload => res_agi.so\n";
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
		$add_modules_settings .= "noload => chan_gtalk.so\n";
		$add_modules_settings .= "noload => chan_iax2.so\n";
		$add_modules_settings .= ";noload => chan_local.so\n";
		$add_modules_settings .= ";noload => chan_mgcp.so\n";
		$add_modules_settings .= ";noload => chan_multicast_rtp.so\n";
		$add_modules_settings .= "noload => chan_oss.so\n";
		$add_modules_settings .= ";noload => chan_sip.so\n";
		$add_modules_settings .= "noload => chan_skinny.so\n";
		$add_modules_settings .= ";noload => codec_a_mu.so\n";
		$add_modules_settings .= ";noload => codec_adpcm.so\n";
		$add_modules_settings .= ";noload => codec_alaw.so\n";
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
		$add_modules_settings .= ";noload => pbx_dundi.so\n";
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

                $script_file=file_get_contents($script);
                $script_file .= $add_modules_settings;
                file_put_contents($script, $script_file, LOCK_EX);
        }

	//replace general SIP settings as a newbie hint to start configuration
        $script='/usr/local/etc/asterisk/sip.conf';
        if (file_exists($script)){
		$add_sip_general_settings = "\n";
		$add_sip_general_settings .= "[general]\n";
		$add_sip_general_settings .= ";The following general settings usually work on pfSense boxes.\n";
		$add_sip_general_settings .= ";Adjust them as needed, and further SIP account settings are required.\n";
		$add_sip_general_settings .= ";For more information check asterisk's online documentation or see the sip.conf-dist file.\n";
		$add_sip_general_settings .= "alwaysauthreject=yes\n";
		$add_sip_general_settings .= "maxexpiry=600\n";
		$add_sip_general_settings .= "defaultexpiry=100\n";
		$add_sip_general_settings .= "registerattempts=250\n";
		$add_sip_general_settings .= "registertimeout=15\n";
		$add_sip_general_settings .= "allowguest = no\n";
		$add_sip_general_settings .= "bindport=5060\n";
		$add_sip_general_settings .= "bindaddr=0.0.0.0 ;with this you don't need to setup any firewall rules at all\n";
		$add_sip_general_settings .= ";localnet=192.168.1.0/255.255.255.0 ;adjust this to your local networks where SIP phoners reside\n";
		$add_sip_general_settings .= ";localnet=192.168.2.0/255.255.255.0 ;add a new line for each local network if you have more\n";
		$add_sip_general_settings .= ";externhost=your.domain.name ;can be dyndns too\n";
		$add_sip_general_settings .= ";externrefresh=600 ;how ofteh to check for doman name's IP\n";
		$add_sip_general_settings .= "jbenable=yes\n";
		$add_sip_general_settings .= "disallow=all\n";
		$add_sip_general_settings .= "allow=g729\n";
		$add_sip_general_settings .= "allow=ulaw\n";
		$add_sip_general_settings .= "allow=alaw\n";

		file_put_contents($script, $add_sip_general_settings, LOCK_EX);
        }


	$script='/usr/local/etc/rc.d/asterisk';
	if (file_exists($script)){
		$script_file=file_get_contents($script);
		if (preg_match('/NO/',$script_file)){
			$script_file=preg_replace("/NO/","YES",$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
			}
		chmod ($script,0755);
		mwexec("$script stop");			
		mwexec_bg("$script start");
	}
	#mount filesystem readonly
	conf_mount_ro();
	
}

?>
