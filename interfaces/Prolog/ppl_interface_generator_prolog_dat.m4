divert(-1)

# Classes to be implemented and C++ versions of these classes.
# These lines will be replaced by defintions
# generated by the configuration file.
# define(`m4_interface_class_names',
#   `Polyhedron, LP_Problem, Grid, BD_Shape_int8_t, BD_Shape_int32_t')
# define(`m4_cplusplus_class_names',
#   `Polyhedron, LP_Problem, Grid, BD_Shape<int8_t>, BD_Shape<int32_t>')
define(`m4_interface_class_names', `Polyhedron, LP_Problem, Grid')
define(`m4_cplusplus_class_names', `Polyhedron, LP_Problem, Grid')

# class_group
#
# There are three class groups:
# LP_Problem for LP_Problem class,
# GRID for all the grid-like classes,
# SHAPE for classes denoting subsets of a vector space that have some shape.
define(`m4_class_group',
  `ifelse(m4_class, LP_Problem, LP_Problem, m4_class, Grid, GRID, SHAPE)')

# class_super_group
#
# There are two class super_groups:
# PM for LP_Problem class,
# PD for classes defining some Domain based on sets of Points
# (ie grid and shape classes)
define(`m4_class_super_group',
  `ifelse(m4_class_group, LP_Problem, LP_Problem, POINTS)')

# m4_string_substitution_list
#
# returns a list of patterns (in lowercase) used as a basis
# of the procedure name and code schemas.
define(`m4_string_substitution_list',
`intopology_,
topology_,
represent,
dimension,
generator,
point,
constrainer,
state,
abovebelow,
maxmin,
embedproject,
affimage,
comparison,
binop,
binminop,
widenexp,
box,
describe'))

# num_class_widenexps
# class_widenexp
#
# The widening and extrapolation operators.
define(`num_Polyhedron_widenexps', 2)
define(`Polyhedron_widenexp1', `BHRZ03')
define(`Polyhedron_widenexp2', `H79')
define(`num_Grid_widenexps', 2)
define(`Grid_widenexp1', `congruence')
define(`Grid_widenexp2', `generator')
#define(`Grid_widenexp3', `BDHMZ06')
define(`Grid_widenexp3', `')
define(`num_BD_Shape_widenexps',3)
define(`BD_Shape_widenexp1',`CC76')
define(`BD_Shape_widenexp2',`BHMZ05')
define(`BD_Shape_widenexp3',`H79')
define(`num_Octagon_widenexps',1)
define(`Octagon_widenexp1',CH78)

# num_class_topology_s
# class_topology_
#
# Some classes can have a topology. The "_" only needed when topology exists.
define(`num_topology_s', 1)
define(`topology_1', `')
define(`num_Polyhedron_topology_s', 2)
define(`Polyhedron_topology_1', `C_')
define(`Polyhedron_topology_2', `NNC_')
define(`num_intopology_s', 1)
define(`intopology_1', `')
define(`num_Polyhedron_intopology_s', 2)
define(`Polyhedron_intopology_1', `C_')
define(`Polyhedron_intopology_2', `NNC_')

# num_class_boxs
# class_box
#
# The shape classes have bounding boxes while the grid classes also
# have covering boxes.
define(`num_boxs', 1)
define(`box1', `bounding_box')
define(`num_Grid_boxs', 2)
define(`Grid_box2', `covering_box')
define(`alt_Grid_box1', `shrink_bounding_box')
define(`alt_Grid_box2', `get_covering_box')

# num_class_dimensions
# class_dimension
#
#  space or affine dimensions
define(`num_dimensions', 2)
define(`dimension1', `space_dimension')
define(`dimension2', `affine_dimension')
define(`num_LP_Problem_dimensions', 1)

# num_class_generates
# class_generate
#
#  The different kinds of objects use to generate a class.
define(`num_generators', 1)
define(`generator1', `generator')
define(`Grid_generator1', `grid_generator')

# num_class_points
# class_point
#
#  The different kinds of objects use to generate a class.
define(`num_points', 1)
define(`point1', `point')
define(`Grid_point1', `grid_point')

# num_class_constrainers
# class_constrainer
#
#  The constrainer objects used to define a class.
define(`num_constrainers', 1)
define(`constrainer1', `constraint')
define(`Grid_constrainer1', `congruence')

# num_class_represents
# class_represent?
#
#  The different kinds of objects use to construct a class.
define(`num_represents', 2)
define(`represent1', `constraint')
define(`represent2', `generator')
define(`num_Grid_represents', 3)
define(`Grid_represent2', `generator')
define(`alt_Grid_represent2', `grid_generator')
define(`Grid_represent3', `congruence')
define(`num_BD_Shape_represents', 1)
define(`num_Octagon_represents', 1)


# num_class_describes
# class_describe
#
#  The different kinds of objects use to describe a class.
define(`num_describes', 2)
define(`describe1', `constraint')
define(`describe2', `generator')
define(`num_Grid_describe', 2)
define(`Grid_describe1', `congruence')
define(`Grid_describe2', `generator')
define(`alt_Grid_describe2', `grid_generator')
define(`num_BD_Shape_describes', 1)
define(`num_Octagon_describes', 1)

# num_class_states
# class_State
#
#  the "is" predicates
define(`num_states', 4)
define(`state1', `empty')
define(`state2', `universe')
define(`state3', `bounded')
define(`state4', `topologically_closed')
define(`num_Grid_states', 5)
define(`Grid_state4', `topologically_closed')
define(`Grid_state5', `discrete')

# num_class_bounds
# class_bounds
#
#  above or below
define(`num_abovebelows', 2)
define(`abovebelow1', `above')
define(`abovebelow2', `below')

# num_class_maxmins
# class_maxmin
#
#  Maximize or Minimize
define(`num_maxmins', 2)
define(`maxmin1', `maximize')
define(`maxmin2', `minimize')

# num_class_embedprojects
# class_embedproject
#
#  Embed or project
define(`num_embedprojects', 2)
define(`embedproject1', `and_embed')
define(`embedproject2', `and_project')

# num_class_affimages
# class_affimage
#
#  affine_image or affine_preimage
define(`num_affimages', 2)
define(`affimage1', `affine_image')
define(`affimage2', `affine_preimage')

# num_class_comparisons
# class_comparison
#
#  One object can be contained, strictly contained or disjoint in the other.
define(`num_comparisons', 3)
define(`comparison1', `contains')
define(`comparison2', `strictly_contains')
define(`comparison3', `is_disjoint_from')

# num_class_binops
# class_binop
#
#  The different kinds of binary operators.
define(`num_binops', 4)
define(`binop1', `intersection_assign')
define(`binop2', `join_assign')
define(`binop3', `difference_assign')
define(`binop4', `time_elapse_assign')
define(`Polyhedron_binop2', `poly_hull_assign')
define(`Polyhedron_binop3', `poly_difference_assign')
define(`num_BD_Shape_binops', 3)
define(`num_Octagon_binops', 3)

# num_class_binminops
# class_binminop
#
#  The different kinds of "and_minimize" binary operators.
define(`num_binminops', 2)
define(`binminop1', `binop1`'_and_minimize')
define(`Polyhedron_binminop2',  `Polyhedron_binop2`'_and_minimize')
define(`binminop2',  `binop2`'_and_minimize')

# Grid Generator and Generator_System classes  are called Grid_Generator
# and Grid_Generator_System while, for the rest of the domains, they are just
# called Generator and Generator_System.
# define(`FullName',
#       `ifelse(class, Grid,
#         ifelse(`$1', generator, Grid_Generator, capfirstletter(`$1')),
#           capfirstletter(`$1'))')

# Library predicate list.
define(`m4_library_predicate_list',
`ppl_version_major/1
ppl_version_minor/1
ppl_version_revision/1
ppl_version_beta/1
ppl_version/1
ppl_banner/1
ppl_max_space_dimension/1
ppl_Coefficient_is_bounded/0
ppl_Coefficient_max/1
ppl_Coefficient_min/1
ppl_initialize/0 nofail
ppl_finalize/0 nofail
ppl_set_timeout_exception_atom/1 nofail
ppl_timeout_exception_atom/1
ppl_set_timeout/1 nofail
ppl_reset_timeout/0 nofail
')

# m4_procedure_list
# This class using patterns wherever possible.
# Which classes the schema applies to is determined by the following codes:
# If code is POINTS = the point-domain classes ie grid and polyhedra classes;
#            All = all classes
#            SHAPE = the polyhedra-shape classes;
# A class name with an "X" in front means it is not included.
# Where "4CLASS4" is replaced by the class name, then that class only
# is applicable for that schema.
#
# Note that the code for the schema "<name>_code" must be defined
# in the ppl_prolog_icc.m4 file. The <name> must be exactly as written here.
#
define(`m4_procedure_list',
`ppl_new_4TOPOLOGY_44CLASS4_from_space_dimension/3 POINTS
ppl_new_4TOPOLOGY_44CLASS4_from_4INTOPOLOGY_44CLASS4/2 All
ppl_new_4TOPOLOGY_44CLASS4_from_4REPRESENT4s/2 POINTS
ppl_new_4TOPOLOGY_44CLASS4_from_4BOX4/2 POINTS
ppl_4CLASS4_swap/2 nofail All
ppl_delete_4CLASS4/1 nofail All
ppl_4CLASS4_4DIMENSION4/2 All
ppl_4CLASS4_get_4DESCRIBE4s/2 POINTS
ppl_4CLASS4_get_minimized_4DESCRIBE4s/2 POINTS
ppl_4CLASS4_relation_with_4DESCRIBE4/3 POINTS
ppl_4CLASS4_get_4BOX4/3 SHAPE
ppl_Grid_get_4BOX4/2
ppl_4CLASS4_is_4STATE4/1 POINTS
ppl_4CLASS4_topological_closure_assign/1 nofail POINTS
ppl_4CLASS4_bounds_from_4ABOVEBELOW4/2 POINTS
ppl_4CLASS4_4MAXMIN4/5 POINTS
ppl_4CLASS4_4MAXMIN4_with_point/6 POINTS
ppl_4CLASS4_4COMPARISON4_4CLASS4/2 POINTS
ppl_4CLASS4_equals_4CLASS4/2 POINTS
ppl_4CLASS4_OK/1 All
ppl_4CLASS4_add_4REPRESENT4/2 nofail POINTS
ppl_4CLASS4_add_4REPRESENT4_and_minimize/2 POINTS
ppl_4CLASS4_add_4REPRESENT4s/2 nofail POINTS
ppl_4CLASS4_add_4REPRESENT4s_and_minimize/2 POINTS
ppl_4CLASS4_4BINOP4/2 nofail POINTS
ppl_4CLASS4_4BINMINOP4/2 POINTS
ppl_4CLASS4_4AFFIMAGE4/4 nofail POINTS
ppl_4CLASS4_bounded_4AFFIMAGE4/5 nofail SHAPE
ppl_4CLASS4_generalized_4AFFIMAGE4/5 SHAPE
ppl_4CLASS4_generalized_4AFFIMAGE4_lhs_rhs/4 SHAPE
ppl_Grid_generalized_4AFFIMAGE4/5
ppl_Grid_generalized_4AFFIMAGE4_lhs_rhs/4
ppl_4CLASS4_4WIDENEXP4_widening_assign_with_tokens/4 POINTS
ppl_4CLASS4_4WIDENEXP4_widening_assign/2 nofail POINTS
ppl_4CLASS4_limited_4WIDENEXP4_extrapolation_assign_with_tokens/5 POINTS
ppl_4CLASS4_limited_4WIDENEXP4_extrapolation_assign/3 nofail POINTS
ppl_4CLASS4_bounded_4WIDENEXP4_extrapolation_assign_with_tokens/5 SHAPE
ppl_4CLASS4_bounded_4WIDENEXP4_extrapolation_assign/3 nofail SHAPE
ppl_BD_Shape_CC76_narrowing_assign/2
ppl_4CLASS4_add_space_dimensions_and_project/2 nofail POINTS
ppl_4CLASS4_add_space_dimensions_and_embed/2 nofail POINTS
ppl_4CLASS4_concatenate_assign/2 nofail POINTS
ppl_4CLASS4_remove_space_dimensions/2 POINTS
ppl_4CLASS4_remove_higher_space_dimensions/2 nofail POINTS
ppl_4CLASS4_expand_space_dimension/3 nofail POINTS
ppl_4CLASS4_fold_space_dimensions/3  POINTS
ppl_4CLASS4_map_space_dimensions/2 POINTS
ppl_new_LP_Problem_trivial/1
ppl_new_LP_Problem/4
ppl_LP_Problem_constraints/2
ppl_LP_Problem_objective_function/2
ppl_LP_Problem_optimization_mode/2
ppl_LP_Problem_clear/1
ppl_LP_Problem_add_constraint/2
ppl_LP_Problem_add_constraints/2
ppl_LP_Problem_set_objective_function/2
ppl_LP_Problem_set_optimization_mode/2
ppl_LP_Problem_is_satisfiable/1
ppl_LP_Problem_solve/2
ppl_LP_Problem_feasible_point/2
ppl_LP_Problem_optimizing_point/2
ppl_LP_Problem_optimal_value/3
ppl_LP_Problem_evaluate_objective_function/4'
)
divert`'dnl

