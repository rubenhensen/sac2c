/*
 * $Log$
 * Revision 1.4  2004/08/06 14:41:32  sah
 * adding support for new ast
 *
 * Revision 1.3  2004/07/31 16:16:57  sah
 * added support for flags and moved to memory saving attribute
 * structure.
 *
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

#define MakeIdFromIds(ids) MakeId (IDS_NAME (ids), IDS_MOD (ids), IDS_STATUS (ids))
#define MakeId(name, mod, status) MakeId (MakeIds (name, mod, status))
#define MakeSSAcnt(next, count, baseid) MakeSSAcnt (count, baseid, next)
#define MakeLet(expr, ids) MakeLet (ids, expr)
#define MakeArray(elems, shp) MakeArray (shp, elems)
#define MakeVinfo(flag, type, next, dollar) MakeVinfo (flag, type, dollar, next)
#define MakeNGenerator(b1, b2, p1, p2, s, w) MakeNGenerator (p1, p2, b1, b2, s, w)
#define MakeCast(a, b) MakeCast (b, a)
#define MakeMop(a, b, c) MakeMop (b, c, a)
#define MakeNWith2(a, b, c, d, e) MakeNWith2 (e, a, b, d, c)
#define MakeSSAstack(a, b) MakeSSAstack (b, a)

#define NGEN_BOUND1(n) NGENERATOR_BOUND1 (n)
#define NGEN_BOUND2(n) NGENERATOR_BOUND2 (n)
#define NGEN_STEP(n) NGENERATOR_STEP (n)
#define NGEN_WIDTH(n) NGENERATOR_WIDTH (n)
#define NGEN_OP1(n) NGENERATOR_OP1 (n)
#define NGEN_OP2(n) NGENERATOR_OP2 (n)
#define NGEN_OP1_ORIG(n) NGENERATOR_OP1_ORIG (n)
#define NGEN_OP2_ORIG(n) NGENERATOR_OP2_ORIG (n)
#define NPART_GEN(n) NPART_GENERATOR (n)

/***
 * The NWithOP node has three sons, but only one at a time is used. The
 * new AST currently encodes this as three separate fields, thus
 * it is just copied to all three fields
 ***/
#define MakeNWithOp(type, shp_array_neutral)                                             \
    MakeNWithOp (type, shp_array_neutral, shp_array_neutral, shp_array_neutral)

#define MakeFlatArray(aelems) MakeArray (aelems, SHCreateShape (1, CountExprs (aelems)))

/* N_annotate uses its own flags, here are the defines */
#define CALL_FUN 0x0001
#define RETURN_FROM_FUN 0x0002
#define INL_FUN 0x0004
#define LIB_FUN 0x0008
#define OVRLD_FUN 0x0010

#else /* NO_COMPAT */

#undef MakeIdFromIds
#undef MakeId
#undef MakeSSAcnt
#undef MakeLet
#undef MakeNWithOp
#undef MakeArray
#undef MakeFlatArray
#undef MakeCast
#undef MakeMop
#undef MakeNWith2
#undef MakeSSAstack

#undef NGEN_BOUND1
#undef NGEN_BOUND2
#undef NGEN_STEP
#undef NGEN_WIDTH
#undef NGEN_OP1
#undef NGEN_OP2
#undef NGEN_OP1_ORIG
#undef NGEN_OP2_ORIG
#undef NPART_GEN

#endif
