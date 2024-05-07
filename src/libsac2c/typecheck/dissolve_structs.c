#include "dissolve_structs.h"

#include "deserialize.h"
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "LookUpTable.h"
#include "memory.h"
#include "namespaces.h"
#include "new_types.h"
#include "shape.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "type_utils.h"
#include "user_types.h"

#define DBUG_PREFIX "DSS"
#include "debug.h"

#include "hide_structs.h"

/**
 *
 * @file dissolve_structs.c
 *
 * Remove all traces of struct usage.
 *
 * This compiler phase replaces all uses of struct values by new separate
 * values, one for each of the fields of the struct. This effectively removes
 * the notion of structs from the AST.
 *
 * For example, with struct definition
 *     struct body {
 *       double[3] pos;
 *       int mass;
 *     };
 *
 *   We turn
 *     int combinedMass(struct body b1,
 *                      struct body b2)
 *     {
 *       return get_mass(b1) + get_mass(b2);
 *     }
 *
 *   Into
 *     int combinedMass(double[3] b1_pos, int b1_mass,
 *                      double[3] b2_pos, int b2_mass)
 *     {
 *       return get_mass(b1_pos, b1_mass) + get_mass(b2_pos, b2_mass);
 *     }
 *
 * When encountering N_vardec or N_arg of a struct type, the associated
 * N_avis is expanded using its struct definition.
 * The resulting new N_avis are inserted into a look up table that maps the
 * pointer of the old N_avis to pointers to the new N_avises.
 * This look up table is used in assignments to create a replacing assignment
 * whenever a struct type avis is found in it.
 *
 * Example:
 *   Consider the definition for a struct 'body':
 *
 *     struct body {
 *       double[3] pos;
 *       int mass;
 *     };
 *
 *   We consider the following variable declaration:
 *
 *     ...
 *     struct body mars;
 *     ...
 *
 *   First we visit the vardec, and decide that 'mars' should be expanded as
 *   it is of a record type.
 *   We add to the look up table a mapping of the avis belonging to 'mars' to
 *   its replacement avises; ones for 'mars_pos' of type double[3], and
 *   'mars_mass' of type int.
 *
 *   We then append a variable declaration for each entry in
 *   the look up table:
 *
 *     struct body mars;
 *     double[3] mars_pos;
 *     int mars_mass;
 *
 *   The original variable declaration of 'mars' is removed in an anonymous
 *   clean up traversal after traversing the body of this N_fundef,
 *   but is kept around for now.
 *
 *
 *   Consider we later have the following:
 *
 *     ...
 *     mars = _struct_con_mars(pos, mass);
 *     ...
 *
 *   Here we have an application of a user-defined function on the rhs.
 *   We then expand, in place, any struct type values on either side.
 *   The avis that belongs to 'mars' is put into the lookup table, which gives
 *   two replacement avises. For each of these avises, we create a new N_exprs
 *   node:
 *
 *     ...
 *     mars_pos, mars_mass = _struct_con_mars(pos, mass);
 *     ...
 *
 *
 */

 /*************************************
 *
 * @enum traverse_mode
 *
 * @brief Decides how to expand struct values. There are two strategies for
 * expanding structs.
 * The first one is to expand in place.
 * For example in user function applications:
 *     f(mars);
 * Turns into
 *     f(mars_pos, mars_mass);
 *
 * The second strategy is to duplicate a construction n times, and replace the
 * struct value by one of its fully expanded fields.
 * For example in array literals:
 *     bodies = [earth, mars];
 * Turns into
 *     bodies_pos = [earth_pos, mars_pos];
 *     bodies_mass = [earth_mass, mars_mass];
 *
 * This second strategy requires us to figure out how many copies of the
 * enclosing N_let we need. Since bodies is an array of struct body, we need 2
 * copies, one for 'pos', and one for 'mass'. Then in each copy we replace the
 * 'earth' and 'mars' by their pos component, and mass component respectively.
 *
 * The replication strategy requires there to be no more than one struct type
 * in the part that we try to duplicate.
 *
 * The second strategy is the default for N_let, overriden where needed,
 * and is also used the for the with-loop's operations.
 *
 * @param mode_undefined Default value that signals that there is no mode set.
 *        Set when entering an N_block, and after finishing the traversal of
 *        an assignment.
 * @param mode_in_place Expand struct values in place
 *        Set when in an N_ap. Expand N_id of struct type and create.
 *        a new N_exprs chain that replaces the current exprs node.
 * @param mode_repl_count Count how often an N_let or with-loop operator must be
 *        replicated.
 *        Set when entering a let node, modarray, genarray or fold operation of
 *        the with-loop. When in repl mode, the value of numrepls is set on
 *        visiting an N_id, N_ids, some built-in functions and N_array.
 *        After visiting with mode_repl_count, the enclosing N_let or with-loop
 *        operation can be duplicated <numrepls> times.
 * @param mode_replace Replace struct value with that of a specific element.
 *        In replace mode, a visited N_id or N_type is replaced by the
 *        replacement on the index given by <replace_by>
 *
 *************************************/
enum traverse_mode {
    mode_undefined,
    mode_in_place,
    mode_repl_count,
    mode_replace
};

/**
 * INFO structure
 *
 * @param lut Look up table mapping an old N_avis to its replacements.
 * @param replaceexprs List of exprs that replace the current exprs.
 * @param replaceassigns List of assignments that replace
 *        the current assignment.
 * @param newvardecs List of new assignments that are created by traversing
 *        an assignment.
 * @param newvardecs List of new vardecs that are created by traversing
 *        a struct vardec.
 * @param traverse_mode The current strategy for replacing struct values,
 *        either by expanding in place, or by duplicating N_let and replacing
 *        the struct value in each copy with the corresponding field.
 * @param numrepls Set to the amount of replacements for a struct when an
 *        id or type is found belonging to a struct.
 * @param defrepls Save the structdef we used to set numrepls.
 *        In mode_replace we can only replace the occurance of _one_
 *        struct type. When we first encounter a struct in mode_repl_count,
 *        we save the structdef in addition to setting numrepls.
 *        This allows us to ensure that we do not have multiple struct types
 *        in a single N_let whenever we attempt to duplicate it.
 * @param replaceby Index into the list of replacements for a struct value.
 * @param inprfde Used to see if visiting dispatch_error.
 * @param replacecount Amount of types that we have as an argument for the
 *        predefined function dispatch_error after expanding
 *        a struct value.
 * @param curassign Pointer to the current assignment in the current block, is
          used to set SSAASSIGN.
 * @param newfundefs List of new function definitions that are created after
 *        giving a body to the function declarations of constructors,
 *        getters and setters.
 * @param markedelement Pointer to the element that a getter or setter
 *        operates on.
 * @param in_block Keeps track of the level of nesting, used to make sure that
 *        N_vardec are inserted on the top level block only.
 */
struct INFO {

    lut_t *lut;

    node *replaceexprs;
    node *replaceassigns;
    node *newassigns;
    node *newvardecs;

    enum traverse_mode mode;
    int numrepls;
    node *defrepls;
    int replaceby;

    int inprfde;
    int replacecount;

    node *curassign;
    node *newfundefs;
    node *markedelement;

    int in_block;
};


/**
 * INFO macros
 */

#define INFO_LUT(n) ((n)->lut)

#define INFO_REPLACE_EXPRS(n) ((n)->replaceexprs)
#define INFO_REPLACE_ASSIGNS(n) ((n)->replaceassigns)
#define INFO_NEW_ASSIGNS(n) ((n)->newassigns)
#define INFO_NEW_VARDECS(n) ((n)->newvardecs)

#define INFO_MODE(n) ((n)->mode)
#define INFO_NUM_REPLS(n) ((n)->numrepls)
#define INFO_DEF_REPLS(n) ((n)->defrepls)
#define INFO_REPLACE_BY(n) ((n)->replaceby)

#define INFO_IN_PRF_DE(n) ((n)->inprfde)
#define INFO_REPLACE_COUNT(n) ((n)->replacecount)

#define INFO_CURRENT_ASSIGN(n) ((n)->curassign)
#define INFO_NEW_FUNDEFS(n) ((n)->newfundefs)
#define INFO_MARKED_ELEMENT(n) ((n)->markedelement)
#define INFO_IN_BLOCK(n) ((n)->in_block)



/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LUT (result) = NULL;

    INFO_REPLACE_EXPRS (result) = NULL;
    INFO_REPLACE_ASSIGNS (result) = NULL;
    INFO_NEW_ASSIGNS (result) = NULL;
    INFO_NEW_VARDECS (result) = NULL;

    INFO_MODE (result) = mode_undefined;
    INFO_NUM_REPLS (result) = 0;
    INFO_DEF_REPLS (result) = NULL;
    INFO_REPLACE_BY (result) = -1;

    INFO_IN_PRF_DE (result) = 0;
    INFO_REPLACE_COUNT (result) = 0;

    INFO_CURRENT_ASSIGN (result) = NULL;
    INFO_NEW_FUNDEFS (result) = NULL;
    INFO_MARKED_ELEMENT (result) = NULL;

    INFO_IN_BLOCK (result) = 0;

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
 * @fn static node *GetStructDef (ntype *type)
 *
 * @brief Gives a pointer to the struct definition if a type represents
 * a struct.
 *
 ******************************************************************************/
static node *
GetStructDef (ntype *type)
{
    node *orig_td;
    node *sd = NULL;

    DBUG_ENTER ();

    DBUG_EXECUTE_TAG ("DSS_TYPE", {
                      char *tmp_str = TYtype2DebugString (type, FALSE, 0);
                      DBUG_PRINT_TAG ("DSS_TYPE", "Inspecting type %s", tmp_str);
                      tmp_str =  MEMfree (tmp_str); });

    if (TUisArrayOfHidden (type)) {
        orig_td = UTgetTdef (TYgetHiddenUserType (TYgetScalar (type)));
        DBUG_ASSERT (orig_td != NULL, "No typedef found for this user type");
        sd = TYPEDEF_STRUCTDEF (orig_td);
        if (sd == NULL) {
            DBUG_PRINT_TAG ("DSS_TYPE",
                            "    is not a struct type");
        } else {
            DBUG_PRINT_TAG ("DSS_TYPE",
                            "    is a struct type");
        }
    } else {
       DBUG_PRINT_TAG ("DSS_TYPE", "    is not an array of hidden type");
    }

    DBUG_RETURN (sd);
}

