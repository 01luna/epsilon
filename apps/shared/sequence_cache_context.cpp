#include "sequence_cache_context.h"

#include <poincare/addition.h>
#include <poincare/rational.h>
#include <poincare/serialization_helper.h>

#include <cmath>

#include "poincare_helpers.h"
#include "sequence.h"
#include "sequence_store.h"

using namespace Poincare;

namespace Shared {

template <typename T>
SequenceCacheContext<T>::SequenceCacheContext(SequenceContext *sequenceContext,
                                              int sequenceBeingComputed,
                                              bool independent)
    : ContextWithParent(sequenceContext),
      m_values{{NAN, NAN, NAN}, {NAN, NAN, NAN}, {NAN, NAN, NAN}},
      m_sequenceContext(sequenceContext),
      m_sequenceBeingComputed(sequenceBeingComputed),
      m_independent(independent) {}

template <typename T>
const Expression SequenceCacheContext<T>::protectedExpressionForSymbolAbstract(
    const SymbolAbstract &symbol, bool clone,
    ContextWithParent *lastDescendantContext) {
  if (symbol.type() != ExpressionNode::Type::Sequence) {
    return ContextWithParent::protectedExpressionForSymbolAbstract(
        symbol, clone, lastDescendantContext);
  }
  T result = NAN;
  /* Do not use recordAtIndex : if the sequences have been reordered, the
   * name index and the record index may not correspond. */
  char name = static_cast<const Symbol &>(symbol).name()[0];
  int index = SequenceStore::SequenceIndexForName(name);
  Ion::Storage::Record record =
      m_sequenceContext->sequenceStore()->recordAtNameIndex(index);
  if (record.isNull()) {
    return Float<T>::Builder(result);
  }
  assert(record.fullName()[0] == symbol.name()[0]);
  Sequence *seq = m_sequenceContext->sequenceStore()->modelForRecord(record);
  if (!seq->fullName()) {
    return Float<T>::Builder(result);
  }
  Expression rankExpression = symbol.childAtIndex(0).clone();
  for (int depth = 0; depth < SequenceStore::k_maxRecurrenceDepth + 1;
       depth++) {
    if (rankExpression.isRankNPlusK(depth)) {
      result = m_values[index][depth];
    }
  }
  /* If the symbol was not in the two previous ranks, we try to approximate
   * the sequence independently from the others at the required rank (this
   * will solve u(n) = 5*n, v(n) = u(n+10) for instance). But we avoid doing
   * so if the sequence referencing itself to avoid an infinite loop. */
  if (std::isnan(result) &&
      (!m_independent || index != m_sequenceBeingComputed)) {
    /* The lastDesendantContext might contain informations on variables
     * that are contained in the rank expression. */
    T rankValue = PoincareHelpers::ApproximateToScalar<T>(
        rankExpression, lastDescendantContext ? lastDescendantContext : this);
    // If the rank is not an int, return NAN
    if (std::floor(rankValue) == rankValue) {
      result = seq->valueAtRank<T>(rankValue, m_sequenceContext, true);
    }
  }
  return Float<T>::Builder(result);
}

template <typename T>
void SequenceCacheContext<T>::setValue(T value, int nameIndex, int depth) {
  assert(0 <= nameIndex && nameIndex < SequenceStore::k_maxNumberOfSequences);
  assert(0 <= depth && depth < SequenceStore::k_maxRecurrenceDepth + 1);
  m_values[nameIndex][depth] = value;
}

template class SequenceCacheContext<float>;
template class SequenceCacheContext<double>;

}  // namespace Shared
