/*
 *
 * $Log$
 * Revision 1.8  1995/10/19 11:16:00  cg
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
 - fun-analysis
 - obj-analysis
 - write-SIB
 - obj-handling
 - optimize
 - psi-optimize
 - refcount
 - compile

In general, the implementation section consists of the declaration of
a create function for the respective structure and one basic access
macro for each slot. The definitions of all functions declared in this
file can be found in tree_basic.c

============================================================================*/

#ifndef _sac_tree_basic_h

#define _sac_tree_basic_h

/*
 *   Global Access-Macros
 *   --------------------
 */

#define NODE_TYPE(n) (n->nodetype)

#define NODE_LINE(n) (n->lineno)

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
 ***/

extern types *MakeType (simpletype basetype, int dim, shpseg *shpseg, char *name,
                        char *mod);

#define TYPES_BASETYPE(t) (t->simpletype)
#define TYPES_DIM(t) (t->dim)
#define TYPES_SHPSEG(t) (t->shpseg)
#define TYPES_NAME(t) (t->name)
#define TYPES_MOD(t) (t->name_mod)
#define TYPES_NEXT(t) (t->next)

/*--------------------------------------------------------------------------*/

/***
 ***  IDS :
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       MOD     (O)
 ***    statustype  ATTRIB
 ***    ids*        NEXT    (O)
 ***
 ***  temporary attributes:
 ***
 ***    int         REFCNT       (refcount -> )
 ***    node*       DECL         (typecheck -> )
 ***    node*       DEF          (optimize -> )
 ***    node*       USE          (optimize -> )
 ***    statustype  STATUS       (obj-handling -> compile !!)
 ***/

/*
 *  ATTRIB: ST_local      :  local variable or function parameter
 *          ST_global     :  reference to global object
 *  STATUS: ST_regular    :  from original source code
 *          ST_artificial :  added by obj-handling
 */

extern ids *MakeIds (char *name, char *mod, statustype status);

#define IDS_NAME(i) (i->id)
#define IDS_MOD(i) (i->mod)
#define IDS_REFCNT(i) (i->refcnt)
#define IDS_NEXT(i) (i->next)
#define IDS_DECL(i) (i->node)
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
 *  Possible values for ATTRIB : ST_resolved | ST_unresolved
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
 ***/

extern node *MakeModul (char *name, file_type filetype, node *imports, node *types,
                        node *objs, node *funs);

#define MODUL_NAME(n) (n->info.id)
#define MODUL_FILETYPE(n) ((file_type) (n->varno))
#define MODUL_IMPORTS(n) (n->node[0])
#define MODUL_TYPES(n) (n->node[1])
#define MODUL_OBJS(n) (n->node[3])
#define MODUL_FUNS(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_moddec :
 ***
 ***  sons:
 ***
 ***    node*  IMPORTS  (O)  (N_implist)
 ***    node*  OWN      (O)  (N_explist)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    char*  PREFIX   (O)
 ***/

extern node *MakeModdec (char *name, char *prefix, node *imports, node *exports);

#define MODDEC_NAME(n) (n->info.fun_name.id)
#define MODDEC_PREFIX(n) (n->info.fun_name.id_mod)
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
 ***    char*  PREFIX   (O)
 ***/

extern node *MakeClassdec (char *name, char *prefix, node *imports, node *exports);

#define CLASSDEC_NAME(n) (n->info.fun_name.id)
#define CLASSDEC_PREFIX(n) (n->info.fun_name.id_mod)
#define CLASSDEC_IMPORTS(n) (n->node[1])
#define CLASSDEC_OWN(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_sib :
 ***
 ***  sons:
 ***
 ***    node*     TYPES    (O)  (N_typedef)
 ***    node*     FUNS     (O)  (N_fundef)
 ***
 ***  permanent attributes:
 ***
 ***    strings*  LINKLIST (O)
 ***/

/*
 *  This node structure is used as head structure for SIBs. The LINKLIST
 *  contains all C-modules which must be linked to the importing SAC-file.
 */

extern node *MakeSib (node *types, node *funs, strings *linklist);

#define SIB_TYPES(n) (n->node[0])
#define SIB_FUNS(n) (n->node[1])
#define SIB_LINKLIST(n) ((strings *)(n->node[2]))

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
 ***    char*       MOD   (O)
 ***    types*      TYPE
 ***    statustype  ATTRIB
 ***    statustype  STATUS
 ***
 ***  temporary attributes:
 ***
 ***    types*      IMPL  (O)               (import -> )
 ***                                        ( -> writesib !!)
 ***/

