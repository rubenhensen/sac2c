/*
 * $Log$
 * Revision 1.85  2000/07/28 08:32:37  mab
 * added INFO_APC_WITH
 *
 * Revision 1.84  2000/07/25 08:11:55  mab
 * added INFO_APC_COUNT_CHANGES
 *
 * Revision 1.83  2000/07/21 15:14:45  mab
 * added INFO_APC_UNSUPPORTED
 *
 * Revision 1.82  2000/07/21 14:47:01  nmw
 * macros for INFO_IMPSPEC_ added
 *
 * Revision 1.81  2000/07/21 11:30:35  jhs
 * Added FUNDEF_MT2USE, FUNDEF_MT2DEF and FUNDEF_IDENTIFIER.
 *
 * Revision 1.80  2000/07/21 08:20:42  nmw
 * Modspec added
 *
 * Revision 1.79  2000/07/19 15:41:58  nmw
 * flag ICM_END_OF_STATEMENT added, forces printing of ;
 *
 * Revision 1.78  2000/07/14 13:17:14  nmw
 * macro for precompile added, comments updated
 *
 * Revision 1.77  2000/07/13 11:58:12  jhs
 * Splited ICM_INDENT into ICM_INDENT_BEFORE and ICM_INDENT_AFTER.
 *
 * Revision 1.76  2000/07/12 17:22:57  dkr
 * comments for N_typedef added
 *
 * Revision 1.75  2000/07/12 15:20:14  dkr
 * INFO_DUP_BASEFUNDEF and INFO_DUP_DFMBASE removed
 *
 * Revision 1.74  2000/07/12 09:23:23  nmw
 * macros changed PIH_ and MCW_
 *
 * Revision 1.73  2000/07/11 15:44:14  jhs
 * Added ST_ALLOC and ST_SYNC.
 * DFMfoldmask changed name to vardec.
 *
 * Revision 1.72  2000/07/10 14:22:57  cg
 * Added new field type_status in types struct as a dedicated status field
 * for the type itself.
 *
 * Revision 1.71  2000/07/07 15:48:30  bs
 * The following compound macros moved from tree_basic to tree_compound:
 * INFO_WLAA_ARRAYSHP, INFO_WLAA_INDEXDIM, INFO_WLAA_ARRAYDIM
 * INFO_TSI_ARRAYSHP, INFO_TSI_INDEXDIM, INFO_TSI_ARRAYDIM
 *
 * Revision 1.70  2000/07/07 08:57:01  nmw
 * MCW_CNT_ARTIFICIAL macro added
 *
 * Revision 1.68  2000/07/06 15:32:04  mab
 * added INFO_APT_ASSIGNMENTS
 *
 * Revision 1.67  2000/07/06 14:43:21  mab
 * added INFO_APT_FUNDEF
 *
 * Revision 1.66  2000/07/04 14:35:21  jhs
 * Added CopyDFMfoldmask.
 *
 * Revision 1.65  2000/06/30 14:27:46  mab
 * added NCODE_APT_DUMMY_CODE
 *
 * Revision 1.64  2000/06/30 14:10:29  mab
 * removed INFO_APT_WITH_PARTS, INFO_APT_WITH_CODE, INFO_APT_WITH_PART
 * added INFO_APT_WITH
 *
 * Revision 1.63  2000/06/30 11:00:54  mab
 * removed INFO_APT_CODE_ID
 * added INFO_APT_WITH_CODE and INFO_APT_WITH_PART
 *
 * Revision 1.62  2000/06/29 16:05:28  mab
 * added INFO_APT_CODE_ID
 *
 * Revision 1.61  2000/06/29 14:56:47  mab
 * added INFO_APT_WITHOP_TYPE and INFO_APT_WITH_PARTS
 *
 * Revision 1.60  2000/06/29 12:18:21  nmw
 * additional macros for INFO_PIW_ added
 *
 * Revision 1.59  2000/06/29 10:33:09  mab
 * added NPART_PADDED
 * changed type of *_PADDED from int to bool
 *
 * Revision 1.58  2000/06/29 10:25:02  mab
 * added NPART_PADDED
 * changed type of *_PADDED from int to bool
 *
 * Revision 1.57  2000/06/28 15:12:46  nmw
 * added macros for INFO_PIW and INFO_PIH
 *
 * Revision 1.56  2000/06/23 16:41:04  nmw
 * macros for INFO_MCW added
 *
 * Revision 1.55  2000/06/23 15:31:35  nmw
 * N_cwrapper node added
 *
 * Revision 1.54  2000/06/23 14:17:14  dkr
 * macros for old with-loop removed
 * NWITH_COMPLEX removed
 *
 * Revision 1.53  2000/06/22 13:32:07  mab
 * INFO_APT_EXPRESSION_PADDED corrected
 *
 * Revision 1.52  2000/06/22 09:54:54  nmw
 * changes made in N_info access macros for usage in PIW
 *
 * Revision 1.51  2000/06/21 15:00:49  mab
 * added macros *_PADDED for ARG and VARDEC
 * added INFO_APT_EXPRESSION_PADDED
 *
 * Revision 1.50  2000/06/21 12:36:54  jhs
 * Added MT_ALLOC.
 *
 * Revision 1.49  2000/06/16 15:00:03  nmw
 * N_info for function->wrapper matching added
 *
 * Revision 1.48  2000/06/14 12:05:37  jhs
 * Added ST_IDENTIFIER and MT_IDENTIFIER.
 * Each new N_mt and N_st will be provided with an unique id.
 *
 * Revision 1.47  2000/06/13 13:41:20  dkr
 * Make...() functions for old with-loop removed
 *
 * Revision 1.46  2000/06/08 12:15:38  jhs
 * Added some INFO_DFA_XXX stuff
 *
 * Revision 1.45  2000/05/29 14:30:18  dkr
 * minor error in comment of N_Ncode corrected
 *
 * Revision 1.44  2000/05/26 11:11:12  jhs
 * Added INFO_COMP_ACTUALATTRIB
 *
 * Revision 1.43  2000/05/25 22:52:01  dkr
 * some comments changed for N_fundef
 *
 * Revision 1.42  2000/05/24 18:57:15  dkr
 * macros for old with-loop separated
 *
 * Revision 1.41  2000/04/20 11:38:33  jhs
 * Added comment at MT_FUNDEF.
 *
 * Revision 1.40  2000/04/13 09:00:48  jhs
 * Added INFO_MUTH_TOPDOWN.
 *
 * Revision 1.39  2000/04/12 17:27:26  jhs
 * Added INFO_MUTH_ALLOW_OOOC.
 *
 * Revision 1.38  2000/03/31 14:10:26  dkr
 * comment corrected
 *
 * Revision 1.37  2000/03/31 12:26:37  jhs
 * Added INFO_DUP_BASEFUNDEF
 *
 * Revision 1.36  2000/03/30 15:13:13  jhs
 * Added adjustcalls
 *
 * Revision 1.35  2000/03/29 16:10:49  jhs
 * MT_WORKERFUN and MT_MASTERFUN added.
 *
 * Revision 1.34  2000/03/24 00:50:24  dkr
 * INFO_DUP_LUT added
 *
 * Revision 1.33  2000/03/23 14:03:33  jhs
 * Added macros for DFMfoldmask_t (DFMFM) ann MakeDFMfoldmask.
 *
 * Revision 1.32  2000/03/22 17:37:28  jhs
 * Added N_MTsignal, N_MTalloc, N_MTsync macros.
 *
 * [...]
 *
 * Revision 1.1  2000/01/21 15:38:35  dkr
 * Initial revision
 *
 * Revision 2.63  2000/01/21 12:42:52  dkr
 * function MakeIds1 added
 * macros INFO_LAC2FUN_... added
 *
 * [...]
 *
 * Revision 2.1  1999/02/23 12:39:56  sacbase
 * new release made
 *
 * Revision 1.216  1999/02/12 17:44:51  cg
 * WLSEGX mechanism now also used for WLSEG_SCHEDULING and WLSEGVAR_SCHEDULING
 *
 * [...]
 *
 * Revision 1.176  1998/05/17 00:08:15  dkr
 * WLGRID_CEXPR_TEMPLATE is now WLGRID_CODE_TEMPLATE
 *
 * [...]
 */

/*============================================================================

How to read this file ?

This file contains the basic structure of the new virtual syntax tree.

First, it distinguishes between non-node structures such as SHAPES or
TYPES and node structures such as N_fundef or N_modul. The node
structures build up the main part of the syntax tree while the
non-node structures are designed for special purposes within the syntax
tree.

Each structure is described in three sections: the description section,
the remark section, and the implementation section. The description
section presents a detailed view of the structure in a formal way. The
optional remark section gives additional information in plain text where
necessary. The implementation section contains function declarations and
macro definitions.

The description section is organized as follows:
Each structure consists of a name and couple of slots. Each slot consists
of a type and a name. If the type is node*, then the required node type
is given as well. All types of global use are stored in types.h.

Three different kinds of slots are distinguished: sons, permant attributes,
and temporary attributes. All of them may be only optional which is marked
by an (O).

Sons are traversed (if present) by the automatic traversal mechanism.

Permanent attributes are unchanged throughout the
different compilation phases. They mainly contain information which is
derived from the SAC source code.

Temporary attributes contain additional
information which is gathered during one of the many compilation steps.
It may then stay unchanged in following compliation steps, be changed,
or even discarded. These situations are characterized in the following
way:

(typecheck -> )

means that the attribute is derived by the typechecker and then stays
unchanged.

(refcount -> compile -> )

means that the attribute is derived by the refcounter and is used during
compilation but it stays unchanged.

(typecheck -> compile !!)

means that the attribute is derived by the typechecker and is used
during compilation but is not available any further.

The following compilation steps are used:

 - scanparse
 - objinit
 - import
 - flatten
 - typecheck
 - impl_types
 - analysis
 - write-SIB
 - obj-handling
 - unique-check
 - rm-void-fun
 - optimize
   - inl  = function inlining
   - ael  = array elimination
   - wli  = withloop information
   - wlf  = withloop folding
   - dcr1 = search for redundant code
   - dcr2 = dead code removal
 - psi-optimize
 - refcount
 - precompile
 - compile

In general, the implementation section consists of the declaration of
a create function for the respective structure and one basic access
macro for each slot. The definitions of all functions declared in this
file can be found in tree_basic.c

============================================================================*/

#ifndef _sac_tree_basic_h

#define _sac_tree_basic_h

#include "types.h"

/*
 * Uncomment the #define statement to use new virtual syntaxtree
 */

#define NEWTREE

/*
 *   Decalarations of global variables exported by tree_basic.c
 */

extern char *prf_name_str[];

/*
 *   Global Access-Macros
 *   --------------------
 */

#define NODE_TYPE(n) ((n)->nodetype)

#define NODE_LINE(n) ((n)->lineno)

#define NODE_FILE(n) ((n)->src_file)

#define NODE_TEXT(n) (mdb_nodetype[NODE_TYPE (n)])

/*
 *   Non-node-structures
 *   -------------------
 */

/***
 ***  SHAPES :
 ***
 ***  permanent attributes:
 ***
 ***    int                DIM
 ***    int[SHP_SEG_SIZE]  SELEMS  (O)
 ***
 ***/

#define SHAPES_DIM(s) (s->dim)
#define SHAPES_SHPSEG(s) (s->shpseg)
#define SHAPES_SELEMS(s) (s->shpseg->shp)
#define SHAPES_SNEXT(s) (s->shpseg->next)

/*--------------------------------------------------------------------------*/

/***
 ***  SHPSEG :
 ***
 ***  permanent attributes:
 ***
 ***    int[SHP_SEG_SIZE]  SHAPE
 ***
 ***/

extern shpseg *MakeShpseg (nums *num);

#define SHPSEG_SHAPE(s, x) (s->shp[x])
#define SHPSEG_NEXT(s) (s->next)

/*--------------------------------------------------------------------------*/

/***
 ***  TYPES :
 ***
 ***  permanent attributes:
 ***
 ***    simpletype         BASETYPE
 ***    int                DIM
 ***    shpseg*            SHPSEG    (O)
 ***    char*              NAME      (O)
 ***    char*              MOD       (O)
 ***    statustype         STATUS
 ***    types*             NEXT      (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*              TDEF      (O)  (typecheck -> )
 ***/

/*
 *  STATUS is usually ST_regular, but
 *    ST_artificial marks artificial return types due to the resolution of
 *      reference parameters and global objects and
 *    ST_crettype marks that return type of a function that is compiled
 *      to the actual return type of the resulting C function.
 *
 *  TDEF is a reference to the defining N_typedef node of a user-defined
 *  type.
 */

extern types *MakeType (simpletype basetype, int dim, shpseg *shpseg, char *name,
                        char *mod);

#define TYPES_BASETYPE(t) (t->simpletype)
#define TYPES_DIM(t) (t->dim)
#define TYPES_SHPSEG(t) (t->shpseg)
#define TYPES_NAME(t) (t->name)
#define TYPES_MOD(t) (t->name_mod)
#define TYPES_STATUS(t) (t->type_status)
#define TYPES_TDEF(t) (t->tdef)
#define TYPES_NEXT(t) (t->next)

/*--------------------------------------------------------------------------*/

/***
 ***  IDS :
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       MOD         (O)
 ***    statustype  ATTRIB
 ***    ids*        NEXT        (O)
 ***
 ***  temporary attributes:
 ***
 ***    int         REFCNT                          (refcount -> )
 ***    int         NAIVE_REFCNT                    (refcount -> )
 ***    node*       VARDEC       (N_vardec/N_arg)   (typecheck -> )
 ***    node*       DEF                             (psi-optimize -> )
 ***    node*       USE                             (psi-optimize -> )
 ***    statustype  STATUS                          (obj-handling -> compile !!)
 ***/

/*
 *  ATTRIB: ST_regular    :  local variable or function parameter
 *          ST_global     :  reference to global object
 *
 *  STATUS: ST_regular    :  from original source code
 *          ST_artificial :  added by obj-handling
 *
 */

extern ids *MakeIds (char *name, char *mod, statustype status);

extern ids *MakeIds1 (char *name);

#define IDS_NAME(i) (i->id)
#define IDS_MOD(i) (i->mod)
#define IDS_REFCNT(i) (i->refcnt)
#define IDS_NAIVE_REFCNT(i) (i->naive_refcnt)
#define IDS_NEXT(i) (i->next)
#define IDS_VARDEC(i) (i->node)
#define IDS_DEF(i) (i->def)
#define IDS_USE(i) (i->use)
#define IDS_STATUS(i) (i->status)
#define IDS_ATTRIB(i) (i->attrib)

/*--------------------------------------------------------------------------*/

/***
 ***  NUMS :
 ***
 ***  permanent attributes:
 ***
 ***    int    NUM
 ***    nums*  NEXT  (O)
 ***/

extern nums *MakeNums (int num, nums *next);

#define NUMS_NUM(n) (n->num)
#define NUMS_NEXT(n) (n->next)

/*--------------------------------------------------------------------------*/

/***
 ***  STRINGS :
 ***
 ***  permanent attributes:
 ***
 ***    char*  STRING
 ***    nums*  NEXT    (O)
 ***/

extern strings *MakeStrings (char *string, strings *next);

#define STRINGS_STRING(s) (s->name)
#define STRINGS_NEXT(s) (s->next)

/*--------------------------------------------------------------------------*/

/***
 ***  DEPS :
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       DECNAME
 ***    char*       LIBNAME
 ***    statustype  STATUS
 ***    deps*       SUB        (O)
 ***    deps*       NEXT       (O)
 ***/

extern deps *MakeDeps (char *name, char *decname, char *libname, statustype status,
                       deps *sub, deps *next);

#define DEPS_NAME(d) (d->name)
#define DEPS_DECNAME(d) (d->decname)
#define DEPS_LIBNAME(d) (d->libname)
#define DEPS_STATUS(d) (d->status)
#define DEPS_SUB(d) (d->sub)
#define DEPS_NEXT(d) (d->next)

/*--------------------------------------------------------------------------*/

/***
 ***  NODELIST :
 ***
 ***  permanent attributes:
 ***
 ***    node*       NODE
 ***    statustype  ATTRIB
 ***    void*       ATTRIB2
 ***    statustype  STATUS
 ***    nodelist*   NEXT    (O)
 ***/

/*
 *  Possible values for ATTRIB :
 *      in function node lists : ST_resolved | ST_unresolved
 *      in object node lists   : ST_reference | ST_readonly_reference
 *      in typedef node lists  : ST_regular
 *  Possible values for STATUS : ST_regular | ST_artificial
 */

/* srs: to use a nodelist in more general situations I have inserted a
   new attribut ATTRIB2 which can store any suitable information.
   Functions to handle a general node list can be found in tree_compound.h,
   starting with NodeList... .
   MakeNodelist(), MakeNodelistNode() are not needed to create the general
   node list. */

extern nodelist *MakeNodelist (node *node, statustype status, nodelist *next);
extern nodelist *MakeNodelistNode (node *node, nodelist *next);

#define NODELIST_NODE(n) (n->node)
#define NODELIST_ATTRIB(n) (n->attrib)
#define NODELIST_ATTRIB2(n) (n->attrib2)
#define NODELIST_STATUS(n) (n->status)
#define NODELIST_NEXT(n) (n->next)

/*--------------------------------------------------------------------------*/

