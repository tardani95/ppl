/* CO_Tree class implementation
   Copyright (C) 2001-2010 Roberto Bagnara <bagnara@cs.unipr.it>

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

#include <ppl-config.h>

#include "CO_Tree.defs.hh"

namespace PPL = Parma_Polyhedra_Library;

PPL::CO_Tree::CO_Tree(const std::vector<data_type>& v) {

  dimension_type n = 0;

  for (std::vector<data_type>::const_iterator i = v.begin(), i_end = v.end();
       i != i_end; ++i)
    if (*i != 0)
      ++n;

  init(n);

  if (n == 0) {
    PPL_ASSERT(OK());
    return;
  }

  tree_iterator root(*this);
  dimension_type index = 0;

  while (v[index] == 0) {
    ++index;
    PPL_ASSERT(index < v.size());
  }

  // This is static and with static allocation, to improve performance.
  // CHAR_BIT*sizeof(dimension_type) is the maximum k such that 2^k-1 is a
  // dimension_type, so it is the maximum tree height.
  // For each node level, the stack may contain up to 4 elements: two elements
  // with operation 0, one element with operation 2 and one element
  // with operation 3. An additional element with operation 1 can be at the
  // top of the tree.
  static std::pair<dimension_type,char> stack[4*CHAR_BIT*sizeof(dimension_type)+1];
  dimension_type stack_first_empty = 0;

  // A pair (n, operation) in the stack means:
  //
  // * Go to the parent, if operation is 0.
  // * Go to the left child, then fill the current tree with n elements, if
  //   operation is 1.
  // * Go to the right child, then fill the current tree with n elements, if
  //   operation is 2.
  // * Fill the current tree with n elements, if operation is 3.

  stack[0].first = n;
  stack[0].second = 3;
  ++stack_first_empty;

  while (stack_first_empty != 0) {

    // top_n         = stack.top().first;
    // top_operation = stack.top().second;
    const dimension_type top_n = stack[stack_first_empty - 1].first;
    const char top_operation = stack[stack_first_empty - 1].second;

    switch (top_operation) {

    case 0:
      root.get_parent();
      --stack_first_empty;
      continue;

    case 1:
      root.get_left_child();
      break;

    case 2:
      root.get_right_child();
      break;
#ifndef NDEBUG
    case 3:
      break;

    default:
      // We should not be here
      PPL_ASSERT(false);
#endif
    }

    // We now visit the current tree

    if (top_n == 0) {
      --stack_first_empty;
    } else {
      if (top_n == 1) {
        PPL_ASSERT(root->first == unused_index);
        PPL_ASSERT(index < v.size());
        root->first = index;
        new (&(root->second)) data_type(v[index]);
        ++index;
        while (index < v.size() && v[index] == 0)
          ++index;
        --stack_first_empty;
      } else {
        PPL_ASSERT(stack_first_empty + 3 < sizeof(stack)/sizeof(stack[0]));

        const dimension_type half = (top_n + 1) / 2;
        stack[stack_first_empty - 1].second = 0;
        stack[stack_first_empty    ] = std::make_pair(top_n - half, 2);
        stack[stack_first_empty + 1] = std::make_pair(1, 3);
        stack[stack_first_empty + 2].second = 0;
        stack[stack_first_empty + 3] = std::make_pair(half - 1, 1);
        stack_first_empty += 4;
      }
    }
  }
  size = n;
  PPL_ASSERT(OK());
}

PPL::CO_Tree::iterator
PPL::CO_Tree::insert(iterator itr, dimension_type key1) {
  PPL_ASSERT(key1 != unused_index);

  if (empty()) {
    insert_in_empty_tree(key1, Coefficient_zero());
    return iterator(*this);
  }

  if (itr == end() || itr == before_begin())
    return insert(key1);

  iterator candidate1 = bisect_near(itr, key1);

  if (key1 == candidate1->first)
    return candidate1;

  iterator candidate2 = candidate1;
  if (key1 < candidate1->first)
    --candidate2;
  else
    ++candidate2;

  tree_iterator candidate1_node(candidate1, *this);

  if (candidate2 == before_begin() || candidate2 == end())
    // Use candidate1
    return iterator(insert_precise(key1, Coefficient_zero(),
                                   candidate1_node));

  tree_iterator candidate2_node(candidate2, *this);

  // Adjacent nodes in an in-order visit of a tree always have different
  // heights. This fact can be easily proven by induction on the tree's
  // height, using the definition of the in-order layout.
  PPL_ASSERT(candidate1_node.get_offset() != candidate2_node.get_offset());

  if (candidate1_node.get_offset() < candidate2_node.get_offset()) {
    PPL_ASSERT(candidate1_node.depth() > candidate2_node.depth());
    // candidate1_node is deeper in the tree than candidate2_node, so use
    // candidate1_node.
    return iterator(insert_precise(key1, Coefficient_zero(),
                                   candidate1_node));
  } else {
    PPL_ASSERT(candidate1_node.depth() < candidate2_node.depth());
    // candidate2_node is deeper in the tree than candidate1_node, so use
    // candidate2_node.
    return iterator(insert_precise(key1, Coefficient_zero(),
                                    candidate2_node));
  }
}

PPL::CO_Tree::iterator
PPL::CO_Tree::insert(iterator itr, dimension_type key1, const data_type& data1) {
  PPL_ASSERT(key1 != unused_index);

  if (empty()) {
    insert_in_empty_tree(key1, data1);
    return iterator(*this);
  }

  if (itr == end() || itr == before_begin())
    return insert(key1, data1);

  iterator candidate1 = bisect_near(itr, key1);

  if (key1 == candidate1->first) {
    candidate1->second = data1;
    return candidate1;
  }

  iterator candidate2(candidate1);
  if (key1 < candidate1->first)
    --candidate2;
  else
    ++candidate2;

  tree_iterator candidate1_node(candidate1, *this);

  if (candidate2 == before_begin() || candidate2 == end())
    // Use candidate1
    return iterator(insert_precise(key1, data1, candidate1_node));

  tree_iterator candidate2_node(candidate2, *this);

  // Adjacent nodes in an in-order visit of a tree always have different
  // heights. This fact can be easily proven by induction on the tree's
  // height, using the definition of the in-order layout.
  PPL_ASSERT(candidate1_node.get_offset() != candidate2_node.get_offset());

  if (candidate1_node.get_offset() < candidate2_node.get_offset()) {
    PPL_ASSERT(candidate1_node.depth() > candidate2_node.depth());
    // candidate1_node is deeper in the tree than candidate2_node, so
    // use candidate1_node.
    return iterator(insert_precise(key1, data1, candidate1_node));
  } else {
    PPL_ASSERT(candidate1_node.depth() < candidate2_node.depth());
    // candidate2_node is deeper in the tree than candidate1_node, so
    // use candidate2_node.
    return iterator(insert_precise(key1, data1, candidate2_node));
  }
}

void
PPL::CO_Tree::erase_element_and_shift_left(dimension_type key) {
  iterator itr = erase(key);
  if (itr == end())
    return;
  dimension_type i = &(itr->second) - data;
  dimension_type* p = indexes + i;
  dimension_type* p_end = indexes + reserved_size + 1;
  for ( ; p != p_end; ++p)
    if (*p != unused_index)
      --(*p);
  PPL_ASSERT(OK());
}

void
PPL::CO_Tree::increase_keys_after(dimension_type key, dimension_type n) {
  if (empty())
    return;
  dimension_type* p = indexes + reserved_size;
  while (*p == unused_index)
    --p;
  while (p != indexes && *p >= key) {
    *p += n;
    --p;
    while (*p == unused_index)
      --p;
  }
  PPL_ASSERT(OK());
}

PPL::dimension_type
PPL::CO_Tree::bisect_in(dimension_type first, dimension_type last,
                   dimension_type key) const {
  PPL_ASSERT(first != 0);
  PPL_ASSERT(last <= reserved_size);
  PPL_ASSERT(first <= last);
  PPL_ASSERT(indexes[first] != unused_index);
  PPL_ASSERT(indexes[last] != unused_index);

  while (first < last) {
    dimension_type half = (first + last) / 2;
    dimension_type new_half = half;

    while (indexes[new_half] == unused_index)
      ++new_half;

    if (indexes[new_half] == key)
      return new_half;

    if (indexes[new_half] > key) {

      while (indexes[half] == unused_index)
        --half;

      last = half;

    } else {

      ++new_half;
      while (indexes[new_half] == unused_index)
        ++new_half;

      first = new_half;
    }
  }

  // It is important that last is returned instead of first, because first
  // may have increased beyond last, even beyond the original value of last
  // at the beginning of this method.
  return last;
}

PPL::dimension_type
PPL::CO_Tree::bisect_near(dimension_type hint, dimension_type key) const {
  PPL_ASSERT(hint != 0);
  PPL_ASSERT(hint <= reserved_size);
  PPL_ASSERT(indexes[hint] != unused_index);

  if (indexes[hint] == key)
    return hint;

  dimension_type new_hint;
  dimension_type offset = 1;

  if (indexes[hint] > key) {
    // The searched element is before `hint'.

    while (1) {

      if (hint <= offset) {
        // The searched element is in (0,hint).
        new_hint = hint;
        hint = 1;
        // The searched element is in [hint,new_hint).
        while (indexes[hint] == unused_index)
          ++hint;
        if (indexes[hint] >= key)
          return hint;
        // The searched element is in (hint,new_hint) and both indexes point
        // to valid elements.
        break;
      } else
        new_hint = hint - offset;

      PPL_ASSERT(new_hint > 0);
      PPL_ASSERT(new_hint <= reserved_size);

      // Find the element at `new_hint' (or the first after it).
      while (indexes[new_hint] == unused_index)
        ++new_hint;

      PPL_ASSERT(new_hint <= hint);

      if (indexes[new_hint] == key)
        return new_hint;
      else
        if (indexes[new_hint] < key) {
          // The searched element is in (new_hint,hint)
          std::swap(hint, new_hint);
          // The searched element is now in (hint,new_hint).
          break;
        }

      hint = new_hint;
      offset *= 2;
    }

  } else {
    // The searched element is after `hint'.
    while (1) {

      if (hint + offset > reserved_size) {
        // The searched element is in (hint,reserved_size+1).
        new_hint = reserved_size;
        // The searched element is in (hint,new_hint].
        while (indexes[new_hint] == unused_index)
          --new_hint;
        if (indexes[new_hint] <= key)
          return new_hint;
        // The searched element is in (hint,new_hint) and both indexes point
        // to valid elements.
        break;
      } else
        new_hint = hint + offset;

      PPL_ASSERT(new_hint > 0);
      PPL_ASSERT(new_hint <= reserved_size);

      // Find the element at `new_hint' (or the first after it).
      while (indexes[new_hint] == unused_index)
        --new_hint;

      PPL_ASSERT(hint <= new_hint);

      if (indexes[new_hint] == key)
        return new_hint;
      else
        if (indexes[new_hint] > key)
          // The searched element is in (hint,new_hint).
          break;

      hint = new_hint;
      offset *= 2;
    }
  }
  // The searched element is in (hint,new_hint).
  PPL_ASSERT(hint > 0);
  PPL_ASSERT(hint <= new_hint);
  PPL_ASSERT(new_hint <= reserved_size);
  PPL_ASSERT(indexes[hint] != unused_index);
  PPL_ASSERT(indexes[new_hint] != unused_index);

  if (hint == new_hint)
    return hint;

  ++hint;
  while (indexes[hint] == unused_index)
    ++hint;

  if (hint == new_hint)
    return hint;

  --new_hint;
  while (indexes[new_hint] == unused_index)
    --new_hint;

  PPL_ASSERT(hint <= new_hint);
  PPL_ASSERT(indexes[hint] != unused_index);
  PPL_ASSERT(indexes[new_hint] != unused_index);

  return bisect_in(hint, new_hint, key);
}

PPL::CO_Tree::tree_iterator
PPL::CO_Tree::insert_precise(dimension_type key1, const data_type& data1,
                             tree_iterator itr) {
  PPL_ASSERT(key1 != unused_index);
  PPL_ASSERT(!empty());

#ifndef NDEBUG
  tree_iterator itr2(*this);
  itr2.go_down_searching_key(key1);
  PPL_ASSERT(itr == itr2);
#endif

  if (itr->first == key1) {
    itr->second = data1;
    PPL_ASSERT(OK());
    return itr;
  }

  if (is_greater_than_ratio(size + 1, reserved_size, max_density_percent)) {

    rebuild_bigger_tree();

    // itr was invalidated by the rebuild operation
    itr.get_root();
    itr.go_down_searching_key(key1);

    PPL_ASSERT(itr->first != key1);
  }

  PPL_ASSERT(!is_greater_than_ratio(size + 1, reserved_size,
                                    max_density_percent));

  size++;

  if (!itr.is_leaf()) {
    if (key1 < itr->first)
      itr.get_left_child();
    else
      itr.get_right_child();
    PPL_ASSERT(itr->first == unused_index);

    itr->first = key1;
    new (&(itr->second)) data_type(data1);

  } else {

    itr = rebalance(itr, key1, data1);

    itr.go_down_searching_key(key1);

    PPL_ASSERT(itr->first == key1);
  }
  PPL_ASSERT(OK());

  return itr;
}

PPL::CO_Tree::iterator
PPL::CO_Tree::erase(tree_iterator itr) {
  PPL_ASSERT(itr->first != unused_index);

  PPL_ASSERT(size != 0);

  if (size == 1) {
    // Deleting the only element of this tree, now it is empty.
    destroy();
    init(0);
    return end();
  }

  if (is_less_than_ratio(size - 1, reserved_size, min_density_percent)
      && !is_greater_than_ratio(size - 1, reserved_size/2,
                                max_density_percent)) {

    const dimension_type key = itr->first;

    PPL_ASSERT(!is_greater_than_ratio(size, reserved_size, max_density_percent));

    rebuild_smaller_tree();
    itr.get_root();
    itr.go_down_searching_key(key);

    PPL_ASSERT(itr->first == key);
  }

#ifndef NDEBUG
  if (size > 1)
    PPL_ASSERT(!is_less_than_ratio(size - 1, reserved_size,
                                   min_density_percent)
               || is_greater_than_ratio(size - 1, reserved_size/2,
                                        max_density_percent));
#endif

  const dimension_type deleted_key = itr->first;
  tree_iterator deleted_node = itr;
  itr->second.~data_type();
  while (1) {
    dimension_type& current_key  = itr->first;
    data_type&      current_data = itr->second;
    if (itr.is_leaf())
      break;
    itr.get_left_child();
    if (itr->first != unused_index)
      // The left child has a value.
      itr.follow_right_childs_with_value();
    else {
      // The left child hasn't a value, try the right child.
      itr.get_parent();
      itr.get_right_child();
      if (itr->first != unused_index)
        // The right child has a value.
        itr.follow_left_childs_with_value();
      else {
        // The right child hasn't a value, too.
        itr.get_parent();
        break;
      }
    }
    std::swap(current_key, itr->first);
    move_data_element(current_data, itr->second);
  }

  PPL_ASSERT(itr->first != unused_index);
  itr->first = unused_index;
  --size;

  PPL_ASSERT(OK());

  itr = rebalance(itr, 0, Coefficient_zero());

  itr = least_common_ancestor(itr, deleted_node);
  itr.go_down_searching_key(deleted_key);

  iterator result(itr);

  if (result->first < deleted_key)
    ++result;

  PPL_ASSERT(OK());
  if (!(result == end() || result->first > deleted_key))
    PPL_ASSERT(false);
#ifndef NDEBUG
  if (!empty()) {
    iterator last = end();
    --last;
    PPL_ASSERT((result == end()) == (last->first < deleted_key));
  }
#endif

  return result;
}

void
PPL::CO_Tree::init(dimension_type reserved_size1) {

  if (reserved_size1 == 0) {
    indexes = NULL;
    data = NULL;
    size = 0;
    reserved_size = 0;
    max_depth = 0;
  } else {
    max_depth = integer_log2(reserved_size1) + 1;

    size = 0;
    reserved_size = ((dimension_type)1 << max_depth) - 1;
    indexes = new dimension_type[reserved_size + 2];
    data = static_cast<data_type*>(operator new(sizeof(data_type)
                                                * (reserved_size + 1)));
    // Mark all pairs as unused.
    for (dimension_type i = 1; i <= reserved_size; ++i)
      indexes[i] = unused_index;

    // These are used as markers by iterators.
    indexes[0] = 0;
    indexes[reserved_size + 1] = 0;
  }

  refresh_cached_iterators();

  PPL_ASSERT(structure_OK());
}

void
PPL::CO_Tree::destroy() {

  if (reserved_size != 0) {
    for (dimension_type i = 1; i <= reserved_size; ++i) {
      if (indexes[i] != unused_index)
        data[i].~data_type();
    }

    delete [] indexes;
    operator delete(static_cast<void*>(data));
  }
}

bool
PPL::CO_Tree::structure_OK() const {

  if (size > reserved_size)
    return false;

  if (reserved_size == 0) {
    if (indexes != NULL)
      return false;
    if (data != NULL)
      return false;
    if (max_depth != 0)
      return false;

    return true;
  }

  if (reserved_size < 3)
    return false;

  if (reserved_size != ((dimension_type)1 << max_depth) - 1)
    return false;

  if (data == NULL)
    return false;

  if (indexes == NULL)
    return false;

  if (max_depth == 0)
    return false;

  if (size == 0) {

    // This const_cast could be removed by adding a const_tree_iterator,
    // but it would add much code duplication without a real need.
    tree_iterator itr(*const_cast<CO_Tree*>(this));
    if (itr->first != unused_index)
      return false;

  } else {
    // This const_cast could be removed by adding a const_tree_iterator,
    // but it would add much code duplication without a real need.
    tree_iterator itr(*const_cast<CO_Tree*>(this));
    dimension_type real_size = count_used_in_subtree(itr);
    if (real_size != size)
      // There are \p real_size elements in the tree that are reachable by the
      // root, but size is \p size.
      return false;
  }

  if (size != 0) {
    const_iterator itr = begin();
    const_iterator itr_end = end();

    if (itr != itr_end) {
      dimension_type last_index = itr->first;
      for (++itr; itr != itr_end; ++itr) {
        if (last_index >= itr->first)
          // Found index \p itr->first after index \p last_index.
          return false;
        last_index = itr->first;
      }
    }
  }

  if (const_iterator(cached_before_begin) != const_iterator(*this, 0))
    return false;
  if (const_iterator(cached_end) != const_iterator(*this, reserved_size + 1))
    return false;
  if (cached_const_before_begin != const_iterator(*this, 0))
    return false;
  if (cached_const_end != const_iterator(*this, reserved_size + 1))
    return false;

  return true;
}

bool
PPL::CO_Tree::OK() const {

  if (!structure_OK())
    return false;

  {
    dimension_type real_size = 0;

    for (const_iterator itr = begin(), itr_end = end(); itr != itr_end; ++itr)
      ++real_size;

    if (real_size != size)
      // There are \p real_size elements in the tree, but size is \p size.
      return false;
  }

  if (reserved_size > 0) {
    if (is_greater_than_ratio(size, reserved_size, max_density_percent)
        && reserved_size != 3)
      // Found too high density.
      return false;
    if (is_less_than_ratio(size, reserved_size, min_density_percent)
        && !is_greater_than_ratio(size, reserved_size/2, max_density_percent))
      // Found too low density
      return false;
  }

  return true;
}

unsigned
PPL::CO_Tree::integer_log2(dimension_type n) {
  PPL_ASSERT(n != 0);
  unsigned result = 0;
  while (n != 1) {
    n /= 2;
    ++result;
  }
  return result;
}

void
PPL::CO_Tree::dump_subtree(tree_iterator itr) {
  if (!itr.is_leaf()) {
    itr.get_left_child();
    dump_subtree(itr);
    itr.get_parent();
  }
  std::cout << "At depth: " << itr.depth();
  if (itr->first == unused_index)
    std::cout << " (no data)" << std::endl;
  else
    std::cout << " pair (" << itr->first << "," << itr->second << ")" << std::endl;
  if (!itr.is_leaf()) {
    itr.get_right_child();
    dump_subtree(itr);
    itr.get_parent();
  }
}

PPL::CO_Tree::tree_iterator
PPL::CO_Tree::least_common_ancestor(tree_iterator itr1, tree_iterator itr2) {
  while (itr1.get_offset() > itr2.get_offset())
    itr2.get_parent();
  while (itr2.get_offset() > itr1.get_offset())
    itr1.get_parent();
  // Now itr1 and itr2 have the same depth.
  PPL_ASSERT(itr1.depth() == itr2.depth());
  while (itr1 != itr2) {
    itr1.get_parent();
    itr2.get_parent();
  }
  return itr1;
}

void
PPL::CO_Tree::rebuild_bigger_tree() {
  if (reserved_size == 0) {
    init(3);
    PPL_ASSERT(structure_OK());
    return;
  }

  dimension_type new_reserved_size = reserved_size*2 + 1;

  dimension_type* new_indexes = new dimension_type[new_reserved_size + 2];
  data_type* new_data
    = static_cast<data_type*>(operator new(sizeof(data_type)
                                            * (new_reserved_size + 1)));

  new_indexes[1] = unused_index;

  for (dimension_type i = 1, j = 2; i <= reserved_size; ++i, ++j) {
    new_indexes[j] = indexes[i];
    if (indexes[i] != unused_index)
      move_data_element(new_data[j], data[i]);
    ++j;
    new_indexes[j] = unused_index;
  }

  // These are used as markers by iterators.
  new_indexes[0] = 0;
  new_indexes[new_reserved_size + 1] = 0;

  delete [] indexes;
  operator delete(data);

  indexes = new_indexes;
  data = new_data;
  reserved_size = new_reserved_size;
  ++max_depth;

  refresh_cached_iterators();

  PPL_ASSERT(structure_OK());
}

PPL::CO_Tree::tree_iterator
PPL::CO_Tree::rebalance(tree_iterator itr, dimension_type key,
                        const data_type& value) {
#ifndef NDEBUG
  if (itr->first != unused_index && !itr.is_leaf()) {
    tree_iterator itr_left = itr;
    itr_left.get_left_child();
    PPL_ASSERT(itr_left->first == unused_index);
    tree_iterator itr_right = itr;
    itr_right.get_right_child();
    PPL_ASSERT(itr_right->first == unused_index);
  }
#endif
  height_t itr_depth_minus_1 = itr.depth() - 1;
  height_t height = max_depth - itr_depth_minus_1;
  dimension_type subtree_size;
  dimension_type subtree_reserved_size = ((dimension_type)1 << height) - 1;
  const bool deleting = itr->first == unused_index;
  PPL_ASSERT(deleting || key != unused_index);
  if (deleting)
    subtree_size = 0;
  else
    // The existing element and the element with index key we want to add.
    subtree_size = 2;

  while (is_greater_than_ratio(subtree_size, subtree_reserved_size,
                               max_density_percent + itr_depth_minus_1
                               *(100 - max_density_percent)
                               /(max_depth - 1))
         || is_less_than_ratio(subtree_size, subtree_reserved_size,
                               min_density_percent - itr_depth_minus_1
                               *(min_density_percent - min_leaf_density_percent)
                               /(max_depth - 1))) {
    if (itr_depth_minus_1 == 0) {
      // TODO: Check if this is correct
      // We may arrive here, because we are using a fake subtree_size (it
      // isn't the real tree size, because the inserted/deleted element is
      // already taken into account).
#ifndef NDEBUG
      dimension_type real_subtree_size = subtree_size;
      if (!deleting)
        --real_subtree_size;
      PPL_ASSERT(!is_greater_than_ratio(real_subtree_size,
                                        subtree_reserved_size,
                                        max_density_percent));
      if (is_greater_than_ratio(real_subtree_size, subtree_reserved_size,
                                min_density_percent))
        PPL_ASSERT(is_greater_than_ratio(real_subtree_size,
                                         subtree_reserved_size/2,
                                         max_density_percent));
#endif
      break;
    }
    bool is_right_brother = itr.is_right_child();
    itr.get_parent();
    if (is_right_brother)
      itr.get_left_child();
    else
      itr.get_right_child();
    subtree_size += count_used_in_subtree(itr);
    itr.get_parent();
    PPL_ASSERT(itr->first != unused_index);
    ++subtree_size;
    subtree_reserved_size = 2*subtree_reserved_size + 1;
    --itr_depth_minus_1;
    PPL_ASSERT(itr.depth() - 1 == itr_depth_minus_1);
  };

  // Now the subtree rooted at itr has been chosen as the subtree to be
  // rebalanced.

  // Step 1: compact elements of this subtree in the rightmost end, from right
  //         to left.
  dimension_type last_index_in_subtree = itr.index() + itr.get_offset() - 1;

  dimension_type first_unused
    = compact_elements_in_the_rightmost_end(last_index_in_subtree, subtree_size,
                                            key, value, !deleting);

  // Step 2: redistribute the elements, from left to right.
  redistribute_elements_in_subtree(itr.index(), subtree_size,
                                   first_unused + 1, key, value,
                                   first_unused != last_index_in_subtree
                                                   - subtree_size);

  PPL_ASSERT(OK());

  return itr;
}

PPL::dimension_type
PPL::CO_Tree
::compact_elements_in_the_rightmost_end(dimension_type last_in_subtree,
                                        dimension_type subtree_size,
                                        dimension_type key,
                                        const data_type& value,
                                        bool add_element) {

  if (subtree_size == 0)
    return last_in_subtree;

  if (subtree_size == 1 && add_element) {
    // Just add the requested element.
    PPL_ASSERT(indexes[last_in_subtree] == unused_index);
    indexes[last_in_subtree] = key;
    new (&(data[last_in_subtree])) data_type(value);
    return last_in_subtree - 1;
  }

  dimension_type first_unused_index = last_in_subtree;
  while (indexes[last_in_subtree] == unused_index)
    --last_in_subtree;

  // From now on, last_in_subtree points to the rightmost node with a value in
  // the subtree and first_unused_index points to the rightmost unused node in
  // the subtree.

  if (add_element)
    while (subtree_size != 0) {
      --subtree_size;
      if (last_in_subtree == 0 || key > indexes[last_in_subtree]) {
        if (last_in_subtree == 0 || last_in_subtree != first_unused_index) {
          PPL_ASSERT(first_unused_index != 0);
          PPL_ASSERT(indexes[first_unused_index] == unused_index);
          indexes[first_unused_index] = key;
          new (&(data[first_unused_index])) data_type(value);
          --first_unused_index;
        }
        break;
      } else {
        if (last_in_subtree != first_unused_index) {
          PPL_ASSERT(first_unused_index != 0);
          PPL_ASSERT(last_in_subtree != 0);
          PPL_ASSERT(indexes[first_unused_index] == unused_index);
          indexes[first_unused_index] = indexes[last_in_subtree];
          indexes[last_in_subtree] = unused_index;
          move_data_element(data[first_unused_index], data[last_in_subtree]);
        }
        --last_in_subtree;
        while (indexes[last_in_subtree] == unused_index)
          --last_in_subtree;
        --first_unused_index;
      }
    }
  while (subtree_size != 0) {
    if (last_in_subtree != first_unused_index) {
      PPL_ASSERT(first_unused_index != 0);
      PPL_ASSERT(last_in_subtree != 0);
      PPL_ASSERT(indexes[first_unused_index] == unused_index);
      indexes[first_unused_index] = indexes[last_in_subtree];
      indexes[last_in_subtree] = unused_index;
      move_data_element(data[first_unused_index], data[last_in_subtree]);
    }
    --last_in_subtree;
    while (indexes[last_in_subtree] == unused_index)
      --last_in_subtree;
    --first_unused_index;
    --subtree_size;
  }

  return first_unused_index;
}

void
PPL::CO_Tree::redistribute_elements_in_subtree(dimension_type root_index,
                                               dimension_type subtree_size,
                                               dimension_type last_used,
                                               dimension_type key,
                                               const data_type& value,
                                               bool add_element) {
  // This is static and with static allocation, to improve performance.
  // CHAR_BIT*sizeof(dimension_type) is the maximum k such that 2^k-1 is a
  // dimension_type, so it is the maximum tree height.
  // For each node level, the stack may contain up to two element (one for the
  // subtree rooted at the right son of a node of that level, and one for the
  // node itself). An additional element can be at the top of the tree.
  static std::pair<dimension_type,dimension_type> stack[2*CHAR_BIT*sizeof(dimension_type)+1];
  std::pair<dimension_type,dimension_type>* stack_first_empty = stack;

  // A pair (n, i) in the stack means to visit the subtree with root index i
  // and size n.

  if (subtree_size == 0)
    return;

  stack_first_empty->first  = subtree_size;
  stack_first_empty->second = root_index;
  ++stack_first_empty;

  while (stack_first_empty != stack) {

    --stack_first_empty;

    // top_n = stack.top().first;
    // top_i = stack.top().second;
    const dimension_type top_n = stack_first_empty->first;
    const dimension_type top_i = stack_first_empty->second;

    PPL_ASSERT(top_n != 0);
    if (top_n == 1) {
      if (add_element && (last_used > reserved_size || indexes[last_used] > key)) {
        PPL_ASSERT(last_used != top_i);
        PPL_ASSERT(indexes[top_i] == unused_index);
        add_element = false;
        indexes[top_i] = key;
        new (&(data[top_i])) data_type(value);
      } else {
        if (last_used != top_i) {
          PPL_ASSERT(indexes[top_i] == unused_index);
          indexes[top_i] = indexes[last_used];
          indexes[last_used] = unused_index;
          move_data_element(data[top_i], data[last_used]);
        }
        ++last_used;
      }
    } else {
      PPL_ASSERT(stack_first_empty + 2
                 < stack + sizeof(stack)/sizeof(stack[0]));

      const dimension_type offset = (top_i & -top_i) / 2;
      const dimension_type half = (top_n + 1) / 2;

      PPL_ASSERT(half > 0);

      // Right subtree
      PPL_ASSERT(top_n - half > 0);
      stack_first_empty->first  = top_n - half;
      stack_first_empty->second = top_i + offset;
      ++stack_first_empty;

      // Root of the current subtree
      stack_first_empty->first   = 1;
      stack_first_empty->second  = top_i;
      ++stack_first_empty;

      // Left subtree
      if (half - 1 != 0) {
        stack_first_empty->first   = half - 1;
        stack_first_empty->second  = top_i - offset;
        ++stack_first_empty;
      }
    }
  }

  PPL_ASSERT(!add_element);
}

void
PPL::CO_Tree::move_data_from(CO_Tree& tree) {
  PPL_ASSERT(size == 0);
  if (tree.size == 0)
    return;

  tree_iterator root(*this);

  dimension_type source_index = 1;
  while (tree.indexes[source_index] == unused_index)
    ++source_index;

  // This is static and with static allocation, to improve performance.
  // CHAR_BIT*sizeof(dimension_type) is the maximum k such that 2^k-1 is a
  // dimension_type, so it is the maximum tree height.
  // For each node level, the stack may contain up to 4 elements: two elements
  // with operation 0, one element with operation 2 and one element
  // with operation 3. An additional element with operation 1 can be at the
  // top of the tree.
  static std::pair<dimension_type,char> stack[5*CHAR_BIT*sizeof(dimension_type)];
  dimension_type stack_first_empty = 0;

  // A pair (n, operation) in the stack means:
  //
  // * Go to the parent, if operation is 0.
  // * Go to the left child, then visit the current tree (with size n), if
  //   operation is 1.
  // * Go to the right child, then visit the current tree (with size n), if
  //   operation is 2.
  // * Visit the current tree (with size n), if operation is 3.

  stack[0].first = tree.size;
  stack[0].second = 3;
  ++stack_first_empty;

  while (stack_first_empty != 0) {

    // top_n         = stack.top().first;
    // top_operation = stack.top().second;
    const dimension_type top_n = stack[stack_first_empty - 1].first;
    const char top_operation = stack[stack_first_empty - 1].second;

    switch (top_operation) {

    case 0:
      root.get_parent();
      --stack_first_empty;
      continue;

    case 1:
      root.get_left_child();
      break;

    case 2:
      root.get_right_child();
      break;
#ifndef NDEBUG
    case 3:
      break;

    default:
      // We should not be here
      PPL_ASSERT(false);
#endif
    }

    // We now visit the current tree

    if (top_n == 0) {
      --stack_first_empty;
    } else {
      if (top_n == 1) {
        PPL_ASSERT(root->first == unused_index);
        PPL_ASSERT(tree.indexes[source_index] != unused_index);
        root->first = tree.indexes[source_index];
        tree.indexes[source_index] = unused_index;
        move_data_element(root->second, tree.data[source_index]);
        PPL_ASSERT(source_index <= tree.reserved_size);
        ++source_index;
        while (tree.indexes[source_index] == unused_index)
          ++source_index;
        --stack_first_empty;
      } else {
        PPL_ASSERT(stack_first_empty + 3 < sizeof(stack)/sizeof(stack[0]));

        const dimension_type half = (top_n + 1) / 2;
        stack[stack_first_empty - 1].second = 0;
        stack[stack_first_empty    ] = std::make_pair(top_n - half, 2);
        stack[stack_first_empty + 1] = std::make_pair(1, 3);
        stack[stack_first_empty + 2].second = 0;
        stack[stack_first_empty + 3] = std::make_pair(half - 1, 1);
        stack_first_empty += 4;
      }
    }
  }
  size = tree.size;
  tree.size = 0;
  PPL_ASSERT(tree.structure_OK());
  PPL_ASSERT(structure_OK());
}

void
PPL::CO_Tree::copy_data_from(const CO_Tree& x) {

  PPL_ASSERT(size == 0);
  PPL_ASSERT(reserved_size == x.reserved_size);

  if (x.size == 0) {
    PPL_ASSERT(OK());
    return;
  }

  for (dimension_type i = x.reserved_size; i > 0; --i)
    if (x.indexes[i] != unused_index) {
      indexes[i] = x.indexes[i];
      new (&(data[i])) data_type(x.data[i]);
    } else
      PPL_ASSERT(indexes[i] == unused_index);

  size = x.size;
  PPL_ASSERT(OK());
}

PPL::dimension_type
PPL::CO_Tree::count_used_in_subtree(tree_iterator itr) {
  dimension_type n = 0;

  const dimension_type k = itr.get_offset();
  const dimension_type root_index = itr.index();

  // The complete subtree rooted at itr has 2*k - 1 nodes.

  const dimension_type limit = root_index + (k - 1);

  PPL_ASSERT(root_index > (k - 1));

  const dimension_type* indexes = itr.tree.indexes;

  for (dimension_type j = root_index - (k - 1); j <= limit; ++j)
    if (indexes[j] != unused_index)
      ++n;

  return n;
}

bool
PPL::CO_Tree::const_iterator::OK() const {
#ifndef NDEBUG
  if (tree == 0) {
    if (current_index != 0)
      return false;
    if (current_data != 0)
      return false;
  } else
    if (tree->reserved_size == 0) {
      if (!((current_index == 0 && current_data == 0)
            || (current_index == 1 + (dimension_type*)0
                && current_data == 1 + (data_type*)0)))
        return false;
    } else {
      if (current_index < &(tree->indexes[0]))
        return false;
      if (current_index > &(tree->indexes[tree->reserved_size + 1]))
        return false;
      if (current_data < &(tree->data[0]))
        return false;
      if (current_data > &(tree->data[tree->reserved_size + 1]))
        return false;
      if (*current_index == unused_index)
        return false;
      if (current_index - tree->indexes != current_data - tree->data)
        return false;
    }
#endif
  return true;
}

bool
PPL::CO_Tree::iterator::OK() const {
#ifndef NDEBUG
  if (tree == 0) {
    if (current_index != 0)
      return false;
    if (current_data != 0)
      return false;
  } else
    if (tree->reserved_size == 0) {
      if (!((current_index == 0 && current_data == 0)
            || (current_index == 1 + (dimension_type*)0
                && current_data == 1 + (data_type*)0)))
        return false;
    } else {
      if (current_index < &(tree->indexes[0]))
        return false;
      if (current_index > &(tree->indexes[tree->reserved_size + 1]))
        return false;
      if (current_data < &(tree->data[0]))
        return false;
      if (current_data > &(tree->data[tree->reserved_size + 1]))
        return false;
      if (*current_index == unused_index)
        return false;
      if (current_index - tree->indexes != current_data - tree->data)
        return false;
    }
#endif
  return true;
}

bool
PPL::CO_Tree::tree_iterator::OK() const {
  if (i == 0 || i > tree.reserved_size)
    return false;

  // This assumes two's complement encoding.
  dimension_type correct_offset = i & -i;

  if (offset != correct_offset)
    return false;

  return true;
}

void
PPL::CO_Tree::tree_iterator::go_down_searching_key(dimension_type key) {
  // *this points to a node, so the tree is not empty.
  PPL_ASSERT(!tree.empty());
  PPL_ASSERT(key != unused_index);
  PPL_ASSERT((*this)->first != unused_index);
  while (!is_leaf()) {
    if (key == (*this)->first)
      break;
    if (key < (*this)->first) {
      get_left_child();
      if ((*this)->first == unused_index) {
        get_parent();
        break;
      }
    } else {
      get_right_child();
      if ((*this)->first == unused_index) {
        get_parent();
        break;
      }
    }
  }
}
