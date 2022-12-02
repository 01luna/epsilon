#ifndef GRAPH_INTEREST_VIEW_H
#define GRAPH_INTEREST_VIEW_H

#include <escher/view.h>
#include <apps/shared/dots.h>
#include <poincare/solver.h>

namespace Graph {

class GraphView;

class InterestView : public Escher::View {
public:
  constexpr static Shared::Dots::Size k_dotsSize = Shared::Dots::Size::Tiny;
  InterestView(GraphView * parentView) : Escher::View(), m_interest(Poincare::Solver<double>::Interest::None), m_parentView(parentView), m_nextPointIndex(0), m_computePoints(false) {}
  void drawRect(KDContext * ctx, KDRect rect) const override;
  void setInterest(Poincare::Solver<double>::Interest interest) { m_interest = interest; }
  void resetPointIndex() { m_nextPointIndex = 0; }
  /* This is called when the user is idle */
  void resumePointsOfInterestDrawing();
private:
  Poincare::Solver<double>::Interest m_interest;
  GraphView * m_parentView;
  mutable int m_nextPointIndex;
  mutable bool m_computePoints;
};

}

#endif