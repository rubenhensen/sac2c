#ifndef _GRAPHTYPES_H_
#define _GRAPHTYPES_H_

#include "types.h"

#define COMPINFO_CSRC(n) ((n)->csrc)
#define COMPINFO_CTAR(n) ((n)->ctar)
#define COMPINFO_TLTABLE(n) ((n)->tltable)
#define COMPINFO_EULERTOUR(n) ((n)->eulertour)
#define COMPINFO_CROSSCLOS(n) ((n)->crossclos)
#define COMPINFO_TLC(n) ((n)->tlc)
#define COMPINFO_LUB(n) ((n)->lub)
#define COMPINFO_LUBPOS(n, i) ((n)->lub[i])
#define COMPINFO_DIST(n) ((n)->dist)
#define COMPINFO_TOPOLIST(n) ((n)->topolist)

extern compinfo **freeCompInfoArr (compinfo **cia, int n);

#endif
