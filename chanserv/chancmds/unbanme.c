/* Automatically generated by refactor.pl.
 *
 *
 * CMDNAME: unbanme
 * CMDLEVEL: QCMD_AUTHED
 * CMDARGS: 1
 * CMDDESC: Removes any bans affecting you from a channel.
 * CMDFUNC: csc_dounbanme
 * CMDPROTO: int csc_dounbanme(void *source, int cargc, char **cargv);
 */

#include "../chanserv.h"
#include "../../nick/nick.h"
#include "../../lib/flags.h"
#include "../../lib/irc_string.h"
#include "../../channel/channel.h"
#include "../../parser/parser.h"
#include "../../irc/irc.h"
#include "../../localuser/localuserchannel.h"
#include <string.h>
#include <stdio.h>

int csc_dounbanme(void *source, int cargc, char **cargv) {
  nick *sender=source;
  regchan *rcp;
  chanindex *cip;
  modechanges changes;
  chanban **cbh;

  if (cargc<1) {
    chanservstdmessage(sender, QM_NOTENOUGHPARAMS, "unbanme");
    return CMD_ERROR;
  }

  if (!(cip=cs_checkaccess(sender, cargv[0], CA_OPPRIV, NULL, "unbanme", 0, 0)))
    return CMD_ERROR;

  rcp=cip->exts[chanservext];

  if (cip->channel) {
    localsetmodeinit(&changes, cip->channel, chanservnick);

    for (cbh=&(cip->channel->bans);*cbh;) {
      if (nickmatchban(sender, *cbh))
	localdosetmode_ban(&changes, bantostring(*cbh), MCB_DEL);
      else
	cbh=&((*cbh)->next);
    }

    localsetmodeflush(&changes, 1);
  }

  cs_log(sender,"UNBANME %s",cip->name->content);
  chanservstdmessage(sender, QM_DONE);
  return CMD_OK;
}     