#ifndef INFERENCE_STATISTIC_TEST_HYPOTHESIS_CONTROLLER_H
#define INFERENCE_STATISTIC_TEST_HYPOTHESIS_CONTROLLER_H

#include <escher/button_cell.h>
#include <escher/expression_cell_with_editable_text_with_message.h>
#include <escher/highlight_cell.h>
#include <escher/input_event_handler_delegate.h>
#include <escher/palette.h>
#include <escher/selectable_list_view_controller.h>
#include <escher/spacer_cell.h>
#include <escher/stack_view_controller.h>
#include <escher/text_field_delegate.h>
#include <escher/view.h>

#include "inference/statistic/chi_square_and_slope/input_slope_controller.h"
#include "inference/statistic/comparison_operator_popup_data_source.h"
#include "inference/statistic/expression_cell_with_sublabel_and_dropdown.h"
#include "inference/statistic/input_controller.h"

namespace Inference {

class HypothesisController
    : public Escher::ExplicitSelectableListViewController,
      public Escher::TextFieldDelegate,
      public Escher::DropdownCallback,
      public Escher::SelectableListViewDelegate {
 public:
  HypothesisController(Escher::StackViewController* parent,
                       InputController* inputController,
                       InputSlopeController* inputSlopeController,
                       Escher::InputEventHandlerDelegate* handler, Test* test);
  static bool ButtonAction(HypothesisController* controller, void* s);

  // SelectableListViewController
  ViewController::TitlesDisplay titlesDisplay() override {
    return ViewController::TitlesDisplay::DisplayLastTitle;
  };
  const char* title() override;
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;
  Escher::HighlightCell* cell(int i) override;
  int numberOfRows() const override { return 4; }

  // TextFieldDelegate
  bool textFieldDidReceiveEvent(Escher::AbstractTextField* textField,
                                Ion::Events::Event event) override;
  bool textFieldShouldFinishEditing(Escher::AbstractTextField* textField,
                                    Ion::Events::Event event) override;
  bool textFieldDidFinishEditing(Escher::AbstractTextField* textField,
                                 const char* text,
                                 Ion::Events::Event event) override;
  bool textFieldDidAbortEditing(Escher::AbstractTextField* textField) override;
  bool textFieldIsEditable(Escher::AbstractTextField* textField) override {
    return selectedRow() != 0 ||
           m_test->significanceTestType() != SignificanceTestType::Slope;
  }
  bool textFieldIsStorable(Escher::AbstractTextField* textField) override {
    return false;
  }

  // SelectableListViewDelegate
  bool canStoreContentOfCell(Escher::SelectableListView* t,
                             int row) const override {
    return false;
  }

  // DropdownCallback
  void onDropdownSelected(int selectedRow) override;

 private:
  void loadHypothesisParam();
  const char* symbolPrefix();

  constexpr static int k_indexOfH0 = 0;
  constexpr static int k_indexOfHa = 1;
  constexpr static int k_cellBufferSize =
      7 /* μ1-μ2 */ + 1 /* = */ +
      Constants::k_shortFloatNumberOfChars /* float */ + 1 /* \0 */;
  InputController* m_inputController;
  InputSlopeController* m_inputSlopeController;

  ComparisonOperatorPopupDataSource m_operatorDataSource;

  Escher::ExpressionCellWithEditableTextWithMessage m_h0;
  ExpressionCellWithSublabelAndDropdown m_ha;
  Escher::SpacerCell m_spacer;
  Escher::ButtonCell m_next;

  constexpr static int k_titleBufferSize =
      Ion::Display::Width / KDFont::GlyphWidth(KDFont::Size::Small);
  char m_titleBuffer[k_titleBufferSize];
  Test* m_test;
};

}  // namespace Inference

#endif
