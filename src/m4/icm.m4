divert(-1)

dnl icm( `SAC_BASE_MACRO_NAME', 
dnl      `spec', `spec', `spec', `SAC_TARGET_MACRO_NAME',
dnl      `CATnn', `argsbefore', `argsafter')
dnl
dnl This macro will expand the given SAC_BASE_MACRO_NAME to the
dnl target macro SAC_TARGET_MACRO_NAME according to the spec tag
dnl specifiers. Valid values for spec are:
dnl
dnl   AUD | AKD | AKS | SCL | *
dnl   NHD | HID | *
dnl   NUQ | UNQ | *
dnl   
dnl Where * will be expanded to all possibilities of that specifier.
dnl Specification may overlap, in which case only the first hit will
dnl take effect. Target macro names must be unique for all icm
dnl calls. CATnn is the level of the current CAT macro, which will be
dnl increased automatically (e.g. 8, 9, ...). Argsbefore and argsafter
dnl is the count of arguments before and after the nametuple argument.
dnl
dnl You have to make sure that all your calls to icm() together
dnl cover the entire spec space for the base macro name (ie that all
dnl tag combinations are covered).

define(`icm',
`ifelse(`$2', `*',
`icm(`$1', `SCL', `$3', `$4', `$5', `$6', `$7', `$8')
icm(`$1', `AKS', `$3', `$4', `$5', `$6', `$7', `$8')
icm(`$1', `AKD', `$3', `$4', `$5', `$6', `$7', `$8')
icm(`$1', `AUD', `$3', `$4', `$5', `$6', `$7', `$8')',
`$3', `*',
`icm(`$1', `$2', `NHD', `$4', `$5', `$6', `$7', `$8')
icm(`$1', `$2', `HID', `$4', `$5', `$6', `$7', `$8')',
`$4', `*',
`icm(`$1', `$2', `$3', `NUQ', `$5', `$6', `$7', `$8')
icm(`$1', `$2', `$3', `UNQ', `$5', `$6', `$7', `$8')',
`_do_icm1(`$1', `$2', `$3', `$4', `$5', `$6', `$7', `$8')')')

dnl This macro concatenates the shape element of the tag ($2)
dnl to the macro name, and then calls _do_icm2 with the
dnl remainder of the arguments.
define(`_do_icm1',
`#ifndef DEF_$1
`#'define DEF_$1
`#'define $1(extra_args(`0', `$7')`'ifelse(`$7', `0', `', `, ')var_NT`'ifelse(`$8', `0', `', `, ')extra_args(`$7', `eval($7+$8)'))  \
  CAT$6( $1___, NT_SHP( var_NT) \
  BuildArgs`'eval($7+$8+1)( \
extra_args(`0', `$7')`'ifelse(`$7', `0', `', `, ')var_NT`'ifelse(`$8', `0', `', `, ')extra_args(`$7', `eval($7+$8)'))  \
)
#endif

_do_icm2(`$1___$2', `$3', `$4', `$5', increase_cat($6), `$7', `$8')')

dnl This macro concatenates the hidden element of the tag ($2)
dnl to the macro name, and then calls _do_icm3 with the
dnl remainder of the arguments.

define(`_do_icm2',
`#ifndef DEF_$1
`#'define DEF_$1
`#'define $1(extra_args(`0', `$6')`'ifelse(`$6', `0', `', `, ')var_NT`'ifelse(`$7', `0', `', `, ')extra_args(`$6', `eval($6+$7)'))  \
  CAT$5( $1___, NT_HID( var_NT) \
  BuildArgs`'eval($6+$7+1)( \
extra_args(`0', `$6')`'ifelse(`$6', `0', `', `, ')var_NT`'ifelse(`$7', `0', `', `, ')extra_args(`$6', `eval($6+$7)'))  \
)
#endif

_do_icm3(`$1___$2', `$3', `$4', increase_cat($5), `$6', `$7')')

dnl This macro concatenates the unique element of the tag ($2)
dnl to the macro name, and then calls _do_icm4 with the
dnl remainder of the arguments.

define(`_do_icm3',
`#ifndef DEF_$1
`#'define DEF_$1
`#'define $1(extra_args(`0', `$5')`'ifelse(`$5', `0', `', `, ')var_NT`'ifelse(`$6', `0', `', `, ')extra_args(`$5', `eval($5+$6)'))  \
  CAT$4( $1___, NT_UNQ( var_NT) \
  BuildArgs`'eval($5+$6+1)( \
extra_args(`0', `$5')`'ifelse(`$5', `0', `', `, ')var_NT`'ifelse(`$6', `0', `', `, ')extra_args(`$5', `eval($5+$6)'))  \
)
#endif

_do_icm4(`$1___$2', `$3', `$5', `$6')')

dnl This macro simply defines macro $1 to macro $2.

define(`_do_icm4',
`#ifndef DEF_$1
`#'define DEF_$1
`#'define $1(extra_args(`0', `$3')`'ifelse(`$3', `0', `', `, ')var_NT`'ifelse(`$4', `0', `', `, ')extra_args(`$3', `eval($3+$4)'))  \
  $2( \
extra_args(`0', `$3')`'ifelse(`$3', `0', `', `, ')var_NT`'ifelse(`$4', `0', `', `, ')extra_args(`$3', `eval($3+$4)'))
#endif

')

dnl Increases counter for CATnn macro's.
define(`increase_cat', `incr(`$1')')

dnl Prints extra arguments before the basename, nametuple pair.
define(`extra_args',
`ifelse(eval($1 >= $2), `1', `', `arg$1`'ifelse(eval(incr($1) >= $2), `1', `', `, ')`'extra_args(incr($1), `$2')')')

divert(0)
