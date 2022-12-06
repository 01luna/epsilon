#ifndef ESCHER_STACK_VIEW_CONTROLLER_H
#define ESCHER_STACK_VIEW_CONTROLLER_H

#include <escher/view_controller.h>
#include <escher/stack_view_header.h>
#include <escher/palette.h>
#include <escher/solid_color_view.h>
#include <ion/ring_buffer.h>
#include <stdint.h>

namespace Escher {

constexpr uint8_t k_MaxNumberOfStacks = 7;
static_assert(k_MaxNumberOfStacks < 8, "Bit mask representation relies on less than 8 stacks (uint8_t).");

class StackViewController : public ViewController {
  class ControllerView;
public:
  enum class Style {
    GrayGradation,
    PurpleWhite,
    WhiteUniform
  };
  StackViewController(Responder * parentResponder, ViewController * rootViewController, Style style, bool extendVertically = true);

  /* Push creates a new StackView and adds it */
  void push(ViewController * vc);
  void pop();
  void popUntilDepth(int depth, bool shouldSetupTopViewController);

  int depth() const { return m_numberOfChildren; }
  View * view() override { return &m_view; }
  ControllerView * controllerView() { return &m_view; }
  ViewController * topViewController();
  const char * title() override;
  bool handleEvent(Ion::Events::Event event) override;
  void didEnterResponderChain(Responder * previousFirstResponder) override;
  void didBecomeFirstResponder() override;
  void initView() override;
  void viewWillAppear() override;
  void viewDidDisappear() override;
  void setupActiveView();
  void setupHeadersBorderOverlaping(bool headersOverlapHeaders = true, bool headersOverlapContent = false, KDColor headersContentBorderColor = Palette::GrayBright) {
    m_view.setupHeadersBorderOverlaping(headersOverlapHeaders, headersOverlapContent, headersContentBorderColor);
  }
  constexpr static uint8_t k_maxNumberOfChildren = k_MaxNumberOfStacks;
private:
  class ControllerView : public View {
    friend StackViewController;
  public:
    ControllerView(Style style, bool extendVertically);
    int8_t numberOfStacks() const { return m_stackViewHeaders.length(); }
    void setContentView(View * view);
    void setupHeadersBorderOverlaping(bool headersOverlapHeaders, bool headersOverlapContent, KDColor headersContentBorderColor);
    void pushStack(ViewController * controller);
    void reload() { markRectAsDirty(bounds()); }
    void resetStack() { m_stackViewHeaders.reset(); }
  protected:
#if ESCHER_VIEW_LOGGING
  const char * className() const override;
#endif
  private:
    KDSize minimalSizeForOptimalDisplay() const override;
    int numberOfSubviews() const override;
    View * subviewAtIndex(int index) override;
    void layoutSubviews(bool force = false) override;
    bool borderShouldOverlapContent() const;

    int numberOfDisplayedHeaders() const;
    // Returns the index in m_stackViews for a given display index
    int stackHeaderIndex(int displayIndex);

    Ion::RingBuffer<StackViewHeader, k_MaxNumberOfStacks> m_stackViewHeaders;
    SolidColorView m_borderView;
    View * m_contentView;
    Style m_style;
    // TODO: enum class? Some combination can't exist?
    bool m_headersOverlapHeaders;
    bool m_headersOverlapContent;
    bool m_extendVertically;
  };
  ControllerView m_view;
  void pushModel(ViewController * vc);
  void setupActiveViewController();
  bool shouldStoreHeaderOnStack(ViewController * vc, int index);
  void updateStack(ViewController::TitlesDisplay titleDisplay);
  void dismissPotentialModal();
  virtual void didExitPage(ViewController * controller) const;
  virtual void willOpenPage(ViewController * controller) const;
  ViewController * m_childrenController[k_maxNumberOfChildren];
  uint8_t m_numberOfChildren;
  bool m_isVisible;
  bool m_displayedAsModal;
  /* Represents the stacks to display, _starting from the end_.
   * m_headersDisplayMask = 0b11111011   ->  shouldn't display m_stackViews[numberOfStacks - 1 - 2]. */
  uint8_t m_headersDisplayMask;
};

}
#endif
