/*
*
* Copyright (c) 2006  Antonio Benojar <zz.stalker@gmail.com>
* Copyright (c) 2005  Antonio Benojar <zz.stalker@gmail.com>
*
* Copyright (c) 2003, 2004 Armin Wolfermann:
* 
* s2c_pf_block and s2c_pf_unblock functions are based 
* in Armin's Wolfermann pftabled-1.03 functions.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/* 
 TODO
 
 - num. max ips.
 - ipwhitelisting structure
 - best ip regex expr
*/


#ifndef LIST_END
#define LIST_END(head) NULL
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "event.h"
#include "decode.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "debug.h"
#include "parser.h"
#include "util.h"
#include "log.h"
#include "mstring.h"

#include "snort.h"

#include "spo_pf.h"

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
                        
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <ctype.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/pfvar.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include <unistd.h>
#include <regex.h>

#define PFDEVICE        "/dev/pf"

typedef struct _SpoAlertPfData {
    FILE *wlfile; 
    char *pftable; 
    int fd;
    struct wlist_head head;
} SpoAlertPfData;

void AlertPfInit(u_char *);
SpoAlertPfData *ParseAlertPfArgs(char *);
void AlertPf(Packet *, char *, void *, Event *);
void AlertPfCleanExit(int, void *);
void AlertPfRestart(int, void *);

int s2c_pf_init(void);
int s2c_pf_block(int, char *, char *, int);
int s2c_pf_intbl(int, char *, int);

int s2c_parse_line(char *, FILE*);
int s2c_parse_load_wl(FILE*, struct wlist_head*, int);
int s2c_parse_search_wl(char *, struct wlist_head);
int s2c_parse_free_wl(struct wlist_head*);
int s2c_parse_ip(char *, char *, int); 

 
void AlertPfSetup(void)
{
    RegisterOutputPlugin("alert_pf", OUTPUT_TYPE_FLAG__ALERT, AlertPfInit);
    
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output plugin: AlertPf is setup...\n"););
}

void AlertPfInit(u_char *args)
{
    SpoAlertPfData *data;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output: AlertPf Initialized\n"););

    data = ParseAlertPfArgs(args);
    		
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Linking AlertPf functions to call lists...\n"););

    AddFuncToOutputList(AlertPf, OUTPUT_TYPE_FLAG__ALERT, data);
    AddFuncToCleanExitList(AlertPfCleanExit, data);
    AddFuncToRestartList(AlertPfRestart, data);
}


void AlertPf(Packet *p, char *msg, void *arg, Event *event)
{
    SpoAlertPfData *data = (SpoAlertPfData *)arg;
    char *ip;
    int ret;
    
    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "spoink block'n!!\n"););
    
    ip = inet_ntoa(p->iph->ip_src);
    
    if (ip == NULL)
    	FatalError("AlertPf() => inet_ntoa() = NULL\n", strerror(errno));

   ret = s2c_parse_search_wl(ip, data->head);
   
   if (ret == 0) 
    	s2c_pf_block(data->fd, data->pftable, inet_ntoa(p->iph->ip_src), 0);
   
    return;
}

SpoAlertPfData *ParseAlertPfArgs(char *args)
{
    char **toks;
    int num_toks;
    SpoAlertPfData *data;
    
    int res = 0;
    
    data = (SpoAlertPfData *)SnortAlloc(sizeof(SpoAlertPfData));
    
    if(args == NULL) 
	FatalError("Unable to load pf args\n", strerror(errno));    

    data->fd = s2c_pf_init();

    if (data->fd == -1)
    	FatalError("s2c_pf_init() => no pf device\n");
    
    DEBUG_WRAP(DebugMessage(DEBUG_LOG,"ParseAlertPfArgs: %s\n", args););

    toks = mSplit(args, ",", 2, &num_toks, 0);
    
    if(num_toks <= 1) 
        FatalError("snort.conf => You must supply TWO arguments for the pf plugin...\n", strerror(errno));

    if(strstr(toks[0], "..") != NULL) 
        FatalError("snort.conf => File definition contains \"..\".  Do not do that!\n");

    data->wlfile = fopen(toks[0], "r");
    
    if (data->wlfile == NULL)
	FatalError("snort.conf => Unable to open whitelist file\n", strerror(errno));
	    	
    if (toks[1] == NULL)
     	FatalError("snort.conf => No pf table defined\n", strerror(errno));                
    else
     	data->pftable = toks[1];

    if (s2c_pf_intbl(data->fd, data->pftable, 0) == 0)
    	FatalError("pf.conf => Table %s don't exists in packet filter\n", data->pftable, strerror(errno));	
    
    res = s2c_parse_load_wl(data->wlfile, &data->head, 0);
    if (res == -1)
    	FatalError("snort.conf => Unable to load whitelist\n", strerror(errno)); 
    
    return data;
}

