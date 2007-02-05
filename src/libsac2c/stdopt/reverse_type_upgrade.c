/**<!--******************************************************************-->
 *
 * $Id$
 *
 * @file reverse_type_upgrade.c
 *
 * This file implements functionality of reverse type upgrade (infer
 * types of rhs dependent on lhs of an assignment).
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "dbug.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "globals.h"
#include "constants.h"
#include "shape.h"
#include "tree_compound.h"

#include "reverse_type_upgrade.h"

typedef enum { let, undef } origin;
typedef enum { fundef, module } travstart;

/*<!--*************************************************************-->
 *
 * INFO STRUCTURE
 *
 ********************************************************************/

struct INFO {
    node *fundef;
    node *idstypes;
    node *withid;
    origin origin;
    travstart travstart;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_IDSTYPES(n) (n->idstypes)
#define INFO_WITHID(n) (n->withid)
#define INFO_ORIGIN(n) (n->origin)
#define INFO_TRAVSTART(n) (n->travstart)

static info *
MakeInfo ()
{
    info *result;
    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_IDSTYPES (result) = NULL;
    INFO_WITHID (result) = NULL;
    INFO_ORIGIN (result) = undef;
    INFO_TRAVSTART (result) = fundef;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*<!--*************************************************************-->
 *
 * LOCAL HELPER FUNCTIONS
 *
 ********************************************************************/

static node *
UpdateGeneratorElement (node *elem, ntype *type)
{
    DBUG_ENTER ("UpdateGeneratorElement");

    if (N_id == NODE_TYPE (elem)) {

        DBUG_ASSERT ((NULL != ID_AVIS (elem)), "Missing AVIS node!");

        if (N_vardec == NODE_TYPE (AVIS_DECL (ID_AVIS (elem)))) {

            /*
             * elem is a local identifier an no input argument
             * check if type in id is less special as type to be assigned
             */
            if (TY_lt == TYcmpTypes (type, AVIS_TYPE (ID_AVIS (elem)))) {

                global.optcounters.tup_rtu_expr++;

                AVIS_TYPE (ID_AVIS (elem)) = TYfreeType (AVIS_TYPE (ID_AVIS (elem)));
                AVIS_TYPE (ID_AVIS (elem)) = TYcopyType (type);

                if (AVIS_DECLTYPE (ID_AVIS (elem)) != NULL) {
                    AVIS_DECLTYPE (ID_AVIS (elem))
                      = TYfreeType (AVIS_DECLTYPE (ID_AVIS (elem)));
                    AVIS_DECLTYPE (ID_AVIS (elem)) = TYcopyType (type);
                }
            }
        }
    }
    DBUG_RETURN (elem);
}

/**********************************************************************
 *
 * GLOBAL FUNCTIONS
 *
 *********************************************************************/

node *
RTUPdoReverseTypeUpgrade (node *arg_node)
{
    DBUG_ENTER ("RTUPdoReverseTypeUpgrade");

    DBUG_ASSERT ((N_module == NODE_TYPE (arg_node)), "N_module node expected!");

    if (arg_node != NULL) {
        info *arg_info;

        arg_info = MakeInfo ();

        INFO_TRAVSTART (arg_info) = module;

        TRAVpush (TR_rtup);
        arg_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RTUPdoReverseTypeUpgradeOneFundef (node *arg_node)
{
    DBUG_ENTER ("RTUPdoReverseTypeUpgradeOneFundef");

    DBUG_ASSERT ((N_fundef == NODE_TYPE (arg_node)), "N_fundef node expected!");

    if (arg_node != NULL) {
        info *arg_info;

        arg_info = MakeInfo ();

        INFO_TRAVSTART (arg_info) = fundef;

        TRAVpush (TR_rtup);
        arg_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *RTUPfundef(node *arg_node, info *arg_info)
 *
 * @brief: handles fundef nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
RTUPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RTUPfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    if (NULL != FUNDEF_BODY (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (module == INFO_TRAVSTART (arg_info)) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *RTUPassign(node *arg_node, info *arg_info)
 *
 * @brief: traverse into assign nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
RTUPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RTUPassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *RTUPlet(node *arg_node, info *arg_info)
 *
 * @brief: handle let nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
RTUPlet (node *arg_node, info *arg_info)
{
    node *old_idstypes;

    DBUG_ENTER ("RTUPlet");

    old_idstypes = INFO_IDSTYPES (arg_info);
    INFO_IDSTYPES (arg_info) = NULL;

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    if (N_id == NODE_TYPE (LET_EXPR (arg_node))) {
        INFO_ORIGIN (arg_info) = let;
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_ORIGIN (arg_info) = undef;

    while (INFO_IDSTYPES (arg_info) != NULL) {
        SET_MEMBER (INFO_IDSTYPES (arg_info))
          = FREEdoFreeTree (SET_MEMBER (INFO_IDSTYPES (arg_info)));

        INFO_IDSTYPES (arg_info) = FREEdoFreeNode (INFO_IDSTYPES (arg_info));
    }

    INFO_IDSTYPES (arg_info) = old_idstypes;

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *RTUPids(node *arg_node, info *arg_info)
 *
 * @brief: handle ids nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
RTUPids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RTUPids");

    TCSetAdd (&(INFO_IDSTYPES (arg_info)),
              TBmakeType (TYcopyType (AVIS_TYPE (IDS_AVIS (arg_node)))));

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *RTUPid(node *arg_node, info *arg_info)
 *
 * @brief: handle is nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
RTUPid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RTUPid");

    if (INFO_ORIGIN (arg_info) == let) {

        DBUG_ASSERT ((SET_NEXT (INFO_IDSTYPES (arg_info)) == NULL),
                     "SET contains more than one list element!");

        if (N_vardec == NODE_TYPE (AVIS_DECL (ID_AVIS (arg_node)))) {

            /*
             * arg_node is no input argument
             * get type of local identifier
             */

            if (TYcmpTypes (TYPE_TYPE (SET_MEMBER (INFO_IDSTYPES (arg_info))),
                            AVIS_TYPE (ID_AVIS (arg_node)))
                == TY_lt) {

                AVIS_TYPE (ID_AVIS (arg_node))
                  = TYfreeType (AVIS_TYPE (ID_AVIS (arg_node)));
                AVIS_TYPE (ID_AVIS (arg_node))
                  = TYcopyType (TYPE_TYPE (SET_MEMBER (INFO_IDSTYPES (arg_info))));

                if (AVIS_DECLTYPE (ID_AVIS (arg_node)) != NULL) {
                    AVIS_DECLTYPE (ID_AVIS (arg_node))
                      = TYfreeType (AVIS_DECLTYPE (ID_AVIS (arg_node)));
                    AVIS_DECLTYPE (ID_AVIS (arg_node))
                      = TYcopyType (TYPE_TYPE (SET_MEMBER (INFO_IDSTYPES (arg_info))));
                }

                /*
                 * update counter
                 */
                global.optcounters.tup_rtu_expr++;
            }
        } else if (N_arg == NODE_TYPE (AVIS_DECL (ID_AVIS (arg_node)))) {

            /*
             * arg_node is an input argument
             */
        }
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *RTUPwith(node *arg_node, info *arg_info)
 *
 * @brief: handles with nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
RTUPwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RTUPwith");

    INFO_WITHID (arg_info) = PART_WITHID (WITH_PART (arg_node));
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *RTUPpart(node *arg_node, info *arg_info)
 *
 * @brief: handles part nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
RTUPpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RTUPpart");

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *RTUPgenerator(node *arg_node, info *arg_info)
 *
 * @brief: does reverse type upgrade in withloop generator
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
RTUPgenerator (node *arg_node, info *arg_info)
{
    ntype *current;
    node *withid;

    DBUG_ENTER ("TUPgenerator");

    withid = INFO_WITHID (arg_info);
    current = TYcopyType (AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid))));

    /*
     * 'current' contains now the most precise non-AKV type of
     * all compared types
     *
     * do reverse type upgrade for generator indeces
     */

    GENERATOR_BOUND1 (arg_node)
      = UpdateGeneratorElement (GENERATOR_BOUND1 (arg_node), current);

    GENERATOR_BOUND2 (arg_node)
      = UpdateGeneratorElement (GENERATOR_BOUND2 (arg_node), current);

    if (GENERATOR_STEP (arg_node) != NULL) {
        GENERATOR_STEP (arg_node)
          = UpdateGeneratorElement (GENERATOR_STEP (arg_node), current);
    }

    if (GENERATOR_WIDTH (arg_node) != NULL) {
        GENERATOR_WIDTH (arg_node)
          = UpdateGeneratorElement (GENERATOR_WIDTH (arg_node), current);
    }

    DBUG_RETURN (arg_node);
}
