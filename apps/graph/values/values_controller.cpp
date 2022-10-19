#include "values_controller.h"
#include <assert.h>
#include <escher/clipboard.h>
#include <poincare/decimal.h>
#include <poincare/layout_helper.h>
#include <poincare/serialization_helper.h>
#include <poincare/string_layout.h>
#include "../../shared/poincare_helpers.h"
#include "../../shared/utils.h"
#include "../../constant.h"
#include "../app.h"

using namespace Shared;
using namespace Poincare;
using namespace Escher;

namespace Graph {

// Constructors

ValuesController::ValuesController(Responder * parentResponder, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate, ButtonRowController * header, FunctionColumnParameterController * functionParameterController) :
  Shared::ValuesController(parentResponder, header),
  m_selectableTableView(this),
  m_prefacedView(0, this, &m_selectableTableView, this, this),
  m_functionParameterController(functionParameterController),
  m_intervalParameterController(this, inputEventHandlerDelegate),
  m_derivativeParameterController(this),
  m_setIntervalButton(this, I18n::Message::IntervalSet, Invocation([](void * context, void * sender) {
    ValuesController * valuesController = (ValuesController *) context;
    StackViewController * stack = ((StackViewController *)valuesController->stackController());
    IntervalParameterSelectorController * intervalSelectorController = valuesController->intervalParameterSelectorController();
    if (intervalSelectorController->numberOfRows() == 1) {
      IntervalParameterController * intervalController = valuesController->intervalParameterController();
      intervalController->setInterval(valuesController->intervalAtColumn(0));
      int i = 1;
      intervalSelectorController->setStartEndMessages(intervalController, valuesController->symbolTypeAtColumn(&i));
      stack->push(intervalController);
      return true;
    }
    stack->push(intervalSelectorController);
    return true;
  }, this), k_font),
  // TODO: Change text
  m_exactValuesButton(this, I18n::Message::PoolMemoryFull2, Invocation([](void * context, void * sender) {
    ValuesController * valuesController = (ValuesController *) context;
    valuesController->exactValuesButtonAction();
    return true;
  }, this), &m_exactValuesDotView, KDFont::Size::Small)
{
  for (int i = 0; i < k_maxNumberOfDisplayableFunctions; i++) {
    m_functionTitleCells[i].setFont(KDFont::Size::Small);
  }
  for (int i = 0; i < k_maxNumberOfDisplayableCells; i++) {
    m_valueCells[i].setFont(KDFont::Size::Small);
    m_valueCells[i].setAlignment(KDContext::k_alignRight, KDContext::k_alignCenter);
    // TODO: Factorize margin computation
    m_valueCells[i].setLeftMargin(Escher::EvenOddCell::k_horizontalMargin + 1);
    m_valueCells[i].setRightMargin(Escher::EvenOddCell::k_horizontalMargin + 1);
  }
  m_exactValuesButton.setState(false);
  setupSelectableTableViewAndCells(inputEventHandlerDelegate);
}

void ValuesController::viewDidDisappear() {
  for (int i = 0; i < k_maxNumberOfDisplayableCells; i++) {
    m_valueCells[i].setLayout(Layout());
  }
  Shared::ValuesController::viewDidDisappear();
}

// TableViewDataSource

KDCoordinate ValuesController::nonMemoizedColumnWidth(int i) {
  if (i == 0) {
    return k_abscissaCellWidth;
  }
  return k_cellWidth;
}

// TODO: Properly compute row height.
KDCoordinate ValuesController::nonMemoizedRowHeight(int j) {
  return Shared::ValuesController::nonMemoizedRowHeight(j);
}

void ValuesController::willDisplayCellAtLocation(HighlightCell * cell, int i, int j) {
  // Handle hidden cells
  int typeAtLoc = typeAtLocation(i,j);
  if (typeAtLoc == k_editableValueCellType) {
    StoreCell * storeCell = static_cast<StoreCell *>(cell);
    storeCell->setSeparatorLeft(i > 0);
  }
  if (typeAtLoc == k_notEditableValueCellType || typeAtLoc == k_editableValueCellType) {
    const int numberOfElementsInCol = numberOfElementsInColumn(i);
    // TODO: Create HideableEvenOddExpressionCell. I leave this here to keep the code for future commits
    //Shared::Hideable * hideableCell = hideableCellFromType(cell, typeAtLoc);
    //hideableCell->setHide(j > numberOfElementsInCol + 1);
    if (j >= numberOfElementsInCol + 1) {
      static_cast<EvenOddCell *>(cell)->setEven(j%2 == 0);
      //hideableCell->reinit();
      return;
    }
  }
  Shared::ValuesController::willDisplayCellAtLocation(cell, i, j);
}

int ValuesController::fillColumnName(int columnIndex, char * buffer) {
  if (typeAtLocation(columnIndex, 0) == k_functionTitleCellType) {
    const size_t bufferNameSize = ContinuousFunction::k_maxNameWithArgumentSize + 1;
    bool isDerivative = false;
    Ion::Storage::Record record = recordAtColumn(columnIndex, &isDerivative);
    Shared::ExpiringPointer<ContinuousFunction> function = functionStore()->modelForRecord(record);
    if (isDerivative) {
      return function->derivativeNameWithArgument(buffer, bufferNameSize);
    } else {
      if (function->isNamed()) {
        return function->nameWithArgument(buffer, bufferNameSize);
      } else {
        int size = PoincareHelpers::Serialize(function->originalEquation(), buffer, bufferNameSize, Preferences::VeryShortNumberOfSignificantDigits);
        // Serialization may have introduced system parentheses.
        SerializationHelper::ReplaceSystemParenthesesByUserParentheses(buffer, bufferNameSize - 1);
        return size;
      }
    }
  }
  return Shared::ValuesController::fillColumnName(columnIndex, buffer);
}

void ValuesController::setTitleCellText(HighlightCell * cell, int columnIndex) {
  if (typeAtLocation(columnIndex,0) == k_functionTitleCellType) {
    BufferFunctionTitleCell * myTitleCell = static_cast<BufferFunctionTitleCell *>(cell);
    fillColumnName(columnIndex, const_cast<char *>(myTitleCell->text()));
    return;
  }
  Shared::ValuesController::setTitleCellText(cell, columnIndex);
}

void ValuesController::setTitleCellStyle(HighlightCell * cell, int columnIndex) {
  if (typeAtLocation(columnIndex, 0)  == k_abscissaTitleCellType) {
    AbscissaTitleCell * myCell = static_cast<AbscissaTitleCell *>(cell);
    myCell->setSeparatorLeft(columnIndex > 0);
    return;
  }
  Shared::ValuesController::setTitleCellStyle(cell, columnIndex);
}

int ValuesController::typeAtLocation(int i, int j) {
  symbolTypeAtColumn(&i);
  return Shared::ValuesController::typeAtLocation(i, j);
}

// SelectableTableViewDelegate

void ValuesController::tableViewDidChangeSelection(SelectableTableView * t, int previousSelectedCellX, int previousSelectedCellY, bool withinTemporarySelection) {
  if (withinTemporarySelection) {
    return;
  }
  const int i = selectedColumn();
  int j = selectedRow();
  const int numberOfElementsInCol = numberOfElementsInColumn(i);
  if (j > 1 + numberOfElementsInCol) {
    selectCellAtLocation(i, 1 + numberOfElementsInCol);
    j = 1 + numberOfElementsInCol;
  }
}

// AlternateEmptyViewDelegate

bool ValuesController::isEmpty() const {
  if (functionStore()->numberOfActiveFunctionsInTable() == 0) {
    return true;
  }
  return false;
}

I18n::Message ValuesController::emptyMessage() {
  if (functionStore()->numberOfDefinedModels() == 0) {
    return I18n::Message::NoFunction;
  }
  return I18n::Message::NoActivatedFunction;
}

// Values controller

void ValuesController::setStartEndMessages(Shared::IntervalParameterController * controller, int column) {
  int c = column+1;
  m_intervalParameterSelectorController.setStartEndMessages(controller, symbolTypeAtColumn(&c));
}

// Number of columns memoization

void ValuesController::updateNumberOfColumns() const {
  for (size_t symbolTypeIndex = 0; symbolTypeIndex < k_maxNumberOfSymbolTypes; symbolTypeIndex++) {
    m_numberOfValuesColumnsForType[symbolTypeIndex] = 0;
  }
  int numberOfActiveFunctionsInTable = functionStore()->numberOfActiveFunctionsInTable();
  for (int i = 0; i < numberOfActiveFunctionsInTable; i++) {
    Ion::Storage::Record record = functionStore()->activeRecordInTableAtIndex(i);
    ExpiringPointer<ContinuousFunction> f = functionStore()->modelForRecord(record);
    int symbolTypeIndex = static_cast<int>(f->symbolType());
    m_numberOfValuesColumnsForType[symbolTypeIndex] += numberOfColumnsForRecord(record);
  }
  m_numberOfColumns = 0;
  for (size_t symbolTypeIndex = 0; symbolTypeIndex < k_maxNumberOfSymbolTypes; symbolTypeIndex++) {
    // Count abscissa column if the sub table does exist
    m_numberOfColumns += numberOfColumnsForSymbolType(symbolTypeIndex);
  }
}

// Model getters

Ion::Storage::Record ValuesController::recordAtColumn(int i) {
  bool isDerivative = false;
  return recordAtColumn(i, &isDerivative);
}

Ion::Storage::Record ValuesController::recordAtColumn(int i, bool * isDerivative) {
  assert(typeAtLocation(i, 0) == k_functionTitleCellType);
  ContinuousFunction::SymbolType symbolType = symbolTypeAtColumn(&i);
  int index = 1;
  int numberOfActiveFunctionsOfSymbolType = functionStore()->numberOfActiveFunctionsOfSymbolType(symbolType);
  for (int k = 0; k < numberOfActiveFunctionsOfSymbolType; k++) {
    Ion::Storage::Record record = functionStore()->activeRecordOfSymbolTypeAtIndex(symbolType, k);
    const int numberOfColumnsForCurrentRecord = numberOfColumnsForRecord(record);
    if (index <= i && i < index + numberOfColumnsForCurrentRecord) {
      ExpiringPointer<ContinuousFunction> f = functionStore()->modelForRecord(record);
      *isDerivative = i != index && f->canDisplayDerivative();
      return record;
    }
    index += numberOfColumnsForCurrentRecord;
  }
  assert(false);
  return nullptr;
}

Shared::Interval * ValuesController::intervalAtColumn(int columnIndex) {
  return App::app()->intervalForSymbolType(symbolTypeAtColumn(&columnIndex));
}

Shared::ExpiringPointer<ContinuousFunction> ValuesController::functionAtIndex(int column, int row, double * abscissa, bool * isDerivative) {
  *abscissa = intervalAtColumn(column)->element(row-1); // Subtract the title row from row to get the element index
  Ion::Storage::Record record = recordAtColumn(column, isDerivative);
  return functionStore()->modelForRecord(record);
}

// Number of columns

int ValuesController::numberOfColumnsForAbscissaColumn(int column) {
  return numberOfColumnsForSymbolType((int)symbolTypeAtColumn(&column));
}

int ValuesController::numberOfColumnsForRecord(Ion::Storage::Record record) const {
  ExpiringPointer<ContinuousFunction> f = functionStore()->modelForRecord(record);
  return 1 +
    (f->isAlongXOrY() && f->displayDerivative());
}

int ValuesController::numberOfColumnsForSymbolType(int symbolTypeIndex) const {
  return m_numberOfValuesColumnsForType[symbolTypeIndex] + (m_numberOfValuesColumnsForType[symbolTypeIndex] > 0); // Count abscissa column if there is one
}

int ValuesController::numberOfAbscissaColumnsBeforeColumn(int column) {
  int result = 0;
  int symbolType = column < 0 ?  k_maxNumberOfSymbolTypes : (int)symbolTypeAtColumn(&column) + 1;
  for (int symbolTypeIndex = 0; symbolTypeIndex < symbolType; symbolTypeIndex++) {
    result += (m_numberOfValuesColumnsForType[symbolTypeIndex] > 0);
  }
  return result;
}

int ValuesController::numberOfValuesColumns() {
  return m_numberOfColumns - numberOfAbscissaColumnsBeforeColumn(-1);
}

ContinuousFunction::SymbolType ValuesController::symbolTypeAtColumn(int * i) const {
  size_t symbolTypeIndex = 0;
  while (*i >= numberOfColumnsForSymbolType(symbolTypeIndex)) {
    *i -= numberOfColumnsForSymbolType(symbolTypeIndex++);
    assert(symbolTypeIndex < k_maxNumberOfSymbolTypes);
  }
  return static_cast<ContinuousFunction::SymbolType>(symbolTypeIndex);
}

// Function evaluation memoization

int ValuesController::valuesColumnForAbsoluteColumn(int column) {
  return column - numberOfAbscissaColumnsBeforeColumn(column);
}

int ValuesController::absoluteColumnForValuesColumn(int column) {
  int abscissaColumns = 0;
  int valuesColumns = 0;
  size_t symbolTypeIndex = 0;
  do {
    assert(symbolTypeIndex < k_maxNumberOfSymbolTypes);
    const int numberOfValuesColumnsForType = m_numberOfValuesColumnsForType[symbolTypeIndex++];
    valuesColumns += numberOfValuesColumnsForType;
    abscissaColumns += (numberOfValuesColumnsForType > 0);
  } while (valuesColumns <= column);
  return column + abscissaColumns;
}

void ValuesController::createMemoizedLayout(int column, int row, int index) {
  if (m_exactValuesButton.state()) {
    *memoizedLayoutAtIndex(index) = StringLayout::Builder("test");
    return;
  }
  double evaluationX = NAN;
  double evaluationY = NAN;
  double abscissa;
  bool isDerivative = false;
  Shared::ExpiringPointer<ContinuousFunction> function = functionAtIndex(column, row, &abscissa, &isDerivative);
  Poincare::Context * context = textFieldDelegateApp()->localContext();
  bool isParametric = function->symbolType() == ContinuousFunction::SymbolType::T;
  if (isDerivative) {
    assert(function->canDisplayDerivative());
    evaluationY = function->approximateDerivative(abscissa, context);
  } else {
    Poincare::Coordinate2D<double> eval = function->evaluate2DAtParameter(abscissa, context);
    evaluationY = eval.x2();
    if (isParametric) {
      evaluationX = eval.x1();
    }
  }
  Expression approximation;
  if (isParametric) {
    approximation = Matrix::Builder();
    static_cast<Matrix&>(approximation).addChildAtIndexInPlace(Float<double>::Builder(evaluationX), 0, 0);
    static_cast<Matrix&>(approximation).addChildAtIndexInPlace(Float<double>::Builder(evaluationY), 1, 1);
    static_cast<Matrix&>(approximation).setDimensions(2, 1);
  } else {
    approximation = Float<double>::Builder(evaluationY);
  }
  *memoizedLayoutAtIndex(index) = approximation.createLayout(Preferences::PrintFloatMode::Decimal, Preferences::VeryLargeNumberOfSignificantDigits, context);
}

// TODO: Properly compute exact layout. I leave this here for futur commits.

/*
void ValuesController::setExactValueCellLayouts(int column, int row) {
  // Compute approximate layout
  char * approximateResult = memoizedBufferForCell(column, row);
  Layout approximateLayout = Poincare::LayoutHelper::String(approximateResult);

  // Compute exact layout
  Layout exactLayout = Layout();
  Expression exactExpression = Expression();
  double abscissa;
  bool isDerivative = false;
  Shared::ExpiringPointer<ContinuousFunction> function = functionAtIndex(column, row, &abscissa, &isDerivative);
  Poincare::Context * context = textFieldDelegateApp()->localContext();
  if (!isDerivative) { // Do not compute exact derivative
    exactExpression = function->expressionReduced(context);
    if (function->symbol() != ContinuousFunction::k_parametricSymbol) {
      // Do not display exact value of parametric functions
      Poincare::VariableContext abscissaContext = Poincare::VariableContext(Shared::Function::k_unknownName, context);
      Poincare::Expression abscissaExpression = Poincare::Decimal::Builder<double>(abscissa);
      abscissaContext.setExpressionForSymbolAbstract(abscissaExpression, Symbol::Builder(Shared::Function::k_unknownName, strlen(Shared::Function::k_unknownName)));
      PoincareHelpers::CloneAndSimplify(&exactExpression, &abscissaContext, Poincare::ExpressionNode::ReductionTarget::User);
      if (!Utils::ShouldOnlyDisplayApproximation(function->originalEquation(), exactExpression, context)) {
        // Do not show exact expressions in certain cases
        exactLayout = exactExpression.createLayout(Poincare::Preferences::PrintFloatMode::Decimal, Poincare::Preferences::VeryLargeNumberOfSignificantDigits, context);
      }
    }
  }
  m_exactValueCell.setLayouts(exactLayout, approximateLayout);

  // Decide if the exact layout must be displayed
  bool displayExactLayout = !exactLayout.isUninitialized() && !exactLayout.isIdenticalTo(approximateLayout, true);
  m_exactValueCell.setDisplayCenter(displayExactLayout);

  // Decide if the equal sign is an approximate
  assert(!displayExactLayout || !exactExpression.isUninitialized());
  bool areExactlyEqual = displayExactLayout && Poincare::Expression::ExactAndApproximateBeautifiedExpressionsAreEqual(exactExpression, PoincareHelpers::ParseAndSimplify(approximateResult, textFieldDelegateApp()->localContext()));
  m_exactValueCell.setEqualMessage(areExactlyEqual ? I18n::Message::Equal : I18n::Message::AlmostEqual);
}*/

// Parameter controllers

template <class T>
T * ValuesController::parameterController() {
  bool isDerivative = false;
  Ion::Storage::Record record = recordAtColumn(selectedColumn(), &isDerivative);
  if (!functionStore()->modelForRecord(record)->isAlongXOrY()) {
    return nullptr;
  }
  if (isDerivative) {
    m_derivativeParameterController.setRecord(record);
    return &m_derivativeParameterController;
  }
  m_functionParameterController->setRecord(record);
  return m_functionParameterController;
}

I18n::Message ValuesController::valuesParameterMessageAtColumn(int columnIndex) const {
  return ContinuousFunction::MessageForSymbolType(symbolTypeAtColumn(&columnIndex));
}

// Cells & View

Shared::Hideable * ValuesController::hideableCellFromType(HighlightCell * cell, int type) {
  if (type == k_notEditableValueCellType) {
    Shared::HideableEvenOddBufferTextCell * myCell = static_cast<Shared::HideableEvenOddBufferTextCell *>(cell);
    return static_cast<Shared::Hideable *>(myCell);
  }
  assert(type == k_editableValueCellType);
  Shared::StoreCell * myCell = static_cast<Shared::StoreCell *>(cell);
  return static_cast<Shared::Hideable *>(myCell);
}

Shared::BufferFunctionTitleCell * ValuesController::functionTitleCells(int j) {
  assert(j >= 0 && j < k_maxNumberOfDisplayableFunctions);
  return &m_functionTitleCells[j];
}

EvenOddExpressionCell * ValuesController::valueCells(int j) {
  assert(j >= 0 && j < k_maxNumberOfDisplayableCells);
  return &m_valueCells[j];
}

