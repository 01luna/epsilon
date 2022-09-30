#include "calculation_parameter_controller.h"
#include "area_between_curves_parameter_controller.h"
#include "graph_controller.h"
#include "../app.h"
#include <assert.h>
#include <cmath>

using namespace Shared;
using namespace Escher;

namespace Graph {

CalculationParameterController::CalculationParameterController(Responder * parentResponder, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate, GraphView * graphView, BannerView * bannerView, InteractiveCurveViewRange * range, CurveViewCursor * cursor, GraphController * graphController) :
  SelectableListViewController(parentResponder),
  m_preimageCell(I18n::Message::Preimage),
  m_graphController(graphController),
  m_preimageParameterController(nullptr, inputEventHandlerDelegate, range, cursor, &m_preimageGraphController),
  m_preimageGraphController(nullptr, graphView, bannerView, range, cursor),
  m_derivativeCell(I18n::Message::GraphDerivative),
  m_tangentGraphController(nullptr, graphView, bannerView, range, cursor),
  m_integralGraphController(nullptr, inputEventHandlerDelegate, graphView, range, cursor),
  m_areaParameterController(nullptr, &m_areaGraphController),
  m_areaGraphController(nullptr, inputEventHandlerDelegate, graphView, range, cursor),
  m_minimumGraphController(nullptr, graphView, bannerView, range, cursor),
  m_maximumGraphController(nullptr, graphView, bannerView, range, cursor),
  m_rootGraphController(nullptr, graphView, bannerView, range, cursor),
  m_intersectionGraphController(nullptr, graphView, bannerView, range, cursor)
{
}

const char * CalculationParameterController::title() {
  return I18n::translate(I18n::Message::Compute);
}

void CalculationParameterController::viewWillAppear() {
  ViewController::viewWillAppear();
  m_selectableTableView.reloadData();
}

void CalculationParameterController::didBecomeFirstResponder() {
  if (selectedRow() < 0) {
    m_selectableTableView.selectCellAtLocation(0, 0);
  }
  Container::activeApp()->setFirstResponder(&m_selectableTableView);
}

bool CalculationParameterController::handleEvent(Ion::Events::Event event) {
  int row = selectedRow();

  if (event == Ion::Events::Left) {
    StackViewController * stack = static_cast<StackViewController *>(parentResponder());
    stack->pop();
    return true;
  }

  if (event != Ion::Events::OK && event != Ion::Events::EXE && (event != Ion::Events::Right || !RightEventIsEquivalentToEnterEventOnRow(row))) {
    return false;
  }

  static ViewController * controllers[] = {
    &m_preimageParameterController,
    &m_intersectionGraphController,
    &m_maximumGraphController,
    &m_minimumGraphController,
    &m_rootGraphController,
    &m_tangentGraphController,
    &m_integralGraphController,
    &m_areaParameterController
  };

  bool displayIntersection = ShouldDisplayIntersection();
  int realDerivativeRowIndex = k_derivativeRowIndex + displayIntersection;
  if (row == realDerivativeRowIndex) {
    m_graphController->setDisplayDerivativeInBanner(!m_graphController->displayDerivativeInBanner());
    m_selectableTableView.reloadData();
    return true;
  }

  int indexController = row == 0 ? 0 : row + !displayIntersection - (row > realDerivativeRowIndex);
  ViewController * controller = controllers[indexController];

  if (controller == &m_preimageParameterController) {
    assert(row == 0);
    m_preimageParameterController.setRecord(m_record);
  } else if (controller == &m_tangentGraphController) {
    assert(row == realDerivativeRowIndex +1);
    m_tangentGraphController.setRecord(m_record);
  } else if (controller == &m_integralGraphController) {
    assert(row == realDerivativeRowIndex + 2);
    m_integralGraphController.setRecord(m_record);
  } else if (controller == &m_areaParameterController) {
    assert(row == realDerivativeRowIndex + 3);
    if (App::app()->functionStore()->numberOfActiveDerivableFunctions() == 2) {
      controller = &m_areaGraphController;
      m_areaGraphController.setRecord(m_record);
      Ion::Storage::Record secondRecord = AreaBetweenCurvesParameterController::DerivableActiveFunctionAtIndex(0, m_record);
      m_areaGraphController.setSecondRecord(secondRecord);
    } else {
      m_areaParameterController.setRecord(m_record);
    }
  } else {
    assert((controller == &m_intersectionGraphController && row == 1 && displayIntersection)
           || (controller == &m_rootGraphController && row == realDerivativeRowIndex - 1)
           || (controller == &m_maximumGraphController && row == realDerivativeRowIndex - 2)
           || (controller == &m_maximumGraphController && row == realDerivativeRowIndex - 3));
    static_cast<CalculationGraphController *>(controller)->setRecord(m_record);
  }

  StackViewController * stack = static_cast<StackViewController *>(parentResponder());
  if (controller != &m_preimageParameterController && controller != &m_areaParameterController) {
    /* setupActiveViewController() must be called here because the graph view
     * must be re-layouted before pushing the controller */
    stack->popUntilDepth(Shared::InteractiveCurveViewController::k_graphControllerStackDepth, true);
  }
  stack->push(controller);
  return true;
}

int CalculationParameterController::numberOfRows() const {
  /* The intersection option should always be displayed if the area between
   * curve is displayed. If not, k_areaRowIndex would be false. */
  assert(!ShouldDisplayAreaBetweenCurves() || ShouldDisplayIntersection());
  /* Inverse row + [optional intersection row] + [optional area between curves
   * row] + derivative + all other rows (max, min zeros, derivative, tangent,
   * integral) */
  return 1 + ShouldDisplayIntersection() + ShouldDisplayAreaBetweenCurves() + 1 + k_totalNumberOfReusableCells - 1;
};

HighlightCell * CalculationParameterController::reusableCell(int index, int type) {
  assert(index >= 0);
  assert(index < reusableCellCount(type));
  switch (type){
  case k_defaultCellType:
    return &m_cells[index];
  case k_derivativeCellType:
    return &m_derivativeCell;
  case k_areaCellType:
    return &m_areaCell;
  default:
    assert(type == k_preImageCellType);
    return &m_preimageCell;
  }
}

int CalculationParameterController::reusableCellCount(int type) {
  return type == k_defaultCellType ? k_totalNumberOfReusableCells : 1;
}

int CalculationParameterController::typeAtIndex(int index) {
  if (index == 0) {
    return k_preImageCellType;
  }
  if (index == k_areaRowIndex) {
    return k_areaCellType;
  }
  if (index == k_derivativeRowIndex + ShouldDisplayIntersection()) {
    return k_derivativeCellType;
  }
  return k_defaultCellType;
}

void CalculationParameterController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  assert(index >= 0 && index <= numberOfRows());
  if (cell == &m_derivativeCell) {
    m_derivativeCell.setState(m_graphController->displayDerivativeInBanner());
    return;
  } else if (cell == &m_preimageCell) {
    return;
  } else if (cell != &m_areaCell) {
    I18n::Message titles[] = {
      I18n::Message::Intersection,
      I18n::Message::Maximum,
      I18n::Message::Minimum,
      I18n::Message::Zeros,
      I18n::Message::Default, // always skipped
      I18n::Message::Tangent,
      I18n::Message::Integral
    };
    static_cast<MessageTableCell *>(cell)->setMessage(titles[index - 1 + !ShouldDisplayIntersection()]);
    return;
  }

