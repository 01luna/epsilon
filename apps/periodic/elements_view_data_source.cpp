#include "elements_view_data_source.h"
#include <assert.h>

namespace Periodic {

void ElementsViewDataSource::setSelectedElement(AtomicNumber z) {
  if (z != m_selectedElement) {
    assert(m_delegate);
    m_previousElement = m_selectedElement;
    m_selectedElement = z;
    m_delegate->selectedElementHasChanged();
  }
}

void ElementsViewDataSource::setField(const DataField * field) {
  if (field != m_field) {
    m_field = field;
    m_delegate->activeDataFieldHasChanged();
  }
}

}
