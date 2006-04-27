/* Octagon class declaration.
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

#ifndef PPL_Octagon_defs_hh
#define PPL_Octagon_defs_hh 1

#include "Octagon.types.hh"
#include "globals.types.hh"
#include "Constraint.types.hh"
#include "Generator.types.hh"
#include "Linear_Expression.types.hh"
#include "Constraint_System.types.hh"
#include "Generator_System.types.hh"
#include "OR_Matrix.defs.hh"
#include "Poly_Con_Relation.defs.hh"
#include "Poly_Gen_Relation.defs.hh"
#include "Polyhedron.types.hh"
#include "Variable.defs.hh"
#include "Checked_Number.defs.hh"
#include <vector>
#include <cstddef>
#include <climits>
#include <iosfwd>


namespace Parma_Polyhedra_Library {

namespace IO_Operators {

//! Output operator.
/*! \relates Parma_Polyhedra_Library::Octagon
  Writes a textual representation of \p oct on \p s:
  <CODE>false</CODE> is written if \p oct is an empty polyhedron;
  <CODE>true</CODE> is written if \p oct is a universe polyhedron;
  a system of constraints defining \p oct is written otherwise,
  all constraints separated by ", ".
*/
template <typename T>
std::ostream&
operator<<(std::ostream& s, const Octagon<T>& oct);

} // namespace IO_Operators

/*! \brief
  Returns <CODE>true</CODE> if and only if \p x and \p y are the same octagon.

  \relates Octagon
  Note that \p x and \p y may be dimension-incompatible shapes:
  in this case, the value <CODE>false</CODE> is returned.
*/
template <typename T>
bool operator==(const Octagon<T>& x, const Octagon<T>& y);

/*! \brief
  Returns <CODE>true</CODE> if and only if \p x and \p y are different shapes.

  \relates Octagon
  Note that \p x and \p y may be dimension-incompatible shapes:
  in this case, the value <CODE>true</CODE> is returned.
*/
template <typename T>
bool operator!=(const Octagon<T>& x, const Octagon<T>& y);

#ifdef PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
//! Decodes the constraint \p c as an octagonal difference.
/*!
  \return
  <CODE>true</CODE> if the constraint \p c is an octagonal difference;
  <CODE>false</CODE> otherwise.

  \param c
  The constraint to be decoded.

  \param c_space_dim
  The space dimension of the constraint \p c (it is <EM>assumed</EM>
  to match the actual space dimension of \p c).

  \param c_num_vars
  If <CODE>true</CODE> is returned, then it will be set to the number
  of variables having a non-zero coefficient. The only legal values
  will therefore be 0, 1 and 2.

  \param c_first_var
  If <CODE>true</CODE> is returned and if \p c_num_vars is not set to 0,
  then it will be set to the index of the first variable having
  a non-zero coefficient in \p c.

  \param c_second_var
  If <CODE>true</CODE> is returned and if \p c_num_vars is set to 2,
  then it will be set to the index of the second variable having
  a non-zero coefficient in \p c.

  \param c_coeff
  If <CODE>true</CODE> is returned and if \p c_num_vars is not set to 0,
  then it will be set to the value of the first non-zero coefficient
  in \p c.

  \param c_term
  If <CODE>true</CODE> is returned and if \p c_num_vars is not set to 0,
  then it will be set to the right value of the inhomogeneous term
  of \p c.
*/
#endif // PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
bool extract_octagonal_difference(const Constraint& c,
				  const dimension_type c_space_dim,
				  dimension_type& c_num_vars,
				  dimension_type& c_first_var,
				  dimension_type& c_second_var,
				  Coefficient& c_coeff,
				  Coefficient& c_term);

} // namespace Parma_Polyhedra_Library

