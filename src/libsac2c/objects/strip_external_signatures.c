/* $Id$ */

#include "strip_external_signatures.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "free.h"
#include "map_fun_trav.h"
#include "tree_basic.h"

static node *
StripArgs (node *args)
{
    DBUG_ENTER ();

    if (args != NULL) {
        ARG_NEXT (args) = StripArgs (ARG_NEXT (args));

        if (ARG_ISARTIFICIAL (args)) {
            args = FREEdoFreeNode (args);
        } else if (ARG_WASREFERENCE (args)) {
            ARG_WASREFERENCE (args) = FALSE;
            ARG_ISREFERENCE (args) = TRUE;
        }
    }

    DBUG_RETURN (args);
}

static node *
StripRets (node *rets)
{
    DBUG_ENTER ();

    if (rets != NULL) {
        RET_NEXT (rets) = StripRets (RET_NEXT (rets));

        if (RET_ISARTIFICIAL (rets)) {
            rets = FREEdoFreeNode (rets);
        }
    }

    DBUG_RETURN (rets);
}

static node *
StripFunction (node *fundef, info *info)
{
    DBUG_ENTER ();

    FUNDEF_ARGS (fundef) = StripArgs (FUNDEF_ARGS (fundef));
    FUNDEF_RETS (fundef) = StripRets (FUNDEF_RETS (fundef));

    DBUG_RETURN (fundef);
}

node *
SESstripOneFunction (node *fundef)
{
    DBUG_ENTER ();

    fundef = StripFunction (fundef, NULL);

    DBUG_RETURN (fundef);
}

node *
SESdoStripExternalSignatures (node *syntax_tree)
{
    DBUG_ENTER ();

    MFTdoMapFunTrav (MODULE_FUNS (syntax_tree), NULL, StripFunction);
    MFTdoMapFunTrav (MODULE_FUNDECS (syntax_tree), NULL, StripFunction);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
