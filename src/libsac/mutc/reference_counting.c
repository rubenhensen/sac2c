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
#include <svp/delegate.h>

#ifdef SAC_MUTC_DEBUG_RC
#include <stdio.h>
#undef SAC_MUTC_DEBUG_RC
#define SAC_MUTC_DEBUG_RC(a) a
#define SAC_MUTC_MAX_RC 5
#define SAC_MUTC_ASSERT_RC(a, b)                                                         \
    SAC_MUTC_DEBUG_RC (if (a < 0 || a > SAC_MUTC_MAX_RC) {                               \
        printf ("%p looks corrupt with value %d\n", b, a);                               \
        abort ();                                                                        \
    })
#else
#define SAC_MUTC_DEBUG_RC(a)
#define SAC_MUTC_ASSERT_RC(a, b)
#endif

/*   Original functions */
sl_def (SAC_set_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_set_rc( %p, %d) was: %d\n", sl_getp (desc),
                               sl_getp (rc), DESC_RC (sl_getp (desc))););
    DESC_RC (sl_getp (desc)) = sl_getp (rc);
}
sl_enddef

sl_def (SAC_inc_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_inc_rc( %p, %d) was: %d\n", sl_getp (desc),
                               sl_getp (rc), DESC_RC (sl_getp (desc))););
    SAC_MUTC_ASSERT_RC (DESC_RC (sl_getp (desc)), sl_getp (desc));
    DESC_RC (sl_getp (desc)) += sl_getp (rc);
}
sl_enddef

sl_def (SAC_dec_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_dec_rc( %p, %d) was: %d\n", sl_getp (desc),
                               sl_getp (rc), DESC_RC (sl_getp (desc))););
    SAC_MUTC_ASSERT_RC (DESC_RC (sl_getp (desc)), sl_getp (desc));
    DESC_RC (sl_getp (desc)) -= sl_getp (rc);
}
sl_enddef

sl_def (SAC_dec_and_get_rc, void, sl_glparm (int *, desc), sl_shparm (int, val))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_dec_and_get_rc( %p, %d) was: %d\n", sl_getp (desc),
                               sl_getp (val), DESC_RC (sl_getp (desc))););
    SAC_MUTC_ASSERT_RC (DESC_RC (sl_getp (desc)), sl_getp (desc));
    DESC_RC (sl_getp (desc)) -= sl_getp (val);
    sl_setp (val, DESC_RC (sl_getp (desc)));
}
sl_enddef

sl_def (SAC_get_rc, void, sl_glparm (int *, desc), sl_shparm (int, val))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_get_rc( %p, %d) was: %d\n", sl_getp (desc),
                               sl_getp (val), DESC_RC (sl_getp (desc))););
    SAC_MUTC_ASSERT_RC (DESC_RC (sl_getp (desc)), sl_getp (desc));
    int dummy = sl_getp (val);
    sl_setp (val, (DESC_RC (sl_getp (desc)) + dummy));
}
sl_enddef

/*  Wrapper functions   */
sl_def (SAC_set_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    sl_create (, PLACE_LOCAL | 1, , , , , sl__exclusive, SAC_set_rc,
               sl_glarg (int *, , sl_getp (desc)), sl_glarg (int, , sl_getp (rc)));
    sl_detach ();
}
sl_enddef

sl_def (SAC_inc_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    sl_create (, PLACE_LOCAL | 1, , , , , sl__exclusive, SAC_inc_rc,
               sl_glarg (int *, , sl_getp (desc)), sl_glarg (int, , sl_getp (rc)));
    sl_detach ();
}
sl_enddef

sl_def (SAC_dec_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    sl_create (, PLACE_LOCAL | 1, , , , , sl__exclusive, SAC_dec_rc,
               sl_glarg (int *, , sl_getp (desc)), sl_glarg (int, , sl_getp (rc)));
    sl_detach ();
}
sl_enddef
