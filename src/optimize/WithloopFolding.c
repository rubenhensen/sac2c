
/*******************************************************************************
 * This file realizes the WL-folding for the new SAC-WLs.
 *
 *
 *
 *
 *
 *
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "types.h"
#include "internal_lib.h"
#include "free.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "WithloopFolding.h"

/******************************************************************************
 *
 * function:
 *   node *WLFWithloopFolding(node *arg_node, node* arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLFWithloopFolding (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFWithloopFolding");

    act_tab = wlf_tab;

    Trav (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
