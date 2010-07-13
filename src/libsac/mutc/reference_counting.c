/* $Id$ */

/*****************************************************************************
 *
 * file:   reference _counting.c
 *
 * prefix: SAC_MUTC_
 *
 * description:
 *
 *   This file is part of the implementation of asynchronous reference counting for MUTC.
 *
 *
 *   It provides definitions of functions to maintain reference count.
 *
 *****************************************************************************/

#include <sac.h>

sl_def (set_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    DESC_RC (sl_getp (desc)) = sl_getp (rc);
}
sl_enddef

sl_def (inc_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    DESC_RC (sl_getp (desc)) += sl_getp (rc);
}
sl_enddef

sl_def (dec_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    DESC_RC (sl_getp (desc)) -= sl_getp (rc);
}
sl_enddef

sl_def (dec_and_get_rc, void, sl_glparm (int *, desc), sl_shparm (int, val))
{
    DESC_RC (sl_getp (desc)) -= sl_getp (val);
    sl_setp (val, DESC_RC (sl_getp (desc)));
}
sl_enddef

sl_def (get_rc, void, sl_glparm (int *, desc), sl_shparm (int, val))
{
    int dummy = sl_getp (val);
    sl_setp (val, DESC_RC (sl_getp (desc)));
}
sl_enddef
