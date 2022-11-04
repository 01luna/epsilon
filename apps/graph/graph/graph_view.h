#ifndef GRAPH_GRAPH_VIEW_H
#define GRAPH_GRAPH_VIEW_H

#include <apps/shared/continuous_function.h>
#include <apps/shared/function_graph_view.h>

namespace Graph {

class GraphView : public Shared::FunctionGraphView, public Shared::PlotPolicy::WithCurves {
public:
  GraphView(Shared::InteractiveCurveViewRange * graphRange, Shared::CurveViewCursor * cursor, Shared::BannerView * bannerView, Shared::CursorView * cursorView);

  // FunctionGraphView
  void reload(bool resetInterrupted = false, bool force = false) override;
  /* We override setAreaHighlightColor to make it reload nothing as the
   * highlightColor and the non-highlightColor are identical in the graph view
   * of the application graph. We thereby avoid to uselessly reload some part
   * of the graph where the area under the curve is colored. */
  void setAreaHighlightColor(bool highlightColor) override {};
  int numberOfDrawnRecords() const override;
  void drawRecord(int i, KDContext *, KDRect) const override;
  void drawForeground(KDContext *, KDRect) const override;
  void tidyModel(int i) const override;
  void setFocus(bool focus) override;

  void drawTangent(bool tangent) { m_tangent = tangent; }
  void setInterest(Poincare::Solver<double>::Interest interest) { m_interest = interest; }

private:
  constexpr static int k_externRectMargin = 2;

  static bool FunctionIsDiscontinuousBetweenFloatValues(float x1, float x2, void * model, void * context);

  void drawCartesian(KDContext * ctx, KDRect rect, Shared::ContinuousFunction * f, Ion::Storage::Record record, float tMin, float tMax, float tStep, DiscontinuityTest discontinuity, Axis axis, KDCoordinate rectMin, KDCoordinate rectMax) const;
  void drawPolar(KDContext * ctx, KDRect rect, Shared::ContinuousFunction * f, float tMin, float tMax, float tStep, DiscontinuityTest discontinuity) const;
  void drawParametric(KDContext * ctx, KDRect rect, Shared::ContinuousFunction * f, float tMin, float tMax, float tStep, DiscontinuityTest discontinuity) const;

  mutable int m_areaIndex;
  Poincare::Solver<double>::Interest m_interest;
  bool m_tangent;
};

}

#endif
