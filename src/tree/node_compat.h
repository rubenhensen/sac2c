/*
 * $Log$
 * Revision 1.2  2004/07/11 18:08:56  sah
 * ongoing new ast implementation
 *
 * Revision 1.1  2004/07/03 15:12:39  sah
 * Initial revision
 *
 *
 */

#ifndef _sac_node_compat_h
#define _sac_node_compat_h

/*****************************************************************************
 * This section defines Macros that should be moved to tree_compound.h
 * in the near future.
 */

/*****************************************************************************
 * N_Id:
 */

#define ID_NAME(n) (IDS_NAME (ID_IDS (n)))
#define ID_AVIS(n) (IDS_AVIS (ID_IDS (n)))
#define ID_VARDEC(n) (IDS_VARDEC (ID_IDS (n)))
#define ID_OBJDEF(n) (IDS_VARDEC (ID_IDS (n)))
#define ID_MOD(n) (IDS_MOD (ID_IDS (n)))
#define ID_STATUS(n) (IDS_STATUS (ID_IDS (n)))
#define ID_DEF(n) (IDS_DEF (ID_IDS (n)))
#define ID_REFCNT(n) (IDS_REFCNT (ID_IDS (n)))
#define ID_NAIVE_REFCNT(n) (IDS_NAIVE_REFCNT (ID_IDS (n)))

#endif

#ifndef AST_NO_COMPAT
/*****************************************************************************
 * This section defines compatibility macros for code written for the old ast
 */

#define MakeId(name, mod, status) MakeId (MakeIds (name, mod, status))
#define MakeSSAcnt(next, count, baseid) MakeSSAcnt (count, baseid, next)
#define MakeLet(expr, ids) MakeLet (ids, expr)
#define MakeArray(elems, shp) MakeArray (shp, elems)
#define MakeVinfo(flag, type, next, dollar) MakeVinfo (flag, type, dollar, next)
#define MakeNGenerator(b1, b2, p1, p2, s, w) MakeNGenerator (p1, p2, b1, b2, s, w)

/***
 * The NWithOP node has three sons, but only one at a time is used. The
 * new AST currently encodes this as three separate fields, thus
 * it is just copied to all three fields
 ***/
#define MakeNWithOp(type, shp_array_neutral)                                             \
    MakeNWithOp (type, shp_array_neutral, shp_array_neutral, shp_array_neutral)

#define MakeFlatArray(aelems) MakeArray (aelems, SHCreateShape (1, CountExprs (aelems)))
#else

#undef MakeId
#undef MakeSSAcnt
#undef MakeLet
#undef MakeNWithOp
#undef MakeArray
#undef MakeFlatArray
#endif
