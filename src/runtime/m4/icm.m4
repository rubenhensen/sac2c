divert(-1)

dnl To add new tags or new values for tags update the define _star

dnl Definition of all * tags
dnl Return definition of given * tag 
dnl
dnl $1  * tag
define(_star, `ifelse(`$1', `*SHP', `SCL, AKS, AKD, AUD, ___',
                      `$1', `*HID', `NHD, HID, HNS, ___',
                      `$1', `*UNQ', `NUQ, UNQ, ___',
                      `$1', `*REG', `INT, FLO, ___',
                      `$1', `*SCO', `SHR, GLO, ___',
                      `$1', `*USG', `TPM, TPA, TAG, TPO, FTA, FPM, FPA, FPO, FAG, NON, ___',
                      `$1', `*BIT', `YES, NOT, ___',
                      `$1', `*DIS', `DIS, NDI, ___',
                      `$1', `*CBT', `INT, FLO, DOU, UCH, BOO, BYT, SHO, LON, LLO, UBY, USH, UIN, ULO, ULL, OTH, ___',
                      `errprint(`Unknown wild card "$1"
')')')

dnl ___ Undefined

dnl TPM Thread param
dnl TPA Thread param Thread arg
dnl TAG Thread arg
dnl TPO Thread out param
dnl FTA Funtion param thread arg
dnl FPM Funtion param
dnl FPA Funtion param function arg
dnl FPO Funtion out param
dnl FAG Funtion arg
dnl NON None of the above

dnl DIS Potentially distributed (allocated in DSM memory)
dnl NDI Not distributed

dnl CBT Declared type in generated C-code

dnl Start ifndef block
dnl Remember namespace for use with cat macros.
dnl
dnl $1  name space
define(`start_icm_definition', `define(`CAT_M4_NAME', $1)' `define(`CAT_M4_COUNT', 0)'
`#ifndef DEF_FILE_$1
`#'define DEF_FILE_$1'
)dnl

dnl Define CAT macros
dnl End of ifndef block
define(`end_icm_definition', def_cat
`#endif'
);

