#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "oled.h"
#include "sid.h"
#include "ui.h"
#include "uart.h"
#include "midi.h"
#include "menu.h"
#include "settings.h"
   
/* ---------- PIN CONFIGURATION ----------
 *
 * PORTB
 *  0 - sw3
 *  1 - sid_clk
 *  2 - spi_ss (sw4) :(
 *  3 - spi_mosi
 *  4 - spi_miso
 *  5 - spi_sck
 *
 * PORTC
 *  0 - enc1_a
 *  1 - enc1_b 
 *  2 - enc2_a 
 *  3 - enc2_b 
 *  4 - sw1 
 *  5 - sw2
 *
 * PORTD
 *  0 - midi rx
 *  1 - 595_data
 *  2 - 595_clock
 *  3 - 595_latch
 *  4 - oled dc
 *  5 - oled cs
 *  6 - sid_cs
 *  7 - sid_rw
 *
 * ------------------------------------ */

const uint8_t midi_rx     = PD0;
const uint8_t hc595_data  = PD1;
const uint8_t hc595_clk   = PD2;
const uint8_t hc595_latch = PD3;

const uint8_t sid_clk     = PB1;
const uint8_t sid_cs      = PD6;
const uint8_t sid_rw      = PD7;
 
const uint8_t spi_mosi    = PB3;
const uint8_t spi_miso    = PB4;
const uint8_t spi_clk     = PB5; 
const uint8_t spi_ss      = PB2; 

const uint8_t enc1_a      = PC0;
const uint8_t enc1_b      = PC1;
const uint8_t enc2_a      = PC2;
const uint8_t enc2_b      = PC3;
const uint8_t sw1         = PC4;
const uint8_t sw2         = PC5;
const uint8_t sw3         = PB0;

const uint16_t notes[] PROGMEM = 
{
  291, 308, 326, 346, 366, 388, 411, 435, 461, 489, 518, 549, 
  581, 616, 652, 691, 732, 776, 822, 871, 923, 978, 1036,1097,
  1163, 1232, 1305, 1383, 1465, 1552, 1644, 1742, 1845, 1955, 2071, 2195,
  2325, 2463, 2610, 2765, 2930, 3104, 3288, 3484, 3691, 3910, 4143, 4650,
  4927, 5220, 5530, 5859, 6207, 6577, 6968, 7382, 7821, 8286, 8779, 9301,
  9854, 10440, 11060, 11718, 12415, 13153, 13935, 14764, 15642, 16572, 17557, 18601,
  19709, 20897, 22121, 23436, 24830, 26306, 27871, 29528, 31234, 33144, 35115, 37203,
  39415, 41759, 44242, 46873, 49660, 52613, 55741, 59056, 62567,
};

namespace strings
{
  const char voice1     [] PROGMEM = "VOICE 1";
  const char voice2     [] PROGMEM = "VOICE 2";
  const char voice3     [] PROGMEM = "VOICE 3";
  const char attack     [] PROGMEM = "ATTACK";
  const char decay      [] PROGMEM = "DECAY";
  const char sustain    [] PROGMEM = "SUSTAIN";
  const char release    [] PROGMEM = "RELEASE";
  const char frequency  [] PROGMEM = "FREQ";
  const char shape      [] PROGMEM = "SHAPE";
  const char gate       [] PROGMEM = "GATE";
  const char pulsewidth [] PROGMEM = "PW";
  const char tri        [] PROGMEM = "TRI";
  const char saw        [] PROGMEM = "SAW";
  const char squ        [] PROGMEM = "SQU";
  const char noise      [] PROGMEM = "NOI";
  const char filter     [] PROGMEM = "FILTER";
  const char mark       [] PROGMEM = ">";
  const char resonance  [] PROGMEM = "RES";
  const char cutoff     [] PROGMEM = "CUTOFF";
  const char type       [] PROGMEM = "TYPE";
  const char lp         [] PROGMEM = "LP";
  const char bp         [] PROGMEM = "BP";
  const char hp         [] PROGMEM = "HP";
}

