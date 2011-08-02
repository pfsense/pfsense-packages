/* $Id$ */
/*
** Copyright (C) 2002-2010 Sourcefire, Inc.
** Copyright (C) 2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fnmatch.h>
#endif /* !WIN32 */

#include <stdarg.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#ifndef WIN32
#include <grp.h>
#include <pwd.h>
#include <netdb.h>
#include <limits.h>
#endif /* !WIN32 */

#include <fcntl.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef ZLIB
#include <zlib.h>
#endif

#include "snort.h"
#include "mstring.h"
#include "debug.h"
#include "util.h"
#include "parser.h"
#include "inline.h"
#include "build.h"
#include "plugbase.h"
#include "sf_types.h"
#include "sflsq.h"
#include "ipv6_port.h"

#include "pcre.h"

#include "mpse.h"

#include "ppm.h"

#ifdef TARGET_BASED
#include "sftarget_reader.h"
#endif

#ifdef WIN32
#include "win32/WIN32-Code/name.h"
#endif

#include "stream5_common.h"

#ifdef PATH_MAX
#define PATH_MAX_UTIL PATH_MAX
#else
#define PATH_MAX_UTIL 1024
#endif /* PATH_MAX */

extern Stream5Stats s5stats;
extern int datalink;
extern pcap_t *pcap_handle;
extern PreprocStatsFuncNode *preproc_stats_funcs;

static PcapPktStats pkt_stats;

/*
 * you may need to adjust this on the systems which don't have standard
 * paths defined
 */
#ifndef _PATH_VARRUN
static char _PATH_VARRUN[STD_BUF];
#endif


#ifdef NAME_MAX
#define NAME_MAX_UTIL NAME_MAX
#else
#define NAME_MAX_UTIL 256
#endif /* NAME_MAX */

#define FILE_MAX_UTIL  (PATH_MAX_UTIL + NAME_MAX_UTIL)

/****************************************************************************
 *
 * Function: CalcPct(uint64_t, uint64_t)
 *
 * Purpose:  Calculate the percentage of a value compared to a total
 *
 * Arguments: cnt => the numerator in the equation
 *            total => the denominator in the calculation
 *
 * Returns: pct -> the percentage of cnt to value
 *
 ****************************************************************************/
double CalcPct(uint64_t cnt, uint64_t total)
{
    double pct = 0.0;

    if (total == 0.0)
    {
        pct = (double)cnt;
    }
    else
    {
        pct = (double)cnt / (double)total;
    }

    pct *= 100.0;

    return pct;
}


/****************************************************************************
 *
 * Function: DisplayBanner()
 *
 * Purpose:  Show valuable proggie info
 *
 * Arguments: None.
 *
 * Returns: 0 all the time
 *
 ****************************************************************************/
int DisplayBanner(void)
{
    const char * info;
    const char * pcre_ver;
#ifdef ZLIB
    const char * zlib_ver;
#endif

    info = getenv("HOSTTYPE");
    if( !info )
    {
        info="";
    }

    pcre_ver = pcre_version();
#ifdef ZLIB
    zlib_ver = zlib_version;
#endif

    LogMessage("\n");
    LogMessage("   ,,_     -*> Snort! <*-\n");
    LogMessage("  o\"  )~   Version %s%s%s (Build %s) %s %s\n",
               VERSION,
#ifdef SUP_IP6
               " IPv6",
#else
               "",
#endif
#ifdef GRE
               " GRE",
#else
               "",
#endif
               BUILD,
#ifdef GIDS
               "inline",
#else
               "",
#endif
               info);
    LogMessage("   ''''    By Martin Roesch & The Snort Team: http://www.snort.org/snort/snort-team\n");
    LogMessage("           Copyright (C) 1998-2010 Sourcefire, Inc., et al.\n");
    LogMessage("           Using PCRE version: %s\n", pcre_ver);
#ifdef ZLIB
    LogMessage("           Using ZLIB version: %s\n", zlib_ver);
#endif
    LogMessage("\n");
	LogMessage("     ___   Built Date for Snort on Pfsense 2.0 is May 25 2010.\n");
	LogMessage(" ___/ f \\  Orion IPS Output Code Copyright (C) 2009-2010 Robert Zelaya.\n");
	LogMessage("/ p \\___/Sense\n");
	LogMessage("\\___/   \\\n");
	LogMessage("    \\___/  Using Snort.org dynamic plugins and Orion IPS source.\n");
	LogMessage("\n");

    return 0;
}



/****************************************************************************
 *
 * Function: ts_print(register const struct, char *)
 *
 * Purpose: Generate a time stamp and stuff it in a buffer.  This one has
 *          millisecond precision.  Oh yeah, I ripped this code off from
 *          TCPdump, props to those guys.
 *
 * Arguments: timeval => clock struct coming out of libpcap
 *            timebuf => buffer to stuff timestamp into
 *
 * Returns: void function
 *
 ****************************************************************************/
void ts_print(register const struct timeval *tvp, char *timebuf)
{
    register int s;
    int    localzone;
    time_t Time;
    struct timeval tv;
    struct timezone tz;
    struct tm *lt;    /* place to stick the adjusted clock data */

    /* if null was passed, we use current time */
    if(!tvp)
    {
        /* manual page (for linux) says tz is never used, so.. */
        bzero((char *) &tz, sizeof(tz));
        gettimeofday(&tv, &tz);
        tvp = &tv;
    }

    localzone = snort_conf->thiszone;
   
    /*
    **  If we're doing UTC, then make sure that the timezone is correct.
    */
    if (ScOutputUseUtc())
        localzone = 0;
        
    s = (tvp->tv_sec + localzone) % 86400;
    Time = (tvp->tv_sec + localzone) - s;

    lt = gmtime(&Time);

    if (ScOutputIncludeYear())
    {
        (void) SnortSnprintf(timebuf, TIMEBUF_SIZE, 
                        "%02d/%02d/%02d-%02d:%02d:%02d.%06u ", 
                        lt->tm_mon + 1, lt->tm_mday, lt->tm_year - 100, 
                        s / 3600, (s % 3600) / 60, s % 60, 
                        (u_int) tvp->tv_usec);
    } 
    else 
    {
        (void) SnortSnprintf(timebuf, TIMEBUF_SIZE,
                        "%02d/%02d-%02d:%02d:%02d.%06u ", lt->tm_mon + 1,
                        lt->tm_mday, s / 3600, (s % 3600) / 60, s % 60,
                        (u_int) tvp->tv_usec);
    }
}



/****************************************************************************
 *
 * Function: gmt2local(time_t)
 *
 * Purpose: Figures out how to adjust the current clock reading based on the
 *          timezone you're in.  Ripped off from TCPdump.
 *
 * Arguments: time_t => offset from GMT
 *
 * Returns: offset seconds from GMT
 *
 ****************************************************************************/
int gmt2local(time_t t)
{
    register int dt, dir;
    register struct tm *gmt, *loc;
    struct tm sgmt;

    if(t == 0)
        t = time(NULL);

    gmt = &sgmt;
    *gmt = *gmtime(&t);
    loc = localtime(&t);

    dt = (loc->tm_hour - gmt->tm_hour) * 60 * 60 +
        (loc->tm_min - gmt->tm_min) * 60;

    dir = loc->tm_year - gmt->tm_year;

    if(dir == 0)
        dir = loc->tm_yday - gmt->tm_yday;

    dt += dir * 24 * 60 * 60;

    return(dt);
}




/****************************************************************************
 *
 * Function: copy_argv(u_char **)
 *
 * Purpose: Copies a 2D array (like argv) into a flat string.  Stolen from
 *          TCPDump.
 *
 * Arguments: argv => 2D array to flatten
 *
 * Returns: Pointer to the flat string
 *
 ****************************************************************************/
char *copy_argv(char **argv)
{
    char **p;
    u_int len = 0;
    char *buf;
    char *src, *dst;
    //void ftlerr(char *,...);

    p = argv;
    if(*p == 0)
        return 0;

    while(*p)
        len += strlen(*p++) + 1;

    buf = (char *) calloc(1,len);

    if(buf == NULL)
    {
        FatalError("calloc() failed: %s\n", strerror(errno));
    }
    p = argv;
    dst = buf;

    while((src = *p++) != NULL)
    {
        while((*dst++ = *src++) != '\0');
        dst[-1] = ' ';
    }

    dst[-1] = '\0';

    /* Check for an empty string */
    dst = buf;
    while (isspace((int)*dst))
        dst++;

    if (strlen(dst) == 0)
    {
        free(buf);
        buf = NULL;
    }

    return buf;
}


/****************************************************************************
 *
 * Function: strip(char *)
 *
 * Purpose: Strips a data buffer of CR/LF/TABs.  Replaces CR/LF's with
 *          NULL and TABs with spaces.
 *
 * Arguments: data => ptr to the data buf to be stripped
 *
 * Returns: void
 *
 * 3/7/07 - changed to return void - use strlen to get size of string
 *
 * Note that this function will turn all '\n' and '\r' into null chars
 * so, e.g. 'Hello\nWorld\n' => 'Hello\x00World\x00'
 * note that the string is now just 'Hello' and the length is shortened
 * by more than just an ending '\n' or '\r'
 ****************************************************************************/
void strip(char *data)
{
    int size;
    char *end;
    char *idx;

    idx = data;
    end = data + strlen(data);
    size = end - idx;

    while(idx != end)
    {
        if((*idx == '\n') ||
                (*idx == '\r'))
        {
            *idx = 0;
            size--;
        }
        if(*idx == '\t')
        {
            *idx = ' ';
        }
        idx++;
    }
}

/*
 * Function: ErrorMessage(const char *, ...)
 *
 * Purpose: Print a message to stderr.
 *
 * Arguments: format => the formatted error string to print out
 *            ... => format commands/fillers
 *
 * Returns: void function
 */
void ErrorMessage(const char *format,...)
{
    char buf[STD_BUF+1];
    va_list ap;

    if (snort_conf == NULL)
        return;

    va_start(ap, format);

    if (ScDaemonMode() || ScLogSyslog())
    {
        vsnprintf(buf, STD_BUF, format, ap);
        buf[STD_BUF] = '\0';
        syslog(LOG_CONS | LOG_DAEMON | LOG_ERR, "%s", buf);
    }
    else
    {
        vfprintf(stderr, format, ap);
    }
    va_end(ap);
}

/*
 * Function: LogMessage(const char *, ...)
 *
 * Purpose: Print a message to stderr or with logfacility.
 *
 * Arguments: format => the formatted error string to print out
 *            ... => format commands/fillers
 *
 * Returns: void function
 */
void LogMessage(const char *format,...)
{
    char buf[STD_BUF+1];
    va_list ap;

    if (snort_conf == NULL)
        return;

    if (ScLogQuiet() && !ScDaemonMode() && !ScLogSyslog())
        return;

    va_start(ap, format);

    if (ScDaemonMode() || ScLogSyslog())
    {
        vsnprintf(buf, STD_BUF, format, ap);
        buf[STD_BUF] = '\0';
        syslog(LOG_DAEMON | LOG_NOTICE, "%s", buf);
    }
    else
    {
        vfprintf(stderr, format, ap);
    }

    va_end(ap);
}

/*
 * Function: CreateApplicationEventLogEntry(const char *)
 *
 * Purpose: Add an entry to the Win32 "Application" EventLog
 *
 * Arguments: szMessage => the formatted error string to print out
 *
 * Returns: void function
 */
