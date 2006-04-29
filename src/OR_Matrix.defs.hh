/* OR_Matrix class declaration.
   Copyright (C) 2001-2003 Roberto Bagnara <bagnara@cs.unipr.it>

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
site: http://www.cs.unipr.it/ppl/ .  */

#ifndef PPL_OR_Matrix_defs_hh
#define PPL_OR_Matrix_defs_hh 1

#include "globals.defs.hh"
#include "Ptr_Iterator.defs.hh"
#include "OR_Matrix.types.hh"
#include "DB_Row.defs.hh"
#include "Checked_Number.defs.hh"
#include <cstddef>
#include <iosfwd>

#ifndef EXTRA_ROW_DEBUG
#define EXTRA_ROW_DEBUG 1
#endif

namespace Parma_Polyhedra_Library {

// Put it in the namespace here to declare it friend later.
template <typename T>
bool operator==(const OR_Matrix<T>& x, const OR_Matrix<T>& y);

namespace IO_Operators {

#ifdef PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
//! Output operator.
/*! \relates Parma_Polyhedra_Library::OR_Matrix */
#endif // PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
template <typename T>
std::ostream&
operator<<(std::ostream& s, const OR_Matrix<T>& m);

} // namespace IO_Operators

} // namespace Parma_Polyhedra_Library


#ifdef PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
//! A matrix representing octagonal constraints.
/*!
  An OR_Matrix object is a DB_Row object that allows
  the representation of a \em pseudo-triangular matrix,
  like the following:

<PRE>
         _ _
   0    |_|_|
   1    |_|_|_ _
   2    |_|_|_|_|
   3    |_|_|_|_|_ _
   4    |_|_|_|_|_|_|
   5    |_|_|_|_|_|_|
         . . .
         _ _ _ _ _ _       _
 2n-2   |_|_|_|_|_|_| ... |_|
 2n-1   |_|_|_|_|_|_| ... |_|
         0 1 2 3 4 5  ... 2n-1

</PRE>

  It is characterized by parameter n that defines the structure,
  and such that there are 2*n rows (and 2*n columns).
  It provides row_iterators for the access to the rows
  and element_iterators for the access to the elements.
*/
#endif // PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS

template <typename T>
class Parma_Polyhedra_Library::OR_Matrix {
private:
  /*! \brief
    An object that behaves like a matrix's row with respect to
    the subscript operators.
  */
  template <typename U>
  class Pseudo_Row {
  public:
    /*! \brief
      Copy-constructor allowing the construction of a const pseudo-row
      from a non-const pseudo-row.
      Ordinary copy constructor.
    */
    template <typename V>
    Pseudo_Row(const Pseudo_Row<V>& y);

    //! Destructor.
    ~Pseudo_Row();

    //! Subscript operator.
    U& operator[](dimension_type k) const;

    // FIXME!!!
    //private:
  public:
    //! Holds a reference to the beginning of this row.
    U* first;

    //! Default constructor: creates a past-the-end object.
    Pseudo_Row();

#if EXTRA_ROW_DEBUG

    //! Private constructor for a Pseudo_Row with size \p s beginning at \p y.
    Pseudo_Row(U& y, dimension_type s);

#else // !EXTRA_ROW_DEBUG

    //! Private constructor for a Pseudo_Row beginning at \p y.
    explicit Pseudo_Row(U& y);

#endif // !EXTRA_ROW_DEBUG

    //! Assignment operator.
    Pseudo_Row& operator=(const Pseudo_Row& y);

#if EXTRA_ROW_DEBUG

    //! The size of the row.
    dimension_type size_;

    //! Returns the size of the row.
    dimension_type size() const;

#endif // EXTRA_ROW_DEBUG

    template <typename V> friend class Pseudo_Row;
    template <typename V> friend class any_row_iterator;
    friend class OR_Matrix;
  }; // class Pseudo_Row

public:
  //! A (non const) reference to a matrix's row.
  typedef Pseudo_Row<T> row_reference_type;

  //! A const reference to a matrix's row.
  typedef Pseudo_Row<const T> const_row_reference_type;

private:
  /*! \brief
    A template class to derive both OR_Matrix::iterator
    and OR_Matrix::const_iterator.
  */
  template <typename U>
  class any_row_iterator {
  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef Pseudo_Row<U> value_type;
    typedef long difference_type;
    typedef const Pseudo_Row<U>* pointer;
    typedef const Pseudo_Row<U>& reference;

    //! Constructor to build past-the-end objects.
    any_row_iterator(dimension_type n_rows);

    /*! \brief
      Builds an iterator pointing at the beginning of an OR_Matrix whose
      first element is \p base;
    */
    explicit any_row_iterator(U& base);

