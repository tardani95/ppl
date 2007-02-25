/* Exceptions used internally by the Prolog interfaces.
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

class internal_exception {
private:
  Prolog_term_ref t;
  const char* w;

public:
  internal_exception(Prolog_term_ref term, const char* where)
    : t(term),
      w(where) {
  }

  virtual ~internal_exception() {
  }

  virtual Prolog_term_ref term() const {
    return t;
  }

  virtual const char* where() const {
    return w;
  }
};

class Prolog_unsigned_out_of_range : public internal_exception {
private:
  unsigned long m;

public:
  Prolog_unsigned_out_of_range(Prolog_term_ref term,
			       const char* where,
			       unsigned long max)
    : internal_exception(term, where),
      m(max) {
  }

  unsigned long max() const {
    return m;
  }
};

class non_linear : public internal_exception {
public:
  non_linear(Prolog_term_ref term, const char* where)
    : internal_exception(term, where) {
  }
};

class not_an_integer : public internal_exception {
public:
  not_an_integer(Prolog_term_ref term, const char* where)
    : internal_exception(term, where) {
  }
};

class not_unsigned_integer : public internal_exception {
public:
  not_unsigned_integer(Prolog_term_ref term, const char* where)
    : internal_exception(term, where) {
  }
};

class not_a_variable : public internal_exception {
public:
  not_a_variable(Prolog_term_ref term, const char* where)
    : internal_exception(term, where) {
  }
};

class not_an_optimization_mode : public internal_exception {
public:
  not_an_optimization_mode(Prolog_term_ref term, const char* where)
    : internal_exception(term, where) {
  }
};

class not_a_complexity_class : public internal_exception {
public:
  not_a_complexity_class(Prolog_term_ref term, const char* where)
    : internal_exception(term, where) {
  }
};

class not_universe_or_empty : public internal_exception {
public:
  not_universe_or_empty(Prolog_term_ref term, const char* where)
    : internal_exception(term, where) {
  }
};

class not_a_relation : public internal_exception {
public:
  not_a_relation(Prolog_term_ref term, const char* where)
    : internal_exception(term, where) {
  }
};

class not_a_nil_terminated_list : public internal_exception {
public:
  not_a_nil_terminated_list(Prolog_term_ref term, const char* where)
    : internal_exception(term, where) {
  }
};

class PPL_integer_out_of_range {
private:
  Parma_Polyhedra_Library::Coefficient n;

public:
  PPL_integer_out_of_range(const Parma_Polyhedra_Library::Coefficient& value)
    : n(value) {
  }

  const Parma_Polyhedra_Library::Coefficient value() const {
    return n;
  }
};

class ppl_handle_mismatch : public internal_exception {
public:
  ppl_handle_mismatch(Prolog_term_ref term, const char* where)
    : internal_exception(term, where) {
  }
};

class unknown_interface_error {
private:
  const char* w;

public:
  unknown_interface_error(const char* s)
    : w(s) {
  }

  const char* where() const {
    return w;
  }
};
