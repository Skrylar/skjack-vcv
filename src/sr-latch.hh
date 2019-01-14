#pragma once

#include <atomic>

class sr_latch {
  std::atomic<bool> m_latch;
public:
  sr_latch() : m_latch(false) {}

  bool try_set() {
    if (m_latch) {
      return false;
    } else {
      m_latch = true;
      return true;
    }
  }

  void reset() {
    m_latch = false;
  }

  bool latched() const {
    return m_latch;
  }
};
