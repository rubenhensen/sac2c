
MACRO (CHECK_FUN_ATTRIB src def_no def_yes name)
    CHECK_C_SOURCE_COMPILES ("${src}" HAVE_${name}_ATTR)
    SET (${name}_NONE "${def_no}")
    IF (HAVE_${name}_ATTR)
        SET (${name} "${${name}_NONE} ${def_yes}")
    ENDIF ()
ENDMACRO ()


CHECK_FUN_ATTRIB (
"void log (const char *format, ...) __attribute__ ((format (printf, 1, 2)));
 int main () { return 0; }"

 "#define PRINTF_FORMAT(__str_arg_no__, __va_args_no__) "

"__attribute__ ((format (printf, __str_arg_no__, __va_args_no__)))"

MACRO_FORMAT_PRINTF)


CHECK_FUN_ATTRIB (
"void test (int t) __attribute__ ((unused));
 int main () { return 0; }"

"#define UNUSED "

"__attribute__ ((unused))"

MACRO_UNUSED)


CHECK_FUN_ATTRIB (
"void * test (unsigned sz) __attribute__ ((malloc));
 int main () { return 0; }"

 "#define FUN_ATTR_MALLOC "

"__attribute__ ((malloc))"

MACRO_MALLOC)


CHECK_FUN_ATTRIB (
"void * test (void * t1, void * t2) __attribute__ ((nonull (1, 2)));
 int main () { return 0; }"

"#define FUN_ATTR_NONULL(...) "

"__attribute__ ((nonull (__VA_ARGS__)))"

MACRO_NONULL)


CHECK_FUN_ATTRIB (
"void die (const char * msg) __attribute__ ((noreturn));
 int main () { return 0; }"

 "#define FUN_ATTR_NORETURN "

"__attribute__ ((noreturn))"

MACRO_NORETURN)


CHECK_FUN_ATTRIB (
"int stringlen (const char * s) __attribute__ ((pure));
 int main () { return 0; }"

 "#define FUN_ATTR_PURE "

"__attribute__ ((pure))"

MACRO_PURE)

CHECK_FUN_ATTRIB (
"int plusone (const int arg) __attribute__ ((const));
 int main () { return 0; }"

 "#define FUN_ATTR_CONST "

"__attribute__ ((const))"

MACRO_CONST)
