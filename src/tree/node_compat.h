/*
 * $Log$
 * Revision 1.10  2004/10/21 17:19:42  sah
 * removes macro for MakeNWith as it is no more needed
 *
 * Revision 1.9  2004/10/13 15:18:28  sah
 * MakeIdFromIds works in NEW_AST mode now!
 *
 * Revision 1.8  2004/10/12 13:21:41  sah
 * added special handling of WLseg WLsegVar MakeNWith2
 *
 * Revision 1.7  2004/10/05 16:16:52  sah
 * added MakeCSEinfo
 *
 * Revision 1.6  2004/09/23 21:12:55  sah
 * added NWith (temporarily)
 *
 * Revision 1.5  2004/08/29 18:10:05  sah
 * general improvements
 *
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

#define MakeId(name, mod, status) MakeId (MakeIds (name, mod, status))
#define MakeSSAcnt(next, count, baseid) MakeSSAcnt (count, baseid, next)
#define MakeLet(expr, ids) MakeLet (ids, expr)
#define MakeArray(elems, shp) MakeArray (shp, elems)
#define MakeVinfo(flag, type, next, dollar) MakeVinfo (flag, type, dollar, next)
#define MakeNGenerator(b1, b2, p1, p2, s, w) MakeNGenerator (p1, p2, b1, b2, s, w)
#define MakeNPart(withid, gen, code) MakeNPart (code, withid, gen)
#define MakeCast(a, b) MakeCast (b, a)
#define MakeMop(a, b, c) MakeMop (b, c, a)
#define MakeSSAstack(a, b) MakeSSAstack (b, a)
#define MakeCSEinfo(a, b, c) MakeCSEinfo (b, c, a)
#define MakeNWith2(withid, seg, code, withop, dims)                                      \
    MakeNWith2 (dims, withid, seg, code, withop)
#define MakeWLgrid(level, dim, b1, b2, unr, nextd, next, code)                           \
    MakeWLgrid (level, dim, b1, b2, unr, code, nextd, next)
#define MakeWLgridVar(level, dim, b1, b2, nextd, next, code)                             \
    MakeWLgridVar (level, dim, b1, b2, code, nextd, next)

#define NWITHOP_ARRAY(n) NWITHOP_SHAPEARRAYNEUTRAL (n)
#define NWITHOP_SHAPE(n) NWITHOP_SHAPEARRAYNEUTRAL (n)
#define NWITHOP_NEUTRAL(n) NWITHOP_SHAPEARRAYNEUTRAL (n)

#define NGEN_BOUND1(n) NGENERATOR_BOUND1 (n)
#define NGEN_BOUND2(n) NGENERATOR_BOUND2 (n)
#define NGEN_STEP(n) NGENERATOR_STEP (n)
#define NGEN_WIDTH(n) NGENERATOR_WIDTH (n)
#define NGEN_OP1(n) NGENERATOR_OP1 (n)
#define NGEN_OP2(n) NGENERATOR_OP2 (n)
#define NGEN_OP1_ORIG(n) NGENERATOR_OP1_ORIG (n)
#define NGEN_OP2_ORIG(n) NGENERATOR_OP2_ORIG (n)
#define NPART_GEN(n) NPART_GENERATOR (n)

#define MakeFlatArray(aelems) MakeArray (aelems, SHCreateShape (1, CountExprs (aelems)))

/* N_annotate uses its own flags, here are the defines */
#define CALL_FUN 0x0001
#define RETURN_FROM_FUN 0x0002
#define INL_FUN 0x0004
#define LIB_FUN 0x0008
#define OVRLD_FUN 0x0010

#else /* NO_COMPAT */

#undef MakeId
#undef MakeSSAcnt
#undef MakeLet
#undef MakeNWithOp
#undef MakeArray
#undef MakeFlatArray
#undef MakeCast
#undef MakeMop
#undef MakeSSAstack
#undef MakeNGenerator
#undef MakeNPart
#undef MakeCSEinfo
#undef MakeNWith2
#undef MakeWLgrid
#undef MakeWLgridVar

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
