/*
 *
 * $Log$
 * Revision 3.7  2004/11/26 21:07:14  jhb
 * compile
 *
 * Revision 3.6  2004/11/19 10:18:14  sah
 * updated objinit
 *
 * Revision 3.5  2004/07/17 14:30:09  sah
 * switch to INFO structure
 * PHASE I
 *
 * Revision 3.4  2004/02/20 08:27:38  mwe
 * now functions with (MODUL_FUNS) and without (MODUL_FUNDECS) body are separated
 * changed tree traversal according to that
 *
 * Revision 3.3  2002/10/18 16:54:30  dkr
 * interpretation of OBJDEF_MOD modified and corrected
 *
 * Revision 3.2  2002/02/20 14:54:28  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.1  2000/11/20 18:02:00  sacbase
 * new release made
 *
 * Revision 2.6  2000/10/26 14:20:19  dkr
 * CopyShpseg replaced by DupShpseg (DupTree.[ch])
 *
 * Revision 2.5  2000/10/24 11:50:55  dkr
 * MakeType renamed into MakeTypes
 *
 * Revision 2.4  2000/08/04 17:19:40  dkr
 * NEWTREE removed
 *
 * Revision 2.3  2000/07/12 15:15:39  dkr
 * function DuplicateTypes renamed into DupTypes
 *
 * Revision 2.2  2000/02/17 16:19:35  cg
 * File now includes DupTree.h instead of typecheck.h in order
 * to use function DuplicateTypes().
 *
 * Revision 2.1  1999/02/23 12:43:23  sacbase
 * new release made
 *
 * Revision 1.11  1998/12/02 16:30:37  cg
 * Now, generic object creation functions for imported global objects
 * have status ST_objinitfun rather than ST_imported. This forces the
 * typechecker to typecheck them and thereby to qualify them as being
 * used.
 *
 * Revision 1.10  1997/11/07 14:44:48  dkr
 * eliminated another nnode
 *
 * Revision 1.9  1997/04/25 14:23:48  sbs
 * OIobjdef: pragma is deleted (FreeNode) iff it is != NULL !
 *
 * Revision 1.8  1996/01/26  15:32:21  cg
 * function status ST_classfun now supported
 *
 * Revision 1.7  1996/01/25  18:46:09  cg
 * In class implementations, the typedef node for the class type
 * is generated here as well as the fundef nodes of the generic
 * conversion functions.
 *
 * Revision 1.6  1996/01/22  18:38:56  cg
 * Now, an object initialization expression is always generated
 * with respect to pragma initfun.
 *
 * Revision 1.5  1995/12/01  20:30:51  cg
 * extern declarations are now generated for generic object init
 * functions if it is a SAC-program (not module implementation)
 *
 * Revision 1.4  1995/12/01  17:25:00  cg
 * now shape segments and strings are always copied when generated
 * from existing nodes.
 *
 * Revision 1.3  1995/10/18  13:35:36  cg
 * now Malloc is used instead of malloc,
 * so error messages are no longer needed.
 *
 * Revision 1.2  1995/10/17  08:28:15  cg
 * all automatically generated functions now have status ST_objinitfun.
 * This tag is used by the typechecker.
 *
 * Revision 1.1  1995/10/16  12:22:44  cg
 * Initial revision
 *
 */

#define NEW_INFO

#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "free.h"
#include "new_types.h"
#include "shape.h"

#include "DupTree.h"
#include "filemgr.h"

#ifndef OBJI_DEACTIVATED

/*
 * INFO structure
 */
struct INFO {
    node *modul;
};

/*
 * INFO macros
 */
