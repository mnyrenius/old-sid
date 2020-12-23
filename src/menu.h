#ifndef _MENU_H
#define _MENU_H

class Menu;
class MenuItem;

class MenuItem
{
  public:
    using ReadF =  void (*)(char *);
    using WriteF = void (*)(int8_t);

    MenuItem ()
    {
    }

    MenuItem (const char * text, ReadF read, WriteF write)
      : _text (text)
      , _read (read)
      , _write (write)
      , _next (nullptr)
      , _prev (nullptr)
      , _parent (nullptr)
      , _next_page (nullptr)
      , _prev_page (nullptr)
    {
    }

    void set_next (MenuItem & next)
    {
      _next = & next;
    }

    void set_prev (MenuItem & prev)
    {
      _prev = & prev;
    }

    void set_next_page (MenuItem & next)
    {
      _next_page = & next;
    }
 
    void set_prev_page (MenuItem & prev)
    {
      _prev_page = & prev;
    }
     
    MenuItem * get_next ()
    {
      return _next;
    } 
    
    MenuItem * get_prev ()
    {
      return _prev;
    } 

    MenuItem * get_next_page ()
    {
      return _next_page;
    }

    MenuItem * get_prev_page ()
    {
      return _prev_page;
    }

    void set_parent (MenuItem & parent)
    {
      _parent = & parent;
    }

    void edit (int8_t v)
    {
      if (_write)
        _write (v);
    }

    const char * get_text ()
    {
      return _text;
    }

    void get_value (char * value)
    {
      if (_read)
        return _read (value);
    }

  private:
    const char * _text;
    ReadF   _read;
    WriteF  _write; 
    MenuItem * _next;
    MenuItem * _prev;
    MenuItem * _parent;
    MenuItem * _next_page;
    MenuItem * _prev_page;
};

class Menu
{
  public:
    using RenderF = void (*)(uint8_t x, uint8_t y, const char * text, const char * val, bool current);

    Menu ()
    {
    }

    Menu (MenuItem * item, RenderF render_f, const char * active_marker)
      : _current (item)
      , _render (render_f)
      , _active_marker (active_marker)
    {
    }
    
    template<uint8_t N>
    void init (MenuItem * (&items)[N])
    {
      for (uint8_t i = 0; i < N; ++i)
      {
        auto & item = items[i][0];

        if (i > 0)
          item.set_prev_page (items[i - 1][0]);

        if (i < N - 1)
          item.set_next_page (items[i + 1][0]);
      }
    }

    template<uint8_t N>
    static void link_items (MenuItem (&items)[N])
    {
      for (uint8_t i = 0; i < N; ++i)
      {
        auto & item = items[i];

        if (i > 0) 
          item.set_prev (items[i - 1]);

        if (i < N - 1)
          item.set_next (items[i + 1]);
      }
    }

    void navigate (int8_t direction)
    {
      MenuItem * current = _current;;

      switch (direction)
      {
        case -1:
          current = _current->get_prev ();
          break;

        case 1:
          current = _current->get_next ();
          break;

        default:
          break;
      }
      if (current)
        _current = current; 
    }

    void edit (int8_t direction)
    {
      if (!_current->get_prev_page () && !_current->get_next_page ())
      {
        _current->edit (direction);
        return;
      }

      MenuItem * current = _current;;

      switch (direction)
      {
        case -1:
          current = _current->get_prev_page ();
          break;

        case 1:
          current = _current->get_next_page ();
          break;

        default:
          break;
      }
      if (current)
        _current = current;
    }

    void render ()
    {     
      char buffer[8] {};

      MenuItem * item = _current;

      int8_t max_prevs = 0;

      while (item->get_prev () && max_prevs < 7 )
      {
        item = item->get_prev ();
        max_prevs++;
      }

      for (int8_t i = 0; i < 8; ++i)
      {
        if (item)
        {
          item->get_value (buffer);
          _render (10, i, item->get_text (), buffer, item == _current);
          item = item->get_next ();
        }
        else
        {
          _render (10, i, nullptr, nullptr, false);
        }
      } 
    }

  private:
    MenuItem *   _current;
    RenderF      _render;
    const char * _active_marker;
};

/*
#include "oled.h"

constexpr uint8_t NumMenuItems = 3;

enum MenuItems
{
  VOICE_1 = 0,
  VOICE_2,
  VOICE_3,
};

class MenuItem
{
  public:
    MenuItem ()
    {
    }

    MenuItem (const char * text)
      : _text (text)
      , _current (false)
    {
    }
     
    const char * get_text ()
    {
      if (_current)
        return "fest";

      return _text;
    }

    void set_current (bool current)
    {
      _current = current;
    }

    bool is_current ()
    {
      return _current;
    }

    ~MenuItem ()
    {
    }

  private:

    const char * _text;
    bool         _current;
};
 
MenuItem voice_1 ("VOICE 1");
MenuItem voice_2 ("VOICE 2");
MenuItem voice_3 ("VOICE 3");
                               

class Menu
{
  public:

    Menu (Oled & oled)
      : _oled (oled)
      , _current (VOICE_1)
    {
      _items[VOICE_1] = voice_1;
      _items[VOICE_2] = voice_2;
      _items[VOICE_3] = voice_3;
      _items[_current].set_current (true);
    }

    ~Menu ()
    {
    }

    void navigate (int8_t direction)
    {
      _items[_current].set_current (false);
      _current = (_current + direction) % NumMenuItems;
      _items[_current].set_current (true);
    }

    void print ()
    {
      _oled.clear ();

      for (uint8_t i = 0; i < NumMenuItems; ++i)
      {
        _oled.write_text (10, i, _items[i].get_text (), _items[i].is_current ());
      }

      _oled.render ();
    }

  private:
    Oled &   _oled;
    MenuItem _items[NumMenuItems];
    uint8_t  _current;
};
 */
#endif /* _MENU_H */