/***
 ***  ACCESS_T :
 ***
 ***  permanent attributes:
 ***
 ***    node*         ARRAY         (N_vardec/N_arg)
 ***    node*         IV            (N_vardec/N_arg)
 ***    accessclass_t CLASS
 ***    shpseg*       OFFSET  (O)
 ***    access_t*     NEXT    (O)
 ***/

extern access_t *MakeAccess (node *array, node *iv, accessclass_t class, shpseg *offset,
                             accessdir_t direction, access_t *next);

#define ACCESS_ARRAY(a) (a->array_vardec)
#define ACCESS_IV(a) (a->iv_vardec)
#define ACCESS_CLASS(a) (a->accessclass)
#define ACCESS_OFFSET(a) (a->offset)
#define ACCESS_DIR(a) (a->direction)
#define ACCESS_NEXT(a) (a->next)

/*--------------------------------------------------------------------------*/

/***
 *** DFMfoldmask_t :
 ***
 ***/

extern DFMfoldmask_t *MakeDFMfoldmask (node *vardec, node *foldop, DFMfoldmask_t *next);
extern DFMfoldmask_t *CopyDFMfoldmask (DFMfoldmask_t *mask);

#define DFMFM_VARDEC(n) (n->vardec)
#define DFMFM_FOLDOP(n) (n->foldop)
#define DFMFM_NEXT(n) (n->next)

/*==========================================================================*/

/*
 *   Node-structures
 *   ---------------
 */

/*--------------------------------------------------------------------------*/

/***
 ***  N_modul :
 ***
 ***  sons:
 ***
 ***    node*      IMPORTS   (O)  (N_implist)
 ***    node*      TYPES     (O)  (N_typedef)
 ***    node*      OBJS      (O)  (N_objdef)
 ***    node*      FUNS      (O)  (N_fundef)
 ***
 ***  permanent attributes:
 ***
 ***    char*      NAME      (O)
 ***    file_type  FILETYPE
 ***    types*     CLASSTYPE (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*      DECL      (O)  (N_moddec, N_classdec)  (check-dec -> )
 ***                                                      ( -> write-SIB !!)
 ***    node*      STORE_IMPORTS (O) (N_implist)          (import -> )
 ***                                                      ( -> checkdec !!)
 ***    node*      FOLDFUNS  (O)  (N_fundef)              (compile -> )
 ***
 ***/

/*
 *  CLASSTYPE points to the type of a class implementation.
 *
 *  The temporary attributes DECL, FOLDFUN, and STORE_IMPORTS are mapped
 *  to the same real node because they are never used in the same
 *  phase of compilation.
 */

extern node *MakeModul (char *name, file_type filetype, node *imports, node *types,
                        node *objs, node *funs);

#define MODUL_NAME(n) (n->info.id)
#define MODUL_FILETYPE(n) ((file_type) (n->varno))
#define MODUL_IMPORTS(n) (n->node[0])
#define MODUL_TYPES(n) (n->node[1])
#define MODUL_OBJS(n) (n->node[3])
#define MODUL_FUNS(n) (n->node[2])
#define MODUL_DECL(n) (n->node[4])
#define MODUL_STORE_IMPORTS(n) (n->node[4])
#define MODUL_FOLDFUNS(n) (n->node[4])
#define MODUL_CWRAPPER(n) (n->node[5])
#define MODUL_CLASSTYPE(n) ((types *)(n->info2))

/*--------------------------------------------------------------------------*/

/***
 ***  N_moddec :
 ***
 ***  sons:
 ***
 ***    node*  IMPORTS    (O)  (N_implist)
 ***    node*  OWN        (O)  (N_explist)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    deps*  LINKWITH   (O)
 ***    int    ISEXTERNAL
 ***/

extern node *MakeModdec (char *name, deps *linkwith, int isexternal, node *imports,
                         node *exports);

#define MODDEC_NAME(n) (n->info.fun_name.id)
#define MODDEC_LINKWITH(n) ((deps *)(n->info.fun_name.id_mod))
#define MODDEC_ISEXTERNAL(n) (n->refcnt)
#define MODDEC_IMPORTS(n) (n->node[1])
#define MODDEC_OWN(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_classdec :
 ***
 ***  sons:
 ***
 ***    node*  IMPORTS  (O)  (N_implist)
 ***    node*  OWN      (O)  (N_explist)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    deps*  LINKWITH (O)
 ***    int    ISEXTERNAL
 ***/

extern node *MakeClassdec (char *name, deps *linkwith, int isexternal, node *imports,
                           node *exports);

#define CLASSDEC_NAME(n) (n->info.fun_name.id)
#define CLASSDEC_LINKWITH(n) ((deps *)(n->info.fun_name.id_mod))
#define CLASSDEC_ISEXTERNAL(n) (n->refcnt)
#define CLASSDEC_IMPORTS(n) (n->node[1])
#define CLASSDEC_OWN(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_sib :
 ***
 ***  sons:
 ***
 ***    node*     TYPES    (O)  (N_typedef)
 ***    node*     OBJS     (O)  (N_objdef)
 ***    node*     FUNS     (O)  (N_fundef)
 ***    node*     NEXT     (O)  (N_sib)
 ***
 ***  permanent attributes:
 ***
 ***    char*     NAME
 ***    int       LINKSTYLE
 ***    deps*     LINKWITH
 ***/

/*
 *  This node structure is used as head structure for SIBs.
 *  LINKSTYLE corresponds to the global variable linkstyle.
 */

extern node *MakeSib (char *name, int linkstyle, deps *linkwith, node *types, node *objs,
                      node *funs);

#define SIB_TYPES(n) (n->node[0])
#define SIB_OBJS(n) (n->node[1])
#define SIB_FUNS(n) (n->node[2])
#define SIB_LINKSTYLE(n) (n->varno)
#define SIB_NAME(n) (n->info.fun_name.id)
#define SIB_LINKWITH(n) ((deps *)(n->info.fun_name.id_mod))
#define SIB_NEXT(n) (n->node[3])

/*--------------------------------------------------------------------------*/

/***
 ***  N_implist :
 ***
 ***  sons:
 ***
 ***    node*  NEXT    (O)  (N_implist)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    ids*   ITYPES  (O)
 ***    ids*   ETYPES  (O)
 ***    ids*   OBJS    (O)
 ***    ids*   FUNS    (O)
 ***/

extern node *MakeImplist (char *name, ids *itypes, ids *etypes, ids *objs, ids *funs,
                          node *next);

#define IMPLIST_NAME(n) (n->info.id)
#define IMPLIST_ITYPES(n) ((ids *)(n->node[1]))
#define IMPLIST_ETYPES(n) ((ids *)(n->node[2]))
#define IMPLIST_OBJS(n) ((ids *)(n->node[4]))
#define IMPLIST_FUNS(n) ((ids *)(n->node[3]))
#define IMPLIST_NEXT(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_explist :
 ***
 ***  sons:
 ***
 ***    node* ITYPES  (O)  (N_typedef)
 ***    node* ETYPES  (O)  (N_typedef)
 ***    node* OBJS    (O)  (N_objdef)
 ***    node* FUNS    (O)  (N_fundef)
 ***/

extern node *MakeExplist (node *itypes, node *etypes, node *objs, node *funs);

#define EXPLIST_ITYPES(n) (n->node[0])
#define EXPLIST_ETYPES(n) (n->node[1])
#define EXPLIST_OBJS(n) (n->node[3])
#define EXPLIST_FUNS(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_typedef :
 ***
 ***  sons:
 ***
 ***    node*       NEXT  (O)  (N_typedef)
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       MOD     (O)
 ***    types*      TYPE
 ***    statustype  ATTRIB
 ***    statustype  STATUS
 ***
 ***  temporary attributes:
 ***
 ***    types*      IMPL         (O)        (import -> )
 ***                                        ( -> writesib !!)
 ***    node*       PRAGMA       (O)        (import -> readsib !!)
 ***    char*       COPYFUN      (O)        (readsib -> compile -> )
 ***    char*       FREEFUN      (O)        (readsib -> compile -> )
 ***    node*       TYPEDEC_DEF  (O)        (checkdec -> writesib !!)
 ***/

/*
 *  The ATTRIB indicates whether a type is unique or not.
 *  Possible values: ST_regular | ST_unique
 *
 *  The STATUS indicates whether a type is defined or imported.
 *  Possible values: ST_regular | ST_imported
 *
 *  The TYPEDEC_DEF slot is only used when a typedef node is used as a
 *  representation of a type declaration. It then points to the
 *  typedef node which contains the respective definition.
 *
 *  For each Non-SAC hidden type the name of a copy and a free function
 *  is stored in COPYFUN and FREEFUN, respectively. These must be provided
 *  with the external module/class. The names may be generic or user-defined
 *  using pragmas.
 */

extern node *MakeTypedef (char *name, char *mod, types *type, statustype attrib,
                          node *next);

#define TYPEDEF_NAME(n) (n->info.types->id)
#define TYPEDEF_MOD(n) (n->info.types->id_mod)
#define TYPEDEF_TYPE(n) (n->info.types)
#define TYPEDEF_ATTRIB(n) (n->info.types->attrib)
#define TYPEDEF_STATUS(n) (n->info.types->status)
#define TYPEDEF_IMPL(n) (n->info.types->next)
#define TYPEDEF_NEXT(n) (n->node[0])
#define TYPEDEC_DEF(n) (n->node[1])
#define TYPEDEF_PRAGMA(n) (n->node[2])
#define TYPEDEF_COPYFUN(n) ((char *)(n->node[3]))
#define TYPEDEF_FREEFUN(n) ((char *)(n->node[4]))

/*--------------------------------------------------------------------------*/

/***
 ***  N_objdef :
 ***
 ***  sons:
 ***
 ***    node*       EXPR  (O)  ("N_expr")
 ***    node*       NEXT  (O)  (N_objdef)
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       MOD     (O)
 ***    char*       LINKMOD (O)
 ***    types*      TYPE
 ***    statustype  ATTRIB
 ***    statustype  STATUS
 ***
 ***  temporary attributes:
 ***
 ***    char*       VARNAME      (typecheck -> obj-handling ->
 ***                             ( -> precompile -> compile -> )
 ***    node*       PRAGMA    (O)  (N_pragma)  (import -> readsib ->
 ***                                            precompile -> )
 ***    node*       ARG       (O)  (obj-handling !!)
 ***    node*       ICM       (O)  (compile ->)
 ***    node*       SIB       (O)  (readsib !!)
 ***    nodelist*   NEEDOBJS  (O)  (import -> analysis -> objects -> )
 ***
 ***    node*       OBJDEC_DEF (O)  (checkdec -> writesib -> )
 ***/

/*
 *  The STATUS indicates whether an object is defined or imported.
 *  Possible values: ST_regular | ST_imported
 *
 *  ATTRIB : ST_regular | ST_resolved
 *  used in objects.c to distinguish between already initialized and
 *  not yet initialized global objects.
 *
 *  The VARNAME is a combination of NAME and MOD. It's used as parameter
 *  name when making global objects local.
 *
 *  ARG is a pointer to the additional argument which is added to a function's
 *  parameter list for this global object. ARG changes while traversing
 *  the functions !!
 *
 *  ICM contains a pointer to the respective icm if the global object
 *  is an array (ND_KS_DECL_ARRAY_GLOBAL or ND_KD_DECL_ARRAY_EXTERN)
 *
 *  ATTENTION: ARG, INIT, and ICM are mapped to the same real node !
 *
 *  LINKMOD contains the name of the module which has to be linked with
 *  in order to make the code of this function available. If LINKMOD is
 *  NULL, then link with the module given by MOD.
 *
 *  The OBJDEC_DEF slot is only used when an objdef node is used as a
 *  representation of an object declaration. It then points to the
 *  objdef node which contains the respective definition.
 *
 */

extern node *MakeObjdef (char *name, char *mod, types *type, node *expr, node *next);

#define OBJDEF_NAME(n) (n->info.types->id)
#define OBJDEF_MOD(n) (n->info.types->id_mod)
#define OBJDEF_LINKMOD(n) (n->info.types->id_cmod)
#define OBJDEF_TYPE(n) (n->info.types)
#define OBJDEF_STATUS(n) (n->info.types->status)
#define OBJDEF_ATTRIB(n) (n->info.types->attrib)
#define OBJDEF_VARNAME(n) ((char *)(n->info2))
#define OBJDEF_NEXT(n) (n->node[0])
#define OBJDEF_EXPR(n) (n->node[1])
#define OBJDEC_DEF(n) (n->node[2])
#define OBJDEF_ARG(n) (n->node[3])
#define OBJDEF_ICM(n) (n->node[3])
#define OBJDEF_SIB(n) (n->node[3])
#define OBJDEF_PRAGMA(n) (n->node[4])
#define OBJDEF_NEEDOBJS(n) ((nodelist *)(n->node[5]))

/*--------------------------------------------------------------------------*/

/***
 ***  N_fundef :
 ***
 ***  sons:
 ***
 ***    node*           BODY     (O)   (N_block)
 ***    node*           ARGS     (O)   (N_arg)
 ***    node*           NEXT     (O)   (N_fundef)
 ***
 ***  permanent attributes:
 ***
 ***    char*           NAME
 ***    char*           MOD      (O)
 ***    char*           LINKMOD  (O)
 ***    types*          TYPES
 ***    statustype      STATUS
 ***    statustype      ATTRIB        (?? -> multithread  && multithread!! )
 ***                                  FLAGS IS CHANGED IN multithread!!!
 ***    bool            INLINE
 ***    int             FUNNO
 ***    node*           PRAGMA   (O)   (N_pragma)
 ***
 ***  temporary attributes:
 ***
 ***    node*           SIB      (O)   (N_sib)    (readsib !!)
 ***    node*           RETURN         (N_return) (typecheck -> compile !!)
 ***    nodelist*       NEEDOBJS (O)              (import -> )
 ***                                              ( -> analysis -> )
 ***                                              ( -> write-SIB -> )
 ***                                              ( -> obj-handling -> )
 ***                                              ( -> liftspmd !!)
 ***    node*           ICM            (N_icm)    (compile -> print )
 ***    int             VARNO                     (optimize -> )
 ***    long*           MASK[x]                   (optimize -> )
 ***    int             INLREC                    (inl !!)
 ***
 ***    DFMmask_base_t  DFM_BASE              (lac2fun/refcount -> spmd -> compile -> )
 ***
 ***
 ***    node*           FUNDEC_DEF  (N_fundef)    (checkdec -> writesib !!)
 ***
 ***  temporary attributes for ST_foldfun fundefs only:
 ***
 ***    ---
 ***
 ***  temporary attributes for ST_spmdfun fundefs only:
 ***
 ***    node*           LIFTEDFROM  (N_fundef)    (liftspmd -> compile -> )
 ***    node*           COMPANION (N_fundef)         (rfin and mtfin)
 ***                                      FLAG WILL BE CLEANED before mt-phases!!!
 ***/

/*
 *  STATUS: ST_regular      function defined in this module
 *          ST_objinitfun   generic function for object initialization
 *          ST_imported     imported function (maybe declaration only)
 *          ST_generic      class conversion function
 *          ST_foldfun      function used within fold-operation of with-loop
 *          ST_spmdfun      function containing lifted SPMD-region
 *          ST_loopfun      function represents a loop
 *          ST_repfun       function replicated for multithreaded execution
 *
 *  before multithreading:
 *  ATTRIB: ST_regular      dimension-dependent or non-array function
 *          ST_independent  dimension-independent array function
 *          ST_generic      generic function derived from dimension-
 *                          independent array function
 *  whlie/after multithreading:
 *  ATTRIB: ST_call_any       default_flag
 *                            (will be installed before using ATTRIB in mt-phases,
 *                             should not occur after mt-phases done)
 *          ST_call_st        function is CALL_ST
 *          ST_call_mt_master function is CALL_MT to be used by master
 *          ST_call_mt_worker function is CALL_MT to be used by workers
 *          ST_call_rep       function is CALL_REP
 *          ST_call_mtlift    function is a thread-function
 *
 *
 *  The FUNDEC_DEF slot is only used when a fundef node is used as a
 *  representation of a function declaration. It then points to the
 *  fundef node which contains the respective definition.
 *
 *  LINKMOD contains the name of the module which has to be linked with
 *  in order to make the code of this function available. If LINKMOD is
 *  NULL, then link with the module given by MOD.
 *
 *  If the fundef is a definition of a LAC-dummy-function, LAC_LET is a link to
 *  the (unambiguous!) let-node outside of the fundef-body containing a call of
 *  this function.
 */

extern node *MakeFundef (char *name, char *mod, types *types, node *args, node *body,
                         node *next);

