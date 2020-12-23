#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <avr/eeprom.h>

uint16_t EEMEM eeprom_settings[30];

enum Setting
{
  VOICE_FREQUENCY = 0,
  VOICE_SHAPE,
  VOICE_PW,
  VOICE_ATTACK,
  VOICE_DECAY,
  VOICE_SUSTAIN,
  VOICE_RELEASE,
  VOICE_GATE,
  VOICE_FILTER,
  
  FILTER_CUTOFF,
  FILTER_RESONANCE,
  FILTER_MODE,
};

Setting & operator++ (Setting & s)
{
  s = static_cast<Setting> (static_cast<int8_t> (s) + 1);
  return s;
}

namespace
{

bool max_15 (int16_t n)
{
  return (n >= 0 && n < 16);
}

bool max_127 (int16_t n)
{
  return (n >= 0 && n < 128);
}

}

class Settings
{
  public:
    Settings ()
      : _frequency {5, 5, 5}
      , _shape {0, 0, 0}
      , _pw {64, 64, 64}
      , _attack {0, 0, 0}
      , _decay {8, 8, 8}
      , _sustain {15, 15, 15}
      , _release {8, 8, 8}
      , _gate {0, 0, 0}
      , _filter_enable {0, 0, 0}
      , _cutoff (127)
      , _resonance (0)
      , _filter_mode (0)
    {
    }

    ~Settings ()
    {
    }

    void set (Setting setting, uint8_t voice, int16_t v)
    {
      switch (setting)
      {
        case VOICE_FREQUENCY:
          if (max_127 (v))
            _frequency[voice] = v;
          break;
        case VOICE_SHAPE:
          if (v >= 0 && v <=3)
            _shape[voice] = v;
          break;
 
        case VOICE_PW:
          if (max_127 (v))
            _pw[voice] = v;
          break;

        case VOICE_ATTACK:
          if (max_15 (v))
            _attack[voice] = v;
          break;

        case VOICE_DECAY:
          if (max_15 (v))
            _decay[voice] = v;
          break;

        case VOICE_SUSTAIN:
          if (max_15 (v))
            _sustain[voice] = v;
          break;

        case VOICE_RELEASE:
          if (max_15 (v))
            _release[voice] = v;
          break;

        case VOICE_GATE:
          if (v >= 0 && v < 2)
            _gate[voice] = v;
          break;
 
        case VOICE_FILTER:
          if (v >= 0 && v < 2)
            _filter_enable[voice] = v;
          break;

        case FILTER_CUTOFF:
          if (max_127 (v))
            _cutoff = v; 
          break;

        case FILTER_RESONANCE:
          if (max_15 (v))
            _resonance = v;
          break;

        case FILTER_MODE:
          if (v >= 0 && v <=2)
            _filter_mode = v; 
          break;
      }
    }

    int16_t get (Setting setting, uint8_t voice)
    {
      int16_t ret = 0;
  
      switch (setting)
      {
        case VOICE_FREQUENCY:
          ret = _frequency[voice];
          break;

        case VOICE_SHAPE:
          ret = _shape[voice];
          break;
 
        case VOICE_PW:
          ret = _pw[voice];
          break; 

        case VOICE_ATTACK:
          ret = _attack[voice];
          break;

        case VOICE_DECAY:
          ret = _decay[voice];
          break;

        case VOICE_SUSTAIN:
          ret = _sustain[voice];
          break;

        case VOICE_RELEASE:
          ret = _release[voice];
          break;

        case VOICE_GATE:
          ret = _gate[voice];
          break;
 
        case VOICE_FILTER:
          ret = _filter_enable[voice];
          break;
 
        case FILTER_CUTOFF:
          ret = _cutoff;
          break;
 
        case FILTER_RESONANCE:
          ret = _resonance;
          break;
 
        case FILTER_MODE:
          ret = _filter_mode;
          break; 
      }

      return ret;
    }

    void save ()
    {
      uint8_t e_idx = 0;

      for (uint8_t i = 0; i < 3; ++i)
      {
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _frequency[i]);
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _shape[i]);
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _pw[i]);
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _attack[i]);
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _decay[i]);
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _sustain[i]);
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _release[i]);
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _gate[i]);
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _filter_enable[i]);
      }
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _cutoff);
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _resonance);
        eeprom_write_word (& eeprom_settings[e_idx++], (uint16_t) _filter_mode);
    }

    void load ()
    {
      uint8_t e_idx = 0;

      for (uint8_t i = 0; i < 3; ++i)
      { 
        set (VOICE_FREQUENCY, i, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
        set (VOICE_SHAPE,     i, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
        set (VOICE_PW,        i, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
        set (VOICE_ATTACK,    i, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
        set (VOICE_DECAY,     i, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
        set (VOICE_SUSTAIN,   i, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
        set (VOICE_RELEASE,   i, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
        set (VOICE_GATE,      i, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
        set (VOICE_FILTER,    i, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
      }

      set (FILTER_CUTOFF, 0, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
      set (FILTER_RESONANCE, 0, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
      set (FILTER_MODE, 0, (int16_t) eeprom_read_word(& eeprom_settings[e_idx++]));
    }

  private:

    int16_t _frequency[3];
    int16_t _shape[3];
    int16_t _pw[3];
    int16_t _attack[3];
    int16_t _decay[3];
    int16_t _sustain[3];
    int16_t _release[3];
    int16_t _gate[3];
    int16_t _filter_enable[3];
    int16_t _cutoff;
    int16_t _resonance;
    int16_t _filter_mode;

};

#endif /* _SETTINGS_H */
