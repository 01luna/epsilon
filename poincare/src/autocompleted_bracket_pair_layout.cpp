#include <poincare/autocompleted_bracket_pair_layout.h>
#include <poincare/empty_layout.h>
#include <poincare/horizontal_layout.h>
#include <poincare/layout.h>

namespace Poincare {

void AutocompletedBracketPairLayoutNode::deleteBeforeCursor(LayoutCursor * cursor) {
  Side deletionSide;
  if (cursor->isEquivalentTo(LayoutCursor(this, LayoutCursor::Position::Right))) {
    deletionSide = Side::Right;
  } else if (cursor->isEquivalentTo(LayoutCursor(childLayout(), LayoutCursor::Position::Left))) {
    deletionSide = Side::Left;
  } else {
    return BracketPairLayoutNode::deleteBeforeCursor(cursor);
  }
  if (childLayout()->isEmpty() && isTemporary(OtherSide(deletionSide))) {
    return BracketPairLayoutNode::deleteBeforeCursor(cursor);
  }

  LayoutCursor nextCursor = cursorAfterDeletion(deletionSide);
  makeTemporary(deletionSide, cursor);
  cursor->setTo(&nextCursor);
}

void AutocompletedBracketPairLayoutNode::balanceAfterInsertion(Side insertedSide, LayoutCursor * cursor) {
  assert(cursor);
  Layout thisRef(this);
  makeTemporary(OtherSide(insertedSide), cursor);
  if (insertedSide == Side::Left) {
    cursor->setPosition(LayoutCursor::Position::Left);
    cursor->setLayout(thisRef.childAtIndex(0));
  } else {
    cursor->setPosition(LayoutCursor::Position::Right);
    cursor->setLayout(thisRef);
  }
}

static bool isAutocompletedBracket(Layout l) { return l.type() == LayoutNode::Type::ParenthesisLayout || l.type() == LayoutNode::Type::CurlyBraceLayout; }

static AutocompletedBracketPairLayoutNode * autocompletedParent(Layout l) {
  Layout p = l.parent();
  while (!p.isUninitialized()) {
    if (isAutocompletedBracket(p)) {
      return static_cast<AutocompletedBracketPairLayoutNode *>(p.node());
    }
    if (p.type() != LayoutNode::Type::HorizontalLayout) {
      break;
    }
    p = p.parent();
  }
  return nullptr;
}

bool AutocompletedBracketPairLayoutNode::makeTemporary(Side side, LayoutCursor * cursor) {
  if (isTemporary(side)) {
    return false;
  }
  Layout thisRef(this);
  absorbSiblings(side, cursor);
  AutocompletedBracketPairLayoutNode * p = autocompletedParent(thisRef);
  if (!(p && p->makeTemporary(side, cursor))) {
    /* 'this' was the topmost pair without a temporary bracket on this side. It
     * may be invalid after the call to absorbSiblings. */
    AutocompletedBracketPairLayoutNode * newThis = static_cast<AutocompletedBracketPairLayoutNode *>(thisRef.node());
    newThis->m_status |= MaskForSide(side);
    newThis->removeIfCompletelyTemporary(cursor);
  }
  return true;
}

void AutocompletedBracketPairLayoutNode::removeIfCompletelyTemporary(LayoutCursor * cursor) {
  if (!(isTemporary(Side::Left) && isTemporary(Side::Right))) {
    return;
  }
  assert(parent());
  Layout thisRef = Layout(this);
  Layout childRef = thisRef.childAtIndex(0);
  Layout parentRef = thisRef.parent();
  parentRef.replaceChild(thisRef, childRef, cursor);
}

void AutocompletedBracketPairLayoutNode::absorbSiblings(Side side, LayoutCursor * cursor) {
  Layout thisRef = Layout(this);
  Layout p = parent();
  assert(!p.isUninitialized());
  if (p.type() != LayoutNode::Type::HorizontalLayout) {
    return;
  }
  HorizontalLayout h = static_cast<HorizontalLayout &>(p);
  int thisIndex = h.indexOfChild(thisRef);

  HorizontalLayout newChild = HorizontalLayout::Builder();
  Layout oldChild = Layout(childLayout());
  thisRef.replaceChild(oldChild, newChild, cursor);
  newChild.addOrMergeChildAtIndex(oldChild, 0, true, cursor);

  int injectionIndex, removalStart, removalEnd;
  if (side == Side::Left) {
    injectionIndex = 0;
    removalStart = thisIndex - 1;
    removalEnd = 0;
  } else {
    injectionIndex = newChild.isEmpty() ? 0 : newChild.numberOfChildren();
    removalStart = h.numberOfChildren() - 1;
    removalEnd = thisIndex + 1;
  }
  for (int i = removalStart; i >= removalEnd; i--) {
    Layout l = h.childAtIndex(i);
    h.removeChild(l, cursor);
    newChild.addOrMergeChildAtIndex(l, injectionIndex, true, cursor);
  }
  if (newChild.numberOfChildren() == 0) {
    thisRef.replaceChild(newChild, EmptyLayout::Builder(EmptyLayoutNode::Color::Yellow, EmptyLayoutNode::Visibility::Never));
  }
}

static Layout childBypassHorizontalLayout(Layout child, int index) {
  if (child.type() == LayoutNode::Type::HorizontalLayout) {
    assert(child.numberOfChildren() > 0);
    return child.childAtIndex(index);
  }
  return child;
}

LayoutCursor AutocompletedBracketPairLayoutNode::cursorAfterDeletion(Side side) const {
  Layout thisRef(this);
  Layout childRef(childLayout());
  Layout parentRef = thisRef.parent();
  assert(!parentRef.isUninitialized());
  int thisIndex = parentRef.indexOfChild(thisRef);
  bool willDisappear = isTemporary(OtherSide(side));

  if (side == Side::Left) {
    if (thisIndex > 0) {
      return LayoutCursor(parentRef.childAtIndex(thisIndex - 1), LayoutCursor::Position::Right);
    }
    if (willDisappear) {
      assert(!childRef.isEmpty());
      return LayoutCursor(childBypassHorizontalLayout(childRef, 0), LayoutCursor::Position::Left);
    }
    return LayoutCursor(thisRef, LayoutCursor::Position::Left);
  }

  assert(side == Side::Right);
  if (!childRef.isEmpty()) {
    return LayoutCursor(childBypassHorizontalLayout(childRef, childRef.numberOfChildren() - 1), LayoutCursor::Position::Right);
  }
  assert(!willDisappear);
  if (thisIndex < parentRef.numberOfChildren() - 1) {
    return LayoutCursor(parentRef.childAtIndex(thisIndex + 1), LayoutCursor::Position::Left);
  }
  return LayoutCursor(childRef, LayoutCursor::Position::Left);
}

}