#define FUNDEF_FUNNO(n) (n->counter)
#define FUNDEF_NAME(n) (n->info.types->id)
#define FUNDEF_MOD(n) (n->info.types->id_mod)
#define FUNDEF_LINKMOD(n) (n->info.types->id_cmod)
#define FUNDEF_STATUS(n) (n->info.types->status)
#define FUNDEF_ATTRIB(n) (n->info.types->attrib)
#define FUNDEF_TYPES(n) (n->info.types)
#define FUNDEF_BODY(n) (n->node[0])
#define FUNDEF_ARGS(n) (n->node[2])
#define FUNDEF_NEXT(n) (n->node[1])
#define FUNDEF_RETURN(n) (n->node[3])
#define FUNDEF_SIB(n) (n->node[3])
#define FUNDEF_ICM(n) (n->node[3])
#define FUNDEC_DEF(n) (n->node[3])
#define FUNDEF_NEEDOBJS(n) ((nodelist *)(n->node[4]))
#define FUNDEF_PRAGMA(n) (n->node[5])
#define FUNDEF_VARNO(n) (n->varno)
#define FUNDEF_MASK(n, x) (n->mask[x])
#define FUNDEF_INLINE(n) (n->flag)
#define FUNDEF_INLREC(n) (n->refcnt)
#define FUNDEF_DFM_BASE(n) (n->dfmask[0])
#define FUNDEF_IDENTIFIER(n) ((int)(n->dfmask[1]))
#define FUNDEF_MT2USE(n) (n->dfmask[2])
#define FUNDEF_MT2DEF(n) (n->dfmask[3])
#define FUNDEF_LIFTEDFROM(n) ((node *)(n->dfmask[4]))
#define FUNDEF_WORKER(n) ((node *)(n->dfmask[5]))
#define FUNDEF_COMPANION(n) ((node *)(n->dfmask[6]))

/*--------------------------------------------------------------------------*/

/***
 ***  N_arg :
 ***
 ***  sons:
 ***
 ***    node*       NEXT    (O)  (N_arg)
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    types*      TYPE
 ***    statustype  STATUS
 ***    statustype  ATTRIB
 ***
 ***  temporary attributes:
 ***
 ***    int         VARNO                        (optimize -> )
 ***    int         REFCNT                       (refcount -> compile -> )
 ***    int         NAIVE_REFCNT                 (refcount -> concurrent -> )
 ***    bool        PADDED                       (ap -> )
 ***    char*       TYPESTRING (O)               (precompile -> )
 ***    node*       OBJDEF     (O)  (N_objdef)   (obj-handling ->
 ***                                             ( -> precompile !!)
 ***    node*       ACTCHN     (O)  (N_vinfo)    (psi-optimize -> )
 ***    node*       COLCHN     (O)  (N_vinfo)    (psi-optimize -> )
 ***    node*       FUNDEF     (O)  (N_fundef)   (psi-optimize -> )
 ***/

/*
 *  STATUS: ST_regular     original argument
 *          ST_artificial  additional argument added by object-handler
 *
 *  ATTRIB: ST_regular     non-unique parameter
 *          ST_unique      unique parameter
 *          ST_reference   (unique) reference parameter
 *          ST_readonly_reference (unique) reference parameter which remains
 *                                         unmodified
 *
 *  TYPESTRING contains the argument's type as a string, used for renaming
 *             of functions.
 *
 * ATTENTION:
 *   N_vardec and Narg node have to have the same structure. See remark
 *   at N_id node.
 */

extern node *MakeArg (char *name, types *type, statustype status, statustype attrib,
                      node *next);

#define ARG_NAME(n) (n->info.types->id)
#define ARG_TYPE(n) (n->info.types)
#define ARG_STATUS(n) (n->info.types->status)
#define ARG_ATTRIB(n) (n->info.types->attrib)
#define ARG_VARNO(n) (n->varno)
#define ARG_REFCNT(n) (n->refcnt)
#define ARG_NAIVE_REFCNT(n) (n->int_data)
#define ARG_PADDED(n) ((bool)(n->flag))
#define ARG_NEXT(n) (n->node[0])
#define ARG_TYPESTRING(n) ((char *)(n->node[1]))
#define ARG_OBJDEF(n) (n->node[2])
#define ARG_ACTCHN(n) (n->node[3])
#define ARG_COLCHN(n) (n->node[4])
#define ARG_FUNDEF(n) (n->node[5])

/*--------------------------------------------------------------------------*/

/***
 ***  N_block :
 ***
 ***  sons:
 ***
 ***    node*      INSTR           (N_assign, N_empty)
 ***    node*      VARDEC     (O)  (N_vardec)
 ***
 ***  permanent attributes:
 ***
 ***    char*      CACHESIM   (O)
 ***
 ***  temporary attributes:
 ***
 ***    nodelist*  NEEDFUNS   (O)         (analysis -> )
 ***                                      ( -> analysis -> )
 ***                                      ( -> write-SIB -> DFR!! )
 ***    nodelist*  NEEDTYPES  (O)         (analysis -> )
 ***                                      ( -> write-SIB -> )
 ***    long*      MASK[x]                (optimize -> )
 ***    int        VARNO                  (optimize -> )
 ***
 ***    node*      SPMD_PROLOG_ICMS (O)   (N_fundef)  (compile !!)
 ***    node*      SPMD_SETUP_ARGS (O)    (N_fundef)  (compile !!)
 ***/

/*
 * In spmd-functions SPMD_PROLOG_ICMS points to an assign-chain that
 * contains the prolog memory management ICMs derived from the first
 * synchronisation block. These are required for the compilation of
 * the corresponding spmd-block.
 */

extern node *MakeBlock (node *instr, node *vardec);