dnl Define a pattern for an icm
dnl 
dnl $1  icm name
dnl $2  num args before nt
dnl $3  num args after nt
dnl $4* patten
define(pat, `ifelse(`$#', `3', `patnopat($@)', `patpat($@)')')

dnl Define CPP macro to handle nt there is a pattern
dnl
dnl $1  icm name
dnl $2  num args before nt
dnl $3  num args after nt
dnl $4+ patten
define(patpat, ``#'define $1(_args($2) nt _args2($3)) use_cat`'($1_, patcat(shift3($*)))(_args($2) nt _args2($3))')

dnl Define CPP macro to handle nt there is NO pattern
dnl
dnl $1  icm name
dnl $2  num args before nt
define(patnopat, ``#'define $1(...) $1_(__VA_ARGS__)')

dnl Join part of the NT together using CPP macros to form a single
dnl string that can be used for a macro name
dnl
dnl $1+ nt accessors
define(patcat, `ifelse(`$#', `1', `$1(nt)', `use_cat`'($1(nt), patcat(shift($*)))')')

dnl Generate a list of arguments ending in ,
dnl arg1, arg2, ...,
dnl
dnl $1  Number of arguments to create
define(_args, `ifelse($1, `0', `', `_args(eval($1-1)) arg$1,')')

dnl Generate a list of arguments starting with , and with a different name
dnl to that of _args
dnl , gra1, gra2, ...
dnl
dnl $1  Number of arguments to create
define(_args2, `ifelse($1, `0', `', `_args2(eval($1-1)), gra$1')')

dnl Create CPP macro for this rule only if the CPP macro does not exist
dnl
dnl $1  source icm name
dnl $2  target icm name
dnl $3* tag patten
define(_rule, `ifdef(M4_DEF_$1_`'cjoin(shift2($*)), `', `_ruleDef($@)')')

dnl Defince CPP macro for this rule and remember that we have created this
dnl CPP macro
dnl
dnl $1  source icm name
dnl $2  target icm name
dnl $3* tag patten
define(_ruleDef, `define(M4_DEF_$1_`'cjoin(shift2($*)))'`_ruleDefC($@)')


dnl Defince CPP macro for this rule
dnl
dnl $1  source icm name
dnl $2  target icm name
dnl $3* tag patten
define(_ruleDefC, ``#'define $1_`'cjoin(shift2($*))(...) $2(__VA_ARGS__)')

dnl Define a CPP macro for each combination that this rule covers
dnl 
dnl $1  from icm
dnl $2  to icm
dnl $3+ list of lists of tags that this rule applies to
define(rule, `foreach(`x', `_rule2(shift2($*))', `_rule($1,$2,x)
')')

dnl Return a list of all possible tag combination defined by this patten
dnl
dnl $1* patten
define(_rule2, `ifelse(`$1', `', `', `_rule2more($@)')')

dnl Return a list of all possible tag combination defined by this patten
dnl 
dnl $1+ patten
define(_rule2more, `ifelse(substr(`$1', `0', `1'), `*', `_rule2star($@)', `foreach(`y', `_rule2(shift($*))', `$1`'y,')')')

dnl Return a list of all possible tag combination defined by this patten
dnl that starts with a * tag
dnl
dnl $1  star tag (*...)
dnl $2* rest of pattern
define(_rule2star, `star($1, _rule2(shift($*)))')

dnl Expand a star tag and produce the product of the * tag posible values and 
dnl the given tag values
dnl 
dnl $1  star tag
dnl $2* tag values list
define(star, `foreach(`z', `_star($1)', `starNest(z, `shift($@)')')')

dnl Join $1 with each item in $2
dnl
dnl $1  tag
dnl $2+ list of list of tags
define(starNest, `foreach(`a', `$2', `$1`'a,')')

dnl Implimentation of a foreach loop
dnl 
dnl $1  variable to be used in each iteration
dnl $2  list of values to use
dnl $3  code to run for each iteration
define(`foreach', `_foreach(`$1', `$3', $2)')

dnl Implement a foreach loop
dnl
dnl $1  variable to be used in each iteration
dnl $2  code to run for each iteration
dnl $3* values iterated over
define(`_foreach', `ifelse(`$4', `', `_foreachLast($@)', `_foreachMore($@)')')

dnl This is the last iteration of a foreach loop
dnl
dnl $1  variable to be used in this iteration
dnl $2  code to run
dnl $3  value to use in this iteration
define(_foreachLast, `pushdef(`$1', `$3')`'$2`'`'popdef(`$1')')

dnl Iterate through foreach loop
dnl
dnl $1  variable to be used in each iteration
dnl $2  code to run in each iteration
dnl $3  value to use in this iteration
dnl $4* values for future iterations
define(_foreachMore, `pushdef(`$1', `$3')`'$2`'`'popdef(`$1')`'_foreach(`$1', `$2', shift3($@))')

dnl Shift twice
dnl
dnl $1  value to through away
dnl $2  value to through away
dnl $3* value(s) to return
define(`shift2', `shift(shift($@))')

dnl Shift three times
dnl
dnl $1  value to through away
dnl $2  value to through away
dnl $3  value to through away
dnl $4* value(s) to return
define(`shift3', `shift(shift(shift($@)))')

dnl Join arguments together to form a string
dnl
dnl $1+ arguments to join
define(`cjoin', `ifelse(`$#', `1', `$1', `$1`'cjoin(shift($*))')')

dnl Return a new cat macro call for use and remember to define it later
define(`use_cat', `define(`CAT_M4_COUNT', incr(CAT_M4_COUNT))'`CAT_M4_`'CAT_M4_NAME`'_`'CAT_M4_COUNT')dnl

dnl Define all needed cat macros in the current name space
define(`def_cat', `_def_cat(CAT_M4_COUNT)')

dnl Define 0 or more cat macros
dnl
dnl $1  Number of cat macros to define
define(`_def_cat', `ifelse($1, 0, `', `__def_cat(`'CAT_M4_NAME`'_$1) _def_cat(eval($1-1)) ')')

dnl Define a cat macro
dnl
dnl $1  name of cat macro to define
define(`__def_cat', `
#define CAT_M4_$1(x, y) xCAT_M4_$1(x, y)
#define xCAT_M4_$1(x, y) x##y
')


divert(0)
