/*
 *
 * $Log$
 * Revision 2.4  1999/03/24 19:56:03  bs
 * Access macro ID_INDEX added.
 *
 * Revision 2.3  1999/03/17 21:24:09  bs
 * Access macro ARRAY_CHARVEC added.
 *
 * Revision 2.2  1999/03/15 13:54:23  bs
 * Renamed:
 * ID_CONSTARRAY => ID_INTVEC
 * ID_ARRAYLENGTH => ID_VECLEN
 * ARRAY_INTARRAY => ARRAY_INTVEC
 * ARRAY_LENGTH => ARRAY_VECLEN
 * Added:
 * ARRAY_FLOATVEC, ARRAY_DOUBLEVEC, ARRAY_VECTYPE
 *
 * Revision 2.1  1999/02/23 12:39:56  sacbase
 * new release made
 *
 * Revision 1.216  1999/02/12 17:44:51  cg
 * WLSEGX mechanism now also used for WLSEG_SCHEDULING and WLSEGVAR_SCHEDULING
 *
 * Revision 1.215  1999/02/12 12:32:28  bs
 * ID_CONSTARRAY and ID_ARRAYLENGTH inserted
 *
 * Revision 1.214  1999/02/11 13:35:22  cg
 * OBJDEC_DEF() and OBJDEF_VARNAME() no longer use the same entry in the
 * underlying node structure. This caused severe memory management problems
 * when freeing the own declaration of a module/class implementation after
 * writing the SIB.
 *
 * Revision 1.213  1999/02/11 08:31:22  bs
 * INFO_FLTN_INTARRAY and INFO_FLTN_ARRAYLENGTH inserted
 *
 * Revision 1.212  1999/02/11 08:21:21  bs
 * added access macros for the storage of constant integer arrays as
 * arrays of integer ( int []).
 *
 * Revision 1.211  1999/02/06 12:52:47  srs
 * added MakeNodelistNode() and macro ATTRIB2 for struct NODELIST
 *
 * Revision 1.210  1999/01/27 16:01:52  dkr
 * comment about NWITH_PRAGMA changed
 *
 * Revision 1.209  1999/01/19 20:31:40  srs
 * inserted NWITH_REFERENCES_FOLDED
 *
 * Revision 1.208  1999/01/18 10:05:31  cg
 * added access macros for the usage of N_info in readsib.c
 *
 * Revision 1.207  1999/01/15 15:18:55  cg
 * added access macros for new data structure access_t used in tile size
 * selection.
 *
 * Revision 1.206  1999/01/07 14:02:33  sbs
 * new tab opt_tab inserted and old "opt_tab" renamed to genmask_tab!
 *
 * Revision 1.205  1999/01/06 13:03:33  cg
 * extern declaration of prf_name_str moved from tree.h to tree_basic.h
 *
 * Revision 1.204  1998/12/09 08:52:29  sbs
 * INFO_CF_VARNO added
 *
 * Revision 1.203  1998/11/08 13:25:33  dkr
 * remark about NCODE_CBLOCK corrected
 *
 * Revision 1.202  1998/08/13 22:20:23  dkr
 * WLNODE_INNERSTEP added
 *
 * Revision 1.201  1998/08/11 14:32:56  dkr
 * some WL... stuff changed
 *
 * Revision 1.200  1998/08/11 10:59:57  dkr
 * WLSEG_INNERSTEP changed
 *
 * Revision 1.199  1998/08/11 00:04:53  dkr
 * N_WLsegVar changed
 *
 * Revision 1.198  1998/08/10 18:04:16  dkr
 * changed a comment for N_Nwithop
 *
 * Revision 1.197  1998/08/07 14:35:24  dkr
 * stuff for N_WLsegVar added
 *
 * Revision 1.196  1998/07/16 15:55:55  srs
 * added INFO_UNR_FUNDEF
 *
 * Revision 1.195  1998/07/16 15:27:02  dkr
 * WL..._INNERSTEP, WLSEG_MAXHOMDIM, WLSEGVAR_MAXHOMDIM inserted
 *
 * Revision 1.194  1998/07/14 12:58:47  srs
 * added remarks to N_id, N_vardec and N_arg
 *
 * Revision 1.193  1998/07/03 10:16:03  cg
 * attributes of N_spmd node completely changed.
 *
 * Revision 1.192  1998/06/23 12:42:13  cg
 * Attribute SPMD_LIFTED_FROM removed
 * Attribute SPMD_FUNNAME replaced by SPMD_FUNDEF
 * Attribute NWITH2_MT added to specify those with-loops which
 * are to executed in parallel.
 *
 * Revision 1.191  1998/06/19 19:36:58  dkr
 * added INFO_REUSE_NEGMASK
 *
 * Revision 1.190  1998/06/12 14:05:23  cg
 * added access macros/attributes for schedulings
 *
 * Revision 1.189  1998/06/09 16:46:09  dkr
 * IDX_MIN, IDX_MAX now segment-specific
 *
 * Revision 1.188  1998/06/08 13:48:06  dkr
 * added INFO_REUSE_DEC_RC_IDS
 *
 * Revision 1.187  1998/06/07 18:38:12  dkr
 * added INFO_REUSE_... macros
 *
 * Revision 1.186  1998/06/06 18:30:35  dkr
 * added SYNC_WITH_PTRS
 *
 * Revision 1.185  1998/06/04 16:58:52  cg
 *  information about refcounted variables in the context of loops,
 * conditionals and the old with-loop are now stored in ids-chains
 *  instead of N_exprs lists.
 *
 * Revision 1.183  1998/06/03 14:29:04  cg
 * Attribute WITH_USEDVARS renamed to WITH_USEVARS analogously to the
 * loop nodes.
 * Attributes WLGRIDVAR_BOUND[12] made sons
 * Attributes WLSTRIVAR_BOUND[12] and WLSTRIVAR_STEP made sons
 *
 * Revision 1.182  1998/05/28 16:32:02  dkr
 * added ICM_INDENT: indent-mechanismus for H-ICMs
 *
 * Revision 1.181  1998/05/28 07:49:45  cg
 * added temporary attribute FUNDEF_LIFTEDFROM for spmd functions
 *
 * Revision 1.180  1998/05/27 13:16:38  sbs
 * INFO_TC_LHSVARS added
 *
 * Revision 1.179  1998/05/24 00:40:00  dkr
 * removed WLGRID_CODE_TEMPLATE
 *
 * Revision 1.178  1998/05/21 13:29:42  dkrnode *PRECWith(node *arg_node, node *arg_info)
 * renamed NCODE_DEC_RC_IDS into NCODE_INC_RC_IDS
 *
 * Revision 1.177  1998/05/21 10:14:42  dkr
 * changed some comments
 *
 * Revision 1.176  1998/05/17 00:08:15  dkr
 * WLGRID_CEXPR_TEMPLATE is now WLGRID_CODE_TEMPLATE
 *
 *
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
 ***    types*             NEXT      (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*              TDEF      (O)  (typecheck -> )
 ***/

