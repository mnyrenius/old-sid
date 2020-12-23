#ifndef _SID_H
#define _SID_H

#include "avr_types.h"

enum registers {
  Voice_1_freq_lo = 0  ,
  Voice_1_freq_hi      ,
  Voice_1_pw_lo        ,
  Voice_1_pw_hi        ,
  Voice_1_control      ,
  Voice_1_ad           ,
  Voice_1_sr           ,
  Voice_2_freq_lo      ,
  Voice_2_freq_hi      ,
  Voice_2_pw_lo        ,
  Voice_2_pw_hi        ,
  Voice_2_control      ,
  Voice_2_ad           ,
  Voice_2_sr           ,
  Voice_3_freq_lo      ,
  Voice_3_freq_hi      ,
  Voice_3_pw_lo        ,
  Voice_3_pw_hi        ,
  Voice_3_control      ,
  Voice_3_ad           ,
  Voice_3_sr           ,
  Filter_cutoff_lo     ,
  Filter_cutoff_hi     ,
  Filter_res_en        ,
  Filter_mode_vol      ,

  Last_register        ,
};

static constexpr uint8_t Gate_bit    = _BV (0);
static constexpr uint8_t Sync_bit    = _BV (1);
static constexpr uint8_t Ringmod_bit = _BV (2);
static constexpr uint8_t Tri_bit     = _BV (4);
static constexpr uint8_t Saw_bit     = _BV (5);
static constexpr uint8_t Square_bit  = _BV (6);
static constexpr uint8_t Noise_bit   = _BV (7);

static constexpr uint8_t Filt1_bit    = _BV (0);
static constexpr uint8_t Filt2_bit    = _BV (1);
static constexpr uint8_t Filt3_bit    = _BV (2);
static constexpr uint8_t Filtex_bit   = _BV (3);

static constexpr uint8_t Filt_lp_bit   = _BV (4);
static constexpr uint8_t Filt_hp_bit   = _BV (5);
static constexpr uint8_t Filt_bp_bit   = _BV (6);

static constexpr uint8_t Vol_0_bit   = _BV (0);
static constexpr uint8_t Vol_1_bit   = _BV (1);
static constexpr uint8_t Vol_2_bit   = _BV (2);
static constexpr uint8_t Vol_3_bit   = _BV (3);

static constexpr uint8_t _1 = 0;
static constexpr uint8_t _2 = 1;
static constexpr uint8_t _3 = 2;

static const uint8_t Control_registers[] =
{
  Voice_1_control,
  Voice_2_control,
  Voice_3_control,
};

static const uint8_t Filter_enable_bits[] =
{
  Filt1_bit,
  Filt2_bit,
  Filt3_bit,
};

uint8_t get_ad_reg (uint8_t voice)
{
  uint8_t reg = 0;

  switch (voice)
  {
    case _1:
      reg = Voice_1_ad;
      break;

    case _2:
      reg = Voice_2_ad;
      break;

    case _3:
      reg = Voice_3_ad;
      break;

    default:
      break;
  }

  return reg;
} 

uint8_t get_sr_reg (uint8_t voice)
{
  uint8_t reg = 0;

  switch (voice)
  {
    case _1:
      reg = Voice_1_sr;
      break;

    case _2:
      reg = Voice_2_sr;
      break;

    case _3:
      reg = Voice_3_sr;
      break;

    default:
      break;
  }

  return reg;
}

template<class Device>
class Sid
{
  struct Register
  {
    Register ()
      : current  (0)
      , previous (0)
    {
    }

    void copy ()
    {
      previous = current;
    }

    uint8_t current;
    uint8_t previous;
  };

  public:

    Sid ()
    {
    }

    ~Sid ()
    {
    }

    void init ()
    {
      for (int i = 0; i < 25; ++i)
      {
        Device::write (i, 0);
      }
    }

    void update ()
    {
      for (uint8_t reg = Voice_1_freq_lo; reg < Last_register; ++reg)
      {
        if (_registers[reg].current != _registers[reg].previous)
          Device::write (reg, _registers[reg].current);
      }
    }

