// Copyright (c) 2023 The InterpretML Contributors
// Licensed under the MIT license.
// Author: Paul Koch <code@koch.ninja>

#ifndef MATH_HPP
#define MATH_HPP

#include "libebm.h"
#include "logging.h" // EBM_ASSERT
#include "unzoned.h" // INLINE_ALWAYS

namespace DEFINED_ZONE_NAME {
#ifndef DEFINED_ZONE_NAME
#error DEFINED_ZONE_NAME must be defined
#endif // DEFINED_ZONE_NAME

template<typename TFloat> static INLINE_ALWAYS TFloat Mantissa32(const TFloat& val) noexcept {
   return TFloat::ReinterpretFloat((TFloat::ReinterpretInt(val) & 0x007FFFFF) | 0x3F000000);
}

template<typename TFloat> static INLINE_ALWAYS typename TFloat::TInt Exponent32(const TFloat& val) noexcept {
   return ((TFloat::ReinterpretInt(val) << 1) >> 24) - typename TFloat::TInt(0x7F);
}

template<typename TFloat> static INLINE_ALWAYS TFloat Power2(const TFloat val) {
   return TFloat::ReinterpretFloat(TFloat::ReinterpretInt(val + TFloat{8388608 + 127}) << 23);
}

template<typename TFloat>
static INLINE_ALWAYS TFloat Polynomial(const TFloat x,
      const TFloat c0,
      const TFloat c1,
      const TFloat c2,
      const TFloat c3,
      const TFloat c4,
      const TFloat c5) {
   TFloat x2 = x * x;
   TFloat x4 = x2 * x2;
   return FusedMultiplyAdd(FusedMultiplyAdd(c3, x, c2),
         x2,
         FusedMultiplyAdd(FusedMultiplyAdd(c5, x, c4), x4, FusedMultiplyAdd(c1, x, c0)));
}

template<typename TFloat>
static INLINE_ALWAYS TFloat Polynomial(const TFloat x,
      const TFloat c0,
      const TFloat c1,
      const TFloat c2,
      const TFloat c3,
      const TFloat c4,
      const TFloat c5,
      const TFloat c6,
      const TFloat c7,
      const TFloat c8) {
   TFloat x2 = x * x;
   TFloat x4 = x2 * x2;
   TFloat x8 = x4 * x4;
   return FusedMultiplyAdd(FusedMultiplyAdd(FusedMultiplyAdd(c7, x, c6), x2, FusedMultiplyAdd(c5, x, c4)),
         x4,
         FusedMultiplyAdd(FusedMultiplyAdd(c3, x, c2), x2, FusedMultiplyAdd(c1, x, c0) + c8 * x8));
}

template<typename TFloat,
      bool bNegateInput = false,
      bool bNaNPossible = true,
      bool bUnderflowPossible = true,
      bool bOverflowPossible = true>
static INLINE_ALWAYS TFloat Exp32(const TFloat val) {
   // algorithm comes from:
   // https://github.com/vectorclass/version2/blob/f4617df57e17efcd754f5bbe0ec87883e0ed9ce6/vectormath_exp.h#L501

   // k_expUnderflow is set to a value that prevents us from returning a denormal number.
   static constexpr float k_expUnderflow = -87.25f; // this is exactly representable in IEEE 754
   static constexpr float k_expOverflow = 87.25f; // this is exactly representable in IEEE 754

   // TODO: make this negation more efficient
   TFloat x = bNegateInput ? -val : val;
   const TFloat rounded = Round(x * TFloat{1.44269504088896340736f});
   x = FusedNegateMultiplyAdd(rounded, TFloat{0.693359375f}, x);
   x = FusedNegateMultiplyAdd(rounded, TFloat{-2.12194440e-4f}, x);

   const TFloat x2 = x * x;
   TFloat ret = Polynomial(x,
         TFloat{1} / TFloat{2},
         TFloat{1} / TFloat{6},
         TFloat{1} / TFloat{24},
         TFloat{1} / TFloat{120},
         TFloat{1} / TFloat{720},
         TFloat{1} / TFloat{5040});
   ret = FusedMultiplyAdd(ret, x2, x);

   const TFloat rounded2 = Power2(rounded);

   ret = (ret + TFloat{1}) * rounded2;

   // TODO: handling overflow/underflow possible faster see vectormath version2 code
   if(bOverflowPossible) {
      if(bNegateInput) {
         ret = IfLess(val,
               static_cast<typename TFloat::T>(-k_expOverflow),
               std::numeric_limits<typename TFloat::T>::infinity(),
               ret);
      } else {
         ret = IfLess(static_cast<typename TFloat::T>(k_expOverflow),
               val,
               std::numeric_limits<typename TFloat::T>::infinity(),
               ret);
      }
   }
   if(bUnderflowPossible) {
      if(bNegateInput) {
         ret = IfLess(static_cast<typename TFloat::T>(-k_expUnderflow), val, 0.0f, ret);
      } else {
         ret = IfLess(val, static_cast<typename TFloat::T>(k_expUnderflow), 0.0f, ret);
      }
   }
   if(bNaNPossible) {
      ret = IfNaN(val, val, ret);
   }

#ifndef NDEBUG
   TFloat::Execute(
         [](int, typename TFloat::T orig, typename TFloat::T ret) {
            EBM_ASSERT(IsApproxEqual(std::exp(orig), ret, typename TFloat::T{1e-6}));
         },
         bNegateInput ? -val : val,
         ret);
#endif // NDEBUG

   return ret;
}

template<typename TFloat,
      bool bNegateOutput = false,
      bool bNaNPossible = true,
      bool bNegativePossible = true,
      bool bZeroPossible = true,
      bool bPositiveInfinityPossible = true>
static INLINE_ALWAYS TFloat Log32(const TFloat& val) noexcept {
   // algorithm comes from:
   // https://github.com/vectorclass/version2/blob/f4617df57e17efcd754f5bbe0ec87883e0ed9ce6/vectormath_exp.h#L1147

   TFloat x = Mantissa32(val);
   typename TFloat::TInt exponent = Exponent32(val);

   const auto comparison = x <= TFloat(float{1.41421356237309504880} * 0.5f);
   x = IfThenElse(comparison, x + x, x);
   exponent = IfThenElse(TFloat::ReinterpretInt(~comparison), exponent + 1, exponent);

   TFloat exponentFloat = TFloat(exponent);

   x -= 1.0f;

   TFloat ret = Polynomial(x,
         TFloat{3.3333331174E-1f},
         TFloat{-2.4999993993E-1f},
         TFloat{2.0000714765E-1f},
         TFloat{-1.6668057665E-1f},
         TFloat{1.4249322787E-1f},
         TFloat{-1.2420140846E-1f},
         TFloat{1.1676998740E-1f},
         TFloat{-1.1514610310E-1f},
         TFloat{7.0376836292E-2f});
   TFloat x2 = x * x;
   ret *= x2 * x;

   ret = FusedMultiplyAdd(exponentFloat, TFloat{-2.12194440E-4f}, ret);
   ret += FusedNegateMultiplyAdd(x2, 0.5f, x);
   ret = FusedMultiplyAdd(exponentFloat, TFloat{0.693359375f}, ret);

   if(bNegateOutput) {
      // TODO: do this with an alternate to FusedMultiplyAdd
      ret = -ret;
   }

   if(bZeroPossible) {
      ret = IfLess(val,
            std::numeric_limits<typename TFloat::T>::min(),
            bNegateOutput ? std::numeric_limits<typename TFloat::T>::infinity() :
                            -std::numeric_limits<typename TFloat::T>::infinity(),
            ret);
   }
   if(bNegativePossible) {
      ret = IfLess(val, 0.0, std::numeric_limits<typename TFloat::T>::quiet_NaN(), ret);
   }
   if(bNaNPossible) {
      if(bPositiveInfinityPossible) {
         ret = IfLess(val, std::numeric_limits<typename TFloat::T>::infinity(), ret, bNegateOutput ? -val : val);
      } else {
         ret = IfNaN(val, val, ret);
      }
   } else {
      if(bPositiveInfinityPossible) {
         ret = IfEqual(std::numeric_limits<typename TFloat::T>::infinity(),
               val,
               bNegateOutput ? -std::numeric_limits<typename TFloat::T>::infinity() :
                               std::numeric_limits<typename TFloat::T>::infinity(),
               ret);
      }
   }

#ifndef NDEBUG
   TFloat::Execute(
         [](int, typename TFloat::T orig, typename TFloat::T ret) {
            EBM_ASSERT(IsApproxEqual(std::log(orig), ret, typename TFloat::T{1e-6}));
         },
         val,
         bNegateOutput ? -ret : ret);
#endif // NDEBUG

   return ret;
}

} // namespace DEFINED_ZONE_NAME

#endif // REGISTRATION_HPP