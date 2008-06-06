/* Test Polyhedron::refine_with_congruences().
   Copyright (C) 2001-2008 Roberto Bagnara <bagnara@cs.unipr.it>

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

#include "ppl_test.hh"

namespace {

bool
test01() {
  Variable A(0);
  Variable B(1);

  C_Polyhedron ph(2);
  ph.refine_with_congruence(A %= 0);
  ph.refine_with_congruence(B %= 0);

  Congruence_System cgs;

  print_constraints(ph, "*** ph ***");
  print_congruences(cgs, "*** cgs ***");

  C_Polyhedron known_result(2);

  ph.refine_with_congruences(cgs);

  bool ok = (ph == known_result);

  print_constraints(ph, "*** ph ***");

  return ok;
}

bool
test02() {
  Variable A(0);
  Variable B(1);

  Constraint_System cs1;
  cs1.insert(A + B >= 0);
  C_Polyhedron ph(cs1);

  print_constraints(ph, "*** ph ***");

  Linear_Expression e(1);
  Congruence_System cgs2;
  cgs2.insert((e %= 0) / 0);
  ph.refine_with_congruences(cgs2);

  C_Polyhedron known_result(2, EMPTY);

  bool ok = (ph == known_result);

  print_constraints(ph, "*** after ph.refine_with_congruences(cgs2) ***");

  return ok;
}

bool
test03() {
  Variable A(0);

  C_Polyhedron ph(3);
  ph.refine_with_congruence(A %= 1);

  print_constraints(ph, "*** ph ***");

  C_Polyhedron computed_result(3);

  bool ok = (computed_result == ph);

  print_constraints(computed_result,
		    "*** after refine_with_congruences ***");

  return ok;
}

bool
test04() {
  Variable A(0);

  C_Polyhedron ph(3, EMPTY);

  Congruence_System cgs;
  cgs.insert(A %= 4);

  print_constraints(ph, "*** ph ***");
  print_congruences(cgs, "*** cgs ***");

  ph.refine_with_congruences(cgs);

  C_Polyhedron computed_result(3, EMPTY);

  bool ok = (ph == computed_result);

  print_constraints(ph, "*** after refine_with_congruences(cs) ***");

  return ok;
}

bool
test05() {
  C_Polyhedron ph;
  ph.refine_with_congruence(Linear_Expression(-2) %= 0);

  Congruence_System cgs;
  cgs.insert((Linear_Expression(-1) %= 0) / 2);

  print_constraints(ph, "*** ph ***");
  print_congruences(cgs, "*** cgs ***");

  ph.refine_with_congruences(cgs);

  C_Polyhedron known_result(0);

  bool ok = (known_result == ph);

  print_constraints(ph, "*** after refine_with_congruences ***");

  return ok;
}

bool
test06() {
  Variable A(0);
  Variable B(1);

  Generator_System gs;
  gs.insert(point());
  gs.insert(ray(A));
  gs.insert(ray(A + B));

  C_Polyhedron ph(gs);

  Congruence_System cgs;
  cgs.insert((A %= 3) / 2);

  print_generators(ph, "*** ph ***");
  print_congruences(cgs, "*** cgs ***");

  ph.refine_with_congruences(cgs);

  C_Polyhedron known_result(2);
  known_result.add_constraint(B >= 0);
  known_result.add_constraint(A - B >= 0);

  bool ok = (known_result == ph);

  print_constraints(ph, "*** after refine_with_congruences ***");

  return ok;
}

bool
test07() {
  Variable A(0);
  Variable B(1);

  C_Polyhedron ph1(2);

  Congruence_System cgs;
  cgs.insert(A - B == 0);

  print_constraints(ph1, "*** ph1 ***");
  print_congruences(cgs, "*** cgs ***");

  ph1.refine_with_congruences(cgs);

  C_Polyhedron known_result(2);
  known_result.add_constraint(A - B == 0);

  bool ok = (ph1 == known_result);

  print_constraints(ph1, "*** after ph1.refine_with_congruences(cgs) ***");

  return ok;
}

bool
test08() {
  Variable A(0);
  Variable B(1);

  C_Polyhedron ph1(2);

  Congruence_System cgs;
  cgs.insert((A - B %= 0) / 0);

  print_constraints(ph1, "*** ph1 ***");
  print_congruences(cgs, "*** cgs ***");

  ph1.refine_with_congruences(cgs);

  C_Polyhedron known_result(2);
  known_result.add_constraint(A - B == 0);

  bool ok = (ph1 == known_result);

  print_constraints(ph1, "*** after ph1.refine_with_congruences(cgs) ***");

  return ok;
}

bool
test09() {
  Variable A(0);
  Variable B(1);

  C_Polyhedron ph1(2);

  Congruence_System cgs;
  cgs.insert((A - B %= 1) / 0);
  cgs.insert(A + B %= 2);

  print_constraints(ph1, "*** ph1 ***");
  print_congruences(cgs, "*** cgs ***");

  ph1.refine_with_congruences(cgs);

  C_Polyhedron known_result(2);
  known_result.add_constraint(A - B == 1);

  bool ok = (ph1 == known_result);

  print_constraints(ph1, "*** after ph1.refine_with_congruences(cgs) ***");

  return ok;
}

bool
test10() {
  Variable A(0);
  Variable B(1);

  NNC_Polyhedron ph1(2);

  Congruence_System cgs;
  cgs.insert((A - B %= 0) / 0);

  print_constraints(ph1, "*** ph1 ***");
  print_congruences(cgs, "*** cgs ***");

  ph1.refine_with_congruences(cgs);

  NNC_Polyhedron known_result(2);
  known_result.add_constraint(A - B == 0);

  bool ok = (ph1 == known_result);

  print_constraints(ph1, "*** after ph1.refine_with_congruences(cgs) ***");

  return ok;
}

bool
test11() {
  Variable A(0);
  Variable B(1);


  NNC_Polyhedron ph1(2);
  Congruence_System cgs;
  cgs.insert((A - B %= 1) / 0);
  cgs.insert(A + B %= 2);

  print_constraints(ph1, "*** ph1 ***");
  print_congruences(cgs, "*** cgs ***");

  ph1.refine_with_congruences(cgs);

  NNC_Polyhedron known_result(2);
  known_result.add_constraint(A - B == 1);

  bool ok = (ph1 == known_result);

  print_constraints(ph1, "*** after ph1.refine_with_congruences(cgs) ***");

  return ok;
}

bool
test12() {
  C_Polyhedron ph;
  ph.refine_with_congruence(Linear_Expression(-2) %= 0);

  Congruence_System cgs;
  cgs.insert((Linear_Expression(-1) %= 0) / 0);

  print_constraints(ph, "*** ph ***");
  print_congruences(cgs, "*** cgs ***");

  ph.refine_with_congruences(cgs);

  C_Polyhedron known_result(0, EMPTY);

  bool ok = (known_result == ph);

  print_constraints(ph, "*** after refine_with_congruences ***");

  return ok;
}

} // namespace

BEGIN_MAIN
  DO_TEST(test01);
  DO_TEST(test02);
  DO_TEST(test03);
  DO_TEST(test04);
  DO_TEST(test05);
  DO_TEST(test06);
  DO_TEST(test07);
  DO_TEST(test08);
  DO_TEST(test09);
  DO_TEST(test10);
  DO_TEST(test11);
  DO_TEST(test12);
END_MAIN