#if defined(WIN32) && defined(ENABLE_WIN32_SERVICE)
void CreateApplicationEventLogEntry(const char *msg)
{
    HANDLE hEventLog; 
    char*  pEventSourceName = "SnortService";

    /* prepare to write to Application log on local host
      * with Event Source of SnortService
      */
    AddEventSource(pEventSourceName);
    hEventLog = RegisterEventSource(NULL, pEventSourceName);
    if (hEventLog == NULL)
    {
        /* Could not register the event source. */
        return;
    }
 
    if (!ReportEvent(hEventLog,   /* event log handle               */
            EVENTLOG_ERROR_TYPE,  /* event type                     */
            0,                    /* category zero                  */
            EVMSG_SIMPLE,         /* event identifier               */
            NULL,                 /* no user security identifier    */
            1,                    /* one substitution string        */
            0,                    /* no data                        */
            &msg,                 /* pointer to array of strings    */
            NULL))                /* pointer to data                */
    {
        /* Could not report the event. */
    }
 
    DeregisterEventSource(hEventLog); 
} 
#endif  /* WIN32 && ENABLE_WIN32_SERVICE */


/*
 * Function: FatalError(const char *, ...)
 *
 * Purpose: When a fatal error occurs, this function prints the error message
 *          and cleanly shuts down the program
 *
 * Arguments: format => the formatted error string to print out
 *            ... => format commands/fillers
 *
 * Returns: void function
 */
NORETURN void FatalError(const char *format,...)
{
    char buf[STD_BUF+1];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, STD_BUF, format, ap);
    va_end(ap);

    buf[STD_BUF] = '\0';

    if ((snort_conf != NULL) && (ScDaemonMode() || ScLogSyslog()))
    {
        syslog(LOG_CONS | LOG_DAEMON | LOG_ERR, "FATAL ERROR: %s", buf);
    }
    else
    {
        fprintf(stderr, "ERROR: %s", buf);
        fprintf(stderr,"Fatal Error, Quitting..\n");
#if defined(WIN32) && defined(ENABLE_WIN32_SERVICE)
        CreateApplicationEventLogEntry(buf);
#endif
    }

    exit(1);
}


/****************************************************************************
 *
 * Function: CreatePidFile(char *)
 *
 * Purpose:  Creates a PID file
 *
 * Arguments: Interface opened.
 *
 * Returns: void function
 *
 ****************************************************************************/
static FILE *pid_lockfile = NULL;
static FILE *pid_file = NULL;
void CreatePidFile(char *intf)
{
    struct stat pt;
    int pid = (int) getpid();
#ifdef WIN32
    char dir[STD_BUF + 1];
#endif

    if (!ScReadMode())
    {
        LogMessage("Checking PID path...\n");

        if (strlen(snort_conf->pid_path) != 0)
        {
            if((stat(snort_conf->pid_path, &pt) == -1) ||
                !S_ISDIR(pt.st_mode) || access(snort_conf->pid_path, W_OK) == -1)
            {
#ifndef WIN32
                /* Save this just in case it's reset with LogMessage call */
                int err = errno;

                LogMessage("WARNING: %s is invalid, trying "
                           "/var/run...\n", snort_conf->pid_path);
                if (err)
                {
                    LogMessage("Previous Error, errno=%d, (%s)\n",
                               err, strerror(err) == NULL ? "Unknown error" : strerror(err));
                }
#endif
                memset(snort_conf->pid_path, 0, sizeof(snort_conf->pid_path));
            }
            else
            {
                LogMessage("PID path stat checked out ok, "
                           "PID path set to %s\n", snort_conf->pid_path);
            }
        }

        if (strlen(snort_conf->pid_path) == 0)
        {
#ifndef _PATH_VARRUN
# ifndef WIN32
            SnortStrncpy(_PATH_VARRUN, "/var/run/", sizeof(_PATH_VARRUN));
# else
            if (GetCurrentDirectory(sizeof(dir) - 1, dir))
                SnortStrncpy(_PATH_VARRUN, dir, sizeof(_PATH_VARRUN));
# endif  /* WIN32 */
#else
            LogMessage("PATH_VARRUN is set to %s on this operating "
                       "system\n", _PATH_VARRUN);
#endif  /* _PATH_VARRUN */

            stat(_PATH_VARRUN, &pt);

            if(!S_ISDIR(pt.st_mode) || access(_PATH_VARRUN, W_OK) == -1)
            {
                LogMessage("WARNING: _PATH_VARRUN is invalid, trying "
                           "/var/log...\n");
                SnortStrncpy(snort_conf->pid_path, "/var/log/", sizeof(snort_conf->pid_path));
                stat(snort_conf->pid_path, &pt);

                if(!S_ISDIR(pt.st_mode) || access(snort_conf->pid_path, W_OK) == -1)
                {
                    LogMessage("WARNING: %s is invalid, logging Snort "
                               "PID path to log directory (%s)\n", snort_conf->pid_path,
                               snort_conf->log_dir);
                    CheckLogDir();
                    SnortSnprintf(snort_conf->pid_path, sizeof(snort_conf->pid_path),
                                  "%s/", snort_conf->log_dir);
                }
            }
            else
            {
                LogMessage("PID path stat checked out ok, "
                           "PID path set to %s\n", _PATH_VARRUN);
                SnortStrncpy(snort_conf->pid_path, _PATH_VARRUN, sizeof(snort_conf->pid_path));
            }
        }
    }

    if(intf == NULL || strlen(snort_conf->pid_path) == 0)
    {
        /* snort_conf->pid_path should have some value by now
         * so let us just be sane. */
        FatalError("CreatePidFile() failed to lookup interface or pid_path is unknown!\n");
    }

    SnortSnprintf(snort_conf->pid_filename, sizeof(snort_conf->pid_filename),
                  "%s/snort_%s%s.pid", snort_conf->pid_path, intf, snort_conf->pidfile_suffix);

#ifndef WIN32
    if (!ScNoLockPidFile())
    {
        char pid_lockfilename[STD_BUF+1];
        int lock_fd;

        /* First, lock the PID file */
        SnortSnprintf(pid_lockfilename, STD_BUF, "%s.lck", snort_conf->pid_filename);
        pid_lockfile = fopen(pid_lockfilename, "w");

        if (pid_lockfile)
        {
            struct flock lock;
            lock_fd = fileno(pid_lockfile);

            lock.l_type = F_WRLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = 0;
            lock.l_len = 0;

            if (fcntl(lock_fd, F_SETLK, &lock) == -1)
            {
                ClosePidFile();
                FatalError("Failed to Lock PID File \"%s\" for PID \"%d\"\n", snort_conf->pid_filename, pid);
            }
        }
    }
#endif

    /* Okay, were able to lock PID file, now open and write PID */
    pid_file = fopen(snort_conf->pid_filename, "w");
    if(pid_file)
    {
        LogMessage("Writing PID \"%d\" to file \"%s\"\n", pid, snort_conf->pid_filename);
        fprintf(pid_file, "%d\n", pid);
        fflush(pid_file);
    }
    else
    {
        ErrorMessage("Failed to create pid file %s", snort_conf->pid_filename);
        snort_conf->pid_filename[0] = 0;
    }
}

/****************************************************************************
 *
 * Function: ClosePidFile(char *)
 *
 * Purpose:  Releases lock on a PID file
 *
 * Arguments: None
 *
 * Returns: void function
 *
 ****************************************************************************/
void ClosePidFile(void)
{
    if (pid_file)
    {
        fclose(pid_file);
        pid_file = NULL;
    }
    if (pid_lockfile)
    {
        fclose(pid_lockfile);
        pid_lockfile = NULL;
    }
}

/****************************************************************************
 *
 * Function: SetUidGid()
 *
 * Purpose:  Sets safe UserID and GroupID if needed
 *
 * Arguments: none
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetUidGid(int user_id, int group_id)
{
#ifndef WIN32

    if ((group_id != -1) && (getgid() != (gid_t)group_id))
    {
        if (!InlineModeSetPrivsAllowed())
        {
            ErrorMessage("Cannot set uid and gid when running Snort in "
                         "inline mode.\n");
            return;
        }

        if (setgid(group_id) < 0)
            FatalError("Cannot set gid: %d\n", group_id);

        DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Set gid to %d\n", group_id););
    }

    if ((user_id != -1) && (getuid() != (uid_t)user_id))
    {
        struct passwd *pw = getpwuid(user_id);

        if (!InlineModeSetPrivsAllowed())
        {
            ErrorMessage("Cannot set uid and gid when running Snort in "
                         "inline mode.\n");
            return;
        }

        if (pw != NULL)
        {
            /* getpwuid and initgroups may use the same static buffers */
            char *username = SnortStrdup(pw->pw_name);

            if ((getuid() == 0) && (initgroups(username, group_id) < 0))
            {
                free(username);
                FatalError("Can not initgroups(%s,%d)",
                           username, group_id);
            }

            free(username);
        }

        /** just to be on a safe side... **/
        endgrent();
        endpwent();

        if (setuid(user_id) < 0)
            FatalError("Can not set uid: %d\n", user_id);

        DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Set uid to %d\n", user_id););
    }
#endif  /* WIN32 */
}

#ifdef TIMESTATS

static IntervalStats istats = {0};
time_t start_time;

void InitTimeStats(void)
{
    start_time = time(NULL);
}

void ResetTimeStats(void)
{
    memset(&istats, 0, sizeof(istats));
}

/* This function prints out stats based on a configurable time
 * interval.  It is an indication on how well snort is */
