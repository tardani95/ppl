dnl This file generates ppl_prolog.icc.
dnl
dnl Include files defining macros that generate the non-fixed part.
include(`ppl_interface_generator_prolog_icc_code.m4')dnl
include(`ppl_interface_generator_common.m4')dnl
include(`ppl_interface_generator_prolog_dat.m4')dnl
dnl
divert(-1)dnl

dnl m4_add_term_to_class_handle_code(Class, CPP_Class)
dnl
dnl Adds the code to convert a term to a Class handle.
define(`m4_add_term_to_class_handle_code', `dnl
m4_replace_class_patterns($1, m4_term_to_class_handle_code)dnl
')

dnl m4_add_bop_assign_code(Class, CPP_Class)
dnl
dnl Adds the extra code used by the binary operators.
define(`m4_add_bop_assign_code', `dnl
m4_replace_class_patterns($1, bop_assign_code)dnl
')

dnl m4_add_widening_extrapolation_code(Class_Counter, Class_Kind)
dnl
dnl Adds the extra code used by the widening and extrapolation predicates.
define(`m4_add_widening_extrapolation_code', `dnl
define(`m4_exists_widenexp', `ifdef(`m4_$2_widenexp_replacement', 1, 0)')dnl
define(`m4_constrainer',
  `m4_ifndef(`m4_$2_constrainer_replacement', m4_constrainer_replacement)')dnl
ifelse(m4_exists_widenexp, 0, ,
    `patsubst(patsubst(
      m4_replace_class_patterns($1, widening_extrapolation_code),
           m4_pattern_delimiter`'UCONSTRAINER`'m4_pattern_delimiter,
             m4_capfirstletters(m4_constrainer)),
           m4_pattern_delimiter`'CONSTRAINER`'m4_pattern_delimiter,
             m4_constrainer)')dnl
undefine(`m4_constrainer')dnl
undefine(`m4_exists_widenexp')dnl
')

dnl m4_pre_extra_class_code(Class_Counter, Class_Kind)
dnl Prefix extra code for each class.
define(`m4_pre_extra_class_code', `dnl
m4_add_term_to_class_handle_code($1)`'dnl
m4_add_bop_assign_code($1)`'dnl
m4_add_widening_extrapolation_code($1, $2)`'dnl
')

divert`'dnl
dnl
dnl Output the fixed preamble.
include(`ppl_interface_generator_prolog_icc_preamble')
dnl
dnl Generate the non-fixed part of the file.
m4_all_classes_code`'dnl
dnl
dnl End of file generation.

