#include "vertical_sequence_title_cell.h"
#include <poincare/layout.h>
#include <algorithm>

namespace Sequence {

VerticalSequenceTitleCell::VerticalSequenceTitleCell() : SequenceTitleCell(k_font) {
  /* We do not care here about the vertical alignment, it will be set properly
   * in layoutSubviews */
  m_titleTextView.setAlignment(k_horizontalAlignment, 0.0f);
}

void VerticalSequenceTitleCell::drawRect(KDContext * ctx, KDRect rect) const {
  KDColor backgroundColor = m_even ? KDColorWhite : Escher::Palette::WallScreen;
  // Draw the color indicator
  ctx->fillRect(KDRect(0, 0, k_colorIndicatorThickness, bounds().height()), m_functionColor);
  // Draw some background
  ctx->fillRect(KDRect(bounds().width() - k_equalWidthWithMargins, 0, k_equalWidthWithMargins, bounds().height()), backgroundColor);
  // Draw '='
  KDPoint p = KDPoint(bounds().width() - k_equalWidthWithMargins, m_baseline - KDFont::Font(k_font)->glyphSize().height()/2 - 1); // -1 is visually needed
  ctx->drawString("=", p, k_font, m_functionColor, backgroundColor);
}

KDRect VerticalSequenceTitleCell::subviewFrame() const {
  return KDRect(k_colorIndicatorThickness, 0, bounds().width() - k_colorIndicatorThickness - k_equalWidthWithMargins, bounds().height());
}

void VerticalSequenceTitleCell::layoutSubviews(bool force) {
  m_titleTextView.setAlignment(k_horizontalAlignment, verticalAlignment());
  SequenceTitleCell::layoutSubviews(force);
}

float VerticalSequenceTitleCell::verticalAlignment() const {
  if (m_baseline < 0) {
    return KDContext::k_alignCenter;
  }
  Poincare::Layout l = layout();
  float alignment = static_cast<float>(m_baseline - l.baseline(k_font)) / static_cast<float>(subviewFrame().height() - l.layoutSize(k_font).height());
  return std::max(0.0f, std::min(1.0f, alignment));
}

}  // namespace Sequence