//! An octagonal shape.
/*!
  The class template Octagon<T> allows for the efficient representation
  of a restricted kind of <EM>topologically closed</EM> convex polyhedra
  called <EM>octagonal shapes</EM> (OSs, for short).  The name comes
  from the fact that, in a vector space of dimension 2, bounded OSs
  are polygons with at most eight sides.
  The closed affine half-spaces that characterize the OS can be expressed
  by constraints of the form
  \f[
    ax_i + bx_j \leq k
  \f]
  where \f$a, b \in \{-1, 0, 1\}\f$ and \f$k\f$ is a rational number,
  which are called <EM>octagonal constraints</EM>.

  Based on the class template type parameter \p T, a family of extended
  numbers is built and used to approximate the inhomogeneous term of
  octagonal constraints. These extended numbers provide a representation
  for the value \f$+\infty\f$, as well as <EM>rounding-aware</EM>
  implementations for several arithmetic functions.
  The value of the type parameter \p T may be one of the following:
    - a bounded precision integer type (e.g., \c int32_t or \c int64_t);
    - a bounded precision floating point type (e.g., \c float or \c double);
    - an unbounded integer or rational type, as provided by GMP
      (i.e., \c mpz_class or \c mpq_class).

  The user interface for OSs is meant to be as similar as possible to
  the one developed for the polyhedron class C_Polyhedron.  At the
  interface level, octagonal constraints are specified using objects of
  type Constraint: such a constraint is an octagonal constraint if it is
  of the form
    \f[
      a_i x_i + a_j x_j \relsym b
    \f]
  where \f$\mathord{\relsym} \in \{ \leq, =, \geq \}\f$ and
  \f$a_i\f$, \f$a_j\f$, \f$b\f$ are integer coefficients such that
  \f$a_i = 0\f$, or \f$a_j = 0\f$, or \f$\abs{a_i} = \abs{a_j}\f$.
  The user is warned that the above Constraint object will be mapped
  into a <EM>correct</EM> approximation that, depending on the expressive
  power of the chosen template argument \p T, may loose some precision.
  In particular, constraint objects that do not encode an octagonal
  constraint will be simply (and safely) ignored.

  For instance, a Constraint object encoding \f$3x + 3y \leq 1\f$ will be
  approximated by:
    - \f$x + y \leq 1\f$,
      if \p T is a (bounded or unbounded) integer type;
    - \f$x + y \leq \frac{1}{3}\f$,
      if \p T is the unbounded rational type \c mpq_class;
    - \f$x + y \leq k\f$, where \f$k > \frac{1}{3}\f$,
      if \p T is a floating point type (having no exact representation
      for \f$\frac{1}{3}\f$).

  On the other hand, a Constraint object encoding \f$3x - y \leq 1\f$
  will be safely ignored in all of the above cases.

  In the following examples it is assumed that the type argument \p T
  is one of the possible instances listed above and that variables
  <CODE>x</CODE>, <CODE>y</CODE> and <CODE>z</CODE> are defined
  (where they are used) as follows:
  \code
    Variable x(0);
    Variable y(1);
    Variable z(2);
  \endcode

  \par Example 1
  The following code builds an OS corresponding to a cube in \f$\Rset^3\f$,
  given as a system of constraints:
  \code
    Constraint_System cs;
    cs.insert(x >= 0);
    cs.insert(x <= 3);
    cs.insert(y >= 0);
    cs.insert(y <= 3);
    cs.insert(z >= 0);
    cs.insert(z <= 3);
    Octagon<T> oct(cs);
  \endcode
  Since only those constraints having the syntactic form of an
  <EM>octagonal constraint</EM> are considered, the following code
  will build the same OS as above (i.e., the constraints 7, 8, and 9
  are ignored):
  \code
    Constraint_System cs;
    cs.insert(x >= 0);
    cs.insert(x <= 3);
    cs.insert(y >= 0);
    cs.insert(y <= 3);
    cs.insert(z >= 0);
    cs.insert(z <= 3);
    cs.insert(x - 3*y <= 5);    // (7)
    cs.insert(x - y + z <= 5);  // (8)
    cs.insert(x + y + z <= 5);  // (9)
    Octagon<T> oct(cs);
  \endcode

  // ENEA: checked up to this point.

  \par Example 2
  The following code shows the use of the function
  <CODE>affine_image</CODE>:
  \code
    Octagon<T> oc(2);
    oc.add_constraint(x >= 0);
    oc.add_constraint(y >= 0);
    oc.add_constraint(x + y >= 0);
    Linear_Expression expr(2*x + 2);
    oc.affine_image(x, expr, 2);
  \endcode
  The linear epressions must be at most in one variable and
  the absolute value of coefficient of the single variable must be
  equal to absolute value of denominator (like in the example above).
  In this example the starting octagon is the first quadrant in
  \f$\Rset^2\f$, the considered variable is \f$x\f$ and the affine
  expression is \f$2*x+2\f$. The resulting octagon is the same octagon
  traslated to right.  Moreover, if the affine transformation for
  the same variable \p x is \f$2*y\f$:
  \code
    Linear_Expression expr = 2*y;
  \endcode
  the resulting octagon is the part non-negative of the first bisector.

  \par Example 3
  The following code shows the use of the function
  <CODE>affine_preimage</CODE>:
  \code
    Octagon<T> oc(2);
    oc.add_constraint(x >= 0);
    oc.add_constraint(x - y >= 3);
    oc.add_constraint(y >= 0);
    Linear_Expression expr(y - 1);

    oc.affine_preimage(x, expr);
  \endcode
  In this example the starting octagon is an half-plane in
  \f$\Rset^2\f$, the considered variable is \f$x\f$ and the affine
  expression is \f$y\f$. The resulting octagon is half-plane \f$y >= 0\f$.

  \par Example 4
  The following code shows the use of the function
  <CODE>BHMZ05_widening_assign</CODE>:
  \code
    Octagon<T> oc1(2);
    oc1.add_constraint(x >= 0);
    oc1.add_constraint(y >= 0);
    oc1.add_constraint(x + y >= 0);

    Octagon<T> oc2(2);
    oc2.add_constraint(x >= 5);
    oc2.add_constraint(y >= 0);
    oc2.add_constraint(x + y >= 0);

    oc1.BHMZ05_widening_assign(oc2);
  \endcode
  In this example the starting octagon oc1 is the first quadrant
  and oc2 is an half-plane in \f$\Rset^2\f$. The resulting octagon
  is the half-plane \f$y >= 0\f$.

  \par Example 5
  The following code shows the use of the function
  <CODE>CC76_extrapolation_assign</CODE>:
  \code
    Octagon<T> oc1(2);
    oc1.add_constraint(x >= 0);
    oc1.add_constraint(y >= 0);
    oc1.add_constraint(x + y >= 0);

    Octagon<T> oc2(2);
    oc2.add_constraint(x >= 5);
    oc2.add_constraint(y >= 0);
    oc2.add_constraint(x + y >= 0);

    oc1.CC76_extrapolation_assign(oc2);
  \endcode
  In this example the starting octagon oc1 is the first quadrant
  and oc2 is an half-plane in \f$\Rset^2\f$. The resulting octagon
  is still oc1.

  \par Example 6
  The following code shows the use of the function
  <CODE>CC76_narrowing_assign</CODE>:
  \code
    Octagon<T> oc1(2);
    Octagon<T> oc2(2);

    Constraint_System cs;
    cs.insert(x <= 0);
    oc2.add_constraints(cs);

    oc1.CC76_narrowing_assign(oc2);
  \endcode
  In this example the starting octagon oc1 is the universe one
  and oc2 is an half-plane in \f$\Rset^2\f$. The resulting octagon
  is the same oc2.

*/
template <typename T>
class Parma_Polyhedra_Library::Octagon {
private:
  /*! \brief
    The (extended) numeric type of the inhomogeneous term of
    the inequalities defining an OS.
  */
  typedef Checked_Number<T, Extended_Number_Policy> N;

public:
  //! The numeric base type upon which OSs are built.
  typedef T base_type;