void AlertPfCleanExit(int signal, void *arg)
{
    SpoAlertPfData *data = (SpoAlertPfData *)arg;
    DEBUG_WRAP(DebugMessage(DEBUG_LOG,"AlertPfCleanExit\n"););
 
    s2c_parse_free_wl(&data->head);
    fclose(data->wlfile);
    close(data->fd);
    
    free(data);
}

void AlertPfRestart(int signal, void *arg)
{
    SpoAlertPfData *data = (SpoAlertPfData *)arg;
    DEBUG_WRAP(DebugMessage(DEBUG_LOG,"AlertPfRestart\n"););
    
    s2c_parse_free_wl(&data->head);
    fclose(data->wlfile);
    close(data->fd);
    
    free(data);
}


int s2c_pf_init(void)
{
	return(open(PFDEVICE, O_RDWR));
}

int  s2c_pf_block(int dev, char *tablename, char *ip, int debug) 
{ 

	struct pfioc_table io; 
    	struct pfr_table table; 
      	struct pfr_addr addr; 
      	struct in_addr *net_addr=NULL;
      
        memset(&io,    0x00, sizeof(struct pfioc_table)); 
        memset(&table, 0x00, sizeof(struct pfr_table)); 
        memset(&addr,  0x00, sizeof(struct pfr_addr)); 
            
        strlcpy(table.pfrt_name, tablename, PF_TABLE_NAME_SIZE); 
	net_addr=(struct in_addr*)malloc(sizeof(struct in_addr));                
        
        if (net_addr == NULL ) 
        	FatalError("s2c_pf_block() => malloc()\n", strerror(errno)); 
        
        inet_aton(ip, (struct in_addr *)&net_addr);
        memcpy(&addr.pfra_ip4addr.s_addr, &net_addr, sizeof(struct in_addr));
        
        addr.pfra_af  = AF_INET; 
        addr.pfra_net = 32; 
        
        io.pfrio_table  = table; 
        io.pfrio_buffer = &addr; 
        io.pfrio_esize  = sizeof(struct pfr_addr); 
        io.pfrio_size   = 1; 
        
        if (ioctl(dev, DIOCRADDADDRS, &io)) 
		FatalError("s2c_pf_block() => ioctl() DIOCRADDADDRS\n", strerror(errno));
 
        return(0); 
}

int  s2c_pf_intbl(int dev, char * tablename, int debug)
{
	int i;
	struct pfioc_table io;
	struct pfr_table *table_aux = NULL;
	
	memset(&io, 0x00, sizeof(struct pfioc_table));
	
	io.pfrio_buffer = table_aux;
	io.pfrio_esize  = sizeof(struct pfr_table);
	io.pfrio_size   = 0;
	
	if(ioctl(dev, DIOCRGETTABLES, &io))  
		FatalError("s2c_pf_intbl() => ioctl() DIOCRGETTABLES\n", strerror(errno));
	
	table_aux = (struct pfr_table*)malloc(sizeof(struct pfr_table)*io.pfrio_size);
	
	if (table_aux == NULL) 
		FatalError("s2c_pf_intbl() => malloc()\n", strerror(errno));
	
	io.pfrio_buffer = table_aux;
	io.pfrio_esize = sizeof(struct pfr_table);
	
	if(ioctl(dev, DIOCRGETTABLES, &io)) 
		FatalError("s2c_pf_intbl() => ioctl() DIOCRGETTABLES\n", strerror(errno));

	for(i=0; i< io.pfrio_size; i++) {
		if (!strcmp(table_aux[i].pfrt_name, tablename))
			return 1;	
	}
	
	return 0;

}


int s2c_parse_line(char buf[WLMAX] , FILE* wfile)
{
	static char     next_ch = ' ';
        int             i = 0;
        
	if (feof(wfile)) {
	        return (0);
	}                                
	do {
		next_ch = fgetc(wfile);
		if (i < WLMAX)
	        	buf[i++] = next_ch;
	} while (!feof(wfile) && !isspace(next_ch));
	if (i >= WLMAX) {
		return (-1);
	}		                 
	
	buf[i] = '\0';
	return (1);
}


