/*
 * This file contains external declarations for all functions/global variables
 * provided by the scanner/parser which are used from outside.
 */

#ifndef _SAC_SCNPRS_H_
#define _SAC_SCNPRS_H_

#include "types.h"

extern node *SPdoLocateSource (node *syntax_tree);
extern node *SPdoRunPreProcessor (node *syntax_tree);
extern node *SPdoScanParse (node *syntax_tree);

extern int SPmyYyparse (void);

#endif /* _SAC_SCNPRS_H_ */
