#include <poincare/derivative.h>
#include <poincare/symbol.h>
#include <poincare/float.h>

extern "C" {
#include <assert.h>
#include <math.h>
#include <float.h>
}

Derivative::Derivative() :
  Function("diff")
{
}

Expression::Type Derivative::type() const {
  return Type::Derivative;
}

Expression * Derivative::cloneWithDifferentOperands(Expression** newOperands,
        int numberOfOperands, bool cloneOperands) const {
  assert(numberOfOperands == 2);
  assert(newOperands != nullptr);
  Derivative * d = new Derivative();
  d->setArgument(newOperands, numberOfOperands, cloneOperands);
  return d;
}

float Derivative::approximate(Context& context) const {
  VariableContext xContext = VariableContext('x', &context);
  Symbol xSymbol = Symbol('x');
  float x = m_args[1]->approximate(context);
  Float e = Float(x);
  xContext.setExpressionForSymbolName(&e, &xSymbol);
  float functionValue = m_args[0]->approximate(xContext);

  /* Ridders' Algorithm
   * Blibliography:
   * - Press, W. H., Teukolsky, S. A., Vetterling, W. T., & Flannery, B. P.
   * (1992). Numerical recipies in C.
   * - Ridders, C.J.F. 1982, Advances in Engineering Software, vol. 4, no. 2,
   * pp. 75–76. */

  // Initiate hh
  float h = fabsf(x) < FLT_MIN ? k_minInitialRate : x/1000.0f;
  float f2 = approximateDerivate2(x, h, xContext);
  f2 = fabsf(f2) < FLT_MIN ? k_minInitialRate : f2;
  float hh = sqrtf(fabsf(functionValue/(f2/(powf(h,2.0f)))))/10.0f;
  hh = fabsf(hh) <FLT_MIN ? k_minInitialRate : hh;
  /* Make hh an exactly representable number */
  volatile float temp =  x+hh;
  hh = temp - x;
  /* a is matrix storing the function extrapolations for different stepsizes at
  * different order */
  float a[10][10];
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      a[i][j] = 1.0f;
    }
  }
  a[0][0] = growthRateAroundAbscissa(x, hh, xContext);
  float err = FLT_MAX;
  float ans = 0.0f;
  float errt = 0.0f;
  /* Loop on i: change the step size */
  for (int i = 1; i < 10; i++) {
    hh /= k_rateStepSize;
    /* Make hh an exactly representable number */
    volatile float temp =  x+hh;
    hh = temp - x;
    a[0][i] = growthRateAroundAbscissa(x, hh, xContext);
    float fac = k_rateStepSize*k_rateStepSize;
    /* Loop on j: compute extrapolation for several orders */
    for (int j = 1; j < 10; j++) {
      a[j][i] = (a[j-1][i]*fac-a[j-1][i-1])/(fac-1.0f);
      fac = k_rateStepSize*k_rateStepSize*fac;
      errt = fabsf(a[j][i]-a[j-1][i]) > fabsf(a[j][i]-a[j-1][i-1]) ? fabsf(a[j][i]-a[j-1][i]) : fabsf(a[j][i]-a[j-1][i-1]);
      /* Update error and answer if error decreases */
      if (errt < err) {
        err = errt;
        ans = a[j][i];
      }
    }
    /* If higher extrapolation order significantly increases the error, return
     * early */
    if (fabsf(a[i][i]-a[i-1][i-1]) > 2.0f*err) {
      break;
    }
  }
  err = powf(10.0f, (int)log10f(fabsf(err))+2.0f);
  /* if the error is too big regarding the value, do not return the answer */
  if (err/ans > k_maxErrorRateOnApproximation || isnan(err)) {
    return NAN;
  }
  if (err < FLT_MIN) {
    return ans;
  }
  return roundf(ans/err)*err;
}

float Derivative::growthRateAroundAbscissa(float x, float h, VariableContext xContext) const {
  Symbol xSymbol = Symbol('x');
  Float e = Float(x + h);
  xContext.setExpressionForSymbolName(&e, &xSymbol);
  float expressionPlus = m_args[0]->approximate(xContext);
  e = Float(x-h);
  xContext.setExpressionForSymbolName(&e, &xSymbol);
  float expressionMinus = m_args[0]->approximate(xContext);
  return (expressionPlus - expressionMinus)/(2*h);
}

float Derivative::approximateDerivate2(float x, float h, VariableContext xContext) const {
  Symbol xSymbol = Symbol('x');
  Float e = Float(x + h);
  xContext.setExpressionForSymbolName(&e, &xSymbol);
  float expressionPlus = m_args[0]->approximate(xContext);
  e = Float(x);
  xContext.setExpressionForSymbolName(&e, &xSymbol);
  float expression = m_args[0]->approximate(xContext);
  e = Float(x-h);
  xContext.setExpressionForSymbolName(&e, &xSymbol);
  float expressionMinus = m_args[0]->approximate(xContext);
  return expressionPlus - 2.0f*expression + expressionMinus;
}
