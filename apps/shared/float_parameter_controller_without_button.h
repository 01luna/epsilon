#ifndef SHARED_FLOAT_PARAMETER_CONTROLLER_WITHOUT_BUTTON_H
#define SHARED_FLOAT_PARAMETER_CONTROLLER_WITHOUT_BUTTON_H

#include "parameter_text_field_delegate.h"
#include <escher/selectable_list_view_controller.h>
#include <escher/stack_view_controller.h>
#include <escher/message_table_cell_with_editable_text.h>

namespace Shared {

/* This controller edits float parameter of any model (given through
 * parameterAtIndex and setParameterAtIndex). */

template<typename T, typename M=Escher::MessageTableCellWithEditableText>
class FloatParameterControllerWithoutButton : public Escher::SelectableListViewController<Escher::MemoizedListViewDataSource>, public ParameterTextFieldDelegate {
public:
  FloatParameterControllerWithoutButton(Escher::Responder * parentResponder);
  void didBecomeFirstResponder() override;
  void viewWillAppear() override;
  void viewDidDisappear() override;
  bool handleEvent(Ion::Events::Event event) override;

  int typeAtIndex(int index) override;
  void willDisplayCellForIndex(Escher::HighlightCell * cell, int index) override;
  KDCoordinate nonMemoizedRowHeight(int j) override;
  bool textFieldShouldFinishEditing(Escher::TextField * textField, Ion::Events::Event event) override;
  bool textFieldDidFinishEditing(Escher::TextField * textField, const char * text, Ion::Events::Event event) override;

protected:
  static constexpr int k_parameterCellType = 0;

  enum class InfinityTolerance {
    None,
    PlusInfinity,
    MinusInfinity
  };
  int activeCell() { return selectedRow(); }
  Escher::StackViewController * stackController() { return static_cast<Escher::StackViewController *>(parentResponder()); }
  virtual T parameterAtIndex(int index) = 0;
  virtual bool isCellEditing(Escher::HighlightCell * cell, int index);
  virtual void setTextInCell(Escher::HighlightCell * cell, const char * text, int index);

private:
  virtual InfinityTolerance infinityAllowanceForRow(int row) const { return InfinityTolerance::None; }
  virtual bool setParameterAtIndex(int parameterIndex, T f) = 0;
};

}

#endif
