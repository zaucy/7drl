#pragma once
#include "buffer2d.h"
#include "bitset2d.h"

namespace librl {

struct pfield_t {

  typedef buffer2d_t<uint8_t> buffer_t;

  pfield_t(buffer_t &m, const bitset2d_t &walls)
    : walls(walls)
    , map(m)
    , state(0)
    , a(new buffer_t(m.width, m.height))
    , b(new buffer_t(m.width, m.height))
  {
    a->clear(0);
    b->clear(0);
  }

  const buffer_t &read() const {
    return *(state ? a : b);
  }

  buffer_t &read() {
    return *(state ? a : b);
  }

  buffer_t &write() {
    return *(state ? b : a);
  }

  void update();

  void diff(uint32_t x, uint32_t y, int32_t &dx, int32_t &dy, uint8_t thresh);

  void drop(uint32_t x, uint32_t y, uint8_t);

protected:

  const bitset2d_t &walls;

  uint8_t sample(uint32_t x, uint32_t y) const;

  buffer_t &map;
  uint32_t state;
  std::unique_ptr<buffer_t> a;
  std::unique_ptr<buffer_t> b;
};

}  // namespace librl
