#include "function_column_parameter_controller.h"
#include "../app.h"

namespace Graph {

FunctionColumnParameterController::FunctionColumnParameterController(Responder * parentResponder, I18n::Message functionColorMessage, I18n::Message deleteFunctionMessage, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate, GraphController * graphController, ValuesController * valuesController) :
  FunctionParameterController(parentResponder, functionColorMessage, deleteFunctionMessage, inputEventHandlerDelegate, graphController),
  m_valuesController(valuesController)
{}

Shared::ClearColumnHelper * FunctionColumnParameterController::clearColumnHelper() {
  return m_valuesController;
}

}
