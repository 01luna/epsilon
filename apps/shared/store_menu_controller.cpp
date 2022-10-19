#include "store_menu_controller.h"
#include <escher/clipboard.h>
#include <escher/invocation.h>
#include <poincare/store.h>
#include "poincare_helpers.h"
#include "text_field_delegate_app.h"

using namespace Poincare;
using namespace Shared;
using namespace Ion;
using namespace Escher;

StoreMenuController::InnerListController::InnerListController(StoreMenuController * dataSource, SelectableTableViewDelegate * delegate) :
  ViewController(dataSource),
  m_selectableTableView(this, dataSource, dataSource, delegate)
{
  m_selectableTableView.setMargins(0);
  m_selectableTableView.setDecoratorType(ScrollView::Decorator::Type::None);
}

void StoreMenuController::open() {
  Container::activeApp()->displayModalViewController(this, KDContext::k_alignCenter, KDContext::k_alignCenter, 0, Metric::PopUpLeftMargin, 0, Metric::PopUpRightMargin);
}

void StoreMenuController::InnerListController::didBecomeFirstResponder() {
  m_selectableTableView.selectCellAtLocation(0, 0);
  m_selectableTableView.reloadData();
}

StoreMenuController::StoreMenuController() :
  ModalViewController(this, &m_stackViewController),
  m_stackViewController(nullptr, &m_listController, StackViewController::Style::PurpleWhite),
  m_listController(this),
  m_cell(this, nullptr, this, this),
  m_abortController(
    Invocation([](void * context, void * sender) {
      StoreMenuController * storeMenu = static_cast<StoreMenuController*>(context);
      storeMenu->dismissModalViewController();
      Container::activeApp()->dismissModalViewController();
      return true;
    }, this),
    Invocation([](void * context, void * sender) {
      StoreMenuController * storeMenu = static_cast<StoreMenuController*>(context);
      storeMenu->dismissModalViewController();
      return true;
    }, this),
    I18n::Message::Warning, I18n::Message::Ok, I18n::Message::Cancel
    ),
  m_preventReload(false)
{
  m_abortController.setContentMessage(I18n::Message::InvalidInputWarning);
  /* We need to set the width early since minimalSizeForOptimalDisplay will be
   * called before willDisplayCell. */
  m_cell.setFrame(KDRect(0, 0, Ion::Display::Width - Metric::PopUpLeftMargin - Metric::PopUpRightMargin, 0), false);
}

void StoreMenuController::didBecomeFirstResponder() {
  Container::activeApp()->setFirstResponder(&m_listController);
}

void StoreMenuController::setText(const char * text) {
  m_preventReload = true;
  m_cell.expressionField()->setEditing(true);
  m_cell.expressionField()->setText(text);
  m_cell.expressionField()->handleEventWithText("→");
  if (text[0] == 0) {
    m_cell.expressionField()->putCursorLeftOfField();
  }
  m_stackViewController.setupActiveView();
  m_preventReload = false;
}

void StoreMenuController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  m_cell.reloadCell();
}

void StoreMenuController::layoutFieldDidChangeSize(LayoutField * layoutField) {
  if (!m_preventReload) {
    m_preventReload = true;
    Container::activeApp()->modalViewController()->reloadModalViewController();
  }
  m_preventReload = false;
}

void StoreMenuController::openAbortWarning() {
  /* We want to open the warning but the current store menu is likely too small
   * to display it entirely. We open the pop-up and then reload which will cause
   * the store menu to be relayouted with the minimalsizeForOptimalDisplay of
   * the warning. We could save a reload by moving the centering logic after the
   * embedded pop-up. */
  displayModalViewController(&m_abortController, KDContext::k_alignCenter, KDContext::k_alignCenter);
  Container::activeApp()->modalViewController()->reloadModalViewController();
}

bool StoreMenuController::layoutFieldDidFinishEditing(Escher::LayoutField * layoutField, Poincare::Layout layoutR, Ion::Events::Event event) {
  constexpr size_t bufferSize = TextField::maxBufferSize();
  char buffer[bufferSize];
  layoutR.serializeForParsing(buffer, bufferSize);
  Expression exp = Expression::Parse(buffer, Container::activeApp()->localContext());
  m_preventReload = true;
  if (exp.isUninitialized()) {
    openAbortWarning();
    return false;
  }
  Expression reducedExp = PoincareHelpers::ParseAndSimplify(buffer, Container::activeApp()->localContext());
  if (reducedExp.type() != ExpressionNode::Type::Store) {
    openAbortWarning();
    return false;
  }
  Store store = static_cast<Store&>(reducedExp);
  store.storeValueForSymbol(Container::activeApp()->localContext());
  Container::activeApp()->dismissModalViewController();
  return true;
}

bool StoreMenuController::textFieldDidFinishEditing(Escher::AbstractTextField * textField, const char * text, Ion::Events::Event event) {
  Expression exp = Expression::Parse(text, Container::activeApp()->localContext());
  m_preventReload = true;
  if (exp.isUninitialized()) {
    openAbortWarning();
    return false;
  }
  PoincareHelpers::ParseAndSimplify(text, Container::activeApp()->localContext());
  Container::activeApp()->dismissModalViewController();
  return true;
}

bool StoreMenuController::layoutFieldDidAbortEditing(Escher::LayoutField * layoutField) {
  /* Since dismissing the controller will call layoutFieldDidChangeSize, we need
   * to set the flag to avoid reloadData from happening which would otherwise
   * setFirstResponder on the store menu while it is hidden. */
  m_preventReload = true;
  Container::activeApp()->dismissModalViewController();
  return true;
}

bool StoreMenuController::textFieldDidAbortEditing(Escher::AbstractTextField * textField) {
  Container::activeApp()->dismissModalViewController();
  return true;
}

bool StoreMenuController::layoutFieldDidReceiveEvent(Escher::LayoutField * layoutField, Ion::Events::Event event) {
  if (event == Ion::Events::Sto) {
    layoutField->handleEventWithText("→");
    return true;
  }
  // We short circuit the LayoutFieldDelegate to avoid calls to displayWarning
  return textFieldDelegateApp()->fieldDidReceiveEvent(layoutField, layoutField, event);
}

bool StoreMenuController::textFieldDidReceiveEvent(AbstractTextField * textField, Ion::Events::Event event) {
  if (event == Ion::Events::Sto) {
    textField->handleEventWithText("→");
    return true;
  }
  return TextFieldDelegate::textFieldDidReceiveEvent(textField, event);
}
