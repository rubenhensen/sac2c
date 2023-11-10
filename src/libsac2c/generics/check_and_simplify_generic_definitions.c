/******************************************************************************
 *
 * This module checks that the variable bindings in polymorphic types within a
 * function do comply to the following restrictions:
 *
 * - Only argument positions bind new identifiers. Both, the return types and
 *   the function body may only contain previously bound identifiers.
 *
 * - Identifiers used for referencing the shape of a polymorphic user type may
 *   not be used in polymorphic types
 *
 * - Identifiers used for referencing the shape of a polymorphic user type may
 *   not be shared between different types
 *
 * - Implicit denesting is allowed only in argument position
 *
 * - Implicit renesting is allowed only in return position
 *
 * Furthermore, the de- and renesting operations are made explicit by
 * introducing appropriate casts and the shape variabled are bound to a call of
 * F_nested_shape.
 *
 ******************************************************************************/
#include "ctinfo.h"
#include "memory.h"
#include "new_types.h"
#include "shape.h"
#include "str.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "type_utils.h"

#define DBUG_PREFIX "CSGD"
#include "debug.h"

#include "check_and_simplify_generic_definitions.h"

enum csgd_mode_t {
    CSGD_normal,
    CSGD_checkarg,
    CSGD_checkret,
    CSGD_checkcast,
    CSGD_checkavis,
    CSGD_renest,
    CSGD_denest,
    CSGD_bindshape,
    CSGD_strip
};

struct INFO {
    node *args;
    node *rets;
    node *fundef;
    node *current;
    node *preassigns;
    node *retexprs;
    enum csgd_mode_t mode;
    bool isgeneric;
    int retno;
    bool outerdefined;
    bool innerdefined;
    bool shapedefined;
};

