#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <stdint.h>

template<typename T, uint8_t Size>
class RingBuffer
{
  public:
    RingBuffer ()
      : _read_pos (0)
      , _write_pos (0)
  {
  }

  inline bool empty () const 
  {
    return (_write_pos - _read_pos) == 0;
  }

  inline bool full () const
  {
    return (_write_pos - _read_pos) == (Size - 1);
  }

  inline void write (T data)
  {
    while (full ());

    auto w = _write_pos;
    _buffer[w] = data;
    _write_pos = (w + 1) & (Size - 1);
  }

  inline T read ()
  {
    while (empty ());

    auto r = _read_pos;
    auto value = _buffer[r];
    _read_pos = (r + 1) & (Size - 1);
    return value;
  }

  bool available ()
  {
    return _read_pos != _write_pos;
  }

  private:
    T       _buffer[Size];
    volatile uint8_t _read_pos;
    volatile uint8_t _write_pos;

};

#endif // _RING_BUFFER_H_
