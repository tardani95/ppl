/* PIP_Tree related class implementation: non-inline functions.
   Copyright (C) 2001-2010 Roberto Bagnara <bagnara@cs.unipr.it>

This file is part of the Parma Polyhedra Library (PPL).

The PPL is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
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

#include <ppl-config.h>
#include "PIP_Tree.defs.hh"
#include "PIP_Problem.defs.hh"

#include <algorithm>
#include <memory>

namespace Parma_Polyhedra_Library {

namespace {

// Calculate positive modulo of x % y
void
mod_assign(Coefficient& z,
           Coefficient_traits::const_reference x,
           Coefficient_traits::const_reference y) {
  z = x % y;
  if (z < 0)
    z += y;
}

// Compute x += c * y
void
add_mul_assign_row(Row& x,
                   Coefficient_traits::const_reference c, const Row& y) {
  PPL_ASSERT(x.size() == y.size());
  for (dimension_type i = x.size(); i-- > 0; )
    add_mul_assign(x[i], c, y[i]);
}

// Compute x -= y
void
sub_assign(Row& x, const Row& y) {
  PPL_ASSERT(x.size() == y.size());
  for (dimension_type i = x.size(); i-- > 0; )
    x[i] -= y[i];
}

// Merge constraint system to a Matrix-form context such as x = x U y
void
merge_assign(Matrix& x,
             const Constraint_System& y,
             const Variables_Set& parameters) {
  PPL_ASSERT(parameters.size() == x.num_columns() - 1);
  const dimension_type new_rows = std::distance(y.begin(), y.end());
  if (new_rows == 0)
    return;
  const dimension_type old_num_rows = x.num_rows();
  x.add_zero_rows(new_rows, Row::Flags());

  // Compute once for all.
  const dimension_type cs_space_dim = y.space_dimension();
  const Variables_Set::const_iterator param_begin = parameters.begin();
  const Variables_Set::const_iterator param_end = parameters.end();

  dimension_type i = old_num_rows;
  for (Constraint_System::const_iterator y_i = y.begin(),
         y_end = y.end(); y_i != y_end; ++y_i, ++i) {
    PPL_ASSERT(y_i->is_nonstrict_inequality());
    Row& x_i = x[i];
    x_i[0] = y_i->inhomogeneous_term();
    Variables_Set::const_iterator pj;
    dimension_type j = 1;
    for (pj = param_begin; pj != param_end; ++pj, ++j) {
      Variable vj(*pj);
      if (vj.space_dimension() > cs_space_dim)
        break;
      x_i[j] = y_i->coefficient(vj);
    }
  }
}

// Assigns to row x the negation of row y.
void
neg_assign_row(Row& x, const Row& y) {
  PPL_ASSERT(x.size() == y.size());
  for (dimension_type i = x.size(); i-- > 0; )
    neg_assign(x[i], y[i]);
}

// Given context row \p y and denominator \p den,
// to be interpreted as expression expr = y / den,
// assigns to context row \p x a new value such that
//     x / den == - expr - 1.
inline void
complement_assign(Row& x, const Row& y,
                  Coefficient_traits::const_reference den) {
  PPL_ASSERT(den > 0);
  neg_assign_row(x, y);
  if (den == 1)
    --x[0];
  else {
    PPL_DIRTY_TEMP_COEFFICIENT(mod);
    mod_assign(mod, x[0], den);
    x[0] -= (mod == 0) ? den : mod;
  }
}

// Update given context matrix using local artificials
dimension_type
update_context(Matrix& context,
               const PIP_Tree_Node::Artificial_Parameter_Sequence& ap) {
  const dimension_type ap_size = ap.size();
  if (ap_size > 0)
    context.add_zero_columns(ap_size);
  return ap_size;
}

// Update given context matrix and parameter set using local artificials
void
update_context(Variables_Set& params, Matrix& context,
               const PIP_Tree_Node::Artificial_Parameter_Sequence& ap,
               dimension_type& space_dimension) {
  const dimension_type ap_size = update_context(context, ap);
  // Update parameters.
  for (dimension_type i = 0; i < ap_size; ++i)
    params.insert(space_dimension + i);
  // Update space dimension.
  space_dimension += ap_size;
}

/* Compares two columns lexicographically in revised simplex tableau
  - Returns true if (column ja)*(-cst_a)/pivot_a[ja]
                    << (column jb)*(-cst_b)/pivot_b[jb]
  - Returns false otherwise
*/
bool
column_lower(const Matrix& tableau,
             const std::vector<dimension_type>& mapping,
             const std::vector<bool>& basis,
             const Row& pivot_a,
             const dimension_type ja,
             const Row& pivot_b,
             const dimension_type jb,
             Coefficient_traits::const_reference cst_a = -1,
             Coefficient_traits::const_reference cst_b = -1) {
  const Coefficient& sij_a = pivot_a[ja];
  const Coefficient& sij_b = pivot_b[jb];
  PPL_ASSERT(sij_a > 0);
  PPL_ASSERT(sij_b > 0);

  PPL_DIRTY_TEMP_COEFFICIENT(lhs_coeff);
  PPL_DIRTY_TEMP_COEFFICIENT(rhs_coeff);
  lhs_coeff = cst_a * sij_b;
  rhs_coeff = cst_b * sij_a;

  if (ja == jb) {
    // Same column: just compare the ratios.
    // This works since all columns are lexico-positive.
    // return cst_a * sij_b > cst_b * sij_a;
    return lhs_coeff > rhs_coeff;
  }

  PPL_DIRTY_TEMP_COEFFICIENT(lhs);
  PPL_DIRTY_TEMP_COEFFICIENT(rhs);
  const dimension_type num_vars = mapping.size();
  dimension_type k = 0;
  // While loop guard is: (k < num_rows && lhs == rhs).
  // Return value is false, if k >= num_rows; lhs < rhs, otherwise.
  // Try to optimize the computation of lhs and rhs.
  while (true) {
    const dimension_type mk = mapping[k];
    const bool in_base = basis[k];
    if (++k >= num_vars)
      return false;
    if (in_base) {
      // Reconstitute the identity submatrix part of tableau.
      if (mk == ja) {
        // Optimizing for: lhs == lhs_coeff && rhs == 0;
        if (lhs_coeff == 0)
          continue;
        else
          return lhs_coeff > 0;
      }
      if (mk == jb) {
        // Optimizing for: lhs == 0 && rhs == rhs_coeff;
        if (rhs_coeff == 0)
          continue;
        else
          return 0 > rhs_coeff;
      }
      // Optimizing for: lhs == 0 && rhs == 0;
      continue;
    } else {
      // Not in base.
      const Row& t_mk = tableau[mk];
      lhs = lhs_coeff * t_mk[ja];
      rhs = rhs_coeff * t_mk[jb];
      if (lhs == rhs)
        continue;
      else
        return lhs > rhs;
    }
  }
  // This point should be unreachable.
  throw std::runtime_error("PPL internal error");
}

/* Find the column j in revised simplex tableau such as
  - pivot_row[j] is positive
  - (column j) / pivot_row[j] is lexico-minimal
*/
bool
find_lexico_minimum_column(const Matrix& tableau,
                           const std::vector<dimension_type>& mapping,
                           const std::vector<bool>& basis,
                           const Row& pivot_row,
                           const dimension_type start_j,
                           dimension_type& j_out) {
  const dimension_type num_cols = tableau.num_columns();
  bool has_positive_coefficient = false;

  j_out = num_cols;
  for (dimension_type j = start_j; j < num_cols; ++j) {
    const Coefficient& c = pivot_row[j];
    if (c <= 0)
      continue;
    has_positive_coefficient = true;
    if (j_out == num_cols
        || column_lower(tableau, mapping, basis,
                        pivot_row, j, pivot_row, j_out))
      j_out = j;
  }
  return has_positive_coefficient;
}

// Divide all coefficients in row x and denominator y by their GCD.
void
row_normalize(Row& x, Coefficient& den) {
  if (den == 1)
    return;
  const dimension_type x_size = x.size();
  PPL_DIRTY_TEMP_COEFFICIENT(gcd);
  gcd = den;
  for (dimension_type i = x_size; i-- > 0; ) {
    const Coefficient& x_i = x[i];
    if (x_i != 0) {
      gcd_assign(gcd, x_i, gcd);
      if (gcd == 1)
        return;
    }
  }
  // Divide the coefficients by the GCD.
  for (dimension_type i = x_size; i-- > 0; ) {
    Coefficient& x_i = x[i];
    exact_div_assign(x_i, x_i, gcd);
  }
  // Divide the denominator by the GCD.
  exact_div_assign(den, den, gcd);
}

} // namespace

namespace IO_Operators {

std::ostream&
operator<<(std::ostream& os, const PIP_Tree_Node::Artificial_Parameter& x) {
  const Linear_Expression& expr = static_cast<const Linear_Expression&>(x);
  os << "(" << expr << ") div " << x.denominator();
  return os;
}

} // namespace IO_Operators

PIP_Tree_Node::PIP_Tree_Node()
  : parent_(0),
    constraints_(),
    artificial_parameters() {
}

PIP_Tree_Node::PIP_Tree_Node(const PIP_Tree_Node& y)
  : parent_(0), // Parent is not copied.
    constraints_(y.constraints_),
    artificial_parameters(y.artificial_parameters) {
}

bool
PIP_Tree_Node::Artificial_Parameter
::operator==(const PIP_Tree_Node::Artificial_Parameter& y) const {
  const Artificial_Parameter& x = *this;
  if (x.space_dimension() != y.space_dimension())
    return false;
  if (x.denom != y.denom)
    return false;
  if (x.inhomogeneous_term() != y.inhomogeneous_term())
    return false;
  for (dimension_type i = x.space_dimension(); i-- > 0; )
    if (x.coefficient(Variable(i)) != y.coefficient(Variable(i)))
      return false;
  return true;
}

bool
PIP_Tree_Node::Artificial_Parameter
::operator!=(const PIP_Tree_Node::Artificial_Parameter& y) const {
  return !operator==(y);
}

void
PIP_Tree_Node::Artificial_Parameter::ascii_dump(std::ostream& s) const {
  s << "artificial_parameter ";
  Linear_Expression::ascii_dump(s);
  s << " / " << denom << "\n";
}

bool
PIP_Tree_Node::Artificial_Parameter::ascii_load(std::istream& s) {
  std::string str;
  if (!(s >> str) || str != "artificial_parameter")
    return false;
  if (!Linear_Expression::ascii_load(s))
    return false;
  if (!(s >> str) || str != "/")
    return false;
  if (!(s >> denom))
    return false;
  PPL_ASSERT(OK());
  return true;
}

PPL_OUTPUT_DEFINITIONS(PIP_Tree_Node::Artificial_Parameter)

