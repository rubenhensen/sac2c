#include "globals.h"
#include "memory.h"
#include "namespaces.h"
#include "new_types.h"
#include "print.h"
#include "shape.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "HS"
#include "debug.h"

#include "hide_structs.h"

/**
 *
 * @file hide_structs.c
 *
 * Hide struct definitions behind typedefs and accessors.
 *
 * After parsing there can now exist various function applications in
 * the AST that operate on structs; constructors, accessors and mutators.
 * In order to allow type inference, we need to make sure that we have
 * function declarations for these primitive functions.
 *
 *
 * At this point we do not know how records are represented
 * in memory, and assume that an implementation is given during
 * code generation. In practice we implement these primitives during DSS.
 *
 * The actual implementation is deferred to this later phase as this allows
 * us to do the implementation after the function _accu_ has been made explicit.
 * That in turn allows us to implement the fold with-loop for arrays of
 * structures.
 * Additionally it is more convenient to do the implementation when the AST is
 * flattened and in SSA form, as this makes reasoning about the transformations
 * we do easier.
 *
 * In addition to the primitives that we later generate in DSS, we may also
 * have applications of sel, zero, accessors and mutators on arrays of records.
 * For these, we can provide a function definition, rather
 * than a function declaration. This can be done without fully implementing
 * structs as we can use the primitive functions as building blocks.
 * We define these functions here, rather than in DSS, as their definition
 * can be done in a way that is implementation-independent.
 * For other datatypes, these functions are implemented in the standard library,
 * but for records we would need users to write them along with each distinct
 * struct definition.
 * For the user's convenience we have decided to generate them here for every
 * struct definition.
 *
 * Example:
 *
 * struct body { double[3] pos; int mass; };
 *
 * is parsed into an N_typedef with the above
 * definition as son TYPEDEF_STRUCTDEF,
 *
 * external nested _struct_body;
 *
 * From this definition we generate the following
 *   Constructor, and a default constructor.
 *   This default constructor sets all fields to their zero value.
 *
 * _struct_body    _struct_con_body (double[3] pos, int mass);
 * _struct_body    _struct_con_body ();
 *
 *
 *   Accessors and Mutators:
 *
 * double[3]     _struct_get_pos  (_struct_body s);
 * _struct_body  _struct_set_pos  (double[3] e, _struct_body s);
 * int           _struct_get_mass (_struct_body s);
 * _struct_body  _struct_set_mass (int e, _struct_body s);
 *
 *   Array accessors and array mutators:
 *
 * double[+]        _struct_get_pos  (_struct_body[+] s) { ... }
 * _struct_body[+]  _struct_set_pos  (double[+] e, _struct_body[+] s) { ... }
 * int[+]           _struct_get_mass (_struct_body[+] s) { ... }
 * _struct_body[+]  _struct_set_mass (int[+] e, _struct_body[+] s) { ... }
 *
 *   Zero and sel:
 *
 * _struct_Body     zero (_struct_body[*] e) { ... }
 * _struct_body[*]  sel  (int[.] idx, _struct_body[*] array) { ... }
 *
 *
 *
 * The array accessors and array muators are implemented as a with-loop
 * that simply maps the corresponding scalar version.
 * Zero is implemented by returning the default constructor; _struct_body_con().
 * Sel is implemented with a with-loop over that uses _sel_VxA_
 * to select elements from 'array', allows selecting subarrays.
 *
 * A later phase (DSS) is expected to give implementations for:
 * the scalar _struct_get_pos & _struct_set_pos,
 * _struct_con_body() and _struct_con_body(double[3] pos, int mass)
 * _shape_A_, _sel_VxA_ and _dim_A_.
 *
 * The renaming conventions are being defined in hide_structs.h
 * via macros STRUCT_TYPE_PREFIX, etc.
 * they are being shared with the parser in parser.c.
 *
 * The implementation traverses struct definitions only.
 *
 * In N_structelem, we generate fundecs for the (array-) setter
 * and (array-) getter for that element.
 * Additionally we create an argument which we store in the info struct.
 *
 * In N_structdef we generate the functions 'zero' and 'sel', we also generate
 * the declaration for the constructor using the arguments generated
 * by each N_structelem.
 *
 * Afterwards, at the N_module, we add these fundefs and fundecs to the module.
 *
 * The struct definitions are kept around, as they are needed in DSS.
 *
 */

/**
 * INFO structure
 *
 * @param new_fundecs List of N_fundec to be inserted into the module.
 * @param init_args List of N_arg to be used to create the constructor.
 * @param structdef Pointer to the current structdef when traversing the struct
 *        elements.
 * @param structtype Pointer to a ntype object that represents the userntype of
 *        the structure that is currently being traversed. NULL if there is no
 *        structdef in the current traversal stack.
 * @param elem_count used to provide the position of the structelem within its
 *        structdef.
 *        e.g in struct body the position of pos is 0,
 *        and the position of mass is 1.
 * @param namespace The namespace of the current module
 */
struct INFO {
    node *new_fundecs;
    node *new_funs;
    node *init_args;
    node *structdef;
    ntype *structtype;
    int elem_count;
    namespace_t *namespace;
};

/**
 * INFO macros
 */
