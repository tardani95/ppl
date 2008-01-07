/* Specialized "checked" functions for native floating-point numbers.
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

#ifndef PPL_checked_float_inlines_hh
#define PPL_checked_float_inlines_hh 1

#ifndef __alpha
#include <cmath>
#endif

namespace Parma_Polyhedra_Library {

#if PPL_SUPPORTED_FLOAT
template <> struct Is_Native<float> : public True { };
#endif
#if PPL_SUPPORTED_DOUBLE
template <> struct Is_Native<double> : public True { };
#endif
#if PPL_SUPPORTED_LONG_DOUBLE
template <> struct Is_Native<long double> : public True { };
#endif

namespace Checked {

inline float
fma(float x, float y, float z) {
#if PPL_HAVE_DECL_FMAF && !defined(__alpha)
  return ::fmaf(x, y, z);
#else
  return x*y + z;
#endif
}

inline double
fma(double x, double y, double z) {
#if PPL_HAVE_DECL_FMA && !defined(__alpha)
  return ::fma(x, y, z);
#else
  return x*y + z;
#endif
}

inline long double
fma(long double x, long double y, long double z) {
#if PPL_HAVE_DECL_FMAL && !defined(__alpha)
  return ::fmal(x, y, z);
#else
  return x*y + z;
#endif
}

#if PPL_HAVE_DECL_RINTF
inline float
rint(float x) {
  return ::rintf(x);
}
#endif

inline double
rint(double x) {
  return ::rint(x);
}

#if PPL_HAVE_DECL_RINTL
inline long double
rint(long double x) {
  return ::rintl(x);
}
#elif !PPL_CXX_PROVIDES_PROPER_LONG_DOUBLE
// If proper long doubles are not provided, this is most likely
// because long double and double are the same type: use rint().
inline long double
rint(long double x) {
  return ::rint(x);
}
#endif

inline bool
fpu_direct_rounding(Rounding_Dir dir) {
  return round_direct(dir) || round_ignore(dir);
}

inline bool
fpu_inverse_rounding(Rounding_Dir dir) {
  return round_inverse(dir);
}

// The FPU mode is "round down".
//
// The result of the rounded down multiplication is thus computed directly.
//
//   a = 0.3
//   b = 0.1
//   c_i = a * b = 0.03
//   c = c_i = 0.0
//
// To obtain the result of the rounded up multiplication
// we do -(-a * b).
//
//   a = 0.3
//   b = 0.1
//   c_i = -a * b = -0.03
//
// Here c_i should be forced to lose excess precision, otherwise the
// FPU will truncate using the rounding mode in force, which is "round down".
//
//   c_i = -c_i = 0.03
//   c = c_i = 0.0
//
// Wrong result: we should have obtained c = 0.1.

inline float
limit_precision(float v) {
  float x = v;
  avoid_cse(x);
  return x;
}

inline double
limit_precision(double v) {
  double x = v;
  avoid_cse(x);
  return x;
}

inline long double
limit_precision(long double v) {
  return v;
}

template <typename Policy, typename T>
inline Result
classify_float(const T v, bool nan, bool inf, bool sign) {
  Float<T> f(v);
  if ((nan || sign) && CHECK_P(Policy::has_nan, f.u.binary.is_nan()))
    return VC_NAN;
  if (inf) {
    int i = CHECK_P(Policy::has_infinity, f.u.binary.is_inf());
    if (i < 0)
      return VC_MINUS_INFINITY;
    if (i > 0)
      return VC_PLUS_INFINITY;
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

template <typename Policy, typename T>
inline bool
is_nan_float(const T v) {
  Float<T> f(v);
  return CHECK_P(Policy::has_nan, f.u.binary.is_nan());
}

template <typename Policy, typename T>
inline int
is_inf_float(const T v) {
  Float<T> f(v);
  return CHECK_P(Policy::has_infinity, f.u.binary.is_inf());
}
template <typename Policy, typename T>
inline bool
is_minf_float(const T v) {
  return is_inf_float<Policy>(v) < 0;
}

template <typename Policy, typename T>
inline bool
is_pinf_float(const T v) {
  return is_inf_float<Policy>(v) > 0;
}


template <typename Policy, typename T>
inline bool
is_int_float(const T v) {
  return rint(v) == v;
}

template <typename Policy, typename T>
inline Result
set_special_float(T& v, Result r) {
  switch (classify(r)) {
  case VC_MINUS_INFINITY:
    v = -HUGE_VAL;
    break;
  case VC_PLUS_INFINITY:
    v = HUGE_VAL;
    break;
  case VC_NAN:
    v = NAN;
    break;
  default:
    break;
  }
  return r;
}

template <typename T>
inline void
pred_float(T& v) {
  Float<T> f(v);
  assert(!f.u.binary.is_nan());
  assert(f.u.binary.is_inf() >= 0);
  if (f.u.binary.is_zero() > 0) {
    f.u.binary.negate();
    f.u.binary.inc();
  }
  else if (f.u.binary.sign_bit()) {
    f.u.binary.inc();
  }
  else {
    f.u.binary.dec();
  }
  v = f.value();
}

template <typename T>
inline void
succ_float(T& v) {
  Float<T> f(v);
  assert(!f.u.binary.is_nan());
  assert(f.u.binary.is_inf() <= 0);
  if (f.u.binary.is_zero() < 0) {
    f.u.binary.negate();
    f.u.binary.inc();
  }
  else if (!f.u.binary.sign_bit()) {
    f.u.binary.inc();
  }
  else {
    f.u.binary.dec();
  }
  v = f.value();
}

template <typename Policy, typename To>
inline Result
round_lt_float(To& to, Rounding_Dir dir) {
  if (round_down(dir)) {
    pred_float(to);
    return V_GT;
  }
  return V_LT;
}

template <typename Policy, typename To>
inline Result
round_gt_float(To& to, Rounding_Dir dir) {
  if (round_up(dir)) {
    succ_float(to);
    return V_LT;
  }
  return V_GT;
}


template <typename Policy>
inline void
prepare_inexact(Rounding_Dir dir) {
  if (Policy::fpu_check_inexact && round_fpu_check_inexact(dir))
    fpu_reset_inexact();
}

template <typename Policy>
inline Result
result_relation(Rounding_Dir dir) {
  if (Policy::fpu_check_inexact && round_fpu_check_inexact(dir)) {
    switch (fpu_check_inexact()) {
    case 0:
      return V_EQ;
    case -1:
      goto unknown;
    case 1:
      break;
    }
    switch (round_dir(dir)) {
    case ROUND_DOWN:
      return V_GT;
    case ROUND_UP:
      return V_LT;
    default:
      return V_NE;
    }
  }
  else {
  unknown:
    switch (round_dir(dir)) {
    case ROUND_DOWN:
      return V_GE;
    case ROUND_UP:
      return V_LE;
    default:
      return V_LGE;
    }
  }
}

template <typename To_Policy, typename From_Policy, typename To, typename From>
inline Result
assign_float_float_exact(To& to, const From from, Rounding_Dir) {
  if (To_Policy::check_nan_result && is_nan<From_Policy>(from))
    return set_special<To_Policy>(to, VC_NAN);
  to = from;
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename To, typename From>
inline Result
assign_float_float_inexact(To& to, const From from, Rounding_Dir dir) {
  if (To_Policy::check_nan_result && is_nan<From_Policy>(from))
    return set_special<To_Policy>(to, VC_NAN);
  prepare_inexact<To_Policy>(dir);
  if (fpu_direct_rounding(dir))
    to = from;
  else if (fpu_inverse_rounding(dir))
    to = -static_cast<To>(-from);
  else {
    fpu_rounding_control_word_type old = fpu_save_rounding_direction(round_fpu_dir(dir));
    avoid_cse(from);
    to = from;
    avoid_cse(to);
    fpu_restore_rounding_direction(old);
  }
  return result_relation<To_Policy>(dir);
}

template <typename To_Policy, typename From_Policy, typename To, typename From>
inline Result
assign_float_float(To& to, const From from, Rounding_Dir dir) {
  if (sizeof(From) > sizeof(To))
    return assign_float_float_inexact<To_Policy, From_Policy>(to, from, dir);
  else
    return assign_float_float_exact<To_Policy, From_Policy>(to, from, dir);
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
floor_float(Type& to, const Type from, Rounding_Dir) {
  if (To_Policy::check_nan_result && is_nan<From_Policy>(from))
    return set_special<To_Policy>(to, VC_NAN);
  if (fpu_direct_rounding(ROUND_DOWN))
    to = rint(from);
  else if (fpu_inverse_rounding(ROUND_DOWN))
    to = -limit_precision(rint(-from));
  else {
    fpu_rounding_control_word_type old = fpu_save_rounding_direction(round_fpu_dir(ROUND_DOWN));
    avoid_cse(from);
    to = rint(from);
    avoid_cse(to);
    fpu_restore_rounding_direction(old);
  }
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
ceil_float(Type& to, const Type from, Rounding_Dir) {
  if (To_Policy::check_nan_result && is_nan<From_Policy>(from))
    return set_special<To_Policy>(to, VC_NAN);
  if (fpu_direct_rounding(ROUND_UP))
    to = rint(from);
  else if (fpu_inverse_rounding(ROUND_UP))
    to = -limit_precision(rint(-from));
  else {
    fpu_rounding_control_word_type old = fpu_save_rounding_direction(round_fpu_dir(ROUND_UP));
    avoid_cse(from);
    to = rint(from);
    avoid_cse(to);
    fpu_restore_rounding_direction(old);
  }
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
trunc_float(Type& to, const Type from, Rounding_Dir dir) {
  if (To_Policy::check_nan_result && is_nan<From_Policy>(from))
    return set_special<To_Policy>(to, VC_NAN);
  if (from >= 0)
    return floor<To_Policy, From_Policy>(to, from, dir);
  else
    return ceil<To_Policy, From_Policy>(to, from, dir);
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
neg_float(Type& to, const Type from, Rounding_Dir) {
  if (To_Policy::check_nan_result && is_nan<From_Policy>(from))
    return set_special<To_Policy>(to, VC_NAN);
  to = -from;
  return V_EQ;
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy,
	  typename Type>
inline Result
add_float(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_inf_add_inf
      && is_inf_float<From1_Policy>(x) && x == -y)
    return set_special<To_Policy>(to, V_INF_ADD_INF);
  prepare_inexact<To_Policy>(dir);
  if (fpu_direct_rounding(dir))
    to = x + y;
  else if (fpu_inverse_rounding(dir))
    to = -limit_precision(-x - y);
  else {
    fpu_rounding_control_word_type old = fpu_save_rounding_direction(round_fpu_dir(dir));
    avoid_cse(x);
    avoid_cse(y);
    to = x + y;
    avoid_cse(to);
    fpu_restore_rounding_direction(old);
  }
  if (To_Policy::check_nan_result && is_nan<To_Policy>(to))
    return VC_NAN;
  return result_relation<To_Policy>(dir);
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy,
	  typename Type>
inline Result
sub_float(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_inf_sub_inf
      && is_inf_float<From1_Policy>(x) && x == y)
    return set_special<To_Policy>(to, V_INF_SUB_INF);
  prepare_inexact<To_Policy>(dir);
  if (fpu_direct_rounding(dir))
    to = x - y;
  else if (fpu_inverse_rounding(dir))
    to = -limit_precision(y - x);
  else {
    fpu_rounding_control_word_type old = fpu_save_rounding_direction(round_fpu_dir(dir));
    avoid_cse(x);
    avoid_cse(y);
    to = x - y;
    avoid_cse(to);
    fpu_restore_rounding_direction(old);
  }
  if (To_Policy::check_nan_result && is_nan<To_Policy>(to))
    return VC_NAN;
  return result_relation<To_Policy>(dir);
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy,
	  typename Type>
inline Result
mul_float(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_inf_mul_zero
      && ((x == 0 && is_inf_float<From2_Policy>(y)) ||
	  (y == 0 && is_inf_float<From1_Policy>(x))))
    return set_special<To_Policy>(to, V_INF_MUL_ZERO);
  prepare_inexact<To_Policy>(dir);
  if (fpu_direct_rounding(dir))
    to = x * y;
  else if (fpu_inverse_rounding(dir))
    to = -limit_precision(x * -y);
  else {
    fpu_rounding_control_word_type old = fpu_save_rounding_direction(round_fpu_dir(dir));
    avoid_cse(x);
    avoid_cse(y);
    to = x * y;
    avoid_cse(to);
    fpu_restore_rounding_direction(old);
  }
  if (To_Policy::check_nan_result && is_nan<To_Policy>(to))
    return VC_NAN;
  return result_relation<To_Policy>(dir);
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy,
	  typename Type>
inline Result
div_float(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_inf_div_inf
      && is_inf_float<From1_Policy>(x) && is_inf_float<From2_Policy>(y))
    return set_special<To_Policy>(to, V_INF_DIV_INF);
  if (To_Policy::check_div_zero && y == 0)
    return set_special<To_Policy>(to, V_DIV_ZERO);
  prepare_inexact<To_Policy>(dir);
  if (fpu_direct_rounding(dir))
    to = x / y;
  else if (fpu_inverse_rounding(dir))
    to = -limit_precision(x / -y);
  else {
    fpu_rounding_control_word_type old = fpu_save_rounding_direction(round_fpu_dir(dir));
    avoid_cse(x);
    avoid_cse(y);
    to = x / y;
    avoid_cse(to);
    fpu_restore_rounding_direction(old);
  }
  if (To_Policy::check_nan_result && is_nan<To_Policy>(to))
    return VC_NAN;
  return result_relation<To_Policy>(dir);
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy,
	  typename Type>
inline Result
idiv_float(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  Type temp;
  // The inexact check is useless
  dir = round_dir(dir);
  Result r = div<To_Policy, From1_Policy, From2_Policy>(temp, x, y, dir);
  if (is_special(r)) {
    to = temp;
    return r;
  }
  Result r1 = trunc<To_Policy, To_Policy>(to, temp, ROUND_NOT_NEEDED);
  assert(r1 == V_EQ);
  if (r == V_EQ || to != temp)
    return r1;
  // FIXME: Prove that it's impossibile to return a strict relation
  return dir == ROUND_UP ? V_LE : V_GE;
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy,
	  typename Type>
inline Result
rem_float(Type& to, const Type x, const Type y, Rounding_Dir) {
  if (To_Policy::check_inf_mod && is_inf_float<From1_Policy>(x))
    return set_special<To_Policy>(to, V_INF_MOD);
  if (To_Policy::check_div_zero && y == 0)
    return set_special<To_Policy>(to, V_MOD_ZERO);
  to = std::fmod(x, y);
  if (To_Policy::check_nan_result && is_nan<To_Policy>(to))
    return VC_NAN;
  return V_EQ;
}

struct Float_2exp {
  const_bool_nodef(has_nan, false);
  const_bool_nodef(has_infinity, false);
};

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
mul2exp_float(Type& to, const Type x, int exp, Rounding_Dir dir) {
  if (To_Policy::check_nan_result && is_nan<From_Policy>(x))
    return set_special<To_Policy>(to, VC_NAN);
  if (exp < 0)
    return div2exp<To_Policy, From_Policy>(to, x, -exp, dir);
  assert(static_cast<unsigned int>(exp) < sizeof(unsigned long long) * 8);
  return mul<To_Policy, From_Policy, Float_2exp>(to, x, static_cast<Type>(1ULL << exp), dir);
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
div2exp_float(Type& to, const Type x, int exp, Rounding_Dir dir) {
  if (To_Policy::check_nan_result && is_nan<From_Policy>(x))
    return set_special<To_Policy>(to, VC_NAN);
  if (exp < 0)
    return mul2exp<To_Policy, From_Policy>(to, x, -exp, dir);
  assert(static_cast<unsigned int>(exp) < sizeof(unsigned long long) * 8);
  return div<To_Policy, From_Policy, Float_2exp>(to, x, static_cast<Type>(1ULL << exp), dir);
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
abs_float(Type& to, const Type from, Rounding_Dir) {
  if (To_Policy::check_nan_result && is_nan<From_Policy>(from))
    return set_special<To_Policy>(to, VC_NAN);
  to = std::abs(from);
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename Type>
inline Result
sqrt_float(Type& to, const Type from, Rounding_Dir dir) {
  if (To_Policy::check_nan_result && is_nan<From_Policy>(from))
    return set_special<To_Policy>(to, VC_NAN);
  if (To_Policy::check_sqrt_neg && from < 0)
    return set_special<To_Policy>(to, V_SQRT_NEG);
  prepare_inexact<To_Policy>(dir);
  if (fpu_direct_rounding(dir))
    to = std::sqrt(from);
  else {
    fpu_rounding_control_word_type old = fpu_save_rounding_direction(round_fpu_dir(dir));
    avoid_cse(from);
    to = std::sqrt(from);
    avoid_cse(to);
    fpu_restore_rounding_direction(old);
  }
  return result_relation<To_Policy>(dir);
}

template <typename Policy, typename Type>
inline Result
sgn_float(const Type x) {
  return classify<Policy>(x, false, false, true);
}

template <typename Policy1, typename Policy2, typename Type>
inline Result
cmp_float(const Type x, const Type y) {
  if (x > y)
    return V_GT;
  if (x < y)
    return V_LT;
  if (x == y)
    return V_EQ;
  return V_UNORD_COMP;
}

template <typename To_Policy, typename From_Policy, typename To, typename From>
inline Result
assign_float_int_inexact(To& to, const From from, Rounding_Dir dir) {
  prepare_inexact<To_Policy>(dir);
  if (fpu_direct_rounding(dir))
    to = from;
  else {
    fpu_rounding_control_word_type old = fpu_save_rounding_direction(round_fpu_dir(dir));
    avoid_cse(from);
    to = from;
    avoid_cse(to);
    fpu_restore_rounding_direction(old);
  }
  return result_relation<To_Policy>(dir);
}

template <typename To_Policy, typename From_Policy, typename To, typename From>
inline Result
assign_float_int(To& to, const From from, Rounding_Dir dir) {
  if (sizeof(From) * 8 > Float<To>::Binary::MANTISSA_BITS)
    return assign_float_int_inexact<To_Policy, From_Policy>(to, from, dir);
  else
    return assign_exact<To_Policy, From_Policy>(to, from, dir);
}

template <typename Policy, typename T>
inline Result
set_neg_overflow_float(T& to, Rounding_Dir dir) {
  switch (round_dir(dir)) {
  case ROUND_UP:
    {
      Float<T> f;
      f.u.binary.set_max(true);
      to = f.value();
      return V_LT;
    }
  default:
    to = -HUGE_VAL;
    return V_GT;
  }
}

template <typename Policy, typename T>
inline Result
set_pos_overflow_float(T& to, Rounding_Dir dir) {
  switch (round_dir(dir)) {
  case ROUND_DOWN:
    {
      Float<T> f;
      f.u.binary.set_max(false);
      to = f.value();
      return V_GT;
    }
  default:
    to = HUGE_VAL;
    return V_LT;
  }
}

template <typename To_Policy, typename From_Policy, typename T>
inline Result
assign_float_mpz(T& to, const mpz_class& _from, Rounding_Dir dir)
{
  mpz_srcptr from = _from.get_mpz_t();
  int sign = mpz_sgn(from);
  if (sign == 0) {
    to = 0;
    return V_EQ;
  }
  size_t exponent = mpz_sizeinbase(from, 2) - 1;
  if (exponent > static_cast<size_t>(Float<T>::Binary::EXPONENT_MAX)) {
    if (sign < 0)
      return set_neg_overflow_float<To_Policy>(to, dir);
    else
      return set_pos_overflow_float<To_Policy>(to, dir);
  }
  unsigned long zeroes = mpn_scan1(from->_mp_d, 0);
  size_t meaningful_bits = exponent - zeroes;
  mpz_t mantissa;
  mpz_init(mantissa);
  if (exponent > Float<T>::Binary::MANTISSA_BITS)
    mpz_tdiv_q_2exp(mantissa,
		    from,
		    exponent - Float<T>::Binary::MANTISSA_BITS);
  else
    mpz_mul_2exp(mantissa, from, Float<T>::Binary::MANTISSA_BITS - exponent);
  Float<T> f(to);
  f.u.binary.build(sign < 0, mantissa, exponent);
  mpz_clear(mantissa);
  to = f.value();
  if (meaningful_bits > Float<T>::Binary::MANTISSA_BITS) {
    if (sign < 0)
      return round_lt_float<To_Policy>(to, dir);
    else
      return round_gt_float<To_Policy>(to, dir);
  }
  return V_EQ;
}

template <typename To_Policy, typename From_Policy, typename T>
inline Result
assign_float_mpq(T& to, const mpq_class& from, Rounding_Dir dir)
{
  const mpz_class& _num = from.get_num();
  const mpz_class& _den = from.get_den();
  if (_den == 1)
    return assign_float_mpz<To_Policy, From_Policy>(to, _num, dir);
  mpz_srcptr num = _num.get_mpz_t();
  mpz_srcptr den = _den.get_mpz_t();
  int sign = mpz_sgn(num);
  signed long exponent = mpz_sizeinbase(num, 2) - mpz_sizeinbase(den, 2);
  if (exponent < Float<T>::Binary::EXPONENT_MIN_DENORM) {
    to = 0;
  inexact:
    if (sign < 0)
      return round_lt_float<To_Policy>(to, dir);
    else
      return round_gt_float<To_Policy>(to, dir);
  }
  if (exponent > static_cast<signed int>(Float<T>::Binary::EXPONENT_MAX + 1)) {
  overflow:
    if (sign < 0)
      return set_neg_overflow_float<To_Policy>(to, dir);
    else
      return set_pos_overflow_float<To_Policy>(to, dir);
  }
  unsigned int needed_bits = Float<T>::Binary::MANTISSA_BITS + 1;
  if (exponent < Float<T>::Binary::EXPONENT_MIN)
    needed_bits -= Float<T>::Binary::EXPONENT_MIN - exponent;
  mpz_t mantissa;
  mpz_init(mantissa);
  signed long shift = needed_bits - exponent;
  if (shift > 0) {
    mpz_mul_2exp(mantissa, num, shift);
    num = mantissa;
  }
  else if (shift < 0) {
    mpz_mul_2exp(mantissa, den, -shift);
    den = mantissa;
  }
  mpz_t r;
  mpz_init(r);
  mpz_tdiv_qr(mantissa, r, num, den);
  size_t bits = mpz_sizeinbase(mantissa, 2);
  bool inexact = (mpz_sgn(r) != 0);
  mpz_clear(r);
  if (bits == needed_bits + 1) {
    inexact = (inexact || mpz_odd_p(mantissa));
    mpz_div_2exp(mantissa, mantissa, 1);
  }
  else
    --exponent;
  if (exponent > static_cast<signed int>(Float<T>::Binary::EXPONENT_MAX)) {
    mpz_clear(mantissa);
    goto overflow;
  }
  else if (exponent < Float<T>::Binary::EXPONENT_MIN - 1) {
    // Denormalized.
    exponent = Float<T>::Binary::EXPONENT_MIN - 1;
  }
  Float<T> f(to);
  f.u.binary.build(sign < 0, mantissa, exponent);
  mpz_clear(mantissa);
  to = f.value();
  if (inexact)
    goto inexact;
  return V_EQ;
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy,
	  typename Type>
inline Result
add_mul_float(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_inf_mul_zero
      && ((x == 0 && is_inf_float<From2_Policy>(y)) ||
	  (y == 0 && is_inf_float<From1_Policy>(x))))
    return set_special<To_Policy>(to, V_INF_MUL_ZERO);
  // FIXME: missing check_inf_add_inf
  prepare_inexact<To_Policy>(dir);
  if (fpu_direct_rounding(dir))
    to = fma(x, y, to);
  else if (fpu_inverse_rounding(dir))
    to = -limit_precision(fma(-x, y, -to));
  else {
    fpu_rounding_control_word_type old = fpu_save_rounding_direction(round_fpu_dir(dir));
    avoid_cse(x);
    avoid_cse(y);
    to = fma(x, y, to);
    avoid_cse(to);
    fpu_restore_rounding_direction(old);
  }
  if (To_Policy::check_nan_result && is_nan<To_Policy>(to))
    return VC_NAN;
  return result_relation<To_Policy>(dir);
}

template <typename To_Policy, typename From1_Policy, typename From2_Policy, typename Type>
inline Result
sub_mul_float(Type& to, const Type x, const Type y, Rounding_Dir dir) {
  if (To_Policy::check_inf_mul_zero
      && ((x == 0 && is_inf_float<From2_Policy>(y)) ||
	  (y == 0 && is_inf_float<From1_Policy>(x))))
    return set_special<To_Policy>(to, V_INF_MUL_ZERO);
  // FIXME: missing check_inf_add_inf
  prepare_inexact<To_Policy>(dir);
  if (fpu_direct_rounding(dir))
    to = fma(x, -y, to);
  else if (fpu_inverse_rounding(dir))
    to = -limit_precision(fma(x, y, -to));
  else {
    fpu_rounding_control_word_type old = fpu_save_rounding_direction(round_fpu_dir(dir));
    avoid_cse(x);
    avoid_cse(y);
    to = fma(x, -y, to);
    avoid_cse(to);
    fpu_restore_rounding_direction(old);
  }
  if (To_Policy::check_nan_result && is_nan<To_Policy>(to))
    return VC_NAN;
  return result_relation<To_Policy>(dir);
}

template <typename Policy, typename Type>
inline Result
output_float(std::ostream& os, const Type from, const Numeric_Format&,
	     Rounding_Dir) {
  if (from == 0)
    os << "0";
  else if (is_minf<Policy>(from))
    os << "-inf";
  else if (is_pinf<Policy>(from))
    os << "+inf";
  else if (is_nan<Policy>(from))
    os << "nan";
  else {
    int old_precision = os.precision(10000);
    os << from;
    os.precision(old_precision);
  }
  return V_EQ;
}

#if PPL_SUPPORTED_FLOAT
SPECIALIZE_ASSIGN(assign_float_float_exact, float, float)
#if PPL_SUPPORTED_DOUBLE
SPECIALIZE_ASSIGN(assign_float_float, float, double)
SPECIALIZE_ASSIGN(assign_float_float_exact, double, float)
#endif
#if PPL_SUPPORTED_LONG_DOUBLE
SPECIALIZE_ASSIGN(assign_float_float, float, long double)
SPECIALIZE_ASSIGN(assign_float_float_exact, long double, float)
#endif
#endif

#if PPL_SUPPORTED_DOUBLE
SPECIALIZE_ASSIGN(assign_float_float_exact, double, double)
#if PPL_SUPPORTED_LONG_DOUBLE
SPECIALIZE_ASSIGN(assign_float_float, double, long double)
SPECIALIZE_ASSIGN(assign_float_float_exact, long double, double)
#endif
#endif

#if PPL_SUPPORTED_LONG_DOUBLE
SPECIALIZE_ASSIGN(assign_float_float_exact, long double, long double)
#endif

#if PPL_SUPPORTED_FLOAT
SPECIALIZE_CLASSIFY(classify_float, float)
SPECIALIZE_IS_NAN(is_nan_float, float)
SPECIALIZE_IS_MINF(is_minf_float, float)
SPECIALIZE_IS_PINF(is_pinf_float, float)
SPECIALIZE_SET_SPECIAL(set_special_float, float)
SPECIALIZE_ASSIGN(assign_float_int, float, signed char)
SPECIALIZE_ASSIGN(assign_float_int, float, signed short)
SPECIALIZE_ASSIGN(assign_float_int, float, signed int)
SPECIALIZE_ASSIGN(assign_float_int, float, signed long)
SPECIALIZE_ASSIGN(assign_float_int, float, signed long long)
SPECIALIZE_ASSIGN(assign_float_int, float, unsigned char)
SPECIALIZE_ASSIGN(assign_float_int, float, unsigned short)
SPECIALIZE_ASSIGN(assign_float_int, float, unsigned int)
SPECIALIZE_ASSIGN(assign_float_int, float, unsigned long)
SPECIALIZE_ASSIGN(assign_float_int, float, unsigned long long)
SPECIALIZE_ASSIGN(assign_float_mpz, float, mpz_class)
SPECIALIZE_ASSIGN(assign_float_mpq, float, mpq_class)
SPECIALIZE_COPY(copy_generic, float)
SPECIALIZE_IS_INT(is_int_float, float)
SPECIALIZE_FLOOR(floor_float, float, float)
SPECIALIZE_CEIL(ceil_float, float, float)
SPECIALIZE_TRUNC(trunc_float, float, float)
SPECIALIZE_NEG(neg_float, float, float)
SPECIALIZE_ABS(abs_float, float, float)
SPECIALIZE_ADD(add_float, float, float, float)
SPECIALIZE_SUB(sub_float, float, float, float)
SPECIALIZE_MUL(mul_float, float, float, float)
SPECIALIZE_DIV(div_float, float, float, float)
SPECIALIZE_REM(rem_float, float, float, float)
SPECIALIZE_MUL2EXP(mul2exp_float, float, float)
SPECIALIZE_DIV2EXP(div2exp_float, float, float)
SPECIALIZE_SQRT(sqrt_float, float, float)
SPECIALIZE_GCD(gcd_exact, float, float, float)
SPECIALIZE_GCDEXT(gcdext_exact, float, float, float, float, float)
SPECIALIZE_LCM(lcm_gcd_exact, float, float, float)
SPECIALIZE_SGN(sgn_float, float)
SPECIALIZE_CMP(cmp_float, float, float)
SPECIALIZE_ADD_MUL(add_mul_float, float, float, float)
SPECIALIZE_SUB_MUL(sub_mul_float, float, float, float)
SPECIALIZE_INPUT(input_generic, float)
SPECIALIZE_OUTPUT(output_float, float)
#endif

#if PPL_SUPPORTED_DOUBLE
SPECIALIZE_CLASSIFY(classify_float, double)
SPECIALIZE_IS_NAN(is_nan_float, double)
SPECIALIZE_IS_MINF(is_minf_float, double)
SPECIALIZE_IS_PINF(is_pinf_float, double)
SPECIALIZE_SET_SPECIAL(set_special_float, double)
SPECIALIZE_ASSIGN(assign_float_int, double, signed char)
SPECIALIZE_ASSIGN(assign_float_int, double, signed short)
SPECIALIZE_ASSIGN(assign_float_int, double, signed int)
SPECIALIZE_ASSIGN(assign_float_int, double, signed long)
SPECIALIZE_ASSIGN(assign_float_int, double, signed long long)
SPECIALIZE_ASSIGN(assign_float_int, double, unsigned char)
SPECIALIZE_ASSIGN(assign_float_int, double, unsigned short)
SPECIALIZE_ASSIGN(assign_float_int, double, unsigned int)
SPECIALIZE_ASSIGN(assign_float_int, double, unsigned long)
SPECIALIZE_ASSIGN(assign_float_int, double, unsigned long long)
SPECIALIZE_ASSIGN(assign_float_mpz, double, mpz_class)
SPECIALIZE_ASSIGN(assign_float_mpq, double, mpq_class)
SPECIALIZE_COPY(copy_generic, double)
SPECIALIZE_IS_INT(is_int_float, double)
SPECIALIZE_FLOOR(floor_float, double, double)
SPECIALIZE_CEIL(ceil_float, double, double)
SPECIALIZE_TRUNC(trunc_float, double, double)
SPECIALIZE_NEG(neg_float, double, double)
SPECIALIZE_ABS(abs_float, double, double)
SPECIALIZE_ADD(add_float, double, double, double)
SPECIALIZE_SUB(sub_float, double, double, double)
SPECIALIZE_MUL(mul_float, double, double, double)
SPECIALIZE_DIV(div_float, double, double, double)
SPECIALIZE_REM(rem_float, double, double, double)
SPECIALIZE_MUL2EXP(mul2exp_float, double, double)
SPECIALIZE_DIV2EXP(div2exp_float, double, double)
SPECIALIZE_SQRT(sqrt_float, double, double)
SPECIALIZE_GCD(gcd_exact, double, double, double)
SPECIALIZE_GCDEXT(gcdext_exact, double, double, double, double, double)
SPECIALIZE_LCM(lcm_gcd_exact, double, double, double)
SPECIALIZE_SGN(sgn_float, double)
SPECIALIZE_CMP(cmp_float, double, double)
SPECIALIZE_ADD_MUL(add_mul_float, double, double, double)
SPECIALIZE_SUB_MUL(sub_mul_float, double, double, double)
SPECIALIZE_INPUT(input_generic, double)
SPECIALIZE_OUTPUT(output_float, double)
#endif

#if PPL_SUPPORTED_LONG_DOUBLE
SPECIALIZE_CLASSIFY(classify_float, long double)
SPECIALIZE_IS_NAN(is_nan_float, long double)
SPECIALIZE_IS_MINF(is_minf_float, long double)
SPECIALIZE_IS_PINF(is_pinf_float, long double)
SPECIALIZE_SET_SPECIAL(set_special_float, long double)
SPECIALIZE_ASSIGN(assign_float_int, long double, signed char)
SPECIALIZE_ASSIGN(assign_float_int, long double, signed short)
SPECIALIZE_ASSIGN(assign_float_int, long double, signed int)
SPECIALIZE_ASSIGN(assign_float_int, long double, signed long)
SPECIALIZE_ASSIGN(assign_float_int, long double, signed long long)
SPECIALIZE_ASSIGN(assign_float_int, long double, unsigned char)
SPECIALIZE_ASSIGN(assign_float_int, long double, unsigned short)
SPECIALIZE_ASSIGN(assign_float_int, long double, unsigned int)
SPECIALIZE_ASSIGN(assign_float_int, long double, unsigned long)
SPECIALIZE_ASSIGN(assign_float_int, long double, unsigned long long)
SPECIALIZE_ASSIGN(assign_float_mpz, long double, mpz_class)
SPECIALIZE_ASSIGN(assign_float_mpq, long double, mpq_class)
SPECIALIZE_COPY(copy_generic, long double)
SPECIALIZE_IS_INT(is_int_float, long double)
SPECIALIZE_FLOOR(floor_float, long double, long double)
SPECIALIZE_CEIL(ceil_float, long double, long double)
SPECIALIZE_TRUNC(trunc_float, long double, long double)
SPECIALIZE_NEG(neg_float, long double, long double)
SPECIALIZE_ABS(abs_float, long double, long double)
SPECIALIZE_ADD(add_float, long double, long double, long double)
SPECIALIZE_SUB(sub_float, long double, long double, long double)
SPECIALIZE_MUL(mul_float, long double, long double, long double)
SPECIALIZE_DIV(div_float, long double, long double, long double)
SPECIALIZE_REM(rem_float, long double, long double, long double)
SPECIALIZE_MUL2EXP(mul2exp_float, long double, long double)
SPECIALIZE_DIV2EXP(div2exp_float, long double, long double)
SPECIALIZE_SQRT(sqrt_float, long double, long double)
SPECIALIZE_GCD(gcd_exact, long double, long double, long double)
SPECIALIZE_GCDEXT(gcdext_exact, long double, long double, long double,
		  long double, long double)
SPECIALIZE_LCM(lcm_gcd_exact, long double, long double, long double)
SPECIALIZE_SGN(sgn_float, long double)
SPECIALIZE_CMP(cmp_float, long double, long double)
SPECIALIZE_ADD_MUL(add_mul_float, long double, long double, long double)
SPECIALIZE_SUB_MUL(sub_mul_float, long double, long double, long double)
SPECIALIZE_INPUT(input_generic, long double)
SPECIALIZE_OUTPUT(output_float, long double)
#endif

} // namespace Checked

} // namespace Parma_Polyhedra_Library

#endif // !defined(PPL_checked_int_inlines_hh)
