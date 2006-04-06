/* Octagon class implementation: non-inline template functions.
   Copyright (C) 2001-2006 Roberto Bagnara <bagnara@cs.unipr.it>

This file is part of the Parma Polyhedra Library (PPL).

The PPL is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

The PPL is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307, USA.

For the most up-to-date information see the Parma Polyhedra Library
site: http://www.cs.unipr.it/ppl/ . */

#ifndef PPL_Octagon_templates_hh
#define PPL_Octagon_templates_hh 1

#include "Poly_Con_Relation.defs.hh"
#include "Poly_Gen_Relation.defs.hh"
#include <cassert>
#include <vector>
#include <deque>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// FIXME: this is only to get access to
// Implementation::BD_Shapes::div_round_up().
#include "BD_Shape.defs.hh"

namespace Parma_Polyhedra_Library {

template <typename T>
void
Octagon<T>::add_constraint(const Constraint& c) {
  using Implementation::BD_Shapes::div_round_up;

  const dimension_type c_space_dim = c.space_dimension();
  // Dimension-compatibility check.
  if (c_space_dim > space_dim)
    throw_dimension_incompatible("add_constraint(c)", c);
  // Strict inequalities are not allowed.
  if (c.is_strict_inequality())
    throw_constraint_incompatible("add_constraint(c)");

  dimension_type num_vars = 0;
  dimension_type i = 0;
  dimension_type j = 0;
  Coefficient coeff;
  Coefficient term = c.inhomogeneous_term();
  // Constraints that are not octagonal differences are ignored.
  if (!extract_octagonal_difference(c, c_space_dim, num_vars, i, j, coeff, term))
    return;

  if (num_vars == 0) {
    // Dealing with a trivial constraint.
    if (c.inhomogeneous_term() < 0)
      status.set_empty();
    return;
  }

  // Select the cell to be modified for the "<=" part of constraint.
  typename OR_Matrix<N>::row_iterator k = matrix.row_begin() + i;
  typename OR_Matrix<N>::row_reference_type r = *k;
  N& r_j = r[j];
  // Set `coeff' to the absolute value of itself.
  if (coeff < 0)
    coeff = -coeff;

  bool changed = false;
  // Compute the bound for `r_j', rounding towards plus infinity.
  N d;
  div_round_up(d, term, coeff);
  changed = change(changed, r_j, d);

  if (c.is_equality()) {
    // Select the right row of the cell.
    (i%2 == 0) ? ++k : --k;

    typename OR_Matrix<N>::row_reference_type r1 = *k;
    // Select the right column of the cell.
    dimension_type h = coherent_index(j);

    N& r1_h = r1[h];
    N d;
    div_round_up(d, -term, coeff);
    changed = change(changed, r1_h, d);
  }

  // This method not preserve closure.
  if (changed && marked_strongly_closed())
    status.reset_strongly_closed();
  assert(OK());
}

template <typename T>
void
Octagon<T>::concatenate_assign(const Octagon<T>& y) {
  // If `y' is an empty 0-dim space octagon, let `*this' become empty.
  // If `y' is an universal 0-dim space octagon, we simply return.
  if (y.space_dim == 0) {
    if (y.marked_empty())
      set_empty();
    return;
  }

  // If `*this' is an empty 0-dim space octagon, then it is sufficient
  // to adjust the dimension of the vector space.
  if (space_dim == 0) {
    if (marked_empty())
      add_space_dimensions_and_embed(y.space_dim);
    else
      *this = y;
    return;
  }

  // This is the old number of rows in the matrix. It is equal to
  // the first index of columns to change.
  dimension_type onr = matrix.num_rows();
  // First we increase the space dimension of `*this' by adding
  // `y.space_dimension()' new dimensions.
  // The matrix for the new octagon is obtained
  // by leaving the old system of constraints in the upper left-hand side
  // and placing the constraints of `y' in the lower right-hand side.
  add_space_dimensions_and_embed(y.space_dim);
  typename OR_Matrix<N>::const_element_iterator y_it = y.matrix.element_begin();
  for(typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + onr,
	iend = matrix.row_end(); i != iend; ++i) {
    typename OR_Matrix<N>::row_reference_type r = *i;
    dimension_type rs_i = i.row_size();
    for (dimension_type j = onr; j < rs_i; ++j) {
      r[j] = *y_it;
      ++y_it;
    }
  }

  // The concatenation don't preserve the closure.
  if (marked_strongly_closed())
    status.reset_strongly_closed();
  assert(OK());
}

template <typename T>
bool
Octagon<T>::contains(const Octagon& y) const {
  // Dimension-compatibility check.
  if (space_dim != y.space_dim)
    throw_dimension_incompatible("contains(y)", y);

  // The zero-dimensional universe octagon contains any other
  // dimension-compatible octagon.
  // The zero-dimensional empty octagon only contains another
  // zero-dimensional empty octagon.
  if (space_dim == 0) {
    if (!marked_empty())
      return true;
    else
      return y.marked_empty();
  }

  // `y' needs to be transitively closed.
  y.strong_closure_assign();
  // An empty octagon is in any other dimension-compatible octagons.
  if (y.marked_empty())
    return true;

  // `*this' contains `y' if and only if every element of `*this'
  // is greater than or equal to the correspondent one of `y'.
  for (typename OR_Matrix<N>::const_element_iterator i = matrix.element_begin(),
	 j = y.matrix.element_begin(), iend = matrix.element_end(); i != iend; ++i, ++j)
    if (*i < *j)
      return false;
  return true;
}

template <typename T>
bool
Octagon<T>::is_universe() const {
  // An empty octagon isn't, of course, universe.
  if (marked_empty())
    return false;

  // If the octagon is non-empty and zero-dimensional,
  // then it is necessarily the universe octagon.
  if (space_dim == 0)
    return true;

  // An universe octagon can only contains trivial  constraints.
  for (typename OR_Matrix<N>::const_element_iterator i = matrix.element_begin(),
	 iend = matrix.element_end(); i != iend; ++i)
    if (!is_plus_infinity(*i))
      return false;

  return true;
}

// FIXME!!!
template <typename T>
bool
Octagon<T>::is_strong_coherent() const {
  // It is used only in Octagon<T>::OK() to check if a closed octagon
  // is also strong-coherent, as it must be.
  dimension_type num_rows = matrix.num_rows();

  // The strong-coherence is: for every indexes i and j
  // matrix[i][j] <= (matrix[i][ci] + matrix[cj][j])/2
  // where ci = i + 1, if i is even number or
  //       ci = i - 1, if i is odd.
  // Ditto for cj.
  for (dimension_type i = 0; i < num_rows; ++i) {
    dimension_type ci = coherent_index(i);
    typename OR_Matrix<N>::const_row_iterator iter = matrix.row_begin() + i;
    typename OR_Matrix<N>::const_row_reference_type r_i = *iter;
    const N& m_i_ci = r_i[ci];
    for (dimension_type j = 0; j < matrix.row_size(i); ++j) {
      if (i != j){
	dimension_type cj = coherent_index(j);
	N d;
	const N& m_cj_j = matrix[cj][j];
	// Assigns to `d':
	// plus_infinity,         if `m_i_ci' or/and `m_cj_j' are plus infinity;
	// (m_i_ci + m_cj_j)/2,   otherwise.
	if (is_plus_infinity(m_i_ci) ||
	    is_plus_infinity(m_cj_j))
	  d = PLUS_INFINITY;
	else {
	  // Compute (m_i_ci + m_cj_j)/2 into `d', rounding the result
	  // towards plus infinity.
	  N sum;
	  add_assign_r(sum, m_i_ci, m_cj_j, ROUND_UP);
	  div2exp_assign_r(d, sum, 1, ROUND_UP);
	}
	if (r_i[j] > d)
	  return false;
      }
    }
  }
  return true;
}

template <typename T>
bool
Octagon<T>::is_transitively_reduced() const {
  // An empty octagon is already transitively reduced.
  if (marked_empty())
    return true;

  Octagon x = *this;
  for (typename OR_Matrix<N>::const_row_iterator iter = matrix.row_begin(),
	 iend = matrix.row_end(); iter != iend; ++iter) {
    typename OR_Matrix<N>::const_row_reference_type m_i = *iter;
    dimension_type rs_i = iter.row_size();
    dimension_type i = iter.index();
    for (dimension_type j = 0; j < rs_i; ++j) {
      if (!is_plus_infinity(m_i[j])) {
	Octagon x_copy = *this;
	x_copy.matrix[i][j] = PLUS_INFINITY;
	if (x_copy == x)
	  return false;
      }
    }
  }
  // The octagon is just reduced.
  return true;
}

template <typename T>
Poly_Con_Relation
Octagon<T>::relation_with(const Constraint& c) const {
  dimension_type c_space_dim = c.space_dimension();

  // Dimension-compatibility check.
  if (c_space_dim > space_dim)
    throw_dimension_incompatible("relation_with(c)", c);

  // The closure needs to make explicit the implicit constraints.
  strong_closure_assign();

  if (marked_empty())
    return Poly_Con_Relation::saturates()
      && Poly_Con_Relation::is_included()
      && Poly_Con_Relation::is_disjoint();

  if (space_dim == 0) {
    // Trivially false zero-dimensional constraint.
    if ((c.is_equality() && c.inhomogeneous_term() != 0)
	|| (c.is_inequality() && c.inhomogeneous_term() < 0))
      return Poly_Con_Relation::is_disjoint();
    else if (c.is_strict_inequality() && c.inhomogeneous_term() == 0)
      // The constraint 0 > 0 implicitly defines the hyperplane 0 = 0;
      // thus, the zero-dimensional point also saturates it.
      return Poly_Con_Relation::saturates()
	&& Poly_Con_Relation::is_disjoint();

    // Trivially true zero-dimensional constraint.
    else if (c.is_equality() || c.inhomogeneous_term() == 0)
      return Poly_Con_Relation::saturates()
	&& Poly_Con_Relation::is_included();
    else
      // The zero-dimensional point saturates
      // neither the positivity constraint 1 >= 0,
      // nor the strict positivity constraint 1 > 0.
      return Poly_Con_Relation::is_included();
  }

  dimension_type num_vars = 0;
  dimension_type i = 0;
  dimension_type j = 0;
  Coefficient coeff;
  Coefficient term = c.inhomogeneous_term();
  // Constraints that are not octagonal differences are ignored.
  if (!extract_octagonal_difference(c, c_space_dim, num_vars, i, j, coeff, term))
    throw_constraint_incompatible("relation_with(c)");

  if (num_vars == 0) {
    // Dealing with a trivial constraint.
    switch (sgn(c.inhomogeneous_term())) {
    case -1:
      return Poly_Con_Relation::is_disjoint();
    case 0:
      if (c.is_strict_inequality())
	return Poly_Con_Relation::saturates()
	  && Poly_Con_Relation::is_disjoint();
      else
	return Poly_Con_Relation::saturates()
	  && Poly_Con_Relation::is_included();
    case 1:
      return Poly_Con_Relation::is_included();
    }
  }

  // Select the cell to be checked for the "<=" part of constraint.
  typename OR_Matrix<N>::const_row_iterator k = matrix.row_begin() + i;
  typename OR_Matrix<N>::const_row_reference_type r = *k;
  const N& r_j = r[j];
  // Set `coeff' to the absolute value of itself.
  if (coeff < 0)
    coeff = -coeff;

  // Compute the bound for `r_j', rounding the result towards plus infinity.
  N d;
  div_round_up(d, term, coeff);
  N d1;
  div_round_up(d1, -term, coeff);

  // Select the cell to be checked for the ">=" part of constraint.
  // Select the right row of the cell.
  (i%2 == 0) ? ++k : --k;
    typename OR_Matrix<N>::const_row_reference_type r1 = *k;
  // Select the right column of the cell.
  dimension_type h = coherent_index(j);
  const N& r1_h = r1[h];

  switch (c.type()) {
  case Constraint::EQUALITY:
    if (d == r_j && d1 == r1_h)
      return Poly_Con_Relation::saturates()
	&& Poly_Con_Relation::is_included();
    else if (d > r_j && d1 < r1_h)
      return Poly_Con_Relation::is_disjoint();
    else
      return Poly_Con_Relation::strictly_intersects();
  case Constraint::NONSTRICT_INEQUALITY:
    if (d >= r_j && d1 >= r1_h)
      return Poly_Con_Relation::saturates()
	&& Poly_Con_Relation::is_included();
    else if (d >= r_j)
      return Poly_Con_Relation::is_included();
    else if (d < r_j && d1 > r1_h)
      return Poly_Con_Relation::is_disjoint();
    else
      return Poly_Con_Relation::strictly_intersects();
  case Constraint::STRICT_INEQUALITY:
    if (d >= r_j && d1 >= r1_h)
      return Poly_Con_Relation::saturates()
	&& Poly_Con_Relation::is_disjoint();
    else if (d > r_j)
      return Poly_Con_Relation::is_included();
    else if (d <= r_j && d1 >= r1_h)
      return Poly_Con_Relation::is_disjoint();
    else
      return Poly_Con_Relation::strictly_intersects();
  }
  // Quiet a compiler warning: this program point is unreachable.
  throw std::runtime_error("PPL internal error");
}

#if 0
template <typename T>
Poly_Con_Relation
Octagon<T>::relation_with(const Constraint& c) const {
  using namespace IO_Operators;
  C_Polyhedron ph(constraints());
  Poly_Con_Relation p_ret = ph.relation_with(c);
  Poly_Con_Relation o_ret = this->real_relation_with(c);
  if (p_ret != o_ret) {
    std::cout << "Relation of" <<std::endl
	      << *this << std::endl
	      << "a.k.a." << std::endl
	      << ph << std::endl
	      << "with" << std::endl
	      << c << std::endl
	      << "gives " << o_ret << " with Octagon" << std::endl
	      << "and " << p_ret << " with C_Polyhedron" << std::endl;
  }
  return o_ret;
}
#endif

template <typename T>
Poly_Gen_Relation
Octagon<T>::relation_with(const Generator& g) const {
  dimension_type g_space_dim = g.space_dimension();

  // Dimension-compatibility check.
  if (space_dim < g_space_dim)
    throw_dimension_incompatible("relation_with(g)", g);

  // The empty octagon cannot subsume a generator.
  if (marked_empty())
    return Poly_Gen_Relation::nothing();

  // A universe octagon in a zero-dimensional space subsumes
  // all the generators of a zero-dimensional space.
  if (space_dim == 0)
    return Poly_Gen_Relation::subsumes();


  const bool is_line = g.is_line();

  // The relation between the octagon and the given generator is obtained
  // checking if the generator satisfies all the constraints in the octagon.
  // To check if the generator satisfies all the constraints it's enough
  // studying the sign of the scalar product between the generator and
  // all the contraints in the octagon.

  // We find in `*this' all the constraints.
  for (typename OR_Matrix<N>::const_row_iterator i_iter = matrix.row_begin(),
	 i_end = matrix.row_end(); i_iter != i_end; i_iter += 2) {
    dimension_type i = i_iter.index();
    typename OR_Matrix<N>::const_row_reference_type r_i = *i_iter;
    typename OR_Matrix<N>::const_row_reference_type r_ii = *(i_iter+1);
    const N& c_i_ii = r_i[i+1];
    const N& c_ii_i = r_ii[i];
    // We have the unary constraints.
    const Variable x(i/2);
    const bool dimension_incompatible = x.space_dimension() > g_space_dim;
    N negated_c_i_ii;
    const bool is_unary_equality
      = assign_neg(negated_c_i_ii, c_i_ii, ROUND_IGNORE) == V_EQ
      && negated_c_i_ii == c_ii_i;
    if (is_unary_equality) {
      // The constraint has form ax = b.
      // To satisfy the constraint it's necessary that the scalar product
      // is not zero.It happens when the coefficient of the variable 'x'
      // in the generator is not zero, because the scalar
      // product has the form:
      // 'a * x_i' where x_i = g.coefficient(x).
      if (!dimension_incompatible && g.coefficient(x) != 0)
	return Poly_Gen_Relation::nothing();
    }
    // We have 0, 1 or 2 inequality constraints.
    else {
      if (!is_plus_infinity(c_i_ii)) {
	// The constraint has form -ax <= b.
	// If the generator is a line it's necessary to check if
	// the scalar product is not zero.
	if (is_line && (!dimension_incompatible && g.coefficient(x) != 0))
	  return Poly_Gen_Relation::nothing();
	else
	  // If the generator is not a line it's necessary to check
	  // that the scalar product sign is not negative and
	  // it happens when the coefficient of the variable 'x'
	  // in the generator is not negative, because the scalar
	  // product has the form:
	  // 'a * x_i' where x_i = g.coefficient(x).
	  if (g.coefficient(x) < 0)
	    return Poly_Gen_Relation::nothing();
      }
      if (!is_plus_infinity(c_ii_i)) {
	// The constraint has form ax <= b.
	if (is_line && (!dimension_incompatible && g.coefficient(x) != 0))
	  return Poly_Gen_Relation::nothing();
	else
	  // If the generator is not a line it's necessary to check
	  // that the scalar product sign is not negative and
	  // it happens when the coefficient of the variable 'x'
	  // in the generator is not positive, because the scalar
	  // product has the form:
	  // '-a * x_i' where x_i = g.coefficient(x).
	  if (g.coefficient(x) > 0)
	    return Poly_Gen_Relation::nothing();
      }
    }
  }

  // We have the binary constraints.
  for (typename OR_Matrix<N>::const_row_iterator i_iter = matrix.row_begin(),
	 i_end = matrix.row_end(); i_iter != i_end; i_iter += 2) {
    dimension_type i = i_iter.index();
    typename OR_Matrix<N>::const_row_reference_type r_i = *i_iter;
    typename OR_Matrix<N>::const_row_reference_type r_ii = *(i_iter+1);
    for (dimension_type j = 0; j < i; j += 2) {
      const N& c_i_j = r_i[j];
      const N& c_ii_jj = r_ii[j+1];
      const N& c_ii_j = r_ii[j];
      const N& c_i_jj = r_i[j+1];
      const Variable x(j/2);
      const Variable y(i/2);
      const bool x_dimension_incompatible = x.space_dimension() > g_space_dim;
      const bool y_dimension_incompatible = y.space_dimension() > g_space_dim;
      const bool is_trivial_zero = (x_dimension_incompatible && g.coefficient(y) == 0)
	|| (y_dimension_incompatible && g.coefficient(x) == 0)
	|| (x_dimension_incompatible && y_dimension_incompatible);
      // FIXME! Find better names.
      N negated_c_ii_jj;
      const bool is_binary_equality
	= assign_neg(negated_c_ii_jj, c_ii_jj, ROUND_IGNORE) == V_EQ
	&& negated_c_ii_jj == c_i_j;
      N negated_c_i_jj;
      const bool is_a_binary_equality
	= assign_neg(negated_c_i_jj, c_i_jj, ROUND_IGNORE) == V_EQ
	&& negated_c_i_jj == c_ii_j;

      Coefficient g_coefficient_y;

      if (is_binary_equality || is_a_binary_equality) {
	// The scalar product has the form
	// 'a * y_i - a * x_j' (or  'a * x_j + a * y_i')
	// where y_i is equal to g.coefficient(y) or -g.coefficient(y)
	// and x_j = g.coefficient(x).
	// It is not zero when both the coefficients of the
	// variables x and y are not zero or when these coefficients
	// are not equals.
	if (is_a_binary_equality)
	  // The constraint has form ax + ay = b.
	  g_coefficient_y = -g.coefficient(y);
	else
	  // The constraint has form ax - ay = b.
 	  g_coefficient_y = g.coefficient(y);
	if (!is_trivial_zero && g.coefficient(x) != g_coefficient_y)
 	  return Poly_Gen_Relation::nothing();
      }
      else
	if (!is_plus_infinity(c_i_j) || !is_plus_infinity(c_ii_j)) {
	  if (!is_plus_infinity(c_ii_j))
	    // The constraint has form ax + ay <= b.
	    g_coefficient_y = -g.coefficient(y);
	  else
	    // The constraint has form ax - ay <= b.
	    g_coefficient_y = g.coefficient(y);
	  // The scalar product has the form
	  // '-a * x_j + a* y_i' (or '-a * x_j - a * y_i').
	  if (is_line
	      && !is_trivial_zero
	      && g.coefficient(x) != g_coefficient_y)
	    return Poly_Gen_Relation::nothing();
	  // The product scalar sign is not negative when the
	  // coefficient of the variable y is strictly smaller
	  // than the coefficient of the variable x in the generator.
	  else if (g_coefficient_y < g.coefficient(x))
	    return Poly_Gen_Relation::nothing();
	}
	else
	  if (!is_plus_infinity(c_ii_jj) || !is_plus_infinity(c_i_jj)) {
	    if (!is_plus_infinity(c_i_jj))
	      // The constraint has form -ax - ay <= b.
	      g_coefficient_y = -g.coefficient(y);
	    else
	      // The constraint has form ay - ax <= b.
	      g_coefficient_y = g.coefficient(y);
	    // The scalar product has the form
	    // 'a * x_j - a* y_i' (or 'a * x_j + a* y_i').
	    if (is_line
		&& !is_trivial_zero
		&& g.coefficient(x) != g_coefficient_y)
	      return Poly_Gen_Relation::nothing();
	    // The product scalar sign is not negative when the
	    // coefficient of the variable x is strictly smaller
	    // than the coefficient of the variable y in the generator.
	    else if (g.coefficient(x) < g_coefficient_y)
	      return Poly_Gen_Relation::nothing();
	  }
    }
  }
  // If this point is reached the constraint 'g' satisfies
  // all the constraints in the octagon.
  return Poly_Gen_Relation::subsumes();
}

template <typename T>
void
Octagon<T>::strong_closure_assign3() const {
  // Do something only if necessary.
  if (marked_empty() || marked_strongly_closed())
    return;

  // Zero-dimensional octagons are necessarily strongly closed.
  if (space_dim == 0)
    return;

#ifndef NDEBUG
  Octagon x_copy = *this;
  x_copy.strong_closure_assign1();
#endif

  // Even though the octagon will not change, its internal representation
  // is going to be modified by the closure algorithm.
  Octagon& x = const_cast<Octagon<T>&>(*this);

  // Fill the main diagonal with zeros.
  for (typename OR_Matrix<N>::row_iterator i = x.matrix.row_begin(),
	 m_end = x.matrix.row_end(); i != m_end; ++i) {
    typename OR_Matrix<N>::row_reference_type r = *i;
    assert(is_plus_infinity(r[i.index()]));
    assign_r(r[i.index()], 0, ROUND_NOT_NEEDED);
  }

  // This algorithm is given by two step, first one is the `closure' that
  // uses a modification of the Floyd-Warshall algorithm,
  // the second one is the `strong-coherence'.
  // It is important to note that after the strong-coherence,
  // the octagon is closed yet.
  // The step of closure is divided in three cases that describe
  // how the three indexes `i', `j' and `k' (used in this algorithm)
  // interdepende.
  // We recall that, given an index `h', we indicate with `ch'
  // the index such that:
  // ch = h + 1, if h is even number or
  // ch = h - 1, if h is odd.

  // Step 1: closure.
  for (typename OR_Matrix<N>::row_iterator k_iter = x.matrix.row_begin(),
	 k_end = x.matrix.row_end(); k_iter != k_end; ++k_iter) {
    typename OR_Matrix<N>::row_reference_type x_k = *k_iter;
    dimension_type rs_k = k_iter.row_size();
    dimension_type k = k_iter.index();
    typename OR_Matrix<N>::row_iterator ck_iter = (k%2) ? k_iter-1 : k_iter+1;
    typename OR_Matrix<N>::row_reference_type x_ck = *ck_iter;
    dimension_type ck = ck_iter.index();
    // First case: i < rs_k e rs_k > j.
    // Indicated with `m' the matrix of `*this', in this case
    // the step is given the following way:
    // m_i_j = min(m_i_j,
    //             m_ck_ci + m_k_j,
    //             m_k_ci + m_ck_j).
    for (dimension_type i = 0; i < rs_k; ++i) {
      dimension_type ci = coherent_index(i);
      N& x_ck_ci = x_ck[ci];
      N& x_k_ci = x_k[ci];
      if (!is_plus_infinity(x_ck_ci) || !is_plus_infinity(x_k_ci)) {
	typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin()+i;
	typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
	dimension_type rs_i = i_iter.row_size();
	for (dimension_type j = 0; j < rs_i; ++j) {
	  N& x_k_j = x_k[j];
	  N& x_ck_j = x_ck[j];
#if 1
	  if (!is_plus_infinity(x_ck_j) || !is_plus_infinity(x_k_j)) {
	    N sub_sum1;
	    add_assign_r(sub_sum1, x_ck_ci, x_k_j, ROUND_UP);
	    N sub_sum2;
	    add_assign_r(sub_sum2, x_k_ci, x_ck_j, ROUND_UP);
	    Implementation::Octagons::assign_min(x_i[j], sub_sum1, sub_sum2);
	  }
#else
	  // Try to avoid computing unnecessaty additions.
	  N& x_i_j = x_i[j];
	  if (!is_plus_infinity(x_ck_ci) && !is_plus_infinity(x_k_j)) {
	    N sub_sum3;
	    add_assign_r(sub_sum3, x_ck_ci, x_k_j, ROUND_UP);
	    Implementation::Octagons::assign_min(x_i_j, sub_sum3);
	  }
	  if(!is_plus_infinity(x_k_ci) && !is_plus_infinity(x_ck_j)) {
	    N sub_sum4;
	    add_assign_r(sub_sum4, x_k_ci, x_ck_j, ROUND_UP);
	    Implementation::Octagons::assign_min(x_i_j, sub_sum4);
	  }

#endif
	}
      }
    }
    // Second case: i >= rs_k.
    for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin()+rs_k,
	   iend = x.matrix.row_end(); i_iter != iend; ++i_iter) {
      typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
      dimension_type rs_i = i_iter.row_size();
      N& x_i_k = x_i[k];
      N& x_i_ck = x_i[ck];
      if (!is_plus_infinity(x_i_k) || !is_plus_infinity(x_i_ck)) {
	// And j < rs_k.
	// the step is given the following way:
	// m_i_j = min(m_i_j,
	//             m_i_k + m_k_j,
	//             m_i_ck + m_ck_j).
	for (dimension_type j = 0; j < rs_k; ++j) {
	  N& x_k_j = x_k[j];
	  N& x_ck_j = x_ck[j];
#if 1
	  if (!is_plus_infinity(x_ck_j) || !is_plus_infinity(x_k_j)) {
	    N sub_sum1;
	    add_assign_r(sub_sum1, x_i_k, x_k_j, ROUND_UP);
	    N sub_sum2;
	    add_assign_r(sub_sum1, x_i_ck, x_ck_j, ROUND_UP);
	    Implementation::Octagons::assign_min(x_i[j], sub_sum1, sub_sum2);
	  }
#else
	  // Try to avoid computing unnecessaty additions.
	    N& x_i_j = x_i[j];
	    if (!is_plus_infinity(x_i_k) && !is_plus_infinity(x_k_j)) {
	      N sub_sum3;
	      add_assign_r(sub_sum3, x_i_k, x_k_j, ROUND_UP);
	      Implementation::Octagons::assign_min(x_i_j, sub_sum3);
	    }
	    if(!is_plus_infinity(x_i_ck) && !is_plus_infinity(x_ck_j)) {
	      N sub_sum4;
	      add_assign_r(sub_sum4, x_i_ck, x_ck_j, ROUND_UP);
	      Implementation::Octagons::assign_min(x_i_j, sub_sum4);
	    }
#endif

	}

	// And j >= rs_k.
	// the step is given the following way:
	// m_i_j = min(m_i_j,
	//             m_i_k + m_cj_ck,
	//             m_i_ck + m_cj_k).
	for (dimension_type j = rs_k; j < rs_i; ++j) {
	  dimension_type cj = coherent_index(j);
	  dimension_type ck = coherent_index(k);
	  typename OR_Matrix<N>::row_reference_type x_cj = *(x.matrix.row_begin()+cj);
	  N& x_cj_k = x_cj[k];
	  N& x_cj_ck = x_cj[ck];
#if 1
	  if (!is_plus_infinity(x_cj_k) || !is_plus_infinity(x_cj_ck)) {
	    N sub_sum1;
	    add_assign_r(sub_sum1, x_i_k, x_cj_ck, ROUND_UP);
	    N sub_sum2;
	    add_assign_r(sub_sum1, x_i_ck, x_cj_k, ROUND_UP);
	    Implementation::Octagons::assign_min(x_i[j], sub_sum1, sub_sum2);
	  }
#else
	  // Try to avoid computing unnecessaty additions.
	    N& x_i_j = x_i[j];
	    if (!is_plus_infinity(x_i_k) && !is_plus_infinity(x_cj_ck)) {
	      N sub_sum3;
	      add_assign_r(sub_sum3, x_i_k, x_cj_ck, ROUND_UP);
	      Implementation::Octagons::assign_min(x_i_j, sub_sum3);
	    }
	    if(!is_plus_infinity(x_i_ck) && !is_plus_infinity(x_cj_k)) {
	      N sub_sum4;
	      add_assign_r(sub_sum4, x_i_ck, x_cj_k, ROUND_UP);
	      Implementation::Octagons::assign_min(x_i_j, sub_sum4);
	    }
#endif

	}
      }
    }
  }