  /*! \brief
    The (extended) numeric type of the inhomogeneous term of the
    inequalities defining an OS.
  */
  typedef N coefficient_type;

  //! Returns the maximum space dimension that an OS can handle.
  static dimension_type max_space_dimension();

  //! \name Constructors, Assignment, Swap and Destructor
  //@{

  //! Builds an universe or empty OS of the specified space dimension.
  /*!
    \param num_dimensions
    The number of dimensions of the vector space enclosing the OS;

    \param kind
    Specifies whether the universe or the empty OS has to be built.
  */
  explicit Octagon(dimension_type num_dimensions = 0,
		   Degenerate_Element kind = UNIVERSE);

  //! Ordinary copy-constructor.
  Octagon(const Octagon& x);

  //! Builds an OS from the system of constraints \p cs.
  /*!
    The OS inherits the space dimension of \p cs.

    \param cs
    A system of constraints: constraints that are not
    \ref Octagonal_Shapes "octagonal constraints"
    are ignored (even though they may have contributed
    to the space dimension).

    \exception std::invalid_argument
    Thrown if the system of constraints \p cs contains strict inequalities.
  */
  Octagon(const Constraint_System& cs);

  //! Builds an OS from the system of generators \p gs.
  /*!
    Builds the smallest OS containing the polyhedron defined by \p gs.
    The OS inherits the space dimension of \p gs.

    \exception std::invalid_argument
    Thrown if the system of generators is not empty but has no points.
  */
  Octagon(const Generator_System& gs);

  //! Builds an OS from the polyhedron \p ph.
  /*!
    Builds an OS containing \p ph using algorithms whose complexity
    does not exceed the one specified by \p complexity.  If
    \p complexity is \p ANY_COMPLEXITY, then the OS built is the
    smallest one containing \p ph.
  */
  // FIXME: this has to be implemented.
  Octagon(const Polyhedron& ph, Complexity_Class complexity = ANY_COMPLEXITY);

  /*! \brief
    The assignment operator.
    (\p *this and \p y can be dimension-incompatible.)
  */
  Octagon& operator=(const Octagon& y);

  /*! \brief
    Swaps \p *this with octagon \p y.
    (\p *this and \p y can be dimension-incompatible.)
  */
  void swap(Octagon& y);

  //! Destructor.
  ~Octagon();

  //@} Constructors, Assignment, Swap and Destructor

  //! \name Member Functions that Do Not Modify the Octagonal_Shape
  //@{

  //! Returns the dimension of the vector space enclosing \p *this.
  dimension_type space_dimension() const;

  /*! \brief
    Returns \f$0\f$, if \p *this is empty; otherwise, returns the
    \ref Affine_Independence_and_Affine_Dimension "affine dimension"
    of \p *this.
  */
  dimension_type affine_dimension() const;

  //! Returns the system of constraints defining \p *this.
  Constraint_System constraints() const;

  //! Returns a minimized system of constraints defining \p *this.
  Constraint_System minimized_constraints() const;

  //! Returns <CODE>true</CODE> if and only if \p *this contains \p y.
  /*!
    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  bool contains(const Octagon& y) const;

  //! Returns <CODE>true</CODE> if and only if \p *this strictly contains \p y.
  /*!
    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  bool strictly_contains(const Octagon& y) const;

  /*! \brief
    Returns the relations holding between \p *this and the constraint \p c.

    \exception std::invalid_argument
    Thrown if \p *this and constraint \p c are dimension-incompatible
    or if \p c is a strict inequality or if \p c is not an octagonal
    constraint.
  */
  Poly_Con_Relation relation_with(const Constraint& c) const;

  /*! \brief
    Returns the relations holding between \p *this and the generator \p g.

    \exception std::invalid_argument
    Thrown if \p *this and generator \p g are dimension-incompatible.
  */
  Poly_Gen_Relation relation_with(const Generator& g) const;

  //! Returns <CODE>true</CODE> if and only if \p *this is an empty OS.
  bool is_empty() const;

  //! Returns <CODE>true</CODE> if and only if \p *this is a universe OS.
  bool is_universe() const;

  //! Checks if all the invariants are satisfied.
  bool OK() const;

  //@} Member Functions that Do Not Modify the Octagonal_Shape

  //! \name Space-Dimension Preserving Member Functions that May Modify the Octagonal_Shape
  //@{

