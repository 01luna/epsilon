#ifndef POINCARE_EQUAL_H
#define POINCARE_EQUAL_H

#include <poincare/comparison_operator.h>

namespace Poincare {

class EqualNode final : public ComparisonOperatorNode {
public:

  // TreeNode
  size_t size() const override { return sizeof(EqualNode); }
#if POINCARE_TREE_LOG
  void logNodeName(std::ostream & stream) const override {
    stream << "Equal";
  }
#endif
  // ExpressionNode
  Type type() const override { return Type::Equal; }
private:
  // ComparisonOperatorNode
  CodePoint comparisonCodePoint() const override { return '='; };
  const char * comparisonString() const override { return "="; };
};

class Equal final : public ExpressionTwoChildren<Equal, EqualNode, ComparisonOperator> {
public:
  using ExpressionBuilder::ExpressionBuilder;
};

class NotEqualNode final : public ComparisonOperatorNode {
public:

  // TreeNode
  size_t size() const override { return sizeof(NotEqualNode); }
#if POINCARE_TREE_LOG
  void logNodeName(std::ostream & stream) const override {
    stream << "Equal";
  }
#endif
  // ExpressionNode
  Type type() const override { return Type::NotEqual; }
private:
  // ComparisonOperatorNode
  CodePoint comparisonCodePoint() const override { return UCodePointNotEqual; };
  const char * comparisonString() const override { return "≠"; };
};

class NotEqual final : public ExpressionTwoChildren<NotEqual, NotEqualNode, ComparisonOperator> {
public:
  using ExpressionBuilder::ExpressionBuilder;
};

}

#endif
