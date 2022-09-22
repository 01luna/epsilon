#ifndef SHARED_RING_CURSOR_VIEW_H
#define SHARED_RING_CURSOR_VIEW_H

#include "memoized_cursor_view.h"
#include "dots.h"

namespace Shared {

class RingCursorView : public MemoizedCursorView {
  void drawCursor(KDContext * ctx, KDRect rect) const override;
  KDCoordinate size() const override { return Dots::LargeRingDiameter; }
  KDColor * underneathPixelBuffer() const override { return m_underneathPixelBuffer; }
  mutable KDColor m_underneathPixelBuffer[Dots::LargeRingDiameter * Dots::LargeRingDiameter];
};

}

#endif
