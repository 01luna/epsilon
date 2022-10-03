#ifndef SHARED_EXPLICIT_FLOAT_PARAMETER_CONTROLLER
#define SHARED_EXPLICIT_FLOAT_PARAMETER_CONTROLLER

#include "parameter_text_field_delegate.h"
#include <escher/explicit_selectable_list_view_controller.h>
#include <escher/selectable_list_view_controller.h>
#include <escher/stack_view_controller.h>
#include <escher/message_table_cell_with_editable_text.h>

namespace Shared {

/* This controller edits float parameter of any model (given through
 * parameterAtIndex and setParameterAtIndex). */

template<typename T, typename M=Escher::MessageTableCellWithEditableText>
class ExplicitFloatParameterController : public Escher::ExplicitSelectableListViewController, public ParameterTextFieldDelegate {
public:
  ExplicitFloatParameterController(Escher::Responder * parentResponder);
  void didBecomeFirstResponder() override;
  void viewWillAppear() override;
  void viewDidDisappear() override;
  bool handleEvent(Ion::Events::Event event) override;

  void willDisplayCellForIndex(Escher::HighlightCell * cell, int index) override;
  KDCoordinate nonMemoizedRowHeight(int j) override;
  bool textFieldShouldFinishEditing(Escher::AbstractTextField * textField, Ion::Events::Event event) override;
  bool textFieldDidFinishEditing(Escher::AbstractTextField * textField, const char * text, Ion::Events::Event event) override;

protected:
  enum class InfinityTolerance {
    None,
    PlusInfinity,
    MinusInfinity
  };
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