    /*! \brief
      Copy-constructor allowing the construction of a const_iterator
      from a non-const iterator.
    */
    template <typename V>
    any_row_iterator(const any_row_iterator<V>& y);

    /*! \brief
      Assignment operator allowing the assignment of a non-const iterator
      to a const_iterator.
    */
    template <typename V>
    any_row_iterator& operator=(const any_row_iterator<V>& y);

    //! Dereference operator.
    reference operator*() const;

    //! Indirect member selector.
    pointer operator->() const;

    //! Prefix increment operator.
    any_row_iterator& operator++();

    //! Postfix increment operator.
    any_row_iterator operator++(int);

    //! Prefix decrement operator.
    any_row_iterator& operator--();

    //! Postfix decrement operator.
    any_row_iterator operator--(int);

    //! Subscript operator.
    reference operator[](difference_type m) const;

    //! Assignment-increment operator.
    any_row_iterator& operator+=(difference_type m);

    //! Assignment-decrement operator.
    any_row_iterator& operator-=(difference_type m);

    //! Returns the difference between \p *this and \p y.
    difference_type operator-(const any_row_iterator& y) const;

    //! Returns the sum of \p *this and \p m.
    any_row_iterator operator+(difference_type m) const;

    //! Returns the difference of \p *this and \p m.
    any_row_iterator operator-(difference_type m) const;

    //! Returns <CODE>true</CODE> if and only if \p *this is equal to \p y.
    bool operator==(const any_row_iterator& y) const;

    /*! \brief
      Returns <CODE>true</CODE> if and only if \p *this
      is different from \p y.
    */
    bool operator!=(const any_row_iterator& y) const;

    //! Returns <CODE>true</CODE> if and only if \p *this is less than \p y.
    bool operator<(const any_row_iterator& y) const;

    /*! \brief
      Returns <CODE>true</CODE> if and only if \p *this is less than
      or equal to \p y.
    */
    bool operator<=(const any_row_iterator& y) const;

    //! Returns <CODE>true</CODE> if and only if \p *this is greater than \p y.
    bool operator>(const any_row_iterator& y) const;

    /*! \brief
      Returns <CODE>true</CODE> if and only if \p *this is greater than
      or equal to \p y.
    */
    bool operator>=(const any_row_iterator& y) const;

    dimension_type row_size() const;

    dimension_type index() const;

  private:
    //! Represents the beginning of a row.
    Pseudo_Row<U> value;

    //! External index.
    dimension_type e;

    //! Internal index: <CODE>i = (e+1)*(e+1)/2</CODE>.
    dimension_type i;

    template <typename V> friend class any_row_iterator;
  }; // class any_row_iterator

public:
  //! A (non const) row iterator.
  typedef any_row_iterator<T> row_iterator;

  //! A const row iterator.
  typedef any_row_iterator<const T> const_row_iterator;

  //! A (non const) element iterator.
  typedef typename DB_Row<T>::iterator element_iterator;

  //! A const element iterator.
  typedef typename DB_Row<T>::const_iterator const_element_iterator;

public:
  //! Returns the maximum number of rows of a OR_Matrix.
  static dimension_type max_num_rows();

  //! Builds a matrix with specified dimensions.
  /*!
    \param space_dim
    The space dimension of the matrix that will be created.

    This constructor creates a matrix with \p 2*space_dim rows.
    Each element is inizialized to plus infinity.
  */
  OR_Matrix(dimension_type space_dim);

  //! Copy-constructor.
  OR_Matrix(const OR_Matrix& y);

  //! Destructor.
  ~OR_Matrix();

  //! Assignment operator.
  OR_Matrix& operator=(const OR_Matrix& y);

private:
  //! Contains the rows of the matrix.
  /*!
    Creates a DB_Row which contains the rows of the OR_Matrix
    inserting each successive row to the end of the vec.
    To contain all the elements of OR_Matrix the size of the DB_Row
    is 2*n*(n+1), where the n is the caracteristic parameter of
    OR_Matrix.
  */
  DB_Row<T> vec;

  //! Contains the dimension of the space of the matrix.
  dimension_type space_dim;

  //! Contains the capacity of \p vec.
  dimension_type vec_capacity;

  // Private and not implemented: default construction is not allowed.
  OR_Matrix();

  /*! \brief
    Returns the index into <CODE>vec</CODE> of the first element
    of the row of index \p k.
  */
  static dimension_type row_first_element_index(dimension_type k);

public:
  //! Returns the size of the row of index \p k.
  static dimension_type row_size(dimension_type k);

  //! Swaps \p *this with \p y.
  void swap(OR_Matrix& y);