  // Check for emptyness: the octagon is empty if and only if there is a
  // negative value in the main diagonal.
  for (typename OR_Matrix<N>::row_iterator i = x.matrix.row_begin(),
	 m_end = x.matrix.row_end(); i != m_end; ++i) {
    typename OR_Matrix<N>::row_reference_type r = *i;
    N& x_i_i = r[i.index()];
    if (x_i_i < 0) {
      x.status.set_empty();
      return;
    }
    else {
      assert(x_i_i == 0);
      // Restore PLUS_INFINITY on the main diagonal.
      x_i_i = PLUS_INFINITY;
    }
  }

  // The octagon is not empty and it is now strongly closed.
  x.status.set_strongly_closed();

  // Step 2: we enforce the strong coherence.
  // The strong-coherence is: for every indexes i and j
  // m_i_j <= (m_i_ci + m_cj_j)/2
  // where ci = i + 1, if i is even number or
  //       ci = i - 1, if i is odd.
  // Ditto for cj.
  for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin(),
	 i_end = x.matrix.row_end(); i_iter != i_end; ++i_iter) {
    typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
    dimension_type rs_i = i_iter.row_size();
    dimension_type i = i_iter.index();
    dimension_type ci = coherent_index(i);
    N& x_i_ci = x_i[ci];
    // Avoid to do unnecessary sums.
    if (!is_plus_infinity(x_i_ci))
      for (dimension_type j = 0; j < rs_i; ++j) {
	if (i != j) {
	  dimension_type cj = coherent_index(j);
	  N& x_cj_j = x.matrix[cj][j];
	  if (!is_plus_infinity(x_cj_j)) {
	    N& x_i_j = x_i[j];
	    N d;
	    N sub_sum;
	    add_assign_r(sub_sum, x_i_ci, x_cj_j, ROUND_UP);
	    div2exp_assign_r(d, sub_sum, 1, ROUND_UP);
	    Implementation::Octagons::assign_min(x_i_j, d);
	  }
	}
      }
  }

#ifndef NDEBUG
  assert(matrix == x_copy.matrix);
#endif

}

template <typename T>
void
Octagon<T>::strong_closure_assign1() const {
  using Implementation::Octagons::assign_min;

  // Do something only if necessary.
  if (marked_empty() || marked_strongly_closed())
    return;

  // Zero-dimensional octagons are necessarily strongly closed.
  if (space_dim == 0)
    return;

  // Even though the octagon will not change, its internal representation
  // is going to be modified by the closure algorithm.
  Octagon& x = const_cast<Octagon<T>&>(*this);

  // We need the auxiliary matrix, 'cause, when we interleave the two steps,
  // introduced below, we choose the right element between that element we obtain
  // by closure step and that element we obtain by successive coherence step.
  OR_Matrix<N> aux(2*space_dim);

  typename OR_Matrix<N>::row_iterator m_end = x.matrix.row_end();
  // Fill the main diagonal with zeros.
  for (typename OR_Matrix<N>::row_iterator i = x.matrix.row_begin();
       i != m_end; ++i) {
    typename OR_Matrix<N>::row_reference_type r = *i;
    assert(is_plus_infinity(r[i.index()]));
    assign_r(r[i.index()], 0, ROUND_NOT_NEEDED);
  }

  // This algorithm is a modification of the Floyd-Warshall algorithm for
  // the pseudo-triangular matrix. It consists for every k to interleave
  // a step of closure with a step of strong coherence.
  // The step of closure is divided in three cases that describe
  // how the three indexes `i', `j' and `k' (used in this algorithm)
  // interdepende.
  // We recall that, given an index `h', we indicate with `ch'
  // the index such that:
  // ch = h + 1, if h is even number or
  // ch = h - 1, if h is odd.
  for (typename OR_Matrix<N>::row_iterator k_iter = x.matrix.row_begin();
       k_iter != m_end; k_iter += 2) {
    typename OR_Matrix<N>::row_reference_type x_k = *k_iter;
    typename OR_Matrix<N>::row_reference_type x_kk = *(k_iter+1);
    dimension_type k = k_iter.index();
    N& x_k_kk = x_k[k+1];
    N& x_kk_k = x_kk[k];
    dimension_type rs_k = k_iter.row_size();

    bool check_sum;
    // Step1: closure.
    // First case: i < rs_k e rs_k > j.
    // Indicated with `m' the matrix of `*this', in this case
    // the step is given the following way:
    // m_i_j = min(m_i_j,
    //             m_kk_ci + m_k_j,
    //             m_k_ci + m_kk_j,
    //             m_kk_ci + m_k_kk + m_kk_j,
    //             m_k_ci + m_kk_k + m_k_j).
    for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin(),
	   a_iter = aux.row_begin(), i_end = i_iter+rs_k;
	 i_iter != i_end; ++i_iter, ++a_iter) {
      typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
      typename OR_Matrix<N>::row_reference_type aux_i = *a_iter;
      dimension_type rs_i = i_iter.row_size();
      dimension_type ci = coherent_index(i_iter.index());
      N& x_k_ci = x_k[ci];
      N& x_kk_ci = x_kk[ci];
      check_sum = !is_plus_infinity(x_kk_ci) || !is_plus_infinity(x_k_ci);
      for (dimension_type j = 0; j < rs_i; ++j) {
	N& x_k_j = x_k[j];
	N& x_kk_j = x_kk[j];
	// Now we compute the sums for the minimum of the closure.
	N& aux_i_j = aux_i[j];
	aux_i_j = x_i[j];
	// Avoid to do unnecessary sums.
	if (check_sum) {
	  N sub_sum1;
	  add_assign_r(sub_sum1, x_kk_ci, x_k_j, ROUND_UP);
	  N sub_sum2;
	  add_assign_r(sub_sum2, x_k_ci, x_kk_j, ROUND_UP);
	  N sub_sub_sum3;
	  add_assign_r(sub_sub_sum3, x_kk_ci, x_k_kk, ROUND_UP);
	  N sub_sum3;
	  add_assign_r(sub_sum3, sub_sub_sum3, x_kk_j, ROUND_UP);
	  N sub_sub_sum4;
	  add_assign_r(sub_sub_sum4, x_k_ci, x_kk_k, ROUND_UP);
	  N sub_sum4;
	  add_assign_r(sub_sum4, sub_sub_sum4, x_k_j, ROUND_UP);
	  assign_min(aux_i_j, sub_sum1, sub_sum2, sub_sum3, sub_sum4);
	}
      }
    }

    // Second case: i >= rs_k.
    for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin()+rs_k,
	   a_iter = aux.row_begin()+rs_k; i_iter != m_end; ++i_iter, ++a_iter) {
      typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
      typename OR_Matrix<N>::row_reference_type aux_i = *a_iter;
      N& x_i_k = x_i[k];
      N& x_i_kk = x_i[k+1];
      check_sum = !is_plus_infinity(x_i_kk) || !is_plus_infinity(x_i_k);
      // And j < rs_k.
      // the step is given the following way:
      // m_i_j = min(m_i_j,
      //             m_i_k + m_k_j,
      //             m_i_kk + m_kk_j,
      //             m_i_k + m_k_kk + m_kk_j,
      //             m_i_kk + m_kk_k + m_k_j).
      for (dimension_type j = 0; j < rs_k; ++j) {
	N& x_k_j = x_k[j];
	N& x_kk_j = x_kk[j];
	// Now we compute the sums for the minimum of the closure.
	N& aux_i_j = aux_i[j];
	aux_i_j = x_i[j];
	// Avoid to do unnecessary sums.
	if (check_sum) {
	  N sub_sum1;
	  add_assign_r(sub_sum1, x_i_k, x_k_j, ROUND_UP);
	  N sub_sum2;
	  add_assign_r(sub_sum2, x_i_kk, x_kk_j, ROUND_UP);
	  N sub_sub_sum3;
	  add_assign_r(sub_sub_sum3, x_i_kk, x_kk_k, ROUND_UP);
	  N sub_sum3;
	  add_assign_r(sub_sum3, sub_sub_sum3, x_k_j, ROUND_UP);
	  N sub_sub_sum4;
	  add_assign_r(sub_sub_sum4, x_i_k, x_k_kk, ROUND_UP);
	  N sub_sum4;
	  add_assign_r(sub_sum4, sub_sub_sum4, x_kk_j, ROUND_UP);
	  assign_min(aux_i_j, sub_sum1, sub_sum2, sub_sum3, sub_sum4);
	}
      }

      // And j >= rs_k.
      // the step is given the following way:
      // m_i_j = min(m_i_j,
      //             m_i_k + m_cj_kk,
      //             m_i_kk + m_cj_k,
      //             m_i_k + m_k_kk + m_cj_kk,
      //             m_i_kk + m_kk_k + m_cj_k).
      dimension_type rs_i = i_iter.row_size();
      for (dimension_type j = rs_k; j < rs_i; ++j) {
	dimension_type cj = coherent_index(j);
	typename OR_Matrix<N>::row_reference_type x_cj = *(x.matrix.row_begin()+cj);
	N& x_cj_k = x_cj[k];
	N& x_cj_kk = x_cj[k+1];
	// Now we compute the sums for the minimum of the closure.
	N& aux_i_j = aux_i[j];
	aux_i_j = x_i[j];
	// Avoid to do unnecessary sums.
	if (check_sum) {
	  N sum1;
	  add_assign_r(sum1, x_i_k, x_cj_kk, ROUND_UP);
	  N sum2;
	  add_assign_r(sum2, x_i_kk, x_cj_k, ROUND_UP);
	  N sub_sub_sum3;
	  add_assign_r(sub_sub_sum3, x_i_kk, x_kk_k, ROUND_UP);
	  N sum3;
	  add_assign_r(sum3, sub_sub_sum3, x_cj_kk, ROUND_UP);
	  N sub_sub_sum4;
	  add_assign_r(sub_sub_sum4, x_i_k, x_k_kk, ROUND_UP);
	  N sum4;
	  add_assign_r(sum4, sub_sub_sum4, x_cj_k, ROUND_UP);
	  assign_min(aux_i_j, sum1, sum2, sum3, sum4);
	}
      }
    }

    // Step2: strong-coherence.
    // The step of strong coherence consist: for every indexes i and j
    // m_i_j = min(m_i_j, (m_i_ci + m_cj_j)/2).
    for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin(),
	   ai_iter = aux.row_begin(); i_iter != m_end; i_iter += 2, ai_iter += 2) {
      typename OR_Matrix<N>::row_reference_type aux_i = *ai_iter;
      typename OR_Matrix<N>::row_reference_type aux_ii = *(ai_iter+1);
      typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
      typename OR_Matrix<N>::row_reference_type x_ii = *(i_iter+1);
      dimension_type i = i_iter.index();
      N& aux_i_ii = aux_i[i+1];
      N& aux_ii_i = aux_ii[i];
      dimension_type rs_i = i_iter.row_size();
      typename OR_Matrix<N>::row_iterator aj_iter = aux.row_begin();
      for (dimension_type j = 0; j < rs_i; j += 2, aj_iter += 2) {
	typename OR_Matrix<N>::row_reference_type aux_j = *aj_iter;
	typename OR_Matrix<N>::row_reference_type aux_jj = *(aj_iter+1);
	N& aux_j_jj = aux_j[j+1];
	N& aux_jj_j = aux_jj[j];
	strong_coherence_local_step(x_i[j], aux_i_ii,
				    aux_jj_j, aux_i[j]);
	strong_coherence_local_step(x_ii[j], aux_ii_i,
				    aux_jj_j, aux_ii[j]);
	strong_coherence_local_step(x_i[j+1], aux_i_ii,
				    aux_j_jj, aux_i[j+1]);
	strong_coherence_local_step(x_ii[j+1], aux_ii_i,
				    aux_j_jj, aux_ii[j+1]);
      }
    }
  }

  // Check for emptyness: the octagon is empty if and only if there is a
  // negative value in the main diagonal.
  for (typename OR_Matrix<N>::row_iterator i = x.matrix.row_begin();
       i != m_end; ++i) {
    typename OR_Matrix<N>::row_reference_type r = *i;
    N& x_i_i = r[i.index()];
    if (x_i_i < 0 ) {
      x.status.set_empty();
      return;
    }
    else {
      assert(x_i_i == 0);
      // Restore PLUS_INFINITY on the main diagonal.
      x_i_i = PLUS_INFINITY;
    }
  }

  // The octagon is not empty and it is now strongly closed.
  x.status.set_strongly_closed();

}

template <typename T>
void
Octagon<T>::strong_closure_assign() const {
  using Implementation::Octagons::assign_min;

  // Do something only if necessary.
  if (marked_empty() || marked_strongly_closed())
    return;

  // Zero-dimensional octagons are necessarily strongly closed.
  if (space_dim == 0)
    return;

#ifndef NDEBUG
  Octagon x_copy = *this;
  x_copy.strong_closure_assign1();
#endif

  // Even though the octagon will not change, its internal representation
  // is going to be modified by the closure algorithm.
  Octagon& x = const_cast<Octagon<T>&>(*this);

  // Fill the main diagonal with zeros.
  for (typename OR_Matrix<N>::row_iterator i = x.matrix.row_begin(),
	 m_end = x.matrix.row_end(); i != m_end; ++i) {
    typename OR_Matrix<N>::row_reference_type r = *i;
    assert(is_plus_infinity(r[i.index()]));
    assign_r(r[i.index()], 0, ROUND_NOT_NEEDED);
  }

  // This algorithm is given by two step, first one is the `closure' that
  // uses a modification of the Floyd-Warshall algorithm,
  // the second one is the `strong-coherence'.
  // It is important to note that after the strong-coherence,
  // the octagon is closed yet.
  // The step of closure is divided in three cases that describe
  // how the three indexes `i', `j' and `k' (used in this algorithm)
  // interdepende.
  // We recall that, given an index `h', we indicate with `ch'
  // the index such that:
  // ch = h + 1, if h is even number or
  // ch = h - 1, if h is odd.

  // Step 1: closure.
  for (typename OR_Matrix<N>::row_iterator k_iter = x.matrix.row_begin(),
	 k_end = x.matrix.row_end(); k_iter != k_end; ++k_iter) {
    dimension_type k = k_iter.index();
    dimension_type rs_k = k_iter.row_size();
    typename OR_Matrix<N>::row_reference_type x_k = *k_iter;
    typename OR_Matrix<N>::row_iterator ck_iter = (k%2) ? k_iter-1 : k_iter+1;
    typename OR_Matrix<N>::row_reference_type x_ck = *ck_iter;

    for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin(),
	   i_end = x.matrix.row_end(); i_iter != i_end; ++i_iter) {
      dimension_type i = i_iter.index();
      dimension_type rs_i = i_iter.row_size();
      typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
      typename OR_Matrix<N>::row_iterator ci_iter = (i%2) ? i_iter-1 : i_iter+1;
      typename OR_Matrix<N>::row_reference_type x_ci = *ci_iter;
      const N& x_i_k = (k < rs_i) ? x_i[k] : x_ck[coherent_index(i)];
      if (!is_plus_infinity(x_i_k)) {
	dimension_type n_rows = 2*space_dim;
	for (dimension_type j = 0; j < n_rows; ++j) {
	  dimension_type cj = coherent_index(j);
	  typename OR_Matrix<N>::row_reference_type x_cj = *(x.matrix.row_begin()+cj);
	  const N& x_k_j = (j < rs_k) ? x_k[j] : x_cj[coherent_index(k)];
	  if (!is_plus_infinity(x_k_j)) {
	    N& x_i_j = (j < rs_i) ? x_i[j] : x_cj[coherent_index(i)];
	    N sum;
	    add_assign_r(sum, x_i_k, x_k_j, ROUND_UP);
	    assign_min(x_i_j, sum);
	  }
	}
      }
    }
  }

  // Check for emptyness: the octagon is empty if and only if there is a
  // negative value in the main diagonal.
  for (typename OR_Matrix<N>::row_iterator i = x.matrix.row_begin(),
	 m_end = x.matrix.row_end(); i != m_end; ++i) {
    typename OR_Matrix<N>::row_reference_type r = *i;
    N& x_i_i = r[i.index()];
    if (x_i_i < 0) {
      x.status.set_empty();
      return;
    }
    else {
      assert(x_i_i == 0);
      // Restore PLUS_INFINITY on the main diagonal.
      x_i_i = PLUS_INFINITY;
    }
  }

  // The octagon is not empty and it is now strongly closed.
  x.status.set_strongly_closed();

  // Step 2: we enforce the strong coherence.
  // The strong-coherence is: for every indexes i and j
  // m_i_j <= (m_i_ci + m_cj_j)/2
  // where ci = i + 1, if i is even number or
  //       ci = i - 1, if i is odd.
  // Ditto for cj.
  for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin(),
	 i_end = x.matrix.row_end(); i_iter != i_end; ++i_iter) {
    typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
    dimension_type rs_i = i_iter.row_size();
    dimension_type i = i_iter.index();
    dimension_type ci = coherent_index(i);
    N& x_i_ci = x_i[ci];
    // Avoid to do unnecessary sums.
    if (!is_plus_infinity(x_i_ci))
      for (dimension_type j = 0; j < rs_i; ++j) {
	if (i != j) {
	  dimension_type cj = coherent_index(j);
	  N& x_cj_j = x.matrix[cj][j];
	  if (!is_plus_infinity(x_cj_j)) {
	    N& x_i_j = x_i[j];
	    N sum;
	    add_assign_r(sum,x_i_ci, x_cj_j, ROUND_UP);
	    N d;
	    div2exp_assign_r(d, sum, 1, ROUND_UP);
	    assign_min(x_i_j, d);
	  }
	}
      }
  }

#ifndef NDEBUG
  assert(matrix == x_copy.matrix);
#endif

}

