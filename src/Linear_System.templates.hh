/* Linear_System class implementation: non-inline template functions.
   Copyright (C) 2001-2010 Roberto Bagnara <bagnara@cs.unipr.it>
   Copyright (C) 2010-2011 BUGSENG srl (http://bugseng.com)

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

#ifndef PPL_Linear_System_templates_hh
#define PPL_Linear_System_templates_hh 1

// TODO: Remove this.
// It was added to please KDevelop4.
#include "Linear_System.defs.hh"

#include "Bit_Matrix.defs.hh"
#include "Scalar_Products.defs.hh"
#include <algorithm>
#include <iostream>
#include <string>
#include <deque>

#include "swapping_sort.icc"

namespace Parma_Polyhedra_Library {

template <typename Row>
dimension_type
Linear_System<Row>::num_lines_or_equalities() const {
  PPL_ASSERT(num_pending_rows() == 0);
  const Linear_System& x = *this;
  dimension_type n = 0;
  for (dimension_type i = num_rows(); i-- > 0; )
    if (x[i].is_line_or_equality())
      ++n;
  return n;
}

template <typename Row>
void
Linear_System<Row>::merge_rows_assign(const Linear_System& y) {
  PPL_ASSERT(num_columns() >= y.num_columns());
  // Both systems have to be sorted and have no pending rows.
  PPL_ASSERT(check_sorted() && y.check_sorted());
  PPL_ASSERT(num_pending_rows() == 0 && y.num_pending_rows() == 0);

  Linear_System& x = *this;

  // A temporary vector...
  Swapping_Vector<Row> tmp;
  // ... with enough capacity not to require any reallocations.
  tmp.reserve(compute_capacity(x.rows.size() + y.rows.size(),
                               tmp.max_num_rows()));

  dimension_type xi = 0;
  dimension_type x_num_rows = x.num_rows();
  dimension_type yi = 0;
  dimension_type y_num_rows = y.num_rows();

  while (xi < x_num_rows && yi < y_num_rows) {
    const int comp = compare(x[xi], y[yi]);
    if (comp <= 0) {
      // Elements that can be taken from `x' are actually _stolen_ from `x'
      tmp.resize(tmp.size() + 1);
      std::swap(tmp.back(), x.rows[xi++]);
      if (comp == 0)
	// A duplicate element.
	++yi;
    }
    else {
      // (comp > 0)
      tmp.resize(tmp.size() + 1);
      Row copy(y[yi++], num_columns(), num_columns());
      std::swap(tmp.back(), copy);
    }
  }
  // Insert what is left.
  if (xi < x_num_rows)
    while (xi < x_num_rows) {
      tmp.resize(tmp.size() + 1);
      std::swap(tmp.back(), x.rows[xi++]);
    }
  else
    while (yi < y_num_rows) {
      tmp.resize(tmp.size() + 1);
      Row copy(y[yi++], num_columns(), num_columns());
      std::swap(tmp.back(), copy);
    }

  // We get the result matrix and let the old one be destroyed.
  std::swap(tmp, rows);
  // There are no pending rows.
  unset_pending_rows();
  PPL_ASSERT(check_sorted());
}

template <typename Row>
void
Linear_System<Row>::set_rows_topology() {
  Linear_System& x = *this;
  if (is_necessarily_closed())
    for (dimension_type i = num_rows(); i-- > 0; )
      x.rows[i].set_necessarily_closed();
  else
    for (dimension_type i = num_rows(); i-- > 0; )
      x.rows[i].set_not_necessarily_closed();
}

template <typename Row>
void
Linear_System<Row>::ascii_dump(std::ostream& s) const {
  // Prints the topology, the number of rows, the number of columns
  // and the sorted flag.  The specialized methods provided by
  // Constraint_System and Generator_System take care of properly
  // printing the contents of the system.
  const Linear_System& x = *this;
  dimension_type x_num_rows = x.num_rows();
  dimension_type x_num_columns = x.num_columns();
  s << "topology " << (is_necessarily_closed()
		       ? "NECESSARILY_CLOSED"
		       : "NOT_NECESSARILY_CLOSED")
    << "\n"
    << x_num_rows << " x " << x_num_columns
    << (x.sorted ? "(sorted)" : "(not_sorted)")
    << "\n"
    << "index_first_pending " << x.first_pending_row()
    << "\n";
  for (dimension_type i = 0; i < rows.size(); ++i)
    rows[i].ascii_dump(s);
}

PPL_OUTPUT_TEMPLATE_DEFINITIONS_ASCII_ONLY(Row, Linear_System<Row>)

template <typename Row>
bool
Linear_System<Row>::ascii_load(std::istream& s) {
  std::string str;
  if (!(s >> str) || str != "topology")
    return false;
  if (!(s >> str))
    return false;
  if (str == "NECESSARILY_CLOSED")
    set_necessarily_closed();
  else {
    if (str != "NOT_NECESSARILY_CLOSED")
      return false;
    set_not_necessarily_closed();
  }

  dimension_type nrows;
  dimension_type ncols;
  if (!(s >> nrows))
    return false;
  if (!(s >> str) || str != "x")
    return false;
  if (!(s >> ncols))
    return false;
  clear();
  num_columns_ = ncols;

  if (!(s >> str) || (str != "(sorted)" && str != "(not_sorted)"))
    return false;
  bool sortedness = (str == "(sorted)");
  dimension_type index;
  if (!(s >> str) || str != "index_first_pending")
    return false;
  if (!(s >> index))
    return false;

  Row row;
  for (dimension_type i = 0; i < nrows; ++i) {
    if (!row.ascii_load(s))
      return false;
    add_recycled_row(row);
  }
  set_index_first_pending_row(index);
  set_sorted(sortedness);

  // Check invariants.
  PPL_ASSERT(OK(true));
  return true;
}

template <typename Row>
void
Linear_System<Row>::insert(const Row& r) {
  Row tmp = r;
  insert_recycled(tmp);
}

template <typename Row>
void
Linear_System<Row>::insert_recycled(Row& r) {
  // TODO: A Grid_Generator_System may contain non-normalized lines that
  // represent parameters, so this check is disabled. Consider re-enabling it
  // when it's possibile.
  /*
    // The added row must be strongly normalized and have the same
    // topology of the system.
    PPL_ASSERT(r.check_strong_normalized());
  */
  PPL_ASSERT(topology() == r.topology());
  // This method is only used when the system has no pending rows.
  PPL_ASSERT(num_pending_rows() == 0);

  const dimension_type old_num_rows = rows.size();
  const dimension_type old_num_columns = num_columns_;
  const dimension_type r_size = r.size();

  // Resize the system, if necessary.
  if (r_size > old_num_columns) {
    add_zero_columns(r_size - old_num_columns);
    if (!is_necessarily_closed() && old_num_rows != 0)
      // Move the epsilon coefficients to the last column
      // (note: sorting is preserved).
      swap_columns(old_num_columns - 1, r_size - 1);
    add_recycled_row(r);
  }
  else if (r_size < old_num_columns) {
    // Resize the row.
    r.resize(old_num_columns);
    // If needed, move the epsilon coefficient to the last position.
    if (!is_necessarily_closed())
      std::swap(r[r_size - 1], r[old_num_columns - 1]);
    add_recycled_row(r);
  }
  else
    // Here r_size == old_num_columns.
    add_recycled_row(r);

  // The added row was not a pending row.
  PPL_ASSERT(num_pending_rows() == 0);
  // Do not check for strong normalization,
  // because no modification of rows has occurred.
  PPL_ASSERT(OK(false));
}