  //! Makes the matrix grow by adding more space dimensions.
  /*!
    \param new_dim
    The new dimension of the resized matrix.

    Adds new rows of right dimension to the end if
    there is enough capacity; otherwise, creates a new matrix,
    with the specified dimension, copying the old elements
    in the upper part of the new matrix, which is
    then assigned to \p *this.
    Each new element is inizialized to plus infinity.
  */
  void grow(dimension_type new_dim);

  //! Makes the matrix shrink by removing the last space dimensions.
  /*!
    \param new_dim
    The new dimension of the resized matrix.

    Erases from matrix to the end the rows with index
    greater than 2*new_dim-1.
  */
  void shrink(dimension_type new_dim);

  //! Resizes the matrix without worrying about the old contents.
  /*!
    \param new_dim
    The new dimension of the resized matrix.

    If the new dimension is greater than the old one, it
    adds new rows of right dimension to the end if
    there is enough capacity; otherwise, it creates a new matrix,
    with the specified dimension, copying the old elements
    in the upper part of the new matrix, which is
    then assigned to \p *this. In this case each element is
    inizialized to plus infinity.
    If the new dimension is less than the old one, it erase
    from matrix to the end the rows with index greater than 2*new_dim-1
  */
  void resize_no_copy(dimension_type new_dim);

  //! Returns the space-dimension of the matrix.
  dimension_type space_dimension() const;

  //! Returns the number of rows in the matrix.
  dimension_type num_rows() const;

  //! \name Subscript operators.
  //@{
  //! Returns a reference to the \p k-th row of the matrix.
  row_reference_type operator[](dimension_type k);

  //! Returns a constant reference to the \p k-th row of the matrix.
  const_row_reference_type operator[](dimension_type k) const;
  //@}


  /*! \brief
    Returns an iterator pointing to the first row,
    if \p *this is not empty;
    otherwise, returns the past-the-end const_iterator.
  */
  row_iterator row_begin();

  //! Returns the past-the-end const_iterator.
  row_iterator row_end();

  /*! \brief
    Returns a const row iterator pointing to the first row,
    if \p *this is not empty;
    otherwise, returns the past-the-end const_iterator.
  */
  const_row_iterator row_begin() const;

  //! Returns the past-the-end const row iterator.
  const_row_iterator row_end() const;

  /*! \brief
    Returns an iterator pointing to the first element,
    if \p *this is not empty;
    otherwise, returns the past-the-end const_iterator.
  */
  element_iterator element_begin();

  //! Returns the past-the-end const_iterator.
  element_iterator element_end();

  /*! \brief
    Returns a const element iterator pointing to the first element,
    if \p *this is not empty;
    otherwise, returns the past-the-end const_iterator.
  */
  const_element_iterator element_begin() const;

  //! Returns the past-the-end const element iterator.
  const_element_iterator element_end() const;

  //! Clears the matrix deallocating all its rows.
  void clear();

  PPL_OUTPUT_DECLARATIONS

  /*! \brief
    Loads from \p s an ASCII representation (as produced by \ref ascii_dump)
    and sets \p *this accordingly.  Returns <CODE>true</CODE>
    if successful, <CODE>false</CODE> otherwise.
  */
  bool ascii_load(std::istream& s);

  friend bool Parma_Polyhedra_Library::operator==<T>(const OR_Matrix<T>& x,
						     const OR_Matrix<T>& y);

  //! Checks if all the invariants are satisfied.
  bool OK() const;
};

namespace std {

#ifdef Parma_Polyhedra_Library_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
//! Specializes <CODE>std::swap</CODE>.
/*! \relates Parma_Polyhedra_Library::OR_Matrix */
#endif // Parma_Polyhedra_Library_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
template <typename T>
void swap(Parma_Polyhedra_Library::OR_Matrix<T>& x,
	  Parma_Polyhedra_Library::OR_Matrix<T>& y);

} // namespace std


namespace Parma_Polyhedra_Library {

#ifdef Parma_Polyhedra_Library_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
//! Returns <CODE>true</CODE> if and only if \p x and \p y are identical.
/*! \relates OR_Matrix */
#endif // Parma_Polyhedra_Library_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
template <typename T>
bool operator==(const OR_Matrix<T>& x, const OR_Matrix<T>& y);

#ifdef Parma_Polyhedra_Library_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
//! Returns <CODE>true</CODE> if and only if \p x and \p y are different.
/*! \relates OR_Matrix */
#endif // Parma_Polyhedra_Library_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
template <typename T>
bool operator!=(const OR_Matrix<T>& x, const OR_Matrix<T>& y);


} // namespace Parma_Polyhedra_Library

#include "OR_Matrix.inlines.hh"
#include "OR_Matrix.templates.hh"

#endif // !defined(PPL_OR_Matrix_defs_hh)