template <typename T>
void
Octagon<T>::strong_closure_assign5() const {
  using Implementation::Octagons::assign_min;

  // Do something only if necessary.
  if (marked_empty() || marked_strongly_closed())
    return;

  // Zero-dimensional octagons are necessarily strongly closed.
  if (space_dim == 0)
    return;

#ifndef NDEBUG
  Octagon x_copy = *this;
  x_copy.strong_closure_assign1();
#endif

  // Even though the octagon will not change, its internal representation
  // is going to be modified by the closure algorithm.
  Octagon& x = const_cast<Octagon<T>&>(*this);

  dimension_type n_rows = 2*space_dim;

  // Fill the main diagonal with zeros.
  for (dimension_type i = n_rows; i-- > 0;) {
    assert(is_plus_infinity(x.matrix[i][i]));
    assign_r(x.matrix[i][i], 0, ROUND_NOT_NEEDED);
  }

  // This algorithm is given by two step, first one is the `closure' that
  // uses a modification of the Floyd-Warshall algorithm,
  // the second one is the `strong-coherence'.
  // It is important to note that after the strong-coherence,
  // the octagon is closed yet.
  // The step of closure is divided in three cases that describe
  // how the three indexes `i', `j' and `k' (used in this algorithm)
  // interdepende.
  // We recall that, given an index `h', we indicate with `ch'
  // the index such that:
  // ch = h + 1, if h is even number or
  // ch = h - 1, if h is odd.

  // Step 1: closure.
  N sum;
  for (dimension_type k = n_rows; k-- > 0; ) {
    for (dimension_type i = n_rows; i-- > 0; ) {
      const N& m_i_k = position_cell(matrix, i, k);
      if (!is_plus_infinity(m_i_k))
	for (dimension_type j = n_rows; j-- > 0; ) {
	  const N& m_k_j = position_cell(matrix, k, j);
	  if (!is_plus_infinity(m_k_j)) {
	    N& m_i_j = position_cell(x.matrix, i, j);
	    add_assign_r(sum, m_i_k, m_k_j, ROUND_UP);
	    assign_min(m_i_j, sum);
	  }
	}
    }
  }

  // Check for emptyness: the octagon is empty if and only if there is a
  // negative value in the main diagonal.
  for (dimension_type i = n_rows; i-- > 0; ) {
    N& x_i_i = x.matrix[i][i];
    if (x_i_i < 0) {
      x.status.set_empty();
      return;
    }
    else {
      assert(x_i_i == 0);
      // Restore PLUS_INFINITY on the main diagonal.
      x_i_i = PLUS_INFINITY;
    }
  }

  // The octagon is not empty and it is now strongly closed.
  x.status.set_strongly_closed();

  // Step 2: we enforce the strong coherence.
  // The strong-coherence is: for every indexes i and j
  // m_i_j <= (m_i_ci + m_cj_j)/2
  // where ci = i + 1, if i is even number or
  //       ci = i - 1, if i is odd.
  // Ditto for cj.
  for (dimension_type i = n_rows; i-- > 0; ) {
    dimension_type ci = coherent_index(i);
    N& x_i_ci = position_cell(x.matrix, i, ci);
    // Avoid to do unnecessary sums.
    if (!is_plus_infinity(x_i_ci))
      for (dimension_type j = n_rows; j-- > 0; ) {
	if (i != j) {
	  dimension_type cj = coherent_index(j);
	  N& x_cj_j = position_cell(x.matrix, cj, j);
	  if (!is_plus_infinity(x_cj_j)) {
	    N& x_i_j = position_cell(x.matrix, i, j);
	    N sub_sum;
	    add_assign_r(sub_sum, x_i_ci, x_cj_j, ROUND_UP);
	    N d;
	    div_assign_r(d, sub_sum, 2, ROUND_UP);
	    assign_min(x_i_j, d);
	  }
	}
      }
  }

#ifndef NDEBUG
  assert(matrix == x_copy.matrix);
#endif

}

template <typename T>
void
Octagon<T>::incremental_strong_closure_assign_of_mine(Variable var) const {
  using Implementation::Octagons::assign_min;

  // `var' should be one of the dimensions of the octagon.
  dimension_type num_var = var.id() + 1;
  if (num_var > space_dim)
    throw_dimension_incompatible("incremental_strong_closure_assign_of_mine(v)", var.id());

  // If `*this' is already empty or closed,
  // then the incremental_strong_closure_assign is not necessary.
  if (marked_empty() || marked_strongly_closed())
    return;

  // If `*this' is zero-dimensional,
  // then it is necessarily a closed octagon.
  if (space_dim == 0)
    return;

  Octagon& x = const_cast<Octagon<T>&>(*this);
  dimension_type n_rows = matrix.num_rows();

  // Fill the main diagonal with zeros.
  for (typename OR_Matrix<N>::row_iterator i = x.matrix.row_begin(),
	 m_end = x.matrix.row_end(); i != m_end; ++i) {
    typename OR_Matrix<N>::row_reference_type r = *i;
    r[i.index()] = 0;
  }

  // This algorithm uses the incremental Floyd-Warshall algorithm.
  // It is constituted by two steps: first we modify all
  // constraints on variable `var'. Infact,
  // the constraints on variable v are changed, and it is possible
  // that these constraints aren't tightest anymore; then
  // second we change also the other constraints.
  // For each step we must apply two rules: a transitivity rule and a
  // strong-coherence rule.
  // Since the matrix, contening all constraints, is pseudo-triangular,
  // the two rules are divided in many cases, that describe
  // how the three indexes `i', `j' and `v' (used in this algorithm)
  // interdepende.
  // We recall that, given an index `h', we indicate with `ch'
  // the index such that:
  // ch = h + 1, if h is even number or
  // ch = h - 1, if h is odd.


  // Step 1: Modify all constraints on variable `var'.
  // Rule: TRANSITIVITY. Here we use `v' to indicate one of
  // the indeces of variable `var', the other index is indicated with `cv'.
  dimension_type v = 2*var.id();
  dimension_type cv = coherent_index(v);
  typename OR_Matrix<N>::row_iterator v_iter = x.matrix.row_begin() + v;
  typename OR_Matrix<N>::row_reference_type x_v = *v_iter;
  typename OR_Matrix<N>::row_reference_type x_cv = *(v_iter+1);
  dimension_type rs_v = v_iter.row_size();
  typename OR_Matrix<N>::row_iterator rs_vend = x.matrix.row_begin() + rs_v;
  N& x_v_cv = x_v[cv];
  N& x_cv_v = x_cv[v];
  // Case 1. i <= v.
  for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin();
       i_iter != rs_vend; ++i_iter) {
    typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
    dimension_type i = i_iter.index();
    typename OR_Matrix<N>::row_reference_type x_ci = (i%2) ? *(i_iter-1) : *(i_iter+1);
    dimension_type ci = coherent_index(i);
    dimension_type rs_i = i_iter.row_size();
    N& x_v_i = x_v[i];
    N& x_cv_i = x_cv[i];
    N& x_v_ci = x_v[ci];
    N& x_cv_ci = x_cv[ci];
    // Case 1.A: j <= i.
    // The transitivity assumption is the following: for each `i, j'
    // m_v_j = min(m_v_j,
    //             m_v_i + m_i_j,
    //             m_v_ci + m_ci_j).
    // Analogous for m_cv_j.
    for (dimension_type j = 0; j < rs_i; ++j) {
      N& x_v_j = x_v[j];
      N& x_cv_j = x_cv[j];
      N& x_i_j = x_i[j];
      N& x_ci_j = x_ci[j];
      if (!is_plus_infinity(x_i_j)) {
	if (!is_plus_infinity(x_v_i)) {
	  N sum1;
	  add_assign_r(sum1, x_v_i, x_i_j, ROUND_UP);
	  assign_min(x_v_j, sum1);
	}
	if(!is_plus_infinity(x_cv_i)) {
	  N sum2;
	  add_assign_r(sum2, x_cv_i, x_i_j, ROUND_UP);
	  assign_min(x_cv_j, sum2);
	}
      }
      // Avoid to do unnecessary sums.
      if (!is_plus_infinity(x_ci_j)) {
	if (!is_plus_infinity(x_v_ci)) {
	  N sum3;
	  add_assign_r(sum3, x_v_ci, x_ci_j, ROUND_UP);
	  assign_min(x_v_j, sum3);
	}
	if(!is_plus_infinity(x_cv_i)) {
	  N sum4;
	  add_assign_r(sum4, x_cv_ci, x_ci_j, ROUND_UP);
	  assign_min(x_cv_j, sum4);
	}
      }
    }
    // Case 1.B: i < j <= v.
    // The transitivity assumption is the following: for each `i, j'
    // m_v_j = min(m_v_j,
    //             m_v_i + m_cj_ci,
    //             m_v_ci + m_cj_i).
    for (typename OR_Matrix<N>::row_iterator j_iter = x.matrix.row_begin() + rs_i;
	 j_iter != rs_vend; ++j_iter) {
      dimension_type j = j_iter.index();
      typename OR_Matrix<N>::row_reference_type x_cj = (j%2) ? *(j_iter-1) : *(j_iter+1);
      N& x_v_j = x_v[j];
      N& x_cv_j = x_cv[j];
      N& x_cj_ci = x_cj[ci];
      N& x_cj_i = x_cj[i];
      if (!is_plus_infinity(x_cj_ci)) {
	if (!is_plus_infinity(x_v_i)) {
	  N sum1;
	  add_assign_r(sum1, x_v_i, x_cj_ci, ROUND_UP);
	  assign_min(x_v_j, sum1);
	}
	if (!is_plus_infinity(x_cv_i)) {
	  N sum2;
	  add_assign_r(sum2, x_cv_i, x_cj_ci, ROUND_UP);
	  assign_min(x_cv_j, sum2);
	}
      }
      // Avoid to do unnecessary sums.
      if (!is_plus_infinity(x_cj_i)) {
	if (!is_plus_infinity(x_v_ci)) {
	  N sum3;
	  add_assign_r(sum3, x_v_ci, x_cj_i, ROUND_UP);
	  assign_min(x_v_j, sum3);
	}
	if (!is_plus_infinity(x_cv_ci)) {
	  N sum4;
	  add_assign_r(sum4, x_cv_ci, x_cj_i, ROUND_UP);
	  assign_min(x_cv_j, sum4);
	}
      }
    }
    // Case 1.C: v < j.
    // The transitivity assumption is the following: for each `i, j'
    // m_cj_cv = min(m_cj_cv,
    //               m_v_i + m_cj_ci,
    //               m_v_ci + m_cj_i).
    for (typename OR_Matrix<N>::row_iterator j_iter = rs_vend,
	   jend = x.matrix.row_end(); j_iter != jend; ++j_iter) {
      dimension_type j = j_iter.index();
      typename OR_Matrix<N>::row_reference_type x_cj = (j%2) ? *(j_iter-1) : *(j_iter+1);
      N& x_cj_v = x_cj[v];
      N& x_cj_cv = x_cj[cv];
      N& x_cj_ci = x_cj[ci];
      N& x_cj_i = x_cj[i];
      if(!is_plus_infinity(x_cj_ci)) {
	if(!is_plus_infinity(x_v_i)) {
	  N sum1;
	  add_assign_r(sum1, x_v_i, x_cj_ci, ROUND_UP);
	  assign_min(x_cj_cv, sum1);
	}
	if(!is_plus_infinity(x_cv_i)) {
	  N sum2;
	  add_assign_r(sum2, x_cv_i, x_cj_ci, ROUND_UP);
	  assign_min(x_cj_v, sum2);
	}
      }
      // Avoid to do unnecessary sums.
      if(!is_plus_infinity(x_cj_i)) {
	if(!is_plus_infinity(x_v_ci)) {
	  N sum3;
	  add_assign_r(sum3, x_v_ci, x_cj_i, ROUND_UP);
	  assign_min(x_cj_cv, sum3);
	}
	if(!is_plus_infinity(x_cv_ci)) {
	  N sum4;
	  add_assign_r(sum4, x_cv_ci, x_cj_i, ROUND_UP);
	  assign_min(x_cj_v, sum4);
	}
      }
    }
  }

  // Step 2: i > v!
  for (typename OR_Matrix<N>::row_iterator i_iter = rs_vend,
	 iend = x.matrix.row_end(); i_iter != iend; ++i_iter) {
    typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
    dimension_type i = i_iter.index();
    dimension_type ci = coherent_index(i);
    typename OR_Matrix<N>::row_reference_type x_ci = (i%2) ? *(i_iter-1) : *(i_iter+1);
    dimension_type rs_i = i_iter.row_size();
    N& x_ci_v = x_ci[v];
    N& x_ci_cv = x_ci[cv];
    N& x_i_v = x_i[v];
    N& x_i_cv = x_i[cv];
    // Case 2.A: j <= v.
    // The transitivity assumption is the following: for each `i, j'
    // m_v_j = min(m_v_j,
    //             m_ci_cv + m_i_j,
    //             m_i_cv + m_ci_j).
    for (dimension_type j = 0; j < rs_v; ++j) {
      N& x_v_j = x_v[j];
      N& x_cv_j = x_cv[j];
      N& x_i_j = x_i[j];
      N& x_ci_j = x_ci[j];
      if (!is_plus_infinity(x_i_j)) {
	if (!is_plus_infinity(x_ci_cv)) {
	  N sum1;
	  add_assign_r(sum1, x_ci_cv, x_i_j, ROUND_UP);
	  assign_min(x_v_j, sum1);
	}
	if (!is_plus_infinity(x_ci_v)) {
	  N sum2;
	  add_assign_r(sum2, x_ci_v, x_i_j, ROUND_UP);
	  assign_min(x_cv_j, sum2);
	}
      }
      // Avoid to do unnecessary sums.
      if (!is_plus_infinity(x_ci_j)) {
	if (!is_plus_infinity(x_i_cv)) {
	  N sum3;
	  add_assign_r(sum3, x_i_cv, x_ci_j, ROUND_UP);
	  assign_min(x_v_j, sum3);
	}
	if (!is_plus_infinity(x_i_v)) {
	  N sum4;
	  add_assign_r(sum4, x_i_v, x_ci_j, ROUND_UP);
	  assign_min(x_cv_j, sum4);
	}
      }
    }
    // Case 2.B: v < j <= i.
    // The transitivity assumption is the following: for each `i, j'
    // m_cj_cv = min(m_cj_cv,
    //               m_ci_cv + m_i_j,
    //               m_i_cv + m_ci_j).
    for (typename OR_Matrix<N>::row_iterator j_iter = rs_vend,
	   jend = x.matrix.row_begin()+rs_i; j_iter != jend; ++j_iter) {
      dimension_type j = j_iter.index();
      typename OR_Matrix<N>::row_reference_type x_cj = (j%2) ? *(j_iter-1) : *(j_iter+1);
      N& x_cj_v = x_cj[v];
      N& x_cj_cv = x_cj[cv];
      N& x_i_j = x_i[j];
      N& x_ci_j = x_ci[j];
      if (!is_plus_infinity(x_i_j)) {
	if (!is_plus_infinity(x_ci_cv)) {
	  N sum1;
	  add_assign_r(sum1, x_ci_cv, x_i_j, ROUND_UP);
	  assign_min(x_cj_cv, sum1);
	}
     	if (!is_plus_infinity(x_ci_v)) {
	  N sum2;
	  add_assign_r(sum2, x_ci_v, x_i_j, ROUND_UP);
	  assign_min(x_cj_v, sum2);
	}
      }
      // Avoid to do unnecessary sums.
      if (!is_plus_infinity(x_ci_j)) {
	if (!is_plus_infinity(x_i_cv)) {
	  N sum3;
	  add_assign_r(sum3, x_i_cv, x_ci_j, ROUND_UP);
	  assign_min(x_cj_cv, sum3);
	}
	if (!is_plus_infinity(x_i_v)) {
	  N sum4;
	  add_assign_r(sum4, x_i_v, x_ci_j, ROUND_UP);
	  assign_min(x_cj_v, sum4);
	}
      }
    }
    // Case 2.C: i < j.
    // The transitivity assumption is the following: for each `i, j'
    // m_cj_cv = min(m_cj_cv,
    //               m_ci_cv + m_cj_ci,
    //               m_i_cv + m_cj_i).
    for (typename OR_Matrix<N>::row_iterator j_iter = x.matrix.row_begin()+rs_i,
	   jend = x.matrix.row_end(); j_iter != jend; ++j_iter) {
      dimension_type j = j_iter.index();
      typename OR_Matrix<N>::row_reference_type x_cj = (j%2) ? *(j_iter-1) : *(j_iter+1);
      N& x_cj_v = x_cj[v];
      N& x_cj_cv = x_cj[cv];
      N& x_cj_ci = x_cj[ci];
      N& x_cj_i = x_cj[i];
      if (!is_plus_infinity(x_cj_ci)) {
	if (!is_plus_infinity(x_ci_cv)) {
	  N sum1;
	  add_assign_r(sum1, x_ci_cv, x_cj_ci, ROUND_UP);
	  assign_min(x_cj_cv, sum1);
	}
	if(!is_plus_infinity(x_ci_v)) {
	  N sum2;
	  add_assign_r(sum2, x_ci_v, x_cj_ci, ROUND_UP);
	  assign_min(x_cj_v, sum2);
	}
      }
      // Avoid to do unnecessary sums.
      if (!is_plus_infinity(x_cj_i)) {
	if (!is_plus_infinity(x_i_cv)) {
	  N sum3;
	  add_assign_r(sum3, x_i_cv, x_cj_i, ROUND_UP);
	  assign_min(x_cj_cv, sum3);
	}
	if(!is_plus_infinity(x_i_v)) {
	  N sum4;
	  add_assign_r(sum4, x_i_v, x_cj_i, ROUND_UP);
	  assign_min(x_cj_v, sum4);
	}
      }
    }
  }

  // Rule STRONG-COHERENCE: Now, we apply the strong-coherence always on `var':
  // Case: i <= v.
  // m_v_i = min(m_v_i,
  //            (m_v_cv + m_ci_i)/2 ).
  for (dimension_type i = 0; i < rs_v; ++i) {
    dimension_type ci = coherent_index(i);
    N& x_ci_i = x.matrix[ci][i];
    N& x_v_i = x_v[i];
    N& x_cv_i = x_cv[i];
    // Avoid to do unnecessary sums.
    if (!is_plus_infinity(x_v_cv) && !is_plus_infinity(x_ci_i)){
      N sub_sum1;
      add_assign_r(sub_sum1, x_v_cv, x_ci_i, ROUND_UP);
      N d;
      div_assign_r(d, sub_sum1, 2, ROUND_UP);
      assign_min(x_v_i, d);
    }
    // Avoid to do unnecessary sums.
    if (!is_plus_infinity(x_cv_v) && !is_plus_infinity(x_ci_i)){
      N sub_sum2;
      add_assign_r(sub_sum2, x_cv_v, x_ci_i, ROUND_UP);
      N d;
      div_assign_r(d, sub_sum2, 2, ROUND_UP);
      assign_min(x_cv_i, d);
    }
  }
  // Case: i > v.
  // m_ci_cv = min(m_ci_cv,
  //              (m_v_cv + m_ci_i)/2 ).
  for (dimension_type i = rs_v; i < n_rows; ++i) {
    dimension_type ci = coherent_index(i);
    typename OR_Matrix<N>::row_iterator ci_iter = x.matrix.row_begin()+ci;
    typename OR_Matrix<N>::row_reference_type x_ci = *ci_iter;
    N& x_ci_i = x_ci[i];
    N& x_ci_v = x_ci[v];
    N& x_ci_cv = x_ci[cv];
    // Avoid to do unnecessary sums.
    if (!is_plus_infinity(x_v_cv) && !is_plus_infinity(x_ci_i)){
      N sub_sum1;
      add_assign_r(sub_sum1, x_v_cv, x_ci_i, ROUND_UP);
      N d;
      div_assign_r(d, sub_sum1, 2, ROUND_UP);
      assign_min(x_ci_cv, d);
    }
    // Avoid to do unnecessary sums.
    if (!is_plus_infinity(x_cv_v) && !is_plus_infinity(x_ci_i)){
      N sub_sum2;
      add_assign_r(sub_sum2, x_cv_v, x_ci_i, ROUND_UP);
      N d;
      div_assign_r(d, sub_sum2, 2, ROUND_UP);
      assign_min(x_ci_v, d);
    }
  }

  // Step 2: finally we find the tighter constraints also
  // for the other variable using the right value of `var'.
  // And thus, we apply in primis the transitivity assumption.

  // Case: j <= i <= v.
  // m_i_j = min(m_i_j,
  //             m_cv_i + m_v_j,
  //             m_v_ci + m_cv_j,
  //             m_cv_ci + m_v_cv + m_cv_j,
  //             m_v_ci + m_cv_v + m_v_j).
  for (dimension_type i = 0; i < rs_v; ++i){
    dimension_type ci = coherent_index(i);
    typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin()+i;
    typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
    dimension_type rs_i = i_iter.row_size();
    N& x_v_ci = x_v[ci];
    N& x_cv_ci = x_cv[ci];
    for (dimension_type j = 0; j < rs_i; ++j) {
      N& x_v_j = x_v[j];
      N& x_cv_j = x_cv[j];
      N& x_i_j = x_i[j];
      N sum1;
      add_assign_r(sum1, x_cv_ci, x_v_j, ROUND_UP);
      N sum2;
      add_assign_r(sum2, x_v_ci, x_cv_j, ROUND_UP);
      N sub_sum3;
      add_assign_r(sub_sum3, x_cv_ci, x_v_cv, ROUND_UP);
      N sum3;
      add_assign_r(sum3, sub_sum3, x_cv_j, ROUND_UP);
      N sub_sum4;
      add_assign_r(sub_sum4, x_v_ci, x_cv_v, ROUND_UP);
      N sum4;
      add_assign_r(sum4, sub_sum4, x_v_j, ROUND_UP);
      assign_min(x_i_j, sum1, sum2, sum3, sum4);
    }
  }
  // Case: j <= v < i.
  // m_i_j = min(m_i_j,
  //             m_i_v + m_v_j,
  //             m_i_cv + m_cv_j,
  //             m_i_v + m_v_cv + m_cv_j,
  //             m_i_cv + m_cv_v + m_v_j).
  for (typename OR_Matrix<N>::row_iterator i_iter = rs_vend,
	 iend = x.matrix.row_end(); i_iter != iend; ++i_iter) {
    typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
    dimension_type rs_i = i_iter.row_size();
    N& x_i_v = x_i[v];
    N& x_i_cv = x_i[cv];
    for (dimension_type j = 0; j < rs_v; ++j){
      N& x_v_j = x_v[j];
      N& x_cv_j = x_cv[j];
      N& x_i_j = x_i[j];
      N sum1;
      add_assign_r(sum1, x_i_v, x_v_j, ROUND_UP);
      N sum2;
      add_assign_r(sum2, x_i_cv, x_cv_j, ROUND_UP);
      N sub_sum3;
      add_assign_r(sub_sum3, x_i_v, x_v_cv, ROUND_UP);
      N sum3;
      add_assign_r(sum3, sub_sum3, x_cv_j, ROUND_UP);
      N sub_sum4;
      add_assign_r(sub_sum4, x_i_cv, x_cv_v, ROUND_UP);
      N sum4;
      add_assign_r(sum4, sub_sum4, x_v_j, ROUND_UP);
      assign_min(x_i_j, sum1, sum2, sum3, sum4);
    }
    // Case: v < j <= i.
    // m_i_j = min(m_i_j,
    //             m_i_v + m_cj_cv,
    //             m_i_cv + m_cj_v,
    //             m_i_v + m_v_cv + m_cj_v,
    //             m_i_cv + m_cv_v + m_cj_cv).
    for (dimension_type j = rs_v; j < rs_i; ++j) {
      dimension_type cj = coherent_index(j);
      typename OR_Matrix<N>::row_iterator cj_iter = x.matrix.row_begin()+cj;
      typename OR_Matrix<N>::row_reference_type x_cj = *cj_iter;
      N& x_cj_v = x_cj[v];
      N& x_cj_cv = x_cj[cv];
      N& x_i_j = x_i[j];
      N sum1;
      add_assign_r(sum1, x_i_v, x_cj_cv, ROUND_UP);
      N sum2;
      add_assign_r(sum2, x_i_cv, x_cj_v, ROUND_UP);
      N sub_sum3;
      add_assign_r(sub_sum3, x_i_v, x_v_cv, ROUND_UP);
      N sum3;
      add_assign_r(sum3, sub_sum3, x_cj_v, ROUND_UP);
      N sub_sum4;
      add_assign_r(sub_sum4, x_i_cv, x_cv_v, ROUND_UP);
      N sum4;
      add_assign_r(sum4, sub_sum4, x_cj_cv, ROUND_UP);
      assign_min(x_i_j, sum1, sum2, sum3, sum4);
    }
  }

  // Rule STRONG-COHERENCE.
  // The strong-coherence is: for every indexes i and j
  // m_i_j <= (m_i_ci + m_cj_j)/2
  // where ci = i + 1, if i is even number or
  //       ci = i - 1, if i is odd.
  // Ditto for cj.
  for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin(),
	 i_end = x.matrix.row_end(); i_iter != i_end; ++i_iter) {
    typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
    dimension_type rs_i = i_iter.row_size();
    dimension_type i = i_iter.index();
    dimension_type ci = coherent_index(i);
    N& x_i_ci = x_i[ci];
    // Avoid to do unnecessary sums.
    if (!is_plus_infinity(x_i_ci))
      for (dimension_type j = 0; j < rs_i; ++j) {
	if (i != j) {
	  dimension_type cj = coherent_index(j);
	  N& x_cj_j = x.matrix[cj][j];
	  if (!is_plus_infinity(x_cj_j)) {
	    N& x_i_j = x_i[j];
	    N sub_sum;
	    add_assign_r(sub_sum, x_i_ci, x_cj_j, ROUND_UP);
	    N d;
	    div_assign_r(d, sub_sum, 2, ROUND_UP);
	    assign_min(x_i_j, d);
	  }
	}
      }
  }

  // Now we check if the octagon is empty and fill the diagonal
  // with plus_infinity.
  // An octagon is empty if there is a negative element in the diagonal.
  x.status.set_strongly_closed();
  for (typename OR_Matrix<N>::row_iterator i = x.matrix.row_begin(),
	 m_end = x.matrix.row_end(); i != m_end; ++i) {
    typename OR_Matrix<N>::row_reference_type r = *i;
    N& x_i_i = r[i.index()];
    if (x_i_i < 0)
      x.status.set_empty();
    x_i_i = PLUS_INFINITY;
  }

  assert(OK());
}

