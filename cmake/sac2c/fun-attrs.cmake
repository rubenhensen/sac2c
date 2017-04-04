
CHECK_C_SOURCE_COMPILES ("
void log (const char *format, ...) __attribute__ ((format (printf, 1, 2)));
int main () { return 0; }
" HAVE_FORMAT_PRINTF_ATTR)

SET (MACRO_FORMAT_PRINTF_NONE "#define PRINTF_FORMAT(__str_arg_no__, __va_args_no__) ")
IF (HAVE_FORMAT_PRINTF_ATTR)
  SET (MACRO_FORMAT_PRINTF
      "${MACRO_FORMAT_PRINTF_NONE} __attribute__ ((format (printf, __str_arg_no__, __va_args_no__)))")
ENDIF ()

CHECK_C_SOURCE_COMPILES ("
void test (int t) __attribute__ ((unused));
int main () { return 0; }
" HAVE_UNUSED_ATTR)

SET (MACRO_UNUSED_NONE "#define UNUSED ")
IF (HAVE_UNUSED_ATTR)
    SET (MACRO_UNUSED
        "${MACRO_UNUSED_NONE} __attribute__ ((unused))")
ENDIF ()

