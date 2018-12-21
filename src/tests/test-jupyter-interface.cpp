#include "gtest/gtest.h"
#include "regex.h"

#include <string>

extern "C" {
int jupyter_init (void);
char *jupyter_parse_from_string (const char *s);
int jupyter_finalize (void);

}


bool
parsed_ok (char * s, int status) {
    regex_t regex_status;
    regex_t regex_ret;

    (void)regcomp (&regex_status, ".*\"status\": \"ok\"", REG_NEWLINE);
    std::stringstream ss;
    ss << ".*\"ret\": " << status;
    (void)regcomp (&regex_ret, ss.str ().c_str (), REG_NEWLINE);
    bool r = !regexec (&regex_status, s, 0, NULL, 0)
             && !regexec (&regex_ret, s, 0, NULL, 0);
    regfree (&regex_status);
    regfree (&regex_ret);
    return r;
}


// This test verifies that when creating a function declaration of
// a simple function, the Print<icm-name> function succeeds.
TEST (JupyterInterface, Simple)
{
    char *s;

    jupyter_init ();
    
    s = jupyter_parse_from_string (std::string ("2 + 2").c_str ());
    ASSERT_TRUE (parsed_ok (s, 1));

    s = jupyter_parse_from_string (std::string ("a = 1; b = 2;").c_str ());
    ASSERT_TRUE (parsed_ok (s, 2));

    s = jupyter_parse_from_string (std::string ("int id (int x) { return x; }").c_str ());
    ASSERT_TRUE (parsed_ok (s, 3));

    s = jupyter_parse_from_string (std::string ("typedef int myint;").c_str ());
    ASSERT_TRUE (parsed_ok (s, 4));

    jupyter_finalize ();
}
