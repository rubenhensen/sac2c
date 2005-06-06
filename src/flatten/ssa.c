/*
 * $Log$
 * Revision 1.16  2005/06/06 13:21:00  jhb
 * removed SSATransformExplicitAllocs
 *
 * Revision 1.15  2005/05/13 16:44:40  ktr
 * UndoSSA now uses lacinlining.
 *
 * Revision 1.14  2005/04/19 17:37:23  ktr
 * added "lacinl"
 *
 * Revision 1.13  2005/04/15 13:05:11  ktr
 * global.lacinline
 *
 * Revision 1.12  2005/04/12 15:49:00  ktr
 * INLdoLACInlining is used instead of INLdoInlining
 *
 * Revision 1.11  2005/03/10 09:41:09  cg
 * Added some missing includes.
 *
 * Revision 1.10  2005/03/04 21:21:42  cg
 * Added application of function inlining after LaC2Fun.
 *
 * Revision 1.9  2004/12/19 19:49:29  sbs
 * calls to TNTdoToNewTypesXXX ( previously CheckAvis)
 * eliminated
 *
 * Revision 1.8  2004/11/24 19:15:29  mwe
 * SacDevCamp: compiles!!\
 *
 * Revision 1.7  2004/11/18 14:34:31  mwe
 * changed CheckAvis and chkavis to ToNewTypes and to tonewtypes
 *
 * Revision 1.6  2004/09/18 15:58:34  ktr
 * added RestoreSSAExplicitAllocs
 *
 * Revision 1.5  2004/08/08 16:05:08  sah
 * fixed some includes.
 *
 * Revision 1.4  2004/02/25 15:53:06  cg
 * New functions RestoreSSAOneFunction and RestoreSSAOneFundef
 * now provide access to SSA transformations on a per function
 * basis.
 * Only functions from ssa.[ch] should be used to initiate the
 * transformation process in either direction!
 *
 * Revision 1.3  2004/02/25 08:22:32  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 *
 * Revision 1.2  2004/02/04 12:32:28  skt
 * some comments added
 *
 * Revision 1.1  2004/02/02 15:47:38  skt
 * Initial revision
 *
 */

/**
 * @defgroup ssa Static Single Assignment
 *
 * This group contains all those files/ modules that deal with administrative
 * tasks of the SSA-form
 * @{
 */

/**
 *
 * @file ssa.c
 * @brief Tool package to create resp. to reverse the ssa-form
 *
 * Until now we need to call several functions to create the ssa-form,
 * e.g. Lac2Fun, SSATransform.
 * This module allows to use just one function call
 * for creation and another for reversion
 */

#include "globals.h"
#include "lac2fun.h"
#include "SSATransform.h"
#include "fun2lac.h"
#include "node_basic.h"
#include "internal_lib.h"
#include "UndoSSATransform.h"
#include "string.h"
#include "tree_basic.h"
#include "lacinlining.h"

#include "ssa.h"

/** <!--********************************************************************-->
 *
 * @fn node *SSArestoreSsaOneFunction(node *syntax_tree)
 *
 *   @brief  takes an N_fundef node and restores ssa-form
 *
 *   <pre>
 *   1. Check for correct Avis nodes in vardec/arg nodes. All backrefs
 *      from N_id or IDS structures are checked for consistent values.
 *      This traversal is needed for compatiblity with old code without
 *      knowledge of the avis nodes.
 *   2. Transform code in SSA form.
 *      Every variable has exaclty one definition.
 *      After all the valid_ssaform flag is set to TRUE.
 *
 *   Unlike RestoreSSAOneFundef, this function implicitly traverses applied
 *   loop and conditional special functions.
 *   </pre>
 *
 *   @param fundef  nomen est omen
 *   @return fundef in restored ssa-form
 *
 *****************************************************************************/

node *
SSArestoreSsaOneFunction (node *fundef)
{
    DBUG_ENTER ("SSArestoreSsaOneFunction");

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "SSArestoreSsaOneFunction called without N_fundef node.");

    fundef = SSATdoTransformOneFunction (fundef);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSArestoreSsaOneFundef(node *syntax_tree)
 *
 *   @brief  takes an N_fundef node and restores ssa-form
 *
 *   <pre>
 *   1. Check for correct Avis nodes in vardec/arg nodes. All backrefs
 *      from N_id or IDS structures are checked for consistent values.
 *      This traversal is needed for compatiblity with old code without
 *      knowledge of the avis nodes.
 *   2. Transform code in SSA form.
 *      Every variable has exaclty one definition.
 *      After all the valid_ssaform flag is set to TRUE.
 *
 *   Unlike RestoreSSAOneFunction, this function does NOT traverse nested
 *   loop and conditional special functions.
 *   </pre>
 *
 *   @param fundef  nomen est omen
 *   @return fundef in restored ssa-form
 *
 *****************************************************************************/

node *
SSArestoreSsaOneFundef (node *fundef)
{
    DBUG_ENTER ("SSArestoreSsaOneFundef");

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "SSArestoreSsaOneFundef called without N_fundef node.");

    fundef = SSATdoTransformOneFundef (fundef);

    DBUG_RETURN (fundef);
}

/**
 * @}
 */
