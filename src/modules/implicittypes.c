/*
 *
 * $Log$
 * Revision 1.3  1997/10/29 14:33:04  srs
 * free -> FREE
 *
 * Revision 1.2  1995/11/10 15:04:37  cg
 * converted to new error macros
 *
 * Revision 1.1  1995/10/05  16:12:45  cg
 * Initial revision
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "convert.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"

/*
 *
 *  functionname  : RetrieveImplicitTypeInfo
 *  arguments     : 1) syntax tree
 *  description   : retrieves information about non-external implicit types
 *                  which is stored since import of SIB-file in the syntax
 *                  tree.
 *  global vars   : act_tab, impl_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
RetrieveImplicitTypeInfo (node *arg_node)
{
    DBUG_ENTER ("RetrieveImplicitTypeInfo");

    act_tab = impltype_tab;

    DBUG_RETURN (Trav (arg_node, NULL));
}

/*
 *
 *  functionname  : SearchImplementation
 *  arguments     : 1) user-defined type
 *                  2) start node of type definitions
 *  description   : This function returns the concrete implementation of a
 *                  user-defined type.
 *                  After typechecking all typedefs have a concrete type
 *                  implementation or again a user-defined type. In the
 *                  latter case this user-defined type must be T_hidden.
 *                  This situation usually is destroyed when the implementation
 *                  of formerly implicit types is added by additional SIB-
 *                  information.
 *                  Based on the type retrieval of the typechecker the
 *                  implementation of a user-defined type is returned.
 *                  This may be a user-defined type as well, but then this
 *                  definitely must be an external implicit type (void*).
 *  global vars   : ---
 *  internal funs :
 *  external funs : SearchTypedef
 *  macros        : ERROR, FREE
 *
 *  remarks       :
 *
 */

types *
SearchImplementation (types *type, node *alltypes)
{
    node *tdef;
    shpseg *tobefreed;

    DBUG_ENTER ("SearchImplementation");

    DBUG_PRINT ("IMPLTYPES", ("Searching implementation of user-defined type %s:%s",
                              TYPES_MOD (type), TYPES_NAME (type)));

    tdef = SearchTypedef (TYPES_NAME (type), TYPES_MOD (type), alltypes);

    if (tdef == NULL) {
        SYSABORT (("Definition of implicit type '%s:%s` missing", TYPES_MOD (type),
                   TYPES_NAME (type)));
    }

    /*
     *  tdef points to the typedef which defines 'type'.
     */

    DBUG_PRINT ("IMPLTYPES", ("Found defining typedef BT:%s, Name:%s, Dim:%d",
                              type_string[TYPEDEF_BASETYPE (tdef)], TYPEDEF_TNAME (tdef),
                              TYPEDEF_DIM (tdef)));

    if (TYPEDEF_BASETYPE (tdef) == T_hidden) {
        if (TYPEDEF_TNAME (tdef) != NULL) {

            /*
             *  tdef itself is based on a still hidden type.
             */

            TYPES_BASETYPE (type) = T_hidden;
            TYPES_NAME (type) = TYPEDEF_TNAME (tdef);
            TYPES_MOD (type) = TYPEDEF_TMOD (tdef);

            if (TYPEDEF_DIM (tdef) != 0) {
                if (TYPES_DIM (type) == 0) {

                    /*
                     *  tdef is an array and type is not an array.
                     */

                    TYPES_DIM (type) = TYPEDEF_DIM (tdef);
                    TYPES_SHPSEG (type) = CopyShpseg (TYPEDEF_SHPSEG (tdef));
                } else {

                    /*
                     *  tdef is an array and type is also an array.
                     */

                    tobefreed = TYPES_SHPSEG (type);
                    TYPES_SHPSEG (type)
                      = MergeShpseg (TYPES_SHPSEG (type), TYPES_DIM (type),
                                     TYPEDEF_SHPSEG (tdef), TYPEDEF_DIM (tdef));

                    FREE (tobefreed);

                    TYPES_DIM (type) += TYPEDEF_DIM (tdef);
                }
            }

            /*
             *  if tdef is not an array, then no more actions are necessary.
             */

        } else {
            DBUG_PRINT ("IMPLTYPES", ("Defining type is plain hidden"));
        }

    } else {

        /*
         *  tdef itself is based on a concrete type implementation.
         */

        TYPES_BASETYPE (type) = TYPEDEF_BASETYPE (tdef);
        TYPES_NAME (type) = NULL;
        TYPES_MOD (type) = NULL;

        if (TYPEDEF_DIM (tdef) != 0) {
            if (TYPES_DIM (type) == 0) {
                TYPES_DIM (type) = TYPEDEF_DIM (tdef);
                TYPES_SHPSEG (type) = CopyShpseg (TYPEDEF_SHPSEG (tdef));
            } else {
                tobefreed = TYPES_SHPSEG (type);
                TYPES_SHPSEG (type)
                  = MergeShpseg (TYPES_SHPSEG (type), TYPES_DIM (type),
                                 TYPEDEF_SHPSEG (tdef), TYPEDEF_DIM (tdef));

                FREE (tobefreed);

                TYPES_DIM (type) += TYPEDEF_DIM (tdef);
            }
        }
    }

    DBUG_PRINT ("IMPLTYPES", ("Found implementation for %s", Type2String (type, 0)));

    DBUG_RETURN (type);
}

