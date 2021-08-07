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


typedef enum {
   juOff,
   juPause,
   juRun

} t_juDMAState;

typedef enum {
  juDMA,
  juIT
} t_juTransferMode;

class JetiExBusDMASerial : public JetiExBusSerial {
private:
  UART_HandleTypeDef * huart;
  uint8_t dmaRecvBuffer[JETI_DMA_BUF_SIZE];
  uint8_t dmaTransmitBuffer[JETI_DMA_BUF_SIZE];
  Callback<void()> _rx_complete;
  Callback<void()> _tx_complete;
  Callback<void(uint32_t)> _uart_error;
  t_juDMAState rxState;
  t_juDMAState txState;




  //Rx FIFO helpers
  size_t _readIndex;
  size_t _writeIndex; // only used in IT Mode
  int lastCNDTR;
  bool rxIrqMode;
  t_juTransferMode txMode;
  inline size_t readIndex() { return _readIndex; }
  inline size_t writeIndex() { return  (rxIrqMode)?_writeIndex:JETI_DMA_BUF_SIZE- __HAL_DMA_GET_COUNTER(huart->hdmarx); }



  static JetiExBusDMASerial *current;
  // HAL Callbacks
  static void TxDMA_Complete(UART_HandleTypeDef *huart);
  static void RxDMA_Complete(UART_HandleTypeDef *huart); // Used in DMA Mode
  static void RxIT_Complete(UART_HandleTypeDef *huart);  // Used in IRQ Mode
  static void RxDMA_Abort(UART_HandleTypeDef *huart);
  static void UART_Error(UART_HandleTypeDef *huart);



public:
  virtual void  begin(uint32_t baud, uint32_t format);
  virtual size_t write(const uint8_t *buffer, size_t size);
  virtual int available(void);
  virtual int read(void);

  inline void setTxMode(t_juTransferMode m) { txMode = m; }
  inline t_juTransferMode getTXMode() { return txMode; }

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
  bool Start_Receive_IT();


  bool rxDMAAbort() {
    bool res = rxState!=juOff && HAL_UART_AbortReceive_IT(huart)==HAL_OK;
    if (res) rxState = juOff;
    return res;
  }

  bool txDMAAbort() {
    bool res = txState!=juOff && HAL_UART_AbortTransmit_IT(huart)==HAL_OK;
    if (res) txState = juOff;
    return res;
  }

  bool rxDMAPause()
  {
    if (rxIrqMode) return true;
    bool res = rxState==juRun && HAL_UART_DMAPause(huart)==HAL_OK;
    if (res) rxState=juPause;
    return res;
  }

  bool rxDMAResume()
  {
    if (rxIrqMode) return true;
    bool res = (rxState==juPause) && HAL_UART_DMAResume(huart)==HAL_OK;
    if (res) rxState=juRun;
    return res;
  }


  JetiExBusDMASerial(UART_HandleTypeDef *h):
        huart(h),rxState(juOff),txState(juOff),_readIndex(0),_writeIndex(0),lastCNDTR(0),rxIrqMode(false),txMode(juDMA)
  {
    current=this;
  }

};


#endif