#define INFO_NEW_FUNDECS(n) ((n)->new_fundecs)
#define INFO_NEW_FUNS(n) ((n)->new_funs)
#define INFO_INIT_ARGS(n) ((n)->init_args)
#define INFO_STRUCTDEF(n) ((n)->structdef)
#define INFO_STRUCTTYPE(n) ((n)->structtype)
#define INFO_ELEMCOUNT(n) ((n)->elem_count)
#define INFO_NAMESPACE(n) ((n)->namespace)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_NEW_FUNDECS (result) = NULL;
    INFO_NEW_FUNS (result) = NULL;
    INFO_INIT_ARGS (result) = NULL;
    INFO_STRUCTDEF (result) = NULL;
    INFO_STRUCTTYPE (result) = NULL;
    INFO_ELEMCOUNT (result) = 0;
    INFO_NAMESPACE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *generateConstructor (node *arg_node, info *arg_info)
 *
 * @brief Create a constructor for this structdef, taking every field as
 * an argument. These fields are not expanded in the case of a nested record.
 * Example with struct body:
 *     external _struct_body _struct_con_body (double[3] pos, int mass);
 *
 * Example with a nested record:
 *     struct outer{struct body inner;};
 *     external _struct_outer _struct_con_outer (struct body inner);
 *
 ******************************************************************************/
static node *
generateConstructor (ntype *structtype, const char *name, info *arg_info)
{
    node *fundec;

    DBUG_ENTER ();

    // Constructor from fields
    fundec = TBmakeFundef (STRcat (STRUCT_CON_PREFIX, name),
                           NSdupNamespace(INFO_NAMESPACE (arg_info)),
                           TBmakeRet (TYcopyType (structtype), NULL),
                           INFO_INIT_ARGS (arg_info),
                           NULL,
                           NULL);
    INFO_INIT_ARGS (arg_info) = NULL;

    FUNDEF_ISEXTERN (fundec) = TRUE;
    FUNDEF_ISSTRUCTCONSTR (fundec) = TRUE;

    DBUG_PRINT ("generated declaration for the constructor:");
    DBUG_EXECUTE ( PRTdoPrintHeaderFile (stderr, fundec););


    DBUG_RETURN (fundec);
}

/** <!--********************************************************************-->
 *
 * @fn static node *zeroElem (node *elem)
 *
 * @brief Create an application of the function 'zero' for a struct's element.
 * If the element type is an array, create a withloop to build an array of
 * empty elements using the shape of the element type.
 *
 ******************************************************************************/
static node *
zeroElem (node *elem)
{
  ntype *ty, *aty;
  namespace_t *ns = NULL;

  node *res, *arr, *zero, *shape;

  DBUG_ENTER();
  DBUG_ASSERT (elem != NULL,
               "expected a valid struct element in zeroElem, but got NULL");
  ty = STRUCTELEM_TYPE (elem);

  // get the zero function from the prelude if not a symbol type
  if (!TYisArray (ty) || !TYisSymb (TYgetScalar (ty))) {
    ns = NSgetNamespace (global.preludename);
  }


  // we need to look at the dimensionality of the type...., if not a scalar
  // then we need to withloop an array
  if ( TYgetDim (ty) == 0 ) {
    // `zero([e_i])`
    arr = TBmakeArray (TYcopyType (STRUCTELEM_TYPE (elem)),
                       SHcreateShape (1,0), NULL);
    res = TBmakeSpap (TBmakeSpid (ns, STRcpy ("zero") ),
                      TBmakeExprs (arr,NULL));
  } else {
    // first create the same `zero([e_i])`, removing the shape
    aty = TYmakeAKS (TYcopyType (TYgetScalar (STRUCTELEM_TYPE (elem))),
                     SHcreateShape (0));
    arr = TBmakeArray (aty , SHcreateShape (1,0), NULL);
    zero = TBmakeSpap (TBmakeSpid (ns, STRcpy ("zero") ),
                       TBmakeExprs (arr,NULL));

    // get the shape from ty, and create an exprs chain of it....
    shape = SHshape2Array (TYgetShape (ty));

    // `with {}: genarray([shp], zero([e_i])` where shp is
    // the shape of `e_i`s type
    res = TBmakeWith (NULL, NULL, TBmakeGenarray (shape, zero));
  }

  DBUG_RETURN(res);
}

/** <!--********************************************************************-->
 *
 * @fn static node *zeroElems (node *elem)
 *
 * @brief Recursively create an application of the function 'zero' for
 * every element of a struct.
 *
 ******************************************************************************/
static node *
zeroElems (node *elem)
{
  node *res = NULL;
  DBUG_ENTER();

  if (elem != NULL) {
    res = TBmakeExprs (zeroElem(elem), zeroElems (STRUCTELEM_NEXT (elem)));
  }

  DBUG_RETURN(res);
}

/** <!--********************************************************************-->
 *
 * @fn static node *generateDefaultCons (ntype *basestructtype, info *arg_info,
 *                                    node* structdef)
 *
 * @brief Create a constructor for this structdef, taking no elements as
 * argument. Returns an empty struct by calling a chain of `zero`
 * applications for each element.
 * Example with struct body:
 * @example
 * _struct_body _struct_con_body(){
 *     return _struct_con_body(
 *         with {}: genarray ([ 3 ], sacprelude_d::zero ([:double])),
 *         sacprelude_d::zero ([:int])
 *     );
 * }
 *
 ******************************************************************************/
