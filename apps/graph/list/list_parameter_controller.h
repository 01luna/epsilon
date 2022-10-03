#ifndef GRAPH_LIST_LIST_PARAM_CONTROLLER_H
#define GRAPH_LIST_LIST_PARAM_CONTROLLER_H

#include <apps/shared/list_parameter_controller.h>
#include <apps/exam_mode_configuration.h>
#include <escher/message_table_cell_with_chevron_and_message.h>
#include <escher/message_table_cell_with_chevron_and_buffer.h>
#include "details_parameter_controller.h"
#include "domain_parameter_controller.h"

namespace Graph {

class ListController;

class ListParameterController : public Shared::ListParameterController {
public:
  ListParameterController(Escher::Responder * parentResponder, I18n::Message functionColorMessage, I18n::Message deleteFunctionMessage, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate);
  void setRecord(Ion::Storage::Record record) override;
  // MemoizedListViewDataSource
  Escher::HighlightCell * cell(int index) override;
  void willDisplayCellForIndex(Escher::HighlightCell * cell, int index) override;
  bool handleEvent(Ion::Events::Event event) override;
  // Shared cells + m_detailsCell + m_functionDomain
  int numberOfRows() const override { return 2 + Shared::ListParameterController::numberOfRows(); }
private:
  bool displayDetails() const { return !ExamModeConfiguration::implicitPlotsAreForbidden() && m_detailsParameterController.detailsNumberOfSections() > 0; }
  bool displayDomain() const { return m_domainParameterController.isVisible() > 0; }
  void detailsPressed();
  void functionDomainPressed();
  Escher::MessageTableCellWithChevronAndMessage m_detailsCell;
  Escher::MessageTableCellWithChevronAndBuffer m_functionDomainCell;
  DetailsParameterController m_detailsParameterController;
  DomainParameterController m_domainParameterController;
};

}

#endif