template <typename T>
void
Octagon<T>::incremental_strong_closure_assign1(Variable var) const {
  using Implementation::Octagons::assign_min;

  // `var' should be one of the dimensions of the octagon.
  dimension_type num_var = var.id() + 1;
  if (num_var > space_dim)
    throw_dimension_incompatible("incremental_strong_closure_assign1(v)", var.id());

  // If `*this' is already empty or closed,
  // then the incremental_strong_closure_assign is not necessary.
  if (marked_empty() || marked_strongly_closed())
    return;

  // If `*this' is zero-dimensional,
  // then it is necessarily a closed octagon.
  if (space_dim == 0)
    return;

#ifndef NDEBUG
  Octagon x_copy = *this;
  x_copy.incremental_strong_closure_assign_of_mine(var);
#endif

  Octagon& x = const_cast<Octagon<T>&>(*this);
  dimension_type n_rows = matrix.num_rows();

  // Fill the main diagonal with zeros.
  for (dimension_type i = n_rows; i-- > 0; )
    x.matrix[i][i] = 0;

  // This algorithm uses the incremental Floyd-Warshall algorithm.
  // It is constituted by two steps: first we modify all
  // constraints on variable `var'. Infact,
  // the constraints on variable v are changed, and it is possible
  // that these constraints aren't tightest anymore; then
  // second we change also the other constraints.

  // Step 1: Modify all constraints on variable `var'.
  const dimension_type v = var.id();
  const dimension_type cv = v + 1;

  // Step 1: incremental Floyd-Warshall.
  for (dimension_type k = 0; k < n_rows; ++k) {
    // First of all we modify the columns on variable `var'.
    for (dimension_type i = 0; i < n_rows; ++i) {
      const N& m_i_k = position_cell(x.matrix, i, k);
      if (!is_plus_infinity(m_i_k)) {
	const N& m_k_v = position_cell(x.matrix, k, v);
	if (!is_plus_infinity(m_k_v)) {
	  N& m_i_v = position_cell(x.matrix, i, v);
	  N sum1;
	  add_assign_r(sum1, m_i_k, m_k_v, ROUND_UP);
	  assign_min(m_i_v, sum1);
	}

	const N& m_k_cv = position_cell(x.matrix, k, cv);
	if (!is_plus_infinity(m_k_cv)) {
	  N& m_i_cv = position_cell(x.matrix, i, cv);
	  N sum2;
	  add_assign_r(sum2, m_i_k, m_k_cv, ROUND_UP);
	  assign_min(m_i_cv, sum2);
	}
      }

      // Then we modify the rights on variable `var'.
      const N& m_k_i = position_cell(x.matrix, k, i);
      if (!is_plus_infinity(m_k_i)) {
	const N& m_v_k = position_cell(x.matrix, v, k);
	if (!is_plus_infinity(m_v_k)) {
	  N& m_v_i = position_cell(x.matrix, v, i);
	  N sum3;
	  add_assign_r(sum3, m_v_k, m_k_i, ROUND_UP);
	  assign_min(m_v_i, sum3);
	}

	const N& m_cv_k = position_cell(x.matrix, cv, k);
	if (!is_plus_infinity(m_cv_k)) {
	  N& m_cv_i = position_cell(x.matrix, cv, i);
	  N sum4;
	  add_assign_r(sum4, m_cv_k, m_k_i, ROUND_UP);
	  assign_min(m_cv_i, sum4);
	}
      }
    }
  }

  // Step 2: finally we find the tighter constraints also
  // for the other variable using the right value of `var'.
  for (dimension_type i = 0; i < n_rows; ++i) {
    const N& m_i_v = position_cell(x.matrix, i, v);
    const N& m_i_cv = position_cell(x.matrix, i, cv);
    for (dimension_type j = 0; j < n_rows; ++j) {
      if (!is_plus_infinity(m_i_v)) {
	const N& m_v_j = position_cell(x.matrix, v, j);
	if (!is_plus_infinity(m_v_j)) {
	  N& m_i_j = position_cell(x.matrix, i, j);
	  N sum1;
	  add_assign_r(sum1, m_i_v, m_v_j, ROUND_UP);
	  assign_min(m_i_j, sum1);
	}
      }
      if (!is_plus_infinity(m_i_cv)) {
	const N& m_cv_j = position_cell(x.matrix, cv, j);
	if (!is_plus_infinity(m_cv_j)) {
	  N& m_i_j = position_cell(x.matrix, i, j);
	  N sum2;
	  add_assign_r(sum2, m_i_cv, m_cv_j, ROUND_UP);
	  assign_min(m_i_j, sum2);
	}
      }
    }
  }


  // Check for emptyness:the octagon is empty if and only if there is a
  // negative value in the main diagonal.
  for (dimension_type i = n_rows; i-- > 0; ) {
    N& x_i_i = x.matrix[i][i];
    if (x_i_i < 0) {
      x.status.set_empty();
      return;
    }
    else {
      assert(x_i_i == 0);
      // Restore PLUS_INFINITY on the main diagonal.
      x_i_i = PLUS_INFINITY;
    }
  }

  // The octagon is not empty and it is now strongly closed.
  x.status.set_strongly_closed();

  // Step 2: we enforce the strong coherence.
  // The strong-coherence is: for every indexes i and j
  // m_i_j <= (m_i_ci + m_cj_j)/2
  // where ci = i + 1, if i is even number or
  //       ci = i - 1, if i is odd.
  // Ditto for cj.
  for (dimension_type i = 0; i < n_rows; ++i) {
    dimension_type ci = coherent_index(i);
    N& x_i_ci = position_cell(x.matrix, i, ci);
    // Avoid to do unnecessary sums.
    if (!is_plus_infinity(x_i_ci))
      for (dimension_type j = 0; j < n_rows; ++j) {
	if (i != j) {
	  dimension_type cj = coherent_index(j);
	  N& x_cj_j = position_cell(x.matrix, cj, j);
	  if (!is_plus_infinity(x_cj_j)) {
	    N& x_i_j = position_cell(x.matrix, i, j);
	    N sum;
	    add_assign_r(sum, x_i_ci, x_cj_j, ROUND_UP);
	    N d;
	    div_assign_r(d, sum, 2, ROUND_UP);
	    assign_min(x_i_j, d);
	  }
	}
      }
  }

#ifndef NDEBUG
  assert(matrix == x_copy.matrix);
#endif

  assert(OK());
}

template <typename T>
void
Octagon<T>::incremental_strong_closure_assign(Variable var) const {
  using Implementation::Octagons::assign_min;

  // `var' should be one of the dimensions of the octagon.
  dimension_type num_var = var.id() + 1;
  if (num_var > space_dim)
    throw_dimension_incompatible("incremental_strong_closure_assign(v)", var.id());

  // Do something only if necessary.
  if (marked_empty() || marked_strongly_closed())
    return;

  // Zero-dimensional octagons are necessarily strongly closed.
  if (space_dim == 0)
    return;

#ifndef NDEBUG
  Octagon x_copy = *this;
  x_copy.incremental_strong_closure_assign_of_mine(var);
#endif

  Octagon& x = const_cast<Octagon<T>&>(*this);

  // Fill the main diagonal with zeros.
  for (typename OR_Matrix<N>::row_iterator i = x.matrix.row_begin(),
	 m_end = x.matrix.row_end(); i != m_end; ++i) {
    typename OR_Matrix<N>::row_reference_type r = *i;
    r[i.index()] = 0;
  }

  // This algorithm uses the incremental Floyd-Warshall algorithm.
  // It is constituted by two steps: first we modify all
  // constraints on variable `var'. Infact,
  // the constraints on variable v are changed, and it is possible
  // that these constraints aren't tightest anymore; then
  // second we change also the other constraints.

  // Step 1: Modify all constraints on variable `var'.
  // Rule: TRANSITIVITY. Here we use `v' to indicate one of
  // the indeces of variable `var', the other index is indicated with `cv'
  const dimension_type v = 2*var.id();
  const dimension_type cv = v+1;

  typename OR_Matrix<N>::row_iterator v_iter = x.matrix.row_begin() + v;
  typename OR_Matrix<N>::row_iterator cv_iter = v_iter+1;
  typename OR_Matrix<N>::row_reference_type m_v = *v_iter;
  typename OR_Matrix<N>::row_reference_type m_cv = *cv_iter;

  dimension_type rs_v = v_iter.row_size();
  const dimension_type n_rows = 2*space_dim;

  for (typename OR_Matrix<N>::row_iterator k_iter = x.matrix.row_begin(),
	 k_end = x.matrix.row_end(); k_iter != k_end; ++k_iter) {
    dimension_type k = k_iter.index();
    dimension_type rs_k = k_iter.row_size();
    typename OR_Matrix<N>::row_reference_type m_k = *k_iter;
    typename OR_Matrix<N>::row_reference_type m_ck = (k%2) ? *(k_iter-1) : *(k_iter+1);

    for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin(),
	   i_end = x.matrix.row_end(); i_iter != i_end; ++i_iter) {
      dimension_type i = i_iter.index();
      dimension_type rs_i = i_iter.row_size();
      typename OR_Matrix<N>::row_reference_type m_i = *i_iter;
      typename OR_Matrix<N>::row_reference_type m_ci = (i%2) ? *(i_iter-1) : *(i_iter+1);
      const N& m_i_k = (k < rs_i) ? m_i[k] : m_ck[coherent_index(i)];
      // We adjust the columns on variable 'var'.
      if (!is_plus_infinity(m_i_k)) {
	const N& m_k_v = (v < rs_k) ? m_k[v] : m_cv[coherent_index(k)];
	if (!is_plus_infinity(m_k_v)) {
	  N& m_i_v = (v < rs_i) ? m_i[v] : m_cv[coherent_index(i)];
	  N sum1;
	  add_assign_r(sum1, m_i_k, m_k_v, ROUND_UP);
	  assign_min(m_i_v, sum1);
	}

	const N& m_k_cv = (cv < rs_k) ? m_k[cv] : m_v[coherent_index(k)];
	if (!is_plus_infinity(m_k_cv)) {
	  N& m_i_cv = (cv < rs_i) ? m_i[cv] : m_v[coherent_index(i)];
	  N sum2;
	  add_assign_r(sum2, m_i_k, m_k_cv, ROUND_UP);
	  assign_min(m_i_cv, sum2);
	}
      }

      // Then we adjust the rights on variable 'var'.
      const N& m_k_i = (i < rs_k) ? m_k[i] : m_ci[coherent_index(k)];
      if (!is_plus_infinity(m_k_i)) {
	const N& m_v_k = (k < rs_v) ? m_v[k] : m_ck[cv];
	if (!is_plus_infinity(m_v_k)) {
	  N& m_v_i = (i < rs_v) ? m_v[i] : m_ci[cv];
	  N sum3;
	  add_assign_r(sum3, m_v_k, m_k_i, ROUND_UP);
	  assign_min(m_v_i, sum3);
	}

	const N& m_cv_k = (k < rs_v) ? m_cv[k] : m_ck[v];
	if (!is_plus_infinity(m_cv_k)) {
	  N& m_cv_i = (i < rs_v) ? m_cv[i] : m_ci[v];
	  N sum4;
	  add_assign_r(sum4, m_cv_k, m_k_i, ROUND_UP);
	  assign_min(m_cv_i, sum4);
	}
      }
    }
  }

  // Step 2: finally we find the tighter constraints also
  // for the other variable using the right value of `var'.
  for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin(),
	 i_end = x.matrix.row_end(); i_iter != i_end; ++i_iter) {
    dimension_type i = i_iter.index();
    dimension_type rs_i = i_iter.row_size();
    typename OR_Matrix<N>::row_reference_type m_i = *i_iter;
    typename OR_Matrix<N>::row_reference_type m_ci = (i%2) ? *(i_iter-1) : *(i_iter+1);
    const N& m_i_v = (v < rs_i) ? m_i[v] : m_cv[coherent_index(i)];
    for (dimension_type j = 0; j < n_rows; ++j) {
      dimension_type cj = coherent_index(j);
      typename OR_Matrix<N>::row_reference_type m_cj = *(x.matrix.row_begin()+cj);
      N& m_i_j = (j < rs_i) ? m_i[j] : m_cj[coherent_index(i)];
      if (!is_plus_infinity(m_i_v)) {
	const N& m_v_j = (j < rs_v) ? m_v[j] : m_cj[cv];
	if (!is_plus_infinity(m_v_j)) {
	  N sum1;
	  add_assign_r(sum1, m_i_v, m_v_j, ROUND_UP);
	  assign_min(m_i_j, sum1);
	}
      }
      const N& m_i_cv = (cv < rs_i) ? m_i[cv] : m_v[coherent_index(i)];
      if (!is_plus_infinity(m_i_cv)) {
	const N& m_cv_j = (j < rs_v) ? m_cv[j] : m_cj[v];
	if (!is_plus_infinity(m_cv_j)) {
	  N sum2;
	  add_assign_r(sum2, m_i_cv, m_cv_j, ROUND_UP);
	  assign_min(m_i_j, sum2);
	}
      }
    }
  }

  // Check for emptyness:the octagon is empty if and only if there is a
  // negative value in the main diagonal.
  for (dimension_type i = n_rows; i-- > 0; ) {
    N& x_i_i = x.matrix[i][i];
    if (x_i_i < 0) {
      x.status.set_empty();
      return;
    }
    else {
      // Restore PLUS_INFINITY on the main diagonal.
      assert(x_i_i == 0);
      x_i_i = PLUS_INFINITY;
    }
  }

  // The octagon is not empty and it is now strongly closed.
  x.status.set_strongly_closed();

  // Step 2: we enforce the strong coherence.
  // The strong-coherence is: for every indexes i and j
  // m_i_j <= (m_i_ci + m_cj_j)/2
  // where ci = i + 1, if i is even number or
  //       ci = i - 1, if i is odd.
  // Ditto for cj.
  for (typename OR_Matrix<N>::row_iterator i_iter = x.matrix.row_begin(),
	 i_end = x.matrix.row_end(); i_iter != i_end; ++i_iter) {
    typename OR_Matrix<N>::row_reference_type x_i = *i_iter;
    dimension_type rs_i = i_iter.row_size();
    dimension_type i = i_iter.index();
    dimension_type ci = coherent_index(i);
    N& x_i_ci = x_i[ci];
    // Avoid to do unnecessary sums.
    if (!is_plus_infinity(x_i_ci))
      for (dimension_type j = 0; j < rs_i; ++j) {
	if (i != j) {
	  dimension_type cj = coherent_index(j);
	  N& x_cj_j = x.matrix[cj][j];
	  if (!is_plus_infinity(x_cj_j)) {
	    N& x_i_j = x_i[j];
	    N sum;
	    add_assign_r(sum, x_i_ci, x_cj_j, ROUND_UP);
	    N d;
	    div_assign_r(d, sum, 2, ROUND_UP);
	    assign_min(x_i_j, d);
	  }
	}
      }
  }

#ifndef NDEBUG
  assert(matrix == x_copy.matrix);
#endif

  assert(OK());
}

// FIXME!!!!
template <typename T>
void
Octagon<T>::transitive_reduction_assign() const {
  Octagon<T> backup_copy(*this);

  // This algorithm is a minimization algorithm.
  // Given a constraints system, it constructs an equivalent reduced
  // system removing redundant constraints.
  dimension_type n_rows = matrix.num_rows();

  // We close to find tightest constraints.
  strong_closure_assign();

  // If `*this' is empty, then this operation is not necessary.
  if (marked_empty())
    return;

  // The vector `leader' is used to indicate the smallest variable
  // that belongs to the equivalence class.
  std::vector<dimension_type> leader(n_rows);

  // The vector `next' is used to indicate the smallest next
  // equivalent node that has to be connected with node `i'.
  // If the node is the greatest node of the class, then it has got itself
  // as next node.
  std::vector<dimension_type> next(n_rows);

  // The vector `sing' is used to indicate if the equivalence class
  // is singular (true) or not (false). A class is singular iff
  // it contains nodes and their coherent nodes.
  std::vector<bool> sing(n_rows);

  // We store the three vectors.
  for (dimension_type i = n_rows; i-- > 0; ) {
    leader[i] = i;
    next[i] = i;
    sing[i] = false;
  }

  // Step 1: we store really the vectors with the corrected value.
  // Two nodes are equivalent or zero-equivalent, if there is
  // a zero-cycle containing them both.
  // That is traslated here below as follow:
  // if i > j and x_i_j == -(x_ci_cj), then:
  // 1. leader[i] = leader[j].
  // If `i' is equivalent to its coherent node `c_i', then
  // the class is singular and so:
  // 2. sing[ld_i] = true.
  for (typename OR_Matrix<N>::const_row_iterator i_iter = matrix.row_begin(),
	 iend = matrix.row_end(); i_iter != iend; ++i_iter) {
    typename OR_Matrix<N>::const_row_reference_type m_i = *i_iter;
    dimension_type i = i_iter.index();
    typename OR_Matrix<N>::const_row_iterator ci_iter = (i%2) ? i_iter-1 : i_iter+1;
    typename OR_Matrix<N>::const_row_reference_type m_ci = *ci_iter;
    for (dimension_type j = 0; j < i; ++j) {
      dimension_type cj = coherent_index(j);
      N neg_m_ci_cj;
      assign_neg(neg_m_ci_cj, m_ci[cj], ROUND_DOWN);
      // Check for zero-equivalence.
      N neg_up_m_ci_cj;
      assign_neg(neg_up_m_ci_cj, m_ci[cj], ROUND_UP);
      if (neg_m_ci_cj == neg_up_m_ci_cj &&
	  neg_m_ci_cj == m_i[j]) {
	dimension_type& ld_i = leader[i];
	ld_i = leader[j];
	if (cj == ld_i)
	  sing[ld_i] = true;
      }
    }
  }

  // Store `next' vector.
  for (dimension_type i = n_rows; i-- > 0; ) {
    typename OR_Matrix<N>::const_row_iterator i_iter = matrix.row_begin()+i;
    typename OR_Matrix<N>::const_row_reference_type m_i = *i_iter;
    typename OR_Matrix<N>::const_row_iterator ci_iter = (i%2) ? i_iter-1 : i_iter+1;
    typename OR_Matrix<N>::const_row_reference_type m_ci = *ci_iter;
    for (dimension_type j = 0; j < i; ++j) {
      dimension_type cj = coherent_index(j);
      N neg_m_ci_cj;
      assign_neg(neg_m_ci_cj, m_ci[cj], ROUND_DOWN);
      // Check for zero-equivalence.
      N neg_up_m_ci_cj;
      assign_neg(neg_up_m_ci_cj, m_ci[cj], ROUND_UP);
      if (neg_m_ci_cj == neg_up_m_ci_cj &&
	  neg_m_ci_cj == m_i[j])
	next[j] = i;
    }
  }
  // If the class is singular, then we connect all positive node between them.
  for(dimension_type i = 0; i < n_rows; i += 2) {
    if (sing[leader[i]] == true)
      next[i] = next[i+1];
  }

  // Step 2: If a variable has got a definite value, then
  // we could  remove all binary constraints with this
  // variable and others leaders variable.
  Octagon<T>& x = const_cast<Octagon<T>&>(*this);
  typename OR_Matrix<N>::row_iterator m_end = x.matrix.row_end();
  for (typename OR_Matrix<N>::row_iterator iter = x.matrix.row_begin();
       iter != m_end; iter += 2) {
    typename OR_Matrix<N>::row_reference_type x_i = *iter;
    typename OR_Matrix<N>::row_reference_type x_ci = *(iter+1);
    dimension_type rs_i = iter.row_size();
    dimension_type i = iter.index();
    dimension_type ld_i = leader[i];
    dimension_type ci = coherent_index(i);
    if (ld_i == i && leader[ci] == i) {
      // Remove the constraints in the rows.
      for (dimension_type j = 0; j < i; j += 2)
	if (j == leader[j]) {
	  dimension_type cj = coherent_index(j);
	  x_i[j] = PLUS_INFINITY;
	  x_ci[cj] = PLUS_INFINITY;
	  x_i[cj] = PLUS_INFINITY;
	  x_ci[j] = PLUS_INFINITY;
	}
      // Remove the constraints in the columns.
      for (typename OR_Matrix<N>::row_iterator j_iter = x.matrix.row_begin()+rs_i;
	   j_iter != m_end; j_iter += 2) {
	typename OR_Matrix<N>::row_reference_type x_j = *j_iter;
	typename OR_Matrix<N>::row_reference_type x_cj = *(j_iter+1);
	dimension_type j = j_iter.index();
	if (j == leader[j]) {
	  x_j[i] = PLUS_INFINITY;
	  x_j[ci] = PLUS_INFINITY;
	  x_cj[i] = PLUS_INFINITY;
	  x_cj[ci] = PLUS_INFINITY;
	}
      }
    }
  }

  Octagon<T> aux(space_dim);
  // Step 3: we add only no-redundant constraints to auxiliary octagon.
  // We construct a ``cycle'' with the nodes in the same equivalence
  // class. Moreover, considering leaders,
  // we added only the constraints that aren't obtained by closure or by
  // strong-coherence.
  typename OR_Matrix<N>::row_iterator aux_iter = aux.matrix.row_begin();
  for(typename OR_Matrix<N>::const_row_iterator i_iter = matrix.row_begin(),
	 iend = matrix.row_end(); i_iter != iend; ++i_iter) {
    typename OR_Matrix<N>::const_row_reference_type m_i = *i_iter;
    dimension_type i = i_iter.index();
    typename OR_Matrix<N>::row_reference_type aux_i = *(aux_iter + i);
    dimension_type nxt_i = next[i];
    dimension_type ld_i = leader[i];
    dimension_type ci = coherent_index(i);
    dimension_type c_ld_i = coherent_index(ld_i);
    // If `i' is a no-leader, then we must connect `i' with its next node.
    if (ld_i != i) {
      // If the equivalence class is no-singular, we add the constraints
      // of the positive class no-singular, because the constraints of
      // dual class are obtainable by coherence of the positive class.
      if (sing[ld_i] == false && ld_i%2 == 0) {
	// The greatest node of the class must to connected to leader.
	if (nxt_i != i)
	  aux.matrix[nxt_i][i] = matrix[nxt_i][i];
	else
	  aux.matrix[ci][c_ld_i] = matrix[ci][c_ld_i];
      }
      // If the class is singular, we connected only the positive node.
      if (sing[ld_i] == true && i%2 == 0)
	// The greatest node of the class must to connected to its coherent
	// node.
	if (nxt_i == ci)
	  aux.matrix[ci][i] = matrix[ci][i];
	else
	  aux.matrix[nxt_i][i] = matrix[nxt_i][i];
    }
    // `i' is a leader.
    // The no-redundant constraints are added in the auxiliary octagon `aux'.
    // A constraint is redundant, if we could obtain it by strong-closure
    // (closure and strong-coherence).
    else {
      // The class has at least two different variables (two nodes
      // not coherent between them).
      if (nxt_i != i) {
	// This check work always with singular class. If the class
	// is no-singular, then `i' has to be connected with its next node,
	// iff `i' is a positive leader (All constraints of
	// dual class are obtainable by coherence).
	if (ld_i%2 == 0)
	  aux.matrix[nxt_i][i] = matrix[nxt_i][i];
	// In the singular class with more than 2 nodes, we
	// connected l;eader with its coherent node.
	if (sing[ld_i] == true)
	  aux.matrix[i][ci] = matrix[i][ci];
      }
      else {
	// The singular class has only two coherent nodes.
	if (leader[ci] == i) {
	  aux.matrix[i][ci] = matrix[i][ci];
	  aux.matrix[ci][i] = matrix[ci][i];
	}
      }

      dimension_type rs_i = i_iter.row_size();
      const N& m_i_ci = m_i[ci];
      for (dimension_type j = 0; j < rs_i; ++j) {
	// Checks if the constraint is no-redundant, and in this case
	// it is added in the auxiliary octagon.
	bool to_add = true;
	// `j' is a leader. We consider only the constraints between leaders.
	if (j != i && leader[j] == j) {
	  dimension_type cj = coherent_index(j);
	  const N& m_cj_j = matrix[cj][j];
	  const N& m_i_j = m_i[j];
	  N d;
	  // Assigns to `d':
	  // plus_infinity,        if `m_i_ci' or/and `m_cj_j' are plus infinity;
	  // (m_i_ci + m_cj_j)/2,  otherwise.
	  if (is_plus_infinity(m_i_ci) ||
 	      is_plus_infinity(m_cj_j))
	    d = PLUS_INFINITY;
	  else {
	    N sum;
	    add_assign_r(sum, m_i_ci, m_cj_j, ROUND_UP);
	    div_assign_r(d, sum, 2, ROUND_UP);
	  }
	  if(ci != j)
	    // Checks if the constraint is obtainable by strong-coherence.
	    if (m_i_j >= d)
	      to_add = false;

	  // If the constraint is already redundant, because it is
	  // obtainable by strong-coherence, it is not necessary
	  // to control if the constraint is redundant also by closure.
	  if (to_add)
	    for (dimension_type k = 0; k < n_rows; ++k) {
	      if (k == leader[k] && k != i && k != j) {
		dimension_type ck = coherent_index(k);
		// The constraint m_i_j is redundant, if there is a path from i
		// to j (i = i_0, ... , i_n = j), such that
		// m_i_j = sum_{k=0}^{n-1} m_{i_k}_{i_(k+1)}. In this case
		// the `m_i_j' constraint isn't added to auxiliary matrix.
		// Since the octagon is already closed, the above relation
		// is reduced to three case, in accordance with k, i, j inter-depend:
		// exit k such that
		// 1. m_i_j = m_i_k + m_k_j, or
		// 2. m_i_j = m_i_k + m_cj_ck, or
		// 3. m_i_j = m_ck_ci + m_k_j.
		if (k < j) {
		  N c;
		  add_assign_r(c, m_i[k], matrix[cj][ck], ROUND_UP);
		  if (m_i_j >= c) {
		    to_add = false;
		    break;
		  }
		}
		else if (k < i) {
		  N c;
		  add_assign_r(c, m_i[k], matrix[k][j], ROUND_UP);
		  if (m_i_j >= c) {
		    to_add = false;
		    break;
		  }
		}
		else {
		  N c;
		  add_assign_r(c, matrix[ck][ci], matrix[k][j], ROUND_UP);
		  if (m_i_j >= c) {
		    to_add = false;
		    break;
		  }
		}
	      }
	    }

	  // After to have checked if the constraint is redundant
	  // by strong-coherence or by closure, if it is no-redundant,
	  // it is added in `aux'.
	  if (to_add)
	    aux_i[j] = m_i_j;
	}
      }
    }
  }
  //std::swap(aux, x);
  aux.status.reset_strongly_closed();
  x = aux;
  assert(aux == backup_copy);

  // This method not preserve the closure.
  //x.status.reset_strongly_closed();
  assert(is_transitively_reduced());

#if 0
  if (!is_transitively_reduced()) {
    using namespace IO_Operators;
    std::cout << "TRANSITIVE REDUCTION BUG!" << std::endl
	      << backup_copy << std::endl
	      << "RESULT" << std::endl
	      << *this << std::endl;
  }
#endif
}

