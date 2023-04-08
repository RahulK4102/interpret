// Copyright (c) 2023 The InterpretML Contributors
// Licensed under the MIT license.
// Author: Paul Koch <ebm@koch.ninja>

#ifndef INTERACTION_CORE_HPP
#define INTERACTION_CORE_HPP

#include <stdlib.h> // free
#include <stddef.h> // size_t, ptrdiff_t
#include <limits> // numeric_limits
#include <atomic>

#include "libebm.h" // ErrorEbm
#include "zones.h"

#include "DataSetInteraction.hpp"

namespace DEFINED_ZONE_NAME {
#ifndef DEFINED_ZONE_NAME
#error DEFINED_ZONE_NAME must be defined
#endif // DEFINED_ZONE_NAME

class FeatureInteraction;

class InteractionCore final {

   // std::atomic_size_t used to be standard layout and trivial, but the C++ standard comitee judged that an error
   // and revoked the trivial nature of the class.  So, this means our InteractionCore class needs to have a constructor 
   // and destructor
   // https://stackoverflow.com/questions/48794325/why-stdatomic-is-not-trivial-type-in-only-visual-c
   // https://stackoverflow.com/questions/41308372/stdatomic-for-built-in-types-non-lock-free-vs-trivial-destructor
   std::atomic_size_t m_REFERENCE_COUNT;

   ptrdiff_t m_cClasses;

   size_t m_cFeatures;
   FeatureInteraction * m_aFeatures;

   DataSetInteraction m_dataFrame;

   LossWrapper m_loss;

   inline ~InteractionCore() {
      // this only gets called after our reference count has been decremented to zero

      m_dataFrame.Destruct();
      free(m_aFeatures);
      FreeLossWrapperInternals(&m_loss);
   };

   inline InteractionCore() noexcept :
      m_REFERENCE_COUNT(1), // we're not visible on any other thread yet, so no synchronization required
      m_cClasses(0),
      m_cFeatures(0),
      m_aFeatures(nullptr)
   {
      m_dataFrame.InitializeUnfailing();
      InitializeLossWrapperUnfailing(&m_loss);
   }

public:

   inline void AddReferenceCount() {
      // incrementing reference counts can be relaxed memory order since we're guaranteed to be above 1, 
      // so no result will change our behavior below
      // https://www.boost.org/doc/libs/1_59_0/doc/html/atomic/usage_examples.html
      m_REFERENCE_COUNT.fetch_add(1, std::memory_order_relaxed);
   };

   inline ptrdiff_t GetCountClasses() {
      return m_cClasses;
   }

   inline const DataSetInteraction * GetDataSetInteraction() const {
      return &m_dataFrame;
   }
   inline DataSetInteraction * GetDataSetInteraction() {
      return &m_dataFrame;
   }

   inline const FeatureInteraction * GetFeatures() const {
      return m_aFeatures;
   }

   inline size_t GetCountFeatures() const {
      return m_cFeatures;
   }

   static void Free(InteractionCore * const pInteractionCore);
   static ErrorEbm Create(
      const unsigned char * const pDataSetShared,
      const BagEbm * const aBag,
      const char * const sObjective,
      const double * const experimentalParams,
      InteractionCore ** const ppInteractionCoreOut
   );

   ErrorEbm InitializeInteractionGradientsAndHessians(
      const unsigned char * const pDataSetShared,
      const BagEbm * const aBag,
      const double * const aInitScores
   );

   inline ErrorEbm LossApplyUpdate(ApplyUpdateBridge * const pData) {
      return (*m_loss.m_pApplyUpdateC)(&m_loss, pData);
   }

   inline bool IsMse() {
      return EBM_FALSE != m_loss.m_bMse;
   }

   inline bool IsHessian() {
      return EBM_FALSE != m_loss.m_bLossHasHessian;
   }
};

} // DEFINED_ZONE_NAME

#endif // INTERACTION_CORE_HPP