/* Pointset_Powerset class implementation: non-inline functions.
   Copyright (C) 2001-2007 Roberto Bagnara <bagnara@cs.unipr.it>

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

#include <config.h>
#include "Pointset_Powerset.defs.hh"
#include "Grid.defs.hh"
#include <utility>

namespace PPL = Parma_Polyhedra_Library;

template <>
void
PPL::Pointset_Powerset<PPL::NNC_Polyhedron>
::poly_difference_assign(const Pointset_Powerset& y) {
  Pointset_Powerset& x = *this;
  // Ensure omega-reduction.
  x.omega_reduce();
  y.omega_reduce();
  Sequence new_sequence = x.sequence;
  for (const_iterator yi = y.begin(), y_end = y.end(); yi != y_end; ++yi) {
    const NNC_Polyhedron& py = yi->element();
    Sequence tmp_sequence;
    for (Sequence_const_iterator nsi = new_sequence.begin(),
	   ns_end = new_sequence.end(); nsi != ns_end; ++nsi) {
      std::pair<NNC_Polyhedron, Pointset_Powerset<NNC_Polyhedron> > partition
	= linear_partition(py, nsi->element());
      const Pointset_Powerset<NNC_Polyhedron>& residues = partition.second;
      // Append the contents of `residues' to `tmp_sequence'.
      std::copy(residues.begin(), residues.end(), back_inserter(tmp_sequence));
    }
    std::swap(tmp_sequence, new_sequence);
  }
  std::swap(x.sequence, new_sequence);
  x.reduced = false;
  assert(x.OK());
}

template <>
bool
PPL::Pointset_Powerset<PPL::NNC_Polyhedron>
::geometrically_covers(const Pointset_Powerset& y) const {
  const Pointset_Powerset& x = *this;
  for (const_iterator yi = y.begin(), y_end = y.end(); yi != y_end; ++yi)
    if (!check_containment(yi->element(), x))
      return false;
  return true;
}

/*! \relates Parma_Polyhedra_Library::Pointset_Powerset */
bool
PPL::check_containment(const NNC_Polyhedron& ph,
		       const Pointset_Powerset<NNC_Polyhedron>& ps) {
  if (ph.is_empty())
    return true;
  Pointset_Powerset<NNC_Polyhedron> tmp(ph.space_dimension(), EMPTY);
  tmp.add_disjunct(ph);
  for (Pointset_Powerset<NNC_Polyhedron>::const_iterator
	 i = ps.begin(), ps_end = ps.end(); i != ps_end; ++i) {
    const NNC_Polyhedron& pi = i->element();
    for (Pointset_Powerset<NNC_Polyhedron>::iterator
	   j = tmp.begin(); j != tmp.end(); ) {
      const NNC_Polyhedron& pj = j->element();
      if (pi.contains(pj))
	j = tmp.drop_disjunct(j);
      else
	++j;
    }
    if (tmp.empty())
      return true;
    else {
      Pointset_Powerset<NNC_Polyhedron> new_disjuncts(ph.space_dimension(),
						      EMPTY);
      for (Pointset_Powerset<NNC_Polyhedron>::iterator
	     j = tmp.begin(); j != tmp.end(); ) {
	const NNC_Polyhedron& pj = j->element();
	if (pj.is_disjoint_from(pi))
	  ++j;
	else {
	  std::pair<NNC_Polyhedron, Pointset_Powerset<NNC_Polyhedron> >
	    partition = linear_partition(pi, pj);
	  new_disjuncts.upper_bound_assign(partition.second);
	  j = tmp.drop_disjunct(j);
	}
      }
      tmp.upper_bound_assign(new_disjuncts);
    }
  }
  return false;
}


namespace {

#if PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS
//! Uses the congruence \p c to approximately partition the grid \p qq.
/*! \relates Parma_Polyhedra_Library::Pointset_Powerset
  On exit, the intersection of \p qq and congruence \p c is stored
  in \p qq, whereas a finite set of grids whose set-theoretic union
  contains the intersection of \p qq with the negation of \p c
  is added, as a set of new disjuncts, to the powerset \p r.
*/
#endif // defined(PPL_DOXYGEN_INCLUDE_IMPLEMENTATION_DETAILS)
bool
approximate_partition_aux(const PPL::Congruence& c,
			  PPL::Grid& qq,
			  PPL::Pointset_Powerset<PPL::Grid>& r) {
  using namespace PPL;
  const Coefficient& c_modulus = c.modulus();
  Grid qq_copy(qq);

  if (!qq.add_congruence_and_minimize(c)) {
    r.add_disjunct(qq_copy);
    return true;
  }

  Congruence_System cgs = qq.congruences();
  Congruence_System cgs_copy = qq_copy.minimized_congruences();
  // When c is an equality, not satisfied by Grid qq
  // then add qq to the set r. There is no finite
  // partition in this case.
  if (c_modulus == 0) {
    if (cgs.num_equalities() != cgs_copy.num_equalities()) {
      r.add_disjunct(qq_copy);
      return false;
    }
    return true;
  }

  // When c is a proper congruence but, in qq, this direction has
  // no congruence, then add qq to the set r. There is no finite
  // partition in this case.
  if (cgs.num_proper_congruences() != cgs_copy.num_proper_congruences()) {
    r.add_disjunct(qq_copy);
    return false;
  }

  // When  c is a proper congruence and qq also is discrete
  // in this direction, then there is a finite partition and that
  // is added to r.
  const Coefficient& c_inhomogeneous_term = c.inhomogeneous_term();
  Linear_Expression le(c);
  le -= c_inhomogeneous_term;
  TEMP_INTEGER(n);
  rem_assign(n, c_inhomogeneous_term, c_modulus);
  TEMP_INTEGER(i);
  for (i = c_modulus; i-- > 0; )
    if (i != n) {
      Grid qqq(qq_copy);
      if (qqq.add_congruence_and_minimize((le+i %= 0) / c_modulus))
	r.add_disjunct(qqq);
    }
  return true;
}

} // namespace

