#ifndef POINCARE_PIECEWISE_OPERATOR_LAYOUT_NODE_H
#define POINCARE_PIECEWISE_OPERATOR_LAYOUT_NODE_H

#include <poincare/grid_layout.h>
#include <poincare/layout_cursor.h>
#include <poincare/layout.h>

namespace Poincare {

class PiecewiseOperatorLayoutNode final : public GridLayoutNode {
  friend class PiecewiseOperatorLayout;
public:
  using GridLayoutNode::GridLayoutNode;

  // Layout
  Type type() const override { return Type::PiecewiseOperatorLayout; }

  // SerializableNode
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;

  // TreeNode
  size_t size() const override { return sizeof(PiecewiseOperatorLayoutNode); }
#if POINCARE_TREE_LOG
  void logNodeName(std::ostream & stream) const override {
    stream << "PiecewiseOperatorLayout";
  }
#endif

private:
  // Grid layout node
  KDCoordinate horizontalGridEntryMargin(KDFont::Size font) const override { return 2 * k_gridEntryMargin + KDFont::GlyphWidth(font); }
  bool numberOfColumnsIsFixed() const override { return true; }

  // LayoutNode
  KDSize computeSize(KDFont::Size font) override;
  KDPoint positionOfChild(LayoutNode * l, KDFont::Size font) override;
  KDCoordinate computeBaseline(KDFont::Size font) override;
  void render(KDContext * ctx, KDPoint p, KDFont::Size font, KDColor expressionColor, KDColor backgroundColor, Layout * selectionStart = nullptr, Layout * selectionEnd = nullptr, KDColor selectionColor = KDColorRed) override;

};

class PiecewiseOperatorLayout /*final*/ : public GridLayout {
  friend class PiecewiseOperatorLayoutNode;
public:
  PiecewiseOperatorLayout(const PiecewiseOperatorLayoutNode * n) : GridLayout(n) {}
  static PiecewiseOperatorLayout EmptyPiecewiseOperatorBuilder();
private:
  PiecewiseOperatorLayoutNode * node() const { return static_cast<PiecewiseOperatorLayoutNode *>(Layout::node()); }
};

}

#endif
