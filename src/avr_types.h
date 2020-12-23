#ifndef _AVR_TYPES_H
#define _AVR_TYPES_H

#include <stdint.h> 

namespace atm8
{
  using reg = volatile uint8_t &;
  using pin = uint8_t;
}
 
namespace bit
{

inline void set (volatile uint8_t & port, uint8_t bit)
{
  port |= _BV (bit);
}

inline void clear (volatile uint8_t & port, uint8_t bit)
{
  port &= ~ _BV (bit);
}

} 

#endif /* _AVR_TYPES_H */
