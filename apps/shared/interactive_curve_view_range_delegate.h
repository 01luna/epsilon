#ifndef SHARED_INTERACTIVE_CURVE_VIEW_DELEGATE_H
#define SHARED_INTERACTIVE_CURVE_VIEW_DELEGATE_H

#include <poincare/context.h>
#include <poincare/range.h>
#include <assert.h>
#include "function_store.h"

namespace Shared {

class InteractiveCurveViewRange;
class FunctionStore;

class InteractiveCurveViewRangeDelegate {
public:
  constexpr static float k_defaultXHalfRange = Poincare::Range1D::k_defaultHalfLength;

  static float DefaultAddMargin(float x, float range, bool isVertical, bool isMin, float top, float bottom, float left, float right);

  virtual float interestingXMin() const { return -k_defaultXHalfRange; }
  virtual bool defaultRangeIsNormalized() const { return false; }
  virtual Poincare::Range2D optimalRange(bool computeX, bool computeY, Poincare::Range1D intrinsicYRange) const { assert(false); }
  virtual float addMargin(float x, float range, bool isVertical, bool isMin) = 0;
  Poincare::Range2D addMargins(Poincare::Range2D range);
  virtual bool canShrinkWhenNormalizing() const { return false; }
  virtual void updateBottomMargin() = 0;
  virtual void updateZoomButtons() = 0;
  virtual void tidyModels() = 0;
};

}

#endif
