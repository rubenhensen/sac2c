/*
 *
 * $Log$
 * Revision 1.78  1998/08/03 10:49:59  cg
 * added initialization of WLSEG_MAXHOMDIM
 *
 * Revision 1.77  1998/06/24 10:37:43  dkr
 * removed WL_END from indent-mechanismus
 *
 * Revision 1.76  1998/06/23 12:40:57  cg
 * added indentation information for mt-ICMs
 *
 * Revision 1.75  1998/06/08 08:57:34  cg
 * handling of attribute ARRAY_TYPE corrected.
 *
 * Revision 1.74  1998/05/30 15:41:43  dkr
 * added some ICM_INDENT cases
 *
 * Revision 1.73  1998/05/28 23:55:58  dkr
 * in MakeIcm:
 *   added some ICM_INDENTs
 *
 * Revision 1.72  1998/05/28 16:30:47  dkr
 * added an indent-machanismus for H-ICMs
 *
 * Revision 1.71  1998/05/24 00:39:30  dkr
 * removed WLGRID_CODE_TEMPLATE
 *
 * Revision 1.70  1998/05/17 00:08:39  dkr
 * WLGRID_CEXPR_TEMPLATE is now WLGRID_CODE_TEMPLATE
 *
 * Revision 1.69  1998/05/15 23:04:22  dkr
 * changed MakeNwith2: NWITH2_IDX_MIN, NWITH2_IDX_MAX are no longer
 * preallocated
 *
 * Revision 1.68  1998/05/12 22:43:55  dkr
 * changed MakeNwith2:
 *   added NWITH2_DIM, NWITH2_IDX_MIN, NWITH2_IDX_MAX
 *
 * Revision 1.67  1998/05/12 15:51:18  dkr
 * removed ???_VARINFO
 *
 * Revision 1.66  1998/05/06 14:32:02  dkr
 * added support for DataFlowMasks
 *
 * Revision 1.65  1998/04/26 21:51:43  dkr
 * MakeSPMD renamed to MakeSpmd
 *
 * Revision 1.64  1998/04/25 12:36:03  dkr
 * NCODE_COPY is now initialized in MakeNCode
 *
 * Revision 1.63  1998/04/24 01:14:59  dkr
 * added N_sync
 *
 * Revision 1.62  1998/04/21 13:31:00  dkr
 * NWITH2_SEG renamed to NWITH2_SEGS
 *
 * Revision 1.61  1998/04/17 17:26:22  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.60  1998/04/17 16:22:20  dkr
 * MakeWLgrid, MakeWLgridvar now adjust NCODE_USED
 *
 * Revision 1.59  1998/04/10 02:24:52  dkr
 * changed MakeWLseg
 *
 * Revision 1.58  1998/04/02 17:39:23  dkr
 * added MakeConc
 *
 * Revision 1.57  1998/04/02 12:11:09  srs
 * fixed bug in MakeShpseg
 *
 * Revision 1.56  1998/04/01 23:58:04  dkr
 * added MakeWLstriVar, MakeWLgridVar
 *
 * Revision 1.55  1998/03/30 23:42:15  dkr
 * added attribute LEVEL for N_WLgrid
 *
 * Revision 1.54  1998/03/29 23:27:21  dkr
 * added temp. attribute WLGRID_MODIFIED
 *
 * Revision 1.53  1998/03/27 18:37:02  dkr
 * WLPROJ... renamed in WLSTRIDE
 *
 * Revision 1.52  1998/03/26 14:00:07  dkr
 * changed usage of MakeWLgrid
 *
 * Revision 1.51  1998/03/24 21:09:17  dkr
 * changed MakeWLproj
 *
 * Revision 1.50  1998/03/24 10:18:14  srs
 * Changed MakeNPart
 *
 * Revision 1.49  1998/03/22 15:32:31  dkr
 * N_WLproj: OFFSET, WIDTH -> BOUND1, BOUND2.
 *
 * Revision 1.48  1998/03/22 14:21:45  dkr
 * WLBLOCK_BLOCKING -> WLBLOCK_STEP
 *
 * Revision 1.47  1998/03/21 19:33:27  dkr
 * removed unused var in MakeWLseg
 *
 * Revision 1.46  1998/03/21 18:54:34  srs
 * MakeNWith initializes NWITH_PARTS with -1 instead of 0
 *
 * Revision 1.45  1998/03/21 14:05:50  dkr
 * changed MakeWLublock
 *
 * Revision 1.44  1998/03/20 20:51:07  dkr
 * changed usage of MakeWLseg
 *
 * Revision 1.43  1998/03/20 17:24:47  dkr
 * in N_WL... nodes: INNER is now called CONTENTS
 *
 * Revision 1.41  1998/03/18 10:48:38  dkr
 * changed MakeWLproj
 *
 * Revision 1.40  1998/03/17 10:36:06  dkr
 * changed MakeWLseg()
 *
 * Revision 1.39  1998/03/13 16:21:33  dkr
 * new nodes added:
 *   N_WLblock, N_WLublock
 *
 * Revision 1.38  1998/03/06 13:22:29  srs
 * modified MakeAssign and MakeNwith
 *
 * Revision 1.37  1998/03/03 19:39:34  dkr
 * renamed N_WLindex to N_WLproj
 *
 * Revision 1.36  1998/03/03 17:50:18  dkr
 * changed MakeWLgrid(), MakeWLindex()
 *
 * Revision 1.35  1998/03/03 16:13:03  dkr
 * changed something in WLindex, WLgrid
 *
 * Revision 1.34  1998/03/02 22:28:23  dkr
 * added nodes for precompilation of new with-loop
 *
 * Revision 1.33  1998/02/28 23:37:16  dkr
 * MakeCond() uses now MakeInfo instead of INIT_NODE(tmp->node[3])
 *
 * Revision 1.32  1998/02/27 13:20:09  dkr
 * change in MakeDo(), MakeWhile():
 *   ->node[2] initialized with MakeInfo()
 *
 * Revision 1.31  1998/02/16 16:34:57  srs
 * Changed MakeNwith
 *
 * Revision 1.30  1998/02/11 17:14:37  srs
 * changed NPART_IDX to NPART_WITHID
 *
 * Revision 1.29  1998/02/10 14:56:19  dkr
 * changed MakeWith (node[2] not allocated anymore)
 *
 * Revision 1.28  1998/02/09 16:06:45  srs
 * changed MakeNWithid
 *
 * Revision 1.27  1998/02/09 15:41:40  sbs
 * forced check in 8-(((
 *  not yet cleaned up!
 *
 * Revision 1.26  1997/12/10 14:24:37  sbs
 * a couple of NEWTREE's killed 8-)
 *
 * Revision 1.25  1997/11/18 18:05:23  srs
 * changed new WL-functions
 *
 * Revision 1.24  1997/11/18 17:33:19  dkr
 * modified MakeNWith()
 *
 * Revision 1.23  1997/11/13 16:17:09  srs
 * added functions for creation of WL-nodes (new syntaxtree)
 *
 * Revision 1.22  1997/10/30 19:09:14  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.21  1997/10/29 14:32:47  srs
 * free -> FREE
 *
 * Revision 1.20  1997/05/16 09:54:01  sbs
 * ANALSE-TOOL extended to function-application specific timing
 *
 * Revision 1.19  1997/05/14  08:16:43  sbs
 * N_annotate added
 *
 * Revision 1.18  1997/04/25  09:12:13  sbs
 * DBUG_ASSERT in MakeNodelist adjusted (no varargs)
 *
 * Revision 1.17  1997/03/19  13:38:16  cg
 * Added new data type 'deps' with respective access macros and
 * creation function
 *
 * Revision 1.16  1996/02/11  20:19:01  sbs
 * some minor corrections on stuff concerning N_vinfo,
 * VARDEC_ACTCHN, VARDEC_COLCHN, ARG_ACTCHN, and ARG_COLCHN added.
 *
 * Revision 1.15  1996/01/22  14:04:50  asi
 * added MakeId2
 *
 * Revision 1.14  1996/01/05  14:14:10  asi
 * added While2Do: This function converts a while-loop into a do-loop
 *
 * Revision 1.13  1996/01/02  15:51:33  cg
 * function MakeType now initializes new struct entries tdef and id_cmod
 * of struct types
 *
 * Revision 1.12  1995/12/29  10:29:45  cg
 * modified MakeNodelist and MakeSib, added MakeInfo
 *
 * Revision 1.11  1995/12/21  10:07:33  cg
 * now MakePragma has no argument at all, new macros PRAGMA_LINKSIGNNUMS etc.
 * for temporary storage of pragma as list of nums instead of array.
 *
 * Revision 1.10  1995/12/20  08:16:22  cg
 * added MakeChar, modified MakePragma.
 *
 * Revision 1.9  1995/12/01  17:09:20  cg
 * added new node type N_pragma
 * removed macro FUNDEF_ALIAS
 *
 * Revision 1.8  1995/11/16  19:40:20  cg
 * new function MakeStr for generating N_str node
 *
 * Revision 1.7  1995/11/02  13:13:31  cg
 * added new macros OBJDEF_ARG(n) and renamed IDS_DECL(i)
 * to IDS_VARDEC(i).
 *
 * Revision 1.6  1995/11/01  07:08:46  sbs
 * neutral addded to N_foldprf;
 * some DBUG_PRINTS inserted
 *
 * Revision 1.5  1995/10/20  13:45:58  cg
 * added DBUG_PRINT in MakeNodelist
 *
 * Revision 1.4  1995/10/19  10:05:54  cg
 * memory allocation now via function 'Malloc` from internal_lib
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

#include "types.h"
#include "tree_basic.h"

#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "scnprs.h"
#include "free.h"
#include "internal_lib.h"

/*--------------------------------------------------------------------------*/