/*
 *  TDEF is a reference to the defining N_typedef node of a user-defined
 *  type (not yet implemented).
 */

extern types *MakeType (simpletype basetype, int dim, shpseg *shpseg, char *name,
                        char *mod);

#define TYPES_BASETYPE(t) (t->simpletype)
#define TYPES_DIM(t) (t->dim)
#define TYPES_SHPSEG(t) (t->shpseg)
#define TYPES_NAME(t) (t->name)
#define TYPES_MOD(t) (t->name_mod)
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
 ***    int         REFCNT       (refcount -> )
 ***    node*       VARDEC       (typecheck -> )
 ***    node*       DEF          (psi-optimize -> )
 ***    node*       USE          (psi-optimize -> )
 ***    statustype  STATUS       (obj-handling -> compile !!)
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

#define IDS_NAME(i) (i->id)
#define IDS_MOD(i) (i->mod)
#define IDS_REFCNT(i) (i->refcnt)
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
                             access_t *next);

#define ACCESS_ARRAY(a) (a->array_vardec)
#define ACCESS_IV(a) (a->iv_vardec)
#define ACCESS_CLASS(a) (a->accessclass)
#define ACCESS_OFFSET(a) (a->offset)
#define ACCESS_NEXT(a) (a->next)

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
#define MODUL_CLASSTYPE(n) ((types *)(n->node[5]))

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
#define TYPEDEF_PRAGMA(n) (n->node[2])
#define TYPEDEF_COPYFUN(n) ((char *)(n->node[3]))
#define TYPEDEF_FREEFUN(n) ((char *)(n->node[4]))

#define TYPEDEC_DEF(n) (n->node[1])

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
#define OBJDEF_EXPR(n) (n->node[1])
#define OBJDEF_NEXT(n) (n->node[0])
#define OBJDEF_STATUS(n) (n->info.types->status)
#define OBJDEF_ATTRIB(n) (n->info.types->attrib)
#define OBJDEF_VARNAME(n) ((char *)(n->info2))
#define OBJDEF_ARG(n) (n->node[3])
#define OBJDEF_PRAGMA(n) (n->node[4])
#define OBJDEF_ICM(n) (n->node[3])
#define OBJDEF_NEEDOBJS(n) ((nodelist *)(n->node[5]))
#define OBJDEF_SIB(n) (n->node[3])

#define OBJDEC_DEF(n) (n->node[2])

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
 ***    statustype      ATTRIB
 ***    int             INLINE
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
 ***    DFMmask_base_t  DFM_BASE                  (refcount -> spmd -> )
 ***                                              ( -> compile -> )
 ***
 ***    node*           FUNDEC_DEF (O) (N_fundef) (checkdec -> writesib !!)
 ***    node*           LIFTEDFROM (O) (N_fundef) (liftspmd -> compile -> )
 ***
 ***/

/*
 *  STATUS: ST_regular      function defined in this module
 *          ST_objinitfun   generic function for object initialization
 *          ST_imported     imported function (maybe declaration only)
 *          ST_generic      class conversion function
 *          ST_spmdfun      function containing lifted SPMD-region
 *
 *  ATTRIB: ST_regular      dimension-dependent or non-array function
 *          ST_independent  dimension-independent array function
 *          ST_generic      generic function derived from dimension-
 *                          independent array function
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
 *  LIFTEDFROM is a link back to the original function definition in the
 *  case of an SPMD function. Since SPMD functions are generated relatively
 *  late during the entire compilation process when the NEEDOBJS attribute
 *  is no longer required, LIFTEDFROM and NEEDOBJS may share the same node entry.
 */

extern node *MakeFundef (char *name, char *mod, types *types, node *args, node *body,
                         node *next);

#define FUNDEF_FUNNO(n) (n->counter)
#define FUNDEF_NAME(n) (n->info.types->id)
#define FUNDEF_MOD(n) (n->info.types->id_mod)
#define FUNDEF_LINKMOD(n) (n->info.types->id_cmod)
#define FUNDEF_PRAGMA(n) (n->node[5])
#define FUNDEF_TYPES(n) (n->info.types)
#define FUNDEF_BODY(n) (n->node[0])
#define FUNDEF_ARGS(n) (n->node[2])
#define FUNDEF_NEXT(n) (n->node[1])
#define FUNDEF_RETURN(n) (n->node[3])
#define FUNDEF_SIB(n) (n->node[3])
#define FUNDEF_NEEDOBJS(n) ((nodelist *)(n->node[4]))
#define FUNDEF_ICM(n) (n->node[3])
#define FUNDEF_VARNO(n) (n->varno)
#define FUNDEF_MASK(n, x) (n->mask[x])
#define FUNDEF_STATUS(n) (n->info.types->status)
#define FUNDEF_ATTRIB(n) (n->info.types->attrib)
#define FUNDEF_INLINE(n) (n->flag)
#define FUNDEF_INLREC(n) (n->refcnt)
#define FUNDEF_DFM_BASE(n) (n->dfmask[0])
#define FUNDEF_LIFTEDFROM(n) (n->node[4])

#define FUNDEC_DEF(n) (n->node[3])

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
#define ARG_NEXT(n) (n->node[0])
#define ARG_VARNO(n) (n->varno)
#define ARG_REFCNT(n) (n->refcnt)
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
 ***  temporary attributes:
 ***
 ***    nodelist*  NEEDFUNS   (O)         (analysis -> )
 ***                                      ( -> analysis -> )
 ***                                      ( -> write-SIB -> )
 ***    nodelist*  NEEDTYPES  (O)         (analysis -> )
 ***                                      ( -> write-SIB -> )
 ***    long*      MASK[x]                (optimize -> )
 ***    int        VARNO                  (optimize -> )
 ***
 ***    node*      SPMD_PROLOG_ICMS (O)   (N_fundef)  (compile !!)
 ***/

