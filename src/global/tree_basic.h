/*
 *
 * $Log$
 * Revision 1.67  1998/02/09 16:07:51  srs
 * changed N_Nwithid and introduced new
 * macros for N_info
 *
 * Revision 1.66  1998/02/05 16:25:16  srs
 * added REFCNT to N_pre and N_post.
 * added FUNDEF to  N_Nwithop.
 *
 * Revision 1.65  1998/01/30 17:54:15  srs
 * changed node structure of N_Nwithid
 *
 * Revision 1.64  1997/12/20 17:33:24  srs
 * changed comment for N_vardec
 *
 * Revision 1.63  1997/12/10 18:45:33  srs
 * *** empty log message ***
 *
 * Revision 1.62  1997/12/02 18:47:10  srs
 * changed comments
 *
 * Revision 1.61  1997/11/24 17:05:06  sbs
 * Nwithop modified :
 * FUN points to funname.id
 * and MOD points to funname.modid
 *
 * Revision 1.60  1997/11/22 23:32:14  dkr
 * uups! it is not recommended to use c-comments in this message ... :(
 *
 * Revision 1.59  1997/11/22 23:26:03  dkr
 * "N_NCode :" -> "N_Ncode :"
 *
 * Revision 1.58  1997/11/21 09:45:15  srs
 * *** empty log message ***
 *
 * Revision 1.57  1997/11/18 18:05:43  srs
 * changed new WL-macros
 *
 * Revision 1.56  1997/11/18 17:33:31  dkr
 * modified MakeNWith()
 *
 * Revision 1.55  1997/11/13 16:16:00  srs
 * created access macros for new WL-syntaxtree
 *
 * Revision 1.54  1997/05/28 12:36:51  sbs
 * Profiling integrated
 *
 * Revision 1.53  1997/05/16  09:54:01  sbs
 * ANALSE-TOOL extended to function-application specific timing
 *
 * Revision 1.52  1997/05/14  08:16:43  sbs
 * N_annotate added
 *
 * Revision 1.51  1997/05/02  13:52:39  sbs
 * SHAPES_SHPSEG inserted
 *
 * Revision 1.50  1997/04/30  11:49:29  cg
 * new macro VARDEC_OBJDEF
 *
 * Revision 1.49  1997/03/19  13:38:16  cg
 * Added new data type 'deps' with respective access macros and
 * creation function
 *
 * Revision 1.48  1996/02/11  20:19:01  sbs
 * some minor corrections on stuff concerning N_vinfo,
 * VARDEC_ACTCHN, VARDEC_COLCHN, ARG_ACTCHN, and ARG_COLCHN added.
 *
 * Revision 1.47  1996/01/26  15:29:02  cg
 * added macro ID_MAKEUNIQUE(n)
 *
 * Revision 1.46  1996/01/25  18:39:05  cg
 * added macro MODUL_CLASSTYPE(n)
 *
 * Revision 1.45  1996/01/22  17:24:40  cg
 * added macro OBJDEC_DEF(n)
 *
 * Revision 1.44  1996/01/22  14:44:44  asi
 * added ASSIGN_CSE
 *
 * Revision 1.43  1996/01/22  14:37:45  cg
 * modified some macros for N_objdef nodes
 *
 * Revision 1.42  1996/01/22  14:04:50  asi
 * added MakeId2
 *
 * Revision 1.41  1996/01/22  09:03:48  cg
 * added some new macros for N_objdef node
 *
 * Revision 1.40  1996/01/16  16:55:15  cg
 * added some macros for N_info nodes
 *
 * Revision 1.39  1996/01/15  11:06:07  asi
 * added ID_DEF
 *
 * Revision 1.38  1996/01/12  15:53:00  asi
 * added LET_VARDEC
 *
 * Revision 1.37  1996/01/12  14:56:36  cg
 * added some new macros for N_info node, corrected ID_REFCNT.
 *
 * Revision 1.36  1996/01/09  12:04:59  cg
 * added macro ARG_REFCNT
 *
 * Revision 1.35  1996/01/07  16:54:49  cg
 * modified some pragma-related macros
 *
 * Revision 1.34  1996/01/05  14:33:22  cg
 * removed macros MODARRAY_RETURN, GENARRAY_RETURN,
 * FOLDFUN_RETURN, FOLDPRF_RETURN
 *
 * Revision 1.33  1996/01/05  14:14:10  asi
 * added While2Do: This function converts a while-loop into a do-loop
 *
 * Revision 1.32  1996/01/02  14:44:35  cg
 * macro MODUL_CCCALL removed again
 *
 * Revision 1.31  1995/12/29  14:22:41  cg
 * added macro MODUL_CCCALL
 *
 * Revision 1.30  1995/12/29  10:31:18  cg
 * new macros for N_sib and N_info nodes, added new macros to restore
 * the new with-loop syntax from the internal representation.
 * added macros LINKMOD for fundef and objdef nodes
 *
 * Revision 1.29  1995/12/21  13:24:16  asi
 * added OPERATOR_MASK for N_genarray, N_modarray, N_foldprf and N_foldfun - nodes
 *
 * Revision 1.28  1995/12/21  10:07:33  cg
 * now MakePragma has no argument at all, new macros PRAGMA_LINKSIGNNUMS etc.
 * for temporary storage of pragma as list of nums instead of array.
 *
 * Revision 1.27  1995/12/20  08:15:10  cg
 * renamed macro WITH_BODY to WITH_OPERTATOR
 * new macros for new N_char node
 * changed macros for N_pragma node with respect to using arrays for pragmas linksign,
 * refcounting, and readonly.
 *
 * Revision 1.26  1995/12/19  13:38:46  cg
 * renamed macro WITH_BODY to WITH_OPERATOR
 * added macros for new N_char node
 *
 * Revision 1.25  1995/12/18  18:27:11  cg
 * added macro OBJDEF_ICM
 *
 * Revision 1.24  1995/12/13  13:32:46  asi
 * added ASSIGN_STATUS, modified ICM_NAME
 *
 * Revision 1.23  1995/12/13  09:38:26  cg
 * modified macro RETURN_REFERENCE(n)
 *
 * Revision 1.22  1995/12/12  18:26:02  asi
 * added VARDEC_FLAG
 *
 * Revision 1.21  1995/12/07  16:23:29  asi
 * comment added for INLREC
 *
 * Revision 1.20  1995/12/04  15:03:58  asi
 * added temporary attribute FUNDEF_INLREC(n)
 *
 * Revision 1.19  1995/12/01  20:26:14  cg
 * removed macro OBJDEF_INITFUN
 *
 * Revision 1.18  1995/12/01  17:09:20  cg
 * added new node type N_pragma
 * removed macro FUNDEF_ALIAS
 *
 * Revision 1.17  1995/11/16  19:40:45  cg
 * Macros for accessing Masks modified,
 * old style macros for masks moved to tree_compound.h
 *
 * Revision 1.16  1995/11/06  09:22:21  cg
 * added macro VARDEC_ATTRIB
 *
 * Revision 1.15  1995/11/02  13:13:31  cg
 * added new macros OBJDEF_ARG(n) and renamed IDS_DECL(i)
 * to IDS_VARDEC(i).
 *
 * Revision 1.14  1995/11/01  16:27:04  cg
 * added some comments
 *
 * Revision 1.13  1995/11/01  07:10:12  sbs
 * neutral addded to N_foldprf;
 *
 * Revision 1.12  1995/10/30  09:53:35  cg
 * added temporary attribute FUNDEF_INLINE(n)
 *
 * Revision 1.11  1995/10/26  15:58:58  cg
 * new macro MODUL_STORE_IMPORTS(n)
 *
 * Revision 1.10  1995/10/22  17:33:12  cg
 * new macros FUNDEC_DEF and TYPEDEC_DEF
 * new slot DECL of N_modul node to store declaration when compiling
 * a module/class implementation
 *
 * Revision 1.9  1995/10/20  11:29:43  cg
 * *** empty log message ***
 *
 * Revision 1.8  1995/10/19  11:16:00  cg
 * bug in macro AP_FUNDEF fixed.
 *
 * Revision 1.7  1995/10/19  10:06:42  cg
 * new slots for VARDEC nodes: VARDEC_STATUS and VARDEC_TYPEDEF
 *
 * Revision 1.6  1995/10/16  13:00:38  cg
 * new macro OBJDEF_VARNAME added.
 *
 * Revision 1.5  1995/10/12  13:45:15  cg
 * new macros TYPEDEF_STATUS, FUNDEF_STATUS and OBJDEF_STATUS to mark
 * imported items
 *
 * Revision 1.4  1995/10/12  12:25:19  cg
 * bug in N_block macros fixed
 *
 * Revision 1.3  1995/10/06  17:16:50  cg
 * basic access facilities for new type nodelist added
 * IDS structure modified to store global objects.
 * MakeIds extended to 3 parameters
 * basic facilities for compiler steps obj-analysis and fun-analysis added.
 *
 * Revision 1.2  1995/09/29  17:50:51  cg
 * new access structures for strings, nums, shpseg.
 * shape handling modified.
 *
 * Revision 1.1  1995/09/27  15:13:12  cg
 * Initial revision
 *
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

/* Uncomment the #define statement to use new virtual syntaxtree
 *
 * #define NEWTREE
 */

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