template <typename T>
void
Octagon<T>::poly_hull_assign(const Octagon& y) {
  // Dimension-compatibility check.
  if (space_dim != y.space_dim)
    throw_dimension_incompatible("poly_hull_assign(y)", y);

  // The poly-hull of an octagon `oc' with an empty octagon is `oc'.
  y.strong_closure_assign();
  if (y.marked_empty())
    return;
  strong_closure_assign();
  if (marked_empty()) {
    *this = y;
    return;
  }

  // The poly_hull consist to costruct '*this' with the maximum
  // elements selected from '*this' or 'y'.
  typename OR_Matrix<N>::const_element_iterator j = y.matrix.element_begin();
  for (typename OR_Matrix<N>::element_iterator i = matrix.element_begin(),
	 iend = matrix.element_end(); i != iend; ++i, ++j) {
    N& elem = *i;
    const N& y_elem = *j;
    if (elem < y_elem)
      elem = y_elem;
  }
  // The result is still closed.
  assert(OK());
}

template <typename T>
void
Octagon<T>::poly_difference_assign(const Octagon& y) {
  // Dimension-compatibility check.
  if (space_dim != y.space_dim)
    throw_dimension_incompatible("poly_difference_assign(y)", y);

  Octagon& x = *this;

  // Being lazy here is only harmful.
  // We close.
  x.strong_closure_assign();
  // The difference of an empty octagon and of an octagon `p' is empty.
  if (x.marked_empty())
    return;
  // The difference of a octagon `p' and an empty octagon is `p'.
  if (y.marked_empty())
    return;

  // If both octagons are zero-dimensional,
  // then at this point they are necessarily universe octagons,
  // so that their difference is empty.
  if (x.space_dim == 0) {
    x.set_empty();
    return;
  }

  // TODO: This is just an executable specification.
  //       Have to find a more efficient method.
  if (y.contains(x)) {
    x.set_empty();
    return;
  }

  Octagon new_oct(space_dim, EMPTY);
  // We take a constraint of the octagon y at the time and we
  // consider its complementary. Then we intersect the union
  // of these complementaries with the octagon x.
  const Constraint_System& y_cs = y.constraints();
  for (Constraint_System::const_iterator i = y_cs.begin(),
	 y_cs_end = y_cs.end(); i != y_cs_end; ++i) {
    const Constraint& c = *i;
    Octagon z = x;
    const Linear_Expression e = Linear_Expression(c);
    bool change = false;
    if (c.is_nonstrict_inequality())
      change = z.add_constraint_and_minimize(e <= 0);
    if (c.is_equality()) {
      Octagon w = x;
      if (w.add_constraint_and_minimize(e <= 0))
	new_oct.poly_hull_assign(w);
      change = z.add_constraint_and_minimize(e >= 0);
    }
    if (change)
      new_oct.poly_hull_assign(z);
  }
  *this = new_oct;
  // The result is still transitively closed, because both
  // poly_hull_assign() and add_constraint_and_minimize()
  // preserve closure.
  assert(OK());
}

template <typename T>
void
Octagon<T>::add_space_dimensions_and_embed(dimension_type m) {
  // Adding no dimensions is a no-op.
  if (m == 0)
    return;

  dimension_type new_dim = space_dim + m;
  bool was_zero_dim_univ = (!marked_empty() && space_dim == 0);

  // To embed an n-dimension space octagon in a (n+m)-dimension space,
  // we just add `m' variables in the matrix of constraints.
  matrix.grow(2*new_dim);
  // Fill the bottom of the matrix with plus_infinity.
  for (typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + 2*space_dim,
	 iend = matrix.row_end(); i != iend; ++i) {
    typename OR_Matrix<N>::row_reference_type r = *i;
    dimension_type rs_i = i.row_size();
    for (dimension_type j = 0; j < rs_i; ++j)
      r[j] = PLUS_INFINITY;
  }
  space_dim = new_dim;
  // If `*this' was the zero-dim space universe octagon,
  // then we can set the strongly closure flag.
  if (was_zero_dim_univ)
    status.set_strongly_closed();

  assert(OK());
}

template <typename T>
void
Octagon<T>::add_space_dimensions_and_project(dimension_type m) {
  // Adding no dimensions is a no-op.
  if (m == 0)
    return;

  dimension_type n = matrix.num_rows();

  // To project an n-dimension space octagon in a (space_dim+m)-dimension space,
  // we just add `m' columns and rows in the matrix of constraints.
  add_space_dimensions_and_embed(m);
  // We insert 0 where it needs.
  // Attention: now num_rows of matrix is update!
  for (typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n,
	 iend =  matrix.row_end(); i != iend; i += 2) {
    typename OR_Matrix<N>::row_reference_type x_i = *i;
    typename OR_Matrix<N>::row_reference_type x_ci = *(i+1);
    dimension_type ind = i.index();
    assign_r(x_i[ind+1], 0, ROUND_NOT_NEEDED);
    assign_r(x_ci[ind], 0, ROUND_NOT_NEEDED);
  }

  if (marked_strongly_closed())
    status.reset_strongly_closed();
  assert(OK());
}

template <typename T>
void
Octagon<T>::remove_space_dimensions(const Variables_Set& to_be_removed) {
  // The removal of no dimensions from any octagon is a no-op.
  // Note that this case also captures the only legal removal of
  // dimensions from a octagon in a 0-dim space.
  if (to_be_removed.empty()) {
    assert(OK());
    return;
  }

  // Dimension-compatibility check: the variable having
  // maximum cardinality is the one occurring last in the set.
  dimension_type max_dim_to_be_removed = to_be_removed.rbegin()->id();
  if (max_dim_to_be_removed >= space_dim)
    throw_dimension_incompatible("remove_space_dimensions(vs)",
				 max_dim_to_be_removed);

  dimension_type new_space_dim = space_dim - to_be_removed.size();

  strong_closure_assign();
  // When removing _all_ dimensions from a non-empty octagon,
  // we obtain the zero-dimensional universe octagon.
  if (new_space_dim == 0) {
    matrix.resize_no_copy(0);
    if (!marked_empty())
      // We set the zero_dim_univ flag.
      set_zero_dim_univ();
    space_dim = new_space_dim;
    assert(OK());
    return;
  }

  // We consider every variable and we check if it is to be removed.
  // If it is to be removed, we pass to the successive one, elsewhere
  // we move its cells in the right position.
  Variables_Set::const_iterator tbr = to_be_removed.begin();
  dimension_type ftr = tbr->id();
  dimension_type i = ftr + 1;
  dimension_type ftr_size = 2*ftr*(ftr+1);
  typename OR_Matrix<N>::element_iterator iter = matrix.element_begin()+ftr_size;
  while (i < space_dim) {
    if (to_be_removed.count(Variable(i)))
      ++i;
    else {
      typename OR_Matrix<N>::const_row_iterator row_iter = matrix.row_begin()+2*i;
      typename OR_Matrix<N>::const_row_reference_type row_ref = *row_iter;
      typename OR_Matrix<N>::const_row_reference_type row_ref1 = *(++row_iter);
      // If variable(j) is to remove, we pass another variable,
      // else we shift its cells to up right.
      // Attention: first we shift the cells corrispondent to the first
      // row of variable(j), then we shift the cells corrispondent to the
      // second row. We recall that every variable is represented in the `matrix'
      // by two rows and two rows.
      for (dimension_type j = 0; j <= i; ++j)
	if (!to_be_removed.count(Variable(j))) {
	  *(iter++) = row_ref[2*j];
	  *(iter++) = row_ref[2*j+1];
	}
      for (dimension_type j = 0; j <= i; ++j)
	if (!to_be_removed.count(Variable(j))) {
	  *(iter++) = row_ref1[2*j];
	  *(iter++) = row_ref1[2*j+1];
	}
      ++i;
    }
  }
  // Update the space dimension.
  matrix.remove_rows(2*new_space_dim);
  space_dim = new_space_dim;
  assert(OK());
}

template <typename T>
template <typename PartialFunction>
void
Octagon<T>::map_space_dimensions(const PartialFunction& pfunc) {
  if (space_dim == 0)
    return;

  if (pfunc.has_empty_codomain()) {
    // All dimensions vanish: the octagon becomes zero_dimensional.
    remove_higher_space_dimensions(0);
    assert(OK());
    return;
  }

  const dimension_type new_space_dim = pfunc.max_in_codomain() + 1;
  // If the new dimension of space is strict less than the old one,
  // since we don't want to loose solutions, we must close. In fact,
  // we have this octagon:
  // x - y <= 1;
  // y     <= 2.
  // and we have this function:
  // x --> x.
  // If we don't close, we loose the constraint: x <= 3.
  if (new_space_dim < space_dim)
    strong_closure_assign();

  // If the octagon is empty, then it is sufficient to adjust
  // the space dimension of the octagon.
  if (marked_empty()) {
    remove_higher_space_dimensions(new_space_dim);
    return;
  }

  // We create a new matrix with the new space dimension.
  OR_Matrix<N> x(2*new_space_dim);
  for (typename OR_Matrix<N>::row_iterator i_iter = matrix.row_begin(),
	 i_end = matrix.row_end(); i_iter != i_end; i_iter += 2) {
    dimension_type new_i;
    dimension_type i = i_iter.index()/2;
    // We copy and place in the position into `x' the only cells of the `matrix'
    // that refer to both mapped variables, the variable `i' and `j'.
    if (pfunc.maps(i, new_i)) {
      typename OR_Matrix<N>::row_reference_type r_i = *i_iter;
      typename OR_Matrix<N>::row_reference_type r_ii = *(i_iter + 1);
      dimension_type new_i_ = 2*new_i;
      typename OR_Matrix<N>::row_iterator x_iter = x.row_begin() + new_i_;
      typename OR_Matrix<N>::row_reference_type x_i = *x_iter;
      typename OR_Matrix<N>::row_reference_type x_ii = *(x_iter + 1);
      for(dimension_type j = 0; j <= i; ++j) {
	dimension_type new_j;
	// If also the second variable is mapped, we work.
	if (pfunc.maps(j, new_j)) {
	  dimension_type j_ = 2*j;
	  dimension_type new_j_ = 2*new_j;
	  // Mapped the constraints, exchanging the indexes.
	  // Attention: our matrix is pseudo-triangular.
	  // If new_j > new_i, we must consider, as rows, the rows of the variable
	  // new_j, and not of new_i ones.
	  if (new_i >= new_j) {
	    x_i[new_j_] = r_i[j_];
	    x_ii[new_j_] = r_ii[j_];
	    x_ii[new_j_+1] = r_ii[j_ + 1];
	    x_i[new_j_+1] = r_i[j_ + 1];
	  }
	  else {
	    typename OR_Matrix<N>::row_iterator xj_iter = x.row_begin() + new_j_;
	    typename OR_Matrix<N>::row_reference_type x_j = *xj_iter;
	    typename OR_Matrix<N>::row_reference_type x_jj = *(xj_iter + 1);
	    x_jj[new_i_+1] = r_i[j_];
	    x_jj[new_i_] = r_ii[j_];
	    x_j[new_i_+1] = r_i[j_+1];
	    x_j[new_i_] = r_ii[j_+1];
	  }

	}
      }
    }
  }

  std::swap(matrix, x);
  space_dim = new_space_dim;

  assert(OK());
}

template <typename T>
void
Octagon<T>::intersection_assign(const Octagon& y) {
  // Dimension-compatibility check.
  if (space_dim != y.space_dim)
    throw_dimension_incompatible("intersection_assign(y)", y);

  // If one of the two octagons is empty, the intersection is empty.
  if (marked_empty())
    return;
  if (y.marked_empty()) {
    set_empty();
    return;
  }
  // If both octagons are zero-dimensional,then at this point
  // they are necessarily non-empty,
  // so that their intersection is non-empty too.
  if (space_dim == 0)
    return;

  // To intersect two octagons we compare the constraints
  // and we choose the less values.
  bool changed = false;

  typename OR_Matrix<N>::const_element_iterator j = y.matrix.element_begin();
  for (typename OR_Matrix<N>::element_iterator i = matrix.element_begin(),
	 iend = matrix.element_end(); i != iend; ++i, ++j) {
    N& elem = *i;
    const N& y_elem = *j;
    if (y_elem < elem) {
      elem = y_elem;
      changed = true;
    }
  }

  // This method not preserve the closure.
  if (changed && marked_strongly_closed())
    status.reset_strongly_closed();
  assert(OK());
}

template <typename T>
template <typename Iterator>
void
Octagon<T>::CC76_extrapolation_assign(const Octagon& y,
				      Iterator first, Iterator last) {
  // Dimension-compatibility check.
  if (space_dim != y.space_dim)
    throw_dimension_incompatible("CC76_extrapolation_assign(y)", y);

#ifndef NDEBUG
  {
    // We assume that `y' is contained in or equal to `*this'.
    const Octagon x_copy = *this;
    const Octagon y_copy = y;
    assert(x_copy.contains(y_copy));
  }
#endif

  // If both octagons are zero-dimensional,
  // since `*this' contains `y', we simply return `*this'.
  if (space_dim == 0)
    return;

  strong_closure_assign();
  // If `*this' is empty, since `*this' contains `y', `y' is empty too.
  if (marked_empty())
    return;
  y.strong_closure_assign();
  // If `y' is empty, we return.
  if (y.marked_empty())
    return;

  // We compare a constraint of `y' at the time to the corresponding
  // constraint of `*this'. If the value of constraint of `y' is
  // less than `*this' one, we further compare the constraint of
  // `*this' to elements in a sorted container, given by the user,
  // and, if in the container there is a value that is greater than
  // or equal to the value of the constraint, we take this value,
  // otherwise we remove this constraint.
  typename OR_Matrix<N>::const_element_iterator j = y.matrix.element_begin();
  for (typename OR_Matrix<N>::element_iterator i = matrix.element_begin(),
	 iend = matrix.element_end(); i != iend; ++i, ++j) {
    const N& y_elem = *j;
    N& elem = *i;
    if (y_elem < elem) {
      Iterator k = std::lower_bound(first, last, elem);
      if (k != last) {
	if (elem < *k)
	  elem = *k;
      }
      else
	elem = PLUS_INFINITY;
    }
  }

  // This method not preserve the closure.
  status.reset_strongly_closed();
  assert(OK());
}

template <typename T>
void
Octagon<T>::get_limiting_octagon(const Constraint_System& cs,
				 Octagon& limiting_octagon) const {
  const dimension_type cs_space_dim = cs.space_dimension();
  // Private method: the caller has to ensure the following.
  assert(cs_space_dim <= space_dim);

  bool changed = false;
  for (Constraint_System::const_iterator i = cs.begin(),
	 iend = cs.end(); i != iend; ++i) {
    const Constraint& c = *i;
    dimension_type num_vars = 0;
    dimension_type i = 0;
    dimension_type j = 0;
    Coefficient coeff;
    Coefficient term = c.inhomogeneous_term();

    // Constraints that are not octagonal differences are ignored.
    if (extract_octagonal_difference(c, cs_space_dim, num_vars, i, j, coeff, term)) {
      // Select the cell to be modified for the "<=" part of the constraint.
      typename OR_Matrix<N>::const_row_iterator k = matrix.row_begin() + i;
      typename OR_Matrix<N>::const_row_reference_type r = *k;
      const N& r_j = r[j];
      OR_Matrix<N>& lo_mat = limiting_octagon.matrix;
      typename OR_Matrix<N>::row_iterator h = lo_mat.row_begin() + i;
      typename OR_Matrix<N>::row_reference_type s = *h;
      N& s_j = s[j];
      if (coeff < 0)
	coeff = -coeff;
      // Compute the bound for `r_j', rounding towards plus infinity.
      N d;
      div_round_up(d, term, coeff);
      if (r_j <= d)
	if (c.is_inequality())
	  changed = change(changed, s_j, d);
	else {
	  // Select the right row of the cell.
	  typename OR_Matrix<N>::const_row_iterator ck = (i%2 == 0) ? ++k : --k;
	  (i%2 == 0) ? ++h : --h;
	  typename OR_Matrix<N>::const_row_reference_type r1 = *ck;
	  typename OR_Matrix<N>::row_reference_type s1 = *h;
	  // Select the right column of the cell.
	  dimension_type cj = coherent_index(j);
	  const N& r1_cj = r1[cj];
	  N& s1_cj = s1[cj];
	  div_round_up(d, -term, coeff);
	  if (r1_cj <= d)
	    changed = change(changed, s1_cj, d);
	}

    }
  }
  // In general, adding a constraint does not preserve the strongly
  // closure of the octagon.
  if (changed && limiting_octagon.marked_strongly_closed())
    limiting_octagon.status.reset_strongly_closed();
}

template <typename T>
void
Octagon<T>::limited_CC76_extrapolation_assign(const Octagon& y,
					      const Constraint_System& cs,
					      unsigned* /*tp*/) {

  // Dimension-compatibility check.
  if (space_dim != y.space_dim)
    throw_dimension_incompatible("limited_CC76_extrapolation_assign(y, cs)",
				 y);
  // `cs' must be dimension-compatible with the two octagons.
  const dimension_type cs_space_dim = cs.space_dimension();
  if (space_dim < cs_space_dim)
    throw_constraint_incompatible("limited_CC76_extrapolation_assign(y, cs)");

  // Strict inequalities not allowed.
  if (cs.has_strict_inequalities())
    throw_constraint_incompatible("limited_CC76_extrapolation_assign(y, cs)");

  // The limited CC76-extrapolation between two octagons in a
  // zero-dimensional space is a octagon in a zero-dimensional
  // space, too.
  if (space_dim == 0)
    return;

#ifndef NDEBUG
  {
    // We assume that `y' is contained in or equal to `*this'.
    const Octagon x_copy = *this;
    const Octagon y_copy = y;
    assert(x_copy.contains(y_copy));
  }
#endif

  // If `*this' is empty, since `*this' contains `y', `y' is empty too.
  if (marked_empty())
    return;
  // If `y' is empty, we return.
  if (y.marked_empty())
    return;

  Octagon<T> limiting_octagon(space_dim, UNIVERSE);
  get_limiting_octagon(cs, limiting_octagon);
  CC76_extrapolation_assign(y);
  intersection_assign(limiting_octagon);
  assert(OK());
}

