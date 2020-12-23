#ifndef _UART_H_
#define _UART_H_

#include "ringbuffer.h"

#include <avr/io.h>
#include <avr/interrupt.h>  
#include <stdint.h>

RingBuffer<uint8_t, 32> _buffer;

class Uart
{
  public:
    Uart ()
    {
      UBRR0 = 31;

      UCSR0C = _BV (UCSZ01) | _BV (UCSZ00);
      UCSR0B = _BV (RXEN0)  | _BV (RXCIE0);
    }                                       

    bool available ()
    {
      return _buffer.available ();
    }

    inline uint8_t receive ()
    {
      return _buffer.read ();
    }
};

ISR(USART_RX_vect) 
{
  char data = UDR0;
  _buffer.write (data);
}
    

#endif /* _UART_H_ */
