#include <escher/highlight_cell.h>

namespace Escher {

HighlightCell::HighlightCell() :
  View(),
  m_state(State::Visible)
{
}

void HighlightCell::setHighlighted(bool highlight) {
  if (m_state == State::Hidden) {
    // An hidden cell might be deselected but not selected
    assert(highlight == false);
    return;
  }
  if (isHighlighted() != highlight) {
    HighlightCell::setHighlightedWithoutReload(highlight);
    reloadCell();
  }
}

void HighlightCell::setHighlightedWithoutReload(bool highlight) {
  m_state = highlight ? State::Highlighted : State::Visible;
}

void HighlightCell::reloadCell() {
  markRectAsDirty(bounds());
}

}
