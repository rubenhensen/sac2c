/*
 * $Log$
 * Revision 1.6  1995/10/06 17:07:47  cg
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

#include "access_macros.h"

#include "compile.h"

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
 * on those arrays( array identifiers) that are used as index in a selection at
 * least once!
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
 *     node[1]           : withbelonging shape if info.use == IDX
 *
 * there are two chaines of such nodes attached to the N_vardec/N_arg nodes:
 * The "actual ones" at node[1] and the "collected ones" at node[2] which
 * are used later for the insertion of new variable delarations.
 *
 * So, whenever an array identifier is met in index-position of an array-selection,
 * the respective IDX(...) attribute is attached to the variable's declaration.
 *
 * When meeting a "N_let" with an array variable to be defined, the actual info
 * node(s) attached to the variable's declaration( "actual chain")
 * are inserted in the ids structure at ids->use;
 * subsequently copies of all nodes from the "actual chain" are appended to the
 * "collected chain" of the vardec/arg node as long as there are no dublicates.
 *
 */

/*
 * Some functions for handling N_vinfo nodes:
 * ------------------------------------------
 * node * GenVect()				 : generates a new Vect-node
 *
 * node * GenIdx( types *shape)			 : generates a new Idx-node
 *
 * node * FindVect( node *N_vinfo_chain)	 : checks whether VECT is in the chain
 *						   of N_vinfo nodes
 * node * FindIdx( node *chain, types *shape)	 : dito for a specific shape
 *
 * node * InsertInChain( node *node, node *chain): prepands the node to the chain
 *
 * node * SetVect( node *chain)			 : inserts VECT-node in chain if
 *                                                 not already present
 * node * SetIdx( node *chain, types *shape)	 : inserts IDX-node if not already present
 */

