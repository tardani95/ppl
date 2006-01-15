/* Grid_Generator_System class implementation: inline functions.
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

#ifndef PPL_Grid_Generator_System_inlines_hh
#define PPL_Grid_Generator_System_inlines_hh 1

#include "Grid_Generator.defs.hh"
#include "Generator_System.inlines.hh"

namespace Parma_Polyhedra_Library {

inline
Grid_Generator_System::Grid_Generator_System()
  : Generator_System(NECESSARILY_CLOSED) {
  adjust_topology_and_space_dimension(NECESSARILY_CLOSED, 1);
  set_sorted(false);
}

inline
Grid_Generator_System::Grid_Generator_System(const Grid_Generator_System& gs)
  : Generator_System(gs) {
}

inline
Grid_Generator_System::Grid_Generator_System(dimension_type dim)
  : Generator_System(NECESSARILY_CLOSED) {
  adjust_topology_and_space_dimension(NECESSARILY_CLOSED, dim + 1);
  set_sorted(false);
}

inline
Grid_Generator_System::Grid_Generator_System(const Grid_Generator& g)
  : Generator_System(g) {
  set_sorted(false);
}

inline dimension_type
Grid_Generator_System::max_space_dimension() {
  return Generator_System::max_space_dimension() - 1 /* parameter divisor */;
}

inline dimension_type
Grid_Generator_System::space_dimension() const {
  assert(Generator_System::space_dimension() > 0);
  return Generator_System::space_dimension() - 1 /* parameter divisor */;
}

inline void
Grid_Generator_System::clear() {
  Generator_System::clear();
  add_zero_columns(1);		// Parameter divisor column.
  set_sorted(false);
  unset_pending_rows();
}

inline void
Grid_Generator_System::swap(Grid_Generator_System& y) {
  Generator_System::swap(y);
}

inline memory_size_type
Grid_Generator_System::external_memory_in_bytes() const {
  return Generator_System::external_memory_in_bytes();
}

inline memory_size_type
Grid_Generator_System::total_memory_in_bytes() const {
  return Generator_System::total_memory_in_bytes();
}

inline dimension_type
Grid_Generator_System::num_rows() const {
  return Generator_System::num_rows();
}

inline dimension_type
Grid_Generator_System::num_rays() const {
  return Generator_System::num_rays();
}

inline dimension_type
Grid_Generator_System::num_lines() const {
  return Generator_System::num_lines();
}

inline
Grid_Generator_System::const_iterator::const_iterator()
  : Generator_System::const_iterator() {
}

inline
Grid_Generator_System::const_iterator::const_iterator(const const_iterator& y)
  : Generator_System::const_iterator(y) {
}

inline
Grid_Generator_System::const_iterator::~const_iterator() {
}

inline
Grid_Generator_System::const_iterator&
Grid_Generator_System::const_iterator::operator=(const const_iterator& y) {
  return static_cast<Grid_Generator_System::const_iterator&>
    (Generator_System::const_iterator::operator=(y));
}

inline const Grid_Generator&
Grid_Generator_System::const_iterator::operator*() const {
  return static_cast<const Grid_Generator&>
    (Generator_System::const_iterator::operator*());
}

inline const Grid_Generator*
Grid_Generator_System::const_iterator::operator->() const {
  return static_cast<const Grid_Generator*>
    (Generator_System::const_iterator::operator->());
}

inline Grid_Generator_System::const_iterator&
Grid_Generator_System::const_iterator::operator++() {
  return static_cast<Grid_Generator_System::const_iterator&>
    (Generator_System::const_iterator::operator++());
}

inline Grid_Generator_System::const_iterator
Grid_Generator_System::const_iterator::operator++(int) {
  const const_iterator tmp = *this;
  operator++();
  return tmp;
}

inline bool
Grid_Generator_System::const_iterator::operator==(const const_iterator& y) const {
  return Generator_System::const_iterator::operator==(y);
}

inline bool
Grid_Generator_System::const_iterator::operator!=(const const_iterator& y) const {
  return Generator_System::const_iterator::operator!=(y);
}

inline Grid_Generator_System::const_iterator
Grid_Generator_System::begin() const {
  return static_cast<Grid_Generator_System::const_iterator>
    (Generator_System::begin());
}

inline Grid_Generator_System::const_iterator
Grid_Generator_System::end() const {
  return static_cast<Grid_Generator_System::const_iterator>
    (Generator_System::end());
}

inline
Grid_Generator_System::const_iterator::const_iterator(const Generator_System::const_iterator& y)
  : Generator_System::const_iterator::const_iterator(y) {
}

inline bool
Grid_Generator_System::has_points() const {
  return Generator_System::has_points();
}

inline Grid_Generator&
Grid_Generator_System::operator[](const dimension_type k) {
  return static_cast<Grid_Generator&>(Generator_System::operator[](k));
}

inline const Grid_Generator&
Grid_Generator_System::operator[](const dimension_type k) const {
  return static_cast<const Grid_Generator&>(Generator_System::operator[](k));
}

inline void
Grid_Generator_System::ascii_dump(std::ostream& s) const {
  return Generator_System::ascii_dump(s);
}

inline void
Grid_Generator_System::set_sorted(bool b) {
  Generator_System::set_sorted(b);
}

inline bool
Grid_Generator_System::is_equal_to(const Grid_Generator_System y) const {
  return operator==(static_cast<const Generator_System&>(*this),
		    static_cast<const Generator_System&>(y));
}

/*! \relates Grid_Generator_System */
inline bool
operator==(const Grid_Generator_System& x,
	   const Grid_Generator_System& y) {
  return x.is_equal_to(y);
}

} // namespace Parma_Polyhedra_Library


namespace std {

/*! \relates Parma_Polyhedra_Library::Constraint_System */
inline void
swap(Parma_Polyhedra_Library::Grid_Generator_System& x,
     Parma_Polyhedra_Library::Grid_Generator_System& y) {
  x.swap(y);
}

} // namespace std

#endif // !defined(PPL_Grid_Generator_System_inlines_hh)
