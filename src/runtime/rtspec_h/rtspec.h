/**<!--*********************************************************************-->
 *
 * @file  dynspec.h
 *
 * @brief Contains the ICMs for the creation of wrapper entry functions.
 *
 * @author tvd
 *
 *****************************************************************************/

#ifdef SAC_DO_RTSPEC

/*
 * Print the code necessary to setup the optimization controller.
 */
#define SAC_RTSPEC_SETUP()                                                               \
    {                                                                                    \
        SAC_setupController ();                                                          \
    }

/*
 * Print the code necessary to finalize the optimization controller.
 */
#define SAC_RTSPEC_FINALIZE()                                                            \
    {                                                                                    \
        SAC_finalizeController ();                                                       \
    }

#endif

/*
 * Wrapper entry begin
 */
#define SAC_WE_DEF_FUN_BEGIN2(num_args, name, type, ...)                                 \
    SAC_ND_DECL_FUN2 (name, type, __VA_ARGS__)                                           \
    {

/*
 * Wrapper entry end.
 */
#define SAC_WE_FUN_DEF_END2(...) }

/*
 * Declare the flag needed to check if registration has taken place.
 */
#define SAC_WE_DECL_REG_FLAG static int SAC_registered = 0;

/*
 * Declare the registry object needed to store the pointer to the original
 * wrapper.
 */
#define SAC_WE_DECL_REG_OBJ static reg_obj_t *SAC_reg_obj;

/*
 * Store the name of the original module, needed for the registry and the
 * request.
 */
#define SAC_WE_DECL_MOD(name) static char *SAC_module = name;

/*
 * Store the name of the original function, needed for the registry and the
 * request.
 */
#define SAC_WE_DECL_FUN(name) static char *SAC_function = name;

/*
 * Declare the function pointer used to call the wrapper.
 */
#define SAC_WE_DECL_FN_POINTER(rettype, ...) rettype (*SAC_func_ptr) (__VA_ARGS__);

/*
 * Declare the integer array that will hold the shapes of the arguments at
 * runtime.
 */
#define SAC_WE_DECL_SHAPE_ARRAY int *SAC_shapes = NULL;

/*
 * Declare and initialize two integers needed to traverse the shape array and
 * the descriptors.
 */
#define SAC_WE_DECL_I_J int i = 0, j = 0;

/*
 * Print the declaration and initialization of the variable 'size'. This
 * variable is used to allocate the correct amount of memory for the shape
 * array.
 */
#define SAC_WE_CALC_SIZE(dim_sum) int size = 1 + dim_sum;

/*
 * Print the condition statement and action for registering a function.
 */
#define SAC_WE_REGISTRATION(module, name)                                                \
    if (!SAC_registered) {                                                               \
        SAC_reg_obj = SAC_registrate (module, (void *)&name);                            \
        SAC_registered = 1;                                                              \
    }

/*
 * Print the function call to enqueue a request for optimization.
 */
#define SAC_WE_ENQ_REQ(types, name)                                                      \
    SAC_enqueueRequest (SAC_function, SAC_reg_obj->module, #types, SAC_shapes,           \
                        SAC_reg_obj);

/*
 * Print the 'update' of the function pointer.
 */
#define SAC_WE_PTR_UPDATE SAC_func_ptr = SAC_reg_obj->func_ptr;

/*
 * Print the correct indexation of the descriptor to get the dimensions of a
 * certain argument.
 */
#define SAC_WE_GET_DIM(arg) arg##__desc[1]

/*
 * Allocate memory for the shape array.
 */
#define SAC_WE_ALLOC_SHAPE_ARRAY SAC_shapes = malloc (size * sizeof (int));

/*
 * Set the first item of the shape array to the number of arguments.
 */
#define SAC_WE_SET_NUM_ARGS(num)                                                         \
    SAC_shapes[i] = num;                                                                 \
    i++;

/*
 * Print the code that encodes the shape of an argument at runtime.
 */
#define SAC_WE_GET_SHAPE(arg)                                                            \
    SAC_shapes[i] = SAC_WE_GET_DIM (arg);                                                \
    i++;                                                                                 \
    for (j = 0; j < SAC_WE_GET_DIM (arg); j++) {                                         \
        SAC_shapes[i++] = arg##__desc[3 + j];                                            \
    }

/*
 * Print all the code needed to:
 *
 * - allocate the registry object.
 * - enqueue a request for optimization.
 * - update the function pointer.
 * - use the function pointer to call the 'normal' wrapper.
 */
#define SAC_WE_FUNAP2(types, name, ...)                                                  \
    SAC_WE_REGISTRATION (SAC_module, name)                                               \
    SAC_WE_ENQ_REQ (types, name)                                                         \
    SAC_WE_PTR_UPDATE (*SAC_func_ptr) (__VA_ARGS__);