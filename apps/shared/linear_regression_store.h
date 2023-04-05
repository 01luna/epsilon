#ifndef SHARED_LINEAR_REGRESSION_STORE_H
#define SHARED_LINEAR_REGRESSION_STORE_H

#include <float.h>
#include <poincare/statistics_dataset.h>

#include "double_pair_store.h"

namespace Shared {

namespace LinearModelHelper {

double Slope(double covariance, double variance);
double YIntercept(double meanOfY, double meanOfX, double slope);

}  // namespace LinearModelHelper

class LinearRegressionStore : public Shared::DoublePairStore {
 public:
  constexpr static const char* const* k_columnNames =
      DoublePairStore::k_regressionColumNames;

  LinearRegressionStore(Shared::GlobalContext* context,
                        DoublePairStorePreferences* preferences);

  // DoublePairStore
  char columnNamePrefixAtIndex(int column) const override {
    assert(column >= 0 && column < DoublePairStore::k_numberOfColumnsPerSeries);
    assert(strlen(k_columnNames[column]) == 1);
    return k_columnNames[column][0];
  }
  double defaultValueForColumn1() const override { return 0.0; }

  // Calculation
  double doubleCastedNumberOfPairsOfSeries(int series) const;
  double squaredOffsettedValueSumOfColumn(
      int series, int i, double offset = 0,
      Parameters parameters = Parameters()) const;
  double squaredValueSumOfColumn(int series, int i,
                                 Parameters parameters = Parameters()) const;
  double leastSquaredSum(int series) const;
  double columnProductSum(int series, Parameters parameters) const;
  double columnProductSum(int series) const {
    return columnProductSum(series, Parameters());
  }
  double meanOfColumn(int series, int i,
                      Parameters parameters = Parameters()) const;
  double varianceOfColumn(int series, int i,
                          Parameters parameters = Parameters()) const;
  double standardDeviationOfColumn(int series, int i,
                                   Parameters parameters = Parameters()) const;
  double sampleStandardDeviationOfColumn(
      int series, int i, Parameters parameters = Parameters()) const;
  double covariance(int series, Parameters parameters) const;
  double covariance(int series) const {
    return covariance(series, Parameters());
  }
  double slope(int series, Parameters parameters = Parameters()) const;
  double yIntercept(int series, Parameters parameters = Parameters()) const;
  double correlationCoefficient(int series) const;  // R

 private:
  Poincare::StatisticsDataset<double> createDatasetFromColumn(
      int series, int i, Parameters parameters = Parameters()) const;
};

}  // namespace Shared

#endif
