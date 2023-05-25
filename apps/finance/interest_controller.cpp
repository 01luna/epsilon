#include "interest_controller.h"

#include <apps/apps_container.h>
#include <apps/i18n.h>
#include <apps/shared/poincare_helpers.h>
#include <escher/input_event_handler_delegate.h>
#include <escher/invocation.h>
#include <escher/stack_view_controller.h>
#include <poincare/print.h>

#include "app.h"

using namespace Finance;
using namespace Escher;

InterestController::InterestController(StackViewController *parent,
                                       InputEventHandlerDelegate *handler,
                                       ResultController *resultController)
    : Shared::FloatParameterController<double>(parent),
      m_dropdown(&m_selectableListView, &m_dropdownDataSource, this),
      m_resultController(resultController) {
  for (size_t i = 0; i < k_numberOfReusableInputs; i++) {
    m_cells[i].setParentResponder(&m_selectableListView);
    m_cells[i].setDelegates(handler, this);
  }
  m_dropdownCell.accessory()->setDropdown(&m_dropdown);
}

const char *InterestController::title() {
  uint8_t unknownParam = App::GetInterestData()->getUnknown();
  const char *label =
      I18n::translate(App::GetInterestData()->labelForParameter(unknownParam));
  int length = Poincare::Print::SafeCustomPrintf(
      m_titleBuffer, k_titleBufferSize,
      I18n::translate(I18n::Message::FinanceSolving), label,
      I18n::translate(
          App::GetInterestData()->sublabelForParameter(unknownParam)));
  if (length >= k_titleBufferSize) {
    // Title did not fit, use a reduced pattern
    Poincare::Print::CustomPrintf(
        m_titleBuffer, k_titleBufferSize,
        I18n::translate(I18n::Message::FinanceSolvingReduced), label);
  }
  return m_titleBuffer;
}

void InterestController::didBecomeFirstResponder() {
  // Init from data
  m_dropdownDataSource.setMessages(
      App::GetInterestData()->dropdownMessageAtIndex(0),
      App::GetInterestData()->dropdownMessageAtIndex(1));
  selectCell(0);
  m_dropdown.selectRow(App::GetInterestData()->m_booleanParam ? 0 : 1);
  m_dropdown.init();
  m_dropdown.reloadAllCells();
  resetMemoization();
  m_selectableListView.reloadData(true);
}

bool InterestController::handleEvent(Ion::Events::Event event) {
  return popFromStackViewControllerOnLeftEvent(event);
}

void InterestController::willDisplayCellAtRow(HighlightCell *cell, int row) {
  int type = typeAtRow(row);
  if (type == k_buttonCellType) {
    return;
  }
  uint8_t param = interestParamaterAtIndex(row);
  if (type == k_dropdownCellType) {
    assert(&m_dropdownCell == cell);
    m_dropdownCell.label()->setMessage(
        App::GetInterestData()->labelForParameter(param));
    m_dropdownCell.subLabel()->setMessage(
        App::GetInterestData()->sublabelForParameter(param));
    return;
  }
  MenuCellWithEditableText<MessageTextView, MessageTextView> *myCell =
      static_cast<MenuCellWithEditableText<MessageTextView, MessageTextView> *>(
          cell);
  myCell->label()->setMessage(App::GetInterestData()->labelForParameter(param));
  myCell->subLabel()->setMessage(
      App::GetInterestData()->sublabelForParameter(param));
  return Shared::FloatParameterController<double>::willDisplayCellAtRow(cell,
                                                                        row);
}

int InterestController::typeAtRow(int row) const {
  if (row < indexOfDropdown()) {
    return k_inputCellType;
  }
  return (row == indexOfDropdown()) ? k_dropdownCellType : k_buttonCellType;
}

KDCoordinate InterestController::nonMemoizedRowHeight(int row) {
  int type = typeAtRow(row);
  if (type == k_inputCellType) {
    MenuCellWithEditableText<MessageTextView, MessageTextView> tempCell;
    return heightForCellAtRowWithWidthInit(&tempCell, row);
  } else if (type == k_dropdownCellType) {
    return heightForCellAtRow(&m_dropdownCell, row);
  }
  assert(type == k_buttonCellType);
  return Shared::FloatParameterController<double>::nonMemoizedRowHeight(row);
}

int InterestController::numberOfRows() const {
  return App::GetInterestData()->numberOfParameters();
}

void InterestController::onDropdownSelected(int selectedRow) {
  App::GetInterestData()->m_booleanParam = (selectedRow == 0);
}

uint8_t InterestController::interestParamaterAtIndex(int index) const {
  uint8_t unknownParam = App::GetInterestData()->getUnknown();
  assert(unknownParam < App::GetInterestData()->numberOfUnknowns());
  if (unknownParam <= index) {
    index += 1;
  }
  assert(index < App::GetInterestData()->numberOfParameters());
  return index;
}

int InterestController::reusableParameterCellCount(int type) {
  if (type == k_inputCellType) {
    return k_numberOfReusableInputs;
  }
  return 1;
}

HighlightCell *InterestController::reusableParameterCell(int i, int type) {
  switch (type) {
    case k_inputCellType:
      assert(i < k_numberOfReusableInputs);
      return m_cells + i;
    default:
      assert(type == k_dropdownCellType && i == 0);
      return &m_dropdownCell;
  }
}

Escher::TextField *InterestController::textFieldOfCellAtIndex(
    Escher::HighlightCell *cell, int index) {
  assert(typeAtRow(index) == k_parameterCellType);
  return static_cast<
             MenuCellWithEditableText<MessageTextView, MessageTextView> *>(cell)
      ->textField();
}

double InterestController::parameterAtIndex(int index) {
  return App::GetInterestData()->getValue(interestParamaterAtIndex(index));
}

bool InterestController::setParameterAtIndex(int parameterIndex, double f) {
  uint8_t param = interestParamaterAtIndex(parameterIndex);
  if (!App::GetInterestData()->checkValue(param, f)) {
    App::app()->displayWarning(I18n::Message::UndefinedValue);
    return false;
  }
  App::GetInterestData()->setValue(param, f);
  return true;
}

int InterestController::indexOfDropdown() const {
  return App::GetInterestData()->numberOfDoubleValues() - 1;
}
