/* Octagonal_Shape class implementation: incremental strong-closure function.
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

template <typename T>
void
Octagonal_Shape<T>
::incremental_strong_closure_assign(const Variable var) const {
  // Split version: unrolling both the first loop and the second loop.
  using Implementation::BD_Shapes::min_assign;

  // `var' should be one of the dimensions of the octagon.
  if (var.id() >= space_dim)
    throw_dimension_incompatible("incremental_strong_closure_assign(v)",
				 var.id());

  // Do something only if necessary.
  if (marked_empty() || marked_strongly_closed())
    return;

  // Zero-dimensional octagons are necessarily strongly closed.
  if (space_dim == 0)
    return;

  Octagonal_Shape& x = const_cast<Octagonal_Shape<T>&>(*this);

  // Use these type aliases for short.
  typedef typename OR_Matrix<N>::row_iterator Row_Iterator;
  typedef typename OR_Matrix<N>::row_reference_type Row_Reference;
  // Avoid recomputations.
  const Row_Iterator m_begin = x.matrix.row_begin();
  const Row_Iterator m_end = x.matrix.row_end();

  // Fill the main diagonal with zeros.
  for (Row_Iterator i = m_begin; i != m_end; ++i) {
    assert(is_plus_infinity((*i)[i.index()]));
    assign_r((*i)[i.index()], 0, ROUND_NOT_NEEDED);
  }

#if COUNT
  dimension_type count = 0;
  dimension_type min_count = 0;
  dimension_type add_count = 0;
#endif

  // Using the incremental Floyd-Warshall algorithm.
  // Step 1: Improve all constraints on variable `var'.
  const dimension_type v = 2*var.id();
  const dimension_type cv = v+1;
  Row_Iterator v_iter = m_begin + v;
  Row_Iterator cv_iter = v_iter + 1;
  Row_Reference x_v = *v_iter;
  Row_Reference x_cv = *cv_iter;
  const dimension_type rs_v = v_iter.row_size();
  const dimension_type n_rows = x.matrix.num_rows();
  N sum1, sum2;
  Row_Reference x_k, x_ck, x_i, x_ci;

  // Split version.
  for (Row_Iterator k_iter = m_begin; k_iter != m_end; k_iter += 2) {
    const dimension_type k = k_iter.index();
    const dimension_type ck = k+1;
    const dimension_type rs_k = k_iter.row_size();
    x_k = *k_iter;
    x_ck = *(k_iter+1);
    const N& x_k_v = (v < rs_k) ? x_k[v] : x_cv[ck];
    const N& x_k_cv = (cv < rs_k) ? x_k[cv] : x_v[ck];
    const N& x_v_k = (k < rs_v) ? x_v[k] : x_ck[cv];
    const N& x_cv_k = (k < rs_v) ? x_cv[k] : x_ck[v];
    const N& x_ck_v = (v < rs_k) ? x_ck[v] : x_cv[k];
    const N& x_ck_cv = (cv < rs_k) ? x_ck[cv] : x_v[k];
    const N& x_v_ck = (k < rs_v) ? x_v[ck] : x_k[cv];
    const N& x_cv_ck = (k < rs_v) ? x_cv[ck] : x_k[v];

    // Working on v-th and cv-th columns.
    for (Row_Iterator i_iter = m_begin; i_iter != m_end; i_iter += 2) {
      const dimension_type i = i_iter.index();
      const dimension_type ci = i+1;
      const dimension_type rs_i = i_iter.row_size();
      x_i = *i_iter;
      x_ci = *(i_iter+1);

      const N& x_i_k = (k < rs_i) ? x_i[k] : x_ck[ci];
      const N& x_i_ck = (ck < rs_i) ? x_i[ck] : x_k[ci];
      const N& x_ci_k = (k < rs_i) ? x_ci[k] : x_ck[i];
      const N& x_ci_ck = (ck < rs_i) ? x_ci[ck] : x_k[i];

      if (cv < rs_i) {
	add_assign_r(sum1, x_i_k, x_k_v, ROUND_UP);
	add_assign_r(sum2, x_i_ck, x_ck_v, ROUND_UP);
	min_assign(sum1, sum2);
	N& x_i_v = x_i[v];
	min_assign(x_i_v, sum1);

	add_assign_r(sum1, x_ci_k, x_k_v, ROUND_UP);
	add_assign_r(sum2, x_ci_ck, x_ck_v, ROUND_UP);
	min_assign(sum1, sum2);
	N& x_ci_v = x_ci[v];
	min_assign(x_ci_v, sum1);

	add_assign_r(sum1, x_i_k, x_k_cv, ROUND_UP);
	add_assign_r(sum2, x_i_ck, x_ck_cv, ROUND_UP);
	min_assign(sum1, sum2);
	N& x_i_cv = x_i[cv];
	min_assign(x_i_cv, sum1);

	add_assign_r(sum1, x_ci_k, x_k_cv, ROUND_UP);
	add_assign_r(sum2, x_ci_ck, x_ck_cv, ROUND_UP);
	min_assign(sum1, sum2);
	N& x_ci_cv = x_ci[cv];
	min_assign(x_ci_cv, sum1);

#if COUNT
	min_count+=8;
	add_count+=8;
#endif

      }
      else {
	add_assign_r(sum1, x_i_k, x_k_v, ROUND_UP);
	add_assign_r(sum2, x_i_k, x_k_v, ROUND_UP);
	min_assign(sum1, sum2);
	N& x_i_v = x_cv[ci];
	min_assign(x_i_v, sum1);

	add_assign_r(sum1, x_ci_k, x_k_v, ROUND_UP);
	add_assign_r(sum2, x_ci_ck, x_ck_v, ROUND_UP);
	min_assign(sum1, sum2);
	N& x_ci_v = x_cv[i];
	min_assign(x_ci_v, sum1);

	add_assign_r(sum1, x_i_k, x_k_cv, ROUND_UP);
	add_assign_r(sum2, x_i_ck, x_ck_cv, ROUND_UP);
	min_assign(sum1, sum2);
	N& x_i_cv = x_v[ci];
	min_assign(x_i_cv, sum1);

	add_assign_r(sum1, x_ci_k, x_k_cv, ROUND_UP);
	add_assign_r(sum2, x_ci_ck, x_ck_cv, ROUND_UP);
	min_assign(sum1, sum2);
	N& x_ci_cv = x_v[i];
	min_assign(x_ci_cv, sum1);

#if COUNT
	min_count+=8;
	add_count+=8;
#endif

      }
    }

    // Working on v-th and cv-th rows.
    for (Row_Iterator i_iter = m_begin; i_iter != m_end; i_iter += 2) {
      const dimension_type i = i_iter.index();
      const dimension_type ci = i+1;
      x_i = *i_iter;
      x_ci = *(i_iter+1);

      const N& x_k_i = (i < rs_k) ? x_k[i] : x_ci[ck];
      const N& x_ck_i = (i < rs_k) ? x_ck[i] : x_ci[k];
      const N& x_k_ci = (ci < rs_k) ? x_k[ci] : x_i[ck];
      const N& x_ck_ci = (ci < rs_k) ? x_ck[ci] : x_i[k];

      if (ci < rs_v) {
	N& x_v_i = x_v[i];
	add_assign_r(sum1, x_v_k, x_k_i, ROUND_UP);
	add_assign_r(sum2, x_v_ck, x_ck_i, ROUND_UP);
	min_assign(sum1, sum2);
	min_assign(x_v_i, sum1);

	N& x_v_ci = x_v[ci];
	add_assign_r(sum1, x_v_k, x_k_ci, ROUND_UP);
	add_assign_r(sum2, x_v_ck, x_ck_ci, ROUND_UP);
	min_assign(sum1, sum2);
	min_assign(x_v_ci, sum1);

	N& x_cv_i = x_cv[i];
	add_assign_r(sum1, x_cv_k, x_k_i, ROUND_UP);
	add_assign_r(sum2, x_cv_ck, x_ck_i, ROUND_UP);
	min_assign(sum1, sum2);
	min_assign(x_cv_i, sum1);

	N& x_cv_ci = x_cv[ci];
	add_assign_r(sum1, x_cv_k, x_k_ci, ROUND_UP);
	add_assign_r(sum2, x_cv_ck, x_ck_ci, ROUND_UP);
	min_assign(sum1, sum2);
	min_assign(x_cv_ci, sum1);

#if COUNT
	min_count+=8;
	add_count+=8;
#endif

      }
      else {
	N& x_v_i = x_ci[cv];
	add_assign_r(sum1, x_v_k, x_k_i, ROUND_UP);
	add_assign_r(sum2, x_v_ck, x_ck_i, ROUND_UP);
	min_assign(sum1, sum2);
	min_assign(x_v_i, sum1);

	N& x_v_ci = x_i[cv];
	add_assign_r(sum1, x_v_k, x_k_ci, ROUND_UP);
	add_assign_r(sum2, x_v_ck, x_ck_ci, ROUND_UP);
	min_assign(sum1, sum2);
	min_assign(x_v_ci, sum1);

	N& x_cv_i = x_ci[v];
	add_assign_r(sum1, x_cv_k, x_k_i, ROUND_UP);
	add_assign_r(sum2, x_cv_ck, x_ck_i, ROUND_UP);
	min_assign(sum1, sum2);
	min_assign(x_cv_i, sum1);

	N& x_cv_ci = x_i[v];
	add_assign_r(sum1, x_cv_k, x_k_ci, ROUND_UP);
	add_assign_r(sum2, x_cv_ck, x_ck_ci, ROUND_UP);
	min_assign(sum1, sum2);
	min_assign(x_cv_ci, sum1);

#if COUNT
	min_count+=8;
	add_count+=8;
#endif

      }
    }
  }

  // Step 2: improve the other bounds by using the precise bounds
  // for the constraints on `var'.
  // Split original version.
  for (Row_Iterator i_iter = m_begin; i_iter != m_end; i_iter += 2) {
    const dimension_type i = i_iter.index();
    const dimension_type ci = i+1;
    const dimension_type rs_i = i_iter.row_size();
    x_i = *i_iter;
    x_ci = *(i_iter+1);
    const N& x_i_v = (v < rs_i) ? x_i[v] : x_cv[ci];
    const N& x_i_cv = (cv < rs_i) ? x_i[cv] : x_v[ci];
    const N& x_ci_v = (v < rs_i) ? x_ci[v] : x_cv[i];
    const N& x_ci_cv = (cv < rs_i) ? x_ci[cv] : x_v[i];
    // TODO: see if it is possible to optimize this inner loop
    // by splitting it into several parts, so as to avoid
    // conditional expressions.
    for (dimension_type j = 0; j < n_rows; j += 2) {
      const dimension_type cj = j+1;
      Row_Reference x_j = *(m_begin+j);
      Row_Reference x_cj = *(m_begin+cj);
      const N& x_v_j = (j < rs_v) ? x_v[j] : x_cj[cv];
      const N& x_cv_j = (j < rs_v) ? x_cv[j] : x_cj[v];
      const N& x_v_cj = (cj < rs_v) ? x_v[cj] : x_j[cv];
      const N& x_cv_cj = (cj < rs_v) ? x_cv[cj] : x_j[v];

      N& x_i_j = (j < rs_i) ? x_i[j] : x_cj[ci];
      N& x_i_cj = (cj < rs_i) ? x_i[cj] : x_j[ci];

      add_assign_r(sum1, x_i_v, x_v_j, ROUND_UP);
      min_assign(x_i_j, sum1);

      add_assign_r(sum1, x_i_cv, x_cv_j, ROUND_UP);
      min_assign(x_i_j, sum1);

      add_assign_r(sum1, x_i_v, x_v_cj, ROUND_UP);
      min_assign(x_i_cj, sum1);

      add_assign_r(sum1, x_i_cv, x_cv_cj, ROUND_UP);
      min_assign(x_i_cj, sum1);

      N& x_ci_j = (j < rs_i) ? x_ci[j] : x_cj[i];
      N& x_ci_cj = (cj < rs_i) ? x_ci[cj] : x_j[i];

      add_assign_r(sum1, x_ci_v, x_v_j, ROUND_UP);
      min_assign(x_ci_j, sum1);

      add_assign_r(sum1, x_ci_cv, x_cv_j, ROUND_UP);
      min_assign(x_ci_j, sum1);

      add_assign_r(sum1, x_ci_v, x_v_cj, ROUND_UP);
      min_assign(x_ci_cj, sum1);

      add_assign_r(sum1, x_ci_cv, x_cv_cj, ROUND_UP);
      min_assign(x_ci_cj, sum1);

#if COUNT
      min_count+=8;
      add_count+=8;
#endif

    }
  }

#if COUNT
  std::cout << "Il numero di minimi e': " << min_count << std::endl;
  std::cout << "Il numero di addizioni e': " << add_count << std::endl;
  count = min_count + add_count;
  std::cout << "Il numero totale di operazioni per la chiusura "
	    << "incrementale e': " << count << std::endl;
#endif

  // Check for emptyness: the octagon is empty if and only if there is a
  // negative value on the main diagonal.
  for (Row_Iterator i = m_begin; i != m_end; ++i) {
    N& x_i_i = (*i)[i.index()];
    if (x_i_i < 0) {
      x.status.set_empty();
      return;
    }
    else {
      // Restore PLUS_INFINITY on the main diagonal.
      assert(x_i_i == 0);
      x_i_i = PLUS_INFINITY;
    }
  }

  // Step 3: we enforce the strong coherence.
  x.strong_coherence_assign();
  // The octagon is not empty and it is now strongly closed.
  x.status.set_strongly_closed();
}