#define INFO_OBJINIT_MODULE(n) (n->modul)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_OBJINIT_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 *
 *  functionname  : OImodule
 *  arguments     : 1) pointer to N_modul node
 *                  2) arg_info unused
 *  description   : traverses all global object definitions
 *                  In class implementations, a typedef node for the
 *                  class type is generated as well as generic conversion
 *                  functions.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : TRAVdo, strcpy, strcat, MakeTypedef, MakeFundef,
 *                  Malloc
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
OImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OImodule");

    if (MODULE_OBJS (arg_node) != NULL) {
        /* pass on reference to N_modul node */
        INFO_OBJINIT_MODULE (arg_info) = arg_node;

        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);

        /* reset to NULL (mainly for debugging) */
        INFO_OBJINIT_MODULE (arg_info) = NULL;
    }

    if (MODULE_FILETYPE (arg_node) == F_classimp) {
        char *toclass;
        char *fromclass;

        MODULE_TYPES (arg_node) = TBmakeTypedef (ILIBstringCopy (MODULE_NAME (arg_node)),
                                                 ILIBstringCopy (MODULE_NAME (arg_node)),
                                                 TYcopyType (MODULE_CLASSTYPE (arg_node)),
                                                 MODULE_TYPES (arg_node));

        toclass = (char *)ILIBmalloc (MAX_FILE_NAME);
        fromclass = (char *)ILIBmalloc (MAX_FILE_NAME);

        strcpy (toclass, "to_");
        strcpy (fromclass, "from_");

        strcat (toclass, MODULE_NAME (arg_node));
        strcat (fromclass, MODULE_NAME (arg_node));

        MODULE_FUNDECS (arg_node)
          = TBmakeFundef (toclass, ILIBstringCopy (MODULE_NAME (arg_node)),

                          TCreturnTypes2Ret (
                            TBmakeTypes (T_user, 0, NULL,
                                         ILIBstringCopy (MODULE_NAME (arg_node)),
                                         ILIBstringCopy (MODULE_NAME (arg_node)))),

                          TBmakeArg (TBmakeAvis (ILIBstringCopy ("obj"),
                                                 TYcopyType (
                                                   MODULE_CLASSTYPE (arg_node))),
                                     NULL),
                          NULL, MODULE_FUNDECS (arg_node));

        FUNDEF_ISCLASSCONVERSIONFUN (MODULE_FUNDECS (arg_node)) = TRUE;

        MODULE_FUNDECS (arg_node)

          = TBmakeFundef (fromclass, ILIBstringCopy (MODULE_NAME (arg_node)),

                          TCreturnTypes2Ret (
                            TBmakeTypes (T_user, 0, NULL,
                                         ILIBstringCopy (MODULE_NAME (arg_node)),
                                         ILIBstringCopy (MODULE_NAME (arg_node)))),

                          TBmakeArg (TBmakeAvis (ILIBstringCopy ("obj"),
                                                 TYcopyType (
                                                   MODULE_CLASSTYPE (arg_node))),
                                     NULL),
                          NULL, MODULE_FUNDECS (arg_node));

        FUNDEF_ISCLASSCONVERSIONFUN (MODULE_FUNDECS (arg_node)) = TRUE;

        MODULE_CLASSTYPE (arg_node) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : OIobjdef
 *  arguments     : 1) pointer to objdef node
 *                  2) INFO structure containing pointer to N_modul
 *                     node (head of syntax tree)
 *  description   : A new function definition is generated that has no
 *                  formal parameters, the return type is identical to
 *                  the type of the global object and the initialization
 *                  expression is moved to the return-statement of the
 *                  new function. In the objdef node it's replaced by
 *                  an application of the new function.
 *
 *                  This mechanism allows for arbitrary expression to
 *                  initialize global objects.
 *  external funs : Malloc, strlen, strcpy, strcat,
 *                  TBmakeType, TBmakeExprs, MakeReturn, MakeAssign, MakeBlock,
 *                  MakeFundef, TBmakeAp, TRAVdo
 *
 */

