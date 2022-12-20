#ifndef GRAPH_VALUES_CONTROLLER_H
#define GRAPH_VALUES_CONTROLLER_H

#include "../continuous_function_store.h"
#include <apps/shared/editable_cell_selectable_table_view.h>
#include <apps/shared/expression_function_title_cell.h>
#include <apps/shared/interval_parameter_controller.h>
#include <apps/shared/scrollable_two_expressions_cell.h>
#include <apps/shared/store_cell.h>
#include <apps/shared/values_controller.h>
#include "abscissa_title_cell.h"
#include "derivative_parameter_controller.h"
#include "function_column_parameter_controller.h"
#include "interval_parameter_selector_controller.h"
#include <escher/button_state.h>
#include <escher/toggleable_dot_view.h>

namespace Graph {

class ValuesController : public Shared::ValuesController, public Escher::SelectableTableViewDelegate, public Shared::PrefacedTableViewDelegate {
public:
  ValuesController(Escher::Responder * parentResponder, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate, Escher::ButtonRowController * header, FunctionColumnParameterController * functionParameterController);
  bool displayExactValues() const;

  // View controller
  Escher::View * view() override { return &m_prefacedTableView; }
  void viewDidDisappear() override;

  // TableViewDataSource
  void willDisplayCellAtLocation(Escher::HighlightCell * cell, int i, int j) override;
  int typeAtLocation(int i, int j) override;

  // ButtonRowDelegate
  int numberOfButtons(Escher::ButtonRowController::Position) const override { return isEmpty() ? 0 : 1 + displayExactValues(); }
  Escher::AbstractButtonCell * buttonAtIndex(int index, Escher::ButtonRowController::Position position) const override;

  // AlternateEmptyViewDelegate
  bool isEmpty() const override { return functionStore()->numberOfActiveFunctionsInTable() == 0; }
  I18n::Message emptyMessage() override { return functionStore()->numberOfDefinedModels() == 0 ? I18n::Message::NoFunction : I18n::Message::NoActivatedFunction; }

  // Shared::ValuesController
  Shared::IntervalParameterController * intervalParameterController() override { return &m_intervalParameterController; }
  IntervalParameterSelectorController * intervalParameterSelectorController() { return &m_intervalParameterSelectorController; }

  // PrefacedTableViewDelegate
  KDCoordinate maxPrefaceHeight() const override { return 3 * k_cellHeight; }

private:
  constexpr static size_t k_maxNumberOfSymbolTypes = Shared::ContinuousFunctionProperties::k_numberOfSymbolTypes;
  constexpr static int k_maxNumberOfDisplayableFunctions = 4;
  constexpr static int k_maxNumberOfDisplayableSymbolTypes = 2;
  constexpr static int k_maxNumberOfDisplayableAbscissaCells = k_maxNumberOfDisplayableSymbolTypes * k_maxNumberOfDisplayableRows;
  constexpr static int k_maxNumberOfDisplayableCells = k_maxNumberOfDisplayableFunctions * k_maxNumberOfDisplayableRows;
  constexpr static int k_valuesCellBufferSize = 2 * Poincare::PrintFloat::charSizeForFloatsWithPrecision(Poincare::Preferences::VeryLargeNumberOfSignificantDigits) + 3; // The largest buffer holds (-1.234567E-123;-1.234567E-123)
  static KDSize ApproximatedParametricCellSize();
  static KDSize CellSizeWithLayout(Poincare::Layout l);
  static KDCoordinate MaxColumnWidth() { return 2 * k_cellWidth; }
  static KDCoordinate MaxRowHeight() { return 5 * k_cellHeight; }

  // TabTableController
  Escher::SelectableTableView * selectableTableView() override { return &m_selectableTableView; }

  // TableViewDataSource
  /* Note: computing the total height and width of the table
   * when exact results are switched on is slow because all layouts
   * need to be computed. The speed optimization could come from either
   * a change of API or a change in the way scrollView/tableView work. */
  KDCoordinate nonMemoizedColumnWidth(int i) override;
  KDCoordinate nonMemoizedRowHeight(int j) override;
  Escher::TableSize1DManager * columnWidthManager() override { return &m_widthManager; }
  Escher::TableSize1DManager * rowHeightManager() override { return &m_heightManager; }

  // ColumnHelper
  int fillColumnName(int columnIndex, char * buffer) override;

  // EditableCellTableViewController
  void reloadEditedCell(int column, int row) override;
  void updateSizeMemoizationForRow(int row, KDCoordinate rowPreviousHeight) override { m_heightManager.updateMemoizationForIndex(row, rowPreviousHeight); }
  void setTitleCellText(Escher::HighlightCell * titleCell, int columnIndex) override;
  void setTitleCellStyle(Escher::HighlightCell * titleCell, int columnIndex) override;