template <typename Row>
void
Linear_System<Row>::insert_pending(const Row& r) {
  Row tmp = r;
  insert_pending_recycled(tmp);
}

template <typename Row>
void
Linear_System<Row>::increase_space_dimension(dimension_type n) {
  add_zero_columns(n - num_columns());
  if (!is_necessarily_closed() && num_rows() != 0)
    // Move the epsilon coefficients to the last column
    // (note: sorting is preserved).
    swap_columns(num_columns() - 1, n - 1);
}

template <typename Row>
void
Linear_System<Row>::insert_pending_recycled(Row& r) {
  // The added row must be strongly normalized and have the same
  // topology of the system.
  PPL_ASSERT(r.check_strong_normalized());
  PPL_ASSERT(topology() == r.topology());

  const dimension_type old_num_rows = rows.size();
  const dimension_type old_num_columns = num_columns();
  const dimension_type r_size = r.size();

  // Resize the system, if necessary.
  if (r_size > old_num_columns) {
    increase_space_dimension(r_size);
    add_recycled_pending_row(r);
  }
  else if (r_size < old_num_columns)
    if (is_necessarily_closed() || old_num_rows == 0) {
      r.resize(old_num_columns);
      add_recycled_pending_row(r);
    } else {
      // Resize the row (and move the epsilon coefficient to its last
      // position).
      r.resize(old_num_columns);
      std::swap(r[r_size - 1], r[old_num_columns - 1]);
      add_recycled_pending_row(r);
    }
  else
    // Here r_size == old_num_columns.
    add_recycled_pending_row(r);

  // The added row was a pending row.
  PPL_ASSERT(num_pending_rows() > 0);
  // Do not check for strong normalization,
  // because no modification of rows has occurred.
  PPL_ASSERT(OK(false));
}

