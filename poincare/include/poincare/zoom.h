#ifndef POINCARE_ZOOM_H
#define POINCARE_ZOOM_H

#include <poincare/range.h>
#include <poincare/solver.h>
#include <ion/display.h>

/* The unit tests need to be able to read the working values of
 * m_interestingRange and m_magnitudeYRange, but we do not want to make public
 * getters for those as it would weaken the Zoom API. */
class ZoomTest;

namespace Poincare {

class Zoom {
  friend class ::ZoomTest;
public:
  constexpr static float k_smallUnitMantissa = 1.f;
  constexpr static float k_mediumUnitMantissa = 2.f;
  constexpr static float k_largeUnitMantissa = 5.f;

  typedef Coordinate2D<float> (*Function2DWithContext)(float, const void *, Context *);

  /* Sanitize will turn any random range into a range fit for display (see
   * comment on range() method below), that includes the original range. */
  static Range2D Sanitize(Range2D range, float normalRatio, float maxFloat);
  static Range2D DefaultRange(float normalRatio, float maxFloat) { return Sanitize(Range2D(), normalRatio, maxFloat); }

  Zoom(float tMin, float tMax, float normalRatio, Context * context) : m_bounds(tMin, tMax), m_context(context), m_normalRatio(normalRatio) {
    /* The calculator screen is wider than it is high, but nothing in Zoom
     * relies on this assumption. */
    // assert(m_normalRatio < 1.f);
  }

  /* This method is guaranteed to return a displayable range, that is a range
   * with non-nan bounds, and non-empty axes of some minimal length, with
   * bounds smaller than maxFloat in absolute value. */
  Range2D range(float maxFloat, bool forceNormalization) const;
  void setBounds(float min, float max) { m_bounds = Range1D(min, max); }
  void setForcedRange(Range2D range) { m_forcedRange = range; }
  /* These three functions will extend both X and Y axes. */
  void fitFullFunction(Function2DWithContext f, const void * model);
  void fitPointsOfInterest(Function2DWithContext f, const void * model, bool vertical = false);
  void fitIntersections(Function2DWithContext f1, const void * model1, Function2DWithContext f2, const void * model2, bool vertical = false);
  /* This function will only touch the Y axis. */
  void fitMagnitude(Function2DWithContext f, const void * model, bool vertical = false);

private:
  class HorizontalAsymptoteHelper {
    /* This helper is used to keep track of the slope at each step of the
     * search for points of interest, in order to detect horizontal asymptotes.
     * - if the bound is INFINITY, no slope greater than the threshold has been
     *   found yet.
     * - if the bound is NAN, the last slope was greater than the threshold,
     *   and finding a slope lower than the threshold will mark the beginning
     *   of an asymptote.
     * - if the bound is finite, we have found an asymptote ; finding a slope
     *   greater than the threshold will invalidate it.
     * We introduce hysteresis to avoid constantly finding and invalidating
     * asymptotes on functions such as y=0.2x, which oscillates around the
     * threshold due to imprecisions. */
  public:
    HorizontalAsymptoteHelper(float center) : m_center(center), m_left(-INFINITY, NAN), m_right(INFINITY, NAN) {}

    Coordinate2D<float> left() const { return privateGet(&m_left); }
    Coordinate2D<float> right() const { return privateGet(&m_right); }
    void update(Coordinate2D<float> x, float slope);

  private:
    constexpr static float k_threshold = 0.2f; // TODO Tune
    constexpr static float k_hysteresis = 0.01f; // TODO Tune

    Coordinate2D<float> privateGet(const Coordinate2D<float> * p) const { return std::isfinite(p->x1()) ? *p : Coordinate2D<float>(); }

    float m_center;
    Coordinate2D<float> m_left;
    Coordinate2D<float> m_right;
  };

  struct InterestParameters {
    Function2DWithContext f;
    const void * model;
    Context * context;
    HorizontalAsymptoteHelper * asymptotes;
    float (Coordinate2D<float>::*ordinate)() const;
  };

  constexpr static size_t k_sampleSize = Ion::Display::Width / 2;

  // Static methods for the Solver API
  static Solver<float>::Interest PointIsInteresting(Coordinate2D<float> a, Coordinate2D<float> b, Coordinate2D<float> c, const void * aux);
  static Coordinate2D<float> HonePoint(Solver<float>::FunctionEvaluation f, const void * aux, float a, float b, Solver<float>::Interest, float precision);

  Range2D sanitizedRange() const;
  Range2D prettyRange(bool forceNormalization) const;
  void fitWithSolver(bool * leftInterrupted, bool * rightInterrupted, Solver<float>::FunctionEvaluation evaluator, const void * aux, Solver<float>::BracketTest test, Solver<float>::HoneResult hone, bool vertical);
  /* Return true if the search was interrupted because too many points were
   * found. */
  bool fitWithSolverHelper(float start, float end, Solver<float>::FunctionEvaluation evaluator, const void * aux, Solver<float>::BracketTest test, Solver<float>::HoneResult hone, bool vertical);

  /* m_interestingRange is edited by fitFullFunction, fitPointsOfInterest and
   * fitIntersections, and will always be included in the final range. */
  Range2D m_interestingRange;
  Range2D m_magnitudeRange;
  Range2D m_forcedRange;
  Range1D m_bounds;
  Context * m_context;
  float m_normalRatio;
};

}

#endif
