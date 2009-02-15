/* Automatically generated by refactor.pl.
 *
 *
 * CMDNAME: suspenduser
 * CMDLEVEL: QCMD_OPER
 * CMDARGS: 1
 * CMDDESC: Suspend/Delay GLINE/Instantly GLINE a user.
 * CMDFUNC: csu_dosuspenduser
 * CMDPROTO: int csu_dosuspenduser(void *source, int cargc, char **cargv);
 * CMDHELP: Usage: suspenduser <username> [<duration>] <reason>
 * CMDHELP:                    -nokill <username> [<duration>] <reason>
 * CMDHELP:                    -gline <username> [<duration>] <reason>
 * CMDHELP:                    -instantgline <username> [<duration>] <reason>
 * CMDHELP:                    -password <password> [<duration>] <reason>
 * CMDHELP:                    -email <email address> [<duration>] <reason>
 * CMDHELP: Suspends one or more users, either based on username, email
 * CMDHELP: address or password.
 * CMDHELP: By default the user will be immediately disconnected unless
 * CMDHELP: nokill is specified.
 * CMDHELP: gline will gline the user at some random period of time after
 * CMDHELP: they auth, instantgline will gline them the moment they auth.
 */

#include "../chanserv.h"
#include "../../lib/irc_string.h"
#include <stdio.h>
#include <string.h>

static int killtheusers(nick *sender, reguser *rup) {
  int count=0;
  authname *anp;
  
  if (!(anp=findauthname(rup->ID)))
    return 0;
  
  while (anp->nicks) {
    chanservstdmessage(sender, QM_DISCONNECTINGUSER, anp->nicks->nick, rup->username);
    chanservkillstdmessage(anp->nicks, QM_SUSPENDKILL);
    count++;
  }
  
  return count;
}

