#ifndef POINCARE_EXAM_MODE_H
#define POINCARE_EXAM_MODE_H

#include <assert.h>
#include <ion/exam_mode.h>

namespace Poincare {

class ExamMode : public Ion::ExamMode::Configuration {
 public:
  using Ruleset = Ion::ExamMode::Ruleset;

  union PressToTestFlags {
    bool operator==(const PressToTestFlags& other) const {
      return value == other.value;
    }

    uint16_t value;
    struct {
      bool forbidEquationSolver : 1;
      bool forbidInequalityGraphing : 1;
      bool forbidImplicitPlots : 1;
      bool forbidStatsDiagnostics : 1;
      bool forbidVectors : 1;
      bool forbidBasedLogarithm : 1;
      bool forbidSum : 1;
      bool forbidExactResults : 1;
      bool forbidElementsApp : 1;
    };
  };

  ExamMode() : Configuration() {}
  explicit ExamMode(Ruleset rules, PressToTestFlags flags = {.value = 0})
      : Configuration(rules, flags.value) {}
  ExamMode(Configuration config) : Configuration(config) {}

  PressToTestFlags flags() const { return {.value = Configuration::flags()}; }

  // Exam mode permissions
  bool forbidSolverApp() const;
  bool forbidElementsApp() const;
  bool forbidCodeApp() const;
  bool forbidLineDetails() const;
  bool forbidInequalityGraphing() const;
  bool forbidImplicitPlots() const;
  bool forbidStatsDiagnostics() const;
  bool forbidVectorProduct() const;
  bool forbidVectorNorm() const;
  bool forbidBasedLogarithm() const;
  bool forbidSum() const;
  bool forbidUnits() const;
  bool forbidAdditionalResults() const;
  bool forbidExactResults() const;
};

static_assert(sizeof(ExamMode) == sizeof(Ion::ExamMode::Configuration),
              "ExamMode size is not compatible with PersistingBytes");

}  // namespace Poincare

#endif
