#ifndef INFERENCE_STATISTIC_RESULTS_DATA_SOURCE_H
#define INFERENCE_STATISTIC_RESULTS_DATA_SOURCE_H

#include <escher/buffer_text_view.h>
#include <escher/button_cell.h>
#include <escher/invocation.h>
#include <escher/layout_view.h>
#include <escher/list_view_data_source.h>
#include <escher/menu_cell.h>
#include <escher/message_text_view.h>

#include "inference/models/statistic/statistic.h"
#include "inference/shared/dynamic_cells_data_source.h"

namespace Inference {

using ResultCell = Escher::MenuCell<Escher::LayoutView, Escher::MessageTextView,
                                    Escher::FloatBufferTextView<>>;
/* A ResultsDataSource is a TableViewDataSource which is meant to
 * represent data provided (and computed) by a Statistic.
 */
class ResultsDataSource
    : public Escher::StandardMemoizedListViewDataSource,
      public DynamicCellsDataSource<ResultCell, k_maxNumberOfResultCells> {
 public:
  ResultsDataSource(Escher::Responder* parent, Statistic* statistic,
                    Escher::Invocation invocation,
                    DynamicCellsDataSourceDelegate<ResultCell>*
                        dynamicCellsDataSourceDelegate);
  int numberOfRows() const override;
  KDCoordinate defaultColumnWidth() override;
  void willDisplayCellAtRow(Escher::HighlightCell* cell, int row) override;
  Escher::HighlightCell* reusableCell(int index, int type) override;
  int reusableCellCount(int type) override;
  int typeAtRow(int row) const override;
  KDCoordinate separatorBeforeRow(int index) override {
    return typeAtRow(index) == k_buttonCellType ? k_defaultRowSeparator : 0;
  }

  constexpr static int k_numberOfReusableCells = 5;

 private:
  constexpr static int k_resultCellType = 0;
  constexpr static int k_buttonCellType = 1;
  Statistic* m_statistic;
  Escher::ButtonCell m_next;
};

}  // namespace Inference

#endif