  /*! \brief
    Adds a copy of constraint \p c to the system of constraints
    defining \p *this.

    \param c
    The constraint to be added. If it is not an octagonal constraint, it
    will be simply ignored.

    \exception std::invalid_argument
    Thrown if \p *this and constraint \p c are dimension-incompatible,
    or \p c is a strict inequality.
  */
  void add_constraint(const Constraint& c);

  /*! \brief
    Adds a copy of constraint \p c to the system of constraints
    defining \p *this.

    \return
    <CODE>false</CODE> if and only if the result is empty.

    \param c
    The constraint to be added. If it is not an octagonal constraint, it
    will be simply ignored.

    \exception std::invalid_argument
    Thrown if \p *this and constraint \p c are dimension-incompatible
    or \p c is a strict inequality.
  */
  bool add_constraint_and_minimize(const Constraint& c);

  /*! \brief
    Adds the constraints in \p cs to the system of constraints
    defining \p *this.

    \param  cs
    The constraints that will be added. Constraints that are not octagonal
    constraints will be simply ignored.

    \exception std::invalid_argument
    Thrown if \p *this and \p cs are dimension-incompatible,
    or if \p cs contains a strict inequality.
  */
  void add_constraints(const Constraint_System& cs);

  /*! \brief
    Adds the constraints in \p cs to the system of constraints
    defining \p *this.

    \return
    <CODE>false</CODE> if and only if the result is empty.

    \param  cs
    The constraints that will be added. Constraints that are not octagonal
    constraints will be simply ignored.

    \exception std::invalid_argument
    Thrown if \p *this and \p cs are dimension-incompatible,
    or if there is in \p cs a strict inequality.
  */
  bool add_constraints_and_minimize(const Constraint_System& cs);

  //! Assigns to \p *this the intersection of \p *this and \p y.
  /*!
    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  void intersection_assign(const Octagon& y);

  //! \brief
  //! Assigns to \p *this the intersection of \p *this and \p y.
  /*!
    \return
    <CODE>false</CODE> if and only if the result is empty.

    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  bool intersection_assign_and_minimize(const Octagon& y);

  /*! \brief
    Assigns to \p *this the smallest OS that contains
    the convex union of \p *this and \p y.

    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  void oct_hull_assign(const Octagon& y);

  /*! \brief
    Assigns to \p *this the smallest OS that contains
    the convex union of \p *this and \p y.

    \return
    <CODE>false</CODE> if and only if the result is empty.

    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  bool oct_hull_assign_and_minimize(const Octagon& y);

  //! Same as oct_hull_assign.
  void upper_bound_assign(const Octagon& y);

  /*! \brief
    If the oct-hull of \p *this and \p y is exact, it is assigned
    to \p *this and <CODE>true</CODE> is returned,
    otherwise <CODE>false</CODE> is returned.

    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  bool oct_hull_assign_if_exact(const Octagon& y);

  //! Same as oct_hull_assign_if_exact.
  bool upper_bound_assign_if_exact(const Octagon& y);

  /*! \brief
    Assigns to \p *this the \ref oct_difference "oct-difference" of
    \p *this and \p y.

    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  void oct_difference_assign(const Octagon& y);

  //! Same as oct_difference_assign.
  void difference_assign(const Octagon& y);

  /*! \brief
    Assigns to \p *this the \ref affine_transformation "affine image"
    of \p *this under the function mapping variable \p var into the
    affine expression specified by \p expr and \p denominator.

    \param var
    The variable to which the affine expression is assigned.

    \param expr
    The numerator of the affine expression.

    \param denominator
    The denominator of the affine expression.

    \exception std::invalid_argument
    Thrown if \p denominator is zero or if \p expr and \p *this
    are dimension-incompatible or if \p var is not a dimension of \p *this.
  */
  void affine_image(Variable var,
		    const Linear_Expression& expr,
		    Coefficient_traits::const_reference  denominator
		    = Coefficient_one());

  /*! \brief
    Assigns to \p *this the \ref affine_transformation "affine preimage"
    of \p *this under the function mapping variable \p var into the
    affine expression specified by \p expr and \p denominator.

    \param var
    The variable to which the affine expression is substituted.

    \param expr
    The numerator of the affine expression.

    \param denominator
    The denominator of the affine expression.

    \exception std::invalid_argument
    Thrown if \p denominator is zero or if \p expr and \p *this
    are dimension-incompatible or if \p var is not a dimension of \p *this.
  */
  void affine_preimage(Variable var,
		       const Linear_Expression& expr,
		       Coefficient_traits::const_reference denominator
		       = Coefficient_one());

  /*! \brief
    Assigns to \p *this the image of \p *this with respect to the
    \ref generalized_image "generalized affine transfer function"
    \f$\mathrm{var}' \relsym \frac{\mathrm{expr}}{\mathrm{denominator}}\f$,
    where \f$\mathord{\relsym}\f$ is the relation symbol encoded
    by \p relsym.

    \param var
    The left hand side variable of the generalized affine transfer function.

    \param relsym
    The relation symbol.

    \param expr
    The numerator of the right hand side affine expression.

    \param denominator
    The denominator of the right hand side affine expression.

    \exception std::invalid_argument
    Thrown if \p denominator is zero or if \p expr and \p *this
    are dimension-incompatible or if \p var is not a dimension of \p *this
    or if \p relsym is a strict relation symbol.
  */
  void generalized_affine_image(Variable var,
				Relation_Symbol relsym,
				const Linear_Expression& expr,
				Coefficient_traits::const_reference denominator
				= Coefficient_one());

