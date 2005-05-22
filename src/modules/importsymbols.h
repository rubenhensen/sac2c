/*
 * $Log$
 * Revision 1.1  2005/05/22 19:46:48  sah
 * Initial revision
 *
 *
 */

/**
 * @file importsymbols.h
 * @brief traversal handling import of functions and types
 * @author Stephan Herhut
 * @date 2005-04-29
 */

#ifndef _SAC_IMPORTSYMBOLS_H_
#define _SAC_IMPORTSYMBOLS_H_

#include "types.h"

extern node *IMPimport (node *arg_node, info *arg_info);
extern node *IMPuse (node *arg_node, info *arg_info);
extern node *IMPexport (node *arg_node, info *arg_info);
extern node *IMPprovide (node *arg_node, info *arg_info);
extern node *IMPsymbol (node *arg_node, info *arg_info);
extern node *IMPmodule (node *arg_node, info *arg_info);
extern node *IMPdoImportSymbols (node *syntax_tree);

#endif