#define PRF_IF(n, s, x, y) y

char *prf_name_str[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

/*--------------------------------------------------------------------------*/
/* local macros for heap allocation                                         */
/*--------------------------------------------------------------------------*/

#define ALLOCATE(var, type) var = (type *)Malloc (sizeof (type))

/*--------------------------------------------------------------------------*/
/* local macros for node initialization                                     */
/*--------------------------------------------------------------------------*/

#define INIT_NODE(v)                                                                     \
    {                                                                                    \
        int i;                                                                           \
        ALLOCATE (v, node);                                                              \
        v->info.id = NULL;                                                               \
        v->info.ids = NULL;                                                              \
        v->info.types = NULL;                                                            \
        v->info.fun_name.id = NULL;                                                      \
        v->info.fun_name.id_mod = NULL;                                                  \
        v->refcnt = 0;                                                                   \
        v->flag = 0;                                                                     \
        v->varno = 0;                                                                    \
        v->counter = 0;                                                                  \
        v->lineno = linenum;                                                             \
        v->info2 = NULL;                                                                 \
        for (i = 0; i < MAX_SONS; i++) {                                                 \
            v->node[i] = NULL;                                                           \
        }                                                                                \
        for (i = 0; i < MAX_MASK; i++) {                                                 \
            v->mask[i] = NULL;                                                           \
            v->dfmask[i] = NULL;                                                         \
        }                                                                                \
    }

/*--------------------------------------------------------------------------*/
/*  Make-functions for non-node structures                                  */
/*--------------------------------------------------------------------------*/

/* attention: the given parameter chain of nums structs is set free here!!!  */
shpseg *
MakeShpseg (nums *numsp)
{
    shpseg *tmp;
    int i;
    nums *oldnumsp;

    DBUG_ENTER ("MakeShpseg");

    ALLOCATE (tmp, shpseg);

    tmp->next = NULL;

    /*   for (i=0;i<SHP_SEG_SIZE;i++) */
    /*     SHPSEG_SHAPE(tmp,i)=-1; */

    i = 0;
    while (numsp != NULL) {
        if (i >= SHP_SEG_SIZE)
            SYSABORT (("Maximum number of dimensions exceeded"));

        SHPSEG_SHAPE (tmp, i) = NUMS_NUM (numsp);

        DBUG_PRINT ("GENTREE", ("shape-element: %d", numsp->num));

        i++;
        oldnumsp = numsp;
        numsp = NUMS_NEXT (numsp);
        FREE (oldnumsp);
    }

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

types *
MakeType (simpletype basetype, int dim, shpseg *shpseg, char *name, char *mod)
{
    types *tmp;

    DBUG_ENTER ("MakeType");

    ALLOCATE (tmp, types);

    tmp->simpletype = basetype;
    tmp->name = name;
    tmp->name_mod = mod;
    tmp->shpseg = shpseg;
    tmp->dim = dim;

    tmp->next = NULL;

    tmp->id = NULL;
    tmp->id_mod = NULL;
    tmp->id_cmod = NULL;

    tmp->status = ST_regular;
    tmp->attrib = ST_regular;
    tmp->tdef = NULL;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

ids *
MakeIds (char *name, char *mod, statustype status)
{
    ids *tmp;
    DBUG_ENTER ("MakeIds");

    ALLOCATE (tmp, ids);
    IDS_NAME (tmp) = name;
    IDS_MOD (tmp) = mod;
    IDS_REFCNT (tmp) = 0;
    IDS_NEXT (tmp) = NULL;
    IDS_VARDEC (tmp) = NULL;
    IDS_DEF (tmp) = NULL;
    IDS_USE (tmp) = NULL;
    IDS_STATUS (tmp) = status;
    IDS_ATTRIB (tmp) = (mod == NULL) ? ST_regular : ST_global;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

nums *
MakeNums (int num, nums *next)
{
    nums *tmp;
    DBUG_ENTER ("MakeNums");

    ALLOCATE (tmp, nums);
    NUMS_NUM (tmp) = num;
    NUMS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

deps *
MakeDeps (char *name, char *decname, char *libname, statustype status, deps *sub,
          deps *next)
{
    deps *tmp;
    DBUG_ENTER ("MakeDeps");

    ALLOCATE (tmp, deps);
    DEPS_NAME (tmp) = name;
    DEPS_DECNAME (tmp) = decname;
    DEPS_LIBNAME (tmp) = libname;
    DEPS_STATUS (tmp) = status;
    DEPS_SUB (tmp) = sub;
    DEPS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

strings *
MakeStrings (char *string, strings *next)
{
    strings *tmp;
    DBUG_ENTER ("MakeStrings");

    ALLOCATE (tmp, strings);
    STRINGS_STRING (tmp) = string;
    STRINGS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

nodelist *
MakeNodelist (node *node, statustype status, nodelist *next)
{
    nodelist *tmp;
    DBUG_ENTER ("MakeNodelist");

    DBUG_PRINT ("ANA", ("New nodelist entry : %s (%s)", ItemName (node),
                        mdb_nodetype[NODE_TYPE (node)]));

    ALLOCATE (tmp, nodelist);
    NODELIST_NODE (tmp) = node;
    NODELIST_STATUS (tmp) = status;
    NODELIST_NEXT (tmp) = next;

    switch (NODE_TYPE (node)) {
    case N_fundef:
        NODELIST_ATTRIB (tmp) = ST_unresolved;
        break;
    case N_objdef:
        NODELIST_ATTRIB (tmp) = ST_reference;
        break;
    case N_typedef:
        NODELIST_ATTRIB (tmp) = ST_regular;
        break;
    default:
        DBUG_ASSERT (0, ("Wrong node type in MakeNodelist"));
    }

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/
/*  Make-functions for node structures                                      */
/*--------------------------------------------------------------------------*/

node *
MakeModul (char *name, file_type filetype, node *imports, node *types, node *objs,
           node *funs)
{
    node *tmp;
    DBUG_ENTER ("MakeModul");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_modul;

    MODUL_FILETYPE (tmp) = filetype;
    MODUL_IMPORTS (tmp) = imports;
    MODUL_TYPES (tmp) = types;
    MODUL_OBJS (tmp) = objs;
    MODUL_FUNS (tmp) = funs;
    MODUL_NAME (tmp) = name;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeModdec (char *name, deps *linkwith, int isexternal, node *imports, node *exports)
{
    node *tmp;
    DBUG_ENTER ("MakeModdec");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_moddec;

    MODDEC_NAME (tmp) = name;
    MODDEC_LINKWITH (tmp) = linkwith;
    MODDEC_ISEXTERNAL (tmp) = isexternal;
    MODDEC_IMPORTS (tmp) = imports;
    MODDEC_OWN (tmp) = exports;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeClassdec (char *name, deps *linkwith, int isexternal, node *imports, node *exports)
{
    node *tmp;
    DBUG_ENTER ("MakeClassdec");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_classdec;

    CLASSDEC_NAME (tmp) = name;
    CLASSDEC_LINKWITH (tmp) = linkwith;
    CLASSDEC_ISEXTERNAL (tmp) = isexternal;
    CLASSDEC_IMPORTS (tmp) = imports;
    CLASSDEC_OWN (tmp) = exports;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeSib (char *name, int linkstyle, deps *linkwith, node *types, node *objs, node *funs)
{
    node *tmp;
    DBUG_ENTER ("MakeSib");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_sib;

    SIB_TYPES (tmp) = types;
    SIB_FUNS (tmp) = funs;
    SIB_OBJS (tmp) = objs;
    SIB_NAME (tmp) = name;
    SIB_LINKSTYLE (tmp) = linkstyle;
    SIB_LINKWITH (tmp) = linkwith;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeImplist (char *name, ids *itypes, ids *etypes, ids *objs, ids *funs, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeImplist");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_implist;

    IMPLIST_NAME (tmp) = name;
    IMPLIST_ITYPES (tmp) = itypes;
    IMPLIST_ETYPES (tmp) = etypes;
    IMPLIST_OBJS (tmp) = objs;
    IMPLIST_FUNS (tmp) = funs;
    IMPLIST_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeExplist (node *itypes, node *etypes, node *objs, node *funs)
{
    node *tmp;
    DBUG_ENTER ("MakeExplist");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_explist;

    EXPLIST_ITYPES (tmp) = itypes;
    EXPLIST_ETYPES (tmp) = etypes;
    EXPLIST_OBJS (tmp) = objs;
    EXPLIST_FUNS (tmp) = funs;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeTypedef (char *name, char *mod, types *type, statustype attrib, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeTypedef");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_typedef;

    TYPEDEF_TYPE (tmp) = type;
    TYPEDEF_NAME (tmp) = name;
    TYPEDEF_MOD (tmp) = mod;
    TYPEDEF_ATTRIB (tmp) = attrib;
    TYPEDEF_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeObjdef (char *name, char *mod, types *type, node *expr, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeObjdef");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_objdef;

    OBJDEF_TYPE (tmp) = type;
    OBJDEF_NAME (tmp) = name;
    OBJDEF_MOD (tmp) = mod;
    OBJDEF_EXPR (tmp) = expr;
    OBJDEF_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeFundef (char *name, char *mod, types *types, node *args, node *body, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeFundef");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_fundef;

    FUNDEF_TYPES (tmp) = types;
    FUNDEF_NAME (tmp) = name;
    FUNDEF_MOD (tmp) = mod;
    FUNDEF_ARGS (tmp) = args;
    FUNDEF_BODY (tmp) = body;
    FUNDEF_NEXT (tmp) = next;
    FUNDEF_FUNNO (tmp) = 0;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeArg (char *name, types *type, statustype status, statustype attrib, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeArg");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_arg;

    ARG_TYPE (tmp) = type;
    ARG_NAME (tmp) = name;
    ARG_STATUS (tmp) = status;
    ARG_ATTRIB (tmp) = attrib;
    ARG_NEXT (tmp) = next;
    ARG_ACTCHN (tmp) = NULL;
    ARG_COLCHN (tmp) = NULL;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeBlock (node *instr, node *vardec)
{
    node *tmp;
    DBUG_ENTER ("MakeBlock");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_block;

    BLOCK_INSTR (tmp) = instr;
    BLOCK_VARDEC (tmp) = vardec;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT " instr: " P_FORMAT " vardec: " P_FORMAT,
                 NODE_LINE (tmp), mdb_nodetype[NODE_TYPE (tmp)], tmp, BLOCK_INSTR (tmp),
                 BLOCK_VARDEC (tmp)));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeVardec (char *name, types *type, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeVardec");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_vardec;

    VARDEC_TYPE (tmp) = type;
    VARDEC_NAME (tmp) = name;
    VARDEC_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeAssign (node *instr, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeAssign");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_assign;

    ASSIGN_INSTR (tmp) = instr;
    ASSIGN_NEXT (tmp) = next;
    ASSIGN_INDEX (tmp) = NULL;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT " instr: " P_FORMAT " next: " P_FORMAT,
                 NODE_LINE (tmp), mdb_nodetype[NODE_TYPE (tmp)], tmp, ASSIGN_INSTR (tmp),
                 ASSIGN_NEXT (tmp)));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeLet (node *expr, ids *ids)
{
    node *tmp;
    DBUG_ENTER ("MakeLet");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_let;

    LET_EXPR (tmp) = expr;
    LET_IDS (tmp) = ids;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeCast (node *expr, types *type)
{
    node *tmp;
    DBUG_ENTER ("MakeCast");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_cast;

    CAST_TYPE (tmp) = type;
    CAST_EXPR (tmp) = expr;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeReturn (node *exprs)
{
    node *tmp;
    DBUG_ENTER ("MakeReturn");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_return;

    RETURN_EXPRS (tmp) = exprs;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeCond (node *cond, node *Then, node *Else)
{
    node *tmp;
    DBUG_ENTER ("MakeCond");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_cond;

    COND_COND (tmp) = cond;
    COND_THEN (tmp) = Then;
    COND_ELSE (tmp) = Else;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeDo (node *cond, node *body)
{
    node *tmp;
    DBUG_ENTER ("MakeDo");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_do;

    DO_COND (tmp) = cond;
    DO_BODY (tmp) = body;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeWhile (node *cond, node *body)
{
    node *tmp;
    DBUG_ENTER ("MakeWhile");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_while;

    WHILE_COND (tmp) = cond;
    WHILE_BODY (tmp) = body;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
While2Do (node *while_node)
{
    DBUG_ENTER ("While2Do");
    NODE_TYPE (while_node) = N_do;
    DBUG_RETURN (while_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeAnnotate (int tag, int funno, int funapno)
{
    node *tmp;
    DBUG_ENTER ("MakeAnnotate");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_annotate;

    ANNOTATE_TAG (tmp) = tag;
    ANNOTATE_FUNNUMBER (tmp) = funno;
    ANNOTATE_FUNAPNUMBER (tmp) = funapno;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeAp (char *name, char *mod, node *args)
{
    node *tmp;
    DBUG_ENTER ("MakeAp");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_ap;

    AP_NAME (tmp) = name;
    AP_MOD (tmp) = mod;
    AP_ARGS (tmp) = args;
    AP_ATFLAG (tmp) = 0;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeWith (node *gen, node *operator)
{
    node *tmp;
    DBUG_ENTER ("MakeWith");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_with;

    WITH_GEN (tmp) = gen;
    WITH_OPERATOR (tmp) = operator;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeGenerator (node *left, node *right, char *id)
{
    node *tmp;
    DBUG_ENTER ("MakeGenerator");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_generator;

    GEN_LEFT (tmp) = left;
    GEN_RIGHT (tmp) = right;
    GEN_ID (tmp) = id;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeGenarray (node *array, node *body)
{
    node *tmp;
    DBUG_ENTER ("MakeGenarray");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_genarray;

    GENARRAY_ARRAY (tmp) = array;
    GENARRAY_BODY (tmp) = body;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeModarray (node *array, node *body)
{
    node *tmp;
    DBUG_ENTER ("MakeModarray");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_modarray;

    MODARRAY_ARRAY (tmp) = array;
    MODARRAY_BODY (tmp) = body;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeFoldprf (prf prf, node *body, node *neutral)
{
    node *tmp;
    DBUG_ENTER ("MakeFoldprf");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_foldprf;

    FOLDPRF_PRF (tmp) = prf;
    FOLDPRF_BODY (tmp) = body;
    FOLDPRF_NEUTRAL (tmp) = neutral;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT " body: " P_FORMAT " neutral: " P_FORMAT,
                 NODE_LINE (tmp), mdb_nodetype[NODE_TYPE (tmp)], tmp, FOLDPRF_BODY (tmp),
                 FOLDPRF_NEUTRAL (tmp)));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeFoldfun (char *name, char *mod, node *body, node *neutral)
{
    node *tmp;
    DBUG_ENTER ("MakeFoldfun");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_foldfun;

    FOLDFUN_NAME (tmp) = name;
    FOLDFUN_MOD (tmp) = mod;
    FOLDFUN_BODY (tmp) = body;
    FOLDFUN_NEUTRAL (tmp) = neutral;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeExprs (node *expr, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeExprs");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_exprs;

    EXPRS_EXPR (tmp) = expr;
    EXPRS_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeArray (node *aelems)
{
    node *tmp;
    DBUG_ENTER ("MakeArray");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_array;

    ARRAY_AELEMS (tmp) = aelems;

    ARRAY_TYPE (tmp) = NULL;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeVinfo (useflag flag, types *type, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeVinfo");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_vinfo;

    VINFO_FLAG (tmp) = flag;
    VINFO_TYPE (tmp) = type;
    VINFO_NEXT (tmp) = next;
    VINFO_VARDEC (tmp) = NULL;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeId (char *name, char *mod, statustype status)
{
    node *tmp;
    DBUG_ENTER ("MakeId");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_id;

    tmp->info.ids = MakeIds (name, mod, status);

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeId2 (ids *ids_node)
{
    node *tmp;
    DBUG_ENTER ("MakeId2");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_id;

    tmp->info.ids = ids_node;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNum (int val)
{
    node *tmp;
    DBUG_ENTER ("MakeNum");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_num;

    NUM_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeChar (char val)
{
    node *tmp;
    DBUG_ENTER ("MakeChar");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_char;

    CHAR_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeFloat (float val)
{
    node *tmp;
    DBUG_ENTER ("MakeFloat");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_float;

    FLOAT_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeDouble (double val)
{
    node *tmp;
    DBUG_ENTER ("MakeDouble");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_double;

    DOUBLE_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeBool (int val)
{
    node *tmp;
    DBUG_ENTER ("MakeBool");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_bool;

    BOOL_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeStr (char *str)
{
    node *tmp;
    DBUG_ENTER ("MakeStr");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_str;

    STR_STRING (tmp) = str;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakePrf (prf prf, node *args)
{
    node *tmp;
    DBUG_ENTER ("MakePrf");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_prf;

    PRF_PRF (tmp) = prf;
    PRF_ARGS (tmp) = args;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeEmpty ()
{
    node *tmp;
    DBUG_ENTER ("MakeEmpty");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_empty;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakePost (int incdec, char *id)
{
    node *tmp;
    DBUG_ENTER ("MakePost");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_post;

    POST_ID (tmp) = id;

    /*
     *  The following lines contain the new representation for INCDEC.
     *  The access macro POST_INCDEC may not be used until its definition in
     *  tree_basic.h is converted to the new representation.
     */

    /*  POST_INCDEC(tmp)=incdec;  */
    tmp->varno = incdec;

    /*
     *  The following lines contain the old representation for INCDEC.
     *  This is needed until all files are converted to the new representation.
     */

    INIT_NODE (tmp->node[0]);
    tmp->node[0]->nodetype = (0 == incdec ? N_dec : N_inc);

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakePre (nodetype incdec, char *id)
{
    node *tmp;
    DBUG_ENTER ("MakePre");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_pre;

    PRE_ID (tmp) = id;

    /*
     *  The following lines contain the new representation for INCDEC.
     *  The access macro PRE_INCDEC may not be used until its definition in
     *  tree_basic.h is converted to the new representation.
     */

    /*  PRE_INCDEC(tmp)=incdec;  */
    tmp->varno = incdec;

    /*
     *  The following lines contain the old representation for INCDEC.
     *  This is needed until all files are converted to the new representation.
     */

    INIT_NODE (tmp->node[0]);
    tmp->node[0]->nodetype = (0 == incdec ? N_dec : N_inc);

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeIcm (char *name, node *args, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeIcm");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_icm;

    ICM_NAME (tmp) = name;
    ICM_ARGS (tmp) = args;
    ICM_NEXT (tmp) = next;

    if (strcmp (name, "MT_START_SYNCBLOCK") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strncmp (name, "MT_SYNC_", 8) == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_NONFOLD_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_FOLD_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_BLOCK_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_BLOCK_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_UBLOCK_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_UBLOCK_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_STRIDE_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_STRIDE_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_STRIDE_UNROLL_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_GRID_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_GRID_UNROLL_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_BLOCK_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_UBLOCK_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_STRIDE_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_STRIDE_UNROLL_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_GRID_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_GRID_UNROLL_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_BLOCK_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_BLOCK_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_UBLOCK_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_UBLOCK_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_STRIDE_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_STRIDE_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_STRIDE_UNROLL_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_GRID_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_GRID_UNROLL_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_BLOCK_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_UBLOCK_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_STRIDE_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_STRIDE_UNROLL_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_GRID_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_GRID_UNROLL_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_NONFOLD_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_FOLD_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strncmp (name, "MT_SCHEDULER_", 13) == 0) {
        if (strcmp (name + strlen (name) - 6, "_BEGIN") == 0)
            ICM_INDENT (tmp) = 1;
        else if (strcmp (name + strlen (name) - 4, "_END") == 0)
            ICM_INDENT (tmp) = -1;
        else
            ICM_INDENT (tmp) = 0;
    } else
        ICM_INDENT (tmp) = 0;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakePragma ()
{
    node *tmp;

    DBUG_ENTER ("MakePragma");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_pragma;

    PRAGMA_NUMPARAMS (tmp) = 0;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeInfo ()
{
    node *tmp;

    DBUG_ENTER ("MakeInfo");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_info;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeSpmd (node *region)
{
    node *tmp;

    DBUG_ENTER ("MakeSpmd");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_spmd;
    SPMD_REGION (tmp) = region;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeSync (node *region, int first)
{
    node *tmp;

    DBUG_ENTER ("MakeSync");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_sync;
    SYNC_REGION (tmp) = region;
    SYNC_FIRST (tmp) = first;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNWith (node *part, node *code, node *withop)
{
    node *tmp;

    DBUG_ENTER ("MakeNWith");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_Nwith;
    NWITH_PART (tmp) = part;
    NWITH_CODE (tmp) = code;
    NWITH_WITHOP (tmp) = withop;

    tmp->info2 = Malloc (sizeof (wl_info));
    NWITH_PARTS (tmp) = -1;
    NWITH_REFERENCED (tmp) = 0;
    NWITH_REFERENCED_FOLD (tmp) = 0;
    NWITH_COMPLEX (tmp) = 0;
    NWITH_FOLDABLE (tmp) = 0;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNPart (node *withid, node *generator, node *code)
{
    node *tmp;

    DBUG_ENTER ("MakeNPart");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_Npart;
    NPART_GEN (tmp) = generator;
    NPART_WITHID (tmp) = withid;
    NPART_CODE (tmp) = code;
    if (code != NULL) {      /* may be NULL in sac.y */
        NCODE_USED (code)++; /* see remarks of N_Ncode in tree_basic.h */
    }

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNWithid (ids *vec, ids *scalars)
{
    node *tmp;

    DBUG_ENTER ("MakeNWithid");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_Nwithid;
    NWITHID_VEC (tmp) = vec;
    NWITHID_IDS (tmp) = scalars;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNGenerator (node *bound1, node *bound2, prf op1, prf op2, node *step, node *width)
{
    node *tmp;

    DBUG_ENTER ("MakeNGenerator");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_Ngenerator;
    NGEN_BOUND1 (tmp) = bound1;
    NGEN_BOUND2 (tmp) = bound2;
    NGEN_OP1 (tmp) = op1;
    NGEN_OP2 (tmp) = op2;
    NGEN_STEP (tmp) = step;
    NGEN_WIDTH (tmp) = width;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNWithOp (WithOpType WithOp)
{
    node *tmp;

    DBUG_ENTER ("MakeNWithOp");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_Nwithop;

    /* allocate mem to store WithOpType in. */
    tmp->info2 = MALLOC (sizeof (WithOpType));
    NWITHOP_TYPE (tmp) = WithOp;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNCode (node *block, node *expr)
{
    node *tmp;

    DBUG_ENTER ("MakeNCODE");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_Ncode;
    NCODE_CBLOCK (tmp) = block;
    NCODE_CEXPR (tmp) = expr;
    NCODE_USED (tmp) = 0;
    NCODE_COPY (tmp) = NULL;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNWith2 (node *withid, node *seg, node *code, node *withop, int dims)
{
    node *tmp;

    DBUG_ENTER ("MakeNWith2");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_Nwith2;
    NWITH2_WITHID (tmp) = withid;
    NWITH2_SEGS (tmp) = seg;
    NWITH2_CODE (tmp) = code;
    NWITH2_WITHOP (tmp) = withop;
    NWITH2_DIMS (tmp) = dims;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLseg (int dims, node *contents, node *next)
{
    node *new_node;
    int b, d;

    DBUG_ENTER ("MakeWLseg");
    INIT_NODE (new_node);

    NODE_TYPE (new_node) = N_WLseg;
    WLSEG_DIMS (new_node) = dims;

    WLSEG_CONTENTS (new_node) = contents;
    WLSEG_NEXT (new_node) = next;

    WLSEG_BLOCKS (new_node) = 3; /* three blocking levels */
    for (b = 0; b < WLSEG_BLOCKS (new_node); b++) {
        WLSEG_BV (new_node, b) = (long *)MALLOC (sizeof (long) * dims);
    }
    WLSEG_UBV (new_node) = (long *)MALLOC (sizeof (long) * dims);

    WLSEG_SV (new_node) = (long *)MALLOC (sizeof (long) * dims);
    /* init SV */
    for (d = 0; d < dims; d++) {
        (WLSEG_SV (new_node))[d] = 1;
    }

    WLSEG_MAXHOMDIM (new_node) = -1;
    /*
     * By default, no dimension is homogenious. Since dimensions are counted
     * starting by 0, we must set  MAXHOMDIM to -1 here.
     */

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLblock (int level, int dim, int bound1, int bound2, int step, node *nextdim,
             node *contents, node *next)
{
    node *new_node;

    DBUG_ENTER ("MakeWLblock");
    INIT_NODE (new_node);

    NODE_TYPE (new_node) = N_WLblock;

    WLBLOCK_LEVEL (new_node) = level;
    WLBLOCK_DIM (new_node) = dim;
    WLBLOCK_BOUND1 (new_node) = bound1;
    WLBLOCK_BOUND2 (new_node) = bound2;
    WLBLOCK_STEP (new_node) = step;
    WLBLOCK_NEXTDIM (new_node) = nextdim;
    WLBLOCK_CONTENTS (new_node) = contents;
    WLBLOCK_NEXT (new_node) = next;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLublock (int level, int dim, int bound1, int bound2, int step, node *nextdim,
              node *contents, node *next)
{
    node *new_node;

    DBUG_ENTER ("MakeWLublock");

    new_node = MakeWLblock (level, dim, bound1, bound2, step, nextdim, contents, next);

    NODE_TYPE (new_node) = N_WLublock;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLstride (int level, int dim, int bound1, int bound2, int step, int unrolling,
              node *contents, node *next)
{
    node *new_node;

    DBUG_ENTER ("MakeWLstride");
    INIT_NODE (new_node);

    NODE_TYPE (new_node) = N_WLstride;

    WLSTRIDE_LEVEL (new_node) = level;
    WLSTRIDE_DIM (new_node) = dim;
    WLSTRIDE_BOUND1 (new_node) = bound1;
    WLSTRIDE_BOUND2 (new_node) = bound2;
    WLSTRIDE_STEP (new_node) = step;
    WLSTRIDE_UNROLLING (new_node) = unrolling;
    WLSTRIDE_CONTENTS (new_node) = contents;
    WLSTRIDE_NEXT (new_node) = next;

    WLSTRIDE_PART (new_node) = NULL;
    WLSTRIDE_MODIFIED (new_node) = 0;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLgrid (int level, int dim, int bound1, int bound2, int unrolling, node *nextdim,
            node *next, node *code)
{
    node *new_node;

    DBUG_ENTER ("MakeWLgrid");
    INIT_NODE (new_node);

    NODE_TYPE (new_node) = N_WLgrid;

    WLGRID_LEVEL (new_node) = level;
    WLGRID_DIM (new_node) = dim;
    WLGRID_BOUND1 (new_node) = bound1;
    WLGRID_BOUND2 (new_node) = bound2;
    WLGRID_UNROLLING (new_node) = unrolling;
    WLGRID_NEXTDIM (new_node) = nextdim;
    WLGRID_NEXT (new_node) = next;

    if (code != NULL) {
        NCODE_USED (code)++;
    }
    WLGRID_CODE (new_node) = code;

    WLGRID_MODIFIED (new_node) = 0;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLstriVar (int dim, node *bound1, node *bound2, node *step, node *contents,
               node *next)
{
    node *new_node;

    DBUG_ENTER ("MakeWLstriVar");
    INIT_NODE (new_node);

    NODE_TYPE (new_node) = N_WLstriVar;

    WLSTRIVAR_DIM (new_node) = dim;
    WLSTRIVAR_BOUND1 (new_node) = bound1;
    WLSTRIVAR_BOUND2 (new_node) = bound2;
    WLSTRIVAR_STEP (new_node) = step;
    WLSTRIVAR_CONTENTS (new_node) = contents;
    WLSTRIVAR_NEXT (new_node) = next;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLgridVar (int dim, node *bound1, node *bound2, node *nextdim, node *next, node *code)
{
    node *new_node;

    DBUG_ENTER ("MakeWLgridVar");
    INIT_NODE (new_node);

    NODE_TYPE (new_node) = N_WLgridVar;

    WLGRIDVAR_DIM (new_node) = dim;
    WLGRIDVAR_BOUND1 (new_node) = bound1;
    WLGRIDVAR_BOUND2 (new_node) = bound2;
    WLGRIDVAR_NEXTDIM (new_node) = nextdim;
    WLGRIDVAR_NEXT (new_node) = next;

    if (code != NULL) {
        NCODE_USED (code)++;
    }
    WLGRIDVAR_CODE (new_node) = code;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/