/*
 *
 *  functionname  : RecoverTypedefs
 *  arguments     : 1) ptr to beginning of typedef-chain
 *  description   : This functions recovers the typedefs after implicit
 *                  type information has been retrieved.
 *                  This is necessary because a former hidden type may now
 *                  have a concrete implementation or it may be built from
 *                  another still hidden type. This function restores the
 *                  situation after typechecking with respect to additional
 *                  type information available through the SIB-file.
 *  global vars   : ---
 *  internal funs : SearchImplementation
 *  external funs : ---
 *  macros        : TREE_MACROS
 *
 *  remarks       :
 *
 */

node *
RecoverTypedefs (node *alltypes)
{
    node *tmp;

    DBUG_ENTER ("RecoverTypedefs");

    tmp = alltypes;

    while (tmp != NULL) {
        if ((TYPEDEF_BASETYPE (tmp) == T_user)
            || ((TYPEDEF_BASETYPE (tmp) == T_hidden) && (TYPEDEF_TNAME (tmp) != NULL))) {
            TYPEDEF_TYPE (tmp) = SearchImplementation (TYPEDEF_TYPE (tmp), alltypes);
        }

        tmp = TYPEDEF_NEXT (tmp);
    }

    DBUG_RETURN (alltypes);
}

/*
 *
 *  functionname  : IMPLmodul
 *  arguments     : 1) N_modul node of program or module/class implementation
 *                  2) arg_info node from traversal mechanism (unused)
 *  description   : From previous SIB-Files, information is stored about
 *                  the implementation of non-external implicit types
 *                  This information is now retrieved.
 *  global vars   : ---
 *  internal funs : RecoverTypedefs
 *  external funs : MakeTypedef, MakeType
 *  macros        : FREE
 *
 *  remarks       :
 *
 */

node *
IMPLmodul (node *arg_node, node *arg_info)
{
    node *tmp, *new_typedef, *last, *act;
    types *tobefreed, *new_type;

    DBUG_ENTER ("IMPLmodul");

    tmp = MODUL_TYPES (arg_node);

    while (tmp != NULL) {
        if (TYPEDEF_IMPL (tmp) != NULL) {
            tobefreed = TYPEDEF_TYPE (tmp);
            TYPEDEF_TYPE (tmp) = TYPEDEF_IMPL (tmp);
            /* FREE(tobefreed); */

            /*
             *  hidden implementation is used from now on
             */

            DBUG_PRINT ("IMPLTYPES", ("use SIB-info for implicit type %s:%s",
                                      TYPEDEF_MOD (tmp), TYPEDEF_NAME (tmp)));

            if (TYPEDEF_BASETYPE (tmp) == T_user) {

                /*
                 *  implementtaion is still hidden (based on external implicit type)
                 */

                TYPEDEF_BASETYPE (tmp) = T_hidden;
                new_typedef = SearchTypedef (TYPEDEF_TNAME (tmp), TYPEDEF_TMOD (tmp),
                                             MODUL_TYPES (arg_node));
                if (new_typedef == NULL) {

                    /*
                     *  The external implicit type is yet unknwon, so the respective
                     *  typedef node is generated and added at the beginning of the
                     *  typedef-chain.
                     */

                    new_type = MakeType (T_hidden, 0, NULL, NULL, NULL);
                    new_typedef
                      = MakeTypedef (TYPEDEF_TNAME (tmp), TYPEDEF_TMOD (tmp), new_type,
                                     ST_regular, MODUL_TYPES (arg_node));

                    DBUG_PRINT ("IMPLTYPES", ("Generating new hidden-typedef %s:%s",
                                              TYPEDEF_TMOD (tmp), TYPEDEF_TMOD (tmp)));

                    MODUL_TYPES (arg_node) = new_typedef;
                } else {

                    /*
                     *  The external implicit type is already known, but the respective
                     *  typedef may be located behind this typedef in the typedef chain.
                     *  The following lines assure that the respective typedef becomes
                     *  the head of the typedef chain.
                     */

                    if (MODUL_TYPES (arg_node) != new_typedef) {
                        last = MODUL_TYPES (arg_node);
                        act = TYPEDEF_NEXT (last);
                        while (act != new_typedef) {
                            last = act;
                            act = TYPEDEF_NEXT (act);
                        }

                        TYPEDEF_NEXT (last) = TYPEDEF_NEXT (act);

                        /********************************************************************/
                        if (TYPEDEF_NEXT (act) == NULL) {
                            last->nnode = 0;
                        }
                        /********************************************************************/

                        TYPEDEF_NEXT (new_typedef) = MODUL_TYPES (arg_node);

                        /********************************************************************/
                        if (MODUL_TYPES (arg_node) != NULL) {
                            new_typedef->nnode = 1;
                        }
                        /********************************************************************/

                        MODUL_TYPES (arg_node) = new_typedef;

                        DBUG_PRINT ("IMPLTYPES",
                                    ("Shifting hidden-typedef %s:%s to front of chain",
                                     TYPEDEF_TMOD (tmp), TYPEDEF_TMOD (tmp)));
                    }
                }
            }
        }

        tmp = TYPEDEF_NEXT (tmp);
    }

    MODUL_TYPES (arg_node) = RecoverTypedefs (MODUL_TYPES (arg_node));

    DBUG_RETURN (arg_node);
}
