/*
 *
 * $Log$
 * Revision 1.1  2002/03/13 15:57:52  ktr
 * Initial revision
 *
 *
 */

/****************************************************************************
 *
 * file:    WithloopScalarization.c
 *
 * prefix:  WLS
 *
 * description:
 *
 *   This file realizes the with-loop-scalarization in ssa-form.
 *   Currently this is just a dummy.
 *
 ****************************************************************************/

#include "tree_basic.h"
#include "dbug.h"
#include "optimize.h"

node *
WithloopScalarization (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WithloopScalarization");

    wls_expr = 500;

    DBUG_RETURN (arg_node);
}
