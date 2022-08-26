#ifndef PERIODIC_MAIN_CONTROLLER_H
#define PERIODIC_MAIN_CONTROLLER_H

#include "banner_view.h"
#include "details_list_controller.h"
#include "display_type_controller.h"
#include "elements_view.h"
#include "elements_view_delegate.h"
#include <escher/stack_view_controller.h>

namespace Periodic {

class MainController : public Escher::ViewController, public ElementsViewDelegate, public Escher::TextFieldDelegate {
public:
  MainController(Escher::StackViewController * parentResponder);

  // Escher::ViewController
  Escher::View * view() override { return &m_view; }

  // Escher::Responder
  void didBecomeFirstResponder() override { Escher::Container::activeApp()->setFirstResponder(m_view.bannerView()->textField()); }
  bool handleEvent(Ion::Events::Event e) override;

  // ElementsViewDelegate
  void selectedElementHasChanged() override;
  void activeDataFieldHasChanged() override;

  // Escher::TextFieldDelegate
  bool textFieldShouldFinishEditing(Escher::TextField * textField, Ion::Events::Event event) override;
  void textFieldDidStartEditing(Escher::TextField * textField) override;
  bool textFieldDidReceiveEvent(Escher::TextField * textField, Ion::Events::Event event) override;
  bool textFieldDidFinishEditing(Escher::TextField * textField, const char * text, Ion::Events::Event event) override;
  bool textFieldDidAbortEditing(Escher::TextField * textField) override;
  bool textFieldDidHandleEvent(Escher::TextField * textField, bool returnValue, bool textSizeDidChange) override;

private:
  class ContentView : public Escher::View {
  public:
    ContentView(MainController * mainController) : m_bannerView(mainController, mainController) {}

    ElementsView * elementsView() { return &m_elementsView; }
    BannerView * bannerView() { return &m_bannerView; }

  private:
    int numberOfSubviews() const override { return 2; }
    Escher::View * subviewAtIndex(int index) override;
    void layoutSubviews(bool force = false) override;

    ElementsView m_elementsView;
    BannerView m_bannerView;
  };

  DetailsListController m_detailsController;
  DisplayTypeController m_displayTypeController;
  ContentView m_view;
};

}

#endif