PIP_Solution_Node::PIP_Solution_Node()
  : PIP_Tree_Node(),
    tableau(),
    basis(),
    mapping(),
    var_row(),
    var_column(),
    special_equality_row(0),
    big_dimension(not_a_dimension()),
    sign(),
    solution(),
    solution_valid(false) {
}

PIP_Solution_Node::PIP_Solution_Node(const PIP_Solution_Node& y)
  : PIP_Tree_Node(y),
    tableau(y.tableau),
    basis(y.basis),
    mapping(y.mapping),
    var_row(y.var_row),
    var_column(y.var_column),
    special_equality_row(y.special_equality_row),
    big_dimension(y.big_dimension),
    sign(y.sign),
    solution(y.solution),
    solution_valid(y.solution_valid) {
}

PIP_Solution_Node::PIP_Solution_Node(const PIP_Solution_Node& y,
                                     No_Constraints)
  : PIP_Tree_Node(),
    tableau(y.tableau),
    basis(y.basis),
    mapping(y.mapping),
    var_row(y.var_row),
    var_column(y.var_column),
    special_equality_row(y.special_equality_row),
    big_dimension(y.big_dimension),
    sign(y.sign),
    solution(y.solution),
    solution_valid(y.solution_valid) {
}

PIP_Solution_Node::~PIP_Solution_Node() {
}

PIP_Decision_Node::PIP_Decision_Node(PIP_Tree_Node* fcp,
                                     PIP_Tree_Node* tcp)
  : PIP_Tree_Node(),
    true_child(tcp),
    false_child(fcp) {
  if (fcp != 0)
    fcp->set_parent(this);
  if (tcp != 0)
    tcp->set_parent(this);
}

PIP_Decision_Node ::PIP_Decision_Node(const PIP_Decision_Node& y)
  : PIP_Tree_Node(y),
    true_child(0),
    false_child(0) {
  if (y.true_child != 0) {
    true_child = y.true_child->clone();
    true_child->set_parent(this);
  }
  if (y.false_child != 0) {
    false_child = y.false_child->clone();
    false_child->set_parent(this);
  }
}

PIP_Decision_Node::~PIP_Decision_Node() {
  delete true_child;
  delete false_child;
}

const PIP_Solution_Node*
PIP_Tree_Node::as_solution() const {
  return 0;
}

const PIP_Decision_Node*
PIP_Tree_Node::as_decision() const {
  return 0;
}

const PIP_Solution_Node*
PIP_Solution_Node::as_solution() const {
  return this;
}

const PIP_Decision_Node*
PIP_Decision_Node::as_decision() const {
  return this;
}

dimension_type
PIP_Tree_Node::insert_artificials(Variables_Set& params,
                                  const dimension_type space_dimension) const {
  const dimension_type ap_size = artificial_parameters.size();
  PPL_ASSERT(space_dimension >= ap_size);
  dimension_type sd = space_dimension - ap_size;
  const dimension_type parent_size
    = (parent_ == 0) ? 0 : parent_->insert_artificials(params, sd);
  if (ap_size > 0) {
    for (dimension_type i = 0; i < ap_size; ++i)
      params.insert(sd++);
  }
  return parent_size + ap_size;
}

bool
PIP_Solution_Node::Tableau::OK() const {
  if (s.num_rows() != t.num_rows()) {
#ifndef NDEBUG
    std::cerr << "PIP_Solution_Node::Tableau matrices "
              << "have a different number of rows.\n";
#endif
    return false;
  }

  if (!s.OK() || !t.OK()) {
#ifndef NDEBUG
    std::cerr << "A PIP_Solution_Node::Tableau matrix is broken.\n";
#endif
    return false;
  }

  if (denom <= 0) {
#ifndef NDEBUG
    std::cerr << "PIP_Solution_Node::Tableau with non-positive denominator.\n";
#endif
    return false;
  }

  // All tests passed.
  return true;
}

bool
PIP_Tree_Node::OK() const {
#ifndef NDEBUG
  using std::endl;
  using std::cerr;
#endif
  const Constraint_System::const_iterator begin = constraints_.begin();
  const Constraint_System::const_iterator end = constraints_.end();

  // Parameter constraint system should contain no strict inequalities.
  for (Constraint_System::const_iterator ci = begin; ci != end; ++ci)
    if (ci->is_strict_inequality()) {
#ifndef NDEBUG
      cerr << "The feasible region of the PIP_Problem parameter context"
           << "is defined by a constraint system containing strict "
           << "inequalities."
	   << endl;
      ascii_dump(cerr);
#endif
      return false;
    }
  return true;
}

void
PIP_Tree_Node
::add_constraint(const Row& row, const Variables_Set& parameters) {
  const dimension_type num_params = parameters.size();
  PPL_ASSERT(num_params + 1 == row.size());

  // Compute the expression for the parameter constraint.
  Linear_Expression expr = Linear_Expression(row[0]);
  // NOTE: iterating downward on parameters to avoid reallocations.
  Variables_Set::const_reverse_iterator p_j = parameters.rbegin();
  // NOTE: index j spans [1..num_params] downwards.
  for (dimension_type j = num_params; j > 0; --j) {
    add_mul_assign(expr, row[j], Variable(*p_j));
    // Move to previous parameter.
    ++p_j;
  }

  // Add the parameter constraint.
  constraints_.insert(expr >= 0);
}

bool
PIP_Solution_Node::OK() const {
#ifndef NDEBUG
  using std::cerr;
#endif
  if (!PIP_Tree_Node::OK())
    return false;

  // Check that every member used is OK.

  if (!tableau.OK())
    return false;

  // Check coherency of basis, mapping, var_row and var_column
  if (basis.size() != mapping.size()) {
#ifndef NDEBUG
    cerr << "The PIP_Solution_Node::basis and PIP_Solution_Node::mapping "
         << "vectors do not have the same number of elements.\n";
#endif
    return false;
  }
  if (basis.size() != var_row.size() + var_column.size()) {
#ifndef NDEBUG
    cerr << "The sum of number of elements in the PIP_Solution_Node::var_row "
         << "and PIP_Solution_Node::var_column vectors is different from the "
         << "number of elements in the PIP_Solution_Node::basis vector.\n";
#endif
    return false;
  }
  if (var_column.size() != tableau.s.num_columns()) {
#ifndef NDEBUG
    cerr << "The number of elements in the PIP_Solution_Node::var_column "
         << "vector is different from the number of columns in the "
         << "PIP_Solution_Node::tableau.s Matrix.\n";
#endif
    return false;
  }
  if (var_row.size() != tableau.s.num_rows()) {
#ifndef NDEBUG
    cerr << "The number of elements in the PIP_Solution_Node::var_row "
         << "vector is different from the number of rows in the "
         << "PIP_Solution_Node::tableau.s Matrix.\n";
#endif
    return false;
  }
  for (dimension_type i = mapping.size(); i-- > 0; ) {
    const dimension_type rowcol = mapping[i];
    if (basis[i] && var_column[rowcol] != i) {
#ifndef NDEBUG
      cerr << "Variable " << i << " is basic and corresponds to column "
           << rowcol << " but PIP_Solution_Node::var_column[" << rowcol
           << "] does not correspond to variable " << i << ".\n";
#endif
      return false;
    }
    if (!basis[i] && var_row[rowcol] != i) {
#ifndef NDEBUG
      cerr << "Variable " << i << " is nonbasic and corresponds to row "
           << rowcol << " but PIP_Solution_Node::var_row[" << rowcol
           << "] does not correspond to variable " << i << ".\n";
#endif
      return false;
    }
  }
  // All checks passed.
  return true;
}

bool
PIP_Decision_Node::OK() const {
  /* FIXME: finish me! */

  // Perform base class well-formedness check on this node.
  if (!PIP_Tree_Node::OK())
    return false;

  // Recursively check if child nodes are well-formed.
  if (true_child && !true_child->OK())
    return false;
  if (false_child && !false_child->OK())
    return false;

  // Decision nodes with a false child must have exactly one constraint.
  if (false_child) {
    dimension_type
      dist = std::distance(constraints_.begin(), constraints_.end());
    if (dist != 1) {
#ifndef NDEBUG
      std::cerr << "PIP_Decision_Node with a 'false' child has "
                << dist << " parametric constraints (should be 1).\n";
#endif
      return false;
    }
  }

  // All checks passed.
  return true;
}

void
PIP_Decision_Node::update_tableau(const PIP_Problem& problem,
                                  const dimension_type external_space_dim,
                                  const dimension_type first_pending_constraint,
                                  const Constraint_Sequence& input_cs,
                                  const Variables_Set& parameters) {
  true_child->update_tableau(problem,
                             external_space_dim,
                             first_pending_constraint,
                             input_cs,
                             parameters);
  if (false_child)
    false_child->update_tableau(problem,
                                external_space_dim,
                                first_pending_constraint,
                                input_cs,
                                parameters);
  PPL_ASSERT(OK());
}

PIP_Tree_Node*
PIP_Decision_Node::solve(const PIP_Problem& problem,
                         const Matrix& context,
                         const Variables_Set& params,
                         dimension_type space_dimension) {
  PPL_ASSERT(true_child != 0);
  Matrix context_true(context);
  Variables_Set parameters(params);
  update_context(parameters, context_true, artificial_parameters,
                 space_dimension);
  merge_assign(context_true, constraints_, parameters);
  true_child = true_child->solve(problem, context_true,
                                 parameters, space_dimension);

  if (false_child) {
    // Decision nodes with false child must have exactly one constraint
    PPL_ASSERT(1 == std::distance(constraints_.begin(), constraints_.end()));
    Matrix context_false(context);
    update_context(context_false, artificial_parameters);
    merge_assign(context_false, constraints_, parameters);
    Row& last = context_false[context_false.num_rows()-1];
    complement_assign(last, last, 1);
    false_child = false_child->solve(problem, context_false,
                                     parameters, space_dimension);
  }

  if (true_child != 0 || false_child != 0)
    return this;
  else {
    delete this;
    return 0;
  }
}

void
PIP_Decision_Node::ascii_dump(std::ostream& s) const {
  // Dump base class info.
  PIP_Tree_Node::ascii_dump(s);

  // Dump true child (if any).
  s << "\ntrue_child: ";
  if (true_child == 0)
    s << "BOTTOM\n";
  else if (const PIP_Decision_Node* dec = true_child->as_decision()) {
    s << "DECISION\n";
    dec->ascii_dump(s);
  }
  else {
    const PIP_Solution_Node* sol = true_child->as_solution();
    PPL_ASSERT(sol != 0);
    s << "SOLUTION\n";
    sol->ascii_dump(s);
  }

  // Dump false child (if any).
  s << "\nfalse_child: ";
  if (false_child == 0)
    s << "BOTTOM\n";
  else if (const PIP_Decision_Node* dec = false_child->as_decision()) {
    s << "DECISION\n";
    dec->ascii_dump(s);
  }
  else {
    const PIP_Solution_Node* sol = false_child->as_solution();
    PPL_ASSERT(sol != 0);
    s << "SOLUTION\n";
    sol->ascii_dump(s);
  }
}