template <typename Row>
void
Linear_System<Row>::insert_pending(const Linear_System& y) {
  Linear_System tmp(y, With_Pending());
  insert_pending_recycled(tmp);
}

template <typename Row>
void
Linear_System<Row>::insert_pending_recycled(Linear_System& y) {
  Linear_System& x = *this;
  PPL_ASSERT(x.num_columns() == y.num_columns());

  // Steal the rows of `y'.
  // This loop must use an increasing index (instead of a decreasing one) to
  // preserve the row ordering.
  for (dimension_type i = 0; i < y.num_rows(); i++)
    insert_pending_recycled(y.rows[i]);

  y.clear();

  // Do not check for strong normalization,
  // because no modification of rows has occurred.
  PPL_ASSERT(OK(false));
}

template <typename Row>
void
Linear_System<Row>::insert(const Linear_System& y) {
  Linear_System tmp(y, With_Pending());
  insert_recycled(tmp);
}

template <typename Row>
void
Linear_System<Row>::insert_recycled(Linear_System& y) {
  PPL_ASSERT(num_pending_rows() == 0);

  // Adding no rows is a no-op.
  if (y.has_no_rows())
    return;

  // Check if sortedness is preserved.
  if (is_sorted()) {
    if (!y.is_sorted() || y.num_pending_rows() > 0)
      set_sorted(false);
    else {
      // `y' is sorted and has no pending rows.
      const dimension_type n_rows = num_rows();
      if (n_rows > 0)
        set_sorted(compare(rows[n_rows-1], y[0]) <= 0);
    }
  }

  // Add the rows of `y' as if they were pending.
  insert_pending_recycled(y);

  // TODO: May y have pending rows? Should they remain pending?

  // There are no pending_rows.
  unset_pending_rows();

  // Do not check for strong normalization,
  // because no modification of rows has occurred.
  PPL_ASSERT(OK(false));
}

template <typename Row>
void
Linear_System<Row>::sort_rows() {
  const dimension_type num_pending = num_pending_rows();
  // We sort the non-pending rows only.
  sort_rows(0, first_pending_row());
  set_index_first_pending_row(num_rows() - num_pending);
  sorted = true;
  // Do not check for strong normalization,
  // because no modification of rows has occurred.
  PPL_ASSERT(OK(false));
}

template <typename Row>
void
Linear_System<Row>::sort_rows(const dimension_type first_row,
			      const dimension_type last_row) {
  PPL_ASSERT(first_row <= last_row && last_row <= num_rows());
  // We cannot mix pending and non-pending rows.
  PPL_ASSERT(first_row >= first_pending_row() || last_row <= first_pending_row());

  // First sort without removing duplicates.
  typename Swapping_Vector<Row>::iterator first = rows.begin() + first_row;
  typename Swapping_Vector<Row>::iterator last = rows.begin() + last_row;
  swapping_sort(first, last, Row_Less_Than());
  // Second, move duplicates to the end.
  typename Swapping_Vector<Row>::iterator new_last
    = swapping_unique(first, last);
  // Finally, remove duplicates.
  rows.erase(new_last, last);
  // NOTE: we cannot check all invariants of the system here,
  // because the caller still has to update `index_first_pending'.
}

