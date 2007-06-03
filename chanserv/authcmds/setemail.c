/* Automatically generated by refactor.pl.
 *
 *
 * CMDNAME: setemail
 * CMDLEVEL: QCMD_OPER
 * CMDARGS: 2
 * CMDDESC: Set the email address.
 * CMDFUNC: csa_dosetmail
 * CMDPROTO: int csa_dosetmail(void *source, int cargc, char **cargv);
 */

#include "../chanserv.h"
#include "../authlib.h"
#include "../../lib/irc_string.h"
#include <stdio.h>
#include <string.h>

int csa_dosetmail(void *source, int cargc, char **cargv) {
  reguser *rup;
  nick *sender=source;
  char *dupemail;

  if (cargc<2) {
    chanservstdmessage(sender, QM_NOTENOUGHPARAMS, "setemail");
    return CMD_ERROR;
  }

  if (!(rup=findreguser(sender, cargv[0])))
    return CMD_ERROR;

  if (csa_checkeboy(sender, cargv[1]))
    return CMD_ERROR;

  csdb_accounthistory_insert(sender, NULL, NULL, rup->email, getsstring(cargv[1], EMAILLEN));
  delreguserfrommaildomain(rup,rup->domain);
  freesstring(rup->email);
  rup->email=getsstring(cargv[1],EMAILLEN);
  rup->lastemailchange=time(NULL);
  rup->domain=findorcreatemaildomain(rup->email->content);
  addregusertomaildomain(rup, rup->domain);
  dupemail = strdup(rup->email->content);
  if(local=strchr(dupemail, '@')) {
    *(local++)='\0';
    rup->localpart=getsstring(local,EMAILLEN);
  } else {
    rup->localpart=NULL;
  }
  free(dupemail);

  chanservstdmessage(sender, QM_EMAILCHANGED, cargv[1]);
  cs_log(sender,"SETEMAIL OK username %s <%s>",rup->username,rup->email->content);
  csdb_updateuser(rup);

  return CMD_OK;
}
