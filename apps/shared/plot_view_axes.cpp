#include "plot_view_axes.h"
#include <cmath>

using namespace Poincare;

namespace Shared {
namespace PlotPolicy {

// WithGrid

void WithGrid::drawGrid(const AbstractPlotView * plotView, KDContext * ctx, KDRect rect) const {
  drawGridLines(plotView, ctx, rect, AbstractPlotView::Axis::Horizontal);
  drawGridLines(plotView, ctx, rect, AbstractPlotView::Axis::Vertical);
}

void WithGrid::drawGridLines(const AbstractPlotView * plotView, KDContext * ctx, KDRect rect, AbstractPlotView::Axis parallel) const {
  assert(plotView);
  float min, max, step;
  CurveViewRange * range = plotView->range();
  if (parallel == AbstractPlotView::Axis::Horizontal) {
    min = range->yMin();
    max = range->yMax();
    step = range->yGridUnit();
  } else {
    min = range->xMin();
    max = range->xMax();
    step = range->xGridUnit();
  }

  int iMax = max / step;
  for (int i = min / step; i <= iMax; i++) {
    plotView->drawStraightSegment(ctx, rect, parallel, i * step, -INFINITY, INFINITY, i % 2 == 0 ? k_boldColor : k_lightColor);
  }
}

// UnlabelledAxis

void SimpleAxis::drawAxis(const AbstractPlotView * plotView, KDContext * ctx, KDRect rect, AbstractPlotView::Axis axis) const {
  assert(plotView);

  AbstractPlotView::Axis otherAxis;
  float tMin, tMax;
  float labelPos = 0.f;
  AbstractPlotView::RelativePosition labelRelativeX, labelRelativeY;
  if (axis == AbstractPlotView::Axis::Horizontal) {
    otherAxis = AbstractPlotView::Axis::Vertical;
    tMin = plotView->range()->xMin();
    tMax = plotView->range()->xMax();
    labelRelativeX = AbstractPlotView::RelativePosition::There;
    labelRelativeY = AbstractPlotView::RelativePosition::After;
  } else {
    otherAxis = AbstractPlotView::Axis::Horizontal;
    tMin = plotView->range()->yMin();
    tMax = plotView->range()->yMax();
    labelRelativeX = AbstractPlotView::RelativePosition::Before;
    labelRelativeY = AbstractPlotView::RelativePosition::There;
  }
  float graduationFloatLength = k_labelGraduationHalfLength * plotView->pixelLength(otherAxis);

  if (tMin >= 0.f) {
    labelPos = tMin;
    if (axis == AbstractPlotView::Axis::Horizontal) {
      labelRelativeY = AbstractPlotView::RelativePosition::Before;
    } else {
      labelRelativeX = AbstractPlotView::RelativePosition::After;
    }
  } else if (tMax <= 0.f) {
    labelPos = tMax;
    if (axis == AbstractPlotView::Axis::Horizontal) {
      labelRelativeY = AbstractPlotView::RelativePosition::After;
    } else {
      labelRelativeX = AbstractPlotView::RelativePosition::Before;
    }
  }

  // - Draw plain axis
  plotView->drawStraightSegment(ctx, rect, axis, 0.f, -INFINITY, INFINITY, k_color);

  // - Draw ticks and eventual labels
  int i = 0;
  float t = labelPosition(i, plotView, axis);
  while (t <= tMax) {
    if (t == 0.f) {
      if (axis == AbstractPlotView::Axis::Horizontal) {
        /* FIXME This case is tied to the presence of a second axis. Get rid of
         * this coupling. */
        plotView->drawLabel(ctx, rect, "0", Coordinate2D<float>(0.f, 0.f), AbstractPlotView::RelativePosition::Before, labelRelativeY, k_color);
      }
    } else {
      plotView->drawStraightSegment(ctx, rect, otherAxis, t, -graduationFloatLength, graduationFloatLength, k_color);
      const char * text = labelText(i);
      if (text) {
        Coordinate2D<float> xy = axis == AbstractPlotView::Axis::Horizontal ? Coordinate2D<float>(t, labelPos) : Coordinate2D<float>(labelPos, t);
        plotView->drawLabel(ctx, rect, text, xy, labelRelativeX, labelRelativeY, k_color);
      }
    }

    i++;
    t = labelPosition(i, plotView, axis);
  }
}

float SimpleAxis::labelPosition(int i, const AbstractPlotView * plotView, AbstractPlotView::Axis axis) const {
  float step = labelStep(plotView, axis);
  float tMin = axis == AbstractPlotView::Axis::Horizontal ? plotView->range()->xMin() : plotView->range()->yMin();
  assert(std::fabs(std::round(tMin / step)) < INT_MAX);
  int indexOfOrigin = std::round(-tMin / step) - 1;
  return step * (i - indexOfOrigin);
}

float SimpleAxis::labelStep(const AbstractPlotView * plotView, AbstractPlotView::Axis axis) const {
  float step = axis == AbstractPlotView::Axis::Horizontal ? plotView->range()->xGridUnit() : plotView->range()->yGridUnit();
  return 2.f * step;
}

// LabeledAxis

void LabeledAxis::reloadAxis(AbstractPlotView * plotView, AbstractPlotView::Axis axis) {
  for (size_t i = 0; i < k_maxNumberOfLabels; i++) {
    computeLabel(i, plotView, axis);
  }
}

const char * LabeledAxis::labelText(int i) const {
  assert(i < k_maxNumberOfLabels);
  return m_labels[i];
}

int LabeledAxis::computeLabel(int i, const AbstractPlotView * plotView, AbstractPlotView::Axis axis)  {
  float t = labelPosition(i, plotView, axis);
  return Poincare::PrintFloat::ConvertFloatToText(t, m_labels[i], k_labelBufferMaxSize, k_labelBufferMaxGlyphLength, k_numberSignificantDigits, Preferences::PrintFloatMode::Decimal).GlyphLength;
}

}
}
