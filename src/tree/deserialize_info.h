/*
 *
 * $Log$
 * Revision 1.6  2004/11/22 21:29:55  ktr
 * Big Switch Header! SacDevCamp 04
 *
 * Revision 1.5  2004/11/21 12:48:08  sah
 * removed some old ast info
 *
 * Revision 1.4  2004/11/14 15:26:05  sah
 * implemented support for udts
 *
 * Revision 1.3  2004/11/07 18:11:21  sah
 * added FUNDECS
 *
 * Revision 1.2  2004/10/28 17:20:46  sah
 * now deserialize as an internal state
 *
 * Revision 1.1  2004/10/25 16:07:32  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_DESERIALIZE_INFO_H_
#define _SAC_DESERIALIZE_INFO_H_

#include "types.h"

/*
 * INFO structure
 */
struct INFO {
    node *ret;
    node *ssacounter;
    node *module;
    node *fundefs;
    node *fundecs;
    node *typedefs;
    node *objdefs;
    node *vardecs;
    node *args;
};

/*
 * INFO macros
 */
#define INFO_DS_RETURN(n) (n->ret)
#define INFO_DS_SSACOUNTER(n) (n->ssacounter)
#define INFO_DS_MODULE(n) (n->module)
#define INFO_DS_FUNDEFS(n) (n->fundefs)
#define INFO_DS_FUNDECS(n) (n->fundecs)
#define INFO_DS_TYPEDEFS(n) (n->typedefs)
#define INFO_DS_OBJDEFS(n) (n->objdefs)
#define INFO_DS_VARDECS(n) (n->vardecs)
#define INFO_DS_ARGS(n) (n->args)

#endif /* _SAC_DESERIALIZE_INFO_H_ */