extern nodelist *MakeNodelist (node *node, statustype status, nodelist *next);

#define NODELIST_NODE(n) (n->node)
#define NODELIST_ATTRIB(n) (n->attrib)
#define NODELIST_STATUS(n) (n->status)
#define NODELIST_NEXT(n) (n->next)

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
 ***/

/*
 *  CLASSTYPE points to the type of a class implementation.
 *
 *  The temporary attributes DECL and STORE_IMPORTS are mapped
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
#define MODUL_CLASSTYPE(n) ((types *)n->node[5])

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
#define MODDEC_LINKWITH(n) ((deps *)n->info.fun_name.id_mod)
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
#define CLASSDEC_LINKWITH(n) ((deps *)n->info.fun_name.id_mod)
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
#define SIB_LINKWITH(n) ((deps *)n->info.fun_name.id_mod)
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
#define IMPLIST_ITYPES(n) ((ids *)n->node[1])
#define IMPLIST_ETYPES(n) ((ids *)n->node[2])
#define IMPLIST_OBJS(n) ((ids *)n->node[4])
#define IMPLIST_FUNS(n) ((ids *)n->node[3])
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
#define TYPEDEF_COPYFUN(n) ((char *)n->node[3])
#define TYPEDEF_FREEFUN(n) ((char *)n->node[4])

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
#define OBJDEF_VARNAME(n) ((char *)(n->node[2]))
#define OBJDEF_ARG(n) (n->node[3])
#define OBJDEF_PRAGMA(n) (n->node[4])
#define OBJDEF_ICM(n) (n->node[3])
#define OBJDEF_NEEDOBJS(n) ((nodelist *)n->node[5])
#define OBJDEF_SIB(n) (n->node[3])

#define OBJDEC_DEF(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_fundef :
 ***
 ***  sons:
 ***
 ***    node*       BODY     (O)  (N_block)
 ***    node*       ARGS     (O)  (N_arg)
 ***    node*       NEXT     (O)  (N_fundef)
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       MOD      (O)
 ***    char*       LINKMOD  (O)
 ***    types*      TYPES
 ***    statustype  STATUS
 ***    statustype  ATTRIB
 ***    int         INLINE
 ***    int         FUNNO
 ***    node*       PRAGMA   (O)  (N_pragma)
 ***
 ***  temporary attributes:
 ***
 ***    node*      SIB      (O)  (N_sib)     (readsib !!)
 ***    node*      RETURN        (N_return)  (typecheck -> compile !!)
 ***    nodelist*  NEEDOBJS (O)              (import -> )
 ***                                         (analysis -> )
 ***                                         ( -> analysis -> )
 ***                                         ( -> write-SIB -> )
 ***                                         ( -> obj-handling -> )
 ***    node*      ICM           (N_icm)     (compile -> )
 ***    int        VARNO                     (optimize -> )
 ***    long*      MASK[x]                   (optimize -> )
 ***    int        INLREC                    (inl !!)
 ***
 ***    node*      FUNDEC_DEF (O) (N_fundef) (checkdec -> writesib !!)
 ***/