/*
 *  The STATUS indicates whether a type is defined or imported.
 *  Possible values: ST_regular | ST_imported
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
 ***    char*       MOD   (O)
 ***    types*      TYPE
 ***    statustype  STATUS
 ***
 ***  temporary attributes:
 ***
 ***    char*       VARNAME    (typecheck -> obj-handling -> )
 ***/

/*
 *  The STATUS indicates whether an object is defined or imported.
 *  Possible values: ST_regular | ST_imported
 *
 *  The VARNAME is a combination of NAME and MOD. It's used as parameter
 *  name when making global objects local.
 */

extern node *MakeObjdef (char *name, char *mod, types *type, node *expr, node *next);

#define OBJDEF_NAME(n) (n->info.types->id)
#define OBJDEF_MOD(n) (n->info.types->id_mod)
#define OBJDEF_TYPE(n) (n->info.types)
#define OBJDEF_EXPR(n) (n->node[1])
#define OBJDEF_NEXT(n) (n->node[0])
#define OBJDEF_STATUS(n) (n->info.types->status)
#define OBJDEF_VARNAME(n) ((char *)(n->node[2]))

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
 ***    char*       ALIAS    (O)
 ***    types*      TYPES
 ***    statustype  STATUS
 ***
 ***  temporary attributes:
 ***
 ***    node*      RETURN        (N_return)  (typecheck -> compile !!)
 ***    nodelist*  NEEDOBJS (O)              (import -> )
 ***                                         (fun-analysis -> )
 ***                                         ( -> obj-analysis -> )
 ***                                         ( -> write-SIB -> )
 ***                                         ( -> obj-handling -> )
 ***    node*      ICM           (N_icm)     (compile -> )
 ***    int        VARNO                     (optimize -> )
 ***    long*      DEFMASK                   (optimize -> )
 ***    long*      USEMASK                   (optimize -> )
 ***/

/*
 *  The STATUS indicates whether a function is defined or imported.
 *  Possible values: ST_regular | ST_imported
 */

extern node *MakeFundef (char *name, char *mod, char *alias, types *types, node *args,
                         node *body, node *next);

#define FUNDEF_NAME(n) (n->info.types->id)
#define FUNDEF_MOD(n) (n->info.types->id_mod)
#define FUNDEF_ALIAS(n) ((char *)n->node[5])
#define FUNDEF_TYPES(n) (n->info.types)
#define FUNDEF_BODY(n) (n->node[0])
#define FUNDEF_ARGS(n) (n->node[2])
#define FUNDEF_NEXT(n) (n->node[1])
#define FUNDEF_RETURN(n) (n->node[3])
#define FUNDEF_NEEDOBJS(n) ((nodelist *)(n->node[4]))
#define FUNDEF_ICM(n) (n->node[3])
#define FUNDEF_VARNO(n) (n->varno)
#define FUNDEF_DEFMASK(n) (n->mask[0])
#define FUNDEF_USEMASK(n) (n->mask[1])
#define FUNDEF_STATUS(n) (n->info.types->status)

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
 ***    int         VARNO                (optimize -> )
 ***/

extern node *MakeArg (char *name, types *type, statustype status, statustype attrib,
                      node *next);

#define ARG_NAME(n) (n->info.types->id)
#define ARG_TYPE(n) (n->info.types)
#define ARG_STATUS(n) (n->info.types->status)
#define ARG_ATTRIB(n) (n->info.types->attrib)
#define ARG_NEXT(n) (n->node[0])
#define ARG_VARNO(n) (n->varno)

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
 ***    nodelist*  NEEDFUNS   (O)         (fun-analysis -> )
 ***                                      ( -> obj-analysis -> )
 ***                                      ( -> write-SIB -> )
 ***    nodelist*  NEEDTYPES  (O)         (fun-analysis -> )
 ***                                      ( -> write-SIB -> )
 ***    long*      DEFMASK                (optimize -> )
 ***    long*      USEMASK                (optimize -> )
 ***/

extern node *MakeBlock (node *instr, node *vardec);

#define BLOCK_INSTR(n) (n->node[0])
#define BLOCK_VARDEC(n) (n->node[1])
#define DEFMASK(n) (n->mask[0])
#define USEMASK(n) (n->mask[1])
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
 ***    char*       NAME
 ***    types*      TYPE
 ***    statustype  STATUS
 ***
 ***  temporary attributes:
 ***
 ***    node        TYPEDEF  (O)  (N_typedef)  (typecheck -> fun_analysis -> )
 ***    int         REFCNT                     (refcount -> compile -> )
 ***    int         VARNO                      (optimize -> )
 ***/

