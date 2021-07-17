/*
 Jeti Sensor EX Telemetry C++ Library
 --------------------------------------------------------------------
  
  Copyright (C) 2021 Thomas Hornschuh
  
  

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



#ifndef __JETIEXBUSDMASERIAL_H__
#define __JETIEXBUSDMASERIAL_H__

#include "JetiExBusSerial.h"

#ifndef JETI_DMA_BUF_SIZE
#define JETI_DMA_BUF_SIZE 128
#endif

class JetiExBusDMASerial : public JetiExBusSerial {
private:
  UART_HandleTypeDef * huart;
  uint8_t dmaRecvBuffer[JETI_DMA_BUF_SIZE];
  uint8_t dmaTransmitBuffer[JETI_DMA_BUF_SIZE];
  size_t receivedBytes;
  size_t index;
  Callback<void()> _rx_complete;
  Callback<void()> _tx_complete;
  Callback<void(uint32_t)> _uart_error;
  bool busy;
  
 
  
  static JetiExBusDMASerial *current;
  // HAL Callbacks
  static void TxDMA_Complete(UART_HandleTypeDef *huart);
  static void RxDMA_Complete(UART_HandleTypeDef *huart);
  static void RxDMA_Abort(UART_HandleTypeDef *huart);
  static void UART_Error(UART_HandleTypeDef *huart);

  //Helpers
  static void disableTimeoutIRQ(UART_HandleTypeDef *huart);
  bool busyCheck();


public:
  virtual void  begin(uint32_t baud, uint32_t format);
  virtual size_t write(const uint8_t *buffer, size_t size);
  virtual int available(void);
  virtual int read(void);

  void attachTx(Callback<void()> f)
  {
     _tx_complete = f;
  }

  void attachRx(Callback<void()> f)
  {
     _rx_complete = f;
  }

  void attachError(Callback<void(uint32_t)> f)
  {
     _uart_error = f;
  }

  bool Start_Receive_NB();
  bool receiveBlocking(uint32_t timeout);

    // template <typename T, typename R, typename... ArgTs>
    // void  Start_Receive_NB(T *obj, R(T::*method)(ArgTs...), ArgTs... args)
    // {
    //      // TODO: Find out how this works...
    //     //return call_every(ms, mbed::callback(obj, method), args...);
    //     _Start_Receive_NB();
    // }

  

  JetiExBusDMASerial(UART_HandleTypeDef *h):
        huart(h),receivedBytes(0),index(0),busy(true)
  {
    current=this;
  }
    
};


#endif 