/*
 *  STATUS: ST_regular      function defined in this module
 *          ST_objinitfun   generic function for object initialization
 *          ST_imported     imported function (maybe declaration only)
 *          ST_generic      class conversion function
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
#define ARG_TYPESTRING(n) ((char *)n->node[1])
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
 ***/

extern node *MakeBlock (node *instr, node *vardec);

#define BLOCK_INSTR(n) (n->node[0])
#define BLOCK_VARDEC(n) (n->node[1])
#define BLOCK_MASK(n, x) (n->mask[x])
#define BLOCK_NEEDFUNS(n) ((nodelist *)(n->node[2]))
#define BLOCK_NEEDTYPES(n) ((nodelist *)(n->node[3]))

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
 ***    node*  CSE                        (cse !!)
 ***
 ***/

extern node *MakeAssign (node *instr, node *next);

#define ASSIGN_INSTR(n) (n->node[0])
#define ASSIGN_NEXT(n) (n->node[1])
#define ASSIGN_CSE(n) (n->node[2])
#define ASSIGN_MASK(n, x) (n->mask[x])
#define ASSIGN_STATUS(n) (n->flag)

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
 ***  temporary attributes:
 ***
 ***    node*  VARDEC    (N_vardec)  (typecheck -> )
 ***
 ***/

