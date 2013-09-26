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
#ifdef SAC_BACKEND_MUTC
#define SAC_DO_COMPILE_MODULE 1
#include <sac.h>

#undef SAC_MUTC_DEBUG_RC
#define SAC_MUTC_DEBUG_RC(a)
#define SAC_MUTC_ASSERT_RC(a, b)

/*   Original functions */
sl_def (SAC_set_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_set_rc( %p, %d) was: %d\n", (void *)sl_getp (desc),
                               sl_getp (rc), DESC_RC (sl_getp (desc))););
    DESC_RC (sl_getp (desc)) = sl_getp (rc);
}
sl_enddef

sl_def (SAC_inc_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_inc_rc( %p, %d) was: %d\n", (void *)sl_getp (desc),
                               sl_getp (rc), DESC_RC (sl_getp (desc))););
    SAC_MUTC_ASSERT_RC (DESC_RC (sl_getp (desc)), sl_getp (desc));
    DESC_RC (sl_getp (desc)) += sl_getp (rc);
}
sl_enddef

sl_def (SAC_dec_rc, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_dec_rc( %p, %d) was: %d\n", (void *)sl_getp (desc),
                               sl_getp (rc), DESC_RC (sl_getp (desc))););
    SAC_MUTC_ASSERT_RC (DESC_RC (sl_getp (desc)), sl_getp (desc));
    DESC_RC (sl_getp (desc)) -= sl_getp (rc);
}
sl_enddef

sl_def (SAC_dec_and_get_rc, void, sl_glparm (int *, desc), sl_shparm (int, val))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_dec_and_get_rc( %p, %d) was: %d\n",
                               (void *)sl_getp (desc), sl_getp (val),
                               DESC_RC (sl_getp (desc))););
    SAC_MUTC_ASSERT_RC (DESC_RC (sl_getp (desc)), sl_getp (desc));
    DESC_RC (sl_getp (desc)) -= sl_getp (val);
    sl_setp (val, DESC_RC (sl_getp (desc)));
}
sl_enddef

sl_def (SAC_dec_and_maybeFree_rc, void, sl_glparm (int *, desc), sl_glparm (int, val),
        sl_glparm (void *, data))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_dec_and_maybeFree_rc( %p, %d) was: %d\n",
                               (void *)sl_getp (desc), sl_getp (val),
                               DESC_RC (sl_getp (desc))););
    SAC_MUTC_ASSERT_RC (DESC_RC (sl_getp (desc)), sl_getp (desc));
    DESC_RC (sl_getp (desc)) -= sl_getp (val);
    if (DESC_RC (sl_getp (desc)) == 0) {
        free (sl_getp (data));
        free (sl_getp (desc));
    }
}
sl_enddef

sl_def (SAC_get_rc, void, sl_glparm (int *, desc), sl_shparm (int, val))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_get_rc( %p, %d) was: %d\n", (void *)sl_getp (desc),
                               sl_getp (val), DESC_RC (sl_getp (desc))););
    SAC_MUTC_ASSERT_RC (DESC_RC (sl_getp (desc)), sl_getp (desc));
    int dummy = sl_getp (val);
    sl_setp (val, (DESC_RC (sl_getp (desc)) + dummy - dummy));
}
sl_enddef

sl_def (SAC_rc_barrier, void, sl_glparm (int *, desc))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_rc_barrier( %p)\n", (void *)sl_getp (desc)););
}
sl_enddef

sl_def (SAC_dec_and_maybeFree_parent, void, sl_glparm (SAC_ND_DESC_PARENT_TYPE, parent))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_dec_and_maybeFree_parent( %p) was: %d\n",
                               (void *)sl_getp (desc), sl_getp (parent)[0]););
    sl_getp (parent)[0]--;
    if (sl_getp (parent)[0] == 0) {
        free (sl_getp (parent));
    }
}
sl_enddef

sl_def (SAC_inc_parent_count, void, sl_glparm (SAC_ND_DESC_PARENT_TYPE, parent))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_inc_parent_count( %p) was: %d\n",
                               (void *)sl_getp (desc), sl_getp (parent)[0]););
    sl_getp (parent)[0]++;
}
sl_enddef

sl_def (SAC_get_parent_count, void, sl_glparm (SAC_ND_DESC_PARENT_TYPE, parent),
        sl_shparm (SAC_ND_DESC_PARENT_BASETYPE, val))
{
    SAC_MUTC_DEBUG_RC (printf ("SAC_get_parent_count( %p, %d) was: %d\n",
                               (void *)sl_getp (parent), (int)sl_getp (val),
                               (int)sl_getp (parent)[0]););
    sl_setp (val, sl_getp (parent)[0] + sl_getp (val));
}
sl_enddef

/*  Wrapper functions   */
sl_def (SAC_set_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    sl_create (, SAC_MUTC_RC_PLACES_VAR, , , , , sl__exclusive, SAC_set_rc,
               sl_glarg (int *, , sl_getp (desc)), sl_glarg (int, , sl_getp (rc)));
    sl_detach ();
}
sl_enddef

sl_def (SAC_inc_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    sl_create (, SAC_MUTC_RC_PLACES_VAR, , , , , sl__exclusive, SAC_inc_rc,
               sl_glarg (int *, , sl_getp (desc)), sl_glarg (int, , sl_getp (rc)));
    sl_detach ();
}
sl_enddef

sl_def (SAC_dec_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, rc))
{
    sl_create (, SAC_MUTC_RC_PLACES_VAR, , , , , sl__exclusive, SAC_dec_rc,
               sl_glarg (int *, , sl_getp (desc)), sl_glarg (int, , sl_getp (rc)));
    sl_detach ();
}
sl_enddef

sl_def (SAC_dec_and_maybeFree_rc_w, void, sl_glparm (int *, desc), sl_glparm (int, val),
        sl_glparm (void *, data))
{
    sl_create (, SAC_MUTC_RC_PLACES_VAR, , , , , sl__exclusive, SAC_dec_and_get_rc,
               sl_glarg (int *, , sl_getp (desc)), sl_sharg (int, val2, sl_getp (val)));
    sl_sync ();
    if (sl_geta (val2) == 0) {
        free (sl_getp (data));
        free (sl_getp (desc));
    }
}
sl_enddef

sl_def (SAC_get_rc_w, void, sl_glparm (int *, desc), sl_shparm (int, val))
{
    sl_create (, SAC_MUTC_RC_PLACES_VAR, , , , , sl__exclusive, SAC_get_rc,
               sl_glarg (int *, , sl_getp (desc)), sl_sharg (int, val2, sl_getp (val)));
    sl_sync ();
    sl_setp (val, sl_geta (val2));
}
sl_enddef

sl_def (SAC_rc_barrier_w, void, sl_glparm (int *, desc))
{
    sl_create (, SAC_MUTC_RC_PLACES_VAR, , , , , sl__exclusive, SAC_rc_barrier,
               sl_glarg (int *, , sl_getp (desc)));
    sl_sync ();
}
sl_enddef
#endif
