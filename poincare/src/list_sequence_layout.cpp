#include <poincare/list_sequence_layout.h>
#include <poincare/code_point_layout.h>
#include <poincare/curly_brace_layout.h>
#include <poincare/list_sequence.h>
#include <poincare/serialization_helper.h>
#include <escher/metric.h>
#include <algorithm>

namespace Poincare {

int ListSequenceLayoutNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, ListSequence::s_functionHelper.name(), true);
}

void ListSequenceLayoutNode::moveCursorLeft(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool forSelection) {
  if (cursor->layoutNode() == variableLayout()) {
    assert(cursor->position() == LayoutCursor::Position::Left);
    cursor->setLayoutNode(functionLayout());
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  if (cursor->layoutNode() == functionLayout()) {
    assert(cursor->position() == LayoutCursor::Position::Left);
    cursor->setLayoutNode(this);
    cursor->setPosition(LayoutCursor::Position::Left);
    return;
  }
  if (cursor->layoutNode() == upperBoundLayout()) {
    assert(cursor->position() == LayoutCursor::Position::Left);
    cursor->setLayoutNode(variableLayout());
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  assert(cursor->layoutNode() == this);
  if (cursor->position() == LayoutCursor::Position::Right) {
    cursor->setLayoutNode(upperBoundLayout());
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  assert(cursor->position() == LayoutCursor::Position::Left);
  LayoutNode * parentNode = parent();
  if (parentNode) {
    parentNode->moveCursorLeft(cursor, shouldRecomputeLayout);
  }
}

void ListSequenceLayoutNode::moveCursorRight(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool forSelection) {
  if (cursor->layoutNode() == variableLayout()) {
    assert(cursor->position() == LayoutCursor::Position::Right);
    cursor->setLayoutNode(upperBoundLayout());
    cursor->setPosition(LayoutCursor::Position::Left);
    return;
  }
  if (cursor->layoutNode() == functionLayout()) {
    assert(cursor->position() == LayoutCursor::Position::Right);
    cursor->setLayoutNode(variableLayout());
    cursor->setPosition(LayoutCursor::Position::Left);
    return;
  }
  if (cursor->layoutNode() == upperBoundLayout()) {
    assert(cursor->position() == LayoutCursor::Position::Right);
    cursor->setLayoutNode(this);
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  assert(cursor->layoutNode() == this);
  if (cursor->position() == LayoutCursor::Position::Left) {
    cursor->setLayoutNode(functionLayout());
    cursor->setPosition(LayoutCursor::Position::Left);
    return;
  }
  assert(cursor->position() == LayoutCursor::Position::Right);
  LayoutNode * parentNode = parent();
  if (parentNode) {
    parentNode->moveCursorRight(cursor, shouldRecomputeLayout);
  }
}

Layout ListSequenceLayoutNode::XNTLayout(int childIndex) const {
  if (childIndex == k_functionLayoutIndex) {
    return Layout(childAtIndex(k_variableLayoutIndex)).clone();
  }
  if (childIndex == k_variableLayoutIndex) {
    return CodePointLayout::Builder(CodePoint(ListSequence::k_defaultXNTChar));
  }
  return LayoutNode::XNTLayout();
}

KDSize ListSequenceLayoutNode::computeSize(const KDFont * font) {
  KDPoint upperBoundPosition = positionOfChild(upperBoundLayout(), font);
  KDSize upperBoundSize = upperBoundLayout()->layoutSize(font);
  return KDSize(
      upperBoundPosition.x() + upperBoundSize.width(),
      std::max(upperBoundPosition.y() + upperBoundSize.height(), positionOfVariable(font).y() + variableLayout()->layoutSize(font).height()));
}

KDPoint ListSequenceLayoutNode::positionOfChild(LayoutNode * child, const KDFont * font) {
  if (child == variableLayout()) {
    return positionOfVariable(font);
  }
  if (child == functionLayout()) {
    return KDPoint(CurlyBraceLayoutNode::k_curlyBraceWidth, baseline(font) - functionLayout()->baseline(font));
  }
  assert(child == upperBoundLayout());
  return KDPoint(
      positionOfVariable(font).x() + variableLayout()->layoutSize(font).width() + font->stringSize("≤").width(),
      variableSlotBaseline(font) - upperBoundLayout()->baseline(font));
}

KDPoint ListSequenceLayoutNode::positionOfVariable(const KDFont * font) {
  return KDPoint(
     k_variableHorizontalMargin + bracesWidth(font),
     variableSlotBaseline(font) - variableLayout()->baseline(font));
}

KDCoordinate ListSequenceLayoutNode::variableSlotBaseline(const KDFont * font) {
  return std::max({static_cast<int>(CurlyBraceLayoutNode::HeightGivenChildHeight(functionLayout()->layoutSize(font).height()) + k_variableBaselineOffset),
                  static_cast<int>(upperBoundLayout()->baseline(font)),
                  static_cast<int>(variableLayout()->baseline(font))});
}

KDCoordinate ListSequenceLayoutNode::computeBaseline(const KDFont * font) {
  return CurlyBraceLayoutNode::BaselineGivenChildHeightAndBaseline(functionLayout()->layoutSize(font).height(), functionLayout()->baseline(font));
}

KDCoordinate ListSequenceLayoutNode::bracesWidth(const KDFont * font) {
  return 2 * CurlyBraceLayoutNode::k_curlyBraceWidth + functionLayout()->layoutSize(font).width();
}

void ListSequenceLayoutNode::render(KDContext * ctx, KDPoint p, const KDFont * font, KDColor expressionColor, KDColor backgroundColor, Layout * selectionStart, Layout * selectionEnd, KDColor selectionColor) {
  // Draw {  }
  KDSize functionSize = functionLayout()->layoutSize(font);
  KDPoint functionPosition = positionOfChild(functionLayout(), font);
  KDCoordinate functionBaseline = functionLayout()->baseline(font);

  KDPoint leftBracePosition = LeftCurlyBraceLayoutNode::PositionGivenChildHeightAndBaseline(functionSize, functionBaseline).translatedBy(functionPosition);
  LeftCurlyBraceLayoutNode::RenderWithChildHeight(functionSize.height(), ctx, leftBracePosition.translatedBy(p), expressionColor, backgroundColor);

  KDPoint rightBracePosition = RightCurlyBraceLayoutNode::PositionGivenChildHeightAndBaseline(functionSize, functionBaseline).translatedBy(functionPosition);
  RightCurlyBraceLayoutNode::RenderWithChildHeight(functionSize.height(), ctx, rightBracePosition.translatedBy(p), expressionColor, backgroundColor);

  // Draw k≤...
  KDPoint inferiorEqualPosition = KDPoint(positionOfVariable(font).x() + variableLayout()->layoutSize(font).width(), variableSlotBaseline(font) - font->glyphSize().height() / 2);
  ctx->drawString("≤", inferiorEqualPosition.translatedBy(p), font, expressionColor, backgroundColor);
}

}