extern node *MakeLet (node *expr, ids *ids);

#define LET_EXPR(n) (n->node[0])
#define LET_IDS(n) (n->info.ids)
#define LET_VARDEC(n) (n->info.ids->node)

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
 ***    node*  THENVARS     (N_id)      (refcount -> compile -> )
 ***    node*  ELSEVARS     (N_id)      (refcount -> compile -> )
 ***    long*  MASK[x]                  (optimize -> )
 ***/

extern node *MakeCond (node *cond, node *Then, node *Else);

#define COND_COND(n) (n->node[0])
#define COND_THEN(n) (n->node[1])
#define COND_ELSE(n) (n->node[2])
#define COND_THENVARS(n) (n->node[3]->node[0])
#define COND_ELSEVARS(n) (n->node[3]->node[1])
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
 ***    node*  USEVARS    (N_id)      (refcount -> compile -> )
 ***    node*  DEFVARS    (N_id)      (refcount -> compile -> )
 ***    long*  MASK[x]                (optimize -> )
 ***/

extern node *MakeDo (node *cond, node *body);

#define DO_COND(n) (n->node[0])
#define DO_BODY(n) (n->node[1])
#define DO_USEVARS(n) (n->node[2]->node[0])
#define DO_DEFVARS(n) (n->node[2]->node[1])
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
 ***    node*  USEVARS    (N_id)      (refcount -> compile -> )
 ***    node*  DEFVARS    (N_id)      (refcount -> compile -> )
 ***    long*  MASK[x]                (optimize -> )
 ***/

extern node *MakeWhile (node *cond, node *body);

extern node *While2Do (node *while_node);

#define WHILE_COND(n) (n->node[0])
#define WHILE_BODY(n) (n->node[1])
#define WHILE_USEVARS(n) (n->node[2]->node[0])
#define WHILE_DEFVARS(n) (n->node[2]->node[1])
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
 ***                                    ( -> obj-handling -> )
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
 ***
 ***
 ***  remarks (srs): the 'body' of MakeWith() can be reached by WITH_OPERATOR.
 ***    In MakeWith() a 3rd (!!!) son is initialized.
 ***/

extern node *MakeWith (node *gen, node *body);

#define WITH_GEN(n) (n->node[0])
#define WITH_OPERATOR(n) (n->node[1])
#define WITH_MASK(n, x) (n->mask[x])

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
 ***    node*  ARRAY  (N_array)
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
 ***    node*  AELEMS   (N_exprs)
 ***
 ***  temporary attributes:
 ***
 ***    types* TYPE               (typecheck -> )
 ***/

extern node *MakeArray (node *aelems);

#define ARRAY_AELEMS(n) (n->node[0])
#define ARRAY_TYPE(n) (n->info.types)

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
#define VINFO_TYPE(n) ((types *)n->node[1])
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
 ***
 ***  temporary attributes:
 ***
 ***    node*  VARDEC    (N_vardec/N_arg)  (typecheck -> )
 ***    node*  OBJDEF    (N_objdef)        (typecheck -> )
 ***                                       ( -> analysis -> )
 ***    int    REFCNT                      (refcount -> compile -> )
 ***    int    MAKEUNIQUE                  (precompile -> compile -> )
 ***    node*  DEF                         (Unroll !, Unswitch !)
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

