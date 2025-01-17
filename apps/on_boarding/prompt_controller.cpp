#include "prompt_controller.h"

#include <assert.h>

#include "../apps_container.h"

using namespace Escher;

namespace OnBoarding {

PromptController::MessageViewWithSkip::MessageViewWithSkip(
    const I18n::Message* messages, const KDColor* colors,
    uint8_t numberOfMessages)
    : MessageView(messages, colors, numberOfMessages),
      m_skipView(I18n::Message::Skip,
                 {.style = {.font = KDFont::Size::Small},
                  .horizontalAlignment = KDGlyph::k_alignRight}) {}

int PromptController::MessageViewWithSkip::numberOfSubviews() const {
  return MessageView::numberOfSubviews() + 2;
}

View* PromptController::MessageViewWithSkip::subviewAtIndex(int index) {
  uint8_t numberOfMainMessages = MessageView::numberOfSubviews();
  if (index < numberOfMainMessages) {
    return MessageView::subviewAtIndex(index);
  }
  if (index == numberOfMainMessages) {
    return &m_skipView;
  }
  if (index == numberOfMainMessages + 1) {
    return &m_okView;
  }
  assert(false);
  return nullptr;
}

void PromptController::MessageViewWithSkip::layoutSubviews(bool force) {
  // Layout the main message
  MessageView::layoutSubviews();
  // Layout the "skip (OK)"
  KDCoordinate height = bounds().height();
  KDCoordinate width = bounds().width();
  KDCoordinate textHeight = KDFont::GlyphHeight(KDFont::Size::Small);
  KDSize okSize = m_okView.minimalSizeForOptimalDisplay();
  setChildFrame(
      &m_skipView,
      KDRect(0, height - k_bottomMargin - textHeight,
             width - okSize.width() - k_okMargin - k_skipMargin, textHeight),
      force);
  setChildFrame(&m_okView,
                KDRect(width - okSize.width() - k_okMargin,
                       height - okSize.height() - k_okMargin, okSize),
                force);
}

PromptController::PromptController(const I18n::Message* messages,
                                   const KDColor* colors,
                                   uint8_t numberOfMessages)
    : ViewController(nullptr),
      m_messageViewWithSkip(messages, colors, numberOfMessages) {}

bool PromptController::handleEvent(Ion::Events::Event event) {
  if (event.isKeyPress() && event != Ion::Events::Back &&
      event != Ion::Events::OnOff) {
    Container::activeApp()->modalViewController()->dismissModal();
    AppsContainer* appsContainer = AppsContainer::sharedAppsContainer();
    if (appsContainer->activeApp()->snapshot() ==
        appsContainer->onBoardingAppSnapshot()) {
      appsContainer->switchToBuiltinApp(appsContainer->homeAppSnapshot());
    }
    return true;
  }
  return false;
}

}  // namespace OnBoarding
