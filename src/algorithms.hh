/* A collection of useful convex polyhedra algorithms: inline functions.
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
site: http://www.cs.unipr.it/ppl/ . */

#include "NNC_Polyhedron.defs.hh"
#include "Polyhedra_PowerSet.defs.hh"
#include "Constraint.defs.hh"
#include "LinExpression.defs.hh"
#include "ConSys.defs.hh"
#include <utility>
#include <cassert>

namespace Parma_Polyhedra_Library {

//! Partitions \p q with respect to \p p.
/*!
  Let \p p and \p q be two polyhedra.
  The function returns an object <CODE>r</CODE> of type
  <CODE>std::pair\<PH, Polyhedra_PowerSet\<NNC_Polyhedron\> \></CODE>
  such that
  - <CODE>r.first</CODE> is the intersection of \p p and \p q;
  - <CODE>r.second</CODE> has the property that all its elements are
    not empty, pairwise disjoint, and disjoint from \p p;
  - the union of <CODE>r.first</CODE> with all the elements of
    <CODE>r.second</CODE> gives \p q (i.e., <CODE>r</CODE> is the
    representation of a partition of \p q).

  \if Include_Implementation_Details

  See
  <A HREF="http://www.cs.unipr.it/ppl/Documentation/bibliography#Srivastava93">
  this paper</A> for more information about the implementation.
  \endif
*/
template <typename PH>
std::pair<PH, Polyhedra_PowerSet<NNC_Polyhedron> >
linear_partition(const PH& p, const PH& q);

namespace {

template <typename PH>
void
linear_partition_aux(const Constraint& c,
		     PH& qq,
		     Polyhedra_PowerSet<NNC_Polyhedron>& r) {
  LinExpression le(c);
  Constraint neg_c = c.is_strict_inequality() ? (le <= 0) : (le < 0);
  NNC_Polyhedron qqq(qq);
  if (qqq.add_constraint_and_minimize(neg_c))
    r.add_disjunct(qqq);
  qq.add_constraint(c);
}

} // namespace

template <typename PH>
std::pair<PH, Polyhedra_PowerSet<NNC_Polyhedron> >
linear_partition(const PH& p, const PH& q) {
  Polyhedra_PowerSet<NNC_Polyhedron> r(p.space_dimension(),
				       Polyhedron::EMPTY);
  PH qq = q;
  const ConSys& pcs = p.constraints();
  for (ConSys::const_iterator i = pcs.begin(),
	 pcs_end = pcs.end(); i != pcs_end; ++i) {
    const Constraint c = *i;
    if (c.is_equality()) {
      LinExpression le(c);
      linear_partition_aux(le <= 0, qq, r);
      linear_partition_aux(le >= 0, qq, r);
    }
    else
      linear_partition_aux(c, qq, r);
  }
  return std::pair<PH, Polyhedra_PowerSet<NNC_Polyhedron> >(qq, r);
}

//! If the poly-hull between \p p and \p q is exact it is assigned to \p p.
template <typename PH>
bool
poly_hull_assign_if_exact(PH& p, const PH& q);

template <typename PH>
bool
poly_hull_assign_if_exact(PH& p, const PH& q) {
  PH phull = p;
  NNC_Polyhedron nnc_p(p);
  phull.poly_hull_assign(q);
  std::pair<PH, Polyhedra_PowerSet<NNC_Polyhedron> >
    partition = linear_partition(q, phull);
  const Polyhedra_PowerSet<NNC_Polyhedron>& s = partition.second;
  typedef Polyhedra_PowerSet<NNC_Polyhedron>::const_iterator iter;
  for (iter i = s.begin(), s_end = s.end(); i != s_end; ++i)
    // The polyhedral hull is exact if and only if all the elements
    // of the partition of the polyhedral hull of `p' and `q' with
    // respect to `q' are included in `p'
    if (!nnc_p.contains(i->polyhedron()))
      return false;
  p = phull;
  return true;
}

} // namespace Parma_Polyhedra_Library
