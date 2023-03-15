#include "sequence_cache_context.h"

#include <poincare/addition.h>
#include <poincare/rational.h>
#include <poincare/serialization_helper.h>

#include <cmath>

#include "poincare_helpers.h"
#include "sequence.h"
#include "sequence_store.h"

namespace Shared {

template <typename T>
SequenceCacheContext<T>::SequenceCacheContext(SequenceContext *sequenceContext,
                                              int forbiddenSequenceIndex)
    : ContextWithParent(sequenceContext),
      m_values{{NAN, NAN}, {NAN, NAN}, {NAN, NAN}},
      m_sequenceContext(sequenceContext),
      m_forbiddenSequenceIndex(forbiddenSequenceIndex) {}

template <typename T>
const Poincare::Expression SequenceCacheContext<T>::protectedExpressionForSymbolAbstract(
    const Poincare::SymbolAbstract &symbol, bool clone,
    ContextWithParent *lastDescendantContext) {
  // [u|v|w](n(+1)?)
  if (symbol.type() == Poincare::ExpressionNode::Type::Sequence) {
    T result = NAN;
    int index = nameIndexForSymbol(const_cast<Poincare::Symbol &>(
        static_cast<const Poincare::Symbol &>(symbol)));
    Poincare::Expression rank = symbol.childAtIndex(0).clone();
    if (rank.isIdenticalTo(Poincare::Symbol::Builder(UCodePointUnknown))) {
      result = m_values[index][0];
    } else if (rank.isIdenticalTo(Poincare::Addition::Builder(
                   Poincare::Symbol::Builder(UCodePointUnknown),
                   Poincare::Rational::Builder(1)))) {
      result = m_values[index][1];
    }
    /* If the symbol was not in the two previous ranks, we try to approximate
     * the sequence independently from the others at the required rank (this
     * will solve u(n) = 5*n, v(n) = u(n+10) for instance). But we avoid doing
     * so if the sequence referencing itself to avoid an infinite loop. */
    if (std::isnan(result) && index != m_forbiddenSequenceIndex) {
      /* Do not use recordAtIndex : if the sequences have been reordered, the
       * name index and the record index may not correspond. */
      Ion::Storage::Record record =
          m_sequenceContext->sequenceStore()->recordAtNameIndex(index);
      if (!record.isNull()) {
        assert(record.fullName()[0] == symbol.name()[0]);
        Sequence *seq =
            m_sequenceContext->sequenceStore()->modelForRecord(record);
        /* The lastDesendantContext might contain informations on variables
         * that are contained in the rank expression. */
        T n = PoincareHelpers::ApproximateToScalar<T>(
            rank, lastDescendantContext ? lastDescendantContext : this);
        // In case the sequence referenced is not defined or if the rank is not
        // an int, return NAN
        if (seq->fullName() != nullptr) {
          if (std::floor(n) == n) {
            result = seq->valueAtRank<T>(n, m_sequenceContext);
          }
        }
      }
    }
    return Poincare::Float<T>::Builder(result);
  }
  return ContextWithParent::protectedExpressionForSymbolAbstract(
      symbol, clone, lastDescendantContext);
}

template <typename T>
void SequenceCacheContext<T>::setValue(T value, int nameIndex, int depth) {
  m_values[nameIndex][depth] = value;
}

template <typename T>
int SequenceCacheContext<T>::nameIndexForSymbol(
    const Poincare::Symbol &symbol) {
  char name = symbol.name()[0];
  assert(name >= 'u' && name <= 'w');  // u, v or w
  assert(
      name >= SequenceStore::k_sequenceNames[0][0] &&
      name <=
          SequenceStore::k_sequenceNames[SequenceStore::k_maxNumberOfSequences -
                                         1][0]);
  return name - 'u';
}

template class SequenceCacheContext<float>;
template class SequenceCacheContext<double>;

}  // namespace Shared