template <typename T>
void
Octagon<T>::CH78_widening_assign(const Octagon& y) {
  // Dimension-compatibility check.
  if (space_dim != y.space_dim)
    throw_dimension_incompatible("CH78_widening_assign(y)", y);

#ifndef NDEBUG
  {
    // We assume that `y' is contained in or equal to `*this'.
    const Octagon x_copy = *this;
    const Octagon y_copy = y;
    assert(x_copy.contains(y_copy));
  }
#endif

  // If both octagons are zero-dimensional, since `*this' contains `y',
  // we simply return '*this'.
  if (space_dim == 0)
    return;

  strong_closure_assign();
  // If `*this' is empty, since `*this' contains `y', `y' is empty too.
  if (marked_empty())
    return;

  // Minimize `y'.
  y.transitive_reduction_assign();
  // If `y' is empty, we return.
  if (y.marked_empty())
    return;

  // Extrapolate unstable bounds.
  // We compare a constraint of `y' at the time to the corresponding
  // constraint of `*this'. If the value of constraint of `y' is
  // less than of `*this' one, we remove this constraint.
  typename OR_Matrix<N>::const_element_iterator j = y.matrix.element_begin();
  for (typename OR_Matrix<N>::element_iterator i = matrix.element_begin(),
       iend = matrix.element_end(); i != iend; ++i, ++j) {
    N& elem = *i;
      // Note: in the following line the use of `!=' (as opposed to
      // the use of `<' that would seem -but is not- equivalent) is
      // intentional.
    if (*j != elem) {
      elem = PLUS_INFINITY;
    }
  }

  // This method not preserve the closure.
  status.reset_strongly_closed();
  assert(OK());
}

template <typename T>
void
Octagon<T>::limited_CH78_extrapolation_assign(const Octagon& y,
					      const Constraint_System& cs,
					      unsigned* /*tp*/) {

  // Dimension-compatibility check.
  if (space_dim != y.space_dim)
    throw_dimension_incompatible("limited_CH78_extrapolation_assign(y, cs)",
				 y);
  // `cs' must be dimension-compatible with the two octagons.
  const dimension_type cs_space_dim = cs.space_dimension();
  if (space_dim < cs_space_dim)
    throw_constraint_incompatible("limited_CH78_extrapolation_assign(y, cs)");

  // Strict inequalities not allowed.
  if (cs.has_strict_inequalities())
    throw_constraint_incompatible("limited_CH78_extrapolation_assign(y, cs)");

  // The limited CH78-extrapolation between two octagons in a
  // zero-dimensional space is a octagon in a zero-dimensional
  // space, too.
  if (space_dim == 0)
    return;

#ifndef NDEBUG
  {
    // We assume that `y' is contained in or equal to `*this'.
    const Octagon x_copy = *this;
    const Octagon y_copy = y;
    assert(x_copy.contains(y_copy));
  }
#endif

  // If `*this' is empty, since `*this' contains `y', `y' is empty too.
  if (marked_empty())
    return;
  // If `y' is empty, we return.
  if (y.marked_empty())
    return;


  Octagon<T> limiting_octagon(space_dim, UNIVERSE);
  get_limiting_octagon(cs, limiting_octagon);
  CH78_widening_assign(y);
  intersection_assign(limiting_octagon);
  assert(OK());
}

template <typename T>
void
Octagon<T>::CC76_narrowing_assign(const Octagon& y) {
  // Dimension-compatibility check.
  if (space_dim != y.space_dim)
    throw_dimension_incompatible("CC76_narrowing_assign(y)", y);

#ifndef NDEBUG
  {
    // We assume that `y' is contained in or equal to `*this'.
    const Octagon x_copy = *this;
    const Octagon y_copy = y;
    assert(x_copy.contains(y_copy));
  }
#endif

  // If both octagons are zero-dimensional, since `*this' contains `y',
  // we simply return '*this'.
  if (space_dim == 0)
    return;

  strong_closure_assign();
  // If `*this' is empty, since `*this' contains `y', `y' is empty too.
  if (marked_empty())
    return;
  y.strong_closure_assign();
  // If `y' is empty, we return.
  if (y.marked_empty())
    return;

  // We consider a constraint of `*this', if its value is `plus_infinity',
  // we take the value of the corresponding constraint of `y'.
  bool changed = false;
  typename OR_Matrix<N>::const_element_iterator j = y.matrix.element_begin();
  for (typename OR_Matrix<N>::element_iterator i = matrix.element_begin(),
       iend = matrix.element_end(); i != iend; ++i, ++j) {
    //    if (i->is_plus_infinity()){
    if (is_plus_infinity(*i)){
      *i = *j;
      changed = true;
    }
  }

  if (changed && marked_strongly_closed())
    status.reset_strongly_closed();
  assert(OK());
}

template <typename T>
void
Octagon<T>::affine_image(const Variable var,
                         const Linear_Expression& expr,
			 Coefficient_traits::const_reference denominator) {
  using Implementation::BD_Shapes::div_round_up;

  // The denominator cannot be zero.
  if (denominator == 0)
    throw_generic("affine_image(v, e, d)", "d == 0");

  // Dimension-compatibility checks.
  // The dimension of `expr' should not be greater than the dimension
  // of `*this'.
  const dimension_type expr_space_dim = expr.space_dimension();
  if (space_dim < expr_space_dim)
    throw_dimension_incompatible("affine_image(v, e, d)", "e", expr);

  // `var' should be one of the dimensions of the octagon.
  const dimension_type num_var = var.id();
  if (space_dim < num_var + 1)
    throw_dimension_incompatible("affine_image(v, e, d)", var.id());

  strong_closure_assign();
  // The image of an empty octagon is empty too.
  if (marked_empty())
    return;

  Coefficient b = expr.inhomogeneous_term();

  // Number of non-zero coefficients in `expr': will be set to
  // 0, 1, or 2, the latter value meaning any value greater than 1.
  dimension_type t = 0;

  // Value of inhomogeneous term of `expr' in the case `expr' is a
  // unary.
  Coefficient coeff;

  // Index of the last non-zero coefficient in `expr', if any.
  dimension_type w = 0;

  // Get information about the number of non-zero coefficients in `expr'.
  // The `expr' must not be in two or plus variables.
  for (dimension_type i = expr_space_dim; i-- > 0; )
    if (expr.coefficient(Variable(i)) != 0)
      if (t++ == 1)
	break;
      else {
        w = i;
	coeff = expr.coefficient(Variable(w));
      }
  // Now we know the form of `expr':
  // - If t == 0, then expr == b, with `b' a constant;
  // - If t == 1, then expr == a*w + b, where `w' can be `v' or another
  //   variable; in this second case we have to check whether `a' is
  //   equal to `denominator' or `-denominator', since otherwise we have
  //   to fall back on the general form;
  // - If t == 2, the `expr' is of the general form.
  const dimension_type n_var = 2*num_var;
  const dimension_type k = matrix.row_size(n_var);

//   TEMP_INTEGER(minus_den);
//   neg_assign_r(minus_den, denominator, ROUND_IGNORE);
  Coefficient minus_den = -denominator;
  if (t == 0) {
    // Case 1: expr == b.
    // Remove all constraints on `var'.
    typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var;
    forget_all_octagonal_constraints(i, n_var);
    b *= 2;
    // Add the constraint `var == b/denominator'.
    add_octagonal_constraint(i+1, n_var, b, denominator);
    add_octagonal_constraint(i, n_var+1, b, minus_den);
    assert(OK());
    return;
  }

  if (t == 1) {
    if (coeff == denominator || coeff == minus_den) {
      // Case 2: expr = coeff*w + b, with coeff = +/- denominator.
      if (w == num_var) {
	// `expr' is of the form: coeff*v + b.
	// The `expr' is of the form: -denominator*var + n.
	// First we adjust the matrix of the octagon, swapping x_i^+ with x_i^-.
	if (coeff == denominator) {
	  if (b == 0)
	    // The transformation is the identity function.
	    return;
	  else {
	    // Translate all the constraints on `var' adding or
	    // subtracting the value `b/denominator'.
	    N d;
	    div_round_up(d, b, denominator);
	    N c;
	    div_round_up(c, b, minus_den);
	    N& m_nv_nv1 = matrix[n_var][n_var+1];
	    N& m_nv1_nv = matrix[n_var+1][n_var];
	    typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var;
	    typename OR_Matrix<N>::row_reference_type x_i = *i;
	    typename OR_Matrix<N>::row_reference_type x_ii = *(i+1);
	    for (dimension_type h = k; h-- > 0; ) {
	      if (h != n_var && h != n_var+1) {
		add_assign_r(x_i[h], x_i[h], c, ROUND_UP);
		add_assign_r(x_ii[h], x_ii[h], d, ROUND_UP);
	      }
	      else
		add_assign_r(m_nv1_nv, m_nv1_nv, d, ROUND_UP);
	    }
	    for (typename OR_Matrix<N>::row_iterator iend = matrix.row_end();
		 i != iend; ++i) {
	      typename OR_Matrix<N>::row_reference_type r = *i;
	      dimension_type rs = i.row_size();
	      if (rs != k) {
		add_assign_r(r[n_var], r[n_var], d, ROUND_UP);
		add_assign_r(r[n_var+1], r[n_var+1], c, ROUND_UP);
	      }
	      else
		add_assign_r(m_nv_nv1, m_nv_nv1, c, ROUND_UP);
	    }
	  }
	}

	else {
	  // Here `coeff == -denominator'.
	  typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var;
	  typename OR_Matrix<N>::row_reference_type x_i = *i;
	  typename OR_Matrix<N>::row_reference_type x_ii = *(i+1);
	  for (dimension_type h = k; h-- > 0; ) {
	    std::swap(x_i[h], x_ii[h]);
	  }
	  for (typename OR_Matrix<N>::row_iterator iend = matrix.row_end();
	       i != iend; ++i) {
	    typename OR_Matrix<N>::row_reference_type r = *i;
	    std::swap(r[n_var], r[n_var+1]);
	  }

	  if (b != 0) {
	    // Translate all the constraints on `var' adding or
	    // subtracting the value `b/denominator'.
	    N d;
	    div_round_up(d, b, denominator);
	    N c;
	    div_round_up(c, b, minus_den);
	    N& m_nv_nv1 = matrix[n_var][n_var+1];
	    N& m_nv1_nv = matrix[n_var+1][n_var];
	    typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var;
	    typename OR_Matrix<N>::row_reference_type x_i = *i;
	    typename OR_Matrix<N>::row_reference_type x_ii = *(i+1);
	    for (dimension_type h = k; h-- > 0; ) {
	      if (h != n_var && h != n_var+1) {
		add_assign_r(x_i[h], x_i[h], c, ROUND_UP);
		add_assign_r(x_ii[h], x_ii[h], d, ROUND_UP);
	      }
	      else
		add_assign_r(m_nv1_nv, m_nv1_nv, d, ROUND_UP);
	    }
	    for (typename OR_Matrix<N>::row_iterator iend = matrix.row_end();
		 i != iend; ++i) {
	      typename OR_Matrix<N>::row_reference_type r = *i;
	      dimension_type rs = i.row_size();
	      if (rs != k) {
		add_assign_r(r[n_var], r[n_var], d, ROUND_UP);
		add_assign_r(r[n_var+1], r[n_var+1], c, ROUND_UP);
	      }
	      else
		add_assign_r(m_nv_nv1, m_nv_nv1, c, ROUND_UP);
	    }
	  }
	  status.reset_strongly_closed();
	}
      }

      else {
	// Here `w != var', so that `expr' is of the form
	// +/-denominator * w + b.
	typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var;
	// Remove all constraints on `var'.
	forget_all_octagonal_constraints(i, n_var);
	// Add the new constraint `var - w = b/denominator'.
	if (coeff == denominator) {
	  if (num_var < w) {
	    typename OR_Matrix<N>::row_iterator j = matrix.row_begin() + 2*w;
	    add_octagonal_constraint(j, num_var, b, denominator);
	    add_octagonal_constraint(j+1, num_var+1, b, minus_den);
	  }
	  else if (num_var > w) {
	    add_octagonal_constraint(i+1, w+1, b, denominator);
	    add_octagonal_constraint(i, w, b, minus_den);
	  }
	}
	else {
	// Add the new constraint `var + w = b/denominator'.
	  if (num_var < w) {
	    typename OR_Matrix<N>::row_iterator j = matrix.row_begin() + 2*w;
	    add_octagonal_constraint(j+1, num_var, b, denominator);
	    add_octagonal_constraint(j, num_var+1, b, minus_den);
	  }
	  else if (num_var > w) {
	    typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var;
	    add_octagonal_constraint(i+1, w, b, denominator);
	    add_octagonal_constraint(i, w+1, b, minus_den);
	  }
	}
	status.reset_strongly_closed();
      }
      assert(OK());
      return;
    }
  }

  // General case.
  // Either t == 2, so that
  // expr == a_1*x_1 + a_2*x_2 + ... + a_n*x_n + b, where n >= 2,
  // We find the maximum value `up_sum' of `expr' and the minimum value
  // `low_sum'. Then we remove all the constraints with the `var' and
  // we add the constraints:
  // low_sum <= var,
  // up_sum  >= var.
//   else {
//     // Approximations rispectively from above and from below of the
//     // `expr'.
  const bool is_sc = (denominator > 0);
  TEMP_INTEGER(minus_b);
  neg_assign_r(minus_b, b, ROUND_IGNORE);

  // const Coefficient& minus_b = -b;
  const Coefficient& sc_b = is_sc ? b : minus_b;
  const Coefficient& minus_sc_b = is_sc ? minus_b : b;
  // const Coefficient& minus_den = -denominator;
  // const Coefficient& sc_den = is_sc ? denominator : minus_den;
  const Coefficient& minus_sc_den = is_sc ? minus_den : denominator;
  Linear_Expression minus_expr;
  if (!is_sc)
    minus_expr = -expr;
  const Linear_Expression& sc_expr = is_sc ? expr : minus_expr;

  N pos_sum;
  N neg_sum;
  // Indices of the variables that are unbounded in `this->dbm'.
  // (The initializations are just to quiet a compiler warning.)
  dimension_type pos_pinf_index = 0;
  dimension_type neg_pinf_index = 0;
  // Number of unbounded variables found.
  dimension_type pos_pinf_count = 0;
  dimension_type neg_pinf_count = 0;

  // Approximate the inhomogeneous term.
  assign_r(pos_sum, sc_b, ROUND_UP);
  assign_r(neg_sum, minus_sc_b, ROUND_UP);

  for (dimension_type i = w; i > 0; --i) {
    const Coefficient& sc_i = sc_expr.coefficient(Variable(i));
    const dimension_type j_0 = 2*i;
    const dimension_type j_1 = j_0 + 1;
    // Select the cells to be added in the two sums.
    typename OR_Matrix<N>::const_row_iterator iter = matrix.row_begin() + j_0;
    typename OR_Matrix<N>::const_row_reference_type m_j0 = *iter;
    typename OR_Matrix<N>::const_row_reference_type m_j1 = *(iter+1);
    const int sign_i = sgn(sc_i);
    if (sign_i > 0) {
      N coeff_i;
      assign_r(coeff_i, sc_i, ROUND_UP);
      // Approximating `sc_expr'.
      if (pos_pinf_count <= 1) {
	const N& double_up_approx_i =  m_j1[j_0];
	if (!is_plus_infinity(double_up_approx_i)) {
	  N up_approx_i;
	  div2exp_assign_r(up_approx_i, double_up_approx_i, 1, ROUND_UP);
	  add_mul_assign_r(pos_sum, coeff_i, up_approx_i, ROUND_UP);
	}
	else {
	  ++pos_pinf_count;
	  pos_pinf_index = i;
	}
      }
      // Approximating `-sc_expr'.
      if (neg_pinf_count <= 1) {
	const N& double_up_approx_minus_i = m_j0[j_1];
	if (!is_plus_infinity(double_up_approx_minus_i)) {
	  N up_approx_minus_i;
	  div2exp_assign_r(up_approx_minus_i,
			   double_up_approx_minus_i, 1, ROUND_UP);
	  add_mul_assign_r(neg_sum, coeff_i, up_approx_minus_i, ROUND_UP);
	}
	else {
	  ++neg_pinf_count;
	  neg_pinf_index = i;
	}
      }
    }
    else if (sign_i < 0) {
      TEMP_INTEGER(minus_sc_i);
      neg_assign_r(minus_sc_i, sc_i, ROUND_IGNORE);
      //      Coefficient minus_sc_i = - sc_i;
      N minus_coeff_i;
      assign_r(minus_coeff_i, minus_sc_i, ROUND_UP);
      // Approximating `sc_expr'.
      if (pos_pinf_count <= 1) {
	const N& double_up_approx_minus_i = m_j0[j_1];
	if (!is_plus_infinity(double_up_approx_minus_i)) {
	  N up_approx_minus_i;
	  div2exp_assign_r(up_approx_minus_i,
			   double_up_approx_minus_i, 1, ROUND_UP);
	  add_mul_assign_r(pos_sum,
			 minus_coeff_i, up_approx_minus_i, ROUND_UP);
	}
	else {
	  ++pos_pinf_count;
	  pos_pinf_index = i;
	}
      }
      // Approximating `-sc_expr'.
      if (neg_pinf_count <= 1) {
	const N& double_up_approx_i = m_j1[j_0];
	if (!is_plus_infinity(double_up_approx_i)) {
	  N up_approx_i;
	  div2exp_assign_r(up_approx_i, double_up_approx_i, 1, ROUND_UP);
	  add_mul_assign_r(neg_sum, minus_coeff_i, up_approx_i, ROUND_UP);
	}
	else {
	  ++neg_pinf_count;
	  neg_pinf_index = i;
	}
      }
    }
  }

  // Remove all constraints with var in the rows.
  typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var;
  forget_all_octagonal_constraints(i, n_var);
  // Return immediately if no approximation could be computed.
  if (pos_pinf_count > 1 && neg_pinf_count > 1) {
    assert(OK());
    return;
  }

  // In the following, strongly closure will be definitely lost.
  status.reset_strongly_closed();

  // Before computing quotients, the denominator should be approximated
  // towards zero. Since `sc_den' is known to be positive, this amounts to
  // rounding downwards, which is achieved as usual by rounding upwards
  // `minus_sc_den' and negating again the result.
  N down_sc_den;
  assign_r(down_sc_den, minus_sc_den, ROUND_UP);
  neg_assign_r(down_sc_den, down_sc_den, ROUND_UP);

  // Exploit the upper approximation, if possible.
  if (pos_pinf_count <= 1) {
    // Compute quotient (if needed).
    if (down_sc_den != 1)
      div_assign_r(pos_sum, pos_sum, down_sc_den, ROUND_UP);
    // Add the upper bound constraint, if meaningful.
    if (pos_pinf_count == 0) {
      // Add the constraint `v <= pos_sum'.
      N double_pos_sum = pos_sum;
      //      double_pos_sum *= two;
      mul2exp_assign_r(double_pos_sum, pos_sum, 1, ROUND_IGNORE);
      typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var+1;
      typename OR_Matrix<N>::row_reference_type r = *i;
      assign_r(r[n_var], double_pos_sum, ROUND_UP);
      // Deduce constraints of the form `v - u', where `u != v'.
      //      deduce_v_minus_u_bounds(v, w, sc_expr, sc_den, pos_sum);
    }
//     else
//       // Here `pos_pinf_count == 1'.
//       if (pos_pinf_index != num_var
// 	  && sc_expr.coefficient(Variable(pos_pinf_index)) == sc_den) {
// 	// Add the constraint `v - pos_pinf_index <= pos_sum'.
// 	if (num_var < pos_pinf_index) {
// 	  typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + 2*pos_pinf_index;
// 	  typename OR_Matrix<N>::row_reference_type r_i = *i;
// 	  assign_r(r_i[n_var], pos_sum, ROUND_UP);
// 	}
// 	if (num_var > pos_pinf_index) {
//  	  typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var;
//  	  typename OR_Matrix<N>::row_reference_type r_i = *i;
// 	  assign_r(r_i[2*pos_pinf_index], pos_sum, ROUND_UP);
// 	}
//       }
  }

  // Exploit the lower approximation, if possible.
  if (neg_pinf_count <= 1) {
    // Compute quotient (if needed).
    if (down_sc_den != 1)
      div_assign_r(neg_sum, neg_sum, down_sc_den, ROUND_UP);
    // Add the lower bound constraint, if meaningful.
    if (neg_pinf_count == 0) {
      // Add the constraint `v >= -neg_sum', i.e., `-v <= neg_sum'.
      N double_neg_sum = neg_sum;
      //      double_neg_sum *= two;
      mul2exp_assign_r(double_neg_sum, neg_sum, 1, ROUND_IGNORE);
      typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var;
      typename OR_Matrix<N>::row_reference_type r_i = *i;
      assign_r(r_i[n_var+1], double_neg_sum, ROUND_UP);
      // Deduce constraints of the form `u - v', where `u != v'.
      //     deduce_u_minus_v_bounds(v, w, sc_expr, sc_den, neg_sum);
    }
//     else
//       // Here `neg_pinf_count == 1'.
//       if (neg_pinf_index != num_var
// 	  && sc_expr.coefficient(Variable(neg_pinf_index)) == sc_den)
// 	// Add the constraint `v - neg_pinf_index >= -neg_sum',
// 	// i.e., `neg_pinf_index - v <= neg_sum'.
// 	if (neg_pinf_index < num_var) {
// 	  typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var;
// 	  typename OR_Matrix<N>::row_reference_type r_i = *i;
// 	  assign_r(r_i[2*neg_pinf_index], neg_sum, ROUND_UP);
// 	}
// 	if (neg_pinf_index > num_var) {
// 	  typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + 2*neg_pinf_index + 1;
// 	  typename OR_Matrix<N>::row_reference_type r_i = *i;
// 	  assign_r(r_i[n_var+1], neg_sum, ROUND_UP);
// 	}
  }

//     typename OR_Matrix<N>::row_iterator k = matrix.row_begin() + n_var;
//     // Added the right constraints, if necessary.
//     if (up_sum_ninf)
//       //  add_constraint(denominator*dnm*var <= up_sum);
//       add_octagonal_constraint(k+1, n_var, 2*up_sum, denominator*dnm);
//     if (low_sum_ninf)
//       //      std::cout << n_var[0] << std::endl << n_var[1] << std::endl;
//       //     add_constraint(denominator*dnm1*var >= low_sum);
//       add_octagonal_constraint(k, n_var+1, -2*low_sum, denominator*dnm1);
//   }

//  incremental_strong_closure_assign(var);
  assert(OK());
}

