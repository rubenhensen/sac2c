
/*
 * $Log$
 * Revision 1.3  2000/01/26 17:28:10  dkr
 * type of traverse-function-table changed.
 *
 * Revision 1.1  1999/10/20 12:51:11  sbs
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include "dbug.h"

#include "tree.h"
#include "traverse.h"
#include "globals.h"

#include "user_types.h"
#include "new_types.h"
#include "new_typecheck.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

/*
 */
/*
 * Thus, we finally find the following usages of the arg_info node:
 *
 *    INFO_NTC_????  -
 */

/******************************************************************************
 *
 * function:
 *    node *NewTypeCheck(node *arg_node)
 *
 * description:
 *    starts the new type checking traversal!
 *
 ******************************************************************************/

node *
NewTypeCheck (node *arg_node)
{
    funtab *tmp_tab;

    DBUG_ENTER ("NewTypeCheck");

    tmp_tab = act_tab;
    act_tab = ntc_tab;

    Trav (arg_node, NULL);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 ***
 ***          Here, the traversal functions start!
 ***          ------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    node *NTCmodul(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("NTCmodul");
    /*
     * First, we gather all typedefs and setup the global table
     * which is kept in "new_types".
     */
    if (NULL != MODUL_TYPES (arg_node))
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    DBUG_EXECUTE ("UDT", UTPrintRepository (stderr););
    ABORT_ON_ERROR;

    /*
     * Now, we do the actual typechecking....
     */
#if 0
    if ( NULL != MODUL_FUNS(arg_node))
      MODUL_FUNS(arg_node)=Trav(MODUL_FUNS(arg_node), arg_info);
#endif
    DBUG_RETURN (arg_node);
}

/*******************************************************************************
 *
 * function:
 *   ntype *CheckUdtAndSetBaseType( usertype udt, int* visited)
 *
 * description:
 *  This function checks the integrity of a user defined type, and while doing
 *  so it converts Symb{} types into Udt{} types, it computes its base-type,
 *  AND stores it in the udt-repository!
 *  At the time being, the following restrictions apply:
 *  - the defining type has to be one of Symb{} Simple{}, AKS{ Symb{}},
 *    or AKS{ Simple{}}.
 *  - if the defining type contains a Symb{} type, this type and all further
 *    decendents must be defined without any recursion in type definitions!
 *
 *  The second parameter ("visited") is needed for detecting recusive definitions
 *  only. Therefore, the initial call should be made with (visited == NULL)!
 *
 *  We ASSUME, that the existence of a basetype indicates that the udt has
 *  been checked already!!!
 *  Furthermore, we ASSUME that iff basetype is not yet set, the defining
 *  type either is a simple- or a symbol-type, NOT a user-type!
 *
 ******************************************************************************/

ntype *
CheckUdtAndSetBaseType (usertype udt, int *visited)
{
    ntype *base, *base_elem;
    usertype inner_udt;
    ntype *inner_base;
    ntype *new_base, *new_base_elem;
    int num_udt, i;

    DBUG_ENTER ("CheckUdt");

    base = UTGetBaseType (udt);
    if (base == NULL) {
        base = UTGetTypedef (udt);
        if (!(TYIsScalar (base) || TYIsAKS (base))) {
            ERROR (linenum,
                   ("typedef of %s:%s is illegal; should be either scalar type or"
                    "array type of fixed shape",
                    UTGetMod (udt), UTGetName (udt)));
        } else {
            /*
             * Here, we know that we are either dealing with
             * Symb{} Simple{}, AKS{ Symb{}}, or AKS{ Simple{}}.
             * If we would be dealing with    User{} or AKS{ User{}}
             * base type would have been set already!
             */
            if (TYIsSymb (base) || TYIsAKSSymb (base)) {
                base_elem = (TYIsSymb (base) ? base : TYGetScalar (base));
                inner_udt = UTFindUserType (TYGetName (base_elem), TYGetMod (base_elem));
                if (inner_udt == UT_NOT_DEFINED) {
                    ERROR (linenum, ("typedef of %s:%s is illegal; type %s:%s unknown",
                                     UTGetMod (udt), UTGetName (udt),
                                     TYGetMod (base_elem), TYGetName (base_elem)));
                } else {
                    /*
                     * First, we replace the defining symbol type by the appropriate
                     * user-defined-type, i.e., inner_udt!
                     */
                    new_base_elem = TYMakeUserType (inner_udt);
                    new_base
                      = (TYIsSymb (base)
                           ? new_base_elem
                           : TYMakeAKS (new_base_elem, SHCopyShape (TYGetShape (base))));
                    UTSetTypedef (udt, new_base);
                    TYFreeType (base);
                    base = new_base;

                    /*
                     * If this is the initial call, we have to allocate and
                     * initialize our recursion detection mask "visited".
                     */
                    if (visited == NULL) {
                        /* This is the initial call, so visited has to be initialized! */
                        num_udt = UTGetNumberOfUserTypes ();
                        visited = (int *)MALLOC (sizeof (int) * num_udt);
                        for (i = 0; i < num_udt; i++)
                            visited[i] = 0;
                    }
                    /*
                     * if we have not yet checked the inner_udt, recursively call
                     * CheckUdtAndSetBaseType!
                     */
                    if (visited[inner_udt] == 1) {
                        ERROR (linenum, ("type %s:%s recursively defined", UTGetMod (udt),
                                         UTGetName (udt)));
                    } else {
                        visited[udt] = 1;
                        inner_base = CheckUdtAndSetBaseType (inner_udt, visited);
                        /*
                         * Finally, we compute the resulting base-type by nesting
                         * the inner base type with the actual typedef!
                         */
                        base = TYNestTypes (base, inner_base);
                    }
                }
            } else {
                /*
                 * Here, we deal with Simple{} or AKS{ Simple{}}. In both cases
                 * base is the base type. Therefore, there will be no further
                 * recursice call. This allows us to free "visited".
                 * To be precise, we would have to free "visited in all ERROR-cases
                 * as well, but we neglect that since in case of an error the
                 * program will terminate soon anyways!
                 */
                if (visited != NULL)
                    FREE (visited);
            }
        }
        UTSetBaseType (udt, base);
    }

    DBUG_RETURN (base);
}

/******************************************************************************
 *
 * function:
 *    node *NTCtypedef(node *arg_node, node *arg_info)
 *
 * description:
 *   On the traversal down, we insert all user defined types. While doing so
 *   we check on duplicate definitions and issue ERROR-messages if neccessary.
 *   On the way back up we check on consistency (for the exact restrictions
 *   see "CheckUdtAndSetBaseType") and replace the defining type by its basetype.
 *
 ******************************************************************************/

node *
NTCtypedef (node *arg_node, node *arg_info)
{
    char *name, *mod;
    ntype *nt, *base;
    usertype udt;

    DBUG_ENTER ("NTCtypedef");
    name = TYPEDEF_NAME (arg_node);
    mod = TYPEDEF_MOD (arg_node);
    nt = TYOldType2Type (TYPEDEF_TYPE (arg_node), TY_symb);

    udt = UTFindUserType (name, mod);
    if (udt != UT_NOT_DEFINED) {
        ERROR (linenum, ("type %s:%s multiply defined; previous definition in line %d",
                         mod, name, UTGetLine (udt)));
    }
    udt = UTAddUserType (name, mod, nt, NULL, linenum);

    if (TYPEDEF_NEXT (arg_node) != NULL)
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);

    base = CheckUdtAndSetBaseType (udt, NULL);
    FREE (TYPEDEF_TYPE (arg_node));
    TYPEDEF_TYPE (arg_node) = TYType2OldType (base);

    DBUG_RETURN (arg_node);
}

