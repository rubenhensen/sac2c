#ifndef SAC2C_STR_VEC_H
#define SAC2C_STR_VEC_H

typedef struct STRVEC strvec;

strvec* STRVECempty(size_t preallocate);
strvec* STRVECmake(size_t length, ...);
strvec* STRVECconst(size_t length, char* str);
strvec* STRVECfromArray(char** array, size_t length);
strvec* STRVECgen(size_t length, char* (* generator)(void));
strvec* STRVECfree(strvec* vec);
strvec* STRVECfreeDeep(strvec* vec);
void STRVECresize(strvec* vec, size_t length, char* (* generator)(void));
void STRVECresizeFree(strvec* vec, size_t length, char* (* generator)(void));
bool STRVECisEmpty(strvec* vec);
size_t STRVEClen(strvec* vec);
void STRVECprint(strvec* vec, FILE* stream, size_t linesize, size_t init_linesize);
strvec* STRVECcopy(strvec* source);
strvec* STRVECcopyDeep(strvec* source);
strvec* STRVECappend(strvec* vec, char* str);
strvec* STRVECconcat(strvec* left, strvec* right);
char* STRVECsel(strvec* vec, size_t index);
char* STRVECswap(strvec* vec, size_t index, char* str);
char* STRVECpop(strvec* vec);

#endif //SAC2C_STR_VEC_H
