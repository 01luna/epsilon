#include <poincare/solver.h>
#include <poincare/subtraction.h>
#include <poincare/solver_algorithms.h>

namespace Poincare {

template<typename T>
Solver<T>::Solver(T xStart, T xEnd, const char * unknown, Context * context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) :
  m_xStart(xStart),
  m_xEnd(xEnd),
  m_maximalXStep(MaximalStep(xEnd - xStart)),
  m_yResult(k_NAN),
  m_context(context),
  m_unknown(unknown),
  m_complexFormat(complexFormat),
  m_angleUnit(angleUnit),
  m_lastInterest(Interest::None)
{}

template<typename T>
Coordinate2D<T> Solver<T>::next(FunctionEvaluation f, const void * aux, BracketTest test, HoneResult hone) {
  Coordinate2D<T> p1, p2(start(), f(start(), aux)), p3(nextX(p2.x1(), end()), k_NAN);
  p3.setX2(f(p3.x1(), aux));
  Coordinate2D<T> finalSolution;
  Interest finalInterest = Interest::None;

  /* If the solver is in float, we want it to be fast so the fine search
   * of interest around discontinuities is skipped. */
  static bool searchMorePreciselyIfDiscontinuities = sizeof(T) == sizeof(double);

  while ((start() < p3.x1()) == (p3.x1() < end())) {
    p1 = p2;
    p2 = p3;
    p3.setX1(nextX(p2.x1(), end()));
    p3.setX2(f(p3.x1(), aux));

    Coordinate2D<T> start = p1;
    Coordinate2D<T> middle = p2;
    Coordinate2D<T> end = p3;
    Interest interest = Interest::None;
    if ((interest = test(start, middle, end, aux)) == Interest::None && // assignment in condition
        searchMorePreciselyIfDiscontinuities &&
        DiscontinuityInBracket(start, middle, end, aux) == Interest::Discontinuity) {
      /* If no interest was found and there is a discontinuity in the interval,
       * search for the largest interval that without discontinuity and
       * then recompute the interest in this interval. */
      ExcludeDiscontinuityFromBracket(&start, &middle, &end, f, aux, minimalStep(middle.x1()));
      interest = test(start, middle, end, aux);
    }

    if (interest != Interest::None) {
      Coordinate2D<T> solution = hone(f, aux, start.x1(), end.x1(), interest, k_absolutePrecision);
      if (std::isfinite(solution.x1()) && validSolution(solution.x1())) {
        finalSolution = solution;
        finalInterest = interest;
        break;
      }
    }
  }

  registerSolution(finalSolution, finalInterest, f, aux);
  return result();
}

template<typename T>
Coordinate2D<T> Solver<T>::next(const Expression & e, BracketTest test, HoneResult hone) {
  assert(m_unknown && m_unknown[0] != '\0');
  FunctionEvaluationParameters parameters = { .context = m_context, .unknown = m_unknown, .expression = e, .complexFormat = m_complexFormat, .angleUnit = m_angleUnit };
  FunctionEvaluation f = [](T x, const void * aux) {
    const FunctionEvaluationParameters * p = reinterpret_cast<const FunctionEvaluationParameters *>(aux);
    return p->expression.approximateWithValueForSymbol(p->unknown, x, p->context, p->complexFormat, p->angleUnit);
  };

  return next(f, &parameters, test, hone);
}

template<typename T>
Coordinate2D<T> Solver<T>::nextRoot(const Expression & e) {
  ExpressionNode::Type type = e.type();

  switch (type) {
  case ExpressionNode::Type::Multiplication:
    /* x*y = 0 => x = 0 or y = 0 */
    registerSolution(nextRootInMultiplication(e), Interest::Root);
    return result();

  case ExpressionNode::Type::Power:
  case ExpressionNode::Type::NthRoot:
  case ExpressionNode::Type::Division:
    /* f(x,y) = 0 => x = 0 */
    registerSolution(nextPossibleRootInChild(e, 0), Interest::Root);
    return result();

  case ExpressionNode::Type::AbsoluteValue:
  case ExpressionNode::Type::HyperbolicSine:
  case ExpressionNode::Type::Opposite:
  case ExpressionNode::Type::SquareRoot:
    /* f(x) = 0 <=> x = 0 */
    return nextRoot(e.childAtIndex(0));

  default:
    if (e.isNull(m_context) == TrinaryBoolean::False) {
      registerSolution(Coordinate2D<T>(), Interest::None);
      return Coordinate2D<T>();
    }

    Coordinate2D<T> res = next(e, EvenOrOddRootInBracket, CompositeBrentForRoot);
    if (lastInterest() != Interest::None) {
      m_lastInterest = Interest::Root;
    }
    return res;
  }
}

template<typename T>
Coordinate2D<T> Solver<T>::nextMinimum(const Expression & e) {
  /* TODO We could add a layer of formal resolution:
   * - use the derivative (could be an optional argument to avoid recomputing
   *   it every time)
   * - since d(f°g) = dg×df°g, if f is known to be monotonous (i.e. df≠0), look
   *   for the extrema of g. */
  return next(e, MinimumInBracket, SolverAlgorithms::BrentMinimum);
}

template<typename T>
Coordinate2D<T> Solver<T>::nextIntersection(const Expression & e1, const Expression & e2, Expression * memoizedDifference) {
  if (!memoizedDifference) {
    Expression diff;
    return nextIntersection(e1, e2, &diff);
  }
  assert(memoizedDifference);
  if (memoizedDifference->isUninitialized()) {
    ExpressionNode::ReductionContext reductionContext(m_context, m_complexFormat, m_angleUnit, Preferences::UnitFormat::Metric, ExpressionNode::ReductionTarget::SystemForAnalysis);
    *memoizedDifference = Subtraction::Builder(e1.clone(), e2.clone()).cloneAndSimplify(reductionContext);
  }
  nextRoot(*memoizedDifference);
  if (m_lastInterest == Interest::Root) {
    m_lastInterest = Interest::Intersection;
    m_yResult = e1.approximateWithValueForSymbol<T>(m_unknown, m_xStart, m_context, m_complexFormat, m_angleUnit);
  }
  return result();
}

template<typename T>
void Solver<T>::stretch() {
  T step = maximalStep();
  T stepSign = m_xStart < m_xEnd ? static_cast<T>(1.) : static_cast<T>(-1.);
  m_xStart -= step * stepSign;
  m_xEnd += step * stepSign;
}

template<typename T>
typename Solver<T>::Interest Solver<T>::EvenOrOddRootInBracket(Coordinate2D<T> a, Coordinate2D<T> b, Coordinate2D<T> c, const void * aux) {
  Interest root = OddRootInBracket(a, b, c, aux);
  if (root != Interest::None) {
    return root;
  }
  /* FIXME Check the sign of a,b and c. A minimum can only be a root if b is
   * positive. */
  Interest extremum = MinimumInBracket(a, b, c, aux);
  return extremum == Interest::None ? MaximumInBracket(a, b, c, aux) : extremum;
}

template<typename T>
Coordinate2D<T> Solver<T>::BrentMaximum(FunctionEvaluation f, const void * aux, T xMin, T xMax, Interest interest, T precision) {
  const void * pack[] = {&f, aux};
  FunctionEvaluation minusF = [](T x, const void * aux) {
    const void * const * param = reinterpret_cast<const void * const *>(aux);
    const FunctionEvaluation * f = reinterpret_cast<const FunctionEvaluation *>(param[0]);
    return -(*f)(x, param[1]);
  };
  Coordinate2D<T> res = SolverAlgorithms::BrentMinimum(minusF, pack, xMin, xMax, interest, precision);
  return Coordinate2D<T>(res.x1(), -res.x2());
}

template<typename T>
Coordinate2D<T> Solver<T>::CompositeBrentForRoot(FunctionEvaluation f, const void * aux, T xMin, T xMax, Interest interest, T precision) {
  if (interest == Interest::Root) {
    return SolverAlgorithms::BrentRoot(f, aux, xMin, xMax, interest, precision);
  }
  Coordinate2D<T> res;
  if (interest == Interest::LocalMinimum) {
    res = SolverAlgorithms::BrentMinimum(f, aux, xMin, xMax, interest, precision);
  } else {
    assert(interest == Interest::LocalMaximum);
    res = BrentMaximum(f, aux, xMin, xMax, interest, precision);
  }
  if (std::isfinite(res.x1()) && std::fabs(res.x2()) < NullTolerance(res.x1())) {
    return res;
  }
  return Coordinate2D<T>(k_NAN, k_NAN);
}

template<typename T>
void Solver<T>::ExcludeDiscontinuityFromBracket(Coordinate2D<T> * p1, Coordinate2D<T> * p2, Coordinate2D<T> * p3, FunctionEvaluation f, const void * aux, T minimalSizeOfInterval) {
  assert(DiscontinuityInBracket(*p1, *p2, *p3, aux) == Interest::Discontinuity);
  /* Search for the smallest interval that contains the discontinuity and
   * return the largest interval that does not intersect with it. */
  Coordinate2D<T> dummy(k_NAN, k_NAN);
  Coordinate2D<T> lowerBoundOfDiscontinuity = *p1;
  Coordinate2D<T> middleOfDiscontinuity = *p2;
  Coordinate2D<T> upperBoundOfDiscontinuity = *p3;
  while (upperBoundOfDiscontinuity.x1() - lowerBoundOfDiscontinuity.x1() >= minimalSizeOfInterval)
  {
    if (DiscontinuityInBracket(lowerBoundOfDiscontinuity, dummy, middleOfDiscontinuity, aux) == Interest::Discontinuity) {
      upperBoundOfDiscontinuity = middleOfDiscontinuity;
      middleOfDiscontinuity.setX1((lowerBoundOfDiscontinuity.x1() + middleOfDiscontinuity.x1()) / 2.0);
      middleOfDiscontinuity.setX2(f(middleOfDiscontinuity.x1(), aux));
    } else {
      assert(DiscontinuityInBracket(middleOfDiscontinuity, dummy, upperBoundOfDiscontinuity, aux) == Interest::Discontinuity);
      lowerBoundOfDiscontinuity = middleOfDiscontinuity;
      middleOfDiscontinuity.setX1((middleOfDiscontinuity.x1() + upperBoundOfDiscontinuity.x1()) / 2.0);
      middleOfDiscontinuity.setX2(f(middleOfDiscontinuity.x1(), aux));
    }
    // assert that dummy has no impact
    assert(DiscontinuityInBracket(lowerBoundOfDiscontinuity, middleOfDiscontinuity, upperBoundOfDiscontinuity, aux) == Interest::Discontinuity);
  }
  /* The smallest interval containing the discontinuity is found. Now
   * set p1, p2 and p3 outside of it. */
  if (std::isnan(lowerBoundOfDiscontinuity.x2())) {
    *p1 = upperBoundOfDiscontinuity;
  } else {
    assert(std::isnan(upperBoundOfDiscontinuity.x2()));
    *p3 = lowerBoundOfDiscontinuity;
  }
  p2->setX1((p1->x1() + p3->x1()) / 2.0);
  p2->setX2(f(p2->x1(), aux));
}

template<typename T>
T Solver<T>::MaximalStep(T intervalAmplitude) {
  constexpr T minimalNumberOfSteps = static_cast<T>(100.);
  return std::max(k_minimalPracticalStep, std::fabs(intervalAmplitude) / minimalNumberOfSteps);
}

template<typename T>
T Solver<T>::minimalStep(T x) const {
  return std::max(k_minimalPracticalStep, std::fabs(x) * k_relativePrecision);
}

template<typename T>
bool Solver<T>::validSolution(T x) const {
  T minStep = minimalStep(m_xStart);
  /* NAN is implicitly handled by the comparisons. */
  return m_xStart < m_xEnd ? m_xStart + minStep < x && x < m_xEnd : m_xEnd < x && x < m_xStart - minStep;
}

template<typename T>
T Solver<T>::nextX(T x, T direction) const {
  /* Compute the next step for the bracketing algorithm. The formula is derived
   * from the following criteria:
   * - using a fixed step would either lead to poor precision close to zero or
   *   prohibitive computation times on large intervals.
   * - we assume that for a typical function, distance between two points of
   *   interest is of the same magnitude as their abscissa ; as such, we want
   *   to sample with the same density on ]-10,10[ and ]-1000,1000[.
   * - we further assume that for a typical *high-school* function, points of
   *   interest are more likely to be close to the unit, rather than be
   *   around 1e6 or 1e-4.
   *
   * As such we use a formula of the form t+dt = t * φ(log|t|)
   * The geometric growth ensures we do not over-sample on large intervals,
   * and the term φ allows increasing the ratio in "less interesting" areas.
   *
   * As for the limits, we ensure that:
   * - even when |t| is large, dt never skips an order of magnitude
   *   i.e. 0.1 < |(t+dt)/t| < 10
   * - there is a minimal value for dt, to allow crossing zero.
   * - always sample a minimal number of points in the whole interval. */
  constexpr T baseGrowthSpeed = static_cast<T>(1.05);
  static_assert(baseGrowthSpeed > static_cast<T>(1.), "Growth speed must be greater than 1");
  constexpr T maximalGrowthSpeed = static_cast<T>(10.);
  constexpr T growthSpeedAcceleration = static_cast<T>(1e-2);
  /* Increase density between 0.1 and 100 */
  constexpr T lowerTypicalMagnitude = static_cast<T>(-1.);
  constexpr T upperTypicalMagnitude = static_cast<T>(3.);

  T maxStep = maximalStep();
  T minStep = minimalStep(x);
  T stepSign = x < direction ? static_cast<T>(1.) : static_cast<T>(-1.);

  T magnitude = std::log10(std::fabs(x));
  if (!std::isfinite(magnitude) || minStep >= maxStep) {
    /* minStep can be greater than maxStep if we are operating on a very small
     * intervals of very large numbers */
    return x + stepSign * minStep;
  }
  /* We define a piecewise function φ such that:
   * - φ is constant on [-1,2] and return baseGrowthSpeed, making t progress
   *   geometrically from 0.1 to 100.
   * - φ varies continuously from baseGrowthSpeed, to maximalGrowthSpeed at ∞.
   * The particular shape used there (an exponential of the negative cubed
   * magnitude) provides a smooth transition up until log|t|~±8. */
  T ratio;
  if (lowerTypicalMagnitude <= magnitude && magnitude <= upperTypicalMagnitude) {
    ratio = baseGrowthSpeed;
  } else {
    T magnitudeDelta = magnitude < lowerTypicalMagnitude ? lowerTypicalMagnitude  - magnitude : magnitude - upperTypicalMagnitude;
    assert(magnitudeDelta > 0);
    ratio = maximalGrowthSpeed - (maximalGrowthSpeed - baseGrowthSpeed) * std::exp(growthSpeedAcceleration *  - std::pow(magnitudeDelta, static_cast<T>(3.)));
  }
  /* If the next step is toward zero, divide the postion, overwise multiply. */
  assert(ratio > static_cast<T>(1.));
  T x2 = (x < direction) == (x < k_zero) ? x / ratio : x * ratio;
  if (std::fabs(x - x2) > maxStep) {
    x2 = x + stepSign * maxStep;
  }
  if (std::fabs(x - x2) < minStep) {
    x2 = x + stepSign * minStep;
  }
  assert((x < x2) == (x < direction));
  assert(x2 != x);
  return x2;
}

template<typename T>
Coordinate2D<T> Solver<T>::nextPossibleRootInChild(const Expression & e, int childIndex) const {
  Solver<T> solver = *this;
  Expression child = e.childAtIndex(childIndex);
  T xRoot;
  while (std::isfinite(xRoot = solver.nextRoot(child).x1())) { // assignment in condition
    /* Check the result in case another term is undefined,
     * e.g. (x+1)*ln(x) for x =- 1.
     * This comparison relies on the fact that it is false for a NAN
     * approximation. */
    if (std::fabs(e.approximateWithValueForSymbol<T>(m_unknown, xRoot, m_context, m_complexFormat, m_angleUnit)) < NullTolerance(xRoot)) {
      return Coordinate2D<T>(xRoot, k_zero);
    }
  }
  return Coordinate2D<T>(k_NAN, k_NAN);
}

template<typename T>
Coordinate2D<T> Solver<T>::nextRootInMultiplication(const Expression & e) const {
  assert(e.type() == ExpressionNode::Type::Multiplication);
  T xRoot = k_NAN;
  int n = e.numberOfChildren();
  for (int i = 0; i < n; i++) {
    T xRootChild = nextPossibleRootInChild(e, i).x1();
    if (std::isfinite(xRootChild) && (!std::isfinite(xRoot) || std::fabs(m_xStart - xRootChild) < std::fabs(m_xStart - xRoot))) {
      xRoot = xRootChild;
    }
  }
  return Coordinate2D<T>(xRoot, k_zero);
}

template<typename T>
void Solver<T>::registerSolution(Coordinate2D<T> solution, Interest interest, FunctionEvaluation f, const void * aux) {
  T x = solution.x1();

  if (f) {
    /* When searching for an extremum, the function can take the extremal value
     * on several abscissas, and Brent can pick up any of them. This deviation
     * is particularly visible if the theoretical solution is an integer. */
    T roundX = std::copysign(k_minimalPracticalStep * std::floor(std::fabs(x) / k_minimalPracticalStep), x);
    if (validSolution(roundX)) {
      T fIntX = f(roundX, aux);
      bool roundXIsBetter = fIntX == solution.x2() || (interest == Interest::Root && fIntX == k_zero) || (interest == Interest::LocalMinimum && fIntX < solution.x2()) || (interest == Interest::LocalMaximum && fIntX > solution.x2());
      if (roundXIsBetter) {
        x = roundX;
      }
    }
  }

  if (std::isnan(x)) {
    m_lastInterest = Interest::None;
    m_xStart = k_NAN;
    m_yResult = k_NAN;
  } else {
    assert(validSolution(x));
    m_xStart = x;
    m_yResult = solution.x2();
    if (std::fabs(m_yResult) < NullTolerance(x)) {
      m_yResult = k_zero;
    }
    m_lastInterest = interest;
  }
}

// Explicit template instanciations

template Solver<double>::Solver(double, double, const char *, Context *, Preferences::ComplexFormat, Preferences::AngleUnit);
template Coordinate2D<double> Solver<double>::next(FunctionEvaluation, const void *, BracketTest, HoneResult);
template Coordinate2D<double> Solver<double>::nextRoot(const Expression &);
template Coordinate2D<double> Solver<double>::nextMinimum(const Expression &);
template Coordinate2D<double> Solver<double>::nextIntersection(const Expression &, const Expression &, Expression *);
template void Solver<double>::stretch();
template Coordinate2D<double> Solver<double>::BrentMaximum(FunctionEvaluation, const void *, double, double, Interest, double);
template double Solver<double>::MaximalStep(double);
template void Solver<double>::ExcludeDiscontinuityFromBracket(Coordinate2D<double> * p1, Coordinate2D<double> * p2, Coordinate2D<double> * p3, FunctionEvaluation f, const void * aux, double minimalSizeOfInterval);

template Solver<float>::Interest Solver<float>::EvenOrOddRootInBracket(Coordinate2D<float>, Coordinate2D<float>, Coordinate2D<float>, const void *);
template Solver<float>::Solver(float, float, const char *, Context *, Preferences::ComplexFormat, Preferences::AngleUnit);
template Coordinate2D<float> Solver<float>::next(FunctionEvaluation, const void *, BracketTest, HoneResult);
template float Solver<float>::MaximalStep(float);

}
