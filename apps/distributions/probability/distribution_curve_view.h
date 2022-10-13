#ifndef DISTRIBUTIONS_PROBABILITY_DISTRIBUTION_CURVE_VIEW_H
#define DISTRIBUTIONS_PROBABILITY_DISTRIBUTION_CURVE_VIEW_H

#include "../models/calculation/calculation.h"
#include "../models/distribution/distribution.h"
#include <apps/shared/plot_view_policies.h>
#include <poincare/coordinate_2D.h>

namespace Distributions {

class DistributionPlotPolicy : public Shared::PlotPolicy::WithCurves, public Shared::PlotPolicy::WithHistogram {
protected:
  void drawPlot(const Shared::AbstractPlotView * plotView, KDContext * ctx, KDRect rect) const;
  void drawStandardNormal(const Shared::AbstractPlotView * plotView, KDContext * ctx, KDRect rect, float colorLowerBound, float colorUpperBound) const;

  Distribution * m_distribution;
  Calculation * m_calculation;
};

class DistributionCurveView : public Shared::PlotView<Shared::PlotPolicy::LabeledXAxis, DistributionPlotPolicy, Shared::PlotPolicy::NoBanner, Shared::PlotPolicy::NoCursor> {
public:
  DistributionCurveView(Distribution * distribution, Calculation * calculation);

  // AbstractPlotView
  void reload(bool resetInterruption = false, bool force = false) override;

private:
  constexpr static KDColor k_backgroundColor = Escher::Palette::WallScreen;

  void drawBackground(KDContext * ctx, KDRect rect) const override { ctx->fillRect(rect, k_backgroundColor); }
};

}

#endif
