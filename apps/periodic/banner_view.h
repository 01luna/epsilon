#ifndef PERIODIC_BANNER_VIEW_H
#define PERIODIC_BANNER_VIEW_H

#include "elements_view_data_source.h"
#include <escher/buffer_text_view.h>
#include <escher/ellipsis_view.h>
#include <escher/palette.h>
#include <escher/solid_color_view.h>
#include <escher/text_field.h>
#include <escher/view.h>
#include <ion/display.h>

namespace Periodic {

class BannerView : public Escher::View {
public:
  BannerView(Escher::Responder * textFieldParent, Escher::TextFieldDelegate * textFieldDelegate);

  // Escher::View
  void drawRect(KDContext * ctx, KDRect rect) const override;
  KDSize minimalSizeForOptimalDisplay() const override { return KDSize(Ion::Display::Width, k_bannerHeight + k_borderHeight); }

  void reload();
  Escher::TextField * textField() { return &m_textField; }

private:
  constexpr static KDColor k_legendColor = Escher::Palette::GrayVeryDark;
  constexpr static KDColor k_backgroundColor = Palette::SystemGrayLight;
  constexpr static KDCoordinate k_dotLeftMargin = 16;
  constexpr static KDCoordinate k_dotDiameter = 8;
  constexpr static KDCoordinate k_bannerHeight = 25;
  constexpr static KDCoordinate k_borderHeight = 1;
  constexpr static KDCoordinate k_dotLegendMargin = 12;
  constexpr static KDCoordinate k_buttonWidth = 37;

  class DotView : public Escher::View {
  public:
    DotView() : m_color(k_backgroundColor) {}

    // Escher::View
    void drawRect(KDContext * ctx, KDRect rect) const override;
    KDSize minimalSizeForOptimalDisplay() const override { return KDSize(k_dotDiameter, k_dotDiameter); }

    void setColor(KDColor color);

  private:
    KDColor m_color;
  };

  class EllipsisButton : public Escher::SolidColorView {
  public:
    using Escher::SolidColorView::SolidColorView;
  private:
    int numberOfSubviews() const override { return 1; }
    Escher::View * subviewAtIndex(int index) override { return &m_ellipsisView; }
    void layoutSubviews(bool force = false) override { m_ellipsisView.setFrame(bounds(), force); }
    Escher::EllipsisView m_ellipsisView;
  };

  int numberOfSubviews() const override { return displayTextField() ? 1 : 3; }
  Escher::View * subviewAtIndex(int index) override;
  void layoutSubviews(bool force = false) override;
  bool displayTextField() const { return m_textField.isEditing(); }

  Escher::TextField m_textField;
  DotView m_dotView;
  Escher::BufferTextView m_textView;
  EllipsisButton m_button;
};

}

#endif