/*
 * In spmd-functions SPMD_PROLOG_ICMS points to an assign-chain that
 * contains the prolog memory management ICMs derived from the first
 * synchronisation block. These are required for the compilation of
 * the corresponding spmd-block.
 */

extern node *MakeBlock (node *instr, node *vardec);

#define BLOCK_INSTR(n) (n->node[0])
#define BLOCK_VARDEC(n) (n->node[1])
#define BLOCK_MASK(n, x) (n->mask[x])
#define BLOCK_NEEDFUNS(n) ((nodelist *)(n->node[2]))
#define BLOCK_NEEDTYPES(n) ((nodelist *)(n->node[3]))
#define BLOCK_VARNO(n) (n->varno)
#define BLOCK_SPMD_PROLOG_ICMS(n) (n->node[4])

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
 ***    int         VARNO                      (optimize -> )
 ***    statustype  ATTRIB                     (typecheck -> uniquecheck -> )
 ***    int         FLAG                       (ael  -> dcr2 !! )
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
#define VARDEC_NEXT(n) (n->node[0])
#define VARDEC_VARNO(n) (n->varno)
#define VARDEC_REFCNT(n) (n->refcnt)
#define VARDEC_STATUS(n) (n->info.types->status)
#define VARDEC_ATTRIB(n) (n->info.types->attrib)
#define VARDEC_TYPEDEF(n) (n->node[1])
#define VARDEC_ACTCHN(n) (n->node[2])
#define VARDEC_COLCHN(n) (n->node[3])
#define VARDEC_FLAG(n) (n->flag)
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
 ***    void*  INDEX    (O)               (wli -> wlf ->)
 ***    int    LEVEL                      (wli !!)
 ***
 ***  remarks:
 ***   there is no easy way to remove the INDEX information after wlf (another
 ***   tree traversal would be necessary), so it stays afterwards.
 ***   Nevertheless only wlf will use it. The type of INDEX is index_info*,
 ***   defined in WithloopFolding.c (not in types.h).
 ***/

extern node *MakeAssign (node *instr, node *next);

#define ASSIGN_INSTR(n) (n->node[0])
#define ASSIGN_NEXT(n) (n->node[1])
#define ASSIGN_CSE(n) (n->node[2])
#define ASSIGN_MASK(n, x) (n->mask[x])
#define ASSIGN_STATUS(n) (n->flag)
#define ASSIGN_INDEX(n) (n->info2)
#define ASSIGN_LEVEL(n) (n->info.cint)

/*--------------------------------------------------------------------------*/

/***
 ***  N_let :					( one of "N_instr" )
 ***
 ***  sons:
 ***
 ***    node*  EXPR      ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    ids*   IDS   (O)
 ***
 ***/

extern node *MakeLet (node *expr, ids *ids);

#define LET_EXPR(n) (n->node[0])
#define LET_IDS(n) (n->info.ids)

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
 ***    node*  EXPRS      (N_exprs)  (O)
 ***
 ***
 ***  permanent attributes:
 ***
 ***    int    INWITH
 ***
 ***  temporary attributes:
 ***
 ***    node*  REFERENCE  (N_exprs)  (O)  (precompile -> compile !!)
 ***/

/*
 *  REFERENCE: List of artificial return values which correspond to
 *             reference parameters.
 *
 *  INWITH is used to mark those return statements which are used in the
 *  internal representation of with loops.
 *
 *  ATTENTION: node[1] of N_return node already used by compile.c
 */

extern node *MakeReturn (node *exprs);

#define RETURN_EXPRS(n) (n->node[0])
#define RETURN_REFERENCE(n) (n->node[2])
#define RETURN_INWITH(n) (n->varno)

/*--------------------------------------------------------------------------*/

/***
 ***  N_cond :
 ***
 ***  sons:
 ***
 ***    node*  COND         ("N_expr")
 ***    node*  THEN         (N_block)
 ***    node*  ELSE         (N_block)
 ***
 ***  temporary attributes:
 ***
 ***    ids*   THENVARS                 (refcount -> compile -> )
 ***    ids*   ELSEVARS                 (refcount -> compile -> )
 ***    long*  MASK[x]                  (optimize -> )
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
#define COND_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_do :
 ***
 ***  sons:
 ***
 ***    node*  COND       ("N_expr")
 ***    node*  BODY       (N_block)
 ***
 ***  temporary attributes:
 ***
 ***    ids*   USEVARS                (refcount -> compile -> )
 ***    ids*   DEFVARS                (refcount -> compile -> )
 ***    long*  MASK[x]                (optimize -> )
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

#define DO_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_while :
 ***
 ***  sons:
 ***
 ***    node*  COND       ("N_expr")
 ***    node*  BODY       (N_block)
 ***
 ***  temporary attributes:
 ***
 ***    ids*   USEVARS                (refcount -> compile -> )
 ***    ids*   DEFVARS                (refcount -> compile -> )
 ***    long*  MASK[x]                (optimize -> )
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

#define WHILE_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_annotate :                              ( one of "N_instr" )
 ***
 ***  permanent attributes:
 ***
 ***    int    TAG
 ***	int    FUNNUMBER
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
 ***  N_with :
 ***
 ***  sons:
 ***
 ***    node*  GEN      (N_generator)
 ***    node*  OPERATOR (N_modarray, N_genarray, N_foldprf, N_foldfun)
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK[x]                 (optimize -> )
 ***    ids*   USEVARS                 (refcount -> compile ->)
 ***
 ***  remark (srs): the 'body' of MakeWith() can be reached by WITH_OPERATOR.
 ***
 ***/

/*
 * The temporary attribute USEVARS is a chain of identifiers
 * along with refcounting information which is generated by refcount.c and
 * used by compile.c for generating refcounting instructions in the context
 * of a with-loop.
 */

extern node *MakeWith (node *gen, node *body);