struct SidHandler
{
  static void write (uint8_t address, uint8_t data)
  {
    bit::clear (PORTD, hc595_clk);
    bit::clear (PORTD, hc595_latch);

    bit::set (PORTD, sid_cs);

    for (int i = 7; i >= 0; --i)
    {
      if (data & _BV (i))
      {
        bit::set (PORTD, hc595_data);
      }

      else
      {
        bit::clear (PORTD, hc595_data);
      }

      bit::set (PORTD, hc595_clk);
      bit::clear (PORTD, hc595_clk);
    }

    for (int i = 7; i >= 0; --i)
    {
      if (address & _BV (i))
      {
        bit::set (PORTD, hc595_data);
      }

      else
      {
        bit::clear (PORTD, hc595_data);
      }

      bit::set (PORTD, hc595_clk);
      bit::clear (PORTD, hc595_clk);
    }

    bit::set (PORTD, hc595_latch);
    bit::clear (PORTD, sid_cs);
    _delay_us (4);
    bit::set (PORTD, sid_cs);
  }
};

Oled _oled;
Sid<SidHandler> _sid;
Settings _settings;

struct MidiHandler
{
  static void note_on (uint8_t channel, uint8_t note)
  {
    uint8_t voice = 0;

    switch (channel)
    {
      case 0:
        voice = 0;
        break;

      case 1:
        voice = 1;
        break;

      case 2:
        voice = 2;
        break;

      default:
        break;
    }
    _sid.set_frequency (voice, pgm_read_word (& notes[note]));
    _sid.gate (voice, true);
    _sid.update ();
  }

  static void note_off (uint8_t channel)
  {
    uint8_t voice = 0;

    switch (channel)
    {
      case 0:
        voice = 0;
        break;

      case 1:
        voice = 1;
        break;

      case 2:
        voice = 2;
        break;

      default:
        break;
    }
    _sid.gate (voice, false);
    _sid.update ();
  }

  static void pitch_bend (uint8_t channel, int32_t value)
  {
  }
  
  static void clock (uint8_t counter)
  {
  }
};

void render_item (uint8_t x, uint8_t y, const char * text, const char * val, bool current)
{
  char row[21] {};
  memset (row, ' ', 20);

  if (text)
  {
    memcpy_P (row + 2, text, strlen_P (text));
  }
  if (val)
  {
    memcpy (row + 16, val, strlen (val));
  }
  
  _oled.write_text (0, y, row, current);
}

void read_setting (Setting setting, uint8_t voice, char * val)
{
  itoa (_settings.get (setting, voice), val, 10);
}

void write_setting (Setting setting, uint8_t voice, int8_t val)
{
  _settings.set (setting, voice, _settings.get (setting, voice) + val);
  int16_t new_val = _settings.get (setting, voice);

  switch (setting)
  {
    case VOICE_FREQUENCY:
      _sid.set_frequency (voice, new_val * 512);
      break;

    case VOICE_SHAPE:
      {
        uint8_t shape = 1;

        switch (new_val)
        {
          case 0:
          shape <<= 0;
          break;

          case 1:
          shape <<= 1;
          break;

          case 2:
          shape <<= 2;
          break;

          case 3:
          shape <<= 3;
          break;

          default:
          break;
        }
        _sid.set_shape (voice, shape);
      }
      break;
       
    case VOICE_PW:
      _sid.set_pulsewidth (voice, new_val * 32);
      break; 
    
    case VOICE_ATTACK:
      _sid.set_attack (voice, new_val);
      break;

    case VOICE_DECAY:
      _sid.set_decay (voice, new_val);
      break;

    case VOICE_SUSTAIN:
      _sid.set_sustain (voice, new_val);
      break;

    case VOICE_RELEASE:
      _sid.set_release (voice, new_val);
      break;

    case VOICE_GATE:
      _sid.gate (voice, new_val);
      break;
 
    case VOICE_FILTER:
      _sid.set_filter (voice, new_val);
      break;
 
    case FILTER_CUTOFF:
      _sid.set_filter_cutoff (new_val * 8);
      break;
 
    case FILTER_RESONANCE:
      _sid.set_filter_resonance (new_val);
      break;
 
    case FILTER_MODE:
 {
        uint8_t mode = 1;

        switch (new_val)
        {
          case 0:
          mode <<= 0;
          break;

          case 1:
          mode <<= 1;
          break;

          case 2:
          mode <<= 2;
          break;

          default:
          break;
        }
        _sid.set_filter_mode (mode);
      }
      break;

    default:
      break;
  } 
  _sid.update ();
}

