/*
 *
 * $Log$
 * Revision 3.119  2002/04/12 13:57:06  sbs
 * FUNDEF_TYPE added
 *
 * Revision 3.118  2002/04/09 08:05:58  ktr
 * Support for WithloopScalarization added
 *
 * Revision 3.117  2002/03/07 20:27:11  dkr
 * ICM_END_OF_STATEMENT removed
 *
 * Revision 3.116  2002/03/07 17:51:13  sbs
 * ST_wrapperfun as possible fundef status added
 *
 * Revision 3.115  2002/03/07 02:21:30  dkr
 * INFO_COMP_FIRSTASSIGN removed
 * INFO_PREC... macros renamed
 *
 * Revision 3.114  2002/03/06 03:45:34  dkr
 * RETURN_CRET modified
 *
 * Revision 3.113  2002/03/05 13:58:52  dkr
 * definition of ID_UNQCONV modified
 *
 * Revision 3.112  2002/03/01 02:34:51  dkr
 * - type ARGTAB added
 * - INFO_PREC3_... macros added
 *
 * Revision 3.111  2002/02/22 14:08:24  sbs
 * INFO_INSVD_VARDECS and INFO_INSVD_ARGS added.
 *
 * Revision 3.110  2002/02/22 13:57:03  dkr
 * some more casts modified (cc demands for it)
 *
 * Revision 3.109  2002/02/22 13:24:04  dkr
 * some casts modified (cc demands for it)
 *
 * Revision 3.108  2002/02/22 11:48:33  dkr
 * FUNDEF, TYPEDEF, OBJDEF, VARDEC, ARG:
 * node attributes NAME, MOD, ... are no longer mapped into the TYPES
 * structure
 *
 * Revision 3.107  2002/02/21 18:16:43  dkr
 * TYPEDEF_LINKMOD added
 *
 * Revision 3.106  2002/02/12 15:42:45  dkr
 * implementation of ARG_AVIS and VARDEC_AVIS modified
 *
 * Revision 3.105  2001/12/12 12:44:55  dkr
 * function MakeId_Copy_NT added
 *
 * Revision 3.104  2001/12/11 12:59:50  dkr
 * ID_NT_TAG added
 *
 * Revision 3.103  2001/07/19 16:19:57  cg
 * Added new status entries ST_imported_extmod and ST_imported_extclass.
 *
 * Revision 3.102  2001/07/17 15:12:12  cg
 * Some compound macros moved from tree_basic.h to tree_compound.h
 *
 * Revision 3.101  2001/07/16 08:23:11  cg
 * Added function MakeOk for construction of N_ok nodes.
 *
 * Revision 3.100  2001/06/14 12:32:36  dkr
 * some minor changes in definition of WL nodes done
 *
 * Revision 3.99  2001/06/13 13:05:15  ben
 * WLSEGX_TASKSEL added
 *
 * Revision 3.98  2001/05/30 14:04:43  nmw
 * some arg_info macros moved from SSALIR to SSALIL
 *
 * Revision 3.97  2001/05/18 07:57:54  nmw
 * INFO_CSE_macros added
 *
 * Revision 3.96  2001/05/14 10:21:20  cg
 * Removed attribute BLOCK_SCHEDULER_NUM.
 *
 * Revision 3.95  2001/05/10 15:00:17  cg
 * Added arg_info attributes used in compile.c :
 * SCHEDULER_INIT and SCHEDULER_NUM
 * Renamed SEGID into SCHEDULERID.
 *
 * Revision 3.94  2001/05/09 15:23:13  cg
 * Added two new atrributes to info node used in compile.c:
 * INFO_COMP_SEGID(n) and INFO_COMP_SCHEDULERINIT(n).
 *
 * Revision 3.93  2001/05/08 12:29:10  dkr
 * minor changes done
 *
 * Revision 3.92  2001/05/07 09:06:16  nmw
 * macros for shared usage of WLUnroll adjusted
 *
 * Revision 3.91  2001/05/03 17:32:16  dkr
 * WLSEG_MAXHOMDIM replaced by WLSEG_HOMSV
 *
 * Revision 3.90  2001/04/30 12:27:23  nmw
 * INFO_AE_FUNDEF, INFO_GNM_FUDNEF added
 *
 * Revision 3.89  2001/04/27 17:35:28  nmw
 * INFO_COMP_ASSIGN added
 *
 * Revision 3.88  2001/04/26 13:30:00  nmw
 * INFO_SSACF_INLFUNDEF added
 *
 * Revision 3.87  2001/04/26 11:54:40  nmw
 * ICM_FUNDEF attribute added
 *
 * Revision 3.86  2001/04/25 13:55:06  dkr
 * ST_zombiefun added
 *
 * Revision 3.85  2001/04/24 18:36:50  dkr
 * comment about FUNDEF_USED modified
 *
 * [...]
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
 ***    shpseg*            SHPSEG
 ***
 ***/

#define SHAPES_DIM(s) (s->dim)
#define SHAPES_SHPSEG(s) (s->shpseg)

/*--------------------------------------------------------------------------*/

/***
 ***  SHPSEG :
 ***
 ***  permanent attributes:
 ***
 ***    int[SHP_SEG_SIZE]  SHAPE
 ***    shpseg*            NEXT
 ***
 ***/

extern shpseg *MakeShpseg (nums *num);

#define SHPSEG_ELEMS(s) (s->shp)
#define SHPSEG_SHAPE(s, x) (SHPSEG_ELEMS (s)[x])
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
 * STATUS:
 *   ST_artificial : artificial return type due to the resolution of reference
 *                   parameters and global objects.
 *   ST_crettype   : return type of a function that is compiled to the actual
 *                   return type of the resulting C function.
 *   ST_regular    : otherwise
 *
 * TDEF is a reference to the defining N_typedef node of a user-defined type.
 */

extern types *MakeTypes1 (simpletype btype);