template <typename Row>
void
Linear_System<Row>::add_row(const Row& r) {
  Row tmp = r;
  add_recycled_row(tmp);
}

template <typename Row>
void
Linear_System<Row>::add_recycled_row(Row& r) {
  // This method is only used when the system has no pending rows.
  PPL_ASSERT(num_pending_rows() == 0);

  const bool was_sorted = is_sorted();

  add_recycled_pending_row(r);

  if (was_sorted) {
    const dimension_type nrows = num_rows();
    // The added row may have caused the system to be not sorted anymore.
    if (nrows > 1) {
      // If the system is not empty and the inserted row is the
      // greatest one, the system is set to be sorted.
      // If it is not the greatest one then the system is no longer sorted.
      set_sorted(compare(rows[nrows-2], rows[nrows-1]) <= 0);
    }
    else
      // A system having only one row is sorted.
      set_sorted(true);
  }

  //  We update `index_first_pending', because it must be equal to
  // `rows.size()'.
  unset_pending_rows();
  
  // The added row was not a pending row.
  PPL_ASSERT(num_pending_rows() == 0);
}

template <typename Row>
void
Linear_System<Row>::add_recycled_pending_row(Row& r) {
  // TODO: A Grid_Generator_System may contain non-normalized lines that
  // represent parameters, so this check is disabled. Consider re-enabling it
  // when it's possibile.
  /*
    // The added row must be strongly normalized and have the same
    // number of elements as the existing rows of the system.
    PPL_ASSERT(r.check_strong_normalized());
  */
  PPL_ASSERT(r.topology() == topology());

  rows.resize(rows.size() + 1);
  r.resize(num_columns());
  std::swap(rows.back(), r);

  // Do not check for strong normalization, because no modification of
  // rows has occurred.
  PPL_ASSERT(OK(false));
}

template <typename Row>
void
Linear_System<Row>::add_pending_row(const Row& r) {
  Row tmp = r;
  add_recycled_pending_row(tmp);
}

template <typename Row>
void
Linear_System<Row>::add_pending_row(const typename Row::Flags flags) {
  Row new_row(num_columns(), num_columns(), flags);
  add_recycled_pending_row(new_row);
}

template <typename Row>
void
Linear_System<Row>::normalize() {
  const dimension_type nrows = rows.size();
  // We normalize also the pending rows.
  for (dimension_type i = nrows; i-- > 0; )
    rows[i].normalize();
  set_sorted(nrows <= 1);
}

template <typename Row>
void
Linear_System<Row>::strong_normalize() {
  const dimension_type nrows = rows.size();
  // We strongly normalize also the pending rows.
  for (dimension_type i = nrows; i-- > 0; )
    rows[i].strong_normalize();
  set_sorted(nrows <= 1);
}

template <typename Row>
void
Linear_System<Row>::sign_normalize() {
  const dimension_type nrows = rows.size();
  // We sign-normalize also the pending rows.
  for (dimension_type i = nrows; i-- > 0; )
    rows[i].sign_normalize();
  set_sorted(nrows <= 1);
}

/*! \relates Parma_Polyhedra_Library::Linear_System */
template <typename Row>
bool
operator==(const Linear_System<Row>& x, const Linear_System<Row>& y) {
  if (x.num_columns() != y.num_columns())
    return false;
  const dimension_type x_num_rows = x.num_rows();
  const dimension_type y_num_rows = y.num_rows();
  if (x_num_rows != y_num_rows)
    return false;
  if (x.first_pending_row() != y.first_pending_row())
    return false;
  // TODO: Check if the following comment is up to date.
  // Notice that calling operator==(const Swapping_Vector<Row>&,
  //                                const Swapping_Vector<Row>&)
  // would be wrong here, as equality of the type fields would
  // not be checked.
  for (dimension_type i = x_num_rows; i-- > 0; )
    if (x[i] != y[i])
      return false;
  return true;
}

