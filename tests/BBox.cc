/* Implementation of class BBox (non-inline functions).
   Copyright (C) 2001, 2002 Roberto Bagnara <bagnara@cs.unipr.it>

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

#include "BBox.hh"
#include <iostream>

void
BInterval::raise_lower_bound(bool closed,
			     const Integer& c, const Integer& d) {
  assert(d > 0 && ld >= 0);
  if ((closed && lc*d <= c*ld) || (!closed && lc*d < c*ld))  {
    lc = c;
    ld = d;
    lclosed = closed;
  }
}

void
BInterval::lower_upper_bound(bool closed,
			     const Integer& c, const Integer& d) {
  assert(d > 0 && ud >= 0);
  if ((!closed && uc*d >= c*ud) || (closed && uc*d > c*ud))  {
    uc = c;
    ud = d;
    uclosed = closed;
  }
}

void
BInterval::set_empty() {
  uc = -1;
  lc = 1;
  ud = 1;
  ld = 1;
}

void
BInterval::print(std::ostream& s) const {
  s << "lower bound = ";
  if (ld != 0) {
    if (lclosed == true)
      s << " true ";
    else
      s << " false";
    s << " : " << lc << " / " << ld << "," << std::endl;
  }
  else
    s << " none " << std::endl;

  s << "         ";
  s << "upper bound = ";
  if (ud != 0) {
    if (uclosed == true)
      s << " true ";
    else
      s << " false";
    s << " : " << uc << " / " << ud << "." << std::endl;
  }
  else
    s << " none,  " << std::endl;
}
 

bool
operator==(const BInterval& x, const BInterval& y) {
  return x.lclosed == y.lclosed
    && x.uclosed == y.uclosed
    && x.lc*y.ld == y.lc*x.ld
    && x.uc*y.ud == y.uc*x.ud;
}

bool
operator<=(const BInterval& x, const BInterval& y) {
  int l_sign = sgn(x.ld) * sgn(y.ld);
  int u_sign = sgn(x.ud) * sgn(y.ud);
  if (y.lclosed || (!x.lclosed && !y.lclosed)) {
    if (l_sign > 0 && x.lc * y.ld < y.lc * x.ld)
      return false;
    if (l_sign < 0 && x.lc * y.ld > y.lc * x.ld)
      return false;
    if (l_sign == 0)
      if (x.ld == 0 && x.lc < 0 && (y.ld != 0 || (y.ld == 0 && y.ld > 0)))
	return false;
  }
  else {
    assert(!y.lclosed && x.lclosed);
    if (l_sign > 0 && x.lc * y.ld <= y.lc * x.ld)
      return false;
    if (l_sign < 0 && x.lc * y.ld >= y.lc * x.ld)
      return false;
    if (l_sign == 0)
      if (x.ld == 0 && x.lc < 0)
	return false;
  }
  if (y.uclosed || (!x.uclosed && !y.uclosed)) {
    if (u_sign > 0 && x.uc * y.ud > y.uc * x.ud)
      return false;
    if (u_sign < 0 && x.uc * y.ud < y.uc * x.ud)
      return false;
    if (u_sign == 0)
      if (x.ud == 0 && x.uc > 0 && (y.ud != 0 || (y.ud == 0 && y.ud < 0)))
	return false;
  }
  else {
    assert(!y.uclosed && x.uclosed);
    if (u_sign > 0 && x.uc * y.ud >= y.uc * x.ud)
      return false;
    if (u_sign < 0 && x.uc * y.ud <= y.uc * x.ud)
      return false;
    if (u_sign == 0)
      if (x.ud == 0 && x.uc > 0)
	return false;
  }
  return true;
}

void
BBox::print(std::ostream& s, const std::string& intro) const {
  if (!intro.empty())
    s << intro << std::endl;
  dimension_type dim = box.size();
  for (dimension_type j = 0; j != dim ; j++) {
    s << j << " AXES:  ";
    box[j].print(s);
  }
}

void
BBox::set_empty() {
  for (dimension_type k = box.size(); k-- > 0; )
    box[k].set_empty();
}

bool
operator==(const BBox& x, const BBox& y) {
  dimension_type dimension = x.space_dimension();
  if (dimension != y.space_dimension())
    return false;

  for (dimension_type i = dimension; i-- > 0; )
    if (x[i] != y[i])
      return false;

  return true;
}

bool
operator<=(const BBox& x, const BBox& y) {
  dimension_type dimension = x.space_dimension();
  if (dimension > y.space_dimension())
    return false;

  for (dimension_type i = dimension; i-- > 0; )
    if (!(x[i] <= y[i]))
      return false;

  return true;
}
