(**Interfaces file to define new excpeptions, new types and interfaces function to PPL*)

open Gmp

type linear_expression =
    Variable of int
  | Coefficient of Z.t
  | Unary_Plus of linear_expression
  | Unary_Minus of linear_expression
  | Plus of linear_expression * linear_expression
  | Minus of linear_expression * linear_expression
  | Times of Z.t * linear_expression

type linear_constraint =
    Less_Than of linear_expression * linear_expression
  | Less_Than_Or_Equal of linear_expression * linear_expression
  | Equal of linear_expression * linear_expression
  | Greater_Than of linear_expression * linear_expression
  | Greater_Than_Or_Equal of linear_expression * linear_expression

type linear_generator =
    Line of linear_expression
  | Ray of linear_expression
  | Point of linear_expression * Z.t
  | Closure_Point of linear_expression * Z.t

type linear_congruence = linear_expression * linear_expression * Z.t

type constraint_system = linear_constraint list

type generator_system = linear_generator list

type congruence_system = linear_congruence list

type polyhedron

external ppl_new_C_Polyhedron_from_space_dimension:
  int -> polyhedron = "ppl_new_C_Polyhedron_from_space_dimension"

external ppl_new_C_Polyhedron_from_constraint_system:
  constraint_system -> polyhedron = "ppl_new_C_Polyhedron_from_constraint_system"
external ppl_new_C_Polyhedron_from_generator_system:
  generator_system -> polyhedron = "ppl_new_C_Polyhedron_from_generator_system"

external ppl_new_C_Polyhedron_from_congruence_system:
  congruence_system -> polyhedron = "ppl_new_C_Polyhedron_from_congruence_system"
external ppl_Polyhedron_space_dimension:
  polyhedron -> int = "ppl_Polyhedron_space_dimension"

external ppl_Polyhedron_affine_dimension:
  polyhedron -> int = "ppl_Polyhedron_affine_dimension"

external ppl_Polyhedron_is_empty:
  polyhedron -> bool = "ppl_Polyhedron_is_empty"

external ppl_Polyhedron_is_universe:
  polyhedron -> bool = "ppl_Polyhedron_is_universe"

external ppl_Polyhedron_contains_interger_point:
  polyhedron -> bool = "ppl_Polyhedron_contains_integer_point"

external ppl_Polyhedron_is_topologically_closed:
  polyhedron -> bool = "ppl_Polyhedron_is_topologically_closed"

external ppl_Polyhedron_is_bounded:
  polyhedron -> bool = "ppl_Polyhedron_is_bounded"

external ppl_Polyhedron_bounds_from_above:
  polyhedron -> linear_expression -> bool = "ppl_Polyhedron_bounds_from_above"

external ppl_Polyhedron_bounds_from_below:
  polyhedron -> linear_expression -> bool = "ppl_Polyhedron_bounds_from_below"

external ppl_Polyhedron_add_constraint:
  polyhedron -> linear_constraint -> unit = "ppl_Polyhedron_add_constraint"

external ppl_Polyhedron_add_constraint_and_minimize:
  polyhedron -> linear_constraint -> unit
      = "ppl_Polyhedron_add_constraint_and_minimize"

external ppl_Polyhedron_add_constraints:
  polyhedron -> constraint_system -> unit = "ppl_Polyhedron_add_constraints"

external ppl_Polyhedron_add_constraints_and_minimize:
  polyhedron -> constraint_system -> unit
      = "ppl_Polyhedron_add_constraints_and_minimize"

external ppl_Polyhedron_add_generator:
  polyhedron -> linear_generator -> unit = "ppl_Polyhedron_add_generator"

external ppl_Polyhedron_add_generator_and_minimize:
  polyhedron -> linear_generator -> unit
      = "ppl_Polyhedron_add_generator_and_minimize"

external ppl_Polyhedron_add_generators:
  polyhedron -> generator_system -> unit = "ppl_Polyhedron_add_generators"

external ppl_Polyhedron_add_generators_and_minimize:
  polyhedron -> generator_system -> unit
      = "ppl_Polyhedron_add_generators_and_minimize"

external ppl_Polyhedron_add_congruences:
  polyhedron -> congruence_system -> unit
      = "ppl_Polyhedron_add_congruences"

external ppl_Polyhedron_is_disjoint_from:
  polyhedron -> polyhedron -> bool
      = "ppl_Polyhedron_is_disjoint_from"

external ppl_Polyhedron_contains:
  polyhedron -> polyhedron -> bool
      = "ppl_Polyhedron_contains"

