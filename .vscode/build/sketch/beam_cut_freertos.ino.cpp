#include <Arduino.h>
#line 1 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
// creator: ryan-lenguyen - 18/3/2020

#define configMAX_SYSCALL_INTERRUPT_PRIORITY 5

#include "global_scope.h"
#include "root_freertos.h"

#line 8 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
void setup();
#line 14 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
void loop();
#line 8 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
void setup()
{
    root_freertos_init();
    
}

void loop()
{
    // every thing is done in task
    vTaskSuspend(NULL);
}
