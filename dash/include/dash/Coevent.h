#ifndef COEVENT_H_INCLUDED
#define COEVENT_H_INCLUDED

#include <dash/Exception.h>
#include <dash/Team.h>
#include <dash/Types.h>
#include <dash/GlobPtr.h>
#include <dash/Atomic.h>

#include <dash/Algorithm.h>

#include <dash/coarray/CoEventIter.h>
#include <dash/coarray/CoEventRef.h>

namespace dash {

/**
 * \ingroup DashCoarrayConcept
 * 
 *
 * A fortran style coevent.
 *
 * Coevent can be used for point-to-point synchronization. Events can be posted
 * to any image. Waiting on non-local events is not supported.
 * 
 * 
 * Example:
 * 
 * \code
 * Coevent events;
 * 
 * events(2).post();
 * if(this_image() == 2){
 *  events.wait();
 * }
 * \endcode
 */
class Coevent {
private:
  using event_cnt_t    = dash::Atomic<int>;
  using gptr_t         = GlobPtr<event_cnt_t>;
public:
  // Types
  using iterator       = coarray::CoEventIter;
  using const_iterator = coarray::CoEventIter;
  using reference      = coarray::CoEventRef;
  using size_type      = int;
  
private:
  dash::Array<event_cnt_t> _event_counts;
  
public:
  
  /**
   * Constructor to setup and initialize an Coevent.
   */
  explicit Coevent(Team & team = dash::Team::All())
    : _team(&team) {
      if(dash::is_initialized()){
        initialize(team);
      }
    }
  
  iterator begin() noexcept {
    return iterator(static_cast<gptr_t>(_event_counts.begin()));
  }
  
  const_iterator begin() const noexcept {
    return const_iterator(static_cast<gptr_t>(_event_counts.begin()));
  }
  
  iterator end() {
    DASH_ASSERT_MSG(dash::is_initialized(), "DASH is not initialized");
    return iterator(static_cast<gptr_t>(_event_counts.end()));
  }
  
  const_iterator end() const {
    DASH_ASSERT_MSG(dash::is_initialized(), "DASH is not initialized");
    return const_iterator(static_cast<gptr_t>(_event_counts.end()));
  }
  
  size_type size() const {
    DASH_ASSERT_MSG(dash::is_initialized(), "DASH is not initialized");
    return _team->size();
  }
  
  /**
   * wait for a given number of incoming events.
   * This function is thread-safe
   */
  inline void wait(int count = 1) {
    auto gref = _event_counts.at(_team->myid().id);
    while(!gref.compare_exchange(count, 0)){
#ifdef DASH_DEBUG
      // avoid spamming the logs while busy waiting
      DASH_LOG_DEBUG("waiting for event at gptr",
                     static_cast<gptr_t>(_event_counts.begin()
                                         +_team->myid().id));
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
    };
  }

  /**
   * initializes the Coevent. If it was already initialized in the Ctor,
   * the second initialization is skipped.
   */
  inline void initialize(Team & team) noexcept {
    _team = &team;
    _event_counts.allocate(_team->size());
    dash::fill(_event_counts.begin(), _event_counts.end(), 0);
    _is_initialized = true;
  }
  
  inline Team & team() {
    return *_team;
  }

  /**
   * Operator to select event at given unit.
   */
  inline reference operator()(const int & unit) noexcept {
    DASH_ASSERT_MSG(dash::is_initialized(), "DASH is not initialized");
    auto ptr = static_cast<gptr_t>(_event_counts.begin() + unit);
    return reference(ptr);
  }

  /**
   * Operator to select event at given unit.
   */
  inline reference operator()(const team_unit_t & unit) noexcept {
    return this->operator()(static_cast<int>(unit));
  }
  
private:
  Team * _team;
  bool   _is_initialized = false;
};

} // namespace dash

#endif /* COEVENT_H_INCLUDED */