bool
PIP_Decision_Node::ascii_load(std::istream& s) {
  std::string str;

  // Load base class info.
  if (!PIP_Tree_Node::ascii_load(s))
    return false;

  // Release the "true" subtree (if any).
  delete true_child;
  true_child = 0;

  // Load true child (if any).
  if (!(s >> str) || str != "true_child:")
    return false;
  if (!(s >> str))
    return false;
  if (str == "BOTTOM")
    true_child = 0;
  else if (str == "DECISION") {
    PIP_Decision_Node* dec = new PIP_Decision_Node(0, 0);
    true_child = dec;
    if (!dec->ascii_load(s))
      return false;
  }
  else if (str == "SOLUTION") {
    PIP_Solution_Node* sol = new PIP_Solution_Node;
    true_child = sol;
    if (!sol->ascii_load(s))
      return false;
  }
  else
    // Unknown node kind.
    return false;

  // Release the "false" subtree (if any).
  delete false_child;
  false_child = 0;

  // Load false child (if any).
  if (!(s >> str) || str != "false_child:")
    return false;
  if (!(s >> str))
    return false;
  if (str == "BOTTOM")
    false_child = 0;
  else if (str == "DECISION") {
    PIP_Decision_Node* dec = new PIP_Decision_Node(0, 0);
    false_child = dec;
    if (!dec->ascii_load(s))
      return false;
  }
  else if (str == "SOLUTION") {
    PIP_Solution_Node* sol = new PIP_Solution_Node;
    false_child = sol;
    if (!sol->ascii_load(s))
      return false;
  }
  else
    // Unknown node kind.
    return false;

  // Loaded all info.
  PPL_ASSERT(OK());
  return true;
}


void
PIP_Solution_Node::Tableau::normalize() {
  if (denom == 1)
    return;

  const dimension_type num_rows = s.num_rows();
  const dimension_type s_cols = s.num_columns();
  const dimension_type t_cols = t.num_columns();

  // Compute global gcd.
  PPL_DIRTY_TEMP_COEFFICIENT(gcd);
  gcd = denom;
  for (dimension_type i = num_rows; i-- > 0; ) {
    const Row& s_i = s[i];
    for (dimension_type j = s_cols; j-- > 0; ) {
      const Coefficient& s_ij = s_i[j];
      if (s_ij != 0) {
        gcd_assign(gcd, s_ij, gcd);
        if (gcd == 1)
          return;
      }
    }
    const Row& t_i = t[i];
    for (dimension_type j = t_cols; j-- > 0; ) {
      const Coefficient& t_ij = t_i[j];
      if (t_ij != 0) {
        gcd_assign(gcd, t_ij, gcd);
        if (gcd == 1)
          return;
      }
    }
  }

  PPL_ASSERT(gcd > 1);
  // Normalize all coefficients.
  for (dimension_type i = num_rows; i-- > 0; ) {
    Row& s_i = s[i];
    for (dimension_type j = s_cols; j-- > 0; ) {
      Coefficient& s_ij = s_i[j];
      exact_div_assign(s_ij, s_ij, gcd);
    }
    Row& t_i = t[i];
    for (dimension_type j = t_cols; j-- > 0; ) {
      Coefficient& t_ij = t_i[j];
      exact_div_assign(t_ij, t_ij, gcd);
    }
  }
  // Normalize denominator.
  exact_div_assign(denom, denom, gcd);
}

void
PIP_Solution_Node::Tableau::scale(Coefficient_traits::const_reference ratio) {
  for (dimension_type i = s.num_rows(); i-- > 0; ) {
    Row& s_i = s[i];
    for (dimension_type j = s.num_columns(); j-- > 0; )
      s_i[j] *= ratio;
    Row& t_i = t[i];
    for (dimension_type j = t.num_columns(); j-- > 0; )
      t_i[j] *= ratio;
  }
  denom *= ratio;
}

bool
PIP_Solution_Node::Tableau
::is_better_pivot(const std::vector<dimension_type>& mapping,
                  const std::vector<bool>& basis,
                  const dimension_type row_0,
                  const dimension_type col_0,
                  const dimension_type row_1,
                  const dimension_type col_1) const {
  const dimension_type num_params = t.num_columns();
  const dimension_type num_rows = s.num_rows();
  const Row& s_0 = s[row_0];
  const Row& s_1 = s[row_1];
  const Coefficient& s_0_0 = s_0[col_0];
  const Coefficient& s_1_1 = s_1[col_1];
  const Row& t_0 = t[row_0];
  const Row& t_1 = t[row_1];
  PPL_DIRTY_TEMP_COEFFICIENT(coeff_0);
  PPL_DIRTY_TEMP_COEFFICIENT(coeff_1);
  PPL_DIRTY_TEMP_COEFFICIENT(product_0);
  PPL_DIRTY_TEMP_COEFFICIENT(product_1);
  // On exit from the loop, if j_mismatch == num_params then
  // no column mismatch was found.
  dimension_type j_mismatch = num_params;
  for (dimension_type j = 0; j < num_params; ++j) {
    coeff_0 = t_0[j] * s_1_1;
    coeff_1 = t_1[j] * s_0_0;
    for (dimension_type i = 0; i < num_rows; ++i) {
      const Row& s_i = s[i];
      product_0 = coeff_0 * s_i[col_0];
      product_1 = coeff_1 * s_i[col_1];
      if (product_0 != product_1) {
        // Mismatch found: exit from both loops.
        j_mismatch = j;
        goto end_loop;
      }
    }
  }

 end_loop:
  return (j_mismatch != num_params)
    && column_lower(s, mapping, basis, s_0, col_0, s_1, col_1,
                    t_0[j_mismatch], t_1[j_mismatch]);
}

void
PIP_Tree_Node::ascii_dump(std::ostream& s) const {
  s << "constraints_\n";
  constraints_.ascii_dump(s);
  dimension_type artificial_parameters_size = artificial_parameters.size();
  s << "\nartificial_parameters( " << artificial_parameters_size << " )\n";
  for (dimension_type i = 0; i < artificial_parameters_size; ++i)
    artificial_parameters[i].ascii_dump(s);
}

bool
PIP_Tree_Node::ascii_load(std::istream& s) {
  std::string str;
  if (!(s >> str) || str != "constraints_")
    return false;
  constraints_.ascii_load(s);

  if (!(s >> str) || str != "artificial_parameters(")
    return false;
  dimension_type artificial_parameters_size;
  if (!(s >> artificial_parameters_size))
    return false;
  if (!(s >> str) || str != ")")
    return false;
  Artificial_Parameter ap;
  for (dimension_type i = 0; i < artificial_parameters_size; ++i) {
    if (!ap.ascii_load(s))
      return false;
    artificial_parameters.push_back(ap);
  }

  PPL_ASSERT(OK());
  return true;
}

PIP_Tree_Node*
PIP_Solution_Node::clone() const {
  return new PIP_Solution_Node(*this);
}

PIP_Tree_Node*
PIP_Decision_Node::clone() const {
  return new PIP_Decision_Node(*this);
}

void
PIP_Solution_Node::Tableau::ascii_dump(std::ostream& st) const {
  st << "denominator " << denom << "\n";
  st << "variables ";
  s.ascii_dump(st);
  st << "parameters ";
  t.ascii_dump(st);
}

bool
PIP_Solution_Node::Tableau::ascii_load(std::istream& st) {
  std::string str;
  if (!(st >> str) || str != "denominator")
    return false;
  Coefficient den;
  if (!(st >> den))
    return false;
  denom = den;
  if (!(st >> str) || str != "variables")
    return false;
  if (!s.ascii_load(st))
    return false;
  if (!(st >> str) || str != "parameters")
    return false;
  if (!t.ascii_load(st))
    return false;
  return true;
}

void
PIP_Solution_Node::ascii_dump(std::ostream& s) const {
  PIP_Tree_Node::ascii_dump(s);

  s << "\ntableau\n";
  tableau.ascii_dump(s);

  s << "\nbasis ";
  dimension_type basis_size = basis.size();
  s << basis_size;
  for (dimension_type i = 0; i < basis_size; ++i)
    s << (basis[i] ? " true" : " false");

  s << "\nmapping ";
  dimension_type mapping_size = mapping.size();
  s << mapping_size;
  for (dimension_type i = 0; i < mapping_size; ++i)
    s << " " << mapping[i];

  s << "\nvar_row ";
  dimension_type var_row_size = var_row.size();
  s << var_row_size;
  for (dimension_type i = 0; i < var_row_size; ++i)
    s << " " << var_row[i];

  s << "\nvar_column ";
  dimension_type var_column_size = var_column.size();
  s << var_column_size;
  for (dimension_type i = 0; i < var_column_size; ++i)
    s << " " << var_column[i];
  s << "\n";

  s << "special_equality_row " << special_equality_row << "\n";
  s << "big_dimension " << big_dimension << "\n";

  s << "sign ";
  dimension_type sign_size = sign.size();
  s << sign_size;
  for (dimension_type i = 0; i < sign_size; ++i) {
    s << " ";
    switch (sign[i]) {
    case UNKNOWN:
      s << "UNKNOWN";
      break;
    case ZERO:
      s << "ZERO";
      break;
    case POSITIVE:
      s << "POSITIVE";
      break;
    case NEGATIVE:
      s << "NEGATIVE";
      break;
    case MIXED:
      s << "MIXED";
      break;
    }
  }
  s << "\n";

  dimension_type solution_size = solution.size();
  s << "solution " << solution_size << "\n";
  for (dimension_type i = 0; i < solution_size; ++i)
    solution[i].ascii_dump(s);
  s << "\n";

  s << "solution_valid " << (solution_valid ? "true" : "false") << "\n";
}

