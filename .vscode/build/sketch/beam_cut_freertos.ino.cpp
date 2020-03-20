#include <Arduino.h>
#line 1 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
/* 
* creator: ryan-lenguyen 
* date: 18/3/2020
* github: https://github.com/greatboxS/bc_freertos_esp32
* gmail: phuockha0708@gmail.com
*/

#include "global_scope.h"
#include "root_freertos.h"

#line 11 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
void setup();
#line 18 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
void loop();
#line 11 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
void setup()
{
    // every thing is done in task
    // easy money!
    root_sys_init();
}

void loop()
{
    // every thing is done in task
    vTaskSuspend(NULL);
}