#define WITH_GEN(n) (n->node[0])
#define WITH_OPERATOR(n) (n->node[1])
#define WITH_MASK(n, x) (n->mask[x])
#define WITH_USEVARS(n) ((ids *)n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_generator :
 ***
 ***  sons:
 ***
 ***    node*  LEFT    ("N_expr")
 ***    node*  RIGHT   ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    char*  ID
 ***    ids*   IDS
 ***
 ***  temporary attributes:
 ***
 ***    node*  VARDEC                  (typechecker -> )
 ***    long*  MASK[x]                 (optimize -> )
 ***    node*  USE     (O) (N_vinfo)   (psi-optimize -> )
 ***
 ***  remark: IDS->id == ID
 ***
 ***/

extern node *MakeGenerator (node *left, node *right, char *id);

#define GEN_LEFT(n) (n->node[0])
#define GEN_RIGHT(n) (n->node[1])
#define GEN_ID(n) (n->info.ids->id)
#define GEN_IDS(n) (n->info.ids)
#define GEN_USE(n) (n->info.ids->use)
#define GEN_VARDEC(n) (n->info.ids->node)
#define GEN_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_genarray :
 ***
 ***  sons:
 ***
 ***    node*  ARRAY  (N_array)
 ***    node*  BODY   (N_block)
 ***
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK[x]                 (optimize -> )
 ***/

extern node *MakeGenarray (node *array, node *body);

#define GENARRAY_ARRAY(n) (n->node[0])
#define GENARRAY_BODY(n) (n->node[1])
#define OPERATOR_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_modarray :
 ***
 ***  sons:
 ***
 ***    node*  ARRAY  (N_id)
 ***    node*  BODY   (N_block)
 ***
 ***
 ***  permanent attributes:
 ***
 ***    char*  ID
 ***
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK[x]                 (optimize -> )  (see N_genarray)
 ***/

extern node *MakeModarray (node *array, node *body);

#define MODARRAY_ARRAY(n) (n->node[0])
#define MODARRAY_BODY(n) (n->node[1])
#define MODARRAY_ID(n) (n->info.id)

/*--------------------------------------------------------------------------*/

/***
 ***  N_foldprf :
 ***
 ***  sons:
 ***
 ***    node*  BODY         (N_block)
 ***    node*  NEUTRAL  (O) ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    prf    PRF
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK[x]                 (optimize -> )  (see N_genarray)
 ***/

extern node *MakeFoldprf (prf prf, node *body, node *neutral);