bool
PIP_Solution_Node::ascii_load(std::istream& s) {
  if (!PIP_Tree_Node::ascii_load(s))
    return false;

  std::string str;
  if (!(s >> str) || str != "tableau")
    return false;
  if (!tableau.ascii_load(s))
    return false;

  if (!(s >> str) || str != "basis")
    return false;
  dimension_type basis_size;
  if (!(s >> basis_size))
    return false;
  basis.clear();
  for (dimension_type i = 0; i < basis_size; ++i) {
    if (!(s >> str))
      return false;
    bool val = false;
    if (str == "true")
      val = true;
    else if (str != "false")
      return false;
    basis.push_back(val);
  }

  if (!(s >> str) || str != "mapping")
    return false;
  dimension_type mapping_size;
  if (!(s >> mapping_size))
    return false;
  mapping.clear();
  for (dimension_type i = 0; i < mapping_size; ++i) {
    dimension_type val;
    if (!(s >> val))
      return false;
    mapping.push_back(val);
  }

  if (!(s >> str) || str != "var_row")
    return false;
  dimension_type var_row_size;
  if (!(s >> var_row_size))
    return false;
  var_row.clear();
  for (dimension_type i = 0; i < var_row_size; ++i) {
    dimension_type val;
    if (!(s >> val))
      return false;
    var_row.push_back(val);
  }

  if (!(s >> str) || str != "var_column")
    return false;
  dimension_type var_column_size;
  if (!(s >> var_column_size))
    return false;
  var_column.clear();
  for (dimension_type i = 0; i < var_column_size; ++i) {
    dimension_type val;
    if (!(s >> val))
      return false;
    var_column.push_back(val);
  }

  if (!(s >> str) || str != "special_equality_row")
    return false;
  if (!(s >> special_equality_row))
    return false;

  if (!(s >> str) || str != "big_dimension")
    return false;
  if (!(s >> big_dimension))
    return false;

  if (!(s >> str) || str != "sign")
    return false;
  dimension_type sign_size;
  if (!(s >> sign_size))
    return false;
  sign.clear();
  for (dimension_type i = 0; i < sign_size; ++i) {
    if (!(s >> str))
      return false;
    Row_Sign val;
    if (str == "UNKNOWN")
      val = UNKNOWN;
    else if (str == "ZERO")
      val = ZERO;
    else if (str == "POSITIVE")
      val = POSITIVE;
    else if (str == "NEGATIVE")
      val = NEGATIVE;
    else if (str == "MIXED")
      val = MIXED;
    else
      return false;
    sign.push_back(val);
  }

  if (!(s >> str) || str != "solution")
    return false;
  dimension_type solution_size;
  if (!(s >> solution_size))
    return false;
  solution.clear();
  for (dimension_type i = 0; i < solution_size; ++i) {
    Linear_Expression val;
    if (!val.ascii_load(s))
      return false;
    solution.push_back(val);
  }

  if (!(s >> str) || str != "solution_valid")
    return false;
  if (!(s >> str))
    return false;
  if (str == "true")
    solution_valid = true;
  else if (str == "false")
    solution_valid = false;
  else
    return false;

  PPL_ASSERT(OK());
  return true;
}

// FIXME: this does not (yet) correspond to specification.
const Linear_Expression&
PIP_Solution_Node
::parametric_values(const Variable var,
                    const Variables_Set& parameters) const {
  Variables_Set all_parameters(parameters);
  // Complete the parameter set with artificials.
  insert_artificials(all_parameters,
                     tableau.s.num_columns() + tableau.t.num_columns() - 1);
  {
    PIP_Solution_Node& x = const_cast<PIP_Solution_Node&>(*this);
    x.update_solution(all_parameters);
  }

  const Variables_Set::iterator pos = all_parameters.lower_bound(var.id());
  if (pos == all_parameters.end())
    return solution[var.id()];
  else {
    if (*pos == var.id())
      throw std::invalid_argument("PIP_Solution_Node::"
                                  "parametric_values(v, params): "
                                  "variable v is a parameter.");
    const dimension_type dist = std::distance(all_parameters.begin(), pos);
    return solution[var.id() - dist];
  }
}

PIP_Solution_Node::Row_Sign
PIP_Solution_Node::row_sign(const Row& x,
                            const dimension_type big_dimension) {
  if (big_dimension != not_a_dimension()) {
    // If a big parameter has been set and its coefficient is not zero,
    // then return the sign of the coefficient.
    const Coefficient& x_big = x[big_dimension];
    if (x_big > 0)
      return POSITIVE;
    if (x_big < 0)
      return NEGATIVE;
    // Otherwise x_big == 0, then no big parameter involved.
  }

  PIP_Solution_Node::Row_Sign sign = ZERO;
  for (int i = x.size(); i-- > 0; ) {
    const Coefficient& x_i = x[i];
    if (x_i > 0) {
      if (sign == NEGATIVE)
        return MIXED;
      sign = POSITIVE;
    }
    else if (x_i < 0) {
      if (sign == POSITIVE)
        return MIXED;
      sign = NEGATIVE;
    }
  }
  return sign;
}

bool
PIP_Solution_Node::compatibility_check(const Matrix& ctx, const Row& cnst) {
  Matrix s(ctx);
  // CHECKME: do ctx and cnst have compatible (row) capacity?
  s.add_row(cnst);
  PPL_ASSERT(s.OK());

  // Note: num_rows may increase.
  dimension_type num_rows = s.num_rows();
  const dimension_type num_cols = s.num_columns();
  const dimension_type num_vars = num_cols - 1;

  std::vector<Coefficient> scaling(num_rows, 1);
  std::vector<bool> basis;
  basis.reserve(num_vars + num_rows);
  std::vector<dimension_type> mapping;
  mapping.reserve(num_vars + num_rows);
  std::vector<dimension_type> var_row;
  var_row.reserve(num_rows);
  std::vector<dimension_type> var_column;
  var_column.reserve(num_cols);

  // Column 0 is the constant term, not a variable
  var_column.push_back(not_a_dimension());
  for (dimension_type j = 1; j <= num_vars; ++j) {
    basis.push_back(true);
    mapping.push_back(j);
    var_column.push_back(j-1);
  }
  for (dimension_type i = 0; i < num_rows; ++i) {
    basis.push_back(false);
    mapping.push_back(i);
    var_row.push_back(i+num_vars);
  }

  // Scaling factor (i.e., denominator) for pivot coefficients.
  PPL_DIRTY_TEMP_COEFFICIENT(pivot_den);
  // Allocate once and for all: short life temporaries.
  PPL_DIRTY_TEMP_COEFFICIENT(product);
  PPL_DIRTY_TEMP_COEFFICIENT(gcd);
  PPL_DIRTY_TEMP_COEFFICIENT(scale_factor);

  // Perform simplex pivots on the context
  // until we find an empty solution or an optimum.
  while (true) {
    dimension_type pi = num_rows; // pi is the pivot's row index.
    dimension_type pj = 0;        // pj is the pivot's column index.

    // Look for a negative RHS (i.e., constant term, stored in column 0),
    // maximizing pivot column.
    for (dimension_type i = 0; i < num_rows; ++i) {
      const Row& s_i = s[i];
      if (s_i[0] < 0) {
        dimension_type j;
        if (!find_lexico_minimum_column(s, mapping, basis, s_i, 1, j)) {
          // No positive pivot candidate: unfeasible problem.
          return false;
        }
        // Update pair (pi, pj) if they are still unset or
        // if the challenger pair (i, j) is better in the ordering.
        if (pj == 0
            || column_lower(s, mapping, basis,
                            s[pi], pj, s_i, j,
                            s[pi][0], s_i[0])) {
          pi = i;
          pj = j;
        }
      }
    }

    if (pj == 0) {
      // No negative RHS: fractional optimum found.
      // If it is integer, then the test is successful.
      // Otherwise, generate a new cut.
      bool all_integer_vars = true;
      // NOTE: iterating downwards would be correct, but it would change
      // the ordering of cut generation.
      for (dimension_type i = 0; i < num_vars; ++i) {
        if (basis[i])
          // Basic variable = 0, hence integer.
          continue;
        // Not a basic variable.
        const dimension_type mi = mapping[i];
        const Coefficient& den = scaling[mi];
        if (s[mi][0] % den == 0)
          continue;
        // Here constant term is not integer.
        all_integer_vars = false;
        // Generate a new cut.
        var_row.push_back(mapping.size());
        basis.push_back(false);
        mapping.push_back(num_rows);
        s.add_zero_rows(1, Row::Flags());
        Row& cut = s[num_rows];
        ++num_rows;
        const Row& s_mi = s[mi];
        for (dimension_type j = num_cols; j-- > 0; )
          mod_assign(cut[j], s_mi[j], den);
        cut[0] -= den;
        scaling.push_back(den);
      }
      // Check if an integer solution was found.
      if (all_integer_vars)
        return true;
      else
        continue;
    }

    // Here we have a positive s[pi][pj] pivot.

    // Normalize the tableau before pivoting.
    for (dimension_type i = num_rows; i-- > 0; )
      row_normalize(s[i], scaling[i]);

    // Update basis.
    {
      const dimension_type var_pi = var_row[pi];
      const dimension_type var_pj = var_column[pj];
      var_row[pi] = var_pj;
      var_column[pj] = var_pi;
      basis[var_pi] = true;
      basis[var_pj] = false;
      mapping[var_pi] = pj;
      mapping[var_pj] = pi;
    }

    // Create an identity row corresponding to basic variable pj.
    s.add_zero_rows(1, Row::Flags());
    Row& pivot = s[num_rows];
    pivot[pj] = 1;

    // Swap identity row with the pivot row previously found.
    std::swap(pivot, s[pi]);
    // Save original pivot scaling factor in a temporary,
    // then reset scaling factor for identity row.
    pivot_den = scaling[pi];
    scaling[pi] = 1;

    // Perform a pivot operation on the matrix.
    const Coefficient& pivot_pj = pivot[pj];
    for (dimension_type j = num_cols; j-- > 0; ) {
      if (j == pj)
        continue;
      const Coefficient& pivot_j = pivot[j];
      // Do nothing if the j-th pivot element is zero.
      if (pivot_j == 0)
        continue;
      for (dimension_type i = num_rows; i-- > 0; ) {
        Row& s_i = s[i];
        product = s_i[pj] * pivot_j;
        if (product % pivot_pj != 0) {
          // Must scale row s_i to stay in integer case.
          gcd_assign(gcd, product, pivot_pj);
          exact_div_assign(scale_factor, pivot_pj, gcd);
          for (dimension_type k = num_cols; k-- > 0; )
            s_i[k] *= scale_factor;
          product *= scale_factor;
          scaling[i] *= scale_factor;
        }
        PPL_ASSERT(product % pivot_pj == 0);
        exact_div_assign(product, product, pivot_pj);
        s_i[j] -= product;
      }
    }
    // Update column only if pivot coordinate != 1.
    if (pivot_pj != pivot_den) {
      for (dimension_type i = num_rows; i-- > 0; ) {
        Row& s_i = s[i];
        Coefficient& s_i_pj = s_i[pj];
        product = s_i_pj * pivot_den;
        if (product % pivot_pj != 0) {
          // As above, perform row scaling.
          gcd_assign(gcd, product, pivot_pj);
          exact_div_assign(scale_factor, pivot_pj, gcd);
          for (dimension_type k = num_cols; k-- > 0; )
            s_i[k] *= scale_factor;
          product *= scale_factor;
          scaling[i] *= scale_factor;
        }
        PPL_ASSERT(product % pivot_pj == 0);
        exact_div_assign(s_i_pj, product, pivot_pj);
      }
    }
    // Drop pivot to restore proper matrix size.
    s.erase_to_end(num_rows);
  }

  // This point should be unreachable.
  throw std::runtime_error("PPL internal error");
}

