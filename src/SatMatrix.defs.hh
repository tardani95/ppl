/* SatMatrix class declaration.
   Copyright (C) 2001, 2002 Roberto Bagnara <bagnara@cs.unipr.it>

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
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.

For the most up-to-date information see the Parma Polyhedra Library
site: http://www.cs.unipr.it/ppl/ . */

#ifndef PPL_SatMatrix_defs_hh
#define PPL_SatMatrix_defs_hh 1

#include "SatMatrix.types.hh"
#include "SatRow.defs.hh"
#include <vector>
#include <iosfwd>

#ifdef PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
/*!
  This class specifies a saturation matrix.
  We use a saturation matrix to specify the relation between the
  generators and the constraints of a polyhedron: if a generator
  saturates a constraint the corresponding element of the saturation
  matrix is \f$0\f$, otherwise (i.e. the generator only satisfies
  the constraint) the corresponding element is \f$1\f$.
*/
#endif // PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS

class Parma_Polyhedra_Library::SatMatrix {
public:
  //! Default constructor.
  SatMatrix();

  //! Construct a saturation matrix with \p n_rows rows
  //! and \p n_columns columns.
  SatMatrix(dimension_type n_rows, dimension_type n_columns);

  //! Copy-constructor.
  SatMatrix(const SatMatrix& y);

  //! Destructor.
  ~SatMatrix();

  //! Assignment operator.
  SatMatrix& operator=(const SatMatrix& y);

  //! Swaps \p *this with \p y.
  void swap(SatMatrix& y);

  //! Subscript operator.
  SatRow& operator[](dimension_type k);

  //! Subscript operator.
  const SatRow& operator[](dimension_type k) const;

  //! Clears the matrix deallocating all its rows.
  void clear();

  //! Transposes the matrix.
  void transpose();

  //! Makes \p *this a transposed copy of \p y.
  void transpose_assign(const SatMatrix& y);

  //! Returns the number of columns of \p *this.
  dimension_type num_columns() const;

  //! Returns the number of rows of \p *this.
  dimension_type num_rows() const;

  //! Sorts the rows and removes duplicates.
  void sort_rows();

  //! Looks for \p row in \p *this that is assumed to be sorted.
  bool sorted_contains(const SatRow& row) const;

  //! Adds \p row to \p *this.
  void add_row(const SatRow& row);

  //! Erases the rows from the \p first_to_erase -th to the last one.
  void rows_erase_to_end(dimension_type first_to_erase);

  //! Erases the columns from the \p first_to_erase -th to the last one.
  void columns_erase_to_end(dimension_type first_to_erase);

  //! Resizes the matrix copying the old contents.
  void resize(dimension_type new_n_rows, dimension_type new_n_columns);

  //! Checks if all the invariants are satisfied.
  bool OK() const;

  //! Writes to \p s an ASCII representation of the internal
  //! representation of \p *this.
  void ASCII_dump(std::ostream& s) const;

  //! Loads from \p s an ASCII representation (as produced by \ref
  //! ASCII_dump) and sets \p *this accordingly.  Returns <CODE>true</CODE>
  //! if successful, <CODE>false</CODE> otherwise.
  bool ASCII_load(std::istream& s);

#ifndef NDEBUG
  //! Checks whether \p *this is sorted.
  //! It does NOT check for duplicates.
  bool check_sorted() const;
#endif

private:
  //! Contains the rows of the matrix.
  std::vector<SatRow> rows;

  //! Size of the initialized part of each row.
  dimension_type row_size;

  struct RowCompare {
    bool operator()(const SatRow& x, const SatRow& y) const;
  };
};

namespace std {

#ifdef PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
  //! Specializes <CODE>std::swap</CODE>.
  /*! \relates Parma_Polyhedra_Library::SatMatrix */
#endif // PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
void swap(Parma_Polyhedra_Library::SatMatrix& x,
	  Parma_Polyhedra_Library::SatMatrix& y);

} // namespace std

namespace Parma_Polyhedra_Library {

#ifdef PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
//! \brief Returns <CODE>true</CODE> if and only if
//! \p x and \p y are identical.
/*! \relates SatMatrix */
#endif // PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
bool operator==(const SatMatrix& x, const SatMatrix& y);

#ifdef PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
//! \brief Returns <CODE>true</CODE> if and only if
//! \p x and \p y are different.
/*! \relates SatMatrix */
#endif // PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
bool operator!=(const SatMatrix& x, const SatMatrix& y);

} // namespace Parma_Polyhedra_Library

#include "SatMatrix.inlines.hh"

#endif // !defined(PPL_SatMatrix_defs_hh)