  /*! \brief
    Assigns to \p *this the image of \p *this with respect to the
    \ref generalized_image "generalized affine transfer function"
    \f$\mathrm{lhs}' \relsym \mathrm{rhs}\f$, where
    \f$\mathord{\relsym}\f$ is the relation symbol encoded by \p relsym.

    \param lhs
    The left hand side affine expression.

    \param relsym
    The relation symbol.

    \param rhs
    The right hand side affine expression.

    \exception std::invalid_argument
    Thrown if \p *this is dimension-incompatible with \p lhs or \p rhs
    or if \p relsym is a strict relation symbol.
  */
  void generalized_affine_image(const Linear_Expression& lhs,
				Relation_Symbol relsym,
				const Linear_Expression& rhs);
  /*! \brief
    Assigns to \p *this the preimage of \p *this with respect to the
    \ref Generalized_Affine_Relations "affine relation"
    \f$\mathrm{var}' \relsym \frac{\mathrm{expr}}{\mathrm{denominator}}\f$,
    where \f$\mathord{\relsym}\f$ is the relation symbol encoded
    by \p relsym.

    \param var
    The left hand side variable of the generalized affine transfer function.

    \param relsym
    The relation symbol.

    \param expr
    The numerator of the right hand side affine expression.

    \param denominator
    The denominator of the right hand side affine expression.

    \exception std::invalid_argument
    Thrown if \p denominator is zero or if \p expr and \p *this
    are dimension-incompatible or if \p var is not a dimension
    of \p *this or if \p relsym is a strict relation symbol.
  */
  void generalized_affine_preimage(Variable var,
				   Relation_Symbol relsym,
				   const Linear_Expression& expr,
				   Coefficient_traits::const_reference
				   denominator = Coefficient_one());

  /*! \brief
    Assigns to \p *this the result of computing the
    \ref time_elapse "time-elapse" between \p *this and \p y.

    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  void time_elapse_assign(const Octagon& y);

  /*! \brief
    Assigns to \p *this the result of computing the
    \ref CC76_extrapolation "CC76-extrapolation" between \p *this and \p y.

    \param y
    An OS that <EM>must</EM> be contained in \p *this.

    \param tp
    An optional pointer to an unsigned variable storing the number of
    available tokens (to be used when applying the
    \ref Widening_with_Tokens "widening with tokens" delay technique).

    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  void CC76_extrapolation_assign(const Octagon& y, unsigned* tp = 0);

  /*! \brief
    Assigns to \p *this the result of computing the
    \ref CC76_extrapolation "CC76-extrapolation" between \p *this and \p y.

    \param y
    An OS that <EM>must</EM> be contained in \p *this.

    \param first
    An iterator that points to the first stop_point.

    \param last
    An iterator that points to the last stop_point.

    \param tp
    An optional pointer to an unsigned variable storing the number of
    available tokens (to be used when applying the
    \ref Widening_with_Tokens "widening with tokens" delay technique).

    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  template <typename Iterator>
  void CC76_extrapolation_assign(const Octagon& y,
				 Iterator first, Iterator last,
				 unsigned* tp = 0);

  /*! \brief
    Assigns to \p *this the result of computing the
    \ref BHMZ05_widening "BHMZ05-widening" between \p *this and \p y.

    \param y
    An OS that <EM>must</EM> be contained in \p *this.

    \param tp
    An optional pointer to an unsigned variable storing the number of
    available tokens (to be used when applying the
    \ref widening_with_tokens "widening with tokens" delay technique).

    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  void BHMZ05_widening_assign(const Octagon& y, unsigned* tp = 0);

  /*! \brief
    Improves the result of the \ref BHMZ05_widening "BHMZ05-widening"
    computation by also enforcing those constraints in \p cs that are
    satisfied by all the points of \p *this.

    \param y
    An OS that <EM>must</EM> be contained in \p *this.

    \param cs
    The system of constraints used to improve the widened OS.

    \param tp
    An optional pointer to an unsigned variable storing the number of
    available tokens (to be used when applying the
    \ref widening_with_tokens "widening with tokens" delay technique).

    \exception std::invalid_argument
    Thrown if \p *this, \p y and \p cs are dimension-incompatible or
    if there is in \p cs a strict inequality.
  */
  void limited_BHMZ05_extrapolation_assign(const Octagon& y,
					   const Constraint_System& cs,
					   unsigned* tp = 0);

  /*! \brief
    Restores from \p y the constraints of \p *this, lost by
    \ref CC76_extrapolation "CC76-extrapolation" applications.

    \param y
    An OS that <EM>must</EM> be contained in \p *this.

    \exception std::invalid_argument
    Thrown if \p *this and \p y are dimension-incompatible.
  */
  void CC76_narrowing_assign(const Octagon& y);

  /*! \brief
    Improves the result of the \ref CC76_extrapolation "CC76-extrapolation"
    computation by also enforcing those constraints in \p cs that are
    satisfied by all the points of \p *this.

    \param y
    An OS that <EM>must</EM> be contained in \p *this.

    \param cs
    The system of constraints used to improve the widened OS.

    \param tp
    An optional pointer to an unsigned variable storing the number of
    available tokens (to be used when applying the
    \ref widening_with_tokens "widening with tokens" delay technique).

    \exception std::invalid_argument
    Thrown if \p *this, \p y and \p cs are dimension-incompatible or
    if \p cs contains a strict inequality.
  */
  void limited_CC76_extrapolation_assign(const Octagon& y,
					 const Constraint_System& cs,
					 unsigned* tp = 0);

