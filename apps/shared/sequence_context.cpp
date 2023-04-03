#include "sequence_context.h"

#include <array>
#include <cmath>

#include "../shared/poincare_helpers.h"
#include "sequence_cache_context.h"
#include "sequence_store.h"

using namespace Poincare;

namespace Shared {

template <typename T>
TemplatedSequenceContext<T>::TemplatedSequenceContext(
    SequenceContext *sequenceContext)
    : m_sequenceContext(sequenceContext),
      m_commonRank(-1),
      m_commonRankValues{{NAN, NAN, NAN}, {NAN, NAN, NAN}, {NAN, NAN, NAN}},
      m_independentRanks{-1, -1, -1},
      m_independentRankValues{
          {NAN, NAN, NAN}, {NAN, NAN, NAN}, {NAN, NAN, NAN}} {}

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
void TemplatedSequenceContext<T>::stepUntilRank(int n, int sequenceIndex) {
  assert(IsAcceptableRank(n));
  bool independent = sequenceIndex != -1;
  int *currentRank =
      independent ? m_independentRanks + sequenceIndex : &m_commonRank;
  assert(*currentRank >= 0 || *currentRank == -1);

  // If current rank is superior to n, we need to start computing back the
  // recurrence from the initial rank and step until rank n. Otherwise, we can
  // start at the current rank and step until rank n.
  if (*currentRank > n) {
    *currentRank = -1;
  }
  while (*currentRank < n) {
    stepToNextRank(sequenceIndex);
  }
}

template <typename T>
void TemplatedSequenceContext<T>::stepToNextRank(int sequenceIndex) {
  // First we increment the rank
  bool independent = sequenceIndex != -1;
  if (independent) {
    m_independentRanks[sequenceIndex]++;
  } else {
    m_commonRank++;
  }

  // Then we shift the values stored in the arrays
  int start, stop;
  T *sequencesRankValues;
  if (independent) {
    start = sequenceIndex;
    stop = sequenceIndex + 1;
    sequencesRankValues = reinterpret_cast<T *>(&m_independentRankValues);
  } else {
    start = 0;
    stop = SequenceStore::k_maxNumberOfSequences;
    sequencesRankValues = reinterpret_cast<T *>(&m_commonRankValues);
  }
  for (int sequence = start; sequence < stop; sequence++) {
    T *sequencePointer = sequencesRankValues +
                         sequence * (SequenceStore::k_maxRecurrenceDepth + 1);
    // {u(n), u(n-1), u(n-2)} becomes {NaN, u(n), u(n-1)}
    for (int depth = SequenceStore::k_maxRecurrenceDepth; depth > 0; depth--) {
      *(sequencePointer + depth) = *(sequencePointer + depth - 1);
    }
    *sequencePointer = NAN;
  }

  // We create an array containing the sequences we want to update
  Sequence *sequencesToUpdate[SequenceStore::k_maxNumberOfSequences] = {
      nullptr, nullptr, nullptr};
  SequenceStore *sequenceStore = m_sequenceContext->sequenceStore();
  for (int sequence = start; sequence < stop; sequence++) {
    Ion::Storage::Record record = sequenceStore->recordAtNameIndex(sequence);
    if (record.isNull()) {
      continue;
    }
    Sequence *s = sequenceStore->modelForRecord(record);
    if (!s->isDefined()) {
      continue;
    }
    /* WARNING : values must be ordered by name index (u then v then w) because
     * approximateAtContextRank needs it. */
    int index = SequenceStore::SequenceIndexForName(s->fullName()[0]);
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
   * approximateAtContextRank will handle potential dependencies. */
  for (int i = start; i < stop; i++) {
    if (!sequencesToUpdate[i]) {
      continue;
    }
    for (int j = start; j < stop; j++) {
      if (!sequencesToUpdate[j]) {
        continue;
      }
      T *sequencePointer =
          sequencesRankValues + j * (SequenceStore::k_maxRecurrenceDepth + 1);
      if (std::isnan(*sequencePointer)) {
        *sequencePointer = sequencesToUpdate[j]->approximateAtContextRank<T>(
            m_sequenceContext, independent);
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
  constexpr int numberOfSequencesNames =
      std::size(SequenceStore::k_sequenceNames);
  for (int i = 0; i < numberOfSequencesNames; i++) {
    if (strncmp(identifier, SequenceStore::k_sequenceNames[i], length) == 0) {
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