/** <!--********************************************************************-->
 * @fn node *ExpandAvis (node *original_avis, node *sdef, ntype *outer,
 *                       char *prefix, node *marked_element, bool mark_all,
 *                       info *arg_info)
 *
 * @brief Recursively expands an avis into a collection of avises that will
 * replace it. Pointers to the replacing avises are put into the lookup table
 * of the info struct.
 *
 * For example, if we have an avis with name "b" and type 'struct body[2]'
 * at address 0xa000, we create two new avises ("b_pos",double[2,3]) and
 * ("b_mass", int[2]) at adresses 0xa100 and 0xa101 respectively.
 *
 * Pointers to these avises are put into the look up table. For this we use the
 * address of the original avis, 0xa000 in the example, as the key.
 * The LUT will then contain the following:
 * 0xa000 -> [0xa100, 0xa101]
 *
 * Whenever we now encounter the original avis, we can retrieve these
 * replacements using the avis's pointer. The first look up of 0xa000 retrieves
 * the pointer to "b_pos", and a second retrieves the pointer to "b_mass"
 *
 *
 * Argument marked_element may point to a struct element, this allows us to mark
 * all avises that we create that together describe this specific element.
 * This is used when traversing setters and getters.
 * For example consider the setter '_struct_set_mass(int e, struct body s)',
 * where we have a pointer to the element 'mass' set as 'markelement' in
 * the info struct.
 * When we then traverse the arguments, we encounter argument 's', and create
 * the avises ("s_pos", double[3]) and ("s_mass", int). The avis for "s_mass"
 * is then marked. This allows us to create an appropriate body.
 *
 ******************************************************************************/
static void
ExpandAvis (node *original_avis, node *sdef, ntype *outer, char *prefix,
            node *marked_element, bool mark_all, info *arg_info)
{
    node *selem;
    ntype *type;
    char *new_name;
    ntype *new_outer;
    node *new_sdef, *val;
    bool found_elem;

    DBUG_ENTER ();

    selem = STRUCTDEF_STRUCTELEM (sdef);
    while (selem != NULL){
        type = STRUCTELEM_TYPE (selem);
        new_name = STRcatn (3, prefix ,"_", STRUCTELEM_NAME (selem));

        new_outer = TYnestTypes (outer, type);

        new_sdef = GetStructDef (type);

        if (new_sdef != NULL) {
            // This field is of a struct type, so we recursively expand

            found_elem = mark_all || marked_element == selem;
            ExpandAvis (original_avis, new_sdef, new_outer, new_name,
                        marked_element, found_elem, arg_info);
            new_name = MEMfree (new_name);
        } else {
            // This field is not a struct type, so we create a replacement avis

            DBUG_ASSERT (INFO_LUT (arg_info) != NULL,
                         "No look up table to insert into");

            DBUG_EXECUTE ({
                char *tyname = TYtype2String (new_outer,
                                              FALSE, 0);
                DBUG_PRINT ("Creating new avis with name '%s' and type '%s'",
                            new_name,
                            tyname);
                tyname = MEMfree (tyname);
            });

            val = TBmakeAvis (new_name, new_outer);

            // keep the original decltype for generating the C code
            AVIS_DECLTYPE (val) = TYcopyType (AVIS_TYPE (original_avis) );

            DBUG_EXECUTE ({
                char *dectyname = TYtype2String (AVIS_DECLTYPE (val),
                                                 FALSE, 0);
                DBUG_PRINT ("    setting its decltype to %s",
                            dectyname);
                dectyname = MEMfree (dectyname);
            });

            if (mark_all || marked_element == selem) {
                AVIS_ISMARKEDELEMENT (val) = TRUE;
                DBUG_PRINT ("    marking avis '%s'", new_name);
            }

            DBUG_PRINT ("    appending %p in LUT for key %p",
                        (void *) val,
                        (void *) original_avis);
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info),
                                                     original_avis,
                                                     val);
        }
        selem = STRUCTELEM_NEXT (selem);
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 * @fn node *ExpandRetType (node* sdef, ntype *outer)
 *
 * @brief Recursively expands an N_structdef to an N_ret chain. Does not use
 * the look up table as that uses N_avis as keys, which we do not have.
 *
 ******************************************************************************/
