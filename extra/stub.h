#ifndef STUB_H_INCLUDE
#define STUB_H_INCLUDE

#include <stdint.h>

class EPDLite
{
public:
  EPDLite(const int16_t w, const int16_t h)
  : width(w)
  , height(h)
  , orientation(0)
  {}

  uint8_t getOrientation() const { return orientation; }
  void setOrientation(const uint8_t o) { orientation = o; }

  const int16_t width;
  const int16_t height;
private:
  uint8_t orientation;
};

// stub out pgm_read_byte to just return the dereferenced value
inline uint8_t pgm_read_byte(const uint8_t* p) { return *p; };

#define PROGMEM

#endif
