/*
 *
 * $Log$
 * Revision 1.57  1998/05/13 07:15:43  cg
 * added include of icm2c_mt.h
 *
 * Revision 1.56  1998/05/04 16:01:39  dkr
 * added includes: icm2c_std.h, icm2c_wl.h
 *
 * Revision 1.55  1998/05/03 13:09:17  dkr
 * added icm2c_wl.h
 *
 * Revision 1.54  1998/04/25 16:25:20  sbs
 *  new icm2c / BEtest mechanism implemented!
 *
 *
 */

#include "tree.h"
#include "dbug.h"
#include "globals.h"
#include "convert.h"
#include "my_debug.h"

#include "icm2c_std.h"
#include "icm2c_wl.h"
#include "icm2c_mt.h"

/*
 * Variables needed for the code generated by "icm_icm2c.c":
 *
 * The choice to make them static rather than local to the Print-funktions
 * is motivated by the fact, that this decision spares one "traversal" through
 * icm.data and thus allows us to use ICM_ALL instead of specific includes
 * for each ICM!
 */

#define ICM_ALL
#include "icm_vars.c"
#undef ICM_ALL

/*
 * Now, we can create ALL Print-functions by a single include!!!
 * The respective external declarations are produced locally in print.c!!
 */

#define ICM_ALL
#include "icm_icm2c.c"
#undef ICM_ALL