/* processing packets, including types, drops, etc */
void DropStatsPerTimeInterval(void)
{
    double per_sec, per_minute, per_hour;
    uint64_t recv, drop;
    uint64_t total = 0;
    uint32_t timestats_interval = ScTimestatsInterval();

#ifdef PCAP_CLOSE
    if (UpdatePcapPktStats(0) != -1)
#else
    if (UpdatePcapPktStats() != -1)
#endif
    {
        recv = GetPcapPktStatsRecv();
        drop = GetPcapPktStatsDrop();

        istats.recv = recv - istats.recv_total;
        istats.recv_total = recv;

        istats.drop = drop - istats.drop_total;
        istats.drop_total = drop;

        /* calculate received packets by type */
        istats.tcp = pc.tcp - istats.tcp_total;
        istats.tcp_total = pc.tcp;

        istats.udp = pc.udp - istats.udp_total;
        istats.udp_total = pc.udp;

        istats.icmp = pc.icmp - istats.icmp_total;
        istats.icmp_total = pc.icmp;

        istats.arp = pc.arp - istats.arp_total;
        istats.arp_total = pc.arp;

#ifdef GRE
        istats.ip4ip4 = pc.ip4ip4 - istats.ip4ip4_total;
        istats.ip4ip4_total = pc.ip4ip4;

        istats.ip4ip6 = pc.ip4ip6 - istats.ip4ip6_total;
        istats.ip4ip6_total = pc.ip4ip6;

        istats.ip6ip4 = pc.ip6ip4 - istats.ip6ip4_total;
        istats.ip6ip4_total = pc.ip6ip4;

        istats.ip6ip6 = pc.ip6ip6 - istats.ip6ip6_total;
        istats.ip6ip6_total = pc.ip6ip6;

        istats.gre = pc.gre - istats.gre_total;
        istats.gre_total = pc.gre;

        istats.gre_ip = pc.gre_ip - istats.gre_ip_total;
        istats.gre_ip_total = pc.gre_ip;

        istats.gre_eth = pc.gre_eth - istats.gre_eth_total;
        istats.gre_eth_total = pc.gre_eth;

        istats.gre_arp = pc.gre_arp - istats.gre_arp_total;
        istats.gre_arp_total = pc.gre_arp;

        istats.gre_ipv6 = pc.gre_ipv6 - istats.gre_ipv6_total;
        istats.gre_ipv6_total = pc.gre_ipv6;

        istats.gre_ipx = pc.gre_ipx - istats.gre_ipx_total;
        istats.gre_ipx_total = pc.gre_ipx;

        istats.gre_loopback = pc.gre_loopback - istats.gre_loopback_total;
        istats.gre_loopback_total = pc.gre_loopback;

        istats.gre_vlan = pc.gre_vlan - istats.gre_vlan_total;
        istats.gre_vlan_total = pc.gre_vlan;

        istats.gre_ppp = pc.gre_ppp - istats.gre_ppp_total;
        istats.gre_ppp_total = pc.gre_ppp;
#endif

#ifdef DLT_IEEE802_11   /* if we are tracking wireless, add this to output */
        istats.wifi_mgmt = pc.wifi_mgmt - istats.wifi_mgmt_total;
        istats.wifi_mgmt_total = pc.wifi_mgmt;

        istats.wifi_control = pc.wifi_control - istats.wifi_control_total;
        istats.wifi_control_total = pc.wifi_control;

        istats.wifi_data = pc.wifi_data - istats.wifi_data_total;
        istats.wifi_data_total = pc.wifi_data;
#endif

        istats.ipx = pc.ipx - istats.ipx_total;
        istats.ipx_total = pc.ipx;

        istats.eapol = pc.eapol - istats.eapol_total;
        istats.eapol_total = pc.eapol;

        istats.ipv6 = pc.ipv6 - istats.ipv6_total;
        istats.ipv6_total = pc.ipv6;

        istats.ethloopback = pc.ethloopback - istats.ethloopback_total;
        istats.ethloopback_total = pc.ethloopback;

        istats.other = pc.other - istats.other_total;
        istats.other_total = pc.other;

        istats.discards = pc.discards - istats.discards_total;
        istats.discards_total = pc.discards;

        if (pc.frags > 0) /* do we have any fragmented packets being seen? */
        {
            istats.frags = pc.frags - istats.frags_total;
            istats.frags_total = pc.frags;

            istats.frag_trackers = pc.frag_trackers - istats.frag_trackers_total;
            istats.frag_trackers_total = pc.frag_trackers;

            istats.frag_rebuilt = pc.rebuilt_frags - istats.frag_rebuilt_total;
            istats.frag_rebuilt_total = pc.rebuilt_frags;

            istats.frag_element = pc.rebuild_element - istats.frag_element_total;
            istats.frag_element_total = pc.rebuild_element;

            istats.frag_incomp = pc.frag_incomp - istats.frag_incomp_total;
            istats.frag_incomp_total = pc.frag_incomp;

            istats.frag_timeout = pc.frag_timeout - istats.frag_timeout_total;
            istats.frag_timeout_total = pc.frag_timeout;

            istats.frag_mem_faults = pc.frag_mem_faults - istats.frag_mem_faults_total;
            istats.frag_mem_faults_total = pc.frag_mem_faults;
        }

        if (pc.tcp_stream_pkts > 0) /* do we have TCP stream re-assembly going on? */
        {
            istats.tcp_str_packets = pc.tcp_stream_pkts - istats.tcp_str_packets_total;
            istats.tcp_str_packets_total = pc.tcp_stream_pkts;

            istats.tcp_str_trackers = pc.tcp_streams - istats.tcp_str_trackers_total;
            istats.tcp_str_trackers_total = pc.tcp_streams;

            istats.tcp_str_flushes = pc.rebuilt_tcp - istats.tcp_str_flushes_total;
            istats.tcp_str_flushes_total = pc.rebuilt_tcp;

            istats.tcp_str_segs_used = pc.rebuilt_segs - istats.tcp_str_segs_used_total;
            istats.tcp_str_segs_used_total = pc.rebuilt_segs;

            istats.tcp_str_segs_queued = pc.queued_segs - istats.tcp_str_segs_queued_total;
            istats.tcp_str_segs_queued_total = pc.queued_segs;

            istats.tcp_str_mem_faults = pc.str_mem_faults - istats.tcp_str_mem_faults_total;
            istats.tcp_str_mem_faults_total = pc.str_mem_faults;
        }

        istats.processed = pc.total_processed - istats.processed_total;
        istats.processed_total = pc.total_processed;
        total = istats.processed;

        /* prepare packet type per time interval routine */
        LogMessage("================================================"
                   "===============================\n");

        LogMessage("\n");
        LogMessage("Statistics Report (last %d seconds)\n", timestats_interval);
        LogMessage("\n");

        per_sec = (double)istats.recv / (double)timestats_interval;

        LogMessage("Packet Wire Totals:\n");
        LogMessage("Packets received: " FMTu64("13") "\n", istats.recv);

        if (timestats_interval >= SECONDS_PER_HOUR)
        {
            per_hour = (double)(istats.recv * SECONDS_PER_HOUR) / (double)timestats_interval;
        LogMessage("        per hour: %13.2f\n", per_hour);
        }
        if (timestats_interval >= SECONDS_PER_MIN)
        {
            per_minute = (double)(istats.recv * SECONDS_PER_MIN) / (double)timestats_interval;
        LogMessage("      per minute: %13.2f\n", per_minute);
        }
        LogMessage("      per second: %13.2f\n", per_sec);
        LogMessage(" Packets dropped: " FMTu64("13") "\n", istats.drop);
        LogMessage("\n");
        LogMessage("Packet Breakdown by Protocol (includes rebuilt packets):\n");

        LogMessage("     TCP: " FMTu64("10") " (%.3f%%)\n",
                   istats.tcp, CalcPct(istats.tcp, total));
        LogMessage("     UDP: " FMTu64("10") " (%.3f%%)\n",
                   istats.udp, CalcPct(istats.udp, total));
        LogMessage("    ICMP: " FMTu64("10") " (%.3f%%)\n",
                   istats.icmp, CalcPct(istats.icmp, total));
        LogMessage("     ARP: " FMTu64("10") " (%.3f%%)\n",
                   istats.arp, CalcPct(istats.arp, total));
#ifndef NO_NON_ETHER_DECODER
        LogMessage("   EAPOL: " FMTu64("10") " (%.3f%%)\n",
                   istats.eapol, CalcPct(istats.eapol, total));
#endif
        LogMessage("    IPv6: " FMTu64("10") " (%.3f%%)\n",
                   istats.ipv6, CalcPct(istats.ipv6, total));
        LogMessage(" ETHLOOP: " FMTu64("10") " (%.3f%%)\n",
                   istats.ethloopback, CalcPct(istats.ethloopback, total));
        LogMessage("     IPX: " FMTu64("10") " (%.3f%%)\n",
                   istats.ipx, CalcPct(istats.ipx, total));

#ifdef GRE
        LogMessage(" IP4/IP4: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.ip4ip4, CalcPct(istats.ip4ip4, total));
        LogMessage(" IP4/IP6: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.ip4ip6, CalcPct(istats.ip4ip6, total));
        LogMessage(" IP6/IP4: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.ip6ip4, CalcPct(istats.ip6ip4, total));
        LogMessage(" IP6/IP6: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.ip6ip6, CalcPct(istats.ip6ip6, total));
        LogMessage("     GRE: " FMTu64("10") " (%.3f%%)\n",
                   istats.gre, CalcPct(istats.gre, total));
        LogMessage(" GRE ETH: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.gre_eth, CalcPct(istats.gre_eth, total));
        LogMessage("GRE VLAN: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.gre_vlan, CalcPct(istats.gre_vlan, total));
        LogMessage("  GRE IP: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.gre_ip, CalcPct(istats.gre_ip, total));
        LogMessage("GRE IPv6: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.gre_ipv6, CalcPct(istats.gre_ipv6, total));
        LogMessage("GRE PPTP: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.gre_ppp, CalcPct(istats.gre_ppp, total));
        LogMessage(" GRE ARP: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.gre_arp, CalcPct(istats.gre_arp, total));
        LogMessage(" GRE IPX: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.gre_ipx, CalcPct(istats.gre_ipx, total));
        LogMessage("GRE LOOP: " FMTu64("-10") " (%.3f%%)\n", 
                   istats.gre_loopback, CalcPct(istats.gre_loopback, total));
#endif

        LogMessage("    FRAG: " FMTu64("10") " (%.3f%%)\n",
                   istats.frags, CalcPct(istats.frags, total));
        LogMessage("   OTHER: " FMTu64("10") " (%.3f%%)\n",
                   istats.other, CalcPct(istats.other, total));
        LogMessage(" DISCARD: " FMTu64("10") " (%.3f%%)\n",
                   istats.discards, CalcPct(istats.discards, total));
        LogMessage("   Total: " FMTu64("10") "\n", total);

        LogMessage("\n");


        /*   handle case where wireless is enabled...	*/

#ifndef NO_NON_ETHER_DECODER
#ifdef DLT_IEEE802_11
        if (datalink == DLT_IEEE802_11)
        {
            LogMessage("\n");
            LogMessage("Wireless Stats:\n\n");
            LogMessage("Management Packets: " FMTu64("10") " (%.3f%%)\n",
                       istats.wifi_mgmt, CalcPct(istats.wifi_mgmt, total));
            LogMessage("   Control Packets: " FMTu64("10") " (%.3f%%)\n",
                       istats.wifi_control, CalcPct(istats.wifi_control, total));
            LogMessage("      Data Packets: " FMTu64("10") " (%.3f%%)\n",
                       istats.wifi_data, CalcPct(istats.wifi_data, total));
            LogMessage("\n");
        }

#endif /* if wireless is enabled... */
#endif  // NO_NON_ETHER_DECODER

        /*   handle case where we have snort seeing fragmented packets	*/

        if (pc.frags > 0) /* begin if (pc.frags > 0) */
        {
            LogMessage("\n");
            LogMessage("Fragmentation Stats:\n\n");
            LogMessage("Fragmented IP Packets: " FMTu64("10") "\n", istats.frags);
            LogMessage("    Fragment Trackers: " FMTu64("10") "\n", istats.frag_trackers);
            LogMessage("   Rebuilt IP Packets: " FMTu64("10") "\n", istats.frag_rebuilt);
            LogMessage("   Frag Elements Used: " FMTu64("10") "\n", istats.frag_element);
            LogMessage("Discarded(incomplete): " FMTu64("10") "\n", istats.frag_incomp);
            LogMessage("   Discarded(timeout): " FMTu64("10") "\n", istats.frag_timeout);
            LogMessage("  Frag2 memory faults: " FMTu64("10") "\n", istats.frag_mem_faults);
            LogMessage("\n");
        }   /* end if (pc.frags > 0) */

        /*   handle TCP stream re-assy stuff here */ 

        if (pc.tcp_stream_pkts > 0)
        {
            LogMessage("\n");
            LogMessage("TCP Stream Reassembly Stats:\n\n");
            LogMessage("      TCP Packets Used: " FMTu64("10") "\n", istats.tcp_str_packets);
            LogMessage("       Stream Trackers: " FMTu64("10") "\n", istats.tcp_str_trackers);
            LogMessage("        Stream Flushes: " FMTu64("10") "\n", istats.tcp_str_flushes);
            LogMessage("  Stream Segments Used: " FMTu64("10") "\n", istats.tcp_str_segs_used);
            LogMessage("Stream Segments Queued: " FMTu64("10") "\n", istats.tcp_str_segs_queued);
            LogMessage(" Stream4 Memory Faults: " FMTu64("10") "\n", istats.tcp_str_mem_faults);
            LogMessage("\n");
        }

        //mpse_print_qinfo();

    }  /* end if pcap_stats(ps, &ps) */

    alarm(timestats_interval);   /* reset the alarm to go off again */
}

/* print out stats on how long snort ran */
void TimeStats(void)
{

/*
 *  variable definitions for improved statistics handling
 *
 *  end_time = time which snort finished running (unix epoch)
 *  total_secs = total amount of time snort ran
 *  int_total_secs = used to eliminate casts from this function (temp. var)
 *  days = number of days snort ran
 *  hrs  = number of hrs snort ran
 *  mins = number of minutes snort ran
 *  secs = number of seconds snort ran
 *
 *  ival = temp. variable for integer/modulus math
 *  ppd  = packets per day processed
 *  pph  = packets per hour processed
 *  ppm  = packets per minute processed
 *  pps  = packets per second processed
 *
 *  hflag = used to flag when hrs = zero, but days > 0
 *  mflag = used to flag when min = zero, but hrs > 0
 *
 */

    time_t end_time, total_secs;
    uint32_t days = 0, hrs = 0, mins = 0, secs = 0, tmp = 0;
    uint64_t pps = 0, ppm = 0, pph = 0, ppd = 0;
    uint32_t int_total_secs = 0;
    char hflag = 0, mflag = 0;


    end_time = time(NULL);              /* grab epoch for end time value (in seconds) */
    total_secs = end_time - start_time; /* total_secs is how many seconds snort ran for */

    tmp = (uint32_t)total_secs;        
    int_total_secs = tmp;               /* used for cast elimination */

    days = tmp / SECONDS_PER_DAY;       /* 86400 is number of seconds in a day */
    tmp  = tmp % SECONDS_PER_DAY;       /* grab remainder to process hours */
    hrs  = tmp / SECONDS_PER_HOUR;      /* 3600 is number of seconds in a(n) hour */
    tmp  = tmp % SECONDS_PER_HOUR;      /* grab remainder to process minutes */
    mins = tmp / SECONDS_PER_MIN;       /* 60 is number of seconds in a minute */
    secs = tmp % SECONDS_PER_MIN;       /* grab remainder to process seconds */

    if (total_secs)
        pps = (pc.total_from_pcap / int_total_secs);
    else                                         
        pps = pc.total_from_pcap;     /* guard against division by zero */

    /* Use ErrorMessage because this is logged whether
     * or not logging quietly */
    ErrorMessage("Snort ran for %u Days %u Hours %u Minutes %u Seconds\n",
                 days, hrs, mins, secs);

    if (days > 0)
    {
        ppd = (pc.total_from_pcap / (int_total_secs / SECONDS_PER_DAY));
        ErrorMessage("Snort Analyzed " STDu64 " Packets Per Day\n", ppd);
        hflag = 1;
    }

    if (hrs > 0 || hflag == 1)
    {
        pph = (pc.total_from_pcap / (int_total_secs / SECONDS_PER_HOUR));
        ErrorMessage("Snort Analyzed " STDu64 " Packets Per Hour\n", pph);
        mflag = 1;
    }

    if (mins > 0 || mflag == 1)
    {
        ppm = (pc.total_from_pcap / (int_total_secs / SECONDS_PER_MIN));
        ErrorMessage("Snort Analyzed " STDu64 " Packets Per Minute\n", ppm);
    }

    ErrorMessage("Snort Analyzed " STDu64 " Packets Per Second\n", pps);
    ErrorMessage("\n");
}
#endif /* TIMESTATS */


#ifdef PCAP_CLOSE
int UpdatePcapPktStats(int cacheReturn)
#else
int UpdatePcapPktStats(void)
#endif
{
    struct pcap_stat ps;
    uint32_t recv, drop;
    static char not_initialized = 1;

#ifdef PCAP_CLOSE
    static int priorReturn = 0;
    static int returnWasCached = 0;

    if ( !cacheReturn && returnWasCached )
    {
        returnWasCached = 0;
        return priorReturn;
    }
    priorReturn = -1;
    returnWasCached = cacheReturn;
#endif

    if (not_initialized)
    {
        memset(&pkt_stats, 0, sizeof(PcapPktStats));
        not_initialized = 0;
    }
    
    if ((pcap_handle == NULL) || ScReadMode())
        return -1;

    if (pcap_stats(pcap_handle, &ps) == -1)
    {
        pcap_perror(pcap_handle, "pcap_stats");
        return -1;
    }

    recv = (uint32_t)ps.ps_recv;
    drop = (uint32_t)ps.ps_drop;

#ifdef LINUX_LIBPCAP_DOUBLES_STATS
    recv /= 2;
    drop /= 2;
#endif

#ifdef LIBPCAP_ACCUMULATES
    /* pcap recv wrapped */
    if (recv < pkt_stats.wrap_recv)
        pkt_stats.recv += (uint64_t)UINT32_MAX;

    /* pcap drop wrapped */
    if (drop < pkt_stats.wrap_drop)
        pkt_stats.drop += (uint64_t)UINT32_MAX;

    pkt_stats.wrap_recv = recv;
    pkt_stats.wrap_drop = drop;
#else
    pkt_stats.recv += (uint64_t)recv;
    pkt_stats.drop += (uint64_t)drop;
#endif  /* LIBPCAP_ACCUMULATES */

#ifdef PCAP_CLOSE
    priorReturn = 0;
#endif
    return 0;
}

uint64_t GetPcapPktStatsRecv(void)
{
    return pkt_stats.recv + (uint64_t)pkt_stats.wrap_recv;
}

uint64_t GetPcapPktStatsDrop(void)
{
    return pkt_stats.drop + (uint64_t)pkt_stats.wrap_drop;
}


#ifdef PCAP_CLOSE
/* exiting should be 0 for if not exiting, 1 if restarting, and 2 if exiting */
#else
/* exiting should be 0 for if not exiting and 1 if exiting */
#endif
void DropStats(int exiting)
{
    PreprocStatsFuncNode *idx;
    uint64_t total = 0;
    uint64_t pkts_recv;
    uint64_t pkts_drop;

    total = pc.total_processed;

#ifdef PPM_MGR
    PPM_PRINT_SUMMARY(&snort_conf->ppm_cfg);
#endif

    LogMessage("================================================"
               "===============================\n");

#ifdef TIMESTATS
    TimeStats();     /* how long did snort run? */
#endif

    if (ScReadMode()
#ifdef GIDS
            || ScAdapterInlineMode()
#endif
       )
    {
        LogMessage("Snort processed " STDu64 " packets.\n", total);
    }
    else
    {
#ifdef PCAP_CLOSE
        if (exiting < 2 && (pcap_handle == NULL))
#else
        if (pcap_handle == NULL)
#endif
        {
            LogMessage("Snort received 0 packets\n");
        }
        else
        {
#ifdef PCAP_CLOSE
            if (UpdatePcapPktStats(0) != -1)
#else
            if (UpdatePcapPktStats() != -1)
#endif
            {
                pkts_recv = GetPcapPktStatsRecv();
                pkts_drop = GetPcapPktStatsDrop();

                LogMessage("Packet Wire Totals:\n");
                LogMessage("   Received: " FMTu64("12") "\n", pkts_recv);
                LogMessage("   Analyzed: " FMTu64("12") " (%.3f%%)\n", pc.total_from_pcap,
                        CalcPct(pc.total_from_pcap, pkts_recv));
                LogMessage("    Dropped: " FMTu64("12") " (%.3f%%)\n", pkts_drop,
                        CalcPct(pkts_drop, pkts_recv));
                LogMessage("Outstanding: " FMTu64("12") " (%.3f%%)\n",
                        pkts_recv - pkts_drop - pc.total_from_pcap,
                        CalcPct((pkts_recv - pkts_drop - pc.total_from_pcap), pkts_recv));
            }
            else
            {
                LogMessage("Unable to calculate percentages for stats\n");
                LogMessage("Total number of packets Analyzed: " FMTu64("12") "\n", pc.total_from_pcap);
            }		
        }
    }

    LogMessage("================================================"
               "===============================\n");

    LogMessage("Breakdown by protocol (includes rebuilt packets):\n");

    LogMessage("      ETH: " FMTu64("-10") " (%.3f%%)\n", 
               pc.eth, CalcPct(pc.eth, total));
    LogMessage("  ETHdisc: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ethdisc, CalcPct(pc.ethdisc, total));
#ifdef GIDS
#ifndef IPFW
    LogMessage(" IPTables: " FMTu64("-10") " (%.3f%%)\n", 
               pc.iptables, CalcPct(pc.iptables, total));
#else
    LogMessage("     IPFW: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ipfw, CalcPct(pc.ipfw, total));
#endif  /* IPFW */
#endif  /* GIDS */
    LogMessage("     VLAN: " FMTu64("-10") " (%.3f%%)\n", 
               pc.vlan, CalcPct(pc.vlan, total));

    if (pc.nested_vlan != 0)
    LogMessage("Nested VLAN: " FMTu64("-10") " (%.3f%%)\n", 
               pc.nested_vlan, CalcPct(pc.nested_vlan, total));

    LogMessage("     IPV6: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ipv6, CalcPct(pc.ipv6, total));
    LogMessage("  IP6 EXT: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ip6ext, CalcPct(pc.ip6ext, total));
    LogMessage("  IP6opts: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ipv6opts, CalcPct(pc.ipv6opts, total));
    LogMessage("  IP6disc: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ipv6disc, CalcPct(pc.ipv6disc, total));

    LogMessage("      IP4: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ip, CalcPct(pc.ip, total));
    LogMessage("  IP4disc: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ipdisc, CalcPct(pc.ipdisc, total));

    LogMessage("    TCP 6: " FMTu64("-10") " (%.3f%%)\n", 
               pc.tcp6, CalcPct(pc.tcp6, total));
    LogMessage("    UDP 6: " FMTu64("-10") " (%.3f%%)\n", 
               pc.udp6, CalcPct(pc.udp6, total));
    LogMessage("    ICMP6: " FMTu64("-10") " (%.3f%%)\n", 
               pc.icmp6, CalcPct(pc.icmp6, total));
    LogMessage("  ICMP-IP: " FMTu64("-10") " (%.3f%%)\n", 
               pc.embdip, CalcPct(pc.embdip, total));

    LogMessage("      TCP: " FMTu64("-10") " (%.3f%%)\n", 
               pc.tcp, CalcPct(pc.tcp, total));
    LogMessage("      UDP: " FMTu64("-10") " (%.3f%%)\n", 
               pc.udp, CalcPct(pc.udp, total));
    LogMessage("     ICMP: " FMTu64("-10") " (%.3f%%)\n", 
               pc.icmp, CalcPct(pc.icmp, total));

    LogMessage("  TCPdisc: " FMTu64("-10") " (%.3f%%)\n", 
               pc.tdisc, CalcPct(pc.tdisc, total));
    LogMessage("  UDPdisc: " FMTu64("-10") " (%.3f%%)\n", 
               pc.udisc, CalcPct(pc.udisc, total));
    LogMessage("  ICMPdis: " FMTu64("-10") " (%.3f%%)\n", 
               pc.icmpdisc, CalcPct(pc.icmpdisc, total));

    LogMessage("     FRAG: " FMTu64("-10") " (%.3f%%)\n", 
               pc.frags, CalcPct(pc.frags, total));
    LogMessage("   FRAG 6: " FMTu64("-10") " (%.3f%%)\n", 
               pc.frag6, CalcPct(pc.frag6, total));

    LogMessage("      ARP: " FMTu64("-10") " (%.3f%%)\n", 
               pc.arp, CalcPct(pc.arp, total));
#ifndef NO_NON_ETHER_DECODER
    LogMessage("    EAPOL: " FMTu64("-10") " (%.3f%%)\n", 
               pc.eapol, CalcPct(pc.eapol, total));
#endif
    LogMessage("  ETHLOOP: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ethloopback, CalcPct(pc.ethloopback, total));
    LogMessage("      IPX: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ipx, CalcPct(pc.ipx, total));
#ifdef GRE
    LogMessage("IPv4/IPv4: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ip4ip4, CalcPct(pc.ip4ip4, total));
    LogMessage("IPv4/IPv6: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ip4ip6, CalcPct(pc.ip4ip6, total));
    LogMessage("IPv6/IPv4: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ip6ip4, CalcPct(pc.ip6ip4, total));
    LogMessage("IPv6/IPv6: " FMTu64("-10") " (%.3f%%)\n", 
               pc.ip6ip6, CalcPct(pc.ip6ip6, total));
    LogMessage("      GRE: " FMTu64("-10") " (%.3f%%)\n", 
               pc.gre, CalcPct(pc.gre, total));
    LogMessage("  GRE ETH: " FMTu64("-10") " (%.3f%%)\n", 
               pc.gre_eth, CalcPct(pc.gre_eth, total));
    LogMessage(" GRE VLAN: " FMTu64("-10") " (%.3f%%)\n", 
               pc.gre_vlan, CalcPct(pc.gre_vlan, total));
    LogMessage(" GRE IPv4: " FMTu64("-10") " (%.3f%%)\n", 
               pc.gre_ip, CalcPct(pc.gre_ip, total));
    LogMessage(" GRE IPv6: " FMTu64("-10") " (%.3f%%)\n", 
               pc.gre_ipv6, CalcPct(pc.gre_ipv6, total));
    LogMessage("GRE IP6 E: " FMTu64("-10") " (%.3f%%)\n", 
               pc.gre_ipv6ext, CalcPct(pc.gre_ipv6ext, total));
    LogMessage(" GRE PPTP: " FMTu64("-10") " (%.3f%%)\n", 
               pc.gre_ppp, CalcPct(pc.gre_ppp, total));
    LogMessage("  GRE ARP: " FMTu64("-10") " (%.3f%%)\n", 
               pc.gre_arp, CalcPct(pc.gre_arp, total));
    LogMessage("  GRE IPX: " FMTu64("-10") " (%.3f%%)\n", 
               pc.gre_ipx, CalcPct(pc.gre_ipx, total));
    LogMessage(" GRE LOOP: " FMTu64("-10") " (%.3f%%)\n", 
               pc.gre_loopback, CalcPct(pc.gre_loopback, total));
#endif  /* GRE */
#ifdef MPLS
    LogMessage("     MPLS: " FMTu64("-10") " (%.3f%%)\n", 
                   pc.mpls, CalcPct(pc.mpls, total));
#endif
    LogMessage("    OTHER: " FMTu64("-10") " (%.3f%%)\n", 
               pc.other, CalcPct(pc.other, total));
    LogMessage("  DISCARD: " FMTu64("-10") " (%.3f%%)\n", 
               pc.discards, CalcPct(pc.discards, total));
    LogMessage("InvChkSum: " FMTu64("-10") " (%.3f%%)\n", 
               pc.invalid_checksums, CalcPct(pc.invalid_checksums, total));

    LogMessage("   S5 G 1: " FMTu64("-10") " (%.3f%%)\n", 
               pc.s5tcp1, CalcPct(pc.s5tcp1, total));
    LogMessage("   S5 G 2: " FMTu64("-10") " (%.3f%%)\n", 
               pc.s5tcp2, CalcPct(pc.s5tcp2, total));

    LogMessage("    Total: " FMTu64("-10") "\n", total);

    LogMessage("================================================"
               "===============================\n");

    LogMessage("Action Stats:\n");
    LogMessage("ALERTS: " STDu64 "\n", pc.alert_pkts);
    LogMessage("LOGGED: " STDu64 "\n", pc.log_pkts);
    LogMessage("PASSED: " STDu64 "\n", pc.pass_pkts);

#ifdef TARGET_BASED
    if (ScIdsMode() && IsAdaptiveConfigured(getDefaultPolicy(), 0))
    {
        LogMessage("================================================"
                   "===============================\n");
        LogMessage("Attribute Table Stats:\n");
        LogMessage("    Number Entries: %u\n", SFAT_NumberOfHosts());
        LogMessage("    Table Reloaded: " STDu64 "\n", pc.attribute_table_reloads);
    }
#endif  /* TARGET_BASED */

    //mpse_print_qinfo();

#ifndef NO_NON_ETHER_DECODER
#ifdef DLT_IEEE802_11
    if(datalink == DLT_IEEE802_11)
    {
        LogMessage("================================================"
                   "===============================\n");
        LogMessage("Wireless Stats:\n");
        LogMessage("Breakdown by type:\n");
        LogMessage("    Management Packets: " FMTu64("-10") " (%.3f%%)\n", 
                   pc.wifi_mgmt, CalcPct(pc.wifi_mgmt, total));
        LogMessage("    Control Packets:    " FMTu64("-10") " (%.3f%%)\n", 
                   pc.wifi_control, CalcPct(pc.wifi_control, total));
        LogMessage("    Data Packets:       " FMTu64("-10") " (%.3f%%)\n", 
                   pc.wifi_data, CalcPct(pc.wifi_data, total));
    }
#endif  /* DLT_IEEE802_11 */
#endif  // NO_NON_ETHER_DECODER

    for (idx = preproc_stats_funcs; idx != NULL; idx = idx->next)
    {
        LogMessage("=============================================="
                   "=================================\n");

#ifdef PCAP_CLOSE
        idx->func(exiting ? 1 : 0);
#else
        idx->func(exiting);
#endif
    }

    LogMessage("=============================================="
               "=================================\n");

    return;
}

/****************************************************************************
 *
 * Function: CleanupProtoNames()
 *
 * Purpose: Frees the protocol names
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void CleanupProtoNames(void)
{
    int i;

    for(i = 0; i < 256; i++)
    {
        if( protocol_names[i] != NULL )
        {
            free( protocol_names[i] );
            protocol_names[i] = NULL;
        }
    }
}

/****************************************************************************
 *
 * Function: read_infile(char *)
 *
 * Purpose: Reads the BPF filters in from a file.  Ripped from tcpdump.
 *
 * Arguments: fname => the name of the file containing the BPF filters
 *
 * Returns: the processed BPF string
 *
 ****************************************************************************/
char *read_infile(char *fname)
{
    register int fd, cc;
    register char *cp, *cmt;
    struct stat buf;

    fd = open(fname, O_RDONLY);

    if(fd < 0)
        FatalError("can't open %s: %s\n", fname, pcap_strerror(errno));

    if(fstat(fd, &buf) < 0)
        FatalError("can't stat %s: %s\n", fname, pcap_strerror(errno));

    cp = (char *)SnortAlloc(((u_int)buf.st_size + 1) * sizeof(char));

    cc = read(fd, cp, (int) buf.st_size);

    if(cc < 0)
        FatalError("read %s: %s\n", fname, pcap_strerror(errno));

    if(cc != buf.st_size)
        FatalError("short read %s (%d != %d)\n", fname, cc, (int) buf.st_size);

    cp[(int) buf.st_size] = '\0';

    close(fd);

    /* Treat everything upto the end of the line as a space
     *  so that we can put comments in our BPF filters
     */
    
    while((cmt = strchr(cp, '#')) != NULL)
    {
        while (*cmt != '\r' && *cmt != '\n' && *cmt != '\0')
        {
            *cmt++ = ' ';
        }
    }

    /** LogMessage("BPF filter file: %s\n", fname); **/
    
    return(cp);
}


 /****************************************************************************
  *
  * Function: CheckLogDir()
  *
  * Purpose: CyberPsychotic sez: basically we only check if logdir exist and
  *          writable, since it might screw the whole thing in the middle. Any
  *          other checks could be performed here as well.
  *
  * Arguments: None.
  *
  * Returns: void function
  *
  ****************************************************************************/
void CheckLogDir(void)
{
    struct stat st;

    if (snort_conf->log_dir == NULL)
        return;

    if (stat(snort_conf->log_dir, &st) == -1)
        FatalError("Stat check on log dir failed: %s.\n", strerror(errno));

    if (!S_ISDIR(st.st_mode) || (access(snort_conf->log_dir, W_OK) == -1))
    {
        FatalError("Can not get write access to logging directory \"%s\". "
                   "(directory doesn't exist or permissions are set incorrectly "
                   "or it is not a directory at all)\n",
                   snort_conf->log_dir);
    }
}

/* Signal handler for child process signaling the parent
 * that is is ready */
static int parent_wait = 1;
static void SigChildReadyHandler(int signal)
{
#ifdef DEBUG
    LogMessage("Received Signal from Child\n");
#endif
    parent_wait = 0;
}

/****************************************************************************
 *
 * Function: GoDaemon()
 *
 * Purpose: Puts the program into daemon mode, nice and quiet like....
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void GoDaemon(void)
{
#ifndef WIN32
    int exit_val = 0;
    pid_t fs;

    LogMessage("Initializing daemon mode\n");

    if (ScDaemonRestart())
        return;

    /* Don't daemonize if we've already daemonized and
     * received a SIGHUP. */
    if(getppid() != 1)
    {
        /* Register signal handler that parent can trap signal */
        signal(SIGNAL_SNORT_CHILD_READY, SigChildReadyHandler);
        if (errno != 0) errno=0;

        /* now fork the child */
        fs = fork();

        if(fs > 0)
        {
            /* Parent */

            /* Don't exit quite yet.  Wait for the child
             * to signal that is there and created the PID
             * file.
             */
            while (parent_wait)
            {
                /* Continue waiting until receiving signal from child */
                int status;
                if (waitpid(fs, &status, WNOHANG) == fs)
                {
                    /* If the child is gone, parent should go away, too */
                    if (WIFEXITED(status))
                    {
                        LogMessage("Child exited unexpectedly\n");
                        exit_val = -1;
                        break;
                    }

                    if (WIFSIGNALED(status))
                    {
                        LogMessage("Child terminated unexpectedly\n");
                        exit_val = -2;
                        break;
                    }
                }
#ifdef DEBUG
                LogMessage("Parent waiting for child...\n");
#endif

                sleep(1);
            }

            LogMessage("Daemon parent exiting\n");

            exit(exit_val);                /* parent */
        }

        if(fs < 0)
        {
            /* Daemonizing failed... */
            perror("fork");
            exit(1);
        }

        /* Child */
        setsid();
    }

    close(0);
    close(1);
    close(2);

#ifdef DEBUG
    /* redirect stdin/stdout/stderr to a file */
    open("/tmp/snort.debug", O_CREAT | O_RDWR);  /* stdin, fd 0 */

    /* Change ownership to that which we will drop privileges to */
    if ((snort_conf->user_id != -1) || (snort_conf->group_id != -1))
    {
        uid_t user_id = getuid();
        gid_t group_id = getgid();

        if (snort_conf->user_id != -1)
            user_id = snort_conf->user_id;
        if (snort_conf->group_id != -1)
            group_id = snort_conf->group_id;

        chown("/tmp/snort.debug", user_id, group_id);
    }
#else
    /* redirect stdin/stdout/stderr to /dev/null */
    (void)open("/dev/null", O_RDWR);  /* stdin, fd 0 */
#endif

    dup(0);  /* stdout, fd 0 => fd 1 */
    dup(0);  /* stderr, fd 0 => fd 2 */

    SignalWaitingParent();

#endif /* ! WIN32 */
}

/* Signal the parent that child is ready */
void SignalWaitingParent(void)
{
#ifndef WIN32
    pid_t parentpid = getppid();
#ifdef DEBUG
    LogMessage("Signaling parent %d from child %d\n", parentpid, getpid());
#endif

    if (kill(parentpid, SIGNAL_SNORT_CHILD_READY))
    {
        LogMessage("Daemon initialized, failed to signal parent pid: %d, failure: %d, %s\n", parentpid, errno, strerror(errno));
    }
    else
    {
        LogMessage("Daemon initialized, signaled parent pid: %d\n", parentpid);
    }
#endif
}

/* This function has been moved into mstring.c, since that
*  is where the allocation actually occurs.  It has been
*  renamed to mSplitFree().
*
void FreeToks(char **toks, int num_toks)
{
    if (toks)
    {
        if (num_toks > 0)
        {
            do
            {
                num_toks--;
                free(toks[num_toks]);
            } while(num_toks);
        }
        free(toks);
    }
}
*/


/* Self preserving memory allocator */
void *SPAlloc(unsigned long size, struct _SPMemControl *spmc)
{
    void *tmp;

    spmc->mem_usage += size;

    if(spmc->mem_usage > spmc->memcap)
    {
        spmc->sp_func(spmc);
    }

    tmp = (void *) calloc(size, sizeof(char));

    if(tmp == NULL)
    {
        FatalError("Unable to allocate memory!  (%lu requested, %lu in use)\n",
                size, spmc->mem_usage);
    }

    return tmp;
}

/* Guaranteed to be '\0' terminated even if truncation occurs.
 *
 * returns  SNORT_SNPRINTF_SUCCESS if successful
 * returns  SNORT_SNPRINTF_TRUNCATION on truncation
 * returns  SNORT_SNPRINTF_ERROR on error
 */
int SnortSnprintf(char *buf, size_t buf_size, const char *format, ...)
{
    va_list ap;
    int ret;

    if (buf == NULL || buf_size <= 0 || format == NULL)
        return SNORT_SNPRINTF_ERROR;

    /* zero first byte in case an error occurs with
     * vsnprintf, so buffer is null terminated with
     * zero length */
    buf[0] = '\0';
    buf[buf_size - 1] = '\0';

    va_start(ap, format);

    ret = vsnprintf(buf, buf_size, format, ap);

    va_end(ap);

    if (ret < 0)
        return SNORT_SNPRINTF_ERROR;

    if (buf[buf_size - 1] != '\0' || (size_t)ret >= buf_size)
    {
        /* result was truncated */
        buf[buf_size - 1] = '\0';
        return SNORT_SNPRINTF_TRUNCATION;
    }

    return SNORT_SNPRINTF_SUCCESS;
}

/* Appends to a given string
 * Guaranteed to be '\0' terminated even if truncation occurs.
 * 
 * returns SNORT_SNPRINTF_SUCCESS if successful
 * returns SNORT_SNPRINTF_TRUNCATION on truncation
 * returns SNORT_SNPRINTF_ERROR on error
 */
int SnortSnprintfAppend(char *buf, size_t buf_size, const char *format, ...)
{
    int str_len;
    int ret;
    va_list ap;

    if (buf == NULL || buf_size <= 0 || format == NULL)
        return SNORT_SNPRINTF_ERROR;

    str_len = SnortStrnlen(buf, buf_size);

    /* since we've already checked buf and buf_size an error
     * indicates no null termination, so just start at
     * beginning of buffer */
    if (str_len == SNORT_STRNLEN_ERROR)
    {
        buf[0] = '\0';
        str_len = 0;
    }

    buf[buf_size - 1] = '\0';

    va_start(ap, format);

    ret = vsnprintf(buf + str_len, buf_size - (size_t)str_len, format, ap);

    va_end(ap);

    if (ret < 0)
        return SNORT_SNPRINTF_ERROR;

    if (buf[buf_size - 1] != '\0' || (size_t)ret >= buf_size)
    {
        /* truncation occured */
        buf[buf_size - 1] = '\0';
        return SNORT_SNPRINTF_TRUNCATION;
    }

    return SNORT_SNPRINTF_SUCCESS;
}

/* Guaranteed to be '\0' terminated even if truncation occurs.
 *
 * Arguments:  dst - the string to contain the copy
 *             src - the string to copy from
 *             dst_size - the size of the destination buffer
 *                        including the null byte.
 *
 * returns SNORT_STRNCPY_SUCCESS if successful
 * returns SNORT_STRNCPY_TRUNCATION on truncation
 * returns SNORT_STRNCPY_ERROR on error
 *
 * Note: Do not set dst[0] = '\0' on error since it's possible that
 * dst and src are the same pointer - it will at least be null
 * terminated in any case
 */
int SnortStrncpy(char *dst, const char *src, size_t dst_size)
{
    char *ret = NULL;

    if (dst == NULL || src == NULL || dst_size <= 0)
        return SNORT_STRNCPY_ERROR;

    dst[dst_size - 1] = '\0';

    ret = strncpy(dst, src, dst_size);

    /* Not sure if this ever happens but might as
     * well be on the safe side */
    if (ret == NULL)
        return SNORT_STRNCPY_ERROR;

    if (dst[dst_size - 1] != '\0')
    {
        /* result was truncated */
        dst[dst_size - 1] = '\0';
        return SNORT_STRNCPY_TRUNCATION;
    }

    return SNORT_STRNCPY_SUCCESS;
}

char *SnortStrndup(const char *src, size_t dst_size)
{
	char *ret = SnortAlloc(dst_size + 1);
    int ret_val;

	ret_val = SnortStrncpy(ret, src, dst_size + 1);

    if(ret_val == SNORT_STRNCPY_ERROR) 
	{
		free(ret);
		return NULL;
	}

	return ret;
}

/* Determines whether a buffer is '\0' terminated and returns the
 * string length if so
 *
 * returns the string length if '\0' terminated
 * returns SNORT_STRNLEN_ERROR if not '\0' terminated
 */
int SnortStrnlen(const char *buf, int buf_size)
{
    int i = 0;

    if (buf == NULL || buf_size <= 0)
        return SNORT_STRNLEN_ERROR;

    for (i = 0; i < buf_size; i++)
    {
        if (buf[i] == '\0')
            break;
    }

    if (i == buf_size)
        return SNORT_STRNLEN_ERROR;

    return i;
}

char * SnortStrdup(const char *str)
{
    char *copy = NULL;

    if (!str)
    {
        FatalError("Unable to duplicate string: NULL!\n");
    }

    copy = strdup(str);

    if (copy == NULL)
    {
        FatalError("Unable to duplicate string: %s!\n", str);
    }

    return copy;
}

/*
 * Find first occurrence of char of accept in s, limited by slen.
 * A 'safe' version of strpbrk that won't read past end of buffer s
 * in cases that s is not NULL terminated.
 *
 * This code assumes 'accept' is a static string.
 */
const char *SnortStrnPbrk(const char *s, int slen, const char *accept)
{
    char ch;
    const char *s_end;
    if (!s || !*s || !accept || slen == 0)
        return NULL;

    s_end = s + slen;
    while (s < s_end)
    {
        ch = *s;
        if (strchr(accept, ch))
            return s;
        s++;
    }
    return NULL;
}

/*
 * Find first occurrence of searchstr in s, limited by slen.
 * A 'safe' version of strstr that won't read past end of buffer s
 * in cases that s is not NULL terminated.
 */
const char *SnortStrnStr(const char *s, int slen, const char *searchstr)
{
    char ch, nc;
    int len;
    if (!s || !*s || !searchstr || slen == 0)
        return NULL;

    if ((ch = *searchstr++) != 0)
    {
        len = strlen(searchstr);
        do
        {
            do
            {
                if ((nc = *s++) == 0)
                {
                    return NULL;
                }
                slen--;
                if (slen == 0)
                    return NULL;
            } while (nc != ch);
            if (slen - len < 0)
                return NULL;
        } while (memcmp(s, searchstr, len) != 0);
        s--;
        slen++;
    }
    return s;
}

/*
 * Find first occurrence of substring in s, ignore case.
*/
const char *SnortStrcasestr(const char *s, const char *substr)
{
    char ch, nc;
    int len;

    if (!s || !*s || !substr)
        return NULL;

    if ((ch = *substr++) != 0)
    {
        ch = tolower((char)ch);
        len = strlen(substr);
        do
        {
            do
            {
                if ((nc = *s++) == 0)
                {
                    return NULL;
                }
            } while ((char)tolower((uint8_t)nc) != ch);
        } while (strncasecmp(s, substr, len) != 0);
        s--;
    }
    return s;
}

void *SnortAlloc(unsigned long size)
{
    void *tmp;

    tmp = (void *) calloc(size, sizeof(char));

    if(tmp == NULL)
    {
        FatalError("Unable to allocate memory!  (%lu requested)\n", size);
    }

    return tmp;
}

void * SnortAlloc2(size_t size, const char *format, ...)
{
    void *tmp;

    tmp = (void *)calloc(size, sizeof(char));

    if(tmp == NULL)
    {
        va_list ap;
        char buf[STD_BUF];

        buf[STD_BUF - 1] = '\0';

        va_start(ap, format);

        vsnprintf(buf, STD_BUF - 1, format, ap);

        va_end(ap);

        FatalError("%s", buf);
    }

    return tmp;
}

/** 
 * Chroot and adjust the snort_conf->log_dir reference 
 * 
 * @param directory directory to chroot to
 * @param logstore ptr to snort_conf->log_dir which must be dynamically allocated
 */
void SetChroot(char *directory, char **logstore)
{
#ifdef WIN32
    FatalError("SetChroot() should not be called under Win32!\n");
#else
    char *absdir;
    size_t abslen;
    char *logdir;
    
    if(!directory || !logstore)
    {
        FatalError("Null parameter passed\n");
    }

    logdir = *logstore;

    if(logdir == NULL || *logdir == '\0')
    {
        FatalError("Null log directory\n");
    }    

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"SetChroot: %s\n",
                                       CurrentWorkingDir()););
    
    logdir = GetAbsolutePath(logdir);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "SetChroot: %s\n",
                                       CurrentWorkingDir()));
    
    logdir = SnortStrdup(logdir);

    /* We're going to reset logstore, so free it now */
    free(*logstore);
    *logstore = NULL;

    /* change to the directory */
    if(chdir(directory) != 0)
    {
        FatalError("SetChroot: Can not chdir to \"%s\": %s\n", directory, 
                   strerror(errno));
    }

    /* always returns an absolute pathname */
    absdir = CurrentWorkingDir();

    if(absdir == NULL)                          
    {
        FatalError("NULL Chroot found\n");
    }
    
    abslen = strlen(absdir);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "ABS: %s %d\n", absdir, abslen););
    
    /* make the chroot call */
    if(chroot(absdir) < 0)
    {
        FatalError("Can not chroot to \"%s\": absolute: %s: %s\n",
                   directory, absdir, strerror(errno));
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"chroot success (%s ->", absdir););
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"%s)\n ", CurrentWorkingDir()););
    
    /* change to "/" in the new directory */
    if(chdir("/") < 0)
    {
        FatalError("Can not chdir to \"/\" after chroot: %s\n", 
                   strerror(errno));
    }    

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"chdir success (%s)\n",
                            CurrentWorkingDir()););


    if(strncmp(absdir, logdir, strlen(absdir)))
    {
        FatalError("Absdir is not a subset of the logdir");
    }
    
    if(abslen >= strlen(logdir))
    {
        *logstore = SnortStrdup("/");
    }
    else
    {
        *logstore = SnortStrdup(logdir + abslen);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"new logdir from %s to %s\n",
                            logdir, *logstore));

    LogMessage("Chroot directory = %s\n", directory);

#if 0
    /* XXX XXX */
    /* install the I can't do this signal handler */
    signal(SIGHUP, SigCantHupHandler);
#endif
#endif /* !WIN32 */
}


/**
 * Return a ptr to the absolute pathname of snort.  This memory must
 * be copied to another region if you wish to save it for later use.
 */
char *CurrentWorkingDir(void)
{
    static char buf[PATH_MAX_UTIL + 1];
    
    if(getcwd((char *) buf, PATH_MAX_UTIL) == NULL)
    {
        return NULL;
    }

    buf[PATH_MAX_UTIL] = '\0';

    return (char *) buf;
}

/**
 * Given a directory name, return a ptr to a static 
 */
char *GetAbsolutePath(char *dir)
{
    char *savedir, *dirp;
    static char buf[PATH_MAX_UTIL + 1];

    if(dir == NULL)
    {
        return NULL;
    }

    savedir = strdup(CurrentWorkingDir());

    if(savedir == NULL)
    {
        return NULL;
    }

    if(chdir(dir) < 0)
    {
        LogMessage("Can't change to directory: %s\n", dir);
        free(savedir);
        return NULL;
    }

    dirp = CurrentWorkingDir();

    if(dirp == NULL)
    {
        LogMessage("Unable to access current directory\n");
        free(savedir);
        return NULL;
    }
    else
    {
        strncpy(buf, dirp, PATH_MAX_UTIL);
        buf[PATH_MAX_UTIL] = '\0';
    }

    if(chdir(savedir) < 0)
    {
        LogMessage("Can't change back to directory: %s\n", dir);
        free(savedir);                
        return NULL;
    }

    free(savedir);
    return (char *) buf;
}


#ifndef WIN32
/* very slow sort - do not use at runtime! */
SF_LIST * SortDirectory(const char *path)
{
    SF_LIST *dir_entries;
    DIR *dir;
    struct dirent *direntry;
    int ret = 0;

    if (path == NULL)
        return NULL;

    dir_entries = sflist_new();
    if (dir_entries == NULL)
    {
        ErrorMessage("Could not allocate new list for directory entries\n");
        return NULL;
    }

    dir = opendir(path);
    if (dir == NULL)
    {
        ErrorMessage("Error opening directory: %s: %s\n",
                     path, strerror(errno));
        sflist_free_all(dir_entries, free);
        return NULL;
    }

    /* Reset errno since we'll be checking it unconditionally */
    errno = 0;

    while ((direntry = readdir(dir)) != NULL)
    {
        char *node_entry_name, *dir_entry_name;
        SF_LNODE *node;

        dir_entry_name = SnortStrdup(direntry->d_name);

        for (node = sflist_first_node(dir_entries);
             node != NULL;
             node = sflist_next_node(dir_entries))
        {
            node_entry_name = (char *)node->ndata;
            if (strcmp(dir_entry_name, node_entry_name) < 0)
                break;
        }

        if (node == NULL)
            ret = sflist_add_tail(dir_entries, (NODE_DATA)dir_entry_name);
        else
            ret = sflist_add_before(dir_entries, node, (NODE_DATA)dir_entry_name);

        if (ret == -1)
        {
            ErrorMessage("Error adding directory entry to list\n");
            sflist_free_all(dir_entries, free);
            closedir(dir);
            return NULL;
        }
    }

    if (errno != 0)
    {
        ErrorMessage("Error reading directory: %s: %s\n",
                     path, strerror(errno));
        errno = 0;
        sflist_free_all(dir_entries, free);
        closedir(dir);
        return NULL;
    }

    closedir(dir);

    return dir_entries;
}

int GetFilesUnderDir(const char *path, SF_QUEUE *dir_queue, const char *filter)
{
    SF_LIST *dir_entries;
    char *direntry;
    int ret = 0;
    int num_files = 0;

    if ((path == NULL) || (dir_queue == NULL))
        return -1;

    dir_entries = SortDirectory(path);
    if (dir_entries == NULL)
    {
        ErrorMessage("Error sorting entries in directory: %s\n", path);
        return -1;
    }

    for (direntry = (char *)sflist_first(dir_entries);
         direntry != NULL;
         direntry = (char *)sflist_next(dir_entries))
    {
        char path_buf[PATH_MAX];
        struct stat file_stat;

        /* Don't look at dot files */
        if (strncmp(".", direntry, 1) == 0)
            continue;
            
        ret = SnortSnprintf(path_buf, PATH_MAX, "%s%s%s",
                            path, path[strlen(path) - 1] == '/' ? "" : "/", direntry);
        if (ret == SNORT_SNPRINTF_TRUNCATION)
        {
            ErrorMessage("Error copying file to buffer: Path too long\n");
            sflist_free_all(dir_entries, free);
            return -1;
        }
        else if (ret != SNORT_SNPRINTF_SUCCESS)
        {
            ErrorMessage("Error copying file to buffer\n");
            sflist_free_all(dir_entries, free);
            return -1;
        }

        ret = stat(path_buf, &file_stat);
        if (ret == -1)
        {
            ErrorMessage("Could not stat file: %s: %s\n",
                         path_buf, strerror(errno));
            sflist_free_all(dir_entries, free);
            return -1;
        }

        if (file_stat.st_mode & S_IFDIR)
        {
            ret = GetFilesUnderDir(path_buf, dir_queue, filter);
            if (ret == -1)
            {
                sflist_free_all(dir_entries, free);
                return -1;
            }

            num_files += ret;
        }
        else if (file_stat.st_mode & S_IFREG)
        {
            if ((filter == NULL) || (fnmatch(filter, direntry, 0) == 0))
            {
                char *file = SnortStrdup(path_buf);

                ret = sfqueue_add(dir_queue, (NODE_DATA)file);
                if (ret == -1)
                {
                    ErrorMessage("Could not append item to list: %s\n", file);
                    free(file);
                    sflist_free_all(dir_entries, free);
                    return -1;
                }

                num_files++;
            }
        }
    }

    sflist_free_all(dir_entries, free);

    return num_files;
}
#endif

/****************************************************************************
 *
 * Function: GetUniqueName(char * iface)
 *
 * Purpose: To return a string that has a high probability of being unique
 *          for a given sensor.
 *
 * Arguments: char * iface - The network interface you are sniffing
 *
 * Returns: A char * -- its a static char * so you should not free it
 *
 ***************************************************************************/
char *GetUniqueName(char * iface)
{
    char * rptr;
    static char uniq_name[256];

    if (iface == NULL) LogMessage("Interface is NULL. Name may not be unique for the host\n");
#ifndef WIN32
    rptr = GetIP(iface); 
    if(rptr == NULL || !strcmp(rptr, "unknown"))
#endif
    {
        SnortSnprintf(uniq_name, 255, "%s:%s\n",GetHostname(),iface);
        rptr = uniq_name; 
    }
    if (ScLogVerbose()) LogMessage("Node unique name is: %s\n", rptr);
    return rptr;
}    

/****************************************************************************
 *
 * Function: GetIP(char * iface)
 *
 * Purpose: To return a string representing the IP address for an interface
 *
 * Arguments: char * iface - The network interface you want to find an IP
 *            address for.
 *
 * Returns: A char * -- make sure you call free on this when you are done
 *          with it.
 *
 ***************************************************************************/
char *GetIP(char * iface)
{
    struct ifreq ifr;
    struct sockaddr_in *addr;
    int s;
#ifdef SUP_IP6
    sfip_t ret;
#endif

    if(iface)
    {
        /* Set up a dummy socket just so we can use ioctl to find the
           ip address of the interface */
        s = socket(PF_INET, SOCK_DGRAM, 0);
        if(s == -1)
        {
            FatalError("Problem establishing socket to find IP address for interface: %s\n", iface);
        }

        SnortStrncpy(ifr.ifr_name, iface, strlen(iface) + 1);

#ifndef WIN32
        if(ioctl(s, SIOCGIFADDR, &ifr) < 0) return NULL;
        else
#endif
        {
            addr = (struct sockaddr_in *) &ifr.ifr_broadaddr;
        }
        close(s);

#ifdef SUP_IP6
// XXX-IPv6 uses ioctl to populate a sockaddr_in structure ... but what if the interface only has an IPv6 address?
        sfip_set_raw(&ret, addr, AF_INET); 
        return SnortStrdup(sfip_ntoa(&ret));
#else
        return SnortStrdup(inet_ntoa(addr->sin_addr));
#endif
    }
    else
    {
        return "unknown";
    }
}

/****************************************************************************
 *
 * Function: GetHostname()
 *
 * Purpose: To return a string representing the hostname
 *
 * Arguments: None
 *
 * Returns: A static char * representing the hostname. 
 *
 ***************************************************************************/
char *GetHostname(void)
{
#ifdef WIN32
    DWORD bufflen = 256;
    static char buff[256];
    GetComputerName(buff, &bufflen);
    return buff;
#else
    char * error = "unknown";
    if(getenv("HOSTNAME")) return getenv("HOSTNAME");
    else if(getenv("HOST")) return getenv("HOST");
    else return error;
#endif
}

/****************************************************************************
 *
 * Function: GetTimestamp(register const struct timeval *tvp, int tz)
 *
 * Purpose: Get an ISO-8601 formatted timestamp for tvp within the tz
 *          timezone. 
 *
 * Arguments: tvp is a timeval pointer. tz is a timezone. 
 *
 * Returns: char * -- You must free this char * when you are done with it.
 *
 ***************************************************************************/
char *GetTimestamp(register const struct timeval *tvp, int tz)
{
    struct tm *lt;  /* localtime */
    char * buf;
    int msec;

    buf = (char *)SnortAlloc(SMALLBUFFER * sizeof(char));

    msec = tvp->tv_usec / 1000;

    if (ScOutputUseUtc())
    {
        lt = gmtime((time_t *)&tvp->tv_sec);
        SnortSnprintf(buf, SMALLBUFFER, "%04i-%02i-%02i %02i:%02i:%02i.%03i",
                1900 + lt->tm_year, lt->tm_mon + 1, lt->tm_mday,
                lt->tm_hour, lt->tm_min, lt->tm_sec, msec);
    }
    else
    {
        lt = localtime((time_t *)&tvp->tv_sec);
        SnortSnprintf(buf, SMALLBUFFER,
                "%04i-%02i-%02i %02i:%02i:%02i.%03i+%03i",
                1900 + lt->tm_year, lt->tm_mon + 1, lt->tm_mday,
                lt->tm_hour, lt->tm_min, lt->tm_sec, msec, tz);
    }

    return buf;
}

/****************************************************************************
 *
 * Function: GetLocalTimezone()
 *
 * Purpose: Find the offset from GMT for current host
 *
 * Arguments: none 
 *
 * Returns: int representing the offset from GMT
 *
 ***************************************************************************/
int GetLocalTimezone(void)
{
    time_t      ut;
    struct tm * ltm;
    long        seconds_away_from_utc;

    time(&ut);
    ltm = localtime(&ut);

#if defined(WIN32) || defined(SOLARIS) || defined(AIX) || defined(HPUX)
    /* localtime() sets the global timezone variable,
       which is defined in <time.h> */
    seconds_away_from_utc = timezone;
#else
    seconds_away_from_utc = ltm->tm_gmtoff;
#endif

    return  seconds_away_from_utc/3600;
}

/****************************************************************************
 *
 * Function: GetCurrentTimestamp()
 *
 * Purpose: Generate an ISO-8601 formatted timestamp for the current time.
 *
 * Arguments: none 
 *
 * Returns: char * -- You must free this char * when you are done with it.
 *
 ***************************************************************************/
char *GetCurrentTimestamp(void)
{
    struct tm *lt;
    struct timezone tz;
    struct timeval tv;
    struct timeval *tvp;
    char * buf;
    int tzone;
    int msec;

    buf = (char *)SnortAlloc(SMALLBUFFER * sizeof(char));

    bzero((char *)&tz,sizeof(tz));
    gettimeofday(&tv,&tz);
    tvp = &tv;

    msec = tvp->tv_usec/1000;

    if (ScOutputUseUtc())
    {
        lt = gmtime((time_t *)&tvp->tv_sec);
        SnortSnprintf(buf, SMALLBUFFER, "%04i-%02i-%02i %02i:%02i:%02i.%03i",
                1900 + lt->tm_year, lt->tm_mon + 1, lt->tm_mday,
                lt->tm_hour, lt->tm_min, lt->tm_sec, msec);
    }
    else
    {
        lt = localtime((time_t *)&tvp->tv_sec);

        tzone = GetLocalTimezone();

        SnortSnprintf(buf, SMALLBUFFER,
                "%04i-%02i-%02i %02i:%02i:%02i.%03i+%03i",
                1900 + lt->tm_year, lt->tm_mon + 1, lt->tm_mday,
                lt->tm_hour, lt->tm_min, lt->tm_sec, msec, tzone);
    }

    return buf;
}

/****************************************************************************
 * Function: base64(char * xdata, int length)
 *
 * Purpose: Insert data into the database
 *
 * Arguments: xdata  => pointer to data to base64 encode
 *            length => how much data to encode 
 *
 * Make sure you allocate memory for the output before you pass
 * the output pointer into this function. You should allocate 
 * (1.5 * length) bytes to be safe.
 *
 * Returns: data base64 encoded as a char *
 *
 ***************************************************************************/
char * base64(const u_char * xdata, int length)
{
    int count, cols, bits, c, char_count;
    unsigned char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";  /* 64 bytes */
    char * payloadptr;
    char * output;
    char_count = 0;
    bits = 0;
    cols = 0;

    output = (char *)SnortAlloc( ((unsigned int) (length * 1.5 + 4)) * sizeof(char) );

    payloadptr = output;

    for(count = 0; count < length; count++)
    {
        c = xdata[count];

        if(c > 255)
        {
            ErrorMessage("plugbase.c->base64(): encountered char > 255 (decimal %d)\n If you see this error message a char is more than one byte on your machine\n This means your base64 results can not be trusted", c);
        }

        bits += c;
        char_count++;

        if(char_count == 3)
        {
            *output = alpha[bits >> 18]; output++;
            *output = alpha[(bits >> 12) & 0x3f]; output++;
            *output = alpha[(bits >> 6) & 0x3f]; output++;
            *output = alpha[bits & 0x3f]; output++; 
            cols += 4;
            if(cols == 72)
            {
                *output = '\n'; output++;
                cols = 0;
            }
            bits = 0;
            char_count = 0;
        }
        else
        {
            bits <<= 8;
        }
    }

    if(char_count != 0)
    {
        bits <<= 16 - (8 * char_count);
        *output = alpha[bits >> 18]; output++;
        *output = alpha[(bits >> 12) & 0x3f]; output++;
        if(char_count == 1)
        {
            *output = '='; output++;
            *output = '='; output++;
        }
        else
        {
            *output = alpha[(bits >> 6) & 0x3f]; 
            output++; *output = '='; 
            output++;
        }
    }
    *output = '\0';
    return payloadptr;
} 

/****************************************************************************
 *
 * Function: ascii(u_char *xdata, int length)
 *
 * Purpose: This function takes takes a buffer "xdata" and its length then
 *          returns a string of only the printable ASCII characters.
 *
 * Arguments: xdata is the buffer, length is the length of the buffer in
 *            bytes
 *
 * Returns: char * -- You must free this char * when you are done with it.
 *
 ***************************************************************************/
char *ascii(const u_char *xdata, int length)
{
     char *d_ptr, *ret_val;
     int i,count = 0;
     int size;
     
     if(xdata == NULL)
     {
         return NULL;         
     }
     
     for(i=0;i<length;i++)
     {
         if(xdata[i] == '<')
             count+=4;              /* &lt; */
         else if(xdata[i] == '&')
             count+=5;              /* &amp; */
         else if(xdata[i] == '>')   /* &gt;  */
             count += 4;
     }

     size = length + count + 1;
     ret_val = (char *) calloc(1,size);
     
     if(ret_val == NULL)
     {
         LogMessage("plugbase.c: ascii(): Out of memory, can't log anything!\n");
         return NULL;
     }
     
     d_ptr = ret_val; 
     
     for(i=0;i<length;i++)
     {
         if((xdata[i] > 0x1F) && (xdata[i] < 0x7F))
         {
             if(xdata[i] == '<')
             {
                 SnortStrncpy(d_ptr, "&lt;", size - (d_ptr - ret_val));
                 d_ptr+=4;
             }
             else if(xdata[i] == '&')
             {
                 SnortStrncpy(d_ptr, "&amp;", size - (d_ptr - ret_val));
                 d_ptr += 5;
             }
             else if(xdata[i] == '>')
             {
                 SnortStrncpy(d_ptr, "&gt;", size - (d_ptr - ret_val));
                 d_ptr += 4;
             }
             else
             {
                 *d_ptr++ = xdata[i];
             }
         }
         else
         {
             *d_ptr++ = '.';
         }        
     }
     
     *d_ptr++ = '\0';
     
     return ret_val;
}

/****************************************************************************
 *
 * Function: hex(u_char *xdata, int length)
 *
 * Purpose: This function takes takes a buffer "xdata" and its length then
 *          returns a string of hex with no spaces
 *
 * Arguments: xdata is the buffer, length is the length of the buffer in
 *            bytes
 *
 * Returns: char * -- You must free this char * when you are done with it.
 *
 ***************************************************************************/
char *hex(const u_char *xdata, int length)
{
    int x;
    char *rval = NULL;
    char *buf = NULL;

    if (xdata == NULL)
        return NULL;

    buf = (char *)calloc((length * 2) + 1, sizeof(char));

    if (buf != NULL)
    {
        rval = buf;

        for (x = 0; x < length; x++)
        {
            SnortSnprintf(buf, 3, "%02X", xdata[x]);
            buf += 2;
        } 

        rval[length * 2] = '\0';
    }

    return rval;
}



char *fasthex(const u_char *xdata, int length)
{
    char conv[] = "0123456789ABCDEF";
    char *retbuf = NULL; 
    const u_char *index;
    const u_char *end;
    char *ridx;

    index = xdata;
    end = xdata + length;
    retbuf = (char *)SnortAlloc(((length * 2) + 1) * sizeof(char));
    ridx = retbuf;

    while(index < end)
    {
        *ridx++ = conv[((*index & 0xFF)>>4)];
        *ridx++ = conv[((*index & 0xFF)&0x0F)];
        index++;
    }

    return retbuf;
}

/*
 *   Fatal Integer Parser
 *   Ascii to Integer conversion with fatal error support
 */
long int xatol(const char *s , const char *etext)
{
    long int val;
    char *endptr;
    char *default_error = "xatol() error\n";

    if (etext == NULL)
        etext = default_error;

    if (s == NULL)
        FatalError("%s: String is NULL\n", etext);

    while (isspace((int)*s))
        s++;

    if (strlen(s) == 0)
        FatalError("%s: String is empty\n", etext);


    /*
     *  strtoul - errors on win32 : ERANGE (VS 6.0)
     *            errors on linux : ERANGE, EINVAL
     *               (for EINVAL, unsupported base which won't happen here)
     */ 
    val = SnortStrtol(s, &endptr, 0);

    if ((errno == ERANGE) || (*endptr != '\0'))
        FatalError("%s: Invalid integer input: %s\n", etext, s);

    return val;
}

/*
 *   Fatal Integer Parser
 *   Ascii to Integer conversion with fatal error support
 */
unsigned long int xatou(const char *s , const char *etext)
{
    unsigned long int val;
    char *endptr;
    char *default_error = "xatou() error\n";

    if (etext == NULL)
        etext = default_error;

    if (s == NULL)
        FatalError("%s: String is NULL\n", etext);

    while (isspace((int)*s))
        s++;

    if (strlen(s) == 0)
        FatalError("%s: String is empty\n", etext);

    if (*s == '-') 
    {
        FatalError("%s: Invalid unsigned integer - negative sign found, "
                   "input: %s\n", etext, s);
    }


    /*
     *  strtoul - errors on win32 : ERANGE (VS 6.0)
     *            errors on linux : ERANGE, EINVAL
     */ 
    val = SnortStrtoul(s, &endptr, 0);

    if ((errno == ERANGE) || (*endptr != '\0'))
        FatalError("%s: Invalid integer input: %s\n", etext, s);

    return val;
}

unsigned long int xatoup(const char *s , const char *etext)
{
    unsigned long int val = xatou(s, etext);
    if ( !val ) 
        FatalError("%s: must be > 0\n", etext);
    return val;
}

#ifndef SUP_IP6
char * ObfuscateIpToText(const struct in_addr ip_addr)
#else
char * ObfuscateIpToText(sfip_t *ip)
#endif
{
    static char ip_buf1[INET6_ADDRSTRLEN];
    static char ip_buf2[INET6_ADDRSTRLEN];
    static int buf_num = 0;
    int buf_size = INET6_ADDRSTRLEN;
    char *ip_buf;
#ifndef SUP_IP6
    uint32_t ip = ip_addr.s_addr;
#endif

    if (buf_num)
        ip_buf = ip_buf2;
    else
        ip_buf = ip_buf1;

    buf_num ^= 1;
    ip_buf[0] = 0;

#ifndef SUP_IP6
    if (ip == 0)
        return ip_buf;

    if (snort_conf->obfuscation_net == 0)
    {
        /* Fully obfuscate - just use 'x' */
        SnortSnprintf(ip_buf, buf_size, "xxx.xxx.xxx.xxx");
    }
    else
    {
        if (snort_conf->homenet != 0)
        {
            if ((ip & snort_conf->netmask) == snort_conf->homenet)
                ip = snort_conf->obfuscation_net | (ip & snort_conf->obfuscation_mask);
        }
        else
        {
            ip = snort_conf->obfuscation_net | (ip & snort_conf->obfuscation_mask);
        }

        SnortSnprintf(ip_buf, buf_size, "%s", inet_ntoa(*((struct in_addr *)&ip)));
    }

#else
    if (ip == NULL)
        return ip_buf;

    if (!IS_SET(snort_conf->obfuscation_net))
    {
        if (IS_IP6(ip))
            SnortSnprintf(ip_buf, buf_size, "x:x:x:x::x:x:x:x");
        else
            SnortSnprintf(ip_buf, buf_size, "xxx.xxx.xxx.xxx");
    }
    else
    {
        sfip_t tmp;
        char *tmp_buf;

        IP_COPY_VALUE(tmp, ip);

        if (IS_SET(snort_conf->homenet))
        {
            if (sfip_contains(&snort_conf->homenet, &tmp) == SFIP_CONTAINS)
                sfip_obfuscate(&snort_conf->obfuscation_net, &tmp);
        }
        else
        {
            sfip_obfuscate(&snort_conf->obfuscation_net, &tmp);
        }

        tmp_buf = sfip_to_str(&tmp);
        SnortSnprintf(ip_buf, buf_size, "%s", tmp_buf);
    }
#endif

    return ip_buf;
}

void PrintPacketData(const uint8_t *data, const uint32_t len)
{
    uint32_t i, j;
    uint32_t total_len = 0;
    uint8_t hex_buf[16];
    uint8_t char_buf[16];
    char *length_chars = "       0  1  2  3  4  5  6  7   8  9 10 11 12 13 14 15\n"
                         "------------------------------------------------------\n";

    LogMessage("%s", length_chars);

    for (i = 0; i <= len; i++)
    {
        if ((i%16 == 0) && (i != 0))
        {
            LogMessage("%04x  ", total_len);
            total_len += 16;

            for (j = 0; j < 16; j++)
            {
                LogMessage("%02x ", hex_buf[j]);
                if (j == 7)
                    LogMessage(" ");
            }

            LogMessage(" ");

            for (j = 0; j < 16; j++)
            {
                LogMessage("%c", char_buf[j]);
                if (j == 7)
                    LogMessage(" ");
            }

            LogMessage("\n");
        }

        if (i == len)
            break;

        hex_buf[i%16] = data[i];

        if (isprint((int)data[i]))
            char_buf[i%16] = data[i];
        else
            char_buf[i%16] = '.';
    }

    if ((i-total_len) > 0)
    {
        LogMessage("%04x  ", total_len);

        for (j = 0; j < i-total_len; j++)
        {
            LogMessage("%02x ", hex_buf[j]);
            if (j == 7)
                LogMessage(" ");
        }

        if (j < 8)
            LogMessage(" ");
        LogMessage("%*s", (16-j)*3, "");
        LogMessage(" ");

        for (j = 0; j < i-total_len; j++)
        {
            LogMessage("%c", char_buf[j]);
            if (j == 7)
                LogMessage(" ");
        }
    }

    LogMessage("\n");
}