extern types *MakeTypes (simpletype btype, int dim, shpseg *shpseg, char *name,
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
 ***    char*       MOD
 ***    statustype  ATTRIB
 ***    ids*        NEXT
 ***
 ***  temporary attributes:
 ***
 ***    node*       AVIS      (N_avis)          (ssaform -> )
 ***    node*       VARDEC    (N_vardec/N_arg)  (typecheck -> )
 ***    node*       DEF                         (psi-optimize -> )
 ***    node*       USE                         (psi-optimize -> )
 ***    statustype  STATUS                      (obj-handling -> compile -> )
 ***    int         REFCNT                      (refcount -> compile -> )
 ***    int         NAIVE_REFCNT                (refcount -> )
 ***/

/*
 * STATUS:
 *   ST_regular       : from original source code
 *   ST_artificial    : LHS of a function application which belongs to a
 *                      eliminated reference parameter
 *
 * ATTRIB:
 *   ST_regular       : local variable or function parameter
 *   ST_global        : reference to global object
 *
 * UNQCONV is a flag which is set in those N_id nodes which were
 * arguments to a class conversion (to_class, from_class) function.
 */

extern ids *MakeIds (char *name, char *mod, statustype status);

extern ids *MakeIds_Copy (char *name);

#define IDS_NAME(i) (i->id)
#define IDS_MOD(i) (i->mod)
#define IDS_NEXT(i) (i->next)
#define IDS_AVIS(i) (i->avis)
#define IDS_VARDEC(i) (i->node)
#define IDS_DEF(i) (i->def)
#define IDS_USE(i) (i->use)
#define IDS_STATUS(i) (i->status)
#define IDS_ATTRIB(i) (i->attrib)
#define IDS_REFCNT(i) (i->refcnt)
#define IDS_NAIVE_REFCNT(i) (i->naive_refcnt)

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
 ***    char*        NAME
 ***    char*        DECNAME
 ***    char*        LIBNAME
 ***    statustype   STATUS
 ***    locationtype LOC
 ***    deps*        SUB        (O)
 ***    deps*        NEXT       (O)
 ***/

/*
 * STATUS:
 *   ST_sac      : SAC module/class
 *   ST_external : external module/class
 *   ST_system   : external system library
 *   ST_own      : own declaration of module implementation
 */

extern deps *MakeDeps (char *name, char *decname, char *libname, statustype status,
                       locationtype loc, deps *sub, deps *next);

#define DEPS_NAME(d) (d->name)
#define DEPS_DECNAME(d) (d->decname)
#define DEPS_LIBNAME(d) (d->libname)
#define DEPS_STATUS(d) (d->status)
#define DEPS_LOC(d) (d->location)
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
 * STATUS:
 *   ST_regular
 *   ST_artificial
 *
 * ATTRIB: (in function node lists)
 *   ST_resolved
 *   ST_unresolved
 *
 * ATTRIB: (in object node lists)
 *   ST_reference
 *   ST_readonly_reference
 *
 * ATTRIB: (in typedef node lists)
 *   ST_regular
 */

/*
 * srs:
 * to use a nodelist in more general situations I have inserted a
 * new attribut ATTRIB2 which can store any suitable information.
 * Functions to handle a general node list can be found in tree_compound.h,
 * starting with NodeList... .
 * MakeNodelist(), MakeNodelistNode() are not needed to create the general
 * node list.
 */

extern nodelist *MakeNodelist (node *node, statustype status, nodelist *next);
extern nodelist *MakeNodelistNode (node *node, nodelist *next);

#define NODELIST_NODE(n) (n->node)
#define NODELIST_ATTRIB(n) (n->attrib)
#define NODELIST_ATTRIB2(n) (n->attrib2)
#define NODELIST_STATUS(n) (n->status)
#define NODELIST_NEXT(n) (n->next)
#define NODELIST_INT(n) ((int)(n->attrib))

/*--------------------------------------------------------------------------*/

/***
 ***  ARGTAB :
 ***
 ***  permanent attributes:
 ***
 ***    int*       SIZE
 ***    node*      PTR_IN[]     (N_arg, N_exprs)
 ***    void*      PTR_OUT[]    (TYPES, IDS)
 ***    argtag_t   TAG[]
 ***/

extern argtab_t *MakeArgtab (int size);

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
 ***    node*  DECL          (N_moddec, N_classdec)  (check-dec -> write-SIB !!)
 ***    node*  STORE_IMPORTS (N_implist)             (import -> checkdec !!)
 ***    node*  FOLDFUNS      (N_fundef)              (compile -> )
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
 ***    types*      IMPL         (O)           (import -> writesib !!)
 ***    node*       PRAGMA       (O)           (import -> readsib !!)
 ***    char*       COPYFUN      (O)           (readsib -> compile -> )
 ***    char*       FREEFUN      (O)           (readsib -> compile -> )
 ***    node*       TYPEDEC_DEF  (O)           (checkdec -> writesib !!)
 ***
 ***    node*       ICM          (O)  (N_icm)  (compile -> )
 ***/

/*
 * The STATUS indicates whether a type is defined or imported.
 * Possible values:
 *   ST_imported_mod   : imported from module
 *   ST_imported_class : imported from class
 *   ST_imported_mod   : imported from external module
 *   ST_imported_class : imported from external class
 *
 * The ATTRIB indicates whether a type is unique or not.
 * Possible values: ST_regular | ST_unique
 *
 * The TYPEDEC_DEF slot is only used when a typedef node is used as a
 * representation of a type declaration. It then points to the
 * typedef node which contains the respective definition.
 *
 * For each Non-SAC hidden type the name of a copy and a free function
 * is stored in COPYFUN and FREEFUN, respectively. These must be provided
 * with the external module/class. The names may be generic or user-defined
 * using pragmas.
 *
 * The attribute ICM holds the associated N_icm node in the case of
 * compiled typedefs
 */

extern node *MakeTypedef (char *name, char *mod, types *type, statustype attrib,
                          node *next);

#define TYPEDEF_TYPE(n) (n->info.types)
#define TYPEDEF_NAME(n) (*((char **)(&(n->mask[4]))))        /* for cc */
#define TYPEDEF_MOD(n) (*((char **)(&(n->mask[5]))))         /* for cc */
#define TYPEDEF_LINKMOD(n) (*((char **)(&(n->mask[6]))))     /* for cc */
#define TYPEDEF_STATUS(n) (*((statustype *)(&(n->info2))))   /* for cc */
#define TYPEDEF_ATTRIB(n) (*((statustype *)(&(n->mask[3])))) /* for cc */
#define TYPEDEF_IMPL(n) ((types *)(n->dfmask[0]))
#define TYPEDEF_NEXT(n) (n->node[0])
#define TYPEDEC_DEF(n) (n->node[1])
#define TYPEDEF_PRAGMA(n) (n->node[2])
#define TYPEDEF_COPYFUN(n) ((char *)(n->node[3]))
#define TYPEDEF_FREEFUN(n) ((char *)(n->node[4]))
#define TYPEDEF_ICM(n) (n->node[5])

/*--------------------------------------------------------------------------*/

/***
 ***  N_objdef :
 ***
 ***  sons:
 ***
 ***    node*      EXPR       ("N_expr")
 ***    node*      NEXT       (N_objdef)
 ***
 ***  permanent attributes:
 ***
 ***    char*      NAME
 ***    char*      MOD
 ***    char*      LINKMOD
 ***    types*     TYPE
 ***    statustype ATTRIB
 ***    statustype STATUS
 ***
 ***  temporary attributes:
 ***
 ***    char*      VARNAME                (typecheck -> obj-handling ->
 ***                                      ( -> precompile -> compile -> )
 ***    node*      PRAGMA     (N_pragma)  (import -> readsib -> precompile -> )
 ***    node*      ARG                    (obj-handling !!)
 ***    node*      SIB                    (readsib !!)
 ***    nodelist*  NEEDOBJS               (import -> analysis -> objects -> )
 ***
 ***    node*      OBJDEC_DEF             (checkdec -> writesib -> )
 ***
 ***    node*      ICM        (N_icm)     (compile -> )
 ***/

/*
 *  The STATUS indicates whether an object is defined or imported.
 *  Possible values:
 *   ST_imported_mod   : imported from module
 *   ST_imported_class : imported from class
 *   ST_imported_mod   : imported from external module
 *   ST_imported_class : imported from external class
 *
 *  ATTRIB: ST_unresolved | ST_resolved
 *  used in objects.c to distinguish between already initialized and
 *  not yet initialized global objects.
 *
 *  The VARNAME is a combination of NAME and MOD. It's used as parameter
 *  name when making global objects local.
 *
 *  ARG is a pointer to the additional argument which is added to a function's
 *  parameter list for this global object. ARG changes while traversing
 *  the functions!
 *
 *  ICM contains a pointer to the respective icm if the global object
 *  is an array (ND_KS_DECL_ARRAY_GLOBAL or ND_KD_DECL_ARRAY_EXTERN)
 *
 *  ATTENTION: ARG, INIT, and ICM are mapped to the same real node!
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

#define OBJDEF_TYPE(n) (n->info.types)
#define OBJDEF_NAME(n) (*((char **)(&(n->mask[4]))))        /* for cc */
#define OBJDEF_MOD(n) (*((char **)(&(n->mask[5]))))         /* for cc */
#define OBJDEF_LINKMOD(n) (*((char **)(&(n->mask[6]))))     /* for cc */
#define OBJDEF_STATUS(n) (*((statustype *)(&(n->info2))))   /* for cc */
#define OBJDEF_ATTRIB(n) (*((statustype *)(&(n->mask[3])))) /* for cc */
#define OBJDEF_VARNAME(n) (*((char **)(&(n->int_data))))    /* for cc */
#define OBJDEF_NEXT(n) (n->node[0])
#define OBJDEF_EXPR(n) (n->node[1])
#define OBJDEC_DEF(n) (n->node[2])
#define OBJDEF_ARG(n) (n->node[3])
#define OBJDEF_SIB(n) (n->node[3])
#define OBJDEF_PRAGMA(n) (n->node[4])
#define OBJDEF_NEEDOBJS(n) ((nodelist *)(n->node[5]))
#define OBJDEF_ICM(n) (n->node[3])

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
 ***    char*           MOD
 ***    char*           LINKMOD
 ***    types*          TYPES
 ***    ntype*          TYPE
 ***    statustype      STATUS
 ***    statustype      ATTRIB
 ***    bool            INLINE
 ***    int             FUNNO
 ***    node*           PRAGMA      (N_pragma)
 ***
 ***  permanent attributes for ST_condfun, ST_dofun, ST_whilefun fundefs only:
 ***
 ***    int             USED        [ref. count]  (lac2fun -> )
 ***    nodelist*       EXT_ASSIGNS (N_assign's)  (lac2fun -> )
 ***    node*           INT_ASSIGN  (N_assign)    (lac2fun -> )
 ***
 ***  temporary attributes:
 ***
 ***    node*           SIB         (N_sib)       (readsib !!)
 ***    node*           RETURN      (N_return)    (typecheck -> compile !!)
 ***    nodelist*       NEEDOBJS                  (import -> )
 ***                                                ( -> analysis -> )
 ***                                                ( -> write-SIB -> )
 ***                                                ( -> obj-handling -> )
 ***                                                ( -> liftspmd !!)
 ***    node*           ICM         (N_icm)       (compile -> print )
 ***    int             VARNO                     (optimize -> )
 ***    long*           MASK[x]                   (optimize -> )
 ***    int             INLREC                    (inline !!)
 ***    bool            EXPORT                    (dfr !!)
 ***
 ***    DFMmask_base_t  DFM_BASE             (lac2fun/rc -> spmd -> compile -> )
 ***
 ***    node*           FUNDEC_DEF  (N_fundef)    (checkdec -> writesib !!)
 ***
 ***    argtab_t*       ARGTAB                    (precompile -> compile !!)
 ***
 ***  temporary attributes for ST_spmdfun fundefs only:
 ***
 ***    node*           LIFTEDFROM  (N_fundef)    (liftspmd -> compile -> )
 ***    node*           WORKER      ( ?? )
 ***    node*           COMPANION   (N_fundef)    (rfin, mtfin -> )
 ***/

/*
 * STATUS:
 *   ST_regular        : function defined in this module
 *   ST_imported_mod   : function imported from module
 *   ST_imported_class : function imported from class
 *   ST_imported_mod   : function imported from external module
 *   ST_imported_class : function imported from external class
 *   ST_exported       : function is exported by module/class or program ('main')
 *   ST_objinitfun     : generic function for object initialization
 *   ST_classfun       : class conversion function
 *   ST_Cfun           : function implemented in C
 *   ST_foldfun        : dummy function containing fold code for with-loop
 *   ST_condfun        : function representing an if-else-clause (LaC function)
 *   ST_dofun          : function representing a do-loop (LaC function)
 *   ST_whilefun       : function representing a while-loop (LaC function)
 *   ST_zombiefun      : zombie function
 *   ST_wrapperfun     : wrapper function
 *
 * before multithreading:
 * ATTRIB:
 *   ST_regular    : shape-dependent or non-array function
 *   ST_shp_indep  : shape-independent but dim.-dependent array function
 *   ST_dim_indep  : dimension-independent array function
 *   ST_generic    : generic function derived from shape-independent array
 *                   function
 *   ST_gen_remove : generic function that has been specialized and will be
 *                   removed before typechecking
 *   ST_ignore     : unused generic function when compiling for c library
 *
 * while/after multithreading:
 * ATTRIB:
 *   ST_spmd_fun       : SPMD function
 *
 *   ST_call_any       : default_flag
 *                       (will be installed before using ATTRIB in mt-phases,
 *                        should not occur after mt-phases done)
 *   ST_call_st        : function is CALL_ST
 *   ST_call_mt_master : function is CALL_MT to be used by master
 *   ST_call_mt_worker : function is CALL_MT to be used by workers
 *   ST_call_rep       : function is CALL_REP
 *   ST_call_mtlift    : function is a thread-function
 *
 * The FUNDEC_DEF slot is only used when a fundef node is used as a
 * representation of a function declaration. It then points to the
 * fundef node which contains the respective definition.
 *
 * LINKMOD contains the name of the module which has to be linked with
 * in order to make the code of this function available. If LINKMOD is
 * NULL, then link with the module given by MOD.
 *
 * If USED contains an other value than USED_INACTIVE reference counting
 * for functions is activated.
 * For the time being reference counting is used for special LaC functions
 * only.
 * If the fundef is a definition of a special LaC function, USED counts
 * the number of times this function is referenced outside its own body.
 */

extern node *MakeFundef (char *name, char *mod, types *types, node *args, node *body,
                         node *next);

#define FUNDEF_BODY(n) (n->node[0])
#define FUNDEF_ARGS(n) (n->node[2])
#define FUNDEF_NEXT(n) (n->node[1])
#define FUNDEF_NAME(n) (*((char **)(&(n->mask[4]))))    /* for cc */
#define FUNDEF_MOD(n) (*((char **)(&(n->mask[5]))))     /* for cc */
#define FUNDEF_LINKMOD(n) (*((char **)(&(n->mask[6])))) /* for cc */
#define FUNDEF_TYPES(n) (n->info.types)
#define FUNDEF_TYPE(n) (ntype *)(n->info3)
#define FUNDEF_STATUS(n) (*((statustype *)(&(n->info2))))   /* for cc */
#define FUNDEF_ATTRIB(n) (*((statustype *)(&(n->mask[3])))) /* for cc */
#define FUNDEF_INLINE(n) (n->flag)
#define FUNDEF_FUNNO(n) (n->counter)
#define FUNDEF_PRAGMA(n) (n->node[4])

#define FUNDEF_SIB(n) (n->node[3])
#define FUNDEF_RETURN(n) (n->node[3])
#define FUNDEC_DEF(n) (n->node[3])
#define FUNDEF_NEEDOBJS(n) ((nodelist *)(n->dfmask[6]))
#define FUNDEF_VARNO(n) (n->varno)
#define FUNDEF_INLREC(n) (n->refcnt)
#define FUNDEF_EXPORT(n) (n->refcnt)
#define FUNDEF_MASK(n, x) (n->mask[x])
#define FUNDEF_DFM_BASE(n) ((DFMmask_base_t) (n->dfmask[0]))
#define FUNDEF_ARGTAB(n) ((argtab_t *)(n->dfmask[4]))
#define FUNDEF_ICM(n) (n->node[5])

/* LaC functions */
#define FUNDEF_USED(n) (n->int_data)
#define FUNDEF_EXT_ASSIGNS(n) ((nodelist *)(n->dfmask[1]))
#define FUNDEF_INT_ASSIGN(n) ((node *)(n->dfmask[2]))

/* concurrent: ST_spmdfun */
#define FUNDEF_LIFTEDFROM(n) ((node *)(n->dfmask[1]))
#define FUNDEF_WORKER(n) ((node *)(n->dfmask[2]))
#define FUNDEF_COMPANION(n) ((node *)(n->dfmask[3]))

/* multithreading: ST_spmdfun */
#define FUNDEF_IDENTIFIER(n) (n->int_data)
#define FUNDEF_MT2USE(n) (n->dfmask[1])
#define FUNDEF_MT2DEF(n) (n->dfmask[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_arg :
 ***
 ***  sons:
 ***
 ***    node*       NEXT         (N_arg)
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    types*      TYPE
 ***    statustype  STATUS
 ***    statustype  ATTRIB
 ***    node*       AVIS         (N_avis)
 ***
 ***  temporary attributes:
 ***
 ***    int         VARNO                     (optimize -> )
 ***    int         REFCNT                    (refcount -> compile -> )
 ***    int         NAIVE_REFCNT              (refcount -> concurrent -> )
 ***    bool        PADDED                    (ap -> )
 ***    char*       TYPESTRING                (precompile !!)
 ***    node*       OBJDEF       (N_objdef)   (obj-handling -> precompile !!)
 ***    node*       ACTCHN       (N_vinfo)    (psi-optimize -> )
 ***    node*       COLCHN       (N_vinfo)    (psi-optimize -> )
 ***    node*       FUNDEF       (N_fundef)   (psi-optimize -> )
 ***/

/*
 * STATUS:
 *   ST_regular    : original argument
 *   ST_artificial : additional argument added by object-handler
 *
 * ATTRIB:
 *   ST_regular            : non-unique parameter
 *   ST_unique             : unique parameter
 *   ST_reference          : (unique) reference parameter
 *   ST_readonly_reference : (unique) reference parameter which remains
 *                           unmodified
 *   ST_was_reference      : eliminated (unique) reference parameter
 *
 * TYPESTRING contains the argument's type as a string, used for renaming
 * of functions.
 *
 * ATTENTION:
 *   N_vardec and N_arg node have to have the same structure.
 *   See remark at N_id node.
 */

extern node *MakeArg (char *name, types *type, statustype status, statustype attrib,
                      node *next);

#define ARG_TYPE(n) (n->info.types)
#define ARG_NAME(n) (*((char **)(&(n->mask[4]))))        /* needed for cc */
#define ARG_STATUS(n) (*((statustype *)(&(n->info2))))   /* needed for cc */
#define ARG_ATTRIB(n) (*((statustype *)(&(n->mask[3])))) /* needed for cc */
#define ARG_AVIS(n) ((node *)(n->node[1]))
#define ARG_VARNO(n) (n->varno)
#define ARG_REFCNT(n) (n->refcnt)
#define ARG_NAIVE_REFCNT(n) (n->int_data)
#define ARG_PADDED(n) ((bool)(n->flag))
#define ARG_NEXT(n) (n->node[0])
#define ARG_TYPESTRING(n) ((char *)(n->dfmask[0]))
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
 ***    node*      INSTR             (N_assign, N_empty)
 ***    node*      VARDEC            (N_vardec)
 ***
 ***  permanent attributes:
 ***
 ***    char*      CACHESIM
 ***
 ***  temporary attributes:
 ***
 ***    nodelist*  NEEDFUNS                      (analysis -> )
 ***                                             ( -> analysis -> )
 ***                                             ( -> write-SIB -> DFR !!)
 ***    nodelist*  NEEDTYPES                     (analysis -> )
 ***                                             ( -> write-SIB -> )
 ***    long*      MASK[x]                       (optimize -> )
 ***    int        VARNO                         (optimize -> )
 ***
 ***    node*      SPMD_PROLOG_ICMS  (N_fundef)  (compile !!)
 ***    node*      SPMD_SETUP_ARGS   (N_fundef)  (compile !!)
 ***    node*      SCHEDULER_INIT    (N_assign)  (compile !!)
 ***
 ***    node*      SSACOUNTER        (N_ssacnt)  (ssaform -> optimize !!)
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
#define BLOCK_NEEDFUNS(n) ((nodelist *)(n->dfmask[0]))
#define BLOCK_NEEDTYPES(n) ((nodelist *)(n->dfmask[1]))
#define BLOCK_SPMD_PROLOG_ICMS(n) (n->node[3])
#define BLOCK_SPMD_SETUP_ARGS(n) (n->node[4])
#define BLOCK_SCHEDULER_INIT(n) (n->info2)
#define BLOCK_CACHESIM(n) (n->info.id)
#define BLOCK_SSACOUNTER(n) (n->node[5])

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
 ***    types*      TYPE           ... this struct!!)
 ***    statustype  STATUS        (element of TYPE, too!!)
 ***    node*       AVIS          (N_avis)
 ***
 ***  temporary attributes:
 ***
 ***    node*       OBJDEF   (O)  (N_objdef)   (inline -> precompile !!)
 ***    node*       ACTCHN   (O)  (N_vinfo)    (psi-optimize -> )
 ***    node*       COLCHN   (O)  (N_vinfo)    (psi-optimize -> )
 ***    int         REFCNT                     (refcount -> compile -> )
 ***    int         NAIVE_REFCNT               (refcount -> concurrent -> )
 ***    int         VARNO                      (optimize -> )
 ***    statustype  ATTRIB                     (typecheck -> uniquecheck -> )
 ***    int         FLAG                       (ael -> dcr2 !!)
 ***    bool        PADDED                     (ap -> )
 ***    node*       ICM      (O)  (N_icm)      (compile -> )
 ***/

/*
 * STATUS:
 *   ST_regular    : original vardec in source code
 *   ST_used       : used in typecheck only: vardec necessary?
 *   ST_artificial : artificial vardec produced by function inlining of a
 *                   function which uses a global object.
 *                   Such vardecs are removed by the precompiler.
 *
 * ATTRIB:
 *   ST_regular : normal variable
 *   ST_unique  : unique variable
 *
 * ATTENTION:
 *   N_vardec and N_arg node have to have the same structure.
 *   See remark at N_id node.
 */

extern node *MakeVardec (char *name, types *type, node *next);

#define VARDEC_TYPE(n) (n->info.types)
#define VARDEC_NAME(n) (*((char **)(&(n->mask[4]))))        /* for cc */
#define VARDEC_STATUS(n) (*((statustype *)(&(n->info2))))   /* for cc */
#define VARDEC_ATTRIB(n) (*((statustype *)(&(n->mask[3])))) /* for cc */
#define VARDEC_AVIS(n) (n->node[1])
#define VARDEC_VARNO(n) (n->varno)
#define VARDEC_REFCNT(n) (n->refcnt)
#define VARDEC_NAIVE_REFCNT(n) (n->int_data)
#define VARDEC_FLAG(n) (n->flag)
#define VARDEC_PADDED(n) ((bool)(n->flag))
#define VARDEC_NEXT(n) (n->node[0])
#define VARDEC_ACTCHN(n) (n->node[2])
#define VARDEC_COLCHN(n) (n->node[3])
#define VARDEC_OBJDEF(n) (n->node[4])
#define VARDEC_ICM(n) (n->node[5])

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
 ***    void*  INDEX    (O)               (wli -> wlf -> )
 ***    int    LEVEL                      (wli !!)
 ***
 ***  remarks:
 ***
 ***    there is no easy way to remove the INDEX information after wlf (another
 ***    tree traversal would be necessary), so it stays afterwards.
 ***    Nevertheless only wlf will use it. The type of INDEX is index_info*,
 ***    defined in WithloopFolding.c (not in types.h).
 ***
 ***    CF is used to temporarily store an N_assign node behind another one.
 ***    This additional N_assign node will later be inserted before the original
 ***    one into the assignment chain.
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
 ***    DFMmask_t USEMASK                    (multithread -> )
 ***    DFMmask_t DEFMASK                    (multithread -> )
 ***    int       LIRFLAG                    (ssalir->lirmov!!)
 ***/

extern node *MakeLet (node *expr, ids *ids);

#define LET_EXPR(n) (n->node[0])
#define LET_IDS(n) (n->info.ids)
#define LET_USEMASK(n) (n->dfmask[0])
#define LET_DEFMASK(n) (n->dfmask[1])
#define LET_LIRFLAG(n) (n->flag)

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
 ***    node*     EXPRS      (N_exprs)
 ***
 ***  temporary attributes:
 ***
 ***    node*     REFERENCE  (N_exprs)       (precompile -> compile -> )
 ***    node*     CRET       (N_exprs)       (precompile -> compile -> )
 ***    DFMmask_t USEMASK                    (multithread -> )
 ***    DFMmask_t DEFMASK                    (multithread -> )
 ***/

/*
 *  REFERENCE: List of artificial return values which correspond to
 *             reference parameters.
 *
 *  CRET: Points to the argument in EXPRS which is used as C return value
 *        in the compiled code.
 *
 *  ATTENTION: node[1] of N_return node already used by compile.c
 */

extern node *MakeReturn (node *exprs);

#define RETURN_EXPRS(n) (n->node[0])
#define RETURN_REFERENCE(n) (n->node[2])
#define RETURN_CRET(n) (n->node[3])
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
 ***    long*     MASK[x]                (optimize -> )
 ***    DFMmask_t IN_MASK                (infer_dfms -> lac2fun -> refcount -> )
 ***    DFMmask_t OUT_MASK               (infer_dfms -> lac2fun -> refcount -> )
 ***    DFMmask_t LOCAL_MASK             (infer_dfms -> lac2fun -> refcount -> )
 ***    ids*      THENVARS               (refcount -> compile -> )
 ***    ids*      ELSEVARS               (refcount -> compile -> )
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
#define COND_MASK(n, x) (n->mask[x])
#define COND_IN_MASK(n) (n->dfmask[0])
#define COND_OUT_MASK(n) (n->dfmask[1])
#define COND_LOCAL_MASK(n) (n->dfmask[2])
#define COND_THENVARS(n) ((ids *)(n->node[3]))
#define COND_ELSEVARS(n) ((ids *)(n->node[4]))
#define COND_NAIVE_THENVARS(n) ((ids *)(n->info2))
#define COND_NAIVE_ELSEVARS(n) ((ids *)(n->node[5]))

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
 ***    DFMmask_t IN_MASK                (infer_dfms -> lac2fun -> refcount -> )
 ***    DFMmask_t OUT_MASK               (infer_dfms -> lac2fun -> refcount -> )
 ***    DFMmask_t LOCAL_MASK             (infer_dfms -> lac2fun -> refcount -> )
 ***
 ***  attention:
 ***    - To access do-loops and while-loops with one macro use the
 ***      compound macros DO_OR_WHILE_xxx!
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
#define DO_MASK(n, x) (n->mask[x])
#define DO_IN_MASK(n) (n->dfmask[0])
#define DO_OUT_MASK(n) (n->dfmask[1])
#define DO_LOCAL_MASK(n) (n->dfmask[2])
#define DO_USEVARS(n) ((ids *)(n->node[2]))
#define DO_DEFVARS(n) ((ids *)(n->node[3]))
#define DO_NAIVE_USEVARS(n) ((ids *)(n->node[4]))
#define DO_NAIVE_DEFVARS(n) ((ids *)(n->node[5]))

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
 ***    DFMmask_t IN_MASK                (infer_dfms -> lac2fun -> refcount -> )
 ***    DFMmask_t OUT_MASK               (infer_dfms -> lac2fun -> refcount -> )
 ***    DFMmask_t LOCAL_MASK             (infer_dfms -> lac2fun -> refcount -> )
 ***
 ***  attention:
 ***    - To access do-loops and while-loops with one macro use the
 ***      compound macros DO_OR_WHILE_xxx!
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
#define WHILE_MASK(n, x) (n->mask[x])
#define WHILE_IN_MASK(n) (n->dfmask[0])
#define WHILE_OUT_MASK(n) (n->dfmask[1])
#define WHILE_LOCAL_MASK(n) (n->dfmask[2])
#define WHILE_USEVARS(n) ((ids *)(n->node[2]))
#define WHILE_DEFVARS(n) ((ids *)(n->node[3]))
#define WHILE_NAIVE_USEVARS(n) ((ids *)(n->node[4]))
#define WHILE_NAIVE_DEFVARS(n) ((ids *)(n->node[5]))

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
 ***    node*      ARGS    (O)  (N_exprs)
 ***
 ***  permanent attributes:
 ***
 ***    char*      NAME
 ***    char*      MOD     (O)
 ***    int        ATFLAG  (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*      FUNDEF       (N_fundef)  (typecheck -> analysis -> )
 ***                                          ( -> obj-handling -> compile -> )
 ***    argtab_t*  ARGTAB                   (precompile -> compile !!)
 ***/

extern node *MakeAp (char *name, char *mod, node *args);

#define AP_NAME(n) (n->info.fun_name.id)
#define AP_MOD(n) (n->info.fun_name.id_mod)
#define AP_ATFLAG(n) (n->counter)
#define AP_ARGS(n) (n->node[0])
#define AP_FUNDEF(n) (n->node[1])
#define AP_ARGTAB(n) ((argtab_t *)(n->dfmask[4]))

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
 ***    node*       AVIS      (N_avis)          (ssafrm -> )
 ***    node*       VARDEC    (N_vardec/N_arg)  (typecheck -> )
 ***    node*       OBJDEF    (N_objdef)        (typecheck -> )
 ***                                            ( -> analysis -> )
 ***    int         REFCNT                      (refcount -> compile -> )
 ***    int         NAIVE_REFCNT                (refcount -> concurrent -> )
 ***    unqconv_t   UNQCONV                     (precompile -> compile -> )
 ***    node*       DEF                         (Unroll !!, Unswitch !!)
 ***    node*       WL                          (wli -> wlf !!)
 ***
 ***    void*       CONSTVEC                    (flatten -> )
 ***    int         VECLEN                      (flatten -> )
 ***    simpletype  VECTYPE                     (flatten -> )
 ***    int         ISCONST                     (flatten -> )
 ***    int         NUM
 ***
 ***    node*       NT_TAG    (N_vardec/N_arg)  (compile -> )
 ***
 ***
 ***  remarks:
 ***
 ***    WL is only used in wli, wlf. But every call of DupTree() initializes
 ***    the copy's WL_ID with a pointer to it's original N_id node.
 ***    The function SearchWL() can define WL in another way (pointer to
 ***    N_assign node of WL which is referenced by this Id).
 ***
 ***    VARDEC points to an N_vardec or an N_arg node. This is not
 ***    distinguished in many places of the code. So for example
 ***    VARDEC_NAME and ARG_NAME should both be substitutions for identical
 ***    implementations.
 ***
 ***    ISCONST, VECTYPE, VECLEN, CONSTVEC, and NUM are used for propagation
 ***    of constant integer arrays.
 ***    Usually, there is no constant propagation for arrays since this
 ***    normally slows down the code due to memory allocation/de-allocation
 ***    costs. However for some other optimizations, namely tile size inference,
 ***    a constant value is an advantage.
 ***
 ***    NT_TAG contains a pointer to the vardec iff this id should be printed
 ***    as name tuple (relevant for ICM arguments only).
 ***
 ***  caution:
 ***
 ***    Unroll uses ->flag without a macro :(
 ***    Even worse: Unroll uses ->flag of *every* LET_EXPR node :(((
 ***/

/*
 * STATUS:
 *   ST_regular       : original argument
 *                        in a function application or return-statement
 *   ST_artificial    : additional argument added by object-handler
 *                        in a function application or return-statement
 *
 * ATTRIB:
 *   ST_regular       : ordinary argument
 *                        in a function application or return-statement
 *   ST_global        : global object
 *   ST_readonly_reference/
 *   ST_reference     : argument in a function application which is passed as
 *                      a reference parameter
 *                      -- or --
 *                      additional argument in a return-statement which belongs
 *                      to a reference parameter
 *   ST_was_reference : argument in a function application which is passed as
 *                      a eliminated reference parameter
 *                      -- or --
 *                      additional argument in a return-statement which belongs
 *                      to a eliminated reference parameter
 *
 * UNQCONV is a flag which is set in those N_id nodes which were
 * arguments to a class conversion (to_class, from_class) function.
 */

extern node *MakeId (char *name, char *mod, statustype status);

extern node *MakeId_Copy (char *str);

extern node *MakeId_Copy_NT (node *vardec);

extern node *MakeId_Num (int val);

#define ID_IDS(n) (n->info.ids)
#define ID_NAME(n) (IDS_NAME (ID_IDS (n)))
#define ID_AVIS(n) (IDS_AVIS (ID_IDS (n)))
#define ID_VARDEC(n) (IDS_VARDEC (ID_IDS (n)))
#define ID_OBJDEF(n) (IDS_VARDEC (ID_IDS (n)))
#define ID_MOD(n) (IDS_MOD (ID_IDS (n)))
#define ID_ATTRIB(n) (IDS_ATTRIB (ID_IDS (n)))
#define ID_STATUS(n) (IDS_STATUS (ID_IDS (n)))
#define ID_DEF(n) (IDS_DEF (ID_IDS (n)))
#define ID_REFCNT(n) (IDS_REFCNT (ID_IDS (n)))
#define ID_NAIVE_REFCNT(n) (IDS_NAIVE_REFCNT (ID_IDS (n)))
#define ID_UNQCONV(n) (*((unqconv_t *)(&(n->node[4])))) /* needed for cc */
#define ID_WL(n) (n->node[0])
#define ID_NT_TAG(n) (n->node[5])

#define ID_VECLEN(n) (n->counter)
#define ID_VECTYPE(n) ((simpletype) (n->int_data))
#define ID_CONSTVEC(n) (n->info2)
#define ID_ISCONST(n) (n->varno)
#define ID_NUM(n) (n->refcnt)

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
 ***    bool VAL
 ***/

extern node *MakeBool (bool val);

#define BOOL_VAL(n) ((bool)((n)->info.cint))

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
 ***  N_ok :
 ***
 ***  dummynode, last declared in node_info.mac
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
 ***  N_icm :
 ***
 ***  sons:
 ***
 ***    node*  ARGS  (O)  (N_exprs)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    int    INDENT_BEFORE
 ***    int    INDENT_AFTER
 ***    bool   END_OF_STATEMENT
 ***
 ***  temporary attributes:
 ***
 ***    node*  FUNDEF            (compile -> free!! )
 ***
 ***
 ***  remarks:
 ***
 ***    INDENT_??? are used for indenting ICMs in output. This values are set
 ***    by 'MakeIcm' and used by 'PrintIcm'.
 ***
 ***    The END_OF_STATEMENT flag causes a semicolon to be printed behind
 ***    the ICM or not.
 ***
 ***    FUNDEF is needed for ICMs that reference a N_fundef node
 ***    (e.g. ND_FUN_AP) in order to decrement the FUNDEF_USED counter
 ***    during FreeIcm().
 ***/

extern node *MakeIcm (char *name, node *args);

#define ICM_NAME(n) (n->info.fun_name.id)
#define ICM_INDENT_BEFORE(n) (n->flag)
#define ICM_INDENT_AFTER(n) (n->int_data)
#define ICM_ARGS(n) (n->node[0])
#define ICM_FUNDEF(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_pragma :
 ***
 ***  permanent attributes:
 ***
 ***    char*  LINKNAME         (O)
 ***    int[]  LINKSIGN         (O)               (import -> )
 ***    int[]  REFCOUNTING      (O)               (import -> )
 ***    char*  INITFUN          (O)
 ***
 ***    node*  WLCOMP_APS       (0)   (N_exprs)
 ***    node*  APL              (0)   (N_ap)
 ***
 ***  temporary attributes:
 ***
 ***    int[]  READONLY         (O)               (import -> readsib !!)
 ***    ids*   EFFECT           (O)               (import -> readsib !!)
 ***    ids*   TOUCH            (O)               (import -> readsib !!)
 ***    char*  COPYFUN          (O)               (import -> readsib !!)
 ***    char*  FREEFUN          (O)               (import -> readsib !!)
 ***    ids*   NEEDTYPES        (O)               (import -> readsib !!)
 ***    node*  NEEDFUNS         (O)               (import -> readsib !!)
 ***    char*  LINKMOD          (O)               (import -> readsib !!)
 ***    int    NUMPARAMS        (O)               (import -> readsib !!)
 ***
 ***    nums*  LINKSIGNNUMS     (O)               (scanparse -> import !!)
 ***    nums*  REFCOUNTINGNUMS  (O)               (scanparse -> import !!)
 ***    nums*  READONLYNUMS     (O)               (scanparse -> import !!)
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
#define PRAGMA_APL(n) (n->node[5])

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
 ***  N_cseinfo :
 ***
 ***  sons:
 ***
 ***    node*    NEXT     (O)  (N_cseinfo)
 ***
 ***  permanent attributes:
 ***
 ***    node*    LAYER         (N_cseinfo)
 ***    node*    LET           (N_let)
 ***/

/*
 * chained list of available expressions (N_let nodes) used for
 * common subexpression elimination. The lists are organized
 * in layers.
 * These N_cseinfo nodes are not located in the AST, they are
 * only used internally by CSE
 */

extern node *MakeCSEinfo (node *next, node *layer, node *let);

#define CSEINFO_NEXT(n) (n->node[0])
#define CSEINFO_LAYER(n) (n->node[1])
#define CSEINFO_LET(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_ssacnt :
 ***
 ***  sons:
 ***
 ***    node*    NEXT     (O)  (N_ssacnt)
 ***
 ***  permanent attributes:
 ***
 ***    int      COUNT                    counter of renamed instances
 ***    char*    BASEID                   basename to extend with __ssa<count>
 ***/

/*
 * ssacnt stores for each base identifier the actual rename
 * counter when transforming code in SSA-form. The set of
 * N_ssacnt nodes is located in the main-block of the
 * function using these variables to be renamed.
 */

extern node *MakeSSAcnt (node *next, int count, char *baseid);

#define SSACNT_NEXT(n) (n->node[0])
#define SSACNT_COUNT(n) (n->counter)
#define SSACNT_BASEID(n) ((char *)(n->info2))

/*--------------------------------------------------------------------------*/

/***
 ***  N_ssastack :
 ***
 ***  sons:
 ***
 ***    node*    NEXT     (O)  (N_ssastack)
 ***
 ***  permanent attributes:
 ***
 ***    node*    AVIS           (N_avis)  rename-to AVIS node in stack-list
 ***    bool     INUSE                    mark if stack is in use or to be
 ***/

/*
 * ssastack does the stacking of rename-to avis nodes during ssa-traversal
 * in vardec or arg-avis nodes.
 */

extern node *MakeSSAstack (node *next, node *avis);

#define SSASTACK_NEXT(n) (n->node[0])
#define SSASTACK_AVIS(n) (n->node[1])
#define SSASTACK_INUSE(n) ((bool)(n->flag))

/*--------------------------------------------------------------------------*/

/***
 ***  N_avis :
 ***
 ***  sons:
 ***
 ***  permanent attributes:
 ***
 ***    node*       VARDECORARG     (N_vardec/N_arg)
 ***
 ***  temporary attributes:
 ***
 ***    node*       SSACOUNT        (N_ssacnt)       (ssaform -> undossa !!)
 ***    node*       SSAASSIGN       (N_assign)       (ssaform -> undossa !!)
 ***    node*       SSAASSIGN2      (N_assign)       (ssaform -> undossa !!)
 ***    constant*   SSACONST (O)                     (cf -> undossa !!)
 ***    ssaphit_t   SSAPHITARGET (O)                 (ssaform -> undossa !!)
 ***    bool        SSALPINV (O)                     (lir -> undossa !!)
 ***    node*       SSASTACK (O)    (N_ssastack)     (ssaform -> undossa !!)
 ***    bool        SSAUNDOFLAG (O)                  (ssaform -> undossa !!)
 ***
 ***    the following attributes are only used within ssaform traversal:
 ***    bool        SSADEFINED (O)                   (ssaform!!)
 ***    node*       SSATHEN (O)     (N_avis)         (ssaform!!)
 ***    node*       SSAELSE (O)     (N_avis)         (ssaform!!)
 ***
 ***    the following attributes are used within SSADeadCodeRemoval/SSALIR:
 ***    bool        NEEDCOUNT                        (ssadcr!!, ssalir!!)
 ***
 ***    the following attributes are used within SSACSE,
 ***      SSALIR, UndoSSATransform:
 ***    node*       SUBST (O)       (N_avis)         (ssacse!!. undossa!!)
 ***
 ***    the following attributes are only used within UndoSSATransform:
 ***    node*       SUBSTUSSA (O)   (N_avis)          (undossa!!)
 ***
 ***    the following attributes are only used within SSALIR:
 ***    int         DEFDEPTH (O)    (WITHDEPTH)       (ssalir!!)
 ***    int         LIRMOVE (O)     (bitfield)        (ssalir!!)
 ***    bool        EXPRESULT (O)                     (ssalir!!)
 ***
 ***/

/*
 * avis node store information identical in vardec and args.
 * every arg/vardec node contains exactly one avis node.
 */

extern node *MakeAvis (node *vardecOrArg);

#define AVIS_VARDECORARG(n) (n->node[0])
#define AVIS_SSACOUNT(n) (n->node[1])
#define AVIS_SSAASSIGN(n) (n->node[2])
#define AVIS_SSAASSIGN2(n) (n->node[3])
#define AVIS_SSACONST(n) ((constant *)(n->info2))
#define AVIS_SSAPHITARGET(n) ((ssaphit_t) (n->flag))
#define AVIS_SSALPINV(n) ((bool)(n->refcnt))
#define AVIS_SSASTACK(n) (n->node[4])
#define AVIS_SSAUNDOFLAG(n) ((bool)(n->counter))
/* used only in ssatransform */
#define AVIS_SSADEFINED(n) ((bool)(n->int_data))
#define AVIS_SSATHEN(n) ((node *)(n->dfmask[1]))
#define AVIS_SSAELSE(n) ((node *)(n->dfmask[2]))
/* used only in ssadcr an again in SSALIR */
#define AVIS_NEEDCOUNT(n) (n->int_data)
/* used in ssacse, SSALIR and UndoSSAtransform */
#define AVIS_SUBST(n) ((node *)(n->dfmask[0]))
/* used only in UndoSSAtransform */
#define AVIS_SUBSTUSSA(n) ((node *)(n->dfmask[1]))
/* used only in SSALIR */
#define AVIS_DEFDEPTH(n) (n->varno)
#define AVIS_LIRMOVE(n) (n->lineno)
#define AVIS_EXPRESULT(n) ((bool)(n->info.cint))

/*--------------------------------------------------------------------------*/

/***
 ***  N_info :
 ***
 ***  The N_info node is used to store additional compile time information
 ***  outside the syntax tree. So, its concrete look depends on the
 ***  specific task.
 ***
 ***  when used in flatten.c:
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
 ***    node *     FOLDFUNS       (O)  (N_fundef)
 ***    node *     MODUL          (O)  (N_modul)
 ***
 ***  when used in typecheck.c :
 ***    int        STATUS        (O)
 ***    node *     VARDEC        (O)  (N_vardec)
 ***    node *     FUNDEF        (O)  (N_fundef)
 ***    node *     NEXTASSIGN    (O)  (N_assign)
 ***    node *     LASSIGN       (O)  (N_assign)
 ***    ids*       LHSVARS       (O)
 ***    nodelist * TCCP          (O)
 ***    bool       TCCPSUCCESS   (O)
 ***    bool       ISGWLSHAPE    (O)
 ***
 ***  when used in writesib.c :
 ***    nodelist*  EXPORTTYPES   (O)
 ***    nodelist*  EXPORTOBJS    (O)
 ***    nodelist*  EXPORTFUNS    (O)
 ***
 ***  when used in compile.c :
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
 ***    int        SCHEDULERID   (O)
 ***    node*      SCHEDULERINIT (O)  (N_assign)
 ***
 ***  when used in optimize.c :
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
 ***  when used in DeadFunctionRemoval.c:
 ***    int        SPINE
 ***
 ***  when used in index.c:
 ***    node*      INFO_IVE_FUNDEF           (N_fundef)
 ***    node*      INFO_IVE_VARDECS          (N_vardec)
 ***    ive_mode   INFO_IVE_MODE
 ***    node*      INFO_IVE_CURRENTASSIGN    (N_assign)
 ***    node*      INFO_IVE_TRANSFORM_VINFO  (N_vinfo)
 ***    int        INFO_IVE_NON_SCAL_LEN
 ***
 ***
 ***  old mt!
 ***  when used in managing spmd- and sync blocks in concurrent :
 ***
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
 ***
 ***  new mt!
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
 ***    access_t*  ACCESS
 ***    node*      INDEXVAR           (N_vardec/N_arg)
 ***    feature_t  FEATURE
 ***    WithOpType WOTYPE
 ***    ids*       LASTLETIDS
 ***    int        BELOWAP
 ***    access_t*  TMPACCESS
 ***
 ***  when used in spmd_opt.c :
 ***    node*      LASTASSIGN
 ***    node*      THISASSIGN
 ***    node*      NEXTASSIGN
 ***
 ***  when used in print.c :
 ***    node*      FUNDEF             (N_fundef)
 ***    node*      INT_SYN
 ***    node*      WITH_RET
 ***    node*      NWITH2
 ***    node*      ACCESS
 ***    int        SIB
 ***    int        OMIT_FORMAL_PARAMS
 ***    int        VARNO
 ***    int        PROTOTYPE
 ***    int        SEPARATE
 ***
 ***  when used in refcount.c
 ***    node*      PRF
 ***    node*      WITH
 ***    int*       RCDUMP             (only while traversing a group of N_code's)
 ***    int*       NAIVE_RCDUMP       (only while traversing a group of N_code's)
 ***    int        VARNO
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
 ***    bool       EXPRESSION_PADDED
 ***
 ***  when used in import_specializations.c
 ***    node*      SPECS             (chain of specialized fundefs)
 ***    node*      FUNDEF            (actual working fundef)
 ***
 ***  when used in SSATransform.c
 ***    node*      FUNDEF            (actual working fundef)
 ***    node*      BLOCK             (top-level block of function)
 ***    bool       RETINSTR          (working on return instruction)
 ***    node*      WITHVEC           (avis of withid vec for multi-part with-loops)
 ***    node*      WITHIDS           (avis of withid ids for multi-part with-loops)
 ***    node*      CONDSTMT          (store cond-node for later access)
 ***    int        CONDSTATUS        (store position in conditional traversal)
 ***    node*      ASSIGN            (current assignment node)
 ***    bool       SINGLEFUNDEF      (traversal mode: all fundefs/single fundef)
 ***
 ***  when used in UndoSSATransform.c
 ***    node*      ARGS              (arg chain of fundef)
 ***    node*      TOPBLOCK          (toplevel block with vardec chain)
 ***    node*      FOLDTARGET        (new AVIS as unique fold-target)
 ***    node*      CONSTASSIGNS      (assignments to move from else to pre-return)
 ***    int        ASSIGNOP          (operation to perform on actual assignment)
 ***    node*      MODUL             (current working modul)
 ***
 ***  when used in SSADeadCodeRemoval.c
 ***    int        DEPTH             (recursion depth of special functions)
 ***    bool       REMASSIGN         (flag, if assignment can be removed)
 ***    node*      FUNDEF            (current working fundef)
 ***    bool       REMRESULTS        (flag to remove unused results from return)
 ***    node*      APFUNDEF          (called functions to remove results from)
 ***    int        RESCOUNT          (counter when traversing the results)
 ***    int        RESNEEDED         (counter for needed results of a fun_ap)
 ***    node*      LET               (actual let node)
 ***    node*      ASSIGN            (actual assign node)
 ***    node*      MODUL             (current working modul)
 ***
 ***  when used in SSACSE.c
 ***    bool       REMASSIGN         (flag, if assignment can be removed)
 ***    node*      FUNDEF            (current working fundef)
 ***    node*      CSE               (cseinfo chain of available expressions)
 ***    node*      MODUL             (current working modul)
 ***    node*      ASSIGN            (current working assignment)
 ***    bool       RECFUNAP          (== TRUE when processing recursive ap)
 ***    nodelist*  RESULTARG         (args that are results of a loop fun)
 ***
 ***  when used in compare_tree.c
 ***    cmptree_t  EQFLAG            (current equal flag for whole tree)
 ***    node*      TREE              (tree to compare with)
 ***
 ***  when used in SSAConstantFolding.c
 ***    bool       REMASSIGN         (flag, if assignment can be removed)
 ***    node*      FUNDEF            (current working fundef)
 ***    bool       INSCONST          (flag, if const value should be inserted
 ***                                  for a constant identifier)
 ***    node*      POSTASSIGN        (assignments to add behind assignment)
 ***    node*      TOPBLOCK          (vardec chain of actual function)
 ***    node*      RESULTS           (exprs chain of return statement)
 ***    bool       INLINEAP          (flag, to inline special function)
 ***    node*      MODUL             (current working modul)
 ***    node*      ASSIGN            (current worling assignment)
 ***    bool       INLFUNDEF         (current fundef will be inlined)
 ***
 ***  when used in SSALIR.c
 ***    node*      FUNDEF            (current working fundef)
 ***    bool       REMASSIGN         (flag, if assignment can be removed)
 ***    node*      PREASSIGN         (assignments to add before assignment)
 ***    node*      POSTASSIGN        (assignments to add behind assignment)
 ***    node*      MODUL             (current working modul)
 ***    node*      ASSIGN            (current working assignment)
 ***    int        NONLIRUSE         (counts non lir args on rightside of expr.)
 ***    int        CONDSTATUS        (flag for part of conditional)
 ***    int        WITHDEPTH         (counter for depth of with loops)
 ***    bool       TOPBLOCK          (true when traversing the toplevel block)
 ***    int        FLAG              (controls behavior of some functions)
 ***    node*      EXTPREASSIGN      (assignments to add before calling ap)
 ***    node*      EXTPOSTASSIGN     (assignments to add behind calling ap)
 ***    LUT_t      MOVELUT           (look up table to adjust vardec/avis/ids
 ***                                  when moving assignments out of fundef)
 ***    node*      APARGCHAIN        (argument chain of external application)
 ***    ids*       APRESCHAIN        (result chain of external application)
 ***    node*      EXTFUNDEF         (fundef with ap node to this special fundef)
 ***    node*      RESULTMAP         (nodelist with local avis/external avis map)
 ***    int        MAXDEPTH          (max. definition with-loop depth of used id)
 ***    nodelist*  INSLIST           (special nodelist that holds WLIR assigns)
 ***    int        SETDEPTH          (holds the defintion depth to be set in ids)
 ***
 ***  when used in free.c
 ***    node*      FLAG              (mode flag for FreeTrav/FreeNode)
 ***    node*      ASSIGN            (current working assignment)
 ***
 ***  when used in optimize.c
 ***    node*      MODUL             (current working modul)
 ***
 ***  when used in CheckAvis.c
 ***    node*      FUNDEF            (current working fundef)
 ***    bool       SINGLEFUNDEF      (traversal mode: all fundefs/single fundef)
 ***
 ***  when used in optimize.c
 ***    node*      MODUL             (current working modul)
 ***    node*      FUNDEF            (current working fundef)
 ***    node*      ASSIGN            (current working assignment)
 ***
 ***  when used in SSAInferLI.c
 ***    node*      FUNDEF            (current working fundef)
 ***    node*      ARGCHAIN          (EXPRS chain of recursive funap)
 ***
 ***  when used in insert_vardec.c
 ***    node*      VARDECS           (current vardecs)
 ***    node*      ARGS              (current args)
 ***
 ***
 ***  when used in WithloopScalarization.c
 ***    bool       POSSIBLE          (true, if WLS is possible
 ***    int        PHASEINT          (phase of WLS, casted do wls_phase)
 ***    node*      FUNDEF            (current fundef)
 ***
 ***  remarks:
 ***
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

/* DupTree */
#define INFO_DUP_TYPE(n) (n->flag)
#define INFO_DUP_CONT(n) (n->node[0])
#define INFO_DUP_FUNDEF(n) (n->node[1])
#define INFO_DUP_LUT(n) ((LUT_t) (n->dfmask[0]))
#define INFO_DUP_INSPECIAL(n) ((bool)(n->counter))
#define INFO_DUP_ASSIGN(n) (n->node[2])

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
 * An N_ok-node is used as info-carrier!
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
#define INFO_TC_TCCP(n) (n->info2)
#define INFO_TC_ISGWLSHAPE(n) (n->varno)
#define INFO_TC_TCCPSUCCESS(n) (n->counter)

/* writesib */
#define INFO_WSIB_EXPORTTYPES(n) ((nodelist *)(n->node[0]))
#define INFO_WSIB_EXPORTOBJS(n) ((nodelist *)(n->node[1]))
#define INFO_WSIB_EXPORTFUNS(n) ((nodelist *)(n->node[2]))
/* see also print access macros used in this phase! */

/* refcount */
#define INFO_RC_FUNDEF(n) (n->node[0])
#define INFO_RC_PRF(n) (n->node[1])
#define INFO_RC_WITH(n) (n->node[2])
#define INFO_RC_RCDUMP(n) ((int *)(n->node[3]))
#define INFO_RC_NAIVE_RCDUMP(n) ((int *)(n->node[4]))
#define INFO_RC_VARNO(n) (n->varno)
#define INFO_RC_ONLYNAIVE(n) (n->flag)

/* wltransform */
#define INFO_WL_TYPES(n) (n->info.types)

/* concurrent */
#define INFO_CONC_FUNDEF(n) (n->node[0])

/* concurrent - spmdinit */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SPMDI_LASTSPMD(n) (n->flag)
#define INFO_SPMDI_NEXTSPMD(n) (n->counter)
#define INFO_SPMDI_CONTEXT(n) (n->int_data)
#define INFO_SPMDI_EXPANDCONTEXT(n) (n->varno)
#define INFO_SPMDI_EXPANDSTEP(n) (n->refcnt)

/* concurrent-spmdopt */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SPMDO_THISASSIGN(n) (n->node[1])
#define INFO_SPMDO_NEXTASSIGN(n) (n->node[2])

/* concurrent-spmdlift */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SPMDL_MT(n) (n->counter)

/* concurrent-syncinit */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SYNCI_FIRST(n) (n->flag)
#define INFO_SYNCI_LAST(n) (n->int_data)

/* concurrent-syncopt */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SYNCO_THISASSIGN(n) (n->node[1])
#define INFO_SYNCO_NEXTASSIGN(n) (n->node[2])

/* concurrent-spmdcons */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SPMDC_FIRSTSYNC(n) (n->node[1])

/* concurrent-spmdtrav-reducemasks */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SPMDRM_RESULT(n) (n->dfmask[0])
#define INFO_SPMDRM_CHECK(n) (n->dfmask[1])

/* concurrent-spmdtrav-reduceoccurences */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SPMDCO_RESULT(n) ((int *)(n->node[1]))
#define INFO_SPMDCO_WHICH(n) (n->dfmask[1])

/* concurrent-spmdtrav-reduceoccurences */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SPMDRO_CHECK(n) (n->dfmask[1])
#define INFO_SPMDRO_COUNTERS(n) ((int *)(n->node[1]))

/* concurrent-spmdtrav-lc */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SPMDLC_APPLICATION(n) (n->int_data)

/* concurrent-spmdtrav-deletenested */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SPMDDN_NESTED(n) (n->int_data)

/* concurrent-spmdtrav-producemasks */
/* DO NOT OVERRIDE ANY INFO_CONC_XXX here! */
#define INFO_SPMDPM_IN(n) (n->dfmask[0])
#define INFO_SPMDPM_INOUT(n) (n->dfmask[1])
#define INFO_SPMDPM_OUT(n) (n->dfmask[2])
#define INFO_SPMDPM_LOCAL(n) (n->dfmask[3])
#define INFO_SPMDPM_SHARED(n) (n->dfmask[4])

/* multithread - all mini-phases */
/* DO NOT OVERRIDE ANY INFO_YYYY_xxx HERE, were YYYY is any other miniphase! */
#define INFO_MUTH_FUNDEF(n) (n->node[0])
#define INFO_MUTH_ALLOW_OOOC(n) (n->counter)
#define INFO_MUTH_TOPDOWN(n) (n->refcnt)
#define INFO_MUTH_DRIVER(n) ((funptr) (n->node[1]))
#define INFO_MUTH_IGNORE(n) ((ignorefun) (n->node[2]))

/* multithread - schedule_init */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE! */
#define INFO_SCHIN_SCHEDULING(n) (n->node[3])
#define INFO_SCHIN_INNERWLS(n) (n->int_data)
#define INFO_SCHIN_ALLOWED(n) (n->flag)

/* multithread - repfuns_init */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE! */
#define INFO_RFIN_WITHINWITH(n) (n->int_data)

/* multithread - blocks_expand */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE! */
#define INFO_BLKEX_BLOCKABOVE(n) (n->int_data)

/* multithread - blocks_expand */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE! */
#define INFO_MTFIN_CURRENTATTRIB(n) ((statustype) (n->int_data))

/* multithread - blocks_expand */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE! */
#define INFO_BLKCO_CURRENTATTRIB(n) ((statustype) (n->int_data))
#define INFO_BLKCO_THISASSIGN(n) (n->node[3])

/* multithread - blocks_expand */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE! */
#define INFO_DFA_HEADING(n) (n->int_data)
#define INFO_DFA_USEMASK(n) (n->dfmask[0])
#define INFO_DFA_DEFMASK(n) (n->dfmask[1])
#define INFO_DFA_NEEDCHAIN(n) (n->dfmask[2])
#define INFO_DFA_NEEDBLOCK(n) (n->dfmask[3])
#define INFO_DFA_CONT(n) (n->node[3])
#define INFO_DFA_THISASSIGN(n) (n->node[4])
#define INFO_DFA_INFER_LET_DEFMASK(n) (n->dfmask[4])
#define INFO_DFA_INFER_LET_USEMASK(n) (n->dfmask[5])

/* multithread - barriers_init */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE! */
#define INFO_BARIN_WITHINMT(n) (n->int_data)

/* multithread - adjust_calls */
/* DO NOT OVERRIDE ANY INFO_MUTH_XXX HERE! */
#define INFO_ADJCA_ATTRIB(n) ((statustype) (n->int_data))

/* precompile */
#define INFO_PREC1_OBJINITFUNDEF(n) (n->node[0])
#define INFO_PREC2_PRE_ASSIGNS(n) (n->node[0])
#define INFO_PREC2_POST_ASSIGNS(n) (n->node[1])
#define INFO_PREC3_FUNDEF(n) (n->node[0])
#define INFO_PREC3_LET(n) (n->node[1])
#define INFO_PREC3_LASTASSIGN(n) (n->node[2])
#define INFO_PREC3_CEXPR(n) (n->node[3])

/* ArrayElemination */
#define INFO_AE_TYPES(n) (n->node[1])
#define INFO_AE_FUNDEF(n) (n->node[2])

/* compile */
#define INFO_COMP_MODUL(n) (n->node[0])
#define INFO_COMP_FUNDEF(n) (n->node[1])
#define INFO_COMP_LASTSYNC(n) (n->node[3])
#define INFO_COMP_LASTIDS(n) (n->info.ids)
#define INFO_COMP_FOLDFUNS(n) ((bool)(n->varno))
#define INFO_COMP_ASSIGN(n) (n->node[5])
#define INFO_COMP_SCHEDULERID(n) (n->counter)
#define INFO_COMP_SCHEDULERINIT(n) (n->info2)

/* reuse */
#define INFO_REUSE_WL_IDS(n) (n->info.ids)
#define INFO_REUSE_FUNDEF(n) (n->node[0])
#define INFO_REUSE_IDX(n) ((ids *)(n->node[1]))
#define INFO_REUSE_DEC_RC_IDS(n) ((ids *)(n->node[2]))
#define INFO_REUSE_MASK(n) (n->dfmask[0])
#define INFO_REUSE_NEGMASK(n) (n->dfmask[1])

/* optimize */
#define INFO_MASK(n, x) (n->mask[x])
#define INFO_VARNO(n) (n->varno)

/* generatemasks */
#define INFO_GNM_FUNDEF(n) (n->node[1])

/* inline */
#define INFO_INL_TYPE(n) (n->flag)
#define INFO_INL_LUT(n) ((LUT_t) (n->dfmask[0]))
#define INFO_INL_MODUL(n) (n->node[0])
#define INFO_INL_VARDECS(n) (n->node[1])
#define INFO_INL_PROLOG(n) (n->node[2])
#define INFO_INL_EPILOG(n) (n->node[3])
#define INFO_INL_ARG(n) (n->node[4])
#define INFO_INL_IDS(n) (n->info.ids)
#define INFO_INL_FUNDEF(n) (n->node[5])

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

/* DFR */
#define INFO_DFR_SPINE(n) (n->flag)

/*
 * Unrolling
 * these macros must be mapped to the same components
 * due to the shared usage of WLUnroll code.
 */
#define INFO_UNR_ASSIGN(n) (n->node[0])
#define INFO_UNR_FUNDEF(n) (n->node[1])

/* Print */
#define INFO_PRINT_CONT(n) ((node *)(n->info2))
#define INFO_PRINT_FUNDEF(n) (n->node[0])
#define INFO_PRINT_INT_SYN(n) (n->node[2])
#define INFO_PRINT_NWITH(n) (n->node[4])
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

/* inference of DF masks (infer_dfms.c) */
#define INFO_INFDFMS_FUNDEF(n) (n->node[0])
#define INFO_INFDFMS_IN(n) (n->dfmask[0])
#define INFO_INFDFMS_OUT(n) (n->dfmask[1])
#define INFO_INFDFMS_LOCAL(n) (n->dfmask[2])
#define INFO_INFDFMS_NEEDED(n) (n->dfmask[3])
#define INFO_INFDFMS_ISFIX(n) ((bool)(n->counter))
#define INFO_INFDFMS_FIRST(n) ((bool)(n->flag))
#define INFO_INFDFMS_HIDELOC(n) (n->varno) /* hide locals */

/* cleaning up declarations (cleanup_decls.c) */
#define INFO_CUD_FUNDEF(n) (n->node[0])
#define INFO_CUD_REF(n) (n->dfmask[0])

/* converting loops and conditionals to functions (lac2fun.c) */
#define INFO_L2F_FUNDEF(n) (n->node[0])
#define INFO_L2F_FUNS(n) (n->node[1])
#define INFO_L2F_ASSIGN(n) (n->node[2])

/* reconverting functions to loops and conditionals (fun2lac.c) */
#define INFO_F2L_FUNDEF(n) (n->node[0])
#define INFO_F2L_INLINED(n) (n->node[1])
#define INFO_F2L_LET(n) (n->node[2])

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

/* when used in SSATransform.c */
#define INFO_SSA_FUNDEF(n) (n->node[0])
#define INFO_SSA_BLOCK(n) (n->node[1])
#define INFO_SSA_RETINSTR(n) ((bool)(n->flag))
#define INFO_SSA_WITHVEC(n) ((node *)(n->node[2]))
#define INFO_SSA_WITHIDS(n) ((node *)(n->node[3]))
#define INFO_SSA_CONDSTMT(n) (n->node[4])
#define INFO_SSA_CONDSTATUS(n) (n->int_data)
#define INFO_SSA_ASSIGN(n) (n->node[5])
#define INFO_SSA_SINGLEFUNDEF(n) ((bool)(n->counter))

/* when used in UndoSSATransform.c */
#define INFO_USSA_ARGS(n) (n->node[0])
#define INFO_USSA_TOPBLOCK(n) (n->node[1])
#define INFO_USSA_FOLDTARGET(n) (n->node[2])
#define INFO_USSA_CONSTASSIGNS(n) (n->node[3])
#define INFO_USSA_OPASSIGN(n) (n->int_data)
#define INFO_USSA_MODUL(n) (n->node[4])

/* when used in SSADeadCodeRemoval.c */
#define INFO_SSADCR_DEPTH(n) (n->int_data)
#define INFO_SSADCR_REMASSIGN(n) ((bool)(n->flag))
#define INFO_SSADCR_FUNDEF(n) (n->node[0])
#define INFO_SSADCR_REMRESULTS(n) ((bool)(n->varno))
#define INFO_SSADCR_APFUNDEF(n) (n->node[1])
#define INFO_SSADCR_RESCOUNT(n) (n->counter)
#define INFO_SSADCR_RESNEEDED(n) (n->refcnt)
#define INFO_SSADCR_LET(n) (n->node[2])
#define INFO_SSADCR_ASSIGN(n) (n->node[3])
#define INFO_SSADCR_MODUL(n) (n->node[4])

/* when used in SSACSE.c */
#define INFO_SSACSE_REMASSIGN(n) ((bool)(n->flag))
#define INFO_SSACSE_FUNDEF(n) (n->node[0])
#define INFO_SSACSE_CSE(n) (n->node[1])
#define INFO_SSACSE_MODUL(n) (n->node[2])
#define INFO_SSACSE_ASSIGN(n) (n->node[3])
#define INFO_SSACSE_RECFUNAP(n) ((bool)(n->int_data))
#define INFO_SSACSE_RESULTARG(n) ((nodelist *)(n->node[5]))

/* when used in compare_tree.c */
#define INFO_CMPT_EQFLAG(n) ((cmptree_t) (n->flag))
#define INFO_CMPT_TREE(n) (n->node[0])

/* when used in annotate_fun_calls.c */
#define INFO_PF_FUNDEF(n) (n->node[0])

/* when used in SSAConstantFolding.c */
#define INFO_SSACF_REMASSIGN(n) ((bool)(n->flag))
#define INFO_SSACF_FUNDEF(n) (n->node[0])
#define INFO_SSACF_INSCONST(n) ((bool)(n->varno))
#define INFO_SSACF_POSTASSIGN(n) (n->node[1])
#define INFO_SSACF_TOPBLOCK(n) (n->node[2])
#define INFO_SSACF_RESULTS(n) (n->node[3])
#define INFO_SSACF_INLINEAP(n) ((bool)(n->counter))
#define INFO_SSACF_MODUL(n) (n->node[4])
#define INFO_SSACF_ASSIGN(n) (n->node[5])
#define INFO_SSACF_INLFUNDEF(n) ((bool)(n->refcnt))

/* when used in SSALIR.c */
#define INFO_SSALIR_FUNDEF(n) (n->node[0])
#define INFO_SSALIR_REMASSIGN(n) ((bool)(n->flag))
#define INFO_SSALIR_PREASSIGN(n) (n->node[1])
#define INFO_SSALIR_POSTASSIGN(n) (n->node[2])
#define INFO_SSALIR_MODUL(n) (n->node[3])
#define INFO_SSALIR_ASSIGN(n) (n->node[4])
#define INFO_SSALIR_NONLIRUSE(n) (n->int_data)
#define INFO_SSALIR_CONDSTATUS(n) (n->info.cint)
#define INFO_SSALIR_WITHDEPTH(n) (n->varno)
#define INFO_SSALIR_TOPBLOCK(n) ((bool)(n->counter))
#define INFO_SSALIR_FLAG(n) (n->flag)
#define INFO_SSALIR_EXTPREASSIGN(n) ((node *)(n->dfmask[0]))
#define INFO_SSALIR_EXTPOSTASSIGN(n) ((node *)(n->dfmask[1]))
#define INFO_SSALIR_MOVELUT(n) ((LUT_t) (n->dfmask[2]))
/* INFO_SSALIR_APARGCHAIN and INFO_SSALIR_APRESCHAIN can be mapped to one var */
#define INFO_SSALIR_APARGCHAIN(n) ((node *)(n->dfmask[3]))
#define INFO_SSALIR_APRESCHAIN(n) ((ids *)(n->dfmask[3]))
#define INFO_SSALIR_EXTFUNDEF(n) ((node *)(n->dfmask[4]))
#define INFO_SSALIR_RESULTMAP(n) ((nodelist *)(n->dfmask[5]))
#define INFO_SSALIR_MAXDEPTH(n) (n->refcnt)
#define INFO_SSALIR_SETDEPTH(n) (n->lineno)
#define INFO_SSALIR_INSLIST(n) ((nodelist *)(n->mask[0]))

/* when used in free.c */
#define INFO_FREE_FLAG(n) (n->node[0])
#define INFO_FREE_ASSIGN(n) (n->node[1])
/* when used in optimize.c */
#define INFO_OPT_MODUL(n) (n->node[0])

/* when used in CheckAvis.c */
#define INFO_CAV_FUNDEF(n) (n->node[0])
#define INFO_CAV_SINGLEFUNDEF(n) ((bool)(n->counter))

/*
 * when used in SSALUR.c:
 * INFO_SSALURASSIGN/FUNDEF must be mapped to the same componentes
 * due to the shared usage of old WLUnroll code */
#define INFO_SSALUR_ASSIGN(n) INFO_UNR_ASSIGN (n)
#define INFO_SSALUR_FUNDEF(n) INFO_UNR_FUNDEF (n)
#define INFO_SSALUR_MODUL(n) (n->node[2])
#define INFO_SSALUR_REMASSIGN(n) ((bool)(n->refcnt))
#define INFO_SSALUR_PREASSIGN(n) (n->node[3])

/* when used in SSAInferLI.c */
#define INFO_SSAILI_FUNDEF(n) (n->node[0])
#define INFO_SSAILI_ARGCHAIN(n) (n->node[1])

/* when used in insert_vardec.c */
#define INFO_INSVD_VARDECS(n) (n->node[0])
#define INFO_INSVD_ARGS(n) (n->node[1])

/* when used in WithloopScalarization.c */
#define INFO_WLS_POSSIBLE(n) (n->flag)
#define INFO_WLS_PHASE(n) (n->lineno)
#define INFO_WLS_FUNDEF(n) (n->node[0])
#define INFO_WLS_PARTS(n) (n->counter)

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
 ***    DFMmask_t  IN          (spmdinit -> )
 ***    DFMmask_t  OUT         (spmdinit -> )
 ***    DFMmask_t  INOUT       (spmdinit -> )
 ***    DFMmask_t  LOCAL       (spmdinit -> )
 ***    DFMmask_t  SHARED      (spmdinit -> )
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

#define SPMD_IN(n) (n->dfmask[0])
#define SPMD_INOUT(n) (n->dfmask[1])
#define SPMD_OUT(n) (n->dfmask[2])
#define SPMD_LOCAL(n) (n->dfmask[3])
#define SPMD_SHARED(n) (n->dfmask[4])

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
 ***    node*      WITH_PTRS   (N_exprs)   (syncinit -> syncopt -> compile !!)
 ***/

extern node *MakeSync (node *region);

#define SYNC_FIRST(n) (n->flag)
#define SYNC_LAST(n) (n->int_data)
#define SYNC_FOLDCOUNT(n) (n->varno)
#define SYNC_REGION(n) (n->node[0])

#define SYNC_IN(n) (n->dfmask[0])
#define SYNC_INOUT(n) (n->dfmask[1])
#define SYNC_OUT(n) (n->dfmask[2])
#define SYNC_OUTREP(n) (n->dfmask[3])
#define SYNC_LOCAL(n) (n->dfmask[4])

/*--------------------------------------------------------------------------*/

/***
 ***  N_mt :
 ***
 ***  sons:
 ***
 ***    node*      REGION     (N_block)
 ***
 ***  permanent attributes:
 ***
 ***    int        IDENTIFIER             Will be created by MakeMT, an copied
 ***                                      within DupTree, to identfy
 ***                                      corresponding blocks
 ***
 ***  temporary attributes:
 ***
 ***    DFMmask_t  USEMASK                (multithread.dfa -> )
 ***    DFMmask_t  DEFMASK                (multithread.dfa -> )
 ***    DFMmask_t  NEEDLATER              (multithread.dfa -> )
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
 ***    DFMmask_t  USEMASK        (multithread.dfa -> )
 ***    DFMmask_t  DEFMASK        (multithread.dfa -> )
 ***    DFMmask_t  NEEDLATER_ST   (multithread.dfa -> )
 ***    DFMmask_t  NEEDLATER_MT   (multithread.dfa -> )
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
 ***    node*      PRAGMA     (N_pragma) (scanparse -> wltransform !!)
 ***    int        REFERENCED            (wlt -> wlf !!)
 ***    int        REFERENCED_FOLD       (wlt -> wlf !!)
 ***    int        REFERENCES_FOLDED     (wlt -> wlf !!)
 ***    bool       FOLDABLE              (wlt -> wlf !!)
 ***    bool       NO_CHANCE             (wlt -> wlf !!)
 ***    ids*       DEC_RC_IDS            (refcount -> wltransform !!)
 ***    node*      TSI                   (tile size inference -> )
 ***
 ***    DFMmask_t  IN_MASK            (infer_dfms -> lac2fun -> wltransform !!)
 ***    DFMmask_t  OUT_MASK           (infer_dfms -> lac2fun -> wltransform !!)
 ***    DFMmask_t  LOCAL_MASK         (infer_dfms -> lac2fun -> wltransform !!)
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

#define NWITH_IN_MASK(n) ((n)->dfmask[0])
#define NWITH_OUT_MASK(n) ((n)->dfmask[1])
#define NWITH_LOCAL_MASK(n) ((n)->dfmask[2])

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
 ***    bool   COPY          (Unroll !!)
 ***/

extern node *MakeNPart (node *withid, node *generator, node *code);

#define NPART_WITHID(n) ((n)->node[0])
#define NPART_GEN(n) ((n)->node[1])
#define NPART_NEXT(n) ((n)->node[2])
#define NPART_CODE(n) ((n)->node[3])
#define NPART_COPY(n) ((bool)((n)->flag))
#define NPART_MASK(n, x) ((n)->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwithid :
 ***
 ***  permanent attributes:
 ***
 ***    ids*         VEC
 ***    ids*         IDS
 ***
 ***  remarks:
 ***
 ***    Even for N_Nwith-nodes with multiple parts all with-ids must have
 ***    identical names before phase 16 (with-loop transformation) can
 ***    be applied!!!
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
 ***
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
 ***    node*  SHAPE     ("N_expr": N_array, N_id)  (iff TYPE == WO_genarray)
 ***    node*  ARRAY     ("N_expr": N_array, N_id)  (iff TYPE == WO_modarray)
 ***    node*  NEUTRAL   ("N_expr")                 (otherwise)
 ***
 ***  permanent attributes:
 ***
 ***    WithOpType TYPE
 ***    char*      FUN                              (iff TYPE == WO_foldfun)
 ***    char*      MOD                              (iff TYPE == WO_foldfun)
 ***    prf        PRF                              (iff TYPE == WO_foldprf)
 ***
 ***  temporary attributes:
 ***
 ***    node*  EXPR                  (scanparse !!)
 ***    node*  FUNDEF    (N_fundef)  (typecheck -> precompile -> compile -> )
 ***    long*  MASK                  (optimize -> )
 ***
 ***  remarks:
 ***
 ***    - TYPE is WO_genarray, WO_modarray, WO_foldfun, WO_foldprf.
 ***    - FUNDEF is used if (TYPE == WO_foldfun, WO_foldprf).
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
 ***    int        ID                      (print !!)
 ***    long*      MASK                    (optimize -> )
 ***    node *     USE         (N_vinfo)   (IVE -> )
 ***    bool       FLAG                    (WLI -> WLF)
 ***    ids*       INC_RC_IDS              (refcount -> compile -> )
 ***
 ***    node*      WLAA_INFO(n)            (wlaa -> )
 ***    access_t*  WLAA_ACCESS             (wlaa -> )
 ***    int        WLAA_ACCESSCNT(n)       (wlaa -> )
 ***    int        WLAA_ARRAYDIM(n)        (wlaa -> )
 ***    feature_t* WLAA_FEATURE            (wlaa -> )
 ***    shpseg*    WLAA_ARRAYSHP(n)        (wlaa -> )
 ***    node*      WLAA_WLARRAY(n)         (wlaa -> )
 ***    shpseg*    TSI_TILESHP(n)          (tsi  -> )
 ***
 ***    bool       AP_DUMMY_CODE           (ap -> wltransform -> compile -> )
 ***
 ***  remarks:
 ***
 ***    For *fold* with-loops with multiple codes all NCODE_CEXPR must have
 ***    identical names before phase 19 (pre-compilation) can be applied!!!
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
#define NCODE_INC_RC_IDS(n) ((ids *)((n)->dfmask[0]))
#define NCODE_USE(n) ((n)->node[3])
#define NCODE_USED(n) ((n)->info.cint)
#define NCODE_MASK(n, x) ((n)->mask[x])
#define NCODE_ID(n) ((n)->refcnt)
#define NCODE_FLAG(n) ((bool)((n)->flag))

#define NCODE_WLAA_INFO(n) ((node *)((n)->info2))
#define NCODE_WLAA_ACCESS(n) ((access_t *)(((node *)((n)->info2))->info2))
#define NCODE_WLAA_ACCESSCNT(n) (((node *)((n)->info2))->counter)
#define NCODE_WLAA_FEATURE(n) (((node *)((n)->info2))->varno)
#define NCODE_WLAA_INDEXVAR(n) (((node *)((n)->info2))->node[2])
#define NCODE_WLAA_WLARRAY(n) (((node *)((n)->info2))->node[3])

#define NCODE_TSI_TILESHP(n) ((shpseg *)(((node *)((n)->info2))->node[4]))

#define NCODE_AP_DUMMY_CODE(n) ((bool)((n)->int_data))

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
 ***    bool       MT
 ***    node*      PRAGMA        (N_pragma)
 ***
 ***  temporary attributes:
 ***
 ***    DFMmask_t  IN_MASK              (infer_dfms -> lac2fun -> refcount -> )
 ***    DFMmask_t  OUT_MASK             (infer_dfms -> lac2fun -> refcount -> )
 ***    DFMmask_t  LOCAL_MASK           (infer_dfms -> lac2fun -> refcount -> )
 ***
 ***    DFMmask_t  REUSE                (ReuseWithArrays -> compile !!)
 ***
 ***    ids*       DEC_RC_IDS           (wltransform -> compile -> )
 ***
 ***    bool       OFFSET_NEEDED        (wltransform -> compile -> )
 ***
 ***    bool       ISSCHEDULED          (new_mt -> ...)
 ***                          [is segment scheduled or not?]
 ***/

extern node *MakeNWith2 (node *withid, node *seg, node *code, node *withop, int dims);

#define NWITH2_DIMS(n) ((n)->flag)
#define NWITH2_MT(n) ((bool)((n)->counter))
#define NWITH2_WITHID(n) ((n)->node[0])
#define NWITH2_SEGS(n) ((n)->node[1])
#define NWITH2_CODE(n) ((n)->node[2])
#define NWITH2_WITHOP(n) ((n)->node[3])
#define NWITH2_PRAGMA(n) ((n)->node[4])

#define NWITH2_DEC_RC_IDS(n) ((ids *)((n)->node[5]))
#define NWITH2_OFFSET_NEEDED(n) ((bool)((n)->int_data))
#define NWITH2_ISSCHEDULED(n) ((n)->info.prf_dec.tag)

#define NWITH2_IN_MASK(n) ((n)->dfmask[0])
#define NWITH2_OUT_MASK(n) ((n)->dfmask[1])
#define NWITH2_LOCAL_MASK(n) ((n)->dfmask[2])
#define NWITH2_REUSE(n) ((n)->dfmask[3])

/*--------------------------------------------------------------------------*/

/*
 * Here are some macros for N_WL... nodes
 *
 * CAUTION: Not every macro is suitable for all node types.
 *          e.g. NEXTDIM is not a son of N_WLstride nodes
 *
 *          It would be better to contruct these macros like this:
 *            #define WLNODE_NEXTDIM(n) ((NODE_TYPE(n) == N_WLstride) ?
 *                                        DBUG_ASSERT(...) :
 *                                        (NODE_TYPE(n) == N_WLblock) ?
 *                                         WLBLOCK_NEXTDIM(n) :
 *                                         (NODE_TYPE(n) == N_WLublock) ?
 *                                          WLUBLOCK_NEXTDIM(n) : ...)
 *          but unfortunately this is not a modifiable l-value in ANSI-C :(
 *          so it would be impossible to use them on the left side of an
 *          assignment.
 */

#define WLNODE_LEVEL(n) ((n)->int_data)
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
#define WLSEGX_SCHEDULING(n) ((SCHsched_t *)((n)->info2))
#define WLSEGX_TASKSEL(n) ((SCHtasksel_t *)((n)->node[4]))

/*
 * some macros for N_WLblock, N_WLublock nodes
 */

#define WLXBLOCK_LEVEL(n) (WLNODE_LEVEL (n))
#define WLXBLOCK_DIM(n) (WLNODE_DIM (n))
#define WLXBLOCK_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLXBLOCK_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLXBLOCK_STEP(n) (WLNODE_STEP (n))
#define WLXBLOCK_NEXTDIM(n) (WLNODE_NEXTDIM (n))
#define WLXBLOCK_CONTENTS(n) ((n)->node[2])
#define WLXBLOCK_NEXT(n) (WLNODE_NEXT (n))

/*
 * some macros for N_WLstride, N_WLstrideVar nodes
 */

#define WLSTRIDEX_LEVEL(n) (WLNODE_LEVEL (n))
#define WLSTRIDEX_DIM(n) (WLNODE_DIM (n))
#define WLSTRIDEX_CONTENTS(n) ((n)->node[0])
#define WLSTRIDEX_NEXT(n) (WLNODE_NEXT (n))

/*
 * some macros for N_WLgrid, N_WLgridVar nodes
 */

#define WLGRIDX_LEVEL(n) (WLNODE_LEVEL (n))
#define WLGRIDX_DIM(n) (WLNODE_DIM (n))
#define WLGRIDX_FITTED(n) ((bool)((n)->varno))
#define WLGRIDX_NEXTDIM(n) (WLNODE_NEXTDIM (n))
#define WLGRIDX_NEXT(n) (WLNODE_NEXT (n))
#define WLGRIDX_CODE(n) ((n)->node[4])
#define WLGRIDX_NOOP(n) ((bool)((n)->info.prf_dec.tag))

/*--------------------------------------------------------------------------*/

/***
 *** N_WLseg :
 ***
 ***  sons:
 ***
 ***    node*      CONTENTS      (N_WLblock, N_WLublock, N_WLstride)
 ***    node*      NEXT          (N_WLseg, N_WLsegVar)
 ***
 ***  permanent attributes:
 ***
 ***    int        DIMS       [number of dims]
 ***
 ***    int*       IDX_MIN    [minimal index vector]
 ***    int*       IDX_MAX    [maximal index vector]
 ***
 ***    int*       UBV        [unrolling-blocking vector]
 ***
 ***    int        BLOCKS     [number of blocking levels (0..3)
 ***                            --- without unrolling-blocking]
 ***    int*       BV[]       [blocking vectors]
 ***
 ***  temporary attributes:
 ***
 ***    int*       SV         [step vector]        (wltransform -> )
 ***    int*       HOMSV      [hom. step vector]   (wltransform -> )
 ***
 ***    SCHsched_t   SCHEDULING                    (wltransform -> compile -> )
 ***    SCHtasksel_t TASKSEL                       (wltransform -> compile -> )
 ***
 ***  remarks:
 ***
 ***    - BV[ 0 .. (BLOCKS-1) ]
 ***    - UBV, BV[.], SV, IDX_MIN, IDX_MAX are vectors of size DIMS.
 ***    - SV is the least common multiple of all stride-, ublock- and block-
 ***      steps found in the segment.
 ***    - HOMSV[d] equals SV[d] iff the segment is homogeneous in dimension 'd'.
 ***      HOMSV[d] equals 0 iff the segment is inhomogeneous in dimension 'd'.
 ***/

extern node *MakeWLseg (int dims, node *contents, node *next);

#define WLSEG_DIMS(n) (WLSEGX_DIMS (n))
#define WLSEG_CONTENTS(n) (WLSEGX_CONTENTS (n))
#define WLSEG_NEXT(n) (WLSEGX_NEXT (n))

#define WLSEG_UBV(n) ((int *)((n)->dfmask[0]))
#define WLSEG_BLOCKS(n) ((n)->flag)
#define WLSEG_BV(n, level) ((int *)((n)->dfmask[level + 1]))

#define WLSEG_SV(n) ((int *)((n)->mask[0]))
#define WLSEG_HOMSV(n) ((int *)((n)->mask[1]))
#define WLSEG_IDX_MIN(n) (*((int **)(&((n)->node[2])))) /* needed for cc */
#define WLSEG_IDX_MAX(n) (*((int **)(&((n)->node[3])))) /* needed for cc */

#define WLSEG_SCHEDULING(n) (WLSEGX_SCHEDULING (n))
#define WLSEG_TASKSEL(n) (WLSEGX_TASKSEL (n))

/*--------------------------------------------------------------------------*/

/***
 *** N_WLsegVar :
 ***
 ***  sons:
 ***
 ***    node*      CONTENTS      (N_WLstride, N_WLstrideVar)
 ***    node*      NEXT          (N_WLseg, N_WLsegVar)
 ***
 ***  permanent attributes:
 ***
 ***    int        DIMS       [number of dims]
 ***
 ***    node**     IDX_MIN       (N_num, N_id)
 ***    node**     IDX_MAX       (N_num, N_id)
 ***
 ***  temporary attributes:
 ***
 ***    SCHsched_t   SCHEDULING                   (wltransform -> compile -> )
 ***    SCHtasksel_t TASKSEL                      (wltransform -> compile -> )
 ***
 ***  remarks:
 ***
 ***    - IDX_MIN, IDX_MAX are vectors of size DIMS.
 ***/

extern node *MakeWLsegVar (int dims, node *contents, node *next);

#define WLSEGVAR_DIMS(n) (WLSEGX_DIMS (n))
#define WLSEGVAR_CONTENTS(n) (WLSEGX_CONTENTS (n))
#define WLSEGVAR_NEXT(n) (WLSEGX_NEXT (n))

#define WLSEGVAR_IDX_MIN(n) ((node **)((n)->node[2]))
#define WLSEGVAR_IDX_MAX(n) ((node **)((n)->node[3]))

#define WLSEGVAR_SCHEDULING(n) (WLSEGX_SCHEDULING (n))
#define WLSEGVAR_TASKSEL(n) (WLSEGX_TASKSEL (n))

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

#define WLBLOCK_LEVEL(n) (WLXBLOCK_LEVEL (n))
#define WLBLOCK_DIM(n) (WLXBLOCK_DIM (n))
#define WLBLOCK_BOUND1(n) (WLXBLOCK_BOUND1 (n))
#define WLBLOCK_BOUND2(n) (WLXBLOCK_BOUND2 (n))
#define WLBLOCK_STEP(n) (WLXBLOCK_STEP (n))
#define WLBLOCK_NEXTDIM(n) (WLXBLOCK_NEXTDIM (n))
#define WLBLOCK_CONTENTS(n) (WLXBLOCK_CONTENTS (n))
#define WLBLOCK_NEXT(n) (WLXBLOCK_NEXT (n))

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

#define WLUBLOCK_LEVEL(n) (WLXBLOCK_LEVEL (n))
#define WLUBLOCK_DIM(n) (WLXBLOCK_DIM (n))
#define WLUBLOCK_BOUND1(n) (WLXBLOCK_BOUND1 (n))
#define WLUBLOCK_BOUND2(n) (WLXBLOCK_BOUND2 (n))
#define WLUBLOCK_STEP(n) (WLXBLOCK_STEP (n))
#define WLUBLOCK_NEXTDIM(n) (WLXBLOCK_NEXTDIM (n))
#define WLUBLOCK_CONTENTS(n) (WLXBLOCK_CONTENTS (n))
#define WLUBLOCK_NEXT(n) (WLXBLOCK_NEXT (n))

/*--------------------------------------------------------------------------*/

/***
 *** N_WLstride :
 ***
 ***  sons:
 ***
 ***    node*    CONTENTS         (N_WLgrid, N_WLgridVar)
 ***    node*    NEXT             (N_WLstride, N_WLstrideVar)
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
 ***                                                 (wltransform !!)
 ***    node*    MODIFIED                            (wltransform !!)
 ***
 ***/

extern node *MakeWLstride (int level, int dim, int bound1, int bound2, int step,
                           bool unrolling, node *contents, node *next);

#define WLSTRIDE_LEVEL(n) (WLSTRIDEX_LEVEL (n))
#define WLSTRIDE_DIM(n) (WLSTRIDEX_DIM (n))
#define WLSTRIDE_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLSTRIDE_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLSTRIDE_STEP(n) (WLNODE_STEP (n))
#define WLSTRIDE_UNROLLING(n) ((bool)((n)->info.prf_dec.tc))
#define WLSTRIDE_CONTENTS(n) (WLSTRIDEX_CONTENTS (n))
#define WLSTRIDE_NEXT(n) (WLSTRIDEX_NEXT (n))

#define WLSTRIDE_PART(n) ((n)->node[2])
#define WLSTRIDE_MODIFIED(n) ((n)->node[3])

/*--------------------------------------------------------------------------*/

/***
 *** N_WLstrideVar :
 ***
 ***  sons:
 ***
 ***    node*    BOUND1          (N_num, N_id)
 ***    node*    BOUND2          (N_num, N_id)
 ***    node*    STEP            (N_num, N_id)
 ***    node*    CONTENTS        (N_WLgrid, N_WLgridVar)
 ***    node*    NEXT            (N_WLstride, N_WLstrideVar)
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

extern node *MakeWLstrideVar (int level, int dim, node *bound1, node *bound2, node *step,
                              node *contents, node *next);

#define WLSTRIDEVAR_LEVEL(n) (WLSTRIDEX_LEVEL (n))
#define WLSTRIDEVAR_DIM(n) (WLSTRIDEX_DIM (n))
#define WLSTRIDEVAR_BOUND1(n) ((n)->node[2])
#define WLSTRIDEVAR_BOUND2(n) ((n)->node[3])
#define WLSTRIDEVAR_STEP(n) ((n)->node[4])
#define WLSTRIDEVAR_CONTENTS(n) (WLSTRIDEX_CONTENTS (n))
#define WLSTRIDEVAR_NEXT(n) (WLSTRIDEX_NEXT (n))

/*--------------------------------------------------------------------------*/

/***
 *** N_WLgrid :
 ***
 ***  sons:
 ***
 ***    node*   NEXTDIM     (N_WLblock, N_WLublock, N_WLstride, N_WLstrideVar)
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
 ***    bool    FITTED
 ***
 ***    bool    NOOP
 ***
 ***  temporary attributes:
 ***
 ***    node*   MODIFIED                                (wltransform !!)
 ***
 ***  remarks:
 ***
 ***    - It makes no sense to use the nodes NEXTDIM and CODE simultaneous!
 ***    - (NEXTDIM == NULL) *and* (CODE == NULL) means that this is a dummy
 ***      grid, representing a simple init- (genarray), copy- (modarray),
 ***      noop- (fold) operation.
 ***      If also (NOOP == TRUE) is hold, this dummy grid represents a noop
 ***      even in genarray/modarray with-loops (-> naive compilation,
 ***      array padding, ...) !!
 ***/

extern node *MakeWLgrid (int level, int dim, int bound1, int bound2, bool unrolling,
                         node *nextdim, node *next, node *code);

#define WLGRID_LEVEL(n) (WLGRIDX_LEVEL (n))
#define WLGRID_DIM(n) (WLGRIDX_DIM (n))
#define WLGRID_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLGRID_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLGRID_UNROLLING(n) ((bool)((n)->info.prf_dec.tc))
#define WLGRID_FITTED(n) (WLGRIDX_FITTED (n))
#define WLGRID_NEXTDIM(n) (WLGRIDX_NEXTDIM (n))
#define WLGRID_NEXT(n) (WLGRIDX_NEXT (n))
#define WLGRID_CODE(n) (WLGRIDX_CODE (n))
#define WLGRID_NOOP(n) (WLGRIDX_NOOP (n))

#define WLGRID_MODIFIED(n) ((n)->node[2])

/*--------------------------------------------------------------------------*/

/***
 *** N_WLgridVar :
 ***
 ***  sons:
 ***
 ***    node*    BOUND1          (N_num, N_id)
 ***    node*    BOUND2          (N_num, N_id)
 ***    node*    NEXTDIM         (N_WLstride, N_WLstrideVar)
 ***    node*    NEXT            (N_WLgrid, N_WLgridVar)
 ***
 ***  permanent attributes:
 ***
 ***    node*    CODE            (N_Ncode)
 ***    int      LEVEL
 ***    int      DIM
 ***    bool     FITTED
 ***
 ***    bool     NOOP
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
 ***      If also (NOOP == TRUE) is hold, this dummy grid represents a noop
 ***      even in genarray/modarray with-loops (-> naive compilation,
 ***      array padding, ...) !!
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
#define WLGRIDVAR_FITTED(n) (WLGRIDX_FITTED (n))
#define WLGRIDVAR_NOOP(n) (WLGRIDX_NOOP (n))

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

/***
 ***  N_ok :
 ***
 ***  sons:
 ***
 ***  permanent attributes:
 ***
 ***  temporary attributes:
 ***
 ***/

/*
 * This special node is used to mark the upper limit of integers associated
 * with node types. It is also used by the typechecker for various purposes.
 */

extern node *MakeOk ();

/*--------------------------------------------------------------------------*/

#endif /* _sac_tree_basic_h */
