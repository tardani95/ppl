/* PIP_Tree class declaration.
   Copyright (C) 2001-2009 Roberto Bagnara <bagnara@cs.unipr.it>

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

#ifndef PPL_PIP_Tree_defs_hh
#define PPL_PIP_Tree_defs_hh 1

#include "PIP_Tree.types.hh"
#include "Variable.defs.hh"
#include "Linear_Expression.types.hh"
#include "Constraint_System.defs.hh"
#include "Constraint_System.inlines.hh"
#include "Constraint.defs.hh"
#include "PIP_Problem.types.hh"
#include "Matrix.defs.hh"
#include "Variables_Set.defs.hh"
#include "globals.defs.hh"

namespace Parma_Polyhedra_Library {

/*! \brief
  The base class for the nodes of the trees representing the solutions
  of PIP problems.
*/
class PIP_Tree_Node {
public:
  //! Returns \p this if \p *this is a solution node, 0 otherwise.
  virtual const PIP_Solution_Node* as_solution() const;

  //! Returns \p this if \p *this is a solution node, 0 otherwise.
  virtual PIP_Solution_Node* as_solution();

  //! Returns \p this if \p *this is a decision node, 0 otherwise.
  virtual const PIP_Decision_Node* as_decision() const;

  //! Returns \p this if \p *this is a decision node, 0 otherwise.
  virtual PIP_Decision_Node* as_decision();

  //! Destructor.
  virtual ~PIP_Tree_Node();

  //! Returns \c true if and only if \p *this is well formed.
  bool OK() const;

  /*! \brief
    Returns the system of parameter constraints controlling \p *this.

    The column indices in the constraints are numbered from \c 0 to
    <tt>np-1</tt>, where \c np is the total number of parameters. They are
    ordered with the same order as the parameter indices in the original
    problem.
  */
  const Constraint_System& constraints() const;

protected:
  //! Default constructor.
  PIP_Tree_Node();

  //! Copy constructor.
  PIP_Tree_Node(const PIP_Tree_Node &x);

  //! A type alias for a sequence of constraints.
  typedef std::vector<Constraint> Constraint_Sequence;

  // Only PIP_Problem and PIP_Decision_Node are allowed to use the
  // constructor and methods.
  friend class PIP_Problem;
  friend class PIP_Decision_Node;
  friend class PIP_Solution_Node;

  //! The local system of parameter constraints.
  Constraint_System constraints_;

  /*! \brief
    Populates the parametric simplex tableau using external data, if necessary

    \param parent_ref
    a pointer to the parent reference to \p this

    \param external_space_dim
    size of constraints (variables and parameters added)

    \param first_pending_constraint
    first element in \p input_cs to be added to the tableau, which already
    contains the previous elements

    \param input_cs
    all the constraints of the problem

    \param parameters
    a \c std::set of indices of the parameters in the constraints
  */
  virtual void update_tableau(PIP_Tree_Node **parent_ref,
                              dimension_type external_space_dim,
                              dimension_type first_pending_constraint,
                              const Constraint_Sequence &input_cs,
                              const Variables_Set &parameters) = 0;

  /*! \brief
    Execute a parametric simplex on the tableau, under specified context.

    \param parent_ref
    a pointer to the parent reference to \p this

    \param context
    the context, being a set of constraints on the parameters

    \return
    An PIP_Problem_Status flag indicating the outcome of the optimization
    attempt (unfeasible or optimized problem).
  */
  virtual PIP_Problem_Status solve(PIP_Tree_Node **parent_ref,
                                   const Matrix& context) = 0;
};

//! A tree node representing part of the space of solutions.
class PIP_Solution_Node : public PIP_Tree_Node {
public:
  //! Default constructor.
  PIP_Solution_Node();

  //! Destructor.
  ~PIP_Solution_Node();

  //! Returns \p this.
  virtual const PIP_Solution_Node* as_solution() const;

  //! Returns \p this.
  virtual PIP_Solution_Node* as_solution();