node *
ExpandRetType (node* sdef, ntype *outer)
{
    node *selem;
    ntype *type;
    ntype *new_outer;
    node *new_sdef;
    node *res = NULL;

    DBUG_ENTER ();

    selem = STRUCTDEF_STRUCTELEM (sdef);
    while (selem != NULL){
        type = STRUCTELEM_TYPE (selem);
        new_outer = TYnestTypes (outer, type);

        new_sdef = GetStructDef (type);

        if (new_sdef != NULL) {
            res = TCappendRet (res, ExpandRetType (new_sdef, new_outer));
        } else {
            res = TCappendRet (res, TBmakeRet (new_outer,NULL));
        }
        selem = STRUCTELEM_NEXT (selem);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * @fn node *ExpandExprType (node* sdef, ntype *outer)
 *
 * @brief Recursively expands an N_structdef to an N_exprs chain of N_type.
 * Does not use the look up table as that uses N_avis as keys, which we
 * do not have.
 *
 ******************************************************************************/
node *
ExpandExprType (node* sdef, ntype *outer)
{
    node *selem;
    ntype *type;
    ntype *new_outer;
    node *new_sdef;
    node *res = NULL;

    DBUG_ENTER ();

    selem = STRUCTDEF_STRUCTELEM (sdef);
    while (selem != NULL){
        type = STRUCTELEM_TYPE (selem);

        new_outer = TYnestTypes (outer, type);

        new_sdef = GetStructDef (type);

        if (new_sdef != NULL) {
            res = TCappendExprs (res, ExpandExprType (new_sdef, new_outer));
        } else {
            res = TCappendExprs (res,
                                 TBmakeExprs (TBmakeType (new_outer), NULL));
        }
        selem = STRUCTELEM_NEXT (selem);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * @fn ntype *ExpandTypeAt (node* sdef, ntype *outer, int at, int *pos)
 *
 * @brief Recursively expands an N_structdef and selects the element at
 * position 'at'. 'pos' is used to keep track of the amount of parts that
 * the definition is split up in to this point.
 *
 ******************************************************************************/
ntype *
ExpandTypeAt (node* sdef, ntype *outer, int at, int *pos)
{
    // oof, ugly, we feed pos in from the outside, should be 0
    node *selem;
    ntype *type;
    ntype *new_outer;
    node *new_sdef;
    ntype *res = NULL;

    DBUG_ENTER ();

    selem = STRUCTDEF_STRUCTELEM (sdef);
    while (selem != NULL && res == NULL){
        type = STRUCTELEM_TYPE (selem);
        if (outer != NULL) {
            new_outer = TYnestTypes (outer, type);
        } else {
            new_outer = type;
        }

        new_sdef = GetStructDef (type);

        if (new_sdef != NULL) {
            res = ExpandTypeAt (new_sdef, new_outer, at, pos);
        } else {
            if ((*pos) == at) {
                res = new_outer;
            }
            (*pos)++;
        }
        selem = STRUCTELEM_NEXT (selem);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn int ExpandedSize (node *sdef)
 *
 * @brief Returns the amount of elements that represent the struct def after
 * recursively expanding.
 *
 ******************************************************************************/
int
ExpandedSize (node* sdef)
{
    node *selem;
    ntype *type;
    node *new_sdef;
    int res = 0;

    DBUG_ENTER ();

    selem = STRUCTDEF_STRUCTELEM (sdef);
    while (selem != NULL){
        type = STRUCTELEM_TYPE (selem);
        new_sdef = GetStructDef (type);

        if (new_sdef != NULL) {
            res += ExpandedSize (new_sdef);
        } else {
            res++;
        }
        selem = STRUCTELEM_NEXT (selem);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static node *MakeConstructor (node *arg_node, info *arg_info)
 *
 * @brief Gives this constructor a body that immediately returns all args.
 *
 * @example Turns
 *   double[3], int _struct_con_body (double[3] _s_pos, int _s_mass);
 * into
 *   double[3], int _struct_con_body (double[3] _s_pos, int _s_mass){
 *     return (_s_pos, _s_mass);
 *   }
 *
 ******************************************************************************/
static node *
MakeConstructor (node *arg_node, info *arg_info)
{
    node *body;
    node *exprs;

    DBUG_ENTER ();

    DBUG_PRINT ("Giving constructor %s a body", FUNDEF_NAME (arg_node));

    exprs = TCcreateExprsFromArgs (FUNDEF_ARGS (arg_node));

    body = TBmakeBlock (TBmakeAssign (TBmakeReturn ( exprs ), NULL), NULL);
    FUNDEF_BODY (arg_node) = body;

    // This declaration was marked extern in HS, but now that it has a body
    // we no longer pretend it is external.
    FUNDEF_ISEXTERN (arg_node) = FALSE;
    FUNDEF_ISINLINE (arg_node) = TRUE;


    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static node *GetterArgs2Exprs (node *arg_node)
 *
 * @brief Create the N_exprs chain that we will add to the N_return.
 * Adds the arg if it is marked to belong to the element this function
 * should get.
 *
 ******************************************************************************/
static node *
GetterArgs2Exprs (node *arg_node)
{
    node *r;

    DBUG_ENTER ();

    if (arg_node == NULL ) {
        r = NULL;
    } else if (AVIS_ISMARKEDELEMENT (ARG_AVIS (arg_node))) {
        r = TBmakeExprs(TBmakeId (ARG_AVIS (arg_node)),
                                  GetterArgs2Exprs (ARG_NEXT (arg_node)));
    } else {
        r = GetterArgs2Exprs( ARG_NEXT (arg_node));
    }

    DBUG_RETURN (r);
}

/** <!--********************************************************************-->
 *
 * @fn static node *MakeGetter (node *arg_node, info *arg_info)
 *
 * @brief Gives this getter a body.
 *
 * @example Turns
 *   int _struct_get_mass (double[3] _s_pos, int _s_mass);
 * into
 *   int _struct_get_mass (double[3] _s_pos, int _s_mass){
 *     return _s_mass;
 *   }
 *
 ******************************************************************************/
static node *
MakeGetter (node* arg_node, info *arg_info)
{
    node *body;
    node *exprs;

    DBUG_ENTER ();

    DBUG_PRINT ("Giving getter %s a body", FUNDEF_NAME (arg_node));

    exprs = GetterArgs2Exprs (FUNDEF_ARGS (arg_node));

    body = TBmakeBlock (TBmakeAssign (TBmakeReturn ( exprs ), NULL), NULL);
    FUNDEF_BODY (arg_node) = body;

    // This declaration was marked extern in HS, but now that it has a body
    // we no longer pretend it is external.
    FUNDEF_ISEXTERN (arg_node) = FALSE;
    FUNDEF_ISINLINE (arg_node) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static node *SetterStructStart (node *setter, info *arg_info)
 *
 * @brief Find out what node is the first argument that belongs to the
 * expanded struct.
 *
 * i.e. for a function set_b_(e_x, e_y, s_a, s_b_x, s_b_y)...
 * we return a pointer to here _________/\
 * where s_b_x and s_b_y are marked
 *
 ******************************************************************************/
static node *
SetterStructStart (node *arg_node)
{
    node *cur;
    node *struct_start;

    DBUG_ENTER ();

    cur = arg_node;
    struct_start = arg_node;

    while (cur != NULL) {
        // skip one arg for each arg that we will be setting
        if (AVIS_ISMARKEDELEMENT (ARG_AVIS (cur))) {
            struct_start = ARG_NEXT (struct_start);
        }

        cur = ARG_NEXT (cur);
    }

    DBUG_RETURN (struct_start);
}

/** <!--********************************************************************-->
 *
 * @fn static node *SetterArgs2Exprs (node *struct_start, node *element_replace)
 *
 * @brief Create an N_exprs chain based on the N_args of this function
 *
 ******************************************************************************/
static node *
SetterArgs2Exprs (node *struct_start, node *element_replace)
{
    node *r;

    DBUG_ENTER ();

    if (struct_start == NULL ) {
        r = NULL;
    } else if (AVIS_ISMARKEDELEMENT (ARG_AVIS (struct_start))) {
        r = TBmakeExprs(TBmakeId (ARG_AVIS (element_replace)),
                        SetterArgs2Exprs( ARG_NEXT (struct_start),
                                          ARG_NEXT (element_replace)));
    } else {
        r = TBmakeExprs(TBmakeId (ARG_AVIS (struct_start)),
                        SetterArgs2Exprs( ARG_NEXT (struct_start),
                                          element_replace));
    }

    DBUG_RETURN (r);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeSetter (node *setter, info *arg_info)
 *
 * @brief Gives this setter a body.
 *
 * @example Turns
 *   double[3], int _struct_set_mass (int e, double[3] _s_pos, int _s_mass);
 * into
 *   double[3], int _struct_set_mass (int e, double[3] _s_pos, int _s_mass){
 *     return (_s_pos, e);
 *   }
 *
 *
 ******************************************************************************/
static node *
MakeSetter (node *arg_node, info *arg_info)
{
    node *body;
    node *exprs;
    node *struct_start;

    DBUG_ENTER ();

    DBUG_PRINT ("Giving setter %s a body", FUNDEF_NAME (arg_node));

    struct_start = SetterStructStart (FUNDEF_ARGS (arg_node));
    exprs = SetterArgs2Exprs (struct_start, FUNDEF_ARGS (arg_node));

    body = TBmakeBlock (TBmakeAssign (TBmakeReturn ( exprs ), NULL), NULL);
    FUNDEF_BODY (arg_node) = body;

    // This declaration was marked extern in HS, but now that it has a body
    // we no longer pretend it is external.
    FUNDEF_ISEXTERN (arg_node) = FALSE;
    FUNDEF_ISINLINE (arg_node) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ReplaceAvis (node *arg_node, info *arg_info)
 *
 * @brief  Finds replacement avis on index given by INFO_REPLACE_BY.
 *
 * @example Assume we have the follwing in the look up table:
 * [ ...,
 *      ((b, struct body),
 *             (b_pos, double[3]),
 *             (b_mass, int)),
 *  ...]
 * If arg_avis is a pointer to avis (b, struct body) with INFO_REPLACE_BY set
 * to 1, then we want to return a pointer to the avis (b_mass, int).
 *
 *
 ******************************************************************************/
static node *
ReplaceAvis (node *arg_avis, info *arg_info)
{
    void ** l_entry;
    int i;
    node *result;

    DBUG_ENTER ();

    l_entry = LUTsearchInLutP (INFO_LUT (arg_info), arg_avis);
    i = 0;


    // We do not want to call LUTsearchInLutNextP if it has already
    // retuned NULL once, as it may not be guaranteed to keep
    // returning NULL.
    // When the result of LUTsearchInLutNextP is NULL,
    // then we have exhausted all replacements anyway.
    while (l_entry != NULL && i != INFO_REPLACE_BY (arg_info)) {
        l_entry = LUTsearchInLutNextP ();
        i++;
    }

    if (l_entry == NULL) {
        result = arg_avis; // don't replace if there is no replacement
    } else {
        result = (node *) *l_entry;
    }

    DBUG_RETURN (result);
}


/** <!--********************************************************************-->
 *
 * @fn node *ExpandArg (node* arg_avis)
 *
 * @brief Builds an N_arg chain out of all replacement avises of a struct.
 *
 ******************************************************************************/
static node *
ExpandArg (node* arg_avis)
{
    node* entry;
    node *prev;
    void **l_entry;

    DBUG_ENTER ();

    prev = NULL;
    l_entry = LUTsearchInLutNextP ();

    if (l_entry != NULL) {
        entry = (node *) *l_entry;
        DBUG_ASSERT (NODE_TYPE (entry) == N_avis,
                     "Retrieved non avis node from LUT");
        prev = ExpandArg (entry);
    }

    DBUG_RETURN (TBmakeArg (arg_avis, prev));
}

/** <!--********************************************************************-->
 *
 * @fn node *ExpandIds (node* arg_avis)
 *
 * @brief Builds an N_Ids chain out of all replacement avises of a struct.
 *
 ******************************************************************************/
static node *
ExpandIds (node* arg_avis)
{
    node *prev = NULL;
    node *res;
    void **l_entry;

    DBUG_ENTER ();

    l_entry = LUTsearchInLutNextP ();

    if (l_entry != NULL) {
        DBUG_PRINT ("                  and: %s", AVIS_NAME ((node *) *l_entry));
        prev = ExpandIds ((node *) *l_entry);
    }

    res = TBmakeIds (arg_avis, prev);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static node *ExpandExprsId (node* arg_avis)
 *
 * @brief Recursively build an N_exprs chain of N_id by repeatedly
 * using LUTsearchInLutNextP until all replacements have been exhausted.
 *
 ******************************************************************************/
static node *
ExpandExprsId (node* arg_avis)
{
    node *prev = NULL;
    node *res;
    void **l_entry;

    DBUG_ENTER ();

    l_entry = LUTsearchInLutNextP ();

    if (l_entry != NULL) {
        DBUG_PRINT ("                 and: %s",
                    AVIS_NAME ((node *) *l_entry));
        prev = ExpandExprsId ((node *) *l_entry);
    }

    res = TBmakeExprs (TBmakeId (arg_avis), prev);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static node *ExpandVardec (node* arg_avis)
 *
 * @brief Recursively build an N_vardec chain of struct elements by repeatedly
 * using LUTsearchInLutNextP until all replacements have been exhausted.
 *
 ******************************************************************************/
static node *
ExpandVardec (node* arg_avis)
{
    node *prev = NULL;
    node *res;
    void **l_entry;

    DBUG_ENTER ();

    l_entry = LUTsearchInLutNextP ();

    if (l_entry != NULL) {
        DBUG_PRINT ("                   and: %s", AVIS_NAME((node *) *l_entry));
        prev = ExpandVardec ((node *) *l_entry);
    }

    DBUG_ASSERT (arg_avis != NULL, "Vardec must have an avis");

    res = TBmakeVardec (arg_avis, prev);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static node *CreateAvisAndInsertVardec (ntype *ty, info *arg_info)
 *
 * @brief Creates a new N_avis and puts an N_vardec into the info struct.
 *
 ******************************************************************************/
static node *
CreateAvisAndInsertVardec (ntype *ty, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis = TBmakeAvis (TRAVtmpVar (), ty);

    DBUG_PRINT("Created avis %s", AVIS_NAME (avis));

    INFO_NEW_VARDECS (arg_info) = TBmakeVardec (avis,
                                                INFO_NEW_VARDECS (arg_info));

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn static node *CreateAssignAndInsert (node *avis, node *value,
 *                                         info *arg_info)
 *
 * @brief Creates a new N_assign and inserts it into the info struct.
 *
 ******************************************************************************/
static void
CreateAssignAndInsert (node *avis, node *value, info *arg_info)
{
    node *assign;

    DBUG_ENTER ();

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                                 value ), NULL);
    AVIS_SSAASSIGN (avis) = assign;
    INFO_NEW_ASSIGNS (arg_info) = TCappendAssign (INFO_NEW_ASSIGNS (arg_info),
                                                  assign);


    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn static node *ExpandedTypeDim (node *arg_node)
 *
 * @brief Returns the dimension of the first field of a structdef.
 * For example, ExpandedTypeDim for struct body is 1 since the first field is
 * of type double[3], which is one dimensional.
 * This works recursively for any nested records, which we can do because fields
 * are required to be of a known size. For example, consider the following:
 *     struct Outer{struct Body[5,6] inner;};
 * For this defintion, ExpandedTypeDim will return 3.
 *
 ******************************************************************************/
int
ExpandedTypeDim(node *arg_node)
{
    node *selem;
    ntype *type;
    node *new_sdef;
    int res = 0;

    DBUG_ENTER ();

    selem = STRUCTDEF_STRUCTELEM (arg_node);
    if (selem != NULL){
        type = STRUCTELEM_TYPE (selem);
        res += TYgetDim (type);

        new_sdef = GetStructDef (type);

        // if first field is a struct, recursively add dim of nested types
        if (new_sdef != NULL) {
            res += ExpandedTypeDim (new_sdef);
        }
    } else {
        // record has no components
        // empty records are not supported as of writing
        CTIerror (NODE_LOCATION (arg_node),
                  "\"struct %s\" must have at least one element, but has none",
                  STRUCTDEF_NAME (arg_node));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static node *ExpandPrfDim (node *arg_node, info *arg_info, node* sdef)
 *
 * @brief Rewrites an application of _dim_A_ on a struct array to
 * correct for nesting.
 *
 * Assuming 's' has a matrix as its first field 'a', then
 * x   = _dim_A_(s);
 *   here turns into
 * in  = 2;
 * out = _dim_A_(s_a);
 * x   = _sub_SxS_(out, in);
 *
 ******************************************************************************/
static node *
ExpandPrfDim(node *arg_node, info *arg_info, node* sdef){
    node *in_avis;
    node *out_avis;
    node *sub_args;
    int elem_dim;
    node *sub;

    DBUG_ENTER ();

    elem_dim = ExpandedTypeDim(sdef);

    in_avis = CreateAvisAndInsertVardec (TYmakeAKS (TYmakeSimpleType (T_int),
                                                    SHmakeShape (0)),
                                         arg_info);

    // the dimension of the first element's expanded type
    CreateAssignAndInsert (in_avis, TBmakeNum (elem_dim), arg_info);


    out_avis = CreateAvisAndInsertVardec (TYmakeAKS (TYmakeSimpleType (T_int),
                                                     SHmakeShape (0)),
                                          arg_info);

    // the dimension of the first element's expanded value
    // takes this rhs ('dim(s_a)')
    CreateAssignAndInsert (out_avis, arg_node, arg_info);


    DBUG_PRINT ("ExpandPrfDim with element dimension %d", elem_dim);


    sub_args = TBmakeExprs (TBmakeId (in_avis), NULL);
    sub_args = TBmakeExprs (TBmakeId (out_avis), sub_args);
    sub = TBmakePrf ( F_sub_SxS,  sub_args);

    DBUG_RETURN (sub);
}

/** <!--********************************************************************-->
 *
 * @fn static node *ExpandPrfShape (node *arg_node, info *arg_info, node* sdef)
 *
 * @brief Rewrites an application of _shape_A_ on a struct array to
 * correct for nesting.
 *
 * x   = _shape_A_(s);
 *   here turns into
 * in  = ty_dim(a); // dim of first field
 * out = _shape_A_(s_a);
 * zero = 0;
 * sub = _sub_SxS_(zero, in);
 * x = _drop_SxS_(sub, out);
 *
 ******************************************************************************/
static node *
ExpandPrfShape(node *arg_node, info *arg_info, node* sdef)
{
    node *in_avis;
    node *out_avis;
    node *zero_avis;
    node *sub_args;
    node *sub;
    node *sub_avis;
    node *drop_args;
    node *drop;
    int elem_dim;

    DBUG_ENTER ();

    elem_dim = ExpandedTypeDim(sdef);

    DBUG_PRINT ("ExpandPrfShape with element dimension %d", elem_dim);


    in_avis = CreateAvisAndInsertVardec (TYmakeAKS (TYmakeSimpleType (T_int),
                                                    SHmakeShape (0)),
                                         arg_info);
    // the dimension of the first element's type
    CreateAssignAndInsert (in_avis, TBmakeNum (elem_dim), arg_info);


    out_avis = CreateAvisAndInsertVardec (TYmakeAKD (TYmakeSimpleType (T_int),
                                                     1,
                                                     SHmakeShape (0)),
                                          arg_info);
    // the dimension of the first element's expanded value
    // takes this rhs ('shape(s_a)')
    CreateAssignAndInsert (out_avis, arg_node, arg_info);

    // zero
    zero_avis = CreateAvisAndInsertVardec (TYmakeAKS (TYmakeSimpleType (T_int),
                                                      SHmakeShape (0)),
                                          arg_info);
    CreateAssignAndInsert (zero_avis, TBmakeNum (0), arg_info);

    // _sub_SxS_(zero, in)
    sub_avis = CreateAvisAndInsertVardec (TYmakeAKS (TYmakeSimpleType (T_int),
                                                     SHmakeShape (0)),
                                          arg_info);
    sub_args = TBmakeExprs (TBmakeId (in_avis), NULL);
    sub_args = TBmakeExprs (TBmakeId (zero_avis), sub_args);
    sub = TBmakePrf ( F_sub_SxS,  sub_args);
    CreateAssignAndInsert ( sub_avis, sub, arg_info);

    // _drop_SxV_(sub, out)
    drop_args = TBmakeExprs (TBmakeId (out_avis), NULL);
    drop_args = TBmakeExprs (TBmakeId (sub_avis), drop_args);
    drop = TBmakePrf ( F_drop_SxV, drop_args );

    DBUG_RETURN (drop);
}

/** <!--********************************************************************-->
 *
 * @fn static node *SelHyperplane (node *prf, node* sdef)
 *
 * @brief Rewrites an application of _sel_VxA_ on a struct array to use sel
 * from the prelude to get a sub array.
 *
 * Consider the following record:
 *     struct record{ int[2] inner; };
 * If we apply _sel_VxA_ on an array of such records as follows:
 *     arr = [record{}, record{}];
 *     res = _sel_VxA_([1], arr);
 * Then we would rewrite this to
 *     ...
 *     res_inner = _sel_VxA_([1], arr_inner);
 * However, _sel_VxA_ can only return a scalar value, where we expect an int[2].
 * So we generate the following instead:
 *     res_inner = sacprelude_d::sel( _flat_4, _arr_a);
 * As this function does allow the selection of subarrays.
 *
 ******************************************************************************/
static node *
SelHyperplane(node *prf, node* sdef, info* arg_info)
{
    node *fn;
    node *sel_res_avis;
    node *tc_args;
    node *res;
    int i = 0;
    ntype *nt;

    DBUG_ENTER ();

    nt = ExpandTypeAt (sdef, NULL, INFO_REPLACE_BY (arg_info), &i);
    DBUG_ASSERT (INFO_MODE (arg_info) == mode_replace,
                 "Replacing _sel_VxA_ by sel is only done in replace mode");

    /* Only insert sacprelude::sel if we select a hyperplane */
    if (TUisScalar (nt)) {
        res = prf;
    } else {
        sel_res_avis = CreateAvisAndInsertVardec (TYcopyType(nt), arg_info);

        DBUG_EXECUTE ({ char *tyname = TYtype2String (nt, FALSE, 0);
                    DBUG_PRINT ("res of sel %s ..", tyname);
                    tyname = MEMfree (tyname); });

        fn = DSdispatchFunCall (NSgetNamespace (global.preludename),
                                "sel",
                                PRF_ARGS (prf));
        DBUG_ASSERT (fn != NULL, "DSdispatchFunCall returned NULL");

        CreateAssignAndInsert (sel_res_avis, fn, arg_info);

        tc_args = TBmakeExprs (TBmakeId (sel_res_avis), NULL);
        tc_args = TBmakeExprs (TBmakeType (TYcopyType (nt)), tc_args);

        res = TBmakePrf ( F_type_conv, tc_args);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static node *CleanVardec (node *arg_node, info *arg_info)
 *
 * @brief Removes old struct vardecs as they have been replaced.
 *
 ******************************************************************************/
static node *
CleanVardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT(arg_node),arg_info);

    if (GetStructDef (AVIS_TYPE (VARDEC_AVIS (arg_node))) != NULL) {
        DBUG_PRINT ("    Removing vardec '%s'", VARDEC_NAME (arg_node));
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static node *CleanArg (node *arg_node, info *arg_info)
 *
 * @brief Removes old struct arguments as they have been replaced.
 *
 ******************************************************************************/
static node *
CleanArg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node),arg_info);

    if (GetStructDef (AVIS_TYPE (ARG_AVIS (arg_node))) != NULL) {
        DBUG_PRINT ("    Removing arg '%s'", ARG_NAME (arg_node));
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static node *CleanTypedef (node *arg_node, info *arg_info)
 *
 * @brief Removes old struct typedefs as all uses of this type have
 * been replaced.
 *
 ******************************************************************************/
static node *
CleanTypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    TYPEDEF_NEXT (arg_node) = TRAVopt (TYPEDEF_NEXT (arg_node), arg_info);

    if (TYPEDEF_STRUCTDEF (arg_node) != NULL) {
        DBUG_PRINT ("    Removing typedef '%s'", TYPEDEF_NAME (arg_node));
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSdoDissolveStructs (node *syntax_tree)
 *
 * @brief Expand all struct values and clean all traces of structs.
 *
 ******************************************************************************/
node *
DSSdoDissolveStructs (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting struct removal.");

    info = MakeInfo ();

    DSinitDeserialize (global.syntax_tree);

    TRAVpush (TR_dss);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DSfinishDeserialize (global.syntax_tree);

    DBUG_PRINT ("Removing old N_structdef");
    anontrav_t atrav[2] = {{N_typedef, &CleanTypedef},
                           {(nodetype) 0, NULL}};

    TRAVpushAnonymous (atrav, &TRAVsons);
    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    DBUG_PRINT ("End of clean traversal");

    info = FreeInfo (info);

    DBUG_PRINT ("End of DSS.");

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSmodule (node *arg_node, info *arg_info)
 *
 * @brief Move fundecs that are transformed into fundefs to the start of
 * the fundef list.
 *
 ******************************************************************************/
node *
DSSmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    // Move the new fundefs out of the info struct to the start of the list
    MODULE_FUNS (arg_node) = TCappendFundef (INFO_NEW_FUNDEFS (arg_info),
                                             MODULE_FUNS (arg_node));
    INFO_NEW_FUNDEFS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSfundef (node *arg_node, info *arg_info)
 *
 * @brief Makes sure that all parts of a N_fundef are visited in the
 * right order. Sets INFO_MARKED_ELEMENT if the function is a getter or setter
 * so that we can identify the arguments that belong to the element that
 * is to be accessed or mutated.
 *
 * Getters and setters are removed from the N_module fundec stack as they are
 * now given a body. We reuse the declaration by moving it to the fundef
 * stack through the info struct. The declaration is reused because at this
 * point all N_aps have their Fundef attribute pointing towards this
 * node (it seems). Therefore it cannot just be deleted.
 *
 ******************************************************************************/

node *
DSSfundef (node *arg_node, info *arg_info)
{
    node *next;
    bool added_body;
    ntype *structtype;

    DBUG_ENTER ();

    DBUG_PRINT ( "-------- visiting fundef \"%s\" --------", FUNDEF_NAME (arg_node));

    INFO_MARKED_ELEMENT (arg_info) = NULL;

    if (FUNDEF_ISSTRUCTGETTER (arg_node)){
        structtype = ARG_NTYPE (TCgetNthArg (0, FUNDEF_ARGS (arg_node)));

        DBUG_ASSERT (GetStructDef (structtype) != NULL,
                     "First argument of getter is expected to be a struct.");

        INFO_MARKED_ELEMENT (arg_info) = TCgetNthStructElem (
            (size_t) FUNDEF_STRUCTPOS (arg_node),
                     STRUCTDEF_STRUCTELEM (GetStructDef (structtype)));

    } else if (FUNDEF_ISSTRUCTSETTER (arg_node)){
        structtype = ARG_NTYPE (TCgetNthArg (1, FUNDEF_ARGS (arg_node)));

        DBUG_ASSERT (GetStructDef (structtype) != NULL,
                     "Second argument of setter is expected to be a struct");
        INFO_MARKED_ELEMENT (arg_info) = TCgetNthStructElem (
            (size_t) FUNDEF_STRUCTPOS (arg_node),
                     STRUCTDEF_STRUCTELEM (GetStructDef (structtype)));
    }


    INFO_LUT (arg_info) = LUTgenerateLut ();

    DBUG_PRINT ("Visiting arguments");
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    DBUG_PRINT ("Done visiting arguments");

    INFO_MARKED_ELEMENT (arg_info) = NULL;


    DBUG_PRINT ("Visiting body");
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    DBUG_PRINT ("Visiting body");


    DBUG_PRINT("Visiting return types");

    FUNDEF_RETS (arg_node) = TRAVopt(FUNDEF_RETS (arg_node), arg_info);

    DBUG_PRINT("Done visiting return types");


    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));



    // We run the cleanup now, so that we can build the constructor's body
    // easier. Removing the old struct argument means that we can simply build
    // an exprs chain from the arg chain.
    // Otherwise, we would have and old argument 's' of a struct type in the
    // copy constructor

    DBUG_PRINT ("Removing old N_arg and N_vardec");
    anontrav_t atrav[3] = {{N_vardec, &CleanVardec},
                           {N_arg, &CleanArg},
                           {(nodetype)0, NULL}};

    TRAVpushAnonymous (atrav, &TRAVsons);
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), NULL);
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), NULL);

    TRAVpop ();

    DBUG_PRINT ("End of fundef clean traversal");


    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);


    /**
     * If this is the declaration of a constructor, getter or setter,
     * we will give it a body, and move the fundef out of this fundec stack,
     * and into the info struct.
     *
     * the functions MakeConstructor, MakeGetter and MakeSetter assume
     * that the arguments of these declarations are expanded, and that the
     * original arguments are cleaned up.
     */

    added_body = FALSE;
    if (!FUNDEF_ISWRAPPERFUN (arg_node)){
        if (FUNDEF_ISSTRUCTCONSTR (arg_node)) {
            arg_node = MakeConstructor(arg_node, arg_info);
            added_body = TRUE;
        } else if (FUNDEF_ISSTRUCTGETTER (arg_node)) {
            arg_node = MakeGetter(arg_node, arg_info);
            added_body = TRUE;
        } else if (FUNDEF_ISSTRUCTSETTER (arg_node)) {
            arg_node = MakeSetter(arg_node, arg_info);
            added_body = TRUE;
        }
    }

    if (added_body) {
        DBUG_PRINT ("Moving %s out of fundec stack", FUNDEF_NAME (arg_node));
        // remove this fundef from this fundec stack and put it in
        // the info struct
        next = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = INFO_NEW_FUNDEFS(arg_info);
        INFO_NEW_FUNDEFS (arg_info) = arg_node;

        arg_node = next;
    }

    DBUG_PRINT ( "-------- leaving fundef --------");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSblock (node *arg_node, info *arg_info)
 *
 * @brief Visit variable declarations and assignments and adds the newly
 * generated variable declarations after expanding.
 *
 * Saves the value INFO_CURRENT_ASSIGN so that we can visit the block
 * in a with-loop and still keep track of what assignment we are in.
 * The current mode is also saved, just to make it clear that the mode does not
 * affect any inner or outer N_block.
 *
 ******************************************************************************/
node *
DSSblock (node *arg_node, info *arg_info)
{
    node *curassign;
    enum traverse_mode cur_mode;

    DBUG_ENTER ();

    if (INFO_IN_BLOCK (arg_info) > 0) {
        DBUG_PRINT ("----- Entering a nested N_block -----");
    }

    curassign = INFO_CURRENT_ASSIGN (arg_info);
    cur_mode = INFO_MODE (arg_info);
    INFO_MODE(arg_info) = mode_undefined;

    DBUG_ASSERT(INFO_NEW_ASSIGNS (arg_info) == NULL,
                "There should not be any assignments to insert");
    DBUG_ASSERT(INFO_REPLACE_ASSIGNS (arg_info) == NULL,
                "There should not be any replacement assignments");
    INFO_CURRENT_ASSIGN(arg_info) = NULL;

    // At this phase we only have vardecs at the top level block
    // so we do not visit or append vardecs in nested blocks


    // We need to fill the look up table using the vardecs first
    DBUG_ASSERT ((INFO_IN_BLOCK (arg_info) == 0)
                  || (BLOCK_VARDECS (arg_node) == NULL),
                 "Found variable declaration in nested block. "
                 "This is unexpected in DSS");
    // Vardecs are only visited in a top level block, otherwise
    // block_vardecs is null
    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);

    INFO_IN_BLOCK (arg_info) += 1;
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    INFO_IN_BLOCK (arg_info) -= 1;

    // if this is a top level block, append the newly created vardecs
    if (INFO_IN_BLOCK (arg_info) == 0) {
        BLOCK_VARDECS (arg_node) = TCappendVardec (
                                       INFO_NEW_VARDECS (arg_info),
                                       BLOCK_VARDECS (arg_node));
        INFO_NEW_VARDECS (arg_info) = NULL;
    }

    DBUG_ASSERT(INFO_NEW_ASSIGNS (arg_info) == NULL,
                "There should not be any assignments to insert");
    DBUG_ASSERT(INFO_REPLACE_ASSIGNS (arg_info) == NULL,
                "There should not be any replacement assignments");
    INFO_CURRENT_ASSIGN (arg_info) = curassign;
    INFO_MODE (arg_info) = cur_mode;

    if (INFO_IN_BLOCK (arg_info) > 0) {
        DBUG_PRINT ("-------- Leaving nested N_block --------");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSvardec (node *arg_node, info *arg_info)
 *
 * @brief Visits the avis and expands it in place by building a new
 * N_vardec chain if it is a struct type.
 *
 ******************************************************************************/
node *
DSSvardec (node *arg_node, info *arg_info)
{
    node *new_vardecs;
    void **l_entry;

    DBUG_ENTER ();

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    DBUG_ASSERT (VARDEC_AVIS (arg_node) != NULL, "Vardec must have an avis");
    VARDEC_AVIS (arg_node) = TRAVdo (VARDEC_AVIS (arg_node), arg_info);

    l_entry = LUTsearchInLutP (INFO_LUT (arg_info), VARDEC_AVIS (arg_node));

    if (l_entry != NULL) {
        DBUG_PRINT ("creating replacement vardecs for '%s'", VARDEC_NAME (arg_node));
        DBUG_ASSERT (l_entry != NULL, "Replacement avis must not be NULL");
        DBUG_PRINT ("                 found: %s", AVIS_NAME((node *) *l_entry));
        new_vardecs = ExpandVardec ((node *) *l_entry);

        VARDEC_NEXT (arg_node) = TCappendVardec (new_vardecs,
                                                 VARDEC_NEXT (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSassign (node *arg_node, info *arg_info)
 *
 * @brief Visit the statement and add any new or replacement assignments.
 *
 ******************************************************************************/
node *
DSSassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_PRINT ("Entering DSSassign.");

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_undefined,
                 "Mode should not be set when entering an N_assign");

    DBUG_ASSERT (INFO_CURRENT_ASSIGN (arg_info) == NULL,
                 "Expected current assign to be unset");
    INFO_CURRENT_ASSIGN (arg_info) = arg_node;

    DBUG_ASSERT (INFO_REPLACE_ASSIGNS (arg_info) == NULL,
                 "no replacement assignments expected");

    // we traverse the statement with in-place expansion as default mode
    INFO_MODE (arg_info) = mode_in_place;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_REPLACE_ASSIGNS (arg_info) != NULL) {
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TCappendAssign (INFO_REPLACE_ASSIGNS (arg_info), arg_node);
        INFO_REPLACE_ASSIGNS (arg_info) = NULL;
    }

    if (INFO_NEW_ASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_NEW_ASSIGNS (arg_info), arg_node);
        INFO_NEW_ASSIGNS (arg_info) = NULL;
    }

    DBUG_PRINT ("Leaving DSSassign");
    INFO_MODE (arg_info) = mode_undefined;
    INFO_CURRENT_ASSIGN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSlet (node *arg_node, info *arg_info)
 *
 * @brief Visit the expression, and see what mode we are now in.
 *
 * In mode_repl_count we duplicate the assignment for each of the elements and
 * replace the occurance of a struct avis by that element with mode_replace.
 * Otherwise we only visit the left hand side to expand the N_ids.
 *
 ******************************************************************************/

node *
DSSlet (node *arg_node, info *arg_info)
{
    int i;
    node *new_node;
    node *new_assign;
    node *old_assign;

    DBUG_ENTER ();

    DBUG_PRINT ("Entering DSSlet");

    DBUG_ASSERT (INFO_MODE (arg_info) != mode_repl_count,
                 "We should not be able to be in replication count mode"
                 " when entering DSSlet. It can only be mode_in_place"
                 " after we entered an N_assign, or mode_replace after"
                 " replicating");

    // By default we want to replicate every N_let, though this mode can
    // be overriden in a child node, such as by N_ap.
    if (INFO_MODE (arg_info) == mode_in_place) {
        INFO_MODE (arg_info) = mode_repl_count;
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    switch (INFO_MODE (arg_info)) {
    case mode_undefined:
        DBUG_UNREACHABLE ("One of mode_in_place, mode_repl_count or "
                          "mode_replace must be set in DSSlet");
        break;
    case mode_repl_count:
        if (INFO_NUM_REPLS (arg_info) > 0) {
            DBUG_PRINT ("mode was set to mode_repl_count,"
                        " so we duplicate this N_let, and replace");
            INFO_MODE (arg_info) = mode_replace;
            for (i = 0; i < INFO_NUM_REPLS (arg_info); i++) {

                // we need to update AVIS_SSAASSIGN later
                new_node = DUPdoDupNode (arg_node);

                DBUG_PRINT ("replacing with index %d", i);
                INFO_REPLACE_BY (arg_info) = i;

                new_assign = TBmakeAssign (NULL, NULL);
                old_assign = INFO_CURRENT_ASSIGN (arg_info);
                INFO_CURRENT_ASSIGN (arg_info) = new_assign;

                new_node = TRAVdo (new_node, arg_info);
                ASSIGN_STMT (new_assign) = new_node;

                INFO_CURRENT_ASSIGN (arg_info) = old_assign;

                INFO_REPLACE_ASSIGNS (arg_info) = TCappendAssign (
                                        INFO_REPLACE_ASSIGNS (arg_info),
                                        new_assign);
            }
            INFO_MODE (arg_info) = mode_repl_count;
            INFO_NUM_REPLS (arg_info) = 0;
            INFO_DEF_REPLS (arg_info) = NULL;
            INFO_REPLACE_BY (arg_info) = -1;
        } else {
            DBUG_PRINT ("num repls is 0 in mode_repl_count,"
                        " we do not need to do anything here");
        }
        break;
    case mode_in_place:
    case mode_replace:
        // if we are in mode_in_place, we expanded in place on the right hand
        // side. We now continue in mode_in_place to expand the left hand side.

        // if we just replicated the N_let we need to visit the lhs in
        // mode_replace to substitute the ids of this copy by using the index
        // of INFO_REPLACE_BY

        // Either way, we are in the proper mode
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSreturn (node *arg_node, info *arg_info)
 *
 * @brief Expand the exprs in place.
 *
 ******************************************************************************/
node *
DSSreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_MODE (arg_info) = mode_in_place,
                 "Only mode_in_place is expected in DSSreturn as we want to"
                 " expand the exprs in-place");
    DBUG_PRINT ("In-place expansion in N_return");

    RETURN_EXPRS (arg_node) = TRAVopt (RETURN_EXPRS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSexprs (node *arg_node, info *arg_info)
 *
 * @brief Visit the expression and replace the current exprs node if
 * INFO_REPLACE_EXPRS is set.
 *
 ******************************************************************************/
node *
DSSexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    EXPRS_NEXT (arg_node) = TRAVopt (EXPRS_NEXT (arg_node), arg_info);

    EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);

    if (INFO_REPLACE_EXPRS (arg_info) != NULL) {
        DBUG_PRINT ("Replacing this exprs node");
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TCappendExprs (INFO_REPLACE_EXPRS (arg_info), arg_node);
        INFO_REPLACE_EXPRS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSwith (node *arg_node, info *arg_info)
 *
 * @brief Visit all parts of the withloop with the proper mode set.
 *
 ******************************************************************************/
node *
DSSwith (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();
    DBUG_PRINT ("Entering with-loop");

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_repl_count,
                 "Expected mode to be repl_count when entering N_with");

    // No structs should be encountered in the partition, so we
    // set mode_undefined.
    INFO_MODE (arg_info) = mode_undefined;

    DBUG_PRINT ("visiting with-loop partition");
    WITH_PART (arg_node) = TRAVdo  (WITH_PART (arg_node) , arg_info);

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_undefined,
                 "Mode should not be changed after visiting N_part");

    DBUG_ASSERT(INFO_NEW_ASSIGNS (arg_info) == NULL,
                "Expected newassigns to be empty");

    DBUG_PRINT ("visiting with-loop's N_code");
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node) , arg_info);

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_undefined,
                 "Mode should not be changed after visiting N_code");

    DBUG_PRINT ("visiting op..");
    WITH_WITHOP (arg_node) = TRAVdo  (WITH_WITHOP (arg_node), arg_info);

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_undefined,
                "We should not be in any mode after visiting with-loop"
                "operations");

    // we set mode_in_place, this signals that we want to do in-place expansion
    // of the left hand side in an enclosing N_let
    INFO_MODE (arg_info) = mode_in_place;

    DBUG_PRINT ("leaving withloop");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSScode (node *arg_node, info *arg_info)
 *
 * @brief Visits the nested block, and expand the exprs list
 *
 ******************************************************************************/
node *
DSScode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (INFO_MODE (arg_info) == mode_undefined,
                 "Mode should be undefined as we unset it in DSSwith");

    CODE_CBLOCK (arg_node) = TRAVopt  (CODE_CBLOCK (arg_node), arg_info);

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_undefined,
                 "Mode should be unchanged after returning from an N_block");

    INFO_MODE (arg_info) = mode_in_place; // expand the exprs
    CODE_CEXPRS (arg_node) = TRAVdo  (CODE_CEXPRS (arg_node), arg_info);
    // we unset the mode again so that we do not accidentally leave it
    // as mode_in_place when visiting the next N_code, or when returning
    // to DSSwith
    INFO_MODE (arg_info) = mode_undefined;

    CODE_NEXT (arg_node) = TRAVopt  (CODE_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSmodarray (node *arg_node, info *arg_info)
 *
 * @brief Replicates this modarray operation if applied to a struct array. In
 * each replication, the id is replaced by that for one specific fully expnded
 * field.
 * For example:
 *     modarray(bodies)
 * is transformed to
 *     modarray(bodies_pos), modarray(bodies_mass)
 *
 * And when nesting with e.g. struct outer{int foo, struct body[2] bodies;}:
 *     modarray(outers)
 * turns into
 *     modarray(outers_foo), modarray(outers_bodies_pos), ...
 *
 ******************************************************************************/
node *
DSSmodarray (node *arg_node, info *arg_info)
{
    node *new_node = NULL;
    node *next;

    int i;

    DBUG_ENTER ();

    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_undefined,
                 "with-loop operations should be entered in mode_undefined");

    INFO_MODE (arg_info) = mode_repl_count;

    DBUG_PRINT ("in modarray");

    // see if we need to replicate this modarray
    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_repl_count,
                 "modarray should be in mode repl_count even after"
                 " visiting modarray_array");

    if (INFO_NUM_REPLS (arg_info) > 0) {
        DBUG_PRINT ("expanding '%s' in modarray",
                    ID_NAME (MODARRAY_ARRAY (arg_node)));

        INFO_MODE (arg_info) = mode_replace;
        next = MODARRAY_NEXT (arg_node);

        // we iterate backwards, so that we end up with the chain of modarray
        // in the proper order.
        for (i = INFO_NUM_REPLS (arg_info) - 1; i >= 0; i--) {
            DBUG_PRINT ("replacing with index %d", i);

            // duplicate the original modarray
            new_node = DUPdoDupNode (arg_node);

            // substitute the id with that of the field at position 'i'
            INFO_REPLACE_BY (arg_info) = i;
            MODARRAY_ARRAY (new_node) = TRAVdo (MODARRAY_ARRAY (new_node),
                                                arg_info);

            // point to the next modarray (that is, i+1 or the orignal's next)
            MODARRAY_NEXT (new_node) = next;
            next = new_node;
        }

        DBUG_PRINT ("done with duplicating and replacing");

        // We free the original node, 'next' will take its place as it belongs
        // to the first field of the record that we found in MODARRAY_ARRAY
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = next;

        INFO_NUM_REPLS (arg_info) = 0;
        INFO_DEF_REPLS (arg_info) = NULL;
        INFO_REPLACE_BY (arg_info) = -1;
    }

    INFO_MODE (arg_info) = mode_undefined;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSgenarray (node *arg_node, info *arg_info)
 *
 * @brief Replicates this genarray operation if applied to a struct. In
 * each replication, the id of the default is replaced by that for one specific
 * fully expnded field.
 * For example:
 *     genarray([2,3], mars)
 * is transformed to
 *     genarray([2,3], mars_pos), genarray([2,3], mars_mass)
 *
 * And when nesting with e.g. struct outer{int foo, struct body[2] bodies;}:
 *     genarray([2,3], outer)
 * turns into
 *     genarray([2,3], outer_foo), genarray([2,3], outer_bodies_pos), ...
 *
 ******************************************************************************/
node *
DSSgenarray (node *arg_node, info *arg_info)
{
    node *new_node = NULL;
    node *next;

    int i;

    DBUG_ENTER ();

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_undefined,
                 "with-loop operations should be entered in mode_undefined");
    INFO_MODE (arg_info) = mode_repl_count;

    DBUG_PRINT ("in genarray");

    // see if we need to replicate this genarray by based on its default value
    GENARRAY_DEFAULT (arg_node) = TRAVopt (GENARRAY_DEFAULT (arg_node),
                                           arg_info);

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_repl_count,
                 "genarray should be in mode repl_count even after"
                 " visiting genarray_default");

    if (INFO_NUM_REPLS (arg_info) > 0) {
        DBUG_PRINT ("expanding '%s' in genarray",
                    ID_NAME (GENARRAY_DEFAULT (arg_node)));

        INFO_MODE (arg_info) = mode_replace;
        next = GENARRAY_NEXT (arg_node);

        // we iterate backwards, so that we end up with the chain of genarray
        // in the proper order.
        for (i = INFO_NUM_REPLS (arg_info) - 1; i >= 0; i--) {
            DBUG_PRINT ("replacing with index %d", i);

            // duplicate the original genarray
            new_node = DUPdoDupNode (arg_node);

            // substitute the default with that of the field at position 'i'
            INFO_REPLACE_BY (arg_info) = i;
            GENARRAY_DEFAULT (new_node) = TRAVdo (GENARRAY_DEFAULT (new_node),
                                                  arg_info);

            // point to the next genarray (that is, i+1 or the orignal's next)
            GENARRAY_NEXT (new_node) = next;
            next = new_node;
        }

        DBUG_PRINT ("done with duplicating and replacing");

        // We free the original node, 'next' will take its place as it belongs
        // to the first field of the record that we found in GENARRAY_DEFAULT
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = next;

        INFO_NUM_REPLS (arg_info) = 0;
        INFO_DEF_REPLS (arg_info) = NULL;
        INFO_REPLACE_BY (arg_info) = -1;
    }

    INFO_MODE (arg_info) = mode_undefined;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSfold (node *arg_node, info *arg_info)
 *
 * @brief Replicates this fold operation if the neutral element is of a struct
 * type. In each replication, the id is replaced by that for one specific fully
 * expnded field. The folding function is kept the same, each copy points to
 * the same original fundef.
 * For example:
 *     fold(add_bodies, mars)
 * is transformed to
 *     fold(add_bodies, mars_pos), fold(add_bodies, mars_mass)
 *
 * And when nesting with e.g. struct outer{int foo, struct body[2] bodies;}:
 *     fold(add_outers, outer)
 * turns into
 *     fold(add_outers, outer_foo), fold(add_outers, outer_bodies_pos), ...
 *
 ******************************************************************************/
node *
DSSfold (node *arg_node, info *arg_info)
{
    node *new_node = NULL;
    node *next;

    int i;

    DBUG_ENTER ();

    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_undefined,
                 "with-loop operations should be entered in mode_undefined");

    INFO_MODE (arg_info) = mode_repl_count;

    DBUG_PRINT ("in fold");

    DBUG_ASSERT (FOLD_GUARD (arg_node) == NULL,
                 "Did not expect guard to be set in DSS, but it was set to"
                 " '%s' should this be possible?",
                 ID_NAME (FOLD_GUARD (arg_node)));

    // see if we need to replicate this fold by based on its neutral value
    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    DBUG_ASSERT (INFO_MODE (arg_info) == mode_repl_count,
                 "fold should be in mode repl_count even after"
                 " visiting fold_neutral");

    if (INFO_NUM_REPLS (arg_info) > 0) {
        DBUG_PRINT ("expanding '%s' in fold",
                    ID_NAME (FOLD_NEUTRAL (arg_node)));

        INFO_MODE (arg_info) = mode_replace;
        next = FOLD_NEXT (arg_node);

        // we iterate backwards, so that we end up with the chain of fold
        // operations in the proper order.
        for (i = INFO_NUM_REPLS (arg_info) - 1; i >= 0; i--) {
            DBUG_PRINT ("replacing with index %d", i);

            // duplicate the original fold
            new_node = DUPdoDupNode (arg_node);

            // substitute the id with that of the field at position 'i'
            INFO_REPLACE_BY (arg_info) = i;
            FOLD_NEUTRAL (new_node) = TRAVdo (FOLD_NEUTRAL (new_node),
                                              arg_info);

            // point to the next fold (that is, i+1 or the orignal's next)
            FOLD_NEXT (new_node) = next;
            next = new_node;
        }

        DBUG_PRINT ("done with duplicating and replacing");

        // We free the original node, 'next' will take its place as it belongs
        // to the first field of the record that we found in FOLD_NEUTRAL
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = next;

        INFO_NUM_REPLS (arg_info) = 0;
        INFO_DEF_REPLS (arg_info) = NULL;
        INFO_REPLACE_BY (arg_info) = -1;
    }

    INFO_MODE (arg_info) = mode_undefined;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSarray (node *arg_node, info *arg_info)
 *
 * @brief Set the proper elem type in mode_replace. Set INFO_NUM_REPLS for
 * an array of struct when in mode_repl_count.
 *
 ******************************************************************************/
node *
DSSarray (node *arg_node, info *arg_info)
{
    node *sdef;
    int pos;
    int expandedsize;

    DBUG_ENTER ();

    ARRAY_AELEMS (arg_node) = TRAVopt (ARRAY_AELEMS (arg_node), arg_info);

    switch (INFO_MODE (arg_info)) {
    case mode_undefined:
        DBUG_UNREACHABLE ("One of mode_replace or mode_repl_count must"
                          " be set in DSSarray");
        break;
    case mode_replace:
        sdef = GetStructDef (ARRAY_ELEMTYPE (arg_node));
        if (sdef != NULL) {
            DBUG_PRINT ("Replacing N_id in DSSarray");
            pos = 0;
            ARRAY_ELEMTYPE (arg_node) =
                ExpandTypeAt (sdef,
                    ARRAY_ELEMTYPE (arg_node),
                    INFO_REPLACE_BY (arg_info),
                    &pos);
        }
        break;
    case mode_repl_count:
        sdef = GetStructDef (ARRAY_ELEMTYPE (arg_node));

        // we need to make sure that we set num_repls based on the element type
        // for when the array is empty. In that case we would not have an id
        // that sets num_repls!
        if (sdef != NULL){
            expandedsize = ExpandedSize (sdef);
            DBUG_PRINT ("rhs type, set num_repls to %d", expandedsize);
            INFO_NUM_REPLS (arg_info) = expandedsize;

            if (INFO_DEF_REPLS (arg_info) != NULL) {
                DBUG_ASSERT (INFO_DEF_REPLS (arg_info) == sdef,
                             "Attempt to replicate and replace with two"
                             " distinct struct types '%s' and '%s'",
                             STRUCTDEF_NAME (sdef),
                             STRUCTDEF_NAME (INFO_DEF_REPLS (arg_info)));
            }
            INFO_DEF_REPLS (arg_info) = sdef;
        }
        break;
    case mode_in_place:
        DBUG_UNREACHABLE ("DSS mode_in_place is unexpected in N_array");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSap (node *arg_node, info *arg_info)
 *
 * @brief Sets mode_in_place instead of mode_repl_count so that we expand
 * arguments of user defined functions in place
 *
 ******************************************************************************/
node *
DSSap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_MODE (arg_info) == mode_repl_count) {
        DBUG_PRINT ("In-place expanding in N_ap");
        INFO_MODE (arg_info) = mode_in_place;
    }
    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSprf (node *arg_node, info *arg_info)
 *
 * @brief Rewrites some predefined functions when applied to a struct array.
 *
 ******************************************************************************/
node *
DSSprf (node *arg_node, info *arg_info)
{
    node *sdef;

    DBUG_ENTER ();

    DBUG_ASSERT (INFO_MODE (arg_info) != mode_in_place,
                 "DSSprf should only be entered in mode_replace"
                 " or mode_repl_count");

    DBUG_PRINT ("found application of %s", PRF_NAME (PRF_PRF (arg_node)));

    switch (PRF_PRF (arg_node)) {
    case F_dispatch_error:
        INFO_IN_PRF_DE (arg_info) = 1;

        if (INFO_MODE (arg_info) == mode_repl_count) {
            INFO_MODE (arg_info) = mode_in_place;

            INFO_REPLACE_COUNT (arg_info) = 0;
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
            NUM_VAL (PRF_ARG1 (arg_node)) += INFO_REPLACE_COUNT (arg_info);
            INFO_REPLACE_COUNT (arg_info) = 0;
        } else {
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }

        INFO_IN_PRF_DE (arg_info) = 0;
        break;
    case F_type_fix:
    case F_type_conv:
        // Continue mode_repl_count
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        break;
    case F_dim_A:
    case F_shape_A:
        sdef = GetStructDef (ID_NTYPE (PRF_ARG1 (arg_node)));

        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        if (INFO_MODE (arg_info) == mode_repl_count
            && INFO_NUM_REPLS (arg_info) > 0) {
            // We only want the first element from this function
            DBUG_PRINT ("set num_repls to 1 for _shape_A_");
            DBUG_ASSERT (INFO_DEF_REPLS (arg_info) == sdef,
                         "Attempt to replicate and replace with two"
                         " distinct struct types '%s' and '%s'",
                         STRUCTDEF_NAME (sdef),
                         STRUCTDEF_NAME (INFO_DEF_REPLS (arg_info)));
            INFO_NUM_REPLS (arg_info) = 1;
        }

        if (INFO_MODE (arg_info) == mode_replace && sdef != NULL) {
            switch (PRF_PRF (arg_node)) {
                case F_dim_A:
                    arg_node = ExpandPrfDim (arg_node, arg_info, sdef);
                    break;
                case F_shape_A:
                    arg_node = ExpandPrfShape (arg_node, arg_info, sdef);
                    break;
                default:
                    break;
            }
        }
        break;

    /**
     * Replace application of this primitive function by the generated `zero`.
     */
    case F_zero_A:
        sdef = GetStructDef (ID_NTYPE (PRF_ARG1 (arg_node)));

        INFO_MODE (arg_info) = mode_in_place;
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        if (sdef != NULL) {
            node *new_node;

            DBUG_ASSERT (STRUCTDEF_ZEROFUNCTION (sdef) != NULL,
                         "struct has no zero function");
            DBUG_ASSERT (NODE_TYPE (STRUCTDEF_ZEROFUNCTION (sdef)) == N_fundef,
                         "expected N_fundef; got %s",
                         NODE_TEXT (STRUCTDEF_ZEROFUNCTION (sdef)));
            DBUG_PRINT ("replacing F_zero_A with generated zero function");

            new_node = TBmakeAp (STRUCTDEF_ZEROFUNCTION (sdef),
                                 PRF_ARGS (arg_node));

            PRF_ARGS (arg_node) = NULL;
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = new_node;
        }
        break;

    case F_sel_VxA:
        sdef = GetStructDef( ID_NTYPE (PRF_ARG2 (arg_node)));

        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        if (INFO_MODE (arg_info) == mode_replace && sdef != NULL) {
            /**
             * We might need to replace this _sel_VxA_(idx, element) with
             * prelude::sel(idx, element). We do that if the element is not a
             * scalar value, and we would thus need to select a subarray rather
             * than a scalar value.
             */
            DBUG_PRINT ("expanded sdef sel");
            arg_node = SelHyperplane (arg_node, sdef, arg_info);

        }
        break;
    case F_accu:
        // continue in mode_in_place
        DBUG_ASSERT (INFO_MODE (arg_info) != mode_replace,
                     "_accu_ should only be expanded in-place,"
                     " so mode_replace is unexpected");
        INFO_MODE (arg_info) = mode_in_place;
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        break;
    default:
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        DBUG_ASSERT (INFO_NUM_REPLS (arg_info) == 0,
                     "Attempt to expand a record in an unsupported"
                     " built-in function: %s",
                     PRF_NAME (PRF_PRF (arg_node)));
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSarg (node *arg_node, info *arg_info)
 *
 * @brief If this argument value is a struct, expand it to its elements.
 *
 ******************************************************************************/
node *
DSSarg (node *arg_node, info *arg_info)
{
    node *new_args;
    node* entry;
    void** l_entry = NULL;

    DBUG_ENTER ();

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    ARG_AVIS (arg_node) = TRAVdo (ARG_AVIS (arg_node), arg_info);

    DBUG_ASSERT (INFO_LUT (arg_info) != NULL, "No look up table!");
    l_entry = LUTsearchInLutP (INFO_LUT (arg_info), ARG_AVIS (arg_node));

    if (l_entry != NULL) {
        DBUG_ASSERT (ARG_AVIS (arg_node) != NULL,
                     "Avis unexpectedly NULL");
        DBUG_ASSERT (AVIS_TYPE (ARG_AVIS (arg_node)) != NULL,
                     "Avis' type unexpectedly NULL");
        entry = (node *) *l_entry;

        DBUG_ASSERT (NODE_TYPE (entry) == N_avis, "Retrieved non avis node!");
        new_args = ExpandArg (entry);


        DBUG_ASSERT (arg_node != NULL, "arg_node should not be NULL");

        ARG_NEXT (arg_node) = TCappendArgs (new_args, ARG_NEXT (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSret (node *arg_node, info *arg_info)
 *
 * @brief If this return type is a struct, expand it to its elements.
 *
 ******************************************************************************/
node *
DSSret (node *arg_node, info *arg_info)
{
    node *structdef;
    node *expanded;

    DBUG_ENTER ();

    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);

    structdef = GetStructDef (RET_TYPE (arg_node));
    if (structdef != NULL) {
        DBUG_PRINT ("Expanding N_ret");
        expanded = ExpandRetType (structdef, RET_TYPE (arg_node));
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TCappendRet (expanded, arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSStype (node *arg_node, info *arg_info)
 *
 * @brief Keeps track of the amount of types that we created by expanding
 * when in mode_in_place. This is to properly set the first argument
 * for dispatch error later on.
 * In mode_replace we replace the type with that of the selected element
 *
 ******************************************************************************/
node *
DSStype (node *arg_node, info *arg_info)
{
    node *sdef;
    node *newexprs;
    ntype *expanded;
    int pos;

    DBUG_ENTER ();

    if (INFO_IN_PRF_DE (arg_info) == 1
        && INFO_MODE (arg_info) == mode_in_place) {
        sdef = GetStructDef (TYPE_TYPE (arg_node));
        if (sdef != NULL) {
            DBUG_PRINT ("found type to expand");
            newexprs = ExpandExprType (sdef, TYPE_TYPE (arg_node));
            INFO_REPLACE_COUNT (arg_info) +=
                ((int) TCcountExprs (newexprs)) - 1;
            INFO_REPLACE_EXPRS (arg_info) = newexprs;
        }
    } else if (INFO_MODE (arg_info) == mode_replace) {
        DBUG_EXECUTE ({ char *tyname = TYtype2String (TYPE_TYPE (arg_node),
                                                      FALSE, 0);
                        DBUG_PRINT ("looking at type %s", tyname);
                        tyname = MEMfree (tyname); });

        sdef = GetStructDef (TYPE_TYPE (arg_node));
        if (sdef != NULL) {
            pos = 0;

            expanded = ExpandTypeAt (sdef,
                                     TYPE_TYPE (arg_node),
                                     INFO_REPLACE_BY (arg_info),
                                     &pos);

            DBUG_EXECUTE ({ char *orig = TYtype2String (TYPE_TYPE (arg_node),
                                                        FALSE, 0);
                            char *repl = TYtype2String (expanded, FALSE, 0);
                            DBUG_PRINT ("replacing type %s with %s",
                                        orig, repl);
                            orig = MEMfree (orig);
                            repl = MEMfree (repl); });

            TYPE_TYPE (arg_node) = expanded;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSids (node *arg_node, info *arg_info)
 *
 * @brief In mode_in_place struct types are expanded in place.
 * In mode_repl_count INFO_NUM_REPLS is set to the amount of elements after
 * expansion.
 * In mode_replace the avis is replaced by the element at INFO_REPLACE_BY
 *
 ******************************************************************************/
node *
DSSids (node *arg_node, info *arg_info)
{
    void **l_entry;
    node *sdef;
    node *replacement;
    int expandedsize;

    DBUG_ENTER ();

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    switch (INFO_MODE (arg_info)){
    case mode_in_place:
        DBUG_PRINT ("Entering DSSids '%s' in mode_in_place",
                    IDS_NAME (arg_node));
        l_entry = LUTsearchInLutP (INFO_LUT (arg_info), IDS_AVIS (arg_node));
        if (l_entry != NULL) {
            DBUG_PRINT ("found replacement ids: %s",
                        AVIS_NAME ((node *) *l_entry));

            arg_node = TCappendIds (ExpandIds ((node *) *l_entry),
                                    FREEdoFreeNode (arg_node));
        }
        break;
    case mode_repl_count:
        DBUG_PRINT ("Entering DSSids '%s' in mode_repl_count",
                    IDS_NAME (arg_node));
        sdef = GetStructDef (IDS_NTYPE (arg_node) );
        if (sdef != NULL){
            expandedsize = ExpandedSize (sdef);
            DBUG_PRINT ("set num_repls to %d", expandedsize);
            INFO_NUM_REPLS (arg_info) = expandedsize;

            if (INFO_DEF_REPLS (arg_info) != NULL) {
                DBUG_ASSERT (INFO_DEF_REPLS (arg_info) == sdef,
                             "Attempt to replicate and replace with two"
                             " distinct struct types '%s' and '%s'",
                             STRUCTDEF_NAME (sdef),
                             STRUCTDEF_NAME (INFO_DEF_REPLS (arg_info)));
            }
            INFO_DEF_REPLS (arg_info) = sdef;
        }
        break;
    case mode_replace:
        DBUG_PRINT ("Entering DSSids '%s' in mode_replace",
                    IDS_NAME (arg_node));
        replacement = ReplaceAvis (IDS_AVIS (arg_node), arg_info);
        DBUG_ASSERT (replacement != NULL,
                     "replacement avis should not be NULL");

        // we should make sure that this avis' ssaassign attribute points
        // to the current assignment!

        AVIS_SSAASSIGN (replacement) = INFO_CURRENT_ASSIGN (arg_info);

        IDS_AVIS (arg_node) = replacement;
        break;
    case mode_undefined:
        DBUG_PRINT ("Entering DSSids '%s' in mode_undefined",
                    IDS_NAME (arg_node));
        // if no mode is set, we do not need to rewrite anything
        // we should make sure that this ids is not of a record type though
        sdef = GetStructDef (IDS_NTYPE (arg_node));
        DBUG_ASSERT (sdef == NULL,
                     "DSS is in mode_undefined, but we encountered %s of type"
                      " struct %s",
                      IDS_NAME (arg_node),
                      STRUCTDEF_NAME (sdef));
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSid (node *arg_node, info *arg_info)
 *
 * @brief In mode_in_place struct types are expanded in place.
 * In mode_repl_count INFO_NUM_REPLS is set to the amount of elements after
 * expansion.
 * In mode_replace the avis is replaced by the element at INFO_REPLACE_BY
 *
 ******************************************************************************/
node *
DSSid (node *arg_node, info *arg_info)
{
    void **l_entry;
    node *sdef;
    int expandedsize;
    node *replacement;

    DBUG_ENTER ();


    switch (INFO_MODE (arg_info)) {
    case mode_in_place:
        DBUG_PRINT ("Entering DSSid '%s' in mode_in_place",
                    ID_NAME (arg_node));
        l_entry = LUTsearchInLutP (INFO_LUT (arg_info), ID_AVIS (arg_node));
        if (l_entry != NULL) {
            DBUG_PRINT ("found replacement id: %s",
                        AVIS_NAME ((node *) *l_entry));
            INFO_REPLACE_EXPRS (arg_info) = ExpandExprsId ((node *) *l_entry);
        }
        break;
    case mode_repl_count:
        DBUG_PRINT ("Entering DSSid '%s' in mode_repl_count",
                    ID_NAME (arg_node));
        sdef = GetStructDef (ID_NTYPE (arg_node));
        if (sdef != NULL){
            expandedsize = ExpandedSize (sdef);
            DBUG_PRINT ("set num_repls to %d", expandedsize);
            INFO_NUM_REPLS (arg_info) = expandedsize;

            if (INFO_DEF_REPLS (arg_info) != NULL) {
                DBUG_ASSERT (INFO_DEF_REPLS (arg_info) == sdef,
                             "Attempt to replicate and replace with two"
                             " distinct struct types '%s' and '%s'",
                             STRUCTDEF_NAME (sdef),
                             STRUCTDEF_NAME (INFO_DEF_REPLS (arg_info)));
            }
            INFO_DEF_REPLS (arg_info) = sdef;
        }
        break;
    case mode_replace:
        DBUG_PRINT ("Entering DSSid '%s' in mode_replace",
                    ID_NAME (arg_node));
        replacement = ReplaceAvis (ID_AVIS (arg_node), arg_info);
        DBUG_ASSERT (replacement!=NULL, "replacement avis should not be NULL");
        ID_AVIS (arg_node) = replacement;
        break;
    case mode_undefined:
        DBUG_PRINT ("Entering DSSid '%s' in mode_undefined",
                    ID_NAME (arg_node));
        // if no mode is set, we do not need to rewrite anything
        // we should make sure that this id is not of a record type though
        sdef = GetStructDef (ID_NTYPE (arg_node));
        DBUG_ASSERT (sdef == NULL,
                     "DSS is in mode_undefined, but we encountered %s of type"
                     " struct %s",
                     ID_NAME (arg_node),
                     STRUCTDEF_NAME (sdef));
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSSavis (node *arg_node, info *arg_info)
 *
 * @brief Creates replacement avises for a struct type avis and adds them to
 * the look up table.
 *
 ******************************************************************************/
node *
DSSavis (node *arg_node, info *arg_info)
{
    node *sdef;

    DBUG_ENTER ();

    DBUG_PRINT ("visiting avis %s", AVIS_NAME (arg_node));

    // make sure this avis does not yet have replacements lined up
    DBUG_ASSERT (LUTsearchInLutP (INFO_LUT (arg_info), arg_node) == NULL,
                 "No replacements for avis should be present yet");

    sdef = GetStructDef (AVIS_TYPE (arg_node));
    if (sdef != NULL) {
        ExpandAvis (arg_node,
                    sdef,
                    AVIS_TYPE (arg_node),
                    TRAVtmpVarName (AVIS_NAME (arg_node)),
                    INFO_MARKED_ELEMENT (arg_info),
                    FALSE,
                    arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
