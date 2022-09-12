#include "menu_controller.h"
#include <apps/i18n.h>
#include <escher/container.h>
#include <assert.h>

using namespace Finance;

MenuController::MenuController(Escher::StackViewController * parentResponder, InterestMenuController * interestMenuController, Data * data) :
      Escher::SelectableCellListPage<Escher::MessageTableCellWithChevronAndMessage, k_numberOfFinanceCells, Escher::RegularListViewDataSource>(parentResponder),
      DataController(data),
      m_interestMenuController(interestMenuController) {
  selectRow(0);
  cellAtIndex(k_indexOfSimpleInterest)->setMessage(I18n::Message::SimpleInterest);
  cellAtIndex(k_indexOfSimpleInterest)->setSubtitle(I18n::Message::SimpleInterestDescription);
  cellAtIndex(k_indexOfCompoundInterest)->setMessage(I18n::Message::CompoundInterest);
  cellAtIndex(k_indexOfCompoundInterest)->setSubtitle(I18n::Message::CompoundInterestDescription);
}

void MenuController::didBecomeFirstResponder() {
  m_selectableTableView.reloadData();
}

bool MenuController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE || event == Ion::Events::Right) {
    bool simpleInterestRowSelected = (selectedRow() == k_indexOfSimpleInterest);
    assert(simpleInterestRowSelected || selectedRow() == k_indexOfCompoundInterest);

    // Set the interest data model
    setModel(simpleInterestRowSelected);
    m_interestMenuController->selectRow(0);
    stackOpenPage(m_interestMenuController);
    return true;
  }
  return false;
}