#define ID_NAME(n) (n->info.ids->id)
#define ID_DEF(n) (n->info.ids->def)
#define ID_VARDEC(n) (n->info.ids->node)
#define ID_REFCNT(n) (n->refcnt)
#define ID_MOD(n) (n->info.ids->mod)
#define ID_ATTRIB(n) (n->info.ids->attrib)
#define ID_STATUS(n) (n->info.ids->status)
#define ID_OBJDEF(n) (n->info.ids->node)
#define ID_MAKEUNIQUE(n) (n->flag)

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
 ***/

extern node *MakeIcm (char *name, node *args, node *next);

#define ICM_NAME(n) (n->info.fun_name.id)
#define ICM_ARGS(n) (n->node[0])
#define ICM_NEXT(n) (n->node[1])

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
 *  An objdef pragma may contain LINKNAME only.
 *  And a fundef pragma may contain all pragmas except COPYFUN and FREEFUN,
 *  but LINKMOD, TYPES and FUNS are only for internal use in SIBS.
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
#define PRAGMA_LINKSIGN(n) ((int *)n->mask[0])
#define PRAGMA_LINKSIGNNUMS(n) ((nums *)n->mask[0])
#define PRAGMA_REFCOUNTING(n) ((int *)n->mask[1])
#define PRAGMA_REFCOUNTINGNUMS(n) ((nums *)n->mask[1])
#define PRAGMA_READONLY(n) ((int *)n->mask[2])
#define PRAGMA_READONLYNUMS(n) ((nums *)n->mask[2])
#define PRAGMA_EFFECT(n) ((ids *)n->mask[3])
#define PRAGMA_TOUCH(n) ((ids *)n->mask[4])
#define PRAGMA_COPYFUN(n) ((char *)n->mask[5])
#define PRAGMA_FREEFUN(n) ((char *)n->mask[6])
#define PRAGMA_INITFUN(n) ((char *)n->node[3])
#define PRAGMA_LINKMOD(n) ((char *)n->node[2])
#define PRAGMA_NEEDTYPES(n) ((ids *)n->node[1])
#define PRAGMA_NEEDFUNS(n) (n->node[0])
#define PRAGMA_NUMPARAMS(n) (n->flag)

/*--------------------------------------------------------------------------*/

/***
 ***  N_info :
 ***
 ***  The N_info node is used to store additional compile time information
 ***  outside the syntax tree. So, its concrete look depends on the
 ***  specific task.
 ***
 ***  when used in typecheck.c :
 ***
 ***    node *     NEXTASSIGN    <O>  (N_assign)
 ***    node *     LASSIGN       <O>  (N_assign)
 ***
 ***  when used in writesib.c :
 ***
 ***    nodelist*  EXPORTTYPES   (O)
 ***    nodelist*  EXPORTOBJS    (O)
 ***    nodelist*  EXPORTFUNS    (O)
 ***
 ***  when used in compile.c :
 ***
 ***    ids*       LASTIDS       (O)
 ***    node*      LASTLET       (O)  (N_let)
 ***    node*      LASTASSIGN    (O)  (N_assign)
 ***    node*      VARDECS       (O)  (N_vardec)
 ***    node*      WITHBEGIN     (O)  (N_icm)
 ***
 ***    node*      FIRSTASSIGN   (O)  (N_assign)
 ***    node*      FUNDEF        (O)  (N_fundef)
 ***    int        CNTPARAM
 ***    node**     ICMTAB        (O)
 ***    types**    TYPETAB       (O)
 ***
 ***  when used in compile.c :
 ***    int        IS_WL
 ***    node*      NEXT
 ***    node*      WL
 ***
 ***/

/*
 *  When used in writesib.c, the N_info node collects lists of nodes which
 *  have to be printed to the SIB.
 */

/* srs: the number of sons being traversed is set to MAX_SONS, so
   don't use (temporary) attributes in the node-slots  */

extern node *MakeInfo ();