void
PIP_Solution_Node::update_tableau(const PIP_Problem& problem,
                                  const dimension_type external_space_dim,
                                  const dimension_type first_pending_constraint,
                                  const Constraint_Sequence& input_cs,
                                  const Variables_Set& parameters) {
  // Make sure a parameter column exists, for the inhomogeneous term.
  if (tableau.t.num_columns() == 0)
    tableau.t.add_zero_columns(1);

  // NOTE: here 'params' stands for problem (i.e., non artificial) parameters.
  const dimension_type old_num_vars = tableau.s.num_columns();
  const dimension_type old_num_params
    = problem.internal_space_dim - old_num_vars;
  const dimension_type num_added_dims
    = problem.external_space_dim - problem.internal_space_dim;
  const dimension_type new_num_params = parameters.size();
  const dimension_type num_added_params = new_num_params - old_num_params;
  const dimension_type num_added_vars = num_added_dims - num_added_params;

  const dimension_type old_num_art_params
    = tableau.t.num_columns() - 1 - old_num_params;

  // Resize the two tableau matrices.
  if (num_added_vars > 0)
    tableau.s.add_zero_columns(num_added_vars);
  if (num_added_params > 0)
    tableau.t.add_zero_columns(num_added_params);

  if (num_added_params > 0 && old_num_art_params > 0) {
    // Shift to the right the columns of artificial parameters.
    std::vector<dimension_type> swaps;
    swaps.reserve(3*old_num_art_params);
    const dimension_type first_ap = 1 + old_num_params;
    for (dimension_type i = 0; i < old_num_art_params; ++i) {
      dimension_type old_ap = first_ap + i;
      dimension_type new_ap = old_ap + num_added_params;
      swaps.push_back(old_ap);
      swaps.push_back(new_ap);
      swaps.push_back(0);
    }
    tableau.t.permute_columns(swaps);
  }

  dimension_type new_var_column = old_num_vars;
  const dimension_type initial_space_dim = old_num_vars + old_num_params;
  for (dimension_type i = initial_space_dim; i < external_space_dim; ++i) {
    if (parameters.count(i) == 0) {
      // A new problem variable.
      if (tableau.s.num_rows() == 0) {
        // No rows have been added yet
        basis.push_back(true);
        mapping.push_back(new_var_column);
      }
      else {
        /*
          Need to insert the original variable id
          before the slack variable id's to respect variable ordering.
        */
        basis.insert(basis.begin() + new_var_column, true);
        mapping.insert(mapping.begin() + new_var_column, new_var_column);
        // Update variable id's of slack variables.
        for (dimension_type j = var_row.size(); j-- > 0; )
          if (var_row[j] >= new_var_column)
            ++var_row[j];
        for (dimension_type j = var_column.size(); j-- > 0; )
          if (var_column[j] >= new_var_column)
            ++var_column[j];
        if (special_equality_row > 0)
          ++special_equality_row;
      }
      var_column.push_back(new_var_column);
      ++new_var_column;
    }
  }

  if (big_dimension == not_a_dimension()
      && problem.big_parameter_dimension != not_a_dimension()) {
    // Compute the column number of big parameter in tableau.t matrix.
    Variables_Set::const_iterator pos
      = parameters.find(problem.big_parameter_dimension);
    big_dimension = std::distance(parameters.begin(), pos) + 1;
  }

  const Coefficient& denom = tableau.denominator();
  for (Constraint_Sequence::const_iterator
         c_iter = input_cs.begin() + first_pending_constraint,
         c_end = input_cs.end(); c_iter != c_end; ++c_iter) {
    const Constraint& constraint = *c_iter;
    // (Tentatively) Add new rows to s and t matrices.
    // These will be removed at the end if they turn out to be useless.
    const dimension_type row_id = tableau.s.num_rows();
    tableau.s.add_zero_rows(1,  Row::Flags());
    tableau.t.add_zero_rows(1,  Row::Flags());
    Row& v_row = tableau.s[row_id];
    Row& p_row = tableau.t[row_id];

    // Setting the inhomogeneus term.
    p_row[0] = constraint.inhomogeneous_term();
    if (constraint.is_strict_inequality())
      // Transform (expr > 0) into (expr - 1 >= 0).
      --p_row[0];
    p_row[0] *= denom;

    dimension_type p_index = 1;
    dimension_type v_index = 0;
    for (dimension_type i = 0,
           i_end = constraint.space_dimension(); i != i_end; ++i) {
      const bool is_parameter = (1 == parameters.count(i));
      const Coefficient& coeff_i = constraint.coefficient(Variable(i));
      if (coeff_i == 0) {
        // Optimize computation below: only update p/v index.
        if (is_parameter)
          ++p_index;
        else
          ++v_index;
        // Jump to next iteration.
        continue;
      }

      if (is_parameter) {
        p_row[p_index] = coeff_i * denom;
        ++p_index;
      }
      else {
        const dimension_type mv = mapping[v_index];
        if (basis[v_index])
          // Basic variable : add coeff_i * x_i
          add_mul_assign(v_row[mv], coeff_i, denom);
        else {
          // Non-basic variable : add coeff_i * row_i
          add_mul_assign_row(v_row, coeff_i, tableau.s[mv]);
          add_mul_assign_row(p_row, coeff_i, tableau.t[mv]);
        }
        ++v_index;
      }
    }

    if (row_sign(v_row, not_a_dimension()) == ZERO) {
      // Parametric-only constraints have already been inserted in
      // initial context, so no need to insert them in the tableau.
      tableau.s.erase_to_end(row_id);
      tableau.t.erase_to_end(row_id);
    }
    else {
      const dimension_type var_id = mapping.size();
      sign.push_back(row_sign(p_row, big_dimension));
      basis.push_back(false);
      mapping.push_back(row_id);
      var_row.push_back(var_id);
      if (constraint.is_equality()) {
        // Handle equality constraints.
        // After having added the f_i(x,p) >= 0 constraint,
        // we must add -f_i(x,p) to the special equality row.
        if (special_equality_row == 0 || basis[special_equality_row]) {
          // The special constraint has not been created yet
          // FIXME: for now, we don't handle the case where the variable
          // is basic, and we just create a new row.
          // This might be faster however.
          tableau.s.add_zero_rows(1, Row::Flags());
          tableau.t.add_zero_rows(1, Row::Flags());
          neg_assign_row(tableau.s[1 + row_id], v_row);
          neg_assign_row(tableau.t[1 + row_id], p_row);
          sign.push_back(row_sign(tableau.t[1 + row_id], big_dimension));
          special_equality_row = mapping.size();
          basis.push_back(false);
          mapping.push_back(1 + row_id);
          var_row.push_back(1 + var_id);
        } else {
          // The special constraint already exists and is nonbasic.
          const dimension_type m_eq = mapping[special_equality_row];
          sub_assign(tableau.s[m_eq], v_row);
          sub_assign(tableau.t[m_eq], p_row);
        }
      }
    }
  }
  PPL_ASSERT(OK());
}

void
PIP_Solution_Node::update_solution(const Variables_Set& parameters) {
  if (solution_valid)
    return;

  const dimension_type num_vars = tableau.s.num_columns();
  if (solution.size() != num_vars)
    solution.resize(num_vars);

  // Compute once for all outside loop.
  const dimension_type num_params = parameters.size();
  const Variables_Set::const_reverse_iterator p_rbegin = parameters.rbegin();
  const Variables_Set::const_reverse_iterator p_rend = parameters.rend();

  const Coefficient& den = tableau.denominator();
  for (dimension_type i = num_vars; i-- > 0; ) {
    Linear_Expression& sol_i = solution[i];
    sol_i = Linear_Expression(0);
    if (basis[i])
      continue;
    Row& row = tableau.t[mapping[i]];
    dimension_type k = num_params;
    for (Variables_Set::const_reverse_iterator
           pj = p_rbegin; pj != p_rend; ++pj, --k)
      add_mul_assign(sol_i, row[k]/den, Variable(*pj));
    sol_i += row[0]/den;
  }
  solution_valid = true;
}

