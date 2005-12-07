/*
 *
 * $Id$
 *
 */

#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "free.h"
#include "new_types.h"
#include "namespaces.h"
#include "DupTree.h"

/*
 * INFO structure
 */
struct INFO {
    node *module;
};

/*
 * INFO macros
 */
#define INFO_MODULE(n) ((n)->module)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * helper functions
 */

static node *
BuildTypeConversion (const char *name, const namespace_t *ns, ntype *from, ntype *to)
{
    node *result;
    node *avisarg, *avisres;
    node *assign;
    node *block;

    DBUG_ENTER ("BuildTypeConversion");

    avisarg = TBmakeAvis (ILIBtmpVar (), TYcopyType (from));
    avisres = TBmakeAvis (ILIBtmpVar (), TYcopyType (to));

    /*
     * return( res);
     */
    assign = TBmakeAssign (TBmakeReturn (TBmakeExprs (TBmakeId (avisres), NULL)), NULL);

    /*
     * res = type_conv( restype, arg);
     */
    assign
      = TBmakeAssign (TBmakeLet (TBmakeIds (avisres, NULL),
                                 TCmakePrf2 (F_type_conv, TBmakeType (TYcopyType (to)),
                                             TBmakeId (avisarg))),
                      assign);

    /*
     * create the fundef body block
     */
    block = TBmakeBlock (assign, TBmakeVardec (avisres, NULL));

    /*
     * create the fundef node
     */
    result = TBmakeFundef (ILIBstringCopy (name), NSdupNamespace (ns),
                           TBmakeRet (TYcopyType (to), NULL), TBmakeArg (avisarg, NULL),
                           block, NULL);

    DBUG_RETURN (result);
}

node *
OImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OImodule");

    if (MODULE_OBJS (arg_node) != NULL) {
        /* pass on reference to N_modul node */
        INFO_MODULE (arg_info) = arg_node;

        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);

        /* reset to NULL (mainly for debugging) */
        INFO_MODULE (arg_info) = NULL;
    }

    if (MODULE_FILETYPE (arg_node) == F_classimp) {
#if 0
    char *toclass;
    char *fromclass;

    MODULE_TYPES(arg_node)
      =TBmakeTypedef(ILIBstringCopy(MODULE_NAME(arg_node)),
		     ILIBstringCopy(MODULE_NAME(arg_node)),
		     TYcopyType(MODULE_CLASSTYPE(arg_node)),
		     MODULE_TYPES(arg_node));
    
    toclass=(char*)ILIBmalloc(MAX_FILE_NAME);
    fromclass=(char*)ILIBmalloc(MAX_FILE_NAME);

    strcpy(toclass, "to_");
    strcpy(fromclass, "from_");
    
    strcat(toclass, MODULE_NAME(arg_node));
    strcat(fromclass, MODULE_NAME(arg_node));
    
    MODULE_FUNDECS(arg_node)
      =TBmakeFundef(toclass,
		    ILIBstringCopy( MODULE_NAME( arg_node)),

		    TCreturnTypes2Ret( 
		      TBmakeTypes(T_user, 0, NULL,
			   ILIBstringCopy(MODULE_NAME(arg_node)),
				  ILIBstringCopy(MODULE_NAME(arg_node)))),

		    TBmakeArg( TBmakeAvis( 
				ILIBstringCopy( "obj"),
				TYcopyType( MODULE_CLASSTYPE(arg_node))),
			       NULL),
		    NULL,
		    MODULE_FUNDECS(arg_node));
    
    FUNDEF_ISCLASSCONVERSIONFUN( MODULE_FUNDECS( arg_node)) = TRUE;     

    MODULE_FUNDECS(arg_node)

      =TBmakeFundef(fromclass,
		    ILIBstringCopy( MODULE_NAME( arg_node)),

		    TCreturnTypes2Ret( 
		       TBmakeTypes(T_user, 0, NULL,
			  ILIBstringCopy(MODULE_NAME(arg_node)),
			  ILIBstringCopy(MODULE_NAME(arg_node)))),	      

		       TBmakeArg( TBmakeAvis( 
				    ILIBstringCopy( "obj"),
			            TYcopyType( MODULE_CLASSTYPE(arg_node))), 
				  NULL),
		    NULL,
		    MODULE_FUNDECS(arg_node));

    FUNDEF_ISCLASSCONVERSIONFUN( MODULE_FUNDECS( arg_node)) = TRUE;
    
    MODULE_CLASSTYPE(arg_node)=NULL;
#endif
    }

    DBUG_RETURN (arg_node);
}