static node *
generateDefaultCons (ntype *basestructtype, info *arg_info, node* structdef)
{
    node *ret;
    node *con_args;
    node *cons_name;
    node *n_return;
    node *assigns;
    node *fundef;
    DBUG_ENTER ();

    // return type `struct <x>[0]`
    ret = TBmakeRet (TYmakeAKS (TYcopyType (basestructtype),
                                SHcreateShape (0)),
                                NULL);

    // apply the zero function to every element
    con_args = zeroElems (STRUCTDEF_STRUCTELEM (structdef));

    // `return _struct_cons_<x>( zero(e_0), ..., zero(e_n))`
    cons_name = TBmakeSpid (NULL, STRcat (STRUCT_CON_PREFIX,
                                          STRUCTDEF_NAME (structdef)));
    n_return = TBmakeReturn (TBmakeExprs (TBmakeSpap (cons_name, con_args),
                                          NULL));
    assigns = TBmakeAssign (n_return, NULL);


    fundef = TBmakeFundef (STRcat (STRUCT_CON_PREFIX,
                                   STRUCTDEF_NAME (structdef)),
                           NSdupNamespace (INFO_NAMESPACE (arg_info)),
                           ret,
                           NULL,
                           TBmakeBlock (assigns, NULL),
                           NULL);

    FUNDEF_ISINLINE (fundef) = TRUE;
    DBUG_PRINT ("generated implementation of the default constructor");


    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn static node *generateZero (ntype *basestructtype, info *arg_info,
 *                                node* structdef)
 *
 * @brief Create an implementation for `zero` by calling the
 * default constructor.
 * @example
 * Example with struct body:
 * _struct_body zero (_struct_body[*] e){
 *     return _struct_con_body();
 * }
 *
 ******************************************************************************/
static node *
generateZero ( ntype *basestructtype, info *arg_info, node* structdef)
{
    node *avis_e;
    node *arg;
    node *ret;
    node *cons_name;
    node *n_return;
    node *assigns;
    node *fundef;

    DBUG_ENTER ();

    // argument struct <x>[*] e
    avis_e = TBmakeAvis (STRcpy ("e"), TYmakeAUD (TYcopyType (basestructtype)));
    AVIS_DECLTYPE (avis_e) = TYcopyType (AVIS_TYPE (avis_e));
    arg = TBmakeArg (avis_e,   NULL);
    AVIS_DECL (avis_e) = arg;

    // set return type to struct <x>
    ret = TBmakeRet (TYmakeAKS (TYcopyType (basestructtype),
                                 SHcreateShape (0,0)),
                      NULL);

    // `return _struct_con_<x>();`
    cons_name = TBmakeSpid (NULL, STRcat (STRUCT_CON_PREFIX,
                                          STRUCTDEF_NAME (structdef)));
    n_return = TBmakeReturn (TBmakeExprs (TBmakeSpap (cons_name, NULL), NULL));
    assigns = TBmakeAssign (n_return, NULL);


    fundef = TBmakeFundef (STRcpy ("zero"),
                           NSdupNamespace (INFO_NAMESPACE (arg_info)),
                           ret,
                           arg,
                           TBmakeBlock (assigns, NULL),
                           NULL);

    FUNDEF_ISINLINE (fundef) = TRUE;
    DBUG_PRINT ("generated implementation of \"zero\"");

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn static node *generateSelWith ()
 *
 * @brief Creates the with expression for the general selection in a struct
 * array. See the example of generateSel.
 *
 ******************************************************************************/
static node *
generateSelWith(void){

    node *cat_args;
    node *inner_assigns;
    node *withsel;
    node *code;
    node *gen;
    node *part;
    node *zero_arg;
    node *withop;

    DBUG_ENTER ();

    // `new_idx = _cat_VxV_( idx, iv);`, the body for the code
    cat_args = TBmakeExprs (TBmakeSpid (NULL, STRcpy ("idx")),
                            TBmakeExprs( TBmakeSpid (NULL, STRcpy ("iv")),
                            NULL));
    inner_assigns = TBmakeAssign (TBmakeLet (
      TBmakeSpids (STRcpy ("new_idx"),NULL), TBmakePrf (F_cat_VxV, cat_args)
      ),NULL);

    // `_sel_VxA_(new_idx, array)`, the result expression
    withsel = TBmakePrf (F_sel_VxA,
                         TBmakeExprs (TBmakeSpid (NULL, STRcpy ("new_idx")),
                                      TBmakeExprs (
                                          TBmakeSpid (NULL,
                                                      STRcpy ("array")),
                                      NULL)) );

    code = TBmakeCode (TBmakeBlock (inner_assigns, NULL),
                       TBmakeExprs (withsel, NULL));

    // `( . <= iv <= . )`
    gen = TBmakeGenerator (F_wl_le, F_wl_le, TBmakeDot (1), TBmakeDot (1),
                           NULL, NULL);
    part = TBmakePart (code,
                       TBmakeWithid (TBmakeSpids (STRcpy ("iv"), NULL), NULL),
                       gen);
    CODE_USED (code)++;

    // `genarray( new_shape, zero( array ) )`
    zero_arg = TBmakeExprs (TBmakeSpid (NULL, STRcpy ("array")) ,NULL);
    withop = TBmakeGenarray (TBmakeSpid (NULL, STRcpy ("new_shape")),
                             TBmakeSpap (TBmakeSpid (NULL,
                                                     STRcpy ("zero")),
                                                     zero_arg));

    DBUG_RETURN (TBmakeWith (part, code, withop));

}

/** <!--********************************************************************-->
 *
 * @fn static node *generateSelect ()
 *
 * @brief Creates the general selection for a struct array.
 * Example with struct body:
 * @example
 * _struct_body[*] sel (int [.] dix, _struct_body[*] array){
 *     new_shape = _drop_SxV_ (_sel_VxA_( [ 0 ],
 *                             _shape_A_( idx)),
 *                             _shape_A_( array));
 *     res = with {
 *         (. <= iv <= .) {
 *             new_idx = _cat_VxV_( idx, iv);
 *         } : _sel_VxA_( new_idx, array);
 *     } : genarray( new_shape, zero( array) );
 *
 *     return( res);
 * }
 *
 ******************************************************************************/
static node *
generateSelect ( ntype *basestructtype, info *arg_info)
{
    node *arg;
    node *ret;
    node *fundef;
    node *avis_a;
    node *avis_idx;
    node *assigns;

    node *n_return;


    node *idx;
    node *idx_shape;
    node *arg1;
    node *arg2;

    DBUG_ENTER ();

    // create arguments struct <x>[*] array and int[.] idx

    avis_a = TBmakeAvis (STRcpy ("array"),
                         TYmakeAUD (TYcopyType (basestructtype)));
    avis_idx = TBmakeAvis (STRcpy ("idx"),
                           TYmakeAKD (TYmakeSimpleType (T_int),
                                      1,
                                      SHmakeShape (0)));
    AVIS_DECLTYPE (avis_a) = TYcopyType (AVIS_TYPE (avis_a));
    AVIS_DECLTYPE (avis_idx) = TYcopyType (AVIS_TYPE (avis_idx));
    arg = TBmakeArg (avis_a, NULL);
    AVIS_DECL (avis_a) = arg;
    arg = TBmakeArg (avis_idx, arg);
    AVIS_DECL (avis_idx) = arg;

    // create return type struct<x>[*]

    ret = TBmakeRet (TYmakeAUD (TYcopyType (basestructtype)), NULL);


    // create `return res;`
    n_return = TBmakeReturn (TBmakeExprs (TBmakeSpid (NULL,
                                                      STRcpy ("res")),
                                                      NULL));
    assigns = TBmakeAssign (n_return, NULL);

    // assign the withloop to res
    assigns = TBmakeAssign (TBmakeLet (TBmakeSpids (STRcpy ("res"),NULL),
        generateSelWith ()
      ),assigns);


    // `new_shape = _drop_SxV_ (_sel_VxA_ ([0], _shape_A_(idx)),
    //                                     _shape_A_(array));`
    idx = TCmakeVector (TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)),
                        TBmakeExprs (TBmakeNum (0), NULL));
    idx_shape = TBmakePrf (F_shape_A,
                           TBmakeExprs (TBmakeSpid (NULL,
                                                    STRcpy ("idx")),
                                                    NULL));
    arg1 = TBmakePrf (F_sel_VxA,
                      TBmakeExprs (idx, TBmakeExprs (idx_shape, NULL)));
    arg2 = TBmakePrf (F_shape_A,
                      TBmakeExprs (TBmakeSpid (NULL, STRcpy ("array")), NULL));

    assigns = TBmakeAssign (TBmakeLet (TBmakeSpids (STRcpy ("new_shape"), NULL),
        TBmakePrf (F_drop_SxV, TBmakeExprs (arg1, TBmakeExprs (arg2, NULL)))
      ), assigns);


    fundef = TBmakeFundef (STRcpy ("sel"),
                           NSdupNamespace (INFO_NAMESPACE (arg_info)),
                           ret,
                           arg,
                           TBmakeBlock (assigns, NULL),
                           NULL);

    FUNDEF_ISINLINE (fundef) = TRUE;
    DBUG_PRINT ("generated implementation of \"sel\"");


    DBUG_RETURN (fundef);
}


