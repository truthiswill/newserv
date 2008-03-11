/* Automatically generated by refactor.pl.
 *
 *
 * CMDNAME: getpassword
 * CMDLEVEL: QCMD_OPER
 * CMDARGS: 2
 * CMDDESC: Gets a users password
 * CMDFUNC: csa_dogetpw
 * CMDPROTO: int csa_dogetpw(void *source, int cargc, char **cargv);
 * CMDHELP: Usage: getpassword <username>
 * CMDHELP: Fetches the password for the specified username.
 */

#include "../chanserv.h"
#include "../authlib.h"
#include "../../lib/irc_string.h"
#include <stdio.h>
#include <string.h>

int csa_dogetpw(void *source, int cargc, char **cargv) {
  reguser *rup;
  nick *sender=source;
  reguser *srup=getreguserfromnick(sender);

  if (cargc<1) {
    chanservstdmessage(sender, QM_NOTENOUGHPARAMS, "getpassword");
    return CMD_ERROR;
  }

  if (!(rup=findreguser(sender, cargv[0])))
    return CMD_ERROR;

  if(UHasHelperPriv(rup)) {
    cs_log(sender,"GETPASSWORD FAILED username %s",rup->username);
    chanservwallmessage("%s (%s) just FAILED using GETPASSWORD on %s", sender->nick, srup->username, rup->username);
    chanservsendmessage(sender, "Sorry, that user is privileged.");
    return CMD_ERROR;
  }

  cs_log(sender,"GETPASSWORD OK username %s",rup->username);
  chanservwallmessage("%s (%s) just used GETPASSWORD on %s", sender->nick, srup->username, rup->username);

  chanservsendmessage(sender, "Password is currently: %s",rup->password);
  return CMD_OK;
}
