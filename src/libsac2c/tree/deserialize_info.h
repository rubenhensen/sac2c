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
