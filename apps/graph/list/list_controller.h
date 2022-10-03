#ifndef GRAPH_LIST_CONTROLLER_H
#define GRAPH_LIST_CONTROLLER_H

#include "function_toolbox.h"
#include "../shared/function_parameter_controller.h"
#include "../continuous_function_store.h"
#include <apps/shared/function_list_controller.h>
#include <apps/shared/text_field_delegate.h>
#include <apps/shared/layout_field_delegate.h>
#include <apps/shared/input_event_handler_delegate.h>
#include "function_models_parameter_controller.h"
#include "../graph/graph_controller.h"
#include "function_cell.h"

namespace Graph {

class ListController : public Shared::FunctionListController, public Shared::TextFieldDelegate, public Shared::LayoutFieldDelegate, public Shared::InputEventHandlerDelegate, public Escher::MemoizedListViewDataSource {
public:
  ListController(Escher::Responder * parentResponder, Escher::ButtonRowController * header, Escher::ButtonRowController * footer, GraphController * graphController);
  // ListViewDataSource
  int numberOfRows() const override { return this->numberOfExpressionRows(); }
  int typeAtIndex(int index) override;
  Escher::HighlightCell * reusableCell(int index, int type) override;
  int reusableCellCount(int type) override;
  // ViewController
  const char * title() override;
  Escher::View * view() override { return &m_selectableTableView; }
  // LayoutFieldDelegate
  bool layoutFieldDidReceiveEvent(Escher::LayoutField * layoutField, Ion::Events::Event event) override;
  // TextFieldDelegate
  bool textFieldDidReceiveEvent(Escher::AbstractTextField * textField, Ion::Events::Event event) override;
  // Responder
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;
  // ExpressionModelListController
  KDCoordinate expressionRowHeight(int j) override;
  Escher::SelectableTableView * selectableTableView() override { return &m_selectableTableView; }
  FunctionToolbox * toolboxForInputEventHandler(Escher::InputEventHandler * handler) override;
  Shared::ListParameterController * parameterController() override;
private:
  constexpr static int k_functionCellType = 0;
  constexpr static int k_addNewModelType = 1;
  // 6 rows of undefined empty functions
  constexpr static int k_maxNumberOfDisplayableRows = 6;
  constexpr static CodePoint k_equationSymbols[] = { '=', '>', '<', UCodePointSuperiorEqual, UCodePointInferiorEqual};

  // ExpressionModelListController
  void resetSizesMemoization() override { resetMemoization(); }
  // ListViewDataSource
  KDCoordinate nonMemoizedRowHeight(int j) override { return expressionRowHeight(j); }

  void fillWithDefaultFunctionEquation(char * buffer, size_t bufferSize, FunctionModelsParameterController * modelsParameterController, CodePoint Symbol) const;
  bool layoutRepresentsAnEquation(Poincare::Layout l) const;
  bool layoutRepresentsPolarFunction(Poincare::Layout l) const;
  bool layoutRepresentsParametricFunction(Poincare::Layout l) const;
  bool textRepresentsAnEquation(const char * text) const;
  bool textRepresentsPolarFunction(const char * text) const;
  bool textRepresentsParametricFunction(const char * text) const;
  // Complete the equationField with a valid left equation side
  bool completeEquation(Escher::InputEventHandler * equationField, CodePoint symbol);
  void addModel() override;
  int maxNumberOfDisplayableRows() override;
  Escher::HighlightCell * functionCells(int index) override;
  void willDisplayCellForIndex(Escher::HighlightCell * cell, int j) override;
  ContinuousFunctionStore * modelStore() override;
  Escher::SelectableTableView m_selectableTableView;
  FunctionCell m_expressionCells[k_maxNumberOfDisplayableRows];
  FunctionParameterController m_parameterController;
  FunctionModelsParameterController m_modelsParameterController;
  Escher::StackViewController m_modelsStackController;
  FunctionToolbox m_functionToolbox;
  bool m_parameterColumnSelected;
};

}

#endif
