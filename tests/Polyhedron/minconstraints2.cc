/* Test Polyhedron::minimized_constraints().
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

#include "ppl_test.hh"

namespace {

bool
test01() {
  NNC_Polyhedron ph1;

  Constraint_System cs = ph1.minimized_constraints();

  NNC_Polyhedron ph2(cs);

  bool ok = (ph1 == ph2);

  print_constraints(ph1, "*** ph1 ***");
  print_constraints(cs, "*** cs ***");
  print_constraints(ph2, "*** ph2 ***");

  return ok;
}

bool
test02() {
  Variable x(0), y(1);

  Constraint_System cs;
  cs.insert(x >= 0);
  cs.insert(x < 1);
  cs.insert(y > 0);

  NNC_Polyhedron ph(cs);
  ph.minimized_constraints();

  // FIXME: checking what?
  return true;
}

} // namespace


BEGIN_MAIN
  NEW_TEST(test01);
  NEW_TEST(test02);
END_MAIN

