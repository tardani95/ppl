/* Congruence_System class implementation: inline functions.
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

#ifndef PPL_Congruence_System_inlines_hh
#define PPL_Congruence_System_inlines_hh 1

#include "Congruence.defs.hh"

namespace Parma_Polyhedra_Library {

inline Congruence&
Congruence_System::operator[](const dimension_type k) {
  return static_cast<Congruence&>(rows[k]);
}

inline const Congruence&
Congruence_System::operator[](const dimension_type k) const {
  return static_cast<const Congruence&>(rows[k]);
}

inline dimension_type
Congruence_System::num_rows() const {
  return rows.size();
}

inline bool
Congruence_System::has_no_rows() const {
  return num_rows() == 0;
}


inline dimension_type
Congruence_System::num_columns() const {
  return num_columns_;
}

inline void
Congruence_System::add_zero_columns(dimension_type n) {
  num_columns_ += n;
  for (dimension_type i = num_rows(); i-- > 0; )
    rows[i].resize(num_columns_);
}

inline void
Congruence_System::remove_trailing_columns(dimension_type n) {
  PPL_ASSERT(num_columns_ >= n);
  num_columns_ -= n;
  for (dimension_type i = num_rows(); i-- > 0; )
    rows[i].resize(num_columns_);
}

inline void
Congruence_System::add_zero_rows(dimension_type n, Dense_Row::Flags flags) {
  rows.resize(num_rows() + n);
  for (dimension_type i = n; i > 0; --i) {
    rows[num_rows() - i].set_flags(flags);
    rows[num_rows() - i].resize(num_columns_);
  }
}

inline void
Congruence_System::remove_trailing_rows(dimension_type n) {
  PPL_ASSERT(num_rows() >= n);
  rows.resize(num_rows() - n);
}

inline void
Congruence_System::release_rows(Swapping_Vector<Dense_Row>& v) {
  PPL_ASSERT(v.empty());
  std::swap(rows, v);
}

inline void
Congruence_System::take_ownership_of_rows(Swapping_Vector<Dense_Row>& v) {
  PPL_ASSERT(rows.size() == 0);
  std::swap(rows, v);
}

inline void
Congruence_System::insert(const Congruence& cg) {
  insert_verbatim(cg);
  static_cast<Congruence&>(rows.back()).strong_normalize();
  PPL_ASSERT(OK());
}

inline
Congruence_System::Congruence_System()
  : rows(),
    num_columns_(2) {
}

inline
Congruence_System::Congruence_System(const Congruence& cg)
  : rows(),
    num_columns_(2) {
  insert(cg);
}

inline
Congruence_System::Congruence_System(const Constraint& c)
  : rows(),
    num_columns_(2) {
  insert(c);
}

inline
Congruence_System::Congruence_System(const Congruence_System& cs)
  : rows(cs.rows),
    num_columns_(cs.num_columns_) {
}

inline
Congruence_System::Congruence_System(const dimension_type d)
  : rows(),
    num_columns_(d + 2) {
}

inline
Congruence_System::~Congruence_System() {
}

inline Congruence_System&
Congruence_System::operator=(const Congruence_System& y) {
  rows = y.rows;
  num_columns_ = y.num_columns_;
  return *this;
}

inline dimension_type
Congruence_System::max_space_dimension() {
  return Dense_Row::max_size() - 2;
}

inline dimension_type
Congruence_System::space_dimension() const {
  return num_columns() - 2;
}

inline void
Congruence_System::clear() {
  rows.clear();
  num_columns_ = 2;		// Modulus and constant term.
}

inline void
Congruence_System::resize_no_copy(const dimension_type new_num_rows,
				  const dimension_type new_num_columns) {
  rows.resize(new_num_rows);
  for (dimension_type i = new_num_rows; i-- > 0; )
    rows[i].resize(new_num_columns);
  num_columns_ = new_num_columns;
}

inline const Congruence_System&
Congruence_System::zero_dim_empty() {
  PPL_ASSERT(zero_dim_empty_p != 0);
  return *zero_dim_empty_p;
}

inline
Congruence_System::const_iterator::const_iterator()
  : i(), csp(0) {
}

inline
Congruence_System::const_iterator::const_iterator(const const_iterator& y)
  : i(y.i), csp(y.csp) {
}

inline
Congruence_System::const_iterator::~const_iterator() {
}

inline Congruence_System::const_iterator&
Congruence_System::const_iterator::operator=(const const_iterator& y) {
  i = y.i;
  csp = y.csp;
  return *this;
}

inline const Congruence&
Congruence_System::const_iterator::operator*() const {
  return static_cast<const Congruence&>(*i);
}

inline const Congruence*
Congruence_System::const_iterator::operator->() const {
  return static_cast<const Congruence*>(i.operator->());
}

inline Congruence_System::const_iterator&
Congruence_System::const_iterator::operator++() {
  ++i;
  skip_forward();
  return *this;
}

inline Congruence_System::const_iterator
Congruence_System::const_iterator::operator++(int) {
  const const_iterator tmp = *this;
  operator++();
  return tmp;
}

inline bool
Congruence_System::const_iterator::operator==(const const_iterator& y) const {
  return i == y.i;
}

inline bool
Congruence_System::const_iterator::operator!=(const const_iterator& y) const {
  return i != y.i;
}

inline
Congruence_System::const_iterator::
const_iterator(const Swapping_Vector<Dense_Row>::const_iterator& iter,
	       const Congruence_System& csys)
  : i(iter), csp(&csys.rows) {
}

inline Congruence_System::const_iterator
Congruence_System::begin() const {
  const_iterator i(rows.begin(), *this);
  i.skip_forward();
  return i;
}

inline Congruence_System::const_iterator
Congruence_System::end() const {
  const const_iterator i(rows.end(), *this);
  return i;
}

inline bool
Congruence_System::empty() const {
  return begin() == end();
}

inline void
Congruence_System::swap(Congruence_System& y) {
  std::swap(rows, y.rows);
  std::swap(num_columns_, y.num_columns_);
}

inline memory_size_type
Congruence_System::external_memory_in_bytes() const {
  return rows.external_memory_in_bytes();
}

inline memory_size_type
Congruence_System::total_memory_in_bytes() const {
  return rows.external_memory_in_bytes() + sizeof(*this);
}

} // namespace Parma_Polyhedra_Library


namespace std {

/*! \relates Parma_Polyhedra_Library::Congruence_System */
inline void
swap(Parma_Polyhedra_Library::Congruence_System& x,
     Parma_Polyhedra_Library::Congruence_System& y) {
  x.swap(y);
}

} // namespace std

#endif // !defined(PPL_Congruence_System_inlines_hh)
