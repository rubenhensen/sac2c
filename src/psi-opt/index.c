
/*
 * $Log$
 * Revision 1.24  1998/08/06 17:18:21  dkr
 * changed signature of ICM ND_KS_VECT2OFFSET
 *
 * Revision 1.23  1998/07/16 13:47:35  sbs
 * the arraytype for WLs (modarray + genarray)
 * is now deduced from the LHS of the previous Let-Node
 * rather than ( in the old genarray-case) from the type!!!
 * of the first arg of the operation part !!!
 *
 * Revision 1.22  1998/06/07 18:37:05  dkr
 * ChgId renamed in IdxChangeId
 *
 * Revision 1.21  1998/06/06 13:55:30  dkr
 * changed ChdId():
 *   switched to new variable names.
 *
 * Revision 1.20  1998/05/18 08:16:10  srs
 * fixed bug in IdxPrf(), F_modarray
 *
 * Revision 1.19  1998/05/07 16:19:44  dkr
 * added support for new with-loop
 *
 * Revision 1.18  1998/05/04 19:42:18  dkr
 * added StringCopy(...) ...
 *    ... to first argument of each MakeId(...),
 *    ... to first arguemnt of MakeVardec(...) in VardecIdx(), ArgIdx(),
 *    ...
 * to get rid of shared NAME strings
 *
 * Revision 1.17  1998/05/04 17:46:21  sbs
 * LET_VARDEC(act_let)= VardecIdx( LET_VARDEC(act_let),
 *                                         VINFO_TYPE( vinfo),
 *                                         LET_NAME(act_let));
 *
 * inserted in IdxLet
 * this should guarantee that the backref from LHS vars to their
 * declaration is established!
 *
 * Revision 1.16  1997/11/23 18:00:23  dkr
 * in ChgId(): strdup -> StringCopy (strdup do not use malloc !!)
 *
 * Revision 1.15  1997/11/05 11:30:48  dkr
 * removed a bug with NEWTREE
 *
 * Revision 1.14  1997/11/04 13:22:46  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.13  1997/05/29 13:41:08  sbs
 * IVE for modarray integrated
 *
 * Revision 1.12  1997/05/02  15:40:54  sbs
 * IdxId reverted but IdxPrf case Psi adapted for arrays of unknown shape
 *
 * Revision 1.11  1997/05/02  14:38:31  sbs
 * IdxId : set vect   also if the array (of a psi application for example)
 * is of unknown shape!!
 * That can happen if an external function is involved that returns an
 * array of unknown shape, e.g. FibreScanIntArray.
 *
 * Revision 1.10  1996/04/23  15:48:33  sbs
 * two bugfixes done:
 * 1) IdxArray: when arg_info == NULL traverse all elements
 *              with NULL as well, since we may have nested
 *              array-definitions!
 * 2) IdxLet: if the LHS variables do not have a vinfo at all,
 *            we still have to traverse the RHS with arg_info NULL
 *            since it may contain array-variables. As a special
 *            case this covers a = b; where both of them are array-
 *            variables and a is not used any further!!!
 *
 * Revision 1.9  1996/02/13  10:15:11  sbs
 * counting of eliminations inserted.
 *
 * Revision 1.8  1996/02/12  15:44:46  sbs
 * bug in idxLet fixed; now we have different code for args and vardecs:-)
 *
 * Revision 1.7  1996/02/11  20:17:07  sbs
 * complete revise.
 * should work now on all programs.
 * still some known problems as stated in the file itself.
 *
 * Revision 1.6  1995/10/06  17:07:47  cg
 * adjusted calls to function MakeIds (now 3 parameters)
 *
 * Revision 1.5  1995/10/05  14:55:52  sbs
 * some bug fixes.
 *
 * Revision 1.4  1995/06/26  09:59:28  sbs
 * preliminary version
 *
 * Revision 1.3  1995/06/06  15:18:10  sbs
 * first usable version ; does not include conditional stuff
 *
 * Revision 1.2  1995/06/02  17:05:47  sbs
 * IdxAssign, IdxLet inserted.
 *
 * Revision 1.1  1995/06/02  10:06:56  sbs
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include "dbug.h"
#include "tree.h"
#include "traverse.h"
#include "DupTree.h"
#include "index.h"
#include "psi-opt.h"
#include "free.h"

#include "access_macros.h"

#include "compile.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 * - idx-ops on vects with non-max. shapes for psi
 *
 *
 * III) to be fixed somewhere else:
 */

