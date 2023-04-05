// Copyright (c) 2023 The InterpretML Contributors
// Licensed under the MIT license.
// Author: Paul Koch <code@koch.ninja>

// !! To add a new loss/objective function in C++ follow the steps at the top of the "loss_registrations.hpp" file !!

// DO NOT INCLUDE ANY FILES IN THIS FILE. THEY WILL NOT BE ZONED PROPERLY

// TFloat is a datatype that could hold inside a double, float, or some SIMD intrinsic type.
// See sse2_32.cpp, cuda_32.cpp, and cpu_64.cpp as examples.
template<typename TFloat>
struct PseudoHuberRegressionLoss final : public RegressionLoss {
   LOSS_CLASS_BOILERPLATE(PseudoHuberRegressionLoss, true)

   TFloat m_deltaInverted;
   TFloat m_deltaSquared;

   // IMPORTANT: the constructor parameters here must match the RegisterLoss parameters in loss_registrations.hpp
   inline PseudoHuberRegressionLoss(const Config & config, double delta) {
      if(1 != config.cOutputs) {
         throw ParamMismatchWithConfigException();
      }

      if(delta == 0.0 || std::isnan(delta) || std::isinf(delta)) {
         throw ParamValOutOfRangeException();
      }

      const double deltaSquared = delta * delta;
      if(std::isinf(deltaSquared)) {
         throw ParamValOutOfRangeException();
      }
      m_deltaSquared = deltaSquared;

      const double deltaInverted = 1.0 / delta;
      if(std::isinf(deltaInverted)) {
         throw ParamValOutOfRangeException();
      }
      m_deltaInverted = deltaInverted;
   }

   inline double GradientMultiple() const noexcept {
      return 1.0;
   }

   inline double HessianMultiple() const noexcept {
      return 1.0;
   }

   GPU_DEVICE inline TFloat InverseLinkFunction(TFloat score) const noexcept {
      return score;
   }

   GPU_DEVICE inline void CalcGrad(TFloat prediction, TFloat target, TFloat & gradient) const noexcept {
      TFloat error = prediction - target;
      TFloat errorFraction = error * m_deltaInverted;
      TFloat calc = errorFraction * errorFraction + 1.0;
      TFloat sqrtCalc = calc.Sqrt();
      // the calculations above are shared with the hessian, so the compiler should combine them.
      gradient = error / sqrtCalc;
   }

   GPU_DEVICE inline void CalcGradMetric(TFloat prediction, TFloat target, TFloat & gradient, TFloat & metric) const noexcept {
      TFloat error = prediction - target;
      TFloat errorFraction = error * m_deltaInverted;
      TFloat calc = errorFraction * errorFraction + 1.0;
      TFloat sqrtCalc = calc.Sqrt();
      // the calculations above are shared with the hessian, so the compiler should combine them.
      gradient = error / sqrtCalc;
      metric = m_deltaSquared * (sqrtCalc - 1.0);
   }

   // If the loss function doesn't have a second derivative, then delete CalcGradHess.
   GPU_DEVICE inline void CalcGradHess(TFloat prediction, TFloat target, TFloat & gradient, TFloat & hessian) const noexcept {
      TFloat error = prediction - target;
      TFloat errorFraction = error * m_deltaInverted;
      TFloat calc = errorFraction * errorFraction + 1.0;
      TFloat sqrtCalc = calc.Sqrt();
      // the calculations above are shared with the hessian, so the compiler should combine them.
      gradient = error / sqrtCalc;
      hessian = 1.0 / (calc * sqrtCalc);
   }

   // If the loss function doesn't have a second derivative, then delete CalcGradHessMetric.
   GPU_DEVICE inline void CalcGradHessMetric(TFloat prediction, TFloat target, TFloat & gradient, TFloat & hessian, TFloat & metric) const noexcept {
      TFloat error = prediction - target;
      TFloat errorFraction = error * m_deltaInverted;
      TFloat calc = errorFraction * errorFraction + 1.0;
      TFloat sqrtCalc = calc.Sqrt();
      // the calculations above are shared with the hessian, so the compiler should combine them.
      gradient = error / sqrtCalc;
      hessian = 1.0 / (calc * sqrtCalc);
      metric = m_deltaSquared * (sqrtCalc - 1.0);
   }
};
