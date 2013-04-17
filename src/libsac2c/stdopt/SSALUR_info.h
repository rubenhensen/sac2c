/*****************************************************************************
 * This file contains the INFO structure and macros used by:
 * - SSALUR.c         (creates structure)
 * - SSAWLunroll.c    (uses structure only)
 ****************************************************************************/

#include "types.h"

/*
 * INFO structure
 */
struct INFO {
    node *assign;
    node *ext_assign;
    node *fundef;
    bool remassign;
    node *preassign;
};

/*
 * INFO macros
 */
#define INFO_ASSIGN(n) (n->assign)
#define INFO_EXT_ASSIGN(n) (n->ext_assign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_PREASSIGN(n) (n->preassign)
