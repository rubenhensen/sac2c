/*
 *
 * $Log$
 * Revision 1.1  2004/07/18 20:00:48  sah
 * Initial revision
 *
 */

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
    node *fundef;
    node *modul;
    bool remassign;
    node *preassign;
};

/*
 * INFO macros
 */
#define INFO_SSALUR_ASSIGN(n) (n->assign)
#define INFO_SSALUR_FUNDEF(n) (n->fundef)
#define INFO_SSALUR_MODUL(n) (n->modul)
#define INFO_SSALUR_REMASSIGN(n) (n->remassign)
#define INFO_SSALUR_PREASSIGN(n) (n->preassign)