template <typename Row>
void
Linear_System<Row>::sort_and_remove_with_sat(Bit_Matrix& sat) {
  Linear_System& sys = *this;
  // We can only sort the non-pending part of the system.
  PPL_ASSERT(sys.first_pending_row() == sat.num_rows());
  if (sys.first_pending_row() <= 1) {
    sys.set_sorted(true);
    return;
  }

  // First, sort `sys' (keeping `sat' consistent) without removing duplicates.
  Linear_System_With_Bit_Matrix_iterator<Row> first(rows.begin(), sat.rows.begin());
  Linear_System_With_Bit_Matrix_iterator<Row> last = first + sat.num_rows();
  swapping_sort(first, last, Row_Less_Than(),
                std::mem_fun_ref(&Linear_System_With_Bit_Matrix_iterator<Row>::iter_swap));
  // Second, move duplicates in `sys' to the end (keeping `sat' consistent).
  Linear_System_With_Bit_Matrix_iterator<Row> new_last
    = swapping_unique(first, last,
                      std::mem_fun_ref(&Linear_System_With_Bit_Matrix_iterator<Row>::iter_swap));

  const dimension_type num_duplicates = last - new_last;
  const dimension_type new_first_pending_row
    = sys.first_pending_row() - num_duplicates;

  if (sys.num_pending_rows() > 0) {
    // In this case, we must put the duplicates after the pending rows.
    const dimension_type n_rows = sys.num_rows() - 1;
    for (dimension_type i = 0; i < num_duplicates; ++i)
      sys.swap_rows(new_first_pending_row + i, n_rows - i);
  }
  // Erasing the duplicated rows...
  sys.remove_trailing_rows(num_duplicates);
  sys.set_index_first_pending_row(new_first_pending_row);
  // ... and the corresponding rows of the saturation matrix.
  sat.remove_trailing_rows(num_duplicates);
  PPL_ASSERT(sys.check_sorted());
  // Now the system is sorted.
  sys.set_sorted(true);
}

template <typename Row>
dimension_type
Linear_System<Row>::gauss(const dimension_type n_lines_or_equalities) {
  Linear_System& x = *this;
  // This method is only applied to a well-formed linear system
  // having no pending rows and exactly `n_lines_or_equalities'
  // lines or equalities, all of which occur before the rays or points
  // or inequalities.
  PPL_ASSERT(x.OK(true));
  PPL_ASSERT(x.num_pending_rows() == 0);
  PPL_ASSERT(n_lines_or_equalities == x.num_lines_or_equalities());
#ifndef NDEBUG
  for (dimension_type i = n_lines_or_equalities; i-- > 0; )
    PPL_ASSERT(x[i].is_line_or_equality());
#endif

  dimension_type rank = 0;
  // Will keep track of the variations on the system of equalities.
  bool changed = false;
  for (dimension_type j = x.num_columns(); j-- > 0; )
    for (dimension_type i = rank; i < n_lines_or_equalities; ++i) {
      // Search for the first row having a non-zero coefficient
      // (the pivot) in the j-th column.
      if (x[i][j] == 0)
	continue;
      // Pivot found: if needed, swap rows so that this one becomes
      // the rank-th row in the linear system.
      if (i > rank) {
	std::swap(x.rows[i], x.rows[rank]);
	// After swapping the system is no longer sorted.
	changed = true;
      }
      // Combine the row containing the pivot with all the lines or
      // equalities following it, so that all the elements on the j-th
      // column in these rows become 0.
      for (dimension_type k = i + 1; k < n_lines_or_equalities; ++k)
	if (x.rows[k][j] != 0) {
	  x.rows[k].linear_combine(x.rows[rank], j);
	  changed = true;
	}
      // Already dealt with the rank-th row.
      ++rank;
      // Consider another column index `j'.
      break;
    }
  if (changed)
    x.set_sorted(false);
  // A well-formed system is returned.
  PPL_ASSERT(x.OK(true));
  return rank;
}

