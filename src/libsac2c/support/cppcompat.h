/* This is a compatibility definitions when we compile with C++
   compiler.  No guards, as we will have mainly macros here.  */

#ifdef __cplusplus

/* It seems that va_copy is not in the C++ standard.  I don't know
   what is supposed to be used instead, so for the time being
   I'll use GCC builtin.  */
#define va_copy __builtin_va_copy

#endif /* __cplusplus  */