#if 0
/******************************************************************************
 *
 * function:
 *    node *NTCFundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *NTCFundef(node *arg_node, node *arg_info)
{
  DBUG_ENTER("NTCFundef");
    if ( NULL != FUNDEF_NEXT(arg_node))
      FUNDEF_NEXT(arg_node)=Trav(FUNDEF_NEXT(arg_node), arg_info);
  DBUG_RETURN(arg_node);
}


/******************************************************************************
 *
 * function:
 *    node *NTCArg(node *arg_node, node *arg_info)
 *
 * description: 
 *
 ******************************************************************************/

node *NTCArg(node *arg_node, node *arg_info)
{
  DBUG_ENTER("NTCArg");

  if( NULL != ARG_NEXT(arg_node))
    ARG_NEXT(arg_node)=Trav(ARG_NEXT(arg_node), arg_info);
  DBUG_RETURN(arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * NTCBlock( node *arg_node, node *arg_info )
 *
 * description:
 *
 ******************************************************************************/

node *NTCBlock(node *arg_node, node *arg_info)
{
  DBUG_ENTER("NTCBlock");

  /*
   * First pass through the vardecs: all int[.] vars are initialized by $!
   */
  if( BLOCK_VARDEC( arg_node) != NULL) {
    BLOCK_VARDEC( arg_node) = Trav( BLOCK_VARDEC( arg_node), arg_info);
  }

  if( BLOCK_INSTR( arg_node) != NULL) {
    BLOCK_INSTR( arg_node) = Trav( BLOCK_INSTR( arg_node), arg_info);
  }

  /* 
   * Second pass through the vardecs: we eliminate all superfluous int[.]
   * vardecs. This is indicated by arg_info == NULL !
   */
  if( BLOCK_VARDEC( arg_node) != NULL) {
    BLOCK_VARDEC( arg_node) = Trav( BLOCK_VARDEC( arg_node), NULL);
  }

  DBUG_RETURN(arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * NTCVardec( node *arg_node, node *arg_info )
 *
 * description:
 *   if( arg_info != NULL) 
 *       create an N_vinfo node for all vars with either type int[x]
 *       or type int[.] (needed for the AKD-case!).
 *   else
 *       eliminate all those vardecs, whose COLCHNs do not contain VECT.
 *
 ******************************************************************************/

node *NTCVardec(node *arg_node, node *arg_info)
{
  int dim;
  node * next;

  DBUG_ENTER("NTCVardec");

  if( arg_info != NULL) {
    /*
     * This is the first traversal which initializes the int[.]
     * vardecs with $!
     */
    dim = VARDEC_DIM( arg_node);
    if( (VARDEC_BASETYPE( arg_node) == T_int) 
        && ((dim == 1 ) || (dim == KNOWN_DIM_OFFSET - 1 ))) {
      /* we are dealing with a potential indexing vector ! */
      VARDEC_ACTCHN( arg_node) = MakeVinfoDollar( NULL);
      VARDEC_COLCHN( arg_node) = MakeVinfoDollar( NULL);
    }

    if (VARDEC_NEXT( arg_node) != NULL) {
      VARDEC_NEXT( arg_node) = Trav( VARDEC_NEXT( arg_node), arg_info);
    }
  } else {
   /*
    * This is the second traversal which is done after traversing the body
    * and is used to eliminate index vectors which are no longer needed.
    * As criterium for need the existance of VECT in the COLCHN is taken.
    * I hope that this is sufficient, since I add VECT whenever Vect2Offset
    * is introduced ( see "CreateVect2OffsetIcm").
    */
    dim = VARDEC_DIM( arg_node);
    if( (VARDEC_BASETYPE( arg_node) == T_int)
        && ((dim == 1 ) || (dim == KNOWN_DIM_OFFSET - 1 ))
        && (VINFO_FLAG( FindVect( VARDEC_COLCHN( arg_node))) == DOLLAR )) {
      /* 
       * we are dealing with an indexing vector that is not
       * needed as a vector anymore!
       */
      next = VARDEC_NEXT( arg_node);
      VARDEC_NEXT( arg_node) = NULL;
      FreeTree( arg_node);
      if( next != NULL)
        arg_node = Trav( next, arg_info);
      else
        arg_node = NULL;
    }
    else {
      if (VARDEC_NEXT( arg_node) != NULL) {
        VARDEC_NEXT( arg_node) = Trav( VARDEC_NEXT( arg_node), arg_info);
      }
    }
    
  }

  DBUG_RETURN(arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * NTCAssign( node *arg_node, node *arg_info )
 *
 * description:
 *
 ******************************************************************************/

node *NTCAssign(node *arg_node, node *arg_info)
{
  DBUG_ENTER("NTCAssign");

  /* Bottom up traversal!! */
  if(NULL != ASSIGN_NEXT(arg_node)) {
    ASSIGN_NEXT(arg_node)=Trav(ASSIGN_NEXT(arg_node), arg_info);
  }
  /* store the current N_assign in INFO_IVE_CURRENTASSIGN */
  INFO_IVE_CURRENTASSIGN( arg_info) = arg_node;
  ASSIGN_INSTR(arg_node)=Trav(ASSIGN_INSTR(arg_node), arg_info);
  DBUG_RETURN(arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *NTCReturn(node *arg_node, node *arg_info)
 *
 * description:
 *   initiates the uses-collection. In order to guarantee a "VECT" attribution
 *   for array-variables, INFO_IVE_TRANSFORM_VINFO( arg_info) has to be NULL,
 *   when traversing the return expressions!
 *
 ******************************************************************************/

node *NTCReturn(node *arg_node, node *arg_info)
{
  DBUG_ENTER("NTCReturn");
  INFO_IVE_TRANSFORM_VINFO( arg_info) = NULL;
  RETURN_EXPRS( arg_node) = Trav( RETURN_EXPRS( arg_node), arg_info);
  DBUG_RETURN(arg_node);
}



/******************************************************************************
 *
 * function:
 *    node *NTCLet(node *arg_node, node *arg_info)
 *
 * description: This is the central mechanism for steering the code
 *    transformation. It Duplicates assignments - if required - and adds
 *    the transformation information into arg_info.
 *
 ******************************************************************************/

node *NTCLet(node *arg_node, node *arg_info)
{
  ids *vars;
  node *vardec, *vinfo, *act_let, *newassign;
  node *current_assign, *next_assign, *chain, *rest_chain;

  DBUG_ENTER("NTCLet");
  /* 
   * First, we attach the collected uses-attributes( they are in the
   * actual chain of the vardec) to the variables of the LHS of the
   * assignment!
   */
  vars=LET_IDS(arg_node);
  do {
    vardec = IDS_VARDEC(vars);
    if( NODE_TYPE( vardec) == N_vardec) {
      if( VARDEC_COLCHN( vardec) != NULL) { 
        /* So we are dealing with a potential index var! */
        chain                 = VARDEC_ACTCHN(vardec);
        IDS_USE(vars)         = chain;
        rest_chain            = CutVinfoChn( chain);
        /* Now, re-initialize the actual chain! */
        VARDEC_ACTCHN(vardec) = MakeVinfoDollar( rest_chain);
      }
    }
    else {
      DBUG_ASSERT((  NODE_TYPE( vardec) == N_arg),
                  "backref from let-var neither vardec nor arg!");
      if( ARG_COLCHN( vardec) != NULL) {
        /* So we are dealing with a potential index var! */
        chain              = ARG_ACTCHN(vardec);
        IDS_USE(vars)      = chain;
         rest_chain        = CutVinfoChn( chain);
        /* Now, re-initialize the actual chain! */
        ARG_ACTCHN(vardec) = MakeVinfoDollar( rest_chain);
      }
    }
    vars = IDS_NEXT(vars);
  } while(vars);


  if( INFO_IVE_MODE( arg_info) == M_uses_and_transform) { /* normal run! */

    /* Now, that we have done all that is needed for the "Uses" inference,
     * we do some modifications of the assignment based on the "Uses" information
     * gained.
     * Therefore, we first have to find out, what kind of RHS we are dealing with.
     *  - If it is (an arithmetic operation( +,-,*,\), a variable or a constant),
     *    AND it is neither F_mul_AxA nor F_div_AxA !!!
     *    AND the LHS variable is NOT used as VECT
     *    AND the LHS variable IS used as IDX(...):
     *    we duplicate the assignment for each IDX(shape) and traverse the new
     *    assignments with INFO_IVE_TRANSFORM_VINFO( arginfo) being set to the
     *    N_vinfo that carries IDX(shape)! This traversal will replace
     *    the index-array operations by integer-index operations.The original
     *    (VECT-) version is eliminated (more precisely: reused for the last
     *    IDX(...) version).
     *  - in all other cases:
     *    for each variable of the LHS that is needed as IDX(shape) we
     *    generate an assignment of the form:
     *    ND_KS_VECT2OFFSET( off-name, var-name, dim, dims, shape) 
     *    this instruction will calculate the indices needed from the vector
     *    generated by the RHS.
     *    After doing so, we traverse the assignment with 
     *    INFO_IVE_TRANSFORM_VINFO( arginfo) being set to NULL.
     */
    vars  = LET_IDS(arg_node);  /* pick the first LHS variable */
    vinfo = IDS_USE(vars); /* pick the "Uses"set from the first LHS var */
    if((vinfo != NULL) && (VINFO_FLAG( FindVect(vinfo)) == DOLLAR) && 
       (VINFO_FLAG( vinfo) == IDX) &&
       (
       ((NODE_TYPE( LET_EXPR(arg_node)) == N_prf) 
       && (F_add_SxA <= PRF_PRF(LET_EXPR(arg_node)))
       && (PRF_PRF(LET_EXPR(arg_node)) <= F_div_AxA)
       && (PRF_PRF(LET_EXPR(arg_node)) != F_mul_AxA)
       && (PRF_PRF(LET_EXPR(arg_node)) != F_div_AxA))

       || (NODE_TYPE( LET_EXPR(arg_node)) == N_id)
       || (NODE_TYPE( LET_EXPR(arg_node)) == N_array)
       )) {
      DBUG_ASSERT(((NODE_TYPE( LET_EXPR(arg_node)) == N_id) ||
                   (NODE_TYPE( LET_EXPR(arg_node)) == N_array) ||
                   (PRF_PRF(LET_EXPR(arg_node)) == F_add_SxA) ||
                   (PRF_PRF(LET_EXPR(arg_node)) == F_add_AxS) ||
                   (PRF_PRF(LET_EXPR(arg_node)) == F_add_AxA) ||
                   (PRF_PRF(LET_EXPR(arg_node)) == F_sub_SxA) ||
                   (PRF_PRF(LET_EXPR(arg_node)) == F_sub_AxS) ||
                   (PRF_PRF(LET_EXPR(arg_node)) == F_sub_AxA) ||
                   (PRF_PRF(LET_EXPR(arg_node)) == F_mul_SxA) ||
                   (PRF_PRF(LET_EXPR(arg_node)) == F_mul_AxS) ||
                   (PRF_PRF(LET_EXPR(arg_node)) == F_div_SxA) ||
                   (PRF_PRF(LET_EXPR(arg_node)) == F_div_AxS)),
                   " wrong prf sequence in \"prf_node_info.mac\"!");
      /* Here, we duplicate the operation
       * as many times as we do have different shapes in LET_USE(arg_node)
       * and successively traverse these assignments supplying the resp. N_vinfo
       * in each call's  INFO_IVE_TRANSFORM_VINFO( arg_info)!
       */
      act_let=arg_node;
      while(VINFO_FLAG(vinfo) != DOLLAR) {
        DBUG_ASSERT(((NODE_TYPE(act_let)==N_let)
                    && (NODE_TYPE(INFO_IVE_CURRENTASSIGN( arg_info)) == N_assign)),
                    "wrong let/assign node generated in IdxLet!");
        if(VINFO_FLAG( VINFO_NEXT( vinfo)) != DOLLAR){
          /* There are more indices needed, so we have to duplicate the let
           * node and repeat the let-traversal until there are no shapes left!
           * More precisely, we have to copy the Assign node, who is the father
           * of the let node and whose adress is given by arg_info!
           */
          current_assign = INFO_IVE_CURRENTASSIGN( arg_info);
          next_assign = ASSIGN_NEXT( current_assign);
          ASSIGN_NEXT( current_assign) = NULL;
          newassign=DupTree( current_assign, NULL);

          ASSIGN_NEXT(current_assign)  = newassign;
          ASSIGN_NEXT(newassign) = next_assign;
        }
        /* Now, we transform the RHS of act_let ! */
        INFO_IVE_TRANSFORM_VINFO( arg_info) = vinfo;
        LET_EXPR( act_let) = Trav( LET_EXPR( act_let), arg_info);
        /* Make sure we do have a vardec! */
        LET_NAME( act_let)  = IdxChangeId( LET_NAME(act_let), VINFO_TYPE( vinfo));
        LET_VARDEC( act_let)= VardecIdx( LET_VARDEC(act_let),
                                         VINFO_TYPE( vinfo));
        vinfo = VINFO_NEXT( vinfo);
        DBUG_ASSERT( (vinfo != NULL), " non $-terminated N_vinfo chain encountered!");
        if( VINFO_FLAG( vinfo) != DOLLAR) {
          INFO_IVE_CURRENTASSIGN( arg_info) = newassign;
          act_let  = ASSIGN_INSTR( newassign);
        }
      } 
    }
    else {
      /* We do not have a "pure" address calculation here!
       * Therefore, we insert for each shape-index needed for each variable of
       * the LHS an assignment of the form :
       * ND_KS_VECT2OFFSET( <off-name>, <var-name>, <dim>, <dims>, <shape> )
       */
      do { /* loop over all LHS-Vars */
        vinfo= IDS_USE( vars);
        vardec = IDS_VARDEC(vars);

        while(vinfo != NULL) { /* loop over all "Uses" attributes */
          if (VINFO_FLAG( vinfo)==IDX) {
            newassign = CreateVect2OffsetIcm( vardec, VINFO_TYPE( vinfo));

            current_assign                 = INFO_IVE_CURRENTASSIGN( arg_info);
            ASSIGN_NEXT( newassign)        = ASSIGN_NEXT( current_assign);
            ASSIGN_NEXT( current_assign)   = newassign;

            INFO_IVE_CURRENTASSIGN( arg_info) = newassign;
          }
          vinfo = VINFO_NEXT( vinfo);
        } 

        /* 
         * Now, we take care of the "VECT" version!
         * Make sure, that no transformations are done while traversing
         * the RHS!
         */
        INFO_IVE_TRANSFORM_VINFO( arg_info) = NULL;

        LET_EXPR( arg_node) = Trav(LET_EXPR( arg_node), arg_info);

        vars=IDS_NEXT( vars);
      } while( vars != NULL);
    }

  } else { /* uses inference only! */
    DBUG_ASSERT( INFO_IVE_MODE( arg_info) == M_uses_only,
                 "MODE-flag is neither M_uses_only nod M_uses_and_transform!");
    /* 
     * make sure that no transformations are done while traversing
     * the RHS!
     */
    INFO_IVE_TRANSFORM_VINFO( arg_info) = NULL;

    LET_EXPR( arg_node) = Trav(LET_EXPR( arg_node), arg_info);
  }


  DBUG_RETURN(arg_node);
}

/*
 *
 *  functionname  : NTCPrf
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

node *NTCPrf( node *arg_node, node *arg_info)
{
  node *arg1, *arg2, *arg3, *vinfo;
  types *type;

  DBUG_ENTER("IdxPrf");
  /*
   * INFO_IVE_TRANSFORM_VINFO( arg_info) indicates whether this is just a
   * normal traverse (NULL), or the transformation of an arithmetic
   * index calculation (N_vinfo-node).
   */
  switch( PRF_PRF( arg_node)) {
    case F_psi: 
      arg1 = PRF_ARG1( arg_node);
      arg2 = PRF_ARG2( arg_node);
      DBUG_ASSERT(((arg2->nodetype == N_id) || (arg2->nodetype == N_array)),
                    "wrong arg in F_psi application");
      if( NODE_TYPE( arg2) == N_id)
        type = ID_TYPE( arg2);
      else
        type = ARRAY_TYPE( arg2);
      /*
       * if the shape of the array is unknown, do not(!) replace
       * psi by idx_psi but mark the selecting vector as VECT !
       * this is done by traversal with NULL instead of vinfo!
       */
      if (TYPES_SHPSEG( type) != NULL) { 
        vinfo = MakeVinfo( IDX, type, NULL, NULL);
        INFO_IVE_TRANSFORM_VINFO( arg_info) = vinfo;
        PRF_ARG1( arg_node) = Trav(arg1, arg_info);
        FreeNode( vinfo);
        if( INFO_IVE_MODE( arg_info) == M_uses_and_transform) {
          PRF_PRF( arg_node)  = F_idx_psi;
        }
      } else {
        INFO_IVE_TRANSFORM_VINFO( arg_info) = NULL;
        PRF_ARG1( arg_node) = Trav(arg1, arg_info);
      }
      INFO_IVE_TRANSFORM_VINFO( arg_info) = NULL;
      PRF_ARG2( arg_node) = Trav(arg2, arg_info);
      break;
    case F_modarray: 
      arg1 = PRF_ARG1( arg_node);
      arg2 = PRF_ARG2( arg_node);
      arg3 = PRF_ARG3( arg_node);
      DBUG_ASSERT(((arg1->nodetype == N_id) || (arg1->nodetype == N_array)),
                    "wrong arg in F_modarray application");
      if( NODE_TYPE( arg1) == N_id)
        type = ID_TYPE( arg1);
      else
        type = ARRAY_TYPE( arg1);
      /*
       * if the shape of the array is unknown, do not(!) replace
       * modarray by idx_modarray but mark the selecting vector as VECT !
       * this is done by traversal with NULL instead of vinfo!
       */
      if (TYPES_SHPSEG( type) != NULL) { 
        vinfo = MakeVinfo( IDX, type, NULL, NULL);
        INFO_IVE_TRANSFORM_VINFO( arg_info) = vinfo;
        PRF_ARG2( arg_node) = Trav(arg2, arg_info);
        FreeNode( vinfo);
        if(INFO_IVE_MODE( arg_info) == M_uses_and_transform) {
          PRF_PRF( arg_node)  = F_idx_modarray;
        }
      } else {
        INFO_IVE_TRANSFORM_VINFO( arg_info) = NULL;
        PRF_ARG2( arg_node) = Trav(arg2, arg_info);
      }
      INFO_IVE_TRANSFORM_VINFO( arg_info) = NULL;
      PRF_ARG1( arg_node) = Trav(arg1, arg_info);
      PRF_ARG3( arg_node) = Trav(arg3, arg_info);
      break;
    case F_add_SxA:
      INFO_IVE_NON_SCAL_LEN( arg_info) = ID_SHAPE(PRF_ARG2( arg_node), 0);
      if(INFO_IVE_TRANSFORM_VINFO( arg_info) != NULL)
        PRF_PRF( arg_node) = F_add;
      PRF_ARGS( arg_node) = Trav(PRF_ARGS( arg_node), arg_info);
      ive_op++;
      break;
    case F_add_AxS:
      INFO_IVE_NON_SCAL_LEN( arg_info) = ID_SHAPE(PRF_ARG1( arg_node), 0);
    case F_add_AxA:
      if(INFO_IVE_TRANSFORM_VINFO( arg_info) != NULL)
        PRF_PRF( arg_node) = F_add;
      PRF_ARGS( arg_node) = Trav(PRF_ARGS( arg_node), arg_info);
      ive_op++;
      break;
    case F_sub_SxA:
      INFO_IVE_NON_SCAL_LEN( arg_info) = ID_SHAPE(PRF_ARG2( arg_node), 0);
      if(INFO_IVE_TRANSFORM_VINFO( arg_info) != NULL)
        PRF_PRF( arg_node) = F_sub;
      PRF_ARGS( arg_node) = Trav(PRF_ARGS( arg_node), arg_info);
      ive_op++;
      break;
    case F_sub_AxS:
      INFO_IVE_NON_SCAL_LEN( arg_info) = ID_SHAPE(PRF_ARG1( arg_node), 0);
    case F_sub_AxA:
      if(INFO_IVE_TRANSFORM_VINFO( arg_info) != NULL)
        PRF_PRF( arg_node) = F_sub;
      PRF_ARGS( arg_node) = Trav(PRF_ARGS( arg_node), arg_info);
      ive_op++;
      break;
    case F_mul_SxA:
      if( INFO_IVE_TRANSFORM_VINFO( arg_info) != NULL)
        PRF_PRF( arg_node) =F_mul;
      PRF_ARG2( arg_node) = Trav( PRF_ARG2( arg_node), arg_info);
      ive_op++;
      break;
    case F_mul_AxS:
      if( INFO_IVE_TRANSFORM_VINFO( arg_info) != NULL)
        PRF_PRF( arg_node) =F_mul;
      PRF_ARG1( arg_node) = Trav( PRF_ARG1( arg_node), arg_info);
      ive_op++;
      break;
    case F_div_SxA:
      if( INFO_IVE_TRANSFORM_VINFO( arg_info) != NULL)
        PRF_PRF( arg_node) =F_div;
      PRF_ARG2( arg_node) = Trav( PRF_ARG2( arg_node), arg_info);
      ive_op++;
      break;
    case F_div_AxS:
      if( INFO_IVE_TRANSFORM_VINFO( arg_info) != NULL)
        PRF_PRF( arg_node) =F_div;
      PRF_ARG1( arg_node) = Trav( PRF_ARG1( arg_node), arg_info);
      ive_op++;
      break;
    default:
      DBUG_ASSERT( (INFO_IVE_TRANSFORM_VINFO( arg_info) == NULL),
                   "Inconsistency between IdxLet and IdxPrf");
      PRF_ARGS( arg_node) = Trav(PRF_ARGS( arg_node), arg_info);
      break;
  }
  DBUG_RETURN(arg_node);
}

/*
 *
 *  functionname  : NTCId
 *  arguments     :
 *  description   : examines whether variable is a one-dimensional array;
 *                  if so, examine INFO_IVE_TRANSFORM_VINFO( arg_info):
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

node *NTCId( node *arg_node, node *arg_info)
{
  types *type;
  node *vardec;
  char *newid;

  DBUG_ENTER("NTCId");
  vardec = ID_VARDEC( arg_node);
  DBUG_ASSERT(((NODE_TYPE( vardec) == N_vardec) || (NODE_TYPE( vardec) == N_arg)),
              "non vardec/arg node as backref in N_id!");
  if( NODE_TYPE( vardec) == N_vardec) {
    if ((VARDEC_BASETYPE(vardec) == T_int) 
        && ((VARDEC_DIM( vardec) == 1) || (VARDEC_DIM( vardec) == KNOWN_DIM_OFFSET - 1))) {
      if ( INFO_IVE_TRANSFORM_VINFO( arg_info) == NULL ) {
        DBUG_PRINT("IDX",("assigning VECT to %s:", ID_NAME( arg_node)));
        VARDEC_ACTCHN( vardec) = SetVect( VARDEC_ACTCHN( vardec));
        VARDEC_COLCHN( vardec) = SetVect( VARDEC_COLCHN( vardec));
      }
      else {
        type = VINFO_TYPE( INFO_IVE_TRANSFORM_VINFO( arg_info));
        DBUG_PRINT("IDX",("assigning IDX to %s:", ID_NAME( arg_node)));
        VARDEC_ACTCHN( vardec) = SetIdx( VARDEC_ACTCHN( vardec), type);
        VARDEC_COLCHN( vardec) = SetIdx( VARDEC_COLCHN( vardec), type);
        if( INFO_IVE_MODE( arg_info) == M_uses_and_transform) {
          newid = IdxChangeId( ID_NAME(arg_node), type);
          FREE( ID_NAME(arg_node));
          ID_NAME(arg_node) = newid;
          /* Now, we have to insert the respective declaration */
          /* If the declaration does not yet exist, it has to be created! */
          ID_VARDEC( arg_node) = VardecIdx( vardec, type);
        }
      }
    }
  }
  else {
    if (( ARG_BASETYPE( vardec) == T_int)
        && ( (ARG_DIM( vardec) == 1) || (ARG_DIM( vardec) == KNOWN_DIM_OFFSET - 1))) {
      if ( INFO_IVE_TRANSFORM_VINFO( arg_info) == NULL ) {
        DBUG_PRINT("IDX",("assigning VECT to %s:", ID_NAME( arg_node)));
        ARG_ACTCHN( vardec) = SetVect( ARG_ACTCHN( vardec));
        ARG_COLCHN( vardec) = SetVect( ARG_COLCHN( vardec));
      }
      else {
        type = VINFO_TYPE( INFO_IVE_TRANSFORM_VINFO(arg_info));
        DBUG_PRINT("IDX",("assigning IDX to %s:", ID_NAME( arg_node)));
        ARG_ACTCHN( vardec) = SetIdx( ARG_ACTCHN( vardec), type);
        ARG_COLCHN( vardec) = SetIdx( ARG_COLCHN( vardec), type);
        if( INFO_IVE_MODE( arg_info) == M_uses_and_transform) {
          newid = IdxChangeId( ID_NAME(arg_node), type);
          FREE( ID_NAME(arg_node));
          ID_NAME(arg_node) = newid;
          /* Now, we have to insert the respective declaration */
          /* If the declaration does not yet exist, it has to be created! */
          ID_VARDEC( arg_node) = VardecIdx( vardec, type);
        }
      }
    }
  }
  DBUG_RETURN(arg_node);
}

/*
 *
 *  functionname  : IdxArray
 *  arguments     : 1) node*: N_array node
 *                  2) node*: INFO_IVE_TRANSFORM_VINFO( arg_info) = vinfo
 *                  R) node*: index of N_array in unrolling of shape 
 *  description   : if vinfo == NULL all Array-Elements are traversed with
 *                  INFO_IVE_TRANSFORM_VINFO( arg_info) NULL, since there may be
 *                  some array_variables;
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

node *IdxArray( node * arg_node, node *arg_info)
{
  int i;
  int *shp;
  node *idx;
  node *expr;

  DBUG_ENTER("IdxArray");
  if( INFO_IVE_TRANSFORM_VINFO( arg_info)==NULL) {
    if (ARRAY_AELEMS(arg_node) != NULL) {
       ARRAY_AELEMS( arg_node) = Trav( ARRAY_AELEMS( arg_node), arg_info);
    }
  }
  else {
    if( INFO_IVE_MODE( arg_info) == M_uses_and_transform) {
      expr= ARRAY_AELEMS( arg_node);
      shp= VINFO_SELEMS( INFO_IVE_TRANSFORM_VINFO( arg_info));
      idx = EXPRS_EXPR( expr);
      expr = EXPRS_NEXT( expr);
      for(i=1; i<VINFO_DIM( INFO_IVE_TRANSFORM_VINFO( arg_info)); i++) {
        if(expr != NULL) {
          DBUG_ASSERT((NODE_TYPE(expr) == N_exprs),
                      "corrupted syntax tree at N_array(N_exprs expected)!");
          idx = MakeExprs( idx, MakeExprs( MakeNum( shp[i]), NULL));
          idx = MakePrf( F_mul, idx);
          idx = MakeExprs( idx, expr);
          expr= EXPRS_NEXT( expr);
          EXPRS_NEXT( EXPRS_NEXT( idx)) = NULL;
          idx = MakePrf( F_add, idx);
        }
        else {
          idx = MakeExprs( idx, MakeExprs( MakeNum( shp[i]), NULL));
          idx = MakePrf( F_mul, idx);
        }
      } 
      arg_node = idx;
      ive_expr++;
    }
  }
  DBUG_RETURN(arg_node);
}

/*
 *
 *  functionname  : IdxNum
 *  arguments     : 1) node*: N_num node
 *                  2) node*: INFO_IVE_TRANSFORM_VINFO( arg_info) = shape
 *                  R) node*: index of N_array in unrolling of shape
 *  description   : if shape == NULL nothing has to be done; otherwise
 *                  the index of the vector N_array in
 *                  the unrolling of an array of shape is calculated;
 *
 */

node *IdxNum( node * arg_node, node *arg_info)
{
  int val,i,len_iv, dim_array, sum;
  int *shp;

  DBUG_ENTER("IdxNum");
  if( INFO_IVE_TRANSFORM_VINFO( arg_info)!=NULL) {
    DBUG_ASSERT( (NODE_TYPE( INFO_IVE_TRANSFORM_VINFO(arg_info)) == N_vinfo),
                 "corrupted arg_info node in IdxNum!");
    shp = VINFO_SELEMS( INFO_IVE_TRANSFORM_VINFO( arg_info));
    val = NUM_VAL(arg_node);
    dim_array = VINFO_DIM( INFO_IVE_TRANSFORM_VINFO( arg_info));
    len_iv =  INFO_IVE_NON_SCAL_LEN( arg_info);

    sum = val;
    for(i=1; i< len_iv; i++) {
      sum = (sum * shp[i]) + val;
    } 
    for(; i< dim_array; i++) {
      sum = sum * shp[i];
    } 
    NUM_VAL(arg_node) = sum;
  }
  DBUG_RETURN(arg_node);
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

node *IdxWith(node *arg_node, node *arg_info)
{
  DBUG_ENTER("IdxWith");
    /* Bottom up traversal; the OPERATOR is a N_modarray,
     * N_genarray, N_foldprf, or N_foldfun node.
     * all these nodes do have ordenary N_block-nodes attached!
     */
    WITH_OPERATOR( arg_node) = Trav(WITH_OPERATOR( arg_node), NULL);

    /* When traversing the generator, we potentially want to insert some
     * index conversions into the body; therfore, we supply the
     * N_let node as surplus argument; it does not suffice to
     * submit the body (N_genarray, N_modarray or N_fold node)
     * since the name of the variable that will be generated/modified
     * is needed for the creation of the ND_KS_USE_GENVAR_OFFSET ICM !
     */
    WITH_GEN( arg_node)=Trav(WITH_GEN( arg_node),
                             ASSIGN_INSTR( INFO_IVE_CURRENTASSIGN(arg_info)));
  DBUG_RETURN(arg_node);
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

node *IdxGenerator(node *arg_node, node *arg_info)
{
  node *vardec, *vinfo, *block, *icm_arg, *newassign, *newid, *arrayid;
  node *name_node, *dim_node, *dim_node2, *body, *colvinfo;
  types *artype;
  int i;

  DBUG_ENTER( "IdxGenerator");
  DBUG_ASSERT( (NODE_TYPE( arg_info) == N_let),
               "arg_info is not a N_let-node");
  DBUG_ASSERT( (NODE_TYPE( LET_EXPR( arg_info)) == N_with),
               "N_let node in arg_info contains no with-loop");

  body = WITH_OPERATOR( LET_EXPR( arg_info));
  GEN_LEFT( arg_node) = Trav( GEN_LEFT( arg_node), NULL);
  GEN_RIGHT( arg_node) = Trav( GEN_RIGHT( arg_node), NULL);
  vardec = GEN_VARDEC( arg_node);
  vinfo  = VARDEC_ACTCHN( vardec);

  /* first, we memorize the actuall chain */
  GEN_USE( arg_node) = vinfo;
  /* then, we remove the actual chain! */
  VARDEC_ACTCHN( vardec) = MakeVinfoDollar( CutVinfoChn( vinfo));

  /* for each IDX-vinfo-node we have to instanciate the respective
   * variable as first statement in the body of the with loop.
   * for doing so, we need the root of the body which is supplied
   * via arg_info by IdxWith!
   */
  DBUG_PRINT("IDX",("introducing idx-vars in with-bodies"));
  while(vinfo != NULL) {
    DBUG_PRINT("IDX",("examining vinfo(%p)", vinfo));
    if(VINFO_FLAG( vinfo) == IDX) {
      artype = LET_TYPE( arg_info);
      DBUG_ASSERT((artype != NULL),"missing type-info for LHS of let!");
      switch(body->nodetype) {
        case N_modarray:
          block  = MODARRAY_BODY( body);
          break;
        case N_genarray:
          block = GENARRAY_BODY( body);
          break;
        case N_foldprf:
          block = FOLDPRF_BODY( body);
          break;
        case N_foldfun:
          block = FOLDFUN_BODY( body);
          break;
        default:
          DBUG_ASSERT((0 != 0), "unknown generator type in IdxGenerator");
          /* 
           * the following assignment is used only for convincing the C compiler
           * that block will be initialized in any case!
           */
          block = NULL;
      }

      if( ((NODE_TYPE( body) == N_modarray) || (NODE_TYPE( body) == N_genarray))
          && EqTypes( VINFO_TYPE( vinfo), artype)) {
        /* 
         * we can reuse the genvar as index directly!
         * therefore we create an ICM of the form:
         * ND_KS_USE_GENVAR_OFFSET( <idx-varname>, <result-array-varname>)
         */
        newid = MakeId( IdxChangeId( GEN_ID( arg_node), artype), NULL, ST_regular);
        colvinfo = FindIdx( VARDEC_COLCHN( vardec), artype);
        DBUG_ASSERT(((colvinfo != NULL) && (VINFO_VARDEC( colvinfo) != NULL)),
              "missing vardec for IDX variable!");
        ID_VARDEC( newid) = VINFO_VARDEC( colvinfo);
        arrayid = MakeId( StringCopy( LET_NAME( arg_info)), NULL, ST_regular);
        /* 
         * The backref of the arrayid is set wrongly to the actual
         * integer index-variable. This is done on purpose for
         * fooling the refcount-inference system.
         * The "correct" backref would be LET_VARDEC( arg_info) !
         */
        ID_VARDEC( arrayid) = VINFO_VARDEC( colvinfo);

        CREATE_2_ARY_ICM(newassign, "ND_KS_USE_GENVAR_OFFSET", newid, arrayid);
      }
      else {
        /*
         * we have to instanciate the idx-variable by an ICM of the form:
         * ND_KS_VECT2OFFSET( <off-name>, <var-name>,
         *                    <dim of var>, <dim of array>, shape_elems)
         */
        newid = MakeId( IdxChangeId( GEN_ID( arg_node), VINFO_TYPE( vinfo)),
                        NULL, ST_regular);
        colvinfo = FindIdx( VARDEC_COLCHN( vardec), VINFO_TYPE( vinfo));
        DBUG_ASSERT( ((colvinfo != NULL) && (VINFO_VARDEC( colvinfo) != NULL)),
                     "missing vardec for IDX variable");
        ID_VARDEC( newid) = VINFO_VARDEC( colvinfo);

        name_node = MakeId( StringCopy( GEN_ID( arg_node)), NULL, ST_regular);
        ID_VARDEC( name_node) = vardec;

        dim_node = MakeNum( VARDEC_SHAPE( vardec, 0));

        dim_node2 = MakeNum(VINFO_DIM( vinfo));

        CREATE_4_ARY_ICM(newassign, "ND_KS_VECT2OFFSET",
                                    newid,                  /* off-name */
                                    name_node,              /* var-name */
                                    dim_node,               /* dim of var */
                                    dim_node2);             /* dim of array */

        /* Now, we append the shape elems to the ND_KS_VECT2OFFSET-ICM ! */
        for( i=0; i<VINFO_DIM( vinfo); i++)
          MAKE_NEXT_ICM_ARG( icm_arg, MakeNum( VINFO_SELEMS( vinfo)[i]));

      }
      ASSIGN_NEXT( newassign) = BLOCK_INSTR( block);
      BLOCK_INSTR( block) = newassign;
    }
    vinfo=VINFO_NEXT( vinfo);
  }
  DBUG_RETURN(arg_node);
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

node *IdxNwith( node *arg_node, node *arg_info)
{
  DBUG_ENTER( "IdxNwith");

  NWITH_WITHOP( arg_node) = Trav( NWITH_WITHOP( arg_node), arg_info);
  NWITH_CODE( arg_node) = Trav( NWITH_CODE( arg_node), arg_info);

  /*
   * Finally, we traverse the Npart nodes in order to eliminate
   * superfluous index-vectors!
   */
  NWITH_PART( arg_node) = Trav( NWITH_PART( arg_node), arg_info);

  DBUG_RETURN( arg_node);
}


/******************************************************************************
 *
 * function:
 *   node *IdxNpart( node *arg_node, node *arg_info)
 *
 * description:
 *    Here we make sure that all parts of the generator will be traversed
 *    correctly. Furthermore, we eliminate superfluous generator vars
 *    (provided REFCOUNT_PROBLEM_SOLVED holds).
 *
 ******************************************************************************/

node *IdxNpart( node *arg_node, node *arg_info)
{
  node * vardec, *mem_transform;

  DBUG_ENTER( "IdxNpart");

#ifdef REFCOUNT_PROBLEM_SOLVED
  if( INFO_IVE_MODE( arg_info) == M_uses_and_transform) {
    if( VINFO_FLAG( FindVect( NCODE_USE( NPART_CODE( arg_node)))) == DOLLAR) {
      /*
       * The index vector variable is used as IDX(...) only!
       * => we can eliminate the vector completely!
       */
      FREE( NPART_VEC(arg_node));
    }
  }
#else
  if( INFO_IVE_MODE( arg_info) == M_uses_and_transform) {
    /*
     * This makes sure, that the declaration of the index vector variable
     * will survive the second traversal of the vardecs, even if there is 
     * no further reference to it but the one in the generator!
     * IF REFCOUNT_PROBLEM_SOLVED this can be spared since the one in the
     * generator would be deleted anyways ( see above)!!
     */
    vardec = IDS_VARDEC( NPART_VEC( arg_node));
    SET_VARDEC_OR_ARG_COLCHN( vardec, SetVect( VARDEC_COLCHN( vardec)));
  }
#endif

  /*
   * Now, we want to traverse the bounds and filters in order
   * to obtain all potential VECT uses.
   * For preventing any transformations of these, we have to make
   * sure, that (INFO_IVE_TRANSFORM_VINFO( arg_info) == NULL) during
   * that traversal!
   */
  mem_transform = INFO_IVE_TRANSFORM_VINFO( arg_info);
  INFO_IVE_TRANSFORM_VINFO( arg_info) = NULL; 
  if( NPART_GEN( arg_node) != NULL)
    NPART_GEN( arg_node) = Trav( NPART_GEN( arg_node), arg_info);
  INFO_IVE_TRANSFORM_VINFO( arg_info) = mem_transform;

  /* 
   * Finally, we take care of any subsequent generators!
   */
  if( NPART_NEXT( arg_node) != NULL)
    NPART_NEXT( arg_node) = Trav( NPART_NEXT( arg_node), arg_info);

  DBUG_RETURN( arg_node);
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

node *IdxNcode( node *arg_node, node *arg_info)
{
  node *with, *idx_vardec, *vinfo, *withop_arr, *col_vinfo,
       *new_assign, *let_node, *current_assign,
       *new_id, *array_id;
  types *arr_type;

  DBUG_ENTER( "IdxNcode");

  /*
   * we traverse the current code block
   */
  current_assign = INFO_IVE_CURRENTASSIGN( arg_info);
  let_node = ASSIGN_INSTR( current_assign);

  NCODE_CEXPR( arg_node) = Trav( NCODE_CEXPR( arg_node), arg_info);
  NCODE_CBLOCK( arg_node) = Trav( NCODE_CBLOCK( arg_node), arg_info);

  DBUG_ASSERT( ((NODE_TYPE( let_node) == N_let) &&
                (NODE_TYPE( LET_EXPR( let_node)) == N_Nwith)),
               "arg_info contains no let with a with-loop on the RHS!");

  /*
   * we insert instances of the index-vector-var as first statement of the
   *  current code block.
   */

  with = LET_EXPR( let_node);
  idx_vardec = IDS_VARDEC( NWITH_VEC( with));
  vinfo = VARDEC_ACTCHN( idx_vardec);
  NCODE_USE( arg_node) = vinfo;
  VARDEC_ACTCHN( idx_vardec) = MakeVinfoDollar( CutVinfoChn( vinfo));

  if( INFO_IVE_MODE( arg_info) == M_uses_and_transform) {
    while (VINFO_FLAG( vinfo) != DOLLAR) {

      if (VINFO_FLAG( vinfo) == IDX) {
        arr_type = LET_TYPE( let_node);
        DBUG_ASSERT((arr_type != NULL),"missing type-info for LHS of let!");

        switch (NWITH_TYPE( with)) {

          case WO_modarray:
            withop_arr = NWITHOP_ARRAY( NWITH_WITHOP( with));
            break;

          case WO_genarray:
            withop_arr = NWITHOP_SHAPE( NWITH_WITHOP( with));
            DBUG_ASSERT( (NODE_TYPE( withop_arr) == N_array),
                         "shape of genarray is not N_array");
            break;

          case WO_foldprf:
            /* here is no break missing! */
          case WO_foldfun:
            break;

          default:
            DBUG_ASSERT( (0), "wrong with-loop type");

        }

        if (((NWITH_TYPE( with) == WO_modarray) ||
             (NWITH_TYPE( with) == WO_genarray)) &&
            EqTypes( VINFO_TYPE( vinfo), arr_type)) {

          /*
           * we can reuse the genvar as index directly!
           * therefore we create an ICM of the form:
           * ND_KS_USE_GENVAR_OFFSET( <idx-varname>, <result-array-varname>)
           */

          new_id = MakeId( IdxChangeId( IDS_NAME( NWITH_VEC( with)), arr_type),
                           NULL, ST_regular);
          col_vinfo = FindIdx( VARDEC_COLCHN( idx_vardec), arr_type);
          DBUG_ASSERT( ((col_vinfo != NULL) && (VINFO_VARDEC( col_vinfo) != NULL)),
                       "missing vardec for IDX variable");
          ID_VARDEC( new_id) = VINFO_VARDEC( col_vinfo);
          array_id = MakeId( StringCopy( LET_NAME( let_node)), NULL, ST_regular);

          /* 
           * The backref of the arrayid is set wrongly to the actual
           * integer index-variable. This is done on purpose for
           * fooling the refcount-inference system.
           * The "correct" backref would be LET_VARDEC( let_node) !
           */
          ID_VARDEC( array_id) = VINFO_VARDEC( col_vinfo);

          new_assign = MakeAssign( MakeIcm2( "ND_KS_USE_GENVAR_OFFSET",
                                             new_id,
                                             array_id),
                                   NULL);
        }
        else {
          /*
           * we have to instanciate the idx-variable by an ICM of the form:
           * ND_KS_VECT2OFFSET( <off-name>, <var-name>,
           *                    <dim of var>, <dim of array>, shape_elems)
           */
          new_assign = CreateVect2OffsetIcm( idx_vardec, VINFO_TYPE( vinfo));
        }

        ASSIGN_NEXT( new_assign) = BLOCK_INSTR( NCODE_CBLOCK( arg_node));
        DBUG_ASSERT( (NODE_TYPE( BLOCK_INSTR( NCODE_CBLOCK( arg_node))) != N_empty),
                     "N_empty node in block found");
        BLOCK_INSTR( NCODE_CBLOCK( arg_node)) = new_assign;
      }

      vinfo = VINFO_NEXT( vinfo);
    }
  }

  /*
   * now we traverse the next code block
   */

  if (NCODE_NEXT( arg_node) != NULL) {
    /* restore valid CURRENTASSIGN!! */
    INFO_IVE_CURRENTASSIGN( arg_info) = current_assign;
    NCODE_NEXT( arg_node) = Trav( NCODE_NEXT( arg_node), arg_info);
  }

  DBUG_RETURN( arg_node);
}



/******************************************************************************
 *
 * function:
 *  node * IdxCond( node * arg_node, node * arg_info)
 *
 * description:
 *
 ******************************************************************************/

node * IdxCond( node * arg_node, node * arg_info)
{
  DBUG_ENTER( "IdxCond");

  /* Now, we duplicate the topmost chain of actchn! */
  MAP_TO_ALL_VARDEC_ACTCHNS( DuplicateTop, INFO_IVE_VARDECS( arg_info));

  if( COND_THEN( arg_node) != NULL)
    COND_THEN( arg_node) = Trav( COND_THEN( arg_node), arg_info);

  /* Now, we switch the two topmost chains of actchn! */
  MAP_TO_ALL_VARDEC_ACTCHNS( SwitchTop, INFO_IVE_VARDECS( arg_info));

  if( COND_ELSE( arg_node) != NULL)
    COND_ELSE( arg_node) = Trav( COND_ELSE( arg_node), arg_info);

  /* Now, we merge the topmost chain of actchn into the rest of actchn! */
  MAP_TO_ALL_VARDEC_ACTCHNS( MergeTop, INFO_IVE_VARDECS( arg_info));
   
  DBUG_RETURN( arg_node);
}


/******************************************************************************
 *
 * function:
 *  node * IdxWhile( node * arg_node, node * arg_info)
 *
 * description:
 *
 ******************************************************************************/

node * IdxWhile( node * arg_node, node * arg_info)
{
  int old_uses_mode;

  DBUG_ENTER( "IdxWhile");

  /* Now, we duplicate the topmost chain of actchn! */
  MAP_TO_ALL_VARDEC_ACTCHNS( DuplicateTop, INFO_IVE_VARDECS( arg_info));

  /*
   * We have to memorize the old uses_mode in order to
   * prevent code-transformations during the second traversal of the loop
   * in case the entire loop resides within an outer loop whose body is
   * traversed the first time!
   */
  old_uses_mode = INFO_IVE_MODE( arg_info);
  INFO_IVE_MODE( arg_info) = M_uses_only;

  if( WHILE_BODY( arg_node) != NULL)
    WHILE_BODY( arg_node) = Trav( WHILE_BODY( arg_node), arg_info);

  /* Now, we merge the topmost chain of actchn into the rest of actchn! */
  MAP_TO_ALL_VARDEC_ACTCHNS( MergeTop, INFO_IVE_VARDECS( arg_info));
  MAP_TO_ALL_VARDEC_ACTCHNS( DuplicateTop, INFO_IVE_VARDECS( arg_info));

  /*
   * Now, we restore the uses_mode to the value set before entering the loop 
   * (see comment above!)
   */
  INFO_IVE_MODE( arg_info) = old_uses_mode;
  if( WHILE_BODY( arg_node) != NULL)
    WHILE_BODY( arg_node) = Trav( WHILE_BODY( arg_node), arg_info);

  MAP_TO_ALL_VARDEC_ACTCHNS( FreeTop, INFO_IVE_VARDECS( arg_info));

  DBUG_RETURN( arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * IdxDo( node * arg_node, node * arg_info)
 *
 * description:
 *
 ******************************************************************************/

node * IdxDo( node * arg_node, node * arg_info)
{
  int old_uses_mode;

  DBUG_ENTER( "IdxDo");

  /* Now, we duplicate the topmost chain of actchn! */
  MAP_TO_ALL_VARDEC_ACTCHNS( DuplicateTop, INFO_IVE_VARDECS( arg_info));

  /*
   * We have to memorize the old uses_mode in order to
   * prevent code-transformations during the second traversal of the loop
   * in case the entire loop resides within an outer loop whose body is
   * traversed the first time!
   */
  old_uses_mode = INFO_IVE_MODE( arg_info);
  INFO_IVE_MODE( arg_info) = M_uses_only;

  if( DO_BODY( arg_node) != NULL)
    DO_BODY( arg_node) = Trav( DO_BODY( arg_node), arg_info);

  /* Now, we merge the topmost chain of actchn into the rest of actchn! */
  MAP_TO_ALL_VARDEC_ACTCHNS( MergeCopyTop, INFO_IVE_VARDECS( arg_info));

  /*
   * Now, we restore the uses_mode to the value set before entering the loop
   * (see comment above!)
   */
  INFO_IVE_MODE( arg_info) = old_uses_mode;
  if( DO_BODY( arg_node) != NULL)
    DO_BODY( arg_node) = Trav( DO_BODY( arg_node), arg_info);

  MAP_TO_ALL_VARDEC_ACTCHNS( FreeTop, INFO_IVE_VARDECS( arg_info));

  DBUG_RETURN( arg_node);
}

#endif
