#ifndef __JETIEXBUS_FRAMEWORK_H__
#define __JETIEXBUS_FRAMEWORK_H__


#ifdef MBED
    #define USE_HAL_UART_REGISTER_CALLBACKS 1
    #include "mbed.h"
    #include <string.h>
    #include <stdint.h>

    #include "mbed.h"
    #include <chrono>

    // inline uint32_t millis() { 
    //   using namespace std::chrono;
	  // auto now_ms = time_point_cast<milliseconds>(Kernel::Clock::now()); // Convert time_point to one in microsecond accuracy
	  // return  now_ms.time_since_epoch().count();
    // }
    // inline uint32_t micros() { 
    //     using namespace std::chrono;
	  //   auto now_us = time_point_cast<microseconds>(Kernel::Clock::now()); // Convert time_point to one in microsecond accuracy
	  //   return  now_us.time_since_epoch().count();
    //  } // not really microsecond resolution ....

    //  typedef uint8_t byte;
     #define memcpy_P(...) memcpy(__VA_ARGS__)
     #define strlcpy_P(...) strlcpy(__VA_ARGS__)

     inline void debug_print(const char *str) { 
       #ifndef STM32F1xx
       printf(str);
       #endif 
       
     }


#else

    #if ARDUINO >= 100
     #include <Arduino.h>
    #else
     #include <WProgram.h>
    #endif

    #if !defined(ESP32) and !defined(STM32F3xx)
      #include <new.h>
    #endif 
    #define debug_print(...) Serial.println(__VA_ARGS__)

#endif

#endif