MenuItem voice1_items[]
{  
  { strings::voice1,  nullptr   , nullptr        },
  { strings::frequency, [] (char * val) { read_setting  (VOICE_FREQUENCY, 0, val); }, 
            [] (int8_t v)   { write_setting (VOICE_FREQUENCY, 0, v); } }, 
  { strings::shape,   [] (char * val) 
               { 
                 const char * res = "";

                 switch (_settings.get (VOICE_SHAPE, 0))
                 {
                   case 0:
                     res = strings::tri;
                     break;
                   case 1:
                     res = strings::saw;
                     break;
                   case 2:
                     res =  strings::squ;
                     break;
                   case 3:
                     res = strings::noise;
                   default:
                     break;
                 }; 

                 memcpy_P (val, res, 4);
               }, 
               [] (int8_t v) { write_setting (VOICE_SHAPE, 0, v); } },
  { strings::pulsewidth, [] (char * val) { read_setting  (VOICE_PW, 0, val); }, 
                 [] (int8_t v)   { write_setting (VOICE_PW, 0, v); } },
  { strings::attack,  [] (char * val) { read_setting  (VOICE_ATTACK, 0, val); }, 
               [] (int8_t v)   { write_setting (VOICE_ATTACK, 0, v); } }, 

  { strings::decay,   [] (char * val) { read_setting  (VOICE_DECAY, 0, val); }, 
               [] (int8_t v)   { write_setting (VOICE_DECAY, 0, v); } }, 

  { strings::sustain, [] (char * val) { read_setting  (VOICE_SUSTAIN, 0, val); }, 
               [] (int8_t v)   { write_setting (VOICE_SUSTAIN, 0, v); } }, 

  { strings::release, [] (char * val) { read_setting  (VOICE_RELEASE, 0, val); }, 
               [] (int8_t v)   { write_setting (VOICE_RELEASE, 0, v); } }, 

  { strings::gate,    [] (char * val) { read_setting  (VOICE_GATE, 0, val); }, 
               [] (int8_t v)   { write_setting (VOICE_GATE, 0, v); } }, 
  { strings::filter,  [] (char * val) { read_setting  (VOICE_FILTER, 0, val); }, 
               [] (int8_t v)   { write_setting (VOICE_FILTER, 0, v); } },  
};

