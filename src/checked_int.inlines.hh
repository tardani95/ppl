/* Specialized "checked" functions for native integer numbers.
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

#ifndef PPL_checked_int_inlines_hh
#define PPL_checked_int_inlines_hh 1

#include "Limits.hh"
#include <cerrno>
#include <cstdlib>
#include <climits>
#include <string>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#if !HAVE_DECL_STRTOLL
signed long long
strtoll(const char* nptr, char** endptr, int base);
#endif

#if !HAVE_DECL_STRTOULL
unsigned long long
strtoull(const char* nptr, char** endptr, int base);
#endif

namespace Parma_Polyhedra_Library {

namespace Checked {

#ifndef HAVE_INT_FAST16_T
typedef int16_t int_fast16_t;
#endif

#ifndef HAVE_INT_FAST32_T
typedef int32_t int_fast32_t;
#endif

#ifndef HAVE_INT_FAST64_T
typedef int64_t int_fast64_t;
#endif

#ifndef HAVE_UINT_FAST16_T
typedef uint16_t uint_fast16_t;
#endif

#ifndef HAVE_UINT_FAST32_T
typedef uint32_t uint_fast32_t;
#endif

#ifndef HAVE_UINT_FAST64_T
typedef uint64_t uint_fast64_t;
#endif

template <typename Policy, typename Type>
struct Extended_Int {
  static const Type plus_infinity = Limits<Type>::max;
  static const Type minus_infinity = (Limits<Type>::min >= 0
				      ? Limits<Type>::max - 1
				      : Limits<Type>::min);
  static const Type not_a_number = (Limits<Type>::min >= 0
				    ? Limits<Type>::max - Policy::handle_infinity * 2
				    : Limits<Type>::min + Policy::handle_infinity);
  static const Type min = (Limits<Type>::min
			   + (Limits<Type>::min >= 0 ? 0
			      : (Policy::handle_infinity + Policy::handle_nan)));
  static const Type max = (Limits<Type>::max
			   - (Limits<Type>::min >= 0
			      ? (2 * Policy::handle_infinity + Policy::handle_nan)
			      : Policy::handle_infinity));
};

template <typename Policy, typename To>
inline Result
set_neg_overflow_int(To& to, Rounding_Dir dir) {
  if (round_up(dir)) {
    to = Extended_Int<Policy, To>::min;
    return V_LT;
  }
  else {
    if (Policy::handle_infinity) {
      to = Extended_Int<Policy, To>::minus_infinity;
      return V_GT;
    }
    return V_NEG_OVERFLOW;
  }
}

template <typename Policy, typename To>
inline Result
set_pos_overflow_int(To& to, Rounding_Dir dir) {
  if (round_down(dir)) {
    to = Extended_Int<Policy, To>::max;
    return V_GT;
  }
  else {
    if (Policy::handle_infinity) {
      to = Extended_Int<Policy, To>::plus_infinity;
      return V_LT;
    }
    return V_POS_OVERFLOW;
  }
}

template <typename Policy, typename To>
inline Result
round_lt_int_no_overflow(To& to, Rounding_Dir dir) {
  if (round_down(dir)) {
    --to;
    return V_GT;
  }
  return V_LT;
}

template <typename Policy, typename To>
inline Result
round_gt_int_no_overflow(To& to, Rounding_Dir dir) {
  if (round_up(dir)) {
    ++to;
    return V_LT;
  }
  return V_GT;
}

template <typename Policy, typename To>
inline Result
round_lt_int(To& to, Rounding_Dir dir) {
  if (round_down(dir)) {
    if (to == Extended_Int<Policy, To>::min) {
      if (Policy::handle_infinity) {
	to = Extended_Int<Policy, To>::minus_infinity;
	return V_GT;
      }
      return V_NEG_OVERFLOW;
    }
    else {
      --to;
      return V_GT;
    }
  }
  return V_LT;
}

template <typename Policy, typename To>
inline Result
round_gt_int(To& to, Rounding_Dir dir) {
  if (round_up(dir)) {
    if (to == Extended_Int<Policy, To>::max) {
      if (Policy::handle_infinity) {
	to = Extended_Int<Policy, To>::plus_infinity;
	return V_LT;
      }
      return V_POS_OVERFLOW;
    }
    else {
      ++to;
      return V_LT;
    }
  }
  return V_GT;
}

SPECIALIZE_COPY(copy_generic, signed char)
SPECIALIZE_COPY(copy_generic, signed short)
SPECIALIZE_COPY(copy_generic, signed int)
SPECIALIZE_COPY(copy_generic, signed long)
SPECIALIZE_COPY(copy_generic, signed long long)
SPECIALIZE_COPY(copy_generic, unsigned char)
SPECIALIZE_COPY(copy_generic, unsigned short)
SPECIALIZE_COPY(copy_generic, unsigned int)
SPECIALIZE_COPY(copy_generic, unsigned long)
SPECIALIZE_COPY(copy_generic, unsigned long long)

template <typename Policy, typename Type>
inline Result
classify_int(const Type v, bool nan, bool inf, bool sign) {
  if (Policy::handle_nan
      && (nan || sign)
      && v == Extended_Int<Policy, Type>::not_a_number)
    return VC_NAN;
  if (!inf & !sign)
    return VC_NORMAL;
  if (Policy::handle_infinity) {
    if (v == Extended_Int<Policy, Type>::minus_infinity)
      return inf ? VC_MINUS_INFINITY : V_LT;
    if (v == Extended_Int<Policy, Type>::plus_infinity)
      return inf ? VC_PLUS_INFINITY : V_GT;
  }
  if (sign) {
    if (v < 0)
      return V_LT;
    if (v > 0)
      return V_GT;
    return V_EQ;
  }
  return VC_NORMAL;
}

SPECIALIZE_CLASSIFY(classify_int, signed char)
SPECIALIZE_CLASSIFY(classify_int, signed short)
SPECIALIZE_CLASSIFY(classify_int, signed int)
SPECIALIZE_CLASSIFY(classify_int, signed long)
SPECIALIZE_CLASSIFY(classify_int, signed long long)
SPECIALIZE_CLASSIFY(classify_int, unsigned char)
SPECIALIZE_CLASSIFY(classify_int, unsigned short)
SPECIALIZE_CLASSIFY(classify_int, unsigned int)
SPECIALIZE_CLASSIFY(classify_int, unsigned long)
SPECIALIZE_CLASSIFY(classify_int, unsigned long long)

template <typename Policy, typename Type>
inline bool
is_nan_int(const Type v) {
  return Policy::handle_nan && v == Extended_Int<Policy, Type>::not_a_number;
}

SPECIALIZE_IS_NAN(is_nan_int, signed char)
SPECIALIZE_IS_NAN(is_nan_int, signed short)
SPECIALIZE_IS_NAN(is_nan_int, signed int)
SPECIALIZE_IS_NAN(is_nan_int, signed long)
SPECIALIZE_IS_NAN(is_nan_int, signed long long)
SPECIALIZE_IS_NAN(is_nan_int, unsigned char)
SPECIALIZE_IS_NAN(is_nan_int, unsigned short)
SPECIALIZE_IS_NAN(is_nan_int, unsigned int)
SPECIALIZE_IS_NAN(is_nan_int, unsigned long)
SPECIALIZE_IS_NAN(is_nan_int, unsigned long long)

template <typename Policy, typename Type>
inline bool
is_minf_int(const Type v) {
  return Policy::handle_infinity
    && v == Extended_Int<Policy, Type>::minus_infinity;
}

SPECIALIZE_IS_MINF(is_minf_int, signed char)
SPECIALIZE_IS_MINF(is_minf_int, signed short)
SPECIALIZE_IS_MINF(is_minf_int, signed int)
SPECIALIZE_IS_MINF(is_minf_int, signed long)
SPECIALIZE_IS_MINF(is_minf_int, signed long long)
SPECIALIZE_IS_MINF(is_minf_int, unsigned char)
SPECIALIZE_IS_MINF(is_minf_int, unsigned short)
SPECIALIZE_IS_MINF(is_minf_int, unsigned int)
SPECIALIZE_IS_MINF(is_minf_int, unsigned long)
SPECIALIZE_IS_MINF(is_minf_int, unsigned long long)

template <typename Policy, typename Type>
inline bool
is_pinf_int(const Type v) {
  return Policy::handle_infinity
    && v == Extended_Int<Policy, Type>::plus_infinity;
}

SPECIALIZE_IS_PINF(is_pinf_int, signed char)
SPECIALIZE_IS_PINF(is_pinf_int, signed short)
SPECIALIZE_IS_PINF(is_pinf_int, signed int)
SPECIALIZE_IS_PINF(is_pinf_int, signed long)
SPECIALIZE_IS_PINF(is_pinf_int, signed long long)
SPECIALIZE_IS_PINF(is_pinf_int, unsigned char)
SPECIALIZE_IS_PINF(is_pinf_int, unsigned short)
SPECIALIZE_IS_PINF(is_pinf_int, unsigned int)
SPECIALIZE_IS_PINF(is_pinf_int, unsigned long)
SPECIALIZE_IS_PINF(is_pinf_int, unsigned long long)

template <typename Policy, typename Type>
inline bool
is_int_int(const Type v) {
  return !is_nan<Policy>(v);
}

SPECIALIZE_IS_INT(is_int_int, signed char)
SPECIALIZE_IS_INT(is_int_int, signed short)
SPECIALIZE_IS_INT(is_int_int, signed int)
SPECIALIZE_IS_INT(is_int_int, signed long)
SPECIALIZE_IS_INT(is_int_int, signed long long)
SPECIALIZE_IS_INT(is_int_int, unsigned char)
SPECIALIZE_IS_INT(is_int_int, unsigned short)
SPECIALIZE_IS_INT(is_int_int, unsigned int)
SPECIALIZE_IS_INT(is_int_int, unsigned long)
SPECIALIZE_IS_INT(is_int_int, unsigned long long)

template <typename Policy, typename Type>
inline Result
set_special_int(Type& v, Result r) {
  Result t = classify(r);
  if (Policy::handle_nan && t == VC_NAN)
    v = Extended_Int<Policy, Type>::not_a_number;
  else if (Policy::handle_infinity) {
    switch (t) {
    case VC_MINUS_INFINITY:
      v = Extended_Int<Policy, Type>::minus_infinity;
      break;
    case VC_PLUS_INFINITY:
      v = Extended_Int<Policy, Type>::plus_infinity;
      break;
    default:
      break;
    }
  }
  return r;
}

SPECIALIZE_SET_SPECIAL(set_special_int, signed char)
SPECIALIZE_SET_SPECIAL(set_special_int, signed short)
SPECIALIZE_SET_SPECIAL(set_special_int, signed int)
SPECIALIZE_SET_SPECIAL(set_special_int, signed long)
SPECIALIZE_SET_SPECIAL(set_special_int, signed long long)
SPECIALIZE_SET_SPECIAL(set_special_int, unsigned char)
SPECIALIZE_SET_SPECIAL(set_special_int, unsigned short)
SPECIALIZE_SET_SPECIAL(set_special_int, unsigned int)
SPECIALIZE_SET_SPECIAL(set_special_int, unsigned long)
SPECIALIZE_SET_SPECIAL(set_special_int, unsigned long long)

template <typename To_Policy, typename From_Policy, typename To, typename From>
inline Result
assign_signed_int_signed_int(To& to, const From from, Rounding_Dir dir) {
  if (sizeof(To) <= sizeof(From)) {
    if (CHECK_P(To_Policy::check_overflow,
		from < static_cast<From>(Extended_Int<To_Policy, To>::min)))
      return set_neg_overflow_int<To_Policy>(to, dir);
    if (CHECK_P(To_Policy::check_overflow,
		from > static_cast<From>(Extended_Int<To_Policy, To>::max)))
      return set_pos_overflow_int<To_Policy>(to, dir);
  }
  to = To(from);
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename To, typename From>
inline Result
assign_signed_int_unsigned_int(To& to, const From from, Rounding_Dir dir) {
  if (sizeof(To) <= sizeof(From)) {
    if (CHECK_P(To_Policy::check_overflow,
		from > static_cast<From>(Extended_Int<To_Policy, To>::max)))
      return set_pos_overflow_int<To_Policy>(to, dir);
  }
  to = To(from);
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename To, typename From>
inline Result
assign_unsigned_int_signed_int(To& to, const From from, Rounding_Dir dir) {
  if (CHECK_P(To_Policy::check_overflow, from < 0))
    return set_neg_overflow_int<To_Policy>(to, dir);
  if (sizeof(To) < sizeof(From)) {
    if (CHECK_P(To_Policy::check_overflow,
		from > static_cast<From>(Extended_Int<To_Policy, To>::max)))
      return set_pos_overflow_int<To_Policy>(to, dir);
  }
  to = To(from);
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename To, typename From>
inline Result
assign_unsigned_int_unsigned_int(To& to, const From from, Rounding_Dir dir) {
  if (sizeof(To) <= sizeof(From)) {
    if (CHECK_P(To_Policy::check_overflow,
		from > static_cast<From>(Extended_Int<To_Policy, To>::max)))
      return set_pos_overflow_int<To_Policy>(to, dir);
  }
  to = To(from);
  return V_EQ;
}


#define ASSIGN2_SIGNED_SIGNED(Smaller, Larger) \
SPECIALIZE_ASSIGN(assign_signed_int_signed_int, Smaller, Larger) \
SPECIALIZE_ASSIGN(assign_signed_int_signed_int, Larger, Smaller)

#define ASSIGN2_UNSIGNED_UNSIGNED(Smaller, Larger) \
SPECIALIZE_ASSIGN(assign_unsigned_int_unsigned_int, Smaller, Larger) \
SPECIALIZE_ASSIGN(assign_unsigned_int_unsigned_int, Larger, Smaller)

#define ASSIGN2_UNSIGNED_SIGNED(Smaller, Larger) \
SPECIALIZE_ASSIGN(assign_unsigned_int_signed_int, Smaller, Larger) \
SPECIALIZE_ASSIGN(assign_signed_int_unsigned_int, Larger, Smaller)

#define ASSIGN2_SIGNED_UNSIGNED(Smaller, Larger) \
SPECIALIZE_ASSIGN(assign_signed_int_unsigned_int, Smaller, Larger) \
SPECIALIZE_ASSIGN(assign_unsigned_int_signed_int, Larger, Smaller)

#define ASSIGN_SIGNED(Type) \
SPECIALIZE_ASSIGN(assign_signed_int_signed_int, Type, Type)
#define ASSIGN_UNSIGNED(Type) \
SPECIALIZE_ASSIGN(assign_unsigned_int_unsigned_int, Type, Type)

ASSIGN_SIGNED(signed char)
ASSIGN_SIGNED(signed short)
ASSIGN_SIGNED(signed int)
ASSIGN_SIGNED(signed long)
ASSIGN_SIGNED(signed long long)
ASSIGN_UNSIGNED(unsigned char)
ASSIGN_UNSIGNED(unsigned short)
ASSIGN_UNSIGNED(unsigned int)
ASSIGN_UNSIGNED(unsigned long)
ASSIGN_UNSIGNED(unsigned long long)

ASSIGN2_SIGNED_SIGNED(signed char, signed short)
ASSIGN2_SIGNED_SIGNED(signed char, signed int)
ASSIGN2_SIGNED_SIGNED(signed char, signed long)
ASSIGN2_SIGNED_SIGNED(signed char, signed long long)
ASSIGN2_SIGNED_SIGNED(signed short, signed int)
ASSIGN2_SIGNED_SIGNED(signed short, signed long)
ASSIGN2_SIGNED_SIGNED(signed short, signed long long)
ASSIGN2_SIGNED_SIGNED(signed int, signed long)
ASSIGN2_SIGNED_SIGNED(signed int, signed long long)
ASSIGN2_SIGNED_SIGNED(signed long, signed long long)
ASSIGN2_UNSIGNED_UNSIGNED(unsigned char, unsigned short)
ASSIGN2_UNSIGNED_UNSIGNED(unsigned char, unsigned int)
ASSIGN2_UNSIGNED_UNSIGNED(unsigned char, unsigned long)
ASSIGN2_UNSIGNED_UNSIGNED(unsigned char, unsigned long long)
ASSIGN2_UNSIGNED_UNSIGNED(unsigned short, unsigned int)
ASSIGN2_UNSIGNED_UNSIGNED(unsigned short, unsigned long)
ASSIGN2_UNSIGNED_UNSIGNED(unsigned short, unsigned long long)
ASSIGN2_UNSIGNED_UNSIGNED(unsigned int, unsigned long)
ASSIGN2_UNSIGNED_UNSIGNED(unsigned int, unsigned long long)
ASSIGN2_UNSIGNED_UNSIGNED(unsigned long, unsigned long long)
ASSIGN2_UNSIGNED_SIGNED(unsigned char, signed short)
ASSIGN2_UNSIGNED_SIGNED(unsigned char, signed int)
ASSIGN2_UNSIGNED_SIGNED(unsigned char, signed long)
ASSIGN2_UNSIGNED_SIGNED(unsigned char, signed long long)
ASSIGN2_UNSIGNED_SIGNED(unsigned short, signed int)
ASSIGN2_UNSIGNED_SIGNED(unsigned short, signed long)
ASSIGN2_UNSIGNED_SIGNED(unsigned short, signed long long)
ASSIGN2_UNSIGNED_SIGNED(unsigned int, signed long)
ASSIGN2_UNSIGNED_SIGNED(unsigned int, signed long long)
ASSIGN2_UNSIGNED_SIGNED(unsigned long, signed long long)
ASSIGN2_SIGNED_UNSIGNED(signed char, unsigned char)
ASSIGN2_SIGNED_UNSIGNED(signed char, unsigned short)
ASSIGN2_SIGNED_UNSIGNED(signed char, unsigned int)
ASSIGN2_SIGNED_UNSIGNED(signed char, unsigned long)
ASSIGN2_SIGNED_UNSIGNED(signed char, unsigned long long)
ASSIGN2_SIGNED_UNSIGNED(signed short, unsigned short)
ASSIGN2_SIGNED_UNSIGNED(signed short, unsigned int)
ASSIGN2_SIGNED_UNSIGNED(signed short, unsigned long)
ASSIGN2_SIGNED_UNSIGNED(signed short, unsigned long long)
ASSIGN2_SIGNED_UNSIGNED(signed int, unsigned int)
ASSIGN2_SIGNED_UNSIGNED(signed int, unsigned long)
ASSIGN2_SIGNED_UNSIGNED(signed int, unsigned long long)
ASSIGN2_SIGNED_UNSIGNED(signed long, unsigned long)
ASSIGN2_SIGNED_UNSIGNED(signed long, unsigned long long)
ASSIGN2_SIGNED_UNSIGNED(signed long long, unsigned long long)

template <typename To_Policy, typename From_Policy, typename To, typename From>
inline Result
assign_int_float(To& to, const From from, Rounding_Dir dir) {
  if (is_nan<From_Policy>(from))
    return set_special<To_Policy>(to, VC_NAN);
  else if (is_minf<From_Policy>(from))
    return assign<To_Policy, void>(to, MINUS_INFINITY, dir);
  else if (is_pinf<From_Policy>(from))
    return assign<To_Policy, void>(to, PLUS_INFINITY, dir);
  if (CHECK_P(To_Policy::check_overflow, (from < Extended_Int<To_Policy, To>::min)))
    return set_neg_overflow_int<To_Policy>(to, dir);
  if (CHECK_P(To_Policy::check_overflow, (from > Extended_Int<To_Policy, To>::max)))
    return set_pos_overflow_int<To_Policy>(to, dir);
  to = static_cast<To>(from);
  if (round_ignore(dir))
    return V_LGE;
  if (from < to)
    return round_lt_int<To_Policy>(to, dir);
  else if (from > to)
    return round_gt_int<To_Policy>(to, dir);
  else
    return V_EQ;
}

SPECIALIZE_ASSIGN(assign_int_float, signed char, float)
SPECIALIZE_ASSIGN(assign_int_float, signed short, float)
SPECIALIZE_ASSIGN(assign_int_float, signed int, float)
SPECIALIZE_ASSIGN(assign_int_float, signed long, float)
SPECIALIZE_ASSIGN(assign_int_float, signed long long, float)
SPECIALIZE_ASSIGN(assign_int_float, unsigned char, float)
SPECIALIZE_ASSIGN(assign_int_float, unsigned short, float)
SPECIALIZE_ASSIGN(assign_int_float, unsigned int, float)
SPECIALIZE_ASSIGN(assign_int_float, unsigned long, float)
SPECIALIZE_ASSIGN(assign_int_float, unsigned long long, float)

SPECIALIZE_ASSIGN(assign_int_float, signed char, double)
SPECIALIZE_ASSIGN(assign_int_float, signed short, double)
SPECIALIZE_ASSIGN(assign_int_float, signed int, double)
SPECIALIZE_ASSIGN(assign_int_float, signed long, double)
SPECIALIZE_ASSIGN(assign_int_float, signed long long, double)
SPECIALIZE_ASSIGN(assign_int_float, unsigned char, double)
SPECIALIZE_ASSIGN(assign_int_float, unsigned short, double)
SPECIALIZE_ASSIGN(assign_int_float, unsigned int, double)
SPECIALIZE_ASSIGN(assign_int_float, unsigned long, double)
SPECIALIZE_ASSIGN(assign_int_float, unsigned long long, double)

SPECIALIZE_ASSIGN(assign_int_float, signed char, long double)
SPECIALIZE_ASSIGN(assign_int_float, signed short, long double)
SPECIALIZE_ASSIGN(assign_int_float, signed int, long double)
SPECIALIZE_ASSIGN(assign_int_float, signed long, long double)
SPECIALIZE_ASSIGN(assign_int_float, signed long long, long double)
SPECIALIZE_ASSIGN(assign_int_float, unsigned char, long double)
SPECIALIZE_ASSIGN(assign_int_float, unsigned short, long double)
SPECIALIZE_ASSIGN(assign_int_float, unsigned int, long double)
SPECIALIZE_ASSIGN(assign_int_float, unsigned long, long double)
SPECIALIZE_ASSIGN(assign_int_float, unsigned long long, long double)

#undef ASSIGN2_SIGNED_SIGNED
#undef ASSIGN2_UNSIGNED_UNSIGNED
#undef ASSIGN2_UNSIGNED_SIGNED
#undef ASSIGN2_SIGNED_UNSIGNED

template <typename To_Policy, typename From_Policy, typename To>
inline Result
assign_signed_int_mpz(To& to, const mpz_class& from, Rounding_Dir dir) {
  if (sizeof(To) <= sizeof(signed long)) {
    if (!To_Policy::check_overflow) {
      to = from.get_si();
      return V_EQ;
    }
    if (from.fits_slong_p()) {
      signed long v = from.get_si();
      if (v < Limits<To>::min)
	return set_neg_overflow_int<To_Policy>(to, dir);
      if (v > Limits<To>::max)
	return set_pos_overflow_int<To_Policy>(to, dir);
      to = v;
      return V_EQ;
    }
  }
  else {
    mpz_srcptr m = from.get_mpz_t();
    size_t sz = mpz_size(m);
    if (sz <= sizeof(To) / sizeof(mp_limb_t)) {
      if (sz == 0) {
	to = 0;
	return V_EQ;
      }
      To v;
      mpz_export(&v, 0, -1, sizeof(To), 0, 0, m);
      if (v >= 0) {
	if (::sgn(from) < 0)
	  return neg<To_Policy, To_Policy>(to, v, dir);
	to = v;
	return V_EQ;
      }
    }
  }
  return ::sgn(from) < 0
    ? set_neg_overflow_int<To_Policy>(to, dir)
    : set_pos_overflow_int<To_Policy>(to, dir);
}

SPECIALIZE_ASSIGN(assign_signed_int_mpz, signed char, mpz_class)
SPECIALIZE_ASSIGN(assign_signed_int_mpz, signed short, mpz_class)
SPECIALIZE_ASSIGN(assign_signed_int_mpz, signed int, mpz_class)
SPECIALIZE_ASSIGN(assign_signed_int_mpz, signed long, mpz_class)
SPECIALIZE_ASSIGN(assign_signed_int_mpz, signed long long, mpz_class)

template <typename To_Policy, typename From_Policy, typename To>
inline Result
assign_unsigned_int_mpz(To& to, const mpz_class& from, Rounding_Dir dir) {
  if (CHECK_P(To_Policy::check_overflow, ::sgn(from) < 0))
    return set_neg_overflow_int<To_Policy>(to, dir);
  if (sizeof(To) <= sizeof(unsigned long)) {
    if (!To_Policy::check_overflow) {
      to = from.get_ui();
      return V_EQ;
    }
    if (from.fits_ulong_p()) {
      unsigned long v = from.get_ui();
      if (v > Limits<To>::max)
	return set_pos_overflow_int<To_Policy>(to, dir);
      to = v;
      return V_EQ;
    }
  }
  else {
    mpz_srcptr m = from.get_mpz_t();
    size_t sz = mpz_size(m);
    if (sz <= sizeof(To) / sizeof(mp_limb_t)) {
      if (sz == 0)
	to = 0;
      else
	mpz_export(&to, 0, -1, sizeof(To), 0, 0, m);
      return V_EQ;
    }
  }
  return set_pos_overflow_int<To_Policy>(to, dir);
}

SPECIALIZE_ASSIGN(assign_unsigned_int_mpz, unsigned char, mpz_class)
SPECIALIZE_ASSIGN(assign_unsigned_int_mpz, unsigned short, mpz_class)
SPECIALIZE_ASSIGN(assign_unsigned_int_mpz, unsigned int, mpz_class)
SPECIALIZE_ASSIGN(assign_unsigned_int_mpz, unsigned long, mpz_class)
SPECIALIZE_ASSIGN(assign_unsigned_int_mpz, unsigned long long, mpz_class)

template <typename To_Policy, typename From_Policy, typename To>
inline Result
assign_int_mpq(To& to, const mpq_class& from, Rounding_Dir dir) {
  mpz_srcptr n = from.get_num().get_mpz_t();
  mpz_srcptr d = from.get_den().get_mpz_t();
  mpz_class q;
  mpz_ptr _q = q.get_mpz_t();
  if (round_ignore(dir)) {
    mpz_tdiv_q(_q, n, d);
    Result r = assign<To_Policy, void>(to, q, dir);
    if (r != V_EQ)
      return r;
    return V_LGE;
  }
  mpz_t rem;
  int sign;
  mpz_init(rem);
  mpz_tdiv_qr(_q, rem, n, d);
  sign = mpz_sgn(rem);
  mpz_clear(rem);
  Result r = assign<To_Policy, void>(to, q, dir);
  if (r != V_EQ)
    return r;
  switch (sign) {
  case -1:
    return round_lt_int<To_Policy>(to, dir);
  case 1:
    return round_gt_int<To_Policy>(to, dir);
  default:
    return V_EQ;
  }
}

SPECIALIZE_ASSIGN(assign_int_mpq, signed char, mpq_class)
SPECIALIZE_ASSIGN(assign_int_mpq, signed short, mpq_class)
SPECIALIZE_ASSIGN(assign_int_mpq, signed int, mpq_class)
SPECIALIZE_ASSIGN(assign_int_mpq, signed long, mpq_class)
SPECIALIZE_ASSIGN(assign_int_mpq, signed long long, mpq_class)
SPECIALIZE_ASSIGN(assign_int_mpq, unsigned char, mpq_class)
SPECIALIZE_ASSIGN(assign_int_mpq, unsigned short, mpq_class)
SPECIALIZE_ASSIGN(assign_int_mpq, unsigned int, mpq_class)
SPECIALIZE_ASSIGN(assign_int_mpq, unsigned long, mpq_class)
SPECIALIZE_ASSIGN(assign_int_mpq, unsigned long long, mpq_class)

template <typename To_Policy, typename From_Policy, typename To>
inline Result
assign_int_minf(To& to, const Minus_Infinity&, Rounding_Dir dir) {
  if (To_Policy::handle_infinity) {
    to = Extended_Int<To_Policy, To>::minus_infinity;
    return V_EQ;
  }
  if (round_up(dir)) {
    to = Extended_Int<To_Policy, To>::min;
    return V_LT;
  }
  return VC_MINUS_INFINITY;
}

template <typename To_Policy, typename From_Policy, typename To>
inline Result
assign_int_pinf(To& to, const Plus_Infinity&, Rounding_Dir dir) {
  if (To_Policy::handle_infinity) {
    to = Extended_Int<To_Policy, To>::plus_infinity;
    return V_EQ;
  }
  if (round_down(dir)) {
    to = Extended_Int<To_Policy, To>::max;
    return V_GT;
  }
  return VC_PLUS_INFINITY;
}

template <typename To_Policy, typename From_Policy, typename To>
inline Result
assign_int_nan(To& to, const Not_A_Number&, Rounding_Dir) {
  if (To_Policy::handle_nan) {
    to = Extended_Int<To_Policy, To>::not_a_number;
    return V_EQ;
  }
  return VC_NAN;
}

SPECIALIZE_ASSIGN(assign_int_minf, signed char, Minus_Infinity)
SPECIALIZE_ASSIGN(assign_int_minf, signed short, Minus_Infinity)
SPECIALIZE_ASSIGN(assign_int_minf, signed int, Minus_Infinity)
SPECIALIZE_ASSIGN(assign_int_minf, signed long, Minus_Infinity)
SPECIALIZE_ASSIGN(assign_int_minf, signed long long, Minus_Infinity)
SPECIALIZE_ASSIGN(assign_int_minf, unsigned char, Minus_Infinity)
SPECIALIZE_ASSIGN(assign_int_minf, unsigned short, Minus_Infinity)
SPECIALIZE_ASSIGN(assign_int_minf, unsigned int, Minus_Infinity)
SPECIALIZE_ASSIGN(assign_int_minf, unsigned long, Minus_Infinity)
SPECIALIZE_ASSIGN(assign_int_minf, unsigned long long, Minus_Infinity)

SPECIALIZE_ASSIGN(assign_int_pinf, signed char, Plus_Infinity)
SPECIALIZE_ASSIGN(assign_int_pinf, signed short, Plus_Infinity)
SPECIALIZE_ASSIGN(assign_int_pinf, signed int, Plus_Infinity)
SPECIALIZE_ASSIGN(assign_int_pinf, signed long, Plus_Infinity)
SPECIALIZE_ASSIGN(assign_int_pinf, signed long long, Plus_Infinity)
SPECIALIZE_ASSIGN(assign_int_pinf, unsigned char, Plus_Infinity)
SPECIALIZE_ASSIGN(assign_int_pinf, unsigned short, Plus_Infinity)
SPECIALIZE_ASSIGN(assign_int_pinf, unsigned int, Plus_Infinity)
SPECIALIZE_ASSIGN(assign_int_pinf, unsigned long, Plus_Infinity)
SPECIALIZE_ASSIGN(assign_int_pinf, unsigned long long, Plus_Infinity)

SPECIALIZE_ASSIGN(assign_int_nan, signed char, Not_A_Number)
SPECIALIZE_ASSIGN(assign_int_nan, signed short, Not_A_Number)
SPECIALIZE_ASSIGN(assign_int_nan, signed int, Not_A_Number)
SPECIALIZE_ASSIGN(assign_int_nan, signed long, Not_A_Number)
SPECIALIZE_ASSIGN(assign_int_nan, signed long long, Not_A_Number)
SPECIALIZE_ASSIGN(assign_int_nan, unsigned char, Not_A_Number)
SPECIALIZE_ASSIGN(assign_int_nan, unsigned short, Not_A_Number)
SPECIALIZE_ASSIGN(assign_int_nan, unsigned int, Not_A_Number)
SPECIALIZE_ASSIGN(assign_int_nan, unsigned long, Not_A_Number)
SPECIALIZE_ASSIGN(assign_int_nan, unsigned long long, Not_A_Number)

#if UCHAR_MAX == 0xff
#define CHAR_BITS 8
#else
#error "Unexpected max for unsigned char"
#endif

#if USHRT_MAX == 0xffff
#define SHRT_BITS 16
#else
#error "Unexpected max for unsigned short"
#endif

#if UINT_MAX == 0xffffffff
#define INT_BITS 32
#else
#error "Unexpected max for unsigned int"
#endif

#if ULONG_MAX == 0xffffffffUL
#define LONG_BITS 32
#elif ULONG_MAX == 0xffffffffffffffffULL
#define LONG_BITS 64
#else
#error "Unexpected max for unsigned long"
#endif

#if ULLONG_MAX == 0xffffffffffffffffULL
#define LONG_LONG_BITS 64
#else
#error "Unexpected max for unsigned long long"
#endif


template <typename T>
struct Larger;

// The following may be tuned for performance on specific architectures.
//
// Current guidelines:
//   - avoid division where possible (larger type variant for mul)
//   - use larger type variant for types smaller than architecture bit size

template <>
struct Larger<signed char> {
  const_bool_nodef(use_for_neg, true);
  const_bool_nodef(use_for_add, true);
  const_bool_nodef(use_for_sub, true);
  const_bool_nodef(use_for_mul, true);
  typedef int_fast16_t Type_For_Neg;
  typedef int_fast16_t  Type_For_Add;
  typedef int_fast16_t  Type_For_Sub;
  typedef int_fast16_t  Type_For_Mul;
};

template <>
struct Larger<unsigned char> {
  const_bool_nodef(use_for_neg, true);
  const_bool_nodef(use_for_add, true);
  const_bool_nodef(use_for_sub, true);
  const_bool_nodef(use_for_mul, true);
  typedef int_fast16_t Type_For_Neg;
  typedef uint_fast16_t Type_For_Add;
  typedef int_fast16_t Type_For_Sub;
  typedef uint_fast16_t Type_For_Mul;
};

template <>
struct Larger<signed short> {
  const_bool_nodef(use_for_neg, true);
  const_bool_nodef(use_for_add, true);
  const_bool_nodef(use_for_sub, true);
  const_bool_nodef(use_for_mul, true);
  typedef int_fast32_t Type_For_Neg;
  typedef int_fast32_t Type_For_Add;
  typedef int_fast32_t Type_For_Sub;
  typedef int_fast32_t Type_For_Mul;
};

template <>
struct Larger<unsigned short> {
  const_bool_nodef(use_for_neg, true);
  const_bool_nodef(use_for_add, true);
  const_bool_nodef(use_for_sub, true);
  const_bool_nodef(use_for_mul, true);
  typedef int_fast32_t Type_For_Neg;
  typedef uint_fast32_t Type_For_Add;
  typedef int_fast32_t Type_For_Sub;
  typedef uint_fast32_t Type_For_Mul;
};

template <>
struct Larger<signed int> {
  const_bool_nodef(use_for_neg, (LONG_BITS == 64));
  const_bool_nodef(use_for_add, (LONG_BITS == 64));
  const_bool_nodef(use_for_sub, (LONG_BITS == 64));
  const_bool_nodef(use_for_mul, true);
  typedef int_fast64_t Type_For_Neg;
  typedef int_fast64_t Type_For_Add;
  typedef int_fast64_t Type_For_Sub;
  typedef int_fast64_t Type_For_Mul;
};

template <>
struct Larger<unsigned int> {
  const_bool_nodef(use_for_neg, (LONG_BITS == 64));
  const_bool_nodef(use_for_add, (LONG_BITS == 64));
  const_bool_nodef(use_for_sub, (LONG_BITS == 64));
  const_bool_nodef(use_for_mul, true);
  typedef int_fast64_t Type_For_Neg;
  typedef uint_fast64_t Type_For_Add;
  typedef int_fast64_t Type_For_Sub;
  typedef uint_fast64_t Type_For_Mul;
};

template <>
struct Larger<signed long> {
  const_bool_nodef(use_for_neg, false);
  const_bool_nodef(use_for_add, false);
  const_bool_nodef(use_for_sub, false);
  const_bool_nodef(use_for_mul, (LONG_BITS == 32));
  typedef int_fast64_t Type_For_Neg;
  typedef int_fast64_t Type_For_Add;
  typedef int_fast64_t Type_For_Sub;
  typedef int_fast64_t Type_For_Mul;
};

template <>
struct Larger<unsigned long> {
  const_bool_nodef(use_for_neg, false);
  const_bool_nodef(use_for_add, false);
  const_bool_nodef(use_for_sub, false);
  const_bool_nodef(use_for_mul, (LONG_BITS == 32));
  typedef int_fast64_t Type_For_Neg;
  typedef uint_fast64_t Type_For_Add;
  typedef int_fast64_t Type_For_Sub;
  typedef uint_fast64_t Type_For_Mul;
};

template <>
struct Larger<signed long long> {
  const_bool_nodef(use_for_neg, false);
  const_bool_nodef(use_for_add, false);
  const_bool_nodef(use_for_sub, false);
  const_bool_nodef(use_for_mul, false);
  typedef int_fast64_t Type_For_Neg;
  typedef int_fast64_t Type_For_Add;
  typedef int_fast64_t Type_For_Sub;
  typedef int_fast64_t Type_For_Mul;
};

template <>
struct Larger<unsigned long long> {
  const_bool_nodef(use_for_neg, false);
  const_bool_nodef(use_for_add, false);
  const_bool_nodef(use_for_sub, false);
  const_bool_nodef(use_for_mul, false);
  typedef int_fast64_t Type_For_Neg;
  typedef uint_fast64_t Type_For_Add;
  typedef int_fast64_t Type_For_Sub;
  typedef uint_fast64_t Type_For_Mul;
};

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
neg_int_larger(Type& to, const Type x, Rounding_Dir dir) {
  typename Larger<Type>::Type_For_Neg l = x;
  l = -l;
  return assign<To_Policy, void>(to, l, dir);
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
add_int_larger(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  typename Larger<Type>::Type_For_Add l = x;
  l += y;
  return assign<To_Policy, void>(to, l, dir);
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
sub_int_larger(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  typename Larger<Type>::Type_For_Sub l = x;
  l -= y;
  return assign<To_Policy, void>(to, l, dir);
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
mul_int_larger(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  typename Larger<Type>::Type_For_Mul l = x;
  l *= y;
  return assign<To_Policy, void>(to, l, dir);
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
neg_signed_int(Type& to, const Type from, Rounding_Dir dir) {
  if (To_Policy::check_overflow && Larger<Type>::use_for_neg)
    return neg_int_larger<To_Policy, From_Policy>(to, from, dir);
  if (CHECK_P(To_Policy::check_overflow,
	      (from < -Extended_Int<To_Policy, Type>::max)))
    return set_pos_overflow_int<To_Policy>(to, dir);
  to = -from;
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
neg_unsigned_int(Type& to, const Type from, Rounding_Dir dir) {
  if (To_Policy::check_overflow && Larger<Type>::use_for_neg)
    return neg_int_larger<To_Policy, From_Policy>(to, from, dir);
  if (CHECK_P(To_Policy::check_overflow, from != 0))
    return set_neg_overflow_int<To_Policy>(to, dir);
  to = from;
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
floor_int(Type& to, const Type from, Rounding_Dir) {
  to = from;
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
ceil_int(Type& to, const Type from, Rounding_Dir) {
  to = from;
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
trunc_int(Type& to, const Type from, Rounding_Dir) {
  to = from;
  return V_EQ;
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
add_signed_int(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_overflow && Larger<Type>::use_for_add)
    return add_int_larger<To_Policy, From1_Policy, From2_Policy>(to, x, y, dir);
  if (To_Policy::check_overflow) {
    if (y >= 0) {
      if (x > Extended_Int<To_Policy, Type>::max - y)
	return set_pos_overflow_int<To_Policy>(to, dir);
    }
    else if (x < Extended_Int<To_Policy, Type>::min - y)
	return set_neg_overflow_int<To_Policy>(to, dir);
  }
  to = x + y;
  return V_EQ;
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
add_unsigned_int(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_overflow && Larger<Type>::use_for_add)
    return add_int_larger<To_Policy, From1_Policy, From2_Policy>(to, x, y, dir);
  if (CHECK_P(To_Policy::check_overflow,
	      (x > Extended_Int<To_Policy, Type>::max - y)))
    return set_pos_overflow_int<To_Policy>(to, dir);
  to = x + y;
  return V_EQ;
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
sub_signed_int(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_overflow && Larger<Type>::use_for_sub)
    return sub_int_larger<To_Policy, From1_Policy, From2_Policy>(to, x, y, dir);
  if (To_Policy::check_overflow) {
    if (y >= 0) {
      if (x < Extended_Int<To_Policy, Type>::min + y)
	return set_neg_overflow_int<To_Policy>(to, dir);
    }
    else if (x > Extended_Int<To_Policy, Type>::max + y)
	return set_pos_overflow_int<To_Policy>(to, dir);
  }
  to = x - y;
  return V_EQ;
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
sub_unsigned_int(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_overflow && Larger<Type>::use_for_sub)
    return sub_int_larger<To_Policy, From1_Policy, From2_Policy>(to, x, y, dir);
  if (CHECK_P(To_Policy::check_overflow,
	      (x < Extended_Int<To_Policy, Type>::min + y)))
    return set_neg_overflow_int<To_Policy>(to, dir);
  to = x - y;
  return V_EQ;
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
mul_signed_int(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_overflow && Larger<Type>::use_for_mul)
    return mul_int_larger<To_Policy, From1_Policy, From2_Policy>(to, x, y, dir);
  if (!To_Policy::check_overflow) {
    to = x * y;
    return V_EQ;
  }
  if (y == 0) {
    to = 0;
    return V_EQ;
  }
  if (y == -1)
    return neg_signed_int<To_Policy, From1_Policy>(to, x, dir);
  if (x >= 0) {
    if (y > 0) {
      if (x > Extended_Int<To_Policy, Type>::max / y)
	return set_pos_overflow_int<To_Policy>(to, dir);
    }
    else {
      if (x > Extended_Int<To_Policy, Type>::min / y)
	return set_neg_overflow_int<To_Policy>(to, dir);
    }
  }
  else {
    if (y < 0) {
      if (x < Extended_Int<To_Policy, Type>::max / y)
	return set_pos_overflow_int<To_Policy>(to, dir);
    }
    else {
      if (x < Extended_Int<To_Policy, Type>::min / y)
	return set_neg_overflow_int<To_Policy>(to, dir);
    }
  }
  to = x * y;
  return V_EQ;
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
mul_unsigned_int(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_overflow && Larger<Type>::use_for_mul)
    return mul_int_larger<To_Policy, From1_Policy, From2_Policy>(to, x, y, dir);
  if (!To_Policy::check_overflow) {
    to = x * y;
    return V_EQ;
  }
  if (y == 0) {
    to = 0;
    return V_EQ;
  }
  if (x > Extended_Int<To_Policy, Type>::max / y)
    return set_pos_overflow_int<To_Policy>(to, dir);
  to = x * y;
  return V_EQ;
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
div_signed_int(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (CHECK_P(To_Policy::check_div_zero, y == 0))
    return set_special<To_Policy>(to, V_DIV_ZERO);
  if (To_Policy::check_overflow && y == -1)
    return neg_signed_int<To_Policy, From1_Policy>(to, x, dir);
  to = x / y;
  if (round_ignore(dir))
    return V_LGE;
  Type m = x % y;
  if (m < 0)
    return round_lt_int_no_overflow<To_Policy>(to, dir);
  else if (m > 0)
    return round_gt_int_no_overflow<To_Policy>(to, dir);
  else
    return V_EQ;
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
div_unsigned_int(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (CHECK_P(To_Policy::check_div_zero, y == 0))
    return set_special<To_Policy>(to, V_DIV_ZERO);
  to = x / y;
  if (round_ignore(dir))
    return V_GE;
  Type m = x % y;
  if (m == 0)
    return V_EQ;
  return round_gt_int<To_Policy>(to, dir);
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
rem_int(Type& to, const Type x, const Type y, Rounding_Dir) {
  if (CHECK_P(To_Policy::check_div_zero, y == 0))
    return set_special<To_Policy>(to, V_MOD_ZERO);
  to = x % y;
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
div2exp_unsigned_int(Type& to, const Type x, int exp, Rounding_Dir dir) {
  if (exp < 0)
    return mul2exp<To_Policy, From_Policy>(to, x, -exp, dir);
  if (static_cast<unsigned int>(exp) >= sizeof(Type) * 8) {
    to = 0;
    if (round_ignore(dir))
      return V_GE;
    if (x == 0)
      return V_EQ;
    return round_gt_int_no_overflow<To_Policy>(to, dir);
  }
  to = x >> exp;
  if (round_ignore(dir))
    return V_GE;
  if (x & ((static_cast<Type>(1) << exp) - 1))
    return round_gt_int_no_overflow<To_Policy>(to, dir);
  else
    return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
div2exp_signed_int(Type& to, const Type x, int exp, Rounding_Dir dir) {
  if (exp < 0)
    return mul2exp<To_Policy, From_Policy>(to, x, -exp, dir);
  if (static_cast<unsigned int>(exp) >= sizeof(Type) * 8) {
  zero:
    to = 0;
    if (round_ignore(dir))
      return V_LGE;
    if (x < 0)
      return round_lt_int_no_overflow<To_Policy>(to, dir);
    else if (x > 0)
      return round_gt_int_no_overflow<To_Policy>(to, dir);
    else
      return V_EQ;
  }
  if (static_cast<unsigned int>(exp) >= sizeof(Type) * 8 - 1) {
    if (x == Limits<Type>::min) {
      to = -1;
      return V_EQ;
    }
    goto zero;
  }
#if 0
  to = x / (static_cast<Type>(1) << exp);
  if (round_ignore(dir))
    return V_GE;
  Type r = x % (static_cast<Type>(1) << exp);
  if (r < 0)
    return round_lt_int_no_overflow<To_Policy>(to, dir);
  else if (r > 0)
    return round_gt_int_no_overflow<To_Policy>(to, dir);
  else
    return V_EQ;
#else
  // Faster but compiler implementation dependent (see C++98 5.8.3)
  to = x >> exp;
  if (round_ignore(dir))
    return V_GE;
  if (x & ((static_cast<Type>(1) << exp) - 1))
    return round_gt_int_no_overflow<To_Policy>(to, dir);
  return V_EQ;
#endif
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
mul2exp_unsigned_int(Type& to, const Type x, int exp, Rounding_Dir dir) {
  if (exp < 0)
    return div2exp<To_Policy, From_Policy>(to, x, -exp, dir);
  if (!To_Policy::check_overflow) {
    to = x << exp;
    return V_EQ;
  }
  if (static_cast<unsigned int>(exp) >= sizeof(Type) * 8) {
    if (x == 0) {
      to = 0;
      return V_EQ;
    }
    return set_pos_overflow_int<To_Policy>(to, dir);
  }
  if (x & (((static_cast<Type>(1) << exp) - 1) << (sizeof(Type) * 8 - exp)))
    return set_pos_overflow_int<To_Policy>(to, dir);
  Type n = x << exp;
  if (n > Extended_Int<To_Policy, Type>::max)
    return set_pos_overflow_int<To_Policy>(to, dir);
  to = n;
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
mul2exp_signed_int(Type& to, const Type x, int exp, Rounding_Dir dir) {
  if (exp < 0)
    return div2exp<To_Policy, From_Policy>(to, x, -exp, dir);
  if (!To_Policy::check_overflow) {
    to = x << exp;
    return V_EQ;
  }
  if (static_cast<unsigned int>(exp) >= sizeof(Type) * 8 - 1) {
    if (x < 0)
      return set_neg_overflow_int<To_Policy>(to, dir);
    else if (x > 0)
      return set_pos_overflow_int<To_Policy>(to, dir);
    else {
      to = 0;
      return V_EQ;
    }
  }
  Type mask = ((static_cast<Type>(1) << exp) - 1)
    << (sizeof(Type) * 8 - 1 - exp);
  Type n;
  if (x < 0) {
    if ((x & mask) != mask)
      return set_neg_overflow_int<To_Policy>(to, dir);
    n = x << exp;
    if (n < Extended_Int<To_Policy, Type>::min)
      return set_neg_overflow_int<To_Policy>(to, dir);
  }
  else {
    if (x & mask)
      return set_pos_overflow_int<To_Policy>(to, dir);
    n = x << exp;
    if (n > Extended_Int<To_Policy, Type>::max)
      return set_pos_overflow_int<To_Policy>(to, dir);
  }
  to = n;
  return V_EQ;
}

template <typename Type>
inline void
isqrtrem(Type& q, Type& r, const Type from) {
  q = 0;
  r = from;
  Type t(1);
  for (t <<= 8 * sizeof(Type) - 2; t != 0; t >>= 2) {
    Type s = q + t;
    if (s <= r) {
      r -= s;
      q = s + t;
    }
    q >>= 1;
  }
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
sqrt_unsigned_int(Type& to, const Type from, Rounding_Dir dir) {
  Type rem;
  isqrtrem(to, rem, from);
  if (round_ignore(dir))
    return V_GE;
  if (rem == 0)
    return V_EQ;
  return round_gt_int<To_Policy>(to, dir);
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
sqrt_signed_int(Type& to, const Type from, Rounding_Dir dir) {
  if (CHECK_P(To_Policy::check_sqrt_neg, from < 0))
    return set_special<To_Policy>(to, V_SQRT_NEG);
  return sqrt_unsigned_int<To_Policy, From_Policy>(to, from, dir);
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
add_mul_int(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  Type z;
  Result r = mul<To_Policy, From1_Policy, From2_Policy>(z, x, y, dir);
  switch (r) {
  case V_NEG_OVERFLOW:
  case V_LT:
    if (to <= 0) {
      to = z;
      return r;
    }
    return set_special<To_Policy>(to, V_UNKNOWN_NEG_OVERFLOW);
  case V_POS_OVERFLOW:
  case V_GT:
    if (to >= 0) {
      to = z;
      return r;
    }
    return set_special<To_Policy>(to, V_UNKNOWN_POS_OVERFLOW);
  default:
    return add<To_Policy, To_Policy, To_Policy>(to, to, z, dir);
  }
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
sub_mul_int(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  Type z;
  Result r = mul<To_Policy, From1_Policy, From2_Policy>(z, x, y, dir);
  switch (r) {
  case V_NEG_OVERFLOW:
  case V_LT:
    if (to >= 0)
      return set_pos_overflow_int<To_Policy>(to, dir);
    return V_UNKNOWN_NEG_OVERFLOW;
  case V_POS_OVERFLOW:
  case V_GT:
    if (to <= 0)
      return set_neg_overflow_int<To_Policy>(to, dir);
    return V_UNKNOWN_POS_OVERFLOW;
  default:
    return sub<To_Policy, To_Policy, To_Policy>(to, to, z, dir);
  }
}

template <typename Policy, typename Type>
inline Result
output_char(std::ostream& os, Type& from,
	    const Numeric_Format&, Rounding_Dir) {
  os << (int) from;
  return V_EQ;
}

template <typename Policy, typename Type>
inline Result
output_int(std::ostream& os, Type& from, const Numeric_Format&, Rounding_Dir) {
  os << from;
  return V_EQ;
}

SPECIALIZE_FLOOR(floor_int, signed char, signed char)
SPECIALIZE_FLOOR(floor_int, signed short, signed short)
SPECIALIZE_FLOOR(floor_int, signed int, signed int)
SPECIALIZE_FLOOR(floor_int, signed long, signed long)
SPECIALIZE_FLOOR(floor_int, signed long long, signed long long)
SPECIALIZE_FLOOR(floor_int, unsigned char, unsigned char)
SPECIALIZE_FLOOR(floor_int, unsigned short, unsigned short)
SPECIALIZE_FLOOR(floor_int, unsigned int, unsigned int)
SPECIALIZE_FLOOR(floor_int, unsigned long, unsigned long)
SPECIALIZE_FLOOR(floor_int, unsigned long long, unsigned long long)

SPECIALIZE_CEIL(ceil_int, signed char, signed char)
SPECIALIZE_CEIL(ceil_int, signed short, signed short)
SPECIALIZE_CEIL(ceil_int, signed int, signed int)
SPECIALIZE_CEIL(ceil_int, signed long, signed long)
SPECIALIZE_CEIL(ceil_int, signed long long, signed long long)
SPECIALIZE_CEIL(ceil_int, unsigned char, unsigned char)
SPECIALIZE_CEIL(ceil_int, unsigned short, unsigned short)
SPECIALIZE_CEIL(ceil_int, unsigned int, unsigned int)
SPECIALIZE_CEIL(ceil_int, unsigned long, unsigned long)
SPECIALIZE_CEIL(ceil_int, unsigned long long, unsigned long long)

SPECIALIZE_TRUNC(trunc_int, signed char, signed char)
SPECIALIZE_TRUNC(trunc_int, signed short, signed short)
SPECIALIZE_TRUNC(trunc_int, signed int, signed int)
SPECIALIZE_TRUNC(trunc_int, signed long, signed long)
SPECIALIZE_TRUNC(trunc_int, signed long long, signed long long)
SPECIALIZE_TRUNC(trunc_int, unsigned char, unsigned char)
SPECIALIZE_TRUNC(trunc_int, unsigned short, unsigned short)
SPECIALIZE_TRUNC(trunc_int, unsigned int, unsigned int)
SPECIALIZE_TRUNC(trunc_int, unsigned long, unsigned long)
SPECIALIZE_TRUNC(trunc_int, unsigned long long, unsigned long long)

SPECIALIZE_NEG(neg_signed_int, signed char, signed char)
SPECIALIZE_NEG(neg_signed_int, signed short, signed short)
SPECIALIZE_NEG(neg_signed_int, signed int, signed int)
SPECIALIZE_NEG(neg_signed_int, signed long, signed long)
SPECIALIZE_NEG(neg_signed_int, signed long long, signed long long)
SPECIALIZE_NEG(neg_unsigned_int, unsigned char, unsigned char)
SPECIALIZE_NEG(neg_unsigned_int, unsigned short, unsigned short)
SPECIALIZE_NEG(neg_unsigned_int, unsigned int, unsigned int)
SPECIALIZE_NEG(neg_unsigned_int, unsigned long, unsigned long)
SPECIALIZE_NEG(neg_unsigned_int, unsigned long long, unsigned long long)

SPECIALIZE_ADD(add_signed_int, signed char, signed char, signed char)
SPECIALIZE_ADD(add_signed_int, signed short, signed short, signed short)
SPECIALIZE_ADD(add_signed_int, signed int, signed int, signed int)
SPECIALIZE_ADD(add_signed_int, signed long, signed long, signed long)
SPECIALIZE_ADD(add_signed_int, signed long long, signed long long, signed long long)
SPECIALIZE_ADD(add_unsigned_int, unsigned char, unsigned char, unsigned char)
SPECIALIZE_ADD(add_unsigned_int, unsigned short, unsigned short, unsigned short)
SPECIALIZE_ADD(add_unsigned_int, unsigned int, unsigned int, unsigned int)
SPECIALIZE_ADD(add_unsigned_int, unsigned long, unsigned long, unsigned long)
SPECIALIZE_ADD(add_unsigned_int, unsigned long long, unsigned long long, unsigned long long)

SPECIALIZE_SUB(sub_signed_int, signed char, signed char, signed char)
SPECIALIZE_SUB(sub_signed_int, signed short, signed short, signed short)
SPECIALIZE_SUB(sub_signed_int, signed int, signed int, signed int)
SPECIALIZE_SUB(sub_signed_int, signed long, signed long, signed long)
SPECIALIZE_SUB(sub_signed_int, signed long long, signed long long, signed long long)
SPECIALIZE_SUB(sub_unsigned_int, unsigned char, unsigned char, unsigned char)
SPECIALIZE_SUB(sub_unsigned_int, unsigned short, unsigned short, unsigned short)
SPECIALIZE_SUB(sub_unsigned_int, unsigned int, unsigned int, unsigned int)
SPECIALIZE_SUB(sub_unsigned_int, unsigned long, unsigned long, unsigned long)
SPECIALIZE_SUB(sub_unsigned_int, unsigned long long, unsigned long long, unsigned long long)

SPECIALIZE_MUL(mul_signed_int, signed char, signed char, signed char)
SPECIALIZE_MUL(mul_signed_int, signed short, signed short, signed short)
SPECIALIZE_MUL(mul_signed_int, signed int, signed int, signed int)
SPECIALIZE_MUL(mul_signed_int, signed long, signed long, signed long)
SPECIALIZE_MUL(mul_signed_int, signed long long, signed long long, signed long long)
SPECIALIZE_MUL(mul_unsigned_int, unsigned char, unsigned char, unsigned char)
SPECIALIZE_MUL(mul_unsigned_int, unsigned short, unsigned short, unsigned short)
SPECIALIZE_MUL(mul_unsigned_int, unsigned int, unsigned int, unsigned int)
SPECIALIZE_MUL(mul_unsigned_int, unsigned long, unsigned long, unsigned long)
SPECIALIZE_MUL(mul_unsigned_int, unsigned long long, unsigned long long, unsigned long long)

SPECIALIZE_DIV(div_signed_int, signed char, signed char, signed char)
SPECIALIZE_DIV(div_signed_int, signed short, signed short, signed short)
SPECIALIZE_DIV(div_signed_int, signed int, signed int, signed int)
SPECIALIZE_DIV(div_signed_int, signed long, signed long, signed long)
SPECIALIZE_DIV(div_signed_int, signed long long, signed long long, signed long long)
SPECIALIZE_DIV(div_unsigned_int, unsigned char, unsigned char, unsigned char)
SPECIALIZE_DIV(div_unsigned_int, unsigned short, unsigned short, unsigned short)
SPECIALIZE_DIV(div_unsigned_int, unsigned int, unsigned int, unsigned int)
SPECIALIZE_DIV(div_unsigned_int, unsigned long, unsigned long, unsigned long)
SPECIALIZE_DIV(div_unsigned_int, unsigned long long, unsigned long long, unsigned long long)

SPECIALIZE_REM(rem_int, signed char, signed char, signed char)
SPECIALIZE_REM(rem_int, signed short, signed short, signed short)
SPECIALIZE_REM(rem_int, signed int, signed int, signed int)
SPECIALIZE_REM(rem_int, signed long, signed long, signed long)
SPECIALIZE_REM(rem_int, signed long long, signed long long, signed long long)
SPECIALIZE_REM(rem_int, unsigned char, unsigned char, unsigned char)
SPECIALIZE_REM(rem_int, unsigned short, unsigned short, unsigned short)
SPECIALIZE_REM(rem_int, unsigned int, unsigned int, unsigned int)
SPECIALIZE_REM(rem_int, unsigned long, unsigned long, unsigned long)
SPECIALIZE_REM(rem_int, unsigned long long, unsigned long long, unsigned long long)

SPECIALIZE_MUL2EXP(mul2exp_signed_int, signed char, signed char)
SPECIALIZE_MUL2EXP(mul2exp_signed_int, signed short, signed short)
SPECIALIZE_MUL2EXP(mul2exp_signed_int, signed int, signed int)
SPECIALIZE_MUL2EXP(mul2exp_signed_int, signed long, signed long)
SPECIALIZE_MUL2EXP(mul2exp_signed_int, signed long long, signed long long)
SPECIALIZE_MUL2EXP(mul2exp_unsigned_int, unsigned char, unsigned char)
SPECIALIZE_MUL2EXP(mul2exp_unsigned_int, unsigned short, unsigned short)
SPECIALIZE_MUL2EXP(mul2exp_unsigned_int, unsigned int, unsigned int)
SPECIALIZE_MUL2EXP(mul2exp_unsigned_int, unsigned long, unsigned long)
SPECIALIZE_MUL2EXP(mul2exp_unsigned_int, unsigned long long, unsigned long long)

SPECIALIZE_DIV2EXP(div2exp_signed_int, signed char, signed char)
SPECIALIZE_DIV2EXP(div2exp_signed_int, signed short, signed short)
SPECIALIZE_DIV2EXP(div2exp_signed_int, signed int, signed int)
SPECIALIZE_DIV2EXP(div2exp_signed_int, signed long, signed long)
SPECIALIZE_DIV2EXP(div2exp_signed_int, signed long long, signed long long)
SPECIALIZE_DIV2EXP(div2exp_unsigned_int, unsigned char, unsigned char)
SPECIALIZE_DIV2EXP(div2exp_unsigned_int, unsigned short, unsigned short)
SPECIALIZE_DIV2EXP(div2exp_unsigned_int, unsigned int, unsigned int)
SPECIALIZE_DIV2EXP(div2exp_unsigned_int, unsigned long, unsigned long)
SPECIALIZE_DIV2EXP(div2exp_unsigned_int, unsigned long long, unsigned long long)

SPECIALIZE_SQRT(sqrt_signed_int, signed char, signed char)
SPECIALIZE_SQRT(sqrt_signed_int, signed short, signed short)
SPECIALIZE_SQRT(sqrt_signed_int, signed int, signed int)
SPECIALIZE_SQRT(sqrt_signed_int, signed long, signed long)
SPECIALIZE_SQRT(sqrt_signed_int, signed long long, signed long long)
SPECIALIZE_SQRT(sqrt_unsigned_int, unsigned char, unsigned char)
SPECIALIZE_SQRT(sqrt_unsigned_int, unsigned short, unsigned short)
SPECIALIZE_SQRT(sqrt_unsigned_int, unsigned int, unsigned int)
SPECIALIZE_SQRT(sqrt_unsigned_int, unsigned long, unsigned long)
SPECIALIZE_SQRT(sqrt_unsigned_int, unsigned long long, unsigned long long)

SPECIALIZE_ABS(abs_generic, signed char, signed char)
SPECIALIZE_ABS(abs_generic, signed short, signed short)
SPECIALIZE_ABS(abs_generic, signed int, signed int)
SPECIALIZE_ABS(abs_generic, signed long, signed long)
SPECIALIZE_ABS(abs_generic, signed long long, signed long long)
SPECIALIZE_ABS(abs_generic, unsigned char, unsigned char)
SPECIALIZE_ABS(abs_generic, unsigned short, unsigned short)
SPECIALIZE_ABS(abs_generic, unsigned int, unsigned int)
SPECIALIZE_ABS(abs_generic, unsigned long, unsigned long)
SPECIALIZE_ABS(abs_generic, unsigned long long, unsigned long long)

SPECIALIZE_GCD(gcd_exact, signed char, signed char, signed char)
SPECIALIZE_GCD(gcd_exact, signed short, signed short, signed short)
SPECIALIZE_GCD(gcd_exact, signed int, signed int, signed int)
SPECIALIZE_GCD(gcd_exact, signed long, signed long, signed long)
SPECIALIZE_GCD(gcd_exact, signed long long, signed long long, signed long long)
SPECIALIZE_GCD(gcd_exact, unsigned char, unsigned char, unsigned char)
SPECIALIZE_GCD(gcd_exact, unsigned short, unsigned short, unsigned short)
SPECIALIZE_GCD(gcd_exact, unsigned int, unsigned int, unsigned int)
SPECIALIZE_GCD(gcd_exact, unsigned long, unsigned long, unsigned long)
SPECIALIZE_GCD(gcd_exact, unsigned long long, unsigned long long, unsigned long long)

SPECIALIZE_GCDEXT(gcdext_exact, signed char, signed char, signed char, signed char, signed char)
SPECIALIZE_GCDEXT(gcdext_exact, signed short, signed short, signed short, signed short, signed short)
SPECIALIZE_GCDEXT(gcdext_exact, signed int, signed int, signed int, signed int, signed int)
SPECIALIZE_GCDEXT(gcdext_exact, signed long, signed long, signed long, signed long, signed long)
SPECIALIZE_GCDEXT(gcdext_exact, signed long long, signed long long, signed long long, signed long long, signed long long)
SPECIALIZE_GCDEXT(gcdext_exact, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char)
SPECIALIZE_GCDEXT(gcdext_exact, unsigned short, unsigned short, unsigned short, unsigned short, unsigned short)
SPECIALIZE_GCDEXT(gcdext_exact, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int)
SPECIALIZE_GCDEXT(gcdext_exact, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
SPECIALIZE_GCDEXT(gcdext_exact, unsigned long long, unsigned long long, unsigned long long, unsigned long long, unsigned long long)

SPECIALIZE_LCM(lcm_gcd_exact, signed char, signed char, signed char)
SPECIALIZE_LCM(lcm_gcd_exact, signed short, signed short, signed short)
SPECIALIZE_LCM(lcm_gcd_exact, signed int, signed int, signed int)
SPECIALIZE_LCM(lcm_gcd_exact, signed long, signed long, signed long)
SPECIALIZE_LCM(lcm_gcd_exact, signed long long, signed long long, signed long long)
SPECIALIZE_LCM(lcm_gcd_exact, unsigned char, unsigned char, unsigned char)
SPECIALIZE_LCM(lcm_gcd_exact, unsigned short, unsigned short, unsigned short)
SPECIALIZE_LCM(lcm_gcd_exact, unsigned int, unsigned int, unsigned int)
SPECIALIZE_LCM(lcm_gcd_exact, unsigned long, unsigned long, unsigned long)
SPECIALIZE_LCM(lcm_gcd_exact, unsigned long long, unsigned long long, unsigned long long)

SPECIALIZE_SGN(sgn_generic, signed char)
SPECIALIZE_SGN(sgn_generic, signed short)
SPECIALIZE_SGN(sgn_generic, signed int)
SPECIALIZE_SGN(sgn_generic, signed long)
SPECIALIZE_SGN(sgn_generic, signed long long)
SPECIALIZE_SGN(sgn_generic, unsigned char)
SPECIALIZE_SGN(sgn_generic, unsigned short)
SPECIALIZE_SGN(sgn_generic, unsigned int)
SPECIALIZE_SGN(sgn_generic, unsigned long)
SPECIALIZE_SGN(sgn_generic, unsigned long long)

SPECIALIZE_CMP(cmp_generic, signed char, signed char)
SPECIALIZE_CMP(cmp_generic, signed short, signed short)
SPECIALIZE_CMP(cmp_generic, signed int, signed int)
SPECIALIZE_CMP(cmp_generic, signed long, signed long)
SPECIALIZE_CMP(cmp_generic, signed long long, signed long long)
SPECIALIZE_CMP(cmp_generic, unsigned char, unsigned char)
SPECIALIZE_CMP(cmp_generic, unsigned short, unsigned short)
SPECIALIZE_CMP(cmp_generic, unsigned int, unsigned int)
SPECIALIZE_CMP(cmp_generic, unsigned long, unsigned long)
SPECIALIZE_CMP(cmp_generic, unsigned long long, unsigned long long)

SPECIALIZE_ADD_MUL(add_mul_int, signed char, signed char, signed char)
SPECIALIZE_ADD_MUL(add_mul_int, signed short, signed short, signed short)
SPECIALIZE_ADD_MUL(add_mul_int, signed int, signed int, signed int)
SPECIALIZE_ADD_MUL(add_mul_int, signed long, signed long, signed long)
SPECIALIZE_ADD_MUL(add_mul_int, signed long long, signed long long, signed long long)
SPECIALIZE_ADD_MUL(add_mul_int, unsigned char, unsigned char, unsigned char)
SPECIALIZE_ADD_MUL(add_mul_int, unsigned short, unsigned short, unsigned short)
SPECIALIZE_ADD_MUL(add_mul_int, unsigned int, unsigned int, unsigned int)
SPECIALIZE_ADD_MUL(add_mul_int, unsigned long, unsigned long, unsigned long)
SPECIALIZE_ADD_MUL(add_mul_int, unsigned long long, unsigned long long, unsigned long long)

SPECIALIZE_SUB_MUL(sub_mul_int, signed char, signed char, signed char)
SPECIALIZE_SUB_MUL(sub_mul_int, signed short, signed short, signed short)
SPECIALIZE_SUB_MUL(sub_mul_int, signed int, signed int, signed int)
SPECIALIZE_SUB_MUL(sub_mul_int, signed long, signed long, signed long)
SPECIALIZE_SUB_MUL(sub_mul_int, signed long long, signed long long, signed long long)
SPECIALIZE_SUB_MUL(sub_mul_int, unsigned char, unsigned char, unsigned char)
SPECIALIZE_SUB_MUL(sub_mul_int, unsigned short, unsigned short, unsigned short)
SPECIALIZE_SUB_MUL(sub_mul_int, unsigned int, unsigned int, unsigned int)
SPECIALIZE_SUB_MUL(sub_mul_int, unsigned long, unsigned long, unsigned long)
SPECIALIZE_SUB_MUL(sub_mul_int, unsigned long long, unsigned long long, unsigned long long)

SPECIALIZE_INPUT(input_generic, signed char)
SPECIALIZE_INPUT(input_generic, signed short)
SPECIALIZE_INPUT(input_generic, signed int)
SPECIALIZE_INPUT(input_generic, signed long)
SPECIALIZE_INPUT(input_generic, signed long long)
SPECIALIZE_INPUT(input_generic, unsigned char)
SPECIALIZE_INPUT(input_generic, unsigned short)
SPECIALIZE_INPUT(input_generic, unsigned int)
SPECIALIZE_INPUT(input_generic, unsigned long)
SPECIALIZE_INPUT(input_generic, unsigned long long)

SPECIALIZE_OUTPUT(output_char, signed char)
SPECIALIZE_OUTPUT(output_int, signed short)
SPECIALIZE_OUTPUT(output_int, signed int)
SPECIALIZE_OUTPUT(output_int, signed long)
SPECIALIZE_OUTPUT(output_int, signed long long)
SPECIALIZE_OUTPUT(output_char, unsigned char)
SPECIALIZE_OUTPUT(output_int, unsigned short)
SPECIALIZE_OUTPUT(output_int, unsigned int)
SPECIALIZE_OUTPUT(output_int, unsigned long)
SPECIALIZE_OUTPUT(output_int, unsigned long long)

} // namespace Checked

} // namespace Parma_Polyhedra_Library

#endif // !defined(PPL_checked_int_inlines_hh)
