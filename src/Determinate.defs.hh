/* Determinate class declaration.
   Copyright (C) 2001-2004 Roberto Bagnara <bagnara@cs.unipr.it>

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

#ifndef PPL_Determinate_defs_hh
#define PPL_Determinate_defs_hh

#include "Determinate.types.hh"
#include "ConSys.types.hh"
#include "Variable.defs.hh"
#include "globals.hh"
#include <iosfwd>
#include <cassert>

namespace Parma_Polyhedra_Library {

//! \brief
//! Returns <CODE>true</CODE> if and only if
//! \p x and \p y are the same domain element.
/*!
  \relates Determinate
  \exception std::invalid_argument
  Thrown if \p x and \p y are topology-incompatible or
  dimension-incompatible.
*/
template <typename PH>
bool operator==(const Determinate<PH>& x, const Determinate<PH>& y);

//! \brief
//! Returns <CODE>true</CODE> if and only if
//! \p x and \p y are different domain elements.
/*!
  \relates Determinate
  \exception std::invalid_argument
  Thrown if \p x and \p y are topology-incompatible or
  dimension-incompatible.
*/
template <typename PH>
bool operator!=(const Determinate<PH>& x, const Determinate<PH>& y);

namespace IO_Operators {

//! Output operator.
/*! \relates Parma_Polyhedra_Library::Determinate */
template <typename PH>
std::ostream&
operator<<(std::ostream&, const Determinate<PH>&);

} // namespace IO_Operators

} // namespace Parma_Polyhedra_Library

//! Wraps a PPL class into a determinate constraint system interface.
template <typename PH>
class Parma_Polyhedra_Library::Determinate {
public:

  //! \name Constructors and Destructor
  //@{

  //! \brief
  //! Builds either the top or the bottom of the determinate constraint
  //! system defined on the vector space having \p num_dimensions
  //! dimensions.
  /*!
    The top element, corresponding to the whole vector space,
    is built if \p universe is \c true; otherwise the bottom element,
    corresponding to the emptyset, is built. By default,
    the top of a zero-dimension vector space is built.
  */
  explicit
  Determinate(dimension_type num_dimensions = 0, bool universe = true);

  //! \brief
  //! Injection operator: builds the determinate constraint system element
  //! corresponding to the base-level element \p p.
  Determinate(const PH& p);

  //! \brief
  //! Injection operator: builds the determinate constraint system element
  //! corresponding to the base-level element represented by \p cs.
  Determinate(const ConSys& cs);

  //! Copy constructor.
  Determinate(const Determinate& y);

  //! Destructor.
  ~Determinate();

  //@} // Constructors and Destructor

  //! \name Member Functions that Do Not Modify the Domain Element
  //@{

  //! Returns the dimension of the vector space enclosing \p *this.
  dimension_type space_dimension() const;

  //! Returns the system of constraints.
  const ConSys& constraints() const;

  //! Returns the system of constraints, with no redundant constraint.
  const ConSys& minimized_constraints() const;

  //! Returns a const reference to the embedded element.
  const PH& element() const;

  //! Returns a reference to the embedded element.
  PH& element();

#ifdef PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
  //! \brief
  //! On return from this method, the representation of \p *this
  //! is not shared by different Determinate objects.
#endif // PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
  void mutate();

  //! \brief
  //! Returns <CODE>true</CODE> if and only if \p *this is the top of the
  //! determinate constraint system (i.e., the whole vector space).
  bool is_top() const;

  //! \brief
  //! Returns <CODE>true</CODE> if and only if \p *this is the bottom
  //! of the determinate constraint system (i.e., the emptyset).
  bool is_bottom() const;

  //! \brief
  //! Returns <CODE>true</CODE> if and only if \p *this entails \p y
  //! (i.e., \p *this is contained into \p y).
  bool definitely_entails(const Determinate& y) const;

  //! \brief
  //! Returns <CODE>true</CODE> if and only if \p *this and \p y
  //! are equivalent.
  bool is_definitely_equivalent_to(const Determinate& y) const;

  //! Checks if all the invariants are satisfied.
  bool OK() const;

  //@} // Member Functions that Do Not Modify the Domain Element


  Determinate& operator <<= (dimension_type n);
  Determinate& hide_assign(dimension_type n);

  friend bool
  operator==<PH>(const Determinate<PH>& x, const Determinate<PH>& y);
  friend bool
  operator!=<PH>(const Determinate<PH>& x, const Determinate<PH>& y);

#if 0
  friend Determinate operator +<>(const Determinate& x,
				  const Determinate& y);
  friend Determinate operator *<>(const Determinate& x,
				  const Determinate& y);
#endif

  //! \name Space-Dimension Preserving Member Functions that May Modify the Domain Element
  //@{