  /*! \brief
    Returns a parametric expression of the values of variable \p v.

    The returned linear expression only involves parameters.

    \exception std::invalid_argument
    Thrown if \p v is dimension-incompatible with \p *this
    or if \p v is a parameter.
  */
  const Linear_Expression& parametric_values(Variable v);

  void ascii_dump(std::ostream& s) const;
  bool ascii_load(std::istream& s);

  bool OK() const;

private:
  //! A rational matrix, with a common nenominator
  class Rational_Matrix : public Matrix {
  public:
    //! Builds an empty matrix.
    /*!
      Rows' size and capacity are initialized to \f$0\f$.
    */
    Rational_Matrix();

    //! Builds a zero matrix with specified dimensions and flags.
    /*!
      \param n_rows
      The number of rows of the matrix that will be created;

      \param n_columns
      The number of columns of the matrix that will be created.

      \param row_flags
      The flags used to build the rows of the matrix;
      by default, the rows will have all flags unset.
    */
    Rational_Matrix(dimension_type n_rows, dimension_type n_columns,
	            Row::Flags row_flags = Row::Flags());

    //! Copy constructor.
    Rational_Matrix(const Rational_Matrix& y);

    //! Multiplies all coefficients and denominator with ratio.
    void scale(const Coefficient &ratio);

    //! Normalizes the modulo of coefficients so that they are mutually prime.
    /*!
      Computes the Greatest Common Divisor (GCD) among the elements of
      the matrix and normalizes them and the denominator by the GCD itself.
    */
    void normalize();

    //! Returns the allocated capacity of each Row of the Matrix.
    dimension_type capacity() const;

    //! Tests whether the matrix is integer, \e ie. the denominator is 1.
    bool is_integer() const;

    //! Returns the value of the denominator.
    const Coefficient &get_denominator() const;

    void ascii_dump(std::ostream& s) const;
    bool ascii_load(std::istream& s);

  protected:
    Coefficient denominator;
  };

  //! The type for parametric simplex tableau.
  struct Tableau {
    //! The matrix of simplex coefficients.
    Rational_Matrix s;
    //! The matrix of parameter coefficients.
    Rational_Matrix t;
    //! Returns \c true if and only if \p *this is well formed.
    bool OK() const;
  };

  //! The parametric simplex tableau.
  Tableau tableau;

  /*! \brief
    A boolean vector for identifying the basic variables.

    The value for <tt>basis[i]</tt> is:
     - \b true if variable \p i is basic,
     - \b false if variable \p i is nonbasic.
  */
  std::vector<bool> basis;

  /*! \brief
    A mapping between the tableau rows/columns and the original variables.

    The value of <tt>mapping[i]</tt> depends of the value of <tt>basis[i]</tt>.

     - If <tt>basis[i]</tt> is \b true, <tt>mapping[i]</tt> encodes the column
       index of variable \p i in the \p s matrix of the tableau.
     - If <tt>basis[i]</tt> is \b false, <tt>mapping[i]</tt> encodes the row
       index of variable \p i in the tableau.
  */
  std::vector<dimension_type> mapping;

  //! The possible values for the sign of a parametric linear expression.
  enum Row_Sign {
    //! Not computed yet (default)
    UNKNOWN,
    //! All row coefficients are zero.
    ZERO,
    //! All nonzero row coefficients are positive.
    POSITIVE,
    //! All nonzero row coefficients are negative.
    NEGATIVE,
    //! The row contains positive and negative coefficients.
    MIXED
  };

  //! A cache for computed sign values of constraint parametric RHS.
  std::vector<Row_Sign> sign;

  //! Determines the sign of given Row.
  static Row_Sign row_sign(const Row &x);

  /*! \brief
    Checks whether a constraint is compatible with a context, ie. does not
    make the context empty.

    The algorithm consists in performing simplex pivots on a Matrix consisting
    in the original matrix with the constraint inserted. If the simplex
    terminates with a solution, then the restrained context is not empty.
    Otherwise, it is.
  */
  static bool compatibility_check(const Matrix &ctx, const Row &cnst);

protected:
  //! Copy constructor.
  PIP_Solution_Node(const PIP_Solution_Node &x);