#define BLOCK_VARNO(n) (n->varno)
#define BLOCK_MASK(n, x) (n->mask[x])
#define BLOCK_INSTR(n) (n->node[0])
#define BLOCK_VARDEC(n) (n->node[1])
#define BLOCK_NEEDFUNS(n) ((nodelist *)(n->node[2]))
#define BLOCK_NEEDTYPES(n) ((nodelist *)(n->node[3]))
#define BLOCK_SPMD_PROLOG_ICMS(n) (n->node[4])
#define BLOCK_SPMD_SETUP_ARGS(n) (n->node[5])
#define BLOCK_CACHESIM(n) (n->info.id)

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :
 ***
 ***  sons:
 ***
 ***    node*       NEXT     (O)  (N_vardec)
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME          (NAME is an element of ...
 ***    types*      TYPE           ... this struct)
 ***    statustype  STATUS        (element of TYPE, too)
 ***
 ***  temporary attributes:
 ***
 ***    node*       TYPEDEF  (O)  (N_typedef)  (typecheck -> fun_analysis -> )
 ***    node*       OBJDEF   (O)  (N_objdef)   (inlining ->
 ***                                           ( -> precompile !!)
 ***    node*       ACTCHN   (O)  (N_vinfo)    (psi-optimize -> )
 ***    node*       COLCHN   (O)  (N_vinfo)    (psi-optimize -> )
 ***    int         REFCNT                     (refcount -> compile -> )
 ***    int         NAIVE_REFCNT               (refcount -> concurrent -> )
 ***    int         VARNO                      (optimize -> )
 ***    statustype  ATTRIB                     (typecheck -> uniquecheck -> )
 ***    int         FLAG                       (ael  -> dcr2 !! )
 ***    bool        PADDED                     (ap -> )
 ***/

/*
 * STATUS : ST_regular    :  original vardec in source code
 *          ST_used       :  after typecheck detected necessity of vardec
 *          ST_artificial :  artificial vardec produced by function inlining
 *                           of a function which uses a global object.
 *                           Such vardecs are removed by the precompiler.
 *
 * ATTRIB : ST_regular :  normal variable
 *          ST_unique  :  unique variable
 *
 * TYPEDEF is a reference to the respective typedef node if the type of
 * the declared variable is user-defined.
 *
 * ATTENTION:
 *   N_vardec and Narg node have to have the same structure. See remark
 *   at N_id node.
 */

extern node *MakeVardec (char *name, types *type, node *next);

#define VARDEC_NAME(n) (n->info.types->id)
#define VARDEC_TYPE(n) (n->info.types)
#define VARDEC_STATUS(n) (n->info.types->status)
#define VARDEC_ATTRIB(n) (n->info.types->attrib)
#define VARDEC_VARNO(n) (n->varno)
#define VARDEC_REFCNT(n) (n->refcnt)
#define VARDEC_NAIVE_REFCNT(n) (n->int_data)
#define VARDEC_FLAG(n) (n->flag)
#define VARDEC_PADDED(n) ((bool)(n->flag))
#define VARDEC_NEXT(n) (n->node[0])
#define VARDEC_TYPEDEF(n) (n->node[1])
#define VARDEC_ACTCHN(n) (n->node[2])
#define VARDEC_COLCHN(n) (n->node[3])
#define VARDEC_OBJDEF(n) (n->node[4])

/*--------------------------------------------------------------------------*/

/***
 ***  N_assign :
 ***
 ***  sons:
 ***
 ***    node*  INSTR         ("N_instr")
 ***    node*  NEXT     (O)  (N_assign)
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK[x]                    (optimize -> )
 ***    int    STATUS                     (dcr1 -> dcr2 !!)
 ***    node*  CSE                        (CSE (GenerateMasks()) -> ??? )
 ***    node*  CF                         (CF !!)
 ***    void*  INDEX    (O)               (wli -> wlf ->)
 ***    int    LEVEL                      (wli !!)
 ***
 ***  remarks:
 ***   there is no easy way to remove the INDEX information after wlf (another
 ***   tree traversal would be necessary), so it stays afterwards.
 ***   Nevertheless only wlf will use it. The type of INDEX is index_info*,
 ***   defined in WithloopFolding.c (not in types.h).
 ***
 ***   CF is used to temporarily store an N_assign node behind another one.
 ***   This additional N_assign node will later be inserted before the original
 ***   one into the assignment chain.
 ***/

extern node *MakeAssign (node *instr, node *next);

#define ASSIGN_INSTR(n) (n->node[0])
#define ASSIGN_NEXT(n) (n->node[1])
#define ASSIGN_CSE(n) (n->node[2])
#define ASSIGN_CF(n) (n->node[3])
#define ASSIGN_MASK(n, x) (n->mask[x])
#define ASSIGN_STATUS(n) (n->flag)
#define ASSIGN_INDEX(n) (n->info2)
#define ASSIGN_LEVEL(n) (n->info.cint)

/*--------------------------------------------------------------------------*/

/***
 ***  N_let :                                   ( one of "N_instr" )
 ***
 ***  sons:
 ***
 ***    node*  EXPR      ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    ids*   IDS   (O)
 ***
 ***  temporary attributes:
 ***
 ***    DFMmask_t USEMASK                    (multithread ->)
 ***    DFMmask_t DEFMASK                    (multithread ->)
 ***/

extern node *MakeLet (node *expr, ids *ids);

#define LET_EXPR(n) (n->node[0])
#define LET_IDS(n) (n->info.ids)
#define LET_USEMASK(n) (n->dfmask[0])
#define LET_DEFMASK(n) (n->dfmask[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_cast :
 ***
 ***  sons:
 ***
 ***    node*   EXPR  ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    types*  TYPE
 ***/

extern node *MakeCast (node *expr, types *type);

#define CAST_EXPR(n) (n->node[0])
#define CAST_TYPE(n) (n->info.types)

/*--------------------------------------------------------------------------*/

/***
 ***  N_return :
 ***
 ***  sons:
 ***
 ***    node*     EXPRS      (N_exprs)  (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*     REFERENCE  (N_exprs)  (O)  (precompile -> compile !!)
 ***    DFMmask_t USEMASK                    (multithread ->)
 ***    DFMmask_t DEFMASK                    (multithread ->)
 ***/

/*
 *  REFERENCE: List of artificial return values which correspond to
 *             reference parameters.
 *
 *  ATTENTION: node[1] of N_return node already used by compile.c
 */

extern node *MakeReturn (node *exprs);

#define RETURN_EXPRS(n) (n->node[0])
#define RETURN_REFERENCE(n) (n->node[2])
#define RETURN_USEMASK(n) (n->dfmask[0])
#define RETURN_DEFMASK(n) (n->dfmask[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_cond :
 ***
 ***  sons:
 ***
 ***    node*     COND        ("N_expr")
 ***    node*     THEN        (N_block)
 ***    node*     ELSE        (N_block)
 ***
 ***  temporary attributes:
 ***
 ***    ids*      THENVARS               (refcount -> compile -> )
 ***    ids*      ELSEVARS               (refcount -> compile -> )
 ***    long*     MASK[x]                (optimize -> )
 ***    DFMmask_t IN_MASK                (lac2fun !!)
 ***    DFMmask_t OUT_MASK               (lac2fun !!)
 ***    DFMmask_t LOCAL_MASK             (lac2fun !!)
 ***/

/*
 * The temporary attributes THENVARS and ELSEVARS are chains of identifiers
 * along with refcounting information which are generated by refcount.c and
 * used by compile.c for generating refcounting instructions in the context
 * of a conditional.
 */

extern node *MakeCond (node *cond, node *thenpart, node *elsepart);

#define COND_COND(n) (n->node[0])
#define COND_THEN(n) (n->node[1])
#define COND_ELSE(n) (n->node[2])
#define COND_THENVARS(n) ((ids *)n->node[3])
#define COND_ELSEVARS(n) ((ids *)n->node[4])
#define COND_NAIVE_THENVARS(n) ((ids *)n->info2)
#define COND_NAIVE_ELSEVARS(n) ((ids *)n->node[5])
#define COND_MASK(n, x) (n->mask[x])
#define COND_IN_MASK(n) (n->dfmask[1])
#define COND_OUT_MASK(n) (n->dfmask[2])
#define COND_LOCAL_MASK(n) (n->dfmask[3])

/*--------------------------------------------------------------------------*/

/***
 ***  N_do :
 ***
 ***  sons:
 ***
 ***    node*     COND       ("N_expr")
 ***    node*     BODY       (N_block)
 ***
 ***  temporary attributes:
 ***
 ***    ids*      USEVARS                (refcount -> compile -> )
 ***    ids*      DEFVARS                (refcount -> compile -> )
 ***    long*     MASK[x]                (optimize -> )
 ***    DFMmask_t IN_MASK                (lac2fun !!)
 ***    DFMmask_t OUT_MASK               (lac2fun !!)
 ***    DFMmask_t LOCAL_MASK             (lac2fun !!)
 ***
 ***  attention:
 ***    - Don't mix up USEVARS and USEMASK resp DEFVARS and USEMASK!!!
 ***      Btw. the masks are defined in tree_compound.
 ***    - To access do-loops and while-loops with one macro use the
 ***      compound macros DO_OR_WHILE_xxx!!!
 ***/

/*
 * The temporary attributes USEVARS and DEFVARS are chains of identifiers
 * along with refcounting information which are generated by refcount.c and
 * used by compile.c for generating refcounting instructions in the context
 * of a do-loop.
 */

extern node *MakeDo (node *cond, node *body);

#define DO_COND(n) (n->node[0])
#define DO_BODY(n) (n->node[1])
#define DO_USEVARS(n) ((ids *)n->node[2])
#define DO_DEFVARS(n) ((ids *)n->node[3])
#define DO_NAIVE_USEVARS(n) ((ids *)n->node[4])
#define DO_NAIVE_DEFVARS(n) ((ids *)n->node[5])
#define DO_MASK(n, x) (n->mask[x])
#define DO_IN_MASK(n) (n->dfmask[1])
#define DO_OUT_MASK(n) (n->dfmask[2])
#define DO_LOCAL_MASK(n) (n->dfmask[3])

/*--------------------------------------------------------------------------*/

/***
 ***  N_while :
 ***
 ***  sons:
 ***
 ***    node*     COND       ("N_expr")
 ***    node*     BODY       (N_block)
 ***
 ***  temporary attributes:
 ***
 ***    ids*      USEVARS                (refcount -> compile -> )
 ***    ids*      DEFVARS                (refcount -> compile -> )
 ***    long*     MASK[x]                (optimize -> )
 ***    DFMmask_t IN_MASK                (lac2fun !!)
 ***    DFMmask_t OUT_MASK               (lac2fun !!)
 ***    DFMmask_t LOCAL_MASK             (lac2fun !!)
 ***
 ***  attention:
 ***    - Don't mix up USEVARS and USEMASK resp DEFVARS and USEMASK!!!
 ***      Btw. the masks are defined in tree_compound.
 ***    - To access do-loops and while-loops with one macro use the
 ***      compound macros DO_OR_WHILE_xxx!!!
 ***/

/*
 * The temporary attributes USEVARS and DEFVARS are chains of identifiers
 * along with refcounting information which are generated by refcount.c and
 * used by compile.c for generating refcounting instructions in the context
 * of a while-loop.
 */

extern node *MakeWhile (node *cond, node *body);

extern node *While2Do (node *while_node);

#define WHILE_COND(n) (n->node[0])
#define WHILE_BODY(n) (n->node[1])
#define WHILE_USEVARS(n) ((ids *)n->node[2])
#define WHILE_DEFVARS(n) ((ids *)n->node[3])
#define WHILE_NAIVE_USEVARS(n) ((ids *)n->node[4])
#define WHILE_NAIVE_DEFVARS(n) ((ids *)n->node[5])
#define WHILE_MASK(n, x) (n->mask[x])
#define WHILE_IN_MASK(n) (n->dfmask[1])
#define WHILE_OUT_MASK(n) (n->dfmask[2])
#define WHILE_LOCAL_MASK(n) (n->dfmask[3])

/*--------------------------------------------------------------------------*/

/***
 ***  N_annotate :                              ( one of "N_instr" )
 ***
 ***  permanent attributes:
 ***
 ***    int    TAG
 ***    int    FUNNUMBER
 ***    int    FUNAPNUMBER
 ***/

extern node *MakeAnnotate (int tag, int funnumber, int funapnumber);

#define CALL_FUN 0x0001
#define RETURN_FROM_FUN 0x0002
#define INL_FUN 0x0004
#define LIB_FUN 0x0008
#define OVRLD_FUN 0x0010

#define ANNOTATE_TAG(n) (n->flag)
#define ANNOTATE_FUNNUMBER(n) (n->counter)
#define ANNOTATE_FUNAPNUMBER(n) (n->varno)
#define ANNOTATE_FUN(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_ap :
 ***
 ***  sons:
 ***
 ***    node*  ARGS    (O)  (N_exprs)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    char*  MOD     (O)
 ***    int    ATFLAG  (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*  FUNDEF       (N_fundef)  (typecheck -> analysis -> )
 ***                                    ( -> obj-handling -> compile -> )
 ***/

extern node *MakeAp (char *name, char *mod, node *args);

#define AP_NAME(n) (n->info.fun_name.id)
#define AP_MOD(n) (n->info.fun_name.id_mod)
#define AP_ATFLAG(n) (n->counter)
#define AP_ARGS(n) (n->node[0])
#define AP_FUNDEF(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_exprs :
 ***
 ***  sons:
 ***
 ***    node*  EXPR       ("N_expr")
 ***    node*  NEXT  (O)  (N_exprs)
 ***/

extern node *MakeExprs (node *expr, node *next);

#define EXPRS_EXPR(n) (n->node[0])
#define EXPRS_NEXT(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_array :
 ***
 ***  sons:
 ***
 ***    node*      AELEMS   (N_exprs)
 ***
 ***  permanent attributes:
 ***
 ***    char*      STRING       (O)
 ***
 ***  temporary attributes:
 ***
 ***    types*     TYPE               (typecheck -> )
 ***
 ***    int        ISCONST      (O)   (flatten -> )
 ***    simpletype VECTYPE      (O)   (flatten -> )
 ***    int        VECLEN       (O)   (flatten -> )
 ***    void*      CONSTVEC     (O)   (flatten -> )
 ***/

/*
 * In the case of constant character arrays defined as strings, the
 * optional permanent attribute STRING holds the original definition.
 * This may be retrieved for C code generation.
 *
 * In the case of constant arrays, ISCONST is set to true, VECTYPE
 * indicates the element-type, VECLEN the number of elements, and
 * CONSTVEC holds a compact C-representation of the values.
 * In particular, constant arrays with VECTYPE T_int are of interest
 * for TSI (tile size inference).
 */

extern node *MakeArray (node *aelems);

#define ARRAY_TYPE(n) (n->info.types)
#define ARRAY_AELEMS(n) (n->node[0])
#define ARRAY_STRING(n) ((char *)(n->node[1]))

#define ARRAY_ISCONST(n) (n->refcnt)
#define ARRAY_VECTYPE(n) ((simpletype) (n->varno))
#define ARRAY_VECLEN(n) (n->counter)
#define ARRAY_CONSTVEC(n) (n->info2)

/*--------------------------------------------------------------------------*/

/***
 ***  N_vinfo :
 ***
 ***  sons:
 ***
 ***    node*    NEXT     (O)  (N_vinfo)
 ***
 ***  permanent attributes:
 ***
 ***    useflag  FLAG
 ***    types*   TYPE     (O)
 ***    node*    DOLLAR   (O)  (N_vinfo)
 ***    node*    VARDEC   (O)  (N_vardec)
 ***/

extern node *MakeVinfo (useflag flag, types *type, node *next, node *dollar);

#define VINFO_FLAG(n) (n->info.use)
#define VINFO_TYPE(n) ((types *)n->node[1])
#define VINFO_NEXT(n) (n->node[0])
#define VINFO_DOLLAR(n) (n->node[2])
#define VINFO_VARDEC(n) (n->node[3])

/*--------------------------------------------------------------------------*/

/***
 ***  N_id :
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       MOD     (O)
 ***    statustype  ATTRIB
 ***    statustype  STATUS
 ***    ids*        IDS
 ***
 ***  temporary attributes:
 ***
 ***    node*       VARDEC    (N_vardec/N_arg)  (typecheck -> )
 ***    node*       OBJDEF    (N_objdef)        (typecheck -> )
 ***                                            ( -> analysis -> )
 ***    int         REFCNT                      (refcount -> compile -> )
 ***    int         NAIVE_REFCNT                (refcount -> concurrent -> )
 ***    int         MAKEUNIQUE                  (precompile -> compile -> )
 ***    node*       DEF                         (Unroll !, Unswitch !)
 ***    node*       WL          (O)             (wli -> wlf !!)
 ***
 ***    void*       CONSTVEC    (O)             (flatten -> )
 ***    int         VECLEN      (O)             (flatten -> )
 ***    simpletype  VECTYPE     (O)             (flatten -> )
 ***    int         ISCONST     (O)             (flatten -> )
 ***
 ***  remark:
 ***    ID_WL is only used in wli, wlf. But every call of DupTree() initializes
 ***    the copy's WL_ID with a pointer to it's original N_id node. The function
 ***    SearchWL() can define ID_WL in another way (pointer to N_assign node
 ***    of WL which is referenced by this Id).
 ***
 ***    Unroll uses ->flag without a macro :(
 ***    Even worse: Unroll uses ->flag of *every* LET_EXPR node :(((
 ***
 ***  remark:
 ***    ID_VARDEC points to an N_vardec or an N_arg node. This is not
 ***    distinguished in many places of the code. So for example
 ***    VARDEC_NAME and ARG_NAME should both be substitutions for
 ***    node->info.types->id
 ***
 ***  remark:
 ***    ISCONST, VECTYPE, VECLEN, and CONSTVEC are used for propagation of constant
 ***    integer arrays.
 ***    Usually, there is no constant propagation for arrays since this
 ***    normally slows down the code due to memory allocation/de-allocation
 ***    costs. However for some other optimizations, namely tile size inference,
 ***    a constant value is an advantage.
 ***/

/*
 *  STATUS:  ST_regular     original argument
 *                          in a function application or return-statement
 *           ST_artificial  additional argument added by object-handler
 *                          in a function application or return-statement
 *
 *  ATTRIB:  ST_regular     ordinary argument
 *                          in a function application or return-statement
 *           ST_global      global object
 *           ST_readonly_reference/
 *           ST_reference   argument in a function application which
 *                          is passed as a reference parameter or
 *                          additional argument in a return-statement
 *                          which belongs to a reference parameter
 *
 *  MAKEUNIQUE is a flag which is set in those N_id nodes which were
 *  arguments to class conversion function.
 */

extern node *MakeId (char *name, char *mod, statustype status);

extern node *MakeId1 (char *str);
extern node *MakeId2 (ids *ids_node);
extern node *MakeId3 (ids *ids_node);

#define ID_IDS(n) (n->info.ids)
#define ID_NAME(n) (n->info.ids->id)
#define ID_DEF(n) (n->info.ids->def)
#define ID_VARDEC(n) (n->info.ids->node)
#define ID_OBJDEF(n) (n->info.ids->node)
#define ID_MOD(n) (n->info.ids->mod)
#define ID_ATTRIB(n) (n->info.ids->attrib)
#define ID_STATUS(n) (n->info.ids->status)
#define ID_REFCNT(n) (n->refcnt)
#define ID_NAIVE_REFCNT(n) (n->info.ids->naive_refcnt)
#define ID_MAKEUNIQUE(n) (n->flag)
#define ID_WL(n) (n->node[0])

#define ID_VECLEN(n) (n->counter)
#define ID_VECTYPE(n) ((simpletype) (n->int_data))
#define ID_CONSTVEC(n) (n->info2)
#define ID_ISCONST(n) (n->varno)

/*--------------------------------------------------------------------------*/

/***
 ***  N_num :
 ***
 ***  permanent attributes:
 ***
 ***    int  VAL
 ***/

extern node *MakeNum (int val);

#define NUM_VAL(n) (n->info.cint)

/*--------------------------------------------------------------------------*/

/***
 ***  N_char :
 ***
 ***  permanent attributes:
 ***
 ***    char  VAL
 ***/

extern node *MakeChar (char val);

#define CHAR_VAL(n) (n->info.cchar)

/*--------------------------------------------------------------------------*/

/***
 ***  N_float :
 ***
 ***  permanent attributes:
 ***
 ***    float  VAL
 ***/

extern node *MakeFloat (float val);

#define FLOAT_VAL(n) (n->info.cfloat)

/*--------------------------------------------------------------------------*/

/***
 ***  N_double :
 ***
 ***  permanent attributes:
 ***
 ***    double  VAL
 ***/

extern node *MakeDouble (double val);

#define DOUBLE_VAL(n) (n->info.cdbl)

/*--------------------------------------------------------------------------*/

/***
 ***  N_bool :
 ***
 ***  permanent attributes:
 ***
 ***    int  VAL
 ***/

extern node *MakeBool (int val);

#define BOOL_VAL(n) (n->info.cint)

/*--------------------------------------------------------------------------*/

/***
 ***  N_str :
 ***
 ***  permanent attributes:
 ***
 ***    char*  STRING
 ***/

extern node *MakeStr (char *str);

#define STR_STRING(n) (n->info.id)

/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :
 ***
 ***  sons:
 ***
 ***     node*  ARGS   (N_exprs)
 ***
 ***  permanent attributes:
 ***
 ***     prf    PRF
 ***/

extern node *MakePrf (prf prf, node *args);

#define PRF_PRF(n) (n->info.prf)
#define PRF_ARGS(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_empty :
 ***/

extern node *MakeEmpty ();

/*--------------------------------------------------------------------------*/

/***
 ***  N_post :
 ***
 ***  permanent attributes:
 ***
 ***    node*  INCDEC
 ***    char*  ID
 ***
 ***  temporary attributes:
 ***
 ***    node*  DECL    (N_vardec)  (typecheck -> )
 ***    int    REFCNT              (refcount  -> )
 ***    int    NAIVE_REFCNT        (refcount  -> concurrent -> )
 ***/

extern node *MakePost (int incdec, char *id);

#define POST_INCDEC(n) ((n->node[0]->nodetype == N_dec) ? 0 : 1)

#define POST_ID(n) (n->info.id)
#define POST_DECL(n) (n->node[1])
#define POST_REFCNT(n) (n->info.ids->refcnt)
#define POST_NAIVE_REFCNT(n) (n->info.ids->refcnt)

/*
 * Attention : The way incrementations and decrementation are represented
 * is not changed up to now. The macro POST_INCDEC must not be used on the
 * left side of the assignment operator. In comments the new representation
 * is shown. The MakePost function is intended to support both ways.
 */

/*--------------------------------------------------------------------------*/

/***
 ***  N_ok :
 ***
 ***  dummynode, last declared in node_info.mac
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_inc :
 ***  N_dec :
 ***
 ***  no description yet
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_stop :
 ***
 ***  no description yet
 ***  barely used in typecheck.c
 ***/
/*--------------------------------------------------------------------------*/

/***
 ***  N_pre :
 ***
 ***  permanent attributes:
 ***
 ***    node*  INCDEC
 ***    char*  ID
 ***
 ***  temporary attributes:
 ***
 ***    node*  DECL    (N_vardec)  (typecheck -> )
 ***    int    REFCNT              (refcount  -> )
 ***    int    NAIVE_REFCNT        (refcount  -> concurrent -> )
 ***/

extern node *MakePre (nodetype incdec, char *id);

#define PRE_INCDEC(n) ((n->node[0]->nodetype == N_dec) ? 0 : 1)
#define PRE_ID(n) (n->info.id)
#define PRE_REFCNT(n) (n->info.ids->refcnt)
#define PRE_NAIVE_REFCNT(n) (n->info.ids->naive_refcnt)
#define PRE_DECL(n) (n->node[1])

/*
 * Attention : The way incrementations and decrementation are represented
 * is not changed up to now. The macro POST_INCDEC must not be used on the
 * left side of the assignment operator. In comments the new representation
 * is shown. The MakePost function is intended to support both ways.
 */

/*--------------------------------------------------------------------------*/

/***
 ***  N_icm :
 ***
 ***  sons:
 ***
 ***    node*  ARGS  (O)  (N_exprs)
 ***    node*  NEXT  (O)  (N_icm)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    int    INDENT_BEFORE
 ***    int    INDENT_AFTER
 ***    bool   END_OF_STATEMENT (if true, adds a semicolon after ICM )
 ***
 *** remarks:
 ***    NEXT at least (!) is used for the compilation of N_typedef's
 ***    whenever the defining type is an array type!!
 ***
 ***    INDENT is used for indenting ICMs in output. This value is set
 ***    by 'MakeIcm' and used by 'PrintIcm'.
 ***
 ***/

extern node *MakeIcm (char *name, node *args, node *next);

#define ICM_NAME(n) (n->info.fun_name.id)
#define ICM_INDENT_BEFORE(n) (n->flag)
#define ICM_INDENT_AFTER(n) (n->int_data)
#define ICM_ARGS(n) (n->node[0])
#define ICM_NEXT(n) (n->node[1])
#define ICM_END_OF_STATEMENT(n) ((bool)(n->counter))

/*--------------------------------------------------------------------------*/

/***
 ***  N_pragma :
 ***
 ***  permanent attributes:
 ***
 ***    char*  LINKNAME         (O)
 ***    int[]  LINKSIGN         (O)
 ***    int[]  REFCOUNTING      (O)
 ***    char*  INITFUN          (O)
 ***
 ***    node*  WLCOMP_APS       (0)      (N_exprs)
 ***
 ***  temporary attributes:
 ***
 ***    int[]  READONLY         (O)   (import -> readsib !!)
 ***    ids*   EFFECT           (O)   (import -> readsib !!)
 ***    ids*   TOUCH            (O)   (import -> readsib !!)
 ***    char*  COPYFUN          (O)   (import -> readsib !!)
 ***    char*  FREEFUN          (O)   (import -> readsib !!)
 ***    ids*   NEEDTYPES        (O)   (import -> readsib !!)
 ***    node*  NEEDFUNS         (O)   (import -> readsib !!)
 ***    char*  LINKMOD          (O)   (import -> readsib !!)
 ***    int    NUMPARAMS        (O)   (import -> readsib !!)
 ***
 ***    nums*  LINKSIGNNUMS     (O)   (scanparse -> import !!)
 ***    nums*  REFCOUNTINGNUMS  (O)   (scanparse -> import !!)
 ***    nums*  READONLYNUMS     (O)   (scanparse -> import !!)
 ***/

/*
 *  Not all pragmas may occur at the same time:
 *  A typedef pragma may contain COPYFUN and FREEFUN.
 *  A objdef pragma may contain LINKNAME only.
 *  And a fundef pragma may contain all pragmas except COPYFUN and FREEFUN,
 *  but LINKMOD, TYPES and FUNS are only for internal use in SIBS.
 *  A wlcomp pragma contains WLCOMP only.
 *
 *  NUMPARAMS is not a pragma but gives the number of parameters of the
 *  function (return values + arguments). This is the size of the arrays
 *  which store the LINKSIGN, REFCOUNTING, and READONLY pragmas.
 *
 *  The temporary attributes serve only for parsing the respective
 *  pragmas. Immediately after parsing, the pragmas are checked and
 *  converted into the array representation. So, all usage of these
 *  pragmas must rely on the respective permanent attributes.
 *
 *  Although certain attributes are known to occur only in particular
 *  situations, these should NOT be mapped on the same location within
 *  the "real" data structure because the attributes are used to determine
 *  the kind of pragma, e.g. for printing.
 */

extern node *MakePragma ();

#define PRAGMA_LINKNAME(n) (n->info.id)
#define PRAGMA_NUMPARAMS(n) (n->flag)
#define PRAGMA_INITFUN(n) ((char *)(n->node[3]))
#define PRAGMA_LINKMOD(n) ((char *)(n->node[2]))
#define PRAGMA_NEEDTYPES(n) ((ids *)(n->node[1]))
#define PRAGMA_NEEDFUNS(n) (n->node[0])
#define PRAGMA_WLCOMP_APS(n) (n->node[4])

#define PRAGMA_LINKSIGN(n) ((int *)(n->mask[0]))
#define PRAGMA_LINKSIGNNUMS(n) ((nums *)(n->mask[0]))
#define PRAGMA_REFCOUNTING(n) ((int *)(n->mask[1]))
#define PRAGMA_REFCOUNTINGNUMS(n) ((nums *)(n->mask[1]))
#define PRAGMA_READONLY(n) ((int *)(n->mask[2]))
#define PRAGMA_READONLYNUMS(n) ((nums *)(n->mask[2]))
#define PRAGMA_EFFECT(n) ((ids *)(n->mask[3]))
#define PRAGMA_TOUCH(n) ((ids *)(n->mask[4]))
#define PRAGMA_COPYFUN(n) ((char *)(n->mask[5]))
#define PRAGMA_FREEFUN(n) ((char *)(n->mask[6]))

/*--------------------------------------------------------------------------*/

/***
 ***  N_info :
 ***
 ***  The N_info node is used to store additional compile time information
 ***  outside the syntax tree. So, its concrete look depends on the
 ***  specific task.
 ***
 ***  when used in flatten.c:
 ***
 ***    contextflag CONTEXT       (O)
 ***    node *      LASTASSIGN    (O)  (N_assign)
 ***    node *      LASTWLBLOCK   (O)  (N_block)
 ***    node *      FINALASSIGN   (O)  (N_assign)
 ***    void *      CONSTVEC      (O)  (compact c array)
 ***    int         VECLEN        (O)
 ***    int         VECTYPE       (O)
 ***    int         ISCONST       (O)
 ***
 ***  when used in readsib.c :
 ***
 ***    node *     FOLDFUNS       (O)  (N_fundef)
 ***    node *     MODUL          (O)  (N_modul)
 ***
 ***  when used in typecheck.c :
 ***
 ***    int        STATUS        (O)
 ***    node *     VARDEC        (O)  (N_vardec)
 ***    node *     FUNDEF        (O)  (N_fundef)
 ***    node *     NEXTASSIGN    (O)  (N_assign)
 ***    node *     LASSIGN       (O)  (N_assign)
 ***    ids*       LHSVARS       (O)
 ***
 ***  when used in writesib.c :
 ***
 ***    nodelist*  EXPORTTYPES   (O)
 ***    nodelist*  EXPORTOBJS    (O)
 ***    nodelist*  EXPORTFUNS    (O)
 ***
 ***  when used in precompile.c :
 ***
 ***    char*      NAME          (0)
 ***    node*      FUNDEFS       (0) (N_fundef)
 ***    node*      OBJINITFUNDEF (O) (N_fundef)
 ***
 ***  when used in compile.c :
 ***
 ***    ids*       LASTIDS       (O)
 ***    node*      LASTLET       (O)  (N_let)
 ***    node*      LASTASSIGN    (O)  (N_assign)
 ***    node*      VARDECS       (O)  (N_vardec)
 ***    node*      WITHBEGIN     (O)  (N_icm)
 ***    node*      MODUL         (O)  (N_modul)
 ***
 ***    node*      FIRSTASSIGN   (O)  (N_assign)
 ***    node*      FUNDEF        (O)  (N_fundef)
 ***    int        CNTPARAM
 ***    node**     ICMTAB        (O)
 ***    types**    TYPETAB       (O)
 ***
 ***  when used in optimize.c :
 ***
 ***    long*      MASK[x]
 ***
 ***  when used during withloop folding:
 ***    node*      NEXT               (N_info)
 ***    node*      SUBST              (N_Ncode)
 ***    node*      WL                 (N_Nwith)
 ***    node*      NEW_ID             (N_id)
 ***    node*      ASSIGN             (N_assign)
 ***    node*      FUNDEF             (N_fundef)
 ***    int        FLAG               (0/1)
 ***    node*      ID                 (N_id)
 ***    node*      NCA                (N_assign) (new code assignments)
 ***    node*      LET                (N_let)
 ***    node*      REPLACE            (N_id, N_array or N_num/float...)
 ***
 ***  when used in ConstantFolding.c :
 ***    node*      ASSIGN             (N_assign)
 ***    types      TYPE               (no son)
 ***    int        VARNO
 ***
 ***  when used in DeadCodeRemoval.c:
 ***    nodetype   TRAVTYPE
 ***    int        VARNO
 ***    int        NEWACT
 ***    long*      ACT
 ***
 ***  when used in index.c:
 ***    node *     INFO_IVE_FUNDEF           (N_fundef)
 ***    node *     INFO_IVE_VARDECS          (N_vardec)
 ***    ive_mode   INFO_IVE_MODE
 ***    node *     INFO_IVE_CURRENTASSIGN    (N_assign)
 ***    node *     INFO_IVE_TRANSFORM_VINFO  (N_vinfo)
 ***    int        INFO_IVE_NON_SCAL_LEN
 ***
 ***  old mt!!!
 ***  when used in managing spmd- and sync blocks in concurrent :
 ***    (oa) concurrent.[ch]
 ***    (ob) spmd_init.[ch]
 ***    (oc) spmd_opt.[ch]
 ***    (od) spmd_lift.[ch]
 ***    (oe) sync_init.[ch]
 ***    (of) sync_opt.[ch]
 ***    (og) spmd_cons.[ch]
 ***    (oh) spmd_trav.[ch]
 ***
 ***  in all:
 ***    node*      INFO_CONC_FUNDEF   (N_fundef)
 ***
 ***  in (oa), (ob):
 ***    boolean    INFO_SPMDI_LASTSPMD(n)
 ***    boolean    INFO_SPMDI_NEXTSPMD(n)
 ***    nodetype   INFO_SPMDI_CONTEXT(n)
 ***    boolean    INFO_SPMDI_EXPANDCONTEXT(n)
 ***    boolean    INFO_SPMDI_EXPANDSTEP(n)
 ***
 ***  in (oa), (oc):
 ***    node*      INFO_SPMDO_THISASSIGN(n)          (N_assign)
 ***    node*      INFO_SPMDO_NEXTASSIGN(n)          (N_assign)
 ***
 ***  in (oa), (od):
 ***    boolean    INFO_SPMDL_MT(n)
 ***
 ***  in (oa), (oe):
 ***    boolean    INFO_SYNCI_FIRST(n)
 ***    boolean    INFO_SYNCI_LAST(n)
 ***
 ***  in (oa), (of):
 ***    node*      INFO_SYNCO_THISASSIGN(n)          (N_assign)
 ***    node*      INFO_SYNCO_NEXTASSIGN(n)          (N_assign)
 ***
 ***  in (oa), (og):
 ***    node*      INFO_SPMDC_FIRSTSYNC(n)           (N_sync)
 ***
 ***  in (oa), (oh) in different traversals:
 ***    DFM_mask_t INFO_SPMDRM_RESULT(n)
 ***    DFM_mask_t INFO_SPMDRM_CHECK(n)
 ***    DFM_mask_t INFO_SPMDRO_CHECK(n)
 ***    int*       INFO_SPMDRO_COUNTERS(n)
 ***    boolean    INFO_SPMDLC_APPLICATION(n)
 ***    boolean    INFO_SPMDDN_NESTED(n)
 ***    DFM_mask_t INFO_SPMDPM_IN(n)
 ***    DFM_mask_t INFO_SPMDPM_INOUT(n)
 ***    DFM_mask_t INFO_SPMDPM_OUT(n)
 ***    DFM_mask_t INFO_SPMDPM_LOCAL(n)
 ***    DFM_mask_t INFO_SPMDPM_SHARED(n)
 ***
 ***    node*      ACTUAL_FUNDEF
 ***    DFMmask_t  INFO_SPMDT_RESULT
 ***    DFMmask_t  INFO_SPMDT_CHECK
 ***    int*       INFO_SPMDT_COUTERS
 ***
 ***    node*      INFO_SYNCO_NEXTASSIGN
 ***    node*      INFO_SYNCO_THISASSIGN
 ***
 ***    node*      INFO_SPMDC_FIRSTSYNC
 ***
 ***  new mt!!!
 ***  when used in multithread ...
 ***    (na) multithread.[ch]
 ***    (nb) schedule_init.[ch]
 ***    (nc) repfuns_init.[ch]
 ***    (nd) blocks_init.[ch]
 ***    (ne) blocks_propagate.[ch]
 ***    (nf) blocks_expand.[ch]
 ***    (ng) mtfuns_init.[ch]
 ***    (nh) blocks_cons.[ch]
 ***    (ni) dataflow_analysis.[ch]
 ***    (nj) barriers_init.[ch]
 ***    (nk) blocks_lift.[ch]
 ***    (nl) adjust_calcc.[ch]
 ***
 ***  in all:
 ***    node*      INFO_MUTH_FUNDEF   (N_fundef)
 ***    int(bool)  INFO_MUTH_ALLOW_OOOC
 ***    int(bool)  INFO_MUTH_TOPDOWN
 ***    funptr     INFO_MUTH_DRIVER
 ***    ignorefun  INFO_MUTH_IGNORE
 ***
 ***  in (na), (nb):
 ***    node*      INFO_SCHIN_SCHEDULING
 ***    int        INFO_SCHIN_INNERWLS
 ***    int        INFO_SCHIN_ALLOWED
 ***
 ***  in (na), (nc):
 ***    int(bool)  INFO_RFIN_WITHINWITH
 ***
 ***  in (na), (nd):
 ***    [nothing]
 ***
 ***  in (na), (ne):
 ***    [nothing]
 ***
 ***  in (na), (nf):
 ***    int(bool)  INFO_BLKEX_BLOCKABOVE
 ***
 ***  in (na), (ng):
 ***    statustype INFO_MTFIN_CURRENTATTRIB(n)
 ***
 ***  in (na), (nh):
 ***    statustype INFO_BLKCO_CURRENTATTRIB(n)
 ***    node*      INFO_BLKCO_THISASSIGN(n)     (N_assign)
 ***
 ***  in (na), (ni):
 ***    ####
 ***
 ***  in (na), (nj):
 ***    int(bool)  INFO_BARIN_WITHINMT
 ***
 ***  in (na), (nk):
 ***    ####
 ***
 ***  in (na), (nl):
 ***    statustype INFO_ADJCA_ATTRIB
 ***
 ***
 ***  when used in tile_size_inference.c :
 ***
 ***    access_t*  ACCESS
 ***    node*      INDEXVAR           (N_vardec/N_arg)
 ***    feature_t  FEATURE
 ***    WithOpType WOTYPE
 ***    ids*       LASTLETIDS
 ***    int        BELOWAP
 ***    access_t*  TMPACCESS
 ***
 ***  when used in spmd_opt.c :
 ***
 ***    node*      LASTASSIGN
 ***    node*      THISASSIGN
 ***    node*      NEXTASSIGN
 ***
 ***  when used in print.c :
 ***
 ***    node*      FUNDEF             (N_fundef)
 ***    node*      INT_SYN
 ***    node*      WITH_RET
 ***    node*      NWITH2
 ***    node*      ACCESS
 ***    int        PRAGMA_WLCOMP
 ***    int        SIB
 ***    int        OMIT_FORMAL_PARAMS
 ***    int        VARNO
 ***    int        PROTOTYPE
 ***    int        SEPARATE
 ***
 ***
 ***  when used in refcount.c
 ***
 ***    node*      PRF
 ***    node*      WITH
 ***    int*       RCDUMP             (only while traversing a group of N_code's)
 ***    int*       NAIVE_RCDUMP       (only while traversing a group of N_code's)
 ***    int        VARNO
 ***
 ***
 ***  when used in print_interface.c
 ***    int        PIH_FLAG     (switch between comment and prototype)
 ***    int        PIH_COMMA    (flag, comma neede between outputs)
 ***    int        PIH_COUNTER  (arg or type position)
 ***    int        PIW_FLAG     (switch between different formats)
 ***    int        PIW_COMMA
 ***    int        PIW_COUNTER
 ***
 ***  when used in map_wrapper.c
 ***    node*      MODUL             (access to module node)
 ***    node*      FUNDEF            (fundef parameter)
 ***    int        FLAG
 ***    int        CNT_STANDARD      (counter for standard args)
 ***
 ***  when used in pad_transform.c
 ***
 ***    bool       EXPRESSION_PADDED
 ***
 ***  when used in import_specializations.c
 ***    node*      SPECS             (chain of specialized fundefs)
 ***    node*      FUNDEF            (actual working fundef)
 ***
 ***
 ***
 ***
 ***
 *** remarks:
 ***    N_info is used in many other phases without access macros :((
 ***/

/*
 *  When used in writesib.c, the N_info node collects lists of nodes which
 *  have to be printed to the SIB.
 */

/*
 * srs: the number of sons being traversed is set to MAX_SONS, so
 *  don't use (temporary) attributes in the node-slots.
 */

extern node *MakeInfo ();

/* DupTree
 *
 * ATTENTION: Usage of DUP and INL macros on arg_info are mixed. Be careful
 *            to avoid overlapping addresses.
 *
 *            INFO_INL_TYPES is used when DupTree is called from Inlining,
 *            so do not override that one here. (jhs)
 *
 * INFO_DUP_DFMBASE carries the information of a new DFMbase for DFMmasks to be
 *                  copied, when the body of a function is duplicated (jhs)
 */
#define INFO_DUP_CONT(n) (n->node[1])
/*      INFO_INL_TYPES(n)                      (n->node[2])   See comment!!! */
#define INFO_DUP_FUNDEF(n) (n->node[3])
#define INFO_DUP_TYPE(n) (n->flag)
#define INFO_DUP_ALL(n) (n->int_data)
#define INFO_DUP_LUT(n) ((LUT_t) (n->dfmask[6]))

/* flatten */
#define INFO_FLTN_CONTEXT(n) (n->flag)
#define INFO_FLTN_LASTASSIGN(n) (n->node[0])
#define INFO_FLTN_LASTWLBLOCK(n) (n->node[1])
#define INFO_FLTN_FINALASSIGN(n) (n->node[2])

#define INFO_FLTN_CONSTVEC(n) (n->info2)
#define INFO_FLTN_VECLEN(n) (n->counter)
#define INFO_FLTN_VECTYPE(n) ((simpletype)n->varno)
#define INFO_FLTN_ISCONST(n) (n->refcnt)

/* readsib */
#define INFO_RSIB_FOLDFUNS(n) (n->node[0])
#define INFO_RSIB_MODUL(n) (n->node[1])

/* typecheck */
/* here, some ugly things happen 8-((
 * An N_ok-node is used as info-carrier!!!
 * It carries several values needed for typechecking functions
 * and is stored under info_node->node[0].
 * Since I (sbs) consider this a dirty implementational hack,
 * I define the virtual syntax tree as if the info would be
 * available directly in the N_info-node.
 */
#define INFO_TC_STATUS(n) (n->node[0]->info.cint)
#define INFO_TC_VARDEC(n) (n->node[0]->node[0])
#define INFO_TC_FUNDEF(n) (n->node[0]->node[1])
#define INFO_TC_NEXTASSIGN(n) (n->node[1])
/* WARN: node[2] already used */
#define INFO_TC_LASSIGN(n) (n->node[3])
#define INFO_TC_CURRENTASSIGN(n) (n->node[4])
#define INFO_TC_LHSVARS(n) (n->info.ids)

/* writesib */
#define INFO_WSIB_EXPORTTYPES(n) ((nodelist *)(n->node[0]))
#define INFO_WSIB_EXPORTOBJS(n) ((nodelist *)(n->node[1]))
#define INFO_WSIB_EXPORTFUNS(n) ((nodelist *)(n->node[2]))
/* see also print access macros used in this phase ! */

/* refcount */
#define INFO_RC_PRF(n) (n->node[0])
#define INFO_RC_WITH(n) (n->node[1])
#define INFO_RC_RCDUMP(n) ((int *)(n->node[2]))
#define INFO_RC_NAIVE_RCDUMP(n) ((int *)(n->node[3]))
#define INFO_RC_VARNO(n) (n->varno)
#define INFO_RC_ONLYNAIVE(n) (n->flag)

/* wltransform */
#define INFO_WL_SHPSEG(n) ((shpseg *)(n->node[0]))

/* concurrent */
#define INFO_CONC_FUNDEF(n) (n->node[0])

/* concurrent - spmdinit */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SPMDI_LASTSPMD(n) (n->flag)
#define INFO_SPMDI_NEXTSPMD(n) (n->counter)
#define INFO_SPMDI_CONTEXT(n) (n->int_data)
#define INFO_SPMDI_EXPANDCONTEXT(n) (n->varno)
#define INFO_SPMDI_EXPANDSTEP(n) (n->refcnt)

/* concurrent-spmdopt */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SPMDO_THISASSIGN(n) (n->node[1])
#define INFO_SPMDO_NEXTASSIGN(n) (n->node[2])

/* concurrent-spmdlift */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SPMDL_MT(n) (n->counter)

/* concurrent-syncinit */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SYNCI_FIRST(n) (n->flag)
#define INFO_SYNCI_LAST(n) (n->int_data)

/* concurrent-syncopt */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SYNCO_THISASSIGN(n) (n->node[1])
#define INFO_SYNCO_NEXTASSIGN(n) (n->node[2])

/* concurrent-spmdcons */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SPMDC_FIRSTSYNC(n) (n->node[1])

/* concurrent-spmdtrav-reducemasks */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SPMDRM_RESULT(n) (n->dfmask[0])
#define INFO_SPMDRM_CHECK(n) (n->dfmask[1])

/* concurrent-spmdtrav-reduceoccurences */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SPMDCO_RESULT(n) ((int *)(n->node[1]))
#define INFO_SPMDCO_WHICH(n) (n->dfmask[1])

/* concurrent-spmdtrav-reduceoccurences */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SPMDRO_CHECK(n) (n->dfmask[1])
#define INFO_SPMDRO_COUNTERS(n) ((int *)(n->node[1]))

/* concurrent-spmdtrav-lc */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SPMDLC_APPLICATION(n) (n->int_data)

/* concurrent-spmdtrav-deletenested */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SPMDDN_NESTED(n) (n->int_data)

/* concurrent-spmdtrav-producemasks */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here!!! */
#define INFO_SPMDPM_IN(n) ((DFMmask_t) (n->dfmask[0]))
#define INFO_SPMDPM_INOUT(n) ((DFMmask_t) (n->dfmask[1]))
#define INFO_SPMDPM_OUT(n) ((DFMmask_t) (n->dfmask[2]))
#define INFO_SPMDPM_LOCAL(n) ((DFMmask_t) (n->dfmask[3]))
#define INFO_SPMDPM_SHARED(n) ((DFMmask_t) (n->dfmask[4]))

/* multithread - all mini-phases */
/* DO NOT OVERRIDE ANY INFO_YYYY_xxx HERE, were YYYY is any other miniphase!!! */
#define INFO_MUTH_FUNDEF(n) (n->node[0])
#define INFO_MUTH_ALLOW_OOOC(n) (n->counter)
#define INFO_MUTH_TOPDOWN(n) (n->refcnt)
#define INFO_MUTH_DRIVER(n) ((funptr) (n->node[1]))
#define INFO_MUTH_IGNORE(n) ((ignorefun) (n->node[2]))

/* multithread - schedule_init */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE!!! */
#define INFO_SCHIN_SCHEDULING(n) (n->node[3])
#define INFO_SCHIN_INNERWLS(n) (n->int_data)
#define INFO_SCHIN_ALLOWED(n) (n->flag)

/* multithread - repfuns_init */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE!!! */
#define INFO_RFIN_WITHINWITH(n) (n->int_data)

/* multithread - blocks_expand */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE!!! */
#define INFO_BLKEX_BLOCKABOVE(n) (n->int_data)

/* multithread - blocks_expand */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE!!! */
#define INFO_MTFIN_CURRENTATTRIB(n) ((statustype) (n->int_data))

/* multithread - blocks_expand */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE!!! */
#define INFO_BLKCO_CURRENTATTRIB(n) ((statustype) (n->int_data))
#define INFO_BLKCO_THISASSIGN(n) (n->node[3])

/* multithread - blocks_expand */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE!!! */
#define INFO_DFA_HEADING(n) (n->int_data)
#define INFO_DFA_USEMASK(n) (n->dfmask[0])
#define INFO_DFA_DEFMASK(n) (n->dfmask[1])
#define INFO_DFA_NEEDCHAIN(n) (n->dfmask[2])
#define INFO_DFA_NEEDBLOCK(n) (n->dfmask[3])
#define INFO_DFA_CONT(n) (n->node[3])
#define INFO_DFA_THISASSIGN(n) (n->node[4])
#define INFO_DFA_INFER_LET_DEFMASK(n) (n->dfmask[4])
#define INFO_DFA_INFER_LET_USEMASK(n) (n->dfmask[5])
#define INFO_DFA_INFER_LET_LHSDONE(n) (n->flag)

/* multithread - barriers_init */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE!!! */
#define INFO_BARIN_WITHINMT(n) (n->int_data)

/* multithread - adjust_calls */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE!!! */
#define INFO_ADJCA_ATTRIB(n) ((statustype) (n->int_data))

/* precompile */
#define INFO_PREC_MODUL(n) (n->node[0])
#define INFO_PREC_CNT_ARTIFICIAL(n) (n->lineno)
#define INFO_PREC_OBJINITFUNDEF(n) (n->node[1])

/* ArrayElemination */
#define INFO_AE_TYPES(n) (n->node[1])

/* compile */
#define INFO_COMP_LASTIDS(n) (n->info.ids)
#define INFO_COMP_CNTPARAM(n) (n->lineno)
#define INFO_COMP_TYPETAB(n) ((types **)(n->info.types))
#define INFO_COMP_FIRSTASSIGN(n) (n->node[0])
#define INFO_COMP_LASTASSIGN(n) (n->node[0])
#define INFO_COMP_LASTLET(n) (n->node[1])
#define INFO_COMP_ICMTAB(n) ((node **)(n->node[1]))
#define INFO_COMP_FUNDEF(n) (n->node[2])
#define INFO_COMP_VARDECS(n) (n->node[3])
#define INFO_COMP_WITHBEGIN(n) (n->node[4])
#define INFO_COMP_MODUL(n) (n->node[5])
#define INFO_COMP_LAST_SYNC(n) (*((node **)(&(n->int_data))))
#define INFO_COMP_ACTUALATTRIB(n) ((statustype) (n->counter))

/* reuse */
#define INFO_REUSE_WL_IDS(n) (n->info.ids)
#define INFO_REUSE_FUNDEF(n) (n->node[0])
#define INFO_REUSE_IDX(n) ((ids *)(n->node[1]))
#define INFO_REUSE_DEC_RC_IDS(n) ((ids *)(n->node[2]))
#define INFO_REUSE_MASK(n) ((DFMmask_t) (n->dfmask[0]))
#define INFO_REUSE_NEGMASK(n) ((DFMmask_t) (n->dfmask[1]))

/* optimize */
#define INFO_MASK(n, x) (n->mask[x])
#define INFO_VARNO(n) (n->varno)

/* inline */
/*
 * ATTENTION: Usage of DUP and INL macros on arg_info are mixed. Be careful
 *            to avoid overlapping addresses.
 *
 *            INFO_INL_TYPES is used when DupTree is called by Inlining,
 *            so do not override that one there (INFO_DUP_xxx). (jhs)
 */
#define INFO_INL_FIRST_FUNC(n) (n->node[0])
#define INFO_INL_TYPES(n) (n->node[2])

/* WLF, all phases of WLF use these macros, not only WLI. */
#define INFO_WLI_NEXT(n) (n->node[0])
#define INFO_WLI_SUBST(n) (n->node[0])
#define INFO_WLI_WL(n) (n->node[1])
#define INFO_WLI_NEW_ID(n) (n->node[1])
#define INFO_WLI_ASSIGN(n) (n->node[2])
#define INFO_WLI_FUNDEF(n) (n->node[3])
#define INFO_WLI_ID(n) (n->node[4])
#define INFO_WLI_LET(n) (n->node[4])
#define INFO_WLI_NCA(n) (n->node[5])
#define INFO_WLI_REPLACE(n) (n->node[5])
#define INFO_WLI_FLAG(n) (n->flag)

/* CF */
#define INFO_CF_ASSIGN(n) (n->node[0])
#define INFO_CF_TYPE(n) (n->info.types)

/* IVE */
#define INFO_IVE_FUNDEF(n) (n->node[0])
#define INFO_IVE_VARDECS(n) (n->node[1])
#define INFO_IVE_CURRENTASSIGN(n) (n->node[2])
#define INFO_IVE_TRANSFORM_VINFO(n) (n->node[3])
#define INFO_IVE_MODE(n) (n->flag)
#define INFO_IVE_NON_SCAL_LEN(n) (n->counter)

/* DCR */
#define INFO_DCR_TRAVTYPE(n) (n->flag)
#define INFO_DCR_NEWACT(n) (n->lineno)
#define INFO_DCR_ACT(n) (n->mask[2])

/* Unrolling */
#define INFO_UNR_ASSIGN(n) (n->node[0])
#define INFO_UNR_FUNDEF(n) (n->node[1])

/* Icm2c, ... */
#define INFO_FUNDEF(n) (n->node[0])

/* Print */
#define INFO_PRINT_CONT(n) ((node *)(n->info2))
#define INFO_PRINT_FUNDEF(n) (n->node[0])
#define INFO_PRINT_INT_SYN(n) (n->node[2])
#define INFO_PRINT_WITH_RET(n) (n->node[3])
#define INFO_PRINT_NWITH(n) (n->node[4])
#define INFO_PRINT_ACCESS(n) (n->node[5])
#define INFO_PRINT_PRAGMA_WLCOMP(n) (n->info.cint)
#define INFO_PRINT_SIB(n) (n->flag)
#define INFO_PRINT_OMIT_FORMAL_PARAMS(n) (n->counter)
#define INFO_PRINT_VARNO(n) (n->varno)
#define INFO_PRINT_PROTOTYPE(n) (n->refcnt)
#define INFO_PRINT_SEPARATE(n) (n->int_data)

/* WL access analyze */
#define INFO_WLAA_LASTLETIDS(n) (n->info.ids)
#define INFO_WLAA_ACCESS(n) ((access_t *)(n->info2))
#define INFO_WLAA_COUNT(n) (n->counter)
#define INFO_WLAA_FEATURE(n) ((feature_t) (n->lineno))
#define INFO_WLAA_WOTYPE(n) ((WithOpType) (n->varno))
#define INFO_WLAA_BELOWAP(n) (n->flag)
#define INFO_WLAA_WLLEVEL(n) (n->refcnt)
#define INFO_WLAA_INDEXVAR(n) (n->node[0])
#define INFO_WLAA_ACCESSVEC(n) ((shpseg *)(n->node[1]))
#define INFO_WLAA_TMPACCESS(n) ((access_t *)(n->node[2]))
#define INFO_WLAA_WLARRAY(n) (n->node[3])

/* Tile Size Inference */
#define INFO_TSI_ACCESS(n) ((access_t *)(n->info2))
#define INFO_TSI_ACCESSCNT(n) (n->counter)
#define INFO_TSI_MINLINE(n) (n->flag)
#define INFO_TSI_MAXLINE(n) (n->refcnt)
#define INFO_TSI_FEATURE(n) (n->lineno)
#define INFO_TSI_WLCOMP(n) (n->int_data)
#define INFO_TSI_TILESHP(n) ((shpseg *)(n->node[1]))
#define INFO_TSI_INDEXVAR(n) (n->node[2])
#define INFO_TSI_WLARRAY(n) (n->node[3])
#define INFO_TSI_CACHEPARAM(n) ((int *)(n->node[4]))
#define INFO_TSI_CACHESIZE(n) ((int *)(n->node[4]))[0]
#define INFO_TSI_LINESIZE(n) ((int *)(n->node[4]))[1]
#define INFO_TSI_DATATYPE(n) ((int *)(n->node[4]))[2]

/* converting loops and conditionals to functions (lac2fun.c) */
#define INFO_LAC2FUN_FUNDEF(n) (n->node[0])
#define INFO_LAC2FUN_IN(n) ((DFMmask_t) (n->dfmask[0]))
#define INFO_LAC2FUN_OUT(n) ((DFMmask_t) (n->dfmask[1]))
#define INFO_LAC2FUN_LOCAL(n) ((DFMmask_t) (n->dfmask[2]))
#define INFO_LAC2FUN_NEEDED(n) ((DFMmask_t) (n->dfmask[3]))
#define INFO_LAC2FUN_ISFIX(n) (n->counter)
#define INFO_LAC2FUN_ISTRANS(n) (n->flag)
#define INFO_LAC2FUN_FUNS(n) (n->node[1])

/* cleaning up declarations (cleanup_decls.c) */
#define INFO_CUD_FUNDEF(n) (n->node[0])
#define INFO_CUD_REF(n) (n->dfmask[0])

/* reconverting functions to loops and conditionals (fun2lac.c) */
#define INFO_FUN2LAC_FUNDEF(n) (n->node[0])
#define INFO_FUN2LAC_FUNBLOCK(n) (n->node[1])
#define INFO_FUN2LAC_LET(n) (n->node[2])

/* adjusting identifiers (adjust_ids.c) */
#define INFO_AI_IDS(i) (i->info.ids)
#define INFO_AI_IDS_CHAIN(i) (i->info2)
#define INFO_AI_ARGS(n) (n->node[0])
#define INFO_AI_ARGS_CHAIN(n) (n->node[1])
#define INFO_AI_FUNDEF(n) (n->node[2])
#define INFO_AI_PREASSIGN(n) (n->node[3])
#define INFO_AI_POSTASSIGN(n) (n->node[4])

/* when map_wrapper.c */
#define INFO_MCW_MODUL(n) (n->node[0])
#define INFO_MCW_FUNDEF(n) (n->node[1])
#define INFO_MCW_FLAG(n) (n->flag)
#define INFO_MCW_CNT_STANDARD(n) (n->counter)

/* when used in print_interface.c */
#define INFO_PIH_FLAG(n) (n->flag)
#define INFO_PIH_COMMA(n) (n->varno)
#define INFO_PIH_COUNTER(n) (n->counter)
#define INFO_PIW_FLAG(n) (n->flag)
#define INFO_PIW_COMMA(n) (n->varno)
#define INFO_PIW_COUNTER(n) (n->counter)

/* when used in pad_collect.c */
#define INFO_APC_UNSUPPORTED(n) (n->flag)
#define INFO_APC_COUNT_CHANGES(n) (n->int_data)
#define INFO_APC_WITH(n) (n->node[0])

/* when used in pad_transform.c */
#define INFO_APT_EXPRESSION_PADDED(n) ((bool)(n->flag))
#define INFO_APT_WITHOP_TYPE(n) (n->int_data)
#define INFO_APT_WITH(n) (n->node[0])
#define INFO_APT_FUNDEF(n) (n->node[1])
#define INFO_APT_ASSIGNMENTS(n) (n->node[2])

/* when used in import_specialization.c */
#define INFO_IMPSPEC_SPECS(n) (n->node[0])
#define INFO_IMPSPEC_FUNDEF(n) (n->node[1])
#define INFO_IMPSPEC_MODUL(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_spmd :
 ***
 ***  sons:
 ***
 ***    node      REGION      (N_block)
 ***
 ***  permanent attributes:
 ***
 ***    DFMmask_t  IN          (spmdinit ->)
 ***    DFMmask_t  OUT         (spmdinit ->)
 ***    DFMmask_t  INOUT       (spmdinit ->)
 ***    DFMmask_t  LOCAL       (spmdinit ->)
 ***    DFMmask_t  SHARED      (spmdinit ->)
 ***
 ***    node*      FUNDEF      (N_fundef)
 ***
 ***  temporary attributes:
 ***
 ***    node*      ICM_BEGIN       (N_icm)   (compile -> print -> )
 ***    node*      ICM_PARALLEL    (N_block) (compile -> print -> )
 ***    node*      ICM_ALTSEQ      (N_icm)   (compile -> print -> )
 ***    node*      ICM_SEQUENTIAL  (N_block) (compile -> print -> )
 ***    node*      ICM_END         (N_icm)   (compile -> print -> )
 ***
 ***    int        STATIC
 ***
 ***  remarks:
 ***
 ***    STATIC is a flag used to distinguish between static spmd-blocks, i.e.
 ***    the decision whether to execute in parallel or not is done at compile
 ***    time, and dynamic ones.
 ***/

extern node *MakeSpmd (node *region);

#define SPMD_REGION(n) (n->node[0])

#define SPMD_IN(n) ((DFMmask_t)n->dfmask[0])
#define SPMD_INOUT(n) ((DFMmask_t)n->dfmask[1])
#define SPMD_OUT(n) ((DFMmask_t)n->dfmask[2])
#define SPMD_LOCAL(n) ((DFMmask_t)n->dfmask[3])
#define SPMD_SHARED(n) ((DFMmask_t)n->dfmask[4])

#define SPMD_FUNDEF(n) (n->node[1])
#define SPMD_ICM_BEGIN(n) (n->node[2])
#define SPMD_ICM_ALTSEQ(n) (n->node[3])
#define SPMD_ICM_END(n) (n->node[4])
#define SPMD_ICM_PARALLEL(n) (n->node[5])
#define SPMD_ICM_SEQUENTIAL(n) (n->node[0])

#define SPMD_STATIC(n) (n->varno)

/*--------------------------------------------------------------------------*/

/***
 ***  N_sync :
 ***
 ***  sons:
 ***
 ***    node*      REGION      (N_block)
 ***
 ***  permanent attributes:
 ***
 ***    int        FIRST            (is the sync-region the first one
 ***                                 of the current SPMD-region?)
 ***    int        LAST             (is the sync-region the last one
 ***                                 of the current SPMD-region?)
 ***
 ***    DFMmask_t  IN
 ***    DFMmask_t  OUT
 ***    DFMmask_t  INOUT
 ***    DFMmask_t  LOCAL
 ***
 ***  temporary attributes:
 ***
 ***    node*      WITH_PTRS   (N_exprs)   (syncinit -> syncopt -> compile ! )
 ***    SCHsched_t SCHEDULING              (syncinit (O) -> sched -> compile ! )
 ***/

extern node *MakeSync (node *region);

#define SYNC_FIRST(n) (n->flag)
#define SYNC_LAST(n) (n->int_data)
#define SYNC_FOLDCOUNT(n) (n->varno)
#define SYNC_REGION(n) (n->node[0])
#define SYNC_SCHEDULING(n) ((SCHsched_t) (n->node[1]))

#define SYNC_IN(n) ((DFMmask_t)n->dfmask[0])
#define SYNC_INOUT(n) ((DFMmask_t)n->dfmask[1])
#define SYNC_OUT(n) ((DFMmask_t)n->dfmask[2])
#define SYNC_OUTREP(n) ((DFMmask_t)n->dfmask[3])
#define SYNC_LOCAL(n) ((DFMmask_t)n->dfmask[4])

/*--------------------------------------------------------------------------*/

/***
 ***  N_mt :
 ***
 ***  sons:
 ***    node*      REGION     (N_block)
 ***
 ***  permanent attributes:
 ***    int        IDENTIFIER             Will be created by MakeMT, an copied
 ***                                      within DupTree, to identfy
 ***                                      corresponding blocks
 ***
 ***  temporary attributes:
 ***    DFMmask_t  USEMASK                (multithread.dfa   ->)
 ***    DFMmask_t  DEFMASK                (multithread.dfa   ->)
 ***    DFMmask_t  NEEDLATER              (multithread.dfa   ->)
 ***    DFMmask_t  ALLOC                  (not yet implemented -> compile)
 ***    node*      FUNDEF     (N_fundef)  (multithread.blkli ->
 ***                                       This mt-block was lifted to which
 ***                                       function?)
 ***/

extern node *MakeMT (node *region);

#define MT_IDENTIFIER(n) (n->int_data)
#define MT_REGION(n) (n->node[0])
#define MT_USEMASK(n) (n->dfmask[0])
#define MT_DEFMASK(n) (n->dfmask[1])
#define MT_NEEDLATER(n) (n->dfmask[2])
#define MT_ALLOC(n) (n->dfmask[3])
#define MT_FUNDEF(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_st :
 ***
 ***  sons:
 ***    node*     REGION      (N_block)
 ***
 ***  permanent attributes:
 ***    int        IDENTIFIER             Will be created by MakeST, an copied
 ***                                      within DupTree, to identfy
 ***                                      corresponding blocks
 ***
 ***  temporary attributes:
 ***    DFMmask_t  USEMASK        (multithread.dfa ->)
 ***    DFMmask_t  DEFMASK        (multithread.dfa ->)
 ***    DFMmask_t  NEEDLATER_ST   (multithread.dfa ->)
 ***    DFMmask_t  NEEDLATER_MT   (multithread.dfa ->)
 ***/

extern node *MakeST (node *region);

#define ST_IDENTIFIER(n) (n->int_data)
#define ST_REGION(n) (n->node[0])
#define ST_USEMASK(n) (n->dfmask[0])
#define ST_DEFMASK(n) (n->dfmask[1])
#define ST_NEEDLATER_ST(n) (n->dfmask[2])
#define ST_NEEDLATER_MT(n) (n->dfmask[3])
#define ST_ALLOC(n) (n->dfmask[4])
#define ST_SYNC(n) (n->dfmask[5])

/*--------------------------------------------------------------------------*/

/***
 ***  N_MTsignal :
 ***
 ***  permanent atrributes:
 ***    DFMmask_t   IDSET
 ***/

extern node *MakeMTsignal ();

#define MTSIGNAL_IDSET(n) (n->dfmask[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_MTsync :
 ***
 ***  permanent atrributes:
 ***    DFMmask_t      WAIT
 ***    DFMfoldmask_t  FOLD
 ***    DFMmask_t      ALLOC
 ***/

extern node *MakeMTsync ();

#define MTSYNC_WAIT(n) (n->dfmask[0])
#define MTSYNC_FOLD(n) ((DFMfoldmask_t *)(n->node[0]))
#define MTSYNC_ALLOC(n) (n->dfmask[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_MTalloc :
 ***
 ***  permanent atrributes:
 ***    DFMmask_t   IDSET
 ***/

extern node *MakeMTalloc ();

#define MTALLOC_IDSET(n) (n->dfmask[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith :
 ***
 ***  sons:
 ***
 ***    node*      PART       (N_Npart)
 ***    node*      CODE       (N_Ncode)
 ***    node*      WITHOP     (N_Nwithop)
 ***
 ***  permanent attributes:
 ***
 ***    int        PARTS   (number of N_Npart nodes for this WL.
 ***                         -1: no complete partition, exactly one N_Npart,
 ***                         >0: complete partition.
 ***
 ***  temporary attributes:
 ***
 ***    node*      PRAGMA     (N_pragma)  (scanparse -> wltransform ! )
 ***    int        REFERENCED             (wlt -> wlf !!
 ***    int        REFERENCED_FOLD        (wlt -> wlf !!)
 ***    int        REFERENCES_FOLDED      (wlt -> wlf !!)
 ***    bool       FOLDABLE               (wlt -> wlf !!)
 ***    bool       NO_CHANCE              (wlt -> wlf !!)
 ***    ids*       DEC_RC_IDS             (refcount -> wltransform )
 ***    node*      TSI                    (tile size inference -> )
 ***
 ***    DFMmask_t  IN                     (refcount -> wltransform )
 ***    DFMmask_t  INOUT                  (refcount -> wltransform )
 ***    DFMmask_t  OUT                    (refcount -> wltransform )
 ***    DFMmask_t  LOCAL                  (refcount -> wltransform )
 ***/

extern node *MakeNWith (node *part, node *code, node *withop);

#define NWITH_PART(n) ((n)->node[0])
#define NWITH_CODE(n) ((n)->node[1])
#define NWITH_WITHOP(n) ((n)->node[2])
#define NWITH_PRAGMA(n) ((n)->node[3])
#define NWITH_DEC_RC_IDS(n) ((ids *)((n)->node[4]))
#define NWITH_WLAA(n) ((n)->node[5])

#define NWITH_PARTS(n) (((wl_info *)((n)->info2))->parts)
#define NWITH_REFERENCED(n) (((wl_info *)((n)->info2))->referenced)
#define NWITH_REFERENCED_FOLD(n) (((wl_info *)((n)->info2))->referenced_fold)
#define NWITH_REFERENCES_FOLDED(n) (((wl_info *)((n)->info2))->references_folded)
#define NWITH_FOLDABLE(n) (((wl_info *)((n)->info2))->foldable)
#define NWITH_NO_CHANCE(n) (((wl_info *)((n)->info2))->no_chance)

#define NWITH_IN(n) ((DFMmask_t) (n)->dfmask[0])
#define NWITH_INOUT(n) ((DFMmask_t) (n)->dfmask[1])
#define NWITH_OUT(n) ((DFMmask_t) (n)->dfmask[2])
#define NWITH_LOCAL(n) ((DFMmask_t) (n)->dfmask[3])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Npart :
 ***
 ***  sons:
 ***
 ***    node*  WITHID        (N_Nwithid)
 ***    node*  GEN           (N_Ngenerator)
 ***    node*  NEXT      (O) (N_Npart)
 ***
 ***  permanent attributes:
 ***
 ***    node*  CODE          (N_Ncode)
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK          (optimize -> )
 ***    bool   COPY          (Unroll !)
 ***    bool   PADDED        (ap-> )
 ***/

extern node *MakeNPart (node *withid, node *generator, node *code);

#define NPART_WITHID(n) ((n)->node[0])
#define NPART_GEN(n) ((n)->node[1])
#define NPART_NEXT(n) ((n)->node[2])
#define NPART_CODE(n) ((n)->node[3])
#define NPART_COPY(n) ((bool)((n)->flag))
#define NPART_MASK(n, x) ((n)->mask[x])
#define NPART_PADDED(n) ((bool)((n)->flag))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwithid :
 ***
 ***  permanent attributes:
 ***
 ***    ids*         VEC
 ***    ids*         IDS
 ***/

extern node *MakeNWithid (ids *vec, ids *scalars);

#define NWITHID_VEC(n) ((n)->info.ids)
#define NWITHID_IDS(n) ((ids *)((n)->info2))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Ngenerator :
 ***
 ***  sons:
 ***
 ***    node*  BOUND1    (O)  ("N_expr")
 ***    node*  BOUND2    (O)  ("N_expr")
 ***    node*  STEP      (O)  ("N_expr")
 ***    node*  WIDTH     (O)  ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    prf    OP1
 ***    prf    OP2
 ***    prf    OP1_ORIG     Set in MakeNGenerator, not to be changed!
 ****   prf    OP2_ORIG     Set in MakeNGenerator, not to be changed!
 ***
 ***  remarks:
 ***    the BOUNDs are NULL if upper or lower bounds are not specified.
 ***    if STEP is NULL, step 1 is assumed (no grid)
 ***    if WIDTH is NULL, width 1 is assumed
 ***/

extern node *MakeNGenerator (node *bound1, node *bound2, prf op1, prf op2, node *step,
                             node *width);

#define NGEN_BOUND1(n) ((n)->node[0])
#define NGEN_BOUND2(n) ((n)->node[1])
#define NGEN_STEP(n) ((n)->node[2])
#define NGEN_WIDTH(n) ((n)->node[3])
#define NGEN_OP1(n) ((n)->info.genrel.op1)
#define NGEN_OP2(n) ((n)->info.genrel.op2)
#define NGEN_OP1_ORIG(n) ((n)->info.genrel.op1_orig)
#define NGEN_OP2_ORIG(n) ((n)->info.genrel.op2_orig)

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwithop :
 ***
 ***  the meaning of the sons/attributes of this node depend on WithOpType.
 ***
 ***  sons:
 ***    node*  SHAPE      ("N_expr": N_array, N_id) (iff WithOpType == WO_genarray)
 ***    node*  ARRAY      ("N_expr": N_array, N_id) (iff WithOpType == WO_modarray)
 ***    node*  NEUTRAL    ("N_expr")                (otherwise )
 ***
 ***  permanent attributes:
 ***
 ***    WithOpType TYPE
 ***    char*      FUN         (iff WithOpType == WO_foldfun)
 ***    char*      MOD         (iff WithOpType == WO_foldfun)
 ***    prf        PRF         (iff WithOpType == WO_foldprf)
 ***
 ***  temporary attributes:
 ***
 ***    node*  EXPR            (scanparse, NULL afterwards)
 ***    node*  FUNDEF          (N_fundef)  (typecheck -> )
 ***    long*  MASK                        (optimize -> )
 ***
 ***  remarks:
 ***
 ***    - WithOpType is WO_genarray, WO_modarray, WO_foldfun, WO_foldprf.
 ***    - FUNDEF-node is used if (TYPE == WO_foldfun, WO_foldprf).
 ***/

extern node *MakeNWithOp (WithOpType WithOp);

#define NWITHOP_TYPE(n) (*((WithOpType *)(n)->info2))
#define NWITHOP_FUN(n) ((n)->info.fun_name.id)
#define NWITHOP_MOD(n) ((n)->info.fun_name.id_mod)
#define NWITHOP_PRF(n) ((n)->info.prf)
#define NWITHOP_SHAPE(n) ((n)->node[0])
#define NWITHOP_ARRAY(n) ((n)->node[0])
#define NWITHOP_NEUTRAL(n) ((n)->node[0])
#define NWITHOP_EXPR(n) ((n)->node[1])
#define NWITHOP_FUNDEF(n) ((n)->node[2])
#define NWITHOP_MASK(n, x) ((n)->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Ncode :
 ***
 ***  sons:
 ***
 ***    node*      CBLOCK    (O) (N_block)
 ***    node*      CEXPR         ("N_expr")
 ***    node*      NEXT      (O) (N_Ncode)
 ***
 ***  permanent attributes:
 ***
 ***    int        USED       (number of times this code is used)
 ***
 ***  temporary attributes:
 ***
 ***    int        NO         (unambiguous number for PrintNwith2())
 ***                                      (precompile -> )
 ***    long*      MASK                    (optimize -> )
 ***    node *     USE         (N_vinfo)   (IVE -> )
 ***    bool       FLAG                    (WLI -> WLF)
 ***    ids*       INC_RC_IDS              (refcount -> compile )
 ***
 ***    node*      WLAA_INFO(n)            (wlaa -> )
 ***    access_t*  WLAA_ACCESS             (wlaa -> )
 ***    int        WLAA_ACCESSCNT(n)       (wlaa -> )
 ***    int        WLAA_ARRAYDIM(n)        (wlaa -> )
 ***    feature_t* WLAA_FEATURE            (wlaa -> )
 ***    shpseg*    WLAA_ARRAYSHP(n)        (wlaa -> )
 ***    node*      WLAA_WLARRAY(n)         (wlaa -> )
 ***    shpseg*    TSI_TILESHP(n)          (tsi  -> )
 ***    int        AP_DUMMY_CODE(n)        (ap   -> )
 ***
 ***  remarks:
 ***
 ***    The CBLOCK 'plus' the CEXPR is the whole assignment block
 ***    to calculate each element of the WL. The CEXPR is the pseudo
 ***    return statement of the block.
 ***    In the flatten phase every node unequal N_id is flattened from
 ***    the CEXPR into the CBLOCK.
 ***
 ***    The USED component is a reference counter for the NPART_CODE pointer.
 ***    MakeNPart increments it if the code parameter is != NULL,
 ***    FreeNPart decrements it if NPART_CODE is != NULL.
 ***    DupNpart  increments it (implicitly in MakeNPart, see condition above).
 ***
 ***    FEATURE is a bit mask which characterizes the code of CBLOCK
 ***    (see types.h for details).
 ***
 ***    ACCESS is a list of array accesses.
 ***    Both FEATURE and ACCESS are used for the tile size inference scheme.
 ***
 ***    ACCESCNT is the number of array accesses stored in ACCESS.
 ***/

extern node *MakeNCode (node *block, node *expr);

#define NCODE_CBLOCK(n) ((n)->node[0])
#define NCODE_CEXPR(n) ((n)->node[1])
#define NCODE_NEXT(n) ((n)->node[2])
#define NCODE_INC_RC_IDS(n) ((ids *)((n)->node[3]))
#define NCODE_USE(n) ((n)->node[4])
#define NCODE_USED(n) ((n)->info.cint)
#define NCODE_MASK(n, x) ((n)->mask[x])
#define NCODE_NO(n) ((n)->refcnt)
#define NCODE_FLAG(n) ((bool)((n)->flag))

#define NCODE_WLAA_INFO(n) ((node *)(n)->info2)
#define NCODE_WLAA_ACCESS(n) ((access_t *)(((node *)(n)->info2)->info2))
#define NCODE_WLAA_ACCESSCNT(n) (((node *)(n)->info2)->counter)
#define NCODE_WLAA_FEATURE(n) (((node *)(n)->info2)->varno)
#define NCODE_WLAA_INDEXVAR(n) (((node *)(n)->info2)->node[2])
#define NCODE_WLAA_WLARRAY(n) (((node *)(n)->info2)->node[3])

#define NCODE_WLAA_ARRAYSHP(n) VARDEC_SHPSEG (NCODE_WLAA_WLARRAY (n))
#define NCODE_WLAA_INDEXDIM(n) VARDEC_SHAPE (NCODE_WLAA_INDEXVAR (n), 0)
#define NCODE_WLAA_ARRAYDIM(n) VARDEC_DIM (NCODE_WLAA_WLARRAY (n))

#define NCODE_TSI_TILESHP(n) ((shpseg *)(((node *)(n)->info2)->node[4]))

#define NCODE_APT_DUMMY_CODE(n) ((n)->int_data)

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith2 :
 ***
 ***  sons:
 ***
 ***    node*      WITHID        (N_Nwithid)
 ***    node*      SEGS          (N_WLseg, N_WLsegVar)
 ***    node*      CODE          (N_Ncode)
 ***    node*      WITHOP        (N_Nwithop)
 ***
 ***  permanent attributes:
 ***
 ***    int        DIMS
 ***
 ***    DFMmask_t  IN
 ***    DFMmask_t  INOUT
 ***    DFMmask_t  OUT
 ***    DFMmask_t  LOCAL
 ***    bool       MT
 ***
 ***
 ***  temporary attributes:
 ***
 ***    ids*       DEC_RC_IDS                     (wltransform -> compile )
 ***
 ***    bool       ISSCHEDULED                    (new mt -> ...)
 ***                       [Signals whether any segment is scheduled or not]
 ***
 ***    DFMmask_t  REUSE                          (ReuseWithArrays -> compile ! )
 ***
 ***    SCHsched_t SCHEDULING   (O)               (wltransform -> compile )
 ***/

extern node *MakeNWith2 (node *withid, node *seg, node *code, node *withop, int dims);

#define NWITH2_DIMS(n) ((n)->flag)
#define NWITH2_MT(n) ((bool)((n)->counter))
#define NWITH2_WITHID(n) ((n)->node[0])
#define NWITH2_SEGS(n) ((n)->node[1])
#define NWITH2_CODE(n) ((n)->node[2])
#define NWITH2_WITHOP(n) ((n)->node[3])

#define NWITH2_SCHEDULING(n) ((SCHsched_t) ((n)->node[4]))
#define NWITH2_ISSCHEDULED(n) ((n)->int_data)
#define NWITH2_DEC_RC_IDS(n) ((ids *)((n)->node[5]))

#define NWITH2_IN(n) ((DFMmask_t) ((n)->dfmask[0]))
#define NWITH2_INOUT(n) ((DFMmask_t) ((n)->dfmask[1]))
#define NWITH2_OUT(n) ((DFMmask_t) ((n)->dfmask[2]))
#define NWITH2_LOCAL(n) ((DFMmask_t) ((n)->dfmask[3]))
#define NWITH2_REUSE(n) ((DFMmask_t) ((n)->dfmask[4]))

/*--------------------------------------------------------------------------*/

/*
 * here are some macros for N_WL... nodes
 *
 *
 * CAUTION: not every macro is suitable for all node tpyes.
 *          e.g. NEXTDIM is not a son of N_WLstride nodes
 *
 *          it would be better to contruct these macros like this:
 *            #define WLNODE_NEXTDIM(n) ((NODE_TYPE(n) == N_WLstride) ?
 *                                        DBUG_ASSERT(...) :
 *                                        (NODE_TYPE(n) == N_WLblock) ?
 *                                         WLBLOCK_NEXTDIM(n) :
 *                                         (NODE_TYPE(n) == N_WLublock) ?
 *                                          WLUBLOCK_NEXTDIM(n) : ...)
 *          but unfortunately this is not a modifiable l-value in ANSI-C :(
 *          so it would be impossible to use them on the left side of an
 *          assignment.
 *          because of that I designed this "static" macros to make a
 *          concise modelling of routines still possible.
 */

#define WLNODE_LEVEL(n) ((n)->lineno)
#define WLNODE_DIM(n) ((n)->refcnt)
#define WLNODE_BOUND1(n) ((n)->flag)
#define WLNODE_BOUND2(n) ((n)->counter)
#define WLNODE_STEP(n) ((n)->varno)
#define WLNODE_NEXTDIM(n) ((n)->node[0])
#define WLNODE_NEXT(n) ((n)->node[1])

/*
 * some macros for N_WLseg, N_WLsegVar nodes
 */

#define WLSEGX_DIMS(n) ((n)->refcnt)
#define WLSEGX_CONTENTS(n) ((n)->node[0])
#define WLSEGX_NEXT(n) (WLNODE_NEXT (n))
#define WLSEGX_IDX_MIN(n) (*((int **)(&((n)->node[2]))))
#define WLSEGX_IDX_MAX(n) (*((int **)(&((n)->node[3]))))
#define WLSEGX_BLOCKS(n) ((n)->flag)
#define WLSEGX_SV(n) ((int *)((n)->mask[0]))
#define WLSEGX_BV(n, level) ((int *)((n)->mask[level + 2]))
#define WLSEGX_UBV(n) ((int *)((n)->mask[1]))
#define WLSEGX_SCHEDULING(n) ((SCHsched_t *)(n)->info2)

/*
 * some macros for N_WLstride, N_WLstriVar nodes
 */

#define WLSTRIX_LEVEL(n) (WLNODE_LEVEL (n))
#define WLSTRIX_DIM(n) (WLNODE_DIM (n))
#define WLSTRIX_CONTENTS(n) ((n)->node[0])
#define WLSTRIX_NEXT(n) (WLNODE_NEXT (n))

/*
 * some macros for N_WLgrid, N_WLgridVar nodes
 */

#define WLGRIDX_LEVEL(n) (WLNODE_LEVEL (n))
#define WLGRIDX_DIM(n) (WLNODE_DIM (n))
#define WLGRIDX_NEXTDIM(n) (WLNODE_NEXTDIM (n))
#define WLGRIDX_NEXT(n) (WLNODE_NEXT (n))
#define WLGRIDX_CODE(n) ((n)->node[4])

/*--------------------------------------------------------------------------*/

/***
 *** N_WLseg :
 ***
 ***  sons:
 ***
 ***    node*      CONTENTS       (N_WLblock, N_WLublock, N_WLstride)
 ***    node*      NEXT           (N_WLseg)
 ***
 ***  permanent attributes:
 ***
 ***    int        DIMS      (number of dims)
 ***
 ***  temporary attributes:
 ***
 ***    int*       IDX_MIN                           (wltransform -> compile )
 ***    int*       IDX_MAX                           (wltransform -> compile )
 ***
 ***    int        BLOCKS    (number of blocking levels (0..3)
 ***                           --- without unrolling-blocking)
 ***    int*       SV        (step vector)           (wltransform -> )
 ***    int*       BV[]      (blocking vectors)      (wltransform -> compile )
 ***    int*       UBV       (unrolling-bl. vector)  (wltransform -> compile )
 ***
 ***    SCHsched_t SCHEDULING  (O)                   (wltransform -> compile )
 ***    int        MAXHOMDIM (last homog. dimension) (wltransform -> compile )
 ***    int*       HOMSV     (homog. step vector)    (wltransform -> compile )
 ***
 ***  remarks:
 ***
 ***    - BV[ 0 .. (BLOCKS-1) ]
 ***    - IDX_MIN, IDX_MAX, SV, BV[0], BV[1], ..., UBV, HOMSV are vectors of
 ***      size DIMS.
 ***    - MAXHOMDIM is element of the set {-1, 0, 1, ..., DIMS-1}.
 ***      -1 is the default value (= no homogeneous dimensions).
 ***    - (HOMSV[i] == 0)  <->  (i > MAXHOMDIM),
 ***      (HOMSV[i] > 0)   <->  (i <= MAXHOMDIM).
 ***/

extern node *MakeWLseg (int dims, node *contents, node *next);

#define WLSEG_DIMS(n) (WLSEGX_DIMS (n))
#define WLSEG_CONTENTS(n) (WLSEGX_CONTENTS (n))
#define WLSEG_NEXT(n) (WLSEGX_NEXT (n))

#define WLSEG_IDX_MIN(n) (WLSEGX_IDX_MIN (n))
#define WLSEG_IDX_MAX(n) (WLSEGX_IDX_MAX (n))

#define WLSEG_BLOCKS(n) (WLSEGX_BLOCKS (n))
#define WLSEG_SV(n) (WLSEGX_SV (n))
#define WLSEG_BV(n, level) (WLSEGX_BV (n, level))
#define WLSEG_UBV(n) (WLSEGX_UBV (n))

#define WLSEG_SCHEDULING(n) (WLSEGX_SCHEDULING (n))
#define WLSEG_MAXHOMDIM(n) ((n)->varno)
#define WLSEG_HOMSV(n) ((int *)((n)->mask[6]))

/*--------------------------------------------------------------------------*/

/***
 *** N_WLblock :
 ***
 ***  sons:
 ***
 ***    node*    NEXTDIM       (N_WLblock)
 ***    node*    CONTENTS      (N_WLublock, N_WLstride)
 ***    node*    NEXT          (N_WLblock)
 ***
 ***  permanent attributes:
 ***
 ***    int      LEVEL                (number of blocking-levels so far)
 ***    int      DIM
 ***    int      BOUND1
 ***    int      BOUND2
 ***    int      STEP
 ***
 ***  temporary attributes:
 ***
 ***    ---
 ***
 ***  remarks:
 ***
 ***    - it makes no sense to use the nodes NEXTDIM and CONTENTS simultaneous!
 ***/

extern node *MakeWLblock (int level, int dim, int bound1, int bound2, int step,
                          node *nextdim, node *contents, node *next);

#define WLBLOCK_LEVEL(n) (WLNODE_LEVEL (n))
#define WLBLOCK_DIM(n) (WLNODE_DIM (n))
#define WLBLOCK_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLBLOCK_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLBLOCK_STEP(n) (WLNODE_STEP (n))
#define WLBLOCK_NEXTDIM(n) (WLNODE_NEXTDIM (n))
#define WLBLOCK_CONTENTS(n) ((n)->node[2])
#define WLBLOCK_NEXT(n) (WLNODE_NEXT (n))

/*--------------------------------------------------------------------------*/

/***
 *** N_WLublock :
 ***
 ***  sons:
 ***
 ***    node*    NEXTDIM       (N_WLublock)
 ***    node*    CONTENTS      (N_WLstride)
 ***    node*    NEXT          (N_WLublock)
 ***
 ***  permanent attributes:
 ***
 ***    int      LEVEL
 ***    int      DIM
 ***    int      BOUND1
 ***    int      BOUND2
 ***    int      STEP
 ***
 ***  temporary attributes:
 ***
 ***    ---
 ***
 ***  remarks:
 ***
 ***    - it makes no sense to use the nodes NEXTDIM and CONTENTS simultaneous!
 ***/

extern node *MakeWLublock (int level, int dim, int bound1, int bound2, int step,
                           node *nextdim, node *contents, node *next);

#define WLUBLOCK_LEVEL(n) (WLBLOCK_LEVEL (n))
#define WLUBLOCK_DIM(n) (WLBLOCK_DIM (n))
#define WLUBLOCK_BOUND1(n) (WLBLOCK_BOUND1 (n))
#define WLUBLOCK_BOUND2(n) (WLBLOCK_BOUND2 (n))
#define WLUBLOCK_STEP(n) (WLBLOCK_STEP (n))
#define WLUBLOCK_NEXTDIM(n) (WLBLOCK_NEXTDIM (n))
#define WLUBLOCK_CONTENTS(n) (WLBLOCK_CONTENTS (n))
#define WLUBLOCK_NEXT(n) (WLBLOCK_NEXT (n))

/*--------------------------------------------------------------------------*/

/***
 *** N_WLstride :
 ***
 ***  sons:
 ***
 ***    node*    CONTENTS     (N_WLgrid)
 ***    node*    NEXT         (N_WLstride)
 ***
 ***  permanent attributes:
 ***
 ***    int      LEVEL
 ***    int      DIM
 ***    int      BOUND1
 ***    int      BOUND2
 ***    int      STEP
 ***    bool     UNROLLING    (unrolling wanted?)
 ***
 ***  temporary attributes:
 ***
 ***    node*    PART         (part this stride is generated from)
 ***                                                 (wltransform ! )
 ***    node*    MODIFIED                            (wltransform ! )
 ***/

extern node *MakeWLstride (int level, int dim, int bound1, int bound2, int step,
                           bool unrolling, node *contents, node *next);

#define WLSTRIDE_LEVEL(n) (WLSTRIX_LEVEL (n))
#define WLSTRIDE_DIM(n) (WLSTRIX_DIM (n))
#define WLSTRIDE_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLSTRIDE_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLSTRIDE_STEP(n) (WLNODE_STEP (n))
#define WLSTRIDE_UNROLLING(n) ((bool)((n)->info.prf_dec.tc))
#define WLSTRIDE_CONTENTS(n) (WLSTRIX_CONTENTS (n))
#define WLSTRIDE_NEXT(n) (WLSTRIX_NEXT (n))

#define WLSTRIDE_PART(n) ((n)->node[2])
#define WLSTRIDE_MODIFIED(n) ((n)->node[3])

/*--------------------------------------------------------------------------*/

/***
 *** N_WLgrid :
 ***
 ***  sons:
 ***
 ***    node*   NEXTDIM     (N_WLblock, N_WLublock, N_WLstride, N_WLstriVar)
 ***    node*   NEXT        (N_WLgrid, N_WLgridVar)
 ***
 ***  permanent attributes:
 ***
 ***    node*   CODE        (N_Ncode)
 ***    int     LEVEL
 ***    int     DIM
 ***    int     BOUND1
 ***    int     BOUND2
 ***    bool    UNROLLING
 ***
 ***  temporary attributes:
 ***
 ***    node*   MODIFIED                                (wltransform ! )
 ***
 ***  remarks:
 ***
 ***    - it makes no sense to use the nodes NEXTDIM and CODE simultaneous!
 ***    - (NEXTDIM == NULL) *and* (CODE == NULL) means that this is a dummy grid,
 ***      representing a simple init- (genarray), copy- (modarray), noop- (fold)
 ***      operation.
 ***/

extern node *MakeWLgrid (int level, int dim, int bound1, int bound2, bool unrolling,
                         node *nextdim, node *next, node *code);

#define WLGRID_LEVEL(n) (WLGRIDX_LEVEL (n))
#define WLGRID_DIM(n) (WLGRIDX_DIM (n))
#define WLGRID_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLGRID_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLGRID_UNROLLING(n) ((bool)((n)->info.prf_dec.tag))
#define WLGRID_NEXTDIM(n) (WLGRIDX_NEXTDIM (n))
#define WLGRID_NEXT(n) (WLGRIDX_NEXT (n))
#define WLGRID_CODE(n) (WLGRIDX_CODE (n))

#define WLGRID_MODIFIED(n) ((n)->node[2])

/*--------------------------------------------------------------------------*/

/***
 *** N_WLsegVar :
 ***
 ***  sons:
 ***
 ***    node*      CONTENTS       (N_WLstride, N_WLstriVar)
 ***    node*      NEXT           (N_WLsegVar)
 ***
 ***  permanent attributes:
 ***
 ***    int        DIMS      (number of dims)
 ***
 ***  temporary attributes:
 ***
 ***    int*       IDX_MIN                           (wltransform -> compile )
 ***    int*       IDX_MAX                           (wltransform -> compile )
 ***
 ***    int        BLOCKS    (number of blocking levels
 ***                           --- without unrolling-blocking)
 ***    int*       SV        (step vector)           (wltransform -> )
 ***    int*       BV[]      (blocking vectors)      (wltransform -> compile )
 ***    int*       UBV       (unrolling-b. vector)   (wltransform -> compile )
 ***
 ***    SCHsched_t SCHEDULING  (O)                   (wltransform -> compile )
 ***
 ***  remarks:
 ***
 ***    - BV[ 0 .. (BLOCKS-1) ]
 ***    - IDX_MIN, IDX_MAX, SV, BV[0], BV[1], ..., UBV, HOMSV are vectors of
 ***      size DIMS.
 ***/

extern node *MakeWLsegVar (int dims, node *contents, node *next);

#define WLSEGVAR_DIMS(n) (WLSEGX_DIMS (n))
#define WLSEGVAR_CONTENTS(n) (WLSEGX_CONTENTS (n))
#define WLSEGVAR_NEXT(n) (WLSEGX_NEXT (n))

#define WLSEGVAR_IDX_MIN(n) (WLSEGX_IDX_MIN (n))
#define WLSEGVAR_IDX_MAX(n) (WLSEGX_IDX_MAX (n))

#define WLSEGVAR_BLOCKS(n) (WLSEGX_BLOCKS (n))
#define WLSEGVAR_SV(n) (WLSEGX_SV (n))
#define WLSEGVAR_BV(n, level) (WLSEGX_BV (n, level))
#define WLSEGVAR_UBV(n) (WLSEGX_UBV (n))

#define WLSEGVAR_SCHEDULING(n) (WLSEGX_SCHEDULING (n))

/*--------------------------------------------------------------------------*/

/***
 *** N_WLstriVar :
 ***
 ***  sons:
 ***
 ***    node*    CONTENTS      (N_WLgridVar, N_WLgrid)
 ***    node*    NEXT          (N_WLstriVar)
 ***    node*    BOUND1        (N_num, N_id)
 ***    node*    BOUND2        (N_num, N_id)
 ***    node*    STEP          (N_num, N_id)
 ***
 ***  permanent attributes:
 ***
 ***    int      LEVEL
 ***    int      DIM
 ***
 ***  temporary attributes:
 ***
 ***    ---
 ***/

extern node *MakeWLstriVar (int level, int dim, node *bound1, node *bound2, node *step,
                            node *contents, node *next);

#define WLSTRIVAR_LEVEL(n) (WLSTRIX_LEVEL (n))
#define WLSTRIVAR_DIM(n) (WLSTRIX_DIM (n))
#define WLSTRIVAR_BOUND1(n) ((n)->node[2])
#define WLSTRIVAR_BOUND2(n) ((n)->node[3])
#define WLSTRIVAR_STEP(n) ((n)->node[4])
#define WLSTRIVAR_CONTENTS(n) (WLSTRIX_CONTENTS (n))
#define WLSTRIVAR_NEXT(n) (WLSTRIX_NEXT (n))

/*--------------------------------------------------------------------------*/

/***
 *** N_WLgridVar :
 ***
 ***  sons:
 ***
 ***    node*    NEXTDIM         (N_WLstriVar)
 ***    node*    NEXT            (N_WLgridVar, N_WLgrid)
 ***    node*    BOUND1          (N_num, N_id)
 ***    node*    BOUND2          (N_num, N_id)
 ***
 ***  permanent attributes:
 ***
 ***    node*    CODE            (N_Ncode)
 ***    int      LEVEL
 ***    int      DIM
 ***
 ***  temporary attributes:
 ***
 ***    ---
 ***
 ***  remarks:
 ***
 ***    - it makes no sense to use the nodes NEXTDIM and CODE simultaneous!
 ***    - (NEXTDIM == NULL) *and* (CODE == NULL) means that this is a dummy grid,
 ***      representing a simple init- (genarray), copy- (modarray), noop- (fold)
 ***      operation.
 ***/

extern node *MakeWLgridVar (int level, int dim, node *bound1, node *bound2, node *nextdim,
                            node *next, node *code);

#define WLGRIDVAR_LEVEL(n) (WLGRIDX_LEVEL (n))
#define WLGRIDVAR_DIM(n) (WLGRIDX_DIM (n))
#define WLGRIDVAR_BOUND1(n) ((n)->node[2])
#define WLGRIDVAR_BOUND2(n) ((n)->node[3])
#define WLGRIDVAR_NEXTDIM(n) (WLGRIDX_NEXTDIM (n))
#define WLGRIDVAR_NEXT(n) (WLGRIDX_NEXT (n))
#define WLGRIDVAR_CODE(n) (WLGRIDX_CODE (n))

/*--------------------------------------------------------------------------*/

/***
 *** N_cwrapper :
 ***
 ***  sons:
 ***
 ***    node*     NEXT            (N_cwrapper)
 ***
 ***  permanent attributes:
 ***
 ***    nodelist* FUNS            nodelist with fundefs (mapwrapper -> )
 ***    char*     NAME            name of wrapper function
 ***    char*     MOD             modulename
 ***    int       ARGCOUNT        # of arguments
 ***    int       RESCOUNT        # of results
 ***
 ***  temporary attributes:
 ***
 ***    ---
 ***
 ***  remarks: -
 ***
 ***
 ***
 ***/

extern node *MakeCWrapper (node *next, char *name, char *mod, int argcount, int rescount);

#define CWRAPPER_NEXT(n) (n->node[0])
#define CWRAPPER_NAME(n) (n->info.id)
#define CWRAPPER_MOD(n) ((char *)(n->dfmask[0]))
#define CWRAPPER_ARGCOUNT(n) (n->counter)
#define CWRAPPER_RESCOUNT(n) (n->varno)
#define CWRAPPER_FUNS(n) ((nodelist *)(n->info2))

/*--------------------------------------------------------------------------*/

/***
 ***  N_modspec :
 ***
 ***  sons:
 ***
 ***  permanent attributes:
 ***
 ***    node*  IMPORTS    (O)  (N_implist)
 ***    node*  OWN        (O)  (N_explist)
 ***    char*  NAME
 ***    deps*  LINKWITH   (O)
 ***    int    ISEXTERNAL
 ***/

extern node *MakeModspec (char *name, node *exports);

#define MODSPEC_NAME(n) (n->info.fun_name.id)
#define MODSPEC_OWN(n) (n->node[0])

/*--------------------------------------------------------------------------*/

#endif /* _sac_tree_basic_h */
