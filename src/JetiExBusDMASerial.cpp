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

#ifdef MBED

#include "JetiExBusDMASerial.h"

JetiExBusDMASerial *JetiExBusDMASerial::current=nullptr;


void  JetiExBusDMASerial::TxDMA_Complete(UART_HandleTypeDef *huart)
{
  
   if (current) {
     // HAL_HalfDuplex_EnableReceiver(current->huart);
      if (current->_tx_complete)  current->_tx_complete.call();
   }
}

void JetiExBusDMASerial::RxDMA_Complete(UART_HandleTypeDef *huart)
{
   if (current && current->_rx_complete) {
      current->_rx_complete.call();
   }

}


void JetiExBusDMASerial::begin(uint32_t baud, uint32_t format)
{
  // baud and format are dummy only
  HAL_UART_RegisterCallback(huart,HAL_UART_TX_COMPLETE_CB_ID,TxDMA_Complete);
  HAL_UART_RegisterCallback(huart,HAL_UART_RX_COMPLETE_CB_ID,RxDMA_Complete);
  HAL_HalfDuplex_EnableReceiver(huart);

}

size_t JetiExBusDMASerial::write(const uint8_t *buffer, size_t size)
{
HAL_StatusTypeDef res;

    HAL_HalfDuplex_EnableTransmitter(huart);
    res=HAL_UART_Transmit_DMA(huart,const_cast<uint8_t*>(buffer),size);
    if (res!=HAL_OK) {
      debug("Error on DMA write");
      return 0;
    } else {
      return size;
    }

}

void JetiExBusDMASerial::_Start_Receive_NB()
{
HAL_StatusTypeDef res;

   HAL_HalfDuplex_EnableReceiver(huart);
   receivedBytes=0;
   res=HAL_UART_Receive_DMA(huart,dmaBuffer,sizeof(dmaBuffer));
   if (res!=HAL_OK) {
     debug("Error on _Start_Receive_NB");
   }
}

int JetiExBusDMASerial::available(void)
{
  return 0;
}

int JetiExBusDMASerial::read(void)
{
  return 0;
}




#endif