  //@} Space-Dimension Preserving Member Functions that May Modify [...]

  //! \name Member Functions that May Modify the Dimension of the Vector Space
  //@{

  //! Adds \p m new dimensions and embeds the old OS into the new space.
  /*!
    \param m
    The number of dimensions to add.

    The new dimensions will be those having the highest indexes in the new OS,
    which is characterized by a system of constraints in which the variables
    running through the new dimensions are not constrained.
    For instance, when starting from the OS \f$\cO \sseq \Rset^2\f$
    and adding a third dimension, the result will be the OS
    \f[
      \bigl\{\,
        (x, y, z)^\transpose \in \Rset^3
      \bigm|
        (x, y)^\transpose \in \cO
      \,\bigr\}.
    \f]
  */
  void add_space_dimensions_and_embed(dimension_type m);

  /*! \brief
    Adds \p m new dimensions to the OS
    and does not embed it in the new space.

    \param m
    The number of dimensions to add.

    The new dimensions will be those having the highest indexes
    in the new OS, which is characterized by a system
    of constraints in which the variables running through
    the new dimensions are all constrained to be equal to 0.
    For instance, when starting from the OS \f$\cO \sseq \Rset^2\f$
    and adding a third dimension, the result will be the OS
    \f[
      \bigl\{\,
        (x, y, 0)^\transpose \in \Rset^3
      \bigm|
        (x, y)^\transpose \in \cO
      \,\bigr\}.
    \f]
  */
  void add_space_dimensions_and_project(dimension_type m);

  /*! \brief
    Seeing an OS as a set of tuples (its points), assigns
    to \p *this all the tuples that can be obtained by concatenating,
    in the order given, a tuple of \p *this with a tuple of \p y.

    Let \f$O \sseq \Rset^n\f$ and \f$P \sseq \Rset^m\f$ be the OSs
    represented, on entry, by \p *this and \p y, respectively.
    Upon successful completion, \p *this will represent the OS
    \f$R \sseq \Rset^{n+m}\f$ such that
    \f[
      R
        \defeq
          \Bigl\{\,
            (x_1, \ldots, x_n, y_1, \ldots, y_m)^\transpose
          \Bigm|
            (x_1, \ldots, x_n)^\transpose \in O,
            (y_1, \ldots, y_m)^\transpose \in P
          \,\Bigl\}.
    \f]
    Another way of seeing it is as follows: first increases the space
    dimension of \p *this by adding \p y.space_dimension() new
    dimensions; then adds to the system of constraints of \p *this a
    renamed-apart version of the constraints of \p y.
  */
  void concatenate_assign(const Octagon& y);

  //! Removes all the specified dimensions.
  /*!
    \param to_be_removed
    The set of Variable objects corresponding to the dimensions to be removed.

    \exception std::invalid_argument
    Thrown if \p *this is dimension-incompatible with one of the Variable
    objects contained in \p to_be_removed.
  */
  void remove_space_dimensions(const Variables_Set& to_be_removed);

  /*! \brief
    Removes the higher dimensions so that the resulting space
    will have dimension \p new_dimension.

    \exception std::invalid_argument
    Thrown if \p new_dimension is greater than the space dimension
    of \p *this.
  */
  void remove_higher_space_dimensions(dimension_type new_dimension);

  /*! \brief
    Remaps the dimensions of the vector space
    according to a \ref map_space_dimensions "partial function".

    \param pfunc
    The partial function specifying the destiny of each dimension.

    The template class PartialFunction must provide the following
    methods.
    \code
      bool has_empty_codomain() const
    \endcode
    returns <CODE>true</CODE> if and only if the represented partial
    function has an empty codomain (i.e., it is always undefined).
    The <CODE>has_empty_codomain()</CODE> method will always be called
    before the methods below.  However, if
    <CODE>has_empty_codomain()</CODE> returns <CODE>true</CODE>, none
    of the functions below will be called.
    \code
      dimension_type max_in_codomain() const
    \endcode
    returns the maximum value that belongs to the codomain
    of the partial function.
    \code
      bool maps(dimension_type i, dimension_type& j) const
    \endcode
    Let \f$f\f$ be the represented function and \f$k\f$ be the value
    of \p i.  If \f$f\f$ is defined in \f$k\f$, then \f$f(k)\f$ is
    assigned to \p j and <CODE>true</CODE> is returned.
    If \f$f\f$ is undefined in \f$k\f$, then <CODE>false</CODE> is
    returned.

    The result is undefined if \p pfunc does not encode a partial
    function with the properties described in the
    \ref map_space_dimensions "specification of the mapping operator".
  */
  template <typename PartialFunction>
  void map_space_dimensions(const PartialFunction& pfunc);

  //@} // Member Functions that May Modify the Dimension of the Vector Space

  PPL_OUTPUT_DECLARATIONS

#ifdef PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
  /*! \brief
    Loads from \p s an ASCII representation (as produced by \ref ascii_dump)
    and sets \p *this accordingly.  Returns <CODE>true</CODE> if successful,
    <CODE>false</CODE> otherwise.
  */
#endif // PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
  bool ascii_load(std::istream& s);