  assert(cell == &m_areaCell);
  int numberOfFunctions = App::app()->functionStore()->numberOfActiveDerivableFunctions();
  assert(numberOfFunctions > 1);
  // If there is only two derivable functions, hide the chevron
  m_areaCell.hideChevron(numberOfFunctions == 2);
  // Get the name of the selected function
  ExpiringPointer<ContinuousFunction> mainFunction = App::app()->functionStore()->modelForRecord(m_record);
  constexpr static int bufferSize = Shared::Function::k_maxNameWithArgumentSize;
  char mainFunctionName[bufferSize];
  mainFunction->nameWithArgument(mainFunctionName, bufferSize);

  char secondPlaceHolder[bufferSize];
  if (numberOfFunctions == 2) {
    // If there are only 2 functions, display "Area between f(x) and g(x)"
    secondPlaceHolder[0] = ' ';
    Ion::Storage::Record secondRecord = AreaBetweenCurvesParameterController::DerivableActiveFunctionAtIndex(0, m_record);
    ExpiringPointer<ContinuousFunction> secondFunction = App::app()->functionStore()->modelForRecord(secondRecord);
    secondFunction->nameWithArgument(secondPlaceHolder + 1, bufferSize);
    if (strcmp(mainFunctionName, secondPlaceHolder + 1) == 0) {
      // If both functions are name "y", display "Area between curves"
      m_areaCell.setMessageWithPlaceholders(I18n::Message::AreaBetweenCurves);
      return;
    }
  } else {
    // If there are more than 2 functions, display "Area between f(x) and"
    secondPlaceHolder[0] = 0;
  }
  m_areaCell.setMessageWithPlaceholders(I18n::Message::AreaBetweenCurvesWithFunctionName, mainFunctionName, secondPlaceHolder);
  if (m_areaCell.labelView()->minimalSizeForOptimalDisplay().width() > m_areaCell.innerWidth()) {
    // If there is not enough space in the cell, display "Area between curves"
    m_areaCell.setMessageWithPlaceholders(I18n::Message::AreaBetweenCurves);
  }
}

void CalculationParameterController::setRecord(Ion::Storage::Record record) {
  m_record = record;
}

bool CalculationParameterController::ShouldDisplayIntersection() {
  /* Intersection is handled between all active functions having one subcurve,
   * except Polar and Parametric. */
  ContinuousFunctionStore * store = App::app()->functionStore();
  /* Intersection row is displayed if there is at least two intersectable
   * functions. */
  return store->numberOfIntersectableFunctions() > 1;
}

bool CalculationParameterController::ShouldDisplayAreaBetweenCurves() {
  ContinuousFunctionStore * store = App::app()->functionStore();
  /* Area between curves is displayed if there is at least two derivable
   * functions. */
  return store->numberOfActiveDerivableFunctions() > 1;
}

bool CalculationParameterController::RightEventIsEquivalentToEnterEventOnRow(int row) {
  /* First row is inverse image.
   * Area between curves row does not always have a chevron. */
  return row == 0 || (row == k_areaRowIndex && App::app()->functionStore()->numberOfActiveDerivableFunctions() > 2);
}

}