template <typename T>
void
Octagon<T>::affine_preimage(const Variable var,
			    const Linear_Expression& expr,
			    Coefficient_traits::const_reference denominator) {

  // The denominator cannot be zero.
  if (denominator == 0)
    throw_generic("affine_preimage(v, e, d)", "d == 0");

  // Dimension-compatibility checks.
  // The dimension of `expr' should not be greater than the dimension
  // of `*this'.
  const dimension_type expr_space_dim = expr.space_dimension();
  if (space_dim < expr_space_dim)
    throw_dimension_incompatible("affine_preimage(v, e, d)", "e", expr);
  // `var' should be one of the dimensions of the octagon.
  dimension_type num_var = var.id();
  if (space_dim < num_var + 1)
    throw_dimension_incompatible("affine_preimage(v, e, d)", var.id());

  // Any affine trasformation of an empty octagon is empty.
  if (marked_empty())
    return;

  // Index of the non-zero component of `expr'.
  dimension_type j = 0;

  // Number of non-zero components of `expr'.
  dimension_type t = 0;

  // Value of inhomogeneous term of `expr' in the case `expr' is a
  // unary.
  Coefficient coeff;

  // Compute the number of the non-zero components of `expr'.
  // The `expr' must not be in two or plus variables.
  for (dimension_type i = expr_space_dim; i-- > 0; )
    if (expr.coefficient(Variable(i)) != 0)
      if (t++ >= 1)
	break;
      else {
        j = i;
	coeff = expr.coefficient(Variable(j));
      }


  // Now we have got a form of `expr':
  // if t == 0, expr = n, with n integer.
  // if t == 1, expr = a*z + n, where z can be `var' or another variable.
  // Attention: in the case t == 1, if z is `var', the coefficient
  // a must be equal to `denominator; if z is a different variable, then
  // it happen that |a| == |denominator|.
  Coefficient b = expr.inhomogeneous_term();
  dimension_type n_var[2] = {2*num_var, 2*num_var + 1};
  //  dimension_type k = matrix.row_size(n_var[0]);

  strong_closure_assign();
  // Any affine trasformation of an empty octagon is empty.
  if (marked_empty())
    return;

  if (t == 0) {
    // Case 1: expr = n.
    // All constraints on `var' are lost, so they are removed.
    typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
    forget_all_octagonal_constraints(i, n_var[0]);
  }
  else if (t == 1 && (coeff == denominator || coeff == -denominator)) {
    // Case 2: expr = coeff*z + n, with denominator = +/- coeff.

    // The `expr' is of the form: coeff*var + n.
    if (j == num_var) {
      // In this case the transformation is invertible.
      // We recall the affine_image to invert the transformation.
      affine_image(var, denominator*var - b, coeff);
    }
    else {
      // We have got an expression of the following form:
      // var1 + n, with `var1' != `var'.
      // All constraints on `var' are lost.
      typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
      forget_all_octagonal_constraints(i, n_var[0]);
    }
  }
  // General case. We have an expression of the form:
  // expr = a_1*x_1 + a_2*x_2 + ... + a_n*x_n.
  else {
    const Coefficient& expr_var = expr.coefficient(var);
    if (expr_var != 0) {
      if (expr_var > 0) {
	// The transformation is partially invertible.
	//	Linear_Expression inverse;
 	Linear_Expression inverse = ((expr_var + denominator)*var);
 	inverse -= expr;
	affine_image(var, inverse, expr.coefficient(var));
      }
      else {
	// The transformation is partially invertible.
	Linear_Expression inverse = ((-expr_var - denominator)*var);
	inverse = expr;
	affine_image(var, inverse, -expr_var);
      }
#if 0
      Coefficient coeff1 = expr.coefficient(Variable(num_var));
      Linear_Expression expr1(coeff1*var);
      Linear_Expression expr2(denominator*var);
      Linear_Expression expr3 = expr1 - expr + expr2;
      affine_image(var, expr3, coeff1);
#endif
    }
    else {
      // The transformation is not invertible: all constraints on `var' are lost.
      typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
      forget_all_octagonal_constraints(i, n_var[0]);
    }
  }

  assert(OK());
}

template <typename T>
void
Octagon<T>::generalized_affine_image(Variable var,
				     const Relation_Symbol relsym,
				     const Linear_Expression&  expr ,
				     Coefficient_traits::const_reference
				     denominator) {

  // The denominator cannot be zero.
  if (denominator == 0)
    throw_generic("generalized_affine_image(v, r, e, d)", "d == 0");

  // Dimension-compatibility checks.
  // The dimension of `expr' should not be greater than the dimension
  // of `*this'.
  dimension_type expr_space_dim = expr.space_dimension();
  if (space_dim < expr_space_dim)
    throw_dimension_incompatible("generalized_affine_image(v, r, e, d)", "e",
				 expr);
  // `var' should be one of the dimensions of the octagon.
  dimension_type num_var = var.id();
  if (space_dim < num_var + 1)
    throw_dimension_incompatible("generalized_affine_image(v, r, e, d)",
				 var.id());

  // The relation symbol cannot be a strict relation symbol.
  if (relsym == LESS_THAN || relsym == GREATER_THAN)
    throw_generic("generalized_affine_image(v, r, e, d)",
  		  "r is a strict relation symbol and "
  		  "*this is an Octagon");

  // Any affine trasformation of an empty octagon is empty.
  if (marked_empty())
    return;

  // Index of the non-zero component of `expr'.
  dimension_type j = 0;

  // Number of non-zero components of `expr'.
  dimension_type t = 0;

  // Value of inhomogeneous term of `expr' in the case `expr' is a
  // unary.
  Coefficient coeff;

  // Compute the number of the non-zero components of `expr'.
  // The `expr' must not be in two or plus variables.
  for (dimension_type i = expr_space_dim; i-- > 0; )
    if (expr.coefficient(Variable(i)) != 0) {
      if (t++ >= 1)
	break;
      else {
        j = i;
	coeff = expr.coefficient(Variable(j));
      }
    }

  // Now we have got a form of `expr':
  // if t == 0, expr = n, with n integer;
  // if t == 1, expr = a*z + n, where z can be `var' or another variable.
  // Attention: in the case t == 1, if z is `var', the coefficient
  // a must be equal to `denominator; if z is a different variable, then
  // it happen that |a| == |denominator|.
  Coefficient b = expr.inhomogeneous_term();
  dimension_type n_var[2] = {2*num_var, 2*num_var + 1};
  dimension_type k = matrix.row_size(n_var[0]);

  strong_closure_assign();
  // Any image of an empty octagon is empty.
  if (marked_empty())
    return;

  if (t == 0) {
    // Case 1: expr = n.
    switch (relsym) {
    case LESS_THAN_OR_EQUAL:
      {
	// All constraints of the form `var (- var1) <= const' are lost
	// and the new constraint `var <= n/denominator' is added.
	typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
	forget_all_octagonal_constraints(i, n_var[0]);
	//	add_constraint(denominator*var <= b);
	typename OR_Matrix<N>::row_iterator k = matrix.row_begin() + n_var[0];
	add_octagonal_constraint(k+1, n_var[0], 2*b, denominator);
	break;
      }
    case EQUAL:
      // The relation symbol is "==":
      // this is just an affine image computation.
      affine_image(var, expr, denominator);
      break;
    case GREATER_THAN_OR_EQUAL:
      {
	// All constraints of the form `var (- var1) >= const' are lost
	// and the new constraint `var >= n/denominator' is added.
	typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
	forget_all_octagonal_constraints(i, n_var[0]);
	//	add_constraint(denominator*var >= b);
	typename OR_Matrix<N>::row_iterator k = matrix.row_begin() + n_var[0];
	add_octagonal_constraint(k, n_var[1], -2*b, denominator);
	break;
      }
    default:
      // We already dealt with the case of a strict relation symbol.
      throw std::runtime_error("PPL internal error");
      break;
    }
  }


  else if ((t == 1) && (coeff == denominator || coeff == -denominator)) {
    // Case 2: expr = +/- denominator*z + n.
    switch (relsym) {
    case LESS_THAN_OR_EQUAL:
      {
	if (j == num_var) {
	  if (coeff != denominator) {
	    // The `expr' is of the form: -denominator*var + n.
	    // First we adjust the matrix of the octagon, swapping x_i^+ with x_i^-.
	    typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
	    typename OR_Matrix<N>::row_reference_type x_i = *i;
	    typename OR_Matrix<N>::row_reference_type x_ii = *(i+1);
	    for (dimension_type h = k; h-- > 0; ) {
	      std::swap(x_i[h], x_ii[h]);
	    }
	    for (typename OR_Matrix<N>::row_iterator iend = matrix.row_end();
		 i != iend; ++i) {
	      typename OR_Matrix<N>::row_reference_type r = *i;
	      std::swap(r[n_var[0]], r[n_var[1]]);
	    }
	  }
	  // The `expr' is of the form: coeff*var + n.
	  // The affine transformation is the identity function, if and
	  // only if the inhomegeneous term is equal zero.
	  if (b == 0)
	    return;
	  // Translate all the constraints of the form `var (- var1) <= const'
	  // adding the value `n/denominator'.
	  N d;
	  div_round_up(d, b, denominator);
	  N& m_nv_nv1 = matrix[n_var[0]][n_var[1]];
	  N& m_nv1_nv = matrix[n_var[1]][n_var[0]];
	  typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
	  typename OR_Matrix<N>::row_reference_type x_i = *i;
	  typename OR_Matrix<N>::row_reference_type x_ii = *(i+1);
	  for (dimension_type h = k; h-- > 0; ) {
	    if (h != n_var[0] && h != n_var[1]) {
	      x_i[h] = PLUS_INFINITY;
	      add_assign_r(x_ii[h], x_ii[h], d, ROUND_UP);
	    }
	    else
	      add_assign_r(m_nv1_nv, m_nv1_nv, d, ROUND_UP);
	  }
	  for (typename OR_Matrix<N>::row_iterator iend = matrix.row_end();
	       i != iend; ++i) {
	    typename OR_Matrix<N>::row_reference_type r = *i;
	    dimension_type rs = i.row_size();
	    if (rs != k) {
	      add_assign_r(r[n_var[0]], r[n_var[0]], d, ROUND_UP);
	      r[n_var[1]] = PLUS_INFINITY;;
	    }
	    else
	      m_nv_nv1 = PLUS_INFINITY;
	  }
	}

	// We have got an expression of the following form: +/- var1 + n
	// with var1 != var.
	// We remove all constraints of the form `var (-/+ var1) <= const'
	// and we add the new constraint `var +/- var1 <= n/denominator'.
	else {
	  typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
	  forget_all_octagonal_constraints(i, n_var[0]);
	  if (expr.coefficient(Variable(j)) < 0)
	    add_constraint(denominator*var + denominator*Variable(j) <= b);
	  else
	    add_constraint(denominator*var - denominator*Variable(j) <= b);
	}
	break;
      }

    case EQUAL:
      // The relation symbol is "==":
      // this is just an affine image computation.
      affine_image(var, expr, denominator);
      break;
    case GREATER_THAN_OR_EQUAL:
      {
	if (j == num_var) {
	  // The `expr' is of the form: a*var + n.
	  if (coeff != denominator) {
	    // The `expr' is of the form: -denominator*var + n.
	    // First we adjust the matrix of the octagon, swapping x_i^+ with x_i^-.
	    typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
	    typename OR_Matrix<N>::row_reference_type x_i = *i;
	    typename OR_Matrix<N>::row_reference_type x_ii = *(i+1);
	    for (dimension_type h = k; h-- > 0; ) {
	      std::swap(x_i[h], x_ii[h]);
	    }
	    for (typename OR_Matrix<N>::row_iterator iend = matrix.row_end();
		 i != iend; ++i) {
	      typename OR_Matrix<N>::row_reference_type r = *i;
	      std::swap(r[n_var[0]], r[n_var[1]]);
	    }
	  }

	  // The `expr' is of the form: coeff*var + n.
	  // The affine transformation is the identity function, if and
	  // only if the inhomegeneous term is equal zero.
	  if (b == 0)
	    return;

	  // We translate all the constraints of the form `var (+/- var1) >= const'
	  // subtracting the value `n/denominator'.
	  N c;
	  div_round_up(c, -b, denominator);
	  N& m_nv_nv1 = matrix[n_var[0]][n_var[1]];
	  N& m_nv1_nv = matrix[n_var[1]][n_var[0]];
	  typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
	  typename OR_Matrix<N>::row_reference_type x_i = *i;
	  typename OR_Matrix<N>::row_reference_type x_ii = *(i+1);
	  for (dimension_type h = k; h-- > 0; ) {
	    if (h != n_var[0] && h != n_var[1]) {
	      add_assign_r(x_i[h], x_i[h], c, ROUND_UP);
	      x_ii[h] = PLUS_INFINITY;
	    }
	    else
	      m_nv1_nv = PLUS_INFINITY;
	  }
	  for (typename OR_Matrix<N>::row_iterator iend = matrix.row_end();
	       i != iend; ++i) {
	    typename OR_Matrix<N>::row_reference_type r = *i;
	    dimension_type rs = i.row_size();
	    if (rs != k) {
	      r[n_var[0]] = PLUS_INFINITY;
	      add_assign_r(r[n_var[1]], r[n_var[1]], c, ROUND_UP);
	    }
	    else
	      add_assign_r(m_nv_nv1, m_nv_nv1, c, ROUND_UP);
	  }
	}

	else {
	  // We have got an expression of the following form:
	  // var1 + n, with `var1' != `var'.
	  // We remove all constraints of the form `var (+/- var1) >= const'
	  // and we add the new constraint `var +/- var1 >= n/denominator'.
	  typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
	  forget_all_octagonal_constraints(i, n_var[0]);

	  if (expr.coefficient(Variable(j)) < 0)
	    add_constraint(denominator*var + denominator*Variable(j) >= b);
	  else
	    add_constraint(denominator*var - denominator*Variable(j) >= b);
	}

	break;
      }
    default:
      // We already dealt with the case of a strict relation symbol.
      throw std::runtime_error("PPL internal error");
      break;
    }
  }

  // General case. We have an expression of the form:
  // expr = a_1*x_1 + a_2*x_2 + ... + a_n*x_n.
  // We find the maximum value `up_sum' of `expr' and the minimum value
  // `low_sum'. Then we remove all the constraints with the `var' and
  // we add the constraints:
  // low_sum <= var,
  // up_sum  >= var.
  else {
    switch (relsym) {
    case LESS_THAN_OR_EQUAL:
      {
	Coefficient up_sum = expr.inhomogeneous_term();

	Coefficient dnm = 1;
	Coefficient dnm1 = 1;
	// Checks if in the two approximations there are an infinite value.
	bool up_sum_ninf = true;

	for (dimension_type i = expr_space_dim; i-- > 0; ) {
	  Coefficient expr_coeff_var = expr.coefficient(Variable(i));
	  if (expr_coeff_var != 0) {
	    dimension_type j_0 = 2*i;
	    dimension_type j_1 = j_0 + 1;
	    // Select the cells to be added in the two sums.
	    typename OR_Matrix<N>::const_row_iterator iter = matrix.row_begin() + j_0;
	    typename OR_Matrix<N>::const_row_reference_type m_j0 = *iter;
	    typename OR_Matrix<N>::const_row_reference_type m_j1 = *(iter+1);
	    const N& m_j0_j1 = m_j0[j_1]; // -2*X
	    const N& m_j1_j0 = m_j1[j_0]; // +2*X
	    if (expr_coeff_var > 0) {
	      // Upper approximation.
	      if (up_sum_ninf)
		if (!is_plus_infinity(m_j1_j0)) {
		  N c;
		  div_assign_r(c, m_j1_j0, 2, ROUND_UP);
		  Coefficient a;
		  Coefficient b;
		  //		  c.numer_denom(a, b);
		  Implementation::Octagons::numer_denom(c, a, b);
		  // Pseudo max_com_div, ma non proprio.
		  // Controlla se b divide perfettamente `dnm', se si` aggiorna b,
		  // altrimenti rimette a posto dnm.
		  // FIXME: dovrebbe trovare i divisori in comune.
		  // Adesso noi abbiamo:
		  // sum/dnm + a*expr_coeff_var/b =
		  // (sum*b + a*expr_coeff_var) / (dnm*b).
		  if (dnm % b == 0) {
		    // In tal caso:
		    // sum/dnm + a*expr_coeff_var/b =
		    // (sum + a*expr_coeff_var*(dnm/b)) / (dnm).
		    b = dnm/b;
		    up_sum += (a*expr_coeff_var*b);
		  }
		  else {
		    dnm *= b;
		    up_sum *= b;
		    up_sum += (a*expr_coeff_var*dnm);
		  }
		}
		else
		  up_sum_ninf = false;

	    }
	    // The coefficient is negative, so consider the negative variable
	    // * <= -X <= *. Es.:
	    // x <-- -a1*x1.
	    else {
	      expr_coeff_var = -expr_coeff_var;
	      // Upper approximation.
	      if (up_sum_ninf)
		if (!is_plus_infinity(m_j0_j1)) {
		  N c;
		  div_assign_r(c, m_j0_j1, 2, ROUND_UP);
		  Coefficient a;
		  Coefficient b;
		  Implementation::Octagons::numer_denom(c, a, b);
		  if (dnm % b == 0) {
		    b = dnm/b;
		    up_sum += (a*expr_coeff_var*b);
		  }
		  else {
		    dnm *= b;
		    up_sum *= b;
		    up_sum += (a*expr_coeff_var*dnm);
		  }
		}
		else
		  up_sum_ninf = false;
	    }
	    // If the approximation is infinite, no constraint is added.
	    if (!up_sum_ninf)
	      break;
	  }
	}

	// Remove all constraints with var in the columns.
	typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
	forget_all_octagonal_constraints(i, n_var[0]);

	// Added the right constraint, if necessary.
	if (up_sum_ninf) {
	  //  add_constraint(denominator*dnm*var <= up_sum);
	  typename OR_Matrix<N>::row_iterator k = matrix.row_begin() + n_var[0];
	  add_octagonal_constraint(k+1, n_var[0], 2*up_sum, denominator*dnm);
	}

	break;
      }
    case EQUAL:
      // The relation symbol is "==":
      // this is just an affine image computation.
      affine_image(var, expr, denominator);
      break;
    case GREATER_THAN_OR_EQUAL:
      {
	Coefficient low_sum = expr.inhomogeneous_term();

	Coefficient dnm = 1;
	Coefficient dnm1 = 1;
	// Checks if in the approximation there is an infinite value.
	bool low_sum_ninf = true;
	for (dimension_type i = expr_space_dim; i-- > 0; ) {
	  Coefficient expr_coeff_var = expr.coefficient(Variable(i));
	  if (expr_coeff_var != 0) {
	    dimension_type j_0 = 2*i;
	    dimension_type j_1 = j_0 + 1;
	    // Select the cells to be added in the two sums.
	    typename OR_Matrix<N>::const_row_iterator iter = matrix.row_begin() + j_0;
	    typename OR_Matrix<N>::const_row_reference_type m_j0 = *iter;
	    typename OR_Matrix<N>::const_row_reference_type m_j1 = *(iter+1);
	    const N& m_j0_j1 = m_j0[j_1]; // -2*X
	    const N& m_j1_j0 = m_j1[j_0]; // +2*X
	    if (expr_coeff_var > 0) {
	      // Lower approximation.
	      if (low_sum_ninf)
		if (!is_plus_infinity(m_j0_j1)) {
		  Coefficient a;
		  Coefficient b;
		  N c1;
		  N div;
		  div_assign_r(div, m_j0_j1, 2, ROUND_UP);
		  neg_assign_r(c1, div, ROUND_DOWN);
		  Implementation::Octagons::numer_denom(c1, a, b);
		  if (dnm1 % b == 0) {
		    b = dnm1/b;
		    low_sum += (a*expr_coeff_var*b);
		  }
		  else {
		    dnm1 *= b;
		    low_sum *= b;
		    low_sum += (a*expr_coeff_var*dnm1);
		  }
		}
		else
		  low_sum_ninf = false;

	    }
	    // The coefficient is negative, so consider the negative variable
	    // * <= -X <= *. Es.:
	    // x <-- -a1*x1.
	    else {
	      expr_coeff_var = -expr_coeff_var;
	      // Lower approximation.
	      if (low_sum_ninf)
		if (!is_plus_infinity(m_j1_j0)) {
		  N c1;
		  N div;
		  div_assign_r(div, m_j1_j0, 2, ROUND_UP);
		  neg_assign_r(c1, div, ROUND_DOWN);
		  Coefficient a;
		  Coefficient b;
		  Implementation::Octagons::numer_denom(c1, a, b);
		  // Lower bound.
		  if (dnm1 % b == 0) {
		    b = dnm1/b;
		    low_sum += (a*expr_coeff_var*b);
		  }
		  else {
		    dnm1 *= b;
		    low_sum *= b;
		    low_sum += (a*expr_coeff_var*dnm1);
		  }
		}
		else
		  low_sum_ninf = false;
	    }
	    // If the approximation is infinite, no constraint is added.
	    if (!low_sum_ninf)
	      break;
	  }
	}

	// Remove all constraints with var in the columns.
	typename OR_Matrix<N>::row_iterator i = matrix.row_begin() + n_var[0];
	forget_all_octagonal_constraints(i, n_var[0]);

	// Added the right constraint, if necessary.
	if (low_sum_ninf) {
	  //	  add_constraint(denominator*dnm1*var >= low_sum);
	  typename OR_Matrix<N>::row_iterator k = matrix.row_begin() + n_var[0];
	  add_octagonal_constraint(k, n_var[1], -2*low_sum, denominator*dnm1);
	}

	break;
      }
    default:
      // We already dealt with the case of a strict relation symbol.
      throw std::runtime_error("PPL internal error");
      break;
    }
  }

  //  incremental_strong_closure_assign(var);
  assert(OK());
}

