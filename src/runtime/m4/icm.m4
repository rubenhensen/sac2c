divert(-1)

define(`start_icm_definition', `define(`CAT_M4_NAME', $1)' `define(`CAT_M4_COUNT', 0)'
`#ifndef DEF_FILE_$1
`#'define DEF_FILE_$1'
)dnl
define(`end_icm_definition', def_cat
`#endif'
);

dnl define(_do_icm1, `ifdef(`M4_DEF_$1', `', `_def_nth($1)define(`M4_DEF_$1')')'_do_icm_choice($*))
define(_do_icm1, _do_icm_choice($*))

define(pat, `ifelse(`$#', `2', `patnopat($@)', `patpat($@)')')
define(patpat, ``#'define $1(_args($2) nt, ...) use_cat`'($1_, _pat(shift2($*)))(_args($2) nt ,`#'`#'__VA_ARGS__)')
define(patnopat, ``#'define $1(...) $1_(__VA_ARGS__)')
define(_pat, `ifelse(`$#', `1', `$1(nt)', `use_cat`'($1(nt), _pat(shift($*)))')')

define(_def_nth, ``#'define $1(nt, ...) use_cat`'($1_, use_cat`'(NT_SHP(nt), use_cat`'(NT_HID(nt), use_cat`'(NT_UNQ(nt), use_cat`'(NT_SCL(nt), use_cat`'(NT_TMP(nt), NT_TMP(nt)))))))(nt __VA_ARGS__)
')

define(_args, `ifelse($1, `0', `', `_args(eval($1-1)) arg$1,')')

define(_rule, `ifdef(M4_DEF_$1_`'cjoin(shift2($*)), `', `_ruleDef($@)')')
define(_ruleDef, `define(M4_DEF_$1_`'cjoin(shift2($*)))'`_ruleDefC($@)')
define(_ruleDefC, ``#'define $1_`'cjoin(shift2($*))(...) $2(__VA_ARGS__)')

define(rule, `foreach(`x', `_rule2(shift2($*))', `_rule($1,$2,x)
')')
define(_rule2, `ifelse(`$1', `', `', `_rule2more($@)')')
define(_rule2more, `ifelse(substr(`$1', `0', `1'), `*', `_rule2star($@)', `foreach(`y', `_rule2(shift($*))', `$1`'y,')')')
define(_rule2star, `star($1, _rule2(shift($*)))')

define(star, `foreach(`z', `_star($1)', `starNest(z, `shift($@)')')')
define(starNest, `foreach(`a', `$2', `$1`'a,')')
define(_star, `ifelse(`$1', `*SHP', `SCL, AKS, AKD, AUD, ___',
                      `$1', `*HID', `NHD, HID, ___',
                      `$1', `*UNQ', `NUQ, UNQ, ___',
                      `$1', `*REG', `INT, FLO, ___',
                      `$1', `*SCO', `SHR, GLO, ___',
                      `$1', `*USG', `PAR, ARG, P_A, NON, ___',
                      `errprint(`Unknown wild card "$1"
')')')

define(`foreach', `_foreach(`$1', `$3', $2)')
define(`_foreach', `ifelse(`$4', `', `_foreachLast($@)', `_foreachMore($@)')')
define(_foreachLast, `pushdef(`$1', `$3')`'$2`'`'popdef(`$1')')
define(_foreachMore, `pushdef(`$1', `$3')`'$2`'`'popdef(`$1')`'_foreach(`$1', `$2', shift3($@))')

define(`shift2', `shift(shift($@))')
define(`shift3', `shift(shift(shift($@)))')

define(`_do_icm_choice', `ifdef(`M4_DEF_$1_$2$3$4$5$6$7', `',`define(`M4_DEF_$1_$2$3$4$5$6$7')'``#'define $1_$2$3$4$5$6$7(...) $8(__VA_ARGS__)')')

define(`cjoin', `ifelse(`$#', `1', `$1', `$1`'cjoin(shift($*))')')

define(`use_cat', `define(`CAT_M4_COUNT', incr(CAT_M4_COUNT))'`CAT_M4_`'CAT_M4_NAME`'_`'CAT_M4_COUNT')dnl

define(`def_cat', `_def_cat(CAT_M4_COUNT)')
define(`_def_cat', `ifelse($1, 0, `', `__def_cat(`'CAT_M4_NAME`'_$1) _def_cat(eval($1-1)) ')')
define(`__def_cat2', `$1');
define(`__def_cat', `
#define CAT_M4_$1(x, y) xCAT_M4_$1(x, y)
#define xCAT_M4_$1(x, y) x##y
')


divert(0)
