/*
 *
 * $Log$
 * Revision 1.7  2002/10/18 13:35:22  sbs
 * Now, this phase also detects all references to global objects and marks the according
 * N_id nodes as IS_GLOBAL.
 *
 * Revision 1.6  2002/04/29 14:30:21  sbs
 * Now, identifier are copied when creating new vardecs 8-))))
 *
 * Revision 1.5  2002/03/12 15:11:12  sbs
 * dummy vardecs now have a T_unknown rather than a null type pointer.
 *
 * Revision 1.4  2002/02/22 14:07:45  sbs
 * access macros to INFO nodes moved into tree_basic
 *
 * Revision 1.3  2002/02/22 12:30:46  sbs
 * insvd_tab moved into traverse.c
 *
 * Revision 1.2  2002/02/22 09:26:19  sbs
 * INSVDwithid added .
 *
 * Revision 1.1  2002/02/21 15:12:36  sbs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   insert_vardec.c
 *
 * prefix: INSVD
 *
 * description:
 *
 *   This compiler module inserts vardecs for all identifyers that do not have
 *   one yet and adds proper backrefs to them.
 *   It is needed for running InferDFMS prior to type checking.
 *
 *   For all other identifyers the backref is set to the according N_vardec,
 *   Narg, or N_objdef node !!!
 *
 *   While doing so, all references to global objects are tagged as such, i.e.,
 *   the    IS_GLOBAL flag of N_id nodes   is set accordingly(!)  , and the name of
 *   N_id nodes that refer to global objects is extended by the module name of the
 *   object.
 *
 *
 *
 * usage of arg_info (INFO_INSVD_...):
 *
 *   ...OBJDEFS  holds the pointer to the objdefs chain of the actual module !
 *   ...VARDECS  holds the pointer to the vardec chain of the actual fundef !
 *   ...ARGS     holds the pointer to the argument chain of the actual fundef !
 *
 *****************************************************************************/

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"

#include "insert_vardec.h"

/******************************************************************************
 *
 * function:
 *  node * SearchForNameInVardecs( char *name, node *vardecs)
 *
 * description:
 *   looks up the name in the vardecs given. Returns the address of the vardec
 *   if found, NULL otherwise.
 *
 ******************************************************************************/

