#ifndef STATISTICS_STORE_PARAMETER_CONTROLLER_H
#define STATISTICS_STORE_PARAMETER_CONTROLLER_H

#include <apps/shared/store_parameter_controller.h>
#include <escher/menu_cell.h>
#include <escher/switch_view.h>

#include "../store.h"

namespace Statistics {

class StoreController;

class StoreParameterController : public Shared::StoreParameterController {
 public:
  StoreParameterController(Escher::Responder* parentResponder,
                           StoreController* storeController, Store* m_store);
  void initializeColumnParameters() override;
  bool handleEvent(Ion::Events::Event event) override;
  int numberOfRows() const override;
  int typeAtIndex(int index) const override;
  Escher::AbstractMenuCell* reusableCell(int index, int type) override;
  void willDisplayCellForIndex(Escher::HighlightCell* cell, int index) override;

 private:
  /* When displayed, HideCumulatedFrequencyCell is last and second.
   * Remaining Shared::StoreParameterController are not displayed:
   * m_fillFormula, m_hideCell and m_clearColumn */
  constexpr static int k_indexOfHideCumulatedFrequencyCell =
      Shared::StoreParameterController::k_fillFormulaCellType;
  constexpr static int k_displayCumulatedFrequencyCellType =
      Shared::StoreParameterController::k_numberOfCells;
  constexpr static int k_hideCumulatedFrequencyCellType =
      k_displayCumulatedFrequencyCellType + 1;

  I18n::Message sortMessage() override {
    return (m_columnIndex % 2 == 0) ? I18n::Message::SortValues
                                    : I18n::Message::SortSizes;
  }

  Escher::MenuCell<Escher::MessageTextView, Escher::MessageTextView,
                   Escher::SwitchView>
      m_displayCumulatedFrequencyCell;
  Escher::MenuCell<Escher::MessageTextView, Escher::MessageTextView>
      m_hideCumulatedFrequencyCell;
  Store* m_store;
  bool m_isCumulatedFrequencyColumnSelected;
};

}  // namespace Statistics

#endif