/*
 * 1) Basics
 * ---------
 *
 * The "index optimization" tries to eliminate index vectors which are
 * only used for array selections.
 * Example:
 *
 *            a = reshape([4,4], [1,2,...,16]);
 *            i = [2,3];
 *            z = a[i];
 *
 * is transformed into:
 *
 *            a = reshape([4,4], [1,2,...,16]);
 *            __i_4_4 = 11;
 *            z = idx_psi(a, __i_4_4);
 *
 * For doing so, an attribute "Uses" has to be infered. It is attached to each
 * left hand side of an array assignment, and to each variable/argument declaration
 * of an array.
 * Since we want to eliminate index vectors the attribute attachment is restricted
 * to one-dimensional arrays( array identifiers) !
 * The "Uses" attribute consists of a set (chain) of attributes of the kind:
 *
 *     VECT/ IDX(<shape>).
 *
 * In the above example we obtain:
 *
 *            int z;
 *            int[2] i:IDX(int[4,4]) ;
 *            int[4, 4] a;
 *
 *            a= reshape([4,4], [1,2,...,16]);
 *            i:IDX(int[4,4]) = [2,3];
 *            z= a[i];
 *
 * BTW, this can be made visible by using the -bs option!
 *
 *
 *
 * 2) Attaching the "Uses" attribute
 * ---------------------------------
 *
 * The "Uses" information is stored as follows:
 * During the (bottom up) traversal all information is kept at the N_vardec/N_arg nodes.
 * This is realized by the introduction of a new node: "N_vinfo"
 * which is organized as follows:
 *
 *     info.use (useflag): VECT / IDX
 *     node[0]           : next "N_vinfo node" if existent
 *     node[1]           : withbelonging type if info.use == IDX
 *
 * it can be accessed by:
 *     VINFO_FLAG(n),
 *     VINFO_NEXT(n), and          ( cmp. tree_basic.h )
 *     VINFO_TYPE(n)
 *
 * There are two chains of such nodes attached to the N_vardec/N_arg nodes:
 * The "actual ones" and the "collected ones". The "actual ones" contain all
 * uses-infos collected during the (bottom up) traversal since the last
 * assignment to that particular variable. In contrast, the "collected ones"
 * chain collects all uses-infos within the function body.
 * They can be accessed by :
 *     VARDEC_ACTCHN(n), and
 *     VARDEC_COLCHN(n), or        ( cmp. tree_basic.h )
 *     ARG_ACTCHN(n), and
 *     ARG_COLCHN(n) respectively.
 *
 * Whenever an array identifier is met in index-position of an array-selection,
 * the respective IDX(...) attribute is added (without duplicates) to both chains
 * of the variable's declaration.
 *
 * When meeting a "N_let" with an array variable to be defined, the actual info
 * node(s) attached to the variable's declaration( "actual chain")
 * is(are) inserted in the ids structure of the N_let-node [LET_IDS(n)] at
 * ids->use [IDS_USE(i)] => [LET_USE(n)].   ( cmp. tree_compound.h )
 *
 *
 *
 * 3) Inserting new variable declarations
 * --------------------------------------
 *
 * Since all variables have backreferences to their declarations, the
 * new declarations needed have to be created when the first occurence of the
 * identifier is met. The adresses of these N_vardecs are stored in the
 * respective N_vinfo-nodes of the collected-chain. It can be accessed by:
 *
 *     VINFO_VARDEC(n)             ( cmp. tree_basic.h )
 *
 * Creating the new vardec's while traversing the var-usages requires a pointer
 * to the vardec-section of the function. In case of local variables this can
 * beachieved easily be following the N_id's backref to its N_vardec.
 * For function arguments the situation is more complicated. Their backref
 * leads to the N_arg node from which we can not reach the function's vardec-section.
 * Therefore, we first traverse a function's arguments and insert backrefs to the
 * N_fundef node. This enables us to find the vardec-section when creating
 * new vardecs for variables which are formal parametes of the function.
 *
 *
 *
 * 4) Inserting new Assignments
 * ----------------------------
 */

/*
 * Some functions for handling N_vinfo nodes:
 * ------------------------------------------
 * node * FindVect( node *N_vinfo_chain)	 : checks whether VECT is in the chain
 *						   of N_vinfo nodes
 * node * FindIdx( node *chain, types *shape)	 : dito for a specific shape
 *
 * node * SetVect( node *chain)			 : inserts VECT-node in chain if
 *                                                 not already present
 * node * SetIdx( node *chain, types *shape)	 : inserts IDX-node if not already present
 */

/*
 *
 *  functionname  : FindVect
 *  arguments     : 1) node * chain
 *  description   : checks whether VECT is in the chain and returns
 *                  either NULL (= no VECT in chain) or the adress
 *                  of the VECT-node
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
FindVect (node *chain)
{
    DBUG_ENTER ("FindVect");
    while (chain && (VINFO_FLAG (chain) != VECT))
        chain = VINFO_NEXT (chain);
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : EqTypes
 *  arguments     : 1) types * type1
 *                  2) types * type2
 *  description   : compares two types with respect to the shape and
 *                  returnes 1 iff the types are equal, 0 otherwise.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       : this is a helper function needed from FindIdx only!
 *
 */

int
EqTypes (types *type1, types *type2)
{
    int i, res;

    DBUG_ENTER ("EqTypes");
    if (TYPES_DIM (type1) == TYPES_DIM (type2)) {
        res = 1;
        DBUG_ASSERT ((TYPES_SHPSEG (type1) != NULL) && (TYPES_SHPSEG (type1) != NULL),
                     "EqTypes used on type without shape");
        for (i = 0; i < TYPES_DIM (type2); i++)
            if (TYPES_SHAPE (type1, i) != TYPES_SHAPE (type2, i))
                res = 0;
    } else {
        res = 0;
    }
    DBUG_RETURN (res);
}

/*
 *
 *  functionname  : FindIdx
 *  arguments     : 1) node * chain
 *                  2) types * idx-shape
 *  description   : checks whether IDX(idx-shape) is in the chain and returns
 *                  either NULL (= IDX(idx-shape) not in chain or the adress
 *                  of the IDX-node
 *  global vars   : ---
 *  internal funs : EqTypes
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
FindIdx (node *chain, types *vshape)
{
    DBUG_ENTER ("FindIdx");
    while ((chain != NULL)
           && ((VINFO_FLAG (chain) != IDX) || !EqTypes (VINFO_TYPE (chain), vshape)))
        chain = VINFO_NEXT (chain);
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : SetVect
 *  arguments     : 1) node * chain
 *  description   : inserts a Vect node in the given node-chain if there exists none
 *  global vars   : ---
 *  internal funs : FindVect, GenVect, InsertInChain
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
SetVect (node *chain)
{
    DBUG_ENTER ("SetVect");
    DBUG_PRINT ("IDX", ("VECT assigned"));
    if (FindVect (chain) == NULL) {
        chain = MakeVinfo (VECT, NULL, chain);
    }
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : SetIdx
 *  arguments     : 1) node * chain
 *                  2) types * shape to be inserted
 *  description   : inserts an IDX(shape) node in the given node-chain
 *                  if there exists none
 *  global vars   : ---
 *  internal funs : FindIdx, GenIdx, InsertInChain
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
SetIdx (node *chain, types *vartype)
{
    DBUG_ENTER ("SetIdx");
    if (FindIdx (chain, vartype) == NULL) {
        chain = MakeVinfo (IDX, vartype, chain);
        DBUG_PRINT ("IDX", ("IDX(%p) assigned", chain));
    }
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : IdxChangeId
 *  arguments     : 1) string: varname
 *                  2) types*: shape
 *                  R) string: new varname
 *  description   : appends the shape to the varname; e.g:
 *                  test, int[1,4,2,3]  =>  test_1_4_2_3__
 *                  does not free the argument space!
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

char *
IdxChangeId (char *varname, types *type)
{
    static char buffer[1024];
    static char buffer2[32];
    int i;
    int *shp;

    DBUG_ENTER ("IdxChangeId");
    sprintf (buffer, "%s", varname);
    shp = SHAPES_SELEMS (type);
    for (i = 0; i < TYPES_DIM (type); i++) {
        sprintf (buffer2, "_%d", shp[i]);
        strcat (buffer, buffer2);
    }
    sprintf (buffer2, "__");
    strcat (buffer, buffer2);
    DBUG_RETURN (StringCopy (buffer));
}

/*
 *
 *  functionname  : VardecIdx
 *  arguments     : 1) node*: N_vardec/ N_arg node
 *                  2) types*: shape
 *                  3) char *: varname
 *                  R) node*: (new) N_vardec node
 *  description   : looks up, whether the given variable exists already
 *                  (as variable declaration); if it does so, the pointer
 *                  to the declaration is given, if not a new declaration
 *                  is created.
 *
 */

