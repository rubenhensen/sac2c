#ifndef _SAC_CHECK_LIB_H_
#define _SAC_CHECK_LIB_H_

#include "types.h"

/******************************************************************************
 *
 * Check
 *
 * Prefix: CHK
 *
 *****************************************************************************/
extern node *CHKinsertError (node *arg_node, char *string);
extern node *CHKexistSon (node *son, node *arg_node, char *string);
extern intptr_t CHKexistAttribute (intptr_t attribute, node *arg_node, char *string);
extern node *CHKnotExistAttribute (void *attribute, node *arg_node, char *string);
extern intptr_t CHKnotExist (intptr_t son_attribute, node *arg_node, char *string);
extern node *CHKcorrectTypeInsertError (node *arg_node, char *string);

extern node *CHKattribsIds (node *arg_node);
extern node *CHKattribsAssign (node *arg_node);
extern node *CHKfundefVardecExtrema (node *arg_node);
extern node *CHKisNullSsaassign (node *arg_node);
extern node *CHKcondfun (node *arg_node);
extern node *CHKavisReflection (node *arg_node);
extern node *CHKavisSsaassignNodeType (node *arg_node);
extern node *CHKapArgCount (node *arg_node);
extern node *CHKfundefReturn (node *arg_node);

#endif /*_SAC_CHECK_LIB_H_ */
