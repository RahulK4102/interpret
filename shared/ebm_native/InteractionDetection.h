// Copyright (c) 2018 Microsoft Corporation
// Licensed under the MIT license.
// Author: Paul Koch <ebm@koch.ninja>

#ifndef EBM_INTERACTION_STATE_H
#define EBM_INTERACTION_STATE_H

#include <stdlib.h> // free
#include <stddef.h> // size_t, ptrdiff_t
#include <limits> // numeric_limits

#include "ebm_native.h"
#include "EbmInternal.h"
#include "Logging.h" // EBM_ASSERT & LOG
// feature includes
#include "Feature.h"
// dataset depends on features
#include "DataSetInteraction.h"

class EbmInteractionState final {
   ptrdiff_t m_runtimeLearningTypeOrCountTargetClasses;

   size_t m_cFeatures;
   Feature * m_aFeatures;

   DataSetByFeature m_dataSet;

   unsigned int m_cLogEnterMessages;
   unsigned int m_cLogExitMessages;

   void * operator new(std::size_t) = delete; // we only use malloc/free in this library
   void operator delete (void *) = delete; // we only use malloc/free in this library

public:

   EbmInteractionState() = default; // preserve our POD status
   ~EbmInteractionState() = default; // preserve our POD status

   EBM_INLINE void InitializeZero() {
      m_runtimeLearningTypeOrCountTargetClasses = 0;

      m_cFeatures = 0;
      m_aFeatures = nullptr;

      m_dataSet.InitializeZero();

      m_cLogEnterMessages = 0;
      m_cLogExitMessages = 0;
   }

   EBM_INLINE ptrdiff_t GetRuntimeLearningTypeOrCountTargetClasses() {
      return m_runtimeLearningTypeOrCountTargetClasses;
   }

   EBM_INLINE unsigned int * GetPointerCountLogEnterMessages() {
      return &m_cLogEnterMessages;
   }

   EBM_INLINE unsigned int * GetPointerCountLogExitMessages() {
      return &m_cLogExitMessages;
   }

   EBM_INLINE const DataSetByFeature * GetDataSetByFeature() const {
      return &m_dataSet;
   }

   EBM_INLINE const Feature * GetFeatures() const {
      return m_aFeatures;
   }

   EBM_INLINE size_t GetCountFeatures() const {
      return m_cFeatures;
   }

   static void Free(EbmInteractionState * const pInteractionDetection);
   static EbmInteractionState * Allocate(
      const ptrdiff_t runtimeLearningTypeOrCountTargetClasses,
      const size_t cFeatures,
      const FloatEbmType * const optionalTempParams,
      const EbmNativeFeature * const aNativeFeatures,
      const size_t cInstances,
      const void * const aTargets,
      const IntEbmType * const aBinnedData,
      const FloatEbmType * const aPredictorScores
   );
};
static_assert(std::is_standard_layout<EbmInteractionState>::value,
   "not required, but keep everything standard_layout since some of our classes use the struct hack");
static_assert(std::is_pod<EbmInteractionState>::value,
   "not required, but keep things closer to C by being POD");

#endif // EBM_INTERACTION_STATE_H