int csu_dosuspenduser(void *source, int cargc, char **cargv) {
  nick *sender=source;
  reguser *rup=getreguserfromnick(sender);
  reguser *vrup;
  char *flag;
  char *victim;
  char *dur_p;
  char *reason;
  int kill=1, gline=0, email=0, password=0, hitcount=0;
  time_t expires=0;
  int duration=0;
  char expiresbuf[TIMELEN]="";
  int dgwait;
  
  if (!rup)
    return CMD_ERROR;
  
  if (cargc < 1) {
    chanservstdmessage(sender, QM_NOTENOUGHPARAMS, "suspenduser");
    return CMD_ERROR;
  }
  
  if (cargv[0][0] == '-') {
    flag=cargv[0];
    if (!(victim=strchr(flag, ' '))) {
      chanservstdmessage(sender, QM_NOTENOUGHPARAMS, "suspenduser");
      return CMD_ERROR;
    }
    *(victim++)='\0';
    if (!(dur_p=strchr(victim, ' '))) {
      chanservstdmessage(sender, QM_NOTENOUGHPARAMS, "suspenduser");
      return CMD_ERROR;
    }
    *(dur_p++)='\0';
    if ((reason=strchr(dur_p, ' '))) {
      *(reason++)='\0';
      if ((duration=durationtolong(dur_p))) {
        if ((duration < 86400) || (duration > 2592000)) {
          chanservstdmessage(sender, QM_INVALIDDURATION);
          return CMD_ERROR;
        }
        expires=time(0)+duration;
      }
      else {
        *(reason-1)=' ';
        reason=dur_p;
        expires=0;
      }
    }
    else {
      reason=dur_p;
      expires=0;
    }

    if (!ircd_strcmp(flag, "-nokill")) {
      kill=0;
    }
    else if (!ircd_strcmp(flag, "-gline")) {
      gline=1;
    }
    else if (!ircd_strcmp(flag, "-instantgline")) {
      gline=2;
    }
    else if (!ircd_strcmp(flag, "-email")) {
      email=1;
    }
    else if (!ircd_strcmp(flag, "-password")) {
      password=1;
    }
    else {
      chanservstdmessage(sender, QM_INVALIDCHANLEVCHANGE);
      return CMD_ERROR;
    }
  }
  else {
    victim=cargv[0];
    if (!(dur_p=strchr(victim, ' '))) {
      chanservstdmessage(sender, QM_NOTENOUGHPARAMS, "suspenduser");
      return CMD_ERROR;
    }
    *(dur_p++)='\0';
    if ((reason=strchr(dur_p, ' '))) {
      *(reason++)='\0';
      if ((duration=durationtolong(dur_p))) {
        if ((duration < 86400) || (duration > 2592000)) {
          chanservstdmessage(sender, QM_INVALIDDURATION);
          return CMD_ERROR;
        }
        expires=time(0)+duration;
      }
      else {
        *(reason-1)=' ';
        reason=dur_p;
        expires=0;
      }
    }
    else {
      reason=dur_p;
      expires=0;
    }
  }
  
  if(!checkreason(sender, reason))
    return CMD_ERROR;
    
  if (expires)
    q9strftime(expiresbuf,sizeof(expiresbuf),expires);
  
  if (email) {
    int i;
    
    for (i=0;i<REGUSERHASHSIZE;i++) {
      for (vrup=regusernicktable[i]; vrup; vrup=vrup->nextbyname) {
        if (!strcasecmp(vrup->email->content, victim)) {
          if (UHasSuspension(vrup))
            continue;
          
          if (UHasStaffPriv(vrup))
            continue;
          
          hitcount++;
          vrup->flags|=QUFLAG_SUSPENDED;
          vrup->suspendby=rup->ID;
          vrup->suspendexp=expires;
          vrup->suspendtime=time(NULL);
          vrup->suspendreason=getsstring(reason, strlen(reason)+1);
          
          killtheusers(sender,vrup);
          csdb_updateuser(vrup);
        }
      }
    }
    
    chanservwallmessage("%s (%s) bulk suspended <%s>, hit %d account/s (expires: %s), reason: %s", sender->nick, rup->username, victim, hitcount, expires?expiresbuf:"never", reason);
    cs_log(sender, "SUSPENDUSER (bulk by email: %s, hit: %d) expires: %s, reason: %s", victim, hitcount, expires?expiresbuf:"never", reason);
  }
  else if (password) {
    int i;
    
    for (i=0;i<REGUSERHASHSIZE;i++) {
      for (vrup=regusernicktable[i]; vrup; vrup=vrup->nextbyname) {
        if (!strcmp(vrup->password, victim)) {
          if (UHasSuspension(vrup))
            continue;
          
          if (UHasStaffPriv(vrup))
            continue;
          
          hitcount++;
          vrup->flags|=QUFLAG_SUSPENDED;
          vrup->suspendby=rup->ID;
          vrup->suspendexp=expires;
          vrup->suspendtime=time(NULL);
          vrup->suspendreason=getsstring(reason, strlen(reason)+1);
          
          killtheusers(sender,vrup);
          csdb_updateuser(vrup);
        }
      }
    }
    
    chanservwallmessage("%s (%s) bulk suspended password \"%s\", hit %d account/s (expires: %s), reason: %s", sender->nick, rup->username, victim, hitcount, expires?expiresbuf:"never", reason);
    cs_log(sender, "SUSPENDUSER (bulk by password: %s, hit: %d) expires: %s, reason: %s", victim, hitcount, expires?expiresbuf:"never", reason);
  }
  else {
    if (!(vrup=findreguser(sender, victim)))
      return CMD_ERROR;
    
    if (!ircd_strcmp(vrup->username, rup->username)) {
      chanservsendmessage(sender, "You can't suspend yourself, silly.");
      return CMD_ERROR;
    }
    
    if (UHasSuspension(vrup)) {
      chanservstdmessage(sender, QM_USERALREADYSUSPENDED);
      return CMD_ERROR;
    }
    
    if (UHasStaffPriv(vrup)) {
      char buf[200];
      snprintf(buf, sizeof(buf), "suspenduser on %s (reason: %s)", vrup->username, reason);
      chanservstdmessage(sender, QM_NOACCESS, buf);
      chanservwallmessage("%s (%s) FAILED to suspend %s (reason: %s)", sender->nick, rup->username, vrup->username, reason);
      return CMD_ERROR;
    }
    
    if (gline == 2)
      vrup->flags|=QUFLAG_GLINE;
    else if (gline == 1)
      vrup->flags|=QUFLAG_DELAYEDGLINE;
    else
      vrup->flags|=QUFLAG_SUSPENDED;
    vrup->suspendby=rup->ID;
    vrup->suspendexp=expires;
    vrup->suspendtime=time(NULL);
    vrup->suspendreason=getsstring(reason, strlen(reason)+1);
    
    chanservwallmessage("%s (%s) %s %s (expires: %s), reason: %s", sender->nick, rup->username, (gline)?((gline == 2)?"instantly glined":"delayed glined"):"suspended", vrup->username, expires?expiresbuf:"never", reason);
    cs_log(sender, "SUSPENDUSER %s %s (expires: %s), reason: %s", (gline)?((gline == 2)?"instantly glined":"delayed glined"):"suspended", vrup->username, expires?expiresbuf:"never", reason);

    if (gline) {
      dgwait=(gline==2)?0:rand()%900;
      chanservsendmessage(sender, "Scheduling delayed GLINE for account %s in %d %s", 
        vrup->username, (dgwait>60)?(dgwait/60):dgwait, (dgwait>60)?"minutes":"seconds");
      deleteschedule(NULL, &chanservdgline, (void*)vrup);
      scheduleoneshot(time(NULL)+dgwait, &chanservdgline, (void*)vrup);
    }
    else if (kill) {
      hitcount += killtheusers(sender,vrup);
    }

    csdb_updateuser(vrup);
  }
  
  return CMD_OK;
}
