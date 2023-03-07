#ifndef REGRESSION_APP_H
#define REGRESSION_APP_H

#include <apps/shared/store_app.h>
#include <escher/tab_view_controller.h>

#include "data/store_controller.h"
#include "graph/graph_controller.h"
#include "graph/regression_controller.h"
#include "stats/calculation_controller.h"
#include "store.h"

namespace Regression {

class App : public Shared::StoreApp {
 public:
  class Descriptor : public Escher::App::Descriptor {
   public:
    I18n::Message name() const override;
    I18n::Message upperName() const override;
    const Escher::Image *icon() const override;
  };

  class Snapshot : public Shared::StoreApp::Snapshot {
   public:
    Snapshot();

    App *unpack(Escher::Container *container) override;
    void reset() override;
    const Descriptor *descriptor() const override;
    Shared::InteractiveCurveViewRange *graphRange() { return &m_graphRange; }
    Shared::CurveViewCursor *cursor() { return &m_cursor; }
    int *graphSelectedDotIndex() { return &m_graphSelectedDotIndex; }
    int *selectedCurveIndex() { return &m_selectedCurveIndex; }
    Model::Type *regressionTypes() { return m_regressionTypes; }
    Shared::DoublePairStorePreferences *storePreferences() {
      return &m_storePreferences;
    }

   private:
    Shared::InteractiveCurveViewRange m_graphRange;
    Shared::CurveViewCursor m_cursor;
    int m_graphSelectedDotIndex;
    int m_selectedCurveIndex;
    Model::Type m_regressionTypes[Store::k_numberOfSeries];
    Shared::DoublePairStorePreferences m_storePreferences;
  };

  static App *app() {
    return static_cast<App *>(Escher::Container::activeApp());
  }

  TELEMETRY_ID("Regression");

  Shared::StoreController *storeController() override {
    return &m_storeController;
  }
  RegressionController *regressionController() {
    return &m_regressionController;
  }
  Escher::InputViewController *inputViewController() {
    return &m_inputViewController;
  }
  GraphController *graphController() { return &m_graphController; }
  Snapshot *snapshot() const {
    return static_cast<Snapshot *>(
        Shared::ExpressionFieldDelegateApp::snapshot());
  }
  Shared::InteractiveCurveViewRange *graphRange() const {
    return snapshot()->graphRange();
  }

 private:
  App(Snapshot *snapshot, Poincare::Context *parentContext);

  Store m_store;
  CalculationController m_calculationController;
  Escher::AlternateEmptyViewController
      m_calculationAlternateEmptyViewController;
  Escher::ButtonRowController m_calculationHeader;
  GraphController m_graphController;
  Escher::AlternateEmptyViewController m_graphAlternateEmptyViewController;
  Escher::ButtonRowController m_graphHeader;
  Escher::StackViewController m_graphStackViewController;
  StoreController m_storeController;
  Escher::ButtonRowController m_storeHeader;
  Escher::StackViewController m_storeStackViewController;
  Escher::TabViewController m_tabViewController;
  RegressionController m_regressionController;
  Escher::InputViewController m_inputViewController;
};

}  // namespace Regression

#endif