#define FOLDPRF_PRF(n) (n->info.prf)
#define FOLDPRF_BODY(n) (n->node[0])
#define FOLDPRF_NEUTRAL(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_foldfun :
 ***
 ***  sons:
 ***
 ***    node*  BODY          (N_block)
 ***    node*  NEUTRAL       ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    char*  MOD                (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*  FUNDEF        (N_fundef)  (typecheck -> )
 ***    long*  MASK[x]                   (optimize -> )  (see N_genarray)
 ***/

extern node *MakeFoldfun (char *name, char *mod, node *body, node *neutral);

#define FOLDFUN_NAME(n) (n->info.fun_name.id)
#define FOLDFUN_MOD(n) (n->info.fun_name.id_mod)
#define FOLDFUN_BODY(n) (n->node[0])
#define FOLDFUN_NEUTRAL(n) (n->node[1])
#define FOLDFUN_FUNDEF(n) (n->node[2])

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
 ***    int*       INTVEC       (O)   (flatten -> )
 ***    float*     FLOATVEC     (O)   (flatten -> )
 ***    double*    DOUBLEVEC    (O)   (flatten -> )
 ***    char*      CHARVEC      (O)   (flatten -> )
 ***    int        VECLEN       (O)   (flatten -> )
 ***    simpletype VECTYPE      (O)   (flatten -> )
 ***/

/*
 * In the case of constant character arrays defined as strings, the
 * optional permanent attribute STRING holds the original definition.
 * This may be retrieved for C code generation.
 *
 * In the case of constant arrays, the optional permanent
 * attribute INTVEC holds the original definition. In that case
 * VECLEN holds the number of array elements.
 * This may be retrieved for tiling.
 * For the reasen of consistancy there are also compact propagations
 * float-, double-, boolean- and character-arrays.
 * Boolean-arrays will be stored in INTVEC too.
 */

extern node *MakeArray (node *aelems);

#define ARRAY_AELEMS(n) (n->node[0])
#define ARRAY_TYPE(n) (n->info.types)
#define ARRAY_STRING(n) ((char *)(n->node[1]))
#define ARRAY_INTVEC(n) ((int *)(n->node[2]))
#define ARRAY_FLOATVEC(n) ((float *)(n->node[3]))
#define ARRAY_DOUBLEVEC(n) ((double *)(n->node[4]))
#define ARRAY_CHARVEC(n) ((char *)(n->node[5]))
#define ARRAY_VECLEN(n) (n->counter)
#define ARRAY_VECTYPE(n) ((simpletype)n->varno)

/*--------------------------------------------------------------------------*/

/***
 ***  N_vinfo :
 ***
 ***  sons:
 ***
 ***    node*    NEXT  (O)  (N_vinfo)
 ***
 ***  permanent attributes:
 ***
 ***    useflag  FLAG
 ***    types*   TYPE   (O)
 ***    node*    VARDEC (O)  (N_vardec)
 ***
 ***/

extern node *MakeVinfo (useflag flag, types *type, node *next);

#define VINFO_FLAG(n) (n->info.use)
#define VINFO_TYPE(n) ((types *)(n->node[1]))
#define VINFO_NEXT(n) (n->node[0])
#define VINFO_VARDEC(n) (n->node[2])

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
 ***    int         MAKEUNIQUE                  (precompile -> compile -> )
 ***    node*       DEF                         (Unroll !, Unswitch !)
 ***    node*       WL          (O)             (wli -> wlf !!)
 ***    node*       INDEX       (O)             (tile size inference -> )
 ***    node*       INTVEC      (O) (N_array)   (flatten -> )
 ***    int         VECLEN      (O) (N_array)   (flatten -> )
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
 ***    INDEX is a flag used for the status of a flattened variable. it is set to 0,
 ***    if the variable is not touched by the index vector of a with loop. it is set
 ***    to 1, if it is the index vector and it is set to 2 if it is the index vector
 ***    added with a constant integer vector.
 ***
 ***  remark:
 ***    INTVEC, VECTYPE and VECLEN now are used for propagation of constant
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

extern node *MakeId2 (ids *ids_node);

#define ID_IDS(n) (n->info.ids)
#define ID_NAME(n) (n->info.ids->id)
#define ID_DEF(n) (n->info.ids->def)
#define ID_VARDEC(n) (n->info.ids->node)
#define ID_OBJDEF(n) (n->info.ids->node)
#define ID_MOD(n) (n->info.ids->mod)
#define ID_ATTRIB(n) (n->info.ids->attrib)
#define ID_STATUS(n) (n->info.ids->status)
#define ID_REFCNT(n) (n->refcnt)
#define ID_MAKEUNIQUE(n) (n->flag)
#define ID_WL(n) (n->node[0])
#define ID_INDEX(n) (n->varno)
#define ID_INTVEC(n) ((int *)(n->node[1]))
#define ID_VECLEN(n) (n->counter)

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
 ***/

extern node *MakePost (int incdec, char *id);

#define POST_INCDEC(n) ((n->node[0]->nodetype == N_dec) ? 0 : 1)
#define POST_ID(n) (n->info.id)
#define POST_DECL(n) (n->node[1])
#define POST_REFCNT(n) (n->info.ids->refcnt)

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
 ***/

extern node *MakePre (nodetype incdec, char *id);

#define PRE_INCDEC(n) ((n->node[0]->nodetype == N_dec) ? 0 : 1)
#define PRE_ID(n) (n->info.id)
#define PRE_DECL(n) (n->node[1])
#define PRE_REFCNT(n) (n->info.ids->refcnt)

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
 ***    int    INDENT
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
#define ICM_ARGS(n) (n->node[0])
#define ICM_NEXT(n) (n->node[1])
#define ICM_INDENT(n) (n->flag)

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
 ***
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
 */

extern node *MakePragma ();

#define PRAGMA_LINKNAME(n) (n->info.id)
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
#define PRAGMA_INITFUN(n) ((char *)(n->node[3]))
#define PRAGMA_LINKMOD(n) ((char *)(n->node[2]))
#define PRAGMA_NEEDTYPES(n) ((ids *)(n->node[1]))
#define PRAGMA_NEEDFUNS(n) (n->node[0])
#define PRAGMA_NUMPARAMS(n) (n->flag)

#define PRAGMA_WLCOMP_APS(n) (n->node[0])

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
 ***
 ***  when used in readsib.c :
 ***
 ***    node *     FOLDFUNS       (O)  (N_fundef)
 ***    node *     MODUL          (O)  (N_modul)
 ***
 ***  when used in typecheck.c :
 ***
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
 ***    node*      FUNDEFS       (0)  (N_fundef)
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
 ***  when used in managing spmd- and sync blocks :
 ***
 ***    node*      INFO_SPMD_FUNDEF   (N_fundef)
 ***    int        INFO_SPMD_FIRST
 ***    int        INFO_SPMD_MT
 ***
 ***  when used in tile_size_inference.c :
 ***
 ***    access_t*  ACCESS
 ***    node*      INDEXVAR           (N_vardec/N_arg)
 ***    feature_t  FEATURE
 ***    WithOpType WOTYPE
 ***    ids*       LASTLETIDS
 ***    int        BELOWAP
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
 */
#define INFO_DUP_CONT(n) (n->node[1])
#define INFO_DUP_FOLDINL(n) (n->node[3])

/* flatten */
#define INFO_FLTN_CONTEXT(n) (n->flag)
#define INFO_FLTN_LASTASSIGN(n) (n->node[0])
#define INFO_FLTN_LASTWLBLOCK(n) (n->node[1])
#define INFO_FLTN_FINALASSIGN(n) (n->node[2])
#define INFO_FLTN_CONSTVEC(n) (n->node[3])
#define INFO_FLTN_VECLEN(n) (n->counter)
#define INFO_FLTN_VECTYPE(n) ((simpletype)n->varno)

/* readsib */
#define INFO_RSIB_FOLDFUNS(n) (n->node[0])
#define INFO_RSIB_MODUL(n) (n->node[1])

/* typecheck */
#define INFO_TC_NEXTASSIGN(n) (n->node[1])
/* WARN: node[2] already used */
#define INFO_TC_CURRENTASSIGN(n) (n->node[4])
#define INFO_TC_LASSIGN(n) (n->node[3])
#define INFO_TC_LHSVARS(n) (n->info.ids)

/* writesib */
#define INFO_EXPORTTYPES(n) ((nodelist *)(n->node[0]))
#define INFO_EXPORTOBJS(n) ((nodelist *)(n->node[1]))
#define INFO_EXPORTFUNS(n) ((nodelist *)(n->node[2]))

/* refcount */
#define INFO_RC_PRF(n) (n->node[0])
#define INFO_RC_WITH(n) (n->node[1])
#define INFO_RC_RCDUMP(n) ((int *)(n->node[2]))

/* wltransform */
#define INFO_WL_SHPSEG(n) ((shpseg *)(n->node[0]))

/* spmdregions */
#define INFO_SPMD_FUNDEF(n) (n->node[0])
#define INFO_SPMD_FIRST(n) (n->flag)
#define INFO_SPMD_MT(n) (n->counter)

/* precompile */
#define INFO_PREC_MODUL(n) (n->node[0])
#define INFO_PREC_CNT_ARTIFICIAL(n) (n->lineno)

/* ArrayElemination */
#define INFO_AE_TYPES(n) (n->node[1])

/* compile */
#define INFO_COMP_LASTASSIGN(n) (n->node[0])
#define INFO_COMP_LASTLET(n) (n->node[1])
#define INFO_COMP_LASTIDS(n) (n->info.ids)
#define INFO_COMP_FUNDEF(n) (n->node[2])
#define INFO_COMP_VARDECS(n) (n->node[3])
#define INFO_COMP_WITHBEGIN(n) (n->node[4])
#define INFO_COMP_MODUL(n) (n->node[5])
#define INFO_COMP_FIRSTASSIGN(n) (n->node[0])
#define INFO_COMP_CNTPARAM(n) (n->lineno)
#define INFO_COMP_ICMTAB(n) ((node **)(n->node[1]))
#define INFO_COMP_TYPETAB(n) ((types **)(n->info.types))

/* reuse */
#define INFO_REUSE_FUNDEF(n) (n->node[0])
#define INFO_REUSE_WL_IDS(n) (n->info.ids)
#define INFO_REUSE_IDX(n) ((ids *)(n->node[1]))
#define INFO_REUSE_DEC_RC_IDS(n) ((ids *)(n->node[2]))
#define INFO_REUSE_MASK(n) ((DFMmask_t)n->dfmask[0])
#define INFO_REUSE_NEGMASK(n) ((DFMmask_t)n->dfmask[1])

/* optimize */
#define INFO_MASK(n, x) (n->mask[x])

/* inline */
/* ATTENTION: Usage of DUP and INL macros on arg_info are mixed. Be careful
   to avoid overlapping addresses. */
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
#define INFO_CF_VARNO(n) (n->varno)

/* DCR */
#define INFO_DCR_VARNO(n) (n->varno)
#define INFO_DCR_TRAVTYPE(n) (n->flag)
#define INFO_DCR_ACT(n) (n->mask[2])
#define INFO_DCR_NEWACT(n) (n->lineno)

/* Unrolling */
#define INFO_UNR_ASSIGN(n) (n->node[0])
#define INFO_UNR_FUNDEF(n) (n->node[1])

/* Icm2c, ... */
#define INFO_FUNDEF(n) (n->node[0])

/* Print */
#define INFO_PRINT_FUNDEF(n) (n->node[0])
#define INFO_PRINT_INT_SYN(n) (n->node[2])
#define INFO_PRINT_WITH_RET(n) (n->node[3])
#define INFO_PRINT_NWITH2(n) (n->node[4])

/* Tile Size Inference */
#define INFO_TSI_ACCESS(n) ((access_t *)n->info2)
#define INFO_TSI_INDEXVAR(n) (n->node[0])
#define INFO_TSI_FEATURE(n) ((feature_t)n->lineno)
#define INFO_TSI_WOTYPE(n) ((WithOpType)n->varno)
#define INFO_TSI_LASTLETIDS(n) (n->info.ids)
#define INFO_TSI_BELOWAP(n) (n->flag)
#define INFO_TSI_WLLEVEL(n) ((shpseg *)n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_spmd :
 ***
 ***  sons:
 ***
 ***    node*      REGION      (N_block)
 ***
 ***  permanent attributes:
 ***
 ***    DFMmask_t  IN
 ***    DFMmask_t  OUT
 ***    DFMmask_t  INOUT
 ***    DFMmask_t  LOCAL
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
 ***
 ***/

extern node *MakeSpmd (node *region);

#define SPMD_REGION(n) (n->node[0])

#define SPMD_IN(n) ((DFMmask_t)n->dfmask[0])
#define SPMD_INOUT(n) ((DFMmask_t)n->dfmask[1])
#define SPMD_OUT(n) ((DFMmask_t)n->dfmask[2])
#define SPMD_LOCAL(n) ((DFMmask_t)n->dfmask[3])

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
 ***
 ***  remarks:
 ***
 ***    'WITH_PTRS' is a N_exprs-chain containing pointers to all with-loop
 ***    assignments of this sync-region.
 ***
 ***/

extern node *MakeSync (node *region, int first);

#define SYNC_REGION(n) (n->node[0])
#define SYNC_FIRST(n) (n->flag)

#define SYNC_IN(n) ((DFMmask_t)n->dfmask[0])
#define SYNC_INOUT(n) ((DFMmask_t)n->dfmask[1])
#define SYNC_OUT(n) ((DFMmask_t)n->dfmask[2])
#define SYNC_LOCAL(n) ((DFMmask_t)n->dfmask[3])

#define SYNC_WITH_PTRS(n) (n->node[1])
#define SYNC_SCHEDULING(n) ((SCHsched_t)n->node[2])

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
 ***    int        COMPLEX                (wlt -> wlf !!)
 ***    int        FOLDABLE               (wlt -> wlf !!)
 ***    int        NO_CHANCE              (wlt -> wlf !!)
 ***    ids*       DEC_RC_IDS             (refcount -> wltransform )
 ***
 ***    DFMmask_t  IN                     (refcount -> wltransform )
 ***    DFMmask_t  INOUT                  (refcount -> wltransform )
 ***    DFMmask_t  OUT                    (refcount -> wltransform )
 ***    DFMmask_t  LOCAL                  (refcount -> wltransform )
 ***/

extern node *MakeNWith (node *part, node *code, node *withop);

#define NWITH_PART(n) (n->node[0])
#define NWITH_CODE(n) (n->node[1])
#define NWITH_WITHOP(n) (n->node[2])
#define NWITH_PRAGMA(n) (n->node[3])

#define NWITH_PARTS(n) (((wl_info *)(n->info2))->parts)
#define NWITH_REFERENCED(n) (((wl_info *)(n->info2))->referenced)
#define NWITH_REFERENCED_FOLD(n) (((wl_info *)(n->info2))->referenced_fold)
#define NWITH_REFERENCES_FOLDED(n) (((wl_info *)(n->info2))->references_folded)
#define NWITH_COMPLEX(n) (((wl_info *)(n->info2))->complex)
#define NWITH_FOLDABLE(n) (((wl_info *)(n->info2))->foldable)
#define NWITH_NO_CHANCE(n) (((wl_info *)(n->info2))->no_chance)
#define NWITH_DEC_RC_IDS(n) ((ids *)(n->node[4]))

#define NWITH_IN(n) ((DFMmask_t)n->dfmask[0])
#define NWITH_INOUT(n) ((DFMmask_t)n->dfmask[1])
#define NWITH_OUT(n) ((DFMmask_t)n->dfmask[2])
#define NWITH_LOCAL(n) ((DFMmask_t)n->dfmask[3])

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
 ***    int    COPY          (Unroll!)
 ***
 ***
 ***/

extern node *MakeNPart (node *withid, node *generator, node *code);

#define NPART_WITHID(n) (n->node[0])
#define NPART_GEN(n) (n->node[1])
#define NPART_NEXT(n) (n->node[2])
#define NPART_CODE(n) (n->node[3])
#define NPART_MASK(n, x) (n->mask[x])
#define NPART_COPY(n) (n->flag)

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

#define NWITHID_VEC(n) (n->info.ids)
#define NWITHID_IDS(n) ((ids *)(n->info2))

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
 ***
 ***  remarks:
 ***    the BOUNDs are NULL if upper or lower bounds are not specified.
 ***    if STEP is NULL, step 1 is assumed (no grid)
 ***    if WIDTH is NULL, width 1 is assumed
 ***
 ***/

extern node *MakeNGenerator (node *bound1, node *bound2, prf op1, prf op2, node *step,
                             node *width);

#define NGEN_BOUND1(n) (n->node[0])
#define NGEN_BOUND2(n) (n->node[1])
#define NGEN_STEP(n) (n->node[2])
#define NGEN_WIDTH(n) (n->node[3])
#define NGEN_OP1(n) (n->info.genrel.op1)
#define NGEN_OP2(n) (n->info.genrel.op2)

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
 ***
 ***/

extern node *MakeNWithOp (WithOpType WithOp);

#define NWITHOP_TYPE(n) (*((WithOpType *)(n)->info2))
#define NWITHOP_SHAPE(n) (n->node[0])
#define NWITHOP_ARRAY(n) (n->node[0])
#define NWITHOP_NEUTRAL(n) (n->node[0])
#define NWITHOP_EXPR(n) (n->node[1])
#define NWITHOP_FUN(n) (n->info.fun_name.id)
#define NWITHOP_MOD(n) (n->info.fun_name.id_mod)
#define NWITHOP_PRF(n) (n->info.prf)
#define NWITHOP_FUNDEF(n) (n->node[2])
#define NWITHOP_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Ncode :
 ***
 ***  sons:
 ***
 ***    node*     CBLOCK    (O) (N_block)
 ***    node*     CEXPR         ("N_expr")
 ***    node*     NEXT      (O) (N_exprs)
 ***
 ***  permanent attributes:
 ***
 ***    int       USED       (number of times this code is used)
 ***
 ***  temporary attributes:
 ***
 ***    int       NO         (unambiguous number for PrintNwith2())
 ***                                      (precompile -> )
 ***    long*     MASK                    (optimize -> )
 ***    int       FLAG                    (WLI -> WLF)
 ***    node*     COPY                    ( -> DupTree )
 ***    ids*      INC_RC_IDS              (refcount -> compile )
 ***    feature_t FEATURE                 (tsi -> )
 ***    access_t* ACCESS                  (tsi -> )
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
 ***    ACCESS is a list of array accesses.
 ***    Both FEATURE and ACCESS are used for the tile size inference scheme.
 ***/

extern node *MakeNCode (node *block, node *expr);

#define NCODE_CBLOCK(n) (n->node[0])
#define NCODE_CEXPR(n) (n->node[1])
#define NCODE_NEXT(n) (n->node[2])
#define NCODE_USED(n) (n->info.cint)

#define NCODE_MASK(n, x) (n->mask[x])
#define NCODE_NO(n) (n->refcnt)
#define NCODE_FLAG(n) (n->flag)
#define NCODE_INC_RC_IDS(n) ((ids *)(n->node[3]))
#define NCODE_FEATURE(n) ((feature_t)n->varno)
#define NCODE_ACCESS(n) ((access_t *)n->info2)

#define NCODE_COPY(n) (n->node[4])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith2 :
 ***
 ***  sons:
 ***
 ***    node*      WITHID        (N_Nwithid)
 ***    node*      SEGS          (N_WLseg)
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
 ***    int        MT
 ***
 ***
 ***  temporary attributes:
 ***
 ***    ids*       DEC_RC_IDS         (wltransform -> compile )
 ***
 ***    DFMmask_t  REUSE              (compile ! )
 ***
 ***    SCHsched_t SCHEDULING   (O)   (wltransform -> compile )
 ***
 ***/

extern node *MakeNWith2 (node *withid, node *seg, node *code, node *withop, int dims);

#define NWITH2_WITHID(n) (n->node[0])
#define NWITH2_SEGS(n) (n->node[1])
#define NWITH2_CODE(n) (n->node[2])
#define NWITH2_WITHOP(n) (n->node[3])
#define NWITH2_DIMS(n) (n->flag)

#define NWITH2_IN(n) ((DFMmask_t)n->dfmask[0])
#define NWITH2_INOUT(n) ((DFMmask_t)n->dfmask[1])
#define NWITH2_OUT(n) ((DFMmask_t)n->dfmask[2])
#define NWITH2_LOCAL(n) ((DFMmask_t)n->dfmask[3])
#define NWITH2_MT(n) (n->counter)

#define NWITH2_DEC_RC_IDS(n) ((ids *)(n->node[5]))

#define NWITH2_REUSE(n) ((DFMmask_t)n->dfmask[4])

#define NWITH2_SCHEDULING(n) ((SCHsched_t)n->node[4])

/*--------------------------------------------------------------------------*/

/*
 * here are some macros for N_WL... nodes:
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

#define WLSEGX_DIMS(n) (n->refcnt)
#define WLSEGX_CONTENTS(n) (n->node[0])
#define WLSEGX_NEXT(n) (n->node[1])
#define WLSEGX_IDX_MIN(n) (*((int **)(&(n->node[2]))))
#define WLSEGX_IDX_MAX(n) (*((int **)(&(n->node[3]))))
#define WLSEGX_BLOCKS(n) (n->flag)
#define WLSEGX_BV(n, level) (n->mask[level + 2])
#define WLSEGX_UBV(n) (n->mask[1])
#define WLSEGX_SV(n) (n->mask[0])
#define WLSEGX_MAXHOMDIM(n) (n->varno)
#define WLSEGX_SCHEDULING(n) ((SCHsched_t *)n->info2)

#define WLNODE_LEVEL(n) (n->lineno)
#define WLNODE_DIM(n) (n->refcnt)
#define WLNODE_BOUND1(n) (n->flag)
#define WLNODE_BOUND2(n) (n->counter)
#define WLNODE_STEP(n) (n->varno)
#define WLNODE_NEXTDIM(n) (n->node[0])
#define WLNODE_NEXT(n) (n->node[1])
#define WLNODE_INNERSTEP(n) (n->info.prf_dec.tag)

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
 ***    int        BLOCKS    (number of blocking levels
 ***                           --- without unrolling-blocking)
 ***    long*      SV        (step vector)           (wltransform -> )
 ***    long*      BV        (blocking vector)       (wltransform -> compile )
 ***    long*      UBV       (unrolling-b. vector)   (wltransform -> compile )
 ***
 ***    SCHsched_t SCHEDULING  (O)                   (wltransform -> compile )
 ***    int        MAXHOMDIM (last homog. dimension) (wltransform -> compile )
 ***
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
#define WLSEG_MAXHOMDIM(n) (WLSEGX_MAXHOMDIM (n))

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
 ***    int      INNERSTEP                 (wltransform -> compile )
 ***
 ***  remarks:
 ***
 ***    - it makes no sense to use the nodes NEXTDIM and CONTENTS simultaneous!
 ***    - INNERSTEP is only valid, iff (LEVEL == 0).
 ***
 ***/

extern node *MakeWLblock (int level, int dim, int bound1, int bound2, int step,
                          node *nextdim, node *contents, node *next);

#define WLBLOCK_LEVEL(n) (WLNODE_LEVEL (n))
#define WLBLOCK_DIM(n) (WLNODE_DIM (n))
#define WLBLOCK_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLBLOCK_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLBLOCK_STEP(n) (WLNODE_STEP (n))
#define WLBLOCK_NEXTDIM(n) (WLNODE_NEXTDIM (n))
#define WLBLOCK_CONTENTS(n) (n->node[2])
#define WLBLOCK_NEXT(n) (WLNODE_NEXT (n))

#define WLBLOCK_INNERSTEP(n) (WLNODE_INNERSTEP (n))

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
 ***    int      INNERSTEP                 (wltransform -> compile )
 ***
 ***  remarks:
 ***
 ***    - it makes no sense to use the nodes NEXTDIM and CONTENTS simultaneous!
 ***    - INNERSTEP is only valid, iff (LEVEL == 0).
 ***
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

#define WLUBLOCK_INNERSTEP(n) (WLNODE_INNERSTEP (n))

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
 ***    int      UNROLLING    (unrolling wanted?)
 ***
 ***  temporary attributes:
 ***
 ***    node*    PART         (part this stride is generated from)
 ***                                          (wltransform ! )
 ***    node*    MODIFIED                     (wltransform ! )
 ***    int      INNERSTEP                    (wltransform -> compile )
 ***
 ***  remarks:
 ***
 ***    - INNERSTEP is only valid, iff (LEVEL == 0).
 ***
 ***/

extern node *MakeWLstride (int level, int dim, int bound1, int bound2, int step,
                           int unrolling, node *contents, node *next);

#define WLSTRIDE_LEVEL(n) (WLNODE_LEVEL (n))
#define WLSTRIDE_DIM(n) (WLNODE_DIM (n))
#define WLSTRIDE_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLSTRIDE_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLSTRIDE_STEP(n) (WLNODE_STEP (n))
#define WLSTRIDE_UNROLLING(n) (n->info.prf_dec.tc)
#define WLSTRIDE_CONTENTS(n) (n->node[0])
#define WLSTRIDE_NEXT(n) (WLNODE_NEXT (n))

#define WLSTRIDE_PART(n) (n->node[4])
#define WLSTRIDE_MODIFIED(n) (n->node[5])
#define WLSTRIDE_INNERSTEP(n) (WLNODE_INNERSTEP (n))

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
 ***    int     UNROLLING
 ***
 ***  temporary attributes:
 ***
 ***    node*   MODIFIED                      (wltransform ! )
 ***
 ***  remarks:
 ***
 ***    - it makes no sense to use the nodes NEXTDIM and CODE simultaneous!
 ***    - (NEXTDIM == NULL) *and* (CODE == NULL) means that this is a dummy grid,
 ***      representing a simple init- (genarray), copy- (modarray), noop- (fold)
 ***      operation.
 ***
 ***/

extern node *MakeWLgrid (int level, int dim, int bound1, int bound2, int unrolling,
                         node *nextdim, node *next, node *code);

#define WLGRID_LEVEL(n) (WLNODE_LEVEL (n))
#define WLGRID_DIM(n) (WLNODE_DIM (n))
#define WLGRID_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLGRID_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLGRID_UNROLLING(n) (n->info.prf_dec.tag)
#define WLGRID_NEXTDIM(n) (WLNODE_NEXTDIM (n))
#define WLGRID_NEXT(n) (WLNODE_NEXT (n))
#define WLGRID_CODE(n) (n->node[2])

#define WLGRID_MODIFIED(n) (n->node[4])

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
 ***    long*      SV        (step vector)           (wltransform -> )
 ***    long*      BV        (blocking vector)       (wltransform -> compile )
 ***    long*      UBV       (unrolling-b. vector)   (wltransform -> compile )
 ***
 ***    SCHsched_t SCHEDULING  (O)                   (wltransform -> compile )
 ***    int        MAXHOMDIM (last homog. dimension) (wltransform -> compile )
 ***
 ***  remarks:
 ***
 ***    This node is not yet implemented.
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
#define WLSEGVAR_MAXHOMDIM(n) (WLSEGX_MAXHOMDIM (n))

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
 ***    int      INNERSTEP                 (wltransform -> compile )
 ***
 ***  remarks:
 ***
 ***    - INNERSTEP is only valid, iff (LEVEL == 0).
 ***
 ***/

extern node *MakeWLstriVar (int level, int dim, node *bound1, node *bound2, node *step,
                            node *contents, node *next);

#define WLSTRIVAR_LEVEL(n) (WLNODE_LEVEL (n))
#define WLSTRIVAR_DIM(n) (WLNODE_DIM (n))
#define WLSTRIVAR_BOUND1(n) (n->node[2])
#define WLSTRIVAR_BOUND2(n) (n->node[3])
#define WLSTRIVAR_STEP(n) (n->node[4])
#define WLSTRIVAR_CONTENTS(n) (n->node[0])
#define WLSTRIVAR_NEXT(n) (WLNODE_NEXT (n))

#define WLSTRIVAR_INNERSTEP(n) (n->flag)

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
 ***
 ***/

extern node *MakeWLgridVar (int level, int dim, node *bound1, node *bound2, node *nextdim,
                            node *next, node *code);

#define WLGRIDVAR_LEVEL(n) (WLNODE_LEVEL (n))
#define WLGRIDVAR_DIM(n) (WLNODE_DIM (n))
#define WLGRIDVAR_BOUND1(n) (n->node[2])
#define WLGRIDVAR_BOUND2(n) (n->node[3])
#define WLGRIDVAR_NEXTDIM(n) (WLNODE_NEXTDIM (n))
#define WLGRIDVAR_NEXT(n) (WLNODE_NEXT (n))
#define WLGRIDVAR_CODE(n) (n->node[4])

#endif /* _sac_tree_basic_h */