int s2c_parse_load_wl(FILE *wfile, struct wlist_head *head, int debug)
{

	char cad[WLMAX];
	char ret[WLMAX];
	struct ipwlist *ipw2, *ipw1 = NULL;
	struct flock lock;
	
	if (wfile == NULL)
		FatalError("s2c_parse_load_wl() => Unable to open whitelist file\n", strerror(errno));
	
	memset(&lock, 0x00, sizeof(struct flock));
	lock.l_type = F_RDLCK;
	fcntl(fileno(wfile), F_SETLKW, &lock);
	
	LIST_INIT(head);
	
	if (s2c_parse_line(cad, wfile) == 1) {
		if (s2c_parse_ip(cad, ret, debug) == 1) {
			ipw1 = (struct ipwlist*)malloc(sizeof(struct ipwlist));	
			if (ipw1 == NULL) 
				FatalError("s2c_parse_load_wl() => malloc()\n", strerror(errno));
			inet_aton(ret, &ipw1->waddr);
			LIST_INSERT_HEAD(head, ipw1, elem);		
			
		} else {
			FatalError("s2c_parse_load_wl() => Invalid data in whitelist file\n", strerror(errno));
		}
	}
	
	while(s2c_parse_line(cad, wfile) == 1) {
		if (s2c_parse_ip(cad, ret, debug) == 1) {
			ipw2 = (struct ipwlist*)malloc(sizeof(struct ipwlist));
			if (ipw2 == NULL) 
				FatalError("s2c_parse_load_wl() => malloc()\n", strerror(errno));
			inet_aton(ret, &ipw2->waddr);
			LIST_INSERT_AFTER(ipw1, ipw2, elem);		
			ipw1 = ipw2;
		} else {
			break;
		}
			
	}
	
	lock.l_type = F_UNLCK;
	fcntl(fileno(wfile), F_SETLKW, &lock);
	
	return (0);
}

/* XXX: optimize  */

int
s2c_parse_search_wl(char *ip, struct wlist_head wl)
{
	struct ipwlist *aux2;	
	char *ip_aux, ip1[IPMAX], ip2[IPMAX];
	int ret;
	
	strlcpy(ip1, ip, sizeof(ip1));
	
	for(aux2=wl.lh_first; aux2 !=NULL; aux2=aux2->elem.le_next) {
		ip_aux = inet_ntoa(aux2->waddr);
		strlcpy(ip2, ip_aux, sizeof(ip2));
		ret = strcmp(ip1, ip2);
	
		if (ret == 0)
			return 1;
	}
	return (0);
}


int s2c_parse_free_wl(struct wlist_head *wl)
{
	struct ipwlist *aux, *aux2;
	for(aux = LIST_FIRST(wl); aux != LIST_END(wl); aux = aux2) {
		aux2 = LIST_NEXT(aux, elem);
		LIST_REMOVE(aux, elem);		
		free(aux);
	}
	if (LIST_EMPTY(wl)) { 
		return (1);
	} else { 
		FatalError("s2c_parse_free_wl() => Unable to free whitelist\n", strerror(errno));
		return (0);
	}
}

/* XXX: too much complex ? */

int s2c_parse_ip(char *cad, char ret[WLMAX], int debug)
{
	int len;
	unsigned int enc=1;
	regex_t *expr;
	regmatch_t *resultado;
	expr = (regex_t*)malloc(sizeof(regex_t));	
	
	bzero(ret, WLMAX);
	
	if (expr == NULL) 
		FatalError("s2c_parse_ip() => malloc()\n", strerror(errno));
	
	resultado = (regmatch_t*)malloc(sizeof(regmatch_t));
	
	if (resultado == NULL) 
		FatalError("s2c_parse_ip() => malloc()\n", strerror(errno));
	
	if (regcomp(expr, REG_ADDR, REG_EXTENDED) !=0) 
		FatalError("s2c_parse_ip() => regcomp()\n", strerror(errno));
	
	if (regexec(expr, cad, 1, resultado, 0) !=0) 
		enc=0;
	
	if (enc !=0) {
		len = resultado->rm_eo - resultado->rm_so;
		memcpy(ret, cad + resultado->rm_so, len);
		ret[len]='\0';
	}
	
	free(resultado);
	regfree(expr);
	
	if(enc)
		return (1);
	else {
		errno = EINVAL;
		return (0);
	}
}