template <typename T>
void
Octagon<T>::generalized_affine_image(const Linear_Expression& lhs,
				     const Relation_Symbol relsym,
				     const Linear_Expression& rhs) {


  // Dimension-compatibility checks.
  // The dimension of `lhs' should not be greater than the dimension
  // of `*this'.
  dimension_type lhs_space_dim = lhs.space_dimension();
  if (space_dim < lhs_space_dim)
    throw_dimension_incompatible("generalized_affine_image(e1, r, e2)",
				 "e1", lhs);
  // The dimension of `rhs' should not be greater than the dimension
  // of `*this'.
  const dimension_type rhs_space_dim = rhs.space_dimension();
  if (space_dim < rhs_space_dim)
    throw_dimension_incompatible("generalized_affine_image(e1, r, e2)",
				 "e2", rhs);
  // Strict relation symbols are not admitted for octagons.
  if (relsym == LESS_THAN || relsym == GREATER_THAN)
    throw_generic("generalized_affine_image(e1, r, e2)",
		  "r is a strict relation symbol and "
		  "*this is an Octagon");

  strong_closure_assign();
  // Any image of an empty octagon is empty.
  if (marked_empty())
    return;

  // Number of non-zero components of `lhs'.
  dimension_type t = 0;
  dimension_type j = 0;

  // Compute the number of the non-zero components of `lhs'.
  for (dimension_type i = lhs_space_dim; i-- > 0; )
    if (lhs.coefficient(Variable(i)) != 0) {
	++t;
	j = i;
    }

  // Number of non-zero components of `rhs'.
  dimension_type t1 = 0;
  dimension_type j1 = 0;

  // Compute the number of the non-zero components of `rhs'.
  for (dimension_type i = rhs_space_dim; i-- > 0; )
    if (rhs.coefficient(Variable(i)) != 0) {
	++t1;
	j1 = i;
    }

  Coefficient b = lhs.inhomogeneous_term();
  Coefficient b1 = rhs.inhomogeneous_term();

  // lhs is a constant.
  if (t == 0) {
    // rhs is a constant.
    if (t1 == 0) {
      if (relsym ==  LESS_THAN_OR_EQUAL) {
	if (b > b1)
	  set_empty();
	return;
      }
      else if (relsym == EQUAL) {
	if (b != b1)
	  set_empty();
	return;
      }
      else if (relsym == GREATER_THAN_OR_EQUAL) {
	if (b < b1)
	  set_empty();
	return;
      }
    }

    // rhs is the form: d1*var1 + b1.
    else if (t1 == 1) {
      Coefficient d1 = rhs.coefficient(Variable(j1));
      Linear_Expression e = lhs - b1;

      if (d1 < 0) {
	d1 = -d1;
	generalized_affine_image(Variable(j1), relsym, -e, d1);
      }
      else {
	if (relsym == LESS_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j1),
				   GREATER_THAN_OR_EQUAL, e, d1);
	else if (relsym == GREATER_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j1),
				   LESS_THAN_OR_EQUAL, e, d1);
	else
	  generalized_affine_image(Variable(j1), relsym, e, d1);
      }
    }

    // General case for rhs.
    else {
      Coefficient d1 = rhs.coefficient(Variable(j1));
      Linear_Expression e1(d1*Variable(j1));
      Linear_Expression e = b - rhs + e1;

      if (d1 < 0) {
	d1 = -d1;
	generalized_affine_image(Variable(j1), relsym, -e, d1);
      }
      else {
	if (relsym == LESS_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j1),
				   GREATER_THAN_OR_EQUAL, e, d1);
	else if (relsym == GREATER_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j1),
				   LESS_THAN_OR_EQUAL, e, d1);
	else
	  generalized_affine_image(Variable(j1), relsym, e, d1);
      }
    }
  }

  // lhs is the form: d*var + b.
  else if (t == 1) {
    // rhs is a constant.
    if (t1 == 0) {
      Coefficient d = lhs.coefficient(Variable(j));
      Linear_Expression e = rhs -b;

      if (d < 0) {
	d = -d;
	if (relsym == LESS_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j),
				   GREATER_THAN_OR_EQUAL, -e, d);
	else if (relsym == GREATER_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j),
				   LESS_THAN_OR_EQUAL, -e, d);
	else
	  generalized_affine_image(Variable(j), relsym, -e, d);
      }
      else
	generalized_affine_image(Variable(j), relsym, e, d);
    }

    // rhs is the form: d1*var1 + b1.
    else if (t1 == 1) {
      Coefficient d = lhs.coefficient(Variable(j));
      Linear_Expression e = rhs - b;

      if (d < 0) {
	d = -d;
	if (relsym == LESS_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j),
				   GREATER_THAN_OR_EQUAL, -e, d);
	else if (relsym == GREATER_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j),
				   LESS_THAN_OR_EQUAL, -e, d);
	else
	  generalized_affine_image(Variable(j), relsym, -e, d);
      }
      else
	generalized_affine_image(Variable(j), relsym, e, d);

    }

    // The general case for rhs.
    else {
      Coefficient d = lhs.coefficient(Variable(j));
      Linear_Expression e = rhs - b;

      if (d < 0) {
	d = -d;
	if (relsym == LESS_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j),
				   GREATER_THAN_OR_EQUAL, -e, d);
	else if (relsym == GREATER_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j),
				   LESS_THAN_OR_EQUAL, -e, d);
	else
	  generalized_affine_image(Variable(j), relsym, -e, d);
      }
      else
	generalized_affine_image(Variable(j), relsym, e, d);
    }
  }

  // The general case for lhs.
  else {

    // rhs is a constant.
    if (t1 == 0) {
      Coefficient d = lhs.coefficient(Variable(j));
      Linear_Expression e1(d*Variable(j));
      Linear_Expression e = b1 - lhs + e1;

      if (d < 0) {
	d = -d;
	if (relsym == LESS_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j),
				   GREATER_THAN_OR_EQUAL, -e, d);
	else if (relsym == GREATER_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j),
				   LESS_THAN_OR_EQUAL, -e, d);
	else
	  generalized_affine_image(Variable(j), relsym, -e, d);
      }
      else
	generalized_affine_image(Variable(j), relsym, e, d);
    }

    // rhs is the form: d1*var1 + b1.
    else if (t1 == 1) {
      Coefficient d1 = rhs.coefficient(Variable(j1));
      Linear_Expression e = lhs - b1;

      if (d1 < 0) {
	d1 = -d1;
	generalized_affine_image(Variable(j1), relsym, -e, d1);
      }
      else {
	if (relsym == LESS_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j1),
				   GREATER_THAN_OR_EQUAL, e, d1);
	else if (relsym == GREATER_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j1),
				   LESS_THAN_OR_EQUAL, e, d1);
	else
	  generalized_affine_image(Variable(j1), relsym, e, d1);
      }
    }

    // The general case for rhs.
    else {
      Coefficient d = lhs.coefficient(Variable(j));
      Linear_Expression e1(d*Variable(j));
      Linear_Expression e = rhs - lhs + e1;

      if (d < 0) {
	d = -d;
	if (relsym == LESS_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j),
				   GREATER_THAN_OR_EQUAL, -e, d);
	else if (relsym == GREATER_THAN_OR_EQUAL)
	  generalized_affine_image(Variable(j),
				   LESS_THAN_OR_EQUAL, -e, d);
	else
	  generalized_affine_image(Variable(j), relsym, -e, d);
      }
      else
	generalized_affine_image(Variable(j), relsym, e, d);
    }
  }

  assert(OK());
}

template <typename T>
Constraint_System
Octagon<T>::constraints() const {
  Constraint_System cs;
  if (space_dim == 0) {
    if (marked_empty())
      cs = Constraint_System::zero_dim_empty();
  }
  else if (marked_empty())
    cs.insert(0*Variable(space_dim-1) <= -1);
  else {
    // KLUDGE: in the future `cs' will be constructed of the right dimension.
    // For the time being, we force the dimension with the following line.
    cs.insert(0*Variable(space_dim-1) <= 0);

    for (typename OR_Matrix<N>::const_row_iterator i_iter = matrix.row_begin(),
    	   i_end = matrix.row_end(); i_iter != i_end; i_iter += 2) {
      dimension_type i = i_iter.index();
      typename OR_Matrix<N>::const_row_reference_type r_i = *i_iter;
      typename OR_Matrix<N>::const_row_reference_type r_ii = *(i_iter+1);
      const N& c_i_ii = r_i[i+1];
      const N& c_ii_i = r_ii[i];
      // We have the unary constraints.
      N negated_c_i_ii;
      neg_assign_r(negated_c_i_ii, c_i_ii, ROUND_DOWN);
      N negated_up_c_i_ii;
      neg_assign_r(negated_up_c_i_ii, c_i_ii, ROUND_UP);
      if (negated_c_i_ii == negated_up_c_i_ii &&
	  negated_c_i_ii == c_ii_i) {
	// We have one equality constraint of the following form:
	// ax = b.
	Variable x(i/2);
	Coefficient a;
	Coefficient b;
	//	c_ii_i.numer_denom(b, a);
	Implementation::Octagons::numer_denom(c_ii_i, b, a);
	a *= 2;
	cs.insert(a*x == b);
      }
      // We have 0, 1 or 2 inequality constraints.
      else {
	if (!is_plus_infinity(c_i_ii)) {
	  Variable x(i/2);
	  Coefficient a;
	  Coefficient b;
	  //	  c_i_ii.numer_denom(b, a);
	  Implementation::Octagons::numer_denom(c_i_ii, b, a);
	  a *= 2;
	  cs.insert(-a*x <= b);
	}
	if (!is_plus_infinity(c_ii_i)) {
	  Variable x(i/2);
	  Coefficient a;
	  Coefficient b;
	  //	  c_ii_i.numer_denom(b, a);
	  Implementation::Octagons::numer_denom(c_ii_i, b, a);
	  a *= 2;
	  cs.insert(a*x <= b);
	}
      }
    }
    // We have the binary constraints.
    for (typename OR_Matrix<N>::const_row_iterator i_iter = matrix.row_begin(),
    	   i_end = matrix.row_end(); i_iter != i_end; i_iter += 2) {
      dimension_type i = i_iter.index();
      typename OR_Matrix<N>::const_row_reference_type r_i = *i_iter;
      typename OR_Matrix<N>::const_row_reference_type r_ii = *(i_iter+1);
      for (dimension_type j = 0; j < i; j += 2) {
	const N& c_i_j = r_i[j];
	const N& c_ii_jj = r_ii[j+1];
	// We have one equality constraint of the following form:
	// ax - ay = b.
	N negated_c_ii_jj;
	neg_assign_r(negated_c_ii_jj, c_ii_jj, ROUND_DOWN);
	N negated_up_c_ii_jj;
	neg_assign_r(negated_up_c_ii_jj, c_ii_jj, ROUND_UP);
	if (negated_c_ii_jj == negated_up_c_ii_jj &&
	    negated_c_ii_jj == c_i_j) {
	  Variable x(j/2);
	  Variable y(i/2);
	  Coefficient a;
	  Coefficient b;
	  //	  c_i_j.numer_denom(b, a);
	  Implementation::Octagons::numer_denom(c_i_j, b, a);
	  cs.insert(a*x - a*y == b);
	}
	else {
	  // We have 0, 1 or 2 inequality constraints.
	  if (!is_plus_infinity(c_i_j)) {
	    Variable x(j/2);
	    Variable y(i/2);
	    Coefficient a;
	    Coefficient b;
	    //	    c_i_j.numer_denom(b, a);
	    Implementation::Octagons::numer_denom(c_i_j, b, a);
	    cs.insert(a*x - a*y <= b);
	  }
	  if (!is_plus_infinity(c_ii_jj)) {
	    Variable x(j/2);
	    Variable y(i/2);
	    Coefficient a;
	    Coefficient b;
	    //	    c_ii_jj.numer_denom(b, a);
	    Implementation::Octagons::numer_denom(c_ii_jj, b, a);
	    cs.insert(a*y - a*x <= b);
	  }
	}

	const N& c_ii_j = r_ii[j];
	const N& c_i_jj = r_i[j+1];
	// We have one equality constraint of the following form:
	// ax + ay = b.
	N negated_c_i_jj;
	neg_assign_r(negated_c_i_jj, c_i_jj, ROUND_DOWN);
	N negated_up_c_i_jj;
	neg_assign_r(negated_up_c_i_jj, c_i_jj, ROUND_UP);
	if (negated_c_i_jj == negated_up_c_i_jj &&
	    negated_c_i_jj == c_ii_j) {
	  Variable x(j/2);
	  Variable y(i/2);
	  Coefficient a;
	  Coefficient b;
	  //	  c_ii_j.numer_denom(b, a);
	  Implementation::Octagons::numer_denom(c_ii_j, b, a);
	  cs.insert(a*x + a*y == b);
	}
	else {
	  // We have 0, 1 or 2 inequality constraints.
	  if (!is_plus_infinity(c_i_jj)) {
	    Variable x(j/2);
	    Variable y(i/2);
	    Coefficient a;
	    Coefficient b;
	    //	    c_i_jj.numer_denom(b, a);
	    Implementation::Octagons::numer_denom(c_i_jj, b, a);
	    cs.insert(-a*x - a*y <= b);
	  }
	  if (!is_plus_infinity(c_ii_j)) {
	    Variable x(j/2);
	    Variable y(i/2);
	    Coefficient a;
	    Coefficient b;
	    //	    c_ii_j.numer_denom(b, a);
	    Implementation::Octagons::numer_denom(c_ii_j, b, a);
	    cs.insert(a*x + a*y <= b);
	  }
	}
      }
    }
  }
  return cs;
}

/*! \relates Parma_Polyhedra_Library::Octagon */
template <typename T>
std::ostream&
IO_Operators::operator<<(std::ostream& s, const Octagon<T>& c) {
  typedef typename Octagon<T>::coefficient_type N;
  assert(c.OK());
  if (c.is_universe())
    s << "true" << std::endl;
  else {
    // We control empty octagon.
    if (c.marked_empty())
      s << "false" << std::endl;
    else {
      bool first = true;

      // We print the unary constraints as `X <=/>=/== C'.
      for (typename OR_Matrix<N>::const_row_iterator i_iter = c.matrix.row_begin(),
	     i_end = c.matrix.row_end(); i_iter != i_end; i_iter += 2) {
	dimension_type i = i_iter.index();
	Variable v_i = Variable(i/2);
	typename OR_Matrix<N>::const_row_reference_type r_i = *i_iter;
	typename OR_Matrix<N>::const_row_reference_type r_ii = *(i_iter + 1);
	const N& c_i_ii = r_i[i+1];
	const N& c_ii_i = r_ii[i];
	//	N negated_c_i_ii = negate_round_down(c_i_ii);
	N negated_c_i_ii;
	neg_assign_r(negated_c_i_ii, c_i_ii, ROUND_DOWN);
	N negated_up_c_i_ii;
	neg_assign_r(negated_up_c_i_ii, c_i_ii, ROUND_UP);
	//	if (negated_c_i_ii == negate_round_up(c_i_ii) &&
	if (negated_c_i_ii == negated_up_c_i_ii &&
	    negated_c_i_ii == c_ii_i) {
	    // We will print an equality.
	  if (first)
	    first = false;
	  else
	    s << ", ";
	  N half_up_c_ii_i;
	  div_assign_r(half_up_c_ii_i, c_ii_i, 2, ROUND_UP);
	  N half_dw_c_ii_i;
	  div_assign_r(half_dw_c_ii_i, c_ii_i, 2, ROUND_DOWN);
	  if (half_up_c_ii_i == half_dw_c_ii_i)
	    s << v_i << " == " << half_up_c_ii_i;
	  else {
	    s << v_i << " >= " << half_dw_c_ii_i;
	    s << ", ";
	    s << v_i << " <= " << half_up_c_ii_i;
	  }
	}
	else {
	  if (!is_plus_infinity(c_i_ii)) {
	    // Variable(i) >= cost.
	    if (first)
	      first = false;
	    else
	      s << ", " ;
	    s << v_i;
	    //	    N half_c_i_ii = negate_round_down(div_round_up(c_i_ii, 2));
	    N half_up_c_i_ii;
	    div_assign_r(half_up_c_i_ii, c_i_ii, 2, ROUND_UP);
	    N half_c_i_ii;
	    neg_assign_r(half_c_i_ii, half_up_c_i_ii, ROUND_DOWN);
	    s << " >= " << half_c_i_ii;
	  }
	  if(!is_plus_infinity(c_ii_i)) {
	    // Variable(i) <= cost.
	    if (first)
	      first = false;
	    else
	      s << ", ";
	    s << v_i;
	    N half_c_ii_i;
	    div_assign_r(half_c_ii_i, c_ii_i, 2, ROUND_UP);
	    s << " <= " << half_c_ii_i;
	  }
	}
      }


      for (typename OR_Matrix<N>::const_row_iterator i_iter = c.matrix.row_begin(),
	     i_end = c.matrix.row_end(); i_iter != i_end; i_iter += 2) {
	dimension_type i = i_iter.index();
	Variable v_i = Variable(i/2);
	typename OR_Matrix<N>::const_row_reference_type r_i = *i_iter;
	typename OR_Matrix<N>::const_row_reference_type r_ii = *(i_iter + 1);
	// We print the binary constraints as `X - Y <=/>=/== C'.
	for (dimension_type j = 0; j < i; j += 2) {
	  Variable v_j = Variable(j/2);
	  const N& c_ii_jj = r_ii[j+1];
	  const N& c_i_j = r_i[j];
	  //	  N negated_c_ii_jj = negate_round_down(c_ii_jj);
	  N negated_c_ii_jj;
	  neg_assign_r(negated_c_ii_jj, c_ii_jj, ROUND_DOWN);
	  N negated_up_c_ii_jj;
	  neg_assign_r(negated_up_c_ii_jj, c_ii_jj, ROUND_UP);
	  //	  if (negated_c_ii_jj == negate_round_up(c_ii_jj) &&
	  if (negated_c_ii_jj == negated_up_c_ii_jj &&
	      negated_c_ii_jj == c_i_j) {
	    // We will print an equality of the form X - Y == C.
	    if (first)
	      first = false;
	    else
	      s << ", ";
	    //    if (c_i_j.is_nonnegative()) {
	    if (c_i_j >= 0) {
	      s << v_j << " - " << v_i << " == " << c_i_j;
	    }
	    else {
	      s << v_i << " - " << v_j << " == " << c_ii_jj;
	    }
	  }
	  else {
	    // 0, 1, 2 inequality constraints of the form X - Y <=/>= C.
	    if (!is_plus_infinity(c_i_j)) {
	      if (first)
		first = false;
	      else
		s << ", ";
	      //	      if (c_i_j.is_nonnegative()) {
	      if (c_i_j >= 0) {
		s << v_j << " - " << v_i << " <= " << c_i_j;
	      }
	      else {
		N negated_down_c_i_j;
		neg_assign_r(negated_down_c_i_j, c_i_j, ROUND_DOWN);
		//		s << v_i << " - " << v_j << " >= " << negate_round_down(c_i_j);
		s << v_i << " - " << v_j << " >= " << negated_down_c_i_j;
	      }
	    }
	    if (!is_plus_infinity(c_ii_jj)) {
	      if (first)
		first = false;
	      else
		s << ", ";
	      //	      if (c_ii_jj.is_nonnegative()) {
	      if (c_ii_jj >= 0) {
		s << v_i << " - " << v_j << " <= " << c_ii_jj;
	      }
	      else {
		N negated_down_c_ii_jj;
		neg_assign_r(negated_down_c_ii_jj, c_ii_jj, ROUND_DOWN);
		//		s << v_j << " - " << v_i << " >= " << negate_round_down(c_ii_jj);
		s << v_j << " - " << v_i << " >= " << negated_down_c_ii_jj;
	      }
	    }
	  }

	  // We print the binary constraints as `X + Y <=/>=/== C'.
	  const N& c_i_jj = r_i[j+1];
	  const N& c_ii_j = r_ii[j];
	  //	  N negated_c_i_jj = negate_round_down(c_i_jj);
	  N negated_c_i_jj;
	  neg_assign_r(negated_c_i_jj, c_i_jj, ROUND_DOWN);
	  N negated_up_c_i_jj;
	  neg_assign_r(negated_up_c_i_jj, c_i_jj, ROUND_UP);
	  //	  if (negated_c_i_jj == negate_round_up(c_i_jj) &&
	  if (negated_c_i_jj == negated_up_c_i_jj &&
	      negated_c_i_jj == c_ii_j) {
	    // We will print an equality of the form X + Y == C.
	    if (first)
	      first = false;
	    else
	      s << ", ";
	    s << v_j << " + " << v_i << " == " << c_ii_j;
	  }
	  else {
	    // 0, 1, 2 inequality constraints of the form X + Y <=/>= C.
	    if (!is_plus_infinity(c_i_jj)) {
	      if (first)
		first = false;
	      else
		s << ", ";
	      N negated_down_c_i_jj;
	      neg_assign_r(negated_down_c_i_jj, c_i_jj, ROUND_DOWN);
	      //	      s << v_j << " + " << v_i << " >= " << negate_round_down(c_i_jj);
	      s << v_j << " + " << v_i << " >= " << negated_down_c_i_jj;
	    }
	    if (!is_plus_infinity(c_ii_j)) {
	      if (first)
		first = false;
	      else
		s << ", ";
	      s << v_j << " + " << v_i << " <= " << c_ii_j;
	    }
	  }
	}
      }
    }
  }
  return s;
}

template <typename T>
void
Octagon<T>::ascii_dump(std::ostream& s) const {
  using std::endl;

  s << "space_dim "
    << space_dim
    << endl;
  status.ascii_dump(s);
  s << endl;
  matrix.ascii_dump(s);
}

template <typename T>
bool
Octagon<T>::ascii_load(std::istream& s) {
  std::string str;

  if (!(s >> str) || str != "space_dim")
    return false;

  if (!(s >> space_dim))
    return false;

  if(!status.ascii_load(s))
    return false;

  if(!matrix.ascii_load(s))
    return false;

  assert(OK());
  return true;
}

template <typename T>
bool
Octagon<T>::OK() const {
  // Check whether the matrix is well-formed.
  if (!matrix.OK())
    return false;

  // Check whether the status information is legal.
  if (!status.OK())
    return false;

  // All empty octagons are OK.
  if (marked_empty())
    return true;

  // 0-dim universe octagon is OK.
  if (space_dim == 0)
    return true;

  // On the diagonal we must have plus infinity.
  for (typename OR_Matrix<N>::const_row_iterator i = matrix.row_begin(),
	 m_end = matrix.row_end(); i != m_end; ++i) {
    typename OR_Matrix<N>::const_row_reference_type r = *i;
    const N& m_i_i = r[i.index()];
    if (!is_plus_infinity(m_i_i)) {
#ifndef NDEBUG
      dimension_type j = i.index();
      using namespace Parma_Polyhedra_Library::IO_Operators;
      std::cerr << "Octagon::matrix[" << j << "][" << j << "] = "
		<< m_i_i << "!  (+inf was expected.)"
		<< std::endl;
#endif
      return false;
    }
  }

  // Check whether the closure information is legal.
  if (marked_strongly_closed()) {
    Octagon x = *this;
    x.status.reset_strongly_closed();
    x.strong_closure_assign();
    if (x.matrix != matrix) {
#ifndef NDEBUG
      std::cerr << "Octagon is marked as transitively closed but is it not!"
		<< std::endl;
#endif
      return false;
    }
  }


  // A closed octagon must be strong-coherent.
  if (marked_strongly_closed())
    if (!is_strong_coherent()) {
#ifndef NDEBUG
      std::cerr << "Octagon is not strong-coherent!"
		<< std::endl;
#endif
      return false;
    }

  // All checks passed.
  return true;
}


template <typename T>
void
Octagon<T>::throw_dimension_incompatible(const char* method,
					 const Octagon& y) const {
  std::ostringstream s;
  s << "PPL::";
  s << "Octagon::" << method << ":" << std::endl
    << "this->space_dimension() == " << space_dimension()
    << ", y->space_dimension() == " << y.space_dimension() << ".";
  throw std::invalid_argument(s.str());
}

template <typename T>
void
Octagon<T>::throw_dimension_incompatible(const char* method,
					 dimension_type required_dim) const {
  std::ostringstream s;
  s << "PPL::";
  s << "Octagon::" << method << ":" << std::endl
    << "this->space_dimension() == " << space_dimension()
    << ", required dimension == " << required_dim << ".";
  throw std::invalid_argument(s.str());
}

template <typename T>
void
Octagon<T>::throw_dimension_incompatible(const char* method,
					 const Constraint& c) const {
  std::ostringstream s;
  s << "PPL::";
  s << "Octagon::" << method << ":" << std::endl
    << "this->space_dimension() == " << space_dimension()
    << ", c->space_dimension == " << c.space_dimension() << ".";
  throw std::invalid_argument(s.str());
}

template <typename T>
void
Octagon<T>::throw_dimension_incompatible(const char* method,
					 const Generator& g) const {
  std::ostringstream s;
  s << "PPL::";
  s << "Octagon::" << method << ":" << std::endl
    << "this->space_dimension() == " << space_dimension()
    << ", g->space_dimension == " << g.space_dimension() << ".";
  throw std::invalid_argument(s.str());
}

template <typename T>
void
Octagon<T>::throw_constraint_incompatible(const char* method) const {
  std::ostringstream s;
  s << "PPL::Octagon::" << method << ":" << std::endl
    << "the constraint is incompatible.";
  throw std::invalid_argument(s.str());
}

template <typename T>
void
Octagon<T>::throw_expression_too_complex(const char* method,
					 const Linear_Expression& e) const {
  using namespace IO_Operators;
  std::ostringstream s;
  s << "PPL::Octagon::" << method << ":" << std::endl
    << e << " is too complex.";
  throw std::invalid_argument(s.str());
}


template <typename T>
void
Octagon<T>::throw_dimension_incompatible(const char* method,
					 const char* name_row,
					 const Linear_Expression& y) const {
  std::ostringstream s;
  s << "PPL::";
  s << "Octagon::" << method << ":" << std::endl
    << "this->space_dimension() == " << space_dimension()
    << ", " << name_row << "->space_dimension() == "
    << y.space_dimension() << ".";
  throw std::invalid_argument(s.str());
}


template <typename T>
void
Octagon<T>::throw_generic(const char* method,
			  const char* reason) const {
  std::ostringstream s;
  s << "PPL::";
  s << "Octagon::" << method << ":" << std::endl
    << reason;
  throw std::invalid_argument(s.str());
}

} // namespace Parma_Polyhedra_Library

#endif // !defined(PPL_Octagon_templates_hh)