template <typename Row>
void
Linear_System<Row>
::back_substitute(const dimension_type n_lines_or_equalities) {
  Linear_System& x = *this;
  // This method is only applied to a well-formed system
  // having no pending rows and exactly `n_lines_or_equalities'
  // lines or equalities, all of which occur before the first ray
  // or point or inequality.
  PPL_ASSERT(x.OK(true));
  PPL_ASSERT(x.num_columns() >= 1);
  PPL_ASSERT(x.num_pending_rows() == 0);
  PPL_ASSERT(n_lines_or_equalities <= x.num_lines_or_equalities());
#ifndef NDEBUG
  for (dimension_type i = n_lines_or_equalities; i-- > 0; )
    PPL_ASSERT(x[i].is_line_or_equality());
#endif

  const dimension_type nrows = x.num_rows();
  const dimension_type ncols = x.num_columns();
  // Trying to keep sortedness.
  bool still_sorted = x.is_sorted();
  // This deque of Booleans will be used to flag those rows that,
  // before exiting, need to be re-checked for sortedness.
  std::deque<bool> check_for_sortedness;
  if (still_sorted)
    check_for_sortedness.insert(check_for_sortedness.end(), nrows, false);

  for (dimension_type k = n_lines_or_equalities; k-- > 0; ) {
    // For each line or equality, starting from the last one,
    // looks for the last non-zero element.
    // `j' will be the index of such a element.
    Row& x_k = x.rows[k];
    dimension_type j = ncols - 1;
    while (j != 0 && x_k[j] == 0)
      --j;

    // Go through the equalities above `x_k'.
    for (dimension_type i = k; i-- > 0; ) {
      Row& x_i = x.rows[i];
      if (x_i[j] != 0) {
	// Combine linearly `x_i' with `x_k'
	// so that `x_i[j]' becomes zero.
	x_i.linear_combine(x_k, j);
	if (still_sorted) {
	  // Trying to keep sortedness: remember which rows
	  // have to be re-checked for sortedness at the end.
	  if (i > 0)
	    check_for_sortedness[i-1] = true;
	  check_for_sortedness[i] = true;
	}
      }
    }

    // Due to strong normalization during previous iterations,
    // the pivot coefficient `x_k[j]' may now be negative.
    // Since an inequality (or ray or point) cannot be multiplied
    // by a negative factor, the coefficient of the pivot must be
    // forced to be positive.
    const bool have_to_negate = (x_k[j] < 0);
    if (have_to_negate)
      for (dimension_type h = ncols; h-- > 0; )
	neg_assign(x_k[h]);
    // Note: we do not mark index `k' in `check_for_sortedness',
    // because we will later negate back the row.

    // Go through all the other rows of the system.
    for (dimension_type i = n_lines_or_equalities; i < nrows; ++i) {
      Row& x_i = x.rows[i];
      if (x_i[j] != 0) {
	// Combine linearly the `x_i' with `x_k'
	// so that `x_i[j]' becomes zero.
	x_i.linear_combine(x_k, j);
	if (still_sorted) {
	  // Trying to keep sortedness: remember which rows
	  // have to be re-checked for sortedness at the end.
	  if (i > n_lines_or_equalities)
	    check_for_sortedness[i-1] = true;
	  check_for_sortedness[i] = true;
	}
      }
    }
    if (have_to_negate)
      // Negate `x_k' to restore strong-normalization.
      for (dimension_type h = ncols; h-- > 0; )
	neg_assign(x_k[h]);
  }

  // Trying to keep sortedness.
  for (dimension_type i = 0; still_sorted && i+1 < nrows; ++i)
    if (check_for_sortedness[i])
      // Have to check sortedness of `x[i]' with respect to `x[i+1]'.
      still_sorted = (compare(x[i], x[i+1]) <= 0);
  // Set the sortedness flag.
  x.set_sorted(still_sorted);

  // A well-formed system is returned.
  PPL_ASSERT(x.OK(true));
}

