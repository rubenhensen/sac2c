/******************************************************************************
 *
 * Options
 *
 * Prefix: OPT
 *
 * Description:
 *
 * This file provides external declarations for symbols defined in options.c.
 *
 *****************************************************************************/

#ifndef _SAC_OPTIONS_H_
#define _SAC_OPTIONS_H_

extern void OPTanalyseCommandline (int argc, char *argv[]);
extern void OPTcheckPreSetupOptions (int argc, char *argv[]);
extern void OPTcheckOptionConsistency (void);
extern void OPTcheckOptionConsistencyForTarget (bool in_module);
extern void OPTcheckPostSetupOptions (void);

#endif /* _SAC_OPTIONS_H_ */