  //! Copy constructor, allowing not to copy the constraint system.
  PIP_Solution_Node(const PIP_Solution_Node &x, bool empty_constraints);

  /*! \brief
    Populates the parametric simplex tableau using external data, if necessary

    \param parent_ref
    a pointer to the parent reference to \p this

    \param external_space_dim
    size of constraints (variables and parameters added)

    \param first_pending_constraint
    first element in \p input_cs to be added to the tableau, which already
    contains the previous elements

    \param input_cs
    all the constraints of the problem

    \param parameters
    a \c std::set of indices of the parameters in the constraints
  */
  virtual void update_tableau(PIP_Tree_Node **parent_ref,
                              dimension_type external_space_dim,
                              dimension_type first_pending_constraint,
                              const Constraint_Sequence &input_cs,
                              const Variables_Set &parameters);

  /*! \brief
    Execute a parametric simplex on the tableau, under specified context.

    \param parent_ref
    a pointer to the parent reference to \p this

    \param context
    the context, being a set of constraints on the parameters

    \return
    An PIP_Problem_Status flag indicating the outcome of the optimization
    attempt (unfeasible or optimized problem).
  */
  virtual PIP_Problem_Status solve(PIP_Tree_Node **parent_ref,
                                   const Matrix& context);
  // FIXME: constructors to be decided.
};

//! A tree node representing a decision in the space of solutions.
class PIP_Decision_Node : public PIP_Tree_Node {
public:
  //! Destructor.
  ~PIP_Decision_Node();

  //! Returns \p this.
  const PIP_Decision_Node* as_decision() const;

  //! Returns \p this.
  PIP_Decision_Node* as_decision();

  //! Returns a const pointer to the \p b (true or false) branch of \p *this.
  const PIP_Tree_Node* child_node(bool b) const;

  //! Returns a pointer to the \p v (true or false) branch of \p *this.
  PIP_Tree_Node* child_node(bool v);

  bool OK() const;

private:
  // only PIP_Solution_Node is allowed to use the constructor and methods.
  friend class PIP_Solution_Node;

  //! Pointer to the "true" child of \p *this.
  PIP_Tree_Node* true_child;

  //! Pointer to the "false" child of \p *this.
  PIP_Tree_Node* false_child;

  /*! \brief
    Constructs "if constraints then \p tcp else \p fcp". Initial constraint
    set is empty.

    Constructs a decision node,
    with "true" child \p tcp and "false" child \p fcp.

    \param fcp
    Pointer to "false" child

    \param tcp
    Pointer to "true" child

    \exception std::invalid_argument
    Thrown if \p cs contains strict inequalities.
  */
  PIP_Decision_Node(PIP_Tree_Node* fcp, PIP_Tree_Node* tcp);

protected:
  /*! \brief
    Populates the parametric simplex tableau using external data, if necessary

    \param parent_ref
    a pointer to the parent reference to \p this

    \param external_space_dim
    size of constraints (variables and parameters added)

    \param first_pending_constraint
    first element in \p input_cs to be added to the tableau, which already
    contains the previous elements

    \param input_cs
    all the constraints of the problem

    \param parameters
    a \c std::set of indices of the parameters in the constraints
  */
  virtual void update_tableau(PIP_Tree_Node **parent_ref,
                              dimension_type external_space_dim,
                              dimension_type first_pending_constraint,
                              const Constraint_Sequence &input_cs,
                              const Variables_Set &parameters);

  /*! \brief
    Execute a parametric simplex on the tableau, under specified context.

    \param parent_ref
    a pointer to the parent reference to \p this

    \param context
    the context, being a set of constraints on the parameters

    \return
    An PIP_Problem_Status flag indicating the outcome of the optimization
    attempt (unfeasible or optimized problem).
  */
  virtual PIP_Problem_Status solve(PIP_Tree_Node **parent_ref,
                                   const Matrix& context);
};

typedef const PIP_Tree_Node* PIP_Tree;

} // namespace Parma_Polyhedra_Library

#include "PIP_Tree.inlines.hh"

#endif // !defined(PPL_PIP_Tree_defs_hh)