node *
VardecIdx (node *vardec, types *type, char *varname)
{
    node *newvardec, *vinfo;

    DBUG_ENTER ("VardecIdx");
    vinfo = FindIdx (VARDEC_COLCHN (vardec), type);
    DBUG_ASSERT ((vinfo), "given shape not inserted in collected chain of vardec node!");
    if (VINFO_VARDEC (vinfo) == NULL) {
        newvardec
          = MakeVardec (StringCopy (varname), MakeType (T_int, 0, NULL, NULL, NULL),
                        VARDEC_NEXT (vardec));
        VARDEC_NEXT (vardec) = newvardec;
        VINFO_VARDEC (vinfo) = newvardec;
        DBUG_PRINT ("IDX",
                    ("inserting new vardec %s between %s and %s", varname,
                     VARDEC_NAME (vardec),
                     (VARDEC_NEXT (newvardec) ? VARDEC_NAME (VARDEC_NEXT (newvardec))
                                              : "NULL")));
    }
    DBUG_PRINT ("IDX", ("vinfo %p points to vardec %s", vinfo, varname));

    DBUG_RETURN (VINFO_VARDEC (vinfo));
}

/*
 *
 *  functionname  : ArgIdx
 *  arguments     : 1) node*: N_arg node
 *                  2) types*: type
 *                  3) char *: varname
 *                  R) node*: (new) N_vardec node
 *  description   : looks up, whether the given variable exists already
 *                  ( as variable declaration); if it does so, the pointer
 *                  to the declaration is given, if not a new declaration
 *                  is created.
 *
 */

node *
ArgIdx (node *arg, types *type, char *varname)
{
    node *newvardec, *vinfo, *block;

    DBUG_ENTER ("ArgIdx");
    vinfo = FindIdx (ARG_COLCHN (arg), type);
    DBUG_ASSERT ((vinfo), "given shape not inserted in collected chain of arg node!");
    if (VINFO_VARDEC (vinfo) == NULL) {
        block = FUNDEF_BODY (ARG_FUNDEF (arg));
        newvardec
          = MakeVardec (StringCopy (varname), MakeType (T_int, 0, NULL, NULL, NULL),
                        BLOCK_VARDEC (block));
        BLOCK_VARDEC (block) = newvardec;
        VINFO_VARDEC (vinfo) = newvardec;
        DBUG_PRINT ("IDX",
                    ("inserting new vardec %s between NULL and %s", varname,
                     (VARDEC_NEXT (newvardec) ? VARDEC_NAME (VARDEC_NEXT (newvardec))
                                              : "NULL")));
    }
    DBUG_PRINT ("IDX", ("vinfo %p points to vardec %s", vinfo, varname));

    DBUG_RETURN (VINFO_VARDEC (vinfo));
}