/*
 * STATUS : ST_regular :  original vardec in source code
 *          ST_used    :  after typecheck detected necessity of vardec
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
#define VARDEC_TYPEDEF(n) (n->node[1])

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
 ***    long*  DEFMASK                    (optimize -> )
 ***    long*  USEMASK                    (optimize -> )
 ***/

extern node *MakeAssign (node *instr, node *next);

#define ASSIGN_INSTR(n) (n->node[0])
#define ASSIGN_NEXT(n) (n->node[1])
#define ASSIGN_DEFMASK(n) (n->mask[0])
#define ASSIGN_USEMASK(n) (n->mask[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_let :
 ***
 ***  sons:
 ***
 ***    node*  EXPR      ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    ids*   IDS   (O)
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
 ***    node*  EXPRS   (N_exprs)
 ***/

extern node *MakeReturn (node *exprs);

#define RETURN_EXPRS(n) (n->node[0])

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
 ***    long*  CONDUSEMASK              (optimize -> )
 ***    long*  THENDEFMASK              (optimize -> )
 ***    long*  THENUSEMASK              (optimize -> )
 ***    long*  ELSEDEFMASK              (optimize -> )
 ***    long*  ELSEUSEMASK              (optimize -> )
 ***/

extern node *MakeCond (node *cond, node *Then, node *Else);

#define COND_COND(n) (n->node[0])
#define COND_THEN(n) (n->node[1])
#define COND_ELSE(n) (n->node[2])
#define COND_THENVARS(n) (n->node[3]->node[0])
#define COND_ELSEVARS(n) (n->node[3]->node[1])
#define COND_CONDUSEMASK(n) (n->mask[1])
#define COND_THENDEFMASK(n) (n->node[1]->mask[0])
#define COND_THENUSEMASK(n) (n->node[1]->mask[1])
#define COND_ELSEDEFMASK(n) (n->node[2]->mask[0])
#define COND_ELSEUSEMASK(n) (n->node[2]->mask[1])

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
 ***    long*  DEFMASK                (optimize -> )
 ***    long*  USEMASK                (optimize -> )
 ***/

extern node *MakeDo (node *cond, node *body);

#define DO_COND(n) (n->node[0])
#define DO_BODY(n) (n->node[1])
#define DO_USEVARS(n) (n->node[2]->node[0])
#define DO_DEFVARS(n) (n->node[2]->node[1])
#define DO_DEFMASK(n) (n->mask[0])
#define DO_USEMASK(n) (n->mask[1])

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
 ***    long*  DEFMASK                (optimize -> )
 ***    long*  USEMASK                (optimize -> )
 ***/

extern node *MakeWhile (node *cond, node *body);

#define WHILE_COND(n) (n->node[0])
#define WHILE_BODY(n) (n->node[1])
#define WHILE_USEVARS(n) (n->node[2]->node[0])
#define WHILE_DEFVARS(n) (n->node[2]->node[1])
#define WHILE_DEFMASK(n) (n->mask[0])
#define WHILE_USEMASK(n) (n->mask[1])

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
 ***
 ***  temporary attributes:
 ***
 ***    node*  FUNDEF       (N_fundef)  (typecheck -> )
 ***/

extern node *MakeAp (char *name, char *mod, node *args);

#define AP_NAME(n) (n->info.fun_name.id)
#define AP_MOD(n) (n->info.fun_name.id_mod)
#define AP_ARGS(n) (n->node[0])
#define AP_FUNDEF(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_with :
 ***
 ***  sons:
 ***
 ***    node*  GEN      (N_generator)
 ***    node*  BODY     (N_block)
 ***
 ***  temporary attributes:
 ***
 ***    long*  ARRAYUSEMASK            (optimize -> )
 ***    long*  GENDEFMASK              (optimize -> )
 ***    long*  GENUSEMASK              (optimize -> )
 ***    long*  BODYDEFMASK             (optimize -> )
 ***    long*  BODYUSEMASK             (optimize -> )
 ***/

extern node *MakeWith (node *gen, node *body);

#define WITH_GEN(n) (n->node[0])
#define WITH_BODY(n) (n->node[1])
#define WITH_ARRAYUSEMASK(n) (n->node[1]->mask[1])
#define WITH_GENDEFMASK(n, x) (n->node[0]->mask[0])
#define WITH_GENUSEMASK(n, x) (n->node[0]->mask[1])
#define WITH_BODYDEFMASK(n, x) (n->mask[0])
#define WITH_BODYUSEMASK(n, x) (n->mask[1])

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
 ***/