node *
OIobjdef (node *arg_node, info *arg_info)
{
    node *new_node;
    types *new_fun_type;
    char *new_fun_name;

    DBUG_ENTER ("OIobjdef");
#if 0
  /*
  if (OBJDEF_EXPR(arg_node)!=NULL) {
    new_fun_type=TBmakeTypes(TYgetSimpleType( 
			       TYgetScalar( OBJDEF_TYPE(arg_node))),
			     TYgetDim( OBJDEF_TYPE(arg_node)),
			     SHshape2OldShpseg( 
			       TYgetShape( OBJDEF_TYPE(arg_node))),
			     ILIBstringCopy(OBJDEF_TNAME(arg_node)),
			     OBJDEF_TMOD(arg_node));
  */ /* TODO wrong */
    new_fun_name=(char*)ILIBmalloc(strlen(OBJDEF_NAME(arg_node)) + 10);
  
    new_fun_name=strcpy(new_fun_name, "CREATE__");
    new_fun_name=strcat(new_fun_name, OBJDEF_NAME(arg_node));
    
    new_node=TBmakeExprs(OBJDEF_EXPR(arg_node), NULL);
    NODE_LINE(new_node)=NODE_LINE(OBJDEF_EXPR(arg_node));
  
    new_node=TBmakeReturn(new_node);
    NODE_LINE(new_node)=NODE_LINE(OBJDEF_EXPR(arg_node));
  
    new_node=TBmakeAssign(new_node, NULL);
    NODE_LINE(new_node)=NODE_LINE(OBJDEF_EXPR(arg_node));
    
    new_node=TBmakeBlock(new_node, NULL);
    NODE_LINE(new_node)=NODE_LINE(OBJDEF_EXPR(arg_node));
  
    new_node=TBmakeFundef(new_fun_name, OBJDEF_MOD(arg_node),
                        new_fun_type, NULL, new_node,
                        MODULE_FUNS(INFO_MODULE(arg_info)));
    NODE_LINE(new_node)=NODE_LINE(OBJDEF_EXPR(arg_node));
 
    FUNDEF_STATUS(new_node)=ST_objinitfun;
  
    /*
     * The new functions have status ST_objinitfun.
     * This tag is needed by the typechecker to ensure that these functions
     * are actually typechecked even if they are exclusively applied in
     * a global object initialization of a SAC-program.
     */

  
    MODULE_FUNS(INFO_MODULE(arg_info))=new_node;
  
    new_node= TBmakeAp(ILIBstringCopy(new_fun_name), 
		       ILIBstringCopy( OBJDEF_MOD(arg_node)), NULL);
    OBJDEF_EXPR(arg_node)=new_node;

    AP_FUNDEF(OBJDEF_EXPR(arg_node)) =
      MODULE_FUNS(INFO_MODULE(arg_info));
    
  }
  else {
    new_fun_type=TBmakeTypes(OBJDEF_BASETYPE(arg_node),
                          OBJDEF_DIM(arg_node),
                          DUPdupShpseg(OBJDEF_SHPSEG(arg_node)),
                          ILIBstringCopy(OBJDEF_TNAME(arg_node)),
                          OBJDEF_TMOD(arg_node));

    if (OBJDEF_INITFUN(arg_node)!=NULL) {
      new_fun_name=OBJDEF_INITFUN(arg_node);
      PRAGMA_INITFUN(OBJDEF_PRAGMA(arg_node))=NULL;
    }
    else {
      new_fun_name=(char*)ILIBmalloc(strlen(OBJDEF_NAME(arg_node)) + 10);
  
      if (((sbs==1) && (strcmp( OBJDEF_MOD( arg_node), EXTERN_MOD_NAME) != 0)) ||
          ((sbs==0) && (OBJDEF_MOD( arg_node) != NULL))) {
        strcpy(new_fun_name, "CREATE__");
      }
      else {
        strcpy(new_fun_name, "create_");
      }
      
      strcat(new_fun_name, OBJDEF_NAME(arg_node));
    }
    
    new_node=TBmakeFundef(new_fun_name, OBJDEF_MOD(arg_node),
			  new_fun_type, NULL, NULL,
			  MODULE_FUNDECS(INFO_MODULE(arg_info)));
 
    FUNDEF_STATUS(new_node)=ST_objinitfun;
  
    MODULE_FUNDECS(INFO_MODULE(arg_info))=new_node;

    new_node=TBmakeAp(ILIBstringCopy(new_fun_name), 
		      OBJDEF_MOD(arg_node), NULL);
    
    OBJDEF_EXPR(arg_node)=new_node;
    
    AP_FUNDEF(OBJDEF_EXPR(arg_node)) = 
      MODULE_FUNDECS(INFO_MODULE(arg_info));
  }

  if ((OBJDEF_PRAGMA(arg_node)!=NULL) &&
      (PRAGMA_LINKNAME( OBJDEF_PRAGMA(arg_node))==NULL)) {
    OBJDEF_PRAGMA(arg_node)=FREEdoFreeNode(OBJDEF_PRAGMA(arg_node));
  }
  
  if (OBJDEF_NEXT(arg_node)!=NULL) {
    OBJDEF_NEXT(arg_node)=TRAVdo(OBJDEF_NEXT(arg_node), arg_info);
  }
#endif
    DBUG_RETURN (arg_node);
}

node *
OIdoObjectInit (node *syntax_tree)
{
    DBUG_ENTER ("OIdoObjectInit");

    info *info;

    info = MakeInfo ();

    TRAVpush (TR_objini);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