MenuItem voice2_items[]
{  
  { strings::voice2,  nullptr   , nullptr        },
  { strings::frequency, [] (char * val) { read_setting  (VOICE_FREQUENCY, 1, val); }, 
            [] (int8_t v)   { write_setting (VOICE_FREQUENCY, 1, v); } }, 
  { strings::shape,   [] (char * val) 
               { 
                 const char * res = "";

                 switch (_settings.get (VOICE_SHAPE, 1))
                 {
                   case 0:
                     res = strings::tri;
                     break;
                   case 1:
                     res = strings::saw;
                     break;
                   case 2:
                     res =  strings::squ;
                     break;
                   case 3:
                     res = strings::noise;
                   default:
                     break;
                 }; 

                 memcpy_P (val, res, 4);
               }, 
               [] (int8_t v) { write_setting (VOICE_SHAPE, 1, v); } },
  { strings::pulsewidth, [] (char * val) { read_setting  (VOICE_PW, 1, val); }, 
                 [] (int8_t v)   { write_setting (VOICE_PW, 1, v); } },
  { strings::attack,  [] (char * val) { read_setting  (VOICE_ATTACK, 1, val); }, 
               [] (int8_t v)   { write_setting (VOICE_ATTACK, 1, v); } }, 

  { strings::decay,   [] (char * val) { read_setting  (VOICE_DECAY, 1, val); }, 
               [] (int8_t v)   { write_setting (VOICE_DECAY, 1, v); } }, 

  { strings::sustain, [] (char * val) { read_setting  (VOICE_SUSTAIN, 1, val); }, 
               [] (int8_t v)   { write_setting (VOICE_SUSTAIN, 1, v); } }, 

  { strings::release, [] (char * val) { read_setting  (VOICE_RELEASE, 1, val); }, 
               [] (int8_t v)   { write_setting (VOICE_RELEASE, 1, v); } }, 

  { strings::gate,  [] (char * val) { read_setting  (VOICE_GATE, 1, val); }, 
               [] (int8_t v)   { write_setting (VOICE_GATE, 1, v); } }, 
  { strings::filter,  [] (char * val) { read_setting  (VOICE_FILTER, 1, val); }, 
               [] (int8_t v)   { write_setting (VOICE_FILTER, 1, v); } },
               
};

MenuItem voice3_items[]
{  
  { strings::voice3,  nullptr   , nullptr        },
  { strings::frequency, [] (char * val) { read_setting  (VOICE_FREQUENCY, 2, val); }, 
            [] (int8_t v)   { write_setting (VOICE_FREQUENCY, 2, v); } }, 
  { strings::shape,  [] (char * val) 
               { 
                 const char * res = "";

                 switch (_settings.get (VOICE_SHAPE, 2))
                 {
                   case 0:
                     res = strings::tri;
                     break;
                   case 1:
                     res = strings::saw;
                     break;
                   case 2:
                     res =  strings::squ;
                     break;
                   case 3:
                     res = strings::noise;
                   default:
                     break;
                 }; 

                 memcpy_P (val, res, 4);
               }, 
               [] (int8_t v) { write_setting (VOICE_SHAPE, 2, v); } },
  { strings::pulsewidth, [] (char * val) { read_setting  (VOICE_PW, 2, val); }, 
                 [] (int8_t v)   { write_setting (VOICE_PW, 2, v); } },
  { strings::attack,  [] (char * val) { read_setting  (VOICE_ATTACK, 2, val); }, 
               [] (int8_t v)   { write_setting (VOICE_ATTACK, 2, v); } }, 

  { strings::decay,  [] (char * val) { read_setting  (VOICE_DECAY, 2, val); }, 
               [] (int8_t v)   { write_setting (VOICE_DECAY, 2, v); } }, 

  { strings::sustain, [] (char * val) { read_setting  (VOICE_SUSTAIN, 2, val); }, 
               [] (int8_t v)   { write_setting (VOICE_SUSTAIN, 2, v); } }, 

  { strings::release, [] (char * val) { read_setting  (VOICE_RELEASE, 2, val); }, 
               [] (int8_t v)   { write_setting (VOICE_RELEASE, 2, v); } }, 

  { strings::gate,  [] (char * val) { read_setting  (VOICE_GATE, 2, val); }, 
               [] (int8_t v)   { write_setting (VOICE_GATE, 2, v); } }, 
  { strings::filter,  [] (char * val) { read_setting  (VOICE_FILTER, 2, val); }, 
               [] (int8_t v)   { write_setting (VOICE_FILTER, 2, v); } },
}; 

