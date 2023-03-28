#include "list_parameter_controller.h"

#include <assert.h>

#include "color_names.h"
#include "function_app.h"

using namespace Escher;

namespace Shared {

ListParameterController::ListParameterController(
    Responder *parentResponder, I18n::Message functionColorMessage,
    I18n::Message deleteFunctionMessage,
    SelectableListViewDelegate *listDelegate)
    : ExplicitSelectableListViewController(parentResponder, listDelegate),
      m_deleteCell(deleteFunctionMessage),
      m_colorParameterController(this) {
  m_enableCell.label()->setMessage(
      I18n::Message::ActivateDeactivateListParamTitle);
  m_enableCell.subLabel()->setMessage(
      I18n::Message::ActivateDeactivateListParamDescription);
  // TODO: DefaultInitialization
  m_enableCell.subLabel()->setFont(KDFont::Size::Small);
  m_enableCell.subLabel()->setAlignment(KDContext::k_alignLeft,
                                        KDContext::k_alignCenter);
  m_enableCell.subLabel()->setTextColor(Palette::GrayDark);
}

void ListParameterController::viewWillAppear() {
  ViewController::viewWillAppear();
  if (selectedRow() == -1) {
    selectCell(0);
  } else {
    selectCell(selectedRow());
  }
  resetMemoization();
  m_selectableListView.reloadData();
}

void ListParameterController::willDisplayCellForIndex(HighlightCell *cell,
                                                      int index) {
  if (cell == &m_enableCell && !m_record.isNull()) {
    m_enableCell.accessory()->setState(function()->isActive());
  }
  if (cell == &m_colorCell) {
    m_colorCell.setMessage(I18n::Message::Color);
    m_colorCell.setSubtitle(ColorNames::NameForColor(function()->color()));
  }
}

void ListParameterController::setRecord(Ion::Storage::Record record) {
  m_record = record;
  selectCell(0);
}

bool ListParameterController::handleEvent(Ion::Events::Event event) {
  HighlightCell *cell = selectedCell();
  StackViewController *stack =
      static_cast<StackViewController *>(parentResponder());

  if (cell == &m_enableCell && m_enableCell.enterOnEvent(event)) {
    function()->setActive(!function()->isActive());
    resetMemoization();
    m_selectableListView.reloadData();
    return true;
  }
  if (cell == &m_colorCell && m_colorCell.ShouldEnterOnEvent(event)) {
    m_colorParameterController.setRecord(m_record);
    stack->push(&m_colorParameterController);
    return true;
  }
  if (cell == &m_deleteCell && m_deleteCell.ShouldEnterOnEvent(event)) {
    assert(functionStore()->numberOfModels() > 0);
    m_selectableListView.deselectTable();
    functionStore()->removeModel(m_record);
    StackViewController *stack =
        static_cast<StackViewController *>(parentResponder());
    stack->popUntilDepth(
        Shared::InteractiveCurveViewController::k_graphControllerStackDepth,
        true);
    setRecord(Ion::Storage::Record());
    return true;
  }
  return false;
}

ExpiringPointer<Function> ListParameterController::function() {
  return functionStore()->modelForRecord(m_record);
}

FunctionStore *ListParameterController::functionStore() {
  return FunctionApp::app()->functionStore();
}

}  // namespace Shared
