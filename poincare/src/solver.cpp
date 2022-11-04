#include <poincare/solver.h>
#include <poincare/subtraction.h>
#include <poincare/solver_algorithms.h>

namespace Poincare {

template<typename T>
Solver<T>::Solver(T xStart, T xEnd, const char * unknown, Context * context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) :
  m_xStart(xStart),
  m_xEnd(xEnd),
  m_yResult(k_NAN),
  m_context(context),
  m_unknown(unknown),
  m_complexFormat(complexFormat),
  m_angleUnit(angleUnit),
  m_lastInterest(Interest::None)
{}

template<typename T>
Solver<T>::Solver(const Solver<T> * other) :
  Solver(other->m_xStart, other->m_xEnd, other->m_unknown, other->m_context, other->m_complexFormat, other->m_angleUnit)
{}

template<typename T>
Coordinate2D<T> Solver<T>::next(FunctionEvaluation f, const void * aux, BracketTest test, HoneResult hone) {
  T x1, x2 = start(), x3 = nextX(x2, end());
  T y1, y2 = f(x2, aux), y3 = f(x3, aux);
  Coordinate2D<T> finalSolution(k_NAN, k_NAN);
  Interest finalInterest = Interest::None;

  while ((start() < x3) == (x3 < end())) {
    x1 = x2;
    x2 = x3;
    x3 = nextX(x2, end());
    y1 = y2;
    y2 = y3;
    y3 = f(x3, aux);

    Interest interest = test(y1, y2, y3);
    if (interest != Interest::None) {
      Coordinate2D<T> solution = hone(f, aux, x1, x3, interest, k_absolutePrecision);
      if (std::isfinite(solution.x1()) && validSolution(solution.x1())) {
        finalSolution = solution;
        finalInterest = interest;
        break;
      }
    }
  }

  registerSolution(finalSolution, finalInterest);
  return result();
}

template<typename T>
Coordinate2D<T> Solver<T>::next(Expression e, BracketTest test, HoneResult hone) {
  assert(m_unknown && m_unknown[0] != '\0');
  FunctionEvaluationParameters parameters = { .context = m_context, .unknown = m_unknown, .expression = e, .complexFormat = m_complexFormat, .angleUnit = m_angleUnit };
  FunctionEvaluation f = [](T x, const void * aux) {
    const FunctionEvaluationParameters * p = reinterpret_cast<const FunctionEvaluationParameters *>(aux);
    return p->expression.approximateWithValueForSymbol(p->unknown, x, p->context, p->complexFormat, p->angleUnit);
  };

  return next(f, &parameters, test, hone);
}

template<typename T>
Coordinate2D<T> Solver<T>::nextRoot(Expression e) {
  ExpressionNode::Type type = e.type();

  switch (type) {
  case ExpressionNode::Type::Multiplication:
    /* x*y = 0 => x = 0 or y = 0
     * Check the result in case another term is undefined,
     * e.g. (x+1)*ln(x) for x =- 1*/
    registerSolution(nextRootInMultiplication(e), Interest::Root);
    return result();

  case ExpressionNode::Type::Power:
  case ExpressionNode::Type::NthRoot:
  case ExpressionNode::Type::Division:
    /* f(x,y) = 0 => x = 0
     * Check the result as some values of y may invalidate it (e.g. x^(1/x)
     * or (x-1)/ln(x)). */
    registerSolution(nextPossibleRootInChild(e, 0), Interest::Root);
    return result();

  case ExpressionNode::Type::AbsoluteValue:
  case ExpressionNode::Type::HyperbolicSine:
  case ExpressionNode::Type::Opposite:
  case ExpressionNode::Type::SquareRoot:
    /* f(x) = 0 <=> x = 0 */
    return nextRoot(e.childAtIndex(0));

  default:
    Coordinate2D<T> res = next(e, EvenOrOddRootInBracket, CompositeBrentForRoot);
    if (lastInterest() != Interest::None) {
      m_lastInterest = Interest::Root;
    }
    return res;
  }
}

template<typename T>
Coordinate2D<T> Solver<T>::nextMinimum(Expression e) {
  return next(e, MinimumInBracket, SolverAlgorithms::BrentMinimum);
}

template<typename T>
Coordinate2D<T> Solver<T>::nextIntersection(Expression e1, Expression e2) {
  Expression subtraction = Subtraction::Builder(e1, e2);
  ExpressionNode::ReductionContext reductionContext(m_context, m_complexFormat, m_angleUnit, Preferences::UnitFormat::Metric, ExpressionNode::ReductionTarget::SystemForAnalysis);
  subtraction = subtraction.cloneAndSimplify(reductionContext);
  nextRoot(subtraction);
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
typename Solver<T>::Interest Solver<T>::EvenOrOddRootInBracket(T a, T b, T c) {
  Interest root = OddRootInBracket(a, b, c);
  if (root != Interest::None) {
    return root;
  }
  /* FIXME Check the sign of a,b and c. A minimum can only be a root if b is
   * positive. */
  Interest extremum = MinimumInBracket(a, b, c);
  return extremum == Interest::None ? MaximumInBracket(a, b, c) : extremum;
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
T Solver<T>::maximalStep() const {
  constexpr T minimalNumberOfSteps = static_cast<T>(100.);
  return std::max(k_minimalPracticalStep, std::fabs(m_xEnd - m_xStart) / minimalNumberOfSteps);
}

template<typename T>
T Solver<T>::minimalStep(T x) const {
  return std::max(k_minimalPracticalStep, std::fabs(x) * k_relativePrecision);
}

template<typename T>
bool Solver<T>::validSolution(T x) const {
  T minStep = minimalStep(m_xStart);
  return m_xStart < m_xEnd ? m_xStart + minStep < x && x < m_xEnd : m_xEnd < x && x < m_xStart - minStep;
}

template<typename T>
T Solver<T>::nextX(T x, T direction) const {
  /* Compute the next step for the bracketing algorithm. The formula is derived
   * from the following criteria:
   * - using a fixed step would either lead to poor precision close to zero or
   *   prohibitive computation times on large intervals.
   * - we assume that for a typical function, distance between to points of
   *   interest is of the same magnitude as their abscissa ; as such, we want
   *   to sample with the same density on ]-10,10[ and ]-1000,1000[.
   * - we further assume that for a typical *high-school* function, points of
   *   interest are more likely to be close to the unit, rather than be
   *   around 1e6 or 1e-4.
   *
   * As such we use a formula of the form t+dt = t * (φ(log|t|) + α)
   * The geometric growth ensures we do not over-sample on large intervals,
   * and the term φ allows increasing the ratio in "less interesting" areas.
   *
   * As for the limits, we ensure that:
   * - even when |t| is large, dt never skips an order of magnitude
   *   i.e. 0.1 < |(t+dt)/t| < 10
   * - there is a minimal value for dt, to allow crossing zero.
   * - always sample a minimal number of points in the whole interval. */
  constexpr T baseGrowthSpeed = static_cast<T>(1.1);
  static_assert(baseGrowthSpeed > static_cast<T>(1.), "Growth speed must be greater than 1");
  constexpr T maximalGrowthSpeed = static_cast<T>(10.);
  constexpr T growthSpeedAcceleration = static_cast<T>(1e-2);
  /* Increase density between 0.1 and 100 */
  constexpr T lowerTypicalMagnitude = static_cast<T>(-1.);
  constexpr T upperTypicalMagnitude = static_cast<T>(2.);

  T maxStep = maximalStep();
  T minStep = minimalStep(x);
  T stepSign = x < direction ? static_cast<T>(1.) : static_cast<T>(-1.);

  T magnitude = std::log(std::fabs(x));
  if (!std::isfinite(magnitude)) {
    return x + stepSign * minStep;
  }
  T ratio;
  if (lowerTypicalMagnitude < magnitude && magnitude < upperTypicalMagnitude) {
    ratio = baseGrowthSpeed;
  } else {
    T exponent = magnitude < lowerTypicalMagnitude ? magnitude - lowerTypicalMagnitude : upperTypicalMagnitude - magnitude;
    ratio = maximalGrowthSpeed - (maximalGrowthSpeed - baseGrowthSpeed) * std::exp(growthSpeedAcceleration * std::pow(exponent, 3.));
  }
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
Coordinate2D<T> Solver<T>::nextPossibleRootInChild(Expression e, int childIndex) const {
  Solver<T> solver(this);
  Expression child = e.childAtIndex(childIndex);
  T xRoot = solver.nextRoot(child).x1();
  while (std::isfinite(xRoot)) {
    if (std::fabs(e.approximateWithValueForSymbol<T>(m_unknown, xRoot, m_context, m_complexFormat, m_angleUnit)) < NullTolerance(xRoot)) {
      return Coordinate2D<T>(xRoot, k_zero);
    }
    xRoot = solver.nextRoot(child).x1();
  }
  return Coordinate2D<T>(k_NAN, k_NAN);
}

template<typename T>
Coordinate2D<T> Solver<T>::nextRootInMultiplication(Expression e) const {
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
void Solver<T>::registerSolution(Coordinate2D<T> solution, Interest interest) {
  T x = solution.x1();
  if (std::fabs(x) < NullTolerance(x)) {
    x = k_zero;
  }
  if (std::isnan(x)) {
    m_lastInterest = Interest::None;
  } else {
    assert(validSolution(x));
    m_xStart = x;
    m_yResult = solution.x2();
    if (std::fabs(m_yResult) < NullTolerance(x)) {
      m_yResult = k_zero;
    }
    m_lastInterest = interest;
    assert(m_yResult == 0. || m_lastInterest != Interest::Root);
  }
}

// Explicit template instanciations

template Solver<double>::Solver(double, double, const char *, Context *, Preferences::ComplexFormat, Preferences::AngleUnit);
template Coordinate2D<double> Solver<double>::nextRoot(Expression);
template Coordinate2D<double> Solver<double>::nextMinimum(Expression);
template Coordinate2D<double> Solver<double>::nextIntersection(Expression, Expression);
template void Solver<double>::stretch();

template Solver<float>::Solver(float, float, const char *, Context *, Preferences::ComplexFormat, Preferences::AngleUnit);
template Coordinate2D<float> Solver<float>::next(FunctionEvaluation, const void *, BracketTest, HoneResult);

}