PIP_Tree_Node*
PIP_Solution_Node::solve(const PIP_Problem& problem,
                         const Matrix& ctx,
                         const Variables_Set& params,
                         dimension_type space_dim) {
  Matrix context(ctx);
  Variables_Set parameters(params);
  update_context(parameters, context, artificial_parameters, space_dim);
  merge_assign(context, constraints_, parameters);
  const dimension_type not_a_dim = not_a_dimension();

  // Main loop of the simplex algorithm.
  while (true) {
    PPL_ASSERT(OK());

    const dimension_type num_rows = tableau.t.num_rows();
    const dimension_type num_vars = tableau.s.num_columns();
    const dimension_type num_params = tableau.t.num_columns();
    const Coefficient& tableau_den = tableau.denominator();

#ifdef NOISY_PIP
    tableau.ascii_dump(std::cerr);
    std::cerr << "context ";
    context.ascii_dump(std::cerr);
#endif

    // (Re-) Compute parameter row signs.
    // While at it, keep track of the first parameter rows
    // having negative and mixed sign.
    dimension_type first_negative = not_a_dim;
    dimension_type first_mixed = not_a_dim;
    for (dimension_type i = 0; i < num_rows; ++i) {
      Row_Sign& sign_i = sign[i];
      if (sign_i == UNKNOWN || sign_i == MIXED)
        sign_i = row_sign(tableau.t[i], big_dimension);

      if (sign_i == NEGATIVE && first_negative == not_a_dim)
        first_negative = i;
      else if (sign_i == MIXED && first_mixed == not_a_dim)
        first_mixed = i;
    }

    // If no negative parameter row was found, try to refine the sign of
    // mixed rows using compatibility checks with the current context.
    if (first_negative == not_a_dim && first_mixed != not_a_dim) {
      for (dimension_type i = first_mixed; i < num_rows; ++i) {
        // Consider mixed sign parameter rows only.
        if (sign[i] != MIXED)
          continue;
        const Row& t_i = tableau.t[i];
        Row_Sign new_sign = ZERO;
        // Check compatibility for constraint t_i(z) >= 0.
        if (compatibility_check(context, t_i))
          new_sign = POSITIVE;
        // Check compatibility for constraint t_i(z) < 0,
        // i.e., -t_i(z) - 1 >= 0.
        Row c(num_params, Row::Flags());
        complement_assign(c, t_i, tableau_den);
        if (compatibility_check(context, c))
          new_sign = (new_sign == POSITIVE) ? MIXED : NEGATIVE;
        // Update sign for parameter row i.
        sign[i] = new_sign;
        // Maybe update first_negative and first_mixed.
        if (new_sign == NEGATIVE && first_negative == not_a_dim) {
          first_negative = i;
          if (i == first_mixed)
            first_mixed = not_a_dim;
        }
        else if (new_sign == MIXED) {
          if (first_mixed == not_a_dim)
            first_mixed = i;
        }
        else if (i == first_mixed)
          first_mixed = not_a_dim;
      }
    }

    // If there still is no negative parameter row and a mixed sign
    // parameter row (first_mixed) such that:
    //  - it has at least one positive variable coefficient;
    //  - constraint t_i(z) > 0 is not compatible with the context;
    // then this parameter row can be considered negative.
    if (first_negative == not_a_dim && first_mixed != not_a_dim) {
      for (dimension_type i = first_mixed; i < num_rows; ++i) {
        // Consider mixed sign parameter rows only.
        if (sign[i] != MIXED)
          continue;
        // Check for a positive variable coefficient.
        const Row& s_i = tableau.s[i];
        bool has_positive = false;
        for (dimension_type j = num_vars; j-- > 0; )
          if (s_i[j] > 0) {
            has_positive = true;
            break;
          }
        if (!has_positive)
          continue;
        // Check compatibility of constraint t_i(z) > 0.
        Row row(tableau.t[i]);
        PPL_DIRTY_TEMP_COEFFICIENT(mod);
        mod_assign(mod, row[0], tableau_den);
        row[0] -= (mod == 0) ? tableau_den : mod;
        const bool compatible = compatibility_check(context, row);
        // Maybe update sign (and first_* indices).
        if (compatible) {
          // Sign is still mixed.
          if (first_mixed == not_a_dim)
            first_mixed = i;
        }
        else {
          // Sign becomes negative (i.e., no longer mixed).
          sign[i] = NEGATIVE;
          if (first_negative == not_a_dim)
            first_negative = i;
          if (first_mixed == i)
            first_mixed = not_a_dim;
        }
      }
    }

#ifdef NOISY_PIP
    std::cerr << "sign =";
    for (dimension_type i = 0; i < sign.size(); ++i)
      std::cerr << " " << "?0+-*"[sign[i]];
    std::cerr << std::endl;
#endif

    // If we have found a negative parameter row, then
    // either the problem is unfeasible, or a pivoting step is required.
    if (first_negative != not_a_dim) {

      // Search for the best pivot row.
      dimension_type pi = not_a_dim;
      dimension_type pj = not_a_dim;
      for (dimension_type i = first_negative; i < num_rows; ++i) {
        if (sign[i] != NEGATIVE)
          continue;
        dimension_type j;
        if (!find_lexico_minimum_column(tableau.s, mapping, basis,
                                        tableau.s[i], 0, j)) {
          // No positive s_ij was found: problem is unfeasible.
#ifdef NOISY_PIP
          std::cerr << "No positive pivot found: Solution = _|_\n";
#endif
          delete this;
          return 0;
        }
        if (pj == not_a_dim
            || tableau.is_better_pivot(mapping, basis, i, j, pi, pj)) {
          // Update pivot indices.
          pi = i;
          pj = j;
          if (problem.control_parameters[PIP_Problem::PIVOT_ROW_STRATEGY]
              == PIP_Problem::PIVOT_ROW_STRATEGY_FIRST)
            // Stop at first valid row.
            break;
        }
      }

#ifdef NOISY_PIP
      std::cerr << "Pivot (pi, pj) = (" << pi << ", " << pj << ")\n";
#endif

      // Normalize the tableau before pivoting.
      tableau.normalize();

      // Perform pivot operation.

      // Update basis.
      {
        const dimension_type var_pi = var_row[pi];
        const dimension_type var_pj = var_column[pj];
        var_row[pi] = var_pj;
        var_column[pj] = var_pi;
        basis[var_pi] = true;
        basis[var_pj] = false;
        mapping[var_pi] = pj;
        mapping[var_pj] = pi;
      }

      PPL_DIRTY_TEMP_COEFFICIENT(product);
      PPL_DIRTY_TEMP_COEFFICIENT(gcd);
      PPL_DIRTY_TEMP_COEFFICIENT(scale_factor);

      // Creating identity rows corresponding to basic variable pj:
      // 1. add them to tableau so as to have proper size and capacity;
      tableau.s.add_zero_rows(1, Row::Flags());
      tableau.t.add_zero_rows(1, Row::Flags());
      // 2. swap the rows just added with empty ones.
      Row s_pivot(0, Row::Flags());
      Row t_pivot(0, Row::Flags());
      s_pivot.swap(tableau.s[num_rows]);
      t_pivot.swap(tableau.t[num_rows]);
      // 3. drop rows previously added at end of tableau.
      tableau.s.erase_to_end(num_rows);
      tableau.t.erase_to_end(num_rows);

      // Save current pivot denominator.
      PPL_DIRTY_TEMP_COEFFICIENT(pivot_den);
      pivot_den = tableau.denominator();
      // Let the (scaled) pivot coordinate be 1.
      s_pivot[pj] = pivot_den;

      // Swap identity row with the pivot row previously found.
      s_pivot.swap(tableau.s[pi]);
      t_pivot.swap(tableau.t[pi]);
      sign[pi] = ZERO;

      PPL_DIRTY_TEMP_COEFFICIENT(s_pivot_pj);
      s_pivot_pj = s_pivot[pj];

      // Compute columns s[*][j] :
      // s[i][j] -= s[i][pj] * s_pivot[j] / s_pivot_pj;
      for (dimension_type j = num_vars; j-- > 0; ) {
        if (j == pj)
          continue;
        const Coefficient& s_pivot_j = s_pivot[j];
        // Do nothing if the j-th pivot element is zero.
        if (s_pivot_j == 0)
          continue;
        for (dimension_type i = num_rows; i-- > 0; ) {
          Row& s_i = tableau.s[i];
          product = s_pivot_j * s_i[pj];
          if (product % s_pivot_pj != 0) {
            // Must scale matrix to stay in integer case.
            gcd_assign(gcd, product, s_pivot_pj);
            exact_div_assign(scale_factor, s_pivot_pj, gcd);
            tableau.scale(scale_factor);
            product *= scale_factor;
          }
          PPL_ASSERT(product % s_pivot_pj == 0);
          exact_div_assign(product, product, s_pivot_pj);
          s_i[j] -= product;
        }
      }

      // Compute columns t[*][j] :
      // t[i][j] -= s[i][pj] * t_pivot[j] / s_pivot_pj;
      for (dimension_type j = num_params; j-- > 0; ) {
        const Coefficient& t_pivot_j = t_pivot[j];
        // Do nothing if the j-th pivot element is zero.
        if (t_pivot_j == 0)
          continue;
        for (dimension_type i = num_rows; i-- > 0; ) {
          Row& s_i = tableau.s[i];
          product = t_pivot_j * s_i[pj];
          if (product % s_pivot_pj != 0) {
            // Must scale matrix to stay in integer case.
            gcd_assign(gcd, product, s_pivot_pj);
            exact_div_assign(scale_factor, s_pivot_pj, gcd);
            tableau.scale(scale_factor);
            product *= scale_factor;
          }
          PPL_ASSERT(product % s_pivot_pj == 0);
          exact_div_assign(product, product, s_pivot_pj);
          tableau.t[i][j] -= product;

          // Update row sign.
          Row_Sign& sign_i = sign[i];
          switch (sign_i) {
          case ZERO:
            if (product > 0)
              sign_i = NEGATIVE;
            else if (product < 0)
              sign_i = POSITIVE;
            break;
          case POSITIVE:
            if (product > 0)
              sign_i = MIXED;
            break;
          case NEGATIVE:
            if (product < 0)
              sign_i = MIXED;
            break;
          default:
            break;
          }
        }
      }

      // Compute column s[*][pj] : s[i][pj] /= s_pivot_pj;
      // Update column only if pivot coordinate != 1.
      if (s_pivot_pj != pivot_den) {
        for (dimension_type i = num_rows; i-- > 0; ) {
          Row& s_i = tableau.s[i];
          Coefficient& s_i_pj = s_i[pj];
          product = s_i_pj * pivot_den;
          if (product % s_pivot_pj != 0) {
            // As above, perform matrix scaling.
            gcd_assign(gcd, product, s_pivot_pj);
            exact_div_assign(scale_factor, s_pivot_pj, gcd);
            tableau.scale(scale_factor);
            product *= scale_factor;
          }
          PPL_ASSERT(product % s_pivot_pj == 0);
          exact_div_assign(s_i_pj, product, s_pivot_pj);
        }
      }

      // Pivoting process ended: jump to next iteration.
      solution_valid = false;
      continue;
    } // if (first_negative != not_a_dim)


    PPL_ASSERT(first_negative == not_a_dim);
    // If no negative parameter row was found,
    // but a mixed parameter row was found ...
    if (first_mixed != not_a_dim) {
      // Look for a constraint (i_neg):
      //  - having mixed parameter sign;
      //  - having no positive variable coefficient;
      //  - minimizing the score (sum of parameter coefficients).
      dimension_type i_neg = not_a_dim;
      PPL_DIRTY_TEMP_COEFFICIENT(best_score);
      PPL_DIRTY_TEMP_COEFFICIENT(score);
      for (dimension_type i = first_mixed; i < num_rows; ++i) {
        // Mixed parameter sign.
        if (sign[i] != MIXED)
          continue;
        // No positive variable coefficient.
        bool has_positive = false;
        const Row& s_i = tableau.s[i];
        for (dimension_type j = 0; j < num_vars; ++j)
          if (s_i[j] > 0) {
            has_positive = true;
            break;
          }
        if (has_positive)
          continue;
        // Minimize parameter coefficient score,
        // eliminating implicated tautologies (if any).
        const Row& t_i = tableau.t[i];
        score = 0;
        for (dimension_type j = num_params; j-- > 0; )
          score += t_i[j];
        if (i_neg == not_a_dim || score < best_score) {
          i_neg = i;
          best_score = score;
        }
      }

      if (i_neg != not_a_dim) {
#ifdef NOISY_PIP
        std::cerr << "Found row (" << i_neg << ") with mixed parameter sign "
                  << "and negative variable coefficients.\n"
                  << "==> adding tautology.\n";
#endif
        Row copy(tableau.t[i_neg]);
        copy.normalize();
        context.add_row(copy);
        add_constraint(copy, parameters);
        sign[i_neg] = POSITIVE;
        // Jump to next iteration.
        continue;
      }

      PPL_ASSERT(i_neg == not_a_dim);
      // Heuristically choose "best" (mixed) pivoting row.
      dimension_type best_i = not_a_dim;
      for (dimension_type i = first_mixed; i < num_rows; ++i) {
        if (sign[i] != MIXED)
          continue;
        const Row& t_i = tableau.t[i];
        score = 0;
        for (dimension_type j = num_params; j-- > 0; )
          score += t_i[j];
        if (best_i == not_a_dim || score < best_score) {
          best_score = score;
          best_i = i;
        }
      }

      Row t_test(tableau.t[best_i]);
      t_test.normalize();
#ifdef NOISY_PIP
      {
        Linear_Expression expr = Linear_Expression(t_test[0]);
        dimension_type j = 1;
        for (Variables_Set::const_iterator p = parameters.begin(),
               p_end = parameters.end(); p != p_end; ++p, ++j)
          expr += t_test[j] * Variable(*p);
        using namespace IO_Operators;
        std::cerr << "Found mixed parameter sign row: " << best_i << ".\n"
                  << "Solution depends on sign of parameter "
                  << expr << ".\n";
      }
#endif // #ifdef NOISY_PIP

      // Create a solution node for the "true" version of current node.
      PIP_Tree_Node* t_node = new PIP_Solution_Node(*this, No_Constraints());
      // Protect it from exception safety issues via std::auto_ptr.
      std::auto_ptr<PIP_Tree_Node> wrapped_node(t_node);

      // Add parametric constraint to context.
      context.add_row(t_test);
      // Recusively solve true node wrt updated context.
      t_node = t_node->solve(problem, context, parameters, space_dim);

      // Modify *this in place to become the "false" version of current node.
      PIP_Tree_Node* f_node = this;
      // Swap aside constraints and artificial parameters
      // (these will be later restored if needed).
      Constraint_System cs;
      Artificial_Parameter_Sequence aps;
      cs.swap(f_node->constraints_);
      aps.swap(f_node->artificial_parameters);
      // Compute the complement of the constraint used for the "true" node.
      Row& f_test = context[context.num_rows()-1];
      complement_assign(f_test, t_test, 1);

      // Recusively solve false node wrt updated context.
      f_node = f_node->solve(problem, context, parameters, space_dim);

      // Case analysis on recursive resolution calls outcome.
      if (t_node == 0) {
        if (f_node == 0) {
          // Both t_node and f_node unfeasible.
          return 0;
        }
        else {
          // t_node unfeasible, f_node feasible:
          // restore cs and aps into f_node (i.e., this).
          PPL_ASSERT(f_node == this);
          f_node->constraints_.swap(cs);
          f_node->artificial_parameters.swap(aps);
          // Add f_test to constraints.
          f_node->add_constraint(f_test, parameters);
          return f_node;
        }
      }
      else if (f_node == 0) {
        // t_node feasible, f_node unfeasible:
        // restore cs and aps into t_node.
        t_node->constraints_.swap(cs);
        t_node->artificial_parameters.swap(aps);
        // Add t_test to t_nodes's constraints.
        t_node->add_constraint(t_test, parameters);
        // It is now safe to release previously wrapped t_node pointer
        // and return it to caller.
        return wrapped_node.release();
      }

      // Here both t_node and f_node are feasible:
      // create a new decision node.
      PIP_Tree_Node* parent = new PIP_Decision_Node(f_node, t_node);
      // Protect 'parent' from exception safety issues
      // (previously wrapped t_node is now safe).
      wrapped_node.release();
      wrapped_node = std::auto_ptr<PIP_Tree_Node>(parent);

      // Add t_test to the constraints of the new decision node.
      parent->add_constraint(t_test, parameters);

      if (!cs.empty()) {
        // If node to be solved had tautologies,
        // store them in a new decision node.
        // NOTE: this is exception safe.
        parent = new PIP_Decision_Node(0, parent);
        parent->constraints_.swap(cs);
      }
      parent->artificial_parameters.swap(aps);
      // It is now safe to release previously wrapped decision node.
      wrapped_node.release();
      return parent;
    } // if (first_mixed != not_a_dim)


    PPL_ASSERT(first_negative == not_a_dim);
    PPL_ASSERT(first_mixed == not_a_dim);
    // Here all parameters are positive: we have found a continuous
    // solution. If the solution happens to be integer, then it is the
    // solution of the  integer problem. Otherwise, we may need to generate
    // a new cut to try and get back into the integer case.
#ifdef NOISY_PIP
    std::cout << "All parameters are positive.\n";
#endif
    tableau.normalize();

    // Look for any row having non integer parameter coefficients.
    const Coefficient& den = tableau.denominator();
    for (dimension_type k = 0; k < num_vars; ++k) {
      if (basis[k])
        // Basic variable = 0, hence integer.
        continue;
      const dimension_type i = mapping[k];
      const Row& t_i = tableau.t[i];
      for (dimension_type j = num_params; j-- > 0; ) {
        if (t_i[j] % den != 0)
          goto non_integer;
      }
    }
    // The goto was not taken, the solution is integer.
#ifdef NOISY_PIP
    std::cout << "Solution found for problem in current node.\n";
#endif
    return this;

  non_integer:
    // The solution is non-integer: generate a cut.
    PPL_DIRTY_TEMP_COEFFICIENT(mod);
    dimension_type best_i = not_a_dim;
    dimension_type best_pcount = not_a_dim;

    const PIP_Problem::Control_Parameter_Value cutting_strategy
      = problem.control_parameters[PIP_Problem::CUTTING_STRATEGY];

    if (cutting_strategy == PIP_Problem::CUTTING_STRATEGY_FIRST) {
      // Find the first row with simplest parametric part.
      for (dimension_type k = 0; k < num_vars; ++k) {
        if (basis[k])
          continue;
        const dimension_type i = mapping[k];
        const Row& t_i = tableau.t[i];
        // Count the number of non-integer parameter coefficients.
        dimension_type pcount = 0;
        for (dimension_type j = num_params; j-- > 0; ) {
          mod_assign(mod, t_i[j], den);
          if (mod != 0)
            ++pcount;
        }
        if (pcount > 0 && (best_i == not_a_dim || pcount < best_pcount)) {
          best_pcount = pcount;
          best_i = i;
        }
      }
      // Generate cut using 'best_i'.
      generate_cut(best_i, parameters, context, space_dim);
    }
    else {
      assert(cutting_strategy == PIP_Problem::CUTTING_STRATEGY_DEEPEST
             || cutting_strategy == PIP_Problem::CUTTING_STRATEGY_ALL);
      // Find the row with simplest parametric part
      // which will generate the "deepest" cut.
      PPL_DIRTY_TEMP_COEFFICIENT(best_score);
      best_score = 0;
      PPL_DIRTY_TEMP_COEFFICIENT(score);
      PPL_DIRTY_TEMP_COEFFICIENT(s_score);
      std::vector<dimension_type> all_best_is;

      for (dimension_type k = 0; k < num_vars; ++k) {
        if (basis[k])
          continue;
        const dimension_type i = mapping[k];
        // Compute score and pcount.
        score = 0;
        dimension_type pcount = 0;
        const Row& t_i = tableau.t[i];
        for (dimension_type j = num_params; j-- > 0; ) {
          mod_assign(mod, t_i[j], den);
          if (mod != 0) {
            score += den;
            score -= mod;
            ++pcount;
          }
        }
        // Compute s_score.
        s_score = 0;
        const Row& s_i = tableau.s[i];
        for (dimension_type j = num_vars; j-- > 0; ) {
          mod_assign(mod, s_i[j], den);
          s_score += den;
          s_score -= mod;
        }
        // Combine 'score' and 's_score'.
        score *= s_score;
        /*
          Select row i if it is non integer AND
            - no row has been chosen yet; OR
            - it has fewer non-integer parameter coefficients; OR
            - it has the same number of non-integer parameter coefficients,
              but its score is greater.
        */
        if (pcount != 0
            && (best_i == not_a_dim
                || pcount < best_pcount
                || (pcount == best_pcount && score > best_score))) {
          if (pcount < best_pcount)
            all_best_is.clear();
          best_i = i;
          best_pcount = pcount;
          best_score = score;
        }
        if (pcount > 0)
          all_best_is.push_back(i);
      }
      if (cutting_strategy == PIP_Problem::CUTTING_STRATEGY_DEEPEST)
        generate_cut(best_i, parameters, context, space_dim);
      else {
        PPL_ASSERT(cutting_strategy == PIP_Problem::CUTTING_STRATEGY_ALL);
        for (dimension_type k = all_best_is.size(); k-- > 0; )
          generate_cut(all_best_is[k], parameters, context, space_dim);
      }
    } // End of processing for non-integer solutions.

  } // Main loop of the simplex algorithm

  // This point should be unreachable.
  throw std::runtime_error("PPL internal error");
}