std::pair<PPL::Grid, PPL::Pointset_Powerset<PPL::Grid> >
PPL::approximate_partition(const Grid& p, const Grid& q,
                                  bool& finite_partition) {
  using namespace PPL;
  finite_partition = true;
  Pointset_Powerset<Grid> r(p.space_dimension(), EMPTY);
  Grid qq = q;
  const Congruence_System& pcs = p.congruences();
  for (Congruence_System::const_iterator i = pcs.begin(),
	 pcs_end = pcs.end(); i != pcs_end; ++i)
    if (!approximate_partition_aux(*i, qq, r)) {
      finite_partition = false;
      Pointset_Powerset<Grid> s(q);
      return std::make_pair(qq, s);
    }
  return std::make_pair(qq, r);
}

bool
PPL::check_containment(const Grid& ph,
		       const Pointset_Powerset<Grid>& ps) {
  if (ph.is_empty())
    return true;
  Pointset_Powerset<Grid> tmp(ph.space_dimension(), EMPTY);
  tmp.add_disjunct(ph);
  for (Pointset_Powerset<Grid>::const_iterator
	 i = ps.begin(), ps_end = ps.end(); i != ps_end; ++i) {
    const Grid& pi = i->element();
    for (Pointset_Powerset<Grid>::iterator
	   j = tmp.begin(); j != tmp.end(); ) {
      const Grid& pj = j->element();
      if (pi.contains(pj))
	j = tmp.drop_disjunct(j);
      else
	++j;
    }
    if (tmp.empty())
      return true;
    else {
      Pointset_Powerset<Grid> new_disjuncts(ph.space_dimension(),
						      EMPTY);
      for (Pointset_Powerset<Grid>::iterator
	     j = tmp.begin(); j != tmp.end(); ) {
	const Grid& pj = j->element();
	if (pj.is_disjoint_from(pi))
	  ++j;
	else {
          bool finite_partition;
	  std::pair<Grid, Pointset_Powerset<Grid> >
	    partition = approximate_partition(pi, pj, finite_partition);

          // If there is a finite partition, then we add the new
          // disjuncts to the temporary set of disjuncts and drop pj.
          // If there is no finite partition, then by the
          // specification of approximate_partition(), we can
          // ignore checking the remaining temporary disjuncts as they
          // will all have the same lines and equalities and therefore
          // also not have a finite partition wrt pi.
          if (!finite_partition)
            break;
	  new_disjuncts.upper_bound_assign(partition.second);
	  j = tmp.drop_disjunct(j);
	}
      }
      tmp.upper_bound_assign(new_disjuncts);
    }
  }
  return false;
}

template <>
void
PPL::Pointset_Powerset<PPL::Grid>
::poly_difference_assign(const Pointset_Powerset& y) {
  Pointset_Powerset& x = *this;
  // Ensure omega-reduction.
  x.omega_reduce();
  y.omega_reduce();
  Sequence new_sequence = x.sequence;
  for (const_iterator yi = y.begin(), y_end = y.end(); yi != y_end; ++yi) {
    const Grid& py = yi->element();
    Sequence tmp_sequence;
    for (Sequence_const_iterator nsi = new_sequence.begin(),
	   ns_end = new_sequence.end(); nsi != ns_end; ++nsi) {
      bool finite_partition;
      std::pair<Grid, Pointset_Powerset<Grid> > partition
	= approximate_partition(py, nsi->element(), finite_partition);
      const Pointset_Powerset<Grid>& residues = partition.second;
      // Append the contents of `residues' to `tmp_sequence'.
      std::copy(residues.begin(), residues.end(), back_inserter(tmp_sequence));
    }
    std::swap(tmp_sequence, new_sequence);
  }
  std::swap(x.sequence, new_sequence);
  x.reduced = false;
  assert(x.OK());
}

template <>
bool
PPL::Pointset_Powerset<PPL::Grid>
::geometrically_covers(const Pointset_Powerset& y) const {
  const Pointset_Powerset& x = *this;
  for (const_iterator yi = y.begin(), y_end = y.end(); yi != y_end; ++yi)
    if (!check_containment(yi->element(), x))
      return false;
  return true;
}