 bool ValuesController::exactValuesButtonAction() {
  m_exactValuesButton.setState(!m_exactValuesButton.state());
  resetLayoutMemoization();
  m_selectableTableView.reloadData();
  return true;
}

/* ValuesController::ValuesSelectableTableView */

int writeMatrixBrackets(char * buffer, const int bufferSize, int type) {
  /* Write the double brackets required in matrix notation.
   * - type == 1: "[["
   * - type == 0: "]["
   * - type == -1: "]]"
   */
  int currentChar = 0;
  assert(currentChar < bufferSize-1);
  buffer[currentChar++] = type < 0 ? '[' : ']';
  assert(currentChar < bufferSize-1);
  buffer[currentChar++] = type <= 0 ? '[' : ']';
  return currentChar;
}

bool ValuesController::ValuesSelectableTableView::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Copy || event == Ion::Events::Cut) {
    HighlightCell * cell = selectedCell();
    if (cell) {
      const char * text = cell->text();
      if (text && text[0] == '(') {
        constexpr int bufferSize = 2*PrintFloat::k_maxFloatCharSize + 6; // "[[a][b]]" gives 6 characters in addition to the 2 floats
        char buffer[bufferSize];
        int currentChar = 0;
        currentChar += writeMatrixBrackets(buffer + currentChar, bufferSize - currentChar, -1);
        assert(currentChar < bufferSize-1);
        size_t semiColonPosition = UTF8Helper::CopyUntilCodePoint(buffer+currentChar, TextField::maxBufferSize() - currentChar, text+1, ';');
        currentChar += semiColonPosition;
        currentChar += writeMatrixBrackets(buffer + currentChar, bufferSize - currentChar, 0);
        assert(currentChar < bufferSize-1);
        currentChar += UTF8Helper::CopyUntilCodePoint(buffer+currentChar, TextField::maxBufferSize() - currentChar, text+1+semiColonPosition+1, ')');
        currentChar += writeMatrixBrackets(buffer + currentChar, bufferSize - currentChar, 1);
        assert(currentChar < bufferSize-1);
        buffer[currentChar] = 0;
        Clipboard::sharedClipboard()->store(buffer);
        return true;
      }
    }
  }
  return SelectableTableView::handleEvent(event);
}

}