template <typename Row>
void
Linear_System<Row>::simplify() {
  Linear_System& x = *this;
  // This method is only applied to a well-formed system
  // having no pending rows.
  PPL_ASSERT(x.OK(true));
  PPL_ASSERT(x.num_pending_rows() == 0);

  // Partially sort the linear system so that all lines/equalities come first.
  const dimension_type old_nrows = x.num_rows();
  dimension_type nrows = old_nrows;
  dimension_type n_lines_or_equalities = 0;
  for (dimension_type i = 0; i < nrows; ++i)
    if (x[i].is_line_or_equality()) {
      if (n_lines_or_equalities < i) {
	std::swap(x.rows[i], x.rows[n_lines_or_equalities]);
	// The system was not sorted.
	PPL_ASSERT(!x.sorted);
      }
      ++n_lines_or_equalities;
    }
  // Apply Gaussian elimination to the subsystem of lines/equalities.
  const dimension_type rank = x.gauss(n_lines_or_equalities);
  // Eliminate any redundant line/equality that has been detected.
  if (rank < n_lines_or_equalities) {
    const dimension_type
      n_rays_or_points_or_inequalities = nrows - n_lines_or_equalities;
    const dimension_type
      num_swaps = std::min(n_lines_or_equalities - rank,
			   n_rays_or_points_or_inequalities);
    for (dimension_type i = num_swaps; i-- > 0; )
      std::swap(x.rows[--nrows], x.rows[rank + i]);
    x.remove_trailing_rows(old_nrows - nrows);
    x.unset_pending_rows();
    if (n_rays_or_points_or_inequalities > num_swaps)
      x.set_sorted(false);
    n_lines_or_equalities = rank;
  }
  // Apply back-substitution to the system of rays/points/inequalities.
  x.back_substitute(n_lines_or_equalities);
  // A well-formed system is returned.
  PPL_ASSERT(x.OK(true));
}

template <typename Row>
void
Linear_System<Row>::add_universe_rows_and_columns(const dimension_type n) {
  PPL_ASSERT(n > 0);
  const bool was_sorted = is_sorted();
  const dimension_type old_n_rows = num_rows();
  const dimension_type old_n_columns = num_columns();
  add_zero_columns(n);
  rows.resize(rows.size() + n,
              Row(num_columns(), typename Row::Flags(row_topology)));
  // The old system is moved to the bottom.
  for (dimension_type i = old_n_rows; i-- > 0; )
    std::swap(rows[i], rows[i + n]);
  for (dimension_type i = n, c = old_n_columns; i-- > 0; ) {
    // The top right-hand sub-system (i.e., the system made of new
    // rows and columns) is set to the specular image of the identity
    // matrix.
    Row& r = rows[i];
    r[c++] = 1;
    r.set_is_line_or_equality();
    // Note: `r' is strongly normalized.
  }
  // If the old system was empty, the last row added is either
  // a positivity constraint or a point.
  if (old_n_columns == 0) {
    rows[n-1].set_is_ray_or_point_or_inequality();
    // Since ray, points and inequalities come after lines
    // and equalities, this case implies the system is sorted.
    set_sorted(true);
  }
  else if (was_sorted)
    set_sorted(compare(rows[n-1], rows[n]) <= 0);

  // A well-formed system has to be returned.
  PPL_ASSERT(OK(true));
}

template <typename Row>
void
Linear_System<Row>::add_zero_columns(const dimension_type n) {
  num_columns_ += n;
  for (dimension_type i = rows.size(); i-- > 0; )
    rows[i].resize(num_columns_);
}

template <typename Row>
void
Linear_System<Row>::sort_pending_and_remove_duplicates() {
  PPL_ASSERT(num_pending_rows() > 0);
  PPL_ASSERT(is_sorted());

  // The non-pending part of the system is already sorted.
  // Now sorting the pending part..
  const dimension_type first_pending = first_pending_row();
  sort_rows(first_pending, num_rows());
  // Recompute the number of rows, because we may have removed
  // some rows occurring more than once in the pending part.
  const dimension_type old_num_rows = num_rows();
  dimension_type num_rows = old_num_rows;

  dimension_type k1 = 0;
  dimension_type k2 = first_pending;
  dimension_type num_duplicates = 0;
  // In order to erase them, put at the end of the system
  // those pending rows that also occur in the non-pending part.
  while (k1 < first_pending && k2 < num_rows) {
    const int cmp = compare(rows[k1], rows[k2]);
    if (cmp == 0) {
      // We found the same row.
      ++num_duplicates;
      --num_rows;
      // By initial sortedness, we can increment index `k1'.
      ++k1;
      // Do not increment `k2'; instead, swap there the next pending row.
      if (k2 < num_rows)
        std::swap(rows[k2], rows[k2 + num_duplicates]);
    }
    else if (cmp < 0)
      // By initial sortedness, we can increment `k1'.
      ++k1;
    else {
      // Here `cmp > 0'.
      // Increment `k2' and, if we already found any duplicate,
      // swap the next pending row in position `k2'.
      ++k2;
      if (num_duplicates > 0 && k2 < num_rows)
        std::swap(rows[k2], rows[k2 + num_duplicates]);
    }
  }
  // If needed, swap any duplicates found past the pending rows
  // that has not been considered yet; then erase the duplicates.
  if (num_duplicates > 0) {
    if (k2 < num_rows)
      for (++k2; k2 < num_rows; ++k2)
        std::swap(rows[k2], rows[k2 + num_duplicates]);
    rows.resize(num_rows);
  }
  set_sorted(true);
  // Do not check for strong normalization,
  // because no modification of rows has occurred.
  PPL_ASSERT(OK(false));
}

