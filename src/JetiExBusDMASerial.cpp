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
#include <string.h>

JetiExBusDMASerial *JetiExBusDMASerial::current=nullptr;


void  JetiExBusDMASerial::TxDMA_Complete(UART_HandleTypeDef *huart)
{

   if (current) {
      current->txState =juOff;
      HAL_HalfDuplex_EnableReceiver(current->huart);
      current->rxDMAResume();
      if (current->_tx_complete)  current->_tx_complete.call();
   }
}

void JetiExBusDMASerial::RxDMA_Complete(UART_HandleTypeDef *huart)
{

   if (current) {
      current->lastCNDTR =  __HAL_DMA_GET_COUNTER(huart->hdmarx);
      if (current->_rx_complete) current->_rx_complete.call();

   }

}

void JetiExBusDMASerial::RxIT_Complete(UART_HandleTypeDef *huart)
{
   if (current) {
      current->_writeIndex = (current->_writeIndex + 1) % JETI_DMA_BUF_SIZE;
      HAL_UART_Receive_IT(huart,current->dmaRecvBuffer+current->_writeIndex,1);
      if (current->_rx_complete) current->_rx_complete.call();
   }
}


void JetiExBusDMASerial::RxDMA_Abort(UART_HandleTypeDef *huart)
{
    if (current)
      current->rxState = juOff;
}

void JetiExBusDMASerial::UART_Error(UART_HandleTypeDef *huart)
{

    uint32_t errorcode =  huart->ErrorCode;
    if (errorcode) {
      HAL_UART_Abort(huart);
      if (current) current->rxState = juOff;
      if ( current && current->_uart_error )  current->_uart_error.call(errorcode);
    }


}

void JetiExBusDMASerial::begin(uint32_t baud, uint32_t format)
{
  // baud and format are dummy only
  HAL_UART_RegisterCallback(huart,HAL_UART_TX_COMPLETE_CB_ID,TxDMA_Complete);

  HAL_UART_RegisterCallback(huart,HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID,RxDMA_Abort);
  HAL_UART_RegisterCallback(huart,HAL_UART_ERROR_CB_ID,UART_Error);
  //__HAL_UART_DISABLE_IT(huart,UART_IT_ERR); //TODO: Check if this can be removed
  //HAL_HalfDuplex_EnableReceiver(huart);


}

size_t JetiExBusDMASerial::write(const uint8_t *buffer, size_t size)
{
HAL_StatusTypeDef res;

    if (txState!=juOff) {
       debug("write called wile last write not complete\n");
       return 0;
    }
    if ( size==0 || size>sizeof(dmaTransmitBuffer) ) {
       debug("write failed because of invalid buffer size\n");
       return 0;
    }
    memcpy(dmaTransmitBuffer,buffer,size);
    rxDMAPause(); // suspend rx while sending
    HAL_HalfDuplex_EnableTransmitter(huart);
    if (txMode==juDMA)
      res=HAL_UART_Transmit_DMA(huart,dmaTransmitBuffer,size);
    else  
      res=HAL_UART_Transmit_IT(huart,dmaTransmitBuffer,size);
   
    if (res!=HAL_OK) {
      debug("Error on DMA write\n");
      // In case of error re-enable rx
      HAL_HalfDuplex_EnableReceiver(huart);
      rxDMAResume();
      return 0;
    } else {
      txState=juRun; 
      return size;
    }

}

bool JetiExBusDMASerial::Start_Receive_DMA()
{
HAL_StatusTypeDef res;

   if (rxState!=juOff) return false;

   HAL_UART_RegisterCallback(huart,HAL_UART_RX_COMPLETE_CB_ID,RxDMA_Complete);
   HAL_HalfDuplex_EnableReceiver(huart);
   _readIndex=0;
   memset(dmaRecvBuffer,0,sizeof(dmaRecvBuffer));
   HAL_NVIC_DisableIRQ(USART3_IRQn);
   res=HAL_UART_Receive_DMA(huart,dmaRecvBuffer,sizeof(dmaRecvBuffer));

   if (res!=HAL_OK) {
     rxState=juOff;
     debug("Error on Start_Receive_NB\n");
     HAL_NVIC_EnableIRQ(USART3_IRQn);
     return false;
   } else {
      rxState=juRun;
      rxIrqMode=false;
      HAL_NVIC_EnableIRQ(USART3_IRQn);
   }

   //debug("Start_Receive_NB succesfull\n");
   return true;
}

bool JetiExBusDMASerial::Start_Receive_IT()
{
HAL_StatusTypeDef res;

   if (rxState!=juOff) return false;

   HAL_UART_RegisterCallback(huart,HAL_UART_RX_COMPLETE_CB_ID,RxIT_Complete);
   _readIndex=0;
   _writeIndex=0;
   memset(dmaRecvBuffer,0,sizeof(dmaRecvBuffer));
   HAL_NVIC_DisableIRQ(USART3_IRQn);
   HAL_HalfDuplex_EnableReceiver(huart);
   res=HAL_UART_Receive_IT(huart,dmaRecvBuffer,1);
   if (res!=HAL_OK) {
     rxState=juOff;
     HAL_NVIC_EnableIRQ(USART3_IRQn);
     return false;
   } else {
      rxState=juRun;
      rxIrqMode=true;
      HAL_NVIC_EnableIRQ(USART3_IRQn);
   }
   return true;

}


int JetiExBusDMASerial::available(void)
{
   if (rxState==juOff) return 0;
   size_t w = writeIndex();
   if (w == _readIndex) return 0; // fifo empty
   return ( w - _readIndex) % JETI_DMA_BUF_SIZE;
}

int JetiExBusDMASerial::read(void)
{
int r;

     if (available()) {
       r= dmaRecvBuffer[_readIndex];
       _readIndex = (_readIndex + 1) % JETI_DMA_BUF_SIZE;
       //MBED_ASSERT((_readIndex < JETI_DMA_BUF_SIZE) && (_readIndex > 0 ));
       return r;
     } else {
       return 0;
     }
}




#endif