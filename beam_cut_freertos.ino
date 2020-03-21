/* 
* creator: ryan-lenguyen 
* date: 18/3/2020
* github: https://github.com/greatboxS/bc_freertos_esp32
* gmail: phuockha0708@gmail.com
*/

#include "global_scope.h"
#include "root_freertos.h"

void setup()
{
    // every thing is done in task
    // easy money!
    root_sys_init();
}

void loop()
{
    esp_task_wdt_reset();
    // every thing is done in task
    vTaskSuspend(NULL);
}