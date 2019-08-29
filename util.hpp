#pragma once

#include <utility>

namespace util{

  template<typename T>
  class DirtyTracker
  {
    T _t;

    bool _dirty = true;

public:
    DirtyTracker() {}

    explicit DirtyTracker(DirtyTracker &&t) = default;

    DirtyTracker(const DirtyTracker &o) { *this = o.value(); }
    DirtyTracker &operator=(const DirtyTracker &o) {
      *this = o.value();
      return *this;
    }

    DirtyTracker(const T &t) : _t{t} {}

    DirtyTracker &operator=(const T &t) {
      if (t == _t)
        return *this;

      _t = std::forward<decltype(t)>(t);
      _dirty = true;

      return *this;
    }

    bool dirty() const { return _dirty; }
    bool &dirty() { return _dirty; }

    const T &value() const { return _t; }
  };
};