MenuItem filter_items[]
{
  { strings::filter, nullptr, nullptr     },
  { strings::type,   [] (char * val)    
              { 
                 const char * res = "";

                 switch (_settings.get (FILTER_MODE, 0))
                 {
                   case 0:
                     res = strings::lp;
                     break;
                   case 1:
                     res = strings::bp;
                     break;
                   case 2:
                     res = strings::hp;
                   default:
                     break;
                 }; 

                 memcpy_P (val, res, 4);
               }, 
              [] (int8_t v)   { write_setting (FILTER_MODE, 0, v); } },
  { strings::cutoff, [] (char * val) { read_setting  (FILTER_CUTOFF, 0, val); }, 
              [] (int8_t v)   { write_setting (FILTER_CUTOFF, 0, v); } },
  { strings::resonance,    [] (char * val) { read_setting  (FILTER_RESONANCE, 0, val); }, 
              [] (int8_t v)   { write_setting (FILTER_RESONANCE, 0, v); } },
};

Menu menu (& voice1_items[0], & render_item, strings::mark);

Encoder _e1 (DDRC, PORTC, PINC, enc1_a, enc1_b, sw1);
Encoder _e2 (DDRC, PORTC, PINC, enc2_a, enc2_b, sw2);
Ui _ui (_e1, _e2, menu, _oled, _settings);
Uart _serial;
Midi<MidiHandler> _midi (_serial); 
                                                

ISR(TIMER0_COMPA_vect) 
{
  _ui.read_inputs ();
}

int main ()
{
  DDRD = 0;
  DDRB = 0;

  DDRD |= _BV (hc595_data) 
       |  _BV (hc595_clk) 
       |  _BV (hc595_latch) 
       |  _BV (oled_dc) 
       |  _BV (oled_cs)
       |  _BV (sid_rw)
       |  _BV (sid_cs);
  
  DDRD &= ~ _BV (midi_rx);

  DDRB |= _BV (spi_mosi)
       |  _BV (spi_clk)
       |  _BV (spi_ss)
       |  _BV (sid_clk)
       |  _BV (PD0);

  bit::set (PORTB, sw3);

  // Init SPI
  SPCR = _BV (SPE) | _BV (MSTR);
  SPSR = (1 << SPI2X);   

  // Setup 1MHz clock for SID
  TCCR1A = _BV(COM1A0); 
  OCR1A = 7;
  TCCR1B = _BV(WGM12) | _BV(CS10);

  bit::clear (PORTD, sid_rw);

  bit::set (PORTD, sid_cs);
  
  _settings.load ();
  _oled.init ();
  _oled.on ();
  _sid.init ();

  for (uint8_t voice = 0; voice < 3; ++voice)
  { 
    write_setting (VOICE_FREQUENCY, voice, 0);
    write_setting (VOICE_SHAPE, voice,     0);
    write_setting (VOICE_PW, voice,        0);
    write_setting (VOICE_ATTACK, voice,    0);
    write_setting (VOICE_DECAY, voice,     0);
    write_setting (VOICE_SUSTAIN, voice,   0);
    write_setting (VOICE_RELEASE, voice,   0);
    write_setting (VOICE_FILTER, voice,    0); 
    write_setting (VOICE_GATE, voice,      0);
  } 

  write_setting (FILTER_MODE, 0,      0);
  write_setting (FILTER_CUTOFF, 0,    0);
  write_setting (FILTER_RESONANCE, 0, 0);

  _sid.set_volume (0x0f);
  _sid.update ();

  _e1.init ();
  _e2.init ();

  _ui.init ();

  Menu::link_items (voice1_items);
  Menu::link_items (voice2_items);
  Menu::link_items (voice3_items);
  Menu::link_items (filter_items);

  MenuItem * pages[]
  {
    voice1_items,
    voice2_items,
    voice3_items,
    filter_items,
  };

  menu.init (pages);
  
  _oled.clear ();
  menu.render ();

  OCR0A = 63;	
  TCCR0A |= (1 << WGM01);
  TCCR0B |= (1 << CS02);
  TIMSK0 |= (1 << OCIE0A);

  sei();

  while (true)
  {
    _midi.process_next ();
    _ui.update ();
  }
  
  return 0;
}

