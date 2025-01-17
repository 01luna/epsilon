#include <assert.h>
#include <escher/clipboard.h>
#include <escher/container.h>
#include <escher/metric.h>
#include <escher/selectable_table_view.h>
#include <string.h>

namespace Escher {

SelectableTableView::SelectableTableView(
    Responder* parentResponder, TableViewDataSource* dataSource,
    SelectableTableViewDataSource* selectionDataSource,
    SelectableTableViewDelegate* delegate)
    : TableView(dataSource, selectionDataSource),
      Responder(parentResponder),
      m_selectionDataSource(selectionDataSource),
      m_delegate(delegate) {
  assert(m_selectionDataSource != nullptr);
  setMargins(Metric::CommonTopMargin, Metric::CommonRightMargin,
             Metric::CommonBottomMargin, Metric::CommonLeftMargin);
}

HighlightCell* SelectableTableView::selectedCell() {
  return cellAtLocation(selectedColumn(), selectedRow());
}

int SelectableTableView::firstOrLastSelectableColumnOrRow(bool first,
                                                          bool searchForRow) {
  int nColumnsOrRow =
      searchForRow ? totalNumberOfRows() : totalNumberOfColumns();
  if (nColumnsOrRow == 0) {
    return 0;
  }
  int firstIndex = first ? 0 : nColumnsOrRow - 1;
  for (int cow = firstIndex; first ? cow < nColumnsOrRow : cow >= 0;
       first ? cow++ : cow--) {
    bool isSelectable = searchForRow
                            ? cellAtLocationIsSelectable(selectedColumn(), cow)
                            : cellAtLocationIsSelectable(cow, selectedRow());
    if (isSelectable) {
      return cow;
    }
  }
  assert(false);
  return -1;
}

int SelectableTableView::indexOfNextSelectableColumnOrRow(int delta, int col,
                                                          int row,
                                                          bool searchForRow) {
  assert((searchForRow && col < totalNumberOfColumns() && col >= 0) ||
         (!searchForRow && row < totalNumberOfRows() && row >= 0));
  assert(delta != 0);
  // Let's call our variable cow, as a shortcut for col-or-row
  int cow = searchForRow ? row : col;
  int selectableCow = -1;
  int step = delta > 0 ? 1 : -1;
  const int lastCow =
      (searchForRow ? totalNumberOfRows() : totalNumberOfColumns()) - 1;
  while (delta) {
    cow += step;
    if (cow < 0 || cow > lastCow) {
      if (selectableCow >= 0) {
        return selectableCow;
      }
      return firstOrLastSelectableColumnOrRow(delta < 0, searchForRow);
    }
    bool cellIsSelectable = searchForRow ? cellAtLocationIsSelectable(col, cow)
                                         : cellAtLocationIsSelectable(cow, row);
    if (cellIsSelectable) {
      selectableCow = cow;
      delta -= step;
    }
  }
  return selectableCow;
}

bool SelectableTableView::selectCellAtLocation(int col, int row,
                                               bool setFirstResponder,
                                               bool withinTemporarySelection) {
  if (row < 0 || col < 0 || row >= totalNumberOfRows() ||
      col >= totalNumberOfColumns()) {
    return false;
  }

  if (!cellAtLocationIsSelectable(col, row)) {
    /* If the cell is not selectable, go down by default.
     * This behaviour is only implemented for Explicit. */
    row = indexOfNextSelectableRow(1, col, row);
  }
  // There should always be at least 1 selectable cell in the column
  assert(cellAtLocationIsSelectable(col, row));

  // Unhighlight previous cell
  unhighlightSelectedCell();

  int previousColumn = selectedColumn();
  int previousRow = selectedRow();
  KDPoint previousOffset = contentOffset();

  // Selection
  selectColumn(col);
  selectRow(row);

  // Scroll
  if (selectedRow() >= 0) {
    scrollToCell(selectedColumn(), selectedRow());
  }

  if (m_delegate) {
    m_delegate->tableViewDidChangeSelectionAndDidScroll(
        this, previousColumn, previousRow, previousOffset,
        withinTemporarySelection);
  }

  HighlightCell* cell = selectedCell();
  if (cell) {
    // Update first responder
    Responder* r = cell->responder() ? cell->responder() : this;
    if (setFirstResponder &&
        /* Sometimes reusable cells must be re-set as first responder if the row
           changed. Other times, the row did not change but the responder did
           (when going back in previous menu for example). */
        ((selectedColumn() != previousColumn || selectedRow() != previousRow) ||
         Container::activeApp()->firstResponder() != r)) {
      Container::activeApp()->setFirstResponder(r, true);
    }
    // Highlight new cell
    cell->setHighlighted(true);
  }

  return true;
}

bool SelectableTableView::selectCellAtClippedLocation(
    int col, int row, bool setFirstResponder, bool withinTemporarySelection) {
  col = std::clamp(col, 0, totalNumberOfColumns() - 1);
  row = std::clamp(row, 0, totalNumberOfRows() - 1);
  if (row == selectedRow() && col == selectedColumn()) {
    // Cell was already selected.
    return false;
  }
  return selectCellAtLocation(col, row, setFirstResponder,
                              withinTemporarySelection);
}

bool SelectableTableView::handleEvent(Ion::Events::Event event) {
  assert(totalNumberOfRows() > 0);
  int step = Ion::Events::longPressFactor();
  int col = selectedColumn();
  int row = selectedRow();
  if (event == Ion::Events::Down) {
    return selectCellAtClippedLocation(
        col, indexOfNextSelectableRow(step, col, row));
  }
  if (event == Ion::Events::Up) {
    return selectCellAtClippedLocation(
        col, indexOfNextSelectableRow(-step, col, row));
  }
  if (event == Ion::Events::Left) {
    return selectCellAtClippedLocation(
        indexOfNextSelectableColumn(-step, col, row), row);
  }
  if (event == Ion::Events::Right) {
    return selectCellAtClippedLocation(
        indexOfNextSelectableColumn(step, col, row), row);
  }
  if (event == Ion::Events::Copy || event == Ion::Events::Cut ||
      event == Ion::Events::Sto || event == Ion::Events::Var) {
    HighlightCell* cell = selectedCell();
    if (!cell) {
      return false;
    }
    constexpr int bufferSize = TextField::MaxBufferSize();
    char buffer[bufferSize] = "";
    // Step 1: Determine text to store
    const char* text = cell->text();
    Poincare::Layout layout = cell->layout();
    if (!text && layout.isUninitialized()) {
      return false;
    }
    if (!m_delegate || m_delegate->canStoreContentOfCellAtLocation(
                           this, selectedColumn(), selectedRow())) {
      if (text) {
        strlcpy(buffer, text, bufferSize);
      } else {
        assert(!layout.isUninitialized());
        if (layout.serializeParsedExpression(
                buffer, bufferSize,
                m_delegate ? m_delegate->context() : nullptr) ==
            bufferSize - 1) {
          /* The layout is too large to be serialized in the buffer. Returning
           * false will open an empty store which is better than a broken
           * text. */
          return false;
        };
      }
    }
    // Step 2: Determine where to store it
    if (event == Ion::Events::Copy || event == Ion::Events::Cut) {
      if (buffer[0] != 0) {
        // We don't want to store an empty text in clipboard
        Escher::Clipboard::SharedClipboard()->store(buffer);
      }
    } else {
      Container::activeApp()->storeValue(buffer);
    }
    return true;
  }
  return false;
}

void SelectableTableView::unhighlightSelectedCell() {
  HighlightCell* cell = selectedCell();
  if (cell) {
    cell->setHighlighted(false);
  }
}

void SelectableTableView::deselectTable(bool withinTemporarySelection) {
  unhighlightSelectedCell();
  int previousSelectedCol = selectedColumn();
  int previousSelectedRow = selectedRow();
  KDPoint previousOffset = contentOffset();
  selectColumn(0);
  selectRow(-1);
  if (m_delegate) {
    m_delegate->tableViewDidChangeSelectionAndDidScroll(
        this, previousSelectedCol, previousSelectedRow, previousOffset,
        withinTemporarySelection);
  }
}

void SelectableTableView::reloadData(bool setFirstResponder) {
  int col = selectedColumn();
  int row = selectedRow();
  KDPoint previousOffset = contentOffset();
  deselectTable(true);
  SelectableTableView::layoutSubviews();
  setContentOffset(previousOffset);
  selectCellAtLocation(col, row, setFirstResponder, true);
}

void SelectableTableView::didBecomeFirstResponder() {
  HighlightCell* cell = selectedCell();
  if (cell && cell->responder()) {
    // Update first responder
    Container::activeApp()->setFirstResponder(cell->responder());
  }
}

void SelectableTableView::didEnterResponderChain(
    Responder* previousFirstResponder) {
  int col = selectedColumn();
  int row = selectedRow();
  selectColumn(0);
  selectRow(-1);
  selectCellAtLocation(col, row, false);
}

void SelectableTableView::willExitResponderChain(
    Responder* nextFirstResponder) {
  if (nextFirstResponder != nullptr) {
    unhighlightSelectedCell();
  }
}

void SelectableTableView::layoutSubviews(bool force) {
  TableView::layoutSubviews(force);
  /* The highlighting/unhighlighting of cells might have failed when
   * the table was reloaded because the reusable cells were not populated.
   * To prevent this, their Highlight status is reloaded here. */
  int nReusableCells = numberOfDisplayableCells();
  HighlightCell* thisSelectedCell = selectedCell();
  for (int cellIndex = 0; cellIndex < nReusableCells; cellIndex++) {
    HighlightCell* cell = reusableCellAtIndex(cellIndex);
    if (cell && cell->isVisible()) {
      cell->setHighlighted(thisSelectedCell == cell);
    }
  }
}

}  // namespace Escher