/*
 *
 *  functionname  : GenVect
 *  arguments     : ---
 *  description   : generates a new N_vinfo node containing VECT!
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNode
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
GenVect ()
{
    node *vnode;

    DBUG_ENTER ("GenVect");
    vnode = MakeNode (N_vinfo);
    vnode->info.use = VECT;
    vnode->nnode = 0;
    DBUG_RETURN (vnode);
}

/*
 *
 *  functionname  : GenIdx
 *  arguments     : 1) types * shape
 *  description   : generates a new N_vinfo node containing SHAPE(shape)
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNode
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
GenIdx (types *vshape)
{
    node *vnode;

    DBUG_ENTER ("GenIdx");
    vnode = MakeNode (N_vinfo);
    vnode->info.use = IDX;
    vnode->nnode = 0;
    vnode->node[1] = (node *)vshape;
    DBUG_RETURN (vnode);
}

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
    while (chain && (chain->info.use != VECT))
        chain = chain->node[0];
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
    if (type1->dim == type2->dim) {
        res = 1;
        for (i = 0; i < type2->dim; i++)
            if (type1->shpseg->shp[i] != type2->shpseg->shp[i])
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
           && ((chain->info.use != IDX) || !EqTypes ((types *)chain->node[1], vshape)))
        chain = chain->node[0];
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : InsertInChain
 *  arguments     : 1) node * elem to be inserted
 *                  2) node * chain
 *  description   : prepands elem to chain
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
InsertInChain (node *elem, node *chain)
{
    DBUG_ENTER ("InsertInChain");
    if (chain) {
        elem->node[0] = chain;
        elem->nnode = 1;
    }
    DBUG_RETURN (elem);
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
    if (FindVect (chain) == NULL)
        chain = InsertInChain (GenVect (), chain);
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
    if (FindIdx (chain, vartype) == NULL)
        chain = InsertInChain (GenIdx (vartype), chain);
    DBUG_PRINT ("IDX", ("IDX(%p) assigned", chain));
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : ChgId
 *  arguments     : 1) string: varname
 *                  2) types*: shape
 *                  R) string: new varname
 *  description   : appends the shape to the varname; e.g:
 *                  test, int[1,4,2,3]  =>  __test_1_4_2_3
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
ChgId (char *varname, types *shape)
{
    static char buffer[1024];
    static char buffer2[32];
    int i;
    int *shp;

    DBUG_ENTER ("ChgId");
    sprintf (buffer, "__%s", varname);
    shp = shape->shpseg->shp;
    for (i = 0; i < shape->dim; i++) {
        sprintf (buffer2, "_%d", shp[i]);
        strcat (buffer, buffer2);
    }
    DBUG_RETURN (strdup (buffer));
}

/*
 *
 *  functionname  : VardecIdx
 *  arguments     : 1) node*: N_vardec/ N_arg node
 *                  2) types*: shape
 *                  3) char *: varname
 *                  R) node*: (new) N_vardec node
 *  description   : looks up, whether the given variable exists already
 *                  ( as variable declaration); if it does so, the pointer
 *                  to the declaration is given, if not a new declaration
 *                  is created.
 *  global vars   :
 *  internal funs :
 *  external funs : MakeNode
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
VardecIdx (node *vardec, types *shape, char *varname)
{
    node *newvardec, *vinfo, *vinfo1;

    DBUG_ENTER ("VardecIdx");
    vinfo = FindIdx (vardec->node[2], shape);
    DBUG_ASSERT ((vinfo), "given shape not inserted in vardec node!");
    vinfo1 = FindIdx (vardec->node[1], shape);
    DBUG_ASSERT ((vinfo1), "given shape not inserted in vardec node!");
    if (vinfo->node[2] == NULL) {
        newvardec = MakeNode (N_vardec);
        vinfo1->node[2] = newvardec;
        vinfo->node[2] = newvardec;
        newvardec->info.types = MakeTypes (T_int);
        newvardec->info.types->id = varname;
        /* inserting the new vardec */
        DBUG_PRINT ("IDX",
                    ("inserting new vardec %s between %s and %s", varname,
                     vardec->info.types->id,
                     (vardec->node[0] ? vardec->node[0]->info.types->id : "NULL")));
        newvardec->info.types->next = NULL;
        newvardec->node[0] = vardec->node[0];
        newvardec->nnode = vardec->nnode;
        vardec->node[0] = newvardec;
        vardec->nnode = 1;
    }
    DBUG_PRINT ("IDX", ("vinfo %p points to vardec %s", vinfo, varname));

    DBUG_RETURN (vinfo->node[2]);
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
    if (NULL != arg_node->node[2])
        arg_node->node[2] = Trav (arg_node->node[2], NULL);
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
    int i;

    DBUG_ENTER ("IdxFundef");
    for (i = 0; i < 3; i++)
        if (NULL != arg_node->node[i])
            arg_node->node[i] = Trav (arg_node->node[i], NULL);
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
    if (arg_node->node[1])
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    arg_node->node[0] = Trav (arg_node->node[0], arg_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxWith
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
IdxWith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxWith");
    /* Bottom up traversal!! */
    arg_node->node[1] = Trav (arg_node->node[1], NULL);
    /* When traversing the generator, we potentially want to insert some
     * index conversions into the body; therfore, we supply the
     * N_assign node as surplus argument; it does not surfice to
     * submit the body (N_genarray, N_modarray or N_fold node)
     * since the name of the variable that will be generated/modified
     * is needed for the creation of the ND_KS_USE_GENVAR_OFFSET ICM !
     */
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxGenerator
 *  arguments     : 2) node * arg_info ; prior N_let node!
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
IdxGenerator (node *arg_node, node *arg_info)
{
    node *vardec, *vinfo, *block, *icm_arg, *newassign, *newid, *arrayid;
    node *name_node, *dim_node, *dim_node2, *body;
    int i;

    DBUG_ENTER ("IdxGenerator");
    DBUG_ASSERT ((arg_info->nodetype == N_let), "wrong arg_info node (!N_let)!");
    DBUG_ASSERT ((arg_info->node[0]->nodetype == N_with),
                 "wrong arg_info node (!N_with)!");
    body = arg_info->node[0]->node[1];
    arg_node->node[0] = Trav (arg_node->node[0], NULL);
    arg_node->node[1] = Trav (arg_node->node[1], NULL);
    vardec = arg_node->info.ids->node;
    vinfo = vardec->node[1];
    /* first, we memorize the actuall chain */
    arg_node->info.ids->use = vinfo;
    /* then, we free the actual chain! */
    vardec->node[1] = NULL;
    /* for each IDX-vinfo-node we have to instanciate the respective
     * variable as first statement in the body of the with loop.
     * for doing so, we need the root of the body which is supplied
     * via arg_info by IdxWith!
     */
    DBUG_PRINT ("IDX", ("introducing idx-vars in with-bodies"));
    while (vinfo != NULL) {
        DBUG_PRINT ("IDX", ("examining vinfo(%p)", vinfo));
        if (vinfo->info.use == IDX) {
            switch (body->nodetype) {
            case N_genarray:
            case N_modarray:
                DBUG_ASSERT ((body->node[0]->nodetype == N_id),
                             "non variable in modarray expression!");
                block = body->node[1];
                break;
            case N_foldprf:
            case N_foldfun:
                block = body->node[0];
                break;
            default:
                DBUG_ASSERT ((0 != 0), "unknown generator type in IdxGenerator");
            }

            if (((body->nodetype == N_modarray) || (body->nodetype == N_genarray))
                && EqTypes ((types *)vinfo->node[1],
                            body->node[0]->info.ids->node->info.types)) {
                MAKENODE_ID (newid,
                             ChgId (arg_node->info.ids->id, (types *)vinfo->node[1]));
                DBUG_ASSERT ((vinfo->node[2] != NULL),
                             "missing vardec for IDX variable!");
                newid->info.ids->node = vinfo->node[2];
                MAKENODE_ID (arrayid, arg_info->info.ids->id);
#if 0
          arrayid->info.ids->node=body->node[0]->info.ids->node;
#else
                arrayid->info.ids->node = vinfo->node[2];
#endif
                CREATE_2_ARY_ICM (newassign, "ND_KS_USE_GENVAR_OFFSET", newid, arrayid);
            } else {
                name_node = MakeNode (N_id);
                name_node->info.ids = MakeIds (arg_node->info.ids->id, NULL, ST_regular);
                name_node->info.ids->node = arg_node->info.ids->node;
                dim_node = MakeNum (arg_node->info.ids->node->info.types->shpseg->shp[0]);
                dim_node2 = MakeNum (VINFO_DIM (vinfo));
                CREATE_3_ARY_ICM (newassign, "ND_KS_VECT2OFFSET",
                                  name_node,  /* var-name */
                                  dim_node,   /* dim of var*/
                                  dim_node2); /* dim of array*/

                /* Now, we append the shape elems to the ND_KS_VECT2OFFSET-ICM ! */
                for (i = 0; i < VINFO_DIM (vinfo); i++)
                    MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (VINFO_SELEMS (vinfo)[i]));
            }
            newassign->node[1] = block->node[0];
            newassign->nnode = 2;
            block->node[0] = newassign;
        }
        vinfo = vinfo->node[0];
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxLet
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
IdxLet (node *arg_node, node *arg_info)
{
    ids *vars;
    types *shp;
    node *vardec, *vinfo, *act_let, *newassign, *icm_arg;
    int nnode, i;

    DBUG_ENTER ("IdxLet");
    /* First, we attach the collected uses-attributes( they are in the
     * actual chain of the vardec) to the variables of the LHS of the
     * assignment!
     */
    vars = arg_node->info.ids;
    do {
        vardec = vars->node;
        vars->use = vardec->node[1];
        vardec->node[1] = NULL; /* freeing the actual chain! */
        vars = vars->next;
    } while (vars);

    /* Now, that we have done all that is needed for the "Uses" inference,
     * we do some modifications of the assignment based on the "Uses" information
     * gained.
     * Therefore, we first have to find out, what kind of RHS we are dealing with.
     *  - If it is a arithmetic operation( +,-,*,\), a variable or a constant,
     *    AND the LHS variable is NOT used as VECT:
     *    we duplicate the assignment for each IDX(shape) and traverse the new
     *    assignments with shape as the arg_info! This traversal will replace
     *    the index-array operations by integer-index operations.The original
     *    (VECT-) version is eliminated (more precisely: reused for the last
     *    IDX(...) version).
     *  - in all other cases:
     *    for each variable of the LHS that is needed as IDX(shape) we
     *    generate an assignment of the form:
     *    ND_KS_VECT2OFFSET( var, dim, shape)
     *    this instruction will calculate the indices needed from the vector
     *    generated by the RHS.
     */
    vars = arg_node->info.ids; /* pick the first LHS variable */
    vinfo = vars->use;         /* pick the "Uses"set from the first LHS var */
    if ((FindVect (vinfo) == NULL)
        && (((arg_node->node[0]->nodetype == N_prf)
             && (F_add_SxA <= arg_node->node[0]->info.prf)
             && (arg_node->node[0]->info.prf <= F_div_AxA))

            || (arg_node->node[0]->nodetype == N_id)
            || (arg_node->node[0]->nodetype == N_array))) {
        DBUG_ASSERT (((arg_node->node[0]->nodetype == N_id)
                      || (arg_node->node[0]->nodetype == N_array)
                      || (arg_node->node[0]->info.prf == F_add_SxA)
                      || (arg_node->node[0]->info.prf == F_add_AxS)
                      || (arg_node->node[0]->info.prf == F_add_AxA)
                      || (arg_node->node[0]->info.prf == F_sub_SxA)
                      || (arg_node->node[0]->info.prf == F_sub_AxS)
                      || (arg_node->node[0]->info.prf == F_sub_AxA)
                      || (arg_node->node[0]->info.prf == F_mul_SxA)
                      || (arg_node->node[0]->info.prf == F_mul_AxS)
                      || (arg_node->node[0]->info.prf == F_mul_AxA)
                      || (arg_node->node[0]->info.prf == F_div_SxA)
                      || (arg_node->node[0]->info.prf == F_div_AxS)
                      || (arg_node->node[0]->info.prf == F_div_AxA)),
                     " wrong prf sequence in \"prf_node_info.mac\"!");
        /* Here, we duplicate the operation
         * as many times as we do have different shapes in 'arg_node->info.ids->use'
         * and successively traverse these assignments supplying the resp. types*
         * in each call as arg_info!
         */
        act_let = arg_node;
        while (vinfo != NULL) {
            DBUG_ASSERT (((act_let->nodetype == N_let)
                          && (arg_info->nodetype == N_assign)),
                         "wrong let/assign node generated in IdxLet!");
            DBUG_ASSERT ((vinfo->nodetype == N_vinfo) && (vinfo->info.use == IDX),
                         "wrong N_vinfo node attached to let-variable!");
            shp = (types *)vinfo->node[1];
            vinfo = vinfo->node[0];
            if (vinfo != NULL) {
                /* There are more indices needed, so we have to duplicate the let
                 * node and repeat the let-traversal until there are no shapes left!
                 * More precisely, we have to copy the Assign node, who is the father
                 * of the let node and whose adress is given by arg_info!
                 */
                nnode = arg_info->nnode;
                arg_info->nnode = 1; /* only one assignment is to be copied! */
                newassign = DupTree (arg_info, NULL);
                arg_info->nnode = 2;
                newassign->node[1] = arg_info->node[1];
                newassign->nnode = nnode;
                arg_info->node[1] = newassign;
                arg_info->nnode = 2;
            }
            act_let->node[0] = Trav (act_let->node[0], (node *)shp);
            act_let->info.ids->id = ChgId (act_let->info.ids->id, shp);
            arg_info = newassign;
            act_let = newassign->node[0];
        }
    } else {
        /* We do not have a "pure" address calculation here!
         * Therefore, we insert for each shape-index needed for each variable of
         * the LHS an assignment of the form :
         * ND_KS_VECT2OFFSET( <varname>, <dim>, <shape>)
         */
        do { /* loop over all LHS-Vars */
            vinfo = vars->use;

            while (vinfo != NULL) { /* loop over all "Uses" attributes */
                if (vinfo->info.use == IDX) {
                    node *name_node, *dim_node, *dim_node2;

                    nnode = arg_info->nnode;
                    name_node = MakeNode (N_id);
                    name_node->info.ids = MakeIds (vars->id, NULL, ST_regular);
                    name_node->info.ids->node = vars->node;
                    dim_node = MakeNum (vars->node->info.types->shpseg->shp[0]);
                    dim_node2 = MakeNum (VINFO_DIM (vinfo));
                    CREATE_3_ARY_ICM (newassign, "ND_KS_VECT2OFFSET",
                                      name_node,  /* var-name */
                                      dim_node,   /* dim of var */
                                      dim_node2); /* dim of array */

                    /* Now, we append the shape elems to the ND_KS_VECT2OFFSET-ICM ! */
                    for (i = 0; i < VINFO_DIM (vinfo); i++)
                        MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (VINFO_SELEMS (vinfo)[i]));

                    newassign->node[1] = arg_info->node[1];
                    newassign->nnode = nnode;
                    arg_info->node[1] = newassign;
                    arg_info->nnode = 2;
                    arg_info = newassign;
                }
                vinfo = vinfo->node[0];
            }

            if (arg_node->node[0]->nodetype == N_with)
                arg_node->node[0] = Trav (arg_node->node[0], arg_node);
            else
                arg_node->node[0] = Trav (arg_node->node[0], NULL);
            vars = vars->next;
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
 *  global vars   :
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
    node *arg1, *arg2;
    types *shp;

    DBUG_ENTER ("IdxPrf");
    switch (arg_node->info.prf) {
    case F_psi:
        arg1 = arg_node->node[0]->node[0];
        arg2 = arg_node->node[0]->node[1]->node[0];
        DBUG_ASSERT (((arg2->nodetype == N_id) || (arg2->nodetype == N_array)),
                     "wrong arg in F_psi application");
        shp = arg2->info.ids->node->info.types;
        arg_node->node[0]->node[0] = Trav (arg1, (node *)shp);
        arg_node->info.prf = F_idx_psi;
        Trav (arg2, arg_info);
        break;
    case F_add_SxA:
    case F_add_AxS:
    case F_add_AxA:
        if (arg_info != NULL)
            arg_node->info.prf = F_add;
        Trav (arg_node->node[0], arg_info);
        break;
    case F_sub_SxA:
    case F_sub_AxS:
    case F_sub_AxA:
        if (arg_info != NULL)
            arg_node->info.prf = F_sub;
        Trav (arg_node->node[0], arg_info);
        break;
    case F_mul_SxA:
    case F_mul_AxS:
    case F_mul_AxA:
        if (arg_info != NULL)
            arg_node->info.prf = F_mul;
        Trav (arg_node->node[0], arg_info);
        break;
    case F_div_SxA:
    case F_div_AxS:
    case F_div_AxA:
        if (arg_info != NULL)
            arg_node->info.prf = F_div;
        Trav (arg_node->node[0], arg_info);
        break;
    default:
        Trav (arg_node->node[0], arg_info);
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
 *                         if NULL: SetVect on "N_vardec" belonging to the "N_id" node.
 *                         otherwise: change varname according to shape from arg_info!
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
    types *shp;
    node *vardec;
    char *newid;

    DBUG_ENTER ("IdxId");
    vardec = arg_node->info.ids->node;
    DBUG_ASSERT (((vardec->nodetype == N_vardec) || (vardec->nodetype == N_arg)),
                 "non vardec node as backref in N_id!");
    if (vardec->info.types->dim == 1) {
        if (arg_info == NULL) {
            DBUG_PRINT ("IDX", ("assigning VECT to %s:", arg_node->info.ids->id));
            vardec->node[1] = SetVect (vardec->node[1]);
            vardec->node[2] = SetVect (vardec->node[2]);
        } else {
            shp = (types *)arg_info;
            DBUG_PRINT ("IDX", ("assigning IDX to %s:", arg_node->info.ids->id));
            vardec->node[1] = SetIdx (vardec->node[1], shp);
            vardec->node[2] = SetIdx (vardec->node[2], shp);
            newid = ChgId (arg_node->info.ids->id, shp);
            arg_node->info.ids->id = newid;
            /* Now, we have to insert the respective declaration */
            /* If the declaration does not yet exist, it has to be created! */
            arg_node->info.ids->node = VardecIdx (vardec, shp, newid);
        }
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxArray
 *  arguments     : 1) node*: N_array node
 *                  2) types*: shape
 *                  R) node*: index of N_array in unrolling of shape
 *  description   : if shape == NULL nothing has to be done; otherwise
 *                  the index of the vector N_array in
 *                  the unrolling of an array of shape is calculated; e.g.
 *                  [ 2, 3, 1] in int[7,7,7] => 2*49 + 3*7 +1 = 120
 *                  [ 2] in int[7, 7, 7] => 2*49 = 98
 *  global vars   :
 *  internal funs :
 *  external funs : MakeNode
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxArray (node *arg_node, node *arg_info)
{
    int i;
    int *shp;
    int idx = 0;
    node *expr;

    DBUG_ENTER ("IdxArray");
    if (arg_info != NULL) {
        expr = arg_node->node[0];
        shp = ((types *)arg_info)->shpseg->shp;
        for (i = 1; i < ((types *)arg_info)->dim; i++) {
            if (expr != NULL) {
                DBUG_ASSERT ((expr->nodetype == N_exprs),
                             "corrupted syntax tree at N_array(N_exprs expected)!");
                if (expr->node[0]->nodetype == N_id)
                    DBUG_ASSERT ((NULL), "Sorry, no vars in arrays implemented in "
                                         "index-opt yet!");
                else {
                    DBUG_ASSERT ((expr->node[0]->nodetype == N_num),
                                 "corrupted syntax tree at N_array(N_num expected)!");
                    idx = (idx + expr->node[0]->info.cint) * shp[i];
                }
            } else
                idx *= shp[i];
            expr = expr->node[1];
        }
        if (expr != NULL) {
            DBUG_ASSERT ((expr->nodetype == N_exprs),
                         "corrupted syntax tree at N_array(N_exprs expected)!");
            if (expr->node[0]->nodetype == N_id)
                DBUG_ASSERT ((NULL),
                             "Sorry, no vars in arrays implemented in index-opt yet!");
            else {
                DBUG_ASSERT ((expr->node[0]->nodetype == N_num),
                             "corrupted syntax tree at N_array(N_num expected)!");
                idx += expr->node[0]->info.cint;
            }
        }
        arg_node = MakeNode (N_num);
        arg_node->info.cint = idx;
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
 *  global vars   :
 *  internal funs :
 *  external funs : MakeNode
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxNum (node *arg_node, node *arg_info)
{
    int *shp;

    DBUG_ENTER ("IdxNum");
    if (arg_info != NULL) {
        DBUG_ASSERT ((NULL), "Sorry, IdxNum not yet implemented!");
        shp = ((types *)arg_info)->shpseg->shp;
        /* for(i=0; i<shape->dim; i++) {
        } */
        arg_node = MakeNode (N_num);
        arg_node->info.cint = 1;
    }
    DBUG_RETURN (arg_node);
}