#define INFO_NEXTASSIGN(n) (n->node[1])
#define INFO_LASSIGN(n) (n->node[3])

#define INFO_EXPORTTYPES(n) ((nodelist *)n->node[0])
#define INFO_EXPORTOBJS(n) ((nodelist *)n->node[1])
#define INFO_EXPORTFUNS(n) ((nodelist *)n->node[2])

#define INFO_LASTIDS(n) (n->info.ids)
#define INFO_LASTLET(n) (n->node[1])
#define INFO_LASTASSIGN(n) (n->node[0])
#define INFO_VARDECS(n) (n->node[3])
#define INFO_WITHBEGIN(n) (n->node[2])

#define INFO_FIRSTASSIGN(n) (n->node[0])
#define INFO_FUNDEF(n) (n->node[1])
#define INFO_CNTPARAM(n) (n->lineno)
#define INFO_ICMTAB(n) ((node **)n->node[2])
#define INFO_TYPETAB(n) ((types **)n->info.types)

/* WLF */
#define INFO_IS_WL(n) (n->counter)
#define INFO_NEXT(n) (n->node[0])
#define INFO_WL(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith :
 ***
 ***  sons:
 ***
 ***    node*  PART      (N_Npart)
 ***    node*  CODE      (N_Ncode)
 ***    node*  WITHOP    (N_Nwithop)
 ***
 ***  temporary attributes:
 ***
 ***    ?
 ***
 ***  remarks:
 ***    flatten: it's important that the Npart nodes are flattened before
 ***    Ncode. So assure that with the current traversal mechanism the
 ***    ->node index of Npart is lower than the index of Ncode.
 ***/

extern node *MakeNWith (node *part, node *code, node *withop);

#define NWITH_PART(n) (n->node[0])
#define NWITH_CODE(n) (n->node[1])
#define NWITH_WITHOP(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Npart :
 ***
 ***  sons:
 ***
 ***    node*  IDX           (N_Nwithid)
 ***    node*  GEN           (N_Ngenerator)
 ***    node*  NEXT      (O) (N_Npart)
 ***
 ***  permanent attributes:
 ***
 ***    node*  CODE          (N_Ncode)
 ***/

extern node *MakeNPart (node *withid, node *generator);

#define NPART_IDX(n) (n->node[0])
#define NPART_GEN(n) (n->node[1])
#define NPART_NEXT(n) (n->node[2])
#define NPART_CODE(n) (n->node[3])

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
#define NWITHID_IDS(n) ((ids *)n->info2)

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
 ***    node*  EXPR            (scanparse, stays != NULL afterwards)
 ***    node*  FUNDEF          (N_fundef)  (typecheck -> )
 ***
 ***  remarks: WithOpType is WO_genarray, WO_modarray, WO_foldfun, WO_foldprf.
 ***    FUNDEF-node is used if TYPE == WO_foldfun.
 ***
 ***/

extern node *MakeNWithOp (WithOpType WithOp);

#define NWITHOP_TYPE(n) (*(WithOpType *)(n)->info2)
#define NWITHOP_SHAPE(n) (n->node[0])
#define NWITHOP_ARRAY(n) (n->node[0])
#define NWITHOP_NEUTRAL(n) (n->node[0])
#define NWITHOP_EXPR(n) (n->node[1])
#define NWITHOP_FUN(n) (n->info.fun_name.id)
#define NWITHOP_MOD(n) (n->info.fun_name.id_mod)
#define NWITHOP_PRF(n) (n->info.prf)
#define NWITHOP_FUNDEF(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Ncode :
 ***
 ***  sons:
 ***
 ***    node*  CBLOCK    (O) (N_block)
 ***    node*  CEXPR         ("N_expr")
 ***    node*  NEXT      (O) (N_exprs)
 ***
 ***  permanent attributes:
 ***
 ***    int    USED       (number of times this code is used)
 ***
 ***/

extern node *MakeNCode (node *block, node *expr);

#define NCODE_USED(n) (n->info.cint)
#define NCODE_CBLOCK(n) (n->node[0])
#define NCODE_CEXPR(n) (n->node[1])
#define NCODE_NEXT(n) (n->node[2])

/*--------------------------------------------------------------------------*/

#endif /* _sac_tree_basic_h */
