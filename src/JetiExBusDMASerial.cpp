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


void JetiExBusDMASerial::disableTimeoutIRQ(UART_HandleTypeDef *huart) {
     __HAL_UART_DISABLE_IT(huart,UART_IT_RTO);
    HAL_UART_DisableReceiverTimeout(huart);
  }

bool JetiExBusDMASerial::busyCheck()
{
   if (busy) 
      debug("JetiExBusDMASerial: new tranaction while still busy\n");
   return busy;
}

void  JetiExBusDMASerial::TxDMA_Complete(UART_HandleTypeDef *huart)
{
  
   if (current) {
      current->busy =false; 
      // HAL_HalfDuplex_EnableReceiver(current->huart);
      if (current->_tx_complete)  current->_tx_complete.call();
   }
}

void JetiExBusDMASerial::RxDMA_Complete(UART_HandleTypeDef *huart)
{
   
   disableTimeoutIRQ(huart);
   if (current) {
      current->busy =false;
      current->receivedBytes = huart->RxXferSize; // Check if this is the right field !!!
      current->index=0;
      if (current->_rx_complete) current->_rx_complete.call();
   }

}

void JetiExBusDMASerial::RxDMA_Abort(UART_HandleTypeDef *huart)
{

}

void JetiExBusDMASerial::UART_Error(UART_HandleTypeDef *huart)
{
   
    uint32_t errorcode =  huart->ErrorCode;
    disableTimeoutIRQ(huart);
    HAL_UART_Abort(huart);   
    if (current && current->busy) {     
      current->busy = false;
      // if (errorcode & HAL_UART_ERROR_RTO) { // Receiver Timeout Error
      //    current->Start_Receive_NB(); // Restart DMA Receive  
      // } else {
      if (current->_uart_error) current->_uart_error.call(errorcode);
      //}      
    } 
    
}

void JetiExBusDMASerial::begin(uint32_t baud, uint32_t format)
{
  // baud and format are dummy only
  HAL_UART_RegisterCallback(huart,HAL_UART_TX_COMPLETE_CB_ID,TxDMA_Complete);
  HAL_UART_RegisterCallback(huart,HAL_UART_RX_COMPLETE_CB_ID,RxDMA_Complete);
  HAL_UART_RegisterCallback(huart,HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID,RxDMA_Abort);
  HAL_UART_RegisterCallback(huart,HAL_UART_ERROR_CB_ID,UART_Error);
  //HAL_HalfDuplex_EnableReceiver(huart);
  busy = false;

}

size_t JetiExBusDMASerial::write(const uint8_t *buffer, size_t size)
{
HAL_StatusTypeDef res;

    if (busyCheck()) return 0;
    if ( size==0 || size>sizeof(dmaTransmitBuffer) ) {
       debug("write failed because of invalid buffer size\n");
       return 0;
    }
    busy = true;
    memcpy(dmaTransmitBuffer,buffer,size);
    HAL_HalfDuplex_EnableTransmitter(huart);
    res=HAL_UART_Transmit_DMA(huart,dmaTransmitBuffer,size); // const_cast<uint8_t*>(
    if (res!=HAL_OK) {
      debug("Error on DMA write\n");
      return 0;
    } else {
      return size;
    }

}

bool JetiExBusDMASerial::Start_Receive_NB()
{
HAL_StatusTypeDef res;

   if (busyCheck()) 
     return false;
   HAL_HalfDuplex_EnableReceiver(huart);
   receivedBytes=0;
   //HAL_UART_ReceiverTimeout_Config(huart,2048);
   //HAL_UART_EnableReceiverTimeout(huart);
   //__HAL_UART_ENABLE_IT(huart,UART_IT_RTO);
   res=HAL_UART_Receive_DMA(huart,dmaRecvBuffer,sizeof(dmaRecvBuffer));
   __HAL_UART_DISABLE_IT(huart,UART_IT_ERR);
   if (res!=HAL_OK) {
     debug("Error on Start_Receive_NB\n");
     return false;
   }
   busy = true;
   //debug("Start_Receive_NB succesfull\n");
   return true;
}

bool JetiExBusDMASerial::receiveBlocking(uint32_t timeout)
{
HAL_StatusTypeDef res;

 if (busyCheck()) 
     return false;
   HAL_HalfDuplex_EnableReceiver(huart);
   receivedBytes=0;
   res=HAL_UART_Receive(huart,dmaRecvBuffer,sizeof(dmaRecvBuffer),timeout);
   if (res) JetiExBusDMASerial::RxDMA_Complete(huart);
   return res==HAL_OK;
}

int JetiExBusDMASerial::available(void)
{
  if (!busy) 
     return receivedBytes-index;
   else  
     return 0;
}

int JetiExBusDMASerial::read(void)
{
     if (!busy && available()) {
       return dmaRecvBuffer[index++];
     } else {
       return 0;
     }  
}




#endif