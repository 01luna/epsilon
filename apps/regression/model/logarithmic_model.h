#ifndef REGRESSION_LOGARITHMIC_MODEL_H
#define REGRESSION_LOGARITHMIC_MODEL_H

#include "model.h"

namespace Regression {

class LogarithmicModel : public Model {
 public:
  using Model::Model;

  I18n::Message formulaMessage() const override {
    return I18n::Message::LogarithmicRegressionFormula;
  }
  I18n::Message name() const override { return I18n::Message::Logarithmic; }
  int numberOfCoefficients() const override { return 2; }

  double evaluate(double* modelCoefficients, double x) const override;
  double levelSet(double* modelCoefficients, double xMin, double xMax, double y,
                  Poincare::Context* context) override;

 private:
  Poincare::Expression privateExpression(
      double* modelCoefficients) const override;
  void privateFit(Store* store, int series, double* modelCoefficients,
                  Poincare::Context* context) override;
  bool dataSuitableForFit(Store* store, int series) const override;
};

}  // namespace Regression

#endif