node *
OIobjdef (node *arg_node, info *arg_info)
{
    node *new_node;
    types *new_fun_type;
    char *new_fun_name;

    DBUG_ENTER ("OIobjdef");
    /*
    if (OBJDEF_EXPR(arg_node)!=NULL) {
      new_fun_type=TBmakeTypes(TYgetSimpleType(
                                 TYgetScalar( OBJDEF_TYPE(arg_node))),
                               TYgetDim( OBJDEF_TYPE(arg_node)),
                               SHshape2OldShpseg(
                                 TYgetShape( OBJDEF_TYPE(arg_node))),
                               ILIBstringCopy(OBJDEF_TNAME(arg_node)),
                               OBJDEF_TMOD(arg_node));
    */ /* TODO wrong */
    new_fun_name = (char *)ILIBmalloc (strlen (OBJDEF_NAME (arg_node)) + 10);

    new_fun_name = strcpy (new_fun_name, "CREATE__");
    new_fun_name = strcat (new_fun_name, OBJDEF_NAME (arg_node));

    new_node = TBmakeExprs (OBJDEF_EXPR (arg_node), NULL);
    NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

    new_node = TBmakeReturn (new_node);
    NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

    new_node = TBmakeAssign (new_node, NULL);
    NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

    new_node = TBmakeBlock (new_node, NULL);
    NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

    new_node = TBmakeFundef (new_fun_name, OBJDEF_MOD (arg_node), new_fun_type, NULL,
                             new_node, MODULE_FUNS (INFO_OBJINIT_MODULE (arg_info)));
    NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

    FUNDEF_STATUS (new_node) = ST_objinitfun;

    /*
     * The new functions have status ST_objinitfun.
     * This tag is needed by the typechecker to ensure that these functions
     * are actually typechecked even if they are exclusively applied in
     * a global object initialization of a SAC-program.
     */

    MODULE_FUNS (INFO_OBJINIT_MODULE (arg_info)) = new_node;

    new_node = TBmakeAp (ILIBstringCopy (new_fun_name),
                         ILIBstringCopy (OBJDEF_MOD (arg_node)), NULL);
    OBJDEF_EXPR (arg_node) = new_node;

    AP_FUNDEF (OBJDEF_EXPR (arg_node)) = MODULE_FUNS (INFO_OBJINIT_MODULE (arg_info));
}
else
{
    new_fun_type
      = TBmakeTypes (OBJDEF_BASETYPE (arg_node), OBJDEF_DIM (arg_node),
                     DUPdupShpseg (OBJDEF_SHPSEG (arg_node)),
                     ILIBstringCopy (OBJDEF_TNAME (arg_node)), OBJDEF_TMOD (arg_node));

    if (OBJDEF_INITFUN (arg_node) != NULL) {
        new_fun_name = OBJDEF_INITFUN (arg_node);
        PRAGMA_INITFUN (OBJDEF_PRAGMA (arg_node)) = NULL;
    } else {
        new_fun_name = (char *)ILIBmalloc (strlen (OBJDEF_NAME (arg_node)) + 10);

        if (((sbs == 1) && (strcmp (OBJDEF_MOD (arg_node), EXTERN_MOD_NAME) != 0))
            || ((sbs == 0) && (OBJDEF_MOD (arg_node) != NULL))) {
            strcpy (new_fun_name, "CREATE__");
        } else {
            strcpy (new_fun_name, "create_");
        }

        strcat (new_fun_name, OBJDEF_NAME (arg_node));
    }

    new_node = TBmakeFundef (new_fun_name, OBJDEF_MOD (arg_node), new_fun_type, NULL,
                             NULL, MODULE_FUNDECS (INFO_OBJINIT_MODULE (arg_info)));

    FUNDEF_STATUS (new_node) = ST_objinitfun;

    MODULE_FUNDECS (INFO_OBJINIT_MODULE (arg_info)) = new_node;

    new_node = TBmakeAp (ILIBstringCopy (new_fun_name), OBJDEF_MOD (arg_node), NULL);

    OBJDEF_EXPR (arg_node) = new_node;

    AP_FUNDEF (OBJDEF_EXPR (arg_node)) = MODULE_FUNDECS (INFO_OBJINIT_MODULE (arg_info));
}

if ((OBJDEF_PRAGMA (arg_node) != NULL)
    && (PRAGMA_LINKNAME (OBJDEF_PRAGMA (arg_node)) == NULL)) {
    OBJDEF_PRAGMA (arg_node) = FREEdoFreeNode (OBJDEF_PRAGMA (arg_node));
}

if (OBJDEF_NEXT (arg_node) != NULL) {
    OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
}

DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : objinit
 *  arguments     : 1) syntax_tree (pointer to N_modul node)
 *  description   : starts the traversal mechanism for the objinit
 *                  compilation phase. Here, all initialization expressions
 *                  of global objects are moved to new generated functions
 *                  and replaced by applications of the respective functions.
 *  global vars   : act_tab, objinit_tab
 *  internal funs : ---
 *  external funs : TRAVdo
 *  macros        :
 *
 *  remarks       :
 *
 */

#endif /* OBJI_DEACTIVATED */

node *
objinit (node *syntax_tree)
{
    DBUG_ENTER ("objinit");

#ifndef OBJI_DEACTIVATED

    info *info;

    info = MakeInfo ();

    TRAVpush (TR_objini);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

#endif /* OBJI_DEACTIVATED */

    DBUG_RETURN (syntax_tree);
}
