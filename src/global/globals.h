/*
 *
 * $Log$
 * Revision 1.1  1995/12/30 14:47:06  cg
 * Initial revision
 *
 *
 *
 *
 */

#ifndef _sac_globals_h

#define _sac_globals_h

/*
 *  This header file contains external declarations for all global
 *  variables which are defined and initialized in main.c
 */

#include "filemgr.h"

extern FILE *outfile;

extern char filename[MAX_FILE_NAME];
extern char prgname[MAX_FILE_NAME];
extern char outfilename[MAX_FILE_NAME];
extern char cfilename[MAX_FILE_NAME];
extern char ccflagsstr[MAX_FILE_NAME];

extern int Ccodeonly;

extern int optimize;
extern int sac_optimize;
extern int opt_dcr;
extern int opt_cf;
extern int opt_wr;
extern int opt_lir;
extern int opt_inl;
extern int opt_unr;
extern int opt_uns;
extern int opt_ae;

extern int optvar;
extern int inlnum;
extern int unrnum;
extern int minarray;

extern int max_overload;
extern int max_optcycles;
extern int psi_optimize;
extern int psi_opt_ive;

extern int show_refcnt;
extern int show_idx;
extern int show_icm;
extern int traceflag;

extern int breakae;

extern int check_boundary;

#endif /* _sac_globals_h */