/*
 *
 *  functionname  : IdxModul
 *  arguments     :
 *  description   : Quasi-dummy
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxModul");
    if (NULL != MODUL_FUNS (arg_node))
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), NULL);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxFundef
 *  arguments     :
 *  description   : Quasi-dummy
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxFundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxFundef");
    /*
     * We have to traverse the args first, since we need backrefs
     * to this fundef-node.
     * For doing so we supply the fundef-node itself as arg_info.
     */
    if (NULL != FUNDEF_ARGS (arg_node))
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_node);
    if (NULL != FUNDEF_BODY (arg_node))
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), NULL);
    /*
     * A second pass through the arguments has to be done in order
     * to insert initialisations for index-args. This can NOT be done
     * when traversing the body since the traversal mechanism would
     * automatically eliminate the freshly inserted assign-nodes.
     * The second pass is indicated to IdxArg by a NULL arg_info!
     */
    if (NULL != FUNDEF_ARGS (arg_node))
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), NULL);

    if (NULL != FUNDEF_NEXT (arg_node))
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), NULL);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxArg
 *  arguments     : 2) arg_info contains the pointer to the fundef
 *  description   : IdxArgs inserts the backref to the withbelonging
 *                  fundef.
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxArg (node *arg_node, node *arg_info)
{
    node *newassign, *newid, *name_node, *dim_node, *dim_node2, *vardec, *col_vinfo;
    node *block, *icm_arg, *vinfo;
    int i;

    DBUG_ENTER ("IdxArg");

    if (arg_info != NULL) {
        /* This is the first pass; insert backref to fundef
         * and make sure that all ARG_COLCHN-nodes are NULL
         * before traverswing the body!
         */
        ARG_FUNDEF (arg_node) = arg_info;
    } else {
        /* This is the second pass; insert index-arg initialisations
         * of the form: ND_KS_VECT2OFFSET( <off-name>, <var-name>, <dim>, <dims>, <shape>)
         */
        block = FUNDEF_BODY (ARG_FUNDEF (arg_node));
        vardec = BLOCK_VARDEC (block);
        vinfo = ARG_COLCHN (arg_node);
        while (vinfo != NULL) { /* loop over all "Uses" attributes */
            if (VINFO_FLAG (vinfo) == IDX) {
                newid = MakeId (IdxChangeId (ARG_NAME (arg_node), VINFO_TYPE (vinfo)),
                                NULL, ST_regular);
                col_vinfo = FindIdx (ARG_COLCHN (vardec), VINFO_TYPE (vinfo));
                DBUG_ASSERT (((col_vinfo != NULL) && (VINFO_VARDEC (col_vinfo) != NULL)),
                             "missing vardec for index-arg");
                ID_VARDEC (newid) = VINFO_VARDEC (col_vinfo);

                name_node = MakeId (StringCopy (ARG_NAME (arg_node)), NULL, ST_regular);
                ID_VARDEC (name_node) = arg_node;

                dim_node = MakeNum (ARG_SHAPE (arg_node, 0));

                dim_node2 = MakeNum (VINFO_DIM (vinfo));

                CREATE_4_ARY_ICM (newassign, "ND_KS_VECT2OFFSET", newid, /* off-name */
                                  name_node,                             /* var-name */
                                  dim_node,                              /* dim of var */
                                  dim_node2); /* dim of array */

                /* Now, we append the shape elems to the ND_KS_VECT2OFFSET-ICM ! */
                for (i = 0; i < VINFO_DIM (vinfo); i++)
                    MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (VINFO_SELEMS (vinfo)[i]));

                ASSIGN_NEXT (newassign) = BLOCK_INSTR (block);
                BLOCK_INSTR (block) = newassign;
            }
            vinfo = VINFO_NEXT (vinfo);
        }
    }
    if (NULL != ARG_NEXT (arg_node))
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxAssign
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxAssign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxAssign");
    /* Bottom up traversal!! */
    if (NULL != ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_node);
    } else /* this must be the return-statement!!! */
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), NULL);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxLet
 *  arguments     : 1) node *arg_node  : N_let
 *                  2) node *arg_info  : N_assign
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxLet (node *arg_node, node *arg_info)
{
    ids *vars;
    node *vardec, *vinfo, *act_let, *newassign, *newid, *icm_arg, *col_vinfo;
    node *arg_info1;
    int i;

    DBUG_ENTER ("IdxLet");
    /* First, we attach the collected uses-attributes( they are in the
     * actual chain of the vardec) to the variables of the LHS of the
     * assignment!
     */
    vars = LET_IDS (arg_node);
    do {
        vardec = IDS_VARDEC (vars);
        if (NODE_TYPE (vardec) == N_vardec) {
            IDS_USE (vars) = VARDEC_ACTCHN (vardec);
            VARDEC_ACTCHN (vardec) = NULL; /* freeing the actual chain! */
        } else {
            DBUG_ASSERT ((NODE_TYPE (vardec) == N_arg),
                         "backref from let-var neither vardec nor arg!");
            IDS_USE (vars) = ARG_ACTCHN (vardec);
            ARG_ACTCHN (vardec) = NULL; /* freeing the actual chain! */
        }
        vars = IDS_NEXT (vars);
    } while (vars);

    /* Now, that we have done all that is needed for the "Uses" inference,
     * we do some modifications of the assignment based on the "Uses" information
     * gained.
     * Therefore, we first have to find out, what kind of RHS we are dealing with.
     *  - If it is (an arithmetic operation( +,-,*,\), a variable or a constant),
     *    AND it is neither F_mul_AxA nor F_div_AxA !!!
     *    AND the LHS variable is NOT used as VECT
     *    AND the LHS variable IS used as IDX(...):
     *    we duplicate the assignment for each IDX(shape) and traverse the new
     *    assignments with shape as the arg_info! This traversal will replace
     *    the index-array operations by integer-index operations.The original
     *    (VECT-) version is eliminated (more precisely: reused for the last
     *    IDX(...) version).
     *  - in all other cases:
     *    for each variable of the LHS that is needed as IDX(shape) we
     *    generate an assignment of the form:
     *    ND_KS_VECT2OFFSET( off-name, var-name, dim, dims, shape)
     *    this instruction will calculate the indices needed from the vector
     *    generated by the RHS.
     *    After doing so, we traverse the assignment with NULL as arg_info.
     */
    vars = LET_IDS (arg_node); /* pick the first LHS variable */
    vinfo = IDS_USE (vars);    /* pick the "Uses"set from the first LHS var */
    if ((FindVect (vinfo) == NULL) && (vinfo != NULL)
        && (((NODE_TYPE (LET_EXPR (arg_node)) == N_prf)
             && (F_add_SxA <= PRF_PRF (LET_EXPR (arg_node)))
             && (PRF_PRF (LET_EXPR (arg_node)) <= F_div_AxA)
             && (PRF_PRF (LET_EXPR (arg_node)) != F_mul_AxA)
             && (PRF_PRF (LET_EXPR (arg_node)) != F_div_AxA))

            || (NODE_TYPE (LET_EXPR (arg_node)) == N_id)
            || (NODE_TYPE (LET_EXPR (arg_node)) == N_array))) {
        DBUG_ASSERT (((NODE_TYPE (LET_EXPR (arg_node)) == N_id)
                      || (NODE_TYPE (LET_EXPR (arg_node)) == N_array)
                      || (PRF_PRF (LET_EXPR (arg_node)) == F_add_SxA)
                      || (PRF_PRF (LET_EXPR (arg_node)) == F_add_AxS)
                      || (PRF_PRF (LET_EXPR (arg_node)) == F_add_AxA)
                      || (PRF_PRF (LET_EXPR (arg_node)) == F_sub_SxA)
                      || (PRF_PRF (LET_EXPR (arg_node)) == F_sub_AxS)
                      || (PRF_PRF (LET_EXPR (arg_node)) == F_sub_AxA)
                      || (PRF_PRF (LET_EXPR (arg_node)) == F_mul_SxA)
                      || (PRF_PRF (LET_EXPR (arg_node)) == F_mul_AxS)
                      || (PRF_PRF (LET_EXPR (arg_node)) == F_div_SxA)
                      || (PRF_PRF (LET_EXPR (arg_node)) == F_div_AxS)),
                     " wrong prf sequence in \"prf_node_info.mac\"!");
        /* Here, we duplicate the operation
         * as many times as we do have different shapes in LET_USE(arg_node)
         * and successively traverse these assignments supplying the resp. types*
         * in each call as arg_info!
         */
        act_let = arg_node;
        while (vinfo != NULL) {
            DBUG_ASSERT (((NODE_TYPE (act_let) == N_let)
                          && (NODE_TYPE (arg_info) == N_assign)),
                         "wrong let/assign node generated in IdxLet!");
            DBUG_ASSERT ((NODE_TYPE (vinfo) == N_vinfo) && (VINFO_FLAG (vinfo) == IDX),
                         "wrong N_vinfo node attached to let-variable!");
            if (VINFO_NEXT (vinfo) != NULL) {
                /* There are more indices needed, so we have to duplicate the let
                 * node and repeat the let-traversal until there are no shapes left!
                 * More precisely, we have to copy the Assign node, who is the father
                 * of the let node and whose adress is given by arg_info!
                 */
                arg_info1 = arg_info->node[1];
                arg_info->node[1] = NULL;
                newassign = DupTree (arg_info, NULL);
                arg_info->node[1] = arg_info1;
                ASSIGN_NEXT (newassign) = ASSIGN_NEXT (arg_info);
                ASSIGN_NEXT (arg_info) = newassign;
            }
            LET_EXPR (act_let) = Trav (LET_EXPR (act_let), vinfo);
            LET_NAME (act_let) = IdxChangeId (LET_NAME (act_let), VINFO_TYPE (vinfo));
            LET_VARDEC (act_let)
              = VardecIdx (LET_VARDEC (act_let), VINFO_TYPE (vinfo), LET_NAME (act_let));
            vinfo = VINFO_NEXT (vinfo);
            if (vinfo != NULL) {
                arg_info = newassign;
                act_let = ASSIGN_INSTR (newassign);
            }
        }
    } else {
        /* We do not have a "pure" address calculation here!
         * Therefore, we insert for each shape-index needed for each variable of
         * the LHS an assignment of the form :
         * ND_KS_VECT2OFFSET( <off-name>, <var-name>, <dim>, <dims>, <shape> )
         */
        do { /* loop over all LHS-Vars */
            vinfo = vars->use;

            while (vinfo != NULL) { /* loop over all "Uses" attributes */
                if (VINFO_FLAG (vinfo) == IDX) {
                    node *name_node, *dim_node, *dim_node2;

                    newid = MakeId (IdxChangeId (IDS_NAME (vars), VINFO_TYPE (vinfo)),
                                    NULL, ST_regular);
                    col_vinfo = FindIdx (VARDEC_COLCHN (vardec), VINFO_TYPE (vinfo));
                    DBUG_ASSERT (((col_vinfo != NULL)
                                  && (VINFO_VARDEC (col_vinfo) != NULL)),
                                 "missing vardec for LHS let-var");
                    ID_VARDEC (newid) = VINFO_VARDEC (col_vinfo);

                    name_node = MakeId2 (DupOneIds (vars, NULL));
                    ID_VARDEC (name_node) = IDS_VARDEC (vars);

                    dim_node = MakeNum (vars->node->info.types->shpseg->shp[0]);

                    dim_node2 = MakeNum (VINFO_DIM (vinfo));

                    CREATE_4_ARY_ICM (newassign, "ND_KS_VECT2OFFSET",
                                      newid,      /* off-name */
                                      name_node,  /* var-name */
                                      dim_node,   /* dim of var */
                                      dim_node2); /* dim of array */

                    /* Now, we append the shape elems to the ND_KS_VECT2OFFSET-ICM ! */
                    for (i = 0; i < VINFO_DIM (vinfo); i++)
                        MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (VINFO_SELEMS (vinfo)[i]));

                    newassign->node[1] = arg_info->node[1];
                    arg_info->node[1] = newassign;
                    arg_info = newassign;
                }
                vinfo = VINFO_NEXT (vinfo);
            }

            if ((NODE_TYPE (LET_EXPR (arg_node)) == N_with)
                || (NODE_TYPE (LET_EXPR (arg_node)) == N_Nwith)) {
                LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_node);
            } else {
                LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), NULL);
            }
            vars = IDS_NEXT (vars);
        } while (vars != NULL);
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxPrf
 *  arguments     :
 *  description   : case prf of:
 *                    psi   : SetIdx
 *                    binop : SetVect
 *                    others: SetVect
 *  global vars   : ive_op
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxPrf (node *arg_node, node *arg_info)
{
    node *arg1, *arg2, *arg3, *vinfo;
    types *type;

    DBUG_ENTER ("IdxPrf");
    /*
     * The arg_info indicates whether this is just a normal traverse
     * (arg_info == NULL), or the transformation of an arithmetic
     * index calculation (arg_info == vinfo-node).
     */
    switch (PRF_PRF (arg_node)) {
    case F_psi:
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);
        DBUG_ASSERT (((arg2->nodetype == N_id) || (arg2->nodetype == N_array)),
                     "wrong arg in F_psi application");
        if (NODE_TYPE (arg2) == N_id)
            type = ID_TYPE (arg2);
        else
            type = ARRAY_TYPE (arg2);
        /*
         * if the shape of the array is unknown, do not(!) replace
         * psi by idx_psi but mark the selecting vector as VECT !
         * this is done by traversal with NULL instead of vinfo!
         */
        if (TYPES_SHPSEG (type) != NULL) {
            vinfo = MakeVinfo (IDX, type, NULL);
            PRF_ARG1 (arg_node) = Trav (arg1, vinfo);
            FREE (vinfo);
            PRF_PRF (arg_node) = F_idx_psi;
        } else {
            PRF_ARG1 (arg_node) = Trav (arg1, NULL);
        }
        PRF_ARG2 (arg_node) = Trav (arg2, NULL);
        break;
    case F_modarray:
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);
        arg3 = PRF_ARG3 (arg_node);
        DBUG_ASSERT (((arg1->nodetype == N_id) || (arg1->nodetype == N_array)),
                     "wrong arg in F_modarray application");
        if (NODE_TYPE (arg1) == N_id)
            type = ID_TYPE (arg1);
        else
            type = ARRAY_TYPE (arg1);
        /*
         * if the shape of the array is unknown, do not(!) replace
         * modarray by idx_modarray but mark the selecting vector as VECT !
         * this is done by traversal with NULL instead of vinfo!
         */
        if (TYPES_SHPSEG (type) != NULL) {
            vinfo = MakeVinfo (IDX, type, NULL);
            PRF_ARG2 (arg_node) = Trav (arg2, vinfo);
            FREE (vinfo);
            PRF_PRF (arg_node) = F_idx_modarray;
        } else {
            PRF_ARG2 (arg_node) = Trav (arg2, NULL);
        }
        PRF_ARG1 (arg_node) = Trav (arg1, NULL);
        PRF_ARG3 (arg_node) = Trav (arg3, NULL);
        break;
    case F_add_SxA:
    case F_add_AxS:
    case F_add_AxA:
        if (arg_info != NULL)
            PRF_PRF (arg_node) = F_add;
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        ive_op++;
        break;
    case F_sub_SxA:
    case F_sub_AxS:
    case F_sub_AxA:
        if (arg_info != NULL)
            PRF_PRF (arg_node) = F_sub;
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        ive_op++;
        break;
    case F_mul_SxA:
        if (arg_info != NULL)
            PRF_PRF (arg_node) = F_mul;
        PRF_ARG2 (arg_node) = Trav (PRF_ARG2 (arg_node), arg_info);
        ive_op++;
        break;
    case F_mul_AxS:
        if (arg_info != NULL)
            PRF_PRF (arg_node) = F_mul;
        PRF_ARG1 (arg_node) = Trav (PRF_ARG1 (arg_node), arg_info);
        ive_op++;
        break;
    case F_div_SxA:
        if (arg_info != NULL)
            PRF_PRF (arg_node) = F_div;
        PRF_ARG2 (arg_node) = Trav (PRF_ARG2 (arg_node), arg_info);
        ive_op++;
        break;
    case F_div_AxS:
        if (arg_info != NULL)
            PRF_PRF (arg_node) = F_div;
        PRF_ARG1 (arg_node) = Trav (PRF_ARG1 (arg_node), arg_info);
        ive_op++;
        break;
    default:
        DBUG_ASSERT ((arg_info == NULL), "Inconsistency between IdxLet and IdxPrf");
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        break;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxId
 *  arguments     :
 *  description   : examines whether variable is a one-dimensional array;
 *                  if so, examine arg_info:
 *                    if NULL :
 *                        SetVect on "N_vardec" belonging to the "N_id" node.
 *                    otherwise: change varname according to shape from arg_info!
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxId (node *arg_node, node *arg_info)
{
    types *type;
    node *vardec;
    char *newid;

    DBUG_ENTER ("IdxId");
    vardec = ID_VARDEC (arg_node);
    DBUG_ASSERT (((NODE_TYPE (vardec) == N_vardec) || (NODE_TYPE (vardec) == N_arg)),
                 "non vardec/arg node as backref in N_id!");
    if (NODE_TYPE (vardec) == N_vardec) {
        if (VARDEC_DIM (vardec) == 1) {
            if (arg_info == NULL) {
                DBUG_PRINT ("IDX", ("assigning VECT to %s:", ID_NAME (arg_node)));
                VARDEC_ACTCHN (vardec) = SetVect (VARDEC_ACTCHN (vardec));
                VARDEC_COLCHN (vardec) = SetVect (VARDEC_COLCHN (vardec));
            } else {
                type = VINFO_TYPE (arg_info);
                DBUG_PRINT ("IDX", ("assigning IDX to %s:", ID_NAME (arg_node)));
                VARDEC_ACTCHN (vardec) = SetIdx (VARDEC_ACTCHN (vardec), type);
                VARDEC_COLCHN (vardec) = SetIdx (VARDEC_COLCHN (vardec), type);
                newid = IdxChangeId (ID_NAME (arg_node), type);
                ID_NAME (arg_node) = newid;
                /* Now, we have to insert the respective declaration */
                /* If the declaration does not yet exist, it has to be created! */
                ID_VARDEC (arg_node) = VardecIdx (vardec, type, newid);
            }
        }
    } else {
        if (ARG_DIM (vardec) == 1) {
            if (arg_info == NULL) {
                DBUG_PRINT ("IDX", ("assigning VECT to %s:", ID_NAME (arg_node)));
                ARG_ACTCHN (vardec) = SetVect (ARG_ACTCHN (vardec));
                ARG_COLCHN (vardec) = SetVect (ARG_COLCHN (vardec));
            } else {
                type = VINFO_TYPE (arg_info);
                DBUG_PRINT ("IDX", ("assigning IDX to %s:", ID_NAME (arg_node)));
                ARG_ACTCHN (vardec) = SetIdx (ARG_ACTCHN (vardec), type);
                ARG_COLCHN (vardec) = SetIdx (ARG_COLCHN (vardec), type);
                newid = IdxChangeId (ID_NAME (arg_node), type);
                ID_NAME (arg_node) = newid;
                /* Now, we have to insert the respective declaration */
                /* If the declaration does not yet exist, it has to be created! */
                ID_VARDEC (arg_node) = ArgIdx (vardec, type, newid);
            }
        }
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxArray
 *  arguments     : 1) node*: N_array node
 *                  2) node*: vinfo
 *                  R) node*: index of N_array in unrolling of shape
 *  description   : if vinfo == NULL all Array-Elements are traversed with
 *                  arg_info NULL, since there may be some array_variables;
 *                  otherwise the index of the vector N_array in
 *                  the unrolling of an array of shape from vinfo is calculated; e.g.
 *                  [ 2, 3, 1] in int[7,7,7] => 2*49 + 3*7 +1 = 120
 *                  [ 2] in int[7, 7, 7] => 2*49 = 98
 *                  Since we may have identifyers as components, this calculation
 *                  is not done, but generated as syntax-tree!!!
 *                  WARNING!  this penetrates the flatten-consistency!!
 *  global vars   : ive_expr
 *
 */