    void set_frequency (uint8_t voice, uint16_t frequency)
    {
      uint8_t lo = 0;

      switch (voice)
      {
        case _1:
          lo = Voice_1_freq_lo;
          break;

        case _2:
          lo = Voice_2_freq_lo;
          break;
        
        case _3:
          lo = Voice_3_freq_lo;
          break;
        
        default:
          break;
      }

      auto hi = lo + 1;
      
      _registers[lo].copy ();
      _registers[hi].copy ();
      _registers[lo].current = frequency;
      _registers[hi].current = frequency >> 8; 
    }

    void set_pulsewidth (uint8_t voice, uint16_t pulsewidth)
    {
      uint8_t lo = 0;

      switch (voice)
      {
        case _1:
          lo = Voice_1_pw_lo;
          break;

        case _2:
          lo = Voice_2_pw_lo;
          break;
        
        case _3:
          lo = Voice_3_pw_lo;
          break;
        
        default:
          break;
      }  

      auto hi = lo + 1;
      
      _registers[lo].copy ();
      _registers[hi].copy ();
      _registers[lo].current = pulsewidth;
      _registers[hi].current = (pulsewidth >> 8) & 0x0f;
    }

    void gate (uint8_t voice, bool enabled)
    {
      auto regno = Control_registers[voice];
      auto & reg = _registers[regno];
      reg.copy ();

      if (enabled)
      {
        reg.current |= Gate_bit;
      }

      else
      {
        reg.current &= ~ Gate_bit;
      }
    }

    void set_sync (uint8_t voice, bool enabled)
    {
    }

    void set_ringmod (uint8_t voice, bool enabled)
    {
    }

    void set_shape (uint8_t voice, uint8_t shape_bits)
    {
      auto regno = Control_registers[voice];
      auto & reg = _registers[regno];
      reg.copy ();

      reg.current = (shape_bits << 4) | (reg.previous & 0xf);
    }

    void set_attack (uint8_t voice, uint8_t attack)
    {
      auto regno = get_ad_reg (voice);
      auto & reg = _registers[regno];
      reg.copy ();
      reg.current = (reg.previous & 0x0f) | (attack << 4); 
    }

    void set_decay (uint8_t voice, uint8_t decay)
    {
      auto regno = get_ad_reg (voice);
      auto & reg = _registers[regno];
      reg.copy ();
      reg.current = (reg.previous & 0xf0) | (decay & 0x0f); 
    }
 
    void set_sustain (uint8_t voice, uint8_t sustain)
    {
      auto regno = get_sr_reg (voice);
      auto & reg = _registers[regno];
      reg.copy ();
      reg.current = (reg.previous & 0x0f) | (sustain << 4); 
       
    }
 
    void set_release (uint8_t voice, uint8_t release)
    {
      auto regno = get_sr_reg (voice);
      auto & reg = _registers[regno];
      reg.copy ();
      reg.current = (reg.previous & 0xf0) | (release & 0x0f); 
    }

    void set_filter_cutoff (uint16_t cutoff)
    {
      auto & reg_lo = _registers[Filter_cutoff_lo];
      auto & reg_hi = _registers[Filter_cutoff_hi];
      reg_lo.copy ();
      reg_hi.copy ();
      reg_lo.current = cutoff & 0x07;
      reg_hi.current = cutoff >> 3;
    }

    void set_filter_resonance (uint8_t res)
    {
      auto & reg = _registers[Filter_res_en];
      reg.copy ();
      reg.current = (reg.previous & 0x0f) | (res << 4);
    }

    void set_filter (uint8_t voice, bool enabled)
    {
      auto & reg = _registers[Filter_res_en];
      reg.copy ();
      if (enabled)
      {
        reg.current |= Filter_enable_bits[voice];
      }

      else
      {
        reg.current &= ~ Filter_enable_bits[voice];
      }
    }

    void set_filter_mode (uint8_t mode)
    {
      auto & reg = _registers[Filter_mode_vol];
      reg.copy ();
      reg.current = (reg.previous & 0x0f) | (mode << 4);
    }
    
    void set_volume (uint8_t volume)
    {
      auto & reg = _registers[Filter_mode_vol];
      reg.copy ();
      reg.current = (reg.previous & 0xf0) | volume;
    }

  private:

    Register  _registers[Last_register];

};

#endif /* _SID_H */
