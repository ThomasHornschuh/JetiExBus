/*
Jeti Sensor EX Telemetry C++ Library

JetiExSerial - EX Bus serial I/O implementation
--------------------------------------------------------------------

Copyright (C) 2018 Bernd Wokoeck

Version history:
0.90   02/08/2018  created

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

**************************************************************/

#include "JetiExBusSerial.h"

// Teensy
/////////
#if defined( CORE_TEENSY ) 


  JetiExBusSerial * JetiExBusSerial::CreatePort( int comPort )
  {
    return new JetiExBusTeensySerial( comPort );
  } 

  JetiExBusTeensySerial::JetiExBusTeensySerial( int comPort ) : m_pSerial(0)
  {
    switch( comPort )
    {
    default:
    case 2: m_pSerial = &Serial2; break;
    case 1: m_pSerial = &Serial1; break;
    case 3: m_pSerial = &Serial3; break;
    }
  }

#endif // CORE_TEENSY 

#if  defined ( STM32F3xx )
#pragma message "compiling JetiExBusSerial"

//static HardwareSerial _serial ((void*)USART3,HALF_DUPLEX_ENABLED); // Hardcoded to USART3 currently

  JetiExBusSerial * JetiExBusSerial::CreatePort( int comPort )
  {
    return new JetiExBusSTM32Serial( comPort );
  } 

  JetiExBusSTM32Serial::JetiExBusSTM32Serial( int comPort ) 
  {
    m_pSerial = new HardwareSerial(PB10); //PB10 / D6
    m_pSerial->setHalfDuplex();

  }


  size_t JetiExBusSTM32Serial::write(const uint8_t *buffer, size_t size)
  { size_t res;
    res = m_pSerial->write(buffer, size);
    //m_pSerial->flush();
    m_pSerial->enableHalfDuplexRx();
    return res;
    
  }

#endif // STM32F3xx 