node *
IdxArray (node *arg_node, node *arg_info)
{
    int i;
    int *shp;
    node *idx;
    node *expr;

    DBUG_ENTER ("IdxArray");
    if (arg_info == NULL) {
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
    } else {
        expr = ARRAY_AELEMS (arg_node);
        shp = VINFO_SELEMS (arg_info);
        idx = EXPRS_EXPR (expr);
        expr = EXPRS_NEXT (expr);
        for (i = 1; i < VINFO_DIM (arg_info); i++) {
            if (expr != NULL) {
                DBUG_ASSERT ((NODE_TYPE (expr) == N_exprs),
                             "corrupted syntax tree at N_array(N_exprs expected)!");
                idx = MakeExprs (idx, MakeExprs (MakeNum (shp[i]), NULL));
                idx = MakePrf (F_mul, idx);
                idx = MakeExprs (idx, expr);
                expr = EXPRS_NEXT (expr);
                EXPRS_NEXT (EXPRS_NEXT (idx)) = NULL;
                idx = MakePrf (F_add, idx);
            } else {
                idx = MakeExprs (idx, MakeExprs (MakeNum (shp[i]), NULL));
                idx = MakePrf (F_mul, idx);
            }
        }
        arg_node = idx;
        ive_expr++;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxNum
 *  arguments     : 1) node*: N_num node
 *                  2) types*: shape
 *                  R) node*: index of N_array in unrolling of shape
 *  description   : if shape == NULL nothing has to be done; otherwise
 *                  the index of the vector N_array in
 *                  the unrolling of an array of shape is calculated;
 *
 */

