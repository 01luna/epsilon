#ifndef APPS_SEQUENCE_VERTICAL_SEQUENCE_TITLE_CELL_H
#define APPS_SEQUENCE_VERTICAL_SEQUENCE_TITLE_CELL_H

#include "../sequence_title_cell.h"

namespace Sequence {

class VerticalSequenceTitleCell : public SequenceTitleCell {
public:
  constexpr static KDFont::Size k_font = KDFont::Size::Large;

  VerticalSequenceTitleCell();
  void drawRect(KDContext * ctx, KDRect rect) const override;

private:
  constexpr static KDCoordinate k_equalWidthWithMargins = 10;  // Ad hoc value
  constexpr static float k_horizontalAlignment = 0.9f;

  KDRect subviewFrame() const override;
  void layoutSubviews(bool force = false) override;
  float verticalAlignment() const;
};

}  // namespace Sequence

#endif
