#ifndef _MIDI_HANDLER_H
#define _MIDI_HANDLER_H

#include <stdint.h>
#include "uart.h"
#include <avr/io.h>

template<class TCallback>
class Midi
{
  public:

    Midi (Uart & serial)
      : _serial (serial)
      , _running_status (0)
      , _expected (0)
      , _data_index (0)
      , _running (0)
      , _clk_counter (0)
  {
  }

    void process_next ()
    {
      if (!_serial.available ())
      {
        return;
      }

      auto data = _serial.receive ();

      if (data == 0xfe)
      {
        return;
      }

      if (data >= 0xf8)
      {
        handle_finished (data);
      }

      else 
      {
        if (data >= 0x80)
        {
          _running_status = data;

          switch (data & 0xf0)
          {
            case 0x80:
            case 0x90:
            case 0xe0:
              _expected = 2;
              _data_index = 0;
              break;

            default:
              break;
          }
        }

        else
        {
          _data[_data_index++] = data;
        }


        if (_data_index == _expected)
        {
          handle_finished (_running_status);

          _data_index = 0;
        }
      }

      return;
    }

  private:

    void handle_finished (uint8_t byte)
    {
      auto msb = byte & 0xf0;
      auto lsb = byte & 0x0f;

      switch (msb)
      {
        case 0x90:

          if (_data[1] == 0)
          {
            TCallback::note_off (lsb);
          }

          else
          {
            TCallback::note_on (lsb, _data[0]);
          }

          break;

        case 0x80:
            TCallback::note_off (lsb);
            break;

        case 0xe0:
            TCallback::pitch_bend (msb & 0x0f, (_data[0] & 0x7f) | (_data[1] & 0x7f) << 7);
            break;

        case 0xf0:
            switch (lsb)
            {
              case 0xa:
                _clk_counter = 0;
                _running = true;
                break;

              case 0xb:
                _running = true;
                break;

              case 0xc:
                _running = false;
                break;

              case 0x8:
                if (_running)
                {
                  TCallback::clock (_clk_counter);

                  if (_clk_counter++ >= 24)
                  {
                    _clk_counter = 0;
                  }
                }
                break; 

              default:
                break;
            }

            break;

        default:
            break;
      }
    }

    Uart &                     _serial;
    uint8_t                    _running_status;
    uint8_t                    _data[2];
    uint8_t                    _expected;
    uint8_t                    _data_index;
    bool                       _running;
    uint8_t                    _clk_counter;
};
 
#endif /* _MIDI_HANDLER_H */
