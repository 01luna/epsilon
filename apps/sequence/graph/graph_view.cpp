#include "graph_view.h"

#include <cmath>

using namespace Poincare;
using namespace Shared;

namespace Sequence {

GraphView::GraphView(SequenceStore* sequenceStore,
                     InteractiveCurveViewRange* graphRange,
                     CurveViewCursor* cursor, BannerView* bannerView,
                     CursorView* cursorView)
    : FunctionGraphView(graphRange, cursor, bannerView, cursorView),
      m_sequenceStore(sequenceStore) {}

void GraphView::drawRecord(Ion::Storage::Record record, int index,
                           KDContext* ctx, KDRect rect,
                           bool firstDrawnRecord) const {
  Shared::Sequence* s = m_sequenceStore->modelForRecord(record);

  int xStep = static_cast<int>(std::ceil(pixelWidth()));
  int xMin = static_cast<int>(std::ceil(range()->xMin()));
  int xMax = static_cast<int>(std::floor(range()->xMax()));
  assert(xStep > 0 && xMin <= xMax);

  for (int x = xMin; x <= xMax; x += xStep) {
    float y = s->evaluateXYAtParameter(static_cast<float>(x), context()).y();
    if (std::isnan(y)) {
      continue;
    }
    drawDot(ctx, rect, Dots::Size::Tiny, Coordinate2D<float>(x, y), s->color());
    if (x >= m_highlightedStart && x <= m_highlightedEnd &&
        record == m_selectedRecord) {
      KDColor color = m_shouldColorHighlighted ? s->color() : KDColorBlack;
      drawStraightSegment(ctx, rect, Axis::Vertical, x, y, 0.f, color);
    }
  }
}

}  // namespace Sequence
