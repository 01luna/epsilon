#ifndef SHARED_PLOT_VIEW_H
#define SHARED_PLOT_VIEW_H

#include "banner_view.h"
#include "cursor_view.h"
#include "curve_view_range.h"
#include "dots.h"
#include <poincare/coordinate_2D.h>

/* AbstractPlotView maps a range in R² to the screen and provides methods for
 * drawing basic geometry in R².
 * To instanciate a view, one can use or derive the
 * PlotView<CAXes, CPlot, CBanner, CCursor> template. */

namespace Shared {

class AbstractPlotView : public Escher::View {
public:
  enum class Axis : uint8_t {
    Horizontal = 0,
    Vertical,
  };
  enum class RelativePosition : uint8_t {
    Before,
    There,
    After,
  };

  constexpr static KDColor k_backgroundColor = KDColorWhite;
  constexpr static KDCoordinate k_labelMargin = 4;

  constexpr static Axis OtherAxis(Axis axis) { return static_cast<Axis>(1 - static_cast<uint8_t>(axis)); }

  AbstractPlotView(CurveViewRange * range) : m_range(range), m_stampDashIndex(k_stampIndexNoDash) {}

  // Escher::View
  void drawRect(KDContext * ctx, KDRect rect) const final;

  virtual void reload(bool resetInterruption = false, bool force = false);
  virtual void resetInterruption() {}
  CurveViewRange * range() const { return m_range; }
  bool hasFocus() const { return m_focus; }
  void setFocus(bool f);
  float rangeMin(Axis axis) const { return axis == Axis::Horizontal ? m_range->xMin() : m_range->yMin(); }
  float rangeMax(Axis axis) const { return axis == Axis::Horizontal ? m_range->xMax() : m_range->yMax(); }
  float pixelWidth() const { return (m_range->xMax() - m_range->xMin()) / (bounds().width() - 1); }
  float pixelHeight() const { return (m_range->yMax() - m_range->yMin()) / (bounds().height() - 1); }
  float pixelLength(Axis axis) const { return axis == Axis::Horizontal ? pixelWidth() : pixelHeight(); }
  float floatToPixel(Axis axis, float f) const;
  float pixelToFloat(Axis axis, KDCoordinate c) const;
  Poincare::Coordinate2D<float> floatToPixel2D(Poincare::Coordinate2D<float> p) const { return Poincare::Coordinate2D<float>(floatToPixel(Axis::Horizontal, p.x1()), floatToPixel(Axis::Vertical, p.x2())); }

  // Methods for drawing basic geometry
  /* FIXME These methods are public as they need to be accessible to helper
   * classes used as template parameters to PlotView. A better solution might
   * be to private them and befriend the helpers. */
  void drawStraightSegment(KDContext * ctx, KDRect rect, Axis parallel, float position, float min, float max, KDColor color, KDCoordinate thickness = 1, KDCoordinate dashSize = 0) const;
  void drawSegment(KDContext * ctx, KDRect rect, Poincare::Coordinate2D<float> a, Poincare::Coordinate2D<float> b, KDColor color, bool thick = false) const;
  void drawLabel(KDContext * ctx, KDRect rect, const char * label, Poincare::Coordinate2D<float> xy, RelativePosition xPosition, RelativePosition yPosition, KDColor color) const;
  void drawDot(KDContext * ctx, KDRect rect, Dots::Size size, Poincare::Coordinate2D<float> xy, KDColor color) const;
  /* These methods use the stamping state-machine.
   * FIXME They may be moved into a helper. */
  void setDashed(bool dashed) const { m_stampDashIndex = dashed ? k_stampIndexNoDash : 0; }
  void straightJoinDots(KDContext * ctx, KDRect rect, Poincare::Coordinate2D<float> pixelA, Poincare::Coordinate2D<float> pixelB, KDColor color, bool thick) const;
  void stamp(KDContext * ctx, KDRect rect, Poincare::Coordinate2D<float> p, KDColor color, bool thick) const;
  bool pointsInSameStamp(Poincare::Coordinate2D<float> p1, Poincare::Coordinate2D<float> p2, bool thick) const;

private:
  constexpr static KDCoordinate k_stampDashSize = 5;
  constexpr static KDCoordinate k_stampIndexNoDash = -1;
  constexpr static KDFont::Size k_font = KDFont::Size::Small;

  // Escher::View
  int numberOfSubviews() const { return (bannerView() != nullptr) + (cursorView() != nullptr); }
  Escher::View * subviewAtIndex(int i);
  void layoutSubviews(bool force = false);

  virtual void drawBackground(KDContext * ctx, KDRect rect) const { ctx->fillRect(rect, k_backgroundColor); }
  virtual void drawAxes(KDContext * ctx, KDRect rect) const = 0;
  virtual void reloadAxes() = 0;
  virtual void drawPlot(KDContext * ctx, KDRect rect) const = 0;
  virtual BannerView * bannerView() const = 0;
  virtual KDRect bannerFrame() = 0;
  virtual CursorView * cursorView() const = 0;
  virtual KDRect cursorFrame() = 0;

  CurveViewRange * m_range;
  mutable KDCoordinate m_stampDashIndex;
  uint32_t m_drawnRangeVersion;
  bool m_focus;
};

template<class CAxes, class CPlot, class CBanner, class CCursor>
class PlotView : public AbstractPlotView, protected CAxes, protected CPlot, protected CBanner, protected CCursor {
public:
  using AbstractPlotView::AbstractPlotView;

private:
  void drawAxes(KDContext * ctx, KDRect rect) const override { CAxes::drawAxes(this, ctx, rect); }
  void reloadAxes() override { CAxes::reloadAxes(this); }
  void drawPlot(KDContext * ctx, KDRect rect) const override { CPlot::drawPlot(this, ctx, rect); }
  BannerView * bannerView() const override { return CBanner::bannerView(this); }
  KDRect bannerFrame() override { return CBanner::bannerFrame(this); }
  CursorView * cursorView() const override { return CCursor::cursorView(this); }
  KDRect cursorFrame() override { return CCursor::cursorFrame(this); }
};

}

#endif
