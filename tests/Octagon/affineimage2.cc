/* Test Octagon::affine_image().
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
  Variable A(0);
  Variable B(1);
  Variable C(2);

  TOctagon oc(3);
  oc.add_constraint(C >= 1);
  oc.add_constraint(B >= 0);
  oc.add_constraint(A + B >= 2);

  print_constraints(oc, "*** oc ***");

  oc.affine_image(A, C + B, 1);

  Octagon<mpq_class> known_result(3);
  known_result.add_constraint(A >= 1);
  known_result.add_constraint(B >= 0);
  known_result.add_constraint(C >= 1);
  known_result.add_constraint(B - A <= -1);
  known_result.add_constraint(C - A <= 0);

  bool ok = (Octagon<mpq_class>(oc) == known_result);

  print_constraints(oc, "*** oc.affine_image(A, C + B, 1) ***");

  return ok;
}

bool
test02() {
  Variable A(0);
  Variable B(1);
  Variable C(2);

  TOctagon oc(3);
  oc.add_constraint(C <= 1);
  oc.add_constraint(B >= 0);
  oc.add_constraint(A + B >= 2);

  print_constraints(oc, "*** oc ***");

  oc.affine_image(A, C + B, 1);

  Octagon<mpq_class> known_result(3);
  known_result.add_constraint(B >= 0);
  known_result.add_constraint(C <= 1);
  known_result.add_constraint(A - B <= 1);
  known_result.add_constraint(A - C >= 0);

  bool ok = (Octagon<mpq_class>(oc) == known_result);

  print_constraints(oc, "*** oc.affine_image(A, C + B, 1) ***");

  return ok;
}

bool
test03() {
  Variable A(0);
  Variable B(1);
  Variable C(2);

  TOctagon oc(3);
  oc.add_constraint(C <= 1);
  oc.add_constraint(B >= 0);
  oc.add_constraint(A + B >= 2);
  oc.add_constraint(A >= 2);

  print_constraints(oc, "*** oc ***");

  oc.affine_image(A, -A, 1);

  Octagon<mpq_class> known_result(3);
  known_result.add_constraint(B >= 0);
  known_result.add_constraint(C <= 1);
  known_result.add_constraint(A <= -2);

  bool ok = (Octagon<mpq_class>(oc) == known_result);

  print_constraints(oc, "*** oc.affine_image(A, -A, 1) ***");

  return ok;
}

bool
test04() {
  Variable A(0);
  Variable B(1);
  Variable C(2);

  TOctagon oc(3);
  oc.add_constraint(C <= 1);
  oc.add_constraint(B >= 0);
  oc.add_constraint(A + B <= 2);
  oc.add_constraint(-A + B <= 1);
  oc.add_constraint(A >= 2);

  print_constraints(oc, "*** oc ***");

  oc.affine_image(A, -A, 1);

  Octagon<mpq_class> known_result(3);
  known_result.add_constraint(B >= 0);
  known_result.add_constraint(C <= 1);
  known_result.add_constraint(A <= -2);
  known_result.add_constraint(-A + B <= 2);
  known_result.add_constraint(A + B <= 1);

  bool ok = (Octagon<mpq_class>(oc) == known_result);

  print_constraints(oc, "*** oc.affine_image(A, -A, 1) ***");

  return ok;
}

bool
test05() {
  Variable A(0);
  Variable B(1);
  Variable C(2);
  Variable D(3);

  TOctagon oc(4);
  oc.add_constraint(C <= 1);
  oc.add_constraint(B >= 0);
  oc.add_constraint(A + C <= 3);
  oc.add_constraint(A <= 2);
  oc.add_constraint(A >= 1);
  oc.add_constraint(D >= 1);
  oc.add_constraint(D <= 2);

  print_constraints(oc, "*** oc ***");

  oc.affine_image(A, -A + 2*D, 1);
 
  Octagon<mpq_class> known_result(4);
  known_result.add_constraint(B >= 0);
  known_result.add_constraint(C <= 1);
  known_result.add_constraint(D >= 1);
  known_result.add_constraint(D <= 2);
  known_result.add_constraint(A <= 3);
  known_result.add_constraint(A >= 0);
  known_result.add_constraint(A - D <= 1);
  known_result.add_constraint(D - A <= 1);

  bool ok = (Octagon<mpq_class>(oc) == known_result);

  print_constraints(oc, "*** oc.affine_image(A, -A + 2*D, 1) ***");

  return ok;
}

} // namespace

BEGIN_MAIN
  DO_TEST(test01);
  DO_TEST(test02);
  DO_TEST(test03);
  DO_TEST(test04);
  DO_TEST(test05);
END_MAIN
