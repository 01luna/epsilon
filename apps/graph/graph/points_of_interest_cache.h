#ifndef GRAPH_POINTS_OF_INTEREST_CACHE
#define GRAPH_POINTS_OF_INTEREST_CACHE

#include <poincare/point_of_interest.h>
#include <poincare/range.h>
#include <ion/storage/record.h>

namespace Graph {

class PointsOfInterestCache {
public:
  PointsOfInterestCache() : m_start(NAN), m_end(NAN) {}

  bool isUpToDate() const { return m_record.checksum() == m_checksum; }
  void setRecord(Ion::Storage::Record record) { m_record = record; }
  Poincare::Range1D setBoundsAndCompute(float start, float end);
  Poincare::PointOfInterest<double> pointAtIndex(int i) const { return m_list.pointAtIndex(i); }
  Poincare::PointOfInterest<double> firstPointInDirection(double start, double end, Poincare::Solver<double>::Interest interest = Poincare::Solver<double>::Interest::None) const;

  /* The following classes and methods are used to iterate other a certain type
   * of points of interest. */
  class Iterator {
  public:
    Iterator(int index, const PointsOfInterestCache * list, Poincare::Solver<double>::Interest interest) : m_index(index), m_list(list), m_interest(interest) {}
    Iterator & operator++();
    Poincare::PointOfInterest<double> operator*() { return m_list->pointAtIndex(m_index); }
    bool operator!=(const Iterator & other) const { return m_index != other.m_index; }

  private:
    int m_index;
    const PointsOfInterestCache * m_list;
    Poincare::Solver<double>::Interest m_interest;
  };

  class Filter {
  public:
    Filter(const PointsOfInterestCache * list, Poincare::Solver<double>::Interest interest) : m_list(list), m_interest(interest) {}
    Iterator begin() const { return ++Iterator(-1, m_list, m_interest); }
    Iterator end() const { return Iterator(m_list->m_list.numberOfPoints(), m_list, m_interest); }

  private:
    const PointsOfInterestCache * m_list;
    Poincare::Solver<double>::Interest m_interest;
  };

  Filter filter(Poincare::Solver<double>::Interest interest = Poincare::Solver<double>::Interest::None) const { return Filter(this, interest); }

private:
  void stripOutOfBounds();
  void computeBetween(float start, float end, float * dirtyStart, float * dirtyEnd);
  void append(double x, double y, Poincare::Solver<double>::Interest, float * dirtyStart, float * dirtyEnd, uint32_t data = 0);

  Ion::Storage::Record m_record;
  uint32_t m_checksum;
  float m_start;
  float m_end;
  Poincare::PointsOfInterestList<double> m_list;
};

}

#endif
