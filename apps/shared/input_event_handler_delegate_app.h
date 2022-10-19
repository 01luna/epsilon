#ifndef SHARED_INPUT_EVENT_HANDLER_DELEGATE_APP_H
#define SHARED_INPUT_EVENT_HANDLER_DELEGATE_APP_H

#include "store_menu_controller.h"
#include <escher/app.h>
#include <escher/input_event_handler_delegate.h>

namespace Shared {

class InputEventHandlerDelegateApp : public Escher::App, public Escher::InputEventHandlerDelegate {
public:
  virtual ~InputEventHandlerDelegateApp() = default;
  Escher::Toolbox * toolbox() override;
  Escher::NestedMenuController * variableBox() override;
  bool handleEvent(Ion::Events::Event event) override;
  void storeValue(const char * text) override;
  bool isStoreMenuOpen();
protected:
  InputEventHandlerDelegateApp(Snapshot * snapshot, Escher::ViewController * rootViewController);

private:
  StoreMenuController m_storeController;

};

}

#endif