#define INFO_ARGS(n) ((n)->args)
#define INFO_RETS(n) ((n)->rets)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_CURRENT(n) ((n)->current)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_RETEXPRS(n) ((n)->retexprs)
#define INFO_MODE(n) ((n)->mode)
#define INFO_ISGENERIC(n) ((n)->isgeneric)
#define INFO_RETNO(n) ((n)->retno)
#define INFO_OUTERDEFINED(n) ((n)->outerdefined)
#define INFO_INNERDEFINED(n) ((n)->innerdefined)
#define INFO_SHAPEDEFINED(n) ((n)->shapedefined)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ARGS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_CURRENT (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_RETEXPRS (result) = NULL;
    INFO_MODE (result) = CSGD_normal;
    INFO_ISGENERIC (result) = FALSE;
    INFO_RETNO (result) = 0;
    INFO_OUTERDEFINED (result) = FALSE;
    INFO_INNERDEFINED (result) = FALSE;
    INFO_SHAPEDEFINED (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn bool PolymorphicTypeComplies (ntype *a, ntype *b)
 *
 * @brief Checks whether the type a complies with type b.
 *
 * @param a a polymorphic type
 * @param b a second type
 *
 * @return true if both types comply
 *
 ******************************************************************************/
static bool
PolymorphicTypeComplies (ntype *a, ntype *b)
{
    bool result = TRUE;

    DBUG_ENTER ();

    if (TYisArray (a)) {
        a = TYgetScalar (a);
    }

    if (TYisArray (b)) {
        b = TYgetScalar (b);
    }

    if (TYisPoly (a)) {
        if (TYisPoly (b)) {
            /*
             * these always comply
             */
            result = TRUE;
        } else if (TYisPolyUser (b)) {
            /*
             * here a is not allowed to be used as shape in b
             */
            result = !STReq (TYgetPolyName (a), TYgetPolyUserShape (b));
        }
    } else if (TYisPolyUser (a)) {
        if (TYisPoly (b)) {
            /*
             * here a_shape is not allowed to be b
             */
            result = !STReq (TYgetPolyUserShape (a), TYgetPolyName (b));
        } else if (TYisPolyUser (b)) {
            /*
             * here a_outer and a_inner are not allowed to occur in b_shape
             * and b_outer and b_inner is not allowed to occur in a_shape
             */
            result = !STReq (TYgetPolyUserOuter (a), TYgetPolyUserShape (b))
                  && !STReq (TYgetPolyUserInner (a), TYgetPolyUserShape (b))
                  && !STReq (TYgetPolyUserShape (a), TYgetPolyUserOuter (b))
                  && !STReq (TYgetPolyUserShape (a), TYgetPolyUserInner (b));

            /*
             * if they both use the same shape variable, then the outer and
             * inner types need to be the same
             */
            result = result
                  && (!STReq (TYgetPolyUserShape (a), TYgetPolyUserShape (b))
                      || (STReq (TYgetPolyUserOuter (a), TYgetPolyUserOuter (b))
                          && STReq (TYgetPolyUserInner (a), TYgetPolyUserInner (b))));
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * @fn info *AnnotateDefinedVars (ntype *type, ntype *def, info *arg_info)
 *
 * @brief Checks whether the type variables in the polymorphic type type are
 *        bound in the type def. If so, the corresponding flags in the info
 *        structure are set to TRUE.
 *
 * @param type type to check
 * @param def binding type
 * @param arg_info info structure
 *
 * @return info structure
 *
 ******************************************************************************/
static info *
AnnotateDefinedVars (ntype *type, ntype *def, info *arg_info)
{
    DBUG_ENTER ();

    if (TUisPolymorphic (def)) {
        if (TYisArray (type)) {
            type = TYgetScalar (type);
        }

        if (TYisArray (def)) {
            def = TYgetScalar (def);
        }

        if (TYisPoly (type)) {
            if (TYisPoly (def)) {
                INFO_OUTERDEFINED (arg_info) =
                    INFO_OUTERDEFINED (arg_info)
                    || STReq (TYgetPolyName (type), TYgetPolyName (def));
            } else if (TYisPolyUser (def)) {
                INFO_OUTERDEFINED (arg_info) =
                    INFO_OUTERDEFINED (arg_info)
                    || STReq (TYgetPolyName (type), TYgetPolyUserOuter (def))
                    || STReq (TYgetPolyName (type), TYgetPolyUserInner (def));
            }
        } else if (TYisPolyUser (type)) {
            if (TYisPoly (def)) {
                INFO_OUTERDEFINED (arg_info) =
                    INFO_OUTERDEFINED (arg_info)
                    || STReq (TYgetPolyUserOuter (type), TYgetPolyName (def));
                INFO_INNERDEFINED (arg_info) =
                    INFO_INNERDEFINED (arg_info)
                    || STReq (TYgetPolyUserInner (type), TYgetPolyName (def));
            } else if (TYisPolyUser (def)) {
                INFO_OUTERDEFINED (arg_info) =
                    INFO_OUTERDEFINED (arg_info)
                    || STReq (TYgetPolyUserOuter (type), TYgetPolyUserOuter (def))
                    || STReq (TYgetPolyUserOuter (type), TYgetPolyUserInner (def));
                INFO_INNERDEFINED (arg_info) =
                    INFO_INNERDEFINED (arg_info)
                    || STReq (TYgetPolyUserInner (type), TYgetPolyUserOuter (def))
                    || STReq (TYgetPolyUserInner (type), TYgetPolyUserInner (def));
                INFO_SHAPEDEFINED (arg_info) =
                    INFO_SHAPEDEFINED (arg_info)
                    || STReq (TYgetPolyUserShape (type), TYgetPolyUserShape (def));
            }
        }
    }

    DBUG_RETURN (arg_info);
}

node *
CSGDdoCheckAndSimplifyGenericDefinitions (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "called with non-module node");

    arg_info = MakeInfo ();

    TRAVpush (TR_csgd);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    CTIabortOnError ();

    DBUG_RETURN (arg_node);
}

node *
CSGDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("----- check and simplify generic definitions of %s -----",
                FUNDEF_NAME (arg_node));

    /*
     * 0) store context in info node
     */
    INFO_FUNDEF (arg_info) = arg_node;
    INFO_ARGS (arg_info) = FUNDEF_ARGS (arg_node);
    INFO_RETS (arg_info) = FUNDEF_RETS (arg_node);
    INFO_ISGENERIC (arg_info) = FALSE;

    /*
     * 1) check compliance of args and figure out whether this is a generic
     *    function after all.
     */
    DBUG_PRINT ("checking compliance of args");
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

    FUNDEF_ISGENERIC (arg_node) = INFO_ISGENERIC (arg_info);

    /*
     * 2) check compliance of rets
     */
    DBUG_PRINT ("checking compliance of rets");
    FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);

    /*
     * 3) bind shape vars
     *    this needs to be done before we denest the type in the body, as
     *    otherwise the call to nested_shape will return the wrong result!
     */
    DBUG_PRINT ("binding shape vars");
    INFO_MODE (arg_info) = CSGD_bindshape;
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    INFO_MODE (arg_info) = CSGD_normal;

    /*
     * 4) process denesting
     */
    DBUG_PRINT ("process denesting");
    INFO_MODE (arg_info) = CSGD_denest;
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    INFO_MODE (arg_info) = CSGD_normal;

    /*
     * 5) process renesting and check compliance of body
     */
    DBUG_PRINT ("process renesting");
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    /*
     * 6) strip nesting information from types
     */
    DBUG_PRINT ("stripping nesting information");
    INFO_MODE (arg_info) = CSGD_strip;
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);
    INFO_MODE (arg_info) = CSGD_normal;

    /*
     * 7) next funef
     */
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
CSGDarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_MODE (arg_info)) {
    case CSGD_normal:
        if (TUisPolymorphic (ARG_NTYPE (arg_node))) {
            INFO_ISGENERIC (arg_info) = TRUE;
            INFO_CURRENT (arg_info) = arg_node;

            INFO_MODE (arg_info) = CSGD_checkarg;
            INFO_ARGS (arg_info) = TRAVdo (INFO_ARGS (arg_info), arg_info);
            INFO_MODE (arg_info) = CSGD_normal;

            INFO_CURRENT (arg_info) = NULL;
        }
        break;
    case CSGD_checkarg:
        /*
         * check whether the argument stored in the info structure complies
         * with the current one
         */
        if (!PolymorphicTypeComplies (ARG_NTYPE (INFO_CURRENT (arg_info)),
                                      ARG_NTYPE (arg_node))) {
            CTIerror (NODE_LOCATION (INFO_CURRENT (arg_info)),
                      "In definition of %s: type and shape variables cannot be "
                      "mixed (in arguments %s and %s).",
                      CTIitemName (INFO_FUNDEF (arg_info)),
                      ARG_NAME (INFO_CURRENT (arg_info)), ARG_NAME (arg_node));
        }
        break;
    case CSGD_checkret:
        if (!PolymorphicTypeComplies (RET_TYPE (INFO_CURRENT (arg_info)),
                                      ARG_NTYPE (arg_node))) {
            CTIerror (NODE_LOCATION (INFO_CURRENT (arg_info)),
                      "In definition of %s: type and shape variables cannot be "
                      "mixed (in return type %d and argument %s).",
                      CTIitemName (INFO_FUNDEF (arg_info)), INFO_RETNO (arg_info),
                      ARG_NAME (arg_node));
        }

        arg_info = AnnotateDefinedVars (RET_TYPE (INFO_CURRENT (arg_info)),
                                        ARG_NTYPE (arg_node), arg_info);
        break;
    case CSGD_checkcast:
        if (!PolymorphicTypeComplies (CAST_NTYPE (INFO_CURRENT (arg_info)),
                                      ARG_NTYPE (arg_node))) {
            CTIerror (NODE_LOCATION (INFO_CURRENT (arg_info)),
                      "In definition of %s: type and shape variables cannot be "
                      "mixed (in cast type and argument %s).",
                      CTIitemName (INFO_FUNDEF (arg_info)), ARG_NAME (arg_node));
        }

        arg_info = AnnotateDefinedVars (CAST_NTYPE (INFO_CURRENT (arg_info)),
                                        ARG_NTYPE (arg_node), arg_info);
        break;
    case CSGD_checkavis:
        if (!PolymorphicTypeComplies (AVIS_TYPE (INFO_CURRENT (arg_info)),
                                      ARG_NTYPE (arg_node))) {
            CTIerror (NODE_LOCATION (INFO_CURRENT (arg_info)),
                      "In definition of %s: type and shape variables cannot be "
                      "mixed (in declared type of local variable %s and "
                      "argument %s).",
                      CTIitemName (INFO_FUNDEF (arg_info)),
                      AVIS_NAME (INFO_CURRENT (arg_info)), ARG_NAME (arg_node));
        }

        arg_info = AnnotateDefinedVars (AVIS_TYPE (INFO_CURRENT (arg_info)),
                                        ARG_NTYPE (arg_node), arg_info);
        break;
    case CSGD_bindshape:
        if (TYisPolyUser (TYgetScalar (ARG_NTYPE (arg_node)))) {
            ntype *poly = TYgetScalar (ARG_NTYPE (arg_node));

            INFO_PREASSIGNS (arg_info) = TCappendAssign (
                INFO_PREASSIGNS (arg_info),
                TBmakeAssign (
                    TBmakeLet (TBmakeSpids (STRcpy (TYgetPolyUserShape (poly)), NULL),
                               TCmakePrf1 (F_nested_shape, TBmakeType (
                                   TYmakeAKS (TYmakePolyType (STRcpy (TYgetPolyUserOuter (poly))),
                                              SHmakeShape (0))))),
                    NULL));
        }
        break;
    case CSGD_denest:
        if (TYisPolyUser (TYgetScalar (ARG_NTYPE (arg_node)))
            && TYgetPolyUserDeNest (TYgetScalar (ARG_NTYPE (arg_node)))) {
            ntype *poly = TYgetScalar (ARG_NTYPE (arg_node));

            INFO_PREASSIGNS (arg_info) = TCappendAssign (
                INFO_PREASSIGNS (arg_info),
                TBmakeAssign (
                    TBmakeLet (TBmakeSpids (STRcpy (ARG_NAME (arg_node)), NULL),
                               TBmakeCast (TYmakeAUD (TYmakePolyType (STRcpy (TYgetPolyUserInner (poly)))),
                                           TBmakeSpid (NULL, STRcpy (ARG_NAME (arg_node))))),
                    NULL));
        }
        break;
    case CSGD_strip:
        if (TUisPolymorphic (ARG_NTYPE (arg_node))) {
            ntype *tmp = ARG_NTYPE (arg_node);
            ARG_NTYPE (arg_node) = TUstripImplicitNestingOperations (tmp);
            tmp = TYfreeType (tmp);
        }
        break;
    default:
        DBUG_UNREACHABLE ("unknown traversal mode!");
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
CSGDret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_RETNO (arg_info)++;

    switch (INFO_MODE (arg_info)) {
    case CSGD_normal:
        if (TUisPolymorphic (RET_TYPE (arg_node))) {
            INFO_CURRENT (arg_info) = arg_node;
            INFO_MODE (arg_info) = CSGD_checkret;
            INFO_OUTERDEFINED (arg_info) = FALSE;
            INFO_INNERDEFINED (arg_info) = FALSE;
            INFO_SHAPEDEFINED (arg_info) = FALSE;

            INFO_ARGS (arg_info) = TRAVopt (INFO_ARGS (arg_info), arg_info);

            if (!INFO_OUTERDEFINED (arg_info)) {
                CTIerror (NODE_LOCATION (arg_node),
                          "In definition of %s: Type variable in polymorphic return "
                          "type not bound by any argument (return value %d).",
                          CTIitemName (INFO_FUNDEF (arg_info)), INFO_RETNO (arg_info));
            }

            if (TYisPolyUser (TYgetScalar (RET_TYPE (arg_node)))) {
                if (!INFO_INNERDEFINED (arg_info)) {
                    CTIerror (NODE_LOCATION (arg_node),
                              "In definition of %s: Inner type variable in polymorphic "
                              "return type not bound by any argument (return value %d).",
                              CTIitemName (INFO_FUNDEF (arg_info)),
                              INFO_RETNO (arg_info));
                }

                if (!INFO_SHAPEDEFINED (arg_info)) {
                    CTIerror (NODE_LOCATION (arg_node),
                              "In definition of %s: Shape variable in polymorphic return "
                              "type not bound by any argument (return value %d).",
                              CTIitemName (INFO_FUNDEF (arg_info)),
                              INFO_RETNO (arg_info));
                }
            }

            INFO_MODE (arg_info) = CSGD_normal;
        }
        break;
    case CSGD_renest:
        if (TYisPolyUser (TYgetScalar (RET_TYPE (arg_node)))
            && TYgetPolyUserReNest (TYgetScalar (RET_TYPE (arg_node)))) {
            char *newvar = TRAVtmpVar ();
            ntype *poly = TYgetScalar (RET_TYPE (arg_node));

            INFO_PREASSIGNS (arg_info) = TCappendAssign (
                INFO_PREASSIGNS (arg_info),
                TBmakeAssign (
                    TBmakeLet (TBmakeSpids (STRcpy (newvar), NULL),
                               TBmakeCast (TYmakeAUD (TYmakePolyType (STRcpy (TYgetPolyUserOuter (poly)))),
                                           EXPRS_EXPR (INFO_RETEXPRS (arg_info)))),
                    NULL));

            EXPRS_EXPR (INFO_RETEXPRS (arg_info)) = TBmakeSpid (NULL, newvar);
        }
        break;
    case CSGD_strip:
        if (TUisPolymorphic (RET_TYPE (arg_node))) {
            ntype *tmp = RET_TYPE (arg_node);
            RET_TYPE (arg_node) = TUstripImplicitNestingOperations (tmp);
            tmp = TYfreeType (tmp);
        }
        break;
    default:
        DBUG_UNREACHABLE ("unknown traversal mode.");
    }

    if (INFO_RETEXPRS (arg_info) != NULL) {
        INFO_RETEXPRS (arg_info) = EXPRS_NEXT (INFO_RETEXPRS (arg_info));
    }

    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);

    INFO_RETNO (arg_info)--;

    DBUG_RETURN (arg_node);
}