void
PIP_Solution_Node::generate_cut(const dimension_type index,
                                Variables_Set& parameters,
                                Matrix& context,
                                dimension_type& space_dimension) {
  const dimension_type num_rows = tableau.t.num_rows();
  PPL_ASSERT(index < num_rows);
  const dimension_type num_vars = tableau.s.num_columns();
  const dimension_type num_params = tableau.t.num_columns();
  PPL_ASSERT(num_params == 1 + parameters.size());
  const Coefficient& den = tableau.denominator();

  PPL_DIRTY_TEMP_COEFFICIENT(mod);
  PPL_DIRTY_TEMP_COEFFICIENT(coeff);

#ifdef NOISY_PIP
  std::cout << "Row " << index << " contains non-integer coefficients. "
            << "Cut generation required."
            << std::endl;
#endif // #ifdef NOISY_PIP

  // Test if cut to be generated must be parametric or not.
  bool generate_parametric_cut = false;
  {
    // Limiting the scope of reference row_t (may be later invalidated).
    const Row& row_t = tableau.t[index];
    for (dimension_type j = 1; j < num_params; ++j)
      if (row_t[j] % den != 0) {
        generate_parametric_cut = true;
        break;
      }
  }

  // Column index of already existing Artificial_Parameter.
  dimension_type ap_column = not_a_dimension();
  bool reuse_ap = false;

  if (generate_parametric_cut) {
    // Fractional parameter coefficient found: generate parametric cut.
    Linear_Expression expr;

    // Limiting the scope of reference row_t (may be later invalidated).
    {
      const Row& row_t = tableau.t[index];
      mod_assign(mod, row_t[0], den);
      if (mod != 0) {
        // Optimizing computation: expr += (den - mod);
        expr += den;
        expr -= mod;
      }
      // NOTE: iterating downwards on parameters to avoid reallocations.
      Variables_Set::const_reverse_iterator p_j = parameters.rbegin();
      // NOTE: index j spans [1..num_params-1] downwards.
      for (dimension_type j = num_params; j-- > 1; ) {
        mod_assign(mod, row_t[j], den);
        if (mod != 0) {
          // Optimizing computation: expr += (den - mod) * Variable(*p_j);
          coeff = den - mod;
          add_mul_assign(expr, coeff, Variable(*p_j));
        }
        // Mode to previous parameter.
        ++p_j;
      }
    }
    // Generate new artificial parameter.
    Artificial_Parameter ap(expr, den);

    // Search if the Artificial_Parameter has already been generated.
    ap_column = space_dimension;
    const PIP_Tree_Node* node = this;
    do {
      for (dimension_type j = node->artificial_parameters.size(); j-- > 0; ) {
        --ap_column;
        if (node->artificial_parameters[j] == ap) {
          reuse_ap = true;
          break;
        }
      }
      node = node->parent();
    } while (!reuse_ap && node != 0);

    if (reuse_ap) {
      // We can re-use an existing Artificial_Parameter.
#ifdef NOISY_PIP
      using namespace IO_Operators;
      std::cout << "Re-using parameter " << Variable(ap_column)
                << " = (" << expr << ")/" << den
                << std::endl;
#endif // #ifdef NOISY_PIP
      ap_column = ap_column - num_vars + 1;
    }
    else {
      // Here reuse_ap == false: the Artificial_Parameter does not exist yet.
      // Beware: possible reallocation invalidates row references.
      tableau.t.add_zero_columns(1);
      context.add_zero_columns(1);
      artificial_parameters.push_back(ap);
      parameters.insert(space_dimension);
#ifdef NOISY_PIP
      using namespace IO_Operators;
      std::cout << "Creating new parameter "
                << Variable(space_dimension)
                << " = (" << expr << ")/" << den
                << std::endl;
#endif // #ifdef NOISY_PIP
      ++space_dimension;
      ap_column = num_params;

      // Update current context with constraints on the new parameter.
      const dimension_type ctx_num_rows = context.num_rows();
      context.add_zero_rows(2, Row::Flags());
      Row& ctx1 = context[ctx_num_rows];
      Row& ctx2 = context[ctx_num_rows+1];
      // Recompute row reference after possible reallocation.
      const Row& row_t = tableau.t[index];
      for (dimension_type j = 0; j < num_params; ++j) {
        mod_assign(mod, row_t[j], den);
        if (mod != 0) {
          ctx1[j] = den;
          ctx1[j] -= mod;
          neg_assign(ctx2[j], ctx1[j]);
        }
      }
      neg_assign(ctx1[num_params], den);
      ctx2[num_params] = den;
      // ctx2[0] += den-1;
      ctx2[0] += den;
      --ctx2[0];
#ifdef NOISY_PIP
      {
        using namespace IO_Operators;
        Variables_Set::const_iterator p = parameters.begin();
        Linear_Expression expr1(ctx1[0]);
        Linear_Expression expr2(ctx2[0]);
        for (dimension_type j = 1; j <= num_params; ++j, ++p) {
          expr1 += ctx1[j] * Variable(*p);
          expr2 += ctx2[j] * Variable(*p);
        }
        std::cout << "Inserting into context: "
                  << Constraint(expr1 >= 0) << " ; "
                  << Constraint(expr2 >= 0) << std::endl;
      }
#endif // #ifdef NOISY_PIP
    }
  }

  // Generate new cut.
  tableau.s.add_zero_rows(1, Row::Flags());
  tableau.t.add_zero_rows(1, Row::Flags());
  Row& cut_s = tableau.s[num_rows];
  Row& cut_t = tableau.t[num_rows];
  // Recompute references after possible reallocation.
  const Row& row_s = tableau.s[index];
  const Row& row_t = tableau.t[index];
  for (dimension_type j = 0; j < num_vars; ++j) {
    mod_assign(cut_s[j], row_s[j], den);
  }
  for (dimension_type j = 0; j < num_params; ++j) {
    mod_assign(mod, row_t[j], den);
    if (mod != 0) {
      cut_t[j] = mod;
      cut_t[j] -= den;
    }
  }
  if (ap_column != not_a_dimension())
    // If we re-use an existing Artificial_Parameter
    cut_t[ap_column] = den;

#ifdef NOISY_PIP
  {
    using namespace IO_Operators;
    Linear_Expression expr;
    dimension_type ti = 1;
    dimension_type si = 0;
    for (dimension_type j = 0; j < space_dimension; ++j) {
      if (parameters.count(j) == 1)
        expr += cut_t[ti++] * Variable(j);
      else
        expr += cut_s[si++] * Variable(j);
    }
    std::cout << "Adding cut: "
              << Constraint(expr + cut_t[0] >= 0)
              << std::endl;
  }
#endif
  var_row.push_back(num_rows + num_vars);
  basis.push_back(false);
  mapping.push_back(num_rows);
  sign.push_back(NEGATIVE);
}


