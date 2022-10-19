#include "input_event_handler_delegate_app.h"
#include "../apps_container.h"
#include <cmath>
#include <escher/clipboard.h>
#include <string.h>

using namespace Escher;
using namespace Poincare;

namespace Shared {

InputEventHandlerDelegateApp::InputEventHandlerDelegateApp(Snapshot * snapshot, ViewController * rootViewController) :
  ::App(snapshot, rootViewController, I18n::Message::Warning),
  InputEventHandlerDelegate()
{
}

Toolbox * InputEventHandlerDelegateApp::toolboxForInputEventHandler(InputEventHandler * textInput) {
  Toolbox * toolbox = AppsContainer::sharedAppsContainer()->mathToolbox();
  toolbox->setSender(textInput);
  return toolbox;
}

NestedMenuController * InputEventHandlerDelegateApp::variableBoxForInputEventHandler(InputEventHandler * textInput) {
  MathVariableBoxController * varBox = AppsContainer::sharedAppsContainer()->variableBoxController();
  varBox->setSender(textInput);
  varBox->lockDeleteEvent(MathVariableBoxController::Page::RootMenu);
  return varBox;
}

bool InputEventHandlerDelegateApp::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Sto) {
    storeValue("");
  }
  return App::handleEvent(event);
}

void InputEventHandlerDelegateApp::storeValue(const char * text) {
  if (m_modalViewController.isDisplayingModal()) {
    return;
  }
  m_storeController.setText(text);
  displayModalViewController(&m_storeController, 0.f, 0.f, Metric::PopUpTopMargin, Metric::PopUpLeftMargin, 0, Metric::PopUpRightMargin);
}

}
