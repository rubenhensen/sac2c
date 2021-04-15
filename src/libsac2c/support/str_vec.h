#ifndef SAC2C_STR_VEC_H
#define SAC2C_STR_VEC_H

struct STRVEC;
typedef struct STRVEC* strvec;

strvec STRVECmake(int length, ...);
strvec STRVECfromArray(char** array, unsigned int length);
strvec STRVECgen(unsigned int length, char* (* generator)(void));
void STRVECfree(strvec vec);
void STRVECresize(strvec vec, unsigned int length, char* (* generator)(void));
strvec STRVECcopy(strvec source);
void STRVECappend(strvec vec, char* str);
void STRVECconcat(strvec left, strvec right);
char* STRVECsel(strvec vec, unsigned int index);
char* STRVECsel_shallow(strvec vec, unsigned int index);
void STRVECput(strvec vec, unsigned int index, char* str);

#endif //SAC2C_STR_VEC_H