memory_size_type
PIP_Tree_Node::Artificial_Parameter::external_memory_in_bytes() const {
  return Linear_Expression::external_memory_in_bytes()
    + Parma_Polyhedra_Library::external_memory_in_bytes(denom);
}

memory_size_type
PIP_Tree_Node::Artificial_Parameter::total_memory_in_bytes() const {
  return sizeof(*this) + external_memory_in_bytes();
}

memory_size_type
PIP_Tree_Node::external_memory_in_bytes() const {
  memory_size_type n = constraints_.external_memory_in_bytes();
  // Adding the external memory for `artificial_parameters'.
  n += artificial_parameters.capacity() * sizeof(Artificial_Parameter);
  for (Artificial_Parameter_Sequence::const_iterator
         ap = art_parameter_begin(),
	 ap_end = art_parameter_end(); ap != ap_end; ++ap)
    n += (ap->external_memory_in_bytes());

  return n;
}

memory_size_type
PIP_Decision_Node::external_memory_in_bytes() const {
  memory_size_type n = PIP_Tree_Node::external_memory_in_bytes();
  if (true_child)
    n += true_child->total_memory_in_bytes();
  if (false_child)
    n += false_child->total_memory_in_bytes();
  return n;
}

memory_size_type
PIP_Decision_Node::total_memory_in_bytes() const {
  return sizeof(*this) + external_memory_in_bytes();
}

memory_size_type
PIP_Solution_Node::Tableau::external_memory_in_bytes() const {
  return Parma_Polyhedra_Library::external_memory_in_bytes(denom)
    + s.external_memory_in_bytes()
    + t.external_memory_in_bytes();
}

memory_size_type
PIP_Solution_Node::Tableau::total_memory_in_bytes() const {
  return sizeof(*this) + external_memory_in_bytes();
}

memory_size_type
PIP_Solution_Node::external_memory_in_bytes() const {
  memory_size_type n = PIP_Tree_Node::external_memory_in_bytes();
  n += tableau.external_memory_in_bytes();
  // FIXME: size of std::vector<bool> ?
  n += basis.capacity() * sizeof(bool);
  n += sizeof(dimension_type)
    * (mapping.capacity() + var_row.capacity() + var_column.capacity());
  n += sign.capacity() * sizeof(Row_Sign);
  // FIXME: Adding the external memory for `solution'.
  n += solution.capacity() * sizeof(Linear_Expression);
  for (std::vector<Linear_Expression>::const_iterator
         i = solution.begin(), i_end = solution.end(); i != i_end; ++i)
    n += (i->external_memory_in_bytes());

  return n;
}

memory_size_type
PIP_Solution_Node::total_memory_in_bytes() const {
  return sizeof(*this) + external_memory_in_bytes();
}

void
PIP_Tree_Node::indent_and_print(std::ostream& s,
                                const unsigned indent,
                                const char* str) {
  s << std::setw(2*indent) << "" << str;
}

void
PIP_Tree_Node::print_tree(std::ostream& s,
                          const unsigned indent,
                          const dimension_type space_dim,
                          dimension_type first_art_dim,
                          const Variables_Set& params) const {
  used(space_dim);
  used(params);

  using namespace IO_Operators;

  // Print artificial parameters.
  for (Artificial_Parameter_Sequence::const_iterator
         api = art_parameter_begin(),
         api_end = art_parameter_end(); api != api_end; ++api) {
    indent_and_print(s, indent, "Parameter ");
    s << Variable(first_art_dim) << " = " << *api << "\n";
    ++first_art_dim;
  }

  // Print constraints, if any.
  if (!constraints_.empty()) {
    indent_and_print(s, indent, "if ");

    Constraint_System::const_iterator
      ci = constraints_.begin(),
      ci_end = constraints_.end();
    PPL_ASSERT(ci != ci_end);
    s << *ci;
    for (++ci; ci != ci_end; ++ci)
      s << " and " << *ci;

    s << " then\n";
  }
}

void
PIP_Decision_Node::print_tree(std::ostream& s, unsigned indent,
                              const dimension_type space_dim,
                              const dimension_type first_art_dim,
                              const Variables_Set& params) const {
  // First print info common to decision and solution nodes.
  PIP_Tree_Node::print_tree(s, indent, space_dim, first_art_dim, params);

  // Then print info specific of decision nodes.
  dimension_type child_first_art_dim = first_art_dim + art_parameter_count();

  if (true_child)
    true_child->print_tree(s, indent+1, space_dim,
                           child_first_art_dim, params);
  else
    indent_and_print(s, indent+1, "_|_\n");

  indent_and_print(s, indent, "else\n");

  if (false_child)
    false_child->print_tree(s, indent+1, space_dim,
                            child_first_art_dim, params);
  else
    indent_and_print(s, indent+1, "_|_\n");
}

void
PIP_Solution_Node::print_tree(std::ostream& s, unsigned indent,
                              const dimension_type space_dim,
                              const dimension_type first_art_dim,
                              const Variables_Set& params) const {
  // First print info common to decision and solution nodes.
  PIP_Tree_Node::print_tree(s, indent, space_dim, first_art_dim, params);

  // Then print info specific of solution nodes.
  const bool no_constraints = constraints_.empty();
  bool printed_first_variable = false;
  indent_and_print(s, indent + (no_constraints ? 0 : 1), "{");
  for (dimension_type i = 0; i < space_dim; ++i) {
    if (params.count(i) != 0)
      continue;
    if (printed_first_variable)
      s << " ; ";
    else
      printed_first_variable = true;
    using namespace IO_Operators;
    s << parametric_values(Variable(i), params);
  }
  s << "}\n";

  if (!no_constraints) {
    indent_and_print(s, indent, "else\n");
    indent_and_print(s, indent+1, "_|_\n");
  }
}

} // namespace Parma_Polyhedra_Library