  friend bool Parma_Polyhedra_Library::operator==<T>(const Octagon<T>& x,
						     const Octagon<T>& y);

private:
  //! The matrix that represents the octagonal shape.
  OR_Matrix<N> matrix;

  //! Dimension of the space of the octagonal shape.
  dimension_type space_dim;

  // Please, do not move the following include directive:
  // `Og_Status.idefs.hh' must be included exactly at this point.
  // And please do not remove the space separating `#' from `include':
  // this ensures that the directive will not be moved during the
  // procedure that automatically creates the library's include file
  // (see `Makefile.am' in the `src' directory).
#define PPL_IN_Octagon_CLASS
#include "Og_Status.idefs.hh"
#undef PPL_IN_Octagon_CLASS

  //! The status flags to keep track of the internal state.
  Status status;

  //! Returns <CODE>true</CODE> if the OS is known to be empty.
  /*!
    The return value <CODE>false</CODE> does not necessarily
    implies that \p *this is non-empty.
  */
  bool marked_empty() const;

  /*! \brief
    Returns <CODE>true</CODE> if \c this->matrix is known to be
    strongly closed.

    The return value <CODE>false</CODE> does not necessarily
    implies that \c this->matrix is not strongly closed.
  */
  bool marked_strongly_closed() const;

  //! Turns \p *this into a zero-dimensional universe OS.
  void set_zero_dim_univ();

  //! Turns \p *this into an empty OS.
  void set_empty();

  //! Adds the constraint <CODE>i[j] <= k/den</CODE>.
  void add_octagonal_constraint(typename OR_Matrix<N>::row_iterator i,
				const dimension_type j,
				N k);

  //! Adds the constraint <CODE>i[j] <= num/den</CODE>.
  void add_octagonal_constraint(typename OR_Matrix<N>::row_iterator i,
				const dimension_type j,
				Coefficient_traits::const_reference num,
				Coefficient_traits::const_reference den);

  //! Removes all the constraints on row/column \p v.
  void forget_all_octagonal_constraints(typename OR_Matrix<N>::row_iterator i,
					dimension_type v);

  //! Removes all binary constraints on row/column \p v.
  void forget_binary_octagonal_constraints(dimension_type v);

  //! An helper function for the computation of affine relations.
  /*!
    For each octagon index \p u (less than or equal to \p last_v and different
    from \p v), deduce constraints of the form <CODE>v - u \<= c</CODE>,
    starting from \p pos_sum which is an upper bound for \p v.

    The strong closure is able to deduce the constraint
    <CODE>v - u \<= ub_v - lb_u</CODE>. We can be more precise if variable
    \p u played an active role in the computation of the upper bound for
    \p v, i.e., if the corresponding coefficient
    <CODE>q == sc_expr[u]/sc_den</CODE> is greater than zero. In particular:
      - if <CODE>q \>= 1</CODE>, then <CODE>v - u \<= ub_v - ub_u</CODE>;
      - if <CODE>0 \< q \< 1</CODE>, then
        <CODE>v - u \<= ub_v - (q*ub_u + (1-q)*lb_u)</CODE>.
  */
  void deduce_v_minus_u_bounds(dimension_type v,
			       dimension_type last_v,
			       const Linear_Expression& sc_expr,
			       Coefficient_traits::const_reference sc_den,
			       const N& pos_sum);

  //! An helper function for the computation of affine relations.
  /*!
    For each octagon index \p u (less than or equal to \p last_v and different
    from \p v), deduce constraints of the form <CODE>v + u \<= c</CODE>,
    starting from \p pos_sum which is an upper bound for \p v.

    The strong closure is able to deduce the constraint
    <CODE>v + u \<= ub_v + ub_u</CODE>. We can be more precise if variable
    \p u played an active role in the computation of the lower bound for
    \p v, i.e., if the corresponding coefficient
    <CODE>q == sc_expr[u]/sc_den</CODE> is less than zero. In particular:
      - if <CODE>q \<= -1</CODE>, then <CODE>v + u \<= ub_v + lb_u</CODE>;
      - if <CODE>-1 \< q \< 0</CODE>, then
        <CODE>v + u \<= ub_v + (q*lb_u - (1-q)*ub_u)</CODE>.
  */
  void deduce_v_plus_u_bounds(dimension_type v,
			      dimension_type last_v,
			      const Linear_Expression& sc_expr,
			      Coefficient_traits::const_reference sc_den,
			      const N& neg_sum);

  //! An helper function for the computation of affine relations.
  /*!
    For each octagon index \p u (less than or equal to \p last_v and different
    from \p v), deduce constraints of the form <CODE>u - v \<= c</CODE>,
    starting from \p neg_sum which is a lower bound for \p v.

    The strong closure is able to deduce the constraint
    <CODE>u - v \<= ub_u - lb_v</CODE>. We can be more precise if variable
    \p u played an active role in the computation of the lower bound for
    \p v, i.e., if the corresponding coefficient
    <CODE>q == sc_expr[u]/sc_den</CODE> is greater than zero.
    In particular:
      - if <CODE>q \>= 1</CODE>, then <CODE>u - v \<= lb_u - lb_v</CODE>;
      - if <CODE>0 \< q \< 1</CODE>, then
        <CODE>u - v \<= (q*lb_u + (1-q)*ub_u) - lb_v</CODE>.
  */
  void deduce_u_minus_v_bounds(dimension_type v,
			       dimension_type last_v,
			       const Linear_Expression& sc_expr,
			       Coefficient_traits::const_reference sc_den,
			       const N& neg_sum);