extern node *MakeGenerator (node *left, node *right, char *id);

#define GEN_LEFT(n) (n->node[0])
#define GEN_RIGHT(n) (n->node[1])
#define GEN_ID(n) (n->info.ids->id)

/*--------------------------------------------------------------------------*/

/***
 ***  N_genarray :
 ***
 ***  sons:
 ***
 ***    node*  ARRAY  (N_array)
 ***    node*  BODY   (N_block)
 ***/

extern node *MakeGenarray (node *array, node *body);

#define GENARRAY_ARRAY(n) (n->node[0])
#define GENARRAY_BODY(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_modarray :
 ***
 ***  sons:
 ***
 ***    node*  ARRAY  (N_array)
 ***    node*  BODY   (N_block)
 ***/

extern node *MakeModarray (node *array, node *body);

#define MODARRAY_ARRAY(n) (n->node[0])
#define MODARRAY_BODY(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_foldprf :
 ***
 ***  sons:
 ***
 ***    node*  BODY  (N_block)
 ***
 ***  permanent attributes:
 ***
 ***    prf    PRF
 ***/

extern node *MakeFoldprf (prf prf, node *body);

#define FOLDPRF_PRF(n) (n->info.prf)
#define FOLDPRF_BODY(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_foldfun :
 ***
 ***  sons:
 ***
 ***    node*  BODY          (N_block)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    char*  MOD      (O)
 ***    node*  NEUTRAL       ("N_expr")
 ***
 ***  temporary attributes:
 ***
 ***    node*  FUNDEF        (N_fundef)  (typecheck -> )
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
 ***    shapes*  SHP   (O)
 ***
 ***/

extern node *MakeVinfo (useflag flag, shapes *shp, node *next);

#define VINFO_FLAG(n) (n->info.use)
#define VINFO_SHP(n) ((shapes *)n->node[1])
#define VINFO_NEXT(n) (n->node[0])

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
 ***    node*  VARDEC    (N_vardec)  (typecheck -> )
 ***    node*  OBJDEF    (N_objdef)  (typecheck -> )
 ***                                 ( -> fun-analysis -> )
 ***    int    REFCNT                (refcount -> compile -> )
 ***/

extern node *MakeId (char *name, char *mod, statustype status);

#define ID_NAME(n) (n->info.ids->id)
#define ID_VARDEC(n) (n->info.ids->node)
#define ID_REFCNT(n) (n->info.ids->refcnt)
#define ID_MOD(n) (n->info.ids->mod)
#define ID_ATTRIB(n) (n->info.ids->attrib)
#define ID_STATUS(n) (n->info.ids->status)
#define ID_OBJDEF(n) (n->info.ids->node)

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
 ***    int    INCDEC
 ***    char*  ID
 ***
 ***  temporary attributes:
 ***
 ***    node*  DECL    (N_vardec)  (typecheck -> )
 ***/

extern node *MakePost (int incdec, char *id);

#define POST_INCDEC(n) ((n->node[0]->nodetype == N_dec) ? 0 : 1)
/* #define POST_INCDEC(n) (n->varno) */
#define POST_ID(n) (n->info.id)
#define POST_DECL(n) (n->node[1])

/*
 * Attention : The way incrementations and decrementation are represented
 * is not changed up to now. The macro POST_INCDEC must not be used on the
 * left side of the assignment operator. In comments the new representation
 * is shown. The MakePost function is intended to support both ways.
 */

/*--------------------------------------------------------------------------*/

/***
 ***  N_pre :
 ***
 ***  permanent attributes:
 ***
 ***    int    INCDEC
 ***    char*  ID
 ***
 ***  temporary attributes:
 ***
 ***    node*  DECL    (N_vardec)  (typecheck -> )
 ***/

extern node *MakePre (nodetype incdec, char *id);

extern node *MakePre (nodetype incdec, char *id);

#define PRE_INCDEC(n) ((n->node[0]->nodetype == N_dec) ? 0 : 1)
/* #define PRE_INCDEC(n) (n->varno) */
#define PRE_ID(n) (n->info.id)
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
 ***/

extern node *MakeIcm (char *name, node *args, node *next);

#define ICM_NAME(n) (n->info.id)
#define ICM_ARGS(n) (n->node[0])
#define ICM_NEXT(n) (n->node[1])

/*--------------------------------------------------------------------------*/

#endif /* _sac_tree_basic_h */
