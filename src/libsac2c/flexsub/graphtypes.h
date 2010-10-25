#ifndef _GRAPHTYPES_H_
#define _GRAPHTYPES_H_

#include "types.h"

#define COMPINFO_CSRC(n) ((n)->csrc)
#define COMPINFO_CTAR(n) ((n)->ctar)
#define COMPINFO_TLTABLE(n) ((n)->tltable)
#define COMPINFO_EULERTOUR(n) ((n)->eulertour)
#define COMPINFO_PREARR(n) ((n)->prearr)
#define COMPINFO_CROSSCLOS(n) ((n)->crossclos)
#define COMPINFO_TLC(n) ((n)->tlc)
#define COMPINFO_LUB(n) ((n)->lub)
#define COMPINFO_DIST(n) ((n)->dist)
#define COMPINFO_TOPOLIST(n) ((n)->topolist)

#define LUBINFO_NUMINTRA(n) ((n)->numintra)
#define LUBINFO_BLOCKSIZE(n) ((n)->blocksize)
#define LUBINFO_BLOCKMIN(n) ((n)->blockmin)
#define LUBINFO_INTERMAT(n) ((n)->intermat)
#define LUBINFO_INTRAMATS(n) ((n)->intramats)
#define LUBINFO_INTRAMATS_POS(n, i) ((n)->intramats[i])
#define LUBINFO_PCPTMAT(n) ((n)->pcptmat)
#define LUBINFO_PCPCMAT(n) ((n)->pcpcmat)

extern compinfo **freeCompInfoArr (compinfo **cia, int n);
extern lubinfo *freeLubInfo (lubinfo *linfo);

#endif