  //! An helper function for the computation of affine relations.
  /*!
    For each octagon index \p u (less than or equal to \p last_v and different
    from \p v), deduce constraints of the form <CODE>-v - u \<= c</CODE>,
    starting from \p neg_sum which is a lower bound for \p v.

    The strong closure is able to deduce the constraint
    <CODE>-v - u \<= -lb_v - lb_u</CODE>. We can be more precise if variable
    \p u played an active role in the computation of the lower bound for
    \p v, i.e., if the corresponding coefficient
    <CODE>q == sc_expr[u]/sc_den</CODE> is less than zero.
    In particular:
      - if <CODE>q \<= -1</CODE>, then <CODE>-v - u \<= -lb_v - ub_u</CODE>;
      - if <CODE>-1 \< q \< 0</CODE>, then
        <CODE>-v - u \<= -lb_v - (q*ub_u + (1-q)*lb_u)</CODE>.
  */
  void deduce_minus_v_minus_u_bounds(dimension_type v,
				     dimension_type last_v,
				     const Linear_Expression& sc_expr,
				     Coefficient_traits::const_reference sc_den,
				     const N& neg_sum);

  /*! \brief
    Adds to \p limiting_octagon the octagonal differences in \p cs
    that are satisfied by \p *this.
  */
  void get_limiting_octagon(const Constraint_System& cs,
			    Octagon& limiting_octagon) const;
  //! Compute the (zero-equivalence classes) successor relation.
  /*!
    It is assumed that the octagon is not empty and strongly closed.
  */
  void compute_successors(std::vector<dimension_type>& successor) const;

  //! Compute the leaders of zero-equivalence classes.
  /*!
    It is assumed that the Octagon is not empty and strongly closed.
  */
  void compute_leaders(std::vector<dimension_type>& successor,
		       std::vector<dimension_type>& no_sing_leaders,
		       bool& exist_sing_class,
		       dimension_type& sing_leader) const;

  //! Compute the leaders of zero-equivalence classes.
  /*!
    It is assumed that the Octagon is not empty and strongly closed.
  */
  void compute_leaders(std::vector<dimension_type>& leaders) const;

  //! Removes the redundant constraints from \c this->matrix.
  void strong_reduction_assign() const;

  /*! \brief
    Returns <CODE>true</CODE> if and only if \c this->matrix
    is strongly reduced.
  */
  bool is_strongly_reduced() const;

  //! \brief
  //! Returns <CODE>true</CODE> if in the octagon taken two at a time
  //! unary constraints, there is also the constraint that represent their sum.
  bool is_strong_coherent() const;

  //! Assigns to \c this->matrix its strong closure.
  /*!
    Strong closure is a necessary condition for the precision and/or
    the correctness of many methods. It explicity records into \c matrix
    those constraints that are implicitly obtainable by the other ones,
    therefore obtaining a canonical representation for the OS.
  */
  void strong_closure_assign() const;

  //! Puts in \p *this all implicit constraints and computes the tighter ones.
  void strong_closure_assign1() const;

  //! Puts in \p *this all implicit constraints and computes the tighter ones.
  void strong_closure_assign3() const;

  //! Puts in \p *this all implicit constraints and computes the tighter ones.
  void strong_closure_assign5() const;

  //! Puts in \p *this all implicit constraints and computes the tighter ones.
  /*!
    \param var
    The variable of the altered constraints.

    The octagon `*this' was transitively closed except for the constraint on
    variable `var'. This operation costs only \f$O(n^2)\f$.

  */
  void incremental_strong_closure_assign(Variable var) const;

  void incremental_strong_closure_assign1(Variable var) const;

  void incremental_strong_closure_assign_of_mine(Variable var) const;

#if !defined(__GNUC__) || __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3)
  friend std::ostream&
  Parma_Polyhedra_Library::IO_Operators::operator<<<>(std::ostream& s,
						      const Octagon<T>& c);
#else
  // This is too lax than wanted.
  template <typename S>
  friend std::ostream&
  Parma_Polyhedra_Library::IO_Operators::operator<<(std::ostream& s,
						    const Octagon<S>& c);
#endif

  //! \name Exception Throwers
  //@{
  void throw_dimension_incompatible(const char* method,
				    const Octagon& x) const;

  void throw_dimension_incompatible(const char* method,
				    dimension_type required_dim) const;

  void throw_dimension_incompatible(const char* method,
				    const Constraint& c) const;

  void throw_dimension_incompatible(const char* method,
				    const Generator& g) const;

  void throw_dimension_incompatible(const char* method,
				    const char* name_row,
				    const Linear_Expression& y) const;

  void throw_constraint_incompatible(const char* method) const;

  void throw_expression_too_complex(const char* method,
				    const Linear_Expression& e) const;

  void throw_generic(const char* method, const char* reason) const;
  //@} // Exception Throwers

  static T default_stop_points[];
};

namespace std {

//! Specializes <CODE>std::swap</CODE>.
/*! \relates Parma_Polyhedra_Library::Octagon */
template <typename T>
void swap(Parma_Polyhedra_Library::Octagon<T>& x,
	  Parma_Polyhedra_Library::Octagon<T>& y);

} // namespace std

#include "Og_Status.inlines.hh"
#include "Octagon.inlines.hh"
#include "Octagon.templates.hh"

#endif // !defined(PPL_Octagon_defs_hh)