  // Shared::ValuesController
  ContinuousFunctionStore * functionStore() const override { return static_cast<ContinuousFunctionStore *>(Shared::ValuesController::functionStore()); }
  Ion::Storage::Record recordAtColumn(int i) override;
  void updateNumberOfColumns() const override;
  Poincare::Layout * memoizedLayoutAtIndex(int i) override;
  int numberOfMemoizedColumn() override { return k_maxNumberOfDisplayableFunctions; }
  Shared::PrefacedTableView * prefacedView() override { return &m_prefacedTableView; }
  void setStartEndMessages(Shared::IntervalParameterController * controller, int column) override;
  int numberOfValuesColumns() const override { return numberOfColumns() - numberOfAbscissaColumnsBeforeColumn(-1); }
  int valuesColumnForAbsoluteColumn(int column) override { return column - numberOfAbscissaColumnsBeforeColumn(column); }
  int absoluteColumnForValuesColumn(int column) override;
  void createMemoizedLayout(int column, int row, int index) override;
  int numberOfColumnsForAbscissaColumn(int column) override;
  void updateSizeMemoizationForColumnAfterIndexChanged(int column, KDCoordinate columnPreviousWidth, int changedRow) override;
  Shared::Interval * intervalAtColumn(int columnIndex) override;
  I18n::Message valuesParameterMessageAtColumn(int columnIndex) const override;
  int maxNumberOfCells() override { return k_maxNumberOfDisplayableCells; }
  int maxNumberOfFunctions() override { return k_maxNumberOfDisplayableFunctions; }
  Shared::ExpressionFunctionTitleCell * functionTitleCells(int j) override;
  Escher::EvenOddExpressionCell * valueCells(int j) override;
  int abscissaCellsCount() const override { return k_maxNumberOfDisplayableAbscissaCells; }
  Escher::EvenOddEditableTextCell * abscissaCells(int j) override;
  int abscissaTitleCellsCount() const override { return k_maxNumberOfDisplayableSymbolTypes; }
  Escher::EvenOddMessageTextCell * abscissaTitleCells(int j) override;
  Escher::SelectableViewController * functionParameterController() override { return parameterController<Escher::SelectableViewController>(); }
  Shared::ColumnParameters * functionParameters() override { return parameterController<Shared::ColumnParameters>(); }

  Poincare::Layout functionTitleLayout(int columnIndex);
  template <class T> T * parameterController();
  bool exactValuesButtonAction();
  void activateExactValues(bool activate);
  Ion::Storage::Record recordAtColumn(int i, bool * isDerivative);
  Shared::ExpiringPointer<Shared::ContinuousFunction> functionAtIndex(int column, int row, double * abscissa, bool * isDerivative);
  int numberOfColumnsForRecord(Ion::Storage::Record record) const;
  int numberOfColumnsForSymbolType(int symbolTypeIndex) const;
  int numberOfAbscissaColumnsBeforeColumn(int column) const;
  Shared::ContinuousFunctionProperties::SymbolType symbolTypeAtColumn(int * i) const;
  
  Shared::PrefacedTableView m_prefacedTableView;
  Shared::EditableCellSelectableTableView m_selectableTableView;
  mutable int m_numberOfValuesColumnsForType[k_maxNumberOfSymbolTypes];
  Shared::ExpressionFunctionTitleCell m_functionTitleCells[k_maxNumberOfDisplayableFunctions];
  Escher::EvenOddExpressionCell m_valueCells[k_maxNumberOfDisplayableCells];
  AbscissaTitleCell m_abscissaTitleCells[k_maxNumberOfDisplayableSymbolTypes];
  Shared::StoreCell m_abscissaCells[k_maxNumberOfDisplayableAbscissaCells];
  FunctionColumnParameterController * m_functionParameterController;
  Shared::IntervalParameterController m_intervalParameterController;
  IntervalParameterSelectorController m_intervalParameterSelectorController;
  DerivativeParameterController m_derivativeParameterController;
  Escher::AbstractButtonCell m_setIntervalButton;
  Escher::ButtonState m_exactValuesButton;
  Escher::ToggleableDotView m_exactValuesDotView;
  Escher::ShortMemoizedColumnWidthManager m_widthManager;
  Escher::LongMemoizedRowHeightManager m_heightManager;
  bool m_exactValuesAreActivated;
  mutable Poincare::Layout m_memoizedLayouts[k_maxNumberOfDisplayableCells];
};

}

#endif
