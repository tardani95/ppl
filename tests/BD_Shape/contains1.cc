/* Test BD_Shape::contains().
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
  Variable x(0);
  Variable y(1);
  Variable z(2);

  TBD_Shape bd1(3);
  bd1.add_constraint(x - y <= 1);

  // The BD_Shape is empty, because it has got a negative cycle.
  TBD_Shape bd2(3);
  bd2.add_constraint(x - y <= 2);
  bd2.add_constraint(y - z <= 2);
  bd2.add_constraint(z - x <= -5);

  print_constraints(bd1, "*** bd1 ***");
  print_constraints(bd2, "*** bd2 ***");

  bool ok = bd1.contains(bd2);

  nout << "*** bd1.contains(bd2) ***"
       << endl
       << (ok ? "true" : "false")
       << endl;

  return ok;
}

bool
test02() {
  TBD_Shape bd1;
  TBD_Shape bd2(0, EMPTY);

  print_constraints(bd1, "*** bd1 ***");
  print_constraints(bd2, "*** bd2 ***");

  bool ok = bd1.contains(bd2);

  nout << "*** bd1.contains(bd2) ***"
       << endl
       << (ok ? "true" : "false")
       << endl;

  return ok;
}

bool
test03() {
  TBD_Shape bd1(0, EMPTY);
  TBD_Shape bd2(0, EMPTY);

  print_constraints(bd1, "*** bd1 ***");
  print_constraints(bd2, "*** bd2 ***");

  bool ok = bd1.contains(bd2);

  nout << "*** bd1.contains(bd2) ***"
       << endl
       << (ok ? "true" : "false")
       << endl;

  return ok;
}

bool
test04() {
  Variable x(0);
  Variable y(1);

  TBD_Shape bd1(3);
  bd1.add_constraint(x - y >= 0);

  TBD_Shape bd2(2);
  bd2.add_constraint(x - y == 0);

  try {
    // This is an invalid use of Polyhedron::contains(): it is
    // illegal to apply this method to two polyhedra that are not
    // dimension-compatible.
    bd1.contains(bd2);
  }
  catch (std::invalid_argument& e) {
    nout << "std::invalid_argument: " << e.what() << endl;
  }
  catch (...) {
    return false;
  }
  return true;
}

} // namespace

BEGIN_MAIN
  DO_TEST(test01);
  DO_TEST(test02);
  DO_TEST(test03);
  DO_TEST(test04);
END_MAIN
