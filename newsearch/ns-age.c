/*
 * age functionality
 */

#include "../irc/irc.h"
#include "newsearch.h"

#include <stdio.h>
#include <stdlib.h>

void *age_exe(searchCtx *ctx, struct searchNode *thenode, void *theinput);
void age_free(searchCtx *ctx, struct searchNode *thenode);

struct searchNode *age_parse(searchCtx *ctx, int argc, char **argv) {
  struct searchNode *thenode;

  if (!(thenode=(struct searchNode *)malloc(sizeof (struct searchNode)))) {
    parseError = "malloc: could not allocate memory for this search.";
    return NULL;
  }

  thenode->returntype = RETURNTYPE_INT;
  thenode->localdata = NULL;
  thenode->exe = age_exe;
  thenode->free = age_free;

  return thenode;
}

void *age_exe(searchCtx *ctx, struct searchNode *thenode, void *theinput) {
  nick *np = (nick *)theinput;
  whowas *ww;
  time_t ts;

  if (ctx->searchcmd == reg_nicksearch)
    ts = np->timestamp;
  else {
    ww = (whowas *)np->next;
    ts = ww->timestamp;
  }

  return (void *)(getnettime() - ts);
}

void age_free(searchCtx *ctx, struct searchNode *thenode) {
  free(thenode);
}

