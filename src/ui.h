#ifndef _UI_H
#define _UI_H

#include "encoder.h"
#include "ringbuffer.h"
#include "menu.h"
#include "oled.h"
#include "settings.h"

enum Event_type
{
  ENCODER_ROTATE,
  ENCODER_PRESSED,
};

struct Input_event
{
  Event_type type;
  int8_t     data;
  uint8_t    id;
};

class Ui
{
  public:
    Ui (Encoder & enc1, Encoder & enc2, Menu & menu, Oled & oled, Settings & settings)
      : _enc1 (enc1)
      , _enc2 (enc2)
      , _menu (menu)
      , _oled (oled)
      , _settings (settings)
    {
    }

    ~Ui ()
    {
    }

    void init ()
    {
    }

    // Called from interrupt
    void read_inputs ()
    {
      _enc1.debounce ();

      auto e1 = _enc1.read ();

      if (e1 == 88)
      {  
        Input_event e;
        e.type = ENCODER_PRESSED;
        e.id = 1;
        _ringbuffer.write (e);
      }

      else
      {
        if (e1 != 0)
        {
          Input_event e;
          e.type = ENCODER_ROTATE;
          e.data = e1;
          e.id = 1;
          _ringbuffer.write (e);
        }
      }

      auto e2 = _enc2.read ();
       
      if (e2 == 88)
      {  
        Input_event e;
        e.type = ENCODER_PRESSED;
        e.id = 2;
        _ringbuffer.write (e);
      } 

      else
      {
        if (e2 != 0)
        {
          Input_event e;
          e.type = ENCODER_ROTATE;
          e.data = e2;
          e.id = 2;
          _ringbuffer.write (e);
        } 
      }

    }

    void update ()
    {
      if (!_ringbuffer.available ())
      {
        return;
      }

      auto event = _ringbuffer.read ();

      switch (event.type)
      {
        case ENCODER_ROTATE:
          switch (event.id)
          {
            case 1:
              _menu.navigate (event.data);
              break;

            case 2:
              _menu.edit (event.data);
              break;

            default:
              break;
          }

          break;

        case ENCODER_PRESSED:
          switch (event.id)
          {
            case 1:
              _settings.save ();
              break;

            default:
              break;
          }

        default:
          break;
      }
      //_oled.clear ();
      _menu.render ();
      //_oled.render ();
    }

  private:

    RingBuffer<Input_event, 16> _ringbuffer;
    Encoder &  _enc1;
    Encoder &  _enc2;
    Menu &     _menu;
    Oled &     _oled;
    Settings & _settings;
};

#endif /* _UI_H */
