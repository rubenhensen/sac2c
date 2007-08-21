/* $Id$ */

#include "strip_external_signatures.h"

#include "dbug.h"
#include "free.h"
#include "tree_basic.h"

static node *
StripArgs (node *args)
{
    DBUG_ENTER ("StripArgs");

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
    DBUG_ENTER ("StripRets");

    if (rets != NULL) {
        RET_NEXT (rets) = StripRets (RET_NEXT (rets));

        if (RET_ISARTIFICIAL (rets)) {
            rets = FREEdoFreeNode (rets);
        }
    }

    DBUG_RETURN (rets);
}

node *
SESstripOneFunction (node *fundef)
{
    DBUG_ENTER ("SESstripOneFunction");

    FUNDEF_ARGS (fundef) = StripArgs (FUNDEF_ARGS (fundef));
    FUNDEF_RETS (fundef) = StripRets (FUNDEF_RETS (fundef));

    DBUG_RETURN (fundef);
}

static node *
StripFuns (node *funs)
{
    DBUG_ENTER ("StripFuns");

    if (funs != NULL) {
        funs = SESstripOneFunction (funs);
        FUNDEF_NEXT (funs) = StripFuns (FUNDEF_NEXT (funs));
    }

    DBUG_RETURN (funs);
}

node *
SESdoStripExternalSignatures (node *syntax_tree)
{
    DBUG_ENTER ("SESdoStripExternalSignatures");

    if (MODULE_FUNS (syntax_tree) != NULL) {
        MODULE_FUNS (syntax_tree) = StripFuns (MODULE_FUNS (syntax_tree));
    }

    if (MODULE_FUNDECS (syntax_tree) != NULL) {
        MODULE_FUNDECS (syntax_tree) = StripFuns (MODULE_FUNDECS (syntax_tree));
    }

    DBUG_RETURN (syntax_tree);
}