  //! \brief
  //! Assigns to \p *this the upper bound of \p *this and \p y.
  void upper_bound_assign(const Determinate& y);

  //! Assigns to \p *this the meet of \p *this and \p y.
  void meet_assign(const Determinate& y);

  //! \brief
  //! Assigns to \p *this the meet of \p *this and the element
  //! represented by constraint \p c.
  /*!
    \exception std::invalid_argument
    Thrown if \p *this and constraint \p c are topology-incompatible
    or dimension-incompatible.
  */
  void add_constraint(const Constraint& c);

  //! \brief
  //! Assigns to \p *this the meet of \p *this and the element
  //! represented by the constraints in \p cs.
  /*!
    \param cs
    The constraints to intersect with.  This parameter is not declared
    <CODE>const</CODE> because it can be modified.

    \exception std::invalid_argument
    Thrown if \p *this and \p cs are topology-incompatible or
    dimension-incompatible.
  */
  void add_constraints(ConSys& cs);

  //@} // Space-Dimension Preserving Member Functions that May Modify [...]

  //! \name Member Functions that May Modify the Dimension of the Vector Space
  //@{

  //! Assignment operator.
  Determinate& operator=(const Determinate& y);

  //! Swaps \p *this with \p y.
  void swap(Determinate& y);

  //! \brief
  //! Adds \p m new dimensions and embeds the old domain element
  //! into the new vector space.
  void add_dimensions_and_embed(dimension_type m);

  //! \brief
  //! Adds \p m new dimensions to the domain element
  //! and does not embed it in the new vector space.
  void add_dimensions_and_project(dimension_type m);

  //! \brief
  //! Assigns to \p *this the \ref concatenate "concatenation"
  //! of \p *this and \p y, taken in this order.
  void concatenate_assign(const Determinate& y);

  //! \brief
  //! Removes all the specified dimensions.
  /*!
    \param to_be_removed
    The set of Variable objects corresponding to the dimensions to be
    removed.

    \exception std::invalid_argument
    Thrown if \p *this is dimension-incompatible with one of the
    Variable objects contained in \p to_be_removed.
  */
  void remove_dimensions(const Variables_Set& to_be_removed);

  //! \brief
  //! Removes the higher dimensions so that the resulting space
  //! will have dimension \p new_dimension.
  /*!
    \exception std::invalid_argument
    Thrown if \p new_dimensions is greater than the space dimension
    of \p *this.
  */
  void remove_higher_dimensions(dimension_type new_dimension);

  //! \brief
  //! Remaps the dimensions of the vector space according to
  //! a partial function.
  /*!
    See Polyhedron::map_dimensions.
  */
  template <typename PartialFunction>
  void map_dimensions(const PartialFunction& pfunc);

  //@} // Member Functions that May Modify the Dimension of the Vector Space

private:
  //! The possibly shared representation of a Determinate object.
  /*!
    By adopting the <EM>copy-on-write</EM> technique, a single
    representation of the base-level object may be shared by more than
    one object of the class Determinate.
  */
  class Rep {
  private:
    //! \brief
    //! Count the number of references:
    //! -   0: leaked, \p ph is non-const;
    //! -   1: one reference, \p ph is non-const;
    //! - > 1: more than one reference, \p ph is const.
    mutable unsigned long references;

    //! Private and unimplemented: assignment not allowed.
    Rep& operator=(const Rep& y);

    //! Private and unimplemented: copies not allowed.
    Rep(const Rep& y);

    //! Private and unimplemented: default construction not allowed.
    Rep();

  public:
    //! A possibly shared base-level domain element.
    PH ph;

    //! \brief
    //! Builds a new representation by creating a domain element
    //! of the specified kind, in the specified vector space.
    Rep(dimension_type num_dimensions, Polyhedron::Degenerate_Kind kind);

    //! Builds a new representation by copying base-level element \p p.
    Rep(const PH& p);

    //! Builds a new representation by copying the constraints in \p cs.
    Rep(const ConSys& cs);

    //! Destructor.
    ~Rep();

    //! Registers a new reference.
    void new_reference() const;

    //! \brief
    //! Unregisters one reference; returns <CODE>true</CODE> if and only if
    //! the representation has become unreferenced.
    bool del_reference() const;

    //! True if and only if this representation is currently shared.
    bool is_shared() const;
  };

  //! \brief
  //! A pointer to the possibly shared representation of
  //! the base-level domain element.
  Rep* prep;
};


namespace std {

//! Specializes <CODE>std::swap</CODE>.
/*! \relates Parma_Polyhedra_Library::Determinate */
template <typename PH>
void swap(Parma_Polyhedra_Library::Determinate<PH>& x,
	  Parma_Polyhedra_Library::Determinate<PH>& y);

} // namespace std

#include "Determinate.inlines.hh"

#endif // !defined(PPL_Determinate_defs_hh)