/** <!--********************************************************************-->
 *
 * @fn static node *generateScalarSelect (ntype *basestructtype, info *arg_info)
 *
 * @brief Creates a scalar selection for a struct array.
 *
 * @example
 * _struct_body[*] sel (int idx, _struct_body[*] array)
 * {
 *     return sel([idx], array);
 * }
 *
 ******************************************************************************/
static node *
generateScalarSelect (ntype *basestructtype, info *arg_info)
{
    node *avis_arr, *avis_idx, *args, *ret;
    node *idx_arg, *arr_arg, *sel_args, *n_return;
    node *fundef;

    DBUG_ENTER ();

    avis_arr = TBmakeAvis (STRcpy ("array"),
                           TYmakeAUD (TYcopyType (basestructtype)));
    avis_idx = TBmakeAvis (STRcpy ("idx"),
                           TYmakeAKS (TYmakeSimpleType (T_int),
                                      SHcreateShape (0)));
    AVIS_DECLTYPE (avis_arr) = TYcopyType (AVIS_TYPE (avis_arr));
    AVIS_DECLTYPE (avis_idx) = TYcopyType (AVIS_TYPE (avis_idx));
    args = TBmakeArg (avis_arr, NULL);
    AVIS_DECL (avis_arr) = args;
    args = TBmakeArg (avis_idx, args);
    AVIS_DECL (avis_idx) = args;

    ret = TBmakeRet (TYmakeAUD (TYcopyType (basestructtype)), NULL);

    idx_arg = TBmakeArray (TYmakeAKS (TYmakeSimpleType (T_int),
                                      SHcreateShape (0)),
                           SHcreateShape (1, 1),
                           TBmakeExprs (TBmakeSpid (NULL, STRcpy ("idx")),
                                        NULL));
    arr_arg = TBmakeSpid (NULL, STRcpy ("array"));
    sel_args = TBmakeExprs (idx_arg, TBmakeExprs (arr_arg, NULL));
    n_return = TBmakeSpap (TBmakeSpid (NULL, STRcpy ("sel")), sel_args);
    n_return = TBmakeExprs (n_return, NULL);
    n_return = TBmakeReturn (n_return);
    n_return = TBmakeAssign (n_return, NULL);

    fundef = TBmakeFundef (STRcpy ("sel"),
                           NSdupNamespace (INFO_NAMESPACE (arg_info)),
                           ret,
                           args,
                           TBmakeBlock (n_return, NULL),
                           NULL);

    FUNDEF_ISINLINE (fundef) = TRUE;
    DBUG_PRINT ("generated implementation of scalar \"sel\"");

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn static node *generateGetter (node *arg_node, info *arg_info,
 *                                  ntype *structtype)
 *
 * @brief Create getter for a scalar value of the associated struct.
 * The implementation for this function is given in the phase DSS.
 * external int _struct_get_mass (_struct_body s);
 *
 ******************************************************************************/
static node *
generateGetter(node *arg_node, info *arg_info, ntype *structtype){
    node *avis;
    node *arg;
    node *ret;
    node *fundec;

    DBUG_ENTER();

    // create the struct argument "s"
    avis = TBmakeAvis (STRcpy ("s"), TYcopyType (structtype));
    arg = TBmakeArg (avis, NULL);
    AVIS_DECLTYPE (avis) = TYcopyType (structtype);

    ret = TBmakeRet (TYcopyType (STRUCTELEM_TYPE (arg_node)), NULL);
    fundec = TBmakeFundef (STRcat (STRUCT_GETTER_PREFIX,
                                   STRUCTELEM_NAME (arg_node)),
                           NSdupNamespace (INFO_NAMESPACE (arg_info)),
                           ret,
                           arg,
                           NULL,
                           NULL);
    FUNDEF_ISEXTERN (fundec) = TRUE;
    FUNDEF_ISSTRUCTGETTER (fundec) = TRUE;
    FUNDEF_STRUCTPOS (fundec) = INFO_ELEMCOUNT (arg_info);

    DBUG_PRINT ("generated declaration for the getter of element %s:",
                STRUCTELEM_NAME (arg_node));
    DBUG_EXECUTE ( PRTdoPrintHeaderFile (stderr, fundec););

    DBUG_RETURN(fundec);
}

/** <!--********************************************************************-->
 *
 * @fn static node *generateArrayGetter (node *elem, info *arg_info,
 *                                       ntype *structtype)
 *
 * @brief Create getter for an array of the associated struct.
 * Example with struct body's mass getter:
 * @example
 * int[+] _struct_get_mass (_struct_body[+] s){
 *     return (with {
 *         (. <= iv <= .) : _struct_get_mass( sel(iv,s) );
 *     }: genarray( _shape_A_(s), zero([:int]) ));
 *  }
 *
 ******************************************************************************/
static node *
generateArrayGetter(node *elem, info *arg_info, ntype *structtype){
    node *avis_e;
    node *arg;
    node *ret;

    node *sel_args;
    node *sel;
    node *args;
    node *get;

    node *code;
    node *gen;
    node *part;
    node *shape;
    node *zero;
    node *with;

    node *assigns;

    node *fundef;

    DBUG_ENTER();

    // create argument struct <x>[+] s
    avis_e = TBmakeAvis (STRcpy ("s"),
                         TYmakeAUDGZ (TYcopyType ( TYgetScalar (structtype) )));
    AVIS_DECLTYPE (avis_e) = TYcopyType (AVIS_TYPE (avis_e));
    arg = TBmakeArg (avis_e, NULL);
    AVIS_DECL (avis_e) = arg;

    // create return type `element_type[+]`
    ret = TBmakeRet (TYmakeAUDGZ (
                         TYcopyType (TYgetScalar (STRUCTELEM_TYPE (elem)))),
                         NULL);

    // `_struct_get_x(s[iv])`, cexpr for the partition
    sel_args = TBmakeExprs (TBmakeSpid (NULL, STRcpy("iv")),
                            TBmakeExprs (TBmakeSpid (NULL, STRcpy ("s")),
                                         NULL));
    sel = TBmakeSpap (TBmakeSpid (NULL, STRcpy("sel")), sel_args);
    args = TBmakeExprs (sel, NULL);
    get = TBmakeSpap (TBmakeSpid (NULL,
                                  STRcat (STRUCT_GETTER_PREFIX,
                                          STRUCTELEM_NAME (elem))),
                                  args);

    // `( . <= iv <= . )`
    code = TBmakeCode (TBmakeBlock(NULL,NULL), TBmakeExprs (get, NULL));
    gen = TBmakeGenerator (F_wl_le, F_wl_le, TBmakeDot (1), TBmakeDot (1),
                           NULL, NULL);
    part = TBmakePart (code,
                       TBmakeWithid (TBmakeSpids (STRcpy ("iv"), NULL), NULL),
                       gen);
    CODE_USED (code)++;

    // `_shape_A_(s)` and `zero([:s])` for genarray
    shape = TBmakePrf (F_shape_A,
                       TBmakeExprs (TBmakeSpid (NULL, STRcpy("s")),
                       NULL));
    zero = zeroElem (elem);

    with = TBmakeWith (part, code, TBmakeGenarray (shape, zero));

    assigns = TBmakeAssign (TBmakeReturn (TBmakeExprs ( with, NULL)), NULL);



    fundef = TBmakeFundef (STRcat (STRUCT_GETTER_PREFIX,
                                   STRUCTELEM_NAME (elem)),
                           NSdupNamespace (INFO_NAMESPACE (arg_info)),
                           ret,
                           arg,
                           TBmakeBlock (assigns, NULL),
                           NULL);

    FUNDEF_ISINLINE (fundef) = TRUE;
    DBUG_PRINT ("generated implementation for array getter of element %s",
                STRUCTELEM_NAME (elem));

    DBUG_RETURN(fundef);
}

/** <!--********************************************************************-->
 *
 * @fn static node *generateSetter (node *arg_node, info *arg_info,
 *                                  ntype *structtype)
 *
 * @brief Create setter for a scalar value of the associated struct.
 * The implementation for this function is given in the phase DSS.
 * Example with struct body's mass:
 * @example
 * external _struct_body _struct_set_mass (int e, _struct_body s);
 *
 ******************************************************************************/
static node *
generateSetter(node *arg_node, info *arg_info, ntype *structtype){
    node *avis;
    node *arg;
    node *ret;
    node *fundec;

    DBUG_ENTER();

    // create the struct argument "s"
    avis = TBmakeAvis (STRcpy ("s"), TYcopyType (structtype));
    arg = TBmakeArg (avis, NULL);
    AVIS_DECLTYPE (avis) = TYcopyType (structtype);

    // create the element argument "e"
    avis = TBmakeAvis (STRcpy ("e"), TYcopyType (STRUCTELEM_TYPE (arg_node)));
    arg = TBmakeArg (avis, arg);
    AVIS_DECLTYPE (avis) = TYcopyType (STRUCTELEM_TYPE (arg_node));

    ret = TBmakeRet (TYcopyType (structtype), NULL);
    fundec = TBmakeFundef (STRcat (STRUCT_SETTER_PREFIX,
                                   STRUCTELEM_NAME (arg_node)),
                           NSdupNamespace (INFO_NAMESPACE (arg_info)),
                           ret,
                           arg,
                           NULL,
                           NULL);
    FUNDEF_ISEXTERN (fundec) = TRUE;
    FUNDEF_ISSTRUCTSETTER (fundec) = TRUE;
    FUNDEF_STRUCTPOS (fundec) = INFO_ELEMCOUNT (arg_info);

    DBUG_PRINT ("generated declaration for setter of element %s:",
                STRUCTELEM_NAME (arg_node));
    DBUG_EXECUTE ( PRTdoPrintHeaderFile (stderr, fundec););

    DBUG_RETURN (fundec);
}

/** <!--********************************************************************-->
 *
 * @fn static node *generateArraySetter (node *elem, info *arg_info,
 *                                       ntype *structtype)
 *
 * @brief Create setter for an array of the associated struct.
 * Example with struct body's mass setter:
 * @example
 * _struct_body _struct_set_mass (int[+] e, _struct_body[+] s){
 *     return ( with {
 *         (. <= iv <= .) : _struct_set_mass(sel(iv,e), sel(iv,s) );
 *     }: modarray(s));
 * }
 *
 ******************************************************************************/
static node *
generateArraySetter(node *elem, info *arg_info, ntype *structtype){
    node *avis_e;
    node *avis_s;
    node *arg;
    node *ret;

    ntype* ty;
    namespace_t *ns;

    node *sel_args;
    node *sel;
    node *set;

    node *code;
    node *gen;
    node *part;
    node *with;

    node *assigns;

    node *fundef;

    DBUG_ENTER();

    // create arguments `et[+] e , struct <x>[+] s` where et is the type of elem
    avis_s = TBmakeAvis (STRcpy ("s"),
                         TYmakeAUDGZ (TYcopyType (TYgetScalar (structtype))));
    avis_e = TBmakeAvis (
                 STRcpy ("e"),
                 TYmakeAUDGZ (TYcopyType (
                                  TYgetScalar (STRUCTELEM_TYPE(elem)))));
    AVIS_DECLTYPE (avis_s) = TYcopyType (AVIS_TYPE (avis_s));
    AVIS_DECLTYPE (avis_e) = TYcopyType (AVIS_TYPE (avis_e));
    arg = TBmakeArg (avis_s, NULL);
    AVIS_DECL (avis_s) = arg;
    arg = TBmakeArg (avis_e, arg);
    AVIS_DECL (avis_e) = arg;

    // create return type `et[+]` where et is the type of elem
    ret = TBmakeRet (TYmakeAUDGZ (TYcopyType (TYgetScalar (structtype))), NULL);

    // `_struct_set_<x>(sel(iv,e), sel(iv,s))`, cexpr for the partition
    ty = STRUCTELEM_TYPE ( elem );
    ns = NULL;
    if ( !TYisArray (ty) || !TYisSymb (TYgetScalar (ty)) ) {
      // use sacprelude_p::sel if not a symbol type
      ns = NSgetNamespace (global.preludename);
    }
    sel_args = TBmakeExprs (TBmakeSpid (NULL, STRcpy("iv")),
                            TBmakeExprs (TBmakeSpid (NULL, STRcpy ("s")),
                                         NULL));
    sel = TBmakeExprs (TBmakeSpap (TBmakeSpid (NULL,
                                               STRcpy("sel")),
                                   sel_args),
                       NULL);
    sel_args = TBmakeExprs (TBmakeSpid (NULL, STRcpy("iv")),
                            TBmakeExprs (TBmakeSpid (NULL, STRcpy ("e")),
                                         NULL));
    sel = TBmakeExprs (TBmakeSpap (TBmakeSpid (ns, STRcpy("sel")), sel_args),
                       sel);
    set = TBmakeSpap (TBmakeSpid (NULL,
                                  STRcat (STRUCT_SETTER_PREFIX,
                                          STRUCTELEM_NAME (elem))),
                                  sel);

    // `( . <= iv <= . )`
    code = TBmakeCode (TBmakeBlock(NULL,NULL), TBmakeExprs (set, NULL));
    gen = TBmakeGenerator (F_wl_le, F_wl_le, TBmakeDot (1), TBmakeDot (1),
                           NULL, NULL);
    part = TBmakePart (code,
                       TBmakeWithid (TBmakeSpids (STRcpy ("iv"), NULL), NULL),
                       gen);
    CODE_USED (code)++;

    with = TBmakeWith (part,
                       code,
                       TBmakeModarray (TBmakeSpid (NULL, STRcpy ("s")) ));

    assigns = TBmakeAssign (TBmakeReturn (TBmakeExprs ( with, NULL)), NULL);



    fundef = TBmakeFundef (STRcat (STRUCT_SETTER_PREFIX,
                                   STRUCTELEM_NAME (elem)),
                           NSdupNamespace (INFO_NAMESPACE (arg_info)),
                           ret,
                           arg,
                           TBmakeBlock (assigns, NULL),
                           NULL);

    FUNDEF_ISINLINE (fundef) = TRUE;
    DBUG_PRINT ("generated implementation for array setter of element %s",
                STRUCTELEM_NAME (elem));

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node *HSdoHideStructs (node *syntax_tree)
 *
 * @brief Prepare, initiate and clean up the HideStructs phase.
 *
 ******************************************************************************/
node *
HSdoHideStructs (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting struct hiding.");

    info = MakeInfo ();

    TRAVpush (TR_hs);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("Done hiding all structs.");

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *HSmodule (node *arg_node, info *arg_info)
 *
 * @brief Traverse struct definitions, and add the new fundefs and fundecs
 * which we generated from visiting the structdefs and structelems.
 *
 ******************************************************************************/
node *
HSmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NAMESPACE (arg_info) = MODULE_NAMESPACE (arg_node);

    INFO_NEW_FUNDECS (arg_info) = NULL;
    INFO_NEW_FUNS (arg_info) = NULL;
    // We only traverse the type definitions here!
    MODULE_TYPES (arg_node) = TRAVopt ( MODULE_TYPES (arg_node), arg_info);

    if (INFO_NEW_FUNDECS (arg_info) != NULL) {
      MODULE_FUNDECS (arg_node) = TCappendFundef ( MODULE_FUNDECS (arg_node),
                                                   INFO_NEW_FUNDECS (arg_info));
      INFO_NEW_FUNDECS (arg_info) = NULL;
    }

    if (INFO_NEW_FUNS (arg_info) != NULL) {
      MODULE_FUNS (arg_node) = TCappendFundef ( MODULE_FUNS (arg_node),
                                                INFO_NEW_FUNS (arg_info));
      INFO_NEW_FUNS (arg_info) = NULL;
    }

    INFO_NAMESPACE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HSstructdef (node *arg_node, info *arg_info)
 *
 * @brief For the current struct definition we traverse its elements and then
 * generate the following:
 * - a declaration for the constructor
 * - a default constructor that takes no arguments
 * - an implementation of zero
 * - an implementation of sel
 *
 ******************************************************************************/
node *
HSstructdef (node *arg_node, info *arg_info)
{
    ntype *structtype;
    ntype *basestructtype;
    node *new_fun;
    char *sname;

    DBUG_ENTER ();

    DBUG_PRINT ("----- processing struct \"%s\" -----",
                STRUCTDEF_NAME (arg_node));

    // We create a symbol type in exactly the same way the parser
    // deals with user defined types! We will use this type whenever it is
    // needed as argument or return type for our constructors/ getters/ setters.

    sname = STRcat (STRUCT_TYPE_PREFIX, STRUCTDEF_NAME (arg_node));
    basestructtype = TYmakeSymbType (sname, NULL);
    structtype = TYmakeAKS (TYcopyType (basestructtype), SHmakeShape (0));
    INFO_STRUCTTYPE (arg_info) = structtype;


    // traverse elements, insert elems as args for constructor in
    // INFO_INIT_ARGS, and create setters and getters while doing so:

    DBUG_ASSERT (INFO_INIT_ARGS (arg_info) == NULL,
                 "Garbage constructor arguments lying around in arg_info.");
    INFO_STRUCTDEF (arg_info) = arg_node;
    INFO_ELEMCOUNT (arg_info) = 0;
    STRUCTDEF_STRUCTELEM (arg_node) = TRAVopt (STRUCTDEF_STRUCTELEM (arg_node),
                                               arg_info);

    if (INFO_ELEMCOUNT (arg_info) == 0)
    {
      CTIerror (NODE_LOCATION (arg_node),
                "struct \"%s\" must have at least one field",
                STRUCTDEF_NAME (arg_node));
    }

    new_fun = generateConstructor (structtype,
                                   STRUCTDEF_NAME (arg_node),
                                   arg_info);
    DBUG_PRINT ("Pushing '%s' to the function definition stack of arg_info",
                FUNDEF_NAME (new_fun));
    INFO_NEW_FUNDECS (arg_info) = TCappendFundef (INFO_NEW_FUNDECS (arg_info),
                                                  new_fun);

    new_fun = generateZero (basestructtype, arg_info, arg_node);
    DBUG_PRINT ("Pushing '%s' to the function definition stack of arg_info",
                FUNDEF_NAME (new_fun));
    INFO_NEW_FUNS (arg_info) = TCappendFundef (INFO_NEW_FUNS (arg_info),
                                               new_fun);

    new_fun = generateDefaultCons (basestructtype, arg_info, arg_node);
    DBUG_PRINT ("Pushing '%s' to the function definition stack of arg_info",
                FUNDEF_NAME (new_fun));
    INFO_NEW_FUNS (arg_info) = TCappendFundef (INFO_NEW_FUNS (arg_info),
                                               new_fun);

    new_fun = generateSelect (basestructtype, arg_info);
    DBUG_PRINT ("Pushing '%s' to the function definition stack of arg_info",
                FUNDEF_NAME (new_fun));
    INFO_NEW_FUNS (arg_info) = TCappendFundef (INFO_NEW_FUNS (arg_info),
                                               new_fun);

    new_fun = generateScalarSelect (basestructtype, arg_info);
    DBUG_PRINT ("Pushing '%s' to the function definition stack of arg_info",
                FUNDEF_NAME (new_fun));
    INFO_NEW_FUNS (arg_info) = TCappendFundef (INFO_NEW_FUNS (arg_info),
                                               new_fun);

    STRUCTDEF_NEXT (arg_node) = TRAVopt (STRUCTDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HSstructelem (node *arg_node, info *arg_info)
 *
 * @brief For the current struct element we generate the (array-) getters and
 * setters.
 * Additionally we push this element onto an argument list in the info struct.
 * This will allow us to create a declaration for the constructor.
 *
 ******************************************************************************/

node *
HSstructelem (node *arg_node, info *arg_info)
{
    node *arg;
    node *structdef;
    ntype *structtype;
    node *avis;
    node *new_fun;
    char *tyname;

    DBUG_ENTER ();

    structdef = INFO_STRUCTDEF (arg_info);
    DBUG_ASSERT (structdef != NULL, "No structdef for this struct element.");
    structtype = INFO_STRUCTTYPE (arg_info);
    DBUG_ASSERT (structtype != NULL, "No struct set for this struct element.");

    DBUG_PRINT ( "    processing element \"%s\"", STRUCTELEM_NAME (arg_node));


    if (!TYisAKS(STRUCTELEM_TYPE(arg_node)))
    {
      tyname = TYtype2String (STRUCTELEM_TYPE(arg_node), FALSE, 0);
      CTIerror(
          NODE_LOCATION (arg_node),
          "element \"%s\" of \"struct %s\" must have a "
          "known size but is \"%s\"",
          STRUCTELEM_NAME (arg_node),
          STRUCTDEF_NAME (structdef),
          tyname);
    }

    // getters
    // create the scalar getter function _struct_get_<name>
    new_fun = generateGetter(arg_node, arg_info, structtype);
    DBUG_PRINT ("Adding scalar getter '%s' to the function declaration"
                " stack in arg_info",
                FUNDEF_NAME(new_fun));
    // push the getter on the info struct's fundec stack.
    INFO_NEW_FUNDECS (arg_info) = TCappendFundef (INFO_NEW_FUNDECS (arg_info),
                                                  new_fun);

    // create the array getter function _struct_get_<name>
    new_fun = generateArrayGetter(arg_node, arg_info, structtype);
    DBUG_PRINT ("Adding array getter '%s' to the function definition"
                " stack in arg_info",
                FUNDEF_NAME(new_fun));
    // push the array accessor on the info struct's fundef stack.
    INFO_NEW_FUNS (arg_info) = TCappendFundef (INFO_NEW_FUNS (arg_info),
                                               new_fun);

    // setters
    // create the scalar setter function _struct_set_<name>
    new_fun = generateSetter(arg_node, arg_info, structtype);
    DBUG_PRINT ("Adding scalar setter '%s' to the function declaration"
                " stack in arg_info",
                FUNDEF_NAME(new_fun));
    // push the setter on the info struct's fundec stack.
    INFO_NEW_FUNDECS (arg_info) = TCappendFundef (INFO_NEW_FUNDECS (arg_info),
                                                  new_fun);


    // create the array setter function _struct_get_<name>
    new_fun = generateArraySetter(arg_node, arg_info, structtype);
    DBUG_PRINT ("Adding array setter '%s' to the function definition"
                " stack in arg_info",
                FUNDEF_NAME(new_fun));
    // push the array setter on the info struct's fundef stack.
    INFO_NEW_FUNS (arg_info) = TCappendFundef (INFO_NEW_FUNS (arg_info),
                                               new_fun);

    // Continue with the next structelem.
    INFO_ELEMCOUNT (arg_info)++;
    STRUCTELEM_NEXT (arg_node) = TRAVopt (STRUCTELEM_NEXT (arg_node), arg_info);

    // Create an N_arg for this element so that we can createthe constructor's
    // argument list. This is done bottom-up to so that the order of N_arg is
    // consistent with the order in which the fields are declared in
    // the N_structdef

    avis = TBmakeAvis (STRcpy (STRUCTELEM_NAME (arg_node)),
                       TYcopyType (STRUCTELEM_TYPE (arg_node)));
    arg = TBmakeArg (avis, INFO_INIT_ARGS (arg_info));
    DBUG_PRINT ("Created N_arg '%s', adding to INFO_INIT_ARGS to use it as"
                " an argument for the constructor later on",
                ARG_NAME (arg));

    AVIS_DECL (avis) = arg;
    AVIS_DECLTYPE (avis) = TYcopyType (STRUCTELEM_TYPE (arg_node));

    INFO_INIT_ARGS (arg_info) = arg;

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
