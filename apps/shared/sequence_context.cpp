#include "sequence_context.h"

#include <cmath>

#include "../shared/poincare_helpers.h"
#include "sequence_cache_context.h"
#include "sequence_store.h"

using namespace Poincare;

namespace Shared {

template <typename T>
TemplatedSequenceContext<T>::TemplatedSequenceContext()
    : m_commonRank(-1),
      m_commonRankValues{{NAN, NAN, NAN}, {NAN, NAN, NAN}, {NAN, NAN, NAN}},
      m_independentRanks{-1, -1, -1},
      m_independentRankValues{
          {NAN, NAN, NAN}, {NAN, NAN, NAN}, {NAN, NAN, NAN}} {}

template <typename T>
T TemplatedSequenceContext<T>::valueOfCommonRankSequenceAtPreviousRank(
    int sequenceIndex, int rank) const {
  return m_commonRankValues[sequenceIndex][rank];
}

template <typename T>
void TemplatedSequenceContext<T>::resetCache() {
  /* We only need to reset the ranks. Indeed, when we compute the values of the
   * sequences, we use ranks as memoization indexes. Therefore, we know that the
   * values stored in m_commomValues and m_independentRankValues are dirty
   * and do not use them. */
  m_commonRank = -1;
  for (int i = 0; i < SequenceStore::k_maxNumberOfSequences; i++) {
    m_independentRanks[i] = -1;
  }
}

template <typename T>
bool TemplatedSequenceContext<T>::iterateUntilRank(int n,
                                                   SequenceStore *sequenceStore,
                                                   SequenceContext *sqctx) {
  if (n < 0 || n > k_maxRecurrentRank) {
    return false;
  }
  // If values stored in cache starts are at a rank (m_commonRank) superior to
  // n, we need to start computing back the recurrence from the initial rank and
  // step until rank n. Otherwise, we can start at the common rank and step
  // until rank n.
  if (m_commonRank > n) {
    m_commonRank = -1;
  }
  while (m_commonRank < n) {
    step(sqctx);
  }
  return true;
}

template <typename T>
void TemplatedSequenceContext<T>::step(SequenceContext *sqctx,
                                       int sequenceIndex) {
  // First we increment the rank
  bool stepAllSequences = sequenceIndex == -1;
  if (stepAllSequences) {
    m_commonRank++;
  } else {
    m_independentRanks[sequenceIndex]++;
  }

  // Then we shift the values stored in the arrays
  int start, stop;
  T *sequencesRankValues;
  if (stepAllSequences) {
    start = 0;
    stop = SequenceStore::k_maxNumberOfSequences;
    sequencesRankValues = reinterpret_cast<T *>(&m_commonRankValues);
  } else {
    start = sequenceIndex;
    stop = sequenceIndex + 1;
    sequencesRankValues = reinterpret_cast<T *>(&m_independentRankValues);
  }
  for (int sequence = start; sequence < stop; sequence++) {
    T *sequencePointer =
        sequencesRankValues + sequence * k_numberOfValuesInCachePerSequence;
    // {u(n), u(n-1), u(n-2)} becomes {NaN, u(n), u(n-1)}
    for (int depth = k_numberOfValuesInCachePerSequence - 1; depth > 0;
         depth--) {
      *(sequencePointer + depth) = *(sequencePointer + depth - 1);
    }
    *sequencePointer = NAN;
  }

  // We create an array containing the sequences we want to update
  Sequence *sequencesToUpdate[SequenceStore::k_maxNumberOfSequences] = {
      nullptr, nullptr, nullptr};
  SequenceStore *sequenceStore = sqctx->sequenceStore();
  for (int sequence = start; sequence < stop; sequence++) {
    Ion::Storage::Record record = sequenceStore->recordAtNameIndex(sequence);
    if (record.isNull()) {
      continue;
    }
    Sequence *s = sequenceStore->modelForRecord(record);
    if (!s->isDefined()) {
      continue;
    }
    int index = SequenceStore::sequenceIndexForName(s->fullName()[0]);
    sequencesToUpdate[index] = s;
  }

  // We approximate the value of the next rank for each sequence we want to
  // update
  /* In case stop is SequenceStore::k_maxNumberOfSequences, we approximate u, v
   * and w at the new rank. We have to evaluate all sequences
   * SequenceStore::k_maxNumberOfSequences times in case the evaluations depend
   * on each other. For example, if u expression depends on v and v depends on
   * w. At the first iteration, we can only evaluate w, at the second iteration
   * we evaluate v and u can only be known at the third iteration. In case stop
   * is 1, there is only one sequence we want to update. Moreover, the call to
   * approximateToNextRank will handle potential dependencies. */
  for (int k = 0; k < SequenceStore::k_maxNumberOfSequences; k++) {
    if (!sequencesToUpdate[k]) {
      continue;
    }
    for (int sequence = start; sequence < stop; sequence++) {
      T *sequencePointer =
          sequencesRankValues + sequence * k_numberOfValuesInCachePerSequence;
      if (std::isnan(*sequencePointer)) {
        *sequencePointer =
            sequencesToUpdate[sequence]
                ? sequencesToUpdate[sequence]
                      ->template approximateToNextRank<T>(sqctx, sequenceIndex)
                : NAN;
      }
    }
  }
}

void SequenceContext::tidyDownstreamPoolFrom(char *treePoolCursor) {
  m_sequenceStore->tidyDownstreamPoolFrom(treePoolCursor);
}

Poincare::Context::SymbolAbstractType
SequenceContext::expressionTypeForIdentifier(const char *identifier,
                                             int length) {
  const char *const *sequenceNames = SequenceStore::k_sequenceNames;
  int numberOfSequencesNames =
      sizeof(SequenceStore::k_sequenceNames) / sizeof(char *);
  for (int i = 0; i < numberOfSequencesNames; i++) {
    if (strncmp(identifier, sequenceNames[i], length) == 0) {
      return Poincare::Context::SymbolAbstractType::Sequence;
    }
  }
  return Poincare::ContextWithParent::expressionTypeForIdentifier(identifier,
                                                                  length);
}

template class TemplatedSequenceContext<float>;
template class TemplatedSequenceContext<double>;
template void *SequenceContext::helper<float>();
template void *SequenceContext::helper<double>();

}  // namespace Shared
