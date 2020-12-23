#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "avr_types.h"

class Encoder
{
  public:
    
    Encoder (atm8::reg ddr, atm8::reg pdr, atm8::reg prr, atm8::pin a, atm8::pin b, atm8::pin sw)
      : _ddr (ddr)
      , _pdr (pdr)
      , _prr (prr)
      , _a (a)
      , _b (b)
      , _sw (sw)
      , _enc_current (0xff)
      , _switch (0xff)
      , _a_state (0xff)
      , _b_state (0xff)
    {
    }

    ~Encoder ()
    {
    }

    void init ()
    {
      _ddr &= ~ _BV (_a);
      _ddr &= ~ _BV (_b);
      _ddr &= ~ _BV (_sw);

      _pdr |= _BV (_a) | _BV (_b) | _BV (_sw);
    }

    void debounce ()
    {
      auto sw = _prr & _BV (_sw);
      _switch = (_switch << 1) | !sw;
    }

    int8_t read ()
    {
      auto a = _prr & _BV (_a);
      auto b = _prr & _BV (_b);

      uint8_t result = 0;
      int8_t inc = 0;

      if (a)
      {
        result |= 1 << 1;
      }

      if (b)
      {
        result |= 1 << 0;
      }

      if (result != _enc_current)
      {
        if((_enc_current == 3 && result == 1) || (_enc_current == 0 && result == 2)) 
        { 
          inc = 1;
        } 

        else if ((_enc_current == 2 && result == 0) || (_enc_current == 1 && result == 3)) 
        { 
          inc = -1;
        } 
        
        _enc_current = result;
      }

      if (_switch == 0x7f)
      {
        return 88;
      }

      return inc;
    }

  private:

    atm8::reg _ddr;
    atm8::reg _pdr;
    atm8::reg _prr;
    atm8::pin _a;
    atm8::pin _b;
    atm8::pin _sw;
    uint8_t   _enc_current;
    uint8_t   _switch;
    uint8_t   _a_state;
    uint8_t   _b_state;
};

#endif /* _ENCODER_H_ */