node *
IdxNum (node *arg_node, node *arg_info)
{
    int val, i, sum = 0;
    int *shp;

    DBUG_ENTER ("IdxNum");
    if (arg_info != NULL) {
        DBUG_ASSERT ((NODE_TYPE (arg_info) == N_vinfo),
                     "corrupted arg_info node in IdxNum!");
        shp = VINFO_SELEMS (arg_info);
        val = NUM_VAL (arg_node);
        for (i = 1; i < VINFO_DIM (arg_info); i++) {
            sum = (sum + val) * shp[i];
        }
        NUM_VAL (arg_node) = (sum + val);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IdxWith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
IdxWith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxWith");
    /* Bottom up traversal; the OPERATOR is a N_modarray,
     * N_genarray, N_foldprf, or N_foldfun node.
     * all these nodes do have ordenary N_block-nodes attached!
     */
    WITH_OPERATOR (arg_node) = Trav (WITH_OPERATOR (arg_node), NULL);

    /* When traversing the generator, we potentially want to insert some
     * index conversions into the body; therfore, we supply the
     * N_let node as surplus argument; it does not suffice to
     * submit the body (N_genarray, N_modarray or N_fold node)
     * since the name of the variable that will be generated/modified
     * is needed for the creation of the ND_KS_USE_GENVAR_OFFSET ICM !
     */
    WITH_GEN (arg_node) = Trav (WITH_GEN (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IdxGenerator(node *arg_node, node *arg_info)
 *
 * description:
 *
 * remark:
 *   arg_info contains prior N_let node.
 *
 ******************************************************************************/

node *
IdxGenerator (node *arg_node, node *arg_info)
{
    node *vardec, *vinfo, *block, *icm_arg, *newassign, *newid, *arrayid;
    node *name_node, *dim_node, *dim_node2, *body, *colvinfo;
    types *artype;
    int i;

    DBUG_ENTER ("IdxGenerator");
    DBUG_ASSERT ((NODE_TYPE (arg_info) == N_let), "arg_info is not a N_let-node");
    DBUG_ASSERT ((NODE_TYPE (LET_EXPR (arg_info)) == N_with),
                 "N_let node in arg_info contains no with-loop");

    body = WITH_OPERATOR (LET_EXPR (arg_info));
    GEN_LEFT (arg_node) = Trav (GEN_LEFT (arg_node), NULL);
    GEN_RIGHT (arg_node) = Trav (GEN_RIGHT (arg_node), NULL);
    vardec = GEN_VARDEC (arg_node);
    vinfo = VARDEC_ACTCHN (vardec);

    /* first, we memorize the actuall chain */
    GEN_USE (arg_node) = vinfo;
    /* then, we remove the actual chain! */
    VARDEC_ACTCHN (vardec) = NULL;

    /* for each IDX-vinfo-node we have to instanciate the respective
     * variable as first statement in the body of the with loop.
     * for doing so, we need the root of the body which is supplied
     * via arg_info by IdxWith!
     */
    DBUG_PRINT ("IDX", ("introducing idx-vars in with-bodies"));
    while (vinfo != NULL) {
        DBUG_PRINT ("IDX", ("examining vinfo(%p)", vinfo));
        if (VINFO_FLAG (vinfo) == IDX) {
            artype = LET_TYPE (arg_info);
            DBUG_ASSERT ((artype != NULL), "missing type-info for LHS of let!");
            switch (body->nodetype) {
            case N_modarray:
                block = MODARRAY_BODY (body);
                break;
            case N_genarray:
                block = GENARRAY_BODY (body);
                break;
            case N_foldprf:
                block = FOLDPRF_BODY (body);
                break;
            case N_foldfun:
                block = FOLDFUN_BODY (body);
                break;
            default:
                DBUG_ASSERT ((0 != 0), "unknown generator type in IdxGenerator");
            }

            if (((NODE_TYPE (body) == N_modarray) || (NODE_TYPE (body) == N_genarray))
                && EqTypes (VINFO_TYPE (vinfo), artype)) {
                /*
                 * we can reuse the genvar as index directly!
                 * therefore we create an ICM of the form:
                 * ND_KS_USE_GENVAR_OFFSET( <idx-varname>, <result-array-varname>)
                 */
                newid
                  = MakeId (IdxChangeId (GEN_ID (arg_node), artype), NULL, ST_regular);
                colvinfo = FindIdx (VARDEC_COLCHN (vardec), artype);
                DBUG_ASSERT (((colvinfo != NULL) && (VINFO_VARDEC (colvinfo) != NULL)),
                             "missing vardec for IDX variable!");
                ID_VARDEC (newid) = VINFO_VARDEC (colvinfo);
                arrayid = MakeId (StringCopy (LET_NAME (arg_info)), NULL, ST_regular);
                /*
                 * The backref of the arrayid is set wrongly to the actual
                 * integer index-variable. This is done on purpose for
                 * fooling the refcount-inference system.
                 * The "correct" backref would be LET_VARDEC( arg_info) !
                 */
                ID_VARDEC (arrayid) = VINFO_VARDEC (colvinfo);

                CREATE_2_ARY_ICM (newassign, "ND_KS_USE_GENVAR_OFFSET", newid, arrayid);
            } else {
                /*
                 * we have to instanciate the idx-variable by an ICM of the form:
                 * ND_KS_VECT2OFFSET( <off-name>, <var-name>,
                 *                    <dim of var>, <dim of array>, shape_elems)
                 */
                newid = MakeId (IdxChangeId (GEN_ID (arg_node), VINFO_TYPE (vinfo)), NULL,
                                ST_regular);
                colvinfo = FindIdx (VARDEC_COLCHN (vardec), VINFO_TYPE (vinfo));
                DBUG_ASSERT (((colvinfo != NULL) && (VINFO_VARDEC (colvinfo) != NULL)),
                             "missing vardec for IDX variable");
                ID_VARDEC (newid) = VINFO_VARDEC (colvinfo);

                name_node = MakeId (StringCopy (GEN_ID (arg_node)), NULL, ST_regular);
                ID_VARDEC (name_node) = vardec;

                dim_node = MakeNum (VARDEC_SHAPE (vardec, 0));

                dim_node2 = MakeNum (VINFO_DIM (vinfo));

                CREATE_4_ARY_ICM (newassign, "ND_KS_VECT2OFFSET", newid, /* off-name */
                                  name_node,                             /* var-name */
                                  dim_node,                              /* dim of var */
                                  dim_node2); /* dim of array */

                /* Now, we append the shape elems to the ND_KS_VECT2OFFSET-ICM ! */
                for (i = 0; i < VINFO_DIM (vinfo); i++)
                    MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (VINFO_SELEMS (vinfo)[i]));
            }
            ASSIGN_NEXT (newassign) = BLOCK_INSTR (block);
            BLOCK_INSTR (block) = newassign;
        }
        vinfo = VINFO_NEXT (vinfo);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IdxNwith( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
IdxNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxNwith");

    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), NULL);
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), NULL);

    /*
     * We traverse the code-blocks at last!
     * We potentially want to insert some index conversions into the body;
     * therfore, we supply the N_let node as surplus argument.
     */

    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IdxNcode( node *arg_node, node *arg_info)
 *
 * description:
 *   arg_info points to the previous N_let node!
 *
 ******************************************************************************/

node *
IdxNcode (node *arg_node, node *arg_info)
{
    node *with, *idx_vardec, *vinfo, *withop_arr, *col_vinfo, *new_assign, *icm_arg,
      *new_id, *array_id, *name_node, *dim_node, *dim_node2;
    types *arr_type;
    int i;

    DBUG_ENTER ("IdxNcode");

    /*
     * we traverse the current code block
     */

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), NULL);
    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), NULL);

    DBUG_ASSERT (((NODE_TYPE (arg_info) == N_let)
                  && (NODE_TYPE (LET_EXPR (arg_info)) == N_Nwith)),
                 "arg_info contains no let with a with-loop on the RHS!");

    /*
     * we insert instances of the index-vector-var as first statement of the
     *  current code block.
     */

    with = LET_EXPR (arg_info);
    idx_vardec = IDS_VARDEC (NWITH_VEC (with));
    vinfo = VARDEC_ACTCHN (idx_vardec);
    VARDEC_ACTCHN (idx_vardec) = NULL;

    while (vinfo != NULL) {

        if (VINFO_FLAG (vinfo) == IDX) {
            arr_type = LET_TYPE (arg_info);
            DBUG_ASSERT ((arr_type != NULL), "missing type-info for LHS of let!");

            switch (NWITH_TYPE (with)) {

            case WO_modarray:
                withop_arr = NWITHOP_ARRAY (NWITH_WITHOP (with));
                break;

            case WO_genarray:
                withop_arr = NWITHOP_SHAPE (NWITH_WITHOP (with));
                DBUG_ASSERT ((NODE_TYPE (withop_arr) == N_array),
                             "shape of genarray is not N_array");
                break;

            case WO_foldprf:
                /* here is no break missing! */
            case WO_foldfun:
                break;

            default:
                DBUG_ASSERT ((0), "wrong with-loop type");
            }

            if (((NWITH_TYPE (with) == WO_modarray) || (NWITH_TYPE (with) == WO_genarray))
                && EqTypes (VINFO_TYPE (vinfo), arr_type)) {

                /*
                 * we can reuse the genvar as index directly!
                 * therefore we create an ICM of the form:
                 * ND_KS_USE_GENVAR_OFFSET( <idx-varname>, <result-array-varname>)
                 */

                new_id = MakeId (IdxChangeId (IDS_NAME (NWITH_VEC (with)), arr_type),
                                 NULL, ST_regular);
                col_vinfo = FindIdx (VARDEC_COLCHN (idx_vardec), arr_type);
                DBUG_ASSERT (((col_vinfo != NULL) && (VINFO_VARDEC (col_vinfo) != NULL)),
                             "missing vardec for IDX variable");
                ID_VARDEC (new_id) = VINFO_VARDEC (col_vinfo);
                array_id = MakeId (StringCopy (LET_NAME (arg_info)), NULL, ST_regular);

                /*
                 * The backref of the arrayid is set wrongly to the actual
                 * integer index-variable. This is done on purpose for
                 * fooling the refcount-inference system.
                 * The "correct" backref would be LET_VARDEC( arg_info) !
                 */
                ID_VARDEC (array_id) = VINFO_VARDEC (col_vinfo);

                CREATE_2_ARY_ICM (new_assign, "ND_KS_USE_GENVAR_OFFSET", new_id,
                                  array_id);
            } else {

                /*
                 * we have to instanciate the idx-variable by an ICM of the form:
                 * ND_KS_VECT2OFFSET( <off-name>, <var-name>,
                 *                    <dim of var>, <dim of array>, shape_elems)
                 */

                new_id
                  = MakeId (IdxChangeId (IDS_NAME (NWITH_VEC (with)), VINFO_TYPE (vinfo)),
                            NULL, ST_regular);
                col_vinfo = FindIdx (VARDEC_COLCHN (idx_vardec), VINFO_TYPE (vinfo));
                DBUG_ASSERT (((col_vinfo != NULL) && (VINFO_VARDEC (col_vinfo) != NULL)),
                             "missing vardec for IDX variable");
                ID_VARDEC (new_id) = VINFO_VARDEC (col_vinfo);

                name_node
                  = MakeId (StringCopy (IDS_NAME (NWITH_VEC (with))), NULL, ST_regular);
                ID_VARDEC (name_node) = idx_vardec;

                dim_node = MakeNum (VARDEC_SHAPE (idx_vardec, 0));

                dim_node2 = MakeNum (VINFO_DIM (vinfo));

                CREATE_4_ARY_ICM (new_assign, "ND_KS_VECT2OFFSET", new_id, /* off-name */
                                  name_node,                               /* var-name */
                                  dim_node,   /* dim of var */
                                  dim_node2); /* dim of array */

                /*
                 * Now, we append the shape elems to the 'ND_KS_VECT2OFFSET' ICM
                 */
                for (i = 0; i < VINFO_DIM (vinfo); i++)
                    MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (VINFO_SELEMS (vinfo)[i]));
            }

            ASSIGN_NEXT (new_assign) = BLOCK_INSTR (NCODE_CBLOCK (arg_node));
            BLOCK_INSTR (NCODE_CBLOCK (arg_node)) = new_assign;
        }

        vinfo = VINFO_NEXT (vinfo);
    }

    /*
     * now we traverse the next code block
     */

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
