#ifndef POINCARE_ZOOM_H
#define POINCARE_ZOOM_H

#include <poincare/range.h>
#include <poincare/solver.h>
#include <ion/display.h>

namespace Poincare {

class Zoom {
public:
  constexpr static float k_smallUnitMantissa = 1.f;
  constexpr static float k_mediumUnitMantissa = 2.f;
  constexpr static float k_largeUnitMantissa = 5.f;

  typedef Coordinate2D<float> (*Function2DWithContext)(float, const void *, Context *);

  static Range2D Sanitize(Range2D range, float normalRatio, float maxFloat);
  static Range2D DefaultRange(float normalRatio, float maxFloat) { return Sanitize(Range2D(), normalRatio, maxFloat); }

  Zoom(float tMin, float tMax, float normalRatio, Context * context) : m_bounds(tMin, tMax), m_context(context), m_normalRatio(normalRatio) {}

  /* This method is guaranteed to return a displayable range, that is a range
   * with non-nan bounds, and non-empty axes of some minimal length, with
   * bounds smaller than maxFloat in absolute value. */
  Range2D range(float maxFloat, bool forceNormalization) const;
  void setBounds(float min, float max) { m_bounds = Range1D(min, max); }
  /* These three functions will extend both X and Y axes. */
  void fitFullFunction(Function2DWithContext f, const void * model);
  void fitPointsOfInterest(Function2DWithContext f, const void * model);
  void fitIntersections(Function2DWithContext f1, const void * model1, Function2DWithContext f2, const void * model2);
  /* This function will only touch the Y axis. */
  void fitMagnitude(Function2DWithContext f, const void * model);

private:
  constexpr static size_t k_sampleSize = Ion::Display::Width / 2;
  constexpr static int k_maxPointsOnOneSide = 5;

  // Static methods for the Solver API
  static Solver<float>::Interest PointIsInteresting(float ya, float yb, float yc);
  static Coordinate2D<float> SelectFar(Solver<float>::FunctionEvaluation f, const void * model, float a, float b, Solver<float>::Interest, float precision) { return Coordinate2D<float>(b, f(b, model)); }

  Range1D sanitizedXRange() const;
  Range2D prettyRange() const;
  void fitWithSolver(Solver<float>::FunctionEvaluation evaluator, const void * aux, Solver<float>::BracketTest test);
  void fitWithSolverHelper(float start, float end, Solver<float>::FunctionEvaluation evaluator, const void * aux, Solver<float>::BracketTest test);

  /* m_interestingRange is edited by fitFullFunction, fitPointsOfInterest and
   * fitIntersections, and will always be included in the final range. */
  Range2D m_interestingRange;
  Range1D m_magnitudeYRange;
  Range1D m_bounds;
  Context * m_context;
  float m_normalRatio;
};

#if 0
class Zoom {
public:
  /* FIXME Removing the "context" argument from Solver<>::FunctionEvaluation
   * was shortsighted, and makes it necessary to hack a new lambda in
   * grossFitToInterest. */
  typedef Coordinate2D<float> (*FunctionEvaluation2DWithContext)(float, const void *, Context *);

  constexpr static float k_smallUnitMantissa = 1.f;
  constexpr static float k_mediumUnitMantissa = 2.f;
  constexpr static float k_largeUnitMantissa = 5.f;

  static Range2D Sanitize(Range2D range, float normalYXRatio);
  static Range2D DefaultRange(float normalYXRatio) { return Sanitize(Range2D(), normalYXRatio); }
  // Static methods for the Solver API
  static Solver<float>::Interest PointIsInteresting(float ya, float yb, float yc);
  static Coordinate2D<float> SelectFar(Solver<float>::FunctionEvaluation f, const void * model, float a, float b, Solver<float>::Interest, float precision) { return Coordinate2D<float>(b, f(b, model)); }


  /* A YX ratio is length of Y axis over length of X axis. For instance, a
   * normal YX ratio of 0.5 means the range ([-1,1],[2,3]) is normalized. */
  Zoom(float tMin, float tMax, float normalYXRatio, Context * context) : m_range(), m_tMax(tMax), m_tMin(tMin), m_normalRatio(normalYXRatio), m_context(context), m_function(nullptr), m_sampleUpToDate(false) {}

  Range2D range() const { return m_range; }
  void setBounds(float tMin, float tMax) { m_tMin = tMin; m_tMax = tMax; }
  void setFunction(FunctionEvaluation2DWithContext f, const void * model);
  void includePoint(Coordinate2D<float> p);
  void fitIntersections(FunctionEvaluation2DWithContext otherF, const void * otherModel);
  /* The fitX method will compute an X axis based on the points of interest of
   * the expression, or a default one if none are found. It will also compute
   * the Y range associated with those points. */
  void fitX();
  /* The fitOnlyY method is guaranteed to leave the X axis unmodified. */
  void fitOnlyY();
  /* This methods will tweak both X and Y to further refine the range. */
  void fitBothXAndY(bool forceNormalization);
  void fitFullFunction();

private:
  constexpr static size_t k_sampleSize = Ion::Display::Width / 2;
  constexpr static int k_maxPointsOnOneSide = 5;
  constexpr static float k_defaultHalfRange = Range1D::k_defaultHalfLength;
  /* The tolerance is chosen to normalize sqrt(x) */
  constexpr static float k_orthonormalTolerance = 1.78f;

  Coordinate2D<float> approximate(float x) const { assert(m_function); return m_function(x, m_model, m_context); }
  void sampleY();
  void fitUsingSolver(float xStart, float xEnd, Solver<float>::FunctionEvaluation eval, const void * aux, Solver<float>::BracketTest test);
  bool findNormalYAxis();
  bool findYAxisForOrderOfMagnitude();
  void expandSparseWindow();

  float m_sample[k_sampleSize];
  Range2D m_range;
  float m_tMax, m_tMin;
  float m_normalRatio;
  Context * m_context;
  FunctionEvaluation2DWithContext m_function;
  const void * m_model;
  bool m_sampleUpToDate;
};
#endif

}

#endif
