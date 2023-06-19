// Copyright (c) 2023 The InterpretML Contributors
// Licensed under the MIT license.
// Author: Paul Koch <code@koch.ninja>

#ifndef DATA_SET_INTERACTION_HPP
#define DATA_SET_INTERACTION_HPP

#include <stddef.h> // size_t, ptrdiff_t

#include "libebm.h" // ErrorEbm
#include "logging.h" // EBM_ASSERT
#include "common_c.h" // FloatFast
#include "bridge_c.h" // StorageDataType
#include "zones.h"

namespace DEFINED_ZONE_NAME {
#ifndef DEFINED_ZONE_NAME
#error DEFINED_ZONE_NAME must be defined
#endif // DEFINED_ZONE_NAME

struct DataSetInteraction;

struct DataSubsetInteraction final {
   friend DataSetInteraction;

   DataSubsetInteraction() = default; // preserve our POD status
   ~DataSubsetInteraction() = default; // preserve our POD status
   void * operator new(std::size_t) = delete; // we only use malloc/free in this library
   void operator delete (void *) = delete; // we only use malloc/free in this library

   void DestructDataSubsetInteraction(const size_t cFeatures);

   inline void SafeInitDataSubsetInteraction() {
      m_cSamples = 0;
      m_aGradHess = nullptr;
      m_aaFeatureData = nullptr;
      m_aWeights = nullptr;
   }

   inline size_t GetCountSamples() const {
      return m_cSamples;
   }
   inline FloatFast * GetGradHess() {
      EBM_ASSERT(nullptr != m_aGradHess);
      return m_aGradHess;
   }
   inline const StorageDataType * GetFeatureData(const size_t iFeature) const {
      EBM_ASSERT(nullptr != m_aaFeatureData);
      return m_aaFeatureData[iFeature];
   }
   inline const FloatFast * GetWeights() const {
      return m_aWeights;
   }

private:

   size_t m_cSamples;
   FloatFast * m_aGradHess;
   StorageDataType ** m_aaFeatureData;
   FloatFast * m_aWeights;
};
static_assert(std::is_standard_layout<DataSubsetInteraction>::value,
   "We use the struct hack in several places, so disallow non-standard_layout types in general");
static_assert(std::is_trivial<DataSubsetInteraction>::value,
   "We use memcpy in several places, so disallow non-trivial types in general");


struct DataSetInteraction final {
   DataSetInteraction() = default; // preserve our POD status
   ~DataSetInteraction() = default; // preserve our POD status
   void * operator new(std::size_t) = delete; // we only use malloc/free in this library
   void operator delete (void *) = delete; // we only use malloc/free in this library

   inline void SafeInitDataSetInteraction() {
      m_cSamples = 0;
      m_cSubsets = 0;
      m_aSubsets = nullptr;
      m_weightTotal = 0.0;
   }

   void DestructDataSetInteraction(const size_t cFeatures);

   ErrorEbm InitDataSetInteraction(
      const ObjectiveWrapper * const pObjective,
      const size_t cSubsetItemsMax,
      const size_t cScores,
      const bool bAllocateHessians,
      const unsigned char * const pDataSetShared,
      const size_t cSharedSamples,
      const BagEbm * const aBag,
      const size_t cIncludedSamples,
      const size_t cWeights,
      const size_t cFeatures
   );

   inline size_t GetCountSamples() const {
      return m_cSamples;
   }
   inline size_t GetCountSubsets() const {
      return m_cSubsets;
   }
   inline DataSubsetInteraction * GetSubsets() {
      EBM_ASSERT(nullptr != m_aSubsets);
      return m_aSubsets;
   }
   inline double GetWeightTotal() const {
      return m_weightTotal;
   }

private:

   ErrorEbm InitGradHess(
      const ObjectiveWrapper * const pObjective,
      const size_t cScores,
      const bool bAllocateHessians
   );

   ErrorEbm InitFeatureData(
      const unsigned char * const pDataSetShared,
      const size_t cSharedSamples,
      const BagEbm * const aBag,
      const size_t cFeatures
   );

   ErrorEbm InitWeights(
      const unsigned char * const pDataSetShared,
      const BagEbm * const aBag,
      const size_t cIncludedSamples
   );

   size_t m_cSamples;
   size_t m_cSubsets;
   DataSubsetInteraction * m_aSubsets;
   double m_weightTotal;
};
static_assert(std::is_standard_layout<DataSetInteraction>::value,
   "We use the struct hack in several places, so disallow non-standard_layout types in general");
static_assert(std::is_trivial<DataSetInteraction>::value,
   "We use memcpy in several places, so disallow non-trivial types in general");

} // DEFINED_ZONE_NAME

#endif // DATA_SET_INTERACTION_HPP
