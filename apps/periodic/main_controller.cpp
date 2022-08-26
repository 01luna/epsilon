#include "main_controller.h"
#include "app.h"
#include "table_layout.h"

namespace Periodic {

// MainController::ContentView

Escher::View * MainController::ContentView::subviewAtIndex(int index) {
  if (index == 0) {
    return &m_elementsView;
  }
  assert(index == 1);
  return &m_bannerView;
}

void MainController::ContentView::layoutSubviews(bool force) {
  KDSize bannerSize = m_bannerView.minimalSizeForOptimalDisplay();
  m_bannerView.setFrame(KDRect(0, bounds().height() - bannerSize.height(), bounds().width(), bannerSize.height()), force);
  m_elementsView.setFrame(KDRect(0, 0, bounds().width(), bounds().height() - bannerSize.height()), force);
}

// MainController

MainController::MainController(Escher::StackViewController * parentResponder) :
  ViewController(parentResponder),
  m_detailsController(parentResponder),
  m_displayTypeController(parentResponder),
  m_view(this)
{}

void MainController::selectedElementHasChanged() {
  m_view.bannerView()->reload();
  m_view.elementsView()->cursorMoved();
  m_detailsController.resetMemoization();
}

void MainController::activeDataFieldHasChanged() {
  m_view.bannerView()->reload();
}

bool MainController::handleEvent(Ion::Events::Event e) {
  if (e == Ion::Events::OnOff) {
    /* ElementsView only redraws its background when appearing to avoid
     * blinking. Powering down will discard the background so we need to flag
     * it for redraw. */
    m_view.elementsView()->dirtyBackground();
    return false;
  }

  ElementsViewDataSource * dataSource = App::app()->elementsViewDataSource();
  AtomicNumber z = dataSource->selectedElement();

  if (!ElementsDataBase::IsElement(z)) {
    if (e == Ion::Events::Up) {
      dataSource->setSelectedElement(dataSource->previousElement());
      return true;
    }
    if (e == Ion::Events::OK || e == Ion::Events::EXE) {
      stackOpenPage(&m_displayTypeController);
      return true;
    }
    return false;
  }

  if (e == Ion::Events::OK || e == Ion::Events::EXE) {
    stackOpenPage(&m_detailsController);
    return true;
  }

  AtomicNumber newZ = z;
  if (e == Ion::Events::Up) {
    newZ = TableLayout::NextElement(z, TableLayout::Direction::DecreasingRow);
  } else if (e == Ion::Events::Down) {
    newZ = TableLayout::NextElement(z, TableLayout::Direction::IncreasingRow);
  } else if (e == Ion::Events::Left) {
    newZ = TableLayout::NextElement(z, TableLayout::Direction::DecreasingZ);
  } else if (e == Ion::Events::Right) {
    newZ = TableLayout::NextElement(z, TableLayout::Direction::IncreasingZ);
  }
  dataSource->setSelectedElement(newZ);
  return newZ != z;
}

void MainController::textFieldDidStartEditing(Escher::AbstractTextField * textField) {
  ElementsViewDataSource * dataSource = App::app()->elementsViewDataSource();
  dataSource->setTextFilter(m_view.bannerView()->textField()->text());
  if (ElementsDataBase::IsElement(dataSource->selectedElement())) {
    /* Changing the selected element will reload the banner. */
    dataSource->setSelectedElement(ElementsDataBase::k_noElement);
  } else {
    m_view.bannerView()->reload();
  }
}

bool MainController::textFieldShouldFinishEditing(Escher::AbstractTextField * textField, Ion::Events::Event event) {
  return event == Ion::Events::OK || event == Ion::Events::EXE;
}

bool MainController::textFieldDidReceiveEvent(Escher::AbstractTextField * textField, Ion::Events::Event event) {
  if (textField->isEditing()) {
    if (event == Ion::Events::Up || event == Ion::Events::Down) {
      ElementsViewDataSource * dataSource = App::app()->elementsViewDataSource();
      m_view.bannerView()->textField()->setSuggestion(dataSource->cycleSuggestion(event == Ion::Events::Down));
      return true;
    }
  } else if (event == Ion::Events::OK || event == Ion::Events::EXE) {
    /* OK should not start the edition */
    return handleEvent(event);
  }
  return false;
}

bool MainController::textFieldDidFinishEditing(Escher::AbstractTextField * textField, const char * text, Ion::Events::Event event) {
  ElementsViewDataSource * dataSource = App::app()->elementsViewDataSource();
  AtomicNumber match = dataSource->elementSearchResult();
  if (!ElementsDataBase::IsElement(match)) {
    match = 1;
  }
  dataSource->setSelectedElement(match);
  dataSource->setTextFilter(nullptr);
  return true;
}

bool MainController::textFieldDidAbortEditing(Escher::AbstractTextField * textField) {
  ElementsViewDataSource * dataSource = App::app()->elementsViewDataSource();
  dataSource->setTextFilter(nullptr);
  dataSource->setSelectedElement(dataSource->previousElement());
  return false;
}

bool MainController::textFieldDidHandleEvent(Escher::AbstractTextField * textField, bool returnValue, bool textSizeDidChange) {
  if (textSizeDidChange) {
    if (textField->isEditing()) {
      /* Update suggestion text */
      ElementsViewDataSource * dataSource = App::app()->elementsViewDataSource();
      m_view.bannerView()->textField()->setSuggestion(dataSource->suggestedElementName());
    }
    m_view.elementsView()->reload();
  }
  return returnValue;
}

}