external ppl_Polyhedron_intersection_assign_and_minimize:
   polyhedron -> polyhedron -> bool
       = "ppl_Polyhedron_intersection_assign_and_minimize"

external ppl_Polyhedron_intersection_assign:
   polyhedron -> polyhedron -> unit
       = "ppl_Polyhedron_intersection_assign"

external ppl_Polyhedron_poly_hull_assign_and_minimize:
    polyhedron -> polyhedron -> bool
	= "ppl_Polyhedron_poly_hull_assign_and_minimize"

external ppl_Polyhedron_poly_hull_assign:
    polyhedron -> polyhedron -> unit
	= "ppl_Polyhedron_poly_hull_assign"

external ppl_Polyhedron_upper_bound_assign:
    polyhedron -> polyhedron -> unit
	= "ppl_Polyhedron_upper_bound_assign"

external ppl_Polyhedron_poly_difference_assign:
    polyhedron -> polyhedron -> unit
	= "ppl_Polyhedron_poly_difference_assign"

external ppl_Polyhedron_difference_assign:
    polyhedron -> polyhedron -> unit
	= "ppl_Polyhedron_difference_assign"

external ppl_Polyhedron_time_elapse_assign:
    polyhedron -> polyhedron -> unit
	= "ppl_Polyhedron_time_elapse_assign"

external ppl_Polyhedron_concatenate_assign:
    polyhedron -> polyhedron -> unit
	= "ppl_Polyhedron_concatenate_assign"

external ppl_Polyhedron_add_space_dimensions_and_embed:
  polyhedron -> int -> unit = "ppl_Polyhedron_add_space_dimensions_and_embed"

external ppl_Polyhedron_add_space_dimensions_and_project:
  polyhedron -> int -> unit = "ppl_Polyhedron_add_space_dimensions_and_project"

external ppl_Polyhedron_remove_higher_space_dimensions:
  polyhedron -> int -> unit = "ppl_Polyhedron_remove_higher_space_dimensions"

external ppl_Polyhedron_space_dimension:
  polyhedron -> int = "ppl_Polyhedron_space_dimension"

external ppl_Polyhedron_constraints:
   polyhedron -> constraint_system = "ppl_Polyhedron_constraints"

external ppl_Polyhedron_minimized_constraints:
   polyhedron -> constraint_system = "ppl_Polyhedron_minimized_constraints"

external ppl_Polyhedron_generators:
   polyhedron -> generator_system = "ppl_Polyhedron_generators"

external ppl_Polyhedron_minimized_generators:
   polyhedron -> generator_system = "ppl_Polyhedron_minimized_generators"

external ppl_Polyhedron_congruences:
   polyhedron -> congruence_system = "ppl_Polyhedron_congruences"

external ppl_Polyhedron_minimized_congruences:
   polyhedron -> congruence_system = "ppl_Polyhedron_minimized_congruences"

external ppl_Polyhedron_affine_image:
  polyhedron -> int -> linear_expression -> Z.t -> unit
      = "ppl_Polyhedron_affine_image"

external ppl_Polyhedron_affine_preimage:
  polyhedron -> int -> linear_expression -> Z.t -> unit
      = "ppl_Polyhedron_affine_preimage"

external ppl_Polyhedron_bounded_affine_image:
  polyhedron -> int -> linear_expression -> linear_expression
      -> Z.t -> unit = "ppl_Polyhedron_bounded_affine_image"

external ppl_Polyhedron_bounded_affine_preimage:
  polyhedron -> int -> linear_expression -> linear_expression
      -> Z.t -> unit = "ppl_Polyhedron_bounded_affine_preimage"

external ppl_Polyhedron_BHRZ03_widening_assign:
 polyhedron -> polyhedron -> int -> int
     = "ppl_Polyhedron_BHRZ03_widening_assign"

external limited_BHRZ03_extrapolation_assign:
 polyhedron -> polyhedron -> constraint_system -> int -> int
     = "limited_BHRZ03_extrapolation_assign"

external bounded_BHRZ03_extrapolation_assign:
 polyhedron -> polyhedron -> constraint_system -> int -> int
     = "bounded_BHRZ03_extrapolation_assign"

external test_linear_expression:
  linear_expression -> unit = "test_linear_expression"

external test_linear_constraint:
  linear_constraint -> unit = "test_linear_constraint"

external test_linear_generator:
  linear_generator -> unit = "test_linear_generator"

external test_constraint_system:
  constraint_system -> unit = "test_constraint_system"

external test_generator_system:
  generator_system -> unit = "test_generator_system"
