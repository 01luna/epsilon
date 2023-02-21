#ifndef SOLVER_LIST_CONTROLLER_H
#define SOLVER_LIST_CONTROLLER_H

#include <escher/button_row_controller.h>
#include <escher/even_odd_expression_cell.h>
#include <escher/even_odd_editable_expression_cell.h>
#include <apps/shared/expression_model_list_controller.h>
#include <apps/shared/layout_field_delegate.h>
#include <apps/shared/text_field_delegate.h>
#include "equation_store.h"
#include "equation_list_view.h"
#include "equation_models_parameter_controller.h"
#include <apps/i18n.h>

namespace Solver {

class ListController : public Shared::ExpressionModelListController, public Escher::ButtonRowDelegate, public Escher::MemoizedListViewDataSource, public Shared::LayoutFieldDelegate {
public:
  ListController(Escher::Responder * parentResponder, EquationStore * equationStore, Escher::ButtonRowController * footer);
  /* ButtonRowDelegate */
  int numberOfButtons(Escher::ButtonRowController::Position position) const override;
  Escher::AbstractButtonCell * buttonAtIndex(int index, Escher::ButtonRowController::Position position) const override;
  /* ListViewDataSource */
  int numberOfRows() const override { return numberOfExpressionRows(); }
  int typeAtIndex(int index) const override { return index==m_editedCellIndex ? k_editedCellType : isAddEmptyRow(index); }
  Escher::HighlightCell * reusableCell(int index, int type) override;
  int reusableCellCount(int type) override;
  void willDisplayCellForIndex(Escher::HighlightCell * cell, int index) override;
  /* Responder */
  bool handleEvent(Ion::Events::Event event) override;
  void didBecomeFirstResponder() override;
  void didEnterResponderChain(Escher::Responder * previousFirstResponder) override;
  /* ViewController */
  Escher::View * view() override { return &m_equationListView; }
  TELEMETRY_ID("List");
  /* Layout Field Delegate */
  bool layoutFieldDidReceiveEvent(Escher::LayoutField * layoutField, Ion::Events::Event event) override;
  bool layoutFieldDidFinishEditing(Escher::LayoutField * layoutField, Poincare::Layout layout, Ion::Events::Event event) override;
  void layoutFieldDidChangeSize(Escher::LayoutField * layoutField) override;
  bool layoutFieldDidAbortEditing(Escher::LayoutField * layoutField) override;
  /* ExpressionModelListController */
  void editExpression(Ion::Events::Event event) override;
  /* Specific to Solver */
  void resolveEquations();
private:
  constexpr static int k_editedCellType = 2;
  constexpr static int k_maxNumberOfRows = Escher::Metric::MinimalNumberOfScrollableRowsToFillDisplayHeight(Escher::Metric::StoreRowHeight, Escher::Metric::ButtonRowEmbossedStyleHeightLarge);
  Escher::SelectableTableView * selectableTableView() override;
  void reloadButtonMessage();
  void addModel() override;
  bool removeModelRow(Ion::Storage::Record record) override;
  void reloadBrace();
  EquationStore * modelStore() const override;
  Escher::StackViewController * stackController() const;

  // ListViewDataSource
  KDCoordinate nonMemoizedRowHeight(int j) override;
  // ExpressionModelListController
  void resetSizesMemoization() override { resetMemoization(); }

  EquationListView m_equationListView;
  Escher::EvenOddExpressionCell m_expressionCells[k_maxNumberOfRows];
  Escher::EvenOddEditableExpressionCell m_editableCell;
  Escher::AbstractButtonCell m_resolveButton;
  EquationModelsParameterController m_modelsParameterController;
  Escher::StackViewController m_modelsStackController;
};

}

#endif
