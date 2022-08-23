#ifndef ESCHER_SCROLLABLE_EXPRESSION_VIEW_H
#define ESCHER_SCROLLABLE_EXPRESSION_VIEW_H

#include <escher/scrollable_view.h>
#include <escher/scroll_view_data_source.h>
#include <escher/expression_view.h>

namespace Escher {

class ScrollableExpressionView : public ScrollableView, public ScrollViewDataSource {
public:
  ScrollableExpressionView(Responder * parentResponder,
                           KDCoordinate leftRightMargin,
                           KDCoordinate topBottomMargin,
                           float horizontalAlignment = KDContext::k_alignLeft,
                           float verticalAlignment = KDContext::k_alignCenter,
                           KDColor textColor = KDColorBlack,
                           KDColor backgroundColor = KDColorWhite,
                           const KDFont * font = KDFont::LargeFont);
  Poincare::Layout layout() const;
  const KDFont * font() const { return m_expressionView.font(); }
  void setLayout(Poincare::Layout layout);
  void setTextColor(KDColor color) { m_expressionView.setTextColor(color); }
  void setBackgroundColor(KDColor backgroundColor) override;
  void setExpressionBackgroundColor(KDColor backgroundColor);
private:
  ExpressionView m_expressionView;
};

}
#endif