template <typename Row>
bool
Linear_System<Row>::check_sorted() const {
  const Linear_System& x = *this;
  for (dimension_type i = first_pending_row(); i-- > 1; )
    if (compare(x[i], x[i-1]) < 0)
      return false;
  return true;
}

template <typename Row>
bool
Linear_System<Row>::OK(const bool /* check_strong_normalized */) const {
#ifndef NDEBUG
  using std::endl;
  using std::cerr;
#endif

  for (dimension_type i = rows.size(); i-- > 0; )
    if (rows[i].size() != num_columns()) {
#ifndef NDEBUG
      cerr << "Linear_System has a row with the wrong number of columns!"
           << endl;
#endif
      return false;
    }

  for (dimension_type i = rows.size(); i-- > 0; )
    if (rows[i].topology() != topology()) {
#ifndef NDEBUG
      cerr << "Linear_System has a row with the wrong topology!"
           << endl;
#endif
      return false;
    }

  // `index_first_pending' must be less than or equal to `num_rows()'.
  if (first_pending_row() > num_rows()) {
#ifndef NDEBUG
    cerr << "Linear_System has a negative number of pending rows!"
	 << endl;
#endif
    return false;
  }

  // An empty system is OK,
  // unless it is an NNC system with exactly one column.
  if (has_no_rows()) {
    if (is_necessarily_closed() || num_columns() != 1)
      return true;
    else {
#ifndef NDEBUG
      cerr << "NNC Linear_System has one column" << endl;
#endif
      return false;
    }
  }

  // A non-empty system will contain constraints or generators; in
  // both cases it must have at least one column for the inhomogeneous
  // term and, if it is NNC, another one for the epsilon coefficient.
  const dimension_type min_cols = is_necessarily_closed() ? 1 : 2;
  if (num_columns() < min_cols) {
#ifndef NDEBUG
    cerr << "Linear_System has fewer columns than the minimum "
	 << "allowed by its topology:"
	 << endl
	 << "num_columns is " << num_columns()
	 << ", minimum is " << min_cols
	 << endl;
#endif
    return false;
  }

  const Linear_System& x = *this;
  const dimension_type n_rows = num_rows();
  for (dimension_type i = 0; i < n_rows; ++i) {
    if (!x[i].OK(num_columns()))
      return false;
    // Checking for topology mismatches.
    if (x.topology() != x[i].topology()) {
#ifndef NDEBUG
      cerr << "Topology mismatch between the system "
	   << "and one of its rows!"
	   << endl;
#endif
      return false;
    }
  }

  // TODO: Re-enable this. It was disabled because a Linear_System can be a
  // Grid_Generator_System, which in turn can contain non-normalized rows
  // representing parameters.
  /*
    if (check_strong_normalized) {
      // Check for strong normalization of rows.
      // Note: normalization cannot be checked inside the
      // Row::OK() method, because a Linear_Row object may also
      // implement a Linear_Expression object, which in general cannot
      // be (strongly) normalized.
      Linear_System tmp(x, With_Pending());
      tmp.strong_normalize();
      if (x != tmp) {
  #ifndef NDEBUG
        cerr << "Linear_System rows are not strongly normalized!"
             << endl;
  #endif
        return false;
      }
    }
  */

  if (sorted && !check_sorted()) {
#ifndef NDEBUG
    cerr << "The system declares itself to be sorted but it is not!"
	 << endl;
#endif
    return false;
  }

  // All checks passed.
  return true;
}

} // namespace Parma_Polyhedra_Library

#endif // !defined(PPL_Linear_System_templates_hh)