node *
CSGDassign (node *arg_node, info *arg_info)
{
    node *preassigns = NULL;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        preassigns = INFO_PREASSIGNS (arg_info);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    if (preassigns != NULL) {
        arg_node = TCappendAssign (preassigns, arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
CSGDcast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (CAST_NTYPE (arg_node) != NULL, "ntype is null");

    if (TUisPolymorphic (CAST_NTYPE (arg_node))) {
        INFO_CURRENT (arg_info) = arg_node;
        INFO_MODE (arg_info) = CSGD_checkcast;
        INFO_OUTERDEFINED (arg_info) = FALSE;
        INFO_INNERDEFINED (arg_info) = FALSE;
        INFO_SHAPEDEFINED (arg_info) = FALSE;

        INFO_ARGS (arg_info) = TRAVopt (INFO_ARGS (arg_info), arg_info);

        if (!INFO_OUTERDEFINED (arg_info)) {
            CTIerror (NODE_LOCATION (arg_node),
                      "In definition of %s: Type variable in polymorphic cast "
                      "type not bound by any argument.",
                      CTIitemName (INFO_FUNDEF (arg_info)));
        }

        if (TYisPolyUser (TYgetScalar (CAST_NTYPE (arg_node)))) {
            if (!INFO_INNERDEFINED (arg_info)) {
                CTIerror (NODE_LOCATION (arg_node),
                          "In definition of %s: Inner type variable in "
                          "polymorphic cast type not bound by any argument.",
                          CTIitemName (INFO_FUNDEF (arg_info)));
            }

            if (!INFO_SHAPEDEFINED (arg_info)) {
                CTIerror (NODE_LOCATION (arg_node),
                          "In definition of %s: Shape variable in "
                          "polymorphic cast type not bound by any argument.",
                          CTIitemName (INFO_FUNDEF (arg_info)));
            }

            if (TYgetPolyUserDeNest (TYgetScalar (CAST_NTYPE (arg_node)))) {
                CTIerror (NODE_LOCATION (arg_node),
                          "In definition of %s: Implicit denesting of polymorphic "
                          "user type not allowed in cast expressions.",
                          CTIitemName (INFO_FUNDEF (arg_info)));
            }

            if (TYgetPolyUserReNest (TYgetScalar (CAST_NTYPE (arg_node)))) {
                CTIerror (NODE_LOCATION (arg_node),
                          "In definition of %s: Implicit renesting of polymorphic "
                          "user type not allowed in cast expressions.",
                          CTIitemName (INFO_FUNDEF (arg_info)));
            }
        }

        INFO_MODE (arg_info) = CSGD_normal;
    }

    CAST_EXPR (arg_node) = TRAVdo (CAST_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
CSGDreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RETURN_EXPRS (arg_node) = TRAVopt (RETURN_EXPRS (arg_node), arg_info);

    if (INFO_RETS (arg_info) != NULL) {
        INFO_RETEXPRS (arg_info) = RETURN_EXPRS (arg_node);

        INFO_MODE (arg_info) = CSGD_renest;
        INFO_RETS (arg_info) = TRAVdo (INFO_RETS (arg_info), arg_info);
        INFO_MODE (arg_info) = CSGD_normal;
    }

    DBUG_RETURN (arg_node);
}

node *
CSGDavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (TUisPolymorphic (AVIS_TYPE (arg_node))) {
        INFO_CURRENT (arg_info) = arg_node;
        INFO_MODE (arg_info) = CSGD_checkavis;
        INFO_OUTERDEFINED (arg_info) = FALSE;
        INFO_INNERDEFINED (arg_info) = FALSE;
        INFO_SHAPEDEFINED (arg_info) = FALSE;

        INFO_ARGS (arg_info) = TRAVopt (INFO_ARGS (arg_info), arg_info);

        if (!INFO_OUTERDEFINED (arg_info)) {
            CTIerror (NODE_LOCATION (arg_node),
                      "In definition of %s: Type variable in polymorphic cast "
                      "type not bound by any argument.",
                      CTIitemName (INFO_FUNDEF (arg_info)));
        }

        if (TYisPolyUser (TYgetScalar (AVIS_TYPE (arg_node)))) {
            if (!INFO_INNERDEFINED (arg_info)) {
                CTIerror (NODE_LOCATION (arg_node),
                          "In definition of %s: Inner type variable in polymorphic "
                          "type of local variable %s not bound by any argument.",
                          CTIitemName (INFO_FUNDEF (arg_info)), AVIS_NAME (arg_node));
            }

            if (!INFO_SHAPEDEFINED (arg_info)) {
                CTIerror (NODE_LOCATION (arg_node),
                          "In definition of %s: Shape variable in polymorphic type "
                          "of local variable %s not bound by any argument.",
                          CTIitemName (INFO_FUNDEF (arg_info)), AVIS_NAME (arg_node));
            }

            if (TYgetPolyUserDeNest (TYgetScalar (AVIS_TYPE (arg_node)))) {
                CTIerror (NODE_LOCATION (arg_node),
                          "In definition of %s: Implicit denesting of polymorphic user "
                          "type not allowed in type declaration of local variable %s.",
                          CTIitemName (INFO_FUNDEF (arg_info)), AVIS_NAME (arg_node));
            }

            if (TYgetPolyUserReNest (TYgetScalar (AVIS_TYPE (arg_node)))) {
                CTIerror (NODE_LOCATION (arg_node),
                          "In definition of %s: Implicit renesting of polymorphic user "
                          "type not allowed in type declaration of local variable %s.",
                          CTIitemName (INFO_FUNDEF (arg_info)), AVIS_NAME (arg_node));
            }
        }
    }

    DBUG_RETURN (arg_node);
}