node *
SearchForNameInVardecs (char *name, node *vardecs)
{
    DBUG_ENTER ("SearchForNameInVardecs");

    while ((vardecs != NULL) && (strcmp (VARDEC_NAME (vardecs), name) != 0)) {
        vardecs = VARDEC_NEXT (vardecs);
    }
    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *  node * SearchForNameInArgs( char *name, node *args)
 *
 * description:
 *   looks up the name in the args given. Returns the address of the arg
 *   if found, NULL otherwise.
 *
 ******************************************************************************/

node *
SearchForNameInArgs (char *name, node *args)
{
    DBUG_ENTER ("SearchForNameInArgs");

    while ((args != NULL) && (strcmp (ARG_NAME (args), name) != 0)) {
        args = ARG_NEXT (args);
    }
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *  node * SearchForNameInObjs( char *mod, char *name, node *objs)
 *
 * description:
 *   looks up the name in the objs given. Returns the address of the obj
 *   if found, NULL otherwise. In case no module is specified AND several
 *   objects of the given name (from different classes) are available,
 *   an Error message is issued!
 *
 ******************************************************************************/

node *
SearchForNameInObjs (char *mod, char *name, node *objs)
{
    int num_found, num_found_in_main;
    node *last_found, *last_found_in_main;

    DBUG_ENTER ("SearchForNameInObjs");

    if (mod != NULL) {
        while ((objs != NULL)
               && ((strcmp (mod, OBJDEF_MOD (objs)) != 0)
                   || (strcmp (name, OBJDEF_NAME (objs)) != 0))) {
            objs = OBJDEF_NEXT (objs);
        }
    } else {
        num_found = 0;
        last_found = NULL;
        num_found_in_main = 0;
        last_found_in_main = NULL;
        while (objs != NULL) {
            if (strcmp (name, OBJDEF_NAME (objs)) == 0) {
                num_found++;
                last_found = objs;
                if (strcmp (OBJDEF_MOD (objs), MAIN_MOD_NAME) == 0) {
                    num_found_in_main++;
                    last_found_in_main = objs;
                }
            }
            objs = OBJDEF_NEXT (objs);
        }

        if (num_found_in_main == 1) {
            objs = last_found_in_main;
        } else if ((num_found_in_main == 0) && (num_found == 1)) {
            objs = last_found;
        } else if (num_found > 1) {
            ERROR (linenum,
                   ("Identifier '%s` matches more than one global object", name));
        }
    }

    DBUG_RETURN (objs);
}

/******************************************************************************
 *
 * function:
 *  node * SearchForNameLHS( char *mod, char *name,
 *                           node *vardecs, node *args, node *objs)
 *
 * description:
 *   looks up the name (used as a lambda name, i.e., on the LHS of a let)
 *   in the vardecs/args/objs. Returns
 *    - the address of the vardec/arg/obj if found,
 *    - NULL if it is a local identifyer w/o vardec,
 *    - and an Error-Message otherwise.
 *
 ******************************************************************************/

node *
SearchForNameLHS (char *mod, char *name, node *vardecs, node *args, node *objs)
{
    DBUG_ENTER ("SearchForNameLHS");

    if (mod != NULL) {
        ERROR (linenum, ("Identifier with module/class prefix '%s:%s` not allowed"
                         " as local variable name",
                         mod, name));
    } else {
        if (SearchForNameInObjs (mod, name, objs) != NULL) {
            ERROR (linenum, ("Identifier '%s` as local variable name not allowed"
                             " as it is identical to the name of a global object",
                             name));
        } else {
            vardecs = SearchForNameInVardecs (name, vardecs);
            if (vardecs == NULL) {
                vardecs = SearchForNameInArgs (name, args);
            }
        }
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *  node * SearchForNameRHS( char *mod, char *name,
 *                           node *vardecs, node *args, node *objs)
 *
 * description:
 *   looks up the name (used in expression position, i.e., on the RHS of a let)
 *   in the vardecs/args/objs. Returns
 *    - the address of the vardec/arg/obj if found,
 *    - NULL if it is a local identifyer w/o vardec,
 *    - and an Error-Message otherwise.
 *
 ******************************************************************************/

node *
SearchForNameRHS (char *mod, char *name, node *vardecs, node *args, node *objs)
{
    DBUG_ENTER ("SearchForNameRHS");

    if (mod != NULL) {
        vardecs = SearchForNameInObjs (mod, name, objs);
        if (vardecs == NULL) {
            ERROR (linenum, ("Global object '%s:%s` not available", mod, name));
        }
    } else {
        vardecs = SearchForNameInVardecs (name, vardecs);
        if (vardecs == NULL) {
            vardecs = SearchForNameInArgs (name, args);
            if (vardecs == NULL) {
                vardecs = SearchForNameInObjs (mod, name, objs);
            }
        }
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *  node * CheckIds( ids *idents, node *vardecs, node *args)
 *
 * description:
 *   looks up each identifyer of the idents within the identifyers available in
 *   vardecs, args, objs. If found a propoer backref is set. Otherwise, an
 *   appropriate vardec is prepanded to the vardecs.
 *   The (potentially enlarged) vardecs are returned.
 *   Since the lookup mechanism differs depending on the (LHS/RHS) position of
 *   the idents, the SearchForName - function is given as argument!
 *
 ******************************************************************************/

typedef node *(*search_for_name_fun_t) (char *, char *, node *, node *, node *);

static node *
CheckIds (ids *idents, search_for_name_fun_t SearchForName, node *vardecs, node *args,
          node *objs)
{
    node *vardec;

    DBUG_ENTER ("CheckIds");

    while (idents != NULL) {
        vardec = SearchForName (IDS_MOD (idents), IDS_NAME (idents), vardecs, args, objs);
        if (vardec == NULL) {
            /*
             * The identifyer we are looking for does not have a
             * vardec yet! So we allocate one and prepand it to vardecs.
             */
            vardec = MakeVardec (StringCopy (IDS_NAME (idents)), MakeTypes1 (T_unknown),
                                 vardecs);
            vardecs = vardec;
        }
        IDS_VARDEC (idents) = vardec;
        if (NODE_TYPE (vardec) == N_objdef) {
            IDS_MOD (idents) = OBJDEF_MOD (vardec);
        }
        idents = IDS_NEXT (idents);
    }
    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDfundef( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INSVDfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_INSVD_VARDECS (arg_info) = FUNDEF_VARDEC (arg_node);
        INFO_INSVD_ARGS (arg_info) = FUNDEF_ARGS (arg_node);

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_VARDEC (arg_node) = INFO_INSVD_VARDECS (arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDlet( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INSVDlet");

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_INSVD_VARDECS (arg_info)
      = CheckIds (LET_IDS (arg_node), SearchForNameLHS, INFO_INSVD_VARDECS (arg_info),
                  INFO_INSVD_ARGS (arg_info), INFO_INSVD_OBJDEFS (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDid( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INSVDid");

    INFO_INSVD_VARDECS (arg_info)
      = CheckIds (ID_IDS (arg_node), SearchForNameRHS, INFO_INSVD_VARDECS (arg_info),
                  INFO_INSVD_ARGS (arg_info), INFO_INSVD_OBJDEFS (arg_info));
    SET_FLAG (ID, arg_node, IS_GLOBAL, (NODE_TYPE (ID_VARDEC (arg_node)) == N_objdef));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDwithid( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INSVDwithid");

    INFO_INSVD_VARDECS (arg_info)
      = CheckIds (NWITHID_IDS (arg_node), SearchForNameLHS, INFO_INSVD_VARDECS (arg_info),
                  INFO_INSVD_ARGS (arg_info), INFO_INSVD_OBJDEFS (arg_info));
    INFO_INSVD_VARDECS (arg_info)
      = CheckIds (NWITHID_VEC (arg_node), SearchForNameLHS, INFO_INSVD_VARDECS (arg_info),
                  INFO_INSVD_ARGS (arg_info), INFO_INSVD_OBJDEFS (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * InsertVardecs( node *syntax_tree)
 *
 * description:
 *   Traverses through all functions and inserts vardecs for all identifyers
 *   that do not have one yet. Whenever an N_id node is met, we simply look for
 *   its declaration. If found, a backref to it is inserted (ID_VARDEC).
 *   Otherwise, a new vardec is inserted and a backref is inserted.
 *   The same holds for the ids found at N_let nodes.
 *
 ******************************************************************************/

node *
InsertVardec (node *syntax_tree)
{
    funtab *old_funtab;
    node *info_node;

    DBUG_ENTER ("InsertVardecs");

    old_funtab = act_tab;
    act_tab = insvd_tab;

    info_node = MakeInfo ();

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_modul),
                 ("InsertVardec can only be called on N_modul nodes"));

    INFO_INSVD_OBJDEFS (info_node) = MODUL_OBJS (syntax_tree);

    syntax_tree = Trav (syntax_tree, info_node);
    act_tab = old_funtab;

    info_node = FreeNode (info_node);

    DBUG_RETURN (syntax_tree